// Copyright QUB 2019

#ifndef EcoScale_OCL_DS_HH
#define EcoScale_OCL_DS_HH

// "ds" in the file name stands for Data Structures
// in case you wondered

#include <unordered_set>
#include <cstring>
#include <opencl.h>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

// --- P L A T F O R M   I D -----
// dummy structs
typedef struct _cl_platform_id
{
} _cl_platform_id;

// --- D E V I C E   I D -----
typedef struct _cl_device_id
{
} _cl_device_id;

// --- R E F   C O U N T E R ----
struct ref_counter
{
public:
  ref_counter ()
    : m_counter (1)
  {
  }

  virtual ~ref_counter ()
  {
  }

  void ref_cnt_down ()
  {
    if (m_counter > 0)
    {
      --m_counter;
    }

    if (m_counter == 0)
    {
      // TODO: what do we do here?
      delete this;
    }
  }

  void ref_cnt_up () { ++m_counter; }

  uint32_t counter ()
  {
    return m_counter;
  }

private:
  uint32_t m_counter;
};

// --- C L   C O N T E X T ----
typedef struct _cl_context : public ref_counter
{
public:
  bool is_mem_obj (cl_mem mem)
  {
    return (m_allocated_mems.find (mem) != m_allocated_mems.end ());
  }

  bool add_mem_obj (cl_mem mem)
  {
    return m_allocated_mems.insert (mem).second;
  }

  void remove_mem_obj (cl_mem mem)
  {
    m_allocated_mems.erase (mem);
  }

private:
  std::unordered_set<cl_mem>  m_allocated_mems;
} _cl_context;

// --- C L    P R O G R A M ----
typedef struct _cl_program : public ref_counter
{
  _cl_program (cl_context ctx) { m_context = ctx; }

  // std::string m_sha2; // holds the unique bitstream key - not used
  std::string m_xml_name; // the name of the xml holding the bitstream
  std::string m_build_log;

  cl_context  m_context;
} _cl_program;


// --- T H E   B U F F E R ----

class MLCLCommand;
/*
struct _cl_command_queue : public ref_counter
{
public:
  _cl_command_queue () {}
  ~_cl_command_queue() {}

  cl_int submit_cmd (MLCLCommand* cmd)
  {
    return CL_SUCCESS;
  }

private:
}; // class _cl_command_queue
*/

// this class stores the memory pointer for unimem
// and its size
struct UnimemPtr
{ 
  UnimemPtr ()                       : m_ptr (NULL), m_ptr_size (0) {}
  UnimemPtr (void* ptr, size_t size) : m_ptr (ptr), m_ptr_size (size) {}
    
  void*   m_ptr;
  size_t  m_ptr_size;
}; // class UnimemPtr

// --- M L C L B U F F E R ----
class _cl_mem : public ref_counter
{
public:
  _cl_mem () 
  {
    m_size       = 0;
  }

  void        set_unimem_ptr (UnimemPtr u_ptr, const size_t size) { m_unimem_ptr = u_ptr; m_size = size; }

  UnimemPtr   get_unimem_ptr () { return m_unimem_ptr; }

  unsigned    get_unimem_bus_address ()
  {
    if ((m_unimem_ptr.m_ptr == NULL) || (m_unimem_ptr.m_ptr_size == 0))
    {
      return 0;
    }
    else
    {
      // is the size sufficient?
      if (m_unimem_ptr.m_ptr_size < (sizeof(void*) + sizeof(unsigned*)))
      {
        return 0;
      }
      return *((unsigned*)(m_unimem_ptr.m_ptr + sizeof (void*)));
    }
  }

  size_t      get_size       () { return m_size; }            

private:
  UnimemPtr   m_unimem_ptr;

  size_t      m_size;

};

// --- K E R N E L A R G ----
class KernelArg
{
public:
  KernelArg (const size_t arg_size,
             const void*  arg_value)
    : m_value       (NULL)
    , m_value_size  (arg_size)
  {
    set_values (arg_size, arg_value);
  }

  ~KernelArg ()
  {
    if (m_value)
    {
      free (m_value);
    }
  }

  KernelArg (const KernelArg& copy)
    : m_value      (NULL)
    , m_value_size (copy.m_value_size)
  {
    set_values (copy.m_value_size, copy.m_value);
  }

  void set_values (const size_t arg_size,
                   const void*  arg_value)
  {
    if ((arg_size < 1) || (!arg_value))
    {
      return;
    }

    m_value = static_cast<char*>(malloc (arg_size));
    memcpy (m_value, arg_value, arg_size);
  }

  char*       m_value;
  const int   m_value_size;
};

// --- M L C L K E R N E L ---
class _cl_kernel : public ref_counter
{
public:
  _cl_kernel (cl_program prog) { m_program = prog; m_args_vector.reserve (10); }

  ~_cl_kernel ()
  {
    for (int i = 0; i < m_args_vector.size (); ++i)
    {
      if (m_args_vector[i])
      {
        delete m_args_vector[i];
      }
    }
  }

  void set_args  (const cl_uint arg_index,
                  const size_t  arg_size,
                  const void*   arg_value)
  {
    if ((arg_index + 1) > m_args_vector.size ())
    {
      m_args_vector.resize (arg_index + 1);
    }
    m_args_vector[arg_index] = new KernelArg (arg_size, arg_value);
  }

