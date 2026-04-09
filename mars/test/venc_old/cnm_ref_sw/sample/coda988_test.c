//--=========================================================================--
//  This file is a part of VPUAPi
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2004 - 2014   CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include <getopt.h>
#include "vpuapifunc.h"
#include "main_helper.h"
#include "cvi_enc_internal.h"
#include "vpuapi/coda9/coda9_regdefine.h"
#include "coda988_test.h"


#define STREAM_BUF_SIZE                 0x700000    // max bitstream size
#define DEFAULT_READ_SIZE               1460        // = MTU(ethernet) - (IP_HEADER+TCP_HEADER)

#define MAX_TEST_FRAME_NUM 5
#define TEST_FRAME_START_ADDR 0x82000000
#define RESERVE_PER_FRAME_SIZE  0x100000
// TODO：每一帧src的地址，需要在cmm脚本中LoadBINARY指令将src提前加载到对应地址
unsigned int reserveSrcFrameAddr[MAX_TEST_FRAME_NUM] = {
    TEST_FRAME_START_ADDR,
    TEST_FRAME_START_ADDR + RESERVE_PER_FRAME_SIZE,
    TEST_FRAME_START_ADDR + RESERVE_PER_FRAME_SIZE * 2,
    TEST_FRAME_START_ADDR + RESERVE_PER_FRAME_SIZE * 3,
    TEST_FRAME_START_ADDR + RESERVE_PER_FRAME_SIZE * 4,
};

static Uint32               sizeInWord;
static Uint16*              pusBitCode;
 // !!! should be set in every case, declared in main_helper.h as extern for yuv_feeder.c
unsigned long    YUV_ADDR           = 0x82008000;
size_t           YUV_TOTAL_BYTES    = 1 * 1024 * 1024; // 1MB

