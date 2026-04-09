#include <stdio.h>
#include "boot_test.h"
#include "system_common.h"
#include "cli.h"
#include "command.h"
#include "mmio.h"
 #include "common.h"

#define REG_PWM0_BASE		0x03060000
#define REG_PWM1_BASE		0x03061000
#define REG_PWM2_BASE		0x03062000
#define REG_PWM3_BASE		0x03063000
#define REG_PWM_BASE		REG_PWM1_BASE
#define	HLPERIOD0			0x0
#define PERIOD0				0x4
#define HLPERIOD1			0x8
#define PERIOD1				0xc
#define HLPERIOD2			0x10
#define PERIOD2				0x14
#define HLPERIOD3			0x18
#define PERIOD3				0x1c
#define FREQNUM				0x20
#define FREQDATA			0x24
#define POLARITY			0x40
#define PWMSTART			0x44
#define PCOUNT0				0x50
#define PCOUNT1				0x54
#define PCOUNT2				0x58
#define PCOUNT3				0x5C
#define PCOUNT0_STATUS		0x60
#define PCOUNT1_STATUS		0x64
#define PCOUNT2_STATUS		0x68
#define PCOUNT3_STATUS		0x6C
#define SHIFTCOUNT0			0x80
#define SHIFTCOUNT1			0x84
#define SHIFTCOUNT2			0x88
#define SHIFTCOUNT3			0x8C
#define SHIFTSTART			0x90
#define FREQEN				0x9c
#define FREQE_DONE_NUM		0xc0
#define PWMOE				0xd0

enum pwm_mode {
	MODE_CONTINUE = 0,
	MODE_COUNT,
	MODE_SHIFT
};

enum pwm_chip {
	PWM_CHIP_0 = 0,
	PWM_CHIP_1,
	PWM_CHIP_2,
	PWM_CHIP_3
};

int do_pwm0(int argc, char **argv)
{
	int period, hlp	, pola;

	period = strtol(argv[1], NULL, 0);
	hlp = strtol(argv[2], NULL, 0);
	pola = strtol(argv[3], NULL, 0);

	writel(REG_PWM_BASE + PWMSTART, readl(REG_PWM_BASE + PWMSTART) &= (~(1 << 0)));
	writel(REG_PWM_BASE + PERIOD0, period);
	writel(REG_PWM_BASE + HLPERIOD0, period - hlp);

	if (pola == 0)
		writel(REG_PWM_BASE + POLARITY, readl(REG_PWM_BASE + POLARITY) &= (~(1 << 0)));
	else
		writel(REG_PWM_BASE + POLARITY, readl(REG_PWM_BASE + POLARITY) |= (1 << 0));
	writel(REG_PWM_BASE + PWMOE, readl(REG_PWM_BASE + PWMOE) |= (1 << 0));
	writel(REG_PWM_BASE + PWMSTART, readl(REG_PWM_BASE + PWMSTART) |= (1 << 0));
	uartlog("Output %d%% duty cycle waveform, clk=pclk/%d\n", hlp * 100 / period, period);

	return 0;
}

int do_pwm1(int argc, char **argv)
{
	int period, hlp	, pola;

	period = strtol(argv[1], NULL, 0);
	hlp = strtol(argv[2], NULL, 0);
	pola = strtol(argv[3], NULL, 0);

	writel(REG_PWM_BASE + PWMSTART, readl(REG_PWM_BASE + PWMSTART) &= (~(1 << 1)));
	writel(REG_PWM_BASE + PERIOD1, period);
	writel(REG_PWM_BASE + HLPERIOD1, period - hlp);
	if (pola == 0)
		writel(REG_PWM_BASE + POLARITY, readl(REG_PWM_BASE + POLARITY) &= (~(1 << 1)));
	else
		writel(REG_PWM_BASE + POLARITY, readl(REG_PWM_BASE + POLARITY) |= (1 << 1));
	writel(REG_PWM_BASE + PWMOE, readl(REG_PWM_BASE + PWMOE) |= (1 << 1));
	writel(REG_PWM_BASE + PWMSTART, readl(REG_PWM_BASE + PWMSTART) |= (1 << 1));
	uartlog("Output %d%% duty cycle waveform, clk=pclk/%d\n", hlp * 100 / period, period);

	return 0;
}

