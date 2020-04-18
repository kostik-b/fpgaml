// Copyright QUB 2019

#include "sched_config.h"
#include "wee_config.h"
#include <set>
#include <vector>
#include <tuple>
#include <unistd.h>

#include "sched_config_extractors.h"

FPGAML::SchedConfig::SchedConfig (WeeConfig& config)
{
  // 1. get the config values for the drivers paths
  // 2. open drivers - throw error if can't
  // 3. read the scheduler params from the config file
  // 4. for every new FPGA start it!
  // 5. populate the vectors with schedulers

  open_drivers (config);

  m_opcode = 0;

  double polling_interval = 0.0001f;
  config.get_value_for_key ("SchedPollingInterval", polling_interval);

  std::vector<tuple_3>    prs; // PR = Programmable Region (a.k.a. Island)
  std::vector<Extractor>  extractors = {Extractor("BoardID", 0),
                                        Extractor("FpgaID", 1),
                                        Extractor ("PRID", 2)};

  // get the board ids and fpga ids
  config.get_values_for_key ("PRs", extractors, prs);

  for (int i = 0; i < prs.size (); ++i)
  {
    tuple_3& ids = prs[i];

    const unsigned           fpga_id  = std::get<1>(ids);
    const unsigned long long board_id = std::get<0>(ids);
    const unsigned           pr_id    = std::get<2>(ids);

    Scheduler* scheduler = new Scheduler (board_id, fpga_id, pr_id, polling_interval);

    m_schedulers.push_back (scheduler);
  }
}