#if 0
static void
Help(
    const char *programName
    )
{
    VLOG(INFO, "------------------------------------------------------------------------------\n");
    VLOG(INFO, "%s(API v%d.%d.%d)\n", programName, API_VERSION_MAJOR, API_VERSION_MINOR, API_VERSION_PATCH);
    VLOG(INFO, "\tAll rights reserved by Chips&Media(C)\n");
    VLOG(INFO, "\tSample program controlling the Chips&Media VPU\n");
    VLOG(INFO, "------------------------------------------------------------------------------\n");
    VLOG(INFO, "%s [option] --input bistream\n", programName);
    VLOG(INFO, "-h                          help\n");
    VLOG(INFO, "-v                          print version information\n");
    VLOG(INFO, "-c                          compare with golden\n");
    VLOG(INFO, "                            0 : no comparison\n");
    VLOG(INFO, "                            1 : compare with golden stream that specified --ref-stream_path option\n");
    VLOG(INFO, "--output                    default path: output.bin\n");
    VLOG(INFO, "--codec                     The index of codec (H.264:0, MP4:3, H263:4), default 0\n");
    VLOG(INFO, "--cfg-path                  encoder configuration file path\n");
    VLOG(INFO, "--yuv-base                  YUV base directory. Default is working directory\n");
    VLOG(INFO, "--ref_stream_path           Golden encoded stream to be compared with encoded stream\n");
    VLOG(INFO, "--enable-ringBuffer         enable ring buffer mode, default linebuffer mode\n");
    VLOG(INFO, "--enable-lineBufInt     use line-buffer interrupt when ringbuffer mode is off, default off\n");
    VLOG(INFO, "--maptype                   CODA980: 0~9, default 0(LINEAR_FRAME_MAP)\n");
    VLOG(INFO, "                            Please refer TiledMapType in vpuapi.h.\n");
    VLOG(INFO, "                            Note: If maptype is set, source YUV must be tiled map format or set --enable-linear2tiled\n");
    VLOG(INFO, "--enable-linear2tiled       If this option is enabled, linear source YUV will be converted tiled format depending on --maptyp\n");
    VLOG(INFO, "--enable-cbcrInterleave     enable cbcrInterleave(NV12), default off\n");
    VLOG(INFO, "--enable-nv21               enable NV21, default off\n");
    VLOG(INFO, "--secondary-axi             0~63: bit oring values, Please refer programmer's guide or datasheet\n");
    VLOG(INFO, "--rotate                    90, 180, 270\n");
    VLOG(INFO, "--mirror                    0: none, 1: vertical, 2: horizontal, 3: both\n");
    VLOG(INFO, "--enable-mvc                0: AVC main profile, 1: AVC Stereo profile\n");

	VLOG(INFO, "--enable-svc                0: AVC main profile, 1: SVC \n");
}
#endif
static void RcChangeSetParameter(EncHandle handle, TestEncConfig encConfig, TestArgV_t *pTestArgV, EncOpenParam *encOP, EncParam *encParam, Uint32 frameIdx, Uint32 fieldDone, ENC_CFG cfgParam, vpu_buffer_t vbRoi)
{
    BOOL isIdr              = FALSE;
    BOOL isIPic             = FALSE;
    Uint32 minSkipNum       = 0;
    Uint32 idx              = 0;
    Int32  iPicCnt          = 0;

    if (encOP->EncStdParam.avcParam.fieldFlag) {
        encParam->fieldRun = TRUE;
    }

    if (0 == frameIdx) {
        isIdr = TRUE;
        isIPic = TRUE;
    } else {
        Int32   gopNum = encOP->EncStdParam.avcParam.fieldFlag ? (2 * encOP->gopSize) : encOP->gopSize;
        Int32   picIdx = encOP->EncStdParam.avcParam.fieldFlag ? (2 * frameIdx + fieldDone) : frameIdx;
        if ((encOP->idrInterval>0) && (gopNum> 0)) {
            if ((picIdx >= gopNum)) {
                picIdx = 0;
            }
            if(picIdx == 0) {
                isIPic = TRUE;
                iPicCnt++;
            }
        }
    }

    if (encOP->bitstreamFormat == STD_MPEG4 ) {
        if (isIPic == TRUE && encOP->idrInterval > 0) {
            if ((iPicCnt% encOP->idrInterval) == 0) {
                isIdr = TRUE;
            }
        }
    }
    encParam->forceIPicture = fieldDone ? FALSE : isIdr;

    encParam->skipPicture = 0;

    minSkipNum = encOP->EncStdParam.avcParam.mvcExtension ? 1 : 0;

    for (idx=0; idx<MAX_PIC_SKIP_NUM; idx++) {
        Uint32   numPicSkip = cfgParam.field_flag ? encConfig.skipPicNums[idx]/2 : encConfig.skipPicNums[idx];
        if (numPicSkip > minSkipNum && numPicSkip == (Uint32)frameIdx) {
            if (cfgParam.field_flag == FALSE) {
                encParam->skipPicture = TRUE;
            } else {
                if (encConfig.skipPicNums[idx]%2)
                    encParam->skipPicture = fieldDone;
                else
                    encParam->skipPicture = !(fieldDone);
                /* check next skip field */
                if ((idx+1) < MAX_PIC_SKIP_NUM) {
                    numPicSkip = encConfig.skipPicNums[idx+1]/2;
                    if (numPicSkip == (Uint32)frameIdx) encParam->skipPicture = TRUE;
                }
            }
            break;
        }
    }

    // TODO: Need to do refactoring
    if (encOP->bitstreamFormat == STD_AVC && frameIdx > 0 && frameIdx == encOP->paramChange.ChangeFrameNum)
    {
        if (strcmp(encOP->paramChange.pchChangeCfgFileName, "") != 0)
        {
            EncInfo*    pEncInfo;
            CodecInst*  pCodecInst;

            pCodecInst = handle;
            pEncInfo = &pCodecInst->CodecInfo->encInfo;

            GetEncOpenParamChange(encOP, encConfig.cfgFileName, NULL, handle);
            if (encOP->paramChange.paraEnable & EN_RC_FRAME_RATE) {
                VPU_EncGiveCommand(handle, ENC_SET_FRAME_RATE, &(encOP->paramChange.NewFrameRate));
                pEncInfo->openParam.frameRateInfo = (encOP->paramChange.NewFrameRate & 0xffff);
            }
            if (encOP->paramChange.paraEnable & EN_GOP_NUM) {
                VPU_EncGiveCommand(handle, ENC_SET_GOP_NUMBER, &(encOP->paramChange.NewGopNum));
                pEncInfo->openParam.gopSize = encOP->paramChange.NewGopNum;
            }
            if (encOP->paramChange.paraEnable & EN_RC_BIT_RATE) {
                VPU_EncGiveCommand(handle, ENC_SET_BITRATE, &encOP->paramChange.NewBitrate);
                pEncInfo->openParam.bitRate = encOP->paramChange.NewBitrate;
            }
            if (encOP->paramChange.paraEnable & EN_RC_INTRA_QP) {
                VPU_EncGiveCommand(handle, ENC_SET_INTRA_QP, &(encOP->paramChange.NewIntraQp));
            }
            if (encOP->paramChange.paraEnable & EN_INTRA_REFRESH) {
                VPU_EncGiveCommand(handle, ENC_SET_INTRA_MB_REFRESH_NUMBER, &encOP->paramChange.NewIntraRefresh);
                pEncInfo->openParam.intraRefreshNum = encOP->paramChange.NewIntraRefresh;
            }

            if (encOP->paramChange.paraEnable & EN_RC_MIN_MAX_QP_CHANGE) {
                int rc_max_qp = 51;
                int rc_min_qp = 12;
                VPU_EncGiveCommand(handle, ENC_SET_MIN_MAX_QP, &(encOP->paramChange.minMaxQpParam));

                if (encOP->paramChange.minMaxQpParam.maxQpIEnable)
                    pEncInfo->openParam.userQpMaxI  = encOP->paramChange.minMaxQpParam.maxQpI;
                else
                    pEncInfo->openParam.userQpMaxI  = rc_max_qp;

                if (encOP->paramChange.minMaxQpParam.minQpIEnable)
                    pEncInfo->openParam.userQpMinI = encOP->paramChange.minMaxQpParam.minQpI;
                else
                    pEncInfo->openParam.userQpMinI = rc_min_qp;

                if (encOP->paramChange.minMaxQpParam.maxQpPEnable)
                    pEncInfo->openParam.userQpMaxP = encOP->paramChange.minMaxQpParam.maxQpP;
                else
                    pEncInfo->openParam.userQpMaxP = rc_max_qp;

                if (encOP->paramChange.minMaxQpParam.maxQpPEnable)
                    pEncInfo->openParam.userQpMinP = encOP->paramChange.minMaxQpParam.minQpP;
                else
                    pEncInfo->openParam.userQpMinP = rc_min_qp;

            }
#if defined(RC_PIC_PARACHANGE) && defined(RC_CHANGE_PARAMETER_DEF)
            if (encOP->paramChange.paraEnable & EN_PIC_PARA_CHANGE)
            {
                VPU_EncGiveCommand(handle, ENC_SET_PIC_CHANGE_PARAM, &(encOP->paramChange.changePicParam));

                pEncInfo->openParam.userMaxDeltaQp = encOP->paramChange.changePicParam.MaxdeltaQp;
                pEncInfo->openParam.userMinDeltaQp = encOP->paramChange.changePicParam.MindeltaQp;
                pEncInfo->openParam.HvsQpScaleDiv2 = encOP->paramChange.changePicParam.HvsQpScaleDiv2;
                pEncInfo->openParam.EnHvsQp = encOP->paramChange.changePicParam.EnHvsQp;
                pEncInfo->openParam.EnRowLevelRc = encOP->paramChange.changePicParam.EnRowLevelRc;
                pEncInfo->openParam.RcHvsMaxDeltaQp = encOP->paramChange.changePicParam.RcHvsMaxDeltaQp;
                pEncInfo->openParam.rcInitDelay = encOP->paramChange.changePicParam.RcInitDelay;
                pEncInfo->openParam.userGamma = encOP->paramChange.changePicParam.Gamma;
                pEncInfo->openParam.rcGopIQpOffsetEn = encOP->paramChange.changePicParam.RcGopIQpOffsetEn;
                pEncInfo->openParam.rcGopIQpOffset = encOP->paramChange.changePicParam.RcGopIQpOffset;
                pEncInfo->openParam.rcWeightFactor = encOP->paramChange.changePicParam.rcWeightFactor;
#ifdef AUTO_FRM_SKIP_DROP
                pEncInfo->openParam.enAutoFrmDrop = encOP->paramChange.changePicParam.EnAuToFrmDrop;
                pEncInfo->openParam.enAutoFrmSkip = encOP->paramChange.changePicParam.EnAutoFrmSkip;
                pEncInfo->openParam.vbvThreshold = encOP->paramChange.changePicParam.VbvThreshold;
                pEncInfo->openParam.qpThreshold = encOP->paramChange.changePicParam.QpThreshold;
                pEncInfo->openParam.maxContinuosFrameDropNum = encOP->paramChange.changePicParam.MaxContinuousFrameDropNum;
                pEncInfo->openParam.maxContinuosFrameSkipNum = encOP->paramChange.changePicParam.MaxContinuousFrameSkipNum;

                // frm_rc_set_auto_skip_param(
                //     &pEncInfo->frm_rc,
                //     pEncInfo->openParam.enAutoFrmSkip,
                //     pEncInfo->openParam.enAutoFrmDrop,
                //     pEncInfo->openParam.vbvThreshold,
                //     pEncInfo->openParam.qpThreshold,
                //     pEncInfo->openParam.maxContinuosFrameSkipNum,
                //     pEncInfo->openParam.maxContinuosFrameDropNum
                //     );
#endif /* AUTO_FRM_SKIP_DROP */

	            if (pEncInfo->max_temporal_id != 0)
	            {
		            if (pEncInfo->gop_size != encOP->paramChange.changePicParam.SetDqpNum)
			            pEncInfo->openParam.set_dqp_pic_num = pEncInfo->gop_size;

		            pEncInfo->openParam.enAutoFrmDrop = 0;//encOP->paramChange.changePicParam.EnAuToFrmDrop;
		            pEncInfo->openParam.enAutoFrmSkip = 0;//encOP->paramChange.changePicParam.EnAutoFrmSkip;
	            }
	            else
		            pEncInfo->openParam.set_dqp_pic_num = encOP->paramChange.changePicParam.SetDqpNum;

	            for (idx = 0; idx < 8; idx++)
		            pEncInfo->openParam.dqp[idx] = encOP->paramChange.changePicParam.dqp[idx];

            }
#endif /* (RC_PIC_PARACHANGE) && (RC_CHANGE_PARAMETER_DEF) */
            if (encOP->paramChange.paraEnable & EN_GOP_NUM) {
                int encoded_frames;

                encoded_frames = encParam->fieldRun ? 2*pEncInfo->encoded_frames_in_gop :  pEncInfo->encoded_frames_in_gop;
                if (encoded_frames - encOP->paramChange.NewGopNum >= 0)
                    pEncInfo->encoded_frames_in_gop = 0;
            }
            if (encOP->paramChange.paraEnable &  (EN_RC_FRAME_RATE | EN_RC_BIT_RATE | EN_PIC_PARA_CHANGE))
            {
                int pic_width;
                int pic_height;
                int is_first_pic = (pEncInfo->frameIdx == 0 && fieldDone == FALSE);
                int frame_rate = (pEncInfo->openParam.frameRateInfo & 0xffff) / ((pEncInfo->openParam.frameRateInfo >> 16) + 1);
                int gop_size = pEncInfo->openParam.gopSize;
                int gamma = pEncInfo->openParam.userGamma;
#ifdef CLIP_PIC_DELTA_QP
                int max_delta_qp_minus = pEncInfo->openParam.userMinDeltaQp;
                int max_delta_qp_plus = pEncInfo->openParam.userMaxDeltaQp;
#endif /* CLIP_PIC_DELTA_QP */
                if (pEncInfo->rotationAngle == 90 || pEncInfo->rotationAngle == 270) {
                    pic_width = pEncInfo->openParam.picHeight;
                    pic_height = pEncInfo->openParam.picWidth;
                }
                else {
                    pic_width = pEncInfo->openParam.picWidth;
                    pic_height = pEncInfo->openParam.picHeight;
                }

                if (pEncInfo->openParam.EncStdParam.avcParam.fieldFlag)
                {
                    pic_height = pic_height >> 1;
                    frame_rate = frame_rate << 1;
                    if (gop_size != 1)
                        gop_size = pEncInfo->openParam.gopSize << 1;
                }
                if(gamma ==  -1) gamma = 1;
#ifdef CLIP_PIC_DELTA_QP
                if(max_delta_qp_minus == -1) max_delta_qp_minus = 51;
                if(max_delta_qp_plus == -1) max_delta_qp_plus = 51;
#endif /* CLIP_PIC_DELTA_QP */
	            {
		            int i;

		            for (i = 0; i < pEncInfo->openParam.set_dqp_pic_num; i++)
			            pEncInfo->openParam.gopEntry[i].qp_offset = encOP->paramChange.changePicParam.dqp[i];
	            }
            }
        }
    }

    encParam->coda9RoiEnable = encOP->coda9RoiEnable;

    // DONE: roi param from TestArgV
    if (pTestArgV->roiEnable) {
        encParam->roiQpMapAddr       = vbRoi.phys_addr;
        encParam->coda9RoiPicAvgQp   = pTestArgV->roiPicAvgQp;
        encParam->setROI.mode        = pTestArgV->roiParamMode;
        encParam->setROI.number      = pTestArgV->roiParamNumber;
        if (encParam->setROI.mode == 1)
            encParam->nonRoiQp       = pTestArgV-> nonRoiQp;

        for (int i = 0; i < pTestArgV->roiParamNumber; i++) {
            encParam->setROI.region[i].left   = pTestArgV->roiParamRegion[i][0];
            encParam->setROI.region[i].right  = pTestArgV->roiParamRegion[i][1];
            encParam->setROI.region[i].top    = pTestArgV->roiParamRegion[i][2];
            encParam->setROI.region[i].bottom = pTestArgV->roiParamRegion[i][3];
            encParam->setROI.qp[i]            = pTestArgV->roiParamQP[i];
        }
    }
//     if (encOP->coda9RoiEnable) {
//         encParam->roiQpMapAddr       = vbRoi.phys_addr;
//         encParam->coda9RoiPicAvgQp   = encOP->RoiPicAvgQp;
//         if (encConfig.roi_file_name[0]) {
// #ifdef ROI_MB_RC
//             char lineStr[256] = {0,};
//             int  val;
//             int roiNum;
//             int non_roi_qp, roi_avg_qp;
//             fgets(lineStr, 256, encConfig.roi_file);
//             sscanf(lineStr, "%x\n", &val);
//             if (val != 0xFFFF)
//                 VLOG(ERR, "can't find the picture delimiter \n");

//             fgets(lineStr, 256, encConfig.roi_file);
//             sscanf(lineStr, "%d\n", &val);  // picture index

//             fgets(lineStr, 256, encConfig.roi_file);
//             sscanf(lineStr, "%d %d %d\n", &roiNum, &non_roi_qp, &roi_avg_qp);   // number of roi regions
//             encParam->setROI.number = roiNum;
//             encParam->nonRoiQp = non_roi_qp;
//             encParam->coda9RoiPicAvgQp = roi_avg_qp;
//             for(idx = 0; idx < roiNum; idx++) {
//                 fgets(lineStr, 256, encConfig.roi_file);
//                 sscanf(lineStr, "%d %d %d %d %d\n", &encParam->setROI.region[idx].left, &encParam->setROI.region[idx].right,
//                     &encParam->setROI.region[idx].top, &encParam->setROI.region[idx].bottom, &encParam->setROI.qp[idx]);
//             }
// #else
//             Uint8   *pRoiBuf;
//             int     pic_height = encOP->EncStdParam.avcParam.fieldFlag ?
//                 encOP->picHeight / 2 : encOP->picHeight;
//             Int32   roiMapSize = VPU_ALIGN16(encOP->picWidth) * VPU_ALIGN16(pic_height) / 256;  // 1 Byte per 1 MB.

//             pRoiBuf = (Uint8*)osal_malloc(roiMapSize);

//             if (osal_fread(pRoiBuf, 1, roiMapSize, encConfig.roi_file) != roiMapSize) {
//                 osal_fseek(encConfig.roi_file, 0, SEEK_SET);
//                 osal_fread(pRoiBuf, 1, roiMapSize, encConfig.roi_file);
//             }

//             vdi_write_memory(encConfig.coreIdx, encParam->roiQpMapAddr, pRoiBuf, roiMapSize, VDI_BIG_ENDIAN);
//             osal_free(pRoiBuf);
// #endif /* ROI_MB_RC */
//         }
//     }
}



