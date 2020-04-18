// Copyright QUB 2018

#include "the_buffer.h"
#include "centrum.h"

static void on_new_ndrange_cb (EV_P_ ev_async* w, int revents)                                           
{ 
  FPGAML::Global::get_instance ().get_dispatcher ().read_from_the_buffer ();
}

FPGAML::Signal& FPGAML::Signal::get_instance ()
{
  static Signal* instance = new Signal;

  return *instance;
}

FPGAML::Signal::Signal ()
{
  // init the async watcher
  ev_async_init (&m_on_ndrange_watcher, on_new_ndrange_cb);

  ev_async_start (EV_DEFAULT_ &m_on_ndrange_watcher);
}

void FPGAML::Signal::signal_new_ndrange ()
{
  // this will start the default ev loop
  Global::get_instance ();

  ev_async_send (EV_DEFAULT_ &m_on_ndrange_watcher);
}

// ------------ T H E  B U F F E R ---------------------
FPGAML::TheBuffer::TheBuffer ()
  : m_ring_buffer (100) // an arbitrary value
{
}

FPGAML::TheBuffer& FPGAML::TheBuffer::get_instance ()
{
  static TheBuffer* instance = new TheBuffer;

  return *instance;
}

void FPGAML::TheBuffer::push_back (NDRange* ndrange)
{
  std::lock_guard<std::mutex> lock (m_mutex);

  m_ring_buffer.push_back (ndrange);
}

FPGAML::NDRange* FPGAML::TheBuffer::pop_front ()
{
  std::lock_guard<std::mutex> lock (m_mutex);

  if (m_ring_buffer.occupancy() != 0)
  {
    return m_ring_buffer.pop_front ();
  }
  else
  {
    return NULL;
  }
}

FPGAML::NDRange* FPGAML::TheBuffer::peek_front()
{
  std::lock_guard<std::mutex> lock (m_mutex);

  if (m_ring_buffer.occupancy() != 0)
  {
    return m_ring_buffer[0];
  }
  else
  {
    return NULL;
  }
}

bool FPGAML::TheBuffer::is_full   ()
{
  std::lock_guard<std::mutex> lock (m_mutex);

  return m_ring_buffer.is_full ();
}

bool FPGAML::TheBuffer::is_empty   ()
{
  std::lock_guard<std::mutex> lock (m_mutex);

  return m_ring_buffer.capacity () == m_ring_buffer.occupancy ();
}

