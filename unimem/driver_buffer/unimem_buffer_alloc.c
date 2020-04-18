// Copyright Queen's University Belfast 2018

#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("QUB");
MODULE_DESCRIPTION ("A module to create, destroy and populate UNIMEM arrays");

static bool s_debug = false;
module_param (s_debug, bool, S_IRUGO);

#define LOG_DEBUG(x, ...) if (s_debug) {printk (KERN_CRIT "UBA-DEBUG: " x, ##__VA_ARGS__); }
#define LOG_ERROR(x, ...)              {printk (KERN_CRIT "UBA-ERROR: " x, ##__VA_ARGS__); }
#define LOG_INFO(x, ...)               {printk (KERN_CRIT "UBA-INFO: "  x, ##__VA_ARGS__); }

struct unimem_addr
{
  void*       m_kernel_virt_addr;
  dma_addr_t  m_bus_addr;
  size_t      m_size;
};

/* A UNIMEM address structure is made of
   the virtual kernel address (of the buffer)
   and of the bus (physical) address (needed for transfer). */
struct unimem_return_msg
{
  char                m_type; // just one type for now
  struct unimem_addr  m_unidress;
};

static dev_t                    s_first; // device number
static struct cdev              s_unimem_device; // character device structure
static int                      s_dev_opened = 0; // for now we allow only one user at a time
static struct class*            s_device_class = NULL; // sysfs entry
static struct device*           s_the_device = NULL; // for dma_alloc_coherent
static struct unimem_return_msg s_return_msg;

// IN messages
static const size_t s_C_msg_len       = sizeof (char) + sizeof (size_t);
static const size_t s_D_msg_len       = sizeof (char) + sizeof (struct unimem_addr);
static const size_t s_W_msg_len       = sizeof (char) + sizeof (struct unimem_addr)
                                        + sizeof(char*) + sizeof(size_t);
static const size_t s_R_msg_len       = sizeof (char) + sizeof (struct unimem_addr) +
                                          sizeof(char*) + sizeof(size_t);
// OUT messages (just one)
static const size_t s_z_msg_len       = sizeof (char) + sizeof (struct unimem_addr);

#define MAX_IN_MSG_SIZE s_W_msg_len

#include "buffer_ops.h"

static int      dev_open    (struct inode*  inod, struct file*  fil)
{
  if (s_dev_opened > 0)
  {
    LOG_ERROR ("UNIMEM buffer allocation module can only be opened by one user at a time\n");
    return -1;
  }

  LOG_INFO ("Somebody opened the UNIMEM buffer allocation module\n");

  s_dev_opened = 1;

  return 0;
}

static ssize_t  dev_read    (struct file*   fil,  char*         buf, size_t len, loff_t *off)
{
  LOG_DEBUG ("Somebody trying to read from UNIMEM buffer allocation module\n");

  // 1. if there is no return msg, just return error.
  if (s_return_msg.m_type == '\0')
  {
    LOG_ERROR ("READ - no return msg yet\n");
    return -1;
  }
  // 2. if there is a return msg - check the buf size against the msg size
  if (s_return_msg.m_type != 'z')
  {
    LOG_ERROR ("READ - wrong return msg type (how can this be?)!!\n");
    return -1;
  }

  if (len < s_z_msg_len)
  {
    LOG_ERROR ("READ - insufficient buffer length for return msg\n");
    return -1;
  }

  // 3. if all is good, write the return msg to the buffer
  const char z_letter = 'z';
  copy_to_user (buf, &z_letter, 1);
  copy_to_user (buf + 1, &(s_return_msg.m_unidress), sizeof (s_return_msg.m_unidress));
  LOG_ERROR ("KB--: Wrote %u bytes to user\n", sizeof (s_return_msg.m_unidress)+1);

  // no more return msgs
  s_return_msg.m_type = '\0';

  *off += s_z_msg_len;

  return s_z_msg_len;
}

/*
  *** MSGS ***
  - ALLOC:        Type - C. Fields: Type, Size.
  - DEALLOC:      Type - D. Fields: Type, unimem_addr structure.
  - BUFFER WRITE: Type - W. Fields: Type, unimem_addr structure, user buffer, len
  - BUFFER READ:  Type - R, Parameter - k-buffer ptr, u-buffer ptr, size.
*/

