// Copyright QUB 2018

#include "algo_slicing.h"
#include <cpp/WeeLogger.h>
#include "centrum.h"
#include <tuple>

typedef std::tuple<unsigned, unsigned, unsigned> tuple_3u;

FPGAML::RC FPGAML::AlgoSlicing::route_ndrange (NDRange* ndrange)
{
  if (m_ndranges.is_full())
  {
    return RC::ML_ERROR_TOO_MANY_NDR;
  }
  else
  {
    m_ndranges.push_front (ndrange);
    // TODO -- necesssary to provide the status back to the user
    // m_monitor.add_new_ndrange (ndrange);
    return RC::ML_SUCCESS;
  }
}

namespace FPGAML
{
static int select_best_scheduler (
  SchedulersIdRingBuffer& free_schedulers,
  NDRange*                ndrange,
  const unsigned          quant,
  tuple_3u&               wgs_and_cost)
{
  // in the below loop we do 2 things: finding a suitable island and
  // finding an island with min cost
  size_t            chosen_scheduler_idx   = 0;
  int               max_wgs                = -1;
  unsigned          chosen_reconf_cost     = 0;
  unsigned          chosen_wg_cost         = 0;
  unsigned          chosen_total_cost      = 0;

  Tracker&          tracker                 = Global::get_instance ().get_tracker ();
  ReconfController& reconf                  = Global::get_instance ().get_reconf  ();
  const std::vector<Scheduler*>& schedulers = Global::get_instance().get_schedulers ();

  for (int i = 0; i < free_schedulers.occupancy (); ++i)
  {
    // how long does it take to execute a wg of this ndrange?
    const unsigned     scheduler_idx = free_schedulers[i];
    Scheduler&         scheduler     = *(schedulers[scheduler_idx]);
    const std::string& kernel_name   = ndrange->get_kernel_name ();

    // check if we can fit this kernel on an island
    if (!reconf.check_resource_string (scheduler.id(), ndrange->get_resource_string (), kernel_name))
    {
      continue;
    }

    // get an execution cost
    const int wg_exec_cost = tracker.get_wg_cost_on_island (scheduler_idx, kernel_name);

    // if reconf is needed, it is added to the execution cost
    unsigned reconf_cost = 0;
    if (scheduler.would_need_reconf (kernel_name))
    {
      reconf_cost = tracker.get_reconf_cost_on_island (scheduler_idx, kernel_name);
    }

    // now trying to estimate the wgs cost, the more wgs we can execute the better
    const unsigned after_reconf_cost  = quant - reconf_cost; // time left after reconfiguration
    const unsigned num_wgs            = after_reconf_cost / wg_exec_cost;  // wgs that can fit 
                                                                           // in that time

    if (num_wgs == 0)
    {
      continue;
    }
    const unsigned total_cost         = reconf_cost + num_wgs*wg_exec_cost;
    
    LOG_LOC_DEBUG ("Considering scheduler index: %d, wgs in quant: %d, "
                    "wg_cost: %d, reconf_cost: %d", scheduler_idx, num_wgs, wg_exec_cost, reconf_cost);

    // 1. initialize
    // 2. if we can fit more wgs into the time quant choose this scheduler
    // 3. if the num of wgs is the same as the chosen, but the total cost is 
    //    less, choose this scheduler
    if (((max_wgs < 0) || (num_wgs > max_wgs)) ||
          ((num_wgs == max_wgs) && (total_cost < chosen_total_cost)))
    {
      max_wgs                = num_wgs;
      chosen_scheduler_idx   = scheduler_idx;
      chosen_reconf_cost     = reconf_cost;
      chosen_wg_cost         = wg_exec_cost;
      chosen_total_cost      = total_cost;
    }
  }

  if (max_wgs < 0) // i.e. we could not select the scheduler
  {
    return -1;
  }
  else
  {
    std::get<0>(wgs_and_cost) = max_wgs;
    std::get<1>(wgs_and_cost) = chosen_wg_cost;
    std::get<2>(wgs_and_cost) = chosen_reconf_cost;

    return chosen_scheduler_idx;
  }
}
} // namespace

// this is called by the io/timer
void FPGAML::AlgoSlicing::on_free_schedulers (SchedulersIdRingBuffer& free_schedulers)
{
  if (m_ndranges.empty () || free_schedulers.empty ())
  {
    return;
  }

  // pop the ndrange from the front
  NDRange*          ndrange   = m_ndranges.pop_front ();

  LOG_LOC_DEBUG ("------------------");
  LOG_LOC_DEBUG ("On free scheduler: considering ndrange \"%s\"", ndrange->get_kernel_name().c_str());

  tuple_3u wgs_and_costs;
  LOG_LOC_DEBUG ("--> Trying with default quant: %u", m_min_quant);
  int chosen_scheduler_idx = select_best_scheduler (free_schedulers, ndrange, m_min_quant,
                                                    wgs_and_costs);

  // since we could not select a proper scheduler we will now try with the increased theshold
  if ((chosen_scheduler_idx < 0) && m_max_idle_threshold)
  {
    LOG_LOC_DEBUG ("--> Retrying with larger quant: %u", m_min_quant*m_max_idle_threshold);
    chosen_scheduler_idx = select_best_scheduler (free_schedulers, ndrange,
                                                 m_min_quant*m_max_idle_threshold, wgs_and_costs);
  }

  if (chosen_scheduler_idx < 0)
  {
      m_ndranges.push_back (ndrange); // push it at the very back of the "queue"
      return;
  }

  // now we have the min cost scheduler
  // Scheduler* chosen_scheduler = free_schedulers.pop_at (min_cost_scheduler_num);
  Scheduler& chosen_scheduler = *(Global::get_instance().get_schedulers()[chosen_scheduler_idx]);

  LOG_LOC_DEBUG ("Chose scheduler id: %d, num_wgs: %d, trying to carve out wgr",
                  chosen_scheduler_idx, std::get<0>(wgs_and_costs));

  // now "carve out" the necessary amount of wgs and submit them to the scheduler
  WorkGroupRange wgr = ndrange->carve_wgs (std::get<0>(wgs_and_costs));
  wgr.set_execution_cost (wgr.get_total_size()*std::get<1>(wgs_and_costs) + std::get<2>(wgs_and_costs));

  chosen_scheduler.submit_wgr (wgr);

  if (ndrange->has_more_wgs ())
  {
    m_ndranges.push_back (ndrange); // push it at the very back of the "queue"
  }
  else
  {
    // TODO: how do we communicate back to the app/pocl? <-- probably maintain a map of finished ndranges
    LOG_INFO ("NDRange \"%s\" finished", ndrange->get_kernel_name().c_str());
  }
}