// !!! from Mars SDK `main_enc_cvitest.c`, for reconbuffer mode: addrremp or single luma or nomal
static int _cviVEncRegFrameBuffer( EncHandle handle,
                                    EncOpenParam *pEncOP, TestEncConfig * pEncCfg,
                                    vpu_buffer_t* pVbReconFrameMem, FrameBuffer* pFbRecon,
                                    TestArgV_t *pTestArgV,
                                    int regFrameBufCount, int framebufWidth, int framebufHeight)
{
	int ret = 0;
	int coreIdx;
	int FrameBufSize;
	int framebufStride = 0;
	// cviEncCfg *pcviEc = &pTestEnc->encConfig.cviEc;
	// EncOpenParam *pEncOP = &pTestEnc->encOP;
	int i, mapType;
	CodecInst *pCodecInst = (CodecInst *) handle;


	coreIdx = pEncCfg->coreIdx;

	/* Allocate framebuffers */
	framebufStride =
		CalcStride(framebufWidth, framebufHeight,
			   pEncOP->srcFormat,
			   pEncOP->cbcrInterleave,
			   (TiledMapType)(pEncCfg->mapType & 0x0f),
			   FALSE);
	FrameBufSize = VPU_GetFrameBufSize(
		coreIdx, framebufStride,
		framebufHeight,
		(TiledMapType)(pEncCfg->mapType & 0x0f),
		pEncOP->srcFormat, pEncOP->cbcrInterleave,
		NULL);
	// pTestEnc->regFrameBufCount = pTestEnc->initialInfo.minFrameBufferCount;

	VLOG(INFO, "framebufWidth = %d, framebufHeight = %d, framebufStride = %d\n",
		framebufWidth, framebufHeight, framebufStride);
	VLOG(INFO, "FrameBufSize = 0x%X, regFrameBufCount = %d\n", FrameBufSize,
		   regFrameBufCount);

	if (pEncOP->addrRemapEn)
		pTestArgV->singleLumaBuf = 0;

	pCodecInst->CodecInfo->encInfo.singleLumaBuf = pTestArgV->singleLumaBuf;

    //>>> case: single luma OPEN, NO addrRemap
	if (pTestArgV->singleLumaBuf > 0) {
		int ber, aft;
		int luma_size, chr_size;
		int productId;

		productId = ProductVpuGetId(coreIdx);
		luma_size = CalcLumaSize(
			productId, framebufStride, framebufHeight,
            pEncOP->srcFormat,
			pEncOP->cbcrInterleave,
			(TiledMapType)(pEncCfg->mapType & 0x0f),
			NULL);

		chr_size = CalcChromaSize(
			productId, framebufStride, framebufHeight,
			pEncOP->srcFormat,
			pEncOP->cbcrInterleave,
			(TiledMapType)(pEncCfg->mapType & 0x0f),
			NULL);

		i = 0;
		// note: smart-p case, need long term reference frame buffer, so the first (regFrameBufCount -2) recon buffer allocate normally
		if (pTestArgV->virtualIPeriod > 0 && regFrameBufCount > 2) {
            for (i = 0; i < (regFrameBufCount - 2); i++) {
                pVbReconFrameMem[i].size = FrameBufSize;
                // sprintf(ionName, "VENC_%d_ReconFrameBuf", pCodecInst->s32ChnNum);
                // if (VDI_ALLOCATE_MEMORY(coreIdx,&pTestEnc->vbReconFrameBuf[i],0, ionName) < 0) {
                if(vdi_allocate_dma_memory(coreIdx, &(pVbReconFrameMem[i])) < 0) {
                    VLOG(ERR, "!!error: smart-p case fail to allocate recon buffer\n");
                    return -1;
                }

                pFbRecon[i].bufY = pVbReconFrameMem[i].phys_addr;
                pFbRecon[i].bufCb = (PhysicalAddress)-1;
                pFbRecon[i].bufCr = (PhysicalAddress)-1;
                pFbRecon[i].size = FrameBufSize;
                pFbRecon[i].updateFbInfo = TRUE;
            }
		}
        // note: allocate the last 2 frame buffers as single luma buffer
		aft = i;
		ber = aft + 1;
        // note: framebufsize是一帧完整的yuv的大小，chr_size是单个u或单个v的大小
        // note: 这里相当于分配一帧完整的Y+UV，加上单独的一个UV，luma是single的（共享的）
        pVbReconFrameMem[ber].size = FrameBufSize + chr_size * 2; // YUV + UV
        // memset(ionName, 0, MAX_VPU_ION_BUFFER_NAME);
        // sprintf(ionName, "VENC_%d_ReconFrameBuf", pCodecInst->s32ChnNum);
        // if (VDI_ALLOCATE_MEMORY(coreIdx, &pTestEnc->vbReconFrameBuf[ber], 0, ionName) < 0) {
        if(vdi_allocate_dma_memory(coreIdx, &(pVbReconFrameMem[ber])) < 0) {
            printf("!! error:fail to allocate recon buffer\n");
            return -1;
        }
        pFbRecon[ber].bufY = pVbReconFrameMem[ber].phys_addr;
        pFbRecon[ber].size = FrameBufSize;

		pFbRecon[aft].bufY = pFbRecon[ber].bufY;
		pFbRecon[ber].bufCb = pFbRecon[aft].bufY + luma_size;
		pFbRecon[ber].bufCr = pFbRecon[ber].bufCb + chr_size;
		pFbRecon[aft].bufCb = pFbRecon[ber].bufCr + chr_size;
		pFbRecon[aft].bufCr = pFbRecon[aft].bufCb + chr_size;

		pFbRecon[ber].updateFbInfo = TRUE;
		memcpy(&pVbReconFrameMem[aft],
		       &pVbReconFrameMem[ber], sizeof(vpu_buffer_t));
		pFbRecon[aft].size = pFbRecon[ber].size;
		pFbRecon[aft].updateFbInfo = TRUE;
	}
    //>>> case: single luma CLOSE, see addrRemapEn:
    //     >>> addrRemapEn: call cviInitAddrRemap() to set VbReconFrameMem and FbRecon
    //     >>> !addrRemapEn: allocate recon buffer normally use C&M API
    else {
        if (pEncOP->addrRemapEn) {
            //ret = cviInitAddrRemap(pTestEnc);
            ret = cviInitAddrRemap(pEncCfg, pEncOP, handle, pVbReconFrameMem, pFbRecon, pTestArgV, regFrameBufCount);
            if (ret != 0) {
                printf("!!error: cviInitAddrRemap ret: %d\n", ret);
                return ret;
            }
        } else {
            for (i = 0; i < regFrameBufCount; i++) {
                pVbReconFrameMem[i].size = FrameBufSize;
                // memset(ionName, 0, MAX_VPU_ION_BUFFER_NAME);
                // sprintf(ionName, "VENC_%d_ReconFrameBuf", pCodecInst->s32ChnNum);
                if (vdi_allocate_dma_memory(coreIdx, &pVbReconFrameMem[i]) < 0) {
                    printf("!!error: fail to allocate recon buffer\n");
                    return -1;
                }

                pFbRecon[i].bufY = pVbReconFrameMem[i].phys_addr;
                pFbRecon[i].bufCb = (PhysicalAddress)-1;
                pFbRecon[i].bufCr = (PhysicalAddress)-1;
                pFbRecon[i].size = FrameBufSize;
                pFbRecon[i].updateFbInfo = TRUE;
            }
        }
	}

	mapType = (TiledMapType)(pEncCfg->mapType & 0x0f);

	ret = VPU_EncRegisterFrameBuffer(handle, pFbRecon, regFrameBufCount, framebufStride,
					                 framebufHeight, mapType);

	if (ret != RETCODE_SUCCESS) {
		VLOG(ERR, "!!error: VPU_EncRegisterFrameBuffer failed Error code is 0x%x\n",ret);
		return -1;
	}

	for (i = 0; i < regFrameBufCount; i++) {
		VLOG(INFO, "Reg Recon buffer done: Rec, bufY = 0x%llX, bufCb = 0x%llX, bufCr = 0x%llX\n",
			pFbRecon[i].bufY, pFbRecon[i].bufCb, pFbRecon[i].bufCr);
	}

	return ret;
}

