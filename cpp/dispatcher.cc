// Copyright QUB 2018

#include "dispatcher.h"
#include "the_buffer.h"
#include "cmd_q.h"

#include <cpp/WeeLogger.h>

#include "centrum.h"

FPGAML::Dispatcher::Dispatcher (FreeSchedulersManager& free_sched_mgr,
                                const size_t           num_sched)
  : m_read_timer    (*this)
  , m_algo_slicing  (30, 2)
  , m_algo_simple   (num_sched)
{
  // TODO
  // const char* config_path = getenv ("FPGAML_CONFIG");

  // m_algo_type = SchedAlgoType::ALGO_SLICING;
  m_algo_type = SchedAlgoType::ALGO_SIMPLE;

  free_sched_mgr.register_on_free_schedulers_cb (&m_algo_slicing);
}

FPGAML::RC FPGAML::Dispatcher::try_invoke_algo_slicing (NDRange* ndrange)
{
  LOG_LOC_DEBUG ("Invoking algo slicing");
  return m_algo_slicing.route_ndrange (ndrange);
}

FPGAML::RC FPGAML::Dispatcher::try_invoke_algo_simple (NDRange* ndrange)
{
  LOG_LOC_DEBUG ("Invoking algo simple");
  return m_algo_simple.route_ndrange (ndrange);
}

static cl_int check_read_write_params (
  cl_mem        mem,
  const size_t  size,
  const void*   ptr,
  const char*   cmd_type)
{
  if (!mem)
  {
    LOG_LOC_ERROR ("Received %s command with no cl_mem object!", cmd_type);
    return CL_INVALID_MEM_OBJECT;
  }

  if (size < 1)
  {
    LOG_LOC_ERROR ("Received %s command with size less than 1!", cmd_type);
    return CL_INVALID_VALUE;
  }


  return CL_SUCCESS;
}

void FPGAML::Dispatcher::handle_read_command (
  MLCLCommandRB* rb_cmd,
  cl_int&        err_code)
{
  if (cl_int rc = check_read_write_params (rb_cmd->m_mem,
                                rb_cmd->m_size,
                                rb_cmd->m_ptr,
                                "Read") != CL_SUCCESS)
  {
    err_code = rc;
    return;
  }

  cl_mem mem = rb_cmd->m_mem;

  LOG_LOC_DEBUG ("In handle_read_command. unimem size is %d", mem->get_size ());

  if (mem->get_unimem_ptr().m_ptr == NULL)
  {
    LOG_LOC_ERROR ("Received a Read Command with NULL unimem ptr");
    err_code = CL_INVALID_MEM_OBJECT;
    return;
  }

  if (rb_cmd->m_ptr == NULL)
  {
    LOG_LOC_ERROR ("Received read command with an empty host pointer!");
    err_code = CL_INVALID_VALUE;
    return;
  }

  if (rb_cmd->m_size != mem->get_size ())
  {
    LOG_LOC_ERROR ("Received a Read Command with differing sizes");
    err_code = CL_INVALID_VALUE;
    return;
  }

  UnimemManager& u_mngr = Global::get_instance().get_unimem_manager ();

  if (!u_mngr.copy_from_unimem (rb_cmd->m_ptr, mem->get_unimem_ptr (), mem->get_size ()))
  {
    err_code = CL_MEM_OBJECT_ALLOCATION_FAILURE;
    return;
  }

  err_code = CL_SUCCESS;
}

void FPGAML::Dispatcher::handle_write_command (
  MLCLCommandWB* wb_cmd,
  cl_int&        err_code)
{
  // 1. extract cl_mem
  // 2. do the memory copying via unimem manager
  // 3. update cl_mem with the results
  if (cl_int rc = check_read_write_params (wb_cmd->m_mem,
                                wb_cmd->m_size,
                                wb_cmd->m_ptr,
                                "Write") != CL_SUCCESS)
  {
    err_code = rc;
    return;
  }

  LOG_LOC_DEBUG ("In handle_write_command. m_size is %d", wb_cmd->m_size);

  cl_mem mem = wb_cmd->m_mem;

  UnimemManager& u_mngr = Global::get_instance().get_unimem_manager ();

  UnimemPtr u_ptr = u_mngr.allocate_unimem (wb_cmd->m_size);
  if (!u_ptr.m_ptr)
  {
    LOG_LOC_ERROR ("Could not allocate a unimem pointer!");
    err_code = CL_MEM_OBJECT_ALLOCATION_FAILURE;
    return;
  }

  // if the host pointer is NULL we simply need to allocate a buffer
  // without copying into it
  if (wb_cmd->m_ptr != NULL)
  {
    // try copying the data, if it fails, we need to deallocate
    // the buffer
    if (!u_mngr.copy_to_unimem (u_ptr, wb_cmd->m_ptr, wb_cmd->m_size))
    {
      u_mngr.release_unimem (u_ptr);
      LOG_LOC_ERROR ("Could not copy to unimem buffer");

      err_code = CL_MEM_OBJECT_ALLOCATION_FAILURE;
      return;
    }
  }

  mem->set_unimem_ptr (u_ptr, wb_cmd->m_size);

  err_code = CL_SUCCESS;
}

