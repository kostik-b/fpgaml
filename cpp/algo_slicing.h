// Copyright QUB 2018

#ifndef ecoscale_algo_slicing_h
#define ecoscale_algo_slicing_h

#include "free_sched_mgr.h"
#include "circ_array.hpp"
#include "ndrange.h"
#include "fpgaml_rc.h"

// NB: algo_simple and algo_slicing are operating at different levels.
//     please do not compare them side-by-side. algo_manager hides
//     this complexity from the "user" side.

/*
  The purpose of this algorithm is to enable resource slicing. This is achieved
  by setting the maximum time ("y") that a given accelerator can be occupied.
  The algorithm works as follows:
  - The work arrives. 
  -> If the ring buffer has space, put it in the front of the buffer
    and return.
  -> Otherwise return a TOO_MUCH_WORK error.
  ----
  - The actual algorithm is driven by an island/scheduler becoming free.
  - When an island becomes free, it is added to the list of free islands.
  - We take an island and then look at the NDRange in the ring buffer.
  - We measure the WGs of such size that their time to completion will be below
    "y".
  - We then "carve" a WGR from NDRange and submit to that island.
  - Then we move to the next available island and repeat the procedure.
*/

namespace FPGAML
{

class AlgoSlicing : public OnFreeSchedulers
{
public:
  AlgoSlicing (const unsigned min_quant,
               const unsigned max_idle_threshold)
    : m_min_quant          (min_quant)
    , m_max_idle_threshold (max_idle_threshold)
  {}

  RC    route_ndrange     (NDRange*                 ndrange);
  void  on_free_schedulers(SchedulersIdRingBuffer&  free_schedulers) override;

private:
  typedef CircArray<NDRange*>   NDRangeRingBuffer;

  NDRangeRingBuffer     m_ndranges;

  const unsigned        m_min_quant;
  // in case when we have idle schedulers (i.e. the WG cost is above the m_min_quant)
  // we can still use them, but ONLY if total cost is below m_min_quant*m_max_idle_threshold
  const unsigned        m_max_idle_threshold;
}; // class AlgoSlicing


} // namespace FPGAML

#endif