int SyncParamFromTestArgV(EncOpenParam* pEncOP, EncParam* pEncParam, TestArgV_t* pTestArgV)
{
    int i;

    if (pEncOP == NULL || pTestArgV == NULL) {
        VLOG(ERR, "pEncOP or pTestArgV is NULL\n");
        return -1;
    }

    pEncOP->picWidth                     = pTestArgV->picWidth;
    pEncOP->picHeight                    = pTestArgV->picHeight;
    pEncOP->nv21                         = pTestArgV->enNv21;
    pEncOP->gopSize                      = pTestArgV->gopSize;
    pEncOP->idrInterval                  = pTestArgV->idrInterval;
    pEncOP->EncStdParam.avcParam.profile = pTestArgV->avcProfile;
    pEncOP->rcEnable                     = pTestArgV->rcEnable;
    pEncOP->bitRate                      = pTestArgV->targetBitrate;
    pEncOP->userQpMinI                   = pTestArgV->iqpmin;
    pEncOP->userQpMaxI                   = pTestArgV->iqpmax;
    pEncOP->userQpMinP                   = pTestArgV->pqpmin;
    pEncOP->userQpMaxP                   = pTestArgV->pqpmax;

    // slice parameters
    if (pTestArgV->sliceMode) { //multi slice
        pEncOP->sliceMode.sliceMode                    = 1;
        pEncOP->sliceMode.sliceSizeMode                = 1;
        pEncOP->sliceMode.sliceSize                    = pTestArgV->mbLineNum * (pTestArgV->picWidth + 15) / 16;
    } else { // single slice
        pEncOP->sliceMode.sliceMode                    = 0;
        pEncOP->sliceMode.sliceSizeMode                = 0;
        pEncOP->sliceMode.sliceSize                    = 0;
    }

    // ROI parameters
    pEncOP->coda9RoiEnable               = pTestArgV->roiEnable;
    if (pTestArgV->roiEnable) {
        pEncParam->setROI.mode = pTestArgV->roiParamMode;
        pEncParam->setROI.number = pTestArgV->roiParamNumber;
        for (i = 0; i < pTestArgV->roiParamNumber; i++) {
            pEncParam->setROI.region[i].left = pTestArgV->roiParamRegion[i][0];
            pEncParam->setROI.region[i].top = pTestArgV->roiParamRegion[i][1];
            pEncParam->setROI.region[i].right = pTestArgV->roiParamRegion[i][2];
            pEncParam->setROI.region[i].bottom = pTestArgV->roiParamRegion[i][3];
            pEncParam->setROI.qp[i] = pTestArgV->roiParamQP[i];
        }
    }

    pEncOP->addrRemapEn                  = pTestArgV->addrRemapEn;

    return 0;
}


