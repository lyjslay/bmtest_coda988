#include "boot_test.h"
#include "timer.h"

#define fw_read32(addr)          RAW_READ32((FW_ADDR_T)(addr))
#define fw_write32(addr, val)    RAW_WRITE32((FW_ADDR_T)(addr), (val))

#define DDR_BASE                       0x100000000
#define GDMA_ENGINE_BASE_ADDR          0x58000000
#define GDMA_ENGINE_MAIN_CTRL          (GDMA_ENGINE_BASE_ADDR + 0x0)
#define GDMA_ENGINE_DESC_BASE          (GDMA_ENGINE_BASE_ADDR + 0x4)
#define GDMA_ENGINE_INT_STATUS         (GDMA_ENGINE_BASE_ADDR + 0x8)
#define GDMA_ENGINE_SYNC_STATUS        (GDMA_ENGINE_BASE_ADDR + 0xC)
#define GDMA_DCR_BASE_ADDR             (GDMA_ENGINE_BASE_ADDR + 0x00000010)
#define GDMA_CLEAR_CMD_ID_BIT          2
#define GDMA_INVALID_DES_BIT           1
#define GDMA_CMD_OVRFLW_BIT            2

void setup_GDMA_ctrl(void)
{
  // resync cmd ID
  fw_write32(GDMA_ENGINE_MAIN_CTRL, (1 << GDMA_CLEAR_CMD_ID_BIT) | (1 << 8));
  fw_write32(GDMA_ENGINE_DESC_BASE, 0);
  fw_write32(GDMA_ENGINE_INT_STATUS, (1 << GDMA_INVALID_DES_BIT) | (1 << GDMA_CMD_OVRFLW_BIT));
}

#define SEED 10 //rand seed
void fill_rand(u64 ga, u32 count)
{
  //uartlog("%s, ga:0x%lx\n", __func__, ga);

  srand(SEED);
  for (ulong m = 0; m < count * 4; m+=4) {
    ulong value = rand();
    writel(ga + m, value);
  }
}

void fill_u32_const(u64 ga, u32 val, u32 count)
{
  //uartlog("%s, ga:0x%lx\n", __func__, ga);

  for (u32 m = 0; m < count * 4; m+=4) {
    writel(ga + m, val);
  }
}

int cmp_u32_rand(u64 base, u32 size)
{
  int err = 0;

  //uartlog("%s: ga = %lx\n", __func__, base);

  u32 m;
  srand(SEED);
  for (m = 0; m < size * 4; m+=4) {
    u32 expdata = rand();
    u32 rdata = readl(base + m);
    if (rdata != expdata) {
      uartlog("check error @ %lx, rdata %x expdata %x\n", base + m, rdata, expdata);
      err++;
      return err;
    }
  }

  return err;
}

int cmp_u32_const(u64 base, u32 val, u32 size)
{
  int err = 0;

  //uartlog("%s: ga = %lx\n", __func__, base);

  for (int i = 0; i < size * 4; i+=4) {
    u32 rdata = readl(base + i);
    if (rdata != val) {
      uartlog("check error @ %lx, rdata %x expdata %x\n", base + i, rdata, val);
      err++;
#ifndef DEBUG
      return err;
#endif
    }
  }

  return err;
}


void simple_gdma(u64 src, u64 des, u64 transferSize)
{
  setup_GDMA_ctrl();
  u32 desc[] = {0x000104ad, 0x00000088, 0x00000000, 0x00000000, 0x00400008, 0x00000200, 0x00400008, 0x00000000,
              0x00080008, 0x00080008, 0x00080008, 0x00000000, 0x00000000, 0x00000800, 0x00000000, 0x00000000};
  desc[7] = transferSize;
  desc[11] = des & 0xffffffff;
  desc[12] = src & 0xffffffff;
  desc[13] |= (((((u64)des >> 32) & 0xff) << 16) | ((((u64)src >> 32) & 0xff) << 24));
  for (u32 index = 0; index < 16; index ++)
  {
    fw_write32(GDMA_DCR_BASE_ADDR + index * 4, desc[index]);
  }
  // trigger gdma
  fw_write32(GDMA_ENGINE_MAIN_CTRL, (1)|fw_read32(GDMA_ENGINE_MAIN_CTRL));

  // wait for GDMA done
  while (1) {
    u32 val = fw_read32(GDMA_ENGINE_INT_STATUS);
    int intr = val & (1 << 8);
    if (intr) {
      fw_write32(GDMA_ENGINE_INT_STATUS, val);
      break;
    }
  }
}

int simple_gdma_stress(u32 dataType, u32 dataPattern)
{
  int err = 0;
  u64 src = 0, des = 0;
  u64 start_addr = DDR_BASE + 0x01000000;
  u64 end_addr = DDR_BASE + 0x10000000;
  u64 step_size = 1024*1024;

  switch (dataType) {
    case 0:
      fill_rand(start_addr, step_size/4);
      break;
    case 1:
      fill_u32_const(start_addr, dataPattern, step_size/4);
      break;
    default:
      uartlog("wrong data type!!!\n");
      return 1;
      break;
  }

  //aauartlog("fill done\n");

  for (u32 loop = 0; loop < 10; loop++)
  {
    for (u64 index = 0; (index + start_addr) < end_addr; index += step_size)
    {
      src = start_addr + index;
      des = src + step_size;
      //uartlog("gdma from %lx to %lx start\n", src, des);
      simple_gdma(src, des, step_size);
      //uartlog("gdma done\n");
    }

    switch (dataType) {
      case 0:
        err += cmp_u32_rand(des - step_size, step_size/4);
        break;
      case 1:
        err += cmp_u32_const(des - step_size, dataPattern, step_size/4);
        break;
      default:
        uartlog("wrong data type!!!\n");
        return 1;
        break;
    }
  }

  if (err)
  {
    uartlog("simple_gdma_stress %u %x failed!!!\n", dataType, dataPattern);
    while (1) {}
  }
  else
  {
    uartlog("simple_gdma_stress %u %x pass\n", dataType, dataPattern);
  }
  return err;
}

