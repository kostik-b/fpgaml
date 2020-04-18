// Copyright QUB 2019

#include "wee_config.h"
#include <stdexcept>
#include <pugixml.hpp>
#include <cstring>
#include <vector>
#include <tuple>
#include <cassert>

#include <cpp/WeeLogger.h>

#include "sched_config_extractors.h"

namespace FPGAML
{

struct WeeConfig::WeeConfigImpl
{
  pugi::xml_document  m_doc;
  pugi::xml_node      m_top_node;
}; // class WeeConfig::WeeConfigImpl

} // namespace FPGAML

FPGAML::WeeConfig::WeeConfig  (
  const char* file_path,
  const char* root_node)
  : m_impl (*(new WeeConfigImpl))
{
  // open the file
  // parse as xml
  // iterate over "outer" nodes and populate the map

  if (!file_path)
  {
    file_path = "./default_config.xml";
  }

  if (!root_node)
  {
    root_node = "ConfigParams";
  }

  if (!m_impl.m_doc.load_file(file_path))
  {
    throw std::runtime_error ("Could not open the config file!");
  }

  m_impl.m_top_node = m_impl.m_doc.child(root_node);

  if (m_impl.m_top_node.empty ())
  {
    throw std::runtime_error ("WeeConfig - top node is empty");
  }
}

FPGAML::WeeConfig::~WeeConfig ()
{
  delete &m_impl;
}

namespace FPGAML
{

// specialization with T=int
template<>
bool WeeConfig::get_value_for_key<int>(const char* key, int& value)
{
  // get the node with the following name
  pugi::xml_node n = m_impl.m_top_node.child (key);

  if (n.empty ())
  {
    return false;
  }
  else
  {
    value = atoi (n.child_value());
    return true;
  }
}

// specialization with T=double
template<>
bool WeeConfig::get_value_for_key<double>(const char* key, double& value)
{
  // get the node with the following name
  pugi::xml_node n = m_impl.m_top_node.child (key);

  if (n.empty ())
  {
    return false;
  }
  else
  {
    value = strtod (n.child_value(), NULL);
    return true;
  }
}

// specialization with T=const char*
template<>
bool WeeConfig::get_value_for_key<const char*>(const char* key, const char*& value)
{
  pugi::xml_node n = m_impl.m_top_node.child (key);

  if (n.empty ())
  {
    return false;
  }
  else
  {
    value = n.child_value();
    return true;
  }
}

// specialization with T=bool
template<>
bool WeeConfig::get_value_for_key<bool>(const char* key, bool& value)
{
  pugi::xml_node n = m_impl.m_top_node.child (key);

  if (n.empty ())
  {
    return false;
  }
  else
  {
    value = (strcmp (n.child_value(), "true") == 0);
    return true;
  }
}

// specialization with T=<vector of tuple_3>
template<>
bool WeeConfig::get_values_for_key<tuple_3, Extractor>(
  const char*                 key,
  std::vector<Extractor>&     extractors,
  std::vector<tuple_3>&       values)
{
  // - get a node for the key and make sure it has children
  // - for every children
  // -- for every extractor
  // -- find the child of current node
  // -- set the value back to extractor
  // -- is_finished? yes -> get tuple and push_back; no -> continue

  pugi::xml_node array_node = m_impl.m_top_node.child(key);

  if (array_node.empty ())
  {
    return false;
  }

  for (auto iter = array_node.begin (); iter != array_node.end (); ++iter)
  {
    tuple_3 output;

    pugi::xml_node current_child = *iter;

    for (int i = 0; i < extractors.size (); ++i)
    {
      Extractor& cur_ex = extractors[i];

      const char* value = current_child.child (cur_ex.get_name ()).child_value ();
      if ((value == NULL) || (strlen(value) < 1))
      {
        LOG_ERROR ("Could not extract value %s from the config file (with key set to %s)",
                    cur_ex.get_name (), key);
        continue;
      }

      set_tuple_value (output, cur_ex.get_out_pos (), value);
    }

    values.push_back (output);
  }

  return true;
}



} // namespace FPGAML