// !!! this is the main process of encoding
BOOL TestEncoder(TestEncConfig*  pTestConfig, TestArgV_t* pTestArgV)
{
    EncHandle               handle  = { 0 };
    EncOpenParam            encOP   = { 0 };
    EncParam                encParam    = { 0 };
    EncInitialInfo          initialInfo = { 0 };
    EncOutputInfo           outputInfo  = { 0 };
    SecAxiUse               secAxiUse;
    vpu_buffer_t            vbStream    = {0};
    FrameBuffer             fbRecon[MAX_REG_FRAME];
    vpu_buffer_t            vbReconFrameMem[MAX_REG_FRAME];
    FrameBuffer             fbSrc[MAX_TEST_FRAME_NUM];
    //vpu_buffer_t            vbSrcFrameMem[ENC_SRC_BUF_NUM];
    FrameBufferAllocInfo    fbAllocInfo;
    RetCode                 retApi = RETCODE_SUCCESS;
    EncHeaderParam          encHeaderParam  = { 0 };
    BOOL                    fieldDone;
    size_t                  sizeFb;
    Int32                   ret;
    Int32                   i = 0, srcFrameIdx = 0, frameIdx = 0;
    Int32                   framebufStride = 0, framebufWidth = 0, framebufHeight = 0;
    FrameBufferFormat       srcFrameFormat = FORMAT_420;
    FrameBufferFormat       framebufFormat = FORMAT_420;
    TiledMapType            mapType;
    Int32                   regFrameBufCount=0;
    Int32                   interruptFlag = 0;
    Int32                   timeoutCount = 0;
    Int32                   coreIdx = 0;
    TiledMapConfig          mapCfg;
    MaverickCacheConfig     encCacheConfig;
    YuvFeeder               yuvFeeder               = NULL;
    BitstreamReader         bsReader                = NULL;
    Comparator              comparator              = NULL;
    BOOL                    success                 = TRUE;
    BOOL                    loop                    = TRUE;
    ENC_CFG                 cfgParam;
    char                    yuvPath[MAX_FILE_PATH];
    TiledMapConfig          mapConfig;
    BOOL                    isForceOutNum = FALSE;
    YuvInfo                 yuvFeederInfo;
#ifdef AUTO_FRM_SKIP_DROP
    BOOL isDroppedPic = FALSE;
#endif
    vpu_buffer_t                vbRoi;


    osal_memset(fbSrc, 0x00, sizeof(fbSrc));
    osal_memset(fbRecon, 0x00, sizeof(fbRecon));
    //osal_memset(vbSrcFrameMem, 0x00, sizeof(vbSrcFrameMem));
    osal_memset(vbReconFrameMem, 0x00, sizeof(vbReconFrameMem));
    osal_memset((void*)&cfgParam, 0x00, sizeof(ENC_CFG));
    osal_memset(&yuvFeederInfo,   0x00, sizeof(YuvInfo));

    // step: load firmware
    coreIdx = pTestConfig->coreIdx;
    //NOTE: `pusBitcode` point to `fw_264` through `LoadFirmweare_EX`, which is defined in `fw_264.h`
    retApi = VPU_InitWithBitcode(coreIdx, (const Uint16*)pusBitCode, sizeInWord);
    if (retApi != RETCODE_SUCCESS && retApi != RETCODE_CALLED_BEFORE) {
        VLOG(ERR, "VPU_Init failed Error code is 0x%x \n", retApi);
        success = FALSE;
        goto ERR_ENC_INIT;
    }
    PrintVpuVersionInfo(coreIdx);

    /********************************************************************************
    // step: set encOP
    //NOTE: PARSE ENCODER PARAMETER FILE, here we munaually set encOP values, no read files
     // 1. TestEncConfig* pTestConfig from main function.
     // 2. GetEncOpenParamDefault() in vpuhelper.c to set default encOP and sync pTestConfig
     // 3. SyncParamFromTestArgV() to sync encOP values from `testecase_venc`
    ********************************************************************************/
    // Fill parameters for encoding.
    encOP.bitstreamFormat = pTestConfig->stdMode;
    mapType = (TiledMapType)pTestConfig->mapType;
    isForceOutNum = (pTestConfig->outNum > 0) ? TRUE : FALSE;
    encOP.linear2TiledEnable = pTestConfig->coda9.enableLinear2Tiled;
    if (encOP.linear2TiledEnable == TRUE) {
        encOP.linear2TiledMode = FF_FRAME;
    }
    // enable SVC for advanced termporal reference structure
    pTestConfig->coda9.enableSvc = (encOP.gopPreset != 1);
    encOP.EncStdParam.avcParam.mvcExtension = pTestConfig->coda9.enableMvc;
	encOP.EncStdParam.avcParam.svcExtension = pTestConfig->coda9.enableSvc;
    // DONE: here, we can use `GetEncOpenParamDefault` in vpuhelper.c to set encOP values manually, so that we can avoid `osal_open/read` in NonOS
    // DOING: modify `GetEncOpenParamDefault` to set default encOP
    // ret = GetEncOpenParam(&encOP, pTestConfig, &cfgParam);
    ret = GetEncOpenParamDefault(&encOP, pTestConfig);
    if (ret == 0) {
        VLOG(ERR, "Failed to set default encOP values\n");
        success = FALSE;
        goto ERR_ENC_INIT;
    }
    printf("set default encOP values done\n");

    // DOING: here we use `SyncParamFromTestArgV` to set EncOpenParam/EncParam/CodecInst->EncInfo values from `testecase_venc`
    ret = SyncParamFromTestArgV(&encOP, &encParam, pTestArgV);
    if (encOP.addrRemapEn) { // sync addrRemap parameters to codecInst->CodecInfo->encInfo
        handle->CodecInfo->encInfo.singleLumaBuf = pTestArgV->singleLumaBuf;
        handle->CodecInfo->encInfo.addrRemapEn = 1;
        handle->CodecInfo->encInfo.addrRemap.mode = pTestArgV->ARMode;
        handle->CodecInfo->encInfo.addrRemap.numExtraLine = pTestArgV->numExtraLine;
    }
    printf("SyncParamFromTestArgV done\n");

    if (encOP.EncStdParam.avcParam.fieldFlag == TRUE) {
        if (pTestConfig->rotAngle != 0 || pTestConfig->mirDir != 0) {
            VLOG(WARN, "%s:%d When field Flag is enabled. VPU doesn't support rotation or mirror in field encoding mode.\n",
                __FUNCTION__, __LINE__);
            pTestConfig->rotAngle = 0;
            pTestConfig->mirDir   = MIRDIR_NONE;
        }
    }

    /********************************************************************************
     * step: alloc Bitstream Buffer, open VPU
     ********************************************************************************/
    // note: allocate bitstream buffer through vdi_*,
    // DOING: here we can directly set encOP.bitstreamBuffer and encOP.bitstreamBufferSize to a specific phys address
    // encOP.bitstreamBuffer       = 0x80000000;
    // encOP.bitstreamBufferSize   = STREAM_BUF_SIZE;
    vbStream.size               = STREAM_BUF_SIZE;
    if (vdi_allocate_dma_memory(coreIdx, &vbStream) < 0) {
        VLOG(ERR, "fail to allocate bitstream buffer\n");
        success = FALSE;
        goto ERR_ENC_INIT;
    }
    printf("allocate bitstream buffer done, phys_addr = 0x%x, size = 0x%x\n", vbStream.phys_addr, vbStream.size);

    encOP.bitstreamBuffer       = vbStream.phys_addr;
    encOP.bitstreamBufferSize   = vbStream.size;
    encOP.ringBufferEnable      = pTestConfig->ringBufferEnable;
    encOP.srcFormat            = srcFrameFormat;
    encOP.cbcrInterleave        = pTestConfig->cbcrInterleave;
    encOP.nv21                  = pTestConfig->nv21;
    encOP.frameEndian           = pTestConfig->frame_endian;
    encOP.streamEndian          = pTestConfig->stream_endian;
    encOP.bwbEnable             = VPU_ENABLE_BWB;
    encOP.lineBufIntEn          = pTestConfig->lineBufIntEn;
    encOP.coreIdx               = coreIdx;
    // note: the two param are sync from v410
    encOP.linear2TiledEnable    = TRUE;//pTestConfig->coda9.enableLinear2Tiled;
    encOP.linear2TiledMode      = FF_FRAME;//pTestConfig->coda9.linear2TiledMode;
    encOP.cbcrOrder             = CBCR_ORDER_NORMAL;            // YV12 = CBCR_OERDER_REVERSED

    //VPU_EncSetWrPtr(handle, encOP.bitstreamBuffer, 1);

#ifdef SUPPORT_MULTIPLE_PPS // if SUPPORT_MULTIPLE_PPS is enabled. encoder can include multiple pps in bitstream output.
    if (encOP.bitstreamFormat == STD_AVC) {
        getAvcEncPPS(&encOP); //add PPS before OPEN
    }
#endif
    // Open an instance and get initial information for encoding.
    if ((ret=VPU_EncOpen(&handle, &encOP)) != RETCODE_SUCCESS) {
        VLOG(ERR, "VPU_EncOpen failed Error code is 0x%x \n", ret);
        success = FALSE;
        goto ERR_ENC_INIT;
    }
    printf("VPU_EncOpen done, handle = 0x%x\n", handle);

    //VPU_EncGiveCommand(handle, ENABLE_LOGGING, NULL);
    // TODO: 需要改成碼流直接寫入指定的地址，編碼完成後内存計算md5
    // if ((bsReader=BitstreamReader_Create(encOP.ringBufferEnable, pTestConfig->bitstreamFileName, (EndianMode)pTestConfig->stream_endian, &handle)) == NULL) {
    //     VLOG(ERR, "Failed to BitstreamReader_Create\n");
    //     success = FALSE;
    //     goto ERR_ENC_OPEN;
    // }

    // if ((comparator=Comparator_Create(pTestConfig->compare_type, pTestConfig->ref_stream_path, pTestConfig->cfgFileName)) == NULL) {
    //     success = FALSE;
    //     goto ERR_ENC_OPEN;
    // }

    // step: VPU mirror and rotate set
    //if (pTestConfig->useRot == TRUE) {
    if (pTestArgV->rotate > 0) {
        VPU_EncGiveCommand(handle, ENABLE_ROTATION, NULL);
        VPU_EncGiveCommand(handle, SET_ROTATION_ANGLE, (void*)&pTestArgV->rotate);
    }
    if (pTestArgV->mirror > 0) {
        VPU_EncGiveCommand(handle, ENABLE_MIRRORING, NULL);
        VPU_EncGiveCommand(handle, SET_MIRROR_DIRECTION, (void*)&pTestArgV->mirror);
    }

    // allocate frame buffers for source frame
    // step: VPU secondary AXI set, here H264 use default value: 0xf
    secAxiUse.u.coda9.useBitEnable  = (pTestConfig->secondary_axi>>0)&0x01;
    secAxiUse.u.coda9.useIpEnable   = (pTestConfig->secondary_axi>>1)&0x01;

    secAxiUse.u.coda9.useDbkYEnable = (pTestConfig->secondary_axi>>2)&0x01;
    secAxiUse.u.coda9.useDbkCEnable = (pTestConfig->secondary_axi>>3)&0x01;
    secAxiUse.u.coda9.useBtpEnable  = (pTestConfig->secondary_axi>>4)&0x01;
    secAxiUse.u.coda9.useOvlEnable  = (pTestConfig->secondary_axi>>5)&0x01;
    VPU_EncGiveCommand(handle, SET_SEC_AXI, &secAxiUse);
    printf("VPU_EncGiveCommand SET_SEC_AXI done\n");
    // step: VPU ROI mem allocate
    if (pTestArgV->roiEnable) {
        vbRoi.size = VPU_ALIGN256(VPU_ALIGN16(encOP.picWidth) * VPU_ALIGN16(encOP.picHeight)/256);  // 1 Byte per 1 MB.

        if (vdi_allocate_dma_memory(coreIdx, &vbRoi)  < 0) {
            VLOG(ERR, "Failed to allocate ROI buffer\n");
            return FALSE;
        }
        printf("allocate ROI buffer done, phys_addr = 0x%x, size = 0x%x\n", vbRoi.phys_addr, vbRoi.size);
        // DONE: modify `roi data read from file` to `read from TestArgV`, see RcChangeParam()
        // if (pTestConfig->roi_file_name[0]) {
        //     ChangePathStyle(pTestConfig->roi_file_name);
        //     if ((pTestConfig->roi_file = osal_fopen(pTestConfig->roi_file_name, "r")) == NULL) {
        //         VLOG(ERR, "Failed to open ROI file, %s\n", pTestConfig->roi_file_name);
        //         return FALSE;
        //     }
        // }
    }

    /********************************************************************************
     * step:get INIT_SEQ info
     ********************************************************************************/
    VPU_EncSetWrPtr(handle, encOP.bitstreamBuffer, 1);
    ret = VPU_EncGetInitialInfo(handle, &initialInfo);
    printf("VPU_EncGetInitialInfo ret = 0x%x, initialInfo.minFrameBufferCount = %d\n", ret, initialInfo.minFrameBufferCount);
    if( ret != RETCODE_SUCCESS ) {
        VLOG(ERR, "VPU_EncGetInitialInfo failed Error code is 0x%x \n", ret );
        success = FALSE;
        PrintVpuStatus(coreIdx, PRODUCT_ID_980);
        goto ERR_ENC_OPEN;
    }
    printf("VPU_EncGetInitialInfo done\n");
    PrintVpuStatus(coreIdx, PRODUCT_ID_980);



    // Note: The below values of MaverickCache configuration are best values.
    // !!! DO NOT Modify
    MaverickCache2Config(
        &encCacheConfig,
        0, //encoder
        encOP.cbcrInterleave, // cb cr interleave
        0, /* bypass */
        0, /* burst  */
        3, /* merge mode */
        mapType,
        15 /* shape */);
    VPU_EncGiveCommand(handle, SET_CACHE_CONFIG, &encCacheConfig);

    /********************************************************************************
     * step:ALLOCATE RECON BUFFERS
     * DONE: transplant addrRemap code from v410
     ********************************************************************************/

    srcFrameFormat = encOP.EncStdParam.avcParam.chromaFormat400?FORMAT_400:FORMAT_420;
    framebufFormat = encOP.EncStdParam.avcParam.chromaFormat400?FORMAT_400:FORMAT_420;

    framebufWidth    = (pTestArgV->rotate==1||pTestArgV->rotate ==3)?encOP.picHeight:encOP.picWidth;
    framebufHeight   = (pTestArgV->rotate==1||pTestArgV->rotate ==3)?encOP.picWidth:encOP.picHeight;
    framebufWidth    = VPU_ALIGN16(framebufWidth);
    framebufHeight   = VPU_ALIGN32(framebufHeight); // To cover interlaced picture
    framebufStride   = CalcStride(framebufWidth, framebufHeight, framebufFormat, encOP.cbcrInterleave, mapType, FALSE);
    regFrameBufCount = initialInfo.minFrameBufferCount;

    osal_memset((void*)&fbAllocInfo, 0x00, sizeof(fbAllocInfo));
    osal_memset((void*)fbRecon, 0x00, sizeof(fbRecon));
    osal_memset((void*)vbReconFrameMem, 0x00, sizeof(vbReconFrameMem));
    // fbAllocInfo.format          = FORMAT_420;
    // fbAllocInfo.cbcrInterleave  = encOP.cbcrInterleave;
    // fbAllocInfo.mapType         = mapType;
    // fbAllocInfo.stride          = framebufStride;
    // fbAllocInfo.height          = framebufHeight;
    // fbAllocInfo.lumaBitDepth    = 8;
    // fbAllocInfo.chromaBitDepth  = 8;
    // fbAllocInfo.num             = regFrameBufCount;
    // fbAllocInfo.endian          = encOP.frameEndian;
    // fbAllocInfo.type            = FB_TYPE_CODEC;

    // sizeFb = VPU_GetFrameBufSize(coreIdx, framebufStride, framebufHeight, mapType, framebufFormat, encOP.cbcrInterleave, NULL);
    // for (i=0; i<regFrameBufCount; i++) {
    //     vbReconFrameMem[i].size = sizeFb;
    //     if (vdi_allocate_dma_memory(coreIdx, &vbReconFrameMem[i]) < 0) {
    //         VLOG(ERR, "%s:%d fail to allocate frame buffer\n", __FUNCTION__, __LINE__);
    //         success = FALSE;
    //         goto ERR_ENC_OPEN;
    //     }
    //     fbRecon[i].bufY  = vbReconFrameMem[i].phys_addr;
    //     fbRecon[i].bufCb = (PhysicalAddress)-1;
    //     fbRecon[i].bufCr = (PhysicalAddress)-1;
    //     fbRecon[i].updateFbInfo = TRUE;
    // }

    // ret = VPU_EncAllocateFrameBuffer(handle, fbAllocInfo, fbRecon);
    // if (ret != RETCODE_SUCCESS) {
    //     VLOG(ERR, "VPU_EncAllocateFrameBuffer fail to allocate source frame buffer is 0x%x \n", ret );
    //     success = FALSE;
    //     goto ERR_ENC_OPEN;
    // }

    /********************************************************************************
     * step register Recon Buffer
     ********************************************************************************/
    // ret = VPU_EncRegisterFrameBuffer(handle, fbRecon, regFrameBufCount, framebufWidth, framebufHeight, mapType);
    // if( ret != RETCODE_SUCCESS ) {
    //     VLOG(ERR, "VPU_EncRegisterFrameBuffer failed Error code is 0x%x \n", ret );
    //     success = FALSE;
    //     goto ERR_ENC_OPEN;
    // }
    // VPU_EncGiveCommand(handle, GET_TILEDMAP_CONFIG, &mapCfg);
    // DONE: 该函数涵盖了上边注释掉的步骤，详细内容看函数实现
    ret = _cviVEncRegFrameBuffer(handle, &encOP, pTestConfig,
                                    vbReconFrameMem, fbRecon, pTestArgV,
                                    regFrameBufCount, framebufWidth, framebufHeight);


    /********************************************************************************
     * step: ALLOCATE SROUCE BUFFERS
     ********************************************************************************/
    //DONE: 这里无需再分配SourceBuffer對應的vbSrcFrameMem, 直接使用LoadBINARY指令将src提前加载到对应地址
    //DONE: 將fbSrc[]的個數改爲待測試的縂幀數，所有的幀都在fbSrc中
    fbAllocInfo.format         = srcFrameFormat;
    fbAllocInfo.cbcrInterleave = encOP.cbcrInterleave;
    fbAllocInfo.nv21           = encOP.nv21;
    if (encOP.linear2TiledEnable == TRUE) {
        fbAllocInfo.mapType = LINEAR_FRAME_MAP;
        fbAllocInfo.stride  = encOP.picWidth;
    }
    else {
        fbAllocInfo.mapType = mapType;
        fbAllocInfo.stride  = CalcStride(encOP.picWidth, encOP.picHeight, srcFrameFormat, encOP.cbcrInterleave, mapType, FALSE);
    }
    fbAllocInfo.height  = VPU_ALIGN16(encOP.picHeight);
    fbAllocInfo.num     = MAX_TEST_FRAME_NUM;
    fbAllocInfo.endian  = encOP.frameEndian;
    fbAllocInfo.type    = FB_TYPE_PPU;

    //osal_memset(vbSrcFrameMem, 0x00, sizeof(vbSrcFrameMem));
    sizeFb = VPU_GetFrameBufSize(coreIdx, fbAllocInfo.stride, fbAllocInfo.height, fbAllocInfo.mapType, srcFrameFormat, encOP.cbcrInterleave, NULL);
    for (i=0; i<MAX_TEST_FRAME_NUM; i++) {
        fbSrc[i].bufY  = (PhysicalAddress)reserveSrcFrameAddr[i];
        fbSrc[i].bufCb = (PhysicalAddress)-1;
        fbSrc[i].bufCr = (PhysicalAddress)-1;
        fbSrc[i].updateFbInfo = TRUE;
    }

    ret = VPU_EncAllocateFrameBuffer(handle, fbAllocInfo, fbSrc);
    if (ret != RETCODE_SUCCESS) {
        VLOG(ERR, "VPU_EncAllocateFrameBuffer fail to allocate source frame buffer is 0x%x \n", ret );
        success = FALSE;
        goto ERR_ENC_OPEN;
    }


    /********************************************************************************
     * step: BUILD SEQUENCE HEADER
     ********************************************************************************/
    encParam.forceIPicture = 0;
    encParam.skipPicture   = 0;
    encParam.quantParam    = pTestConfig->picQpY;

    encHeaderParam.zeroPaddingEnable = 0;
    if (encOP.ringBufferEnable == FALSE) {
        encHeaderParam.buf = vbStream.phys_addr;
        encHeaderParam.size = vbStream.size;
    }


    if(encOP.bitstreamFormat == STD_AVC) {
        if (encOP.EncStdParam.avcParam.mvcExtension && encOP.EncStdParam.avcParam.parasetRefreshEn)
            goto SKIP_PUT_HEADER;

	    if (handle->CodecInfo->encInfo.max_temporal_id)
	    {
		    encHeaderParam.headerType = SVC_RBSP_SEI;
		    if (encOP.ringBufferEnable == FALSE) {
			    encHeaderParam.buf  = vbStream.phys_addr;
		    }
		    encHeaderParam.size = vbStream.size;

		    if ((ret = VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, &encHeaderParam)) != RETCODE_SUCCESS) {
			    VLOG(ERR, "VPU_EncGiveCommand ( ENC_PUT_VIDEO_HEADER ) for SPS_RBSP failed Error code is 0x%x \n", ret);
			    success = FALSE;
			    goto ERR_ENC_OPEN;
		    }

		    if (encOP.ringBufferEnable == FALSE) {
			    EnterLock(coreIdx);
			    ret = BitstreamReader_Act(bsReader, encHeaderParam.buf, encHeaderParam.size, 0, comparator);
			    LeaveLock(coreIdx);
			    if (ret == FALSE) {
				    success = FALSE;
				    goto ERR_ENC_OPEN;
			    }
		    }
	    }

        encHeaderParam.headerType = SPS_RBSP;
        if (encOP.ringBufferEnable == FALSE) {
            encHeaderParam.buf  = vbStream.phys_addr;
        }
        encHeaderParam.size = vbStream.size;

        if ((ret=VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, &encHeaderParam)) != RETCODE_SUCCESS) {
            VLOG(ERR, "VPU_EncGiveCommand ( ENC_PUT_VIDEO_HEADER ) for SPS_RBSP failed Error code is 0x%x \n", ret);
            success = FALSE;
            goto ERR_ENC_OPEN;
        }

        if (encOP.ringBufferEnable == FALSE) {
            EnterLock(coreIdx);
            ret = BitstreamReader_Act(bsReader, encHeaderParam.buf, encHeaderParam.size, 0, comparator);
            LeaveLock(coreIdx);
            if (ret == FALSE) {
                success = FALSE;
                goto ERR_ENC_OPEN;
            }
        }
        if (encOP.EncStdParam.avcParam.mvcExtension == TRUE) {
            if (encOP.ringBufferEnable == 0) {
                encHeaderParam.buf  = vbStream.phys_addr;
            }

            encHeaderParam.headerType = SPS_RBSP_MVC;
            encHeaderParam.size       = vbStream.size;
            ret = VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, &encHeaderParam);
            if (ret != RETCODE_SUCCESS) {
                VLOG(ERR, "VPU_EncGiveCommand ( ENC_PUT_VIDEO_HEADER ) for SPS_RBSP_MVC failed Error code is 0x%x \n", ret);
                success = FALSE;
                goto ERR_ENC_OPEN;
            }
            if (encOP.ringBufferEnable == FALSE) {
                EnterLock(coreIdx);
                ret = BitstreamReader_Act(bsReader, encHeaderParam.buf, encHeaderParam.size, 0, comparator);
                LeaveLock(coreIdx);
                if (ret == FALSE) {
                    success = FALSE;
                    goto ERR_ENC_OPEN;
                }
            }
        }
        encHeaderParam.headerType = PPS_RBSP;
        for (i=0; i<encOP.EncStdParam.avcParam.ppsNum; i++) {
            if (encOP.ringBufferEnable == FALSE) {
                encHeaderParam.buf  = vbStream.phys_addr;
                encHeaderParam.pBuf = (BYTE *)(uintptr_t)vbStream.virt_addr;
            }
            encHeaderParam.size = vbStream.size;
            ret = VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, &encHeaderParam);
            if (ret != RETCODE_SUCCESS) {
                VLOG(ERR, "VPU_EncGiveCommand ( ENC_PUT_VIDEO_HEADER ) for PPS_RBSP failed Error code is 0x%x \n", ret);
                success = FALSE;
                goto ERR_ENC_OPEN;
            }
            if (encOP.ringBufferEnable == FALSE) {
                EnterLock(coreIdx);
                ret = BitstreamReader_Act(bsReader, encHeaderParam.buf, encHeaderParam.size, 0, comparator);
                LeaveLock(coreIdx);
                if (ret == FALSE) {
                    success = FALSE;
                    goto ERR_ENC_OPEN;
                }
            }
        }

        if (encOP.EncStdParam.avcParam.mvcExtension) {
            encHeaderParam.headerType = PPS_RBSP_MVC;
            encHeaderParam.size = vbStream.size;
            if ((ret=VPU_EncGiveCommand(handle, ENC_PUT_VIDEO_HEADER, &encHeaderParam)) != RETCODE_SUCCESS) {
                VLOG(ERR, "VPU_EncGiveCommand ( ENC_PUT_VIDEO_HEADER ) for PPS_RBSP_MVC failed Error code is 0x%x \n", ret);
                success = FALSE;
                goto ERR_ENC_OPEN;
            }
            if (encOP.ringBufferEnable == FALSE) {
                EnterLock(coreIdx);
                ret = BitstreamReader_Act(bsReader, encHeaderParam.buf, encHeaderParam.size, 0, comparator);
                LeaveLock(coreIdx);
                if (ret == FALSE) {
                    success = FALSE;
                    goto ERR_ENC_OPEN;
                }
            }
        }
    }
    // step: load YUV
