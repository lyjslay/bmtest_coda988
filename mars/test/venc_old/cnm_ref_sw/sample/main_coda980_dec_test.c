//--=========================================================================--
//  This file is a part of VPUAPI
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

#define STREAM_BUF_SIZE                 0x700000    // max bitstream size
#define MAX_COUNT_NO_RESPONSE           3           // For integration test.
#define PPU_FB_COUNT                    2
#define EXTRA_FRAME_BUFFER_NUM          1

static Uint32             sizeInWord;
static Uint16*            pusBitCode;


static void 
Help(
    const char *programName
    )
{
    VLOG(INFO, "------------------------------------------------------------------------------\n");
    VLOG(INFO, "%s(API v%d.%d.%d)\n", programName, API_VERSION_MAJOR, API_VERSION_MINOR, API_VERSION_PATCH);
    VLOG(INFO, "\tAll rights reserved by Chips&Media(C)\n");
    VLOG(INFO, "------------------------------------------------------------------------------\n");
    VLOG(INFO, "%s [option] --input bistream\n", programName);
    VLOG(INFO, "-h                          help\n");
    VLOG(INFO, "-n [num]                    output frame number\n");
    VLOG(INFO, "-v                          print version information\n");
    VLOG(INFO, "-c                          compare with golden\n");
    VLOG(INFO, "                            0 : no comparison\n");
    VLOG(INFO, "                            1 : compare with golden yuv that specified --ref-yuv option\n");
    VLOG(INFO, "-u                          Enable userdata\n");
    VLOG(INFO, "--input                     bitstream path\n");
    VLOG(INFO, "--output                    YUV path\n");
    VLOG(INFO, "--codec                     The index of codec (H.264:0, VC1:1, MP2:2, MP4:3, H263:4 DIV3:5 RV:6 AVS:7 THO:9 VP8:11)\n");
    VLOG(INFO, "                            default 0(H.264)\n");
    VLOG(INFO, "--bsmode                    0: INTERRUPT MODE, 1: reserved, 2: PICEND MODE\n");
    VLOG(INFO, "--coreIdx                   core index: default 0\n");
    VLOG(INFO, "--loop-count                integer number. loop test, default 0\n");
    VLOG(INFO, "--stream-endian             0~3, default 0(LE) Please refer programmer's guide or datasheet\n");
    VLOG(INFO, "--frame-endian              0~3, default 0(LE) Please refer programmer's guide or datasheet\n");
    VLOG(INFO, "--enable-cbcrinterleave     enable cbcrInterleave(NV12), default off\n");
    VLOG(INFO, "--enable-nv21               enable NV21, default off\n");
    VLOG(INFO, "--secondary-axi             0~63: bit oring values, Please refer programmer's guide or datasheet\n");
    VLOG(INFO, "--rotate                    90, 180, 270\n");
    VLOG(INFO, "--mirror                    0: none, 1: vertical, 2: horizontal, 3: both\n");
    VLOG(INFO, "--enable-dering             MPEG-2, MPEG-4 only\n");
    VLOG(INFO, "--enable-deblock            MPEG-2, MPEG-4 only\n");
    VLOG(INFO, "--mp4class                  0: MPEG-4, 1: divx, 2: xvid, 5: divx4 256: sorenson spark\n");
    VLOG(INFO, "--maptype                   CODA960: 0~6, CODA980: 0~9, default 0\n");
    VLOG(INFO, "                            Please refer TiledMapType in vpuapi.h\n");
    VLOG(INFO, "--enable-tiled2linear       enable tiled2linear. default off\n");
    VLOG(INFO, "--enable-wtl                enable WTL. default off\n");
    VLOG(INFO, "--enable-mvc                enable H.264 MVC, default off\n");
    VLOG(INFO, "--low-delay   mbrows        enable low delay option with the given parameter mbrows. H.264\n");
    VLOG(INFO, "--render                    0 : no rendering picture\n"
               "                            1 : render a picture with the framebuffer device\n");
    VLOG(INFO, "--ref-yuv                   golden yuv path\n");
    VLOG(INFO, "--tid N                     target temporal id (0..2)\n");
}

