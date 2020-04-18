// Copyright QUB 2018

#ifndef ecoscale_qub_fpgaml_H
#define ecoscale_qub_fpgaml_H


/*
  In short, all the complexity of the FPGA ML
  is hidden behind this facade. I.e. we are specifically not
  exposing any internals, i.e. command queue is an internal
  data structure, so we are not exposing it. MLCLCommand etc
  are not internal, so we keep it in the IL part.

*/

#include <unistd.h>
#include <ocl_ds.h>

enum FPML_RC
{
  FPML_SUCCESS = 0,
  FPML_ERR_BUFFER_FULL = -1,
  FPML_ERR_WRONG_DIMS = -2 ,
  FPML_ERR_WRONG_PARAMS = -3,
  FPML_ERR_INVALID_CMDQ = -4,
  FPML_NO_BITSTREAM = -5,
  FPML_BUFFER_ALLOC_ERROR = -6,
  FPML_WRONG_WG_SIZE = -7
};

#if 0
// Copy a buffer from userpace to kernel space and return a ptr to it
void* ml_copy_to_unimem   (const char* buffer_ptr, const size_t buffer_size);
// populate the buffer_ptr from the the unimem_ptr, buffer_size is the same
// for both ptrs
int   ml_copy_from_unimem (char*       buffer_ptr, void*        unimem_ptr, const size_t buffer_size);
#endif

// submit an ndrange for execution
/*
ML_RC ml_submit_ndrange  (const char*     kernel,
                          const unsigned  size_x,
                          const unsigned  size_y,
                          const unsigned  size_z,
                          void**          data_ptrs,
                          size_t          num_data_ptrs,
                          size_t*         data_ptrs_lens);
*/

_cl_command_queue*  ml_create_cmd_q (cl_device_id device/*ignored for now*/);

void                ml_cmd_q_ref_cnt_down
                                    (_cl_command_queue*  cmdq);

FPML_RC             ml_submit_cmd   (_cl_command_queue*  cmdq,
                                     MLCLCommand*        cmd,
                                     uint32_t&           command_id);

FPML_RC             ml_check_bitstream_exists
                                    (cl_program          prog);

FPML_RC             ml_check_wg_size
                                    (const std::string&  xml_name,
                                     const size_t        wg_size);
// TODO ml_check_cmd_status (ml_id);

#endif
