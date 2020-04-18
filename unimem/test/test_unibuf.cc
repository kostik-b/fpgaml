#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <stdlib.h>

static const size_t RD_WR_LEN = 100;
static char         s_read_write_buffer[RD_WR_LEN]; // for communicating with the driver

static const size_t s_alloc_request = 1024;

void populate_user_buffer (char* user_buffer, const size_t user_buffer_len)
{
  for (int i = 0; i < user_buffer_len; ++i)
  {
    user_buffer[i] = (char)(i % 25) + 65;
  }
}

void print_char_buffer (const char* buffer, const size_t len)
{
  printf ("\n--- Printing buffer \n");
  for (int i = 0; i < len; ++i)
  {
    printf ("%c ", buffer[i]);
  }
  printf ("\n--- End of buffer\n");
}

int write_alloc_request (const int fd)
{
  // 2. allocate the buffer
  printf ("- Trying to write an allocation request\n");
  s_read_write_buffer[0] = 'C';
  memcpy (s_read_write_buffer + 1, (void*)&s_alloc_request, sizeof(s_alloc_request));
  int rc = write (fd, s_read_write_buffer, sizeof (char)+sizeof(s_alloc_request));

  if (rc == 0)
  {
    printf ("Hmmm ... for some reason the file got closed on writing\n");
    return -1;
  }
  if (rc < 0)
  {
    printf ("error writing to the file\n");
    return -1;
  }

  return rc;
}

int read_alloc_response (
  const int fd,
  char*     unimem_addr,
  size_t&   unimem_addr_len)
{
  printf ("- Trying to read the unimem address structure\n");
  int rc = read (fd, s_read_write_buffer, RD_WR_LEN);

  if (rc == 0)
  {
    printf ("Hmmm ... for some reason the file got closed on reading\n");
    return -1;
  }

  if (rc < 0)
  {
    printf ("error: for some reason we received error on reading\n");
    return -1;
  }

  const size_t read_msg_len     = rc;
  unimem_addr_len  = read_msg_len - 1;

  if (s_read_write_buffer[0] != 'z')
  {
    printf ("error: reading buffer - wrong msg type: %c\n", s_read_write_buffer[0]);
    return -1;
  }
  else
  {
    memcpy (unimem_addr, s_read_write_buffer + 1, read_msg_len - 1);
  }

  return read_msg_len;
}

void write_destroy_request (
  const int     fd,
  const char*   unimem_addr,
  const size_t  unimem_addr_len)
{
  printf ("- Trying to destroy the buffer\n");
  s_read_write_buffer [0] = 'D';
  memcpy (s_read_write_buffer + 1, unimem_addr, unimem_addr_len);

  int rc = write (fd, s_read_write_buffer, unimem_addr_len + 1);
  if (rc == 0)
  {
    printf ("Hmmm - write command caused the file to be closed\n");
  }
  if (rc < 0)
  {
    printf ("error on destroy msg\n");
  }

  if (rc > 0)
  {
    printf ("- Buffer successfully destroyed\n");
  }

}

void copy_user_buffer_to_unimem (
  const int     fd,
  const char*   unimem_addr,
  const size_t  unimem_addr_len)
{
  const size_t E_1 = sizeof(char);
  const size_t E_2 = unimem_addr_len;
  const size_t E_3 = sizeof(char*);
  const size_t E_4 = sizeof(size_t);

  s_read_write_buffer[0] = 'W';
  // write unimem address
  memcpy (s_read_write_buffer + E_1, unimem_addr, unimem_addr_len);

  const size_t user_buffer_len = 512;
  char* user_buffer = (char*)malloc (user_buffer_len);
  populate_user_buffer (user_buffer, user_buffer_len);

  // write the ptr to user buffer
  memcpy (s_read_write_buffer + E_1 + E_2,       &user_buffer, sizeof (char*));
  // write the size of user buffer
  memcpy (s_read_write_buffer + E_1 + E_2 + E_3, &user_buffer_len, sizeof (size_t));

  int rc = write (fd, s_read_write_buffer,  E_1 + E_2 + E_3 + E_4);
  printf ("- user buffer address is %u\n", user_buffer);

  if (rc < 0)
  {
    printf ("Error writing user buffer to unimem\n");
  }
  else
  {
    printf ("Successfully wrote user buffer to unimem\n");
  }

  free (user_buffer);
}

void copy_unimem_buffer_to_user (
  const int     fd,
  const char*   unimem_addr,
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

  const size_t user_buffer_len = 512;
  char* user_buffer = (char*)malloc (user_buffer_len);
  memset (user_buffer, '\0', user_buffer_len);

  // writing the user buffer ptr
  memcpy (s_read_write_buffer + E_1 + E_2, &user_buffer, sizeof (char*));
  // writing the user buffer len
  memcpy (s_read_write_buffer + E_1 + E_2 + E_3, &user_buffer_len, sizeof (size_t));

  int rc = write (fd, s_read_write_buffer,  E_1 + E_2 + E_3 + E_4);
  printf ("- buffer address is %u\n", user_buffer);

  if (rc < 0)
  {
    printf ("Error writing user buffer to unimem\n");
  }
  else
  {
    printf ("Successfully copied user buffer from unimem, now printing the buffer:\n");
    print_char_buffer (user_buffer, user_buffer_len);  
  }

  free (user_buffer);
}

int main (int argc, char** argv)
{
  if (argc < 2)
  {
    printf ("Error: Did not get the dev path\n");
    return 1;
  }

  const char* dev_path = argv [1];
  printf ("- Device path is %s\n", dev_path);

  // 1. open the device
  int fd = open (dev_path, O_RDWR);
  if (fd < 1)
  {
    printf ("error: could not open device\n");
    return 1;
  }

  printf ("- Opened device\n");

  int rc = write_alloc_request (fd);
  if (rc <= 0)
  {
    exit (1);
  }

  // now read in the result
  char    unimem_addr[100];
  size_t  unimem_addr_len = 0;

  if (read_alloc_response (fd, unimem_addr, unimem_addr_len) < 0)
  {
    exit (1);
  }

  // now copy some things into the buffer.
  copy_user_buffer_to_unimem (fd, unimem_addr, unimem_addr_len);
  
  // now copy these things back from the buffer
  copy_unimem_buffer_to_user (fd, unimem_addr, unimem_addr_len);

  // 3. destroy the buffer
  write_destroy_request      (fd, unimem_addr, unimem_addr_len);

  // 4. close the device  
  close (fd);

  return 0;
}
