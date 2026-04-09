//------------------------------------------------------------------------------
// File: main.c
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include "config.h"
#include "main_helper.h"
#include "vpuapifunc.h"
#define STREAM_BUF_SIZE_HEVC                0xA00000    // max bitstream size(HEVC:10MB,VP9:not specified)
#define STREAM_BUF_SIZE_VP9                 0x1400000    // max bitstream size(HEVC:10MB,VP9:not specified)
#define USERDATA_BUFFER_SIZE                (512*1024)  
#define EXTRA_FRAME_BUFFER_NUM              1
#define MAX_SEQUENCE_MEM_COUNT              16
#define MAX_NOT_DEC_COUNT                   200
#define NUM_VCORES                          1
#define FEEDING_SIZE                        0x20000

typedef struct {
    DecGetFramebufInfo  fbInfo;
    vpu_buffer_t        allocFbMem[MAX_REG_FRAME];
} SequenceMemInfo;

static Uint32              sizeInWord;
static Uint16*             pusBitCode;


static void 
Help(struct OptionExt *opt, const char *programName )
{
    int i;

    VLOG(INFO, "------------------------------------------------------------------------------\n");
    VLOG(INFO, "%s(API v%d.%d.%d)\n", GetBasename(programName), API_VERSION_MAJOR, API_VERSION_MINOR, API_VERSION_PATCH);
    VLOG(INFO, "\tAll rights reserved by Chips&Media(C)\n");
    VLOG(INFO, "------------------------------------------------------------------------------\n");
    VLOG(INFO, "%s [option] --input bistream\n", GetBasename(programName));
    VLOG(INFO, "-h                          help\n");
    VLOG(INFO, "-n [num]                    output frame number\n");
    VLOG(INFO, "-v                          print version information\n");
    VLOG(INFO, "-c                          compare with golden\n");
    VLOG(INFO, "                            0 : no comparison\n");
    VLOG(INFO, "                            1 : compare with golden yuv that specified --ref-yuv option\n");

    for (i = 0;i < MAX_GETOPT_OPTIONS;i++)
    {
        if (opt[i].name == NULL)
            break;
        VLOG(INFO, "%s", opt[i].help);
    }
}

static void ReleasePreviousSequenceResources(
    DecHandle           handle, 
    vpu_buffer_t*       arrFbMem,
    DecGetFramebufInfo* prevSeqFbInfo
    )
{
    Uint32      i;
    Uint32      coreIndex; 

    if (handle == NULL) {
        return;
    }

    coreIndex = VPU_HANDLE_CORE_INDEX(handle);

    for (i=0;i<MAX_REG_FRAME; i++) {
        if (arrFbMem[i].size > 0) 
            vdi_free_dma_memory(coreIndex, &arrFbMem[i]);
    }
    for ( i=0 ; i<MAX_REG_FRAME; i++) {
        if (prevSeqFbInfo->vbFbcYTbl[i].size > 0) vdi_free_dma_memory(coreIndex, &prevSeqFbInfo->vbFbcYTbl[i]);
        if (prevSeqFbInfo->vbFbcCTbl[i].size > 0) vdi_free_dma_memory(coreIndex, &prevSeqFbInfo->vbFbcCTbl[i]);
        if (prevSeqFbInfo->vbMvCol[i].size > 0)   vdi_free_dma_memory(coreIndex, &prevSeqFbInfo->vbMvCol[i]);
    }
}

