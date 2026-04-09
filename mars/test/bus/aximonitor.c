#include "common.h"
#include "system_common.h"

#define AXIMON_INTNUM     AXI_MON_INTR

#define AXIMON_M3_1_BASE  0x08008200
#define AXIMON_M3_2_BASE  0x08008280

#define AXIMON_CONTROL_M3_1    (AXIMON_M3_1_BASE + 0x000)
#define AXIMON_HITSEL_M3_1     (AXIMON_M3_1_BASE + 0x004)
#define AXIMON_ADDR_ST_L_M3_1  (AXIMON_M3_1_BASE + 0x008)
#define AXIMON_ADDR_ST_H_M3_1  (AXIMON_M3_1_BASE + 0x00C)

#define AXIMON_ADDR_SP_L_M3_1  (AXIMON_M3_1_BASE + 0x010)
#define AXIMON_ADDR_SP_H_M3_1  (AXIMON_M3_1_BASE + 0x014)

#define AXIMON_CONTROL_M3_2    (AXIMON_M3_2_BASE + 0x000)
#define AXIMON_HITSEL_M3_2     (AXIMON_M3_2_BASE + 0x004)
#define AXIMON_ADDR_ST_L_M3_2 (AXIMON_M3_2_BASE + 0x008)
#define AXIMON_ADDR_ST_H_M3_2  (AXIMON_M3_2_BASE + 0x00C)

#define AXIMON_ADDR_SP_L_M3_2  (AXIMON_M3_2_BASE + 0x010)
#define AXIMON_ADDR_SP_H_M3_2  (AXIMON_M3_2_BASE + 0x014)


//****************************************************************
/*
0000_8000 AXI Monitor for AXI1 (VIP realtime) Write
          AXI Monitor for AXI1 (7x4) Write
          Latency Bin
0000_8080 AXI Monitor for AXI1 (VIP realtime) Read
          AXI Monitor for AXI1 (7x4) Read
          Latency Bin
0000_8100 AXI Monitor for AXI2 (VIP offline) Write
          AXI Monitor for AXI2、5 (7x4) Write
          Latency Bin
0000_8180 AXI Monitor for AXI2 (VIP offline) Read
          AXI Monitor for AXI2、5 (7x4) Read
          Latency Bin
0000_8200 AXI Monitor for AXI3 (CPU) Write
          AXI Monitor for AXI3、6 (7x4) Write
0000_8280 AXI Monitor for AXI3 (CPU) Read
          AXI Monitor for AXI3、6 (7x4) Read
0000_8300 AXI Monitor for AXI4 (TPU) Write
          AXI Monitor for AXI4、0 (7x4) Write
0000_8380 AXI Monitor for AXI4 (TPU) Read
          AXI Monitor for AXI4、0 (7x4) Read
0000_8400 AXI Monitor for AXI5 (VC) Write
          AXI Monitor for AXI2、5 (7x4) Write
0000_8480 AXI Monitor for AXI5 (VC) Read
          AXI Monitor for AXI2、5 (7x4) Read
0000_8500 AXI Monitor for AXI6 (HSPERI) Write
          AXI Monitor for AXI3、6 (7x4) Write
0000_8580 AXI Monitor for AXI6 (HSPERI) Read
          AXI Monitor for AXI3、6 (7x4) Read
*/
//****************************************************************

int AXIMon_IRQCallBack(int irqn, void *priv)
{
  printf("AXIMon_IRQCallBack() irqn=0x%x, M3_1 reg=0x%x\n", irqn, RAW_READ32(AXIMON_CONTROL_M3_1));
  printf("AXIMon_IRQCallBack() irqn=0x%x, M3_2 reg=0x%x\n", irqn, RAW_READ32(AXIMON_CONTROL_M3_2));

  //clear INT
  RAW_WRITE32(AXIMON_CONTROL_M3_1, 0x00200020);
  RAW_WRITE32(AXIMON_CONTROL_M3_2, 0x00200020);
  RAW_WRITE32(AXIMON_CONTROL_M3_1, 0x00200000);
  RAW_WRITE32(AXIMON_CONTROL_M3_2, 0x00200000);

  return 0;
}

