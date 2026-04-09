#include <stdio.h>
#include "boot_test.h"
#include "system_common.h"
#include "cli.h"
#include "command.h"
#include "mmio.h"

#define REG_PWM_BASE		0x03061000
#define	HLPERIOD0			0x0
#define PERIOD0				0x4
#define HLPERIOD1			0x8
#define PERIOD1				0xc
#define HLPERIOD2			0x10
#define PERIOD2				0x14
#define HLPERIOD3			0x18
#define PERIOD3				0x1c
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

int do_pwm0(int argc, char **argv)
{
	int period, hlp	, pola;

	period = strtol(argv[1], NULL, 0);
	hlp = strtol(argv[2], NULL, 0);
	pola = strtol(argv[3], NULL, 0);
	
	writel(REG_PWM_BASE + PERIOD0, period);	
	writel(REG_PWM_BASE + HLPERIOD0, period - hlp);	
	writel(REG_PWM_BASE + POLARITY, pola);	
	uartlog("Output %d%% duty cycle waveform, clk=pclk/%d\n", hlp * 100 / period, period);

	return 0;
}

int do_pwm1(int argc, char **argv)
{
	int period, hlp	, pola;

	period = strtol(argv[1], NULL, 0);
	hlp = strtol(argv[2], NULL, 0);
	pola = strtol(argv[3], NULL, 0);
	
	writel(REG_PWM_BASE + PERIOD1, period);	
	writel(REG_PWM_BASE + HLPERIOD1, period - hlp);	
	writel(REG_PWM_BASE + POLARITY, pola);	
	uartlog("Output %d%% duty cycle waveform, clk=pclk/%d\n", hlp * 100 / period, period);

	return 0;
}

int do_pwm2(int argc, char **argv)
{
	int period, hlp	, pola;

	period = strtol(argv[1], NULL, 0);
	hlp = strtol(argv[2], NULL, 0);
	pola = strtol(argv[3], NULL, 0);
	
	writel(REG_PWM_BASE + PERIOD2, period);	
	writel(REG_PWM_BASE + HLPERIOD2, period - hlp);	
	writel(REG_PWM_BASE + POLARITY, pola);	
	uartlog("Output %d%% duty cycle waveform, clk=pclk/%d\n", hlp * 100 / period, period);

	return 0;
}

int do_pwm3(int argc, char **argv)
{
	int period, hlp	, pola;

	period = strtol(argv[1], NULL, 0);
	hlp = strtol(argv[2], NULL, 0);
	pola = strtol(argv[3], NULL, 0);
	
	writel(REG_PWM_BASE + PERIOD3, period);	
	writel(REG_PWM_BASE + HLPERIOD3, period - hlp);	
	writel(REG_PWM_BASE + POLARITY, pola);	
	uartlog("Output %d%% duty cycle waveform, clk=pclk/%d\n", hlp * 100 / period, period);

	return 0;
}

