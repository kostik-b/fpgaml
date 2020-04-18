#include <string.h>
#include "../circ_array.hpp"


template<>
void CircArray<int>::print_contents()
{
  // need to populate the map of occupied cells
  char map [_capacity];
  memset (map, 'N', _capacity);
  for (int i = 0; i < _size; ++i)
  {
    map [(_start + i) % _capacity] = 'Y';
  }
  // first print header
  printf ("+");
  for (int i = 0; i < _capacity; ++i)
  {
    printf ("-%02d-", i);
  }
  printf ("+\n");

  // not print contents
  printf ("|");
  for (int i = 0; i < _capacity; ++i)
  {
    if (map[i] == 'N')
    {
      printf ("nnn|");
    }
    else
    {
      printf ("%03d|", _array[i]);
    }
  }

  printf ("\n");

  // now print footer
  printf ("+");
  for (int i = 0; i < _capacity; ++i)
  {
    if (i == _start)
    {
      printf ("-ST-");
    }
    else
    {
      printf ("----");
    }
  }
  printf ("+\n");
}


