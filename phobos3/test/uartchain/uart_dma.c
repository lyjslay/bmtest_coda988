#include <stdint.h>

#include "system_common.h"
#include "uart_dma.h"

void uartdma_init(void)
{
	writel(PINMUX_BASE + 0xB0, 0x1<<20); /* UART0_TX */
	writel(PINMUX_BASE + 0xB4, 0x1<<4); /* UART0_RX */

	writel(0x5800C000, 0x30000000);
	writel(0x5800C008, 0xffff);
	writel(0x5800C03C, 0x1a); //TODO
}

void _uartdma_putc(uint8_t ch)
{
	while(!(readl(0x5800C180) >> 24));
	writel(0x5800C194, ch << 24);
	writel(0x5800C190, 1 << 31 | 1);
}

void uartdma_putc(uint8_t ch)
{
	if (ch == '\n') {
		_uartdma_putc('\r');
	}
	_uartdma_putc(ch);
}

int uartdma_getc(void)
{
	while(!(readl(0x5800C1F0) >> 16));
	writel(0x5800C210, 1 << 31);
	return (int)(readl(0x5800C214) >> 24);
}

