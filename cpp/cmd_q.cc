// Copyright QUB 2018

#include "cmd_q.h"
#include "centrum.h"
#include <fpgaml_rc.h>

static void on_new_cmd_cb (struct ev_loop* loop, ev_async* w, int revents)
{ 
  if (!(w->data))
  {
    printf ("Faulty command queue watcher, no data - how can this be?\n");
    return;
  }

  _cl_command_queue* cmdq = static_cast<_cl_command_queue*>(w->data);

  assert (cmdq != NULL);
  // 1. check if the queue is empty or not
  // 2. if the last event is set and not complete do nothing - or set the timer to check?
  // 3. if the last event is not set OR set and COMPLETE
  //    - decrement counter on previous event
  //    - set new event
  // 4. pop the cmd and pass it to dispatcher
  // 5. if the queue is NOT empty AND the last event is complete
  //    feed the new event

  // 1.
  if (cmdq->is_empty ())
  {
    cmdq->check_self_destroy ();
    return;
  }

  // 2.
  cl_event last_event = cmdq->get_last_event ();
  if (last_event && (last_event->get_event_state () != CL_COMPLETE))
  {
    // do nothing, when the event is complete, the event will feed a new event
    // to THIS asycn watcher
    return;
  }

  // if we get here the last event is either complete or not set
  // 3.
  MLCLCommand* cmd = cmdq->pop_front ();
  cmdq->set_last_event (cmd->get_event ());

  // 4.
  FPGAML::Global::get_instance ().get_dispatcher ().handle_a_cmd (cmd);

  // 5.
  if (!(cmdq->is_empty()) && (cmdq->get_last_event()->get_event_state () == CL_COMPLETE))
  {
    ev_feed_event (loop, w, revents);
  }
  else
  {
    cmdq->check_self_destroy ();
  }

  delete cmd; // terminal station
}

// ------------ T H E  B U F F E R ---------------------

_cl_command_queue::_cl_command_queue ()
  : m_ring_buffer (100) // an arbitrary value
  , m_ref_counter (1)
{
  // init the async watcher
  ev_async_init (&m_on_cmd_watcher, on_new_cmd_cb);

  m_on_cmd_watcher.data = this;

  ev_async_start (EV_DEFAULT_ &m_on_cmd_watcher);

  m_last_event = NULL;
}

_cl_command_queue::~_cl_command_queue ()
{
  ev_async_stop (EV_DEFAULT_ &m_on_cmd_watcher);
}

_cl_command_queue* _cl_command_queue::create_new ()
{
  return new _cl_command_queue;
}

FPGAML::RC _cl_command_queue::push_back (MLCLCommand* ndrange)
{
  std::lock_guard<std::mutex> lock (m_mutex);

  if (m_ref_counter < 1)
  {
    return FPGAML::RC::ML_ERROR_CMDQ_RELEASED;
  }

  m_ring_buffer.push_back (ndrange);

  FPGAML::Global::get_instance ();

  ev_async_send (EV_DEFAULT_ &m_on_cmd_watcher);
}

MLCLCommand* _cl_command_queue::pop_front ()
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

MLCLCommand* _cl_command_queue::peek_front()
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

bool _cl_command_queue::is_full   ()
{
  std::lock_guard<std::mutex> lock (m_mutex);

  return m_ring_buffer.is_full ();
}

bool _cl_command_queue::is_empty   ()
{
  std::lock_guard<std::mutex> lock (m_mutex);

  return m_ring_buffer.empty ();
}

void _cl_command_queue::ref_cnt_down ()
{
  std::lock_guard<std::mutex> lock (m_mutex);

  if (m_ref_counter > 0)
  {
    --m_ref_counter;
  }
  // if counter is 0, do the following:
  // - stop accepting commands in push_back
  // - once the last command has been popped, delete itself
}

void _cl_command_queue::check_self_destroy ()
{
  std::lock_guard<std::mutex> lock (m_mutex);

  if (m_ref_counter < 1)
  {
    delete this;
  }
}

// we feed the new event to continue reading the
// commands from the queue
static void on_event_complete (void* data)
{
  ev_async* watcher = reinterpret_cast<ev_async*>(data);

  ev_feed_event (EV_DEFAULT_ watcher, 0);
}

void _cl_command_queue::set_last_event (cl_event event)
{
  // set the event to that event
  // also register a callback, which would feed an event to the
  // watcher handler

  // decrement the ref counter on the previous event
  // if we got here, it must have been completed!
  if (m_last_event)
  {
    m_last_event->ref_cnt_down ();
  }

  // increment the ref counter on the new event
  m_last_event = event;
  m_last_event->ref_cnt_up ();
  
  m_last_event->register_on_complete_cb_func (on_event_complete, &m_on_cmd_watcher);
}
