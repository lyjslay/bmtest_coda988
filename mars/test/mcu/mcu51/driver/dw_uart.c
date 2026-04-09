#include "config.h"
#include "cvi_mcu51.h"
#include "chip_cv1822.h"
#include "driver/dw_uart.h"

#ifdef CONFIG_DW_UART_RTC
#define uart_base RTC_UART_BASE
#endif

#ifdef CONFIG_DW_UART_TOP
#define uart_base UART0_BASE
#endif

#ifdef CONFIG_PUTCHAR_DW_UART
char putchar(char c)
{
	if (c == '\n')
		dw_uart_putc('\r');
	return dw_uart_putc(c);
}
#endif /* CONFIG_PUTCHAR_DW_UART */

#ifdef uart_base

char dw_uart_init(void)
{
	xdata unsigned int divisor;

#ifdef CONFIG_FPGA
	// write_robot(0x030010A4, 0x2);
	// write_robot(0x03001090, 0x2);

	// FPGA clock
	divisor = MCU51_CLK_HZ / (16 * 115200);
#else
	write_robot(0x030010A4, 0x2);
	write_robot(0x03001090, 0x2);

	divisor = MCU51_CLK_HZ / (16 * 115200);
#endif

	write_robot(uart_base + UARTMCR, 0x10); /* LOOP */

	write_robot(uart_base + UARTLCR, UARTLCR_DLAB);
	write_robot(uart_base + UARTDLL, divisor & 0xff);
	write_robot(uart_base + UARTDLLM, (divisor >> 8) & 0xff);
	write_robot(uart_base + UARTLCR, UARTLCR_WORDSZ_8);

	write_robot(uart_base + UARTIER, 0x0); /* no interrupt */
	write_robot(uart_base + UARTFCR, UARTFCR_FIFOEN | UARTFCR_TXCLR | UARTFCR_RXCLR);
	write_robot(uart_base + UARTMCR, 0x3); /* DTR + RTS */

	return 0;
}

char dw_uart_putc(char c)
{
	while ((read_robot(uart_base + UARTLSR) & (UARTLSR_TEMT | UARTLSR_THRE)) !=
	       (UARTLSR_TEMT | UARTLSR_THRE))
		;

	write_robot(uart_base + UARTTX, c);
	return c;
}
#endif

#if 0
char dw_uart_getc(void)
{
	while ((read_robot(uart_base + UARTLSR) & UARTLSR_RDR) == 0)
		;

	return read_robot(uart_base + UARTRX);
}

char dw_uart_putc_irq(char c)
{
	while ((read_robot_irq(uart_base + UARTLSR) & (UARTLSR_TEMT | UARTLSR_THRE)) !=
	       (UARTLSR_TEMT | UARTLSR_THRE))
		;

	write_robot_irq(uart_base + UARTTX, c);

	return c;
}

#endif