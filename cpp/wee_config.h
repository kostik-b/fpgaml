// Copyright QUB 2019

#ifndef fpgaml__weeconfig_stuff_hh
#define fpgaml__weeconfig_stuff_hh

#include <cstdlib>
#include <vector>

namespace FPGAML
{

/*
  What it aspires to be:
    Config access. Agnostic of the actual file.
    Follows the nested structure of XML and JSON.
*/

class WeeConfig
{
public:
  WeeConfig (const char* file_path,
             const char* root_node);

  ~WeeConfig ();

  // returns success or not
  template<typename T>
  bool get_value_for_key (const char* key, T& value);

  // so the idea here is that <T> is a structure corresponding
  // to whatever entity we are trying to extract and extractor
  // is responsible for extracting one or more elements
  template<typename T, typename Extractor>
  bool get_values_for_key (const char*              key,
                           std::vector<Extractor>&  extractors,
                           std::vector<T>&          values);

private:
  struct WeeConfigImpl;
  WeeConfigImpl& m_impl;

};




} // namespace FPGAML

#endif
