// Copyright QUB 2019

#include "unimem_manager.h"
#include "unimem_utils.h"
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include "wee_config.h"
#include "centrum.h"

#include <cpp/WeeLogger.h>

FPGAML::UnimemManager::UnimemManager (WeeConfig& config)
{

  const char* driver_path = NULL;

  if (!config.get_value_for_key ("UnimemDriverPath", driver_path))
  {
    throw std::runtime_error ("No unimem driver path specified.");
  }

  // open the alloc driver
  m_fd = open (driver_path, O_RDWR | O_SYNC);

  if (m_fd < 1)
  {
    throw std::runtime_error ("Could not open the unimem driver.");
  }
}

FPGAML::UnimemManager::~UnimemManager ()
{
  // close the alloc driver
  if (m_fd > 0)
  {
    close (m_fd);
  }
}

UnimemPtr FPGAML::UnimemManager::allocate_unimem (
  const size_t size)
{
  if (m_fd < 1)
  {
    return UnimemPtr(); // empty ptr
  }
  // 1. write allocation request
  // 2. read allocation response
  // 3. malloc data struct for the response

  // 1.
  int rc = write_alloc_request (m_fd, size);

  if (rc < 0)
  {
    LOG_LOC_ERROR ("Could not allocate %lu bytes of unimem", size);
    return UnimemPtr(); // empty ptr
  }

  // 2.
  const size_t addr_buf_len = 100;
  char addr_buffer [addr_buf_len];
  rc = read_alloc_response (m_fd, addr_buffer, addr_buf_len);

  if (rc < 0)
  {
    return UnimemPtr(); // empty ptr
  }
    
  // 3. 
  void* addr_struct = malloc (rc);
  if (!addr_struct)
  {
    return UnimemPtr(); // empty ptr
  }

  memcpy (addr_struct, addr_buffer, rc);

  return UnimemPtr (addr_struct, rc);
}

void  FPGAML::UnimemManager::release_unimem (
  UnimemPtr unimem_ptr)
{
  write_destroy_request (m_fd, unimem_ptr.m_ptr, unimem_ptr.m_ptr_size);
}

bool FPGAML::UnimemManager::copy_to_unimem (
  UnimemPtr     unimem_ptr,
  const void*   buf_ptr,
  const size_t  buf_size)
{
  int rc = copy_user_buffer_to_unimem (m_fd,
                              unimem_ptr.m_ptr,
                              unimem_ptr.m_ptr_size,
                              buf_ptr,
                              buf_size);
  if (rc < 0)
  {
    return false;
  }
  else
  {
    return true;
  }
}

bool FPGAML::UnimemManager::copy_from_unimem (
  void*         buf_ptr,
  UnimemPtr     unimem_ptr, 
  const size_t  buf_size)
{
  int rc = copy_unimem_buffer_to_user (m_fd,
                              buf_ptr,
                              buf_size,
                              unimem_ptr.m_ptr,
                              unimem_ptr.m_ptr_size);

  if (rc < 0)
  {
    return false;
  }
  else
  {
    return true;
  }
}

