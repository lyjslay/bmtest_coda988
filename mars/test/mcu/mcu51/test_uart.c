#include <stdio.h>
#include "test.h"

#if TEST_RTC_DW_UART
int test_rtc_dw_uart()
{
	char c;

	dw_uart_init();

	printf("input: ");
	while (1) {
		c = dw_uart_getc();
		printf("c=%d\n", (int)c);
		if (c == 'q')
			break;
	}
	printf("test end\n");

	return 0;
}
#endif /* TEST_RTC_DW_UART */

#if TEST_MCU51_UART

char mcu51_putchar(char c)
{
	if (c == '\n') {
		while (!TI)
			;
		TI = 0;
		SBUF = 0x0d; /* output CR  */
	}
	while (!TI)
		;
	TI = 0;
	return (SBUF = c);
}

int test_mcu51_uart()
{
	xdata unsigned char bl;
	printf("mcu51 internel uart test U2\n");
#if 0
	// pinmux
	write_robot(0x030010C0, 0x2);
	write_robot(0x030010C4, 0x2);
	write_robot(0x030010C8, 0x2);
	write_robot(0x030010CC, 0x2);

	write_robot(0x030010A4, 0x2);
	write_robot(0x03001090, 0x2);
#endif

	write_robot(TOP_BASE + 0x1000 + 0xC0, 2); // UART2_TX PWR_UART1_TX 2
	write_robot(TOP_BASE + 0x1000 + 0xC8, 2); // UART2_RX PWR_UART1_RX 2

	printf("A1\n");

	// Timer 2. Baud Rate = 39,062.50

	bl = 0xFF;
	RCAP2H = 0xFF;
	RCAP2L = bl;
	// RCAP2L = 0x0B; // tested value
	T2CON = 0x34;
	SCON = 0x50; /* SCON: mode 1, 8-bit UART, enable rcvr      */
	TI = 1;

	printf("U1\n");
	printf("bl=0x%02x\n", (unsigned int)bl);
	while (1) {
		char *p;
		printf("bl=0x%02x\n", bl);
		xs2 = 0xB1;
		mcu51_putchar('A');
		xs2 = 0xB3;
		mcu51_putchar('B');
		mcu51_putchar('C');
		for(p = "abcdefghijklmnopqr"; *p; p++) {
			mcu51_putchar(*p);
		}
		mcu51_putchar('\n');
		// printf("XXX0x%02xZZZ\n", (unsigned int)bl);
		bl--;
		RCAP2L = bl;
	}

	printf("mcu51 internel uart test ok\n");

	return 0;
}
#endif /* TEST_MCU51_UART */