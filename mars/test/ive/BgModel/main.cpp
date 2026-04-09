

#include <stdio.h>
#include "BgModel.h"
#include <stdlib.h>
#ifdef __unix
#include <unistd.h>
#else
#include "sample_getopt.h"
#endif

int _main( int argc, char* const* argv )
{
	CVI_S32 s32FinalResult = CVI_SUCCESS;
	CVI_S32 s32Result;

	int show = 0, compare = 1;
	int c;
	opterr = 0;
	while ((c = getopt (argc, argv, "s:c:")) != -1)
	{
    	switch (c)
		{
		case 's':
			show = atoi(optarg);
			break;
		case 'c':
    	    compare = atoi(optarg);
    	    break;
		default:
			abort ();
		}
	}
	printf("%s show BgModel sample\n",argv[0]);

	printf("BgModelSample1 start...");
	s32Result = BgModelSample1(show, compare);
	printf("-->end!\n");
	if (s32Result != CVI_SUCCESS) {
		printf("BgModelSample1 failed, ret = %d\n", s32Result);
		s32FinalResult = s32Result;
	}

	if (!compare)  // can't do comparing for sample2
	{
		printf("BgModelSample2 start...");
		s32Result = BgModelSample2(show, compare);
		printf("-->end!\n");
		if (s32Result != CVI_SUCCESS) {
			printf("BgModelSample2 failed, ret = %d\n", s32Result);
			s32FinalResult = s32Result;
		}
	}

	printf("BgModel sample done\n");

	//printf("\npress any key to exit...\n");
	//getchar();
	return s32FinalResult;
}


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


//#include "vip_common.h"
//#include "../vip/reg.h"
//#include "inc/ive_reg.h"
//#include "ive_drv.h"

#include "patterns.h"
#include "cvi_ive.h"
#include "cvi_mpi_ive.h"
#include "module_testcase.h"
int _main( int argc, const char** argv );
extern "C" {
    void * __dso_handle = 0 ;
}
//#include "sensorlist.h"

static pattern_ctx m_ptn_ctx ;

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


int ive_intr_handler(int irqn, void *priv)
{
    pattern_ctx *pctx = (pattern_ctx *)priv;

    if (pctx->pptn->intr_op)
        pctx->pptn->intr_op(pctx);
    else
        uartlog("%s: irq(%d)\n", __func__, irqn);

    return 0;
}

int do_nothing(int argc, char **argv)
{
    uartlog("%s argc: %d\n", __func__, argc);
    return 0;
}

static pattern_ctx* create_pattern_ctx(int (*main)(int argc, char const** argv ))
{
    uartlog("%s ............\n", __func__);

    m_ptn_ctx.phys_regs = NULL;
    m_ptn_ctx.mem_base = DRAM_PHY_BASE;
    m_ptn_ctx.vreg_bases = NULL;
    m_ptn_ctx.vreg_bases_pa = NULL;
    m_ptn_ctx.alloc_size = 0;
    m_ptn_ctx.alloc_cnt = 0;

    m_ptn_ctx.is_offline_preraw = 0;
    m_ptn_ctx.is_offline_postraw = 0;
    m_ptn_ctx.is_offline_scaler = 1;
    m_ptn_ctx.is_stop = 1;
    m_ptn_ctx.is_pre0_frame_done = 0;
    m_ptn_ctx.is_pre1_frame_done = 0;
    m_ptn_ctx.is_post_frame_done = 1;
    memset((void *)&m_ptn_ctx.rc, 0, sizeof(m_ptn_ctx.rc));

    m_ptn_ctx.ptn.main = main;
    //m_ptn_ctx.ptn = &pattern_db[case_num];

    return &m_ptn_ctx;
}

int load_pattern(int (*main)(int argc, char const** argv ))
{

    //int i = 0;
    int ret = 0;
    pattern_ctx *pctx;
    const char* argv[] = {"test"};
//#define DRAM_PHY_BASE (0x80000000)
    uint64_t mem_base = DRAM_PHY_BASE;

    #if 0
    uartlog("using malloc heap memory\n");
    mem_base = (uint64_t)malloc(1024*1024);
    #endif

    uartlog("dram base %lx\n", mem_base);

    uartlog("input test case num:\n");

    pctx = create_pattern_ctx(main);
    if (pctx == NULL) {
        goto _EXIT_MALLOC;
    }
    //uartlog("testcase: %d. %s\n", i, pctx->ptn.name);

    pctx->phys_regs = ive_get_phys_reg_bases();
    // hw's start phy-addr need to be 32byte-align
    pctx->mem_base = (mem_base + 31) & ~31;

    setDmaBase(pctx->mem_base);

    //if (pctx->ptn->intr_op) {
    //    uartlog("request ive irq : %d\n", IVE_INTR_NUM);
    //    request_irq(IVE_INTR_NUM, ive_intr_handler, 0, "ive_intr", pctx);
    //}
    //else uartlog("current case intr_op callback is null\n");

    ret = pctx->ptn.main(0, argv);

#if 0
    ret = pctx->ptn->config_op(pctx);

    if (pctx->ptn->start_op)
        pctx->ptn->start_op(pctx);

    uartlog("%s ive streaming, press enter to stop\n", __func__);
    cli_readline("...", cmdbuf);

    if (pctx->ptn->stop_op)
        pctx->ptn->stop_op(pctx);

    if (pctx->ptn->verify_op)
        pctx->ptn->verify_op(pctx);
#endif
    //destroy_pattern_ctx(pctx);

_EXIT_MALLOC:
    uartlog("free: %lx\n", mem_base);
    if (mem_base) {
        free((void *)mem_base);
        pctx->mem_base = 0;
    }

    return ret;
}


int testcase_ive(void)
{
    unsigned ret = 0;

    uartlog("starting ive test case ...\n");
    setDmaBase(uintptr_t(0x80000000));
    ive_set_base_addr((void*)0xa0a0000);
#if defined BOARD_FPGA  || defined BOARD_ASIC
    //timer_meter_start();
#endif
    //vip_ive_clk_reset();

    ret = load_pattern(_main);

#ifndef VIP_INC_IVE
    do {
        //ret = cli_simple_loop();
    } while (0);
#endif

    return ret;
}

#ifndef VIP_INC_IVE
#ifndef BUILD_TEST_CASE_all
#ifdef ENV_HOSTPC
int _main(int argc, char *argv[])
#else
    //#define _open(p, f, m) open(p, f, m)
extern "C" {
    int testcase_main(void);
    int _open(const char * pathname, int flags, mode_t mode);
}
int _open(const char * pathname, int flags, mode_t mode) {
    printf("fail to open it\n");
    while(1);
    return 0;
}
extern "C" void __cxa_pure_virtual();
extern "C" void __cxa_pure_virtual() {
    // Add custom handler here
    while(1);
}
int testcase_main(void)
#endif
{
    int ret;

    ret = testcase_ive();

    uartlog("testcase ive -- %s\n", (ret < 0) ? "failed" : "passed");

    return ret;
}
#endif	// BUILD_TEST_CASE_all
#endif	// VIP_INC_IVE

#ifdef ENV_HOSTPC
#else
module_testcase("1", testcase_ive);
#endif
