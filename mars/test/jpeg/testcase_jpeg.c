#include "boot_test.h"
#include "uart.h"
#include "cli.h"
#include "jpuconfig.h"
#include "regdefine.h"
#include "jpulog.h"
#include "jpurun.h"
#include "jpuhelper.h"
#include "cache.h"
#include "mmio.h"
#include "jdi_osal.h"

typedef struct _slt_test_cfg_
{
    int mode; // 1 = dec, 2 = enc
    int comparatorFlag;
    int packedFormat;
    int chromaInterleave;
    int usePartialMode;
    int partialBufNum;
    int rotAngle;
    int mirDir;
    int outNum;
    int roiEnable;
    int roiWidth;
    int roiHeight;
    int mjpgChromaFormat;
    int roiOffsetX;
    int roiOffsetY;
    int iHorScaleMode;
    int iVerScaleMode;
    int fileSize;
} slt_test_cfg;

enum _mode_
{
    MODE_DEC = 1,
    MODE_ENC,
};

enum _packFormat_
{
    PACK_PLANAR = 0,
    PACK_YUYV,
    PACK_UYVY,
    PACK_YVYU,
    PACK_VYUY,
    PACK_YUV_444_PACKED,
} packFormat;

enum _partialMode_
{
    PARTIAL_MODE_DISABLE = 0,
    PARTIAL_MODE_ENABLE,
};

enum _rorateAngle_
{
    ROTATE_0 = 0,
    ROTATE_90 = 90,
    ROTATE_180 = 180,
    ROTATE_270 = 270,
};

enum _mirror_dir_
{
    MIRROR_NO = 0,
    MIRROR_VERTICAL,
    MIRROR_HORIZONTAL,
    MIRROR_BOTH,
};

#if defined(BOARD_ASIC)
static void configPLL(void);
#endif

int bitStreamFileSize = 0;
#define JPEG_CVD_TEST
#ifdef JPEG_CVD_TEST
#define CONFIG_PARAM_ADDR 0x13eff0000
#ifdef JPEG_CVD_TEST
void test_jpeg_exit(void);
#endif
int *pConfigParam = (int *)CONFIG_PARAM_ADDR;
#endif

#ifdef SLT_TEST
int jpeg_dec_slt_test(slt_test_cfg *cfg)
#else
int jpeg_dec_test()
#endif
{
  int ret = 0;
  DecConfigParam  decConfig;

  memset(&decConfig, 0x00, sizeof(DecConfigParam));

  decConfig.bitstreamFileName = NULL;
  decConfig.StreamEndian = JPU_STREAM_ENDIAN;
  decConfig.FrameEndian = JPU_FRAME_ENDIAN;
  decConfig.yuvFileName = NULL;
#if defined(SLT_TEST)
  decConfig.comparatorFlag   = cfg->comparatorFlag;
  decConfig.packedFormat     = cfg->packedFormat;
  decConfig.chromaInterleave = cfg->chromaInterleave;
  decConfig.usePartialMode   = cfg->usePartialMode;
  decConfig.partialBufNum    = cfg->partialBufNum;
  decConfig.rotAngle         = cfg->rotAngle;
  decConfig.mirDir           = cfg->mirDir;
  decConfig.outNum           = cfg->outNum;
  decConfig.roiEnable        = cfg->roiEnable;
  decConfig.roiWidth         = cfg->roiWidth;
  decConfig.roiHeight        = cfg->roiHeight;
  decConfig.roiOffsetX       = cfg->roiOffsetX;
  decConfig.roiOffsetY       = cfg->roiOffsetY;
  decConfig.iHorScaleMode    = cfg->iHorScaleMode;
  decConfig.iVerScaleMode    = cfg->iVerScaleMode;
  bitStreamFileSize          = cfg->fileSize;
#elif defined(JPEG_CVD_TEST)
  decConfig.comparatorFlag   = *pConfigParam++;
  decConfig.packedFormat     = *pConfigParam++;
  decConfig.chromaInterleave = *pConfigParam++;
  decConfig.usePartialMode   = *pConfigParam++;
  decConfig.partialBufNum    = *pConfigParam++;
  decConfig.rotAngle         = *pConfigParam++;
  decConfig.mirDir           = *pConfigParam++;
  decConfig.outNum           = *pConfigParam++;
  decConfig.roiEnable        = *pConfigParam++;
  decConfig.roiWidth         = *pConfigParam++;
  decConfig.roiHeight        = *pConfigParam++;
  decConfig.roiOffsetX       = *pConfigParam++;
  decConfig.roiOffsetY       = *pConfigParam++;
  bitStreamFileSize          = *pConfigParam++;
#else
  decConfig.comparatorFlag = 1;

	bitStreamFileSize = 0x23431;

  decConfig.roiEnable = 0;

  decConfig.packedFormat = 0; // Packed stream format output [0](PLANAR) [1](YUYV) [2](UYVY) [3](YVYU) [4](VYUY) [5](YUV_444 PACKED)

  decConfig.chromaInterleave = 0;

  if(!decConfig.roiEnable && !decConfig.packedFormat)
  {
    decConfig.usePartialMode = 0;    // 0 disable 1 enable

    if(decConfig.usePartialMode)
    {
      decConfig.partialBufNum = 4; //  Num of Frame Buffer[ 2 ~ 4 ]
    }
  }

  decConfig.outNum = 1;        // Number of Images that you want to decode(0: decode continue, -1: loop)
#endif
  if(!decConfig.usePartialMode)
  {
    if (decConfig.rotAngle != 0 && decConfig.rotAngle != 90 && decConfig.rotAngle != 180 && decConfig.rotAngle != 270) {
      JLOG(ERR, "Invalid rotation angle.\n");
      return 1;
    }

    if (decConfig.mirDir != 0 && decConfig.mirDir != 1 && decConfig.mirDir != 2 && decConfig.mirDir != 3) {
      JLOG(ERR, "Invalid mirror direction.\n");
      return 1;
    }
    if(decConfig.rotAngle != 0 || decConfig.mirDir != 0)
      decConfig.useRot = 1;
  }
  ret = DecodeTest(&decConfig);
  return 1-ret;
}

