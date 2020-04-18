// Copyright QUB 2019

#ifndef fpgaml__sched_config_extractors_HH
#define fpgaml__sched_config_extractors_HH

#include <tuple>
#include <cstring>

#include <cpp/WeeLogger.h>

namespace FPGAML
{

// board id, fpga id, pr id
typedef std::tuple<unsigned long long, unsigned, unsigned> tuple_3;

static void set_tuple_value (
  tuple_3&        tpl,
  const unsigned  index,
  const char*     value)
{
  if (!value)
  {
    return; 
  }

  switch (index)
  {
    case 0:
      std::get<0>(tpl) = strtoull (value, NULL, 10);
      break;
    case 1:
      std::get<1>(tpl) = atoi (value);
      break;
    case 2:
      std::get<2>(tpl) = atoi (value);
      break;
    default:
      LOG_LOC_ERROR ("Setting tuple value to index %d is not supported", index);
      break;
  }
}

class Extractor
{
public:
  Extractor (const char*    key_name,
             const unsigned output_position)
    : m_output_position (output_position)
  {
    m_key_name = key_name;
  }

  const char* get_name    () { return m_key_name; }

  unsigned    get_out_pos () { return m_output_position; }

public:
  const char*     m_key_name;
  const unsigned  m_output_position;
}; // class Extractor

#if 0
class ExtractBoardID : public Extractor_T2
{
public:
  ExtractBoardID () : Extractor_T2 ("BoardID") {}

  void set_value (tuple_2& output, const char* value) override
  {
    if (!value)
    {
      return;
    }
    std::get<0>(output) = strtoull (value, NULL, 10);
  }
}; // class ExtractBoardID

class ExtractFpgaID : public Extractor_T2
{
public:
  ExtractFpgaID () : Extractor_T2 ("FpgaID") {}

  void set_value (tuple_2& output, const char* value) override
  {
    if (!value)
    {
      return;
    }
    std::get<1>(output) = atoi (value);
  }

}; // class ExtractFpgaID
#endif

} // namespace FPGAML

#endif
