#include <stdio.h>
#include "test.h"

#define PRINT_IN_SUSPEND 1

typedef uint32_t uintptr_t;

#define mmio_read_32 read_robot
#define mmio_write_32 write_robot

void mmio_clrbits_32(uintptr_t addr, uint32_t clear)
{
	mmio_write_32(addr, mmio_read_32(addr) & ~clear);
}

void mmio_setbits_32(uintptr_t addr, uint32_t set)
{
	mmio_write_32(addr, mmio_read_32(addr) | set);
}

void mmio_clrsetbits_32(uintptr_t addr, uint32_t clear, uint32_t set)
{
	mmio_write_32(addr, (mmio_read_32(addr) & ~clear) | set);
}

void setup_osc()
{
	xdata uint32_t pass, pass_pre = 0xFFFFFFFFUL;
	xdata uint32_t count, count_pre = 0;
	xdata uint32_t max_count = 6060 / 60;

	printf("Setup OSC max_count=%lu\n", max_count);

	for (pass = 0xFFFFFFFFUL, count = 0; count <= max_count; pass >>= 1) {
		printf("pass=0x%08lx\n", pass);
		// set reg_osc_enable =0                        //0x0502802c[0]
		mmio_clrbits_32(0x0502802cUL, 1);
		// set reg_icg_enable  =0                       // 0x0502802c[4]
		mmio_clrbits_32(0x0502802cUL, (1UL << 4));
		// set reg_selpass from FFFFFFFF-> 1 //0x0502_800c[31:0]
		mmio_write_32(0x0502800cUL, pass);
		// set reg_osc_enable =1                       //0x0502802c[0] = 1
		mmio_setbits_32(0x0502802cUL, 1);
		// set reg_icg_enable =1                        //0x0502802c[4] =1
		mmio_setbits_32(0x0502802cUL, 1UL << 4);
		// set reg_trig_tog = 1                            //0x05028000 =1
		mmio_write_32(0x05028000UL, 1);
		// polling status reg_status ==0          //0x05028008
		while (mmio_read_32(0x05028008UL) != 0)
			;
		count_pre = count;
		pass_pre = pass;
		count = mmio_read_32(0x05028010UL);
		printf("count=%ld\n", count);
		// Example if reg_count ==425 (DEC)
		// OSC freq = 9.41 Mhz  :   1/ (4000 / 425)
		// freq = (4000 * mul) / (count * mul) * 600;
		// printf("freq=%d mul=%d\n", freq, mul);
	}

	printf("count_pre=%ld pass_pre=0x%08lx\n", count_pre, pass_pre);

	// set reg_osc_enable =0                        //0x0502802c[0]
	mmio_clrbits_32(0x0502802cUL, 1);
	// set reg_icg_enable  =0                       // 0x0502802c[4]
	mmio_clrbits_32(0x0502802cUL, (1UL << 4));
	// set reg_selpass from FFFFFFFF-> 1 //0x0502_800c[31:0]
	mmio_write_32(0x0502800cUL, pass);
	// set reg_osc_enable =1                       //0x0502802c[0] = 1
	mmio_setbits_32(0x0502802cUL, 1);
	// set reg_icg_enable =1                        //0x0502802c[4] =1
	mmio_setbits_32(0x0502802cUL, 1UL << 4);
	// set reg_trig_tog = 1                            //0x05028000 =1
	mmio_write_32(0x05028000UL, 1);
	// polling status reg_status ==0          //0x05028008
	printf("pass=0x%08lx\n", pass);
	while (mmio_read_32(0x05028008UL) != 0)
		;
	// set reg_osc_enable =0                        //0x0502802c[0]
	mmio_clrbits_32(0x0502802cUL, 1);  // osc req ctrl by PMU
}

