// Copryright QUB 2019

#include "sched_config.h"

#include <tuple>
#include <cpp/WeeLogger.h>
#include "sched_config_extractors.h"
#include <set>
#include <vector>

#if 0
FPGAML::SchedConfig::SchedConfig (WeeConfig& config)
{
  m_fd_reg    = 0;
  m_fd_drvr   = 0;
  m_fd_ctrl   = 0;

  m_opcode    = 0;

  std::vector<tuple_2>       fpgas;
  std::vector<Extractor_T2*> extractors = {new ExtractFpgaID, new ExtractBoardID};

  // get the board ids and fpga ids
  config.get_values_for_key ("FPGAs", extractors, fpgas);

  // to keep track of the started fpgas
  std::set<unsigned> fpga_ids;

  for (int i = 0; i < fpgas.size (); ++i)
  {
    tuple_2& ids = fpgas[i];

    const unsigned           fpga_id  = std::get<1>(ids);
    const unsigned long long board_id = std::get<0>(ids);

    // for every fpga we create 4 schedulers
    for (unsigned j = 0; j < 4; ++j)
    {
      Scheduler* scheduler = new Scheduler (board_id, fpga_id, j);

      m_schedulers.push_back (scheduler);
    }

    // has this fpga been already started?
    if (fpga_ids.find (fpga_id) == fpga_ids.end())
    {
      // start a given fpga
      LOG_INFO ("Pretending to start fpga %u", fpga_id);

      fpga_ids.insert (fpga_id);
    }
  }
}
#endif

FPGAML::SchedConfig::~SchedConfig ()
{
}

void FPGAML::SchedConfig::open_drivers (
  WeeConfig&               config)
{
  LOG_INFO ("Simulator: pretentding to be opening drivers ..."); 
}