#define TEST_ARRAY_LEN      0x100
#define DRAM_START_ADDRESS  0x80000000

uint32_t  test_data_array[TEST_ARRAY_LEN] = {0};
int AXIMon_HitMode(void)
{
  uint32_t regValue = 0;
  uint64_t tmp_address = 0;

  //************************************************************************
  //set hit mode into address range
  RAW_WRITE32(AXIMON_HITSEL_M3_1, 0x1);

  //set AXIMON_M3_1_BASE mode
  regValue = RAW_READ32(AXIMON_CONTROL_M3_1);
  RAW_WRITE32(AXIMON_CONTROL_M3_1, regValue | 0x01000100);

  //set address low bound
  tmp_address = (uint64_t)(&test_data_array[0]) - (uint64_t)DRAM_START_ADDRESS;
  RAW_WRITE32(AXIMON_ADDR_ST_L_M3_1, (uint32_t)tmp_address);
  RAW_WRITE32(AXIMON_ADDR_ST_H_M3_1, (uint32_t)(tmp_address >> 32));

  //set address upper bound
  tmp_address = (uint64_t)(&test_data_array[0]) + (uint64_t)TEST_ARRAY_LEN - (uint64_t)DRAM_START_ADDRESS;
  RAW_WRITE32(AXIMON_ADDR_SP_L_M3_1, (uint32_t)tmp_address);
  RAW_WRITE32(AXIMON_ADDR_SP_H_M3_1, (uint32_t)(tmp_address >> 32));

  //enable AXIMon and IRQ
  regValue = RAW_READ32(AXIMON_CONTROL_M3_1);
  RAW_WRITE32(AXIMON_CONTROL_M3_1, regValue | 0x00110011);

  printf("AXIMon_HitMode() write hit trigger\n");
  *(volatile uint32_t *)(&test_data_array[0]) = 0x1000;

  //************************************************************************
  //set hit mode into address range
  RAW_WRITE32(AXIMON_HITSEL_M3_2, 0x1);

  //set AXIMON_M3_2_BASE mode
  regValue = RAW_READ32(AXIMON_CONTROL_M3_2);
  RAW_WRITE32(AXIMON_CONTROL_M3_2, regValue | 0x01000100);

  //set address low bound
  tmp_address = (uint64_t)(&test_data_array[0]) - (uint64_t)DRAM_START_ADDRESS;
  RAW_WRITE32(AXIMON_ADDR_ST_L_M3_2, (uint32_t)tmp_address);
  RAW_WRITE32(AXIMON_ADDR_ST_H_M3_2, (uint32_t)(tmp_address >> 32));

  //set address upper bound
  tmp_address = (uint64_t)(&test_data_array[0]) + (uint64_t)TEST_ARRAY_LEN - (uint64_t)DRAM_START_ADDRESS;
  RAW_WRITE32(AXIMON_ADDR_SP_L_M3_2, (uint32_t)tmp_address);
  RAW_WRITE32(AXIMON_ADDR_SP_H_M3_2, (uint32_t)(tmp_address >> 32));

  //enable AXIMon and IRQ
  regValue = RAW_READ32(AXIMON_CONTROL_M3_2);
  RAW_WRITE32(AXIMON_CONTROL_M3_2, regValue | 0x00110011);

  printf("AXIMon_HitMode() read hit trigger\n");
  *(volatile uint32_t *)(&test_data_array[0]);

  return 0;
}


int AXIMon_Main(void)
{
  printf("AXIMon_Main()...\n");

  //init interrupt
  enable_irq(AXIMON_INTNUM);
  request_irq(AXIMON_INTNUM, AXIMon_IRQCallBack, 0, "AxiMonInt", NULL);

  //test monitor mode
  AXIMon_HitMode();

  //test hit mode
  //BusRec_HitMode();

  return 0;
}