int do_pwm2(int argc, char **argv)
{
	int period, hlp	, pola;

	period = strtol(argv[1], NULL, 0);
	hlp = strtol(argv[2], NULL, 0);
	pola = strtol(argv[3], NULL, 0);

	writel(REG_PWM_BASE + PWMSTART, readl(REG_PWM_BASE + PWMSTART) &= (~(1 << 2)));
	writel(REG_PWM_BASE + PERIOD2, period);
	writel(REG_PWM_BASE + HLPERIOD2, period - hlp);
	if (pola == 0)
		writel(REG_PWM_BASE + POLARITY, readl(REG_PWM_BASE + POLARITY) &= (~(1 << 2)));
	else
		writel(REG_PWM_BASE + POLARITY, readl(REG_PWM_BASE + POLARITY) |= (1 << 2));
	writel(REG_PWM_BASE + PWMOE, readl(REG_PWM_BASE + PWMOE) |= (1 << 2));
	writel(REG_PWM_BASE + PWMSTART, readl(REG_PWM_BASE + PWMSTART) |= (1 << 2));
	uartlog("Output %d%% duty cycle waveform, clk=pclk/%d\n", hlp * 100 / period, period);

	return 0;
}

int do_pwm3(int argc, char **argv)
{
	int period, hlp	, pola;

	period = strtol(argv[1], NULL, 0);
	hlp = strtol(argv[2], NULL, 0);
	pola = strtol(argv[3], NULL, 0);

	writel(REG_PWM_BASE + PWMSTART, readl(REG_PWM_BASE + PWMSTART) &= (~(1 << 3)));
	writel(REG_PWM_BASE + PERIOD3, period);
	writel(REG_PWM_BASE + HLPERIOD3, period - hlp);
	if (pola == 0)
		writel(REG_PWM_BASE + POLARITY, readl(REG_PWM_BASE + POLARITY) &= (~(1 << 3)));
	else
		writel(REG_PWM_BASE + POLARITY, readl(REG_PWM_BASE + POLARITY) |= (1 << 3));
	writel(REG_PWM_BASE + PWMOE, readl(REG_PWM_BASE + PWMOE) |= 1 << 3);
	writel(REG_PWM_BASE + PWMSTART, readl(REG_PWM_BASE + PWMSTART) |= 1 << 3);
	uartlog("Output %d%% duty cycle waveform, clk=pclk/%d\n", hlp * 100 / period, period);

	return 0;
}

static int get_pwm_addr(int chip)
{
	switch (chip) {
	case PWM_CHIP_0:
		uartlog("pls use other pwm chip, because pwm0 use to adjust core power\n");
		return -1;
	case PWM_CHIP_1:
		return REG_PWM1_BASE;
	case PWM_CHIP_2:
		return REG_PWM2_BASE;
	case PWM_CHIP_3:
		return REG_PWM3_BASE;
	default:
		uartlog("invalid chip num\n");
		return -1;
	}
}

int do_pwm_continue(int argc, char **argv)
{
	int chip, period, hlp, pola;
	int base_addr;

	chip = strtol(argv[1], NULL, 0);
	period = strtol(argv[2], NULL, 0);
	hlp = strtol(argv[3], NULL, 0);
	pola = strtol(argv[4], NULL, 0);

	base_addr = get_pwm_addr(chip);
	if (base_addr < 0) {
		return -1;
	}

	mmio_write_32(base_addr + HLPERIOD0, period - hlp);
	mmio_write_32(base_addr + PERIOD0, period);

	mmio_write_32(base_addr + HLPERIOD1, period - hlp);
	mmio_write_32(base_addr + PERIOD1, period);

	mmio_write_32(base_addr + HLPERIOD2, period - hlp);
	mmio_write_32(base_addr + PERIOD2, period);

	mmio_write_32(base_addr + HLPERIOD3, period - hlp);
	mmio_write_32(base_addr + PERIOD3, period);

	uartlog("Output %d%% duty cycle waveform, clk=pclk/%d\n", hlp * 100 / period, period);

	mmio_write_32(base_addr + PWMOE, 0xf);//output
	mmio_write_32(base_addr + POLARITY, (pola ? 0xf : 0x0));
	mmio_write_32(base_addr + PWMSTART, 0x0F);

	uartlog("pwm continue output...\n");

	return 0;
}

