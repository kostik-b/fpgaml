// Copyright QUB 2018

static int handle_write_C (
  const char*   k_buf,
  const size_t  len)
{
  // 1. check msg length
  LOG_DEBUG ("Trying to allocate a unimem buffer\n");
  if (len != s_C_msg_len)
  {
    LOG_ERROR ("Allocate buffer - wrong size of control msg\n");
    return -1;
  }

  // 1a. check buffer len
  const size_t alloc_len = *(size_t*)(k_buf + 1);
  LOG_DEBUG ("Requested buffer size of %lu bytes\n", alloc_len);

  // 2. get the unimem address struct
  struct unimem_addr* unidress = &(s_return_msg.m_unidress); // a shorthand

  // 3. allocate the bufer
  unidress->m_kernel_virt_addr = vmalloc (alloc_len);
  unidress->m_bus_addr = 13; // emulating some value
  /*
  unidress->m_kernel_virt_addr = dma_alloc_coherent (s_the_device, alloc_len,
                                  &(unidress->m_bus_addr), GFP_KERNEL);
  */
  if (unidress->m_kernel_virt_addr == NULL)
  {
    LOG_ERROR ("Could not allocate dma coherent buffer\n");
    LOG_ERROR ("Dma bus addr is %llu\n", unidress->m_bus_addr);
    return -ENOMEM;
  }
  LOG_DEBUG ("Virtual address is %u\n", unidress->m_kernel_virt_addr);
  LOG_DEBUG ("Bus address is %llu\n", unidress->m_bus_addr);

  unidress->m_size = alloc_len;

  // 4. set the return struct
  s_return_msg.m_type = 'z';

  return len;
}


static int handle_write_D (
  const char*   k_buf,
  const size_t  len)
{
  // 1. check msg length
  LOG_DEBUG ("Trying to destroy a buffer\n");
  if (len != s_D_msg_len)
  {
    LOG_ERROR ("Destroy buffer - wrong size of control msg\n");
    return -1;
  }

  // 2. cast the structure
  struct unimem_addr* unidress = (struct unimem_addr*)(k_buf + 1);

  LOG_DEBUG ("Virtual address is %u\n", unidress->m_kernel_virt_addr);
  LOG_DEBUG ("Bus address is %u\n", unidress->m_bus_addr);
  // 3. free the memory
  vfree (unidress->m_kernel_virt_addr); // <---- "emulator" value
/*
  dma_free_coherent (s_the_device, unidress->m_size,
                                   unidress->m_kernel_virt_addr,
                                   unidress->m_bus_addr);
*/

  // job done!
  return len;
}

static int handle_write_W (
  const char*   k_buf,
  const size_t  len)
{
  // 1. check msg length
  LOG_DEBUG ("Trying to copy to unimem buffer\n");
  if (len != s_W_msg_len)
  {
    LOG_ERROR ("Copy to unimem - wrong size of control msg\n");
    return -1;
  }

  // 2. cast the structure
  struct unimem_addr* unidress = (struct unimem_addr*)(k_buf + 1);

  LOG_DEBUG ("Virtual address is %u\n", unidress->m_kernel_virt_addr);
  LOG_DEBUG ("Bus address is %u\n", unidress->m_bus_addr);

  const char*  user_buffer_ptr  = *(const char**)(k_buf + sizeof(char) + sizeof(struct unimem_addr));

  const size_t user_buf_len     = *(size_t*)(k_buf + sizeof(char) + sizeof(struct unimem_addr) + 
                                             sizeof (char*));

  LOG_DEBUG ("User buffer ptr addr is %u, buf len is %lu\n", user_buffer_ptr,
                                                                               user_buf_len);

  unsigned rc = copy_from_user (unidress->m_kernel_virt_addr, user_buffer_ptr, user_buf_len);
  if (rc != 0)
  {
    LOG_ERROR ("Could not copy user buffer in msg W, rc value is %u\n", rc);
    return -1;
  }

  return s_W_msg_len; // all is good
}

static int handle_write_R (
  const char*   k_buf,
  const size_t  len)
{
  // 1. check msg length
  LOG_DEBUG ("Trying to copy from unimem buffer into user buffer\n");
  if (len != s_R_msg_len)
  {
    LOG_ERROR ("Copy from unimem - wrong size of control msg\n");
    return -1;
  }

  // 2. cast the structure
  struct unimem_addr* unidress = (struct unimem_addr*)(k_buf + 1);

  LOG_DEBUG ("Virtual address is %u\n", unidress->m_kernel_virt_addr);
  LOG_DEBUG ("Bus address is %u\n", unidress->m_bus_addr);

  char*        user_buffer_ptr  = *(const char**)(k_buf + sizeof(char) + sizeof(struct unimem_addr));

  const size_t user_buf_len     = *(size_t*)(k_buf + sizeof(char) + sizeof(struct unimem_addr) + 
                                             sizeof (char*));

  LOG_DEBUG ("User buffer ptr addr is %u, buf len is %lu\n", user_buffer_ptr,
                                                                               user_buf_len);

  unsigned rc = copy_to_user (user_buffer_ptr, unidress->m_kernel_virt_addr, user_buf_len);
  if (rc != 0)
  {
    LOG_ERROR ("Could not copy UNIMEM buffer in msg R, rc is %u\n", rc);
    return -1;
  }

  return s_R_msg_len; // all is good
}
