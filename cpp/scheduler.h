// Copyright QUB 2018

#ifndef ecoscale_scheduler_H
#define ecoscale_scheduler_H

#include <string>
#include <circ_array.hpp>
#include "ndrange.h"
#include "fpgaml_rc.h"
#include "ev_timer.h"

namespace FPGAML
{

// each scheduler has a FSM to move through the command sequence
class Scheduler
{
public:
  Scheduler (unsigned long long Board_ID,
             unsigned           FPGA_ID,
             unsigned           PR_ID,
             const double       polling_interval);

  ~Scheduler();

  unsigned  id                 () { return m_id; }
  unsigned  get_remaining_time ();
  bool      would_need_reconf  (const std::string& kernel_name);
  RC        submit_wgr         (WorkGroupRange&    wgr);

  void      on_timer           ();

  bool      is_local           () { return m_is_local; }

private:
  RC        process_wgr        (WorkGroupRange&    wgr);

private:
  SchedulerEvTimer  m_timer;
  // should have a queue here
  typedef CircArray<WorkGroupRange>   WGRBuffer;
  // TODO: in the future
  //typedef FSM<SchedulerAction>        SchedulerFSM;

  WGRBuffer         m_in_queue;
  const unsigned    m_id;

  std::string       m_last_kernel;

public:
  // mock values here
  //unsigned          m_mock_remaining_time;
  bool                m_mock_would_need_reconf;

  ev_tstamp           m_end_time;

  unsigned long long  m_sched_addr;
  unsigned long long  m_dcplr_addr;
  unsigned long long  m_mlbx_wraddr;
  unsigned long long  m_mlbx_rdaddr;

  unsigned long long  m_ICAP_addr;	

  unsigned long long  m_mem_base;

  bool                m_is_local;

  double              m_polling_interval;

}; // class Scheduler

} // namespace FPGAML

#endif
