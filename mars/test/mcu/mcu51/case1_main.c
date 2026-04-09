#include <stdio.h>
#include "test.h"

#define WDT_ENABLE 0

#define WDT_BASE (TOP_BASE + 0x00010000)

unsigned char xdata xs1 = 0xE1;
unsigned char xdata xs2 = 0xE2;
unsigned char xdata xs3 = 0xE3;
unsigned char xdata xs4 = 0xE4;

xdata int int1_count;
xdata uint32_t int1_status;

#if 0
void udelay_loop(unsigned long usec)
{
	usec /= 1000;
	usec = (MCU51_CLK_HZ / 200 / 1000) * usec;
	while (usec--)
		;
}

void mdelay_loop(unsigned long msec)
{
	msec = (MCU51_CLK_HZ / 200 / 1000) * msec;
	while (msec--)
		;
}

void isr_int1(void) interrupt 2 using 1
{
	int1_status = read_robot_irq1(REG_51_INT1_SRC_MASK);
	int1_count++;
}
#endif

typedef uint32_t uintptr_t;
void mmio_clrbits_32(uintptr_t addr, uint32_t clear);
void mmio_setbits_32(uintptr_t addr, uint32_t set);
void mmio_clrsetbits_32(uintptr_t addr, uint32_t clear, uint32_t set);

void mcu51_uart_init(char hi, char lo)
{
	// Timer 2.
	// RCAP2H = 0xFF; RCAP2L = 0x0B; // tested value for Baud Rate = 39,062.50
	RCAP2H = hi;
	RCAP2L = lo;
	T2CON = 0x34;
	SCON = 0x50; /* SCON: mode 1, 8-bit UART, enable rcvr      */
	TI = 1;
}

#if 0
void large_code(void)
{
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 1000\n");
	printf("ABCDEFGH 9999\n");

}
#endif

#if 0
#define PRELOAD_CNT		0xFFFFFFFFUL
void speed_test(void)
{
	int nsec = 1;
	volatile long i;
	uint32_t x;
	const long n = 100000;

	dw_timer_start(TIMER_ID1, PRELOAD_CNT);
	printf("REG_TIMER1_CURRENT_VALUE: 1 0x%08lx\n", read_robot(REG_TIMER1_CURRENT_VALUE));
	printf("n=%ld\n", n);

	for(i = 0; i < n; i++)
		;

	printf("REG_TIMER1_CURRENT_VALUE: 2 0x%08lx\n", read_robot(REG_TIMER1_CURRENT_VALUE));
	x = dw_timer_stop(TIMER_ID1);
	printf("x=%lu\n", x);
}
#endif

void main(void)
{
#if WDT_ENABLE
	// Disable watchdog
	WDTCN = 0xDE;
	WDTCN = 0xAD;
#endif

	CKCON = 0x7;
	xs1 = 0xF1;

#if 0
	write_robot(TOP_BASE + 0x1000 + 0x90, 2); // PWR_WAKEUP0 PWR_UART0_TX 2
	write_robot(TOP_BASE + 0x1000 + 0xA4, 2); // PWR_GPIO0 PWR_UART0_RX 2
#endif

	dw_uart_init();
	// mcu51_uart_init(0xff, 0x0b); // Baud Rate = 39,062.50
	// mcu51_uart_init(0x3f, 0xc2); // Baud Rate = 39,062.50
	dw_ictl_init();

	printf("MCU test start A2\n");

#if 0
	// 51 UART
	write_robot(TOP_BASE + 0x1000 + 0xC0, 2); // UART2_TX PWR_UART1_TX 2
	write_robot(TOP_BASE + 0x1000 + 0xC8, 2); // UART2_RX PWR_UART1_RX 2
	// reg_51_xdata_mode (0x05025020[6])
	// write_robot(0x05025020, read_robot(0x05025020) | (1 << 6));
	// printf("B4\n");
#endif

	// write_robot(0x0502708C, 0x0); // latch PWR_WAKEUP0 pinmux
	// write_robot(0x050270D4, 0x0); // latch UART2_RX pinmux

	// speed_test();


#if 0
	large_code();
#endif

#if 0
	dw_timer_udelay(TIMER_ID1, 1);
	printf("SAR=0x%08lx\n", read_robot(0x0502c000));
#endif

#if 0
	dw_ictl_unmask(ST_CHANGE_INTR);
	dw_ictl_unmask(IRQ_AP2RTC0_INTR);
	dw_ictl_unmask(IRQ_AP2RTC1_INTR);
#endif

#if TSET_POWER_WFI
	test_wfi();
#endif

#if TSET_POWER_OSC
	test_power_osc();
#endif

#if TEST_WDT
	wdt_test();
#endif
#if TEST_ROBOT1
	test_robot1();
#endif
#if TEST_GPIO
	test_gpio();
#endif
#if TEST_DW_TIMER
	test_dw_timer();
#endif
#if TEST_MAILBOX
	test_mailbox();
#endif
#if TEST_IDLE
	test_idle();
#endif
#if TEST_MCU51_TIMER
	test_mcu51_timer();
#endif
#if TEST_MCU51_UART
	test_mcu51_uart();
#endif
#if TEST_RTC_DW_UART
	test_rtc_dw_uart();
#endif
#if TEST_I2C
	test_i2c();
#endif
#if TEST_SPINOR
	test_spinor();
#endif

	printf("MCU test end\n");

	xs1 = 0xF8;

	while (1)
		;
}
