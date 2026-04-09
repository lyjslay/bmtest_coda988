#ifndef __CVI_ENC_INTERNAL_H__
#define __CVI_ENC_INTERNAL_H__
#include "main_helper.h"
#include "vpuapi.h"
#include "vdi.h"
#include "testargv.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define VC_FAB_MMU_CTRL			0x40
#define H265_MAP_REG_OFFSET		0x200
#define NUM_OF_BS_BUF 1
#define CACHE_ENCODE_HEADER 1

//is bitstreambuff is not enough, drop frame
//or alloc ion to store it
#define DROP_FRAME 1


/*
typedef struct _stTestEncoder_ {
	EncHandle handle;
	EncOpenParam encOP;
	EncParam encParam;
	EncInitialInfo initialInfo;
	EncOutputInfo outputInfo;
	SecAxiUse secAxiUse;
	vpu_buffer_t vbStream[NUM_OF_BS_BUF];
	Uint32 bsBufferCount;
	Uint32 bsQueueIndex;
	vpu_buffer_t vbReport;
	vpu_buffer_t vbReconFrameBuf[MAX_REG_FRAME];
	vpu_buffer_t vbSourceFrameBuf[MAX_REG_FRAME];
	FrameBuffer fbSrc[ENC_SRC_BUF_NUM];
#ifdef SUPPORT_DIRECT_YUV
	FrameBuffer fbSrcFixed;
#endif
	FrameBufferAllocInfo fbAllocInfo;
	int ret;
	EncHeaderParam encHeaderParam;
#if CACHE_ENCODE_HEADER
	int bEncHeader;
	EncHeaderParam headerBackup[8];
#endif
	int srcFrameIdx;
	Uint32 frameIdx;
	int srcFrameWidth, srcFrameHeight, srcFrameStride;
	int framebufStride, framebufWidth, framebufHeight;
	int suc;
	int regFrameBufCount;
	int coreIdx;
	TestEncConfig encConfig;
	Int32 productId;
	Comparator comparatorBitStream;
	BitstreamReader bsReader;
	FrameBuffer fbRecon[MAX_REG_FRAME];
	vpu_buffer_t vbRoi[MAX_REG_FRAME];
	vpu_buffer_t vbRoiBin[MAX_REG_FRAME];
	// for encode host rbsp header
	vpu_buffer_t vbPrefixSeiNal[MAX_REG_FRAME];
	vpu_buffer_t vbSuffixSeiNal[MAX_REG_FRAME];
	vpu_buffer_t vbHrdRbsp;
	vpu_buffer_t vbVuiRbsp;
#ifdef TEST_ENCODE_CUSTOM_HEADER
	vpu_buffer_t vbSeiNal[MAX_REG_FRAME];
	vpu_buffer_t vbCustomHeader;
#endif

	Uint8 roiMapBuf[MAX_CTU_NUM];
	void *yuvFeeder;
	YuvInfo yuvFeederInfo;
	Uint32 interruptTimeout;
#ifdef TEST_ENCODE_CUSTOM_HEADER
	hrd_t hrd;
#endif
	feederYuvaddr yuvAddr;

	stStreamPack streamPack;
	Uint8 bIsEncoderInited;

	// for handle BIT_BUF_FULL interrupt
	Uint8 *pu8PreStream;
	Uint32 u32PreStreamLen;
	Uint32 u32BsRotateOffset;
#ifdef DROP_FRAME
	Uint8 bDrop; // Drop bitstream
	int originBitrate; // srore rc origin bitrate
#endif

	cviVEncStreamInfo tStreamInfo;
	Uint64 u64Pts;
} stTestEncoder;
*/


// cvi_enc_internal.c
int cviSetupPicConfig(EncHandle handle, EncOpenParam *pEncOP, EncParam *pEncParam);
int cviInitAddrRemap(TestEncConfig *pEncCfg,
					EncOpenParam *pEncOP, EncHandle handle,
					vpu_buffer_t* pVbReconFrameMem, FrameBuffer* pFbRecon,
					TestArgV_t *pTestArgV, int regFrameBufCount);

// vpuapi.c
RetCode cviConfigEncParam(CodecInst *pCodec, EncOpenParam *param);

// coda9.c / wave4.c
int cviInitAddrRemapFb(CodecInst *pCodecInst);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif