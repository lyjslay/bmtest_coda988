#include "boot_test.h"
#include "uart.h"
#include "testcase_wdt.h"
#include "cli.h"
#include "mmio.h"
#include "system_common.h"

int uart_loopback_test(uint8_t data)
{
  uint8_t recv;

  uart_putc(data);
  recv = (uint8_t)uart_getc();
  if (data != recv)
    return -1;

  return 0;
}

static int irq_handler(int irqn, void *priv)
{

  uartlog("INT: %d\n", irqn);
  return 0;
}

int testcase_wdt(void)
{
  int ret = 0;
  request_irq(WDT_INTR, irq_handler, 0, "wdt interrupt", NULL);
  mmio_write_32(TOP_BASE + 8, 0x4);
  mmio_write_32(TOP_BASE + WDT_BASE + 0x4, 0x20);
  mmio_write_32(TOP_BASE + WDT_BASE + 0xC, 0x76);
  mmio_write_32(TOP_BASE + WDT_BASE, 0x13);

  return ret;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  int ret = testcase_wdt();
  uartlog("testcase wdt %s\n", ret?"failed":"passed");
  return ret;
}
#endif
