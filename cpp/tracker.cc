// Copyright QUB 2018

#include "tracker.h"
#include <cpp/WeeLogger.h>

FPGAML::Tracker::Tracker (
  BitstreamManager&               bits_manager,
  const std::vector<Scheduler*>&  schedulers)
{
  // for every bitstream
  // - for every scheduler
  //   is_local? yes -> set local cost
  //             no  -> set remote cost
  auto iters_pair = bits_manager.get_map_iterators();
  auto i          = iters_pair.first;
  auto end        = iters_pair.second;

  while (i != end)
  {
    const std::string& name = i->first;

    const unsigned local_cost   = i->second.local_cost;
    const unsigned remote_cost  = i->second.remote_cost;

    std::vector<unsigned> costs;
    for (int j = 0; j < schedulers.size (); ++j)
    {
      Scheduler& current_sc = *(schedulers[j]);

      if (current_sc.is_local ())
      {
        costs.push_back (local_cost);
      }
      else
      {
        costs.push_back (remote_cost);
      }
    }

    m_wg_cost_mapping.insert (std::make_pair (name, costs));
    ++i;
  }
}

int FPGAML::Tracker::get_wg_cost_on_island (
  const unsigned      island_num,
  const std::string&  kernel_name)
{
  auto i = m_wg_cost_mapping.find (kernel_name);

  if (i == m_wg_cost_mapping.end ())
  {
    LOG_LOC_ERROR ("Could not find wg cost on island %d for "
                   "kernel %s, defaulting to 1000",
                    island_num, kernel_name.c_str());
    return 1000;
  }
  else
  {
    return (*i).second.at (island_num);
  }
}

int FPGAML::Tracker::get_reconf_cost_on_island (
  const unsigned      island_num,
  const std::string&  kernel_name)
{
  /*std::map<std::string, std::vector<int> >::iterator*/
  auto iter = m_reconf_cost_mapping.find (kernel_name); // trying out C++11 features

  if (iter == m_reconf_cost_mapping.end ())
  {
    LOG_LOC_ERROR ("Could not find reconf cost on island %d for "
                   "kernel %s, defaulting to 1000",
                   island_num, kernel_name.c_str());
    return 1000;
  }
  else
  {
    return (*iter).second.at (island_num);
  }
}


