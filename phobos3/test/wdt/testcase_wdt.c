#include "boot_test.h"
#include "uart.h"
#include "testcase_wdt.h"
#include "cli.h"
#include "mmio.h"
#include "system_common.h"

//#define WDT_BASE      0x11000

int uart_loopback_test(uint8_t data)
{
  uint8_t recv;

  uart_putc(data);
  recv = (uint8_t)uart_getc();
  if (data != recv)
    return -1;

  return 0;
}

static inline uint64_t get_cycles64(void)
{
  uint64_t cyc;
  asm volatile("rdcycle %0" : "=r" (cyc));
  return cyc;
}

#define WDT_TEST_CASE4_2

#if defined(WDT_TEST_CASE3) || defined(WDT_TEST_CASE4_2)
/* test case3, not reset */
static int irq_handler(int irqn, void *priv)
{
  u32 eoi;

  u32 tick_ms = timer_meter_get_ms();
	
	uartlog("timer delay 10s end, meter value=%d\n", tick_ms);

  /* read eoi to clear irq */
  eoi = mmio_read_32(TOP_BASE + WDT1_BASE + 0x14);
  //mmio_write_32(TOP_BASE + WDT_BASE, 0);
  mmio_write_32(TOP_BASE + WDT1_BASE + 0xC, 0x76);
  uartlog("TOP_BASE + WDT_BASE + 0x14=0x%x\n", TOP_BASE + WDT1_BASE + 0x14);
  uartlog("irq has been clear, eoi=%d,INT: %d\n", eoi, irqn);
  
  return 0;
}
#else
static int irq_handler(int irqn, void *priv)
{

  uartlog("INT: %d\n", irqn);
  return 0;
}
#endif

#if defined(WDT_TEST_CASE1)

/* test case 1, delay 3s*/
int testcase_wdt(void)
{
  int ret = 0;
  request_irq(WDT1_INTR, irq_handler, 0, "wdt interrupt", NULL);
  mmio_write_32(TOP_BASE + 8, 0x4);
  mmio_write_32(TOP_BASE + WDT_BASE + 0x4, 10 << 4);
  mmio_write_32(TOP_BASE + WDT_BASE + 0xC, 0x76);
  mmio_write_32(TOP_BASE + WDT_BASE, 0x13);

  return ret;
}

#elif defined(WDT_TEST_CASE3)
/* test case 3, delay 3s, timeout not reset */
int testcase_wdt(void)
{
  int ret = 0;
  request_irq(WDT1_INTR, irq_handler, 0, "wdt interrupt", NULL);
  //mmio_write_32(TOP_BASE + 8, 0x4);
  mmio_write_32(TOP_BASE + WDT_BASE + 0x4, 10 << 4);
  mmio_write_32(TOP_BASE + WDT_BASE + 0xC, 0x76);
  mmio_write_32(TOP_BASE + WDT_BASE, 0x13);

  long start_cyc = get_cycles64();;
  while(1){
  	//uartlog("TOP_BASE + WDT_BASE + 0x14=0x%x\n", mmio_read_32(TOP_BASE + WDT_BASE + 0x14));
  	if(start_cyc % 25000000 == 0){
		uartlog("cpu is still active\n");
	}
	start_cyc = get_cycles64();
  }

  return ret;
}

#elif defined(WDT_TEST_CASE4_1)

/* test case 4.1, delay 3s*/
int testcase_wdt(void)
{
  int ret = 0;
  request_irq(WDT1_INTR, irq_handler, 0, "wdt interrupt", NULL);
  mmio_write_32(TOP_BASE + 8, 0x4);
  mmio_write_32(TOP_BASE + WDT_BASE + 0x4, 5 << 4 | 5);
  mmio_write_32(TOP_BASE + WDT_BASE + 0xC, 0x76);
  mmio_write_32(TOP_BASE + WDT_BASE + 0x1C, 75);
  mmio_write_32(TOP_BASE + WDT_BASE, 0x13 | 1 << 7 | 1 << 6);

  return ret;
}

#elif defined(WDT_TEST_CASE4_2)

