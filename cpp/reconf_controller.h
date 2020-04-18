// Copyright QUB 2018

#ifndef es_reconf_controller_hh
#define es_reconf_controller_hh

#include <string>

namespace FPGAML
{

class ReconfController
{
public:
  bool check_resource_string (const unsigned      island_num,
                              const std::string&  resource_string,
                              const std::string&  kernel_name);

private:
}; // class ReconfController

} // namespace

#endif
