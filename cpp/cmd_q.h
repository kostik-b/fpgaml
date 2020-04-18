// Copyright QUB 2018

#ifndef EcoScale_Cmd_Queue_hh
#define EcoScale_Cmd_Queue_hh

#include "dispatcher.h"
#include "circ_array.hpp"
#include "ndrange.h"
#include <mutex>
#include <ev.h>

class MLCLCommand;

// This is a thread-safe class that sits between the OpenCL IL and the FPGA ML (dispatcher)
class _cl_command_queue
{
public:
  // we want to make sure that this class
  // is only ever allocated on the heap
  static _cl_command_queue* create_new ();

  FPGAML::RC    push_back (MLCLCommand* cmd);

  MLCLCommand*  pop_front ();

  MLCLCommand*  peek_front();

  bool          is_full   ();

  bool          is_empty  ();

  void          ref_cnt_down ();

  void          check_self_destroy ();

  size_t        size () { return m_ring_buffer.occupancy (); }

  void          set_last_event (cl_event event);
  cl_event      get_last_event () { return m_last_event; }

private:
  _cl_command_queue ();
  ~_cl_command_queue ();
  _cl_command_queue (const _cl_command_queue& copy) = delete;
  _cl_command_queue& operator=(const _cl_command_queue& rhs) = delete;

  typedef CircArray<MLCLCommand*> SchedulersRingBuffer;
  SchedulersRingBuffer  m_ring_buffer;

  std::mutex            m_mutex;

  // to signal the new command
  ev_async              m_on_cmd_watcher;

  int                   m_ref_counter;

  cl_event              m_last_event; 
}; // class _cl_command_queue

#endif