SKIP_PUT_HEADER:
    // DONE: 不需要YUV_Feeder，而是直接指定YUV的地址
    DisplayEncodedInformation(handle, encOP.bitstreamFormat, 0, NULL);
    VPU_EncGiveCommand(handle, GET_TILEDMAP_CONFIG, &mapConfig);
    /********************************************************************************
     * step: start ENCODE
     ********************************************************************************/
    while (loop == TRUE) {
        if (osal_kbhit()) {
            success = FALSE;
            break;
        }

        if (frameIdx >= pTestConfig->outNum) {
            break;
        }

	    if (encOP.EncStdParam.avcParam.svcExtension == TRUE)
            VPU_EncGiveCommand(handle, ENC_GET_FRAMEBUF_IDX, &srcFrameIdx);
	    else
            srcFrameIdx = frameIdx; // note:因爲fbSrc的數量就是縂幀數，所以無需%
            //srcFrameIdx = (frameIdx%ENC_SRC_BUF_NUM);

        // if (YuvFeeder_Feed(yuvFeeder, coreIdx, &fbSrc[srcFrameIdx], encOP.picWidth, encOP.picHeight, &mapConfig) == FALSE) {
        //     VLOG(INFO, "Read YUV done!!!\n");
        //     break;
        // }

        encParam.sourceFrame = &fbSrc[srcFrameIdx];

        fieldDone = FALSE;
FILED_ENCODE:
        if (encOP.ringBufferEnable == 0) {
            encParam.picStreamBufferAddr = vbStream.phys_addr; // can set the newly allocated buffer.
            encParam.picStreamBufferSize = vbStream.size;
        }
        encParam.sourceFrame->endian = encOP.frameEndian;
        encParam.sourceFrame->cbcrInterleave = encOP.cbcrInterleave;
        encParam.sourceFrame->sourceLBurstEn = FALSE;

        RcChangeSetParameter(handle, *pTestConfig, pTestArgV, &encOP, &encParam, frameIdx, fieldDone, cfgParam, vbRoi);

        // Start encoding a frame.
#ifdef AUTO_FRM_SKIP_DROP
        isDroppedPic = FALSE;
#endif
        ret=VPU_EncStartOneFrame(handle, &encParam);
#ifdef AUTO_FRM_SKIP_DROP
        if (RETCODE_FRAME_DROP == ret) {
            VLOG(INFO, "%02d %5d Current frame will be dropped \n", coreIdx, frameIdx);
            isDroppedPic = TRUE;
        }
#endif /* AUTO_FRM_SKIP_DROP */
        else if ( ret != RETCODE_SUCCESS ) {
            VLOG(ERR, "VPU_EncStartOneFrame failed Error code is 0x%x \n", ret );
            success = FALSE;
            goto ERR_ENC_OPEN;
        }


#ifdef AUTO_FRM_SKIP_DROP
        if(FALSE == isDroppedPic) {
#else
        {
#endif /* AUTO_FRM_SKIP_DROP */

            timeoutCount = 0;
            while (timeoutCount < VPU_ENC_TIMEOUT) {
                interruptFlag = VPU_WaitInterrupt(coreIdx, VPU_WAIT_TIME_OUT);
                if (interruptFlag == (Int32)-1) {
                    if (timeoutCount*VPU_WAIT_TIME_OUT > VPU_ENC_TIMEOUT) {
                        VLOG(ERR, "Error : encoder timeout happened\n");
                        VPU_SWReset(coreIdx, SW_RESET_SAFETY, handle);
                        success = FALSE;
                        break;
                    }
                    interruptFlag = 0;
                    timeoutCount++;
                }

                if (success == TRUE) {
                    if (encOP.ringBufferEnable == TRUE) {
                        ret = BitstreamReader_Act(bsReader, encOP.bitstreamBuffer, encOP.bitstreamBufferSize, DEFAULT_READ_SIZE, comparator);
                        if (ret == FALSE) {
                            Int32 room;
                            PhysicalAddress rdPtr, wrPtr;
                            success = FALSE;
                            /* Flush bitstream buffer */
                            VPU_EncGetBitstreamBuffer(handle, &rdPtr, &wrPtr, &room);
                            VPU_EncUpdateBitstreamBuffer(handle, room);
                        }
                    }
                    else {
                        // Linebuffer is Full interrupt when linebuffer interrupt mode is set to 1
                        if (interruptFlag & (1<<INT_BIT_BIT_BUF_FULL)) {
                            ret = BitstreamReader_Act(bsReader, encOP.bitstreamBuffer, encOP.bitstreamBufferSize, 0, comparator);
                            if (ret == FALSE) {
                                VPU_ClearInterrupt(coreIdx);
                                VPU_EncGetOutputInfo(handle, &outputInfo);
                                success = FALSE;
                                goto ERR_ENC_OPEN;
                            }
                        }
                    }

                }

                if (interruptFlag > 0) {
                    VPU_ClearInterrupt(coreIdx);
                    if (interruptFlag & (1<<INT_BIT_PIC_RUN)) {
                        break;
                    }
                }
            }
        }

        ret = VPU_EncGetOutputInfo(handle, &outputInfo);
        if (ret != RETCODE_SUCCESS) {
            VLOG(ERR, "VPU_EncGetOutputInfo failed Error code is 0x%x \n", ret );
            success = FALSE;
            goto ERR_ENC_OPEN;
        }

        /* mismatch in WaitInterrupt loop */
        if (success == FALSE) {
            goto ERR_ENC_OPEN;
        }

#ifdef AUTO_FRM_SKIP_DROP
        if (outputInfo.picDropped) {
            VLOG(INFO, "%02d %5d >> dropped frame \n", coreIdx, frameIdx);
            frameIdx++;
            //Back to Execution procedure
            continue;
        }
#endif /* AUTO_FRM_SKIP_DROP */

        DisplayEncodedInformation(handle, encOP.bitstreamFormat, frameIdx, &outputInfo);

        if (encOP.ringBufferEnable == FALSE) {
            if (outputInfo.bitstreamWrapAround == TRUE) {
                // If LineBuffer interrupt is set to 1, it is ok to work.
                VLOG(WARN, "Warning!! BitStream buffer wrap-arounded. prepare more large buffer. Consumed all remained stream\n");
            }
            if (outputInfo.bitstreamSize == 0) {
                VLOG(ERR, "bitstreamsize = 0 \n");
            }

            EnterLock(coreIdx);
            ret = BitstreamReader_Act(bsReader, encOP.bitstreamBuffer, encOP.bitstreamBufferSize, 0, comparator);
            LeaveLock(coreIdx);
            if (ret == FALSE) {
                success = FALSE;
                break;
            }
        }

        if (encParam.fieldRun && fieldDone == FALSE) {
            fieldDone = TRUE;
            goto FILED_ENCODE;
        }
        frameIdx++;
        if (frameIdx > (pTestConfig->outNum-1))
            break;
    }

    if (encOP.ringBufferEnable == TRUE) {
        EnterLock(coreIdx);
        ret = BitstreamReader_Act(bsReader, encOP.bitstreamBuffer, encOP.bitstreamBufferSize, 0, comparator);
        LeaveLock(coreIdx);
        if (ret == FALSE) {
            VLOG(ERR, "ReadBsRingBufHelper failed Error code is 0x%x \n", ret );
            success = FALSE;
            goto ERR_ENC_OPEN;
        }
    }

    if (isForceOutNum == FALSE) {
        if (Comparator_CheckEOF(comparator) == FALSE) {
            VLOG(ERR, "MISMATCH BitStream data size. There is still data to compare.\n");
            goto ERR_ENC_OPEN;
        }
    }

ERR_ENC_OPEN:
    // Now that we are done with encoding, close the open instance.
    VPU_EncClose(handle);

    VLOG(INFO, "\nEnc End. Tot Frame %d\n" , frameIdx);

ERR_ENC_INIT:
    for (i=0; i<regFrameBufCount; i++) {
        if (vbReconFrameMem[i].size > 0) vdi_free_dma_memory(coreIdx, &vbReconFrameMem[i]);
    }
    // for (i=0; i<ENC_SRC_BUF_NUM; i++) {
    //     if (vbSrcFrameMem[i].size > 0 ) vdi_free_dma_memory(coreIdx, &vbSrcFrameMem[i]);
    // }
    vdi_free_dma_memory(coreIdx, &vbRoi);
    vdi_free_dma_memory(coreIdx, &vbStream);
    if (bsReader!= NULL)   BitstreamReader_Destroy(bsReader);
    if (yuvFeeder != NULL) YuvFeeder_Destroy(yuvFeeder);

    if (VPU_DeInit(coreIdx) != RETCODE_SUCCESS) {
        success = FALSE;
    }

    return success;
}

