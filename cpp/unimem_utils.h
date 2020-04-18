// Copyright QUB 2019

#ifndef fpgaml__unimem__mm__utils_HH
#define fpgaml__unimem__mm__utils_HH

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <stdlib.h>

#include <cpp/WeeLogger.h>

static const size_t RD_WR_LEN = 100;
static char         s_read_write_buffer[RD_WR_LEN]; // for communicating with the driver

static int write_alloc_request (
  const int     fd,
  const size_t  alloc_request)
{
  // 2. allocate the buffer
  LOG_LOC_DEBUG ("- Trying to write an allocation request\n");
  s_read_write_buffer[0] = 'C';
  memcpy (s_read_write_buffer + 1, (void*)&alloc_request, sizeof(alloc_request));
  int rc = write (fd, s_read_write_buffer, sizeof (char)+sizeof(alloc_request));

  if (rc == 0)
  {
    LOG_LOC_ERROR ("Hmmm ... for some reason the file got closed on writing\n");
    return -1;
  }
  if (rc < 0)
  {
    LOG_LOC_ERROR ("error writing to the file\n");
    return -1;
  }

  return rc;
}

static int read_alloc_response (
  const int     fd,
  char*         unimem_addr,
  const size_t  unimem_addr_len)
{
  LOG_LOC_DEBUG ("- Trying to read the unimem address structure\n");
  int rc = read (fd, s_read_write_buffer, RD_WR_LEN);

  if (rc == 0)
  {
    LOG_LOC_ERROR ("Hmmm ... for some reason the file got closed on reading\n");
    return -1;
  }

  if (rc < 0)
  {
    LOG_LOC_ERROR ("error: for some reason we received error on reading\n");
    return -1;
  }


  if (s_read_write_buffer[0] != 'z')
  {
    LOG_LOC_ERROR ("error: reading buffer - wrong msg type: %c\n", s_read_write_buffer[0]);
    return -1;
  }

  // check if the buffer is large enough
  if (rc > unimem_addr_len)
  {
    LOG_LOC_ERROR ("error: the buffer supplied for unimem address is too"
            " small (needs to be at least %d bytes)", rc);
    return -1;
  }

  const size_t unidress_len = rc - 1;

  memcpy (unimem_addr, s_read_write_buffer + 1, unidress_len);

  return unidress_len;
}

static void write_destroy_request (
  const int     fd,
  const void*   unimem_addr,
  const size_t  unimem_addr_len)
{
  LOG_LOC_DEBUG ("- Trying to destroy the buffer\n");
  s_read_write_buffer [0] = 'D';
  memcpy (s_read_write_buffer + 1, unimem_addr, unimem_addr_len);

  int rc = write (fd, s_read_write_buffer, unimem_addr_len + 1);
  if (rc == 0)
  {
    LOG_LOC_ERROR ("Hmmm - write command caused the file to be closed\n");
  }
  if (rc < 0)
  {
    LOG_LOC_ERROR ("error on destroy msg\n");
  }

  if (rc > 0)
  {
    LOG_LOC_DEBUG ("- Buffer successfully destroyed\n");
  }

}

static int copy_user_buffer_to_unimem (
  const int     fd,
  const void*   unimem_addr,
  const size_t  unimem_addr_len,
  const void*   user_buffer,
  const size_t  user_buffer_len)
{
  const size_t E_1 = sizeof(char);
  const size_t E_2 = unimem_addr_len;
  const size_t E_3 = sizeof(char*);
  const size_t E_4 = sizeof(size_t);

  s_read_write_buffer[0] = 'W';
  // write unimem address
  memcpy (s_read_write_buffer + E_1, unimem_addr, unimem_addr_len);

  // write the ptr to user buffer
  memcpy (s_read_write_buffer + E_1 + E_2,       &user_buffer, sizeof (char*));
  // write the size of user buffer
  memcpy (s_read_write_buffer + E_1 + E_2 + E_3, &user_buffer_len, sizeof (size_t));

  int rc = write (fd, s_read_write_buffer,  E_1 + E_2 + E_3 + E_4);

  if (rc < 0)
  {
    LOG_LOC_ERROR ("Error writing user buffer to unimem\n");
    return rc;
  }
  else
  {
    LOG_LOC_DEBUG ("Successfully wrote user buffer to unimem\n");
    return rc;
  }
}

static int copy_unimem_buffer_to_user (
  const int     fd,
  void*         user_buffer,
  const size_t  user_buffer_len,
  const void*   unimem_addr,
  const size_t  unimem_addr_len)
{
  // constituent element sizes
  const size_t E_1 = sizeof(char);
  const size_t E_2 = unimem_addr_len;
  const size_t E_3 = sizeof(char*);
  const size_t E_4 = sizeof(size_t);

  s_read_write_buffer[0] = 'R';
  // writing unimem address
  memcpy (s_read_write_buffer + E_1, unimem_addr, unimem_addr_len);

  // writing the user buffer ptr
  memcpy (s_read_write_buffer + E_1 + E_2, &user_buffer, sizeof (char*));
  // writing the user buffer len
  memcpy (s_read_write_buffer + E_1 + E_2 + E_3, &user_buffer_len, sizeof (size_t));

  int rc = write (fd, s_read_write_buffer,  E_1 + E_2 + E_3 + E_4);
  LOG_LOC_DEBUG ("- buffer address is %u\n", user_buffer);

  if (rc < 0)
  {
    LOG_LOC_ERROR ("Error writing user buffer to unimem\n");
    return rc;
  }
  else
  {
    LOG_LOC_DEBUG ("Successfully copied user buffer from unimem.\n");
    return rc;
  }
}

#endif