#if TSET_POWER_WFI
int test_wfi()
{
	uint32_t v;
	// mw.l 0x050260e4 0x1;
	// mw.l 0x05026120 0x1;
	// mw.l 0x050260bc 0x40;
	// mw.l 0x05025004 0xAB18;
	// mw.l 0x05025008 0xFFFF0880;
	printf("MCU WFI Test 5\n");

	printf("Set register\n");
	// ADDR_RTC_ANA_CALIB =1 from f32kless
	// 0x05026000[31] => 1'b1
	mmio_setbits_32(0x05026000UL, 1UL << 31);
	// ADDR_RTC_SEC_PULSE_GEN =1  from f32kless
	// 0x05026004[31] => 1'b1
	mmio_setbits_32(0x05026004UL, 1UL << 31);
	// reg_ct_enable
	// 0x0502a030 =>1
	mmio_write_32(0x0502a030UL, 1UL);
	// reg_period_enable
	// 0x0502a038[0] => 1
	mmio_setbits_32(0x0502a038UL, 1UL);
	// reg_sectick_enable
	// 0x0502a000[8] => 1
	mmio_setbits_32(0x0502a000UL, 1UL << 8);


	// 0X050260e4 => 0X1 // RTC_EN_SUSPEND_REQ
	write_robot(0X050260e4UL, 0x1UL);
	// 0X05026120 => 0X1 // RTC_EN_WAKEUP_REQ
	write_robot(0X05026120UL, 0x1UL);
	// 0X050260bc[6] => 0X1  // RTC_EN_PWR_WRKEUP[6]
	write_robot(0X050260bcLU, 1UL << 6);
	// 0x05025004 => 0xAB18
	write_robot(0x05025004UL, 0xAB18UL);

	printf("suspend and polling\n");

	// 0x05025008 => 0xFFFF0880 //request suspend
	write_robot(0x05025008L, 0xFFFF0880UL);

	while (1) {
		v = read_robot(0x0502500C);
		v >>= 26;
		v &= 0xF;
		if (v == 0x9)
			break;
	}

	// printf("MCU enter into WFI\n");
	PCON = 1;
	// printf("MCU exit from WFI\n");

	xs1 = 0xF8;

	while (1)
		;
}
#endif /* TSET_POWER_WFI */

void suspend_and_polling()
{
	xdata uint32_t v;

	// reg_rtcsys_rstn_src_sel 0x05025018[31]= 1’b1
	// reg_rtcsys_reset_en (0x05025018[24]) => 1'b1
	mmio_setbits_32(0x05025018, (1UL << 24) | (1UL << 31));
	printf("0x18: 0x%lx\n", read_robot(0x05025018));

	// 0X050260e4 => 0X1 // RTC_EN_SUSPEND_REQ
	write_robot(0X050260e4UL, 0x1UL);
	// 0X05026120 => 0X1 // RTC_EN_WAKEUP_REQ
	write_robot(0X05026120UL, 0x1UL);
	// 0X050260bc[6] => 0X1  // RTC_EN_PWR_WRKEUP[6]
	write_robot(0X050260bcLU, 1UL << 6);
	// 0x05025004 => 0xAB18
	write_robot(0x05025004UL, 0xAB18UL);

	printf("suspend and polling\n");
	printf("0x1c=0x%lx\n", read_robot(0x0502501c));
	printf("0x28=0x%lx\n", read_robot(0x05025028));
	printf("0x30=0x%lx\n", read_robot(0x05025030));

#if 1
	printf("Set 0x05025028=0x2032FF00\n");
	write_robot(0x05025028, 0x2032FF00UL);
	printf("set reg_rtcsys_clk25m_req (0x05025028[15])  =  0\n");
	// reg_rtcsys_clk25m_req (0x05025028[15])  =  0
	mmio_clrbits_32(0x05025028, 1UL << 15);
#endif

	printf("0x05025028=0x%08lx\n", read_robot(0x05025028));
	printf(".............................\n");

	// 0x05025008 => 0xFFFF0880 //request suspend
	write_robot(0x05025008L, 0xFFFF0880UL);

#if PRINT_IN_SUSPEND
	printf("polling\n");
#endif
	while (1) {
		v = read_robot(0x0502500C);
		v >>= 26;
		v &= 0xF;
		if (v == 0x9)
			break;
	}
}

#if TSET_POWER_OSC

