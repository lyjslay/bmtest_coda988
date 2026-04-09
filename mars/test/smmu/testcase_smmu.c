#include "boot_test.h"
#include "uart.h"
#include "cli.h"
#include "timer.h"
#include "testcase_smmu.h"
#include <mmio.h>
#include "tv_gen_fw_arraybase.h"

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint8_t u8;

#define MB (1024 * 1024)
#define TIU_DESC_BYTE (112)
#define TDMA_DESC_BYTE (64)
#define TERM_SIM_ADDR 0x00FFFF00
#define TDMA_CMDBUF_BASE_ADDR 0x100000000
#define TDMA_CMDBUF_LEN 0x10

static u64 g_tdma_cmdbuf_start = TDMA_CMDBUF_BASE_ADDR;
static const u64 g_tdma_ctrl_reg_base = 0x0C100000;

// bit 0 -> bValid : 1
// bit 1 -> bReadable : 1;
// bit 2 -> bWriteable : 1;
// bit 3 -> bExecutable : 1;
// bit 4 ->  bUserModeAccessible : 1;
// bit 5 ->  bGlobal : 1;
// bit 6 ->  bA : 1;
// bit 7 ->  bD : 1;
// bit 8-9 ->  Reserved : 2;
// bit 10-18 ->  PFN0_9bits : 9;
// bit 19-33 ->  PFN1_15bits : 15;
// bit 34-63 -> Reserved_30bits : 30;
struct SmmuPageTable __attribute__((section (".smmu.page.table"))) st_SmmuPageTable[8] = {
// TTBR
  { 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ,0x00},
  { 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ,0x00},
  { 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ,0x00},
  { 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ,0x00},
  { 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ,0x00},
  { 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ,0x00},
  { 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ,0x00},
  { 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ,0x00},
};

struct SmmuPageTable __attribute__((section (".smmu.page.table"))) st_SmmuPageTable1[8] = {
// offset 0x40
  { 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ,0x00},
  { 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ,0x00},
  { 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ,0x00},
  { 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ,0x00},
  { 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ,0x00},
  { 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ,0x00},
  { 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ,0x00},
  { 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ,0x00},
};

// bmk1880v2_tdma_g2l_general_copy
// test_param_g2l: 1 bytes from 0:0 to 0
// I1008 11:20:40.024730 18934 tdma.cpp:745] TDMA TG2L General Copy
// 00000000 = 0001041d, 00000004 = 00000000, 00000008 = 00000000, 0000000c = 00000000
// 00000010 = 00010001, 00000014 = 00000001, 00000018 = 00010001, 0000001c = 00000020
// 00000020 = 00010001, 00000024 = 00010001, 00000028 = 00010001, 0000002c = 00000000
// 00000030 = 00000000, 00000034 = 01000001, 00000038 = 00000000, 0000003c = 00000000

static unsigned int __attribute__((section (".tpu.cmd.buf"))) ui_tpu_cmd_buf[0x80] = {
// normal use
0x0001041d, 0x00000000, 0x00000000, 0x00000000, // 0x00
0x00010001, 0x00000001, 0x00010001, 0x00000040, // 0x10
0x00010001, 0x00010001, 0x00010001, 0x00000000, // 0x20
0x00000000, 0x01000001, 0x00000000, 0x00000000, // 0x30
// page fault use
0x0001041d, 0x00000000, 0x00000000, 0x00000000, // 0x00
0x00010001, 0x00000001, 0x00010001, 0x00000040, // 0x10
0x00010001, 0x00010001, 0x00010001, 0x00000000, // 0x20
0x04000000, 0x01000001, 0x00000000, 0x00000000, // 0x30
};