#ifdef SLT_TEST
int jpeg_enc_slt_test(slt_test_cfg *cfg)
#else
int jpeg_enc_test()
#endif
{
    int ret = 0;
    EncConfigParam  encConfig;

    memset(&encConfig, 0x00, sizeof(EncConfigParam));

    encConfig.StreamEndian = JPU_STREAM_ENDIAN;
    encConfig.FrameEndian = JPU_FRAME_ENDIAN;

    encConfig.bEnStuffByte = JPU_STUFFING_BYTE_FF;

    encConfig.yuvFileName = NULL;
    encConfig.bitstreamFileName = NULL;
    encConfig.huffFileName = NULL;
    encConfig.qMatFileName = NULL;
#if defined(SLT_TEST)
    encConfig.compareJpg       = cfg->comparatorFlag;
    encConfig.packedFormat     = cfg->packedFormat;
    encConfig.chromaInterleave = cfg->chromaInterleave;
    encConfig.usePartialMode   = cfg->usePartialMode;
    encConfig.partialBufNum    = cfg->partialBufNum;
    encConfig.rotAngle         = cfg->rotAngle;
    encConfig.mirDir           = cfg->mirDir;
    encConfig.outNum           = cfg->outNum;
    encConfig.roiEnable        = cfg->roiEnable;
    encConfig.picWidth         = cfg->roiWidth;
    encConfig.picHeight        = cfg->roiHeight;
	BM_DBG_TRACE("picWidth = %d, picHeight = %d\n", encConfig.picWidth, encConfig.picHeight);
    encConfig.mjpgChromaFormat = cfg->mjpgChromaFormat;
#elif defined(JPEG_CVD_TEST)
    encConfig.compareJpg = *pConfigParam++;
    encConfig.packedFormat = *pConfigParam++;
    encConfig.chromaInterleave = *pConfigParam++;
    encConfig.usePartialMode = *pConfigParam++;
    encConfig.partialBufNum = *pConfigParam++;
    encConfig.rotAngle = *pConfigParam++;
    encConfig.mirDir = *pConfigParam++;
    encConfig.outNum = *pConfigParam++;
    encConfig.picWidth = *pConfigParam++;
    encConfig.picHeight = *pConfigParam++;
    encConfig.mjpgChromaFormat = *pConfigParam++;
#else
    encConfig.picWidth = 1920;
    encConfig.picHeight = 1088;
    encConfig.chromaInterleave = JPU_CHROMA_INTERLEAVE;

    encConfig.packedFormat = 0; //Frame Format [0](PLANAR) [1](YUYV) [2](UYVY) [3](YVYU) [4](VYUY) [5](YUV_444 PACKED)
    encConfig.mjpgChromaFormat = 1; // Source Chroma Format 0 (4:2:0) / 1 (4:2:2) / 2 (2:2:4 4:2:2 rotated) / 3 (4:4:4) / 4 (4:0:0)

    encConfig.usePartialMode = 0;	//(0: OFF 1: ON)
    encConfig.outNum = 1;
    encConfig.compareJpg = 1;
    if(encConfig.usePartialMode)
    {
        encConfig.partialBufNum = 4;
    }

#endif
    if (encConfig.usePartialMode == 0)
    {
        if (encConfig.rotAngle != 0 && encConfig.rotAngle != 90 && encConfig.rotAngle != 180 && encConfig.rotAngle != 270) {
            JLOG(ERR, "Invalid rotation angle.\n");
            return 1;
        }

        if (encConfig.mirDir != 0 && encConfig.mirDir != 1 && encConfig.mirDir != 2 && encConfig.mirDir != 3) {
            BM_DBG_ERR("Invalid mirror direction.\n");
            return 1;
        }
        if( encConfig.rotAngle != 0 || encConfig.mirDir != 0 )
            encConfig.useRot = 1;
    }

    ret = EncodeTest(&encConfig);

    return 1-ret;
}

