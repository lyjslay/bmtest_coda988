#include <stdio.h>
#include "test.h"

#if TEST_MCU51_TIMER

// #define TIMER0_COUNT 0xDC11 /* 10000h - ((11,059,200 Hz / (12 * FREQ)) - 17) */
#define TIMER0_COUNT 0xF880 // 1ms
static unsigned xdata timer0_tick;

void mcu51_timer0_isr(void) interrupt 1 using 1
{
	unsigned i;

	TR0 = 0; /* stop timer 0 */
	i = TIMER0_COUNT + TL0 + (TH0 << 8);
	TL0 = i;
	TH0 = i >> 8;
	TR0 = 1; /* start timer 0 */

	timer0_tick++;
}

void timer0_initialize(void)
{
	EA = 0; /* disable interrupts */
	TR0 = 0; /* stop timer 0 */
	TMOD &= ~0x0F; /* clear timer 0 mode bits */
	TMOD |= 0x01; /* put timer 0 into 16-bit no prescale */
	// TMOD |= (1 << 2); // counter mode
	TL0 = (TIMER0_COUNT & 0x00FF);
	TH0 = (TIMER0_COUNT >> 8);
	PT0 = 0; /* set low priority for timer 0 */
	ET0 = 1; /* enable timer 0 interrupt */
	TR0 = 1; /* start timer 0 */
	EA = 1; /* enable interrupts */
}

unsigned timer0_count(void)
{
	unsigned t;

	EA = 0;
	xs2 = timer0_tick;
	xs3++;
	t = timer0_tick;
	EA = 1;
	// printf("timer0_count=%u\n", timer0_count);
	return (t);
}

void timer0_delay(unsigned count)
{
	unsigned start_count;

	start_count = timer0_count(); /* get the starting count */
	while ((timer0_count() - start_count) <= count) /* wait for count “ticks” */
		;
}

int test_mcu51_timer()
{
	printf("test_mcu51_timer() T3 TIMER0_COUNT=0x%x:\n", TIMER0_COUNT);

	timer0_tick = 0;
	xs1 = 0xB1;
	// printf("A0 timer0_tick=%u\n", timer0_tick);
	timer0_initialize();

#if 0
	while(1) {
		xs2++;
		printf("timer0_tick=%u\n", timer0_tick);
	}
#else
	xs1 = 0xB2;
	printf("A1 timer0_tick=%u\n", timer0_tick);
	timer0_delay(1000);
	printf("A2 timer0_tick=%u\n", timer0_tick);
	xs1 = 0xB3;
#endif

	return 0;
}

#endif /* TEST_MCU51_TIMER */

#if TEST_DW_TIMER
static int xdata count;
static int xdata dw_timer_tick;
static int xdata dw_timer_tick_int1;
static uint32_t xdata dw_timer_eoi;

void dw_timer1_isr(void)
{
	dw_timer_tick++;
	dw_timer_eoi = read_robot_irq(REG_TIMER1_EOI);
}

void dw_timer1_isr_int1(void) interrupt 2 using 1
{
#if 1
	dw_timer_tick_int1++;
#else
	uint32_t int1_src_mask;

	count += 1;
	int1_src_mask = read_robot_irq(REG_51_INT1_SRC_MASK);

	int1_src_mask &= 0x00800000;
	if (int1_src_mask) {
		dw_timer_tick_int1++;
	}
#endif
}

void power()
{
	uint32_t xdata v;

	printf("low power A2\n");

#if 0
	// 4. enable interrupt
	// 0x05025038[15:0] => 0Xffff //reg_rtcsys_wkint_mask
	v = read_robot(0x05025038);
	v &= 0xffff8000UL;
	write_robot(0x05025038, v);
	// 0x0502507C[15:0] => 0Xffff //reg_51_int1_src_mask
	write_robot(0x0502507C, 0xffffUL);
	// 0x05025038[24] => 0X1      //reg_wkint2rtc_mask
	write_robot(0x05025038, v | (1UL << 24));
#else
	write_robot(0x05025038, 0);
#endif

	printf("low power 3\n");

	// 3. enable 8051, 系統下電
	// 0X050260e4 => 0X1 // RTC_EN_SUSPEND_REQ
	write_robot(0X050260e4, 0x1);
	// 0X05026120 => 0X1 // RTC_EN_WAKEUP_REQ
	write_robot(0X05026120, 0x1);
	// 0X050260bc[6] => 0X1  // RTC_EN_PWR_WRKEUP[6]
	write_robot(0X050260bc, 1 << 6);
	// 0x05025004 => 0xAB18
	write_robot(0x05025004, 0xAB18);
	// 0x05025008 => 0xFFFF0880 //request suspend
	printf("low power 4\n");
	write_robot(0x05025008L, 0xFFFF0880UL);

#if 1
	printf("low power 5\n");
	while(1) {
		v = read_robot(0x0502500C);
		v = (v & 0x3c000000);
		printf("v=0x%08lx\n", v);
		if (v == 0x24000000)
			break;
	}
#endif
	// 4. 51 into WFI

        printf("enter idle I1\n");
	xs2 = 0xD1;
        PCON = 1;
	xs2 = 0xD2;
        printf("waked\n");

	v = read_robot(0x05025038);
	v |= (1UL << 16);
	write_robot(0x05025038, v);
        printf("up\n");
}

int test_dw_timer()
{
	int nsec = 5;

	EA = 0;
	xs1 = 0xB5;

	printf("test_dw_timer() T2:\n");

	dw_timer_tick = 0;
	dw_timer_tick_int1 = 0;

	dw_timer_start(TIMER_ID1, MCU51_CLK_HZ * nsec);
	// dw_timer_start(TIMER_ID1, 32L * 1024L / 2 * nsec);
	printf("REG_TIMER1_LOADCNT: 0x%08lx nsec=%d\n", read_robot(REG_TIMER1_LOADCNT), nsec);

	// Enable dw_timer interrupt
	dw_ictl_unmask(TIMER1_INTR);
	// Unmask int1
	// write_robot(REG_51_INT1_SRC_MASK, 0xff7f);
	printf("REG_51_INT1_SRC_MASK: 0x%08lx\n", read_robot(REG_51_INT1_SRC_MASK));

	EX0 = 1;
	EX1 = 1;
	EA = 1;

	xs1 = 0xB6;

	// power();

#if 0
#if 1
	puts("wait dw_timer_tick.\n");
	while(dw_timer_tick < 1)
		;
#else
	printf("dw_timer_tick=%u dw_timer_tick_int1=%u\n", dw_timer_tick, dw_timer_tick_int1);
	puts("wait dw_timer_tick_int1..\n");
	while (dw_timer_tick_int1 == 0) {
		// printf("T: count=%d %lu %u\n", count, dw_timer_tick, dw_timer_tick_int1);
		// printf("REG_51_INT1_SRC_MASK: 0x%08lx\n", read_robot(REG_51_INT1_SRC_MASK));
	}
#endif
#endif

	printf("T: %u %u 0x%lx\n", dw_timer_tick, dw_timer_tick_int1, read_robot(REG_51_INT1_SRC_MASK));
	printf("REG_TIMERS_INTSTATUS: 0x%08lx\n", read_robot(REG_TIMERS_INTSTATUS));
	printf("REG_TIMERS_RAW_INTSTATUS: 0x%08lx\n", read_robot(REG_TIMERS_RAW_INTSTATUS));

	xs1 = 0xB7;

	return 0;
}

#endif /* TEST_DW_TIMER */