void FPGAML::Dispatcher::handle_ndrange_command (MLCLCommandNDR* ndr_cmd)
{
  // create and NDRange object and set the cl_kernel internally
  // get resource string also
  if (ndr_cmd->m_kernel == NULL)
  {
    LOG_LOC_ERROR ("No kernel in the MLCLCommandNDR");
    return;
  }
  if (ndr_cmd->m_kernel->get_program() == NULL)
  {
    LOG_LOC_ERROR ("No program in the MLCLCommandNDR");
    return;
  }

  std::string resource_string;
  Global::get_instance ().get_bits_manager().get_resource_string (ndr_cmd->m_kernel->get_program()->m_xml_name, resource_string);

  const unsigned num_wgs = ndr_cmd->m_global_work_size[0]/ndr_cmd->m_local_work_size[0];

  LOG_LOC_DEBUG ("Num wgs is %u", num_wgs);

  NDRange* ndrange = new NDRange(ndr_cmd->m_kernel,
                                 ndr_cmd->get_event (),
                                 resource_string,
                                 num_wgs,
                                 1,  // y-size
                                 1); // z-size

  m_algo_simple.route_ndrange (ndrange);
}

void FPGAML::Dispatcher::handle_a_cmd  (MLCLCommand* cmd)
{
  LOG_LOC_DEBUG ("Handling a new command");

  // 1. if it is a write buffer command, we place the data into the memory
  //    and update the mem_obj with unimem coordinates of the data buffer
  // 2. it if is a read buffer command, we use the unimem data coordinates
  //    to copy the data into the user's space
  // 3. if it is an NDRange command, we pass it on to the algo_simple (for now)

  MLCLCommand::Type type = cmd->get_command_type ();

  switch (type)
  {
    case MLCLCommand::WRITE_BUFFER:
      {
        MLCLCommandWB* wb_cmd = static_cast<MLCLCommandWB*>(cmd);

        cl_int err_code = CL_SUCCESS;
        handle_write_command (wb_cmd, err_code);

        wb_cmd->get_event()->set_error (err_code);
        wb_cmd->get_event()->set_as_complete ();
      }
      break;
    case MLCLCommand::READ_BUFFER:
      {
        MLCLCommandRB* rb_cmd = static_cast<MLCLCommandRB*>(cmd);

        cl_int err_code = CL_SUCCESS;
        handle_read_command (rb_cmd, err_code);

        rb_cmd->get_event()->set_error (err_code);
        rb_cmd->get_event()->set_as_complete ();
      }
      break;
    case MLCLCommand::NDRANGE:
      {
        MLCLCommandNDR* ndr_cmd = static_cast<MLCLCommandNDR*>(cmd);
        handle_ndrange_command (ndr_cmd);
      }
      break;
    case MLCLCommand::EMPTY:
      // do exactly nothing
      break;
    default:
      LOG_LOC_DEBUG ("Command type \"%d\" not supported", type);
      break;
  }
}

void FPGAML::Dispatcher::read_from_the_buffer ()
{
  TheBuffer& the_buffer = TheBuffer::get_instance();
  NDRange*   ndrange    = the_buffer.peek_front ();

  if (!ndrange)
  {
    return;
  }

  LOG_LOC_DEBUG ("Got NDRange %s from The Buffer", ndrange->get_kernel_name().c_str ());

  // depending on the scheduling algorithm selected submit the ndrange to
  // one of the algos

  if (m_algo_type == SchedAlgoType::ALGO_SLICING)
  {
    RC rc = try_invoke_algo_slicing (ndrange);
    if (rc != RC::ML_SUCCESS)
    {
      LOG_LOC_ERROR ("ERROR - could not submit ndrange to algo slicing");
      return;
    }
  }
  else
  {
    // defaulting to algo_simple
    RC rc = try_invoke_algo_simple (ndrange);
    if (rc != RC::ML_SUCCESS)
    {
      LOG_LOC_ERROR ("ERROR - could not submit ndrange to algo simple");
      return;
    }
  }

  // we can now remove the ndrange off the buffer
  the_buffer.pop_front ();

  if (!the_buffer.is_empty ())
  {
    m_read_timer.start_timer (0.001f, 0.0); // fire in 1 ms, no repeat
  }
}
