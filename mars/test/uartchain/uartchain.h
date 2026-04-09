#ifndef _UARTCHAIN_H_
#define _UARTCHAIN_H_

void uartchain_init(void);
void uartchain_init_top(void);
int uartchain_getc(void);
int uartchain_tstc(void);
void uartchain_putc(uint8_t ch);
void uartchain_puts(char *str);
void uartchain_start_send(void);
void uartchain_end_send(void);

#endif