void write_reg(unsigned long addr, unsigned int val)
{
  unsigned int ret;
  ret = *(volatile unsigned int *)addr;

  while (ret != val) {
    *(volatile unsigned int *)addr = val;
    flush_dcache_range((long unsigned int)addr, sizeof(unsigned int));
    INV_DCACHE_RANGE((long unsigned int)addr, sizeof(unsigned int));
    ret = *(volatile unsigned int *)addr;
  }
}

#ifdef SLT_TEST
int irq_handler(int irqn, void *priv)
{
  BM_DBG_FLOW("irq interrupt\n");
  return 1;
}

int testcase_jpeg(void)
{
    slt_test_cfg allCfgs[] =
    {
        //        comp
        {MODE_ENC, 1, PACK_PLANAR, 0, PARTIAL_MODE_DISABLE, 2, ROTATE_0, MIRROR_NO, 1,    0,  3840,  2160,    PACK_PLANAR, 0,  0,  0, 0, 40681 },
        //{MODE_DEC, 1, PACK_PLANAR, 0, PARTIAL_MODE_DISABLE, 4, ROTATE_0, MIRROR_NO, 1,    0,   300,   300,    0,           50, 50, 0, 0, 0x23431},
    };

    int idx, ret = 0, all = 0;

#if defined(BOARD_ASIC)
	configPLL();
#endif

	mmio_write_32(TOP_DDR_ADDR_MODE_REG, (1 << DAMR_REG_VD_REMAP_ADDR_39_32_OFFSET));
	mmio_write_32(VC_REG_BASE, (mmio_read_32(VC_REG_BASE) | (0x1f)));

#ifdef SUPPORT_INTERRUPT
    request_irq(JPEG_CODEC_INTR_NUM, irq_handler_jpeg_codec, 0, "jpeg int", NULL);
#endif

	BM_DBG_TRACE("irq num = %d\n", JPEG_INTRPT_REQ);

    for (idx = 0; idx < sizeof(allCfgs) / sizeof(slt_test_cfg); idx++)
    {
        if (allCfgs[idx].mode == MODE_DEC)
            ret = jpeg_dec_slt_test(&allCfgs[idx]);
        else
            ret = jpeg_enc_slt_test(&allCfgs[idx]);
        if (ret)
        {
            JLOG(NONE, "case %d, error\n", idx);
            all = 1;
        }
        else
            JLOG(NONE, "case %d, success\n", idx);
    }

    JLOG(NONE, "testcase jpeg %s\n", all ? "failed":"passed");
    return all;
}

#if defined(BOARD_ASIC)
static void configPLL(void)
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
#endif

module_testcase("1", testcase_jpeg);

#else

int testcase_jpeg(void)
{
    int ret = 0;
    ret = 0;
    uartlog("%s\n", __func__);

#ifdef JPEG_CVD_TEST
    int *pRetMem = (int *)0x13eff1000;
    int test_opt;
    test_opt = *pConfigParam++;
    if(test_opt == 2)
    {
        ret = jpeg_enc_test();
    }
    else
    {
        ret = jpeg_dec_test();
    }
    *pRetMem = ret;
#else
    ret = jpeg_dec_test();

    ret = (ret<<16) | jpeg_enc_test();
#endif

    uartlog("testcase jpeg %s\n", ret?"failed":"passed");

#ifdef JPEG_CVD_TEST
    test_jpeg_exit();
#endif

    return ret;
}

#ifdef JPEG_CVD_TEST
__attribute__((__noinline__,__noclone__)) void test_jpeg_exit(void)
{
  while(1);
}
#endif

#endif
