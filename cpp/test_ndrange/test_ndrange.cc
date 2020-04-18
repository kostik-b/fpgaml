// Copyright QUB 2018

#include <stdio.h>
#include <ndrange.h>

typedef FPGAML::WorkGroupRange WGR;

void check_wgs_finished (FPGAML::NDRange& ndrange)
{
  printf ("NDRange has more wgs = %d, finished = %d\n", ndrange.has_more_wgs (), ndrange.is_finished ());
}

int main (int argc, char** argv)
{
  // 1. create an ndrange - 5*4*3 (=60 wgs in total)
  FPGAML::NDRange ndrange ("mikkelson", "1234abcd", 4, 3, 2);
  // 2. carve wgr of size 19, check if has more wgs, if has finished
  WGR wgr1 = ndrange.carve_wgs (19); check_wgs_finished (ndrange);
  // 3. carve wgr of size 10, check if has more wgs, if has finished
  WGR wgr2 = ndrange.carve_wgs (10); check_wgs_finished (ndrange);
  // 4. carve wgr of size 100, check if has more wgs, if has finished
  WGR wgr3 = ndrange.carve_wgs (100); check_wgs_finished (ndrange);

  // 5. mark 1st, 2nd, 3rd wgr as finshed, check if has finished
  printf ("Marking 1st wgr as finished\n");
  wgr1.mark_as_finished (); check_wgs_finished (ndrange);

  printf ("Marking 2nd wgr as finished\n");
  wgr2.mark_as_finished (); check_wgs_finished (ndrange);

  printf ("Marking 3rd wgr as finished\n");
  wgr3.mark_as_finished (); check_wgs_finished (ndrange);
  // done
  return 0;
}
