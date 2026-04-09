#include <stdio.h>
#include "test.h"

#if TEST_IDLE

void dw_timer1_isr(void)
{

}

int test_idle()
{
        printf("test_idle()\n");

	dw_timer_start(TIMER_ID1, MCU51_CLK_HZ * 5);

	// Enable dw_timer interrupt
	dw_ictl_unmask(TIMER1_INTR);

	EX0 = 1;
	// EX1 = 1;
	EA = 1;

        printf("enter idle\n");
	xs2 = 0xD1;
        PCON = 1;
	xs2 = 0xD2;
        printf("waked\n");
        return 0;
}

#endif /* TEST_IDLE */

#if TEST_IDLE_2
static xdata uint32_t count;

void test_idle_2()
{
#if 0
	// reg_rtc_stint_clr          (0x05025028[24] => 1’b0 -> 1'b1 -> 0)
	count = read_robot(0x05025028);
	count &= 0xfeffffff;
	write_robot(0x05025028, count);
	write_robot(0x05025028, count | 0x01000000UL);
	write_robot(0x05025028, count);

	write_robot(TOP_BASE + 0x1000 + 0x90, 2); // PWR_WAKEUP0 PWR_UART0_TX 2
	write_robot(TOP_BASE + 0x1000 + 0xA4, 2); // PWR_GPIO0 PWR_UART0_RX 2
	write_robot(0x0502708C, 0x0); // latch pinmux
#endif
#if 0
	polling();
#endif

}

void power2()
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
        printf("done\n");
}

void polling()
{
	EA = 0;

	count = 0;

	dw_ictl_unmask(MBOX_INTR);
	dw_ictl_unmask(SD1_INTR);
	dw_ictl_unmask(SD1_WAKEUP_INTR);

	// write_robot(REG_DW_ICTL_MASK_L, 0x00001000);
	// write_robot(REG_DW_ICTL_MASK_L, 0);
	// write_robot(REG_51_INT1_SRC_MASK, 0);

	// power2();

	EX0 = 1;
	EX1 = 1;
	EA = 1;
	while(1) {
		printf("T1 dw_ictl=0x%08lx(%lu) ICTL_FS_L=0x%08lx\n",
		       dw_ictl_get_status(), dw_ictl_get_count(), read_robot(REG_DW_ICTL_FINALSTATUS_L));
		printf("C1 count=%lu int1_status=0x%08x\n", count, read_robot(REG_51_INT1_SRC_MASK));
		count++;
	}

}
#endif /* TEST_IDLE_2 */