int do_pwm_count(int argc, char **argv)
{
	int chip, period, hlp, pola, count;
	int base_addr;
	int i;

	chip = strtol(argv[1], NULL, 0);
	period = strtol(argv[2], NULL, 0);
	hlp = strtol(argv[3], NULL, 0);
	pola = strtol(argv[4], NULL, 0);
	count = strtol(argv[5], NULL, 0);

	base_addr = get_pwm_addr(chip);
	if (base_addr < 0) {
		return -1;
	}

	mmio_write_32(base_addr + HLPERIOD0, period - hlp);
	mmio_write_32(base_addr + PERIOD0, period);

	mmio_write_32(base_addr + HLPERIOD1, period - hlp);
	mmio_write_32(base_addr + PERIOD1, period);

	mmio_write_32(base_addr + HLPERIOD2, period - hlp);
	mmio_write_32(base_addr + PERIOD2, period);

	mmio_write_32(base_addr + HLPERIOD3, period - hlp);
	mmio_write_32(base_addr + PERIOD3, period);

	uartlog("Output %d%% duty cycle waveform, clk=pclk/%d\n", hlp * 100 / period, period);

	mmio_write_32(base_addr + PCOUNT0, count);
	mmio_write_32(base_addr + PCOUNT1, count);
	mmio_write_32(base_addr + PCOUNT2, count);
	mmio_write_32(base_addr + PCOUNT3, count);

	mmio_write_32(base_addr + PWMOE, 0xf);//output
	mmio_write_32(base_addr + POLARITY, (pola ? 0xf : 0x0) | (0xf << 8));
	mmio_write_32(base_addr + PWMSTART, 0x0F);

	mdelay(1000);

	for (i=0; i<4; i++) {
		if (mmio_read_32(base_addr + PCOUNT0_STATUS + i*4) != count) {
			uartlog("pcount[%d] != %d\n", i, count);
			return -1;
		}
	}

	uartlog("pwm count test pass!\n");

	return 0;
}

int do_pwm_shift(int argc, char **argv)
{
	int chip, period, hlp, pola, shift0, shift1, shift2, shift3;
	int base_addr;

	chip = strtol(argv[1], NULL, 0);
	period = strtol(argv[2], NULL, 0);
	hlp = strtol(argv[3], NULL, 0);
	pola = strtol(argv[4], NULL, 0);
	shift0 = strtol(argv[5], NULL, 0);
	shift1 = strtol(argv[6], NULL, 0);
	shift2 = strtol(argv[7], NULL, 0);
	shift3 = strtol(argv[8], NULL, 0);

	base_addr = get_pwm_addr(chip);
	if (base_addr < 0) {
		return -1;
	}

	mmio_write_32(base_addr + HLPERIOD0, period - hlp);
	mmio_write_32(base_addr + PERIOD0, period);

	mmio_write_32(base_addr + HLPERIOD1, period - hlp);
	mmio_write_32(base_addr + PERIOD1, period);

	mmio_write_32(base_addr + HLPERIOD2, period - hlp);
	mmio_write_32(base_addr + PERIOD2, period);

	mmio_write_32(base_addr + HLPERIOD3, period - hlp);
	mmio_write_32(base_addr + PERIOD3, period);

	uartlog("Output %d%% duty cycle waveform, clk=pclk/%d\n", hlp * 100 / period, period);

	mmio_write_32(base_addr + POLARITY, (pola ? 0xf : 0x0) | (1 << 16)); // set shift mode

	mmio_write_32(base_addr + SHIFTCOUNT0, shift0);
	mmio_write_32(base_addr + SHIFTCOUNT1, shift1);
	mmio_write_32(base_addr + SHIFTCOUNT2, shift2);
	mmio_write_32(base_addr + SHIFTCOUNT3, shift3);

	mmio_write_32(base_addr + PWMOE, 0xf);//output
	mmio_write_32(base_addr + SHIFTSTART, 0x01); // shift start
	mmio_write_32(base_addr + PWMSTART, 0x0F); // pwm start

	uartlog("pwm shift output...\n");

	return 0;
}