static struct cmd_entry pwm_cmd_list[] = {
  {"pwm0", do_pwm0, 3, "pwm0 divider duty_cycle polarity## pwm0 100 60 0"},
  {"pwm1", do_pwm1, 3, "pwm1 divider duty_cycle polarity## pwm1 100 60 0"},
  {"pwm2", do_pwm2, 3, "pwm2 divider duty_cycle polarity## pwm2 100 60 0"},
  {"pwm3", do_pwm3, 3, "pwm3 divider duty_cycle polarity## pwm3 100 60 0"},
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

int testcase_pwm(void)
{
	uartlog("testcase_pwm\n");
	pinmux_config(PINMUX_PWM1);
	uartlog("set pinmux done\n");
	mmio_write_32(REG_PWM_BASE + HLPERIOD0, 0x08);
	mmio_write_32(REG_PWM_BASE + PERIOD0, 0x10);

	mmio_write_32(REG_PWM_BASE + HLPERIOD1, 0x08);
	mmio_write_32(REG_PWM_BASE + PERIOD1, 0x18);

	mmio_write_32(REG_PWM_BASE + HLPERIOD2, 0x08);
	mmio_write_32(REG_PWM_BASE + PERIOD2, 0x20);

	mmio_write_32(REG_PWM_BASE + HLPERIOD3, 0x08);
	mmio_write_32(REG_PWM_BASE + PERIOD3, 0x28);

	mmio_write_32(REG_PWM_BASE + PWMSTART, 0x0F);

	return 0;
}

int testcase_pwm_count(void)
{
	uint32_t pcount_status[4] = {0x10, 0x20, 0x30, 0x40};
	uint32_t i = 0;

	uartlog("testcase_pwm_count\n");
	pinmux_config(PINMUX_PWM1);
	uartlog("set pinmux done\n");
	mmio_write_32(REG_PWM_BASE + HLPERIOD0, 0x08);
	mmio_write_32(REG_PWM_BASE + PERIOD0, 0x10);
	mmio_write_32(REG_PWM_BASE + PCOUNT0, pcount_status[0]);

	mmio_write_32(REG_PWM_BASE + HLPERIOD1, 0x08);
	mmio_write_32(REG_PWM_BASE + PERIOD1, 0x18);
	mmio_write_32(REG_PWM_BASE + PCOUNT1, pcount_status[1]);

	mmio_write_32(REG_PWM_BASE + HLPERIOD2, 0x08);
	mmio_write_32(REG_PWM_BASE + PERIOD2, 0x20);
	mmio_write_32(REG_PWM_BASE + PCOUNT2, pcount_status[2]);

	mmio_write_32(REG_PWM_BASE + HLPERIOD3, 0x08);
	mmio_write_32(REG_PWM_BASE + PERIOD3, 0x28);
	mmio_write_32(REG_PWM_BASE + PCOUNT3, pcount_status[3]);

	mmio_write_32(REG_PWM_BASE + POLARITY, 0xF<<8); // set count mode
	mmio_write_32(REG_PWM_BASE + PWMSTART, 0x00);
	mmio_write_32(REG_PWM_BASE + PWMSTART, 0x0F);

	mdelay(1000);

	for (i=0; i<4; i++) {
		if (mmio_read_32(REG_PWM_BASE + PCOUNT0_STATUS + i*4) != pcount_status[i])
			return 1;
		else
			uartlog("pcount[%d] = %d\n", i, pcount_status[i]);
	}
	return 0;
}

int testcase_pwm_shift(void)
{
	uartlog("testcase_pwm_shift\n");
	pinmux_config(PINMUX_PWM1);
	uartlog("set pinmux done\n");
	mmio_write_32(REG_PWM_BASE + HLPERIOD0, 0x08);
	mmio_write_32(REG_PWM_BASE + PERIOD0, 0x10);

	mmio_write_32(REG_PWM_BASE + HLPERIOD1, 0x08);
	mmio_write_32(REG_PWM_BASE + PERIOD1, 0x10);

	mmio_write_32(REG_PWM_BASE + HLPERIOD2, 0x08);
	mmio_write_32(REG_PWM_BASE + PERIOD2, 0x10);

	mmio_write_32(REG_PWM_BASE + HLPERIOD3, 0x08);
	mmio_write_32(REG_PWM_BASE + PERIOD3, 0x10);

	mmio_write_32(REG_PWM_BASE + POLARITY, 1<<16); // set shift mode

	mmio_write_32(REG_PWM_BASE + SHIFTCOUNT0, 0x00);
	mmio_write_32(REG_PWM_BASE + SHIFTCOUNT1, 0x02);
	mmio_write_32(REG_PWM_BASE + SHIFTCOUNT2, 0x04);
	mmio_write_32(REG_PWM_BASE + SHIFTCOUNT3, 0x06);
	
	mmio_write_32(REG_PWM_BASE + SHIFTSTART, 0x01); // shift start
	mmio_write_32(REG_PWM_BASE + PWMSTART, 0x0F); // pwm start

	return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  int ret;

  // bit8: Clock Select for PWM's clock p_clk (aka clk_apb_pwm)
  // 1: Select int_clk_pwm_src as clock source 0: Select int_clk_axi6 as clock source
  mmio_write_32(0x03002020, 0xFFFFFEFF);

  ret = testcase_pwm_count();

  // int ret = testcase_pwm_shift();
  uartlog("testcase pwm %s\n", ret?"failed":"passed");
  return ret;
}
#endif