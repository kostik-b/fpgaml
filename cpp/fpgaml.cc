// Copyright QUB 2018

#include "fpgaml.h"
#include "ndrange.h"
#include "the_buffer.h"
#include <vector>
#include <tuple>
#include "cmd_q.h"
#include <atomic>
#include <centrum.h>

// this id allows to uniquely identify a command
// in the system. this is needed for the purpose
// of support of CLEvents.
static std::atomic<uint32_t> s_unique_command_id (13);

#if 0
ML_RC ml_submit_ndrange (
  const char*     kernel,
  const unsigned  size_x,
  const unsigned  size_y,
  const unsigned  size_z,
  void**          data_ptrs,
  size_t          num_data_ptrs,
  size_t*         data_ptrs_lens)
{
  // construct an NDRange and submit to the RingBuffer
  std::vector<FPGAML::KernelArg> arguments;
  for (int i = 0; i < num_data_ptrs; ++i)
  {
    arguments.push_back (std::make_tuple(data_ptrs[i], data_ptrs_lens[i]));
  }

  FPGAML::NDRange* ndrange = NULL;
  try
  {
    ndrange = new FPGAML::NDRange (kernel, arguments,
                                                  size_x, size_y, size_z);
  }
  catch (std::runtime_error& err)
  {
    return ML_ERR_WRONG_DIMS;
  }

  FPGAML::TheBuffer& the_buffer = FPGAML::TheBuffer::get_instance ();

  if (the_buffer.is_full ())
  {
    return ML_ERR_BUFFER_FULL;
  }

  the_buffer.push_back (ndrange);

  FPGAML::Signal::get_instance ().signal_new_ndrange ();

  return ML_SUCCESS;
}
#endif
_cl_command_queue* ml_create_cmd_q (cl_device_id device/*ignored for now*/)
{
  // create one
  return _cl_command_queue::create_new ();  
}

void ml_cmd_q_ref_cnt_down (_cl_command_queue*  cmdq)
{
  if (!cmdq)
  {
    return;
  }

  cmdq->ref_cnt_down ();
}

FPML_RC ml_submit_cmd (
  _cl_command_queue*  cmdq,
  MLCLCommand*        cmd,
  uint32_t&           command_id)
{
  if ((!cmdq) || (!cmd))
  {
    return FPML_ERR_WRONG_PARAMS;
  }

  command_id = ++s_unique_command_id;

  cmd->set_command_id (command_id);

  FPGAML::RC rc = cmdq->push_back (cmd);
  if (rc == FPGAML::RC::ML_ERROR_CMDQ_RELEASED)
  {
    return FPML_ERR_INVALID_CMDQ;
  }

  return FPML_SUCCESS;
}

FPML_RC ml_check_bitstream_exists (cl_program prog)
{
  // 1. get bitstream manager
  FPGAML::BitstreamCheck& bits_check = FPGAML::Global::get_instance().get_bits_check();

  // 2. get target for a given xml name
  if (bits_check.check_bitstream_exists (prog->m_xml_name))
  {
    return FPML_SUCCESS;
  }
  else
  {
    return FPML_NO_BITSTREAM;
  }
}


FPML_RC ml_check_wg_size (
  const std::string&  xml_name,
  const size_t        wg_size)
{
  FPGAML::WorkGroupSizeCheck& wg_size_check = FPGAML::Global::get_instance().get_wg_size_check();

  // 2. get target for a given xml name
  if (wg_size_check.check_wg_size (xml_name, wg_size))
  {
    return FPML_SUCCESS;
  }
  else
  {
    return FPML_WRONG_WG_SIZE;
  }

}