BOOL TestDecoder(TestDecConfig *param)
{
    DecHandle           handle = NULL;
    DecOpenParam        decOP;
    DecInitialInfo      sequenceInfo;
    DecOutputInfo       outputInfo, *pDisplayInfo;
    DecParam            decParam;
    vpu_buffer_t        vbStream;
    SecAxiUse           secAxiUse;
    RetCode             ret                 = RETCODE_SUCCESS;
    Int32               framebufHeight      = 0;
    BOOL                success             = TRUE;
    BOOL                doDumpImage         = FALSE;
    Int32               fbCount;
    Int32               ppuFbCount          = PPU_FB_COUNT;
    Int32               decodedIdx          = 0;
    Int32               index;
    BOOL                repeat              = TRUE;
    BOOL                seqInited           = FALSE;
    BOOL                seqInitEscape       = FALSE;
    Int32               timeoutCount = 0;
    Int32               interruptFlag = 0;
    Int32               instIdx=0, coreIdx;
    FrameBuffer         pFrame[MAX_REG_FRAME];
    vpu_buffer_t        pFbMem[MAX_REG_FRAME];
    FrameBuffer*        ppuFb;
    FrameBuffer         pPPUFrame[MAX_REG_FRAME];
    vpu_buffer_t        pPPUFbMem[MAX_REG_FRAME];
    vpu_buffer_t*       pvb     = NULL;
    Int32               productId, prevFbIndex;
    Uint32              framebufStride, framebufSize;
    Int32               nWritten;
    BSFeeder            feeder                  = NULL;
    Renderer            renderer                = NULL;
    RenderDeviceType    rendererType            = param->renderType;
    FILE*               saveFp                  = NULL;
    Comparator          comparator              = NULL;
    char*               goldenPath              = NULL;
    BOOL                enablePPU               = FALSE;
    BOOL                waitPostProcessing      = FALSE;
    BOOL                needStream              = FALSE;
    BOOL                seqChangeRequest        = FALSE;
    Int32               seqChangedRdPtr         = 0;
    Int32               seqChangedWrPtr         = 0;
    Int32               seqChangedStreamEndFlag = 0;
    Uint32              noResponseCount         = MAX_COUNT_NO_RESPONSE;
    Queue*              ppuQ                    = NULL;
    VpuRect             rcPpu;
    BOOL                assertedFieldDoneInt    = FALSE;
    FrameBufferAllocInfo fbAllocInfo;
    MaverickCacheConfig cacheCfg;
    Uint32              noFbCount               = 0;
    Uint32              dispIdx                 = 0;
    VpuReportConfig_t   decReportCfg;

    osal_memset(&decOP,         0x00, sizeof(DecOpenParam));
    osal_memset(&sequenceInfo,  0x00, sizeof(DecInitialInfo));
    osal_memset(&outputInfo,    0x00, sizeof(DecOutputInfo));
    osal_memset(&decParam,      0x00, sizeof(DecParam));
    osal_memset(&vbStream,      0x00, sizeof(vpu_buffer_t));
    osal_memset(&secAxiUse,     0x00, sizeof(SecAxiUse));
    osal_memset(&pFbMem,        0x00, sizeof(vpu_buffer_t)*MAX_REG_FRAME);
    osal_memset(&pPPUFbMem,     0x00, sizeof(vpu_buffer_t)*MAX_REG_FRAME);

    param->enableCrop = TRUE;
    coreIdx = param->coreIdx;
    instIdx = param->instIdx;

    if (strlen(param->outputPath) > 0) {
        if ((saveFp=fopen(param->outputPath, "wb")) == NULL) {
            VLOG(ERR, "%s:%d failed to open %s\n", __FUNCTION__, __LINE__);
            goto ERR_DEC_INIT;
        }
    }

    if ((productId=VPU_GetProductId(coreIdx)) == -1) {
        VLOG(ERR, "Failed to get product ID\n");
        success=FALSE;
        goto ERR_DEC_INIT;
    }

    ret = VPU_InitWithBitcode(coreIdx, (const Uint16*)pusBitCode, sizeInWord);
    if (ret != RETCODE_CALLED_BEFORE && ret != RETCODE_SUCCESS) {
        VLOG(ERR, "Failed to boot up VPU(coreIdx: %d, productId: %d)\n", coreIdx, productId);
        goto ERR_DEC_INIT;
    }
    PrintVpuVersionInfo(coreIdx);

    vbStream.size = STREAM_BUF_SIZE;
    if (vdi_allocate_dma_memory(coreIdx, &vbStream) < 0) {
        VLOG(ERR, "fail to allocate bitstream buffer\n" );
        success=FALSE;
        goto ERR_DEC_INIT;
    }

    if (param->bitstreamMode == BS_MODE_PIC_END || param->bitFormat == STD_THO) {
        CodStd      codecId;
        Uint32    mp4Id;
        feeder=BitstreamFeeder_Create(param->inputPath, FEEDING_METHOD_FRAME_SIZE, vbStream.virt_addr, vbStream.size, param->bitFormat, &codecId, &mp4Id, NULL, NULL);
        param->bitFormat      = codecId;
        param->coda9.mp4class = mp4Id;
    }
    else {
        feeder=BitstreamFeeder_Create(param->inputPath, FEEDING_METHOD_FIXED_SIZE, vbStream.virt_addr, vbStream.size, 0x2000);
    }
    if (feeder == NULL) {
        success = FALSE;
        goto DECODE_END;
    }

    /* set up decoder configurations */
    decOP.bitstreamFormat     = (CodStd)param->bitFormat;
    decOP.avcExtension        = param->coda9.enableMvc;
    decOP.coreIdx             = coreIdx;
    decOP.bitstreamBuffer     = vbStream.phys_addr;
    decOP.bitstreamBufferSize = vbStream.size; 
    decOP.bitstreamMode       = param->bitstreamMode;
    decOP.tiled2LinearEnable  = param->coda9.enableTiled2Linear;
    decOP.tiled2LinearMode    = param->coda9.tiled2LinearMode;
    decOP.wtlEnable           = param->enableWTL;
    decOP.wtlMode             = param->wtlMode;
    decOP.cbcrInterleave      = param->cbcrInterleave;
    decOP.nv21                = param->nv21;
    decOP.streamEndian        = param->streamEndian;
    decOP.frameEndian         = param->frameEndian;
    decOP.bwbEnable           = param->coda9.enableBWB;
    decOP.mp4DeblkEnable      = param->coda9.enableDeblock;
    decOP.mp4Class            = param->coda9.mp4class;

    VLOG(INFO, "------------------------------ DECODER OPTIONS ------------------------------\n");
    VLOG(INFO, "[coreIdx            ]: %d\n", decOP.coreIdx            );
    VLOG(INFO, "[mapType            ]: %d\n", param->mapType           );
    VLOG(INFO, "[tiled2linear       ]: %d\n", param->coda9.enableTiled2Linear);
    VLOG(INFO, "[wtlEnable          ]: %d\n", decOP.wtlEnable          );
    VLOG(INFO, "[wtlMode            ]: %d\n", decOP.wtlMode            );
    VLOG(INFO, "[bitstreamBuffer    ]: 0x%08x\n", decOP.bitstreamBuffer);
    VLOG(INFO, "[bitstreamBufferSize]: %d\n", decOP.bitstreamBufferSize);
    VLOG(INFO, "[bitstreamMode      ]: %d\n", decOP.bitstreamMode      );
    VLOG(INFO, "[cbcrInterleave     ]: %d\n", decOP.cbcrInterleave     );
    VLOG(INFO, "[nv21               ]: %d\n", decOP.nv21               );
    VLOG(INFO, "[streamEndian       ]: %d\n", decOP.streamEndian       );
    VLOG(INFO, "[frameEndian        ]: %d\n", decOP.frameEndian        );
    VLOG(INFO, "[BWB                ]: %d\n", decOP.bwbEnable          );
    VLOG(WARN, "-----------------------------------------------------------------------------\n");

    /********************************************************************************
     * CREATE INSTANCE                                                              *
     ********************************************************************************/
    if ((ret=VPU_DecOpen(&handle, &decOP)) != RETCODE_SUCCESS) {
        success = FALSE;
        VLOG(ERR, "VPU_DecOpen failed Error code is 0x%x \n", ret );
        goto DECODE_END;
    }

    if (decOP.bitstreamFormat == STD_AVC) {
        ret = VPU_DecGiveCommand(handle, SET_LOW_DELAY_CONFIG, &param->coda9.lowDelay);
        if (ret != RETCODE_SUCCESS) {
            VLOG(ERR, "VPU_DecGiveCommand[SET_LOW_DELAY_CONFIG] failed Error code is 0x%x \n", ret );
            goto DECODE_END;
        }
    }

    osal_memset((void*)&decReportCfg, 0x00, sizeof(VpuReportConfig_t));
    decReportCfg.userDataEnable     = param->enableUserData;
    decReportCfg.userDataReportMode = 1;
    OpenDecReport(coreIdx, &decReportCfg);
    ConfigDecReport(coreIdx, handle, decOP.bitstreamFormat);

    //VPU_DecGiveCommand(handle, ENABLE_LOGGING, 0);
    if ((renderer=SimpleRenderer_Create(handle, rendererType, param->outputPath)) == NULL) {
        success = FALSE;
        goto DECODE_END;
    }

    if ((nWritten=BitstreamFeeder_Act(handle, feeder, decOP.streamEndian, NULL)) < 0) {
        success = FALSE;
        goto DECODE_END;
    }

    if (decOP.bitstreamMode == BS_MODE_PIC_END) {
        if (decOP.bitstreamFormat != STD_THO && decOP.bitstreamFormat != STD_H263 && decOP.bitstreamFormat != STD_RV) {
            // Need first picture. In case of Theora, ffmpeg returns a coded frame including sequence header.
            BitstreamFeeder_Act(handle, feeder, decOP.streamEndian, NULL);
        }
    }

    /********************************************************************************
     * INIT_SEQ                                                                     *
     ********************************************************************************/
INIT_SEQ:

    seqInitEscape = FALSE;
    ret = VPU_DecSetEscSeqInit(handle, seqInitEscape);
    if (ret != RETCODE_SUCCESS) {
        seqInitEscape = 0;
        VLOG(WARN, "Wanning! can not to set seqInitEscape in the current bitstream mode Option \n");		
    }

    if(seqInitEscape) {
        // if you set to seqInitEscape option to TRUE, It is more easy to control that APP uses VPU_DecGetInitialInfo instead of VPU_DecIssueSeqInit and VPU_DecCompleteSeqInit
        ret = VPU_DecGetInitialInfo(handle, &sequenceInfo);
        if (ret != RETCODE_SUCCESS) 
        {
            VLOG(ERR, "VPU_DecGetInitialInfo failed Error code is 0x%x \n", ret);
            VLOG(ERR, "Error reason is 0x%x\n", sequenceInfo.seqInitErrReason);
            success = FALSE;
            goto DECODE_END;	
        }
    }
    else
    {
        // Scan and retrieve sequence header information from the bitstream buffer.
        if ((ret=VPU_DecIssueSeqInit(handle)) != RETCODE_SUCCESS) {
            VLOG(ERR, "VPU_DecIssueSeqInit failed Error code is 0x%x \n", ret);
            success = FALSE;
            goto DECODE_END;
        }
        timeoutCount = 0;
        while (seqInited == FALSE) { 
            if (osal_kbhit()) {
                break;
            }
            interruptFlag = VPU_WaitInterrupt(coreIdx, VPU_WAIT_TIME_OUT);		//wait for 10ms to save stream filling time.
            if (interruptFlag == -1) {
                if (timeoutCount*VPU_WAIT_TIME_OUT > VPU_DEC_TIMEOUT) {
                    VLOG(ERR, "\n VPU interrupt wait timeout\n");
                    VPU_SWReset(coreIdx, SW_RESET_SAFETY, handle);
                    success = FALSE;
                    goto DECODE_END;
                }
                timeoutCount++;
                interruptFlag = 0;
            }

            if (interruptFlag) {
                VPU_ClearInterrupt(coreIdx);
                if (interruptFlag & (1<<INT_BIT_SEQ_INIT)) {
                    seqInited = TRUE;
                    break;
                }
            }

            if (decOP.bitstreamMode != BS_MODE_PIC_END) {
                if ((nWritten=BitstreamFeeder_Act(handle, feeder, decOP.streamEndian, NULL)) < 0) {
                    success = FALSE;
                    goto DECODE_END;
                }
            }
        }

        if ((ret=VPU_DecCompleteSeqInit(handle, &sequenceInfo)) != RETCODE_SUCCESS) {
                VLOG(ERR, "[ERROR] Failed to SEQ_INIT(ERROR REASON: %d(0x%x)\n", sequenceInfo.seqInitErrReason, sequenceInfo.seqInitErrReason);
            success = FALSE;
            goto DECODE_END;
        }
        if (seqInited == FALSE) {
            success = FALSE;
            goto DECODE_END;
        }
    }


    PrintDecSeqWarningMessages(productId, &sequenceInfo);

    if (comparator == NULL) {
        switch (param->compareType) {
        case NO_COMPARE:  
            goldenPath = NULL; 
            comparator = Comparator_Create(param->compareType, goldenPath);
            break;
        case YUV_COMPARE: 
            goldenPath = param->refYuvPath; 
            comparator = Comparator_Create(param->compareType, goldenPath,
                                           VPU_ALIGN16(sequenceInfo.picWidth), VPU_ALIGN16(sequenceInfo.picHeight), 
                                           param->wtlFormat, param->cbcrInterleave);
            break;
        default:
            success = FALSE;
            goto DECODE_END;
        }
    }

    if (comparator == NULL) {
        success = FALSE;
        goto DECODE_END;
    }

    /********************************************************************************
     * ALLOCATE RECON FRAMEBUFFERS                                                  *
     ********************************************************************************/
    framebufHeight = VPU_ALIGN32(sequenceInfo.picHeight);
    framebufStride = CalcStride(sequenceInfo.picWidth, sequenceInfo.picHeight, FORMAT_420, decOP.cbcrInterleave, param->mapType, FALSE);
    framebufSize   = VPU_GetFrameBufSize(coreIdx, framebufStride, framebufHeight, param->mapType, FORMAT_420, decOP.cbcrInterleave, NULL);
    fbCount        = sequenceInfo.minFrameBufferCount + EXTRA_FRAME_BUFFER_NUM; 

    osal_memset((void*)&fbAllocInfo, 0x00, sizeof(fbAllocInfo));
    osal_memset((void*)pFrame, 0x00, sizeof(FrameBuffer)*fbCount);
    fbAllocInfo.format          = FORMAT_420;
    fbAllocInfo.cbcrInterleave  = decOP.cbcrInterleave;
    fbAllocInfo.mapType         = param->mapType;
    fbAllocInfo.stride          = framebufStride;
    fbAllocInfo.height          = framebufHeight;
    fbAllocInfo.lumaBitDepth    = sequenceInfo.lumaBitdepth;
    fbAllocInfo.chromaBitDepth  = sequenceInfo.chromaBitdepth;
    fbAllocInfo.num             = fbCount;
    fbAllocInfo.endian          = decOP.frameEndian;
    fbAllocInfo.type            = FB_TYPE_CODEC;
    for (index=0; index<fbCount; index++) {
        pvb = &pFbMem[index];
        pvb->size = framebufSize;
        if (vdi_allocate_dma_memory(coreIdx, pvb) < 0) {
            success=FALSE;
            VLOG(ERR, "%s:%d fail to allocate frame buffer\n", __FUNCTION__, __LINE__);
            goto DECODE_END;
        }
        pFrame[index].bufY  = pvb->phys_addr;
        pFrame[index].bufCb = -1;
        pFrame[index].bufCr = -1;
        pFrame[index].updateFbInfo = TRUE;
    }
    if ((ret=VPU_DecAllocateFrameBuffer(handle, fbAllocInfo, pFrame)) != RETCODE_SUCCESS) {
        VLOG(ERR, "%s:%d failed to VPU_DecAllocateFrameBuffer(), ret(%d)\n", __FUNCTION__, __LINE__, ret);
        goto DECODE_END;
    }
    /********************************************************************************
     * ALLOCATE WTL FRAMEBUFFERS                                                    *
     ********************************************************************************/
    if (decOP.wtlEnable == TRUE) {
        TiledMapType linearMapType = decOP.wtlMode == FF_FRAME ? LINEAR_FRAME_MAP : LINEAR_FIELD_MAP;
        Uint32     strideWtl;

        strideWtl    = CalcStride(sequenceInfo.picWidth, sequenceInfo.picHeight, FORMAT_420, decOP.cbcrInterleave, linearMapType, FALSE);
        framebufSize = VPU_GetFrameBufSize(coreIdx, strideWtl, framebufHeight, linearMapType, FORMAT_420, decOP.cbcrInterleave, NULL);
        
        for (index=fbCount; index<fbCount*2; index++) {
            pvb       = &pFbMem[index];
            pvb->size = framebufSize;
            if (vdi_allocate_dma_memory(coreIdx, pvb) < 0) {
                success=FALSE;
                VLOG(ERR, "%s:%d fail to allocate frame buffer\n", __FUNCTION__, __LINE__);
                goto DECODE_END;
            }
            pFrame[index].bufY  = pvb->phys_addr;
            pFrame[index].bufCb = -1;
            pFrame[index].bufCr = -1;
            pFrame[index].updateFbInfo = TRUE;
        }
        fbAllocInfo.mapType        = linearMapType;
        fbAllocInfo.cbcrInterleave = decOP.cbcrInterleave;
        fbAllocInfo.nv21           = decOP.nv21;
        fbAllocInfo.format         = FORMAT_420;
        fbAllocInfo.stride         = strideWtl;
        fbAllocInfo.height         = framebufHeight;
        fbAllocInfo.endian         = decOP.frameEndian;
        fbAllocInfo.lumaBitDepth   = 8;
        fbAllocInfo.chromaBitDepth = 8;
        fbAllocInfo.num            = fbCount;
        fbAllocInfo.type           = FB_TYPE_CODEC;
        ret = VPU_DecAllocateFrameBuffer(handle, fbAllocInfo, &pFrame[fbCount]);
        if (ret != RETCODE_SUCCESS) {
            VLOG(ERR, "%s:%d failed to VPU_DecAllocateFrameBuffer() ret:%d\n", __FUNCTION__, __LINE__, ret);
            goto DECODE_END;
        }
    }

    /********************************************************************************
     * SET_FRAMEBUF                                                                 *
     ********************************************************************************/
    secAxiUse.u.coda9.useBitEnable  = (param->secondaryAXI>>0)&0x01;
    secAxiUse.u.coda9.useIpEnable   = (param->secondaryAXI>>1)&0x01;
    secAxiUse.u.coda9.useDbkYEnable = (param->secondaryAXI>>2)&0x01;
    secAxiUse.u.coda9.useDbkCEnable = (param->secondaryAXI>>3)&0x01;
    secAxiUse.u.coda9.useOvlEnable  = (param->secondaryAXI>>4)&0x01;
    secAxiUse.u.coda9.useBtpEnable  = (param->secondaryAXI>>5)&0x01;
    VPU_DecGiveCommand(handle, SET_SEC_AXI, &secAxiUse);

    MaverickCache2Config(&cacheCfg, TRUE/*Decoder*/, param->cbcrInterleave,
                         param->coda9.frameCacheBypass,
                         param->coda9.frameCacheBurst,
                         param->coda9.frameCacheMerge,
                         param->mapType,
                         param->coda9.frameCacheWayShape);
    VPU_DecGiveCommand(handle, SET_CACHE_CONFIG, &cacheCfg);

    framebufStride = CalcStride(sequenceInfo.picWidth, sequenceInfo.picHeight, FORMAT_420, decOP.cbcrInterleave, 
                                decOP.wtlEnable == TRUE ? LINEAR_FRAME_MAP : param->mapType, FALSE);
    ret = VPU_DecRegisterFrameBuffer(handle, pFrame, fbCount, framebufStride, framebufHeight, (int)param->mapType); 
    if (ret != RETCODE_SUCCESS) {
        success = FALSE;
        if (ret == RETCODE_MEMORY_ACCESS_VIOLATION)
            PrintMemoryAccessViolationReason(coreIdx, NULL);
        VLOG(ERR, "VPU_DecRegisterFrameBuffer failed Error code is 0x%x \n", ret );
        goto DECODE_END;
    }
    /********************************************************************************
     * ALLOCATE PPU FRAMEBUFFERS (When rotator, mirror or tiled2linear are enabled) *
     * NOTE: VPU_DecAllocateFrameBuffer() WITH PPU FRAMEBUFFER SHOULD BE CALLED     *
     *       AFTER VPU_DecRegisterFrameBuffer().                                    *
     ********************************************************************************/
    enablePPU = (BOOL)(param->coda9.rotate > 0 || param->coda9.mirror > 0 || 
                       decOP.tiled2LinearEnable == TRUE || param->coda9.enableDering == TRUE);
    if (enablePPU == TRUE) {
        Uint32 stridePpu;
        Uint32 sizePPUFb;
        Uint32 rotate       = param->coda9.rotate;
        Uint32 rotateWidth  = sequenceInfo.picWidth;
        Uint32 rotateHeight = sequenceInfo.picHeight;

        if (rotate == 90 || rotate == 270) {
            rotateWidth  = sequenceInfo.picHeight;
            rotateHeight = sequenceInfo.picWidth;
        }
        rotateWidth  = VPU_ALIGN32(rotateWidth);
        rotateHeight = VPU_ALIGN32(rotateHeight);

        stridePpu = CalcStride(rotateWidth, rotateHeight, FORMAT_420, decOP.cbcrInterleave, LINEAR_FRAME_MAP, FALSE);
        sizePPUFb = VPU_GetFrameBufSize(coreIdx, stridePpu, rotateHeight, LINEAR_FRAME_MAP, FORMAT_420, decOP.cbcrInterleave, NULL);
        
        for (index=0; index<ppuFbCount; index++) {
            pvb = &pPPUFbMem[index];
            pvb->size = sizePPUFb;
            if (vdi_allocate_dma_memory(coreIdx, pvb) < 0) {
                success=FALSE;
                VLOG(ERR, "%s:%d fail to allocate frame buffer\n", __FUNCTION__, __LINE__);
                goto DECODE_END;
            }
            pPPUFrame[index].bufY  = pvb->phys_addr;
            pPPUFrame[index].bufCb = -1;
            pPPUFrame[index].bufCr = -1;
            pPPUFrame[index].updateFbInfo = TRUE;
        }

        fbAllocInfo.mapType        = LINEAR_FRAME_MAP;
        fbAllocInfo.cbcrInterleave = decOP.cbcrInterleave;
        fbAllocInfo.nv21           = decOP.nv21;
        fbAllocInfo.format         = FORMAT_420;
        fbAllocInfo.stride         = stridePpu;
        fbAllocInfo.height         = rotateHeight;
        fbAllocInfo.endian         = decOP.frameEndian;
        fbAllocInfo.lumaBitDepth   = 8;
        fbAllocInfo.chromaBitDepth = 8;
        fbAllocInfo.num            = ppuFbCount;
        fbAllocInfo.type           = FB_TYPE_PPU;
        if ((ret=VPU_DecAllocateFrameBuffer(handle, fbAllocInfo, pPPUFrame)) != RETCODE_SUCCESS) {
            VLOG(ERR, "%s:%d failed to VPU_DecAllocateFrameBuffer() ret:%d\n", __FUNCTION__, __LINE__, ret);
            goto DECODE_END;
        }
        // Note: Please keep the below call sequence.
        VPU_DecGiveCommand(handle, SET_ROTATION_ANGLE, (void*)&param->coda9.rotate);
        VPU_DecGiveCommand(handle, SET_MIRROR_DIRECTION, (void*)&param->coda9.mirror);
        VPU_DecGiveCommand(handle, SET_ROTATOR_STRIDE, (void*)&stridePpu);

        if ((ppuQ=Queue_Create(MAX_REG_FRAME, sizeof(FrameBuffer))) == NULL) {
            success = FALSE;
            goto DECODE_END;
        }
        for (index=0; index<ppuFbCount; index++) {
            Queue_Enqueue(ppuQ, (void*)&pPPUFrame[index]);
        }
    }

    PrepareDecoderTest(handle);

    doDumpImage         = (BOOL)(param->compareType == YUV_COMPARE || saveFp != NULL || rendererType == RENDER_DEVICE_FBDEV);
    waitPostProcessing  = enablePPU;
    needStream          = FALSE;
    prevFbIndex         = -1;
    /********************************************************************************
     * PIC_RUN                                                                      *
     ********************************************************************************/
    DisplayDecodedInformation(handle, decOP.bitstreamFormat, 0, NULL);
    osal_memset((void*)&decParam, 0x00, sizeof(DecParam));
    while (TRUE) {
        if (osal_kbhit())
            break;

        while ((pDisplayInfo=(DecOutputInfo*)SimpleRenderer_GetFreeFrameInfo(renderer)) != NULL) {
            if (enablePPU == TRUE) {
                Queue_Enqueue(ppuQ, (void*)&pDisplayInfo->dispFrame);
            }
            else {
                VPU_DecClrDispFlag(handle, pDisplayInfo->dispFrame.myIndex);
            }
        }

        if (needStream == TRUE) {
            VPU_DecSetRdPtr(handle, decOP.bitstreamBuffer, TRUE);
            if (assertedFieldDoneInt == FALSE) {
                EnterLock(coreIdx);
                nWritten = BitstreamFeeder_Act(handle, feeder, decOP.streamEndian, NULL);
                LeaveLock(coreIdx);
            }
            else {
                nWritten = BitstreamFeeder_Act(handle, feeder, decOP.streamEndian, NULL);
            }
            if (nWritten < 0) {
                success = FALSE;
                goto DECODE_END;
            }
            needStream = FALSE;
        }

        if (assertedFieldDoneInt == TRUE) {
            VPU_ClearInterrupt(coreIdx);
            assertedFieldDoneInt = FALSE;
        }
        else {
            // When the field done interrupt is asserted, just fill elementary stream into the bitstream buffer.
            if (enablePPU == TRUE) {
                if ((ppuFb=(FrameBuffer*)Queue_Dequeue(ppuQ)) == NULL) {
                    MSleep(0);
                    needStream = FALSE;
                    continue;
                }
                VPU_DecGiveCommand(handle, SET_ROTATOR_OUTPUT, (void*)ppuFb);
                if (param->coda9.rotate > 0) {
                    VPU_DecGiveCommand(handle, ENABLE_ROTATION, NULL);
                }
                if (param->coda9.mirror > 0) {
                    VPU_DecGiveCommand(handle, ENABLE_MIRRORING, NULL);
                }
                if (param->coda9.enableDering == TRUE) {
                    VPU_DecGiveCommand(handle, ENABLE_DERING, NULL);
                }
            }

            VPU_DecGiveCommand(handle, DEC_SET_TARGET_TEMPORAL_ID, (void*)&param->tid);

            // Start decoding a frame.
            if ((ret=VPU_DecStartOneFrame(handle, &decParam)) != RETCODE_SUCCESS) {
                success = FALSE;
                VLOG(ERR, "VPU_DecStartOneFrame failed Error code is 0x%x \n", ret );
                goto DECODE_END;
            }
        }

        timeoutCount        = 0;
        repeat              = TRUE;
        while (repeat == TRUE) {
            if ((interruptFlag=VPU_WaitInterrupt(coreIdx, VPU_WAIT_TIME_OUT)) == -1) {
                //wait for 10ms to save stream filling time.
                if (timeoutCount*VPU_WAIT_TIME_OUT > VPU_DEC_TIMEOUT) {
                    PrintVpuStatus(coreIdx, productId);
                    VLOG(WARN, "\n VPU interrupt wait timeout instIdx=%d\n", instIdx);
                    VPU_SWReset(coreIdx, SW_RESET_SAFETY, handle);
                    success = FALSE;
                    LeaveLock(coreIdx);
                    goto DECODE_END;
                }
                timeoutCount++;
                interruptFlag = 0;
            }

            CheckUserDataInterrupt(coreIdx, handle, outputInfo.indexFrameDecoded, decOP.bitstreamFormat, interruptFlag);

            if (interruptFlag & (1<<INT_BIT_PIC_RUN)) {
                repeat = FALSE;
            }
            if (interruptFlag & (1<<INT_BIT_DEC_FIELD)) {
                if (decOP.bitstreamMode == BS_MODE_PIC_END) {
                    // do not clear interrupt until feeding next field picture.
                    assertedFieldDoneInt = TRUE;
                    break;
                }
            }
            if (interruptFlag & (1<<INT_BIT_DEC_MB_ROWS)) {
                VPU_ClearInterrupt(coreIdx);
                //VPU_DecGiveCommand(handle, GET_LOW_DELAY_OUTPUT, &outputInfo);
                //VLOG(INFO, "MB ROW interrupt is generated displayIdx=%d decodedIdx=%d picType=%d decodeSuccess=%d\n",
                ////    outputInfo.indexFrameDisplay, outputInfo.indexFrameDecoded, outputInfo.picType, outputInfo.decodingSuccess);
            }

            if (interruptFlag > 0) {
                VPU_ClearInterrupt(coreIdx);
            }
            if (repeat == FALSE) break;
            // In PICEND mode, the below codes are not called.
            if (decOP.bitstreamMode == BS_MODE_INTERRUPT && seqChangeRequest == FALSE) {
                if ((nWritten=BitstreamFeeder_Act(handle, feeder, decOP.streamEndian, NULL)) < 0) {
                    success = FALSE;
                    LeaveLock(coreIdx);
                    goto DECODE_END;
                }
            }
        }

        if (assertedFieldDoneInt == TRUE) {
            needStream = TRUE;
            continue;
        }

        ret = VPU_DecGetOutputInfo(handle, &outputInfo);
        if (ret != RETCODE_SUCCESS) {
            VLOG(ERR, "VPU_DecGetOutputInfo failed Error code is 0x%x , instIdx=%d\n", ret, instIdx);
            if (ret == RETCODE_MEMORY_ACCESS_VIOLATION) {
                PrintVpuStatus(coreIdx, productId);
                PrintMemoryAccessViolationReason(coreIdx, &outputInfo);
            }
            if (noResponseCount == 0) {
                break;
            }
            continue;
        }
        if (outputInfo.indexFrameDecoded == DECODED_IDX_FLAG_NO_FB && 
            outputInfo.indexFrameDisplay == DISPLAY_IDX_FLAG_NO_FB) {
            noFbCount++;
        }
        else {
            noFbCount = 0;
        }
        noResponseCount = MAX_COUNT_NO_RESPONSE;
        DisplayDecodedInformation(handle, decOP.bitstreamFormat, decodedIdx, &outputInfo);

        if (outputInfo.sequenceChanged == TRUE) { 
            seqChangeRequest = TRUE;
            seqChangedRdPtr  = outputInfo.rdPtr;
            seqChangedWrPtr  = outputInfo.wrPtr;
            VLOG(INFO, "seqChangeRdPtr: 0x%08x, WrPtr: 0x%08x\n", seqChangedRdPtr, seqChangedWrPtr);
            if ((ret=VPU_DecSetRdPtr(handle, seqChangedRdPtr, TRUE)) != RETCODE_SUCCESS) {
                VLOG(ERR, "%s:%d Failed to VPU_DecSetRdPtr(%d), ret(%d)\n", __FUNCTION__, __LINE__, seqChangedRdPtr, ret);
                success = FALSE;
                break;
            }
            seqChangedStreamEndFlag = outputInfo.streamEndFlag;
            VPU_DecUpdateBitstreamBuffer(handle, 1); // let f/w to know stream end condition in bitstream buffer. force to know that bitstream buffer will be empty.
            VPU_DecUpdateBitstreamBuffer(handle, STREAM_END_SET_FLAG); // set to stream end condition to pump out a delayed framebuffer.
            VLOG(INFO, "---> SEQUENCE CHANGED <---\n");
        }

        if (outputInfo.indexFrameDecoded >= 0)
            decodedIdx++;


        if (enablePPU == TRUE) {
            if (prevFbIndex >= 0) {
                VPU_DecClrDispFlag(handle, prevFbIndex);
            }
            prevFbIndex = outputInfo.indexFrameDisplay;

            if (waitPostProcessing == TRUE) {
                if (outputInfo.indexFrameDisplay >= 0) {
                    waitPostProcessing = FALSE;
                }
                rcPpu = outputInfo.rcDisplay;
                /* Release framebuffer for PPU */
                Queue_Enqueue(ppuQ, (void*)ppuFb);
                needStream = (decOP.bitstreamMode == BS_MODE_PIC_END);
                if (outputInfo.chunkReuseRequired == TRUE) {
                    needStream = FALSE;
                }
                // Not ready for ppu buffer.
                continue;
            }
            else {
                if (outputInfo.indexFrameDisplay < 0) {
                    waitPostProcessing = TRUE;
                }
            }
        }

        if (outputInfo.indexFrameDisplay >= 0 || enablePPU == TRUE) {
            Uint32    width=0, height=0, Bpp;
            size_t      frameSizeInByte;
            Uint8*    pYuv = NULL;
            void*       decodedData = NULL;
            Uint32    decodedDataSize = 0;
            VpuRect     rc = enablePPU == TRUE ? rcPpu : outputInfo.rcDisplay;

            if (doDumpImage == TRUE) {
                    pYuv = GetYUVFromFrameBuffer(handle, &outputInfo.dispFrame, rc, &width, &height, &Bpp, &frameSizeInByte);
                if (saveFp != NULL) {
                    fwrite(pYuv, frameSizeInByte, 1, saveFp);
                }
            }

            switch (param->compareType) {
            case NO_COMPARE:
                break;
            case YUV_COMPARE:
                decodedData     = (void*)pYuv;
                decodedDataSize = frameSizeInByte;
                break;
            }

            if ((success=Comparator_Act(comparator, decodedData, decodedDataSize)) == FALSE) {
                goto DECODE_END;
            }
            /* 
             * pYuv is released at the renderer module.
             * SimpleRenderer releases all framebuffer memory of the previous sequence.
             */
            SimpleRenderer_Act(renderer, &outputInfo, pYuv, width, height);
            dispIdx++;
        }

        if (dispIdx > 0 && dispIdx == param->forceOutNum) {
            break;
        }

        if (outputInfo.indexFrameDisplay == DISPLAY_IDX_FLAG_SEQ_END) {
            if (seqChangeRequest == TRUE) {
                seqChangeRequest = FALSE;
                VPU_DecSetRdPtr(handle, seqChangedRdPtr, TRUE);
                if (seqChangedStreamEndFlag == 1) {
                    VPU_DecUpdateBitstreamBuffer(handle, STREAM_END_SET_FLAG);
                }
                else {
                    VPU_DecUpdateBitstreamBuffer(handle, STREAM_END_CLEAR_FLAG);
                }
                if (seqChangedWrPtr >= seqChangedRdPtr) {
                    VPU_DecUpdateBitstreamBuffer(handle, seqChangedWrPtr-seqChangedRdPtr);
                }
                else {
                    VPU_DecUpdateBitstreamBuffer(handle, (decOP.bitstreamBuffer+decOP.bitstreamBufferSize)-seqChangedRdPtr + 
                                                         (seqChangedWrPtr-decOP.bitstreamBuffer));
                }

                // Flush renderer: waiting all picture displayed.
                SimpleRenderer_Flush(renderer);

                // Release all memory related to framebuffer.
                VPU_DecGiveCommand(handle, DEC_FREE_FRAME_BUFFER, 0x00);
                for (index=0; index<MAX_REG_FRAME; index++) {
                    if (pFbMem[index].size > 0) {
                        vdi_free_dma_memory(coreIdx, &pFbMem[index]);
                    }
                    if (pPPUFbMem[index].size > 0) {
                        vdi_free_dma_memory(coreIdx, &pPPUFbMem[index]);
                    }
                }
                seqInited = FALSE;
                goto INIT_SEQ;
            }
            break;
        }

        if (decOP.bitstreamMode == BS_MODE_PIC_END) {
            if (outputInfo.indexFrameDecoded == DECODED_IDX_FLAG_NO_FB) {
                needStream = FALSE;
            }
            else {
                needStream = TRUE;
            }
        }

        if (outputInfo.chunkReuseRequired == TRUE) {
            needStream = FALSE;
        }
        SaveDecReport(coreIdx, handle, &outputInfo, decOP.bitstreamFormat, 
                      ((sequenceInfo.picWidth+15)&~15)/16, 
                      ((sequenceInfo.picHeight+15)&~15)/16);
    }

DECODE_END:
    CloseDecReport(coreIdx);
    // Now that we are done with decoding, close the open instance.
    VPU_DecUpdateBitstreamBuffer(handle, STREAM_END_SIZE);

    if (param->compareType && outputInfo.indexFrameDisplay == DISPLAY_IDX_FLAG_SEQ_END && param->forceOutNum == 0) {
        if (success == TRUE) {
            success = Comparator_CheckFrameCount(comparator);
        }
    }

    /********************************************************************************
     * DESTROY INSTANCE                                                             *
     ********************************************************************************/

    VPU_DecClose(handle);

    if (feeder != NULL)     BitstreamFeeder_Destroy(feeder);
    if (renderer != NULL)   SimpleRenderer_Destroy(renderer);
    if (comparator != NULL) Comparator_Destroy(comparator);
    if (ppuQ != NULL)       Queue_Destroy(ppuQ);

    for (index=0; index<MAX_REG_FRAME; index++) {
        if (pFbMem[index].size > 0)
            vdi_free_dma_memory(coreIdx, &pFbMem[index]);
        if (pPPUFbMem[index].size > 0)
            vdi_free_dma_memory(coreIdx, &pPPUFbMem[index]);
    }
    VLOG(INFO, "\nDec End. Tot Frame %d\n", decodedIdx);

ERR_DEC_INIT:
    if (vbStream.size > 0) {
        vdi_free_dma_memory(coreIdx, &vbStream);
    }

    if (saveFp != NULL)     fclose(saveFp);

    VPU_DeInit(coreIdx);

    return (success == TRUE && ret == RETCODE_SUCCESS);
}

