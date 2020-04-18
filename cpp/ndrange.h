// Copyright QUB 2018

#ifndef ecoscale_ndrange_h
#define ecoscale_ndrange_h

#include <string>
#include <stdint.h>

#include <tuple>
#include <vector>

#include <ocl_ds.h>

namespace FPGAML
{

//typedef std::tuple<void*, size_t> KernelArg;

/*
  The idea is that an ndrange contains the entire spectrum of the
  wgs. The "user" then "carves out" the necessary WorkGroupRanges.
*/

class NDRange; // forward declaration

class WorkGroupRange
{
public:
  WorkGroupRange ();

  WorkGroupRange (NDRange&        ndrange,
                  const unsigned  start_x, const unsigned size_x,
                  const unsigned  start_y, const unsigned size_y,
                  const unsigned  start_z, const unsigned size_z);

  void     mark_as_finished   ();

  void     set_start_time     (const uint64_t start_time) { m_execution_start_time = start_time; };

  uint64_t get_execution_cost ()                          { return m_execution_cost; }
  void     set_execution_cost (const uint64_t exec_cost)  { m_execution_cost = exec_cost; }

  unsigned get_total_size     ()                          { return m_total_size; }

  const std::string& 
           get_kernel_name    (); 

  const std::vector<KernelArg*>
           get_kernel_args    ();

  unsigned get_start_x        () { return m_start_x; } 
  unsigned get_size_x         () { return m_size_x; } 

  unsigned get_start_y        () { return m_start_y; } 
  unsigned get_size_y         () { return m_size_y; } 

  unsigned get_start_z        () { return m_start_z; } 
  unsigned get_size_z         () { return m_size_z; } 

private:
  NDRange*  m_parent_ndrange; // the completion is reported back to parent

  unsigned  m_start_x;
  unsigned  m_size_x;

  unsigned  m_start_y;
  unsigned  m_size_y;

  unsigned  m_start_z;
  unsigned  m_size_z;

  uint64_t  m_execution_cost; // includes reconfiguration
  uint64_t  m_execution_start_time;

  unsigned  m_total_size;
}; // class WorkGroupRange

enum class Dimensions
{
  ONE_D   = 1,
  TWO_D,
  THREE_D
};


// at this moment this class operates on the level of workgroups
// it doesn't know anything about the workitems - just no need for now
class NDRange
{
public:
  NDRange                       (cl_kernel          kernel,
                                 cl_event           event,
                                 std::string&       resource_string,
                                 const unsigned     size_x,  // num of wgs along the x axis
                                 const unsigned     size_y,  // num of wgs along the y axis
                                 const unsigned     size_z); // num of wgs along the z axis

  ~NDRange ();

  WorkGroupRange      carve_wgs      (const size_t       num_wgs);

  bool                has_more_wgs   ();

  bool                is_finished    () { return (!has_more_wgs () && (m_wgrs_in_the_wild == 0)); }

  const std::string&  get_kernel_name() { return m_kernel->get_program()->m_xml_name; }

  void                set_resource_string (const std::string& resource_string);
  const std::string&  get_resource_string () { return m_resource_string; }

  const unsigned      get_total_size () { return m_total_size; } // for now, just for 1D

  void                decrement_wgrs_counter ();
private:
  typedef std::tuple<unsigned, unsigned, unsigned> tuple_3u;

  tuple_3u get_3d_from_1d (const unsigned one_d_coord);

public:
  cl_kernel         m_kernel;
private:
  cl_event          m_event; // this command's event
  std::string       m_resource_string;

  const unsigned    m_start_x; // this is offset as per OpenCL spec, not used yet
  const unsigned    m_size_x;

  const unsigned    m_start_y;
  const unsigned    m_size_y;

  const unsigned    m_start_z;
  const unsigned    m_size_z;

  unsigned          m_next_wgr; // the idea is that we collapse 2D/3D into 1D and do the counting
  const unsigned    m_total_size;

  // we need to keep track of the total wgrs allocated
  unsigned          m_wgrs_in_the_wild;

  Dimensions        m_dimensions;

}; // class NDRange


} // namespace FPGAML

#endif
