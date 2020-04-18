// Copyright QUB 2018

#include "algo_simple.h"
#include "algo_slicing.h"


int main (int argc, char** argv)
{
  printf ("----- Starting test Algo Simple ------\n");
  // create the algo
  FPGAML::AlgoSimple* algo = new FPGAML::AlgoSimple;

  // test 1. schedule hyperbolic
  algo->route_wgs ("hyperbolic", "abcdef", 4);
  

  return 0;
}
