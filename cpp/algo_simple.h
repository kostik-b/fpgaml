// Copyright QUB 2018

#ifndef ecoscale_algo_simple_H
#define ecoscale_algo_simple_H

#include <vector>
#include <string>
#include <stddef.h>

#include "tracker.h"
#include "reconf_controller.h"
#include "fpgaml_rc.h"
#include "ndrange.h"

/*
  The purpose of the algo is to schedule each
  new work in the most efficient way possible (considering
  time here).
  The algo is as follows:
   - Put together an array T of the suitable islands (can be checked
     with check_resource_string). Mark unsuitable islands with
     a special value. 
   - For every island compute the execution time. Also, if the
     island needs to be reconfigured, then add the reconfiguration
     time. Thus we'll have an array T of N islands. Each element will
     have cost made of the execution and reconfiguration (if applicable).
   - Then run the recursive algo, which would try all permutations of
     the total num of WGs distributed in every possible way over all
     of the suitable islands.
   - During the algo execution keep on recording the min total value
     across all the islands. Use a global array C for that.
   - Upon the completion the array C will contain the combination
     the will schedule all of the WGs in the most efficient way possible.

*/

using std::vector;
using std::string;

namespace FPGAML
{

class AlgoSimple
{
public:
  AlgoSimple (const size_t num_sched);

  RC    route_ndrange      (NDRange*  ndrange);

  void  calculate_total    ();
  
  void  recursive_function (const int num_items,
                            const int start_index);

  void  print_test_input   ();

private:
  const size_t      m_num_islands;
  std::vector<int>  m_wgs_per_islands;
  std::vector<int>  m_work_in_progress_cost;
  std::vector<int>  m_current_work_potential_cost;
  std::vector<int>  m_current_work_reconf_cost;
  std::vector<int>  m_result;

  int               m_current_min;
}; // class AlgoSimple

} // namespace FPGAML

#endif
