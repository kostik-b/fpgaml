// Copyright QUB 2018

#ifndef FPGAML_The_Buffer_hh
#define FPGAML_The_Buffer_hh

#include "dispatcher.h"
#include "circ_array.hpp"
#include "ndrange.h"
#include <mutex>
#include <ev.h>

namespace FPGAML
{

class Signal
{
public:
  static Signal& get_instance ();

  void signal_new_ndrange ();

private:
  Signal ();

  ev_async m_on_ndrange_watcher;
}; // class Signal

// This is a thread-safe class that sits between the user and the FPGA ML (dispatcher)
class TheBuffer
{
public:
  static TheBuffer& get_instance ();

  void      push_back (NDRange* ndrange);

  NDRange*  pop_front ();

  NDRange*  peek_front();

  bool      is_full   ();

  bool      is_empty  ();

private:
  TheBuffer ();
  TheBuffer (const TheBuffer& copy) = delete;
  TheBuffer& operator=(const TheBuffer& rhs) = delete;

  typedef CircArray<NDRange*> SchedulersRingBuffer;
  SchedulersRingBuffer  m_ring_buffer;

  std::mutex            m_mutex;
}; // class TheBuffer

} // namespace

#endif
