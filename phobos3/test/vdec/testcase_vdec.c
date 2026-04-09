#include "boot_test.h"
#include "cache.h"
#include "mmio.h"
#include "main_dec_cvitest.h"
#include "testcase_vdec.h"
#include "cvi_h264_cfg.h"

#ifdef TEST_H264_DEC
static int testcase_h264_vdec(void);
#endif
#ifdef TEST_H265_DEC
static int testcase_h265_vdec(void);
#endif

#if defined(BOARD_ASIC)
#define PLL_MODE_NORMAL	1
#define PLL_MODE_FAST	2
#define PLL_MODE_CURR	PLL_MODE_NORMAL

static void configPLL(int mode);
static void configPLL_Normal(void);
static void configPLL_Fast(void);
#endif

#if defined(TEST_H264_DEC) || defined(TEST_H265_DEC)
static void show_param(int argc, char**argv);
#endif

void inv_dcache_range(unsigned long start, unsigned long size)
{
	invalidate_dcache_range(start, size);
}

#ifdef BUILD_TEST_CASE_all
int testcase_vdec(void)
#else
int testcase_main(void)
#endif
{
    int status = 0;

    invalidate_dcache_all();

#if defined(BOARD_ASIC)
	configPLL(PLL_MODE_CURR);
#endif

#ifdef TEST_H264_DEC
    if (testcase_h264_vdec())
    {
        uartlog("%s = %d, h264 failed\n", __func__, __LINE__);
        status = -1;
    }
#endif

#ifdef TEST_H265_DEC
    if (testcase_h265_vdec())
    {
        uartlog("%s = %d, h265 failed\n", __func__, __LINE__);
        status = -1;
    }
#endif

    return status;
}

#if defined(BOARD_ASIC)
static void configPLL(int mode)
{
	if (mode == PLL_MODE_FAST)
		configPLL_Fast();
	else
		configPLL_Normal();
}

static void configPLL_Normal(void)
{
	unsigned int val;

	uartlog("PLL mode = NORMAL\n");

	// 0x03002840   = 0x3E   //enable synthesizer clock enable
	mmio_write_32(0x03002840, 0x3E);

	// 0x03002854   = 385505882   //set apll synthesizer  104.4480001 M
	mmio_write_32(0x03002854, 385505882);

	//0x03002850 ^= 0x00000001 // bit 0 toggle
	val = mmio_read_32(0x03002850);
	mmio_write_32(0x03002850, (val ^ 0x1));

	// 0x0300280C   = 0x01108201 // set apll *8/2
	mmio_write_32(0x0300280C, 0x01108201);

	// 0x03002800  &= ~0x00000010 //clear apll pd  , apll = 417.792
	val = mmio_read_32(0x03002800);
	mmio_write_32(0x03002800, (val &= (~0x10)));

	// 0x03002884   = 421827145   //set cam1 synthesizer  95.45454549 M
	mmio_write_32(0x03002884, 421827145);

	// 0x03002880 ^= 0x00000001 // bit 0 toggle
	val = mmio_read_32(0x03002880);
	mmio_write_32(0x03002880, (val ^ 0x1));

	// 0x03002818   = 0x00168000 // set cam1 *11/1
	mmio_write_32(0x03002818, 0x00168000);

	// 0x03002800  &= ~0x00010000 //clear cam1pll pd  , cam1pll = 1050
	val = mmio_read_32(0x03002800);
	mmio_write_32(0x03002800, (val &= (~0x00010000)));

	// #=========measure apll
	// 0x03000140  = 0x00000000
	// 0x03000140  = 0x00000049
	// Read 0x03000140 >>8 =  ~ 2085

	// #=========measure cam1pll
	// 0x03000140  = 0x00000000
	// 0x03000140  = 0x00000079
	// Read 0x03000140 >>8 = ~ 5250
}