BOOL TestDecoder(
    TestDecConfig *param
    )
{
    DecHandle           handle = NULL;
    DecOpenParam        decOP;
    DecInitialInfo      initialInfo;
    DecOutputInfo       outputInfo, *pDisplayInfo;
    DecParam            decParam;
    vpu_buffer_t        vbStream[2];
    vpu_buffer_t        vbUserData;
    SecAxiUse           secAxiUse;
    RetCode             ret = RETCODE_SUCCESS;
    Uint32              framebufStride=0;
    BOOL                success = TRUE;
    BOOL                doDumpImage = FALSE;
    Int32               kbhitRet = 0;
    Int32               compressedFbCount, linearFbCount;
    Int32               frameIdx = 0;
    Int32               dispIdx = 0;
    Uint32              index, val;
    BOOL                seqInited = FALSE;
    BOOL                seqInitEscape       = FALSE;
    Int32               timeoutCount = 0;
    Int32               interruptFlag = 0;
    Int32               coreIdx;
    Int32               instIdx;
    FrameBuffer         pFrame[MAX_REG_FRAME];
    vpu_buffer_t        pFbMem[MAX_REG_FRAME];
    ProductId           productId;
    Int32               nWritten;
    BSFeeder            feeder              = NULL;
    Renderer            renderer            = NULL;
    RenderDeviceType    rendererType        = param->renderType;
    FILE*               fpOutput[OUTPUT_FP_NUMBER];
#ifdef SUPPORT_SAVE_PIC_INFO_TO_XML
    FILE*               fpXml               = NULL;
#endif
    Comparator          comparator          = NULL;
    Queue*              displayQ            = NULL;
    Queue*              sequenceQ           = NULL;
    Uint32              bsBufferCount       = 1;            //!<< In PICEND mode, this variable is greater than 1.
    Uint32              bsQueueIndex        = 0;
    BOOL                needStream          = FALSE;        //!<< This variable is used on PICEND mode. 
    BOOL                loop                = TRUE;
    BOOL                repeat              = TRUE;
    SequenceMemInfo     seqMemInfo[MAX_SEQUENCE_MEM_COUNT];
    Int32               decodedIndex, prescanIndex;
    Uint32              sequenceChangeFlag;
    Uint32              sequenceChangeOccurred=0;
    Uint32              notDecodedCount     = 0;
    Uint32              noFbCount           = 0;
    Uint32              compWidth           = 0;
    Uint32              compHeight          = 0;
    Uint8*              pBase        = NULL;
    Uint32              intBitInitSeq, intBitDecPic, intBitEmpty;
    Uint32              loopCount, interruptTimeout = VPU_DEC_TIMEOUT;
    BOOL                fillStreamEmptyInt = FALSE;

    osal_memset(&decOP,         0x00, sizeof(DecOpenParam));
    osal_memset(&initialInfo,   0x00, sizeof(DecInitialInfo));
    osal_memset(&outputInfo,    0x00, sizeof(DecOutputInfo));
    osal_memset(&decParam,      0x00, sizeof(DecParam));
    osal_memset(vbStream,       0x00, sizeof(vbStream));
    osal_memset(&secAxiUse,     0x00, sizeof(SecAxiUse));
    osal_memset(pFbMem,         0x00, sizeof(vpu_buffer_t)*MAX_REG_FRAME);
    osal_memset(seqMemInfo,     0x00, sizeof(seqMemInfo));
    osal_memset(&fpOutput[0],   0x00, sizeof(fpOutput));

    param->enableCrop = TRUE;
    instIdx   = param->instIdx;
    coreIdx   = param->coreIdx;

    /* Check parameters */
    if (param->bitFormat != STD_HEVC && param->bitFormat != STD_VP9) {
        VLOG(ERR, "Not supported video standard: %d\n", param->bitFormat);
        success = FALSE;
        goto ERR_DEC_INIT;
    }


#ifdef SUPPORT_SAVE_PIC_INFO_TO_XML
    if ((fpXml = CreatePicInfoXmlFile(param->outputPath)) == NULL) {
        success = FALSE;
        goto ERR_DEC_INIT;
    }
#endif

    if ((productId=(ProductId)VPU_GetProductId(coreIdx)) == -1) {
        VLOG(ERR, "Failed to get product ID\n");
        success = FALSE;
        goto ERR_DEC_INIT;
    }

    intBitInitSeq = INT_WAVE_DEC_PIC_HDR;
    intBitDecPic  = INT_WAVE_DEC_PIC;
    intBitEmpty   = INT_WAVE_BIT_BUF_EMPTY;

    ret = VPU_InitWithBitcode(coreIdx, (const Uint16*)pusBitCode, sizeInWord);
    if (ret != RETCODE_CALLED_BEFORE && ret != RETCODE_SUCCESS) {
        VLOG(ERR, "Failed to boot up VPU(coreIdx: %d, productId: %d)\n", coreIdx, productId);
        success = FALSE;
        goto ERR_DEC_INIT;
    }

    PrintVpuVersionInfo(coreIdx);

    bsBufferCount = (param->bitstreamMode == BS_MODE_PIC_END) ? 2 : 1;
    if ( param->bitFormat == STD_VP9 ) {
        param->bsSize = STREAM_BUF_SIZE_VP9;
    }
    VLOG(INFO, "Stream Buf size = %d(0x%x)\n", param->bsSize, param->bsSize);
    for (index=0; index<bsBufferCount; index++ ) {
        vbStream[index].size = param->bsSize;
        if (vdi_allocate_dma_memory(coreIdx, &vbStream[index]) < 0) {
            VLOG(ERR, "fail to allocate bitstream buffer\n" );
            success = FALSE;
            goto ERR_DEC_INIT;
        }
    }

    if (param->feedingMode == FEEDING_METHOD_FRAME_SIZE) {
        CodStd  codecId;
        feeder = BitstreamFeeder_Create(param->inputPath, FEEDING_METHOD_FRAME_SIZE, vbStream[0].virt_addr, vbStream[0].size, param->bitFormat, &codecId, NULL, NULL, NULL);
        param->bitFormat = codecId;
    }
    else if (param->feedingMode == FEEDING_METHOD_FIXED_SIZE) {
        feeder = BitstreamFeeder_Create(param->inputPath, FEEDING_METHOD_FIXED_SIZE, vbStream[0].virt_addr, vbStream[0].size, FEEDING_SIZE);
    }
    else {
        /* FEEDING_METHOD_SIZE_PLUS_ES */
        feeder = BitstreamFeeder_Create(param->inputPath, FEEDING_METHOD_SIZE_PLUS_ES, vbStream[0].virt_addr, vbStream[0].size);
    }
    if (feeder == NULL) {
        success = FALSE;
        goto ERR_DEC_OPEN;
    }
    BitstreamFeeder_SetFillMode(feeder, (param->bitstreamMode == BS_MODE_PIC_END) ? BSF_FILLING_LINEBUFFER : BSF_FILLING_RINGBUFFER);

    /* set up decoder configurations */
    decOP.bitstreamFormat     = (CodStd)param->bitFormat;
    decOP.coreIdx             = coreIdx;
    decOP.bitstreamBuffer     = vbStream[0].phys_addr;
    decOP.bitstreamBufferSize = vbStream[0].size * bsBufferCount;
    decOP.bitstreamMode       = param->bitstreamMode;
    decOP.wtlEnable           = param->enableWTL;
    decOP.wtlMode             = param->wtlMode;
    decOP.cbcrInterleave      = param->cbcrInterleave;
    decOP.nv21                = param->nv21;
    decOP.streamEndian        = param->streamEndian;
    decOP.frameEndian         = param->frameEndian;
    decOP.fbc_mode            = param->wave4.fbcMode;
    decOP.bwOptimization      = param->wave4.bwOptimization;

    if ((ret=VPU_DecOpen(&handle, &decOP)) != RETCODE_SUCCESS) {
        success = FALSE;
        VLOG(ERR, "VPU_DecOpen failed Error code is 0x%x \n", ret );
        goto ERR_DEC_OPEN;
    }

    //VPU_DecGiveCommand(handle, ENABLE_LOGGING, 0);

    vbUserData.size = USERDATA_BUFFER_SIZE;
    if (vdi_allocate_dma_memory(coreIdx, &vbUserData) < 0) {
        success = FALSE;
        VLOG(ERR, "%s:%d Failed to allocate memory\n", __FUNCTION__, __LINE__);
        goto ERR_DEC_OPEN;
    }
    pBase = (Uint8*)osal_malloc(USERDATA_BUFFER_SIZE);
    VPU_DecGiveCommand(handle, SET_ADDR_REP_USERDATA, (void*)&vbUserData.phys_addr);
    VPU_DecGiveCommand(handle, SET_SIZE_REP_USERDATA, (void*)&vbUserData.size);
    VPU_DecGiveCommand(handle, ENABLE_REP_USERDATA,   (void*)&param->enableUserData);

    if (param->thumbnailMode == TRUE) {
        VPU_DecGiveCommand(handle, ENABLE_DEC_THUMBNAIL_MODE, NULL);
    }

    if ((renderer=SimpleRenderer_Create(handle, rendererType, param->outputPath)) == NULL) {
        success = FALSE;
        goto ERR_DEC_OPEN;
    }

    if ((nWritten=BitstreamFeeder_Act(handle, feeder, decOP.streamEndian, NULL)) < 0) {
        success = FALSE;
        goto ERR_DEC_OPEN;
    }

    if ((displayQ=Queue_Create(MAX_REG_FRAME, sizeof(DecOutputInfo))) == NULL) {
        success = FALSE;
        goto ERR_DEC_OPEN;
    }
    if ((sequenceQ=Queue_Create(MAX_REG_FRAME, sizeof(Uint32))) == NULL) {
        success = FALSE;
        goto ERR_DEC_OPEN;
    }

    // Scan and retrieve sequence header information from the bitstream buffer.
    seqInitEscape = FALSE;
    ret = VPU_DecSetEscSeqInit(handle, seqInitEscape);
    if (ret != RETCODE_SUCCESS) {
        seqInitEscape = 0;
        VLOG(WARN, "Wanning! can not to set seqInitEscape in the current bitstream mode Option \n");
    }

    if (seqInitEscape == TRUE) {
        VPU_DecUpdateBitstreamBuffer(handle, 0);
    }

    ret = VPU_DecIssueSeqInit(handle);
    if (ret != RETCODE_SUCCESS) {
        VLOG(ERR, "VPU_DecIssueSeqInit failed Error code is 0x%x \n", ret);
        success = FALSE;
        goto ERR_DEC_OPEN;
    }
    timeoutCount = 0;
    while (seqInited == FALSE || (kbhitRet=osal_kbhit()) == 0) {    
        interruptFlag = VPU_WaitInterrupt(coreIdx, VPU_WAIT_TIME_OUT);        //wait for 10ms to save stream filling time.
        if (interruptFlag == -1) {
            if (interruptTimeout > 0 && timeoutCount*VPU_WAIT_TIME_OUT > interruptTimeout) {
                VLOG(ERR, "\n DEC_PIC_HDR TIMEOUT\n");
                EnterLock(coreIdx);
                PrintVpuStatus(coreIdx, productId);
                VPU_SWReset(coreIdx, SW_RESET_SAFETY, handle);
                LeaveLock(coreIdx);
                success = FALSE;
                goto ERR_DEC_OPEN;
            }
            timeoutCount++;
            interruptFlag = 0;
        }

        if (interruptFlag > 0) {
            VPU_ClearInterrupt(coreIdx);
            if (interruptFlag & (1<<intBitInitSeq)) {
                seqInited = TRUE;
                break;
            }
        }

        if (decOP.bitstreamMode == BS_MODE_INTERRUPT) {
            if ((nWritten=BitstreamFeeder_Act(handle, feeder, decOP.streamEndian, NULL)) < 0) {
                success = FALSE;
                goto ERR_DEC_OPEN;
            }
        }
    }

    if (seqInited == TRUE) {
        if ((ret=VPU_DecCompleteSeqInit(handle, &initialInfo)) != RETCODE_SUCCESS) {
            VLOG(ERR, "%s:%d FAILED TO DEC_PIC_HDR: ret(%d), SEQERR(%08x)\n", 
                __FUNCTION__, __LINE__, ret, initialInfo.seqInitErrReason);
            success = FALSE;
            EnterLock(coreIdx);
            HandleDecInitSequenceError(handle, productId, &decOP, &initialInfo, ret);
            LeaveLock(coreIdx);
            goto ERR_DEC_OPEN;
        }
    }

    if (initialInfo.userDataHeader != 0) {
        user_data_entry_t* pEntry = (user_data_entry_t*)pBase;
        VLOG(INFO, "===== USER DATA(SEI OR VUI) : NUM(%d) =====\n", initialInfo.userDataNum);
        VpuReadMem(coreIdx, vbUserData.phys_addr, pBase, vbUserData.size, VPU_USER_DATA_ENDIAN);

        if (initialInfo.userDataHeader & (1<<H265_USERDATA_FLAG_VUI)) {
            h265_vui_param_t*  vui = (h265_vui_param_t*)(pBase + pEntry[H265_USERDATA_FLAG_VUI].offset);
            VLOG(INFO, " VUI SAR(%d, %d)\n", vui->sar_width, vui->sar_height);
            VLOG(INFO, "     VIDEO FORMAT(%d)\n", vui->video_format);
        }
        if (initialInfo.userDataHeader & (1<<H265_USERDATA_FLAG_MASTERING_COLOR_VOL)) {
            h265_mastering_display_colour_volume_t* mastering;

            mastering = (h265_mastering_display_colour_volume_t*)(pBase + pEntry[H265_USERDATA_FLAG_MASTERING_COLOR_VOL].offset);
            VLOG(INFO, " MASTERING DISPLAY COLOR VOLUME\n");
            for (index=0; index<3; index++) {
                VLOG(INFO, " PRIMARIES_X%d : %10d PRIMARIES_Y%d : %10d\n", index, mastering->display_primaries_x[index], index, mastering->display_primaries_y[index]);
            }
            VLOG(INFO, " WHITE_POINT_X: %10d WHITE_POINT_Y: %10d\n", mastering->white_point_x, mastering->white_point_y);
            VLOG(INFO, " MIN_LUMINANCE: %10d MAX_LUMINANCE: %10d\n", mastering->min_display_mastering_luminance, mastering->max_display_mastering_luminance);
        }

        if (initialInfo.userDataHeader & (1<<H265_USERDATA_FLAG_CHROMA_RESAMPLING_FILTER_HINT)) {
            h265_chroma_resampling_filter_hint_t* c_resampleing_filter_hint;
            Uint32   i,j;

            c_resampleing_filter_hint = (h265_chroma_resampling_filter_hint_t*)(pBase + pEntry[H265_USERDATA_FLAG_CHROMA_RESAMPLING_FILTER_HINT].offset);
            VLOG(INFO, " CHROMA_RESAMPLING_FILTER_HINT\n");
            VLOG(INFO, " VER_CHROMA_FILTER_IDC: %10d HOR_CHROMA_FILTER_IDC: %10d\n", c_resampleing_filter_hint->ver_chroma_filter_idc, c_resampleing_filter_hint->hor_chroma_filter_idc);
            VLOG(INFO, " VER_FILTERING_FIELD_PROCESSING_FLAG: %d \n", c_resampleing_filter_hint->ver_filtering_field_processing_flag);
            if (c_resampleing_filter_hint->ver_chroma_filter_idc == 1 || c_resampleing_filter_hint->hor_chroma_filter_idc == 1) {
                VLOG(INFO, " TARGET_FORMAT_IDC: %d \n", c_resampleing_filter_hint->target_format_idc);
                if (c_resampleing_filter_hint->ver_chroma_filter_idc == 1) {
                    VLOG(INFO, " NUM_VERTICAL_FILTERS: %d \n", c_resampleing_filter_hint->num_vertical_filters);
                    for (i=0; i<(int)c_resampleing_filter_hint->num_vertical_filters; i++) {
                        VLOG(INFO, " VER_TAP_LENGTH_M1[%d]: %d \n", i, c_resampleing_filter_hint->ver_tap_length_minus1[i]);
                        for (j=0; j<c_resampleing_filter_hint->ver_tap_length_minus1[i]; j++) {
                            VLOG(INFO, " VER_FILTER_COEFF[%d][%d]: %d \n", i, j, c_resampleing_filter_hint->ver_filter_coeff[i][j]);
                        }
                    }
                }
                if (c_resampleing_filter_hint->hor_chroma_filter_idc == 1) {
                    VLOG(INFO, " NUM_HORIZONTAL_FILTERS: %d \n", c_resampleing_filter_hint->num_horizontal_filters);
                    for (i=0; i<(int)c_resampleing_filter_hint->num_horizontal_filters; i++) {
                        VLOG(INFO, " HOR_TAP_LENGTH_M1[%d]: %d \n", i, c_resampleing_filter_hint->hor_tap_length_minus1[i]);
                        for (j=0; j<c_resampleing_filter_hint->hor_tap_length_minus1[i]; j++) {
                            VLOG(INFO, " HOR_FILTER_COEFF[%d][%d]: %d \n", i, j, c_resampleing_filter_hint->hor_filter_coeff[i][j]);
                        }
                    }
                }
            }
        }

        if (initialInfo.userDataHeader & (1<<H265_USERDATA_FLAG_KNEE_FUNCTION_INFO)) {
            h265_knee_function_info_t* knee_function;

            knee_function = (h265_knee_function_info_t*)(pBase + pEntry[H265_USERDATA_FLAG_KNEE_FUNCTION_INFO].offset);
            VLOG(INFO, " FLAG_KNEE_FUNCTION_INFO\n");
            VLOG(INFO, " KNEE_FUNCTION_ID: %10d\n", knee_function->knee_function_id);
            VLOG(INFO, " KNEE_FUNCTION_CANCEL_FLAG: %d\n", knee_function->knee_function_cancel_flag);
            if (knee_function->knee_function_cancel_flag) {
                VLOG(INFO, " KNEE_FUNCTION_PERSISTENCE_FLAG: %10d\n", knee_function->knee_function_persistence_flag);
                VLOG(INFO, " INPUT_D_RANGE: %d\n", knee_function->input_d_range);
                VLOG(INFO, " INPUT_DISP_LUMINANCE: %d\n", knee_function->input_disp_luminance);
                VLOG(INFO, " OUTPUT_D_RANGE: %d\n", knee_function->output_d_range);
                VLOG(INFO, " OUTPUT_DISP_LUMINANCE: %d\n", knee_function->output_disp_luminance);
                VLOG(INFO, " NUM_KNEE_POINTS_M1: %d\n", knee_function->num_knee_points_minus1);
                for (index=0; index<knee_function->num_knee_points_minus1; index++) {
                    VLOG(INFO, " INPUT_KNEE_POINT: %10d OUTPUT_KNEE_POINT: %10d\n", knee_function->input_knee_point[index], knee_function->output_knee_point[index]);
                }

            }

        }
        VLOG(INFO, "===========================================\n");
    }


    VLOG(INFO, "* Dec InitialInfo =>\n instance #%d, \n minframeBuffercount: %u\n", instIdx, initialInfo.minFrameBufferCount);
    VLOG(INFO, " picWidth: %u\n picHeight: %u\n ",initialInfo.picWidth, initialInfo.picHeight);


    compWidth  = initialInfo.picWidth;
    compHeight = initialInfo.picHeight;

    switch (param->compareType) {
    case NO_COMPARE:
        comparator = Comparator_Create(param->compareType, NULL);
        break;
    case YUV_COMPARE:
        comparator = Comparator_Create(param->compareType, param->refYuvPath,
            compWidth, compHeight, 
            param->wtlFormat, param->cbcrInterleave);
        break;
    default:
        VLOG(ERR, "UNKNOWN COMPARATOR TYPE(%d)\n", param->compareType);
        success = FALSE;
        goto ERR_DEC_OPEN;
    }
    if (comparator == NULL) {
        success = FALSE;
        goto ERR_DEC_OPEN;
    }

    PrintDecSeqWarningMessages(productId, &initialInfo);

    /********************************************************************************
    * ALLOCATE FRAME BUFFER                                                        *
    ********************************************************************************/
    /* Set up the secondary AXI is depending on H/W configuration.
    * Note that turn off all the secondary AXI configuration 
    * if target ASIC has no memory only for IP, LF and BIT.
    */
    secAxiUse.u.wave4.useIpEnable    = (param->secondaryAXI&0x01) ? TRUE : FALSE;
    secAxiUse.u.wave4.useLfRowEnable = (param->secondaryAXI&0x02) ? TRUE : FALSE;
    secAxiUse.u.wave4.useBitEnable   = (param->secondaryAXI&0x04) ? TRUE : FALSE;
    VPU_DecGiveCommand(handle, SET_SEC_AXI, &secAxiUse);

    compressedFbCount = initialInfo.minFrameBufferCount + EXTRA_FRAME_BUFFER_NUM;   // max_dec_pic_buffering
    if (param->enableWTL == TRUE) {
        if (productId == PRODUCT_ID_420 || productId == PRODUCT_ID_420L) {
            linearFbCount = compressedFbCount;
        } 
        else {
            linearFbCount = initialInfo.frameBufDelay + EXTRA_FRAME_BUFFER_NUM + 1;
        }
        if (handle->codecMode == C7_VP9_DEC || handle->codecMode == C7_AVS2_DEC) {
            linearFbCount = compressedFbCount;
        }
        VPU_DecGiveCommand(handle, DEC_SET_WTL_FRAME_FORMAT, &param->wtlFormat);
    }
    else {
        linearFbCount = 0;
    }

    VLOG(INFO, "compressedFbCount=%d, linearFbCount=%d\n", compressedFbCount, linearFbCount);

    osal_memset((void*)pFbMem, 0x00, sizeof(vpu_buffer_t)*MAX_REG_FRAME);
    success = AllocateDecFrameBuffer(handle, param, compressedFbCount, linearFbCount, pFrame, pFbMem, &framebufStride);
    if (success == FALSE) {
        goto ERR_DEC_OPEN;
    }

    /********************************************************************************
    * REGISTER FRAME BUFFER                                                        *
    ********************************************************************************/
    ret = VPU_DecRegisterFrameBufferEx(handle, pFrame, compressedFbCount, linearFbCount, framebufStride, initialInfo.picHeight, COMPRESSED_FRAME_MAP);
    if( ret != RETCODE_SUCCESS ) {
        success = FALSE;
        if (ret == RETCODE_MEMORY_ACCESS_VIOLATION) {
            EnterLock(coreIdx);
            PrintMemoryAccessViolationReason(coreIdx, NULL);
            LeaveLock(coreIdx);
        }

        VLOG(ERR, "VPU_DecRegisterFrameBuffer failed Error code is 0x%x \n", ret );
        goto ERR_DEC_OPEN;
    }

    doDumpImage = (BOOL)(param->compareType == YUV_COMPARE || (strlen(param->outputPath) > 0) || rendererType == RENDER_DEVICE_FBDEV);


    DisplayDecodedInformation(handle, decOP.bitstreamFormat, 0, NULL);

    val = (param->bitFormat == STD_HEVC) ? SEQ_CHANGE_ENABLE_ALL_HEVC : SEQ_CHANGE_ENABLE_ALL_VP9;
    VPU_DecGiveCommand(handle, DEC_SET_SEQ_CHANGE_MASK, (void*)&val);
    /********************************************************************************
    * DEC_PIC                                                                      *
    ********************************************************************************/
    decParam.skipframeMode = param->skipMode;
    loopCount              = param->loopCount;

    while (loop == TRUE) {
        if (osal_kbhit()) {
            break;
        }
        if (notDecodedCount == MAX_NOT_DEC_COUNT) {
            VLOG(ERR, "Continuous not-decoded-cout is %d\nThere is something problem in DPB control.\n", notDecodedCount);
            success = FALSE;
            break;
        }

        if ((pDisplayInfo=(DecOutputInfo*)SimpleRenderer_GetFreeFrameInfo(renderer)) != NULL) {
            Uint32*     ptr;

            ptr = (Uint32*)Queue_Peek(sequenceQ);
            if (ptr && *ptr != pDisplayInfo->sequenceNo) {
                /* Release all framebuffers of previous sequence */
                SequenceMemInfo* p; 
                index = (*ptr) % MAX_SEQUENCE_MEM_COUNT;
                p = &seqMemInfo[index];
                ReleasePreviousSequenceResources(handle, p->allocFbMem, &p->fbInfo);
                osal_memset(p, 0x00, sizeof(SequenceMemInfo));
                Queue_Dequeue(sequenceQ);
            }
            if (pDisplayInfo->sequenceNo == initialInfo.sequenceNo) {
                VPU_DecClrDispFlag(handle, pDisplayInfo->indexFrameDisplay);
            }
        }

        if (needStream == TRUE) {
            bsQueueIndex = (bsQueueIndex+1)%bsBufferCount;
            VPU_DecSetRdPtr(handle, vbStream[bsQueueIndex].phys_addr, TRUE);
            EnterLock(coreIdx);
            nWritten = BitstreamFeeder_Act(handle, feeder, decOP.streamEndian, NULL);
            LeaveLock(coreIdx);
            if (nWritten < 0) {
                success = FALSE;
                goto ERR_DEC_OPEN;
            }
        }

        decParam.skipframeMode = param->skipMode;	
        if (decParam.skipframeMode == 1) {
            SimpleRenderer_Flush(renderer);
        }
        // Start decoding a frame.
        decParam.craAsBlaFlag = param->wave4.craAsBla;
        ret=VPU_DecStartOneFrame(handle, &decParam);
        if (ret != RETCODE_SUCCESS) {
            success = FALSE;
            VLOG(ERR, "VPU_DecStartOneFrame failed Error code is 0x%x \n", ret );
            LeaveLock(coreIdx);
            goto ERR_DEC_OPEN;
        }

        timeoutCount = 0;
        repeat       = TRUE;
        while(repeat == TRUE) {
            if ((interruptFlag=VPU_WaitInterrupt(coreIdx, VPU_WAIT_TIME_OUT)) == -1) {
                //wait for 10ms to save stream filling time.
                if (interruptTimeout > 0 && timeoutCount*VPU_WAIT_TIME_OUT > interruptTimeout) {
                    HandleDecoderError(handle, frameIdx, param, pFbMem, NULL);
                    PrintVpuStatus(coreIdx, productId);
                    VLOG(WARN, "\n VPU interrupt wait timeout\n");
                    VPU_SWReset(coreIdx, SW_RESET_SAFETY, handle);
                    repeat  = FALSE;
                    success = FALSE;
                    break;
                }
                timeoutCount++;
                interruptFlag = 0;
            }

            if (interruptFlag > 0) {
                VPU_ClearInterrupt(coreIdx);
            }

            if (interruptFlag & (1<<intBitDecPic)) {
                break;
            }

            if (interruptFlag & (1<<intBitEmpty)) {
                /* TODO: handling empty interrupt */
                Uint32 avail;
                VPU_DecGetBitstreamBuffer(handle, NULL, NULL, &avail);
                if (avail < FEEDING_SIZE) {
                    VPU_DecUpdateBitstreamBuffer(handle, 0); // Set end-of-stream
                    VLOG(ERR, "Not enough bitstream buffer\n");
                    success = FALSE;
                    break;
                }
            }

            if (decOP.bitstreamMode == BS_MODE_INTERRUPT) {
                BOOL doFillStream = (fillStreamEmptyInt == FALSE) ? TRUE : (BOOL)((interruptFlag>>intBitEmpty)&1);
                if (doFillStream == TRUE) {
                    if ((nWritten=BitstreamFeeder_Act(handle, feeder, decOP.streamEndian, NULL)) < 0) {
                        success = FALSE;
                        LeaveLock(coreIdx);
                        goto ERR_DEC_OPEN;
                    }
                }
            }
        }

        ret = VPU_DecGetOutputInfo(handle, &outputInfo);
        if (ret != RETCODE_SUCCESS) {
            VLOG(ERR, "VPU_DecGetOutputInfo failed Error code is 0x%x\n", ret);
            if (ret == RETCODE_MEMORY_ACCESS_VIOLATION) {
                EnterLock(coreIdx);
                PrintMemoryAccessViolationReason(coreIdx, (void*)&outputInfo);
                PrintVpuStatus(coreIdx, productId);
                LeaveLock(coreIdx);
                success = FALSE;
                break;
            }
            continue;
        }

        if (outputInfo.decOutputExtData.userDataNum > 0) {
            user_data_entry_t* pEntry = (user_data_entry_t*)pBase;

            VpuReadMem(coreIdx, vbUserData.phys_addr, pBase, vbUserData.size, VPU_USER_DATA_ENDIAN);

            for (index=0; index<32; index++) {
                if (outputInfo.decOutputExtData.userDataHeader&(1<<index)) {
                    VLOG(INFO, "USERDATA INDEX: %02d offset: %8d size: %d\n", index, pEntry[index].offset, pEntry[index].size);

                    if (outputInfo.decOutputExtData.userDataHeader & (1<<H265_USERDATA_FLAG_MASTERING_COLOR_VOL)) {
                        h265_mastering_display_colour_volume_t* mastering;
                        int i;

                        mastering = (h265_mastering_display_colour_volume_t*)(pBase + pEntry[H265_USERDATA_FLAG_MASTERING_COLOR_VOL].offset);
                        VLOG(INFO, " MASTERING DISPLAY COLOR VOLUME\n");
                        for (i=0; i<3; i++) {
                            VLOG(INFO, " PRIMARIES_X%d : %10d PRIMARIES_Y%d : %10d\n", i, mastering->display_primaries_x[i], i, mastering->display_primaries_y[i]);
                        }
                        VLOG(INFO, " WHITE_POINT_X: %10d WHITE_POINT_Y: %10d\n", mastering->white_point_x, mastering->white_point_y);
                        VLOG(INFO, " MIN_LUMINANCE: %10d MAX_LUMINANCE: %10d\n", mastering->min_display_mastering_luminance, mastering->max_display_mastering_luminance);
                    }

                    if(outputInfo.decOutputExtData.userDataHeader&(1<<H265_USERDATA_FLAG_VUI))
                    {
                        h265_vui_param_t* vui;

                        vui = (h265_vui_param_t*)(pBase + pEntry[H265_USERDATA_FLAG_VUI].offset);
                        VLOG(INFO, " VUI SAR(%d, %d)\n", vui->sar_width, vui->sar_height);
                        VLOG(INFO, "     VIDEO FORMAT(%d)\n", vui->video_format);
                        VLOG(INFO, "     COLOUR PRIMARIES(%d)\n", vui->colour_primaries);
                        VLOG(INFO, "log2_max_mv_length_horizontal: %d\n", vui->log2_max_mv_length_horizontal);
                        VLOG(INFO, "log2_max_mv_length_vertical  : %d\n", vui->log2_max_mv_length_vertical);
                    }
                    if (outputInfo.decOutputExtData.userDataHeader & (1<<H265_USERDATA_FLAG_CHROMA_RESAMPLING_FILTER_HINT)) {
                        h265_chroma_resampling_filter_hint_t* c_resampleing_filter_hint;
                        Uint32   i,j;

                        c_resampleing_filter_hint = (h265_chroma_resampling_filter_hint_t*)(pBase + pEntry[H265_USERDATA_FLAG_CHROMA_RESAMPLING_FILTER_HINT].offset);
                        VLOG(INFO, " CHROMA_RESAMPLING_FILTER_HINT\n");
                        VLOG(INFO, " VER_CHROMA_FILTER_IDC: %10d HOR_CHROMA_FILTER_IDC: %10d\n", c_resampleing_filter_hint->ver_chroma_filter_idc, c_resampleing_filter_hint->hor_chroma_filter_idc);
                        VLOG(INFO, " VER_FILTERING_FIELD_PROCESSING_FLAG: %d \n", c_resampleing_filter_hint->ver_filtering_field_processing_flag);
                        if (c_resampleing_filter_hint->ver_chroma_filter_idc == 1 || c_resampleing_filter_hint->hor_chroma_filter_idc == 1) {
                            VLOG(INFO, " TARGET_FORMAT_IDC: %d \n", c_resampleing_filter_hint->target_format_idc);
                            if (c_resampleing_filter_hint->ver_chroma_filter_idc == 1) {
                                VLOG(INFO, " NUM_VERTICAL_FILTERS: %d \n", c_resampleing_filter_hint->num_vertical_filters);
                                for (i=0; i<(int)c_resampleing_filter_hint->num_vertical_filters; i++) {
                                    VLOG(INFO, " VER_TAP_LENGTH_M1[%d]: %d \n", i, c_resampleing_filter_hint->ver_tap_length_minus1[i]);
                                    for (j=0; j<c_resampleing_filter_hint->ver_tap_length_minus1[i]; j++) {
                                        VLOG(INFO, " VER_FILTER_COEFF[%d][%d]: %d \n", i, j, c_resampleing_filter_hint->ver_filter_coeff[i][j]);
                                    }
                                }
                            }
                            if (c_resampleing_filter_hint->hor_chroma_filter_idc == 1) {
                                VLOG(INFO, " NUM_HORIZONTAL_FILTERS: %d \n", c_resampleing_filter_hint->num_horizontal_filters);
                                for (i=0; i<(int)c_resampleing_filter_hint->num_horizontal_filters; i++) {
                                    VLOG(INFO, " HOR_TAP_LENGTH_M1[%d]: %d \n", i, c_resampleing_filter_hint->hor_tap_length_minus1[i]);
                                    for (j=0; j<c_resampleing_filter_hint->hor_tap_length_minus1[i]; j++) {
                                        VLOG(INFO, " HOR_FILTER_COEFF[%d][%d]: %d \n", i, j, c_resampleing_filter_hint->hor_filter_coeff[i][j]);
                                    }
                                }
                            }
                        }
                    }

                    if (outputInfo.decOutputExtData.userDataHeader & (1<<H265_USERDATA_FLAG_KNEE_FUNCTION_INFO)) {
                        h265_knee_function_info_t* knee_function;

                        knee_function = (h265_knee_function_info_t*)(pBase + pEntry[H265_USERDATA_FLAG_KNEE_FUNCTION_INFO].offset);
                        VLOG(INFO, " FLAG_KNEE_FUNCTION_INFO\n");
                        VLOG(INFO, " KNEE_FUNCTION_ID: %10d\n", knee_function->knee_function_id);
                        VLOG(INFO, " KNEE_FUNCTION_CANCEL_FLAG: %d\n", knee_function->knee_function_cancel_flag);
                        if (knee_function->knee_function_cancel_flag) {
                            VLOG(INFO, " KNEE_FUNCTION_PERSISTENCE_FLAG: %10d\n", knee_function->knee_function_persistence_flag);
                            VLOG(INFO, " INPUT_D_RANGE: %d\n", knee_function->input_d_range);
                            VLOG(INFO, " INPUT_DISP_LUMINANCE: %d\n", knee_function->input_disp_luminance);
                            VLOG(INFO, " OUTPUT_D_RANGE: %d\n", knee_function->output_d_range);
                            VLOG(INFO, " OUTPUT_DISP_LUMINANCE: %d\n", knee_function->output_disp_luminance);
                            VLOG(INFO, " NUM_KNEE_POINTS_M1: %d\n", knee_function->num_knee_points_minus1);
                            for (index=0; index<knee_function->num_knee_points_minus1; index++) {
                                VLOG(INFO, " INPUT_KNEE_POINT: %10d OUTPUT_KNEE_POINT: %10d\n", knee_function->input_knee_point[index], knee_function->output_knee_point[index]);
                            }

                        }

                    }
                }
            }
        }

        decodedIndex        = outputInfo.indexFrameDecoded;
        prescanIndex        = outputInfo.indexFramePrescan;
        sequenceChangeFlag  = outputInfo.sequenceChanged;
        sequenceChangeOccurred |= sequenceChangeFlag;

        if ((outputInfo.decodingSuccess & 0x01) == 0) {
            if (outputInfo.indexFramePrescan == -2) {
                VLOG(WARN, "Stream is insufficient to prescan\n");
            }
            else {
                VLOG(ERR, "instance(%d) VPU_DecGetOutputInfo decode fail framdIdx %d error(0x%08x) reason(0x%08x), reasonExt(0x%08x)\n", 
                           handle->instIndex, frameIdx, outputInfo.decodingSuccess, outputInfo.errorReason, outputInfo.errorReasonExt);
            }
        }

        if (success == FALSE) {
            break;
        }
        if (sequenceChangeFlag) {
            BOOL     dpbChanged, sizeChanged, bitDepthChanged;

            dpbChanged      = (sequenceChangeFlag&SEQ_CHANGE_ENABLE_DPB_COUNT) ? TRUE : FALSE;
            sizeChanged     = (sequenceChangeFlag&SEQ_CHANGE_ENABLE_SIZE)      ? TRUE : FALSE;
            bitDepthChanged = (sequenceChangeFlag&SEQ_CHANGE_ENABLE_BITDEPTH)  ? TRUE : FALSE;

            if (dpbChanged || sizeChanged || bitDepthChanged) {
                DecOutputInfo*      pDecOutInfo = (DecOutputInfo*)osal_malloc(sizeof(DecOutputInfo)*MAX_GDI_IDX);
                BOOL                remainingFbs[MAX_REG_FRAME];
                Int32               fbIndex;
                Uint32              retNum, dispFlag;
                Uint32              curSeqNo    = outputInfo.sequenceNo;
                Uint32              seqMemIndex = curSeqNo % MAX_SEQUENCE_MEM_COUNT;
                SequenceMemInfo*    pSeqMem     = &seqMemInfo[seqMemIndex];
                DecGetFramebufInfo  prevSeqFbInfo;

                VLOG(INFO, "----- SEQUENCE CHANGED -----\n");

                Queue_Enqueue(sequenceQ, (void*)&curSeqNo);
                osal_memset((void*)remainingFbs, 0x00, sizeof(remainingFbs));
                // Get previous memory related to framebuffer
                VPU_DecGiveCommand(handle, DEC_GET_FRAMEBUF_INFO, (void*)&prevSeqFbInfo);
                // Get current(changed) sequence information. 
                VPU_DecGiveCommand(handle, DEC_GET_SEQ_INFO, &initialInfo);
                // Flush all remaining framebuffers of previous sequence.
                VPU_DecFrameBufferFlush(handle, pDecOutInfo, &retNum);

                VLOG(INFO, "sequenceChanged : %x\n", sequenceChangeFlag);
                VLOG(INFO, "SEQUENCE NO : %d\n", initialInfo.sequenceNo);
                VLOG(INFO, "DPB COUNT: %d\n", initialInfo.minFrameBufferCount);
                VLOG(INFO, "BITDEPTH : LUMA(%d), CHROMA(%d)\n", initialInfo.lumaBitdepth, initialInfo.chromaBitdepth);
                VLOG(INFO, "SIZE     : WIDTH(%d), HEIGHT(%d)\n", initialInfo.picWidth, initialInfo.picHeight);

                // Free allocated framebuffers except ones to be displayed.
                for (index=0; index<retNum; index++) {
                    fbIndex = pDecOutInfo[index].indexFrameDisplay;
                    if (fbIndex >= 0) {
                        VLOG(INFO, "PUSH SEQ[%02d] LINEAR(%02d) COMPRESSED(%02d)\n", 
                            curSeqNo, pDecOutInfo[index].indexFrameDisplay, pDecOutInfo[index].indexFrameDisplayForTiled);
                        Queue_Enqueue(displayQ, (void*)&pDecOutInfo[index]);
                        if (decOP.wtlEnable == TRUE) {
                            fbIndex = VPU_CONVERT_WTL_INDEX(handle, fbIndex);
                            remainingFbs[fbIndex] = TRUE;
                        }
                        else {
                            fbIndex = pDecOutInfo[index].indexFrameDisplayForTiled;
                            remainingFbs[fbIndex] = TRUE;
                        }
                    }
                }

                /* Check Not displayed framebuffers */
                dispFlag = outputInfo.frameDisplayFlag;
                for (index=0; index<MAX_GDI_IDX; index++) {
                    fbIndex = index;
                    if ((dispFlag>>index) & 0x01) {
                        if (decOP.wtlEnable == TRUE) {
                            fbIndex = VPU_CONVERT_WTL_INDEX(handle, fbIndex);
                        }
                        remainingFbs[fbIndex] = TRUE;
                    }
                }

                for (index=0; index<MAX_REG_FRAME; index++) {
                    if (remainingFbs[index] == FALSE) { 
                        // free allocated framebuffer
                        if (pFbMem[index].size > 0) {
                            vdi_free_dma_memory(coreIdx, &pFbMem[index]);
                        }
                    }
                }
                // Free all framebuffers 
                if (handle->codecMode == C7_VP9_DEC || handle->codecMode == C7_AVS2_DEC) {
                    for ( index=0 ; index<MAX_REG_FRAME; index++) {
                        if(prevSeqFbInfo.vbMvCol[index].size > 0) {
                            vdi_free_dma_memory(coreIdx, &prevSeqFbInfo.vbMvCol[index]);
                        }
                        if(prevSeqFbInfo.vbFbcYTbl[index].size > 0) {
                            vdi_free_dma_memory(coreIdx, &prevSeqFbInfo.vbFbcYTbl[index]);
                        }
                        if(prevSeqFbInfo.vbFbcCTbl[index].size > 0) {
                            vdi_free_dma_memory(coreIdx, &prevSeqFbInfo.vbFbcCTbl[index]);
                        }
                    }
                }
                osal_memset(pSeqMem, 0x00, sizeof(SequenceMemInfo));
                osal_memcpy(&pSeqMem->fbInfo, &prevSeqFbInfo, sizeof(DecGetFramebufInfo));
                osal_memcpy(pSeqMem->allocFbMem, pFbMem, sizeof(pFbMem));

                VPU_DecGiveCommand(handle, DEC_RESET_FRAMEBUF_INFO, NULL);


                compressedFbCount = initialInfo.minFrameBufferCount + EXTRA_FRAME_BUFFER_NUM;       /* max_dec_pic_buffering + @, @ >= 1 */
                if (handle->codecMode == C7_VP9_DEC || handle->codecMode == C7_AVS2_DEC)
                    linearFbCount     = compressedFbCount;
                else
                    linearFbCount     = initialInfo.frameBufDelay + (1+EXTRA_FRAME_BUFFER_NUM*2);       /* max_num_reorder_pics + @,  @ >= 1, 
                                                                                                        In most case, # of linear fbs must be greater or equal than max_num_reorder,
                                                                                                        but the expression of @ in the sample code is in order to make the situation
                                                                                                        that # of linear is greater than # of fbc. */
                osal_memset((void*)pFbMem, 0x00, sizeof(vpu_buffer_t)*MAX_REG_FRAME);
                if (AllocateDecFrameBuffer(handle, param, compressedFbCount, linearFbCount, pFrame, pFbMem, &framebufStride) == FALSE) {
                    success = FALSE;
                    if (pDecOutInfo != NULL) {
                        osal_free(pDecOutInfo);
                    }
                    VLOG(ERR, "[SEQ_CHANGE] AllocateDecFrameBuffer failed Error code is 0x%x\n", success);
                    goto ERR_DEC_OPEN;
                }
                ret = VPU_DecRegisterFrameBufferEx(handle, pFrame, compressedFbCount, linearFbCount, 
                    framebufStride, initialInfo.picHeight, COMPRESSED_FRAME_MAP);
                if (ret != RETCODE_SUCCESS) {
                    success = FALSE;
                    if (pDecOutInfo != NULL) {
                        osal_free(pDecOutInfo);
                    }
                    VLOG(ERR, "[SEQ_CHANGE] VPU_DecRegisterFrameBufferEx failed Error code is 0x%x\n", ret);
                    goto ERR_DEC_OPEN;
                }
                /* 
                To reuse bitstream buffer after sequence changed on VP9
                When detected sequence change on VP9,it's not necessary to load next frame.
                VP9 can detect sequence change only on KEY_FRAME.
                So need to reuse same chunk, after udpate frame buffer
                */
                if (handle->codecMode == C7_VP9_DEC)
                    needStream = FALSE;

                VLOG(INFO, "----------------------------\n");

                if (pDecOutInfo != NULL) {
                    osal_free(pDecOutInfo);
                }
            }
        }
        if (handle->codecMode == C7_VP9_DEC && 
            (outputInfo.sequenceChanged&SEQ_CHANGE_INTER_RES_CHANGE || 
            ((outputInfo.indexInterFrameDecoded != -1) && outputInfo.indexFrameDecoded == DECODED_IDX_FLAG_NO_FB))) {
                Uint32      picWidth, picHeight;
                size_t      framebufHeight;
                Int32       fbcCurFrameIdx = 0;
                Int32       bwbCurFrameIdx = 0;
                Int32       i;

                VLOG(INFO, "----- INTER RESOLUTION CHANGED -----\n");
                fbcCurFrameIdx  = (short)(outputInfo.indexInterFrameDecoded & 0xffff);
                bwbCurFrameIdx  = (short)((outputInfo.indexInterFrameDecoded >> 16) & 0xffff);

                picWidth = outputInfo.decPicWidth;
                picHeight = outputInfo.decPicHeight;

                VPU_DecGiveCommand(handle, DEC_SET_BWB_CUR_FRAME_IDX, (void*)&bwbCurFrameIdx);
                VPU_DecGiveCommand(handle, DEC_SET_FBC_CUR_FRAME_IDX, (void*)&fbcCurFrameIdx);

                VLOG(INFO, "Prev SIZE         : WIDTH(%d), HEIGHT(%d)\n", initialInfo.picWidth, initialInfo.picHeight);
                VPU_DecGiveCommand(handle, DEC_GET_SEQ_INFO, &initialInfo);
                VLOG(INFO, "SEQUENCE NO       : %d\n", initialInfo.sequenceNo);
                VLOG(INFO, "FBC CUR FRAME IDX : %d\n", fbcCurFrameIdx);
                VLOG(INFO, "BWB CUR FRAME IDX : %d\n", bwbCurFrameIdx);
                VLOG(INFO, "SIZE              : WIDTH(%d), HEIGHT(%d)\n", initialInfo.picWidth, initialInfo.picHeight);

                if (outputInfo.indexInterFrameDecoded != -1) {
                    for ( i=0 ; i < linearFbCount ; i++) {
                        VPU_DecClrDispFlag(handle, i);
                    }
                }

                if(fbcCurFrameIdx >= 0){
                    VPU_DecGiveCommand(handle, DEC_FREE_MV_BUFFER, (void*)&fbcCurFrameIdx);
                    VPU_DecGiveCommand(handle, DEC_FREE_FBC_TABLE_BUFFER, (void*)&fbcCurFrameIdx);

                    VPU_DecGiveCommand(handle, DEC_ALLOC_MV_BUFFER, (void*)&fbcCurFrameIdx);
                    VPU_DecGiveCommand(handle, DEC_ALLOC_FBC_Y_TABLE_BUFFER, (void*)&fbcCurFrameIdx);
                    VPU_DecGiveCommand(handle, DEC_ALLOC_FBC_C_TABLE_BUFFER, (void*)&fbcCurFrameIdx);
                }


                if ((fbcCurFrameIdx >=0) || (bwbCurFrameIdx >=0)) {
                    FrameBufferAllocInfo    fbAllocInfo;
                    FrameBufferFormat       format = FORMAT_420;

                    if (pFbMem[fbcCurFrameIdx].size > 0) {
                        vdi_free_dma_memory(coreIdx, &pFbMem[fbcCurFrameIdx]);
                        osal_memset((void*)&pFbMem[fbcCurFrameIdx], 0x00, sizeof(vpu_buffer_t));
                    }

                    compressedFbCount = initialInfo.minFrameBufferCount + EXTRA_FRAME_BUFFER_NUM;
                    linearFbCount     = compressedFbCount;

                    osal_memset((void*)&fbAllocInfo, 0x00, sizeof(fbAllocInfo));

                    format         = (initialInfo.lumaBitdepth > 8 || initialInfo.chromaBitdepth > 8) ? FORMAT_420_P10_16BIT_LSB : FORMAT_420;
                    framebufStride = CalcStride(VPU_ALIGN64(picWidth), picHeight, format, param->cbcrInterleave, param->mapType, TRUE);
                    framebufHeight = VPU_ALIGN64(picHeight);

                    fbAllocInfo.format          = format;
                    fbAllocInfo.cbcrInterleave  = param->cbcrInterleave;
                    fbAllocInfo.mapType         = param->mapType;
                    fbAllocInfo.stride          = framebufStride;
                    fbAllocInfo.height          = handle->codecMode == C7_VP9_DEC ? framebufHeight : initialInfo.picHeight;
                    fbAllocInfo.lumaBitDepth    = initialInfo.lumaBitdepth;
                    fbAllocInfo.chromaBitDepth  = initialInfo.chromaBitdepth;
                    fbAllocInfo.num             = compressedFbCount;
                    fbAllocInfo.endian          = param->frameEndian;
                    fbAllocInfo.type            = FB_TYPE_CODEC;

                    pFbMem[fbcCurFrameIdx].size = VPU_GetFrameBufSize(handle->coreIdx, framebufStride, framebufHeight, 
                        param->mapType, format, param->cbcrInterleave, NULL);
                    if (vdi_allocate_dma_memory(coreIdx, &pFbMem[fbcCurFrameIdx]) < 0) {
                        VLOG(ERR, "%s:%d fail to allocate frame buffer\n", __FUNCTION__, __LINE__);
                        return FALSE;
                    }
                    pFrame[fbcCurFrameIdx].bufY  =  pFbMem[fbcCurFrameIdx].phys_addr;
                    pFrame[fbcCurFrameIdx].bufCb = (PhysicalAddress)-1;
                    pFrame[fbcCurFrameIdx].bufCr = (PhysicalAddress)-1;
                    pFrame[fbcCurFrameIdx].updateFbInfo = TRUE;

                    if ((ret=VPU_DecAllocateFrameBuffer(handle, fbAllocInfo, pFrame)) != RETCODE_SUCCESS) {
                        VLOG(ERR, "%s:%d failed to VPU_DecAllocateFrameBuffer(), ret(%d)\n", 
                            __FUNCTION__, __LINE__, ret);
                        //ReleaseVideoMemory(coreIdx, pFbMem[curFrameIdx], compressedFbCount);
                        for (index=0; index<(Uint32)compressedFbCount; index++) {
                            if (pFbMem[index].size) 
                                vdi_free_dma_memory(coreIdx, &pFbMem[index]);
                        }
                        return FALSE;
                    }

                    if (param->enableWTL == TRUE) {
                        size_t  linearStride;
                        size_t  picWidth;
                        size_t  picHeight;
                        size_t  fbHeight;
                        Uint32   linearCurFrameIdx = bwbCurFrameIdx + compressedFbCount;
                        Uint8   mapType = LINEAR_FRAME_MAP;
                        FrameBufferAllocInfo    fbAllocInfo;
                        FrameBufferFormat       outFormat = param->wtlFormat;

                        if (pFbMem[linearCurFrameIdx].size > 0) {
                            vdi_free_dma_memory(coreIdx, &pFbMem[linearCurFrameIdx]);
                            osal_memset((void*)&pFbMem[linearCurFrameIdx], 0x00, sizeof(vpu_buffer_t));
                        }

                        picWidth  = initialInfo.picWidth;
                        picHeight = initialInfo.picHeight;
                        fbHeight  = VPU_ALIGN64(picHeight);
                        linearStride = CalcStride(VPU_ALIGN64(picWidth), picHeight, outFormat, param->cbcrInterleave, (TiledMapType)mapType, TRUE);
                        pFbMem[linearCurFrameIdx].size  = VPU_GetFrameBufSize(coreIdx, linearStride, fbHeight, mapType, outFormat, param->cbcrInterleave, NULL);

                        if (vdi_allocate_dma_memory(coreIdx, &pFbMem[linearCurFrameIdx]) < 0) {
                            VLOG(ERR, "%s:%d fail to allocate frame buffer\n", __FUNCTION__, __LINE__);
                            ReleaseVideoMemory(coreIdx, &pFbMem[linearCurFrameIdx], compressedFbCount);
                            for (index=0; index<(Uint32)compressedFbCount; index++) {
                                if (pFbMem[index].size) 
                                    vdi_free_dma_memory(coreIdx, &pFbMem[index]);
                            }
                            return FALSE;
                        }
                        pFrame[linearCurFrameIdx].bufY  = pFbMem[linearCurFrameIdx].phys_addr;
                        pFrame[linearCurFrameIdx].bufCb = (PhysicalAddress)-1;
                        pFrame[linearCurFrameIdx].bufCr = (PhysicalAddress)-1;
                        pFrame[linearCurFrameIdx].updateFbInfo = TRUE;

                        fbAllocInfo.nv21    = param->nv21;
                        fbAllocInfo.cbcrInterleave  = param->cbcrInterleave;
                        fbAllocInfo.format  = outFormat;
                        fbAllocInfo.num     = linearFbCount;
                        fbAllocInfo.mapType = LINEAR_FRAME_MAP;
                        fbAllocInfo.stride  = linearStride;
                        fbAllocInfo.height  = fbHeight;
                        fbAllocInfo.type    = FB_TYPE_CODEC;
                        fbAllocInfo.endian          = param->frameEndian;
                        fbAllocInfo.lumaBitDepth    = initialInfo.lumaBitdepth;
                        fbAllocInfo.chromaBitDepth  = initialInfo.chromaBitdepth;

                        ret = VPU_DecAllocateFrameBuffer(handle, fbAllocInfo, &pFrame[compressedFbCount]);
                        if (ret != RETCODE_SUCCESS) {
                            VLOG(ERR, "%s:%d failed to VPU_DecAllocateFrameBuffer() ret:%d\n",
                                __FUNCTION__, __LINE__, ret);
                            ReleaseVideoMemory(coreIdx, pFbMem, linearFbCount+compressedFbCount);
                            return FALSE;
                        }
                    }

                    VPU_DecGiveCommand(handle, DEC_SET_INTER_RES_INFO_ON, NULL);
                    ret = VPU_DecRegisterFrameBufferEx(handle, pFrame, compressedFbCount, linearFbCount, 
                        framebufStride, initialInfo.picHeight, COMPRESSED_FRAME_MAP);
                    VPU_DecGiveCommand(handle, DEC_SET_INTER_RES_INFO_OFF, NULL);

                    if (ret != RETCODE_SUCCESS) {
                        success = FALSE;
                        VLOG(ERR, "[SEQ_CHANGE] VPU_DecRegisterFrameBufferEx failed Error code is 0x%x\n", ret);
                        goto ERR_DEC_OPEN;
                    }
                    // To reuse bitstream buffer after sequence changed on VP9
                    /* 
                    When detected sequence change on VP9,it's not necessary to load next frame.
                    VP9 can detect sequence change only on KEY_FRAME.
                    So need to reuse same chunk, after udpate frame buffer
                    */               
                    needStream = FALSE;
                }
#ifdef SUPPORT_REUSE_BUFFER
skip_realloc:
#endif
                VLOG(INFO, "-----INTER SEQUENCE CHANGE END -----\n");
        } 

        if (noFbCount <= 1 && outputInfo.indexFrameDisplay != DISPLAY_IDX_FLAG_SEQ_END) {
            if (sequenceChangeFlag == 0) {
                Queue_Enqueue(displayQ, (void*)&outputInfo);
            }
        }

        repeat = TRUE;
        do {
            if ((pDisplayInfo=(DecOutputInfo*)Queue_Dequeue(displayQ)) == NULL) 
                break;

            osal_memcpy((void*)&outputInfo, pDisplayInfo, sizeof(DecOutputInfo));
            DisplayDecodedInformation(handle, decOP.bitstreamFormat, frameIdx, &outputInfo);
            if (outputInfo.indexFrameDecoded < 0 &&
                outputInfo.indexFrameDisplay == DISPLAY_IDX_FLAG_NO_FB) {
                    notDecodedCount++;
                    continue;
            }
            break;
        } while (repeat == TRUE);

        if (pDisplayInfo != NULL) {
            if (outputInfo.indexFrameDisplay >= 0) {
                Uint32      width=0, height=0, Bpp;
                size_t      frameSizeInByte=0;
                Uint8*      pYuv = NULL;
                void*       decodedData = NULL;
                Uint32      decodedDataSize = 0;
                VpuRect     rcDisplay;

                rcDisplay.left   = 0;
                rcDisplay.top    = 0;
                rcDisplay.right  = outputInfo.dispPicWidth;
                rcDisplay.bottom = outputInfo.dispPicHeight;
                if (doDumpImage == TRUE) {
                        if (strlen(param->outputPath) > 0) {
                            if (fpOutput[0] == NULL) {
                                if (OpenDisplayBufferFile(param->outputPath, rcDisplay, (TiledMapType)outputInfo.dispFrame.mapType, fpOutput) == FALSE)
                                    goto ERR_DEC_OPEN;
                            }
                            SaveDisplayBufferToFile(handle, outputInfo.dispFrame, rcDisplay, fpOutput);
                        }
                        if (param->compareType == YUV_COMPARE || rendererType == RENDER_DEVICE_FBDEV) {
                            pYuv = GetYUVFromFrameBuffer(handle, &outputInfo.dispFrame, rcDisplay, &width, &height, &Bpp, &frameSizeInByte);
                        }
                }
                else {
                    width  = outputInfo.dispPicWidth;
                    height = outputInfo.dispPicHeight;
                }

#ifdef SUPPORT_SAVE_PIC_INFO_TO_XML
                SavePicInfoToXml(fpXml, &outputInfo);
#endif

                switch (param->compareType) {
                case NO_COMPARE:
                    break;
                case YUV_COMPARE:
                    decodedData     = (void*)pYuv;
                    decodedDataSize = frameSizeInByte;
                    break;
                }

                repeat = TRUE;
                while(repeat == TRUE)
                {
                    if ((success=Comparator_Act(comparator, decodedData, decodedDataSize)) == TRUE)
                    {
                        break;
                    }
                    EnterLock(coreIdx);
                    HandleDecoderError(handle, frameIdx, param, pFbMem, &outputInfo);
                    LeaveLock(coreIdx);
                    goto ERR_DEC_OPEN;
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

            if (outputInfo.indexFrameDecoded >= 0) {
                frameIdx++;
                notDecodedCount = 0;
            }
        }
        if (decOP.bitstreamMode == BS_MODE_PIC_END) {
            needStream = TRUE;
            if (decodedIndex == DECODED_IDX_FLAG_NO_FB) {
                needStream = FALSE;
            }

            //super frame(many frames in one packet)
            if (handle->codecMode == C7_VP9_DEC) {
                if (handle->CodecInfo->decInfo.streamRdPtr < handle->CodecInfo->decInfo.streamWrPtr)
                    needStream = FALSE;
            } 
            if (handle->codecMode == C7_HEVC_DEC ) {
                if (prescanIndex == -1 || sequenceChangeFlag != 0) {
                    needStream = FALSE;
                    VPU_DecSetRdPtr(handle, vbStream[bsQueueIndex].phys_addr, FALSE);
                }
            }
        }
        if (outputInfo.indexFrameDisplay == DISPLAY_IDX_FLAG_SEQ_END) { 
            if (Queue_Peek(displayQ) != NULL) {
                VLOG(ERR, "Queue_Peek=%d\n", Queue_Peek(displayQ));
                success = FALSE;
            }
            if (loopCount > 0) {
                loopCount--;
                BitstreamFeeder_Rewind(feeder);
                Comparator_Rewind(comparator);
                VPU_DecUpdateBitstreamBuffer(handle, -1);
            }
            else {
                break;
            }
        }
    }

ERR_DEC_OPEN:
    // Now that we are done with decoding, close the open instance.
    VPU_DecUpdateBitstreamBuffer(handle, STREAM_END_SIZE);

    if ( sequenceChangeOccurred == 0 )
    {
        if (success == TRUE && param->compareType && !param->forceOutNum) {
            success = Comparator_CheckFrameCount(comparator);
        }
    }

    /* Release all previous sequence resources */
    if ( handle ) {
        for (index=0; index<MAX_SEQUENCE_MEM_COUNT; index++) {
            ReleasePreviousSequenceResources(handle, seqMemInfo[index].allocFbMem, &seqMemInfo[index].fbInfo);
        }
    }

    /* Release current sequence resources */
    for (index=0; index<MAX_REG_FRAME; index++) {
        if (pFbMem[index].size > 0)
            vdi_free_dma_memory(coreIdx, &pFbMem[index]);
    }

    for (index=0; index<bsBufferCount; index++) {
        vdi_free_dma_memory(coreIdx, &vbStream[index]);
    }

    vdi_free_dma_memory(coreIdx, &vbUserData);
    VPU_DecClose(handle);

    VLOG(INFO, "\ninst %d Dec End. Tot Frame %d\n", instIdx, frameIdx);

ERR_DEC_INIT:
    CloseDisplayBufferFile(fpOutput);
    if (feeder != NULL)     BitstreamFeeder_Destroy(feeder);
    if (renderer != NULL)   SimpleRenderer_Destroy(renderer);
    if (comparator != NULL) Comparator_Destroy(comparator);
    if (displayQ != NULL)   Queue_Destroy(displayQ);
    if (sequenceQ != NULL)  Queue_Destroy(sequenceQ);

#ifdef SUPPORT_SAVE_PIC_INFO_TO_XML
    ClosePicInfoXMLFile(fpXml);
#endif

    VPU_DeInit(coreIdx);

    return success;
}

int main(int argc, char **argv)
{
    Int32           opt, index, i;
    BOOL            ret;
    TestDecConfig   testConfig;
    char**          stop                = NULL;
    BOOL            showVersion         = FALSE;
    BOOL            isValidParameters   = TRUE;
    Uint32          productId;
    Uint32          argFeeding          = 0;        /* auto, depeding on --bsmode */
    struct option options[MAX_GETOPT_OPTIONS];
    struct OptionExt options_help[] = {
        {"output",                1, NULL, 0, "--output                    YUV path\n"},    /*  0 */
        {"input",                 1, NULL, 0, "--input                     bitstream path\n"},
        {"codec",                 1, NULL, 0, "--codec                     codec index, HEVC : 12, VP9 : 13, AVS2 : 14\n"},
        {"render",                1, NULL, 0, "--render                    0 : no rendering picture\n"
                                              "                            1 : render a picture with the framebuffer device\n"},
        {"bsmode",                1, NULL, 0, "--bsmode                    0: INTERRUPT MODE, 1: reserved, 2: PICEND MODE\n"},
        {"disable-wtl",           0, NULL, 0, "--disable-wtl               disable WTL. default on.\n"},    /*  5 */
        {"coreIdx",               1, NULL, 0, "--coreIdx                   core index: default 0\n"},
        {"loop-count",            1, NULL, 0, "--loop-count                integer number. loop test, default 0\n"},
        {"enable-cbcrinterleave", 0, NULL, 0, "--enable-cbcrinterleave     enable cbcrInterleave(NV12), default off\n"},
        {"stream-endian",         1, NULL, 0, "--stream-endian             16~31, default 31(LE) Please refer programmer's guide or datasheet\n"},
        {"frame-endian",          1, NULL, 0, "--frame-endian              16~31, default 31(LE) Please refer programmer's guide or datasheet\n"},
        {"enable-nv21",           0, NULL, 0, "--enable-nv21               enable NV21, default off\n"},
        {"secondary-axi",         1, NULL, 0, "--secondary-axi             0~7: bit oring values, Please refer programmer's guide or datasheet\n"},
        {"wtl-format",            1, NULL, 0, "--wtl-format                yuv format. default 0.\n"},
        {"num-vcores",            1, NULL, 0, "--num-vcores                number vcores to be used, default 1\n"},
        {"enable-thumbnail",      0, NULL, 0, "--enable-thumbnail          enable thumbnail mode. default off\n"},
        {"ref-yuv",               1, NULL, 0, "--ref-yuv                   golden yuv path\n"},
        {"skip-mode",             1, NULL, 0, "--skip-mode                 0: off, 1: Non-IRAP, 2: Non-Ref, 4: skip-all\n"},
        {"fbc-mode",              1, NULL, 0, "--fbc-mode                  fbc-mode\n"},
        {"bwopt",                 1, NULL, 0, "--bwopt                     bandwidth opimization\n"},
        {"cra-bla",               0, NULL, 0, "--cra-bla                   Handle CRA as BLA\n"},
        {"userdata",              1, NULL, 0, "--userdata      hexadecimal 0 - disable userdata         1 - Reserved               2 - Reserved          4 - VUI\n"
                                              "                            8 - mastering_color_volume  10 - pic_timing            20 - itu_t_35 prefix  40 - unregistered prefix\n"
                                              "                            80 - itu_t_35 suffix        100 - unregistered suffix 200 - recovery point  400 - mastering_display_color\n"
                                              "                            All bits can be or-ing. ex) 404 -> mastering_color_volume | VUI\n"},
        {"feeding",               1, NULL, 0, "--feeding                   0: auto, 1: fixed-size, 2: ffempg, 3: size(4byte)+es\n"},    /* 25 */
        {NULL,                    0, NULL, 0},
    };
    char*           optString = "c:hivn:";
    char*           fwPath    = NULL;


    InitLog();

    //default setting.
    osal_memset(&testConfig, 0, sizeof(testConfig));

    testConfig.bitstreamMode   = BS_MODE_INTERRUPT;
    testConfig.feedingMode     = FEEDING_METHOD_FIXED_SIZE;
    testConfig.streamEndian    = VPU_STREAM_ENDIAN;
    testConfig.frameEndian     = VPU_FRAME_ENDIAN;
    testConfig.cbcrInterleave  = FALSE;
    testConfig.nv21            = FALSE;
    testConfig.bitFormat       = STD_HEVC;
    testConfig.renderType      = RENDER_DEVICE_NULL;
    testConfig.mapType         = COMPRESSED_FRAME_MAP;
    testConfig.enableWTL       = TRUE;
    testConfig.wtlMode         = FF_FRAME;
    testConfig.wtlFormat       = FORMAT_420;            //!<< 8 bit YUV
    testConfig.wave4.fbcMode   = 0x0c;
    testConfig.bsSize          = STREAM_BUF_SIZE_HEVC;
    testConfig.wave4.numVCores = NUM_VCORES;
    testConfig.wave4.bwOptimization = FALSE;            //!<< TRUE: non-ref pictures are not stored at reference picture buffer
                                                        //!<<       for saving bandwidth.


    for (i = 0; i < MAX_GETOPT_OPTIONS;i++) {
        if (options_help[i].name == NULL)
            break;
        osal_memcpy(&options[i], &options_help[i], sizeof(struct option));
    }
    while ((opt=getopt_long(argc, argv, optString, options, &index)) != -1) {
        switch (opt) {
        case 'c':
            testConfig.compareType = atoi(optarg);
            if (testConfig.compareType < NO_COMPARE || testConfig.compareType > YUV_COMPARE) {
                VLOG(ERR, "Invalid compare type(%d)\n", testConfig.compareType);
                Help(options_help, argv[0]);
                exit(1);
            }
            break;
        case 'n':
            testConfig.forceOutNum = atoi(optarg);
            break;
        case 'h':
            Help(options_help, argv[0]);
            return 0;
        case 'v':
            showVersion = TRUE;
            break;
        case 0:
            if (!strcmp(options[index].name, "output")) {
                memcpy(testConfig.outputPath, optarg, strlen(optarg));
                ChangePathStyle(testConfig.outputPath);
            } else if (!strcmp(options[index].name, "input")) {
                memcpy(testConfig.inputPath, optarg, strlen(optarg));
                ChangePathStyle(testConfig.inputPath);
            } else if (!strcmp(options[index].name, "codec")) {
                testConfig.bitFormat = atoi(optarg);
            } else if (!strcmp(options[index].name, "render")) {
                testConfig.renderType = (RenderDeviceType)atoi(optarg);
                if (testConfig.renderType < RENDER_DEVICE_NULL || testConfig.renderType >= RENDER_DEVICE_MAX) {
                    VLOG(ERR, "unknown render device type(%d)\n", testConfig.renderType);
                    Help(options_help, argv[0]);
                    return 1;
                }
            } else if (!strcmp(options[index].name, "bsmode")) {
                testConfig.bitstreamMode = atoi(optarg);
            } else if (!strcmp(options[index].name, "disable-wtl")) {
                testConfig.enableWTL = FALSE;
                testConfig.wtlMode = FF_NONE;
            } else if (!strcmp(options[index].name, "coreIdx")) {
                testConfig.coreIdx = atoi(optarg);
            } else if (!strcmp(options[index].name, "loop-count")) {
                testConfig.loopCount = atoi(optarg);
            } else if (!strcmp(options[index].name, "enable-cbcrinterleave")) {
                testConfig.cbcrInterleave = TRUE;
            } else if (!strcmp(options[index].name, "stream-endian")) {
                testConfig.streamEndian = (EndianMode)atoi(optarg);
            } else if (!strcmp(options[index].name, "frame-endian")) {
                testConfig.frameEndian = (EndianMode)atoi(optarg);
            } else if (!strcmp(options[index].name, "enable-nv21")) {
                testConfig.nv21 = 1;
            } else if (!strcmp(options[index].name, "secondary-axi")) {
                testConfig.secondaryAXI = strtoul(optarg, stop, !strncmp("0x", optarg, 2) ? 16 : 10);
            } else if (!strcmp(options[index].name, "wtl-format")) {
                testConfig.wtlFormat = (FrameBufferFormat)atoi(optarg);
            } else if (!strcmp(options[index].name, "num-vcores")) {
                testConfig.wave4.numVCores = atoi(optarg);
            } else if (!strcmp(options[index].name, "enable-thumbnail")) {
                testConfig.thumbnailMode = TRUE;
            } else if (!strcmp(options[index].name, "ref-yuv")) {
                memcpy(testConfig.refYuvPath, optarg, strlen(optarg));
                ChangePathStyle(testConfig.refYuvPath);
            } else if (!strcmp(options[index].name, "skip-mode")) {
                testConfig.skipMode = atoi(optarg);
            } else if (!strcmp(options[index].name, "fbc-mode")) {
                testConfig.wave4.fbcMode = strtoul(optarg, stop, !strncmp("0x", optarg, 2) ? 16 : 10);
            } else if (!strcmp(options[index].name, "bwopt")) {
                testConfig.wave4.bwOptimization = atoi(optarg);
            } else if (!strcmp(options[index].name, "cra-bla")) {
                testConfig.wave4.craAsBla = TRUE;
            } else if (!strcmp(options[index].name, "userdata")) {
                testConfig.enableUserData = strtoul(optarg, stop, 16);
            } else if (!strcmp(options[index].name, "feeding")) {
                argFeeding = (Uint32)atoi(optarg);
            } else if (!strcmp(options[index].name, "md5")) {
                memcpy(testConfig.md5Path, optarg, strlen(optarg));
                ChangePathStyle(testConfig.md5Path);
            } else if (!strcmp(options[index].name, "fw-path")) {
                memcpy(testConfig.fwPath, optarg, strlen(optarg));
            } else if (!strcmp(options[index].name, "bs-size")) {
                testConfig.bsSize = (size_t)atoi(optarg);
            } else {
                VLOG(ERR, "not exist param = %s\n", options[index].name);
                Help(options_help, argv[0]);
                return 1;
            }
            break;
        default:
            VLOG(ERR, "%s\n", optarg);
            Help(options_help, argv[0]);
            return 1;
        }
    }
    VLOG(INFO, "\n");


    /* Check parameters */
    if (testConfig.skipMode < 0 || testConfig.skipMode == 3 || testConfig.skipMode > 4) {
        VLOG(ERR, "Invalid skip mode: %d\n", testConfig.skipMode);
        isValidParameters = FALSE;
    }
    if ((FORMAT_422 <= testConfig.wtlFormat && testConfig.wtlFormat <= FORMAT_400) ||
        testConfig.wtlFormat < FORMAT_420 || testConfig.wtlFormat > FORMAT_420_P10_32BIT_LSB) {
        VLOG(ERR, "Invalid WTL format(%d)\n", testConfig.wtlFormat);
        isValidParameters = FALSE;
    }
    if (testConfig.renderType < RENDER_DEVICE_NULL || testConfig.renderType >= RENDER_DEVICE_MAX) {
        VLOG(ERR, "unknown render device type(%d)\n", testConfig.renderType);
        isValidParameters = FALSE;
    }
    if (testConfig.thumbnailMode == TRUE && testConfig.skipMode != 0) {
        VLOG(ERR, "Turn off thumbnail mode or skip mode\n");
        isValidParameters = FALSE;
    }

    switch (argFeeding) {
    case 0: /* AUTO */
        if (testConfig.bitstreamMode == BS_MODE_INTERRUPT) {
            testConfig.feedingMode = FEEDING_METHOD_FIXED_SIZE;
        }
        else {
            testConfig.feedingMode = FEEDING_METHOD_FRAME_SIZE;
        }
        break;
    case 1: /* FIXED SIZE */
        testConfig.feedingMode = FEEDING_METHOD_FIXED_SIZE;
        break;
    case 2: /* Using FFMPEG */
        testConfig.feedingMode = FEEDING_METHOD_FRAME_SIZE;
        break;
    case 3: /* for every frame, size(4byte) + es(1frame) */
        testConfig.feedingMode = FEEDING_METHOD_SIZE_PLUS_ES;
        break;
    default:
        VLOG(ERR, "--feeding=%d not supported\n", argFeeding);
        isValidParameters = FALSE;
        break;
    }

    if (isValidParameters == FALSE) {
        Help(options_help, argv[0]);
        return 1;
    }



    productId = (ProductId)VPU_GetProductId(testConfig.coreIdx);

    switch (productId) {
    case PRODUCT_ID_980:  fwPath = CORE_1_BIT_CODE_FILE_PATH; break;
    case PRODUCT_ID_4102: fwPath = CORE_3_BIT_CODE_FILE_PATH; break;
    case PRODUCT_ID_420:  fwPath = CORE_0_BIT_CODE_FILE_PATH; break;
    case PRODUCT_ID_412:  fwPath = CORE_2_BIT_CODE_FILE_PATH; break;
    case PRODUCT_ID_420L: fwPath = CORE_5_BIT_CODE_FILE_PATH; break;
    case PRODUCT_ID_510:  fwPath = CORE_6_BIT_CODE_FILE_PATH; break;
    case PRODUCT_ID_512:  fwPath = CORE_7_BIT_CODE_FILE_PATH; break;
    case PRODUCT_ID_515:  fwPath = CORE_8_BIT_CODE_FILE_PATH; break;
    default:
        VLOG(ERR, "Unknown product id: %d\n", productId);
        return 1;
    }
    VLOG(INFO, "FW PATH = %s\n", fwPath);
    if (LoadFirmware(productId, (Uint8**)&pusBitCode, &sizeInWord, fwPath) < 0) {
        VLOG(ERR, "%s:%d Failed to load firmware: %s\n", __FUNCTION__, __LINE__, fwPath);
        return 1;
    }

    if (showVersion) {
        Uint32   ver, rev;

        ret = VPU_InitWithBitcode(testConfig.coreIdx, (const Uint16*)pusBitCode, sizeInWord);
        if (ret != RETCODE_CALLED_BEFORE && ret != RETCODE_SUCCESS) {
            VLOG(ERR, "Failed to boot up VPU(coreIdx: %d, productId: %d)\n",
                __FUNCTION__, __LINE__, testConfig.coreIdx, productId);
            osal_free(pusBitCode);
            return 1;
        }
        VPU_GetVersionInfo(testConfig.coreIdx, &ver, &rev, &productId);
        VPU_DeInit(testConfig.coreIdx);
        printf("VERSION=%d\n", ver);
        printf("REVISION=%d\n", rev);
        printf("PRODUCT_ID=%d\n", productId);
        osal_free(pusBitCode);
        return 0;
    }

    if (strlen(testConfig.inputPath) == 0) {
        VLOG(ERR, "No input bitstream\n");
        Help(options_help, argv[0]);
        osal_free(pusBitCode);
        return 1;
    }

    osal_init_keyboard();

    InitializeDebugEnv(productId, testConfig.testEnvOptions);

    ret = TestDecoder(&testConfig);

    ReleaseDebugEnv();

    osal_close_keyboard();
    osal_free(pusBitCode);

    return ret == TRUE ? 0 : 1;
}
 
