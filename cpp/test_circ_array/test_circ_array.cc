// Copyright QUB 2017

#include "../circ_array.hpp"

// TODO: need to turn this into the proper unit test

void iterate_over_array (CircArray<int>& circ_array)
{
  for (int i = 0; i < circ_array.occupancy (); ++i)
  {
    printf ("%d-", circ_array[i]);
  }
  printf ("\n");
}

int main (int argc, char** argv)
{
  CircArray<int>  circ_array (4);

  printf ("------\nCreated 4 elements array, testing push functions\n");
  int a = 4, b = 3, c = 8, d = 5;
  circ_array.push_back (a);
  circ_array.print_contents ();

  circ_array.push_back (b);
  circ_array.print_contents ();

  circ_array.push_front (c);
  circ_array.print_contents ();

  circ_array.push_front (d);
  circ_array.print_contents ();

  printf ("Testing no-overwrite functionality\n");

  CircArray<int>::RC rc = circ_array.push_front (a);
  printf ("RC is %d\n", rc);
  circ_array.print_contents ();

  rc = circ_array.push_back (a);
  printf ("RC is %d\n", rc);
  circ_array.print_contents ();

  iterate_over_array (circ_array);

  printf ("Push back to overwrite\n");
  rc = circ_array.push_back (33, true);
  circ_array.print_contents ();

  printf ("Push front to overwrite\n");
  rc = circ_array.push_front (66, true);
  circ_array.print_contents ();

  printf ("-------------\nNow Testing pop functions\n");
  // now poping values
  int pop_value = 0;
  circ_array.pop_front (pop_value);
  circ_array.print_contents ();
  printf ("Popped value is %d\n", pop_value);

  circ_array.pop_front (pop_value);
  circ_array.print_contents ();
  printf ("Popped value is %d\n", pop_value);
  
  circ_array.pop_front (pop_value);
  circ_array.print_contents ();
  printf ("Popped value is %d\n", pop_value);

  circ_array.pop_back (pop_value);
  circ_array.print_contents ();
  printf ("Popped value is %d\n", pop_value);

  printf ("--------- EMPTY BUFFER -------------\n");
  circ_array.push_front (d);
  circ_array.print_contents ();

  circ_array.push_front (c);
  circ_array.print_contents ();

  circ_array.push_front (a);
  circ_array.print_contents ();

  circ_array.pop_back (pop_value);
  circ_array.print_contents ();
  printf ("Popped value is %d\n", pop_value);


  circ_array.pop_back (pop_value);
  circ_array.print_contents ();
  printf ("Popped value is %d\n", pop_value);

  try
  {
    int op_ret = circ_array[1];
  }
  catch (invalid_index_exception& e)
  {
    printf ("Wrong index exception\n");
  }

  int op_ret = circ_array[0];

  printf ("Operator [0] yielded %d\n", op_ret);

  try
  {
    op_ret = circ_array.pop_front ();
    printf ("pop front yielded %d\n", op_ret);
    op_ret = circ_array.pop_front ();
    printf ("pop front(2) yielded %d\n", op_ret);
  } 
  catch (empty_buffer_exception& e)
  {
    printf ("Caught buffer empty exception\n");
  }

  circ_array.resize_by_two ();
  circ_array.print_contents ();

  circ_array.push_back (7);
  circ_array.push_back (8);
  circ_array.push_front (5);
  circ_array.push_front (4);
  circ_array.print_contents ();
  printf ("Resizing again\n");
  circ_array.resize_by_two ();
  circ_array.print_contents ();

  // -----------
  printf ("Copy ctor empty buffer\n");
  CircArray<int> copy;
  CircArray<int> recvr (copy);

  printf ("Testing pop_at function\nFirst printing the contents of what we have\n");
  circ_array.print_contents ();
  pop_value = circ_array.pop_at (0);
  printf ("Popped value %d at index 0, the buffer now is:\n", pop_value);
  circ_array.print_contents ();

  printf ("Iterating over the buffer: 0 to occupancy\n-");
  iterate_over_array (circ_array);

  printf ("push back 67 and 78\n");
  circ_array.push_back (67);
  circ_array.push_back (78);
  iterate_over_array (circ_array);

  return 0;
}
