#include <stdio.h>
#include "test.h"

#if TEST_WDT
static xdata int wdt_intr_count;

void wdt_isr()
{
	wdt_intr_count++;
}

void wdt_test(void)
{
	uint32_t v;
	printf("WDT test 4\n");

	dw_ictl_unmask(WDT_INTR);
	// write_robot(REG_DW_ICTL_MASK_L, 0);
	// write_robot(REG_51_INT1_SRC_MASK, 0x7fff);
	printf("REG_DW_ICTL_MASK_L=0x%08lx REG_DW_ICTL_ENABLE_L=0x%08lx\n", read_robot(REG_DW_ICTL_MASK_L), read_robot(REG_DW_ICTL_ENABLE_L));
	// write_robot(TOP_BASE + 0x8, 4);

	// write_robot(WDT_BASE + 0x4, 0x20); // WDT_TORR
	// write_robot(WDT_BASE + 0xC, 0x76);
	// write_robot(WDT_BASE + 0x0, 0x13); // WDT_CR


	// case2. reset system, and trigger warm reset
	// 0x0502501c[13:12] => 0x1 // apb div 1
	// 0x050260E0 => 0x03​    //RTC_EN_WDG_RST_REQ
	// 0x050250a0[0]​ => 0x1  //enable rtc wdg reset
	// 0x05025d04   => 0x20  // Set timeout range register
	// 0x05025d0c   => 0x76 // Counter restart register
	// 0x05025d00  =>  0x3f  // Response mode and enable WDT
	write_robot(0x050260E8, 0x04);
	write_robot(0x050260E0, 0x03);
	write_robot(0x050250A0, 0x01);

	EX0 = 1;
	EX1 = 1;
	EA = 1;

	printf("Reset WDT\n");
	v = read_robot(0x05025018);
	v &= 0xfffbffff;
	write_robot(0x05025018, v);
	v |= 0x00040000;
	write_robot(0x05025018, v);

	write_robot(RTC_WDT_BASE + 0x4, 0x22); // WDT_TORR
	dw_timer_udelay(TIMER_ID1, 1000000);
	write_robot(RTC_WDT_BASE + 0x0, 0x12); // WDT_CR
	dw_timer_udelay(TIMER_ID1, 1000000);
	write_robot(RTC_WDT_BASE + 0x0, 0x13); // WDT_CR
	dw_timer_udelay(TIMER_ID1, 1000000);
	// write_robot(RTC_WDT_BASE + 0x0, 0x3F); // WDT_CR
	write_robot(RTC_WDT_BASE + 0xC, 0x76);
	dw_timer_udelay(TIMER_ID1, 1000000);
	// write_robot(0x05025034, 0xfdffffff); clk wdt apb

#if 0
	printf("test end\n");
        PCON = 1;
	printf("test wakeup\n");

	printf("A1 WDT_STAT=0x%lx WDT_CCVR=0x%08lx\n", read_robot(RTC_WDT_BASE + 0x10), read_robot(RTC_WDT_BASE + 0x08));
	printf("A2 WDT_STAT=0x%lx WDT_CCVR=0x%08lx\n", read_robot(RTC_WDT_BASE + 0x10), read_robot(RTC_WDT_BASE + 0x08));
	printf("A3 WDT_STAT=0x%lx WDT_CCVR=0x%08lx\n", read_robot(RTC_WDT_BASE + 0x10), read_robot(RTC_WDT_BASE + 0x08));
	printf("A4 WDT_STAT=0x%lx WDT_CCVR=0x%08lx\n", read_robot(RTC_WDT_BASE + 0x10), read_robot(RTC_WDT_BASE + 0x08));
#endif

	while(1) {
		printf("dw_ictl_status=0x%08lx wdt_intr_count=%u\n", dw_ictl_get_status(), wdt_intr_count);
		// printf("int1_count=%d int1_status=0x%08lx 0x%08lx\n", int1_count, int1_status, read_robot(REG_51_INT1_SRC_MASK));
		// printf("wdt_intr_count=%6d WDT_STAT=0x%lx WDT_CCVR=0x%08lx\n", wdt_intr_count, read_robot(RTC_WDT_BASE + 0x10), read_robot(RTC_WDT_BASE + 0x08));
		// printf("wdt_intr_count=%6d\n", wdt_intr_count);
		// printf("WDT_STAT=0x%08lx WDT_CCVR=0x%08lx\n", read_robot(WDT_BASE + 0x10), read_robot(WDT_BASE + 0x08));
		// printf("0x0502507c=0x%08lx\n", read_robot(0x0502507c));
	}
}
#endif