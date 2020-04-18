// Copyright QUB 2018

#include "centrum.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "the_buffer.h"

// not planning to change this very often

FPGAML::Global& FPGAML::Global::get_instance ()
{
  static Global* s_global = new Global ();

  return *s_global;
}

static void run ()
{
  ev_run (EV_DEFAULT, 0);

  // FIXME: should we be doing cleanup here?
}

FPGAML::Global::Global ()
  : m_config          (getenv ("FPGAML_CONFIG_PATH"), "ConfigParams")
  , m_init_log_level  (m_config)
  , m_sched_config    (m_config)
  , m_free_sc_mgr     (m_sched_config.get_schedulers().size())
  , m_dispatcher      (m_free_sc_mgr, m_sched_config.get_schedulers().size())
  , m_bits_manager    (m_config)
  , m_tracker         (m_bits_manager, m_sched_config.get_schedulers())
  , m_bits_check      (m_bits_manager.get_xml_files_list().first,
                       m_bits_manager.get_xml_files_list().second)
  , m_wg_size_check   (m_bits_manager.get_map_iterators().first,
                       m_bits_manager.get_map_iterators().second)
  , m_unimem_manager  (m_config)
{
  // here we set the mock values
  // srand (time(NULL));

  const std::vector<Scheduler*>& schedulers = get_schedulers ();

  srand (45);
  for (int i = 0; i < schedulers.size (); ++i)
  {
    // value from 0 to 99 is fine with me :-)
    // m_schedulers[i].m_mock_remaining_time     = rand () % 100;
    schedulers[i]->m_mock_would_need_reconf  = false;

    m_free_sc_mgr.add_free_scheduler (schedulers[i]->id ());
  }

  if (schedulers.size () > 5)
  {
    schedulers[5]->m_mock_would_need_reconf  = false;
  }
  if (schedulers.size () > 10)
  {
    schedulers[10]->m_mock_would_need_reconf  = false;
  }

  std::thread t (run);
  t.detach ();
}

#if 0
// the purpose of this class is to create Global before main starts
class GlobalGlobalWrapper
{
public:
  GlobalGlobalWrapper ()
  {
   std::thread t (run);
    t.detach ();
  }

} g_global_global;
#endif
