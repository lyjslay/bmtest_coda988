#define INCBIN_PREFIX
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#include "incbin.h"

#include "boot_test.h"
#include "uart.h"
#include "cli.h"
#include "mmio.h"
#include "bitwise_ops.h"
#include "system_common.h"
#include "cache.h"

#include "mars_reg_fmux_gpio.h"
#include "mars_pinlist_swconfig.h"

#include "testcase_mcu.h"

#define PINMUX_MASK(PIN_NAME) FMUX_GPIO_FUNCSEL_##PIN_NAME##_MASK
#define PINMUX_OFFSET(PIN_NAME) FMUX_GPIO_FUNCSEL_##PIN_NAME##_OFFSET
#define PINMUX_VALUE(PIN_NAME, FUNC_NAME) PIN_NAME##__##FUNC_NAME
#define PINMUX_CONFIG(PIN_NAME, FUNC_NAME)                                                                             \
	mmio_clrsetbits_32(PINMUX_BASE + FMUX_GPIO_FUNCSEL_##PIN_NAME,                                                 \
			   PINMUX_MASK(PIN_NAME) << PINMUX_OFFSET(PIN_NAME), PINMUX_VALUE(PIN_NAME, FUNC_NAME))

INCBIN(mcu51_case1, "case1.bin");

int testcase_mcu1(void)
{
	printf("[%s:%d] start\n", __func__, __LINE__);

	REG_WRITE(REG_SOFT_RSTN_MCU, 0);
	printf("REG_SOFT_RSTN_MCU: 0x%lx %d %d\n", REG_SOFT_RSTN_MCU_ADDR, REG_SOFT_RSTN_MCU_MSB,
	       REG_SOFT_RSTN_MCU_LSB);

	mmio_write_32(0x03000248, 1); // enable rtc2ap_ahb

	printf("case1: 0x%p %u\n", mcu51_case1_data, mcu51_case1_size);
	memset((void *)RTC_SRAM_BASE, 0x00, RTC_SRAM_SIZE);
	memcpy((void *)RTC_SRAM_BASE, mcu51_case1_data, mcu51_case1_size);
	flush_dcache_range(RTC_SRAM_BASE, RTC_SRAM_SIZE);

	// Mars
	uintptr_t a;

	a = RTC_CTRL_BASE + 0x20; // rtcsys_mcu51_ctrl0

	// reg_51xdata_ioffset0 = 0x05200000
	mmio_write_32(a, 0x05200000);

	// reg_51_rom_addr_size;
	mmio_setbits_32(a, 0xd);

	// reg_51_rom_addr_def[1];
	mmio_setbits_32(a, 1 << 7);

	printf("0x%08lx=0x%08x\n", a, mmio_read_32(a));

	a = RTC_CTRL_BASE + 0x24;
	mmio_write_32(a, 0x0); // reg_51_pf_mode = 0
	// mmio_write_32(a, 0x0 | (2 << 6)); // prefetch = 2

	printf("0x%08lx=0x%08x\n", a, mmio_read_32(a));

	// REG_WRITE(reg_51_dbg_step_en, 1);

	// printf("request_irq(140)\n");
	// request_irq(140, mcu_irq_handler, 0, "interrupt", NULL);

	printf("Fire MCU\n");
	REG_WRITE(REG_SOFT_RSTN_MCU, 1);

	mdelay(100000);

	return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
	int ret;

	ret = testcase_mcu1();
	printf("testcase_mcu1(): %s\n", ret ? "failed" : "passed");

	while (1)
		;

	return 0;
}
#endif
