// Copyright QUB 2018

#include "free_sched_mgr.h"
#include <cpp/WeeLogger.h>

FPGAML::FreeSchedulersManager::FreeSchedulersManager (
  const size_t free_sc_size)
  : m_free_schedulers   (free_sc_size)
  , m_free_sched_timer  (m_free_schedulers)
{
}

void FPGAML::FreeSchedulersManager::register_on_free_schedulers_cb (
  OnFreeSchedulers*  cb)
{
  m_free_sched_timer.set_cb (cb);
}

void FPGAML::FreeSchedulersManager::add_free_scheduler (const unsigned id)
{
  assert (m_free_schedulers.is_full () == false);
  m_free_schedulers.push_back (id);

  if (!m_free_sched_timer.is_active ())
  {
    m_free_sched_timer.start_timer (0.0f, 0.000001f);
  }
}

void FPGAML::FreeSchedulersManager::remove_free_scheduler (const unsigned id)
{
  for (int i = 0; i < m_free_schedulers.occupancy (); ++i)
  {
    if (m_free_schedulers[i] == id)
    {
      m_free_schedulers.pop_at (i);
      break;
    }
  }
  if (m_free_schedulers.empty ())
  {
    LOG_LOC_DEBUG ("Stopping free sched timer");
    m_free_sched_timer.stop_timer ();
  }
}

