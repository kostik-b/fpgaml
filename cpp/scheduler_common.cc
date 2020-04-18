// Copyright QUB 2019

#include "scheduler.h"
#include <cpp/WeeLogger.h>

// gap filler
static void PR_Slot(unsigned long long Board_ID,unsigned int FPGA_ID, unsigned int PR_ID,
                unsigned long long *sched_addr,
                unsigned long long *dcplr_addr,
                unsigned long long *mlbx_wraddr,
                unsigned long long *mlbx_rdaddr,
                unsigned long long *ICAP_addr)
{
  *sched_addr   = 0;
  *dcplr_addr   = 0;
  *mlbx_wraddr  = 0;
  *mlbx_rdaddr  = 0;
  *ICAP_addr    = 0;
}

FPGAML::Scheduler::Scheduler (
  unsigned long long Board_ID,
  unsigned           FPGA_ID,
  unsigned           PR_ID,
  const double       polling_interval)
  : m_in_queue          (100) // an arbitrary number
  , m_id                (Board_ID*16 + (FPGA_ID-1)*4 + PR_ID)
  , m_timer             (*this)
  , m_polling_interval  (polling_interval)
{
  m_sched_addr  = -1;
  m_dcplr_addr  = -1;
  m_mlbx_wraddr = -1;
  m_mlbx_rdaddr = -1;
  m_ICAP_addr   = -1;


  // fetch all the appropriate addresses
  // this function will always succeed for now. TODO: add return value and throw exception
  PR_Slot (Board_ID, FPGA_ID, PR_ID,
            &m_sched_addr, &m_dcplr_addr, &m_mlbx_wraddr, &m_mlbx_rdaddr, &m_ICAP_addr);

  LOG_INFO ("Created Scheduler (id:%u) with BoardID - %llu, FPGA ID - %u, PR ID - %u",
              m_id, Board_ID, FPGA_ID, PR_ID);
  LOG_INFO ("Scheduler (id:%u) addresses are:\n"
              "\tSchedAddr  - %llX\n"
              "\tDcprlAddr  - %llX\n"
              "\tMlbxWrAddr - %llX\n"
              "\tMlbxRdAddr - %llX\n"
              "\tICAPAddr   - %llX",
              m_id, m_sched_addr, m_dcplr_addr, m_mlbx_wraddr, m_mlbx_rdaddr, m_ICAP_addr);

  m_is_local = ((Board_ID == 0) && (FPGA_ID == 1));

  LOG_INFO ("Scheduler (id:%u) polling interval is %4.6f, is_local is %d",
             m_id, m_polling_interval, m_is_local);

  /*switch (FPGA_ID)
  { 
    case 1:
      m_mem_base = MEM_BASE_F1;
      break;
    case 2:
      m_mem_base = MEM_BASE_F2;
      break;
    case 3:
      m_mem_base = MEM_BASE_F3;
      break;
    case 4:
      m_mem_base = MEM_BASE_F4;
      break;
    default:
      throw std::runtime_error ("wrong fpga_id");
      break;
  }*/
  //PNT m_mem_base is changed wrt Board_ID, not FPGA_ID
  if (Board_ID >=0 && Board_ID <=3)
    m_mem_base = Board_ID << 5;
  else
    throw std::runtime_error ("wrong board_id");

}


