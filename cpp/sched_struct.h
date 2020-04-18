// Copyright QUB 2019

#ifndef TmpAddressesH
#define TmpAddressesH

struct TmpAddresses
{
  unsigned long long Board_ID;
  unsigned long long FPGA_ID;
  unsigned long long PR_ID;

  unsigned long long sched_addr;
  unsigned long long dcplr_addr;
  unsigned long long mlbx_wraddr;
  unsigned long long mlbx_rdaddr;
  unsigned long long ICAP_addr;
};

#endif
