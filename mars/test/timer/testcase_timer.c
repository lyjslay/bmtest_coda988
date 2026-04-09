#include <stdio.h>
#include "boot_test.h"
#include "system_common.h"
#include "timer.h"

int testcase_timer(void)
{
  uartlog("%s\n", __func__);

  u32 ms;
  int i = 0;
  uartlog("current tick %lx\n", timer_get_tick());
  u8 *mem = malloc(1920*1080*1);
  memset(mem, 0, 1920*1080*1);
  *mem = 0x12;
  *(mem+1) = 0x34;
  *(mem+2) = 0x56;
  *(mem+3) = 0x78;
  for (i=0;i<4;i++)
  {
    uartlog("mem[%d] addr=%p, val= %x\n", i, mem+i, mem[i]);
  }

  for (i=0; i<60; i++)
  {
      /* delay & meter */
    u32 test_us = 1000000;
    timer_meter_start();
    timer_udelay(test_us);
    ms = timer_meter_get_ms();
    uartlog("[%d] delay %d us, meter %d ms\n", i, test_us, ms);
  }

  uartlog("%s passed\n", __func__);
  return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  int ret = testcase_timer();
  return ret;
}
#endif