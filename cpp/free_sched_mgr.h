// Copyright QUB 2018

#ifndef es_free_sched_mgr_h
#define es_free_sched_mgr_h

#include "circ_array.hpp"
#include "ev_timer.h"

namespace FPGAML
{

typedef CircArray<unsigned> SchedulersIdRingBuffer;

class OnFreeSchedulers // abstract class
{
public:
  virtual void  on_free_schedulers (SchedulersIdRingBuffer& free_schedulers) = 0;
}; 

// this class connects pending ndranges and free schedulers
class FreeSchedulersManager
{
public:
  FreeSchedulersManager (const size_t free_sc_size);

  void register_on_free_schedulers_cb  (OnFreeSchedulers*  cb);
  void add_free_scheduler              (const unsigned     id);
  void remove_free_scheduler           (const unsigned     id);

//  void                    start_free_sched_timer          ();

private:
  SchedulersIdRingBuffer  m_free_schedulers; // the ptrs to the free schedulers we have

  FreeSchedulersEvTimer   m_free_sched_timer;
};

} // namespace FPGAML

#endif
