#include "boot_test.h"
#include "cache.h"
#include "mmio.h"
#include "main_enc_cvitest.h"
#include "testcase_venc.h"

#ifdef TEST_H264_ENC
static int testcase_h264_venc(void);
#endif
#ifdef TEST_H265_ENC
static int testcase_h265_venc(void);
#endif

#if defined(BOARD_ASIC)
#define PLL_MODE_NORMAL	1
#define PLL_MODE_FAST	2
#define PLL_MODE_CURR	PLL_MODE_NORMAL

static void configPLL(int mode);
static void configPLL_Normal(void);
static void configPLL_Fast(void);
#endif

#if defined(TEST_H264_ENC) || defined(TEST_H265_ENC)
static void show_param(int argc, char**argv);
#endif

void inv_dcache_range(unsigned long start, unsigned long size)
{
	invalidate_dcache_range(start, size);
}

int testcase_venc(void)
{
    int status = 0;

    invalidate_dcache_all();

#if defined(BOARD_ASIC)
	configPLL(PLL_MODE_CURR);
#endif

#ifdef TEST_H264_ENC
    if (testcase_h264_venc())
    {
        uartlog("%s = %d, h264 failed\n", __func__, __LINE__);
        status = -1;
    }
#endif

#ifdef TEST_H265_ENC
    if (testcase_h265_venc())
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

#ifdef TEST_H264_ENC
static int testcase_h264_venc(void)
{
    char *argv[] =
    {
        "bm1882_FPGA_venc",
    };
    int argc;
    int ret = 0;

    uartlog("%s = %d, ==============================\n", __func__, __LINE__);

    argc = sizeof(argv) / sizeof(char*);
    show_param(argc, argv);

#ifdef SUPPORT_INTERRUPT
    request_irq(H264_VENC_INTR_NUM, irq_handler_h264_venc, 0, "h264 int", NULL);
#endif

    if ((ret = cvitest_h264_venc_main(argc, argv)))
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

#ifdef TEST_H265_ENC
int testcase_h265_venc(void)
{
    char *argv[] =
    {
        "bm1882_FPGA_venc",
        "-c", "1",
        "-n", "30",
        "--input=enc-1.yuv",
        "--picWidth=3840",
        "--picHeight=2160",
        "--secondary-axi=7",
        "--code-addr=0x10B000000",
        "--code-size=1048576",
        "--src-yuv-addr=0x11B100000",
        "--src-yuv-size=134217728",
        "--vpu-dram-addr=0x113100000",
        "--vpu-dram-size=134217728",
    };
    int argc;
    int ret = 0;

    uartlog("%s = %d, ==============================\n", __func__, __LINE__);

    argc = sizeof(argv) / sizeof(char*);
    show_param(argc, argv);

#ifdef SUPPORT_INTERRUPT
    request_irq(H265_VENC_INTR_NUM, irq_handler_h265_venc, 0, "h265 int", NULL);
#endif

#if PXP_STRESS_TEST
#if 0
    cvi_venc_pxp_main(argc, argv);
#else
    {
        int ret = 0;
        void *handle;

        handle = cvi_venc_pxp_init(argc, argv);
        if (handle == NULL)
        {
            uartlog("Error: cvi_venc_pxp_init\n");
            return -1;
        }

        while( 1 ) {
            ret = cvi_venc_pxp_enc_one_frame(handle);
            if (ret == TE_STA_ENC_BREAK)
                break;
            else if (ret == TE_STA_ENC_CONTINUE)
                continue;
        }

        return 0;
    }
#endif
#else
    if ((ret = cvitest_venc_main(argc, argv)))
    {
        uartlog("failed\n");
    }
    else
    {
        uartlog("passed\n");
    }
#endif

    return ret;
}
#endif

#if defined(TEST_H264_ENC) || defined(TEST_H265_ENC)
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

module_testcase("1", testcase_venc);