static ssize_t  dev_write   (struct file*   fil,  const char*   buf, size_t len, loff_t *off)
{
  LOG_DEBUG ("Somebody trying to write to UNIMEM buffer"
                    " allocation module, msg size is %d\n", len);

  // 1. read the msg.
  if (len > MAX_IN_MSG_SIZE)
  {
    LOG_ERROR ("WRITE failed - the control msg is too big (%d bytes)!\n", len);
    LOG_ERROR ("--> Max msg size is %d", MAX_IN_MSG_SIZE);
    *off += len;
    return -1;
  }
  char k_buf [MAX_IN_MSG_SIZE];
  memset (k_buf, '\0', MAX_IN_MSG_SIZE);
  copy_from_user (k_buf, buf, len);

  // 2. check the value.
  if ((k_buf[0] != 'C') && (k_buf[0] != 'D') &&
        (k_buf[0] != 'W') && (k_buf[0] != 'R'))
  {
    LOG_ERROR ("WRITE wrong control message type %c\n", k_buf[0]);
    *off += len;
    return -1;
  }

  // 3. act upon the msg type
  int rc = -1;
  switch (k_buf[0])
  {
    case 'C':
      rc = handle_write_C (k_buf, len);
      break;
    case 'D':
      rc = handle_write_D (k_buf, len);
      break;
    case 'W':
      rc = handle_write_W (k_buf, len);
      break;
    case 'R':
      rc = handle_write_R (k_buf, len);
      break;
    default:
      LOG_ERROR ("WRITE unknown control msg type - shouldn't happen!\n");
      break;
  }

#if 0
  if (rc > 0)
  {
    *off += rc;
  }
#endif

  // we mark it as read in any case
  *off += len;

  return rc;
}

static int      dev_release (struct inode*  inod, struct file*  fil)
{
  LOG_INFO ("Somebody released the UNIMEM buffer allocation module\n");

  s_dev_opened = 0;

  return 0;
}

static struct file_operations fops = {
        .read     = dev_read,
        .write    = dev_write,
        .open     = dev_open,
        .release  = dev_release,
};

static int __init init_func (void)
{
  LOG_INFO ("--------------------\n");
  LOG_INFO ("Initializing UNIMEM buffer allocation module\n");

  // get device number - allocated automatically to us
  if (alloc_chrdev_region (&s_first, 0, 1, "unimem_buffer_allocator") < 0)
  {
    LOG_ERROR ("Failed to allocate unimem device number!!!\n");
    return -1;
  }

  // initialize the cdev structure - connect to file_operations
  cdev_init (&s_unimem_device, &fops);

  s_unimem_device.owner = THIS_MODULE;

  // register the cdev structure with the kernel
  // all the fops are now available to the user
  if (cdev_add (&s_unimem_device, s_first, 1) < 0)
  {
    LOG_ERROR ("Failed to add unimem buffer allocation module!!!\n");
    unregister_chrdev_region (s_first, 1);
    return -1;
  }

  // create a class entry in sysfs (an alternative would be a mknode command)
  s_device_class = class_create (THIS_MODULE, "unimem_buffer_allocator");

  if (!s_device_class)
  {
    LOG_ERROR ("Could not create unimem buffer allocator class in sysfs, carrying on without one.\n");
  }

  // this will create an entry in /dev
  s_the_device = device_create (s_device_class, NULL, s_first, NULL, "unimem_buffer_allocator");
  if (s_the_device == NULL)
  {
    LOG_ERROR ("Could not create unimem buffer allocator entry in /dev, carrying on without one\n");
  }

  // initialize some vars
  s_return_msg.m_type = '\0';
  
  return 0; // success
}

static void __exit cleanup_func (void)
{
  LOG_INFO ("Unloading UNIMEM buffer allocation module\n");

  // remove device operations from the system
  cdev_del (&s_unimem_device);

  // now destroy the class in sysfs and device in /dev
  device_destroy (s_device_class, s_first);
  class_destroy (s_device_class);

  // deregister the device number
  unregister_chrdev_region (s_first, 1);
}

module_init (init_func);
module_exit (cleanup_func);