int do_pwm_capture(int argc, char **argv)
{
	int chip, channel;
	int base_addr;
	int value, cycle_cnt, timeout = 1000;

	chip = strtol(argv[1], NULL, 0);
	channel = strtol(argv[2], NULL, 0);

	base_addr = get_pwm_addr(chip);
	if (base_addr < 0) {
		return -1;
	}

	channel = channel % 4;

	mmio_write_32(base_addr + PWMOE, 0);//input

	// Enable capture
	mmio_write_32(base_addr + 0x8 * channel + FREQNUM, 1);
	mmio_write_32(base_addr + FREQEN, 1 << channel);

	// Wait for done status
	while (timeout--) {
		mdelay(1);
		value = mmio_read_32(base_addr + FREQE_DONE_NUM + channel * 4);
		if (value != 0)
			break;
	}

	// Read cycle count
	cycle_cnt = mmio_read_32(base_addr + 0x8 * channel + FREQDATA) + 1;

	// Convert from cycle count to period ns
	cycle_cnt *= 10;// 1/100MHz = 10ns

	uartlog("pwm capture: period = %dns\n", cycle_cnt);

	// Disable capture
	mmio_write_32(base_addr + FREQEN, 0x0);

	return 0;
}

int do_pwm_disable(int argc, char **argv)
{
	int chip;
	int base_addr;

	chip = strtol(argv[1], NULL, 0);

	base_addr = get_pwm_addr(chip);
	if (base_addr < 0) {
		return -1;
	}

	mmio_write_32(base_addr + SHIFTSTART, 0x0); // shift disable
	mmio_write_32(base_addr + PWMSTART, 0x0); // pwm disable
	mmio_write_32(base_addr + POLARITY, 0x0);
	mmio_write_32(base_addr + HLPERIOD0, 0x1);
	mmio_write_32(base_addr + PERIOD0, 0x2);
	mmio_write_32(base_addr + HLPERIOD1, 0x1);
	mmio_write_32(base_addr + PERIOD1, 0x2);
	mmio_write_32(base_addr + HLPERIOD2, 0x1);
	mmio_write_32(base_addr + PERIOD2, 0x2);
	mmio_write_32(base_addr + HLPERIOD3, 0x1);
	mmio_write_32(base_addr + PERIOD3, 0x2);

	return 0;
}

static struct cmd_entry pwm_cmd_list[] = {
  {"pwm0", do_pwm0, 3, "divider duty_cycle polarity ## pwm0 100 60 0"},
  {"pwm1", do_pwm1, 3, "divider duty_cycle polarity ## pwm1 100 60 0"},
  {"pwm2", do_pwm2, 3, "divider duty_cycle polarity ## pwm2 100 60 0"},
  {"pwm3", do_pwm3, 3, "divider duty_cycle polarity ## pwm3 100 60 0"},
  {"pwmcontinue", do_pwm_continue, 4, "chip divider duty_cycle polarity ##e.g. pwmcontinue 1 100 60 0"},
  {"pwmcount", do_pwm_count, 5, "chip divider duty_cycle polarity count ##e.g. pwmcount 1 100 60 0 100"},
  {"pwmshift", do_pwm_shift, 8, "chip divider duty_cycle polarity shift0 shift1 shift2 shift3 ##e.g. pwmshift 1 100 60 0 10 20 30 40"},
  {"pwmcapture", do_pwm_capture, 2, "chip channel ##e.g. pwmcapture 1 1"},
  {"pwmdisable", do_pwm_disable, 1, "chip ##e.g. pwmdisable 1"},
  {NULL, NULL, 0, NULL}
};

int testcase_pwm_cmd(void)
{
	struct cmd_entry *cmd = pwm_cmd_list;

	for(; cmd->name != NULL; cmd++)
	{
		command_add(cmd);
	}
	cli_simple_loop();

	return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  // bit8: Clock Select for PWM's clock p_clk (aka clk_apb_pwm)
  mmio_write_32(0x03002004, mmio_read_32(0x03002004) | (1 << 8));
  // Clock Enable for clk_pwm_src
  mmio_write_32(0x03002010, mmio_read_32(0x03002010) | (1 << 4));
  // 1: Select int_clk_pwm_src as clock source 0: Select int_clk_axi6 as clock source
//  mmio_write_32(0x03002120, 0x000F0009);
	mmio_write_32(0x03001090, 0x7);
  testcase_pwm_cmd();

  return 0;
}
#endif