void testcase_gdma_write(void)
{
  uartlog("%s\n", __func__);
  u64 startAddr = 0x100000000;
  u64 localMemoryAddr = 0x8000000;

  uartlog("************************Local Memory -> DDR write***********************\n");
  fill_u32_const(localMemoryAddr, 0xffffffff, 2*1024*1024/4);
  uartlog("fill_rand done\n");

  for (u32 loop = 0; ; loop++)
  {
    simple_gdma(localMemoryAddr, startAddr + (loop % 200) * 1024 * 1024, 2*1024*1024);
  }
  uartlog("GDMA write test done\n");

  return;
}

void testcase_gdma_read(void)
{
  uartlog("%s\n", __func__);
  u64 startAddr = 0x100000000;
  u64 localMemoryAddr = 0x8000000;

  uartlog("************************DDR > Local Memory read***********************\n");
  fill_u32_const(startAddr, 0xffffffff, 1000*1024*1024/4);
  uartlog("fill_rand done\n");

  for (u32 loop = 0; ; loop++)
  {
    simple_gdma(startAddr + (loop % 200) * 1024 * 1024, localMemoryAddr, 2*1024*1024);
  }
  uartlog("GDMA read test done\n");

  return;
}

void testcase_perf(void)
{
  uartlog("%s\n", __func__);
  u64 startAddr = 0x100000000;
  u64 localMemoryAddr = 0x8000000;

  //uartlog("************************DDR<->DDR RW mix***********************\n");
  fill_rand(startAddr, 200*1024*1024/4);
  //uartlog("fill_rand done\n");

  u64 starttick = timer_get_tick();

  for (u32 loop = 0; loop < 100; loop++)
  {
    simple_gdma(startAddr, startAddr + 1024 * 1024 + 16 * 1024, 200*1024*1024);//source/des on different row
  }

  u64 endtick = timer_get_tick();
  uartlog("DDR<->DDR RW mix. tick: %lu. total data 200M*100*2. BW = %lu kBps\n", endtick - starttick, ((u64)200 * 1024 * 2 * 100 * 1000000000) / (((endtick - starttick))*(u64)20));

  starttick = timer_get_tick();
  simple_gdma(startAddr, startAddr + 1024 * 1024 + 16 * 1024, 200*1024*1024);//source/des on different row
  endtick = timer_get_tick();
  uartlog("DDR<->DDR RW mix. tick: %lu. total data 200M*2\n", endtick - starttick);

  //uartlog("************************Local Memory -> DDR write***********************\n");
  fill_rand(localMemoryAddr, 2*1024*1024/4);
  //uartlog("fill_rand done\n");

  starttick = timer_get_tick();

  for (u32 loop = 0; loop < 20000; loop++)
  {
    simple_gdma(localMemoryAddr, startAddr + (loop % 200) * 1024 * 1024, 2*1024*1024);
  }

  endtick = timer_get_tick();
  uartlog("Local Memory -> DDR write. tick: %lu. total data 2M*20000. BW = %lu kBps\n", endtick - starttick, ((u64)2 * 1024 * 20000 * 1000000000) / (((endtick - starttick))*(u64)20));

  //uartlog("************************DDR > Local Memory read***********************\n");
  starttick = timer_get_tick();

  for (u32 loop = 0; loop < 20000; loop++)
  {
    simple_gdma(startAddr + (loop % 200) * 1024 * 1024, localMemoryAddr, 2*1024*1024);
  }

  endtick = timer_get_tick();
  uartlog("DDR > Local Memory read. tick: %lu. total data 2M*20000. BW = %lu kBps\n", endtick - starttick, ((u64)2 * 1024 * 20000 * 1000000000) / (((endtick - starttick))*(u64)20));

  return;
}

int testcase_simple_gdma_stress(void)
{
  uartlog("%s\n", __func__);

  int err = 0;
  testcase_perf();

  for (u32 loop = 0; loop < 100000; loop++)
  {
    err += simple_gdma_stress(0, 0);
    err += simple_gdma_stress(1, 0x12345678);
    err += simple_gdma_stress(1, 0x87654321);
    err += simple_gdma_stress(1, 0x55555555);
    err += simple_gdma_stress(1, 0xaaaaaaaa);
    err += simple_gdma_stress(1, 0x55aa55aa);
    err += simple_gdma_stress(1, 0xaa55aa55);
    err += simple_gdma_stress(1, 0x5a5a5a5a);
    err += simple_gdma_stress(1, 0xa5a5a5a5);
    err += simple_gdma_stress(1, 0xffffffff);
    err += simple_gdma_stress(1, 0x00000000);
    err += simple_gdma_stress(1, 0x0000ffff);
    err += simple_gdma_stress(1, 0xffff0000);
    err += simple_gdma_stress(1, 0x0f0f0f0f);
    err += simple_gdma_stress(1, 0xf0f0f0f0);
  }

  return err;
}
//module_testcase("1", testcase_gdma_stress);

#ifndef BUILD_TEST_CASE_all
int testcase_main(void)
{
  return testcase_simple_gdma_stress();
}
#endif

