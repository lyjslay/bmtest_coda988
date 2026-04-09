#include <stdio.h>
#include "boot_test.h"
#include "system_common.h"
#include "timer.h"
#include "mmio.h"
#include "axi_mon.h"

int testcase_timer(void)
{
  u32 us;

  timer_meter_start();
  axi_monitor_start();

  for(uint32_t i=0; i<20; i++)
    writel(0x110000000 + i*4, 0x11223344);

  timer_udelay(2000);
  us = timer_meter_get_us();

  axi_monitor_snapshot();
  
  printf("meter %d us\n", us);
  axi_monitor_read_count(); 
  axi_monitor_calculate_bandwidth();

  timer_meter_start();
  axi_monitor_start();

  for(uint32_t i=0; i<200; i++)
    writel(0x110000000 + i*4, 0x11223344);
  us = timer_meter_get_us();

  axi_monitor_snapshot();

  printf("meter %d us\n", us);
  axi_monitor_read_count(); 
  axi_monitor_calculate_bandwidth();

  return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  int ret = testcase_timer();
  return ret;
}
#endif