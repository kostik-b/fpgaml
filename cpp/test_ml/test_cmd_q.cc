// Copyright QUB 2018

#include <fpgaml.h>
#include <stdlib.h>
#include <stdio.h>

int main (int argc, char** argv)
{
  // special conditions:
  // * releasing the queue before the work is over
  // * releasing the queue with some data in it  - what is the official policy?

  // in the loop - 50 times
  // - 1. get 5 the_buffer instances
  // - 2. submit 2 ndranges to each buffer (as currently is)
  // - 3. wait until it's over -through the events mechanism
  // - 4. release buffers

  // submit 20 ndranges and immediately release the queue

  cl_command_queue cmdq = ml_create_cmd_q (NULL);

  MLCLCommandWB* wbc = new MLCLCommandWB (0, 0, NULL);

  for (int i = 0; i < 10; ++i)
  {
    uint32_t    cmd_id  = 0;
    const ML_RC rc      = ml_submit_cmd (cmdq, wbc, cmd_id);
    if (rc != ML_SUCCESS)
    {
      // something went wrong
      printf ("Could not submit a command to the command queue\n");
      return 1;
    }

    printf ("WB command submitted to the queue, id is: %ld\n", cmd_id);

  }
  sleep (20);

  return 0;
}