#if 0
Int32 coda988_test(Int32 argc, char **argv)
{
    Int32           opt, index;
    BOOL            ret;
    TestEncConfig   encConfig;
    struct OptionExt longOptExt[] = {
        {"output",                1, NULL, 0, "--output                    bitstream pathn"},
        {"codec",                 1, NULL, 0, "--codec                     codec index, HEVC: 12n"},
        {"cfg-path",              1, NULL, 0, "--cfgFileName               cfg file path"},
        {"coreIdx",               1, NULL, 0, "--coreIdx                   core index: default 0"},
        {"yuv-base",              1, NULL, 0, "--yuv-base=DIR              YUV directory."},
        {"enable-ringBuffer",     0, NULL, 0, "--enable-ringBuffer         enable stream ring buffer mode"},
        {"enable-lineBufInt",     0, NULL, 0, "--enable-lineBufInt         enable linebuffer interrupt"},
        {"maptype",               1, NULL, 0, "--maptype                   mapType"},
        {"enable-cbcrInterleave", 0, NULL, 0, "--enable-cbcrInterleave     enable cbcr interleave"},
        {"enable-nv21",           0, NULL, 0, "--enable-nv21               enable NV21(must set enable-cbcrInterleave)"},
        {"enable-linear2tiled",   0, NULL, 0, "--enable-linear2tiled       If this option is enabled, linear source YUV will be converted tiled format depending on --maptyp" },
        {"rotate",                1, NULL, 0, "--rotate                    rotation angle(0,90,180,270)" },
        {"mirror",                1, NULL, 0, "--mirror                    1:Vertical, 2: Horizontal, 3:Vert-Horz" },
        {"secondary-axi",         1, NULL, 0, "--secondary-axi             0~63: bit oring values, Please refer programmer's guide or datasheet" },
        {"frame-endian",          1, NULL, 0, "--frame-endian              0~3, default 0(BE) Please refer programmer's guide or datasheet" },
        {"stream-endian",         1, NULL, 0, "--stream-endian             0~3, default 0(BE) Please refer programmer's guide or datasheet" },
        {"ref_stream_path",       1, NULL, 0, "--ref_stream_path           golden data which is compared with encoded stream when -c option"},
        {"enable-mvc",            0, NULL, 0, "--enable-mvc                enable H.264 MVC" },
        {"enable-svc", 0, NULL, 0, "--enable-svc                enable H.264 SVC-T" },
        {NULL,                    0, NULL, 0, NULL },
    };
    struct option*  options;
    BOOL            showVersion = FALSE;
    char*           optString   = "c:hn:v";

    InitLog();

    //default setting.
    osal_memset((void*)&encConfig, 0, sizeof(encConfig));
    encConfig.stdMode       = STD_AVC;
    encConfig.frame_endian  = VDI_LITTLE_ENDIAN;
    encConfig.stream_endian = VDI_LITTLE_ENDIAN;
    encConfig.ringBufferEnable  = FALSE;
    sprintf(encConfig.bitstreamFileName, "output.bin");
    sprintf(encConfig.yuvSourceBaseDir, "./");

    options = ConvertOptions(longOptExt, sizeof(longOptExt)/sizeof(struct OptionExt));

    while ((opt=getopt_long(argc, argv, optString, options, &index)) != -1) {
        switch (opt) {
        case 'c':
            encConfig.compare_type = (TRUE == (BOOL)atoi(optarg)) ? STREAM_COMPARE : NO_COMPARE;
            break;
        case 'h':
            Help(argv[0]);
            return 0;
            break;
        case 'n':
            encConfig.outNum = atoi(optarg);
            break;
        case 'v':
            showVersion = TRUE;
            break;
        case 0:
            if (strcmp(options[index].name, "output") == 0) {
                strcpy(encConfig.bitstreamFileName, optarg);
                ChangePathStyle(encConfig.bitstreamFileName);
            }
            else if (strcmp(options[index].name, "codec") == 0) {
                index = atoi(optarg);
                if (index != (Int32)STD_AVC && index != (Int32)STD_MPEG4 && index != (Int32)STD_H263) {
                    VLOG(ERR, "Invalid codec index(%d)\n", index);
                    Help(argv[0]);
                    exit(1);
                }
                encConfig.stdMode = (CodStd)atoi(optarg);
            }
            else if (strcmp(options[index].name, "cfg-path") == 0) {
                memcpy(encConfig.cfgFileName, optarg, strlen(optarg));
                ChangePathStyle(encConfig.cfgFileName);
            }
            else if (strcmp(options[index].name, "coreIdx") == 0) {
                encConfig.coreIdx = atoi(optarg);
            }
            else if (strcmp(options[index].name, "yuv-base") == 0) {
                strcpy(encConfig.yuvSourceBaseDir, optarg);
                ChangePathStyle(encConfig.yuvSourceBaseDir);
            }
            else if (strcmp(options[index].name, "enable-ringBuffer") == 0) {
                encConfig.ringBufferEnable = TRUE;
            }
            else if (strcmp(options[index].name, "enable-lineBufInt") == 0) {
                encConfig.lineBufIntEn = TRUE;
            }
            else if (strcmp(options[index].name, "maptype") == 0) {
                encConfig.mapType = atoi(optarg);
            }
            else if (strcmp(options[index].name, "enable-cbcrInterleave") == 0) {
                encConfig.cbcrInterleave = TRUE;
            }
            else if (strcmp(options[index].name, "enable-nv21") == 0) {
                encConfig.nv21 = TRUE;
            }
            else if (strcmp(options[index].name, "enable-linear2tiled") == 0) {
                encConfig.coda9.enableLinear2Tiled = TRUE;
                encConfig.coda9.linear2TiledMode   = FF_FRAME;
            }
            else if (strcmp(options[index].name, "rotate") == 0) {
                encConfig.rotAngle = atoi(optarg);
            }
            else if (strcmp(options[index].name, "mirror") == 0) {
                encConfig.mirDir = atoi(optarg);
            }
            else if (strcmp(options[index].name, "secondary-axi") == 0) {
                encConfig.secondary_axi = atoi(optarg);
            }
            else if (strcmp(options[index].name, "frame-endian") == 0) {
                encConfig.frame_endian = atoi(optarg);
            }
            else if (strcmp(options[index].name, "stream-endian") == 0) {
                encConfig.stream_endian = atoi(optarg);
            }
            else if (strcmp(options[index].name, "ref_stream_path") == 0) {
                memcpy(encConfig.ref_stream_path, optarg, strlen(optarg));
                ChangePathStyle(encConfig.ref_stream_path);
            }
            else if (strcmp(options[index].name, "enable-mvc") == 0) {
                encConfig.coda9.enableMvc = TRUE;
            }
	        else if (strcmp(options[index].name, "enable-svc") == 0) {
		        encConfig.coda9.enableSvc = TRUE;
	        }
            break;
        case '?':
            Help(argv[0]);
            exit(1);
            break;
        default:
            VLOG(ERR, "%s\n", optarg);
            Help(argv[0]);
            exit(1);
        }
    }
    osal_free(options);


    if (LoadFirmware(PRODUCT_ID_980, (Uint8**)&pusBitCode, &sizeInWord, CORE_1_BIT_CODE_FILE_PATH) < 0) {
        VLOG(ERR, "%s:%d Failed to load firmware: %s\n", __FUNCTION__, __LINE__, CORE_1_BIT_CODE_FILE_PATH);
        return 1;
    }

    if (showVersion) {
        Uint32   ver, rev;
        Uint32   productId;

        ret = VPU_InitWithBitcode(encConfig.coreIdx, (const Uint16*)pusBitCode, sizeInWord);
        if (ret != RETCODE_CALLED_BEFORE && ret != RETCODE_SUCCESS) {
            VLOG(ERR, "Failed to boot up VPU(coreIdx: %d)\n", __FUNCTION__, __LINE__, encConfig.coreIdx);
            return 1;
        }
        VPU_GetVersionInfo(encConfig.coreIdx, &ver, &rev, &productId);
        VPU_DeInit(encConfig.coreIdx);
        printf("VERSION=%x\n", ver);
        printf("REVISION=%d\n", rev);
        printf("PRODUCT_ID=%d\n", productId);
        return 0;
    }

    if (encConfig.mapType == TILED_FRAME_MB_RASTER_MAP || encConfig.mapType == TILED_FIELD_MB_RASTER_MAP) {
        encConfig.cbcrInterleave = TRUE;
    }

    osal_init_keyboard();

    if (encConfig.rotAngle > 0 || encConfig.mirDir > 0) {
        encConfig.useRot = TRUE;
    }

    ret = TestEncoder(&encConfig);

    osal_close_keyboard();
    osal_free(pusBitCode);

    if (ret == FALSE) {
        VLOG(ERR, "[RESULT] FAILURE\n");
    }

    return ret == TRUE ? 0 : 1;
}
#endif

