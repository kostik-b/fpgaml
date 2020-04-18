// Copyright QUB 2019

#include <opencl.h>
#include <string>
#include "ocl_ds.h"
#include <fpgaml.h>

/*
  This file contains the FPGA-specific implementation of some
  OpenCL functions.
*/

#define LOG_BASIC 1
#define LOG_DEBUG 1


#ifdef LOG_BASIC
  #define LOGB(x) printf(x);
#else
  #define LOGB(x)
#endif

#ifdef LOG_DEBUG
  #define LOGD(x)         printf(x);
  #define LOGDV(msg, ...) printf (msg, ##__VA_ARGS__);
#else
  #define LOGD(x)
#endif

extern "C"
{

CL_API_ENTRY cl_int CL_API_CALL
clGetPlatformIDs(cl_uint          num_entries,
                 cl_platform_id*  platforms,
                 cl_uint*         num_platforms) CL_API_SUFFIX__VERSION_1_0
{
  LOGB ("-- In clGetPlatformIDs\n");

  // first handle the special case
  if ((num_entries == 0) && (platforms == NULL) &&
       (num_platforms != NULL))
  {
    *num_platforms = 1;
    return CL_SUCCESS;
  }

  // a static dummy value
  static _cl_platform_id s_platform_id;

  if ((num_entries < 1) && (platforms == NULL))
  {
    return CL_INVALID_VALUE;
  }

  platforms[0] = &s_platform_id;
  if (num_platforms)
  {
    *num_platforms = 1;
  }

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceIDs(cl_platform_id   platform,
               cl_device_type   device_type, 
               cl_uint          num_entries, 
               cl_device_id*    devices,
               cl_uint*         num_devices) CL_API_SUFFIX__VERSION_1_0
{
  LOGB ("-- In clGetDeviceIDs\n");
  // ignore platform, device_type

  // handle special case first
  if ((num_entries == 0) && (devices == NULL) &&
        (num_devices != NULL))
  {
    *num_devices = 1;
    return CL_SUCCESS;
  }

  // a static dummy value
  static _cl_device_id s_device_id;

  if ((num_entries < 1) && (devices == NULL))
  {
    return CL_INVALID_VALUE;
  }

  devices[0] = &s_device_id;
  if (num_devices)
  {
    *num_devices = 1;
  }

  return CL_SUCCESS;
}

CL_API_ENTRY cl_context CL_API_CALL
clCreateContext(const cl_context_properties * properties,
                cl_uint                 num_devices,
                const cl_device_id *    devices,
                void (CL_CALLBACK *     pfn_notify)(const char *, const void *, size_t, void *),
                void *                  user_data,
                cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
  LOGB ("-- In clCreateContext\n");
  // ignore all params for now 

  if (errcode_ret)
  {
    *errcode_ret = CL_SUCCESS;
  }

  return new _cl_context;
}

CL_API_ENTRY cl_program CL_API_CALL
clCreateProgramWithSource(cl_context        context,
                          cl_uint           count,
                          const char **     strings,
                          const size_t *    lengths,
                          cl_int *          errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
  LOGB ("-- In clCreateProgramWithSource\n");
  // TODO: the ultimate idea is to generate a sha256 signature for entire application
  // For now we will simply extract the bitstream file name form the first line
  // of the kernel file.

  if ((count == 0) || (!strings) || (strings[0] == NULL))
  {
    *errcode_ret = CL_INVALID_VALUE;
    return NULL;
  }

  if (!context)
  {
    *errcode_ret = CL_INVALID_CONTEXT;
    return NULL;
  }

  // the xml file name is located on the first line
  const size_t X_LEN = 64;
  char xml_file_name [X_LEN];
  memset (xml_file_name, '\0', X_LEN);

  const size_t first_line_len = ((lengths == NULL) || (lengths[0] == NULL)) ? strlen (strings[0]) : lengths[0];

  int i = 0, copy_counter = 0;
  while ((strings[0][i] != '\n') && (i < (X_LEN - 1)) && (i < first_line_len))
  {
    const char cp = strings[0][i];

    if ((cp != '\n') && (cp != '/') && (cp != ' '))
    {
      xml_file_name[copy_counter++] = cp;
    }
    ++i;
  }

  LOGDV ("-- Extracted xml file name %s\n", xml_file_name);
  _cl_program* prog = new _cl_program (context);
  prog->m_xml_name = xml_file_name;

// the below is not used
#if 0
  // 1. generate the sha2 key using the first "string"
  // 2. create a program struct and return it
  std::string src_str (strings[0], lengths[0]);
  _cl_program* prog = new _cl_program (context);
  picosha2::hash256_hex_string (src_str, prog->m_sha2);

  LOGDV ("-Generated SHA256 with signature %s\n", prog->m_sha2.c_str());
#endif
  return prog;
}

CL_API_ENTRY cl_int CL_API_CALL
clBuildProgram(cl_program           program,
               cl_uint              num_devices,
               const cl_device_id * device_list,
               const char *         options, 
               void (CL_CALLBACK *  pfn_notify)(cl_program /* program */, void * /* user_data */),
               void *               user_data) CL_API_SUFFIX__VERSION_1_0
{
  LOGB ("-- In clBuildProgram\n");
  // the idea here is to check whether a given bitstream is present in the bitstream manager

  FPML_RC rc = ml_check_bitstream_exists (program);
  if (rc != FPML_SUCCESS)
  {
    program->m_build_log = "ERROR No bitstream file " + program->m_xml_name + " exists";
    return CL_BUILD_PROGRAM_FAILURE;
  }
  else
  {
    program->m_build_log = "Bitstream " + program->m_xml_name + " is present in bitstream manager";
  }

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetProgramBuildInfo(cl_program            program,
                      cl_device_id          device,
                      cl_program_build_info param_name,
                      size_t                param_value_size,
                      void *                param_value,
                      size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
  LOGB ("-- In clGetProgramBuildInfo\n");

  if (!program)
  {
    return CL_INVALID_PROGRAM;
  }
  // we only cater for log query
  switch (param_name)
  {
    case CL_PROGRAM_BUILD_LOG:
      {
        if ((param_value_size < 1) && (param_value_size_ret)) // what is the size of the return value?
        {
          *param_value_size_ret = program->m_build_log.size ();
          return CL_SUCCESS;
        }
        else if (param_value)
        {
          const size_t log_size   = program->m_build_log.size();
          const size_t copy_size  = (log_size > param_value_size - 1) ?
                                      param_value_size - 1 : log_size;
          memset (param_value, '\0', param_value_size);
          strncpy ((char*)param_value, program->m_build_log.c_str(), copy_size);
        }
        else
        {
          return CL_INVALID_VALUE;
        }
      }
      break;
    case CL_PROGRAM_BUILD_STATUS:
    case CL_PROGRAM_BUILD_OPTIONS:
    case CL_PROGRAM_BINARY_TYPE:
      printf ("OCL_API: cl_program_build_info not fully implemented yet\n");
      break;
    default:
      printf ("OCL_API: wrong cl_program_info\n");
      return CL_INVALID_VALUE;
      break;
  }
}

CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_2_DEPRECATED cl_command_queue CL_API_CALL
clCreateCommandQueue(cl_context                     context,
                     cl_device_id                   device,
                     cl_command_queue_properties    properties,
                     cl_int *                       errcode_ret)
{
  LOGB ("-- In clCreateCommandQueue\n");
  // TODO: ask fpgaml allocate us a new buffer

  return ml_create_cmd_q (device);
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateBuffer(cl_context   context,
               cl_mem_flags flags,
               size_t       size,
               void *       host_ptr,
               cl_int *     errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
  LOGB ("-- In clCreateBuffer\n");

  cl_mem mem = new _cl_mem;
  context->add_mem_obj (mem);

  // some sanity check first
  if ((flags & CL_MEM_USE_HOST_PTR)  && (flags & CL_MEM_ALLOC_HOST_PTR))
  {
    if (errcode_ret)
    {
      *errcode_ret = CL_INVALID_VALUE;
    }
    return NULL;
  }

  // if the flag is CL_MEM_USE_HOST_PTR then create a cmd queue,
  // enqueue the command and destroy it afterwards
  if ((host_ptr != NULL) && (flags & CL_MEM_USE_HOST_PTR))
  {
    cl_command_queue cmdq = clCreateCommandQueue (context, NULL, 0, NULL);

    cl_int rc = clEnqueueWriteBuffer (cmdq, mem, CL_TRUE, 0, size, host_ptr, 0, NULL, NULL);

    if (errcode_ret)
    {
      *errcode_ret = rc;
    }

    clReleaseCommandQueue (cmdq);
  }

  if (flags & CL_MEM_ALLOC_HOST_PTR)
  {
    // basically enqueue an empty buffer
    cl_command_queue cmdq = clCreateCommandQueue (context, NULL, 0, NULL);

    // TODO: make sure that the driver allocates an empty buffer of that size
    cl_int rc = clEnqueueWriteBuffer (cmdq, mem, CL_TRUE, 0, size, NULL, 0, NULL, NULL);

    if (errcode_ret)
    {
      *errcode_ret = rc;
    }

    clReleaseCommandQueue (cmdq);
  }

  return mem;
}

CL_API_ENTRY cl_kernel CL_API_CALL
clCreateKernel(cl_program      program,
               const char *    kernel_name,
               cl_int *        errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
  LOGB ("-- In clCreateKernel\n");
  if (!program)
  {
    if (errcode_ret)
    {
      *errcode_ret = CL_INVALID_PROGRAM;
    }
    return NULL;
  }
  return new _cl_kernel (program);
}

CL_API_ENTRY cl_int CL_API_CALL
clSetKernelArg(cl_kernel    kernel,
               cl_uint      arg_index,
               size_t       arg_size,
               const void * arg_value) CL_API_SUFFIX__VERSION_1_0
{
  LOGB ("-- In clSetKernelArg\n");
  if (!kernel)
  {
    return CL_INVALID_KERNEL;
  }

  // TODO check the validity of other params
  kernel->set_args (arg_index, arg_size, arg_value);

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueNDRangeKernel(cl_command_queue command_queue,
                       cl_kernel        kernel,
                       cl_uint          work_dim,
                       const size_t *   global_work_offset,
                       const size_t *   global_work_size,
                       const size_t *   local_work_size,
                       cl_uint          num_events_in_wait_list,
                       const cl_event * event_wait_list,
                       cl_event *       event) CL_API_SUFFIX__VERSION_1_0
{
  LOGB ("-- In clEnqueueNDRangeKernel\n");
  if (!command_queue)
  {
    return CL_INVALID_COMMAND_QUEUE;
  }
  if (!kernel)
  {
    return CL_INVALID_KERNEL;
  }
  // we probably want to limit it just to 1 dimension for now
  if (work_dim > 1)
  {
    return CL_INVALID_WORK_DIMENSION;
  }

  if ((!local_work_size) || (local_work_size[0] < 1))
  {
    return CL_INVALID_WORK_GROUP_SIZE;
  }

  if (ml_check_wg_size (kernel->get_program()->m_xml_name, local_work_size[0]) != FPML_SUCCESS)
  {
    LOGDV("ERROR: Local work size %u is incorrect\n", local_work_size[0]);
    return CL_INVALID_WORK_GROUP_SIZE;
  }
  if (global_work_size[0]%local_work_size[0] != 0)
  {
    LOGDV("ERROR: Global work size %u / local work size %u are incorrect\n",
          global_work_size[0], local_work_size[0]);
    return CL_INVALID_GLOBAL_WORK_SIZE;
  }

  // 1. create an nd range command
  MLCLCommand* cmd = new MLCLCommandNDR (kernel, work_dim, global_work_offset,
                                         global_work_size, local_work_size);
  // 2. enqueue that command to the command queue
  uint32_t command_id = 0;
  if (ml_submit_cmd (command_queue, cmd, command_id) == FPML_ERR_INVALID_CMDQ)
  {
    return CL_INVALID_COMMAND_QUEUE;
  }

  // TODO: now need to set the command_id to event
  // which will be used later to query the status of
  // the command

  return CL_SUCCESS;
}

static cl_int handle_blocking_non_blocking (
  const bool  blocking_cmd,
  cl_event    cmd_event,
  cl_event*   user_event)
{
  if (blocking_cmd)
  {
    cmd_event->wait_until_complete ();
    cl_int error_code = cmd_event->get_error_code ();

    if (!user_event)
    {
      cmd_event->ref_cnt_down ();
    }
    else
    {
      *user_event = cmd_event;
    }
    return error_code;
  }
  else
  {
    if (user_event)
    {
      *user_event = cmd_event;
    }
    else if (cmd_event) // if the user did not request an event, we need to delete the one we have
    {
      cmd_event->ref_cnt_down ();
    }
    return CL_SUCCESS;
  }
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteBuffer(cl_command_queue   command_queue, 
                     cl_mem             buffer, 
                     cl_bool            blocking_write, 
                     size_t             offset, 
                     size_t             size, 
                     const void *       ptr, 
                     cl_uint            num_events_in_wait_list, 
                     const cl_event *   event_wait_list, 
                     cl_event *         event) CL_API_SUFFIX__VERSION_1_0
{
  LOGB ("-- In clEnqueueWriteBuffer\n");
  if (!command_queue)
  {
    return CL_INVALID_COMMAND_QUEUE;
  }
  if (!buffer)
  {
    return CL_INVALID_MEM_OBJECT;
  }

  // 1. create a write buffer command
  MLCLCommand* cmd = new MLCLCommandWB (buffer, offset, size, ptr);

  // 2. submit that command to the command queue
  cl_event cmd_event = cmd->get_event ();
  cmd_event->ref_cnt_up ();

  uint32_t command_id = 0;
  if (ml_submit_cmd (command_queue, cmd, command_id) == FPML_ERR_INVALID_CMDQ)
  {
    if (cmd_event)
    {
      cmd_event->ref_cnt_down ();
    }
    return CL_INVALID_COMMAND_QUEUE;
  }

  // NB: don't use "cmd" anymore - it might have been deleted

  return handle_blocking_non_blocking (blocking_write, cmd_event, event);
}

CL_API_ENTRY cl_int CL_API_CALL
clFinish(cl_command_queue command_queue)
{
  // check if the command queue is valid and issue a blocking command
  if (!command_queue)
  {
    return CL_INVALID_COMMAND_QUEUE;
  }

  MLCLCommand* cmd = new MLCLCommand (MLCLCommand::EMPTY);

  cl_event cmd_event = cmd->get_event ();
  cmd_event->ref_cnt_up ();

  uint32_t command_id = 0;
  if (ml_submit_cmd (command_queue, cmd, command_id) == FPML_ERR_INVALID_CMDQ)
  {
    cmd_event->ref_cnt_down ();
    return CL_INVALID_COMMAND_QUEUE;
  }

  // NB: don't use "cmd" anymore - it might have been deleted

  return handle_blocking_non_blocking (true, cmd_event, NULL);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadBuffer(cl_command_queue    command_queue,
                    cl_mem              buffer,
                    cl_bool             blocking_read,
                    size_t              offset,
                    size_t              size, 
                    void *              ptr,
                    cl_uint             num_events_in_wait_list,
                    const cl_event *    event_wait_list,
                    cl_event *          event) CL_API_SUFFIX__VERSION_1_0
{
  LOGB ("-- In clEnqueueReadBuffer\n");
  if (!command_queue)
  {
    return CL_INVALID_COMMAND_QUEUE;
  }
  if (!buffer)
  {
    return CL_INVALID_MEM_OBJECT;
  }

  // 1. create a write buffer command
  MLCLCommand* cmd = new MLCLCommandRB (buffer, offset, size, ptr);
  // 2. submit that command to the command queue
  cl_event cmd_event = cmd->get_event ();
  cmd_event->ref_cnt_up ();

  uint32_t command_id = 0;
  if (ml_submit_cmd (command_queue, cmd, command_id) == FPML_ERR_INVALID_CMDQ)
  {
    cmd_event->ref_cnt_down ();
    return CL_INVALID_COMMAND_QUEUE;
  }

  // NB: don't use "cmd" anymore - it might have been deleted

  return handle_blocking_non_blocking (blocking_read, cmd_event, event);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseKernel(cl_kernel   kernel) CL_API_SUFFIX__VERSION_1_0
{
  LOGB ("-- In clReleaseKernel\n");
  // decrement reference counter
  if (!kernel)
  {
    return CL_INVALID_KERNEL;
  }

  kernel->ref_cnt_down ();
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseMemObject(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0
{
  LOGB ("-- In clReleaseMemObject\n");
  // decrement reference counter
  if (!memobj)
  {
    return CL_INVALID_MEM_OBJECT;
  }

  memobj->ref_cnt_down ();
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseCommandQueue(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
  LOGB ("-- In clReleaseCommandQueue\n");
  // decrement reference counter
  if (!command_queue)
  {
    return CL_INVALID_COMMAND_QUEUE;
  }

  ml_cmd_q_ref_cnt_down (command_queue);
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseProgram(cl_program program) CL_API_SUFFIX__VERSION_1_0
{
  LOGB ("-- In clReleaseProgram\n");
  // decrement reference counter
  if (!program)
  {
    return CL_INVALID_PROGRAM;
  }

  program->ref_cnt_down ();
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseContext(cl_context context) CL_API_SUFFIX__VERSION_1_0
{
  LOGB ("-- In clReleaseContext\n");
  // decrement reference counter
  if (!context)
  {
    return CL_INVALID_CONTEXT;
  }

  context->ref_cnt_down ();
}

CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceInfo(cl_device_id    device,
                cl_device_info  param_name, 
                size_t          param_value_size, 
                void *          param_value,
                size_t *        param_value_size_ret)
{
  if (!param_value)
  {
    return CL_INVALID_VALUE;
  }

  const char* name    = "FPGA_ACCEL";
  const char* vendor  = "ECOSCALE Consortium";

  switch (param_name)
  {
    case CL_DEVICE_NAME:
      if (param_value_size < strlen (name) + 1)
      {
        return CL_INVALID_VALUE;
      }
      strncpy ((char*)param_value, name, strlen (name) + 1);
      break;
    case CL_DEVICE_VENDOR:
      if (param_value_size < strlen (vendor) + 1)
      {
        return CL_INVALID_VALUE;
      }
      strncpy ((char*)param_value, vendor, strlen (vendor) + 1);
      break;
    case CL_DEVICE_TYPE:
      {
        const unsigned dev_type = CL_DEVICE_TYPE_ACCELERATOR;
        if (param_value_size < sizeof (dev_type))
        {
          return CL_INVALID_VALUE;
        }
        memcpy (param_value, &dev_type, sizeof (dev_type));
      }
      break;
/*
    case CL_DEVICE_MAX_COMPUTE_UNITS:
      {
        const int mm = 4;
        if (param_value_size < sizeof (mm))
        {
          return CL_INVALID_VALUE;
        }
        memcpy (param_value, &mm, sizeof (mm));
      }
      break;
    case CL_DEVICE_MAX_WORK_GROUP_SIZE:
      {
        // hack - this is just for bitstream_2.xml
        const size_t max_wg_size = 1000;
        if (param_value_size < sizeof (max_wg_size))
        {
          return CL_INVALID_VALUE;
        }
        memcpy (param_value, &max_wg_size, sizeof (max_wg_size));
      }
*/
    default:
      return CL_INVALID_VALUE;
      break;
  }

  return CL_SUCCESS;
}


} // extern "C"
