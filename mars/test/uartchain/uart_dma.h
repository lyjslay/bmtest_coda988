#ifndef _UART_DMA_H_
#define _UART_DMA_H_

void uartdma_init(void);
int uartdma_getc(void);
void uartdma_putc(uint8_t ch);

#endif
