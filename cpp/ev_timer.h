// Copyright QUB 2018

#ifndef ecoscale_ev_timer_h
#define ecoscale_ev_timer_h

#include <ev.h>
#include "circ_array.hpp"

namespace FPGAML
{

// getting round the circular dependency
class Scheduler;
class OnFreeSchedulers;
typedef CircArray<unsigned> SchedulersIdRingBuffer;
class Dispatcher;

class EvTimerBase
{
public:
  EvTimerBase ();

  virtual void on_timer () = 0;

  bool         is_active();

  void         start_timer (const double at,
                            const double repeat);

  void         stop_timer  ();

private:
  ev_timer  m_watcher;
}; // class EvTimerBase

// --------------------------------------------
class SchedulerEvTimer : public EvTimerBase
{
public:
  SchedulerEvTimer (Scheduler& scheduler);

  virtual void on_timer ();

private:
  Scheduler&  m_scheduler;
}; // class SchedulerEvTimer

// --------------------------------------------
class FreeSchedulersEvTimer : public EvTimerBase
{
public:
  FreeSchedulersEvTimer (SchedulersIdRingBuffer& free_schedulers);

  void on_timer () override;

  void set_cb (OnFreeSchedulers* cb);

private:
  OnFreeSchedulers*       m_cb;
  SchedulersIdRingBuffer& m_free_schedulers;
}; // class SchedulerEvTimer

// --------------------------------------------
class TheBufferReadTimer : public EvTimerBase
{
public:
  TheBufferReadTimer (Dispatcher& d);

  void on_timer () override;

private:
  Dispatcher& m_disp;

}; // class TheBufferReadTimer

} // namespace FPGAML


#endif
