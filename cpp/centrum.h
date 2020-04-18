// Copyright QUB 2018

#ifndef ecoscale_centrum_h
#define ecoscale_centrum_h

#include <stddef.h>
#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <thread>

#include "ndrange.h"
#include "scheduler.h"
#include "tracker.h"
#include "dispatcher.h"
#include "reconf_controller.h"
#include "circ_array.hpp"
#include "bitstream_manager.h"
#include "bitstream_check.h"
#include "unimem_manager.h"
#include "wee_config.h"
#include "sched_config.h"

/*
  The below classes are the mock "backends" for testing the routing algos
*/

namespace FPGAML
{

class InitLogLevel
{
public:
  InitLogLevel (WeeConfig& config)
  {
    // set the log level
    const char* log_level = NULL;
    if (config.get_value_for_key ("LogLevel", log_level) == false)
    {
      printf ("ERROR: Could not fetch LogLevel from the xml file");
    }

    // FIXME: this will only work if we don't log anything
    // up to this point, i.e. WeeLogger has not been
    // created yet
    if (log_level)
    {
      setenv ("WL_LOG_LEVEL", log_level, 1);
    }
  }
};


/*
 * Global is a global object, it contains all the other globally available objects.
   Global is NOT thread-safe, but some individual objects are
*/
class Global
{
public:

  static Global&          get_instance        ();

  void                    shutdown            (); // TODO: to be implemented
  
  const std::vector<Scheduler*>& 
                          get_schedulers      () { return m_sched_config.get_schedulers(); }

  Tracker&                get_tracker         () { return m_tracker; }
  ReconfController&       get_reconf          () { return m_reconf;  }
  FreeSchedulersManager&  get_free_sched_mgr  () { return m_free_sc_mgr; }
  Dispatcher&             get_dispatcher      () { return m_dispatcher; }
  BitstreamManager&       get_bits_manager    () { return m_bits_manager; }
  BitstreamCheck&         get_bits_check      () { return m_bits_check; }
  WorkGroupSizeCheck&     get_wg_size_check   () { return m_wg_size_check; }
  UnimemManager&          get_unimem_manager  () { return m_unimem_manager; }
  WeeConfig&              get_config          () { return m_config; }
  SchedConfig&            get_sched_config    () { return m_sched_config; }

private:
  Global ();
  Global& operator=(const Global& rhs) = delete; // no assignment
  Global (Global& copy) = delete; // no copy c-tor

private:
  WeeConfig               m_config;

  InitLogLevel            m_init_log_level;

  SchedConfig             m_sched_config;

  ReconfController        m_reconf;
  FreeSchedulersManager   m_free_sc_mgr;

  Dispatcher              m_dispatcher;

  BitstreamManager        m_bits_manager;
  Tracker                 m_tracker;
  BitstreamCheck          m_bits_check;
  WorkGroupSizeCheck      m_wg_size_check;

  UnimemManager           m_unimem_manager;
}; // class Global

} // namespace FPGAML

#endif
