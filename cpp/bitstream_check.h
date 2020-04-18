// Copyright QUB 2019

#ifndef ecoscale_bits_check_hh
#define ecoscale_bits_check_hh

#include <unordered_set>
#include <string>
#include <unordered_map>

/*
  This class is needed for IL (clBuildProgram)
  to check if a particular xml bitstream exists.
  We could do the check inside BitstramManager,
  but this would involve creating locks, etc.
*/

namespace FPGAML
{

class BitstreamCheck
{
public:
  template<typename Iter>
  BitstreamCheck (Iter begin, Iter end)
    : m_bitstreams_list (begin, end)
  {
  }

  bool  check_bitstream_exists (const string& xml_file_name)
  {
    return static_cast<bool>(m_bitstreams_list.count (xml_file_name));
  }
private:
  std::unordered_set<string>  m_bitstreams_list;
}; // class BitstreamCheck

class WorkGroupSizeCheck
{
public:
  template<typename Iter>
  WorkGroupSizeCheck (Iter begin, Iter end)
  {
    // for every element insert an entry
    Iter i = begin;
    for ( ; i != end; ++i)
    {
      const bitstream_s& entry = i->second;

      const size_t wgroup_size = entry.wgroup_size;

      if (wgroup_size > 0)
      {
        m_work_group_sizes.insert (std::make_pair (i->first, wgroup_size));
      }
    }
  }

  bool  check_wg_size (const string& xml_file_name, const size_t wg_size)
  {
    auto rc = m_work_group_sizes.find (xml_file_name);

    if (rc == m_work_group_sizes.end ())
    {
      return false;
    }
    else
    {
      return (wg_size == rc->second);
    }
  }

private:
  std::unordered_map<string, size_t>  m_work_group_sizes;

}; // class BitstreamMapCheck

} // namespace FPGAML

#endif
