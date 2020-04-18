// Copyright QUB 2019

#ifndef fpgaml_sched_config_hh
#define fpgaml_sched_config_hh

#include <vector>
#include "scheduler.h"
#include "wee_config.h"

namespace FPGAML
{

class SchedConfig
{
public:
  SchedConfig (WeeConfig& config);
  ~SchedConfig ();

  int       get_fd_drvr     () { return m_fd_drvr; }
  int       get_fd_ctrl     () { return m_fd_ctrl; }

  unsigned  get_next_opcode ()
  {
    return m_opcode++;
  }

  const std::vector<Scheduler*>&
            get_schedulers  () { return m_schedulers; }

private:
  void      open_drivers    (WeeConfig&               config);

private:
  int                     m_fd_drvr;
  int                     m_fd_ctrl;

  unsigned                m_opcode;

  std::vector<Scheduler*> m_schedulers;

}; // class SchedConfig

} // namespace FPGAML

#endif
