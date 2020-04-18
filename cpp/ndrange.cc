// Copyright QUB 2018

#include "ndrange.h"

#include <stdio.h>
#include <stdexcept>
#include <cassert>
#include <cpp/WeeLogger.h>

// ------------------- W O R K G R O U P R A N G E -----------------------------

FPGAML::WorkGroupRange::WorkGroupRange ()
{  
  m_parent_ndrange = NULL;
  m_start_x = m_size_x = m_start_y = m_size_y = m_start_z = m_size_z = 0;
  m_execution_cost = m_execution_start_time = 0;
}

FPGAML::WorkGroupRange::WorkGroupRange (
  NDRange&        ndrange,
  const unsigned  start_x, const unsigned size_x,
  const unsigned  start_y, const unsigned size_y,
  const unsigned  start_z, const unsigned size_z)
  : m_parent_ndrange  (&ndrange)
  , m_start_x (start_x)
  , m_size_x   (size_x)
  , m_start_y (start_y)
  , m_size_y   (size_y)
  , m_start_z (start_z)
  , m_size_z   (size_z)
{
  m_execution_cost = m_execution_start_time = 0;

  m_total_size = size_x * size_y * size_z;
}

void FPGAML::WorkGroupRange::mark_as_finished ()
{
  assert (m_parent_ndrange != NULL);
  m_parent_ndrange->decrement_wgrs_counter ();
}

const std::string& FPGAML::WorkGroupRange::get_kernel_name ()
{
  return m_parent_ndrange->get_kernel_name ();
}

const std::vector<KernelArg*>
FPGAML::WorkGroupRange::get_kernel_args ()
{
  if (m_parent_ndrange && m_parent_ndrange->m_kernel)
  {
    return m_parent_ndrange->m_kernel->get_kernel_args ();
  }
  else
  {
    return std::vector<KernelArg*>();
  }
}
// ------------------ N D R A N G E -----------------------------------------------
FPGAML::NDRange::NDRange (
  cl_kernel          kernel,
  cl_event           event,
  std::string&       resource_string,
  const unsigned     size_x,
  const unsigned     size_y,
  const unsigned     size_z)
  : m_kernel  (kernel)
  , m_event   (event)
  , m_resource_string (resource_string)
  , m_start_x (0)
  , m_size_x  (size_x)
  , m_start_y (0)
  , m_size_y  (size_y)
  , m_start_z (0)
  , m_size_z  (size_z)
  , m_total_size (m_size_x * m_size_y * m_size_z)
{
  m_kernel->ref_cnt_up ();
  m_event->ref_cnt_up ();

  m_next_wgr         = 0;
  
  m_wgrs_in_the_wild = 0;

  // need to establish the num of dimensions
  int dims = 0;
  if (size_x > 1)
  {
    ++dims;
  }
  if (size_y > 1)
  {
    ++dims;
  }
  if (size_z > 1)
  {
    ++dims;
  }

  m_dimensions = static_cast<Dimensions>(dims);

  if (dims > 1)
  {
    throw std::runtime_error ("Currently only 1D NDRange is supported");
  }

  LOG_LOC_DEBUG ("Created ndrange size_x = %u : size_y = %u : size_z = %u\n",
            m_size_x, m_size_y, m_size_z);
}

FPGAML::NDRange::~NDRange ()
{
  m_kernel->ref_cnt_down ();
  m_event->ref_cnt_down ();
}

bool FPGAML::NDRange::has_more_wgs ()
{
  return m_next_wgr < m_total_size;
}

FPGAML::NDRange::tuple_3u FPGAML::NDRange::get_3d_from_1d (const unsigned one_d_coord)
{
  const unsigned start_z  = one_d_coord % m_size_z; // <- this gives us the z coordinate
  const unsigned xy_coord = one_d_coord / m_size_z; // <- this is corresponding XY coordinate,
                                                 //    i.e. as if Z plane is removed
  const unsigned start_y  = xy_coord % m_size_y; // the remainder is the y coordinate
  const unsigned start_x  = xy_coord / m_size_y; // the result of division is x coordinate

  printf ("1d is %u. X=%u, Y=%u, Z=%u\n", one_d_coord, start_x, start_y, start_z);

  return std::make_tuple (start_x, start_y, start_z);
}

FPGAML::WorkGroupRange FPGAML::NDRange::carve_wgs (
  const size_t num_wgs)
{
  LOG_LOC_DEBUG ("Received a request to carve %lu wgs\n", num_wgs);

  const unsigned start_coord  = m_next_wgr;
  unsigned       wgs_to_carve = 0;
  if (m_next_wgr + num_wgs > m_total_size - 1)
  {
    wgs_to_carve = m_total_size - m_next_wgr;
  }
  else
  {
    wgs_to_carve = num_wgs;
  }

  m_next_wgr += wgs_to_carve;

  ++m_wgrs_in_the_wild;
  
  return WorkGroupRange (*this, start_coord, start_coord +  wgs_to_carve, 0, 1, 0, 1);
#if 0
  // get 3d coordinates from 1d
  // tuple_3u start_coord = get_3d_from_1d (m_current_wg);

  // now advance the current_wg or set it m_total_size
  m_current_wg = (m_current_wg + num_wgs) > (m_total_size - 1) ? 
                      m_total_size : (m_current_wg + num_wgs);

  unsigned end_coord   = m_current_wg - 1;

  printf ("Carving complete: coords are %u-%u", start_coord, end_coord);
  return WorkGroupRange (&m_wgrs_in_the_wild, start_coord, end_coord, 0, 0, 0, 0);
#endif
/*
  const unsigned end_1d = m_current_wg - 1;
  tuple_3u end_coord = get_3d_from_1d (end_1d);
  // need to keep track of wgrs allocated
  ++m_wgrs_in_the_wild;

  printf ("Carving complete. Creating a WGR with params:\n");
  printf ("  start_x = %u : end_x = %u : start_y = %u : end_y = %u : start_z = %u : end_z = %u\n",
              std::get<0>(start_coord), std::get<0>(end_coord),
              std::get<1>(start_coord), std::get<1>(end_coord),
              std::get<2>(start_coord), std::get<2>(end_coord));
  return WorkGroupRange (m_wgrs_in_the_wild,
              std::get<0>(start_coord), std::get<0>(end_coord),
              std::get<1>(start_coord), std::get<1>(end_coord),
              std::get<2>(start_coord), std::get<2>(end_coord));
*/
}

void FPGAML::NDRange::decrement_wgrs_counter ()
{
  // decrement the wgs counter
  // check if there are any more wgs left
  // - if not, set the attached event to COMPLETE
  // - if yes, then do nothing

  --m_wgrs_in_the_wild;

  if (m_wgrs_in_the_wild == 0)
  {
    if (m_event)
    {
      m_event->set_as_complete ();
    }
    // destroy oneself also?
    delete this;
  }
}


