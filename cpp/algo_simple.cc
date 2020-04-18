// Copyright QUB 2018

#include "algo_simple.h"
#include <stdio.h>
#include "centrum.h"
#include <cpp/WeeLogger.h>

static const int UNSUITABLE_ISLAND = -13;

FPGAML::AlgoSimple::AlgoSimple (const size_t num_sched)
  : m_num_islands                 (num_sched)
  , m_wgs_per_islands             (m_num_islands) // preallocate a vector - one entry 
                                                  // per each scheduler-island pair
  , m_work_in_progress_cost       (m_num_islands)
  , m_current_work_potential_cost (m_num_islands)
  , m_current_work_reconf_cost    (m_num_islands)
  , m_result                      (m_num_islands)
{
  m_current_min = -1; // starting value
}

// we only calculate this for leaf nodes, i.e.
// when go full depth
void FPGAML::AlgoSimple::calculate_total ()
{
  // how to find min:
  // - this function is only called per every leaf
  // - for every leaf we calculate HERE the max of total work
  //   of the islands that have current wgs on them
  // - If we reach a new min, we record it in m_current_min
  //   and place the result in m_result_route.

  int max = 0;
  for (int i = 0; i < m_num_islands; ++i)
  {
    if (m_wgs_per_islands[i] > 0)
    {
      int wgs_cost = m_wgs_per_islands[i]*m_current_work_potential_cost[i] + 
                      m_work_in_progress_cost[i] + m_current_work_reconf_cost[i];
      if (wgs_cost > max)
      {
        max = wgs_cost;
      }
    }
  }

  if ((max < m_current_min) || (m_current_min < 0)/*starting condition*/)
  {
    m_current_min = max;
    m_result      = m_wgs_per_islands;
  }
// -----------
// a little hack
#if 0
  const char* log_level = getenv ("WL_LOG_LEVEL");
  if (log_level && ((strcmp(log_level,"1") == 0) || (strcmp(log_level,"1") == 0)))
  {
    // for now just print the current leaf
    printf ("|");
    for (int i = 0; i < m_num_islands; ++i)
    {
      printf ("%04d|", m_wgs_per_islands[i]);
    }

    printf (" <- max is %d", max);

    printf ("\n+");

    for (int i = 0; i < m_num_islands; ++i)
    {
      printf ("----+");
    }
    printf ("\n");
  }
#endif
}

void FPGAML::AlgoSimple::print_test_input ()
{
  const char* log_level = getenv ("WL_LOG_LEVEL");
  if (log_level && ((strcmp(log_level,"1") == 0) || (strcmp(log_level,"1") == 0)))
  {
    printf ("Work in progress cost:\n|");
    for (int i = 0; i < m_work_in_progress_cost.size (); ++i)
    {
      printf ("%04d|", m_work_in_progress_cost[i]);
    }
    printf ("\n\nCurrent work potential cost:\n|");
    for (int i = 0; i < m_current_work_potential_cost.size (); ++i)
    {
      printf ("%04d|", m_current_work_potential_cost[i]);
    }
    printf ("\n");
    printf ("\nCurrent work reconf cost:\n|");
    for (int i = 0; i < m_current_work_reconf_cost.size (); ++i)
    {
      printf ("%04d|", m_current_work_reconf_cost[i]);
    }
    printf ("\n");
  }
}

void FPGAML::AlgoSimple::recursive_function (
  const int num_items,
  const int start_index)
{
  // base condition
  if (start_index == m_num_islands)
  {
    // in the current design we are going to "carry over" 
    // some num_iters beyond the final loop. This is Ok,
    // but we do not want to count them in
    if (num_items == 0)
    {
      calculate_total ();
    }
    return;
  }

  // we just skip unsuitable islands
  if (m_wgs_per_islands[start_index] == UNSUITABLE_ISLAND)
  {
    recursive_function (num_items, start_index + 1);
    return;
  }

  int counter = 0;
  do
  {
    // put current item into the corresponding array "cell"
    m_wgs_per_islands [start_index] = num_items - counter;
    recursive_function (counter, start_index + 1);
    ++counter;
  } while (counter < num_items + 1);
}