void read_saradc()
{
	// CV182X read RTC_SARADC:
	// Step 1: set channel (1~3, 0 is invalid)
	xdata uint32_t channel = 1, v;

	printf("A1\n");
	write_robot(0x0502C004UL, (1UL << (channel + 4)) | 0x1UL); // bit 0 is enable bit. bit 5~7 is ch 1~3

	printf("A2\n");
	// Step 2: wait for ready bit
	while (read_robot(0x0502C008UL) & 0x1UL)
		;

	// Step 3: read channel value
	v = read_robot(0x0502C014UL + (channel - 1UL) * 4UL);
#if PRINT_IN_SUSPEND
	printf("SAR=0x%lx\n", v);
#endif
}

void test_power_osc()
{
	volatile uint8_t a = 0;
	xdata int nsec = 10;
	// xdata unsigned int br = 0xFEEC;
	xdata unsigned int br = 0xFFE8;

	setup_osc();

	printf("Switch to OSC 6\n");
	// rtcsys_clkbyp
	write_robot(0x05025030UL, 0xffffffffUL);
	// write_robot(0x05025030UL, 0xfffffffcUL);

	// 1. RTCSYS clock switch to OSC or XTAL
	// 0x0502503c[3:0] => 0x5	// osc /5 for mcu
	mmio_clrsetbits_32(0x0502503c, 0xf, 0x1);
	// 0x0502503c[11:8] => 0x4		// osc/4 for sd1
	// mmio_clrsetbits_32(0x0502503c, 0xf << 8, 0x4 << 8);
	// 0x0502503c[11:8] => 0x4		// osc/1 for sd1
	mmio_clrsetbits_32(0x0502503c, 0xf << 8, 0x1 << 8);
	// 0x0502503c[29:24] => 12
	mmio_clrsetbits_32(0x0502503c, 0x3f << 24, 12UL << 24);


	// 0x0502501c[7:0] => 0x22 // mcu/sd1 switch to osc
	mmio_clrsetbits_32(0x0502501c, 0xff, 0x22);
	// 0x0502501c[11:8] => 0x5 //timer switch to 32k
	mmio_clrsetbits_32(0x0502501c, 0xfUL << 8, 0x5UL << 8);
	// 0x0502501c[20] -> 1 //sar use osc
	mmio_setbits_32(0x0502501c, 0x1UL << 20);
	// 0x05025030[1:0] => 0x0 // deglitch mux
	mmio_clrbits_32(0x05025030, 0x3);

	printf("Reset 51 UART to 0x%x\n", br);
	mcu51_uart_init(br >> 8, br & 0xFF); // baudrate for 300MHz osc

	printf("Enable TIMER1\n");
	dw_timer_tick = dw_timer_tick_last = 0;
	dw_timer_start(TIMER_ID1, 32L * 1024L / 2L * nsec);
	printf("REG_TIMER1_LOADCNT: 0x%08lx nsec=%d\n", read_robot(REG_TIMER1_LOADCNT), nsec);

	// 0x05025038[15:0] => 0Xffff //reg_rtcsys_wkint_mask
	// v = read_robot(0x05025038);
	// v &= 0xffff8000UL;
	// write_robot(0x05025038, v);
	mmio_clrbits_32(0x05025038UL, 3UL << 2);

	// Enable dw_timer interrupt
	dw_ictl_unmask(TIMER1_INTR);

	EX0 = 1;
	// EX1 = 1;
	EA = 1;

	suspend_and_polling();

#if PRINT_IN_SUSPEND
	printf("Wait dw_timer_tick..\n");
#endif
	while (1) {
		// while(dw_timer_tick == dw_timer_tick_last)
		// 	;

#if PRINT_IN_SUSPEND
		printf("MCU enter into WFI\n");
#endif
		PCON = 1;

		mcu51_uart_init(br >> 8, br & 0xFF); // baudrate for 300MHz osc
#if PRINT_IN_SUSPEND
		printf("T: dw_timer_tick=%d %d\n", dw_timer_tick, dw_timer_tick_last);
#endif
		dw_timer_tick_last = dw_timer_tick;

		read_saradc();

		dw_timer_start(TIMER_ID1, 32L * 1024L / 2L * nsec);
	}

#if 0
	printf("looping...\n");
	while (1) {
		a++;
	}
#endif

}
#endif /* TSET_POWER_OSC */