static void configPLL_Fast(void)
{
	unsigned int val;

	uartlog("PLL mode = FAST\n");

	// ECO mipi 1.2G/2
	// 0x03002020[31]=0
	val = mmio_read_32(0x03002020);
	val &= (~(1 << 31));
	mmio_write_32(0x03002020, val);

	// div source from div0(fpll) to div1(cam1/mipi)
	// 0x03002020[18]=0
	val = mmio_read_32(0x03002020);
	val &= (~(1 << 18));
	mmio_write_32(0x03002020, val);

	// 0x03002840   = 0x3E   //enable synthesizer clock enable
	mmio_write_32(0x03002840, 0x3E);

	// 0x03002854   = 409600000   //set apll synthesizer  98.304 M
	mmio_write_32(0x03002854, 409600000);

	//0x03002850 ^= 0x00000001 // bit 0 toggle
	val = mmio_read_32(0x03002850);
	mmio_write_32(0x03002850, (val ^ 0x1));

	// 0x0300280C   = 0x01188201 // set apll *12/2
	mmio_write_32(0x0300280C, 0x01188201);

	// 0x03002800  &= ~0x00000010 //clear apll pd  , apll = 589.824
	val = mmio_read_32(0x03002800);
	mmio_write_32(0x03002800, (val &= (~0x10)));

	// 0x03002884   = 429496730   //set cam1 synthesizer  93.74999991 M
	mmio_write_32(0x03002884, 429496730);

	// 0x03002880 ^= 0x00000001 // bit 0 toggle
	val = mmio_read_32(0x03002880);
	mmio_write_32(0x03002880, (val ^ 0x1));

	// 0x03002818   = 0x00208000 // set cam1 *16/1
	mmio_write_32(0x03002818, 0x00208000);

	// 0x03002800  &= ~0x00010000 //clear cam1pll pd  , cam1pll = 1499.999999
	val = mmio_read_32(0x03002800);
	mmio_write_32(0x03002800, (val &= (~0x00010000)));

	// #=========measure apll
	// 0x03000140  = 0x00000000
	// 0x03000140  = 0x00000049
	// Read 0x03000140 >>8 =  ~ 2085

	// #=========measure cam1pll
	// 0x03000140  = 0x00000000
	// 0x03000140  = 0x00000079
	// Read 0x03000140 >>8 = ~ 5250
}
#endif

#ifdef TEST_H264_DEC
static int testcase_h264_vdec(void)
{
    char *argv[] =
    {
        "bm1882_FPGA_vdec",
    };
    int argc;
    int ret = 0;

    uartlog("%s = %d, ==============================\n", __func__, __LINE__);

    argc = sizeof(argv) / sizeof(char*);
    show_param(argc, argv);

#ifdef SUPPORT_INTERRUPT
    request_irq(H264_VDEC_INTR_NUM, irq_handler_h264_vdec, 0, "h264 int", NULL);
#endif

    if ((ret = cvitest_h264_vdec_main(argc, argv)))
    {
        uartlog("failed\n");
    }
    else
    {
        uartlog("passed\n");
    }

    return ret;
}
#endif

#ifdef TEST_H265_DEC
static int testcase_h265_vdec(void)
{
    char *argv[] =
    {
        "bm1882_FPGA_vdec",
    };
    int argc;
    int ret = 0;

    uartlog("%s = %d, ==============================\n", __func__, __LINE__);

    argc = sizeof(argv) / sizeof(char*);
    show_param(argc, argv);

#ifdef SUPPORT_INTERRUPT
    request_irq(H265_VDEC_INTR_NUM, irq_handler_h265_vdec, 0, "h265 int", NULL);
#endif

    if ((ret = cvitest_h265_vdec_main(argc, argv)))
    {
        uartlog("failed\n");
    }
    else
    {
        uartlog("passed\n");
    }

    return ret;
}
#endif

#if defined(TEST_H264_DEC) || defined(TEST_H265_DEC)
static void show_param(int argc, char**argv)
{
    int idx;

    uartlog("argc = %d\n", argc);

    uartlog("argv =");
    for (idx = 0; idx < argc; idx++)
    {
        uartlog(" %s", argv[idx]);
    }
    uartlog("\n");
}
#endif

#ifdef BUILD_TEST_CASE_all
module_testcase("1", testcase_vdec);
#endif