Int32 coda988_test(TestArgV_t *argv)
{
    TestEncConfig   encConfig;
    Int32           opt, index;
    BOOL            ret;
    int             showVersion = 0;
    char*           optString   = "c:hn:v";
    unsigned int*   top_reg_28 = (unsigned int *)0x0B030028;


    InitLog();

    //default setting.
    osal_memset((void*)&encConfig, 0, sizeof(encConfig));
    encConfig.stdMode       = STD_AVC;
    encConfig.frame_endian  = VDI_LITTLE_ENDIAN;
    encConfig.stream_endian = VDI_LITTLE_ENDIAN;
    encConfig.ringBufferEnable  = TRUE;
    encConfig.lineBufIntEn = TRUE; // sync from v410
    encConfig.secondary_axi = 0x0; // note: 0xf is from v410 sdk, here we close all, do not use shared ram
    encConfig.mapType = TILED_FRAME_V_MAP; // sync from v410, origin is LINEAR_FRAME_MAP;
    sprintf(encConfig.bitstreamFileName, "output.bin");
    sprintf(encConfig.yuvSourceBaseDir, "./");

    encConfig.outNum = argv->n;
    showVersion = argv->showVersion;
    encConfig.coreIdx = 0; // ! coreIdx in vdi should be verified.
    printf("C906B, int size = %d, long size = %d, long long size = %d\n", sizeof(int), sizeof(long), sizeof(long long));
    //printf("output num: %d, showVersion: %d\n", encConfig.outNum, showVersion);
    printf("set top reg 0x0x0B030028 to 0x7\n");
    *(volatile unsigned int *)top_reg_28 = 0x7; // note: sync from v410


    // if (LoadFirmware(PRODUCT_ID_980, (Uint8**)&pusBitCode, &sizeInWord, CORE_1_BIT_CODE_FILE_PATH) < 0) {
    // if (LoadFirmware(PRODUCT_ID_410, (Uint8**)&pusBitCode, &sizeInWord, "/mnt/nfs/cv182/monet.bin") < 0) {
    // if (LoadFirmware_EX(encConfig.coreIdx, (Uint8**)&pusBitCode, &sizeInWord) < 0) {
    //     VLOG(ERR, "%s:%d Failed to load firmware file!\n", __FUNCTION__, __LINE__);
    //     return 1;
    // }
    //printf("LoadFirmware success, bitcode size: %d\n", sizeInWord);
    Uint8 *firmware8 = NULL;
    if (LoadFirmwareH(PRODUCT_ID_980, (Uint8**)&firmware8, &sizeInWord) < 0) {
        VLOG(ERR, "%s:%d Failed to load firmware: %s\n", __FUNCTION__, __LINE__, CORE_0_BIT_CODE_FILE_PATH);
        return 1;
    }
    pusBitCode = (Uint16*)firmware8;
    firmware8 = NULL;

    if (showVersion) {
        Uint32   ver, rev;
        Uint32   productId;

        ret = VPU_InitWithBitcode(encConfig.coreIdx, (const Uint16*)pusBitCode, sizeInWord);
        //printf("VPU_InitWithBitcode ret: %d\n", ret);
        if (ret != RETCODE_CALLED_BEFORE && ret != RETCODE_SUCCESS) {
            VLOG(ERR, "Failed to boot up VPU(coreIdx: %d)\n", __FUNCTION__, __LINE__, encConfig.coreIdx);
            return 1;
        }
        //printf("VPU_InitWithBitcode success\n");
        VPU_GetVersionInfo(encConfig.coreIdx, &ver, &rev, &productId);
        VPU_DeInit(encConfig.coreIdx);
        printf("VERSION=%x\n", ver);
        printf("REVISION=%d\n", rev);
        printf("PRODUCT_ID=%d\n", productId);
        return 0;
    }

    if (encConfig.mapType == TILED_FRAME_MB_RASTER_MAP || encConfig.mapType == TILED_FIELD_MB_RASTER_MAP) {
        encConfig.cbcrInterleave = TRUE;
    }

    osal_init_keyboard();

    if (encConfig.rotAngle > 0 || encConfig.mirDir > 0) {
        encConfig.useRot = TRUE;
    }
    printf("######## Start TestEncoder ########\n");
    ret = TestEncoder(&encConfig, argv);
    printf("######## End TestEncoder ########\n");

    osal_close_keyboard();
    osal_free(pusBitCode);

    if (ret == FALSE) {
        VLOG(ERR, "[RESULT] FAILURE\n");
    }

    return ret == TRUE ? 0 : 1;
}

