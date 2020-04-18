// Copyright QUB 2018

#include "reconf_controller.h"

#include <cpp/WeeLogger.h>

bool FPGAML::ReconfController::check_resource_string (
  const unsigned      island_num,
  const std::string&  resource_string,
  const std::string&  kernel_name)
{
  return true;

#if 0
  // some kernels only run on 0th and 2nd islands
  if (kernel_name == "bitstream_1.xml")
  {
    if (island_num % 2 == 0)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    // for all other cases we have special simulator values
    if ((island_num == 3) || (island_num == 6))
    {
      return false;
    }
    else
    {
      return true;
    }
  }
#endif
}



