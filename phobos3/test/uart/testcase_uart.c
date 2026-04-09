#include "boot_test.h"
#include "uart.h"
#include "cli.h"
#include "command.h"

int uart_loopback_test(uint8_t data)
{
  uint8_t recv;

  uart_putc(data);
  recv = (uint8_t)uart_getc();
  if (data != recv)
    return -1;

  return 0;
}

#if defined(UART_INTR_TEST)
static int uart_irq_handler(int irqn, void *priv)
{
  disable_irq(irqn);
  printf("enter uart intr:%d\n", irqn);
  return 0;
}
#endif

int testcase_uart(void)
{
  int ret;

  uartlog("%s\n", __func__);
#if defined(UART_INTR_TEST)
  /* request uart intr, user-defined */
  printf("request uart irq number:%d\n", UART_INT);
  request_irq(UART_INT, uart_irq_handler, 0, "uart interrupt", NULL);

  /* pinmux uart setting, user-defined */
  // pwr_uart pimux setting
  writel(0x03001078, 0x2);
  writel(0x0300106c, 0x2);
#endif

  do {
    ret = cli_simple_loop();
  } while (ret == 0);
  return uart_loopback_test('C');
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  int ret = testcase_uart();
  uartlog("testcase uart %s\n", ret?"failed":"passed");
  return ret;
}
#endif