/* test case 4.2, delay 3s no reset*/
int testcase_wdt1(void)
{
  int ret = 0;

  request_irq(WDT1_INTR, irq_handler, 0, "wdt interrupt", NULL);

  mmio_write_32(TOP_BASE + 0x8, 0x2);
  mmio_write_32(TOP_BASE + 0x1a8, 0x77);
  mmio_write_32(TOP_BASE + WDT1_BASE + 0x4, 11 << 4 | 11);
  mmio_write_32(TOP_BASE + WDT1_BASE + 0xC, 0x76);
  mmio_write_32(TOP_BASE + WDT1_BASE + 0x1C, 61036);
  mmio_write_32(TOP_BASE + WDT1_BASE, 0x13 | 0x1 << 7 | 0x1 << 6);

  timer_meter_start();
  //uartlog("timer delay 10s end, meter value=%d\n", test_start);

  long start_cyc = get_cycles64();
  while(1){
  	if (start_cyc % 25000000 == 0) {
      uartlog("cpu is still active\n");
	  }
	  start_cyc = get_cycles64();
  }

  return ret;
}
#endif

int testcase_rtc_wdt(void)
{
  int ret = 0;
  
  request_irq(RTC_WDT_INTRUPT, irq_handler, 0, "wdt interrupt", NULL);

  mmio_write_32(TOP_BASE + 0x8, 0x4);
  mmio_write_32(TOP_BASE + 0x1a8, 0x77);
  mmio_write_32(0x050260E8, 0x04);
  mmio_write_32(0x05026000 + 0x0e0, 0x3);
  mmio_write_32(0x05026000 + 0x0c0, 0x1);
  mmio_write_32(0x05026000 + 0x0cc, 0x1);
  mmio_write_32(0x05025000 + 0x0a0, 0x1);

  uint32_t v = mmio_read_32(0x05025018);
  v &= 0xfffbffff;
  mmio_write_32(0x05025018, v);
  v |= 0x00040000;
  mmio_write_32(0x05025018, v);

  mmio_write_32(RTC_WDT_BASE + 0x4, 3 << 4 | 3);
  mmio_write_32(RTC_WDT_BASE + 0xC, 0x76);
  //mmio_write_32(RTC_WDT_BASE + 0x1C, 61036);
  mmio_write_32(RTC_WDT_BASE, 0x13 /*| 0x1 << 7 | 0x1 << 6*/);

  u32 test_start = timer_meter_get_ms();
  uartlog("timer delay 10s end, meter value=%d\n", test_start);

  long start_cyc = get_cycles64();
  while(1){
  	if (start_cyc % 25000000 == 0) {
      uartlog("cpu is still active\n");
	  }
	  start_cyc = get_cycles64();

    uint32_t v = mmio_read_32(RTC_WDT_BASE + 0x8);
    uartlog("CCVR=%d\n", v);
  }

  return ret;
}

int testcase_wdt2(void)
{
  int ret = 0;

  request_irq(WDT2_INTRUPT, irq_handler, 0, "wdt interrupt", NULL);

  mmio_write_32(TOP_BASE + 0x8, 0x2);
  mmio_write_32(TOP_BASE + 0x1a8, 0x77);
  mmio_write_32(TOP_BASE + WDT2_BASE + 0x4, 11 << 4 | 11);
  mmio_write_32(TOP_BASE + WDT2_BASE + 0xC, 0x76);
  mmio_write_32(TOP_BASE + WDT2_BASE + 0x1C, 61036);
  mmio_write_32(TOP_BASE + WDT2_BASE, 0x13 | 0x1 << 7 | 0x1 << 6);

  u32 test_start = timer_meter_get_ms();
  uartlog("timer delay 10s end, meter value=%d\n", test_start);

  long start_cyc = get_cycles64();
  while(1){
  	if (start_cyc % 25000000 == 0) {
      uartlog("cpu is still active\n");
	  }
	  start_cyc = get_cycles64();
  }

  return ret;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  int ret = testcase_wdt1();
  uartlog("testcase wdt %s\n", ret?"failed":"passed");
  return ret;
}
#endif
