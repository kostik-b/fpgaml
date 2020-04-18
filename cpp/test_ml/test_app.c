// Copyright QUB 2018

#include <fpgaml.h>
#include <stdlib.h>
#include <stdio.h>

int main (int argc, char** argv)
{
  void* ptr1 = NULL;
  void* ptr2 = NULL;

  void** data_ptrs =  (void**)malloc (sizeof(void*)*2);
  data_ptrs[0] = ptr1;
  data_ptrs[1] = ptr2;

  size_t ptr_lens[2] = {0, 0};
  
  ML_RC rc = ml_submit_ndrange ("hyperbolic", 100, 1, 1, data_ptrs, 2, ptr_lens);

  printf ("Just submitted \"hyperbolic\" ndrange, result is %d\n", rc);

  rc = ml_submit_ndrange ("increment3d", 73, 1, 1, data_ptrs, 2, ptr_lens);

  printf ("Just submitted \"increment3d\" ndrange, result is %d\n", rc);

  sleep (20);

  return 0;
}