// NB: the current setup only works for 1D ndranges. The problem we get if we go to 2D and 3D is that
// the num of wgs in the optimal distribution (which we calculate through the recursive function) will
// not be easily carveable, i.e. ndrange will not be able to give us exactly the number of wgs requested
// ... unless it gives us a number of WGRs!!! - OR each WGR can actually contain subranges!!! <-- IDEA!
FPGAML::RC FPGAML::AlgoSimple::route_ndrange (NDRange*  ndrange)
{
  if (!ndrange)
  {
    return RC::ML_WRONG_PARAM;
  }
  const string& kernel_name     = ndrange->get_kernel_name ();
  const string& resource_string = ndrange->get_resource_string ();
  const size_t  num_wgs         = ndrange->get_total_size ();
  m_current_min                 = -1;

  Tracker&          tracker     = Global::get_instance().get_tracker ();
  ReconfController& controller  = Global::get_instance().get_reconf  ();
  // 1. check the resource string for all islands and
  //    populate the m_wgs_per_islands with 0 or
  //    UNSUITABLE_ISLAND
  // 2. for every island get the cost of current work
  //    and populate m_work_in_progress_cost
  for (int i = 0; i < m_num_islands; ++i)
  {
    Scheduler&  current_scheduler     = *(Global::get_instance ().get_schedulers ()[i]);

    if (!controller.check_resource_string (current_scheduler.id(), resource_string, kernel_name))
    {
      m_wgs_per_islands[i]              = UNSUITABLE_ISLAND;
      m_work_in_progress_cost[i]        = UNSUITABLE_ISLAND;
      m_current_work_potential_cost [i] = UNSUITABLE_ISLAND;
      m_current_work_reconf_cost [i]    = UNSUITABLE_ISLAND;
      m_result [i]                      = UNSUITABLE_ISLAND;
    }
    else
    {
      m_wgs_per_islands[i]              = 0; // it's a suitable island

      m_work_in_progress_cost[i]        = current_scheduler.get_remaining_time ();
      // find out the reconf cost
      if (current_scheduler.would_need_reconf (kernel_name))
      {
        m_current_work_reconf_cost [i]  = tracker.get_reconf_cost_on_island (i, kernel_name);
      }
      else
      {
        m_current_work_reconf_cost [i]  = 0;
      }
      m_current_work_potential_cost [i] = tracker.get_wg_cost_on_island     (i, kernel_name);
      m_result[i]                       = 0;
    }
  }

  // debug
  LOG_INFO ("AlgoSimple: Received routing request for kernel %s of %lu wgs",
                  kernel_name.c_str(), num_wgs);
  print_test_input ();

  LOG_LOC_DEBUG ("\n");
  LOG_LOC_DEBUG ("Starting recursive function");

  // -> start the recursive function
  recursive_function (num_wgs, 0);

  // bingo - we should have the result ready

  const size_t LBL = 1024;
  char log_buffer [LBL];
  int write_offset = sprintf (log_buffer, "AlgoSimple routing distribution is:|");
  for (int i = 0; i < m_num_islands; ++i)
  {
    write_offset += sprintf (log_buffer + write_offset, "%04d|", m_result[i]);
  }
  sprintf (log_buffer + write_offset, "\n");
  LOG_INFO ("%s", log_buffer);

  for (int i = 0; i < m_num_islands; ++i)
  {
    if ((m_result[i] == UNSUITABLE_ISLAND) || (m_result[i] == 0))
    {
      continue;
    }

    // carve and then submit a wgr
    WorkGroupRange wgr = ndrange->carve_wgs (m_result[i]);
    wgr.set_execution_cost (m_result[i]*m_current_work_potential_cost[i] + /* m_work_in_progress_cost[i] + */
                              m_current_work_reconf_cost[i]);
    Global::get_instance().get_schedulers()[i]->submit_wgr (wgr);
  }

  assert (ndrange->has_more_wgs () == false);

  return RC::ML_SUCCESS;
}
