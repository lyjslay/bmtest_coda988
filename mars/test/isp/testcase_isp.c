#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

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


#include "vip_common.h"
#include "../vip/reg.h"
#include "inc/isp_reg.h"
#include "isp_drv.h"

#include "patterns.h"
#include "sensorlist.h"


#define _CBSIZE         (32)


#ifdef ENV_HOSTPC
// EMU won't really use image-fb
#else
// the buf-size is capable of 3840x2160 rgb planar
// the 1st set is for 4096*4096
#endif

#if 0
static bool enable_debug = false;
static bool enable_cmdq = false;
#endif

/********************************************************************
 *   APIs to replace bmtest's standard APIs
 ********************************************************************/
extern int cli_readline(char *prompt, char *cmdbuf);

#undef uartlog
#define uartlog(fmt, args...) \
{ \
    printf(fmt, ##args); \
    fflush(stdout); \
}


int isp_intr_handler(int irqn, void *priv)
{
    pattern_ctx *pctx = (pattern_ctx *)priv;

    if (pctx->ptn->intr_op)
        pctx->ptn->intr_op(pctx);
    else
        uartlog("%s: irq(%d)\n", __func__, irqn);

    return 0;
}

int do_nothing(int argc, char **argv)
{
    uartlog("%s argc: %d\n", __func__, argc);
    return 0;
}


int load_pattern(int argc, char **argv)
{

    int i, len, ret = 0;
    char cmdbuf[_CBSIZE];
    pattern_ctx *pctx;
    uint64_t mem_base = DRAM_PHY_BASE;

    #if 1
    uartlog("using malloc heap memory\n");
    mem_base = (uint64_t)malloc(1024*1024);
    #endif

    uartlog("dram base %lx\n", mem_base);

    list_pattern();
    uartlog("input test case num:\n");

    len = cli_readline("#", cmdbuf);
    if (len <= 0) {
        uartlog("error [%d]: invalid input %d\n", __LINE__, len);
        goto _EXIT_MALLOC;
    }

    i = _natoi(cmdbuf, _CBSIZE);
    pctx = create_pattern_ctx(i);
    if (pctx == NULL) {
        goto _EXIT_MALLOC;
    }
    uartlog("testcase: %d. %s\n", i, pctx->ptn->name);

    pctx->phys_regs = isp_get_phys_reg_bases();
    // hw's start phy-addr need to be 32byte-align
    pctx->mem_base = (mem_base + 31) & ~31;

    if (pctx->ptn->intr_op) {
        uartlog("request isp irq : %d\n", ISP_INTR_NUM);
        request_irq(ISP_INTR_NUM, isp_intr_handler, 0, "isp_intr", pctx);
    }
    else uartlog("current case intr_op callback is null\n");

    ret = pctx->ptn->config_op(pctx);

    if (pctx->ptn->start_op)
	pctx->ptn->start_op(pctx);

    uartlog("%s isp streaming, press enter to stop\n", __func__);
    cli_readline("...", cmdbuf);

    if (pctx->ptn->stop_op)
	pctx->ptn->stop_op(pctx);

    if (pctx->ptn->verify_op)
        pctx->ptn->verify_op(pctx);

    destroy_pattern_ctx(pctx);

_EXIT_MALLOC:
    uartlog("free: %lx\n", mem_base);
    if (mem_base) {
        free((void *)mem_base);
        pctx->mem_base = 0;
    }

    return ret;
}


int verify_sensor(int argc, char **argv)
{
    int i, len, ret = 0;
    char cmdbuf[_CBSIZE];
    pattern_ctx *pctx;
    uint64_t mem_base = DRAM_PHY_BASE;

    pinmux_config(PINMUX_CAM0);
    pinmux_config(PINMUX_I2C0);
    pinmux_config(PINMUX_I2C1);
    pinmux_config(PINMUX_I2C2);
    pinmux_config(PINMUX_I2C3);

    #if 1
    uartlog("using malloc heap memory\n");
    mem_base = (uint64_t)malloc(1024*1024);
    #endif

    uartlog("dram base %lx\n", mem_base);

    list_sensor();
    uartlog("input test case num:\n");

    len = cli_readline("#", cmdbuf);
    if (len <= 0) {
        uartlog("error [%d]: invalid input %d\n", __LINE__, len);
        goto _EXIT_MALLOC;
    }

    i = _natoi(cmdbuf, _CBSIZE);
    pctx = create_sensor_ctx(i);
    if (pctx == NULL) {
        goto _EXIT_MALLOC;
    }
    uartlog("testcase: %d. %s\n", i, pctx->ptn->name);

    pctx->vreg_bases = isp_get_phys_reg_bases();
    pctx->vreg_bases_pa = isp_get_phys_reg_bases();
    pctx->phys_regs = isp_get_phys_reg_bases();
    pctx->mem_base = mem_base;

    if (pctx->ptn->intr_op) {
        uartlog("request cif irq : %d\n", CSIMAC0_INTR_NUM);
        request_irq(CSIMAC0_INTR_NUM, isp_intr_handler, 0, "csi0_intr", pctx);
        uartlog("request cif irq : %d\n", CSIMAC1_INTR_NUM);
        request_irq(CSIMAC1_INTR_NUM, isp_intr_handler, 0, "csi1_intr", pctx);
    }
    else uartlog("current case intr_op callback is null\n");

    ret = pctx->ptn->config_op(pctx);

    pctx->ptn->start_op(pctx);

    uartlog("%s steaming, press enter to stop\n", __func__);
    cli_readline("...", cmdbuf);

    pctx->ptn->stop_op(pctx);

    if (pctx->ptn->verify_op)
        pctx->ptn->verify_op(pctx);

    destroy_sensor_ctx(pctx);

_EXIT_MALLOC:
    uartlog("free: %lx\n", mem_base);
    if (mem_base) {
        free((void *)mem_base);
        pctx->mem_base = 0;
    }

    return ret;
}


static struct cmd_entry test_cmd_list[] __attribute__ ((unused)) = {
	{"\n[ISP ========================== ]", do_nothing, 0, ""},
	{"lp", load_pattern, 0, "\t[Load ISP pattern with input test case #]"},
	{"vs", verify_sensor, 0, "\t[Verify sensor]"},
	{"test", do_nothing, 0, "\t[test cmd]"},
	{"\n[CMDQ TBD ===================== ]", do_nothing, 0, ""},
	{NULL, NULL, 0, NULL}
};

int testcase_isp(void)
{
    unsigned ret = 0;
    unsigned i = 0;
    struct isp_param parm, *pparm = &parm;

    memset((void *)pparm, 0, sizeof(*pparm));

    uartlog("starting isp test case ...\n");
    isp_set_base_addr((void*)0x0a000000);
    //vip_isp_clk_reset();
    for(i = 0;i < ARRAY_SIZE(test_cmd_list) - 1;i++) {
        command_add(&test_cmd_list[i]);
    }

#ifndef VIP_INC_ISP
    do {
        ret = cli_simple_loop();
    } while (0);
#endif

    return ret;
}

#ifndef VIP_INC_ISP
#ifndef BUILD_TEST_CASE_all
#ifdef ENV_HOSTPC
int main(int argc, char *argv[])
#else
int testcase_main(void)
#endif
{
    int ret;

    ret = testcase_isp();

    uartlog("testcase isp -- %s\n", (ret < 0) ? "failed" : "passed");

    return ret;
}
#endif	// BUILD_TEST_CASE_all
#endif	// VIP_INC_ISP

#ifdef ENV_HOSTPC
#else
module_testcase("1", testcase_isp);
#endif
