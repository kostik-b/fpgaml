// Copyright QUB 2018

#ifndef es_tracker_hh
#define es_tracker_hh

#include <string>
#include <map>
#include <vector>
#include "bitstream_manager.h"
#include "scheduler.h"

namespace FPGAML
{

class Tracker
{
public:
  Tracker (BitstreamManager&                bits_manager,
           const std::vector<Scheduler*>&   schedulers);

  // -1 indicates an error
  int get_wg_cost_on_island      (const unsigned island_num, const std::string& kernel_name);
  int get_reconf_cost_on_island  (const unsigned island_num, const std::string& kernel_name);

private:
  std::map<std::string, std::vector<unsigned> > m_wg_cost_mapping;
  std::map<std::string, std::vector<unsigned> > m_reconf_cost_mapping;
}; // class Tracker

} // namespace

#endif
