// Copyright QUB and TSI 2019

#ifndef ecoscale__island_scheduler_hh
#define ecoscale__island_scheduler_hh

#include <stdlib.h>
#include <ocl_ds.h>

#include "wee_config.h"

namespace FPGAML
{

class UnimemManager
{
public:
  UnimemManager (WeeConfig& config);
  ~UnimemManager();

  // if successfull returns a UNIMEM pointer, if failed
  // returns a NULL

  UnimemPtr allocate_unimem   (const size_t size);

  void      release_unimem    (UnimemPtr    unimem_ptr);

  // false - failed, true - succeeded
  bool      copy_to_unimem    (UnimemPtr    unimem_ptr,
                               const void*  buf_ptr,
                               const size_t buf_size);

  bool      copy_from_unimem  (void*        buf_ptr,
                               UnimemPtr    unimem_ptr,
                               const size_t buf_size);

private:

  int   m_fd; // driver file descriptor

}; // class UnimemManager

} // namespace FPGAML

#endif
