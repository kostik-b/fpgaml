// Copyright QUB 2018

#ifndef ecoscale_dispatcher_H
#define ecoscale_dispatcher_H

#include "ev_timer.h"
#include "ndrange.h"
#include "fpgaml_rc.h"
#include "algo_slicing.h"
#include "algo_simple.h"
#include <ocl_ds.h>

class _cl_command_queue;

namespace FPGAML
{

enum class SchedAlgoType
{
  ALGO_SLICING,
  ALGO_SIMPLE
};

class Dispatcher
{
public:
  Dispatcher (FreeSchedulersManager& free_sched_mgr,
              const size_t           num_sched);

  // deprecated - not used anymore
  void  read_from_the_buffer    ();

  void  handle_a_cmd            (MLCLCommand*     cmd);

private:
  RC    try_invoke_algo_slicing (NDRange*         ndrange);

  RC    try_invoke_algo_simple  (NDRange*         ndrange);

  void  handle_read_command     (MLCLCommandRB*   rb_cmd,
                                 cl_int&          err_code);
  void  handle_write_command    (MLCLCommandWB*   wb_cmd,
                                 cl_int&          err_code);
  void  handle_ndrange_command  (MLCLCommandNDR*  ndr_cmd);

private:
  TheBufferReadTimer  m_read_timer;

  SchedAlgoType       m_algo_type;
  AlgoSlicing         m_algo_slicing;  
  AlgoSimple          m_algo_simple;
}; // class Dispatcher

} // namespace FPGAML

#endif
