// Copyright QUB 2018

#include "scheduler.h"
#include "centrum.h"
#include <cpp/WeeLogger.h>

#if 0
FPGAML::Scheduler::Scheduler (
  unsigned long long Board_ID,
  unsigned           FPGA_ID,
  unsigned           PR_ID,
  const double       polling_interval)
  : m_in_queue   (100) // an arbitrary number
  , m_id         (Board_ID*16 + (FPGA_ID-1)*4 + PR_ID)
  , m_timer      (*this)
{
  //m_mock_remaining_time     = 0;
  m_mock_would_need_reconf  = false;
  m_end_time                = 0.0f;

  LOG_INFO ("Created Scheduler with BiD - %llu, FPGA ID - %u, PR ID - %u, m_id - %u",
              Board_ID, FPGA_ID, PR_ID, m_id);

  m_is_local = ((Board_ID == 0) && (FPGA_ID == 1));
}
#endif

unsigned FPGAML::Scheduler::get_remaining_time ()
{
  if ((m_end_time - ev_now (EV_DEFAULT)) < 0.0f)
  {
    return 0;
  }
  else
  {
    return unsigned (m_end_time - ev_now (EV_DEFAULT));
  }
}

bool FPGAML::Scheduler::would_need_reconf (const std::string& kernel_name)
{
  return m_mock_would_need_reconf;
}

FPGAML::RC FPGAML::Scheduler::submit_wgr (WorkGroupRange&    wgr)
{
  // 0. add the workgroup range to a ring buffer
  // 1. if the buffer is not empty and timer is active - do nothing.
  // 2. if the buffer is empty - schedule the timer for the duration needed

  m_in_queue.push_back (wgr);

  if (!m_timer.is_active () && (m_in_queue.occupancy () < 2)) 
  {
    m_end_time = ev_now (EV_DEFAULT) + double(wgr.get_execution_cost ())/1000.0f;
    m_timer.start_timer (double(wgr.get_execution_cost ())/1000.0f, 0.0f/*no repeat*/);
    LOG_LOC_DEBUG ("ID: %d. WGR (%s) submitted, set the timer for %f s",
                    m_id, wgr.get_kernel_name().c_str(),
                    double(wgr.get_execution_cost ())/1000.0f);
  }
 

#if 0
  // increase the timer value by the newly arrived wgr's value
  if (!m_mock_timer.is_active ())
  {
    m_end_time = ev_now (EV_DEFAULT) + double(wgr.get_execution_cost ())/1000.0f;
    m_mock_timer.start_timer (double(wgr.get_execution_cost ())/1000.0f, 0.0f/*no repeat*/);
    LOG_LOC_DEBUG ("ID: %d. WGR submitted, set the timer for %f ms",
                    m_id, double(wgr.get_execution_cost ())/1000.0f);
  }
  else
  {
    ev_tstamp remain_time = m_end_time - ev_now (EV_DEFAULT);
    ev_tstamp new_time    = remain_time + double(wgr.get_execution_cost ())/1000.0f;

    m_mock_timer.stop_timer ();
    m_mock_timer.start_timer (new_time, 0.0f /*no repeat*/);
    m_end_time = ev_now (EV_DEFAULT) + new_time;
    LOG_LOC_DEBUG ("ID: %d. WGR submitted, re-set the timer for %f ms", m_id, new_time);
  }

  // ... but for now we just start a timer that will mark the wgr as finished
  // (we assume execution cost in milliseconds, so we divide by 1000 to bring it to seconds)
#endif

  Global::get_instance ().get_free_sched_mgr().remove_free_scheduler (m_id);
}

// this is called on timer
void FPGAML::Scheduler::on_timer ()
{
  // 0. pop the nearest wgr from the queue
  // 1. print the name, size and duration of the wgr (the intented one)
  // 2. if the buffer is empty - deactivate the timer (it should be deactivated anyways)
  // 3. if the buffer is not empty - get the first most wgr, get its time estimate
  //    and schedule the timer, all the while printing the debug info 

  WorkGroupRange wgr = m_in_queue.pop_front ();
  LOG_LOC_DEBUG ("ID: %d, Marking wgr (%s) as finished", m_id, wgr.get_kernel_name().c_str());

  wgr.mark_as_finished ();


  if (m_in_queue.empty ())
  {
    Global::get_instance ().get_free_sched_mgr().add_free_scheduler (m_id);
    m_end_time = 0.0f;
  }
  else
  {
    WorkGroupRange& wgr_new = m_in_queue[0];
    m_end_time = ev_now (EV_DEFAULT) + double(wgr_new.get_execution_cost ())/1000.0f;
    m_timer.start_timer (double(wgr_new.get_execution_cost ())/1000.0f, 0.0f/*no repeat*/);
    LOG_LOC_DEBUG ("ID: %d. WGR (%s) submitted, set the timer for %f s",
                    m_id, wgr_new.get_kernel_name().c_str(),
                    double(wgr_new.get_execution_cost ())/1000.0f);

  }
}