  cl_program get_program     () { return m_program; }

  const std::vector<KernelArg*>
             get_kernel_args () { return m_args_vector; }

private:
  cl_program              m_program;
  std::vector<KernelArg*> m_args_vector;
};

typedef void (*event_cb_func)(void*);

class _cl_event : public ref_counter
{
public:

  _cl_event ()
  {
    m_event_state  = CL_QUEUED;
    m_error_code   = CL_SUCCESS;
  }

  void    set_as_running      () { m_event_state = CL_RUNNING; }

  void    set_as_complete     ()
  {
    std::unique_lock<std::mutex> lock(m_mutex);

    m_event_state = CL_COMPLETE;

    (*m_cb_func)(m_cb_data);

    m_cond_var.notify_all ();    
  }

  void    set_error           (cl_int err_code) { m_error_code = err_code; }

  void    wait_until_complete ()
  {
    std::unique_lock<std::mutex> lock (m_mutex);

    while ((m_event_state != CL_COMPLETE) &&
          (m_event_state > -1))
    {
      m_cond_var.wait (lock);
    }
  }

  cl_int  get_event_state    () { return m_event_state; }
  cl_int  get_error_code     () { return m_error_code; }

  void    register_on_complete_cb_func (event_cb_func cb,
                                        void*         data)
  {
    m_cb_func = cb;
    m_cb_data = data;
  }

private:
  cl_int                  m_event_state;
  cl_int                  m_error_code;

  std::mutex              m_mutex;
  std::condition_variable m_cond_var;

  event_cb_func           m_cb_func;
  void*                   m_cb_data;
}; // class _cl_event

// --- M L C L C O M M A N D ---
// base class for commands
struct MLCLCommand
{
public:

  enum Type
  {
    WRITE_BUFFER,
    READ_BUFFER,
    NDRANGE,
    EMPTY
  };

  MLCLCommand (Type t)
    : m_command_id    (0)
    , m_command_type  (t)
  {
    m_event = new _cl_event;
  }

  virtual ~MLCLCommand ()
  {
    if (m_event)
    {
      m_event->ref_cnt_down ();
    }
  }

  void      set_command_id    (const int id)  { m_command_id = id; }
  int       get_command_id    ()              { return m_command_id; }

  Type      get_command_type  ()              { return m_command_type; }

  cl_event  get_event         ()              { return m_event; }

private:
  Type      m_command_type;
  int       m_command_id;

  cl_event  m_event;
};

struct MLCLCommandWB : public MLCLCommand
{
  MLCLCommandWB (cl_mem       mem,
                 size_t       offset,
                 size_t       size, 
                 const void*  ptr)
    : MLCLCommand (WRITE_BUFFER)
  {
    m_mem     = mem;

    m_mem->ref_cnt_up ();

    m_offset  = offset;
    m_size    = size;
    m_ptr     = ptr;
  }

  ~MLCLCommandWB ()
  {
    m_mem->ref_cnt_down ();
  }

  cl_mem        m_mem;

  size_t        m_offset;
  size_t        m_size;
  const void*   m_ptr;

}; // class MLCLCommandWB

struct MLCLCommandNDR : public MLCLCommand
{
  MLCLCommandNDR (cl_kernel        kernel,
                  cl_uint          work_dim,
                  const size_t *   global_work_offset,
                  const size_t *   global_work_size,
                  const size_t *   local_work_size)
  : MLCLCommand (NDRANGE)
  {
    m_kernel              = kernel;
    m_kernel->ref_cnt_up ();

    m_work_dim            = work_dim;

    if (work_dim > 1) // will be 3 later
    {
      throw std::runtime_error ("MLCLCommandNDR work dimension is > 1");
    }
    
    memset (m_global_work_offset, 0, sizeof (m_global_work_offset));
    memset (m_global_work_size,   0, sizeof (m_global_work_size));
    memset (m_local_work_size,    0, sizeof (m_local_work_size));

    if (global_work_offset)
    {
      m_global_work_offset[0] = global_work_offset[0];
    }
    if (global_work_size)
    {
      m_global_work_size[0] = global_work_size[0];
      m_global_work_size[1] = 1;
      m_global_work_size[2] = 1;
    }
    if (local_work_size)
    {
      m_local_work_size[0] = local_work_size[0];
    }
  }

  ~MLCLCommandNDR ()
  {
    m_kernel->ref_cnt_down ();
  }

  cl_kernel   m_kernel;
  cl_uint     m_work_dim;
  size_t      m_global_work_offset[3];
  size_t      m_global_work_size[3];
  size_t      m_local_work_size[3];

}; // class MLCLCommandNDR

struct MLCLCommandRB : public MLCLCommand
{
  MLCLCommandRB (cl_mem mem,
                 size_t offset,
                 size_t size, 
                 void * ptr)
  : MLCLCommand (READ_BUFFER)
  {
    m_mem     = mem;
    m_mem->ref_cnt_up ();

    m_offset  = offset;
    m_size    = size;
    m_ptr     = ptr;
  }

  ~MLCLCommandRB ()
  {
    m_mem->ref_cnt_down ();
  }

  cl_mem  m_mem;

  size_t  m_offset;
  size_t  m_size;
  void*   m_ptr;
};


#endif
