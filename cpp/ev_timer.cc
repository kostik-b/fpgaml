// Copyright QUB 2018

#include "ev_timer.h"
#include "dispatcher.h"
#include "scheduler.h"
#include "the_buffer.h"
#include "centrum.h"

extern "C"
{

void on_timer_cb (
  struct ev_loop*   loop,
  struct ev_timer*  watcher,
  int               revents)
{ 
  if (!(watcher->data))
  {
    printf ("Error: no data set in watcher - returning");
    return;
  }

  FPGAML::EvTimerBase* cb = (FPGAML::EvTimerBase*)watcher->data;
  
  cb->on_timer ();
}   
    
} // extern "C"


FPGAML::EvTimerBase::EvTimerBase ()
{
  m_watcher.data = this;

  // just init the watcher, no scheduling just yet
  ev_init (&m_watcher, on_timer_cb);
}


bool FPGAML::EvTimerBase::is_active ()
{
  return ev_is_active (&m_watcher);
}

void FPGAML::EvTimerBase::start_timer (
  const double at,
  const double repeat)
{
  ev_timer_set (&m_watcher, at, repeat);

  ev_timer_start (EV_DEFAULT, &m_watcher);
}

void FPGAML::EvTimerBase::stop_timer  ()
{
  ev_timer_stop (EV_DEFAULT, &m_watcher);
}
// --------- SchedulerEvTimer ------------------------
FPGAML::SchedulerEvTimer::SchedulerEvTimer (Scheduler& scheduler)
  : m_scheduler (scheduler)
{
}

void FPGAML::SchedulerEvTimer::on_timer ()
{
  // invoke some method on the Scheduler
  m_scheduler.on_timer ();
}
// ------------- FreeSchedulersEvTimer ----------------
FPGAML::FreeSchedulersEvTimer::FreeSchedulersEvTimer (
  SchedulersIdRingBuffer& free_schedulers)
  : m_cb (NULL)
  , m_free_schedulers (free_schedulers)
{
}

void FPGAML::FreeSchedulersEvTimer::on_timer ()
{
  if (!m_cb)
  {
    return;
  }
  m_cb->on_free_schedulers (m_free_schedulers);

  // the idea is to keep on firing for as long as we have
  // free schedulers
  if (m_free_schedulers.occupancy () != 0)
  {
    start_timer (0.0f, 0.0f);
  }
}

void FPGAML::FreeSchedulersEvTimer::set_cb (OnFreeSchedulers* cb)
{
  m_cb = cb;
}
// ---------------- TheBufferReadTimer ----------------------
FPGAML::TheBufferReadTimer::TheBufferReadTimer (Dispatcher& d)
  : m_disp (d)
{}

void FPGAML::TheBufferReadTimer::on_timer ()
{
  m_disp.read_from_the_buffer ();
}