int main(int argc, char **argv)
{
    Int32           opt, index;
    Int32           ret;
    Int32           val;
    TestDecConfig   testConfig;
    char**          stop = NULL;
    BOOL            showVersion = FALSE;
    Uint32          productId;
    struct option   options[] = {
        {"output",                1, NULL, 0},
        {"input",                 1, NULL, 0},
        {"codec",                 1, NULL, 0},
        {"render",                1, NULL, 0},
        {"maptype",               1, NULL, 0},
        {"disable-wtl",           0, NULL, 0},
        {"coreIdx",               1, NULL, 0},
        {"loop-count",            1, NULL, 0},
        {"enable-cbcrinterleave", 0, NULL, 0},
        {"stream-endian",         1, NULL, 0},
        {"frame-endian",          1, NULL, 0},
        {"enable-nv21",           0, NULL, 0},
        {"secondary-axi",         1, NULL, 0},
        {"bsmode",                1, NULL, 0},
        {"enable-tiled2linear",   0, NULL, 0},
        {"rotate",                1, NULL, 0},
        {"mirror",                1, NULL, 0},
        {"enable-dering",         0, NULL, 0},
        {"enable-deblock",        0, NULL, 0},
        {"mp4class",              1, NULL, 0},
        {"ref-yuv",               1, NULL, 0},
        {"enable-mvc",            0, NULL, 0},
        {"low-delay",             1, NULL, 0},
        {"tid",                   1, NULL, 0},
        {NULL,                    0, NULL, 0},
    };
    char*           optString   = "c:hvn:u";
    BOOL            enableWTL   = TRUE;

    InitLog();

    //default setting.
    osal_memset(&testConfig, 0, sizeof(testConfig));

    testConfig.bitstreamMode            = BS_MODE_INTERRUPT;
    testConfig.streamEndian             = VDI_LITTLE_ENDIAN;
    testConfig.frameEndian              = VDI_LITTLE_ENDIAN;
    testConfig.cbcrInterleave           = FALSE;
    testConfig.nv21                     = FALSE;
    testConfig.bitFormat                = STD_AVC;
    testConfig.renderType               = RENDER_DEVICE_NULL;
    testConfig.mapType                  = LINEAR_FRAME_MAP;
    testConfig.coda9.enableBWB          = VPU_ENABLE_BWB;
    testConfig.coda9.frameCacheBypass   = 0;
    testConfig.coda9.frameCacheBurst    = 0;
    testConfig.coda9.frameCacheMerge    = 3;
    testConfig.coda9.frameCacheWayShape = 15;
    testConfig.tid                      = AVC_MAX_SUB_LAYER_ID;

    while ((opt=getopt_long(argc, argv, optString, options, &index)) != -1) {
        switch (opt) {
        case 'c':
            testConfig.compareType = atoi(optarg);
            if (testConfig.compareType < NO_COMPARE || testConfig.compareType > YUV_COMPARE) {
                VLOG(ERR, "Invalid compare type(%d)\n", testConfig.compareType);
                Help(argv[0]);
                exit(1);
            }
            break;
        case 'n':
            testConfig.forceOutNum = atoi(optarg);
            break;
        case 'h':
            Help(argv[0]);
            return 0;
            break;
        case 'v':
            showVersion = TRUE;
            break;
        case 'u':
            testConfig.enableUserData = TRUE;
            break;
        case 0:
            if (strcmp(options[index].name, "output"                    ) == 0) {
                memcpy(testConfig.outputPath, optarg, strlen(optarg));
                ChangePathStyle(testConfig.outputPath);
            }
            else if (strcmp(options[index].name, "input"                ) == 0) {
                memcpy(testConfig.inputPath, optarg, strlen(optarg));
                ChangePathStyle(testConfig.inputPath);
            }
            else if (strcmp(options[index].name, "codec"                ) == 0) {
                testConfig.bitFormat = atoi(optarg);
            }
            else if (strcmp(options[index].name, "render"               ) == 0) {
                testConfig.renderType = (RenderDeviceType)atoi(optarg);
                if (testConfig.renderType < RENDER_DEVICE_NULL || testConfig.renderType >= RENDER_DEVICE_MAX) {
                    VLOG(ERR, "unknown render device type(%d)\n", testConfig.renderType);
                    Help(argv[0]);
                    exit(1);
                }
            }
            else if (strcmp(options[index].name, "maptype"              ) == 0) {
                testConfig.mapType = (TiledMapType)atoi(optarg);
            }
            else if (strcmp(options[index].name, "disable-wtl"          ) == 0) {
                enableWTL = FALSE;
            }
            else if (strcmp(options[index].name, "coreIdx"              ) == 0) {
                testConfig.coreIdx = atoi(optarg);
            }
            else if (strcmp(options[index].name, "loop-count"           ) == 0) {
                testConfig.loopCount = atoi(optarg);
            }
            else if (strcmp(options[index].name, "enable-cbcrinterleave") == 0) {
                testConfig.cbcrInterleave = TRUE;
            }
            else if (strcmp(options[index].name, "stream-endian"        ) == 0) {
                testConfig.streamEndian = (EndianMode)atoi(optarg);
            }
            else if (strcmp(options[index].name, "frame-endian"         ) == 0) {
                testConfig.frameEndian = (EndianMode)atoi(optarg);
            }
            else if (strcmp(options[index].name, "enable-nv21"          ) == 0) {
                testConfig.nv21           = TRUE;
                testConfig.cbcrInterleave = TRUE;
            }
            else if (strcmp(options[index].name, "secondary-axi"        ) == 0) {
                testConfig.secondaryAXI = strtoul(optarg, stop, !strncmp("0x", optarg, 2) ? 16 : 10);
            }
            else if (strcmp(options[index].name, "bsmode"               ) == 0) {
                testConfig.bitstreamMode = atoi(optarg);
            }
            else if (strcmp(options[index].name, "enable-tiled2linear"  ) == 0) {
                testConfig.coda9.enableTiled2Linear = TRUE;
                testConfig.coda9.tiled2LinearMode   = FF_FRAME;
                enableWTL                           = FALSE;
            }
            else if (strcmp(options[index].name, "rotate"               ) == 0) {
                val = atoi(optarg);
                if ((val%90) != 0) {
                    VLOG(ERR, "Invalid rotation value: %d\n", val);
                    Help(argv[0]);
                    exit(1);
                }
                testConfig.coda9.rotate = val;
            }
            else if (strcmp(options[index].name, "mirror"               ) == 0) {
                val = atoi(optarg);
                if (val < 0 || val > 3) {
                    VLOG(ERR, "Invalid mirror option: %d\n", val);
                    Help(argv[0]);
                    exit(1);
                }
                testConfig.coda9.mirror = val;
            }
            else if (strcmp(options[index].name, "enable-dering"        ) == 0) {
                testConfig.coda9.enableDering = TRUE;
            }
            else if (strcmp(options[index].name, "enable-deblock"       ) == 0) {
                testConfig.coda9.enableDeblock= TRUE;
            }
            else if (strcmp(options[index].name, "mp4class"             ) == 0) {
                testConfig.coda9.mp4class = atoi(optarg);
            }
            else if (strcmp(options[index].name, "ref-yuv"              ) == 0) {
                memcpy(testConfig.refYuvPath, optarg, strlen(optarg));
                ChangePathStyle(testConfig.refYuvPath);
            }
            else if (strcmp(options[index].name, "enable-mvc"           ) == 0) {
                testConfig.coda9.enableMvc = TRUE;
            }
            else if (strcmp(options[index].name, "low-delay"            ) == 0) {
                testConfig.coda9.lowDelay.lowDelayEn = TRUE;
                testConfig.coda9.lowDelay.numRows    = atoi(optarg);
            }
            else if (strcmp(options[index].name, "tid"                  ) == 0) {
                testConfig.tid = (Uint32)atoi(optarg);
            }
            break;
        case '?':
        default:
            VLOG(ERR, "%s\n", optarg);
            Help(argv[0]);
            exit(1);
        }
    }
    VLOG(INFO, "\n");


    if (LoadFirmware(PRODUCT_ID_980, (Uint8**)&pusBitCode, &sizeInWord, CORE_1_BIT_CODE_FILE_PATH) < 0) {
        VLOG(ERR, "%s:%d Failed to load firmware: %s\n", __FUNCTION__, __LINE__, CORE_1_BIT_CODE_FILE_PATH);
        return 1;
    }

    if (showVersion) {
        Uint32 ver, rev;

        ret = VPU_InitWithBitcode(testConfig.coreIdx, (const Uint16*)pusBitCode, sizeInWord);
        if (ret != RETCODE_CALLED_BEFORE && ret != RETCODE_SUCCESS) {
            VLOG(ERR, "Failed to boot up VPU(coreIdx: %d)\n", __FUNCTION__, __LINE__, testConfig.coreIdx);
            osal_free(pusBitCode);
            return 1;
        }
        VPU_GetVersionInfo(testConfig.coreIdx, &ver, &rev, &productId);
        VPU_DeInit(testConfig.coreIdx);
        printf("VERSION=0x%08x\n", ver);
        printf("REVISION=%d\n", rev);
        printf("PRODUCT_ID=%d\n", productId);
        osal_free(pusBitCode);
        return 0;
    }

    if (strlen(testConfig.inputPath) == 0) {
        VLOG(ERR, "No input bitstream\n");
        Help(argv[0]);
        return 1;
    }

    /* Check combination of parameters of decoder */
    VLOG(WARN, "------------------------ WRONG PARAMETER COMBINATION ------------------------\n");
    if (testConfig.mapType != LINEAR_FRAME_MAP) {
        if (enableWTL == TRUE) {
            testConfig.enableWTL = TRUE;
            testConfig.wtlMode   = FF_FRAME;
        }
    }
    switch (testConfig.mapType) {
    case LINEAR_FRAME_MAP:
    case LINEAR_FIELD_MAP:
        if (testConfig.coda9.enableTiled2Linear == TRUE || testConfig.enableWTL == TRUE) {
            VLOG(WARN, "[WARN] It can't enable Tiled2Linear OR WTL option where maptype is LINEAR_FRAME_MAP\n");
            VLOG(WARN, "       Disable WTL or Tiled2Linear\n");
            testConfig.coda9.enableTiled2Linear = FALSE;
            testConfig.coda9.tiled2LinearMode   = FF_NONE;
            testConfig.enableWTL                = FALSE;
            testConfig.wtlMode                  = FF_NONE;
        }
        break;
    case TILED_FRAME_MB_RASTER_MAP:
    case TILED_FIELD_MB_RASTER_MAP:
        if (testConfig.cbcrInterleave == FALSE) {
            VLOG(WARN, "[WARN] CBCR-interleave must be enable when maptype is TILED_FRAME/FIELD_MB_RASTER_MAP.\n"); 
            VLOG(WARN, "       Enable cbcr-interleave\n");
            testConfig.cbcrInterleave = TRUE;
        }
        break;
    default:
        break;
    }

    if (testConfig.coda9.enableTiled2Linear == TRUE) {
        VLOG(WARN, "[WARN] In case of Tiledmap, disabled BWB for better performance.\n");
        testConfig.coda9.enableBWB = FALSE;
    }

    if ((testConfig.bitFormat == STD_DIV3 || testConfig.bitFormat == STD_THO) && testConfig.bitstreamMode != BS_MODE_PIC_END) {
        char* ext;
        VLOG(WARN, "[WARN] VPU can decode DIV3 or Theora codec on picend mode.\n");
        VLOG(WARN, "       BSMODE %d -> %d\n", BS_MODE_INTERRUPT, BS_MODE_PIC_END);

        ext = GetFileExtension(testConfig.inputPath);
        if (ext != NULL && (strcmp(ext, "mp4") == 0 || strcmp(ext, "mkv") == 0 || strcmp(ext, "avi") == 0)) {
            testConfig.bitstreamMode = BS_MODE_PIC_END;
        }
        if (testConfig.bitFormat == STD_THO) {
            testConfig.bitstreamMode = BS_MODE_PIC_END;
        }
    }
    if (testConfig.coda9.lowDelay.lowDelayEn == TRUE && testConfig.bitFormat != STD_AVC) {
        VLOG(WARN, "The low-delay decoding option is valid when a codec is H.264\n");
        testConfig.coda9.lowDelay.lowDelayEn = FALSE;
        testConfig.coda9.lowDelay.numRows    = 0;
    }
    VLOG(WARN, "-----------------------------------------------------------------------------\n");
    /* END OF CHECK COMBINATION */

    osal_init_keyboard();

    ret = TestDecoder(&testConfig);

    osal_close_keyboard();
    osal_free(pusBitCode);

    return ret == TRUE ? 0 : 1;
}
