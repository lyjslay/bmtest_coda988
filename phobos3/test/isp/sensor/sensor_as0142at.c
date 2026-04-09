#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef ENV_HOSTPC
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include "cli/command.h"
#include "cli/cli.h"
#else
#include "command.h"
#include "cli.h"
#include "system_common.h"
#include "module_testcase.h"
#include "uart.h"
#include "timer.h"
#endif

#include "../../vip/reg.h"
#include "../inc/isp_reg.h"

#include "../sensorlist.h"
#include "../vip_common.h"



#undef uartlog
#define uartlog(fmt, args...) \
{ \
    printf(fmt, ##args); \
    fflush(stdout); \
}

#define CIF_DPHY_ENABLE

struct sensor_mode as0142at_mode[] = {
	{
		.name = "720p30",
		.bit_depth = 8,
		.active_w = 1280,
		.active_h = 720,
		.total_w = 1280,
		.total_h = 720,
	},
};

struct sensor_info as0142at_info = {
	.dev = 0x10, //0x30,
	.alen = 2,
	.dlen = 2,
};

int as0142at_poweron(void *priv)
{
	pattern_ctx *ctx = (pattern_ctx *)priv;
	char cmdbuf[8];
	struct sensor_mode *mode = &ar0239_mode[AR0239_MODE_720P30];

	uartlog("streaming start\n");

	cmdbuf[0] = cmdbuf[1] = '0';
	cli_readline("run sensor to dram flow? [n/y]", cmdbuf);
	if (cmdbuf[0] == 'y') {
		uartlog("dump %d x %d \n", mode->total_w, mode->total_h);
		config_csibdg(0, mode->total_w, mode->total_h, ctx, 0, 1);
	}
	return 0;
}

int peri_up_5(void *priv)
{

	char cmdbuf[8];
#if !defined(FLOW_AFTER_ECO)
	unsigned int *fpga_pwr_reg;
	uint32_t value;
#endif

	cmdbuf[0] = '0';
	cli_readline("### enter to start power-on seqeuence. s) to skip : ", cmdbuf);
	if (cmdbuf[0] == 's')
		return 0;

	// power down/reset
	// 1.2V = 1, 1.8V = 1, 2.9V = 1
	// INCK on
	// XCLR = 0
	// wait 500ns, wait INCK stable
	// XCLR = 1
	// wait 20us

#if !defined(FLOW_AFTER_ECO)
	// enable display clk select
	value = readl(0x0a0c8018);
	value |= 0x02;
	writel(0x0a0c8018, value);
	udelay(5000);
#endif

	// Switch the MAC type to TTL
	writel(0x0a0c2000, 0x00000002);
	// configure TTL [0] ttl enable [2:1] bit mode
	writel(0x0a0c2010, 0x00002421);
#if !defined(FLOW_AFTER_ECO)
	fpga_pwr_reg = (unsigned int *)HARD_REG_FPGA_PWRPIN;
	// clear reset and enable phy rx
	writel(fpga_pwr_reg, 0x30100001);
#endif
	uartlog("### device poweron\n");

	return 0;
}

int peri_down_5(void *priv)
{
	char cmdbuf[8] = {0};

	cli_readline("### power-off? y/n", cmdbuf);

	if (cmdbuf[0] == 'y') {
#if !defined(FLOW_AFTER_ECO)
		unsigned int *fpga_pwr_reg;

		fpga_pwr_reg = (unsigned int *)HARD_REG_FPGA_PWRPIN;
		// INCLK = 0
		// XCLR = 0
		// POWER-OFF sequence
		uartlog("power-off");
		writel(fpga_pwr_reg, 0x18);
		udelay(5000);
		writel(fpga_pwr_reg, 0x08);
		udelay(5000);
		writel(fpga_pwr_reg, 0);
		udelay(5000);
#endif
	}

	return 0;
}

