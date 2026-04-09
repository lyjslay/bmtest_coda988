#include <stdint.h>

#include "system_common.h"
#include "uartchain.h"

#define thr rbr
#define iir fcr
#define dll rbr
#define dlm ier

struct dw_regs {
	volatile uint32_t	rbr;		/* 0x00 Data register */
	volatile uint32_t	ier;		/* 0x04 Interrupt Enable Register */
	volatile uint32_t	fcr;		/* 0x08 FIFO Control Register */
	volatile uint32_t	lcr;		/* 0x0C Line control register */
	volatile uint32_t	mcr;		/* 0x10 Line control register */
	volatile uint32_t	lsr;		/* 0x14 Line Status Register */
	volatile uint32_t	msr;		/* 0x18 Modem Status Register */
	volatile uint32_t	spr;		/* 0x20 Scratch Register */
};

struct uartchain_regs {
	volatile uint32_t	rx00;		/* 0x00 Data register */
	volatile uint32_t	rx04;		/* 0x04 Interrupt Enable Register */
	volatile uint32_t	rx08;		/* 0x08 FIFO Control Register */
	volatile uint32_t	rx0c;		/* 0x0C Line control register */
	volatile uint32_t	rx10;		/* 0x10 Line control register */
};

#define UART_LCR_WLS_MSK 0x03       /* character length select mask */
#define UART_LCR_WLS_5  0x00        /* 5 bit character length */
#define UART_LCR_WLS_6  0x01        /* 6 bit character length */
#define UART_LCR_WLS_7  0x02        /* 7 bit character length */
#define UART_LCR_WLS_8  0x03        /* 8 bit character length */
#define UART_LCR_STB    0x04        /* # stop Bits, off=1, on=1.5 or 2) */
#define UART_LCR_PEN    0x08        /* Parity eneble */
#define UART_LCR_EPS    0x10        /* Even Parity Select */
#define UART_LCR_STKP   0x20        /* Stick Parity */
#define UART_LCR_SBRK   0x40        /* Set Break */
#define UART_LCR_BKSE   0x80        /* Bank select enable */
#define UART_LCR_DLAB   0x80        /* Divisor latch access bit */

#define UART_MCR_DTR    0x01        /* DTR   */
#define UART_MCR_RTS    0x02        /* RTS   */
#define UART_MCR_AFCE   (1 << 5)    /* RTS   */

#define UART_LSR_THRE		0x20 /* Transmit-hold-register empty */
#define UART_LSR_DR		0x01 /* Receiver data ready */
#define UART_LSR_TEMT   0x40        /* Xmitter empty */

#define UART_FCR_FIFO_EN    0x01 /* Fifo enable */
#define UART_FCR_RXSR       0x02 /* Receiver soft reset */
#define UART_FCR_TXSR       0x04 /* Transmitter soft reset */

#define UART_MCRVAL (UART_MCR_DTR | UART_MCR_RTS | UART_MCR_AFCE)      /* RTS/DTR */
#define UART_FCR_DEFVAL	(UART_FCR_FIFO_EN | UART_FCR_RXSR | UART_FCR_TXSR)
#define UART_LCR_8N1    0x03


#define UARTCHAIN_TEIE		(1 << 12)
#define UARTCHAIN_HSD		(1 << 7)
#define UARTCHAIN_EN		(1)
#define UARTCHAIN_SWBUSY	(1 << 6)

#define UARTCHAIN_DEFVAL	(UARTCHAIN_HSD | UARTCHAIN_EN | UARTCHAIN_TEIE)


static struct dw_regs *uart = (struct dw_regs *)0x58018000;
static struct uartchain_regs *uartchain = (struct uartchain_regs *)0x58019000;

static volatile int g_wait = 0;

#if 0
static int uartchain_txend(int irqn, void *priv)
{

	printf("txend:0x%x\n", uartchain->rx10);
	uartchain->rx10 = 0;
	g_wait = 0;
	return 0;
}
#endif

void uartchain_init(void)
{
	#if defined(BOARD_FPGA)
	int baudrate = 115200;
	int uartchain_clock = 25 * 1000 * 1000;
	#elif defined(BOARD_PALLADIUM)
	int baudrate = 9600;
	int uartchain_clock = 9600 * 16;
	#else
	int baudrate = 115200;
	int uartchain_clock = 500 * 1000 * 1000;
	#endif
	int divisor = uartchain_clock / (16 * baudrate);

	uart->lcr = uart->lcr | UART_LCR_DLAB | UART_LCR_8N1;
	uart->dll = divisor & 0xff;
	uart->dlm = (divisor >> 8) & 0xff;
	uart->lcr = uart->lcr & (~UART_LCR_DLAB);

	uart->ier = 0;
	uart->mcr = UART_MCRVAL;
	uart->fcr = UART_FCR_DEFVAL;
	
	uart->lcr = 3;


	uartchain->rx00 |= UARTCHAIN_DEFVAL;


	//request_irq(UARTCHAIN_TXEND_INTR, uartchain_txend, IRQF_TRIGGER_NONE, "uartchain txend", NULL);
	//enable_irq(UARTCHAIN_TXEND_INTR);
}

void uartchain_init_top(void)
{
	#if defined(BOARD_FPGA)
	int baudrate = 115200;
	int uartchain_clock = 25 * 1000 * 1000;
	#elif defined(BOARD_PALLADIUM)
	int baudrate = 9600;
	int uartchain_clock = 9600 * 16;
	#else
	int baudrate = 115200;
	int uartchain_clock = 500 * 1000 * 1000;
	#endif
	int divisor = uartchain_clock / (16 * baudrate);

	uart->lcr = uart->lcr | UART_LCR_DLAB | UART_LCR_8N1;
	uart->dll = divisor & 0xff;
	uart->dlm = (divisor >> 8) & 0xff;
	uart->lcr = uart->lcr & (~UART_LCR_DLAB);

	uart->ier = 0;
	uart->mcr = UART_MCRVAL;
	uart->fcr = UART_FCR_DEFVAL;
	
	uart->lcr = 3;

	uartchain->rx00 &= ~UARTCHAIN_DEFVAL;
}

void _uartchain_putc(uint8_t ch)
{
	while (!(uart->lsr & UART_LSR_THRE))
		;
	uart->rbr= ch;
}

void uartchain_putc(uint8_t ch)
{
	if (ch == '\n') {
		_uartchain_putc('\r');
	}
	_uartchain_putc(ch);
}

void uartchain_puts(char *str)
{
	if (!str)
		return;

	while (*str) {
		uartchain_putc(*str++);
	}
}

int uartchain_getc(void)
{
	while (!(uart->lsr & UART_LSR_DR))
		;
	return (int)uart->rbr;
}

int uartchain_tstc(void)
{
	return (!!(uart->lsr & UART_LSR_DR));
}

void uartchain_start_send(void)
{
	g_wait = 1;
	uartchain->rx00 |= UARTCHAIN_SWBUSY;
}

void uartchain_end_send(void)
{
	while (!(uart->lsr & UART_LSR_TEMT));
	uartchain->rx00 &= ~UARTCHAIN_SWBUSY;
	//while(g_wait);
#if 0
	while(!(uartchain->rx10 & 0x1))
	{
		printf("wi\n");
	}
	uartchain->rx10 = 0;
	printf("ci\n\n\n");
#endif
}