static unsigned char __attribute__((section (".pattern.buf"))) uc_array_pattern[0x100] = {
0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00,
0xF0, 0xE0, 0xD0, 0xC0, 0xB0, 0xA0, 0x90, 0x80, 0x70, 0x60, 0x50, 0x40, 0x30, 0x20, 0x10, 0x00,
0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00,
0xF0, 0xE0, 0xD0, 0xC0, 0xB0, 0xA0, 0x90, 0x80, 0x70, 0x60, 0x50, 0x40, 0x30, 0x20, 0x10, 0x00,
0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00,
0xF0, 0xE0, 0xD0, 0xC0, 0xB0, 0xA0, 0x90, 0x80, 0x70, 0x60, 0x50, 0x40, 0x30, 0x20, 0x10, 0x00,
0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00,
0xF0, 0xE0, 0xD0, 0xC0, 0xB0, 0xA0, 0x90, 0x80, 0x70, 0x60, 0x50, 0x40, 0x30, 0x20, 0x10, 0x00,
0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static unsigned char __attribute__((section (".tpu.sram"))) uc_tpu_sram[0x100] = {
0
};

static void enable_pmu(void)
{
  // clear pmu value
  mmio_write_32( (SMMU_PTW_BASE+PTW_PMU_CTRL), 0);
  mmio_write_32( (SMMU_PTW_BASE+PTW_PMU_CTRL), PTW_PMU_CLEAR_BIT);
  mmio_write_32( (SMMU_LAT4_BASE+LAT_PMU_CONTROL), 0);
  mmio_write_32( (SMMU_LAT4_BASE+LAT_PMU_CONTROL), PTW_LAT_CLEAR_BIT);

  // setup PTW PMU0 hit event(0) / PMU1 miss event(2)
  mmio_write_32( (SMMU_PTW_BASE+PTW_PMU_CTRL), 0x200);
  mmio_write_32( (SMMU_PTW_BASE+PTW_PMU_CTRL), 0x201);
  // setup LAT4 PMU0 hit event(0) / PMU1 miss event(2)
  mmio_write_32( (SMMU_LAT4_BASE+LAT_PMU_CONTROL), 0x200);
  mmio_write_32( (SMMU_LAT4_BASE+LAT_PMU_CONTROL), 0x201);

  udelay(1);
}

static void dump_pmu(void)
{
  uartlog("Read PTW PMU0 = 0x%x\n", mmio_read_32(SMMU_PTW_BASE+PTW_PMU0_RESULT));
  uartlog("Read PTW PMU1 = 0x%x\n", mmio_read_32(SMMU_PTW_BASE+PTW_PMU1_RESULT));
  uartlog("Read LAT4 PMU0 = 0x%x\n", mmio_read_32(SMMU_LAT4_BASE+LAT_PMU0_RESULT));
  uartlog("Read LAT4 PMU1 = 0x%x\n", mmio_read_32(SMMU_LAT4_BASE+LAT_PMU1_RESULT));
}

static void dump_utlb_ar_aw(void)
{
  uartlog("Read LAT UTLB AR = 0x%x\n", mmio_read_32(SMMU_LAT4_BASE+LAT_AR_UTLB_ENTRY_VALID));
  uartlog("Read LAT UTLB AW = 0x%x\n", mmio_read_32(SMMU_LAT4_BASE+LAT_AW_UTLB_ENTRY_VALID));
}

int uart_loopback_test(uint8_t data)
{
  uint8_t recv;

  uart_putc(data);
  recv = (uint8_t)uart_getc();
  if (data != recv)
    return -1;

  return 0;
}

unsigned int get_tdma_num_desc(uint8_t *cmdbuf)
{
  unsigned int num_desc = 0;;

  for(uint32_t i = 0 ;; i += TDMA_DESC_BYTE)
  {
    num_desc ++;
    if(cmdbuf[i] & (1 << 2))
    {
      return num_desc;
    }
  }
}

void set_tdma_num_desc(void)
{
  unsigned int num_desc;
  u64 ctrl_reg = g_tdma_ctrl_reg_base;

  num_desc = get_tdma_num_desc((uint8_t *)g_tdma_cmdbuf_start);
  if(num_desc > 0xffff)
    while(1);

  u32 v = mmio_read_32(ctrl_reg);
  v = v & ~(0xffff << 16);
  mmio_write_32(ctrl_reg, v | (num_desc << 16));
}

static void mask_tdma_interrupt(void)
{
  u64 addr = g_tdma_ctrl_reg_base + 8;

  u32 v = mmio_read_32(addr);
  mmio_write_32(addr, v & ~1);
}

static void enable_tdma_dma_mode(void)
{
  u64 addr = g_tdma_ctrl_reg_base;

  u32 v = mmio_read_32(addr);
  mmio_write_32(addr, v | 0b10);
}

static void set_tdma_array_base(u32 index)
{
  u64 addr_base_reg = g_tdma_ctrl_reg_base + 0x70;
//  index : timeline of array_base
  for (u32 i = 0; i < 8; i++) // array base 0~7
    mmio_write_32(addr_base_reg+i*4, (u32) engine_arraybase[index*8+i]);
}

static void trigger_tdma_cmdbuf(void)
{
  u64 ctrl_reg = g_tdma_ctrl_reg_base;
  u64 addr_reg = g_tdma_ctrl_reg_base + 4;

  set_tdma_num_desc();
  u32 cmdbuf_addr = g_tdma_cmdbuf_start >> 6;
  mmio_write_32(addr_reg, cmdbuf_addr);
  u32 v = mmio_read_32(ctrl_reg);
  mmio_write_32(ctrl_reg, v | 1);
}

static void wait_tdma_done(void)
{
  /*
   * Interrupt status reg is at offset 16, but TDMA
   * supports only 32-bit aligned accesses.
   */
  u64 intr_mask_reg = g_tdma_ctrl_reg_base + 8;
  u64 ctrl_reg = g_tdma_ctrl_reg_base;
  while (1) {
    u32 mask_val = mmio_read_32(intr_mask_reg);
    u32 intr = mask_val & (1 << 16);
    if (intr) {
      u32 val = mmio_read_32(ctrl_reg);
      val &= ~1; //clear tdma_enable bit
      mmio_write_32(ctrl_reg, val);
      mmio_write_32(intr_mask_reg, mask_val);
      break;
    }
  }
}

static void reset_tdma_cmd_id(void)
{
  u64 addr = g_tdma_ctrl_reg_base;
  u32 v = mmio_read_32(addr);

  mmio_write_32(addr, v | 0x4);
  mmio_write_32(addr, v & ~0x4);
}

static void start_tdma_copy(int nCommand)
{
  // print tpu cmd buffer
  g_tdma_cmdbuf_start = TDMA_CMDBUF_BASE_ADDR + 0x40*nCommand;
  uartlog("tpu cmd buf\n");
  for(int i=0;i<16;i+=4)
  {
    uartlog("0x%8x  0x%8x  0x%8x  0x%8x\n",ui_tpu_cmd_buf[TDMA_CMDBUF_LEN*nCommand+i], 
                                           ui_tpu_cmd_buf[TDMA_CMDBUF_LEN*nCommand+i+1], 
                                           ui_tpu_cmd_buf[TDMA_CMDBUF_LEN*nCommand+i+2], 
                                           ui_tpu_cmd_buf[TDMA_CMDBUF_LEN*nCommand+i+3]);
  }
  mask_tdma_interrupt();
  enable_tdma_dma_mode();
  reset_tdma_cmd_id();
  set_tdma_array_base(0);
  trigger_tdma_cmdbuf();
  wait_tdma_done();
  //mmio_write_32(TERM_SIM_ADDR,0xABABABAB);
}

static void dump_dst_buf(void)
{
  uartlog("tpu sram data\n");
  for(int i=0;i<16;i+=4)
  {
    uartlog("0x%8x  0x%8x  0x%8x  0x%8x\n",uc_tpu_sram[i], 
                                           uc_tpu_sram[i+1], 
                                           uc_tpu_sram[i+2], 
                                           uc_tpu_sram[i+3]);
  }
}


int ft_simple_translate(void)
{
  uartlog("put PTW TTBR@0x%lx to @0x%lx\n", ((unsigned long)st_SmmuPageTable & 0x00000000FFFFFFFF),  (unsigned long)SMMU_PTW_BASE+PTW_TTBR);
  mmio_write_32( (SMMU_PTW_BASE+PTW_TTBR), ((unsigned long)st_SmmuPageTable & 0x00000000FFFFFFFF));
  
  uartlog("Enable LAT4@0x%lx \n", (unsigned long)SMMU_LAT4_BASE+LAT_CONTROL);
  mmio_write_32( (SMMU_LAT4_BASE+LAT_CONTROL), LAT_EN_BIT );

  start_tdma_copy(0);

  dump_dst_buf();

  start_tdma_copy(1);

  dump_dst_buf();

  return 0;
}

int ft_lat_invalidate(void)
{
  uartlog("put PTW TTBR@0x%lx to @0x%lx\n", (unsigned long)st_SmmuPageTable, (unsigned long)SMMU_PTW_BASE+PTW_TTBR);
  mmio_write_32( (SMMU_PTW_BASE+PTW_TTBR), (unsigned long)st_SmmuPageTable );
  
  uartlog("Enable LAT4@0x%lx \n", (unsigned long)SMMU_LAT4_BASE+LAT_CONTROL);
  mmio_write_32( (SMMU_LAT4_BASE+LAT_CONTROL), LAT_EN_BIT );

  
  dump_utlb_ar_aw();
  
  start_tdma_copy(0);

  dump_utlb_ar_aw();
  
  mmio_write_32( (SMMU_LAT4_BASE+LAT_CONTROL), (mmio_read_32(SMMU_LAT4_BASE+LAT_CONTROL)|LAT_EN_BIT|LAT_UTLB_INV_BIT) );
  uartlog("Invalidate LAT4 ALL\n");

  dump_utlb_ar_aw();

  return 0;
}

int ft_lat_partial_invalidate(void)
{
  uartlog("put PTW TTBR@0x%lx to @0x%lx\n", (unsigned long)st_SmmuPageTable, (unsigned long)SMMU_PTW_BASE+PTW_TTBR);
  mmio_write_32( (SMMU_PTW_BASE+PTW_TTBR), (unsigned long)st_SmmuPageTable );
  
  uartlog("Enable LAT4@0x%lx \n", (unsigned long)SMMU_LAT4_BASE+LAT_CONTROL);
  mmio_write_32( (SMMU_LAT4_BASE+LAT_CONTROL), LAT_EN_BIT );
  
  dump_utlb_ar_aw();

  start_tdma_copy(0);

  dump_utlb_ar_aw();

  mmio_write_32( (SMMU_LAT4_BASE+LAT_PI_START_VPN), 0x00000000);
  mmio_write_32( (SMMU_LAT4_BASE+LAT_PI_END_VPN), 0x00000001);
  mmio_write_32( (SMMU_LAT4_BASE+LAT_CONTROL), (mmio_read_32(SMMU_LAT4_BASE+LAT_CONTROL)|LAT_EN_BIT|LAT_PI_START_BIT) );
  uartlog("Invalidate LAT4 PARTIAL\n");

  dump_utlb_ar_aw();

  return 0;
}

int ft_ptw_partial_invalidate(void)
{
  uartlog("put PTW TTBR@0x%lx to @0x%lx\n", (unsigned long)st_SmmuPageTable, (unsigned long)SMMU_PTW_BASE+PTW_TTBR);
  mmio_write_32( (SMMU_PTW_BASE+PTW_TTBR), (unsigned long)st_SmmuPageTable );
  
  uartlog("Enable LAT4@0x%lx \n", (unsigned long)SMMU_LAT4_BASE+LAT_CONTROL);
  mmio_write_32( (SMMU_LAT4_BASE+LAT_CONTROL), LAT_EN_BIT );

  enable_pmu();
 
  start_tdma_copy(0);

  dump_pmu();
  
  // LAT invalidate all
  mmio_write_32( (SMMU_LAT4_BASE+LAT_CONTROL), LAT_EN_BIT|LAT_UTLB_INV_BIT );
  mmio_write_32( (SMMU_LAT4_BASE+LAT_CONTROL), LAT_EN_BIT );

  // PTW Partial Invalidate
  //Partial Invalidate Start VPN 180 1 0001_0000
  mmio_write_32( (SMMU_PTW_BASE+PTW_PI_START_VPN), 0x00000000);
  //Partial Invalidate End VPN 184 1 0001_0fff
  mmio_write_32( (SMMU_PTW_BASE+PTW_PI_END_VPN), 0x00000001);
  //Partial Invalidate Start 104 1 10 1
  mmio_write_32( (SMMU_PTW_BASE+PTW_CONTROL), PTW_PI_START_BIT );

  // PTW CONTROL should be 30(pi_start and pi_done) 104 0 30 1
  mdelay(1);

  uartlog("Check pi_done 0x%x (should be 30)\n", mmio_read_32(SMMU_PTW_BASE+PTW_CONTROL));
  //Partial Invalidate Clear 104 1 40
  mmio_write_32( (SMMU_PTW_BASE+PTW_CONTROL), PTW_PI_CLEAR_BIT );

  start_tdma_copy(0);

  dump_pmu();

  return 0;
}

int ft_bypass_mode_invalidate(void)
{
  uartlog("put PTW TTBR@0x%lx to @0x%lx\n", (unsigned long)st_SmmuPageTable, (unsigned long)SMMU_PTW_BASE+PTW_TTBR);
  mmio_write_32( (SMMU_PTW_BASE+PTW_TTBR), (unsigned long)st_SmmuPageTable );
  
  uartlog("Enable bypass mode LAT4@0x%lx \n", (unsigned long)SMMU_LAT4_BASE+LAT_CONTROL);
  mmio_write_32( (SMMU_LAT4_BASE+LAT_CONTROL), LAT_EN_BIT|LAT_BPSS_EN_BIT );

  mmio_write_32( (SMMU_LAT4_BASE+LAT_BYPASS_ID_0), 0xFFFFFFFF);
  mmio_write_32( (SMMU_LAT4_BASE+LAT_BYPASS_ID_0_MASK), 0xFFFFFFFF);

  start_tdma_copy(1);
  dump_dst_buf();

  // disable bypass
  mmio_write_32( (SMMU_LAT4_BASE+LAT_CONTROL), LAT_EN_BIT );
  start_tdma_copy(1);
  dump_dst_buf();
  
  mmio_write_32( (SMMU_LAT4_BASE+LAT_CONTROL), LAT_EN_BIT|LAT_BPSS_EN_BIT );

  return 0;
}

int testcase_smmu(void)
{
  int ret;

  uartlog("smmu page table address @0x%lx\n", (unsigned long)st_SmmuPageTable);
  uartlog("test pattern address @0x%lx\n\n", (unsigned long)uc_array_pattern);

  uartlog("Start smmu test - simple_translate\n");
  ft_simple_translate();
  uartlog("End simple_translate\n\n");

  uartlog("Start smmu test - lat_invalidate\n");
  ft_lat_invalidate();
  uartlog("End lat_invalidate\n\n");

  uartlog("Start smmu test - lat_partial_invalidate\n");
  ft_lat_partial_invalidate();
  uartlog("End lat_partial_invalidate\n\n");

  uartlog("Start smmu test - ft_ptw_partial_invalidate\n");
  ft_ptw_partial_invalidate();
  uartlog("End ft_ptw_partial_invalidate\n\n");

  uartlog("Start smmu test - ft_bypass_mode_invalidate\n");
  ft_bypass_mode_invalidate();
  uartlog("End ft_bypass_mode_invalidate\n\n");

  do {
    ret = cli_simple_loop();
  } while (ret == 0);
  return uart_loopback_test('C');
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  int ret = testcase_smmu();
  uartlog("testcase uart %s\n", ret?"failed":"passed");
  return ret;
}
#endif
