/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name:vi_vreg_blocks.h
 * Description:HW register description
 */

#ifndef _VI_VREG_BLOCKS_H_
#define _VI_VREG_BLOCKS_H_

#ifdef __cplusplus
extern "C" {
#endif


/******************************************/
/*          CMDSET Common Define          */
/******************************************/

struct ISPCQ_ADMA_DESC_T {
	union {
		uint64_t    cmdset_addr;
		uint64_t    link_addr;
	};
	uint32_t    cmdset_size;
	union {
		uint32_t _RSV0;
		struct {
			uint32_t END    : 1;
			uint32_t _RSV1  : 1;
			uint32_t _RSV2  : 1;
			uint32_t LINK   : 1;
		} flag;
	};
};

union CMDSET_FIELD {
	uint32_t raw;
	struct {
		uint32_t REG_ADDR           : 20;
		uint32_t BWR_MASK           : 4;
		uint32_t FLAG_END           : 1;
		uint32_t FLAG_INT           : 1;
		uint32_t FLAG_LAST          : 1;
		uint32_t FLAG_RSV           : 1;
		uint32_t ACT                : 4;
	} bits;
};

struct VREG_RESV {
	uint32_t                        resv;
	union CMDSET_FIELD              nop;
};


/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_PRE_RAW_FE_PRE_RAW_CTRL {
	union REG_PRE_RAW_FE_PRE_RAW_CTRL       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_PRE_RAW_FE_PRE_RAW_FRAME_SIZE {
	union REG_PRE_RAW_FE_PRE_RAW_FRAME_SIZE  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_PRE_RAW_FE_PRE_RAW_POST_NO_RSPD_CYC {
	union REG_PRE_RAW_FE_PRE_RAW_POST_NO_RSPD_CYC  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_PRE_RAW_FE_PRE_RAW_FRAME_VLD {
	union REG_PRE_RAW_FE_PRE_RAW_FRAME_VLD  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_PRE_RAW_FE_PRE_RAW_DEBUG_STATE {
	union REG_PRE_RAW_FE_PRE_RAW_DEBUG_STATE  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_PRE_RAW_FE_PRE_RAW_DUMMY {
	union REG_PRE_RAW_FE_PRE_RAW_DUMMY      write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_PRE_RAW_FE_PRE_RAW_DEBUG_INFO {
	union REG_PRE_RAW_FE_PRE_RAW_DEBUG_INFO  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_PRE_RAW_FE_IDLE_INFO {
	union REG_PRE_RAW_FE_IDLE_INFO          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_PRE_RAW_FE_CHECK_SUM {
	union REG_PRE_RAW_FE_CHECK_SUM          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_PRE_RAW_FE_CHECK_SUM_VALUE {
	union REG_PRE_RAW_FE_CHECK_SUM_VALUE    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_PRE_RAW_FE_T {
	struct VREG_PRE_RAW_FE_PRE_RAW_CTRL      PRE_RAW_CTRL;
	struct VREG_PRE_RAW_FE_PRE_RAW_FRAME_SIZE  PRE_RAW_FRAME_SIZE;
	struct VREG_RESV                        _resv_0x8[6];
	struct VREG_PRE_RAW_FE_PRE_RAW_POST_NO_RSPD_CYC  PRE_RAW_POST_NO_RSPD_CYC;
	struct VREG_RESV                        _resv_0x24[1];
	struct VREG_PRE_RAW_FE_PRE_RAW_FRAME_VLD  PRE_RAW_FRAME_VLD;
	struct VREG_PRE_RAW_FE_PRE_RAW_DEBUG_STATE  PRE_RAW_DEBUG_STATE;
	struct VREG_PRE_RAW_FE_PRE_RAW_DUMMY     PRE_RAW_DUMMY;
	struct VREG_PRE_RAW_FE_PRE_RAW_DEBUG_INFO  PRE_RAW_INFO;
	struct VREG_RESV                        _resv_0x38[6];
	struct VREG_PRE_RAW_FE_IDLE_INFO         FE_IDLE_INFO;
	struct VREG_RESV                        _resv_0x54[3];
	struct VREG_PRE_RAW_FE_CHECK_SUM         FE_CHECK_SUM;
	struct VREG_PRE_RAW_FE_CHECK_SUM_VALUE   FE_CHECK_SUM_VALUE;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_CSI_BDG_TOP_CTRL {
	union REG_ISP_CSI_BDG_TOP_CTRL          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_INTERRUPT_CTRL {
	union REG_ISP_CSI_BDG_INTERRUPT_CTRL    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_DMA_DPCM_MODE {
	union REG_ISP_CSI_BDG_DMA_DPCM_MODE     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_DMA_LD_DPCM_MODE {
	union REG_ISP_CSI_BDG_DMA_LD_DPCM_MODE  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH0_SIZE {
	union REG_ISP_CSI_BDG_CH0_SIZE          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH1_SIZE {
	union REG_ISP_CSI_BDG_CH1_SIZE          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH2_SIZE {
	union REG_ISP_CSI_BDG_CH2_SIZE          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH3_SIZE {
	union REG_ISP_CSI_BDG_CH3_SIZE          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH0_CROP_EN {
	union REG_ISP_CSI_BDG_CH0_CROP_EN       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH0_HORZ_CROP {
	union REG_ISP_CSI_BDG_CH0_HORZ_CROP     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH0_VERT_CROP {
	union REG_ISP_CSI_BDG_CH0_VERT_CROP     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH0_BLC_SUM {
	union REG_ISP_CSI_BDG_CH0_BLC_SUM       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH1_CROP_EN {
	union REG_ISP_CSI_BDG_CH1_CROP_EN       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH1_HORZ_CROP {
	union REG_ISP_CSI_BDG_CH1_HORZ_CROP     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH1_VERT_CROP {
	union REG_ISP_CSI_BDG_CH1_VERT_CROP     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH1_BLC_SUM {
	union REG_ISP_CSI_BDG_CH1_BLC_SUM       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH2_CROP_EN {
	union REG_ISP_CSI_BDG_CH2_CROP_EN       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH2_HORZ_CROP {
	union REG_ISP_CSI_BDG_CH2_HORZ_CROP     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH2_VERT_CROP {
	union REG_ISP_CSI_BDG_CH2_VERT_CROP     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH2_BLC_SUM {
	union REG_ISP_CSI_BDG_CH2_BLC_SUM       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH3_CROP_EN {
	union REG_ISP_CSI_BDG_CH3_CROP_EN       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH3_HORZ_CROP {
	union REG_ISP_CSI_BDG_CH3_HORZ_CROP     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH3_VERT_CROP {
	union REG_ISP_CSI_BDG_CH3_VERT_CROP     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH3_BLC_SUM {
	union REG_ISP_CSI_BDG_CH3_BLC_SUM       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_PAT_GEN_CTRL {
	union REG_ISP_CSI_BDG_PAT_GEN_CTRL      write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_PAT_IDX_CTRL {
	union REG_ISP_CSI_BDG_PAT_IDX_CTRL      write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_PAT_COLOR_0 {
	union REG_ISP_CSI_BDG_PAT_COLOR_0       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_PAT_COLOR_1 {
	union REG_ISP_CSI_BDG_PAT_COLOR_1       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_BACKGROUND_COLOR_0 {
	union REG_ISP_CSI_BDG_BACKGROUND_COLOR_0  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_BACKGROUND_COLOR_1 {
	union REG_ISP_CSI_BDG_BACKGROUND_COLOR_1  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_FIX_COLOR_0 {
	union REG_ISP_CSI_BDG_FIX_COLOR_0       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_FIX_COLOR_1 {
	union REG_ISP_CSI_BDG_FIX_COLOR_1       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_MDE_V_SIZE {
	union REG_ISP_CSI_BDG_MDE_V_SIZE        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_MDE_H_SIZE {
	union REG_ISP_CSI_BDG_MDE_H_SIZE        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_FDE_V_SIZE {
	union REG_ISP_CSI_BDG_FDE_V_SIZE        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_FDE_H_SIZE {
	union REG_ISP_CSI_BDG_FDE_H_SIZE        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_HSYNC_CTRL {
	union REG_ISP_CSI_BDG_HSYNC_CTRL        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_VSYNC_CTRL {
	union REG_ISP_CSI_BDG_VSYNC_CTRL        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_TGEN_TT_SIZE {
	union REG_ISP_CSI_BDG_TGEN_TT_SIZE      write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_LINE_INTP_HEIGHT_0 {
	union REG_ISP_CSI_BDG_LINE_INTP_HEIGHT_0  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH0_DEBUG_0 {
	union REG_ISP_CSI_BDG_CH0_DEBUG_0       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH0_DEBUG_1 {
	union REG_ISP_CSI_BDG_CH0_DEBUG_1       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH0_DEBUG_2 {
	union REG_ISP_CSI_BDG_CH0_DEBUG_2       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH0_DEBUG_3 {
	union REG_ISP_CSI_BDG_CH0_DEBUG_3       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH1_DEBUG_0 {
	union REG_ISP_CSI_BDG_CH1_DEBUG_0       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH1_DEBUG_1 {
	union REG_ISP_CSI_BDG_CH1_DEBUG_1       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH1_DEBUG_2 {
	union REG_ISP_CSI_BDG_CH1_DEBUG_2       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH1_DEBUG_3 {
	union REG_ISP_CSI_BDG_CH1_DEBUG_3       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH2_DEBUG_0 {
	union REG_ISP_CSI_BDG_CH2_DEBUG_0       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH2_DEBUG_1 {
	union REG_ISP_CSI_BDG_CH2_DEBUG_1       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH2_DEBUG_2 {
	union REG_ISP_CSI_BDG_CH2_DEBUG_2       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH2_DEBUG_3 {
	union REG_ISP_CSI_BDG_CH2_DEBUG_3       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH3_DEBUG_0 {
	union REG_ISP_CSI_BDG_CH3_DEBUG_0       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH3_DEBUG_1 {
	union REG_ISP_CSI_BDG_CH3_DEBUG_1       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH3_DEBUG_2 {
	union REG_ISP_CSI_BDG_CH3_DEBUG_2       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH3_DEBUG_3 {
	union REG_ISP_CSI_BDG_CH3_DEBUG_3       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_INTERRUPT_STATUS_0 {
	union REG_ISP_CSI_BDG_INTERRUPT_STATUS_0  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_INTERRUPT_STATUS_1 {
	union REG_ISP_CSI_BDG_INTERRUPT_STATUS_1  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_DEBUG {
	union REG_ISP_CSI_BDG_DEBUG             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_OUT_VSYNC_LINE_DELAY {
	union REG_ISP_CSI_BDG_OUT_VSYNC_LINE_DELAY  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_WR_URGENT_CTRL {
	union REG_ISP_CSI_BDG_WR_URGENT_CTRL    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_RD_URGENT_CTRL {
	union REG_ISP_CSI_BDG_RD_URGENT_CTRL    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_DUMMY {
	union REG_ISP_CSI_BDG_DUMMY             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_LINE_INTP_HEIGHT_1 {
	union REG_ISP_CSI_BDG_LINE_INTP_HEIGHT_1  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_SLICE_LINE_INTP_HEIGHT_0 {
	union REG_ISP_CSI_BDG_SLICE_LINE_INTP_HEIGHT_0  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_SLICE_LINE_INTP_HEIGHT_1 {
	union REG_ISP_CSI_BDG_SLICE_LINE_INTP_HEIGHT_1  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_BAYER_TYPE_CLK_GATE_YUV_SWAP {
	union REG_ISP_CSI_BDG_BAYER_TYPE_CLK_GATE_YUV_SWAP  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_WDMA_CH0_CROP_EN {
	union REG_ISP_CSI_BDG_WDMA_CH0_CROP_EN  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_WDMA_CH0_HORZ_CROP {
	union REG_ISP_CSI_BDG_WDMA_CH0_HORZ_CROP  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_WDMA_CH0_VERT_CROP {
	union REG_ISP_CSI_BDG_WDMA_CH0_VERT_CROP  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_WDMA_CH1_CROP_EN {
	union REG_ISP_CSI_BDG_WDMA_CH1_CROP_EN  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_WDMA_CH1_HORZ_CROP {
	union REG_ISP_CSI_BDG_WDMA_CH1_HORZ_CROP  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_WDMA_CH1_VERT_CROP {
	union REG_ISP_CSI_BDG_WDMA_CH1_VERT_CROP  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_WDMA_CH2_CROP_EN {
	union REG_ISP_CSI_BDG_WDMA_CH2_CROP_EN  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_WDMA_CH2_HORZ_CROP {
	union REG_ISP_CSI_BDG_WDMA_CH2_HORZ_CROP  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_WDMA_CH2_VERT_CROP {
	union REG_ISP_CSI_BDG_WDMA_CH2_VERT_CROP  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_WDMA_CH3_CROP_EN {
	union REG_ISP_CSI_BDG_WDMA_CH3_CROP_EN  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_WDMA_CH3_HORZ_CROP {
	union REG_ISP_CSI_BDG_WDMA_CH3_HORZ_CROP  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_WDMA_CH3_VERT_CROP {
	union REG_ISP_CSI_BDG_WDMA_CH3_VERT_CROP  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_TRIG_DLY_CONTROL_0 {
	union REG_ISP_CSI_BDG_TRIG_DLY_CONTROL_0  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_TRIG_DLY_CONTROL_1 {
	union REG_ISP_CSI_BDG_TRIG_DLY_CONTROL_1  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_BAYER_TYPE {
	union REG_ISP_CSI_BDG_BAYER_TYPE        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH0_AI_ISP_CONTROL {
	union REG_ISP_CSI_BDG_CH0_AI_ISP_CONTROL  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH0_AI_ISP_TRANSFORM {
	union REG_ISP_CSI_BDG_CH0_AI_ISP_TRANSFORM  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_AI_ISP_DEBUG_STATUS_CH0 {
	union REG_ISP_CSI_BDG_AI_ISP_DEBUG_STATUS_CH0  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH1_AI_ISP_CONTROL {
	union REG_ISP_CSI_BDG_CH1_AI_ISP_CONTROL  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH1_AI_ISP_TRANSFORM {
	union REG_ISP_CSI_BDG_CH1_AI_ISP_TRANSFORM  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_AI_ISP_DEBUG_STATUS_CH1 {
	union REG_ISP_CSI_BDG_AI_ISP_DEBUG_STATUS_CH1  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH2_AI_ISP_CONTROL {
	union REG_ISP_CSI_BDG_CH2_AI_ISP_CONTROL  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH2_AI_ISP_TRANSFORM {
	union REG_ISP_CSI_BDG_CH2_AI_ISP_TRANSFORM  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_AI_ISP_DEBUG_STATUS_CH2 {
	union REG_ISP_CSI_BDG_AI_ISP_DEBUG_STATUS_CH2  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH3_AI_ISP_CONTROL {
	union REG_ISP_CSI_BDG_CH3_AI_ISP_CONTROL  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH3_AI_ISP_TRANSFORM {
	union REG_ISP_CSI_BDG_CH3_AI_ISP_TRANSFORM  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_AI_ISP_DEBUG_STATUS_CH3 {
	union REG_ISP_CSI_BDG_AI_ISP_DEBUG_STATUS_CH3  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_CH_DMA_420_CFG {
	union REG_ISP_CSI_BDG_CH_DMA_420_CFG    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSI_BDG_T {
	struct VREG_ISP_CSI_BDG_TOP_CTRL         CSI_BDG_TOP_CTRL;
	struct VREG_ISP_CSI_BDG_INTERRUPT_CTRL   CSI_BDG_INTERRUPT_CTRL;
	struct VREG_ISP_CSI_BDG_DMA_DPCM_MODE    CSI_BDG_DMA_DPCM_MODE;
	struct VREG_ISP_CSI_BDG_DMA_LD_DPCM_MODE  CSI_BDG_DMA_LD_DPCM_MODE;
	struct VREG_ISP_CSI_BDG_CH0_SIZE         CH0_SIZE;
	struct VREG_ISP_CSI_BDG_CH1_SIZE         CH1_SIZE;
	struct VREG_ISP_CSI_BDG_CH2_SIZE         CH2_SIZE;
	struct VREG_ISP_CSI_BDG_CH3_SIZE         CH3_SIZE;
	struct VREG_ISP_CSI_BDG_CH0_CROP_EN      CH0_CROP_EN;
	struct VREG_ISP_CSI_BDG_CH0_HORZ_CROP    CH0_HORZ_CROP;
	struct VREG_ISP_CSI_BDG_CH0_VERT_CROP    CH0_VERT_CROP;
	struct VREG_ISP_CSI_BDG_CH0_BLC_SUM      CH0_BLC_SUM;
	struct VREG_ISP_CSI_BDG_CH1_CROP_EN      CH1_CROP_EN;
	struct VREG_ISP_CSI_BDG_CH1_HORZ_CROP    CH1_HORZ_CROP;
	struct VREG_ISP_CSI_BDG_CH1_VERT_CROP    CH1_VERT_CROP;
	struct VREG_ISP_CSI_BDG_CH1_BLC_SUM      CH1_BLC_SUM;
	struct VREG_ISP_CSI_BDG_CH2_CROP_EN      CH2_CROP_EN;
	struct VREG_ISP_CSI_BDG_CH2_HORZ_CROP    CH2_HORZ_CROP;
	struct VREG_ISP_CSI_BDG_CH2_VERT_CROP    CH2_VERT_CROP;
	struct VREG_ISP_CSI_BDG_CH2_BLC_SUM      CH2_BLC_SUM;
	struct VREG_ISP_CSI_BDG_CH3_CROP_EN      CH3_CROP_EN;
	struct VREG_ISP_CSI_BDG_CH3_HORZ_CROP    CH3_HORZ_CROP;
	struct VREG_ISP_CSI_BDG_CH3_VERT_CROP    CH3_VERT_CROP;
	struct VREG_ISP_CSI_BDG_CH3_BLC_SUM      CH3_BLC_SUM;
	struct VREG_ISP_CSI_BDG_PAT_GEN_CTRL     CSI_PAT_GEN_CTRL;
	struct VREG_ISP_CSI_BDG_PAT_IDX_CTRL     CSI_PAT_IDX_CTRL;
	struct VREG_ISP_CSI_BDG_PAT_COLOR_0      CSI_PAT_COLOR_0;
	struct VREG_ISP_CSI_BDG_PAT_COLOR_1      CSI_PAT_COLOR_1;
	struct VREG_ISP_CSI_BDG_BACKGROUND_COLOR_0  CSI_BACKGROUND_COLOR_0;
	struct VREG_ISP_CSI_BDG_BACKGROUND_COLOR_1  CSI_BACKGROUND_COLOR_1;
	struct VREG_ISP_CSI_BDG_FIX_COLOR_0      CSI_FIX_COLOR_0;
	struct VREG_ISP_CSI_BDG_FIX_COLOR_1      CSI_FIX_COLOR_1;
	struct VREG_ISP_CSI_BDG_MDE_V_SIZE       CSI_MDE_V_SIZE;
	struct VREG_ISP_CSI_BDG_MDE_H_SIZE       CSI_MDE_H_SIZE;
	struct VREG_ISP_CSI_BDG_FDE_V_SIZE       CSI_FDE_V_SIZE;
	struct VREG_ISP_CSI_BDG_FDE_H_SIZE       CSI_FDE_H_SIZE;
	struct VREG_ISP_CSI_BDG_HSYNC_CTRL       CSI_HSYNC_CTRL;
	struct VREG_ISP_CSI_BDG_VSYNC_CTRL       CSI_VSYNC_CTRL;
	struct VREG_ISP_CSI_BDG_TGEN_TT_SIZE     CSI_TGEN_TT_SIZE;
	struct VREG_ISP_CSI_BDG_LINE_INTP_HEIGHT_0  LINE_INTP_HEIGHT_0;
	struct VREG_ISP_CSI_BDG_CH0_DEBUG_0      CH0_DEBUG_0;
	struct VREG_ISP_CSI_BDG_CH0_DEBUG_1      CH0_DEBUG_1;
	struct VREG_ISP_CSI_BDG_CH0_DEBUG_2      CH0_DEBUG_2;
	struct VREG_ISP_CSI_BDG_CH0_DEBUG_3      CH0_DEBUG_3;
	struct VREG_ISP_CSI_BDG_CH1_DEBUG_0      CH1_DEBUG_0;
	struct VREG_ISP_CSI_BDG_CH1_DEBUG_1      CH1_DEBUG_1;
	struct VREG_ISP_CSI_BDG_CH1_DEBUG_2      CH1_DEBUG_2;
	struct VREG_ISP_CSI_BDG_CH1_DEBUG_3      CH1_DEBUG_3;
	struct VREG_ISP_CSI_BDG_CH2_DEBUG_0      CH2_DEBUG_0;
	struct VREG_ISP_CSI_BDG_CH2_DEBUG_1      CH2_DEBUG_1;
	struct VREG_ISP_CSI_BDG_CH2_DEBUG_2      CH2_DEBUG_2;
	struct VREG_ISP_CSI_BDG_CH2_DEBUG_3      CH2_DEBUG_3;
	struct VREG_ISP_CSI_BDG_CH3_DEBUG_0      CH3_DEBUG_0;
	struct VREG_ISP_CSI_BDG_CH3_DEBUG_1      CH3_DEBUG_1;
	struct VREG_ISP_CSI_BDG_CH3_DEBUG_2      CH3_DEBUG_2;
	struct VREG_ISP_CSI_BDG_CH3_DEBUG_3      CH3_DEBUG_3;
	struct VREG_ISP_CSI_BDG_INTERRUPT_STATUS_0  INTERRUPT_STATUS_0;
	struct VREG_ISP_CSI_BDG_INTERRUPT_STATUS_1  INTERRUPT_STATUS_1;
	struct VREG_ISP_CSI_BDG_DEBUG            BDG_DEBUG;
	struct VREG_ISP_CSI_BDG_OUT_VSYNC_LINE_DELAY  CSI_OUT_VSYNC_LINE_DELAY;
	struct VREG_ISP_CSI_BDG_WR_URGENT_CTRL   CSI_WR_URGENT_CTRL;
	struct VREG_ISP_CSI_BDG_RD_URGENT_CTRL   CSI_RD_URGENT_CTRL;
	struct VREG_ISP_CSI_BDG_DUMMY            CSI_DUMMY;
	struct VREG_ISP_CSI_BDG_LINE_INTP_HEIGHT_1  LINE_INTP_HEIGHT_1;
	struct VREG_ISP_CSI_BDG_SLICE_LINE_INTP_HEIGHT_0  SLICE_LINE_INTP_HEIGHT_0;
	struct VREG_ISP_CSI_BDG_SLICE_LINE_INTP_HEIGHT_1  SLICE_LINE_INTP_HEIGHT_1;
	struct VREG_ISP_CSI_BDG_BAYER_TYPE_CLK_GATE_YUV_SWAP  BAYER_TYPE_CLK_GATE_YUV_SWAP;
	struct VREG_RESV                        _resv_0x10c[1];
	struct VREG_ISP_CSI_BDG_WDMA_CH0_CROP_EN  WDMA_CH0_CROP_EN;
	struct VREG_ISP_CSI_BDG_WDMA_CH0_HORZ_CROP  WDMA_CH0_HORZ_CROP;
	struct VREG_ISP_CSI_BDG_WDMA_CH0_VERT_CROP  WDMA_CH0_VERT_CROP;
	struct VREG_RESV                        _resv_0x11c[1];
	struct VREG_ISP_CSI_BDG_WDMA_CH1_CROP_EN  WDMA_CH1_CROP_EN;
	struct VREG_ISP_CSI_BDG_WDMA_CH1_HORZ_CROP  WDMA_CH1_HORZ_CROP;
	struct VREG_ISP_CSI_BDG_WDMA_CH1_VERT_CROP  WDMA_CH1_VERT_CROP;
	struct VREG_RESV                        _resv_0x12c[1];
	struct VREG_ISP_CSI_BDG_WDMA_CH2_CROP_EN  WDMA_CH2_CROP_EN;
	struct VREG_ISP_CSI_BDG_WDMA_CH2_HORZ_CROP  WDMA_CH2_HORZ_CROP;
	struct VREG_ISP_CSI_BDG_WDMA_CH2_VERT_CROP  WDMA_CH2_VERT_CROP;
	struct VREG_RESV                        _resv_0x13c[1];
	struct VREG_ISP_CSI_BDG_WDMA_CH3_CROP_EN  WDMA_CH3_CROP_EN;
	struct VREG_ISP_CSI_BDG_WDMA_CH3_HORZ_CROP  WDMA_CH3_HORZ_CROP;
	struct VREG_ISP_CSI_BDG_WDMA_CH3_VERT_CROP  WDMA_CH3_VERT_CROP;
	struct VREG_RESV                        _resv_0x14c[1];
	struct VREG_ISP_CSI_BDG_TRIG_DLY_CONTROL_0  TRIG_DLY_CONTROL_0;
	struct VREG_ISP_CSI_BDG_TRIG_DLY_CONTROL_1  TRIG_DLY_CONTROL_1;
	struct VREG_ISP_CSI_BDG_BAYER_TYPE       BAYER_TYPE;
	struct VREG_RESV                        _resv_0x15c[1];
	struct VREG_ISP_CSI_BDG_CH0_AI_ISP_CONTROL  CH0_AI_ISP_CONTROL;
	struct VREG_ISP_CSI_BDG_CH0_AI_ISP_TRANSFORM  CH0_AI_ISP_TRANSFORM;
	struct VREG_ISP_CSI_BDG_AI_ISP_DEBUG_STATUS_CH0  AI_ISP_DEBUG_STATUS_CH0;
	struct VREG_RESV                        _resv_0x16c[1];
	struct VREG_ISP_CSI_BDG_CH1_AI_ISP_CONTROL  CH1_AI_ISP_CONTROL;
	struct VREG_ISP_CSI_BDG_CH1_AI_ISP_TRANSFORM  CH1_AI_ISP_TRANSFORM;
	struct VREG_ISP_CSI_BDG_AI_ISP_DEBUG_STATUS_CH1  AI_ISP_DEBUG_STATUS_CH1;
	struct VREG_RESV                        _resv_0x17c[1];
	struct VREG_ISP_CSI_BDG_CH2_AI_ISP_CONTROL  CH2_AI_ISP_CONTROL;
	struct VREG_ISP_CSI_BDG_CH2_AI_ISP_TRANSFORM  CH2_AI_ISP_TRANSFORM;
	struct VREG_ISP_CSI_BDG_AI_ISP_DEBUG_STATUS_CH2  AI_ISP_DEBUG_STATUS_CH2;
	struct VREG_RESV                        _resv_0x18c[1];
	struct VREG_ISP_CSI_BDG_CH3_AI_ISP_CONTROL  CH3_AI_ISP_CONTROL;
	struct VREG_ISP_CSI_BDG_CH3_AI_ISP_TRANSFORM  CH3_AI_ISP_TRANSFORM;
	struct VREG_ISP_CSI_BDG_AI_ISP_DEBUG_STATUS_CH3  AI_ISP_DEBUG_STATUS_CH3;
	struct VREG_ISP_CSI_BDG_CH_DMA_420_CFG   CH_DMA_420_CFG;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_DMA_CTL_SYS_CONTROL {
	union REG_ISP_DMA_CTL_SYS_CONTROL       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DMA_CTL_BASE_ADDR {
	union REG_ISP_DMA_CTL_BASE_ADDR         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DMA_CTL_DMA_SEGLEN {
	union REG_ISP_DMA_CTL_DMA_SEGLEN        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DMA_CTL_DMA_STRIDE {
	union REG_ISP_DMA_CTL_DMA_STRIDE        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DMA_CTL_DMA_SEGNUM {
	union REG_ISP_DMA_CTL_DMA_SEGNUM        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DMA_CTL_DMA_STATUS {
	union REG_ISP_DMA_CTL_DMA_STATUS        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DMA_CTL_DMA_SLICESIZE {
	union REG_ISP_DMA_CTL_DMA_SLICESIZE     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DMA_CTL_DMA_DUMMY {
	union REG_ISP_DMA_CTL_DMA_DUMMY         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DMA_CTL_T {
	struct VREG_ISP_DMA_CTL_SYS_CONTROL      SYS_CONTROL;
	struct VREG_ISP_DMA_CTL_BASE_ADDR        BASE_ADDR;
	struct VREG_ISP_DMA_CTL_DMA_SEGLEN       DMA_SEGLEN;
	struct VREG_ISP_DMA_CTL_DMA_STRIDE       DMA_STRIDE;
	struct VREG_ISP_DMA_CTL_DMA_SEGNUM       DMA_SEGNUM;
	struct VREG_ISP_DMA_CTL_DMA_STATUS       DMA_STATUS;
	struct VREG_ISP_DMA_CTL_DMA_SLICESIZE    DMA_SLICESIZE;
	struct VREG_ISP_DMA_CTL_DMA_DUMMY        DMA_DUMMY;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_LSC_SC_WRAP_0 {
	union REG_ISP_LSC_SC_WRAP_0             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSC_SC_WRAP_1 {
	union REG_ISP_LSC_SC_WRAP_1             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSC_SC_WRAP_2 {
	union REG_ISP_LSC_SC_WRAP_2             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSC_SC_WRAP_3 {
	union REG_ISP_LSC_SC_WRAP_3             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSC_SC_WRAP_4 {
	union REG_ISP_LSC_SC_WRAP_4             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSC_SC_WRAP_5 {
	union REG_ISP_LSC_SC_WRAP_5             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSC_SC_WRAP_6 {
	union REG_ISP_LSC_SC_WRAP_6             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSC_SC_WRAP_7 {
	union REG_ISP_LSC_SC_WRAP_7             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSC_SC_WRAP_8 {
	union REG_ISP_LSC_SC_WRAP_8             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSC_SC_WRAP_9 {
	union REG_ISP_LSC_SC_WRAP_9             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSC_SC_WRAP_10 {
	union REG_ISP_LSC_SC_WRAP_10            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSC_SC_WRAP_11 {
	union REG_ISP_LSC_SC_WRAP_11            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSC_SC_WRAP_12 {
	union REG_ISP_LSC_SC_WRAP_12            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSC_SC_WRAP_13 {
	union REG_ISP_LSC_SC_WRAP_13            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSC_SC_WRAP_14 {
	union REG_ISP_LSC_SC_WRAP_14            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSC_T {
	struct VREG_ISP_LSC_SC_WRAP_0            SC_WRAP_0;
	struct VREG_ISP_LSC_SC_WRAP_1            SC_WRAP_1;
	struct VREG_ISP_LSC_SC_WRAP_2            SC_WRAP_2;
	struct VREG_ISP_LSC_SC_WRAP_3            SC_WRAP_3;
	struct VREG_ISP_LSC_SC_WRAP_4            SC_WRAP_4;
	struct VREG_ISP_LSC_SC_WRAP_5            SC_WRAP_5;
	struct VREG_ISP_LSC_SC_WRAP_6            SC_WRAP_6;
	struct VREG_ISP_LSC_SC_WRAP_7            SC_WRAP_7;
	struct VREG_ISP_LSC_SC_WRAP_8            SC_WRAP_8;
	struct VREG_ISP_LSC_SC_WRAP_9            SC_WRAP_9;
	struct VREG_ISP_LSC_SC_WRAP_10           SC_WRAP_10;
	struct VREG_ISP_LSC_SC_WRAP_11           SC_WRAP_11;
	struct VREG_ISP_LSC_SC_WRAP_12           SC_WRAP_12;
	struct VREG_ISP_LSC_SC_WRAP_13           SC_WRAP_13;
	struct VREG_ISP_LSC_SC_WRAP_14           SC_WRAP_14;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_AE_HIST_AE_HIST_STATUS {
	union REG_ISP_AE_HIST_AE_HIST_STATUS    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_HIST_GRACE_RESET {
	union REG_ISP_AE_HIST_AE_HIST_GRACE_RESET  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_HIST_MONITOR {
	union REG_ISP_AE_HIST_AE_HIST_MONITOR   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_HIST_BYPASS {
	union REG_ISP_AE_HIST_AE_HIST_BYPASS    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_KICKOFF {
	union REG_ISP_AE_HIST_AE_KICKOFF        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_STS_AE0_HIST_ENABLE {
	union REG_ISP_AE_HIST_STS_AE0_HIST_ENABLE  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_STS_AE_OFFSETX {
	union REG_ISP_AE_HIST_STS_AE_OFFSETX    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_STS_AE_OFFSETY {
	union REG_ISP_AE_HIST_STS_AE_OFFSETY    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_STS_AE_NUMXM1 {
	union REG_ISP_AE_HIST_STS_AE_NUMXM1     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_STS_AE_NUMYM1 {
	union REG_ISP_AE_HIST_STS_AE_NUMYM1     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_STS_AE_WIDTH {
	union REG_ISP_AE_HIST_STS_AE_WIDTH      write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_STS_AE_HEIGHT {
	union REG_ISP_AE_HIST_STS_AE_HEIGHT     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_STS_AE_STS_DIV {
	union REG_ISP_AE_HIST_STS_AE_STS_DIV    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_STS_HIST_MODE {
	union REG_ISP_AE_HIST_STS_HIST_MODE     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_SHDW_READ_SEL {
	union REG_ISP_AE_HIST_SHDW_READ_SEL     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_HIST_MONITOR_SELECT {
	union REG_ISP_AE_HIST_AE_HIST_MONITOR_SELECT  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_HIST_LOCATION {
	union REG_ISP_AE_HIST_AE_HIST_LOCATION  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_HW_AUTO_CG_EN {
	union REG_ISP_AE_HIST_HW_AUTO_CG_EN     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_STS_IR_AE_OFFSETX {
	union REG_ISP_AE_HIST_STS_IR_AE_OFFSETX  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_STS_IR_AE_OFFSETY {
	union REG_ISP_AE_HIST_STS_IR_AE_OFFSETY  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_STS_IR_AE_NUMXM1 {
	union REG_ISP_AE_HIST_STS_IR_AE_NUMXM1  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_STS_IR_AE_NUMYM1 {
	union REG_ISP_AE_HIST_STS_IR_AE_NUMYM1  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_STS_IR_AE_WIDTH {
	union REG_ISP_AE_HIST_STS_IR_AE_WIDTH   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_STS_IR_AE_HEIGHT {
	union REG_ISP_AE_HIST_STS_IR_AE_HEIGHT  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_STS_IR_AE_STS_DIV {
	union REG_ISP_AE_HIST_STS_IR_AE_STS_DIV  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_HIST_BAYER_STARTING {
	union REG_ISP_AE_HIST_AE_HIST_BAYER_STARTING  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_HIST_DUMMY {
	union REG_ISP_AE_HIST_AE_HIST_DUMMY     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_HIST_CHECKSUM {
	union REG_ISP_AE_HIST_AE_HIST_CHECKSUM  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_WBG_4 {
	union REG_ISP_AE_HIST_WBG_4             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_WBG_5 {
	union REG_ISP_AE_HIST_WBG_5             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_WBG_6 {
	union REG_ISP_AE_HIST_WBG_6             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_WBG_7 {
	union REG_ISP_AE_HIST_WBG_7             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_DMI_ENABLE {
	union REG_ISP_AE_HIST_DMI_ENABLE        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_FACE0_LOCATION {
	union REG_ISP_AE_HIST_AE_FACE0_LOCATION  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_FACE1_LOCATION {
	union REG_ISP_AE_HIST_AE_FACE1_LOCATION  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_FACE2_LOCATION {
	union REG_ISP_AE_HIST_AE_FACE2_LOCATION  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_FACE3_LOCATION {
	union REG_ISP_AE_HIST_AE_FACE3_LOCATION  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_FACE0_SIZE {
	union REG_ISP_AE_HIST_AE_FACE0_SIZE     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_FACE1_SIZE {
	union REG_ISP_AE_HIST_AE_FACE1_SIZE     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_FACE2_SIZE {
	union REG_ISP_AE_HIST_AE_FACE2_SIZE     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_FACE3_SIZE {
	union REG_ISP_AE_HIST_AE_FACE3_SIZE     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_IR_AE_FACE0_LOCATION {
	union REG_ISP_AE_HIST_IR_AE_FACE0_LOCATION  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_IR_AE_FACE1_LOCATION {
	union REG_ISP_AE_HIST_IR_AE_FACE1_LOCATION  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_IR_AE_FACE2_LOCATION {
	union REG_ISP_AE_HIST_IR_AE_FACE2_LOCATION  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_IR_AE_FACE3_LOCATION {
	union REG_ISP_AE_HIST_IR_AE_FACE3_LOCATION  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_IR_AE_FACE0_SIZE {
	union REG_ISP_AE_HIST_IR_AE_FACE0_SIZE  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_IR_AE_FACE1_SIZE {
	union REG_ISP_AE_HIST_IR_AE_FACE1_SIZE  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_IR_AE_FACE2_SIZE {
	union REG_ISP_AE_HIST_IR_AE_FACE2_SIZE  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_IR_AE_FACE3_SIZE {
	union REG_ISP_AE_HIST_IR_AE_FACE3_SIZE  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_FACE_ENABLE_CTRL {
	union REG_ISP_AE_HIST_AE_FACE_ENABLE_CTRL  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_FACE0_STS_DIV {
	union REG_ISP_AE_HIST_AE_FACE0_STS_DIV  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_FACE1_STS_DIV {
	union REG_ISP_AE_HIST_AE_FACE1_STS_DIV  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_FACE2_STS_DIV {
	union REG_ISP_AE_HIST_AE_FACE2_STS_DIV  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_FACE3_STS_DIV {
	union REG_ISP_AE_HIST_AE_FACE3_STS_DIV  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_STS_ENABLE {
	union REG_ISP_AE_HIST_STS_ENABLE        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_ALGO_ENABLE {
	union REG_ISP_AE_HIST_AE_ALGO_ENABLE    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_HIST_LOW {
	union REG_ISP_AE_HIST_AE_HIST_LOW       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_HIST_HIGH {
	union REG_ISP_AE_HIST_AE_HIST_HIGH      write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_TOP {
	union REG_ISP_AE_HIST_AE_TOP            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_BOT {
	union REG_ISP_AE_HIST_AE_BOT            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_OVEREXP_THR {
	union REG_ISP_AE_HIST_AE_OVEREXP_THR    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_NUM_GAPLINE {
	union REG_ISP_AE_HIST_AE_NUM_GAPLINE    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_CHOOSE_GR_EN {
	union REG_ISP_AE_HIST_AE_CHOOSE_GR_EN   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_FRAME_COUNTER_AWB {
	union REG_ISP_AE_HIST_FRAME_COUNTER_AWB  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_SIMPLE2A_RESULT_LUMA_SE {
	union REG_ISP_AE_HIST_AE_SIMPLE2A_RESULT_LUMA_SE  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_SIMPLE2A_RESULT_RGAIN_SE {
	union REG_ISP_AE_HIST_AE_SIMPLE2A_RESULT_RGAIN_SE  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_SIMPLE2A_RESULT_BGAIN_SE {
	union REG_ISP_AE_HIST_AE_SIMPLE2A_RESULT_BGAIN_SE  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_SIMPLE2A_RESULT_LUMA_LE {
	union REG_ISP_AE_HIST_AE_SIMPLE2A_RESULT_LUMA_LE  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_SIMPLE2A_RESULT_RGAIN_LE {
	union REG_ISP_AE_HIST_AE_SIMPLE2A_RESULT_RGAIN_LE  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_SIMPLE2A_RESULT_BGAIN_LE {
	union REG_ISP_AE_HIST_AE_SIMPLE2A_RESULT_BGAIN_LE  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_FACE0_RESULT_SE {
	union REG_ISP_AE_HIST_AE_FACE0_RESULT_SE  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_FACE1_RESULT_SE {
	union REG_ISP_AE_HIST_AE_FACE1_RESULT_SE  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_FACE2_RESULT_SE {
	union REG_ISP_AE_HIST_AE_FACE2_RESULT_SE  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_FACE3_RESULT_SE {
	union REG_ISP_AE_HIST_AE_FACE3_RESULT_SE  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_FACE0_RESULT_LE {
	union REG_ISP_AE_HIST_AE_FACE0_RESULT_LE  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_FACE1_RESULT_LE {
	union REG_ISP_AE_HIST_AE_FACE1_RESULT_LE  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_FACE2_RESULT_LE {
	union REG_ISP_AE_HIST_AE_FACE2_RESULT_LE  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_FACE3_RESULT_LE {
	union REG_ISP_AE_HIST_AE_FACE3_RESULT_LE  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_00 {
	union REG_ISP_AE_HIST_AE_WGT_00         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_01 {
	union REG_ISP_AE_HIST_AE_WGT_01         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_02 {
	union REG_ISP_AE_HIST_AE_WGT_02         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_03 {
	union REG_ISP_AE_HIST_AE_WGT_03         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_04 {
	union REG_ISP_AE_HIST_AE_WGT_04         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_05 {
	union REG_ISP_AE_HIST_AE_WGT_05         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_06 {
	union REG_ISP_AE_HIST_AE_WGT_06         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_07 {
	union REG_ISP_AE_HIST_AE_WGT_07         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_08 {
	union REG_ISP_AE_HIST_AE_WGT_08         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_09 {
	union REG_ISP_AE_HIST_AE_WGT_09         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_10 {
	union REG_ISP_AE_HIST_AE_WGT_10         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_11 {
	union REG_ISP_AE_HIST_AE_WGT_11         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_12 {
	union REG_ISP_AE_HIST_AE_WGT_12         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_13 {
	union REG_ISP_AE_HIST_AE_WGT_13         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_14 {
	union REG_ISP_AE_HIST_AE_WGT_14         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_15 {
	union REG_ISP_AE_HIST_AE_WGT_15         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_16 {
	union REG_ISP_AE_HIST_AE_WGT_16         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_17 {
	union REG_ISP_AE_HIST_AE_WGT_17         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_18 {
	union REG_ISP_AE_HIST_AE_WGT_18         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_19 {
	union REG_ISP_AE_HIST_AE_WGT_19         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_20 {
	union REG_ISP_AE_HIST_AE_WGT_20         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_21 {
	union REG_ISP_AE_HIST_AE_WGT_21         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_22 {
	union REG_ISP_AE_HIST_AE_WGT_22         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_23 {
	union REG_ISP_AE_HIST_AE_WGT_23         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_24 {
	union REG_ISP_AE_HIST_AE_WGT_24         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_25 {
	union REG_ISP_AE_HIST_AE_WGT_25         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_26 {
	union REG_ISP_AE_HIST_AE_WGT_26         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_27 {
	union REG_ISP_AE_HIST_AE_WGT_27         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_28 {
	union REG_ISP_AE_HIST_AE_WGT_28         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_29 {
	union REG_ISP_AE_HIST_AE_WGT_29         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_30 {
	union REG_ISP_AE_HIST_AE_WGT_30         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_AE_WGT_31 {
	union REG_ISP_AE_HIST_AE_WGT_31         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_LE_AE_BLC_OFFSET_R {
	union REG_ISP_AE_HIST_LE_AE_BLC_OFFSET_R  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_LE_AE_BLC_OFFSET_GR {
	union REG_ISP_AE_HIST_LE_AE_BLC_OFFSET_GR  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_LE_AE_BLC_OFFSET_GB {
	union REG_ISP_AE_HIST_LE_AE_BLC_OFFSET_GB  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_LE_AE_BLC_OFFSET_B {
	union REG_ISP_AE_HIST_LE_AE_BLC_OFFSET_B  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_LE_AE_BLC_OFFSET_IR {
	union REG_ISP_AE_HIST_LE_AE_BLC_OFFSET_IR  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_LE_AE_BLC_GAIN_R {
	union REG_ISP_AE_HIST_LE_AE_BLC_GAIN_R  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_LE_AE_BLC_GAIN_GR {
	union REG_ISP_AE_HIST_LE_AE_BLC_GAIN_GR  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_LE_AE_BLC_GAIN_GB {
	union REG_ISP_AE_HIST_LE_AE_BLC_GAIN_GB  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_LE_AE_BLC_GAIN_B {
	union REG_ISP_AE_HIST_LE_AE_BLC_GAIN_B  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_LE_AE_BLC_GAIN_IR {
	union REG_ISP_AE_HIST_LE_AE_BLC_GAIN_IR  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_LE_AE_BLC_ENABLE {
	union REG_ISP_AE_HIST_LE_AE_BLC_ENABLE  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_SE_AE_BLC_OFFSET_R {
	union REG_ISP_AE_HIST_SE_AE_BLC_OFFSET_R  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_SE_AE_BLC_OFFSET_GR {
	union REG_ISP_AE_HIST_SE_AE_BLC_OFFSET_GR  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_SE_AE_BLC_OFFSET_GB {
	union REG_ISP_AE_HIST_SE_AE_BLC_OFFSET_GB  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_SE_AE_BLC_OFFSET_B {
	union REG_ISP_AE_HIST_SE_AE_BLC_OFFSET_B  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_SE_AE_BLC_OFFSET_IR {
	union REG_ISP_AE_HIST_SE_AE_BLC_OFFSET_IR  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_SE_AE_BLC_GAIN_R {
	union REG_ISP_AE_HIST_SE_AE_BLC_GAIN_R  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_SE_AE_BLC_GAIN_GR {
	union REG_ISP_AE_HIST_SE_AE_BLC_GAIN_GR  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_SE_AE_BLC_GAIN_GB {
	union REG_ISP_AE_HIST_SE_AE_BLC_GAIN_GB  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_SE_AE_BLC_GAIN_B {
	union REG_ISP_AE_HIST_SE_AE_BLC_GAIN_B  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_SE_AE_BLC_GAIN_IR {
	union REG_ISP_AE_HIST_SE_AE_BLC_GAIN_IR  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_SE_AE_BLC_ENABLE {
	union REG_ISP_AE_HIST_SE_AE_BLC_ENABLE  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AE_HIST_T {
	struct VREG_ISP_AE_HIST_AE_HIST_STATUS   AE_HIST_STATUS;
	struct VREG_ISP_AE_HIST_AE_HIST_GRACE_RESET  AE_HIST_GRACE_RESET;
	struct VREG_ISP_AE_HIST_AE_HIST_MONITOR  AE_HIST_MONITOR;
	struct VREG_ISP_AE_HIST_AE_HIST_BYPASS   AE_HIST_BYPASS;
	struct VREG_ISP_AE_HIST_AE_KICKOFF       AE_KICKOFF;
	struct VREG_ISP_AE_HIST_STS_AE0_HIST_ENABLE  STS_AE0_HIST_ENABLE;
	struct VREG_ISP_AE_HIST_STS_AE_OFFSETX   STS_AE_OFFSETX;
	struct VREG_ISP_AE_HIST_STS_AE_OFFSETY   STS_AE_OFFSETY;
	struct VREG_ISP_AE_HIST_STS_AE_NUMXM1    STS_AE_NUMXM1;
	struct VREG_ISP_AE_HIST_STS_AE_NUMYM1    STS_AE_NUMYM1;
	struct VREG_ISP_AE_HIST_STS_AE_WIDTH     STS_AE_WIDTH;
	struct VREG_ISP_AE_HIST_STS_AE_HEIGHT    STS_AE_HEIGHT;
	struct VREG_ISP_AE_HIST_STS_AE_STS_DIV   STS_AE_STS_DIV;
	struct VREG_ISP_AE_HIST_STS_HIST_MODE    STS_HIST_MODE;
	struct VREG_ISP_AE_HIST_SHDW_READ_SEL    SHDW_READ_SEL;
	struct VREG_ISP_AE_HIST_AE_HIST_MONITOR_SELECT  AE_HIST_MONITOR_SELECT;
	struct VREG_ISP_AE_HIST_AE_HIST_LOCATION  AE_HIST_LOCATION;
	struct VREG_ISP_AE_HIST_HW_AUTO_CG_EN    HW_AUTO_CG_EN;
	struct VREG_ISP_AE_HIST_STS_IR_AE_OFFSETX  STS_IR_AE_OFFSETX;
	struct VREG_ISP_AE_HIST_STS_IR_AE_OFFSETY  STS_IR_AE_OFFSETY;
	struct VREG_ISP_AE_HIST_STS_IR_AE_NUMXM1  STS_IR_AE_NUMXM1;
	struct VREG_ISP_AE_HIST_STS_IR_AE_NUMYM1  STS_IR_AE_NUMYM1;
	struct VREG_ISP_AE_HIST_STS_IR_AE_WIDTH  STS_IR_AE_WIDTH;
	struct VREG_ISP_AE_HIST_STS_IR_AE_HEIGHT  STS_IR_AE_HEIGHT;
	struct VREG_ISP_AE_HIST_STS_IR_AE_STS_DIV  STS_IR_AE_STS_DIV;
	struct VREG_RESV                        _resv_0x64[1];
	struct VREG_ISP_AE_HIST_AE_HIST_BAYER_STARTING  AE_HIST_BAYER_STARTING;
	struct VREG_ISP_AE_HIST_AE_HIST_DUMMY    AE_HIST_DUMMY;
	struct VREG_ISP_AE_HIST_AE_HIST_CHECKSUM  AE_HIST_CHECKSUM;
	struct VREG_ISP_AE_HIST_WBG_4            WBG_4;
	struct VREG_ISP_AE_HIST_WBG_5            WBG_5;
	struct VREG_ISP_AE_HIST_WBG_6            WBG_6;
	struct VREG_ISP_AE_HIST_WBG_7            WBG_7;
	struct VREG_RESV                        _resv_0x84[7];
	struct VREG_ISP_AE_HIST_DMI_ENABLE       DMI_ENABLE;
	struct VREG_RESV                        _resv_0xa4[3];
	struct VREG_ISP_AE_HIST_AE_FACE0_LOCATION  AE_FACE0_LOCATION;
	struct VREG_ISP_AE_HIST_AE_FACE1_LOCATION  AE_FACE1_LOCATION;
	struct VREG_ISP_AE_HIST_AE_FACE2_LOCATION  AE_FACE2_LOCATION;
	struct VREG_ISP_AE_HIST_AE_FACE3_LOCATION  AE_FACE3_LOCATION;
	struct VREG_ISP_AE_HIST_AE_FACE0_SIZE    AE_FACE0_SIZE;
	struct VREG_ISP_AE_HIST_AE_FACE1_SIZE    AE_FACE1_SIZE;
	struct VREG_ISP_AE_HIST_AE_FACE2_SIZE    AE_FACE2_SIZE;
	struct VREG_ISP_AE_HIST_AE_FACE3_SIZE    AE_FACE3_SIZE;
	struct VREG_ISP_AE_HIST_IR_AE_FACE0_LOCATION  IR_AE_FACE0_LOCATION;
	struct VREG_ISP_AE_HIST_IR_AE_FACE1_LOCATION  IR_AE_FACE1_LOCATION;
	struct VREG_ISP_AE_HIST_IR_AE_FACE2_LOCATION  IR_AE_FACE2_LOCATION;
	struct VREG_ISP_AE_HIST_IR_AE_FACE3_LOCATION  IR_AE_FACE3_LOCATION;
	struct VREG_ISP_AE_HIST_IR_AE_FACE0_SIZE  IR_AE_FACE0_SIZE;
	struct VREG_ISP_AE_HIST_IR_AE_FACE1_SIZE  IR_AE_FACE1_SIZE;
	struct VREG_ISP_AE_HIST_IR_AE_FACE2_SIZE  IR_AE_FACE2_SIZE;
	struct VREG_ISP_AE_HIST_IR_AE_FACE3_SIZE  IR_AE_FACE3_SIZE;
	struct VREG_ISP_AE_HIST_AE_FACE_ENABLE_CTRL  AE_FACE_ENABLE_CTRL;
	struct VREG_ISP_AE_HIST_AE_FACE0_STS_DIV  AE_FACE0_STS_DIV;
	struct VREG_ISP_AE_HIST_AE_FACE1_STS_DIV  AE_FACE1_STS_DIV;
	struct VREG_ISP_AE_HIST_AE_FACE2_STS_DIV  AE_FACE2_STS_DIV;
	struct VREG_ISP_AE_HIST_AE_FACE3_STS_DIV  AE_FACE3_STS_DIV;
	struct VREG_ISP_AE_HIST_STS_ENABLE       STS_ENABLE;
	struct VREG_ISP_AE_HIST_AE_ALGO_ENABLE   AE_ALGO_ENABLE;
	struct VREG_ISP_AE_HIST_AE_HIST_LOW      AE_HIST_LOW;
	struct VREG_ISP_AE_HIST_AE_HIST_HIGH     AE_HIST_HIGH;
	struct VREG_ISP_AE_HIST_AE_TOP           AE_TOP;
	struct VREG_ISP_AE_HIST_AE_BOT           AE_BOT;
	struct VREG_ISP_AE_HIST_AE_OVEREXP_THR   AE_OVEREXP_THR;
	struct VREG_ISP_AE_HIST_AE_NUM_GAPLINE   AE_NUM_GAPLINE;
	struct VREG_ISP_AE_HIST_AE_CHOOSE_GR_EN  AE_CHOOSE_GR_EN;
	uint32_t                                _resv_0x128[2];
	struct VREG_ISP_AE_HIST_SE_AE_BLC_OFFSET_R  SE_AE_BLC_OFFSET_R;
	struct VREG_ISP_AE_HIST_SE_AE_BLC_OFFSET_GR  SE_AE_BLC_OFFSET_GR;
	struct VREG_ISP_AE_HIST_SE_AE_BLC_OFFSET_GB  SE_AE_BLC_OFFSET_GB;
	struct VREG_ISP_AE_HIST_SE_AE_BLC_OFFSET_B  SE_AE_BLC_OFFSET_B;
	struct VREG_ISP_AE_HIST_SE_AE_BLC_OFFSET_IR  SE_AE_BLC_OFFSET_IR;
	struct VREG_ISP_AE_HIST_SE_AE_BLC_GAIN_R  SE_AE_BLC_GAIN_R;
	struct VREG_ISP_AE_HIST_SE_AE_BLC_GAIN_GR  SE_AE_BLC_GAIN_GR;
	struct VREG_ISP_AE_HIST_SE_AE_BLC_GAIN_GB  SE_AE_BLC_GAIN_GB;
	struct VREG_ISP_AE_HIST_SE_AE_BLC_GAIN_B  SE_AE_BLC_GAIN_B;
	struct VREG_ISP_AE_HIST_SE_AE_BLC_GAIN_IR  SE_AE_BLC_GAIN_IR;
	struct VREG_ISP_AE_HIST_SE_AE_BLC_ENABLE  SE_AE_BLC_ENABLE;
	struct VREG_RESV                        _resv_0x15c[1];
	struct VREG_ISP_AE_HIST_FRAME_COUNTER_AWB  FRAME_COUNTER_AWB;
	struct VREG_RESV                        _resv_0x164[1];
	struct VREG_ISP_AE_HIST_AE_SIMPLE2A_RESULT_LUMA_SE  AE_SIMPLE2A_RESULT_LUMA_SE;
	struct VREG_ISP_AE_HIST_AE_SIMPLE2A_RESULT_RGAIN_SE  AE_SIMPLE2A_RESULT_RGAIN_SE;
	struct VREG_ISP_AE_HIST_AE_SIMPLE2A_RESULT_BGAIN_SE  AE_SIMPLE2A_RESULT_BGAIN_SE;
	struct VREG_ISP_AE_HIST_AE_SIMPLE2A_RESULT_LUMA_LE  AE_SIMPLE2A_RESULT_LUMA_LE;
	struct VREG_ISP_AE_HIST_AE_SIMPLE2A_RESULT_RGAIN_LE  AE_SIMPLE2A_RESULT_RGAIN_LE;
	struct VREG_ISP_AE_HIST_AE_SIMPLE2A_RESULT_BGAIN_LE  AE_SIMPLE2A_RESULT_BGAIN_LE;
	struct VREG_ISP_AE_HIST_AE_FACE0_RESULT_SE  AE_FACE0_RESULT_SE;
	struct VREG_ISP_AE_HIST_AE_FACE1_RESULT_SE  AE_FACE1_RESULT_SE;
	struct VREG_ISP_AE_HIST_AE_FACE2_RESULT_SE  AE_FACE2_RESULT_SE;
	struct VREG_ISP_AE_HIST_AE_FACE3_RESULT_SE  AE_FACE3_RESULT_SE;
	struct VREG_ISP_AE_HIST_AE_FACE0_RESULT_LE  AE_FACE0_RESULT_LE;
	struct VREG_ISP_AE_HIST_AE_FACE1_RESULT_LE  AE_FACE1_RESULT_LE;
	struct VREG_ISP_AE_HIST_AE_FACE2_RESULT_LE  AE_FACE2_RESULT_LE;
	struct VREG_ISP_AE_HIST_AE_FACE3_RESULT_LE  AE_FACE3_RESULT_LE;
	struct VREG_RESV                        _resv_0x1a0[24];
	struct VREG_ISP_AE_HIST_AE_WGT_00        AE_WGT_00;
	struct VREG_ISP_AE_HIST_AE_WGT_01        AE_WGT_01;
	struct VREG_ISP_AE_HIST_AE_WGT_02        AE_WGT_02;
	struct VREG_ISP_AE_HIST_AE_WGT_03        AE_WGT_03;
	struct VREG_ISP_AE_HIST_AE_WGT_04        AE_WGT_04;
	struct VREG_ISP_AE_HIST_AE_WGT_05        AE_WGT_05;
	struct VREG_ISP_AE_HIST_AE_WGT_06        AE_WGT_06;
	struct VREG_ISP_AE_HIST_AE_WGT_07        AE_WGT_07;
	struct VREG_ISP_AE_HIST_AE_WGT_08        AE_WGT_08;
	struct VREG_ISP_AE_HIST_AE_WGT_09        AE_WGT_09;
	struct VREG_ISP_AE_HIST_AE_WGT_10        AE_WGT_10;
	struct VREG_ISP_AE_HIST_AE_WGT_11        AE_WGT_11;
	struct VREG_ISP_AE_HIST_AE_WGT_12        AE_WGT_12;
	struct VREG_ISP_AE_HIST_AE_WGT_13        AE_WGT_13;
	struct VREG_ISP_AE_HIST_AE_WGT_14        AE_WGT_14;
	struct VREG_ISP_AE_HIST_AE_WGT_15        AE_WGT_15;
	struct VREG_ISP_AE_HIST_AE_WGT_16        AE_WGT_16;
	struct VREG_ISP_AE_HIST_AE_WGT_17        AE_WGT_17;
	struct VREG_ISP_AE_HIST_AE_WGT_18        AE_WGT_18;
	struct VREG_ISP_AE_HIST_AE_WGT_19        AE_WGT_19;
	struct VREG_ISP_AE_HIST_AE_WGT_20        AE_WGT_20;
	struct VREG_ISP_AE_HIST_AE_WGT_21        AE_WGT_21;
	struct VREG_ISP_AE_HIST_AE_WGT_22        AE_WGT_22;
	struct VREG_ISP_AE_HIST_AE_WGT_23        AE_WGT_23;
	struct VREG_ISP_AE_HIST_AE_WGT_24        AE_WGT_24;
	struct VREG_ISP_AE_HIST_AE_WGT_25        AE_WGT_25;
	struct VREG_ISP_AE_HIST_AE_WGT_26        AE_WGT_26;
	struct VREG_ISP_AE_HIST_AE_WGT_27        AE_WGT_27;
	struct VREG_ISP_AE_HIST_AE_WGT_28        AE_WGT_28;
	struct VREG_ISP_AE_HIST_AE_WGT_29        AE_WGT_29;
	struct VREG_ISP_AE_HIST_AE_WGT_30        AE_WGT_30;
	struct VREG_ISP_AE_HIST_AE_WGT_31        AE_WGT_31;
	struct VREG_ISP_AE_HIST_LE_AE_BLC_OFFSET_R  LE_AE_BLC_OFFSET_R;
	struct VREG_ISP_AE_HIST_LE_AE_BLC_OFFSET_GR  LE_AE_BLC_OFFSET_GR;
	struct VREG_ISP_AE_HIST_LE_AE_BLC_OFFSET_GB  LE_AE_BLC_OFFSET_GB;
	struct VREG_ISP_AE_HIST_LE_AE_BLC_OFFSET_B  LE_AE_BLC_OFFSET_B;
	struct VREG_ISP_AE_HIST_LE_AE_BLC_OFFSET_IR  LE_AE_BLC_OFFSET_IR;
	struct VREG_ISP_AE_HIST_LE_AE_BLC_GAIN_R  LE_AE_BLC_GAIN_R;
	struct VREG_ISP_AE_HIST_LE_AE_BLC_GAIN_GR  LE_AE_BLC_GAIN_GR;
	struct VREG_ISP_AE_HIST_LE_AE_BLC_GAIN_GB  LE_AE_BLC_GAIN_GB;
	struct VREG_ISP_AE_HIST_LE_AE_BLC_GAIN_B  LE_AE_BLC_GAIN_B;
	struct VREG_ISP_AE_HIST_LE_AE_BLC_GAIN_IR  LE_AE_BLC_GAIN_IR;
	struct VREG_ISP_AE_HIST_LE_AE_BLC_ENABLE  LE_AE_BLC_ENABLE;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_WDMA_CORE_SHADOW_RD_SEL   {
	union REG_WDMA_CORE_SHADOW_RD_SEL       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_IP_DISABLE {
	union REG_WDMA_CORE_IP_DISABLE          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_DISABLE_SEGLEN {
	union REG_WDMA_CORE_DISABLE_SEGLEN      write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_UP_RING_BASE {
	union REG_WDMA_CORE_UP_RING_BASE        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NORM_STATUS0 {
	union REG_WDMA_CORE_NORM_STATUS0        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NORM_STATUS1 {
	union REG_WDMA_CORE_NORM_STATUS1        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_EN {
	union REG_WDMA_CORE_RING_BUFFER_EN      write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NORM_PERF  {
	union REG_WDMA_CORE_NORM_PERF           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_PATCH_ENABLE {
	union REG_WDMA_CORE_RING_PATCH_ENABLE   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_SET_RING_BASE {
	union REG_WDMA_CORE_SET_RING_BASE       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BASE_ADDR_L {
	union REG_WDMA_CORE_RING_BASE_ADDR_L    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BASE_ADDR_H {
	union REG_WDMA_CORE_RING_BASE_ADDR_H    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE0 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE0   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE1 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE1   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE2 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE2   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE3 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE3   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE4 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE4   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE5 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE5   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE6 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE6   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE7 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE7   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE8 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE8   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE9 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE9   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE10 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE10  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE11 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE11  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE12 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE12  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE13 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE13  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE14 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE14  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE15 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE15  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE16 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE16  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE17 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE17  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE18 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE18  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE19 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE19  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE20 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE20  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE21 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE21  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE22 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE22  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE23 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE23  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE24 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE24  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE25 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE25  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE26 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE26  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE27 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE27  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE28 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE28  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE29 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE29  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE30 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE30  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_RING_BUFFER_SIZE31 {
	union REG_WDMA_CORE_RING_BUFFER_SIZE31  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS0 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS0  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS1 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS1  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS2 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS2  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS3 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS3  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS4 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS4  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS5 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS5  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS6 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS6  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS7 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS7  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS8 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS8  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS9 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS9  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS10 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS10  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS11 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS11  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS12 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS12  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS13 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS13  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS14 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS14  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS15 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS15  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS16 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS16  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS17 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS17  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS18 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS18  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS19 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS19  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS20 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS20  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS21 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS21  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS22 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS22  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS23 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS23  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS24 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS24  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS25 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS25  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS26 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS26  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS27 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS27  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS28 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS28  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS29 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS29  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS30 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS30  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS31 {
	union REG_WDMA_CORE_NEXT_DMA_ADDR_STS31  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_WDMA_CORE_T {
	struct VREG_WDMA_CORE_SHADOW_RD_SEL      SHADOW_RD_SEL;
	struct VREG_WDMA_CORE_IP_DISABLE         IP_DISABLE;
	struct VREG_WDMA_CORE_DISABLE_SEGLEN     DISABLE_SEGLEN;
	struct VREG_WDMA_CORE_UP_RING_BASE       UP_RING_BASE;
	struct VREG_WDMA_CORE_NORM_STATUS0       NORM_STATUS0;
	struct VREG_WDMA_CORE_NORM_STATUS1       NORM_STATUS1;
	struct VREG_WDMA_CORE_RING_BUFFER_EN     RING_BUFFER_EN;
	struct VREG_RESV                        _resv_0x1c[1];
	struct VREG_WDMA_CORE_NORM_PERF          NORM_PERF;
	struct VREG_RESV                        _resv_0x24[1];
	struct VREG_WDMA_CORE_RING_PATCH_ENABLE  RING_PATCH_ENABLE;
	struct VREG_WDMA_CORE_SET_RING_BASE      SET_RING_BASE;
	struct VREG_WDMA_CORE_RING_BASE_ADDR_L   RING_BASE_ADDR_L;
	struct VREG_WDMA_CORE_RING_BASE_ADDR_H   RING_BASE_ADDR_H;
	struct VREG_RESV                        _resv_0x38[18];
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE0  RING_BUFFER_SIZE0;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE1  RING_BUFFER_SIZE1;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE2  RING_BUFFER_SIZE2;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE3  RING_BUFFER_SIZE3;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE4  RING_BUFFER_SIZE4;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE5  RING_BUFFER_SIZE5;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE6  RING_BUFFER_SIZE6;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE7  RING_BUFFER_SIZE7;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE8  RING_BUFFER_SIZE8;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE9  RING_BUFFER_SIZE9;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE10  RING_BUFFER_SIZE10;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE11  RING_BUFFER_SIZE11;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE12  RING_BUFFER_SIZE12;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE13  RING_BUFFER_SIZE13;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE14  RING_BUFFER_SIZE14;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE15  RING_BUFFER_SIZE15;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE16  RING_BUFFER_SIZE16;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE17  RING_BUFFER_SIZE17;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE18  RING_BUFFER_SIZE18;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE19  RING_BUFFER_SIZE19;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE20  RING_BUFFER_SIZE20;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE21  RING_BUFFER_SIZE21;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE22  RING_BUFFER_SIZE22;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE23  RING_BUFFER_SIZE23;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE24  RING_BUFFER_SIZE24;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE25  RING_BUFFER_SIZE25;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE26  RING_BUFFER_SIZE26;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE27  RING_BUFFER_SIZE27;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE28  RING_BUFFER_SIZE28;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE29  RING_BUFFER_SIZE29;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE30  RING_BUFFER_SIZE30;
	struct VREG_WDMA_CORE_RING_BUFFER_SIZE31  RING_BUFFER_SIZE31;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS0  NEXT_DMA_ADDR_STS0;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS1  NEXT_DMA_ADDR_STS1;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS2  NEXT_DMA_ADDR_STS2;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS3  NEXT_DMA_ADDR_STS3;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS4  NEXT_DMA_ADDR_STS4;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS5  NEXT_DMA_ADDR_STS5;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS6  NEXT_DMA_ADDR_STS6;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS7  NEXT_DMA_ADDR_STS7;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS8  NEXT_DMA_ADDR_STS8;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS9  NEXT_DMA_ADDR_STS9;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS10  NEXT_DMA_ADDR_STS10;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS11  NEXT_DMA_ADDR_STS11;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS12  NEXT_DMA_ADDR_STS12;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS13  NEXT_DMA_ADDR_STS13;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS14  NEXT_DMA_ADDR_STS14;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS15  NEXT_DMA_ADDR_STS15;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS16  NEXT_DMA_ADDR_STS16;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS17  NEXT_DMA_ADDR_STS17;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS18  NEXT_DMA_ADDR_STS18;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS19  NEXT_DMA_ADDR_STS19;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS20  NEXT_DMA_ADDR_STS20;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS21  NEXT_DMA_ADDR_STS21;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS22  NEXT_DMA_ADDR_STS22;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS23  NEXT_DMA_ADDR_STS23;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS24  NEXT_DMA_ADDR_STS24;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS25  NEXT_DMA_ADDR_STS25;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS26  NEXT_DMA_ADDR_STS26;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS27  NEXT_DMA_ADDR_STS27;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS28  NEXT_DMA_ADDR_STS28;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS29  NEXT_DMA_ADDR_STS29;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS30  NEXT_DMA_ADDR_STS30;
	struct VREG_WDMA_CORE_NEXT_DMA_ADDR_STS31  NEXT_DMA_ADDR_STS31;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_RAW_TOP0_RAW_TOP_READ_SEL {
	union REG_RAW_TOP0_RAW_TOP_READ_SEL     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP0_CHK_SUM_EN {
	union REG_RAW_TOP0_CHK_SUM_EN           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP0_RAW_2 {
	union REG_RAW_TOP0_RAW_2                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP0_RAW_3 {
	union REG_RAW_TOP0_RAW_3                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP0_RAW_BAYER_TYPE_TOPLEFT {
	union REG_RAW_TOP0_RAW_BAYER_TYPE_TOPLEFT  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP0_PASS_SEL {
	union REG_RAW_TOP0_PASS_SEL             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP0_IP_CFG_GCLK_EN {
	union REG_RAW_TOP0_IP_CFG_GCLK_EN       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP0_IP_BYPASS {
	union REG_RAW_TOP0_IP_BYPASS            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP0_PG_CFG0 {
	union REG_RAW_TOP0_PG_CFG0              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP0_PG_CFG1 {
	union REG_RAW_TOP0_PG_CFG1              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP0_T {
	struct VREG_RAW_TOP0_RAW_TOP_READ_SEL    RAW_TOP_READ_SEL;
	struct VREG_RAW_TOP0_CHK_SUM_EN          CHK_SUM_EN;
	struct VREG_RAW_TOP0_RAW_2               RAW_2;
	struct VREG_RAW_TOP0_RAW_3               RAW_3;
	struct VREG_RAW_TOP0_RAW_BAYER_TYPE_TOPLEFT  RAW_BAYER_TYPE_TOPLEFT;
	struct VREG_RAW_TOP0_PASS_SEL            PASS_SEL;
	struct VREG_RAW_TOP0_IP_CFG_GCLK_EN      IP_CFG_GCLK_EN;
	struct VREG_RAW_TOP0_IP_BYPASS           IP_BYPASS;
	struct VREG_RAW_TOP0_PG_CFG0             PG_CFG0;
	struct VREG_RAW_TOP0_PG_CFG1             PG_CFG1;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_RAW_TOP1_RAW_TOP_READ_SEL {
	union REG_RAW_TOP1_RAW_TOP_READ_SEL     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP1_CHK_SUM_EN {
	union REG_RAW_TOP1_CHK_SUM_EN           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP1_RAW_2 {
	union REG_RAW_TOP1_RAW_2                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP1_RAW_4 {
	union REG_RAW_TOP1_RAW_4                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP1_RAW_TOP_STATUS {
	union REG_RAW_TOP1_RAW_TOP_STATUS       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP1_RAW_TOP_DEBUG {
	union REG_RAW_TOP1_RAW_TOP_DEBUG        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP1_RAW_BAYER_TYPE {
	union REG_RAW_TOP1_RAW_BAYER_TYPE       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP1_FIFO_REVERSE {
	union REG_RAW_TOP1_FIFO_REVERSE         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP1_RAW_TOP_DMA_IDLE {
	union REG_RAW_TOP1_RAW_TOP_DMA_IDLE     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP1_RAW_TOP_CHECKSUM {
	union REG_RAW_TOP1_RAW_TOP_CHECKSUM     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP1_IP_CFG_GCLK_EN {
	union REG_RAW_TOP1_IP_CFG_GCLK_EN       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP1_IP_BYPASS {
	union REG_RAW_TOP1_IP_BYPASS            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP1_AF_BLC_CFG0 {
	union REG_RAW_TOP1_AF_BLC_CFG0          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP1_AF_BLC_CFG1 {
	union REG_RAW_TOP1_AF_BLC_CFG1          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP1_AF_BLC_CFG2 {
	union REG_RAW_TOP1_AF_BLC_CFG2          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP1_AF_BLC_CFG3 {
	union REG_RAW_TOP1_AF_BLC_CFG3          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP1_AF_BLC_CFG4 {
	union REG_RAW_TOP1_AF_BLC_CFG4          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RAW_TOP1_T {
	struct VREG_RAW_TOP1_RAW_TOP_READ_SEL    RAW_TOP_READ_SEL;
	struct VREG_RAW_TOP1_CHK_SUM_EN          CHK_SUM_EN;
	struct VREG_RAW_TOP1_RAW_2               RAW_2;
	struct VREG_RAW_TOP1_RAW_4               RAW_4;
	struct VREG_RAW_TOP1_RAW_TOP_STATUS      RAW_TOP_STATUS;
	struct VREG_RAW_TOP1_RAW_TOP_DEBUG       RAW_TOP_DEBUG;
	struct VREG_RAW_TOP1_RAW_BAYER_TYPE      RAW_BAYER_TYPE;
	struct VREG_RAW_TOP1_FIFO_REVERSE        FIFO_REVERSE;
	struct VREG_RESV                        _resv_0x20[1];
	struct VREG_RAW_TOP1_RAW_TOP_DMA_IDLE    RAW_TOP_DMA_IDLE;
	struct VREG_RAW_TOP1_RAW_TOP_CHECKSUM    RAW_TOP_CHECKSUM;
	struct VREG_RAW_TOP1_IP_CFG_GCLK_EN      IP_CFG_GCLK_EN;
	struct VREG_RAW_TOP1_IP_BYPASS           IP_BYPASS;
	struct VREG_RAW_TOP1_AF_BLC_CFG0         BLC_CFG0;
	struct VREG_RAW_TOP1_AF_BLC_CFG1         BLC_CFG1;
	struct VREG_RAW_TOP1_AF_BLC_CFG2         BLC_CFG2;
	struct VREG_RAW_TOP1_AF_BLC_CFG3         BLC_CFG3;
	struct VREG_RAW_TOP1_AF_BLC_CFG4         BLC_CFG4;
};


/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_AF_STATUS {
	union REG_ISP_AF_STATUS                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_GRACE_RESET {
	union REG_ISP_AF_GRACE_RESET            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_MONITOR {
	union REG_ISP_AF_MONITOR                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_BYPASS {
	union REG_ISP_AF_BYPASS                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_KICKOFF {
	union REG_ISP_AF_KICKOFF                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_ENABLES {
	union REG_ISP_AF_ENABLES                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_OFFSET_X {
	union REG_ISP_AF_OFFSET_X               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_MXN_IMAGE_WIDTH_M1 {
	union REG_ISP_AF_MXN_IMAGE_WIDTH_M1     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_BLOCK_WIDTH {
	union REG_ISP_AF_BLOCK_WIDTH            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_BLOCK_HEIGHT {
	union REG_ISP_AF_BLOCK_HEIGHT           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_BLOCK_NUM_X {
	union REG_ISP_AF_BLOCK_NUM_X            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_BLOCK_NUM_Y {
	union REG_ISP_AF_BLOCK_NUM_Y            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_HOR_LOW_PASS_VALUE_SHIFT {
	union REG_ISP_AF_HOR_LOW_PASS_VALUE_SHIFT  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_CORNING_OFFSET_HORIZONTAL_0 {
	union REG_ISP_AF_CORNING_OFFSET_HORIZONTAL_0  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_CORNING_OFFSET_HORIZONTAL_1 {
	union REG_ISP_AF_CORNING_OFFSET_HORIZONTAL_1  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_CORNING_OFFSET_VERTICAL {
	union REG_ISP_AF_CORNING_OFFSET_VERTICAL  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_HIGH_Y_THRE {
	union REG_ISP_AF_HIGH_Y_THRE            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_LOW_PASS_HORIZON {
	union REG_ISP_AF_LOW_PASS_HORIZON       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_LOCATION {
	union REG_ISP_AF_LOCATION               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_HIGH_PASS_HORIZON_0 {
	union REG_ISP_AF_HIGH_PASS_HORIZON_0    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_HIGH_PASS_HORIZON_1 {
	union REG_ISP_AF_HIGH_PASS_HORIZON_1    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_HIGH_PASS_VERTICAL_0 {
	union REG_ISP_AF_HIGH_PASS_VERTICAL_0   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_MEM_SW_MODE {
	union REG_ISP_AF_MEM_SW_MODE            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_MONITOR_SELECT {
	union REG_ISP_AF_MONITOR_SELECT         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_IMAGE_WIDTH {
	union REG_ISP_AF_IMAGE_WIDTH            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_DUMMY {
	union REG_ISP_AF_DUMMY                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_MEM_SW_RADDR {
	union REG_ISP_AF_MEM_SW_RADDR           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_MEM_SW_RDATA {
	union REG_ISP_AF_MEM_SW_RDATA           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_MXN_BORDER {
	union REG_ISP_AF_MXN_BORDER             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_TH_LOW    {
	union REG_ISP_AF_TH_LOW                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_GAIN_LOW  {
	union REG_ISP_AF_GAIN_LOW               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_SLOP_LOW {
	union REG_ISP_AF_SLOP_LOW               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_DMI_ENABLE {
	union REG_ISP_AF_DMI_ENABLE             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_SQUARE_ENABLE {
	union REG_ISP_AF_SQUARE_ENABLE          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_OUTSHIFT {
	union REG_ISP_AF_OUTSHIFT               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_NUM_GAPLINE {
	union REG_ISP_AF_NUM_GAPLINE            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_AF_T {
	struct VREG_ISP_AF_STATUS                STATUS;
	struct VREG_ISP_AF_GRACE_RESET           GRACE_RESET;
	struct VREG_ISP_AF_MONITOR               MONITOR;
	struct VREG_ISP_AF_BYPASS                BYPASS;
	struct VREG_ISP_AF_KICKOFF               KICKOFF;
	struct VREG_ISP_AF_ENABLES               ENABLES;
	struct VREG_ISP_AF_OFFSET_X              OFFSET_X;
	struct VREG_ISP_AF_MXN_IMAGE_WIDTH_M1    MXN_IMAGE_WIDTH_M1;
	struct VREG_ISP_AF_BLOCK_WIDTH           BLOCK_WIDTH;
	struct VREG_ISP_AF_BLOCK_HEIGHT          BLOCK_HEIGHT;
	struct VREG_ISP_AF_BLOCK_NUM_X           BLOCK_NUM_X;
	struct VREG_ISP_AF_BLOCK_NUM_Y           BLOCK_NUM_Y;
	struct VREG_RESV                        _resv_0x30[1];
	struct VREG_ISP_AF_HOR_LOW_PASS_VALUE_SHIFT  HOR_LOW_PASS_VALUE_SHIFT;
	struct VREG_ISP_AF_CORNING_OFFSET_HORIZONTAL_0  OFFSET_HORIZONTAL_0;
	struct VREG_ISP_AF_CORNING_OFFSET_HORIZONTAL_1  OFFSET_HORIZONTAL_1;
	struct VREG_ISP_AF_CORNING_OFFSET_VERTICAL  OFFSET_VERTICAL;
	struct VREG_ISP_AF_HIGH_Y_THRE           HIGH_Y_THRE;
	struct VREG_ISP_AF_LOW_PASS_HORIZON      LOW_PASS_HORIZON;
	struct VREG_ISP_AF_LOCATION              LOCATION;
	struct VREG_ISP_AF_HIGH_PASS_HORIZON_0   HIGH_PASS_HORIZON_0;
	struct VREG_ISP_AF_HIGH_PASS_HORIZON_1   HIGH_PASS_HORIZON_1;
	struct VREG_ISP_AF_HIGH_PASS_VERTICAL_0  HIGH_PASS_VERTICAL_0;
	struct VREG_ISP_AF_MEM_SW_MODE           SW_MODE;
	struct VREG_ISP_AF_MONITOR_SELECT        MONITOR_SELECT;
	struct VREG_RESV                        _resv_0x64[2];
	struct VREG_ISP_AF_IMAGE_WIDTH           IMAGE_WIDTH;
	struct VREG_ISP_AF_DUMMY                 DUMMY;
	struct VREG_ISP_AF_MEM_SW_RADDR          SW_RADDR;
	struct VREG_ISP_AF_MEM_SW_RDATA          SW_RDATA;
	struct VREG_ISP_AF_MXN_BORDER            MXN_BORDER;
	struct VREG_ISP_AF_TH_LOW                TH_LOW;
	struct VREG_ISP_AF_GAIN_LOW              GAIN_LOW;
	struct VREG_ISP_AF_SLOP_LOW              SLOP_LOW;
	struct VREG_RESV                        _resv_0x8c[5];
	struct VREG_ISP_AF_DMI_ENABLE            DMI_ENABLE;
	struct VREG_RESV                        _resv_0xa4[45];
	struct VREG_ISP_AF_SQUARE_ENABLE         SQUARE_ENABLE;
	struct VREG_RESV                        _resv_0x15c[2];
	struct VREG_ISP_AF_OUTSHIFT              OUTSHIFT;
	struct VREG_RESV                        _resv_0x168[1];
	struct VREG_ISP_AF_NUM_GAPLINE           NUM_GAPLINE;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_GMS_STATUS {
	union REG_ISP_GMS_STATUS                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GMS_GRACE_RESET {
	union REG_ISP_GMS_GRACE_RESET           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GMS_MONITOR {
	union REG_ISP_GMS_MONITOR               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GMS_ENABLE {
	union REG_ISP_GMS_ENABLE                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GMS_FLOW {
	union REG_ISP_GMS_FLOW                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GMS_START_X {
	union REG_ISP_GMS_START_X               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GMS_START_Y {
	union REG_ISP_GMS_START_Y               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GMS_LOCATION {
	union REG_ISP_GMS_LOCATION              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GMS_X_SIZEM1 {
	union REG_ISP_GMS_X_SIZEM1              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GMS_Y_SIZEM1 {
	union REG_ISP_GMS_Y_SIZEM1              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GMS_X_GAP {
	union REG_ISP_GMS_X_GAP                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GMS_Y_GAP {
	union REG_ISP_GMS_Y_GAP                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GMS_DUMMY {
	union REG_ISP_GMS_DUMMY                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GMS_MEM_SW_MODE {
	union REG_ISP_GMS_MEM_SW_MODE           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GMS_MEM_SW_RADDR {
	union REG_ISP_GMS_MEM_SW_RADDR          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GMS_MEM_SW_RDATA {
	union REG_ISP_GMS_MEM_SW_RDATA          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GMS_MONITOR_SELECT {
	union REG_ISP_GMS_MONITOR_SELECT        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GMS_DMI_ENABLE {
	union REG_ISP_GMS_DMI_ENABLE            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GMS_T {
	struct VREG_ISP_GMS_STATUS               GMS_STATUS;
	struct VREG_ISP_GMS_GRACE_RESET          GMS_GRACE_RESET;
	struct VREG_ISP_GMS_MONITOR              GMS_MONITOR;
	struct VREG_ISP_GMS_ENABLE               GMS_ENABLE;
	struct VREG_RESV                        _resv_0x10[1];
	struct VREG_ISP_GMS_FLOW                 GMS_FLOW;
	struct VREG_ISP_GMS_START_X              GMS_START_X;
	struct VREG_ISP_GMS_START_Y              GMS_START_Y;
	struct VREG_ISP_GMS_LOCATION             GMS_LOCATION;
	struct VREG_RESV                        _resv_0x24[1];
	struct VREG_ISP_GMS_X_SIZEM1             GMS_X_SIZEM1;
	struct VREG_ISP_GMS_Y_SIZEM1             GMS_Y_SIZEM1;
	struct VREG_ISP_GMS_X_GAP                GMS_X_GAP;
	struct VREG_ISP_GMS_Y_GAP                GMS_Y_GAP;
	struct VREG_ISP_GMS_DUMMY                GMS_DUMMY;
	struct VREG_RESV                        _resv_0x3c[1];
	struct VREG_ISP_GMS_MEM_SW_MODE          GMS_SW_MODE;
	struct VREG_ISP_GMS_MEM_SW_RADDR         GMS_SW_RADDR;
	struct VREG_ISP_GMS_MEM_SW_RDATA         GMS_SW_RDATA;
	struct VREG_ISP_GMS_MONITOR_SELECT       GMS_MONITOR_SELECT;
	struct VREG_RESV                        _resv_0x50[20];
	struct VREG_ISP_GMS_DMI_ENABLE           DMI_ENABLE;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_SE_BNR_U1_SE_BNR_ENABLE {
	union REG_ISP_SE_BNR_U1_SE_BNR_ENABLE   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_SE_BNR_U4_SE_BNR_SPATIAL_STR {
	union REG_ISP_SE_BNR_U4_SE_BNR_SPATIAL_STR  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_SE_BNR_U10_SE_BNR_INTENSITY_STR_LUT_0 {
	union REG_ISP_SE_BNR_U10_SE_BNR_INTENSITY_STR_LUT_0  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_SE_BNR_U10_SE_BNR_INTENSITY_STR_LUT_1 {
	union REG_ISP_SE_BNR_U10_SE_BNR_INTENSITY_STR_LUT_1  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_SE_BNR_U10_SE_BNR_INTENSITY_STR_LUT_2 {
	union REG_ISP_SE_BNR_U10_SE_BNR_INTENSITY_STR_LUT_2  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_SE_BNR_U10_SE_BNR_INTENSITY_STR_LUT_3 {
	union REG_ISP_SE_BNR_U10_SE_BNR_INTENSITY_STR_LUT_3  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_SE_BNR_U10_SE_BNR_INTENSITY_STR_LUT_4 {
	union REG_ISP_SE_BNR_U10_SE_BNR_INTENSITY_STR_LUT_4  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_SE_BNR_U10_SE_BNR_INTENSITY_STR_LUT_5 {
	union REG_ISP_SE_BNR_U10_SE_BNR_INTENSITY_STR_LUT_5  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_SE_BNR_SHDW_READ_SEL {
	union REG_ISP_SE_BNR_SHDW_READ_SEL      write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_SE_BNR_HW_AUTO_CG_EN {
	union REG_ISP_SE_BNR_HW_AUTO_CG_EN      write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_SE_BNR_T {
	struct VREG_ISP_SE_BNR_U1_SE_BNR_ENABLE  U1_SE_BNR_ENABLE;
	struct VREG_ISP_SE_BNR_U4_SE_BNR_SPATIAL_STR  U4_SE_BNR_SPATIAL_STR;
	struct VREG_ISP_SE_BNR_U10_SE_BNR_INTENSITY_STR_LUT_0  U10_SE_BNR_INTENSITY_STR_LUT_0;
	struct VREG_ISP_SE_BNR_U10_SE_BNR_INTENSITY_STR_LUT_1  U10_SE_BNR_INTENSITY_STR_LUT_1;
	struct VREG_ISP_SE_BNR_U10_SE_BNR_INTENSITY_STR_LUT_2  U10_SE_BNR_INTENSITY_STR_LUT_2;
	struct VREG_ISP_SE_BNR_U10_SE_BNR_INTENSITY_STR_LUT_3  U10_SE_BNR_INTENSITY_STR_LUT_3;
	struct VREG_ISP_SE_BNR_U10_SE_BNR_INTENSITY_STR_LUT_4  U10_SE_BNR_INTENSITY_STR_LUT_4;
	struct VREG_ISP_SE_BNR_U10_SE_BNR_INTENSITY_STR_LUT_5  U10_SE_BNR_INTENSITY_STR_LUT_5;
	struct VREG_ISP_SE_BNR_SHDW_READ_SEL     SHDW_READ_SEL;
	struct VREG_ISP_SE_BNR_HW_AUTO_CG_EN     HW_AUTO_CG_EN;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_BLC_DG_WB_BASE_CONFIG {
	union REG_ISP_BLC_DG_WB_BASE_CONFIG         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BLC_DG_WBG_LE_GAIN0 {
	union REG_ISP_BLC_DG_WBG_LE_GAIN0           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BLC_DG_WBG_LE_GAIN1 {
	union REG_ISP_BLC_DG_WBG_LE_GAIN1           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BLC_DG_WB_BLC_LE_OFFSET0 {
	union REG_ISP_BLC_DG_WB_BLC_LE_OFFSET0      write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BLC_DG_WB_BLC_LE_OFFSET1 {
	union REG_ISP_BLC_DG_WB_BLC_LE_OFFSET1      write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BLC_DG_WB_BLC_LE_OFFSET_2ND0 {
	union REG_ISP_BLC_DG_WB_BLC_LE_OFFSET_2ND0  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BLC_DG_WB_BLC_LE_OFFSET_2ND1 {
	union REG_ISP_BLC_DG_WB_BLC_LE_OFFSET_2ND1  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BLC_DG_WB_BLC_LE_GAIN0 {
	union REG_ISP_BLC_DG_WB_BLC_LE_GAIN0        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BLC_DG_WB_BLC_LE_GAIN1 {
	union REG_ISP_BLC_DG_WB_BLC_LE_GAIN1        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BLC_DG_WB_BLC_LE_NORMGAIN0 {
	union REG_ISP_BLC_DG_WB_BLC_LE_NORMGAIN0    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BLC_DG_WB_BLC_LE_NORMGAIN1 {
	union REG_ISP_BLC_DG_WB_BLC_LE_NORMGAIN1    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BLC_DG_WBG_SE_GAIN0 {
	union REG_ISP_BLC_DG_WBG_SE_GAIN0           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BLC_DG_WBG_SE_GAIN1 {
	union REG_ISP_BLC_DG_WBG_SE_GAIN1           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BLC_DG_WB_BLC_SE_OFFSET0 {
	union REG_ISP_BLC_DG_WB_BLC_SE_OFFSET0      write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BLC_DG_WB_BLC_SE_OFFSET1 {
	union REG_ISP_BLC_DG_WB_BLC_SE_OFFSET1      write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BLC_DG_WB_BLC_SE_OFFSET_2ND0 {
	union REG_ISP_BLC_DG_WB_BLC_SE_OFFSET_2ND0  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BLC_DG_WB_BLC_SE_OFFSET_2ND1 {
	union REG_ISP_BLC_DG_WB_BLC_SE_OFFSET_2ND1  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BLC_DG_WB_BLC_SE_GAIN0 {
	union REG_ISP_BLC_DG_WB_BLC_SE_GAIN0        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BLC_DG_WB_BLC_SE_GAIN1 {
	union REG_ISP_BLC_DG_WB_BLC_SE_GAIN1        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BLC_DG_WB_BLC_SE_NORMGAIN0 {
	union REG_ISP_BLC_DG_WB_BLC_SE_NORMGAIN0    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BLC_DG_WB_BLC_SE_NORMGAIN1 {
	union REG_ISP_BLC_DG_WB_BLC_SE_NORMGAIN1    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BLC_DG_WB_SHDW_READ_SEL {
	union REG_ISP_BLC_DG_WB_SHDW_READ_SEL       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BLC_DG_WB_AUTO_CG_EN {
	union REG_ISP_BLC_DG_WB_AUTO_CG_EN          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BLC_DG_WB_T {
	struct VREG_ISP_BLC_DG_WB_BASE_CONFIG        BASE_CONFIG;
	struct VREG_ISP_BLC_DG_WBG_LE_GAIN0          WBG_LE_GAIN0;
	struct VREG_ISP_BLC_DG_WBG_LE_GAIN1          WBG_LE_GAIN1;
	struct VREG_ISP_BLC_DG_WB_BLC_LE_OFFSET0     BLC_LE_OFFSET0;
	struct VREG_ISP_BLC_DG_WB_BLC_LE_OFFSET1     BLC_LE_OFFSET1;
	struct VREG_ISP_BLC_DG_WB_BLC_LE_OFFSET_2ND0  BLC_LE_OFFSET_2ND0;
	struct VREG_ISP_BLC_DG_WB_BLC_LE_OFFSET_2ND1  BLC_LE_OFFSET_2ND1;
	struct VREG_ISP_BLC_DG_WB_BLC_LE_GAIN0       BLC_LE_GAIN0;
	struct VREG_ISP_BLC_DG_WB_BLC_LE_GAIN1       BLC_LE_GAIN1;
	struct VREG_ISP_BLC_DG_WB_BLC_LE_NORMGAIN0   BLC_LE_NORMGAIN0;
	struct VREG_ISP_BLC_DG_WB_BLC_LE_NORMGAIN1   BLC_LE_NORMGAIN1;
	struct VREG_ISP_BLC_DG_WBG_SE_GAIN0          WBG_SE_GAIN0;
	struct VREG_ISP_BLC_DG_WBG_SE_GAIN1          WBG_SE_GAIN1;
	struct VREG_ISP_BLC_DG_WB_BLC_SE_OFFSET0     BLC_SE_OFFSET0;
	struct VREG_ISP_BLC_DG_WB_BLC_SE_OFFSET1     BLC_SE_OFFSET1;
	struct VREG_ISP_BLC_DG_WB_BLC_SE_OFFSET_2ND0  BLC_SE_OFFSET_2ND0;
	struct VREG_ISP_BLC_DG_WB_BLC_SE_OFFSET_2ND1  BLC_SE_OFFSET_2ND1;
	struct VREG_ISP_BLC_DG_WB_BLC_SE_GAIN0       BLC_SE_GAIN0;
	struct VREG_ISP_BLC_DG_WB_BLC_SE_GAIN1       BLC_SE_GAIN1;
	struct VREG_ISP_BLC_DG_WB_BLC_SE_NORMGAIN0   BLC_SE_NORMGAIN0;
	struct VREG_ISP_BLC_DG_WB_BLC_SE_NORMGAIN1   BLC_SE_NORMGAIN1;
	struct VREG_ISP_BLC_DG_WB_SHDW_READ_SEL      SHDW_READ_SEL;
	struct VREG_ISP_BLC_DG_WB_AUTO_CG_EN         AUTO_CG_EN;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_FUSION_CTRL_REG {
	union REG_FUSION_CTRL_REG               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FUSION_DS1 {
	union REG_FUSION_DS1                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FUSION_DS2 {
	union REG_FUSION_DS2                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FUSION_BCL_RANGE {
	union REG_FUSION_BCL_RANGE              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FUSION_BCL_P0 {
	union REG_FUSION_BCL_P0                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FUSION_BCL_SLOP {
	union REG_FUSION_BCL_SLOP               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FUSION_BCS_RANGE {
	union REG_FUSION_BCS_RANGE              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FUSION_BCS_P0 {
	union REG_FUSION_BCS_P0                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FUSION_BCS_SLOP {
	union REG_FUSION_BCS_SLOP               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FUSION_DIFF {
	union REG_FUSION_DIFF                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FUSION_LUT_0 {
	union REG_FUSION_LUT_0                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FUSION_LUT_1 {
	union REG_FUSION_LUT_1                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FUSION_LUT_2 {
	union REG_FUSION_LUT_2                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FUSION_HW_AUTO {
	union REG_FUSION_HW_AUTO                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FUSION_SHADOW_RD_SEL {
	union REG_FUSION_SHADOW_RD_SEL          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FUSION_T {
	struct VREG_FUSION_CTRL_REG              ctrl_REG;
	struct VREG_FUSION_DS1                   DS1;
	struct VREG_FUSION_DS2                   DS2;
	struct VREG_FUSION_BCL_RANGE             BCL_RANGE;
	struct VREG_FUSION_BCL_P0                BCL_P0;
	struct VREG_FUSION_BCL_SLOP              BCL_SLOP;
	struct VREG_FUSION_BCS_RANGE             BCS_RANGE;
	struct VREG_FUSION_BCS_P0                BCS_P0;
	struct VREG_FUSION_BCS_SLOP              BCS_SLOP;
	struct VREG_FUSION_DIFF                  DIFF;
	struct VREG_FUSION_LUT_0                 LUT_0;
	struct VREG_FUSION_LUT_1                 LUT_1;
	struct VREG_FUSION_LUT_2                 LUT_2;
	struct VREG_FUSION_HW_AUTO               HW_AUTO;
	struct VREG_FUSION_SHADOW_RD_SEL         SHADOW_RD_SEL;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_MAP_CURVE_01 {
	union REG_ISP_MAP_CURVE_01                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_FLUMW_LUT0 {
	union REG_ISP_MAP_CURVE_FCURVE_FLUMW_LUT0   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_FLUMW_LUT1 {
	union REG_ISP_MAP_CURVE_FCURVE_FLUMW_LUT1   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_FLUMW_LUT2 {
	union REG_ISP_MAP_CURVE_FCURVE_FLUMW_LUT2   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_FLUMW_LUT3 {
	union REG_ISP_MAP_CURVE_FCURVE_FLUMW_LUT3   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_FLUMW_LUT4 {
	union REG_ISP_MAP_CURVE_FCURVE_FLUMW_LUT4   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L0 {
	union REG_ISP_MAP_CURVE_FCURVE_L0           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L1 {
	union REG_ISP_MAP_CURVE_FCURVE_L1           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L2 {
	union REG_ISP_MAP_CURVE_FCURVE_L2           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L3 {
	union REG_ISP_MAP_CURVE_FCURVE_L3           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L4 {
	union REG_ISP_MAP_CURVE_FCURVE_L4           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L5 {
	union REG_ISP_MAP_CURVE_FCURVE_L5           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L6 {
	union REG_ISP_MAP_CURVE_FCURVE_L6           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L7 {
	union REG_ISP_MAP_CURVE_FCURVE_L7           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L8 {
	union REG_ISP_MAP_CURVE_FCURVE_L8           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L9 {
	union REG_ISP_MAP_CURVE_FCURVE_L9           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L10 {
	union REG_ISP_MAP_CURVE_FCURVE_L10          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L11 {
	union REG_ISP_MAP_CURVE_FCURVE_L11          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L12 {
	union REG_ISP_MAP_CURVE_FCURVE_L12          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L13 {
	union REG_ISP_MAP_CURVE_FCURVE_L13          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L14 {
	union REG_ISP_MAP_CURVE_FCURVE_L14          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L15 {
	union REG_ISP_MAP_CURVE_FCURVE_L15          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L16 {
	union REG_ISP_MAP_CURVE_FCURVE_L16          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L17 {
	union REG_ISP_MAP_CURVE_FCURVE_L17          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L18 {
	union REG_ISP_MAP_CURVE_FCURVE_L18          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L19 {
	union REG_ISP_MAP_CURVE_FCURVE_L19          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L20 {
	union REG_ISP_MAP_CURVE_FCURVE_L20          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L21 {
	union REG_ISP_MAP_CURVE_FCURVE_L21          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L22 {
	union REG_ISP_MAP_CURVE_FCURVE_L22          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L23 {
	union REG_ISP_MAP_CURVE_FCURVE_L23          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L24 {
	union REG_ISP_MAP_CURVE_FCURVE_L24          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L25 {
	union REG_ISP_MAP_CURVE_FCURVE_L25          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L26 {
	union REG_ISP_MAP_CURVE_FCURVE_L26          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L27 {
	union REG_ISP_MAP_CURVE_FCURVE_L27          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L28 {
	union REG_ISP_MAP_CURVE_FCURVE_L28          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L29 {
	union REG_ISP_MAP_CURVE_FCURVE_L29          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L30 {
	union REG_ISP_MAP_CURVE_FCURVE_L30          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L31 {
	union REG_ISP_MAP_CURVE_FCURVE_L31          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L32 {
	union REG_ISP_MAP_CURVE_FCURVE_L32          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L33 {
	union REG_ISP_MAP_CURVE_FCURVE_L33          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L34 {
	union REG_ISP_MAP_CURVE_FCURVE_L34          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L35 {
	union REG_ISP_MAP_CURVE_FCURVE_L35          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L36 {
	union REG_ISP_MAP_CURVE_FCURVE_L36          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L37 {
	union REG_ISP_MAP_CURVE_FCURVE_L37          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L38 {
	union REG_ISP_MAP_CURVE_FCURVE_L38          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L39 {
	union REG_ISP_MAP_CURVE_FCURVE_L39          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L40 {
	union REG_ISP_MAP_CURVE_FCURVE_L40          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L41 {
	union REG_ISP_MAP_CURVE_FCURVE_L41          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L42 {
	union REG_ISP_MAP_CURVE_FCURVE_L42          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L43 {
	union REG_ISP_MAP_CURVE_FCURVE_L43          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L44 {
	union REG_ISP_MAP_CURVE_FCURVE_L44          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L45 {
	union REG_ISP_MAP_CURVE_FCURVE_L45          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L46 {
	union REG_ISP_MAP_CURVE_FCURVE_L46          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L47 {
	union REG_ISP_MAP_CURVE_FCURVE_L47          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L48 {
	union REG_ISP_MAP_CURVE_FCURVE_L48          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L49 {
	union REG_ISP_MAP_CURVE_FCURVE_L49          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L50 {
	union REG_ISP_MAP_CURVE_FCURVE_L50          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L51 {
	union REG_ISP_MAP_CURVE_FCURVE_L51          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L52 {
	union REG_ISP_MAP_CURVE_FCURVE_L52          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L53 {
	union REG_ISP_MAP_CURVE_FCURVE_L53          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L54 {
	union REG_ISP_MAP_CURVE_FCURVE_L54          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L55 {
	union REG_ISP_MAP_CURVE_FCURVE_L55          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L56 {
	union REG_ISP_MAP_CURVE_FCURVE_L56          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L57 {
	union REG_ISP_MAP_CURVE_FCURVE_L57          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L58 {
	union REG_ISP_MAP_CURVE_FCURVE_L58          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L59 {
	union REG_ISP_MAP_CURVE_FCURVE_L59          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L60 {
	union REG_ISP_MAP_CURVE_FCURVE_L60          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L61 {
	union REG_ISP_MAP_CURVE_FCURVE_L61          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L62 {
	union REG_ISP_MAP_CURVE_FCURVE_L62          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L63 {
	union REG_ISP_MAP_CURVE_FCURVE_L63          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_L64 {
	union REG_ISP_MAP_CURVE_FCURVE_L64          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_R0 {
	union REG_ISP_MAP_CURVE_FCURVE_R0           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_R1 {
	union REG_ISP_MAP_CURVE_FCURVE_R1           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_R2 {
	union REG_ISP_MAP_CURVE_FCURVE_R2           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_R3 {
	union REG_ISP_MAP_CURVE_FCURVE_R3           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_R4 {
	union REG_ISP_MAP_CURVE_FCURVE_R4           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_R5 {
	union REG_ISP_MAP_CURVE_FCURVE_R5           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_R6 {
	union REG_ISP_MAP_CURVE_FCURVE_R6           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_R7 {
	union REG_ISP_MAP_CURVE_FCURVE_R7           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_R8 {
	union REG_ISP_MAP_CURVE_FCURVE_R8           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_R9 {
	union REG_ISP_MAP_CURVE_FCURVE_R9           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_R10 {
	union REG_ISP_MAP_CURVE_FCURVE_R10          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_R11 {
	union REG_ISP_MAP_CURVE_FCURVE_R11          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_R12 {
	union REG_ISP_MAP_CURVE_FCURVE_R12          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_R13 {
	union REG_ISP_MAP_CURVE_FCURVE_R13          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_R14 {
	union REG_ISP_MAP_CURVE_FCURVE_R14          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_R15 {
	union REG_ISP_MAP_CURVE_FCURVE_R15          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_R16 {
	union REG_ISP_MAP_CURVE_FCURVE_R16          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_END0 {
	union REG_ISP_MAP_CURVE_FCURVE_END0         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_END1 {
	union REG_ISP_MAP_CURVE_FCURVE_END1         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_END2 {
	union REG_ISP_MAP_CURVE_FCURVE_END2         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_END3 {
	union REG_ISP_MAP_CURVE_FCURVE_END3         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_END4 {
	union REG_ISP_MAP_CURVE_FCURVE_END4         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_END5 {
	union REG_ISP_MAP_CURVE_FCURVE_END5         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_END6 {
	union REG_ISP_MAP_CURVE_FCURVE_END6         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_END7 {
	union REG_ISP_MAP_CURVE_FCURVE_END7         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_END8 {
	union REG_ISP_MAP_CURVE_FCURVE_END8         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_END9 {
	union REG_ISP_MAP_CURVE_FCURVE_END9         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_END10 {
	union REG_ISP_MAP_CURVE_FCURVE_END10        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_END11 {
	union REG_ISP_MAP_CURVE_FCURVE_END11        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_END12 {
	union REG_ISP_MAP_CURVE_FCURVE_END12        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_END13 {
	union REG_ISP_MAP_CURVE_FCURVE_END13        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_END14 {
	union REG_ISP_MAP_CURVE_FCURVE_END14        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_END15 {
	union REG_ISP_MAP_CURVE_FCURVE_END15        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_FCURVE_END16 {
	union REG_ISP_MAP_CURVE_FCURVE_END16        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_MAP_CURVE_T {
	struct VREG_ISP_MAP_CURVE_01                 REG_01;
	struct VREG_ISP_MAP_CURVE_FCURVE_FLUMW_LUT0  FCURVE_FLUMW_LUT0;
	struct VREG_ISP_MAP_CURVE_FCURVE_FLUMW_LUT1  FCURVE_FLUMW_LUT1;
	struct VREG_ISP_MAP_CURVE_FCURVE_FLUMW_LUT2  FCURVE_FLUMW_LUT2;
	struct VREG_ISP_MAP_CURVE_FCURVE_FLUMW_LUT3  FCURVE_FLUMW_LUT3;
	struct VREG_ISP_MAP_CURVE_FCURVE_FLUMW_LUT4  FCURVE_FLUMW_LUT4;
	struct VREG_ISP_MAP_CURVE_FCURVE_L0          FCURVE_L0;
	struct VREG_ISP_MAP_CURVE_FCURVE_L1          FCURVE_L1;
	struct VREG_ISP_MAP_CURVE_FCURVE_L2          FCURVE_L2;
	struct VREG_ISP_MAP_CURVE_FCURVE_L3          FCURVE_L3;
	struct VREG_ISP_MAP_CURVE_FCURVE_L4          FCURVE_L4;
	struct VREG_ISP_MAP_CURVE_FCURVE_L5          FCURVE_L5;
	struct VREG_ISP_MAP_CURVE_FCURVE_L6          FCURVE_L6;
	struct VREG_ISP_MAP_CURVE_FCURVE_L7          FCURVE_L7;
	struct VREG_ISP_MAP_CURVE_FCURVE_L8          FCURVE_L8;
	struct VREG_ISP_MAP_CURVE_FCURVE_L9          FCURVE_L9;
	struct VREG_ISP_MAP_CURVE_FCURVE_L10         FCURVE_L10;
	struct VREG_ISP_MAP_CURVE_FCURVE_L11         FCURVE_L11;
	struct VREG_ISP_MAP_CURVE_FCURVE_L12         FCURVE_L12;
	struct VREG_ISP_MAP_CURVE_FCURVE_L13         FCURVE_L13;
	struct VREG_ISP_MAP_CURVE_FCURVE_L14         FCURVE_L14;
	struct VREG_ISP_MAP_CURVE_FCURVE_L15         FCURVE_L15;
	struct VREG_ISP_MAP_CURVE_FCURVE_L16         FCURVE_L16;
	struct VREG_ISP_MAP_CURVE_FCURVE_L17         FCURVE_L17;
	struct VREG_ISP_MAP_CURVE_FCURVE_L18         FCURVE_L18;
	struct VREG_ISP_MAP_CURVE_FCURVE_L19         FCURVE_L19;
	struct VREG_ISP_MAP_CURVE_FCURVE_L20         FCURVE_L20;
	struct VREG_ISP_MAP_CURVE_FCURVE_L21         FCURVE_L21;
	struct VREG_ISP_MAP_CURVE_FCURVE_L22         FCURVE_L22;
	struct VREG_ISP_MAP_CURVE_FCURVE_L23         FCURVE_L23;
	struct VREG_ISP_MAP_CURVE_FCURVE_L24         FCURVE_L24;
	struct VREG_ISP_MAP_CURVE_FCURVE_L25         FCURVE_L25;
	struct VREG_ISP_MAP_CURVE_FCURVE_L26         FCURVE_L26;
	struct VREG_ISP_MAP_CURVE_FCURVE_L27         FCURVE_L27;
	struct VREG_ISP_MAP_CURVE_FCURVE_L28         FCURVE_L28;
	struct VREG_ISP_MAP_CURVE_FCURVE_L29         FCURVE_L29;
	struct VREG_ISP_MAP_CURVE_FCURVE_L30         FCURVE_L30;
	struct VREG_ISP_MAP_CURVE_FCURVE_L31         FCURVE_L31;
	struct VREG_ISP_MAP_CURVE_FCURVE_L32         FCURVE_L32;
	struct VREG_ISP_MAP_CURVE_FCURVE_L33         FCURVE_L33;
	struct VREG_ISP_MAP_CURVE_FCURVE_L34         FCURVE_L34;
	struct VREG_ISP_MAP_CURVE_FCURVE_L35         FCURVE_L35;
	struct VREG_ISP_MAP_CURVE_FCURVE_L36         FCURVE_L36;
	struct VREG_ISP_MAP_CURVE_FCURVE_L37         FCURVE_L37;
	struct VREG_ISP_MAP_CURVE_FCURVE_L38         FCURVE_L38;
	struct VREG_ISP_MAP_CURVE_FCURVE_L39         FCURVE_L39;
	struct VREG_ISP_MAP_CURVE_FCURVE_L40         FCURVE_L40;
	struct VREG_ISP_MAP_CURVE_FCURVE_L41         FCURVE_L41;
	struct VREG_ISP_MAP_CURVE_FCURVE_L42         FCURVE_L42;
	struct VREG_ISP_MAP_CURVE_FCURVE_L43         FCURVE_L43;
	struct VREG_ISP_MAP_CURVE_FCURVE_L44         FCURVE_L44;
	struct VREG_ISP_MAP_CURVE_FCURVE_L45         FCURVE_L45;
	struct VREG_ISP_MAP_CURVE_FCURVE_L46         FCURVE_L46;
	struct VREG_ISP_MAP_CURVE_FCURVE_L47         FCURVE_L47;
	struct VREG_ISP_MAP_CURVE_FCURVE_L48         FCURVE_L48;
	struct VREG_ISP_MAP_CURVE_FCURVE_L49         FCURVE_L49;
	struct VREG_ISP_MAP_CURVE_FCURVE_L50         FCURVE_L50;
	struct VREG_ISP_MAP_CURVE_FCURVE_L51         FCURVE_L51;
	struct VREG_ISP_MAP_CURVE_FCURVE_L52         FCURVE_L52;
	struct VREG_ISP_MAP_CURVE_FCURVE_L53         FCURVE_L53;
	struct VREG_ISP_MAP_CURVE_FCURVE_L54         FCURVE_L54;
	struct VREG_ISP_MAP_CURVE_FCURVE_L55         FCURVE_L55;
	struct VREG_ISP_MAP_CURVE_FCURVE_L56         FCURVE_L56;
	struct VREG_ISP_MAP_CURVE_FCURVE_L57         FCURVE_L57;
	struct VREG_ISP_MAP_CURVE_FCURVE_L58         FCURVE_L58;
	struct VREG_ISP_MAP_CURVE_FCURVE_L59         FCURVE_L59;
	struct VREG_ISP_MAP_CURVE_FCURVE_L60         FCURVE_L60;
	struct VREG_ISP_MAP_CURVE_FCURVE_L61         FCURVE_L61;
	struct VREG_ISP_MAP_CURVE_FCURVE_L62         FCURVE_L62;
	struct VREG_ISP_MAP_CURVE_FCURVE_L63         FCURVE_L63;
	struct VREG_ISP_MAP_CURVE_FCURVE_L64         FCURVE_L64;
	struct VREG_ISP_MAP_CURVE_FCURVE_R0          FCURVE_R0;
	struct VREG_ISP_MAP_CURVE_FCURVE_R1          FCURVE_R1;
	struct VREG_ISP_MAP_CURVE_FCURVE_R2          FCURVE_R2;
	struct VREG_ISP_MAP_CURVE_FCURVE_R3          FCURVE_R3;
	struct VREG_ISP_MAP_CURVE_FCURVE_R4          FCURVE_R4;
	struct VREG_ISP_MAP_CURVE_FCURVE_R5          FCURVE_R5;
	struct VREG_ISP_MAP_CURVE_FCURVE_R6          FCURVE_R6;
	struct VREG_ISP_MAP_CURVE_FCURVE_R7          FCURVE_R7;
	struct VREG_ISP_MAP_CURVE_FCURVE_R8          FCURVE_R8;
	struct VREG_ISP_MAP_CURVE_FCURVE_R9          FCURVE_R9;
	struct VREG_ISP_MAP_CURVE_FCURVE_R10         FCURVE_R10;
	struct VREG_ISP_MAP_CURVE_FCURVE_R11         FCURVE_R11;
	struct VREG_ISP_MAP_CURVE_FCURVE_R12         FCURVE_R12;
	struct VREG_ISP_MAP_CURVE_FCURVE_R13         FCURVE_R13;
	struct VREG_ISP_MAP_CURVE_FCURVE_R14         FCURVE_R14;
	struct VREG_ISP_MAP_CURVE_FCURVE_R15         FCURVE_R15;
	struct VREG_ISP_MAP_CURVE_FCURVE_R16         FCURVE_R16;
	struct VREG_ISP_MAP_CURVE_FCURVE_END0        FCURVE_END0;
	struct VREG_ISP_MAP_CURVE_FCURVE_END1        FCURVE_END1;
	struct VREG_ISP_MAP_CURVE_FCURVE_END2        FCURVE_END2;
	struct VREG_ISP_MAP_CURVE_FCURVE_END3        FCURVE_END3;
	struct VREG_ISP_MAP_CURVE_FCURVE_END4        FCURVE_END4;
	struct VREG_ISP_MAP_CURVE_FCURVE_END5        FCURVE_END5;
	struct VREG_ISP_MAP_CURVE_FCURVE_END6        FCURVE_END6;
	struct VREG_ISP_MAP_CURVE_FCURVE_END7        FCURVE_END7;
	struct VREG_ISP_MAP_CURVE_FCURVE_END8        FCURVE_END8;
	struct VREG_ISP_MAP_CURVE_FCURVE_END9        FCURVE_END9;
	struct VREG_ISP_MAP_CURVE_FCURVE_END10       FCURVE_END10;
	struct VREG_ISP_MAP_CURVE_FCURVE_END11       FCURVE_END11;
	struct VREG_ISP_MAP_CURVE_FCURVE_END12       FCURVE_END12;
	struct VREG_ISP_MAP_CURVE_FCURVE_END13       FCURVE_END13;
	struct VREG_ISP_MAP_CURVE_FCURVE_END14       FCURVE_END14;
	struct VREG_ISP_MAP_CURVE_FCURVE_END15       FCURVE_END15;
	struct VREG_ISP_MAP_CURVE_FCURVE_END16       FCURVE_END16;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_DPC_BASE_CONFIG {
	union REG_ISP_DPC_BASE_CONFIG               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DPC_DARK_THRESHOLD0 {
	union REG_ISP_DPC_DARK_THRESHOLD0           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DPC_DARK_THRESHOLD1 {
	union REG_ISP_DPC_DARK_THRESHOLD1           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DPC_DARK_THRESHOLD2 {
	union REG_ISP_DPC_DARK_THRESHOLD2           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DPC_BRIGHT_THRESHOLD0 {
	union REG_ISP_DPC_BRIGHT_THRESHOLD0         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DPC_BRIGHT_THRESHOLD1 {
	union REG_ISP_DPC_BRIGHT_THRESHOLD1         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DPC_BRIGHT_THRESHOLD2 {
	union REG_ISP_DPC_BRIGHT_THRESHOLD2         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DPC_THRESHOLD_OFFSET {
	union REG_ISP_DPC_THRESHOLD_OFFSET          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DPC_OUTLIER_CNT {
	union REG_ISP_DPC_OUTLIER_CNT               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DPC_OUTLIER_CONFIG {
	union REG_ISP_DPC_OUTLIER_CONFIG            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DPC_GE_CONFIG {
	union REG_ISP_DPC_GE_CONFIG                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DPC_GE_THRESHOLD0 {
	union REG_ISP_DPC_GE_THRESHOLD0             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DPC_GE_THRESHOLD1 {
	union REG_ISP_DPC_GE_THRESHOLD1             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DPC_GE_THRESHOLD1_0 {
	union REG_ISP_DPC_GE_THRESHOLD1_0           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DPC_GE_THRESHOLD1_1 {
	union REG_ISP_DPC_GE_THRESHOLD1_1           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DPC_GE_THRESHOLD2_0 {
	union REG_ISP_DPC_GE_THRESHOLD2_0           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DPC_GE_THRESHOLD2_1 {
	union REG_ISP_DPC_GE_THRESHOLD2_1           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DPC_SHDW_READ_SEL {
	union REG_ISP_DPC_SHDW_READ_SEL             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DPC_SPC_CONFIG {
	union REG_ISP_DPC_SPC_CONFIG                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DPC_SW_READ_0 {
	union REG_ISP_DPC_SW_READ_0                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DPC_WRITE_MEM {
	union REG_ISP_DPC_WRITE_MEM                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DPC_WRITE_MEM_ST_ADDR {
	union REG_ISP_DPC_WRITE_MEM_ST_ADDR         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DPC_AUTO_CG_EN {
	union REG_ISP_DPC_AUTO_CG_EN                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DPC_SW_READ_1 {
	union REG_ISP_DPC_SW_READ_1                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DPC_T {
	struct VREG_ISP_DPC_BASE_CONFIG              BASE_CONFIG;
	struct VREG_ISP_DPC_DARK_THRESHOLD0          DARK_THRESHOLD0;
	struct VREG_ISP_DPC_DARK_THRESHOLD1          DARK_THRESHOLD1;
	struct VREG_ISP_DPC_DARK_THRESHOLD2          DARK_THRESHOLD2;
	struct VREG_ISP_DPC_BRIGHT_THRESHOLD0        BRIGHT_THRESHOLD0;
	struct VREG_ISP_DPC_BRIGHT_THRESHOLD1        BRIGHT_THRESHOLD1;
	struct VREG_ISP_DPC_BRIGHT_THRESHOLD2        BRIGHT_THRESHOLD2;
	struct VREG_ISP_DPC_THRESHOLD_OFFSET         THRESHOLD_OFFSET;
	struct VREG_ISP_DPC_OUTLIER_CNT              OUTLIER_CNT;
	struct VREG_ISP_DPC_OUTLIER_CONFIG           OUTLIER_CONFIG;
	struct VREG_ISP_DPC_GE_CONFIG                GE_CONFIG;
	struct VREG_ISP_DPC_GE_THRESHOLD0            GE_THRESHOLD0;
	struct VREG_ISP_DPC_GE_THRESHOLD1            GE_THRESHOLD1;
	struct VREG_ISP_DPC_GE_THRESHOLD1_0          GE_THRESHOLD1_0;
	struct VREG_ISP_DPC_GE_THRESHOLD1_1          GE_THRESHOLD1_1;
	struct VREG_ISP_DPC_GE_THRESHOLD2_0          GE_THRESHOLD2_0;
	struct VREG_ISP_DPC_GE_THRESHOLD2_1          GE_THRESHOLD2_1;
	struct VREG_ISP_DPC_SHDW_READ_SEL            SHDW_READ_SEL;
	struct VREG_ISP_DPC_SPC_CONFIG               SPC_CONFIG;
	struct VREG_ISP_DPC_SW_READ_0                SW_READ_0;
	struct VREG_ISP_DPC_WRITE_MEM                WRITE_MEM;
	struct VREG_ISP_DPC_WRITE_MEM_ST_ADDR        WRITE_MEM_ST_ADDR;
	struct VREG_ISP_DPC_AUTO_CG_EN               AUTO_CG_EN;
	struct VREG_ISP_DPC_SW_READ_1                SW_READ_1;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_BNR_00 {
	union REG_ISP_BNR_00                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_04 {
	union REG_ISP_BNR_04                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_10 {
	union REG_ISP_BNR_10                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_14 {
	union REG_ISP_BNR_14                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_18 {
	union REG_ISP_BNR_18                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_1C {
	union REG_ISP_BNR_1C                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_20 {
	union REG_ISP_BNR_20                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_24 {
	union REG_ISP_BNR_24                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_28 {
	union REG_ISP_BNR_28                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_2C {
	union REG_ISP_BNR_2C                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_30 {
	union REG_ISP_BNR_30                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_34 {
	union REG_ISP_BNR_34                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_38 {
	union REG_ISP_BNR_38                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_3C {
	union REG_ISP_BNR_3C                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_40 {
	union REG_ISP_BNR_40                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_44 {
	union REG_ISP_BNR_44                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_48 {
	union REG_ISP_BNR_48                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_4C {
	union REG_ISP_BNR_4C                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_50 {
	union REG_ISP_BNR_50                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_54 {
	union REG_ISP_BNR_54                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_58 {
	union REG_ISP_BNR_58                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_5C {
	union REG_ISP_BNR_5C                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_60 {
	union REG_ISP_BNR_60                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_64 {
	union REG_ISP_BNR_64                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_68 {
	union REG_ISP_BNR_68                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_6C {
	union REG_ISP_BNR_6C                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_70 {
	union REG_ISP_BNR_70                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_74 {
	union REG_ISP_BNR_74                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_78 {
	union REG_ISP_BNR_78                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_7C {
	union REG_ISP_BNR_7C                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_80 {
	union REG_ISP_BNR_80                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_84 {
	union REG_ISP_BNR_84                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_88 {
	union REG_ISP_BNR_88                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_8C {
	union REG_ISP_BNR_8C                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_90 {
	union REG_ISP_BNR_90                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_94 {
	union REG_ISP_BNR_94                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_98 {
	union REG_ISP_BNR_98                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_9C {
	union REG_ISP_BNR_9C                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_A0 {
	union REG_ISP_BNR_A0                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_A4 {
	union REG_ISP_BNR_A4                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_A8 {
	union REG_ISP_BNR_A8                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_AC {
	union REG_ISP_BNR_AC                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_B0 {
	union REG_ISP_BNR_B0                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_B4 {
	union REG_ISP_BNR_B4                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_B8 {
	union REG_ISP_BNR_B8                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_BC {
	union REG_ISP_BNR_BC                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_C0 {
	union REG_ISP_BNR_C0                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_C4 {
	union REG_ISP_BNR_C4                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_C8 {
	union REG_ISP_BNR_C8                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_CC {
	union REG_ISP_BNR_CC                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_D0 {
	union REG_ISP_BNR_D0                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_D4 {
	union REG_ISP_BNR_D4                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_D8 {
	union REG_ISP_BNR_D8                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_DC {
	union REG_ISP_BNR_DC                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_E0 {
	union REG_ISP_BNR_E0                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_E4 {
	union REG_ISP_BNR_E4                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_E8 {
	union REG_ISP_BNR_E8                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_EC {
	union REG_ISP_BNR_EC                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_F0 {
	union REG_ISP_BNR_F0                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_F4 {
	union REG_ISP_BNR_F4                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_F8 {
	union REG_ISP_BNR_F8                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_FC {
	union REG_ISP_BNR_FC                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_100 {
	union REG_ISP_BNR_100                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_104 {
	union REG_ISP_BNR_104                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_108 {
	union REG_ISP_BNR_108                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_10C {
	union REG_ISP_BNR_10C                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_110 {
	union REG_ISP_BNR_110                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_114 {
	union REG_ISP_BNR_114                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_118 {
	union REG_ISP_BNR_118                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_11C {
	union REG_ISP_BNR_11C                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_120 {
	union REG_ISP_BNR_120                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_124 {
	union REG_ISP_BNR_124                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_128 {
	union REG_ISP_BNR_128                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_12C {
	union REG_ISP_BNR_12C                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_130 {
	union REG_ISP_BNR_130                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_134 {
	union REG_ISP_BNR_134                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_138 {
	union REG_ISP_BNR_138                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_13C {
	union REG_ISP_BNR_13C                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_140 {
	union REG_ISP_BNR_140                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_144 {
	union REG_ISP_BNR_144                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_148 {
	union REG_ISP_BNR_148                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_14C {
	union REG_ISP_BNR_14C                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_150 {
	union REG_ISP_BNR_150                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_154 {
	union REG_ISP_BNR_154                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_158 {
	union REG_ISP_BNR_158                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_15C {
	union REG_ISP_BNR_15C                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_160 {
	union REG_ISP_BNR_160                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_164 {
	union REG_ISP_BNR_164                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_168 {
	union REG_ISP_BNR_168                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_16C {
	union REG_ISP_BNR_16C                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_170 {
	union REG_ISP_BNR_170                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_174 {
	union REG_ISP_BNR_174                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_178 {
	union REG_ISP_BNR_178                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_17C {
	union REG_ISP_BNR_17C                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_180 {
	union REG_ISP_BNR_180                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_184 {
	union REG_ISP_BNR_184                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_188 {
	union REG_ISP_BNR_188                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_18C {
	union REG_ISP_BNR_18C                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_190 {
	union REG_ISP_BNR_190                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_194 {
	union REG_ISP_BNR_194                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_198 {
	union REG_ISP_BNR_198                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_19C {
	union REG_ISP_BNR_19C                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_1A0 {
	union REG_ISP_BNR_1A0                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_BNR_T {
	struct VREG_ISP_BNR_00                       REG_00;
	struct VREG_ISP_BNR_04                       REG_04;
	struct VREG_RESV                        _resv_0x8[2];
	struct VREG_ISP_BNR_10                       REG_10;
	struct VREG_ISP_BNR_14                       REG_14;
	struct VREG_ISP_BNR_18                       REG_18;
	struct VREG_ISP_BNR_1C                       REG_1C;
	struct VREG_ISP_BNR_20                       REG_20;
	struct VREG_ISP_BNR_24                       REG_24;
	struct VREG_ISP_BNR_28                       REG_28;
	struct VREG_ISP_BNR_2C                       REG_2C;
	struct VREG_ISP_BNR_30                       REG_30;
	struct VREG_ISP_BNR_34                       REG_34;
	struct VREG_ISP_BNR_38                       REG_38;
	struct VREG_ISP_BNR_3C                       REG_3C;
	struct VREG_ISP_BNR_40                       REG_40;
	struct VREG_ISP_BNR_44                       REG_44;
	struct VREG_ISP_BNR_48                       REG_48;
	struct VREG_ISP_BNR_4C                       REG_4C;
	struct VREG_ISP_BNR_50                       REG_50;
	struct VREG_ISP_BNR_54                       REG_54;
	struct VREG_ISP_BNR_58                       REG_58;
	struct VREG_ISP_BNR_5C                       REG_5C;
	struct VREG_ISP_BNR_60                       REG_60;
	struct VREG_ISP_BNR_64                       REG_64;
	struct VREG_ISP_BNR_68                       REG_68;
	struct VREG_ISP_BNR_6C                       REG_6C;
	struct VREG_ISP_BNR_70                       REG_70;
	struct VREG_ISP_BNR_74                       REG_74;
	struct VREG_ISP_BNR_78                       REG_78;
	struct VREG_ISP_BNR_7C                       REG_7C;
	struct VREG_ISP_BNR_80                       REG_80;
	struct VREG_ISP_BNR_84                       REG_84;
	struct VREG_ISP_BNR_88                       REG_88;
	struct VREG_ISP_BNR_8C                       REG_8C;
	struct VREG_ISP_BNR_90                       REG_90;
	struct VREG_ISP_BNR_94                       REG_94;
	struct VREG_ISP_BNR_98                       REG_98;
	struct VREG_ISP_BNR_9C                       REG_9C;
	struct VREG_ISP_BNR_A0                       REG_A0;
	struct VREG_ISP_BNR_A4                       REG_A4;
	struct VREG_ISP_BNR_A8                       REG_A8;
	struct VREG_ISP_BNR_AC                       REG_AC;
	struct VREG_ISP_BNR_B0                       REG_B0;
	struct VREG_ISP_BNR_B4                       REG_B4;
	struct VREG_ISP_BNR_B8                       REG_B8;
	struct VREG_ISP_BNR_BC                       REG_BC;
	struct VREG_ISP_BNR_C0                       REG_C0;
	struct VREG_ISP_BNR_C4                       REG_C4;
	struct VREG_ISP_BNR_C8                       REG_C8;
	struct VREG_ISP_BNR_CC                       REG_CC;
	struct VREG_ISP_BNR_D0                       REG_D0;
	struct VREG_ISP_BNR_D4                       REG_D4;
	struct VREG_ISP_BNR_D8                       REG_D8;
	struct VREG_ISP_BNR_DC                       REG_DC;
	struct VREG_ISP_BNR_E0                       REG_E0;
	struct VREG_ISP_BNR_E4                       REG_E4;
	struct VREG_ISP_BNR_E8                       REG_E8;
	struct VREG_ISP_BNR_EC                       REG_EC;
	struct VREG_ISP_BNR_F0                       REG_F0;
	struct VREG_ISP_BNR_F4                       REG_F4;
	struct VREG_ISP_BNR_F8                       REG_F8;
	struct VREG_ISP_BNR_FC                       REG_FC;
	struct VREG_ISP_BNR_100                      REG_100;
	struct VREG_ISP_BNR_104                      REG_104;
	struct VREG_ISP_BNR_108                      REG_108;
	struct VREG_ISP_BNR_10C                      REG_10C;
	struct VREG_ISP_BNR_110                      REG_110;
	struct VREG_ISP_BNR_114                      REG_114;
	struct VREG_ISP_BNR_118                      REG_118;
	struct VREG_ISP_BNR_11C                      REG_11C;
	struct VREG_ISP_BNR_120                      REG_120;
	struct VREG_ISP_BNR_124                      REG_124;
	struct VREG_ISP_BNR_128                      REG_128;
	struct VREG_ISP_BNR_12C                      REG_12C;
	struct VREG_ISP_BNR_130                      REG_130;
	struct VREG_ISP_BNR_134                      REG_134;
	struct VREG_ISP_BNR_138                      REG_138;
	struct VREG_ISP_BNR_13C                      REG_13C;
	struct VREG_ISP_BNR_140                      REG_140;
	struct VREG_ISP_BNR_144                      REG_144;
	struct VREG_ISP_BNR_148                      REG_148;
	struct VREG_ISP_BNR_14C                      REG_14C;
	struct VREG_ISP_BNR_150                      REG_150;
	struct VREG_ISP_BNR_154                      REG_154;
	struct VREG_ISP_BNR_158                      REG_158;
	struct VREG_ISP_BNR_15C                      REG_15C;
	struct VREG_ISP_BNR_160                      REG_160;
	struct VREG_ISP_BNR_164                      REG_164;
	struct VREG_ISP_BNR_168                      REG_168;
	struct VREG_ISP_BNR_16C                      REG_16C;
	struct VREG_ISP_BNR_170                      REG_170;
	struct VREG_ISP_BNR_174                      REG_174;
	struct VREG_ISP_BNR_178                      REG_178;
	struct VREG_ISP_BNR_17C                      REG_17C;
	struct VREG_ISP_BNR_180                      REG_180;
	struct VREG_ISP_BNR_184                      REG_184;
	struct VREG_ISP_BNR_188                      REG_188;
	struct VREG_ISP_BNR_18C                      REG_18C;
	struct VREG_ISP_BNR_190                      REG_190;
	struct VREG_ISP_BNR_194                      REG_194;
	struct VREG_ISP_BNR_198                      REG_198;
	struct VREG_ISP_BNR_19C                      REG_19C;
	struct VREG_ISP_BNR_1A0                      REG_1A0;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_LSCR_SC_WRAP_0 {
	union REG_ISP_LSCR_SC_WRAP_0            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_SC_WRAP_1 {
	union REG_ISP_LSCR_SC_WRAP_1            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_SC_WRAP_2 {
	union REG_ISP_LSCR_SC_WRAP_2            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_SC_WRAP_3 {
	union REG_ISP_LSCR_SC_WRAP_3            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_SC_WRAP_4 {
	union REG_ISP_LSCR_SC_WRAP_4            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_SC_WRAP_5 {
	union REG_ISP_LSCR_SC_WRAP_5            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_SC_WRAP_6 {
	union REG_ISP_LSCR_SC_WRAP_6            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_SC_WRAP_7 {
	union REG_ISP_LSCR_SC_WRAP_7            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_SC_WRAP_8 {
	union REG_ISP_LSCR_SC_WRAP_8            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_SC_WRAP_9 {
	union REG_ISP_LSCR_SC_WRAP_9            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_SC_WRAP_10 {
	union REG_ISP_LSCR_SC_WRAP_10           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_SC_WRAP_11 {
	union REG_ISP_LSCR_SC_WRAP_11           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_SC_WRAP_12 {
	union REG_ISP_LSCR_SC_WRAP_12           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_SC_WRAP_13 {
	union REG_ISP_LSCR_SC_WRAP_13           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_SC_WRAP_14 {
	union REG_ISP_LSCR_SC_WRAP_14           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_SC_WRAP_15 {
	union REG_ISP_LSCR_SC_WRAP_15           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_SC_WRAP_16 {
	union REG_ISP_LSCR_SC_WRAP_16           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_SC_WRAP_17 {
	union REG_ISP_LSCR_SC_WRAP_17           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_SC_WRAP_18 {
	union REG_ISP_LSCR_SC_WRAP_18           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_SC_WRAP_19 {
	union REG_ISP_LSCR_SC_WRAP_19           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_SC_WRAP_20 {
	union REG_ISP_LSCR_SC_WRAP_20           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_SC_WRAP_21 {
	union REG_ISP_LSCR_SC_WRAP_21           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_SC_WRAP_22 {
	union REG_ISP_LSCR_SC_WRAP_22           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_SC_WRAP_23 {
	union REG_ISP_LSCR_SC_WRAP_23           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_SC_WRAP_24 {
	union REG_ISP_LSCR_SC_WRAP_24           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_SC_WRAP_25 {
	union REG_ISP_LSCR_SC_WRAP_25           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_SC_WRAP_26 {
	union REG_ISP_LSCR_SC_WRAP_26           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LSCR_T {
	struct VREG_ISP_LSCR_SC_WRAP_0           SC_WRAP_0;
	struct VREG_ISP_LSCR_SC_WRAP_1           SC_WRAP_1;
	struct VREG_ISP_LSCR_SC_WRAP_2           SC_WRAP_2;
	struct VREG_ISP_LSCR_SC_WRAP_3           SC_WRAP_3;
	struct VREG_ISP_LSCR_SC_WRAP_4           SC_WRAP_4;
	struct VREG_ISP_LSCR_SC_WRAP_5           SC_WRAP_5;
	struct VREG_ISP_LSCR_SC_WRAP_6           SC_WRAP_6;
	struct VREG_ISP_LSCR_SC_WRAP_7           SC_WRAP_7;
	struct VREG_ISP_LSCR_SC_WRAP_8           SC_WRAP_8;
	struct VREG_ISP_LSCR_SC_WRAP_9           SC_WRAP_9;
	struct VREG_ISP_LSCR_SC_WRAP_10          SC_WRAP_10;
	struct VREG_ISP_LSCR_SC_WRAP_11          SC_WRAP_11;
	struct VREG_ISP_LSCR_SC_WRAP_12          SC_WRAP_12;
	struct VREG_ISP_LSCR_SC_WRAP_13          SC_WRAP_13;
	struct VREG_ISP_LSCR_SC_WRAP_14          SC_WRAP_14;
	struct VREG_ISP_LSCR_SC_WRAP_15          SC_WRAP_15;
	struct VREG_ISP_LSCR_SC_WRAP_16          SC_WRAP_16;
	struct VREG_ISP_LSCR_SC_WRAP_17          SC_WRAP_17;
	struct VREG_ISP_LSCR_SC_WRAP_18          SC_WRAP_18;
	struct VREG_ISP_LSCR_SC_WRAP_19          SC_WRAP_19;
	struct VREG_ISP_LSCR_SC_WRAP_20          SC_WRAP_20;
	struct VREG_ISP_LSCR_SC_WRAP_21          SC_WRAP_21;
	struct VREG_ISP_LSCR_SC_WRAP_22          SC_WRAP_22;
	struct VREG_ISP_LSCR_SC_WRAP_23          SC_WRAP_23;
	struct VREG_ISP_LSCR_SC_WRAP_24          SC_WRAP_24;
	struct VREG_ISP_LSCR_SC_WRAP_25          SC_WRAP_25;
	struct VREG_ISP_LSCR_SC_WRAP_26          SC_WRAP_26;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_DRC_FRAME_DONE {
	union REG_ISP_DRC_FRAME_DONE                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_ENABLE {
	union REG_ISP_DRC_ENABLE                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_BLK_NUM {
	union REG_ISP_DRC_BLK_NUM                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_BLK_SIZE {
	union REG_ISP_DRC_BLK_SIZE                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_BLK_DIV {
	union REG_ISP_DRC_BLK_DIV                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LPF_MODE {
	union REG_ISP_DRC_LPF_MODE                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_HIST {
	union REG_ISP_DRC_HIST                      write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_HIST_STEP {
	union REG_ISP_DRC_HIST_STEP                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_INTENSIT_MODE {
	union REG_ISP_DRC_INTENSIT_MODE             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_HORI_LPF_TH12 {
	union REG_ISP_DRC_HORI_LPF_TH12             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_HORI_LPF_TH34 {
	union REG_ISP_DRC_HORI_LPF_TH34             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_HORI_LPF_TH56 {
	union REG_ISP_DRC_HORI_LPF_TH56             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_Q_COEFF12 {
	union REG_ISP_DRC_Q_COEFF12                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_Q_COEFF34 {
	union REG_ISP_DRC_Q_COEFF34                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_B2Q_TH {
	union REG_ISP_DRC_B2Q_TH                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_FBC {
	union REG_ISP_DRC_FBC                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_ANTI_HALO {
	union REG_ISP_DRC_ANTI_HALO                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_STRENGTH {
	union REG_ISP_DRC_STRENGTH                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_EN {
	union REG_ISP_DRC_LTM_EN                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GAIN {
	union REG_ISP_DRC_GAIN                      write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_SAT_TH {
	union REG_ISP_DRC_SAT_TH                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_SAT_DELTA {
	union REG_ISP_DRC_SAT_DELTA                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_YV_BLD_LUT0_4 {
	union REG_ISP_DRC_YV_BLD_LUT0_4             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_YV_BLD_LUT5_8 {
	union REG_ISP_DRC_YV_BLD_LUT5_8             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_INPUT_BLD_LUT_0_3 {
	union REG_ISP_DRC_INPUT_BLD_LUT_0_3         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_INPUT_BLD_LUT_4_7 {
	union REG_ISP_DRC_INPUT_BLD_LUT_4_7         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_INPUT_BLD_LUT_8_11 {
	union REG_ISP_DRC_INPUT_BLD_LUT_8_11        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_INPUT_BLD_LUT_12_15 {
	union REG_ISP_DRC_INPUT_BLD_LUT_12_15       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_INPUT_BLD_LUT_16 {
	union REG_ISP_DRC_INPUT_BLD_LUT_16          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_0_1 {
	union REG_ISP_DRC_GTM_L_LUT_0_1             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_2_3 {
	union REG_ISP_DRC_GTM_L_LUT_2_3             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_4_5 {
	union REG_ISP_DRC_GTM_L_LUT_4_5             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_6_7 {
	union REG_ISP_DRC_GTM_L_LUT_6_7             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_8_9 {
	union REG_ISP_DRC_GTM_L_LUT_8_9             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_10_11 {
	union REG_ISP_DRC_GTM_L_LUT_10_11           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_12_13 {
	union REG_ISP_DRC_GTM_L_LUT_12_13           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_14_15 {
	union REG_ISP_DRC_GTM_L_LUT_14_15           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_16_17 {
	union REG_ISP_DRC_GTM_L_LUT_16_17           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_18_19 {
	union REG_ISP_DRC_GTM_L_LUT_18_19           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_20_21 {
	union REG_ISP_DRC_GTM_L_LUT_20_21           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_22_23 {
	union REG_ISP_DRC_GTM_L_LUT_22_23           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_24_25 {
	union REG_ISP_DRC_GTM_L_LUT_24_25           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_26_27 {
	union REG_ISP_DRC_GTM_L_LUT_26_27           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_28_29 {
	union REG_ISP_DRC_GTM_L_LUT_28_29           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_30_31 {
	union REG_ISP_DRC_GTM_L_LUT_30_31           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_32_33 {
	union REG_ISP_DRC_GTM_L_LUT_32_33           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_34_35 {
	union REG_ISP_DRC_GTM_L_LUT_34_35           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_36_37 {
	union REG_ISP_DRC_GTM_L_LUT_36_37           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_38_39 {
	union REG_ISP_DRC_GTM_L_LUT_38_39           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_40_41 {
	union REG_ISP_DRC_GTM_L_LUT_40_41           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_42_43 {
	union REG_ISP_DRC_GTM_L_LUT_42_43           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_44_45 {
	union REG_ISP_DRC_GTM_L_LUT_44_45           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_46_47 {
	union REG_ISP_DRC_GTM_L_LUT_46_47           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_48_49 {
	union REG_ISP_DRC_GTM_L_LUT_48_49           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_50_51 {
	union REG_ISP_DRC_GTM_L_LUT_50_51           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_52_53 {
	union REG_ISP_DRC_GTM_L_LUT_52_53           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_54_55 {
	union REG_ISP_DRC_GTM_L_LUT_54_55           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_56_57 {
	union REG_ISP_DRC_GTM_L_LUT_56_57           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_58_59 {
	union REG_ISP_DRC_GTM_L_LUT_58_59           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_60_61 {
	union REG_ISP_DRC_GTM_L_LUT_60_61           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_62_63 {
	union REG_ISP_DRC_GTM_L_LUT_62_63           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_L_LUT_64 {
	union REG_ISP_DRC_GTM_L_LUT_64              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_R_LUT_0_1 {
	union REG_ISP_DRC_GTM_R_LUT_0_1             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_R_LUT_2_3 {
	union REG_ISP_DRC_GTM_R_LUT_2_3             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_R_LUT_4_5 {
	union REG_ISP_DRC_GTM_R_LUT_4_5             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_R_LUT_6_7 {
	union REG_ISP_DRC_GTM_R_LUT_6_7             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_R_LUT_8_9 {
	union REG_ISP_DRC_GTM_R_LUT_8_9             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_R_LUT_10_11 {
	union REG_ISP_DRC_GTM_R_LUT_10_11           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_R_LUT_12_13 {
	union REG_ISP_DRC_GTM_R_LUT_12_13           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_R_LUT_14_15 {
	union REG_ISP_DRC_GTM_R_LUT_14_15           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_GTM_R_LUT_16 {
	union REG_ISP_DRC_GTM_R_LUT_16              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LUMA_PROT_LUT_0_1 {
	union REG_ISP_DRC_LUMA_PROT_LUT_0_1         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LUMA_PROT_LUT_2_3 {
	union REG_ISP_DRC_LUMA_PROT_LUT_2_3         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LUMA_PROT_LUT_4_5 {
	union REG_ISP_DRC_LUMA_PROT_LUT_4_5         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LUMA_PROT_LUT_6_7 {
	union REG_ISP_DRC_LUMA_PROT_LUT_6_7         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LUMA_PROT_LUT_8_9 {
	union REG_ISP_DRC_LUMA_PROT_LUT_8_9         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LUMA_PROT_LUT_10_11 {
	union REG_ISP_DRC_LUMA_PROT_LUT_10_11       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LUMA_PROT_LUT_12_13 {
	union REG_ISP_DRC_LUMA_PROT_LUT_12_13       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LUMA_PROT_LUT_14_15 {
	union REG_ISP_DRC_LUMA_PROT_LUT_14_15       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LUMA_PROT_LUT_16 {
	union REG_ISP_DRC_LUMA_PROT_LUT_16          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_0_1 {
	union REG_ISP_DRC_LTM_L_LUT_0_1             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_2_3 {
	union REG_ISP_DRC_LTM_L_LUT_2_3             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_4_5 {
	union REG_ISP_DRC_LTM_L_LUT_4_5             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_6_7 {
	union REG_ISP_DRC_LTM_L_LUT_6_7             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_8_9 {
	union REG_ISP_DRC_LTM_L_LUT_8_9             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_10_11 {
	union REG_ISP_DRC_LTM_L_LUT_10_11           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_12_13 {
	union REG_ISP_DRC_LTM_L_LUT_12_13           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_14_15 {
	union REG_ISP_DRC_LTM_L_LUT_14_15           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_16_17 {
	union REG_ISP_DRC_LTM_L_LUT_16_17           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_18_19 {
	union REG_ISP_DRC_LTM_L_LUT_18_19           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_20_21 {
	union REG_ISP_DRC_LTM_L_LUT_20_21           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_22_23 {
	union REG_ISP_DRC_LTM_L_LUT_22_23           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_24_25 {
	union REG_ISP_DRC_LTM_L_LUT_24_25           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_26_27 {
	union REG_ISP_DRC_LTM_L_LUT_26_27           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_28_29 {
	union REG_ISP_DRC_LTM_L_LUT_28_29           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_30_31 {
	union REG_ISP_DRC_LTM_L_LUT_30_31           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_32_33 {
	union REG_ISP_DRC_LTM_L_LUT_32_33           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_34_35 {
	union REG_ISP_DRC_LTM_L_LUT_34_35           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_36_37 {
	union REG_ISP_DRC_LTM_L_LUT_36_37           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_38_39 {
	union REG_ISP_DRC_LTM_L_LUT_38_39           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_40_41 {
	union REG_ISP_DRC_LTM_L_LUT_40_41           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_42_43 {
	union REG_ISP_DRC_LTM_L_LUT_42_43           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_44_45 {
	union REG_ISP_DRC_LTM_L_LUT_44_45           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_46_47 {
	union REG_ISP_DRC_LTM_L_LUT_46_47           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_48_49 {
	union REG_ISP_DRC_LTM_L_LUT_48_49           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_50_51 {
	union REG_ISP_DRC_LTM_L_LUT_50_51           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_52_53 {
	union REG_ISP_DRC_LTM_L_LUT_52_53           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_54_55 {
	union REG_ISP_DRC_LTM_L_LUT_54_55           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_56_57 {
	union REG_ISP_DRC_LTM_L_LUT_56_57           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_58_59 {
	union REG_ISP_DRC_LTM_L_LUT_58_59           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_60_61 {
	union REG_ISP_DRC_LTM_L_LUT_60_61           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_62_63 {
	union REG_ISP_DRC_LTM_L_LUT_62_63           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_L_LUT_64 {
	union REG_ISP_DRC_LTM_L_LUT_64              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_R_LUT_0_1 {
	union REG_ISP_DRC_LTM_R_LUT_0_1             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_R_LUT_2_3 {
	union REG_ISP_DRC_LTM_R_LUT_2_3             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_R_LUT_4_5 {
	union REG_ISP_DRC_LTM_R_LUT_4_5             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_R_LUT_6_7 {
	union REG_ISP_DRC_LTM_R_LUT_6_7             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_R_LUT_8_9 {
	union REG_ISP_DRC_LTM_R_LUT_8_9             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_R_LUT_10_11 {
	union REG_ISP_DRC_LTM_R_LUT_10_11           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_R_LUT_12_13 {
	union REG_ISP_DRC_LTM_R_LUT_12_13           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_R_LUT_14_15 {
	union REG_ISP_DRC_LTM_R_LUT_14_15           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_LTM_R_LUT_16 {
	union REG_ISP_DRC_LTM_R_LUT_16              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_IMG_SIZE {
	union REG_ISP_DRC_IMG_SIZE                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_SHDW_SEL {
	union REG_ISP_DRC_SHDW_SEL                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_PIPECTRL_CG {
	union REG_ISP_DRC_PIPECTRL_CG               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_BAYERID {
	union REG_ISP_DRC_BAYERID                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DRC_T {
	struct VREG_ISP_DRC_FRAME_DONE               FRAME_DONE;
	struct VREG_ISP_DRC_ENABLE                   ENABLE;
	struct VREG_ISP_DRC_BLK_NUM                  BLK_NUM;
	struct VREG_ISP_DRC_BLK_SIZE                 BLK_SIZE;
	struct VREG_ISP_DRC_BLK_DIV                  BLK_DIV;
	struct VREG_ISP_DRC_LPF_MODE                 LPF_MODE;
	struct VREG_ISP_DRC_HIST                     HIST;
	struct VREG_ISP_DRC_HIST_STEP                HIST_STEP;
	struct VREG_ISP_DRC_INTENSIT_MODE            INTENSIT_MODE;
	struct VREG_ISP_DRC_HORI_LPF_TH12            HORI_LPF_TH12;
	struct VREG_ISP_DRC_HORI_LPF_TH34            HORI_LPF_TH34;
	struct VREG_ISP_DRC_HORI_LPF_TH56            HORI_LPF_TH56;
	struct VREG_ISP_DRC_Q_COEFF12                Q_COEFF12;
	struct VREG_ISP_DRC_Q_COEFF34                Q_COEFF34;
	struct VREG_ISP_DRC_B2Q_TH                   B2Q_TH;
	struct VREG_ISP_DRC_FBC                      REG_FBC;
	struct VREG_ISP_DRC_ANTI_HALO                ANTI_HALO;
	struct VREG_ISP_DRC_STRENGTH                 STRENGTH;
	struct VREG_ISP_DRC_LTM_EN                   LTM_EN;
	struct VREG_ISP_DRC_GAIN                     GAIN;
	struct VREG_ISP_DRC_SAT_TH                   SAT_TH;
	struct VREG_ISP_DRC_SAT_DELTA                SAT_DELTA;
	struct VREG_ISP_DRC_YV_BLD_LUT0_4            YV_BLD_LUT0_4;
	struct VREG_ISP_DRC_YV_BLD_LUT5_8            YV_BLD_LUT5_8;
	struct VREG_ISP_DRC_INPUT_BLD_LUT_0_3        INPUT_BLD_LUT_0_3;
	struct VREG_ISP_DRC_INPUT_BLD_LUT_4_7        INPUT_BLD_LUT_4_7;
	struct VREG_ISP_DRC_INPUT_BLD_LUT_8_11       INPUT_BLD_LUT_8_11;
	struct VREG_ISP_DRC_INPUT_BLD_LUT_12_15      INPUT_BLD_LUT_12_15;
	struct VREG_ISP_DRC_INPUT_BLD_LUT_16         INPUT_BLD_LUT_16;
	struct VREG_ISP_DRC_GTM_L_LUT_0_1            GTM_L_LUT_0_1;
	struct VREG_ISP_DRC_GTM_L_LUT_2_3            GTM_L_LUT_2_3;
	struct VREG_ISP_DRC_GTM_L_LUT_4_5            GTM_L_LUT_4_5;
	struct VREG_ISP_DRC_GTM_L_LUT_6_7            GTM_L_LUT_6_7;
	struct VREG_ISP_DRC_GTM_L_LUT_8_9            GTM_L_LUT_8_9;
	struct VREG_ISP_DRC_GTM_L_LUT_10_11          GTM_L_LUT_10_11;
	struct VREG_ISP_DRC_GTM_L_LUT_12_13          GTM_L_LUT_12_13;
	struct VREG_ISP_DRC_GTM_L_LUT_14_15          GTM_L_LUT_14_15;
	struct VREG_ISP_DRC_GTM_L_LUT_16_17          GTM_L_LUT_16_17;
	struct VREG_ISP_DRC_GTM_L_LUT_18_19          GTM_L_LUT_18_19;
	struct VREG_ISP_DRC_GTM_L_LUT_20_21          GTM_L_LUT_20_21;
	struct VREG_ISP_DRC_GTM_L_LUT_22_23          GTM_L_LUT_22_23;
	struct VREG_ISP_DRC_GTM_L_LUT_24_25          GTM_L_LUT_24_25;
	struct VREG_ISP_DRC_GTM_L_LUT_26_27          GTM_L_LUT_26_27;
	struct VREG_ISP_DRC_GTM_L_LUT_28_29          GTM_L_LUT_28_29;
	struct VREG_ISP_DRC_GTM_L_LUT_30_31          GTM_L_LUT_30_31;
	struct VREG_ISP_DRC_GTM_L_LUT_32_33          GTM_L_LUT_32_33;
	struct VREG_ISP_DRC_GTM_L_LUT_34_35          GTM_L_LUT_34_35;
	struct VREG_ISP_DRC_GTM_L_LUT_36_37          GTM_L_LUT_36_37;
	struct VREG_ISP_DRC_GTM_L_LUT_38_39          GTM_L_LUT_38_39;
	struct VREG_ISP_DRC_GTM_L_LUT_40_41          GTM_L_LUT_40_41;
	struct VREG_ISP_DRC_GTM_L_LUT_42_43          GTM_L_LUT_42_43;
	struct VREG_ISP_DRC_GTM_L_LUT_44_45          GTM_L_LUT_44_45;
	struct VREG_ISP_DRC_GTM_L_LUT_46_47          GTM_L_LUT_46_47;
	struct VREG_ISP_DRC_GTM_L_LUT_48_49          GTM_L_LUT_48_49;
	struct VREG_ISP_DRC_GTM_L_LUT_50_51          GTM_L_LUT_50_51;
	struct VREG_ISP_DRC_GTM_L_LUT_52_53          GTM_L_LUT_52_53;
	struct VREG_ISP_DRC_GTM_L_LUT_54_55          GTM_L_LUT_54_55;
	struct VREG_ISP_DRC_GTM_L_LUT_56_57          GTM_L_LUT_56_57;
	struct VREG_ISP_DRC_GTM_L_LUT_58_59          GTM_L_LUT_58_59;
	struct VREG_ISP_DRC_GTM_L_LUT_60_61          GTM_L_LUT_60_61;
	struct VREG_ISP_DRC_GTM_L_LUT_62_63          GTM_L_LUT_62_63;
	struct VREG_ISP_DRC_GTM_L_LUT_64             GTM_L_LUT_64;
	struct VREG_ISP_DRC_GTM_R_LUT_0_1            GTM_R_LUT_0_1;
	struct VREG_ISP_DRC_GTM_R_LUT_2_3            GTM_R_LUT_2_3;
	struct VREG_ISP_DRC_GTM_R_LUT_4_5            GTM_R_LUT_4_5;
	struct VREG_ISP_DRC_GTM_R_LUT_6_7            GTM_R_LUT_6_7;
	struct VREG_ISP_DRC_GTM_R_LUT_8_9            GTM_R_LUT_8_9;
	struct VREG_ISP_DRC_GTM_R_LUT_10_11          GTM_R_LUT_10_11;
	struct VREG_ISP_DRC_GTM_R_LUT_12_13          GTM_R_LUT_12_13;
	struct VREG_ISP_DRC_GTM_R_LUT_14_15          GTM_R_LUT_14_15;
	struct VREG_ISP_DRC_GTM_R_LUT_16             GTM_R_LUT_16;
	struct VREG_ISP_DRC_LUMA_PROT_LUT_0_1        LUMA_PROT_LUT_0_1;
	struct VREG_ISP_DRC_LUMA_PROT_LUT_2_3        LUMA_PROT_LUT_2_3;
	struct VREG_ISP_DRC_LUMA_PROT_LUT_4_5        LUMA_PROT_LUT_4_5;
	struct VREG_ISP_DRC_LUMA_PROT_LUT_6_7        LUMA_PROT_LUT_6_7;
	struct VREG_ISP_DRC_LUMA_PROT_LUT_8_9        LUMA_PROT_LUT_8_9;
	struct VREG_ISP_DRC_LUMA_PROT_LUT_10_11      LUMA_PROT_LUT_10_11;
	struct VREG_ISP_DRC_LUMA_PROT_LUT_12_13      LUMA_PROT_LUT_12_13;
	struct VREG_ISP_DRC_LUMA_PROT_LUT_14_15      LUMA_PROT_LUT_14_15;
	struct VREG_ISP_DRC_LUMA_PROT_LUT_16         LUMA_PROT_LUT_16;
	struct VREG_ISP_DRC_LTM_L_LUT_0_1            LTM_L_LUT_0_1;
	struct VREG_ISP_DRC_LTM_L_LUT_2_3            LTM_L_LUT_2_3;
	struct VREG_ISP_DRC_LTM_L_LUT_4_5            LTM_L_LUT_4_5;
	struct VREG_ISP_DRC_LTM_L_LUT_6_7            LTM_L_LUT_6_7;
	struct VREG_ISP_DRC_LTM_L_LUT_8_9            LTM_L_LUT_8_9;
	struct VREG_ISP_DRC_LTM_L_LUT_10_11          LTM_L_LUT_10_11;
	struct VREG_ISP_DRC_LTM_L_LUT_12_13          LTM_L_LUT_12_13;
	struct VREG_ISP_DRC_LTM_L_LUT_14_15          LTM_L_LUT_14_15;
	struct VREG_ISP_DRC_LTM_L_LUT_16_17          LTM_L_LUT_16_17;
	struct VREG_ISP_DRC_LTM_L_LUT_18_19          LTM_L_LUT_18_19;
	struct VREG_ISP_DRC_LTM_L_LUT_20_21          LTM_L_LUT_20_21;
	struct VREG_ISP_DRC_LTM_L_LUT_22_23          LTM_L_LUT_22_23;
	struct VREG_ISP_DRC_LTM_L_LUT_24_25          LTM_L_LUT_24_25;
	struct VREG_ISP_DRC_LTM_L_LUT_26_27          LTM_L_LUT_26_27;
	struct VREG_ISP_DRC_LTM_L_LUT_28_29          LTM_L_LUT_28_29;
	struct VREG_ISP_DRC_LTM_L_LUT_30_31          LTM_L_LUT_30_31;
	struct VREG_ISP_DRC_LTM_L_LUT_32_33          LTM_L_LUT_32_33;
	struct VREG_ISP_DRC_LTM_L_LUT_34_35          LTM_L_LUT_34_35;
	struct VREG_ISP_DRC_LTM_L_LUT_36_37          LTM_L_LUT_36_37;
	struct VREG_ISP_DRC_LTM_L_LUT_38_39          LTM_L_LUT_38_39;
	struct VREG_ISP_DRC_LTM_L_LUT_40_41          LTM_L_LUT_40_41;
	struct VREG_ISP_DRC_LTM_L_LUT_42_43          LTM_L_LUT_42_43;
	struct VREG_ISP_DRC_LTM_L_LUT_44_45          LTM_L_LUT_44_45;
	struct VREG_ISP_DRC_LTM_L_LUT_46_47          LTM_L_LUT_46_47;
	struct VREG_ISP_DRC_LTM_L_LUT_48_49          LTM_L_LUT_48_49;
	struct VREG_ISP_DRC_LTM_L_LUT_50_51          LTM_L_LUT_50_51;
	struct VREG_ISP_DRC_LTM_L_LUT_52_53          LTM_L_LUT_52_53;
	struct VREG_ISP_DRC_LTM_L_LUT_54_55          LTM_L_LUT_54_55;
	struct VREG_ISP_DRC_LTM_L_LUT_56_57          LTM_L_LUT_56_57;
	struct VREG_ISP_DRC_LTM_L_LUT_58_59          LTM_L_LUT_58_59;
	struct VREG_ISP_DRC_LTM_L_LUT_60_61          LTM_L_LUT_60_61;
	struct VREG_ISP_DRC_LTM_L_LUT_62_63          LTM_L_LUT_62_63;
	struct VREG_ISP_DRC_LTM_L_LUT_64             LTM_L_LUT_64;
	struct VREG_ISP_DRC_LTM_R_LUT_0_1            LTM_R_LUT_0_1;
	struct VREG_ISP_DRC_LTM_R_LUT_2_3            LTM_R_LUT_2_3;
	struct VREG_ISP_DRC_LTM_R_LUT_4_5            LTM_R_LUT_4_5;
	struct VREG_ISP_DRC_LTM_R_LUT_6_7            LTM_R_LUT_6_7;
	struct VREG_ISP_DRC_LTM_R_LUT_8_9            LTM_R_LUT_8_9;
	struct VREG_ISP_DRC_LTM_R_LUT_10_11          LTM_R_LUT_10_11;
	struct VREG_ISP_DRC_LTM_R_LUT_12_13          LTM_R_LUT_12_13;
	struct VREG_ISP_DRC_LTM_R_LUT_14_15          LTM_R_LUT_14_15;
	struct VREG_ISP_DRC_LTM_R_LUT_16             LTM_R_LUT_16;
	struct VREG_ISP_DRC_IMG_SIZE                 IMG_SIZE;
	struct VREG_RESV                        _resv_0x1ec[1];
	struct VREG_ISP_DRC_SHDW_SEL                 SHDW_SEL;
	struct VREG_ISP_DRC_PIPECTRL_CG              PIPEctrl_CG;
	struct VREG_ISP_DRC_BAYERID                  BAYERID;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_FBCD_00 {
	union REG_FBCD_00                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FBCD_0C {
	union REG_FBCD_0C                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FBCD_10 {
	union REG_FBCD_10                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FBCD_14 {
	union REG_FBCD_14                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FBCD_18 {
	union REG_FBCD_18                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FBCD_20 {
	union REG_FBCD_20                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FBCD_24 {
	union REG_FBCD_24                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FBCD_28 {
	union REG_FBCD_28                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FBCD_T {
	struct VREG_FBCD_00                      REG_00;
	struct VREG_RESV                        _resv_0x4[2];
	struct VREG_FBCD_0C                      REG_0C;
	struct VREG_FBCD_10                      REG_10;
	struct VREG_FBCD_14                      REG_14;
	struct VREG_FBCD_18                      REG_18;
	struct VREG_RESV                        _resv_0x1c[1];
	struct VREG_FBCD_20                      REG_20;
	struct VREG_FBCD_24                      REG_24;
	struct VREG_FBCD_28                      REG_28;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_FBCE_00 {
	union REG_FBCE_00                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FBCE_10 {
	union REG_FBCE_10                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FBCE_14 {
	union REG_FBCE_14                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FBCE_18 {
	union REG_FBCE_18                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FBCE_1C {
	union REG_FBCE_1C                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FBCE_20 {
	union REG_FBCE_20                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FBCE_24 {
	union REG_FBCE_24                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FBCE_28 {
	union REG_FBCE_28                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FBCE_2C {
	union REG_FBCE_2C                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FBCE_30 {
	union REG_FBCE_30                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FBCE_34 {
	union REG_FBCE_34                       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_FBCE_T {
	struct VREG_FBCE_00                      REG_00;
	struct VREG_RESV                        _resv_0x4[3];
	struct VREG_FBCE_10                      REG_10;
	struct VREG_FBCE_14                      REG_14;
	struct VREG_FBCE_18                      REG_18;
	struct VREG_FBCE_1C                      REG_1C;
	struct VREG_FBCE_20                      REG_20;
	struct VREG_FBCE_24                      REG_24;
	struct VREG_FBCE_28                      REG_28;
	struct VREG_FBCE_2C                      REG_2C;
	struct VREG_FBCE_30                      REG_30;
	struct VREG_FBCE_34                      REG_34;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_CFA_00 {
	union REG_ISP_CFA_00                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_04 {
	union REG_ISP_CFA_04                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_0C {
	union REG_ISP_CFA_0C                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_10 {
	union REG_ISP_CFA_10                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_14 {
	union REG_ISP_CFA_14                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_18 {
	union REG_ISP_CFA_18                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_1C {
	union REG_ISP_CFA_1C                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_20 {
	union REG_ISP_CFA_20                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_30 {
	union REG_ISP_CFA_30                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_34 {
	union REG_ISP_CFA_34                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_38 {
	union REG_ISP_CFA_38                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_3C {
	union REG_ISP_CFA_3C                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_40 {
	union REG_ISP_CFA_40                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_44 {
	union REG_ISP_CFA_44                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_48 {
	union REG_ISP_CFA_48                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_4C {
	union REG_ISP_CFA_4C                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_70 {
	union REG_ISP_CFA_70                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_74 {
	union REG_ISP_CFA_74                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_78 {
	union REG_ISP_CFA_78                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_7C {
	union REG_ISP_CFA_7C                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_80 {
	union REG_ISP_CFA_80                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_90 {
	union REG_ISP_CFA_90                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_94 {
	union REG_ISP_CFA_94                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_98 {
	union REG_ISP_CFA_98                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_9C {
	union REG_ISP_CFA_9C                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_A0 {
	union REG_ISP_CFA_A0                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_A4 {
	union REG_ISP_CFA_A4                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_A8 {
	union REG_ISP_CFA_A8                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_110 {
	union REG_ISP_CFA_110                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_120 {
	union REG_ISP_CFA_120                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_124 {
	union REG_ISP_CFA_124                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_128 {
	union REG_ISP_CFA_128                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_12C {
	union REG_ISP_CFA_12C                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_130 {
	union REG_ISP_CFA_130                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_134 {
	union REG_ISP_CFA_134                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_138 {
	union REG_ISP_CFA_138                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_13C {
	union REG_ISP_CFA_13C                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_140 {
	union REG_ISP_CFA_140                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_144 {
	union REG_ISP_CFA_144                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_148 {
	union REG_ISP_CFA_148                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_14C {
	union REG_ISP_CFA_14C                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_150 {
	union REG_ISP_CFA_150                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_154 {
	union REG_ISP_CFA_154                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_158 {
	union REG_ISP_CFA_158                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_15C {
	union REG_ISP_CFA_15C                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_160 {
	union REG_ISP_CFA_160                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_164 {
	union REG_ISP_CFA_164                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_168 {
	union REG_ISP_CFA_168                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_16C {
	union REG_ISP_CFA_16C                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_170 {
	union REG_ISP_CFA_170                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_174 {
	union REG_ISP_CFA_174                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_178 {
	union REG_ISP_CFA_178                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_17C {
	union REG_ISP_CFA_17C                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_180 {
	union REG_ISP_CFA_180                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_184 {
	union REG_ISP_CFA_184                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_188 {
	union REG_ISP_CFA_188                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_18C {
	union REG_ISP_CFA_18C                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_190 {
	union REG_ISP_CFA_190                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CFA_T {
	struct VREG_ISP_CFA_00                   REG_00;
	struct VREG_ISP_CFA_04                   REG_04;
	struct VREG_RESV                        _resv_0x8[1];
	struct VREG_ISP_CFA_0C                   REG_0C;
	struct VREG_ISP_CFA_10                   REG_10;
	struct VREG_ISP_CFA_14                   REG_14;
	struct VREG_ISP_CFA_18                   REG_18;
	struct VREG_ISP_CFA_1C                   REG_1C;
	struct VREG_ISP_CFA_20                   REG_20;
	struct VREG_RESV                        _resv_0x24[3];
	struct VREG_ISP_CFA_30                   REG_30;
	struct VREG_ISP_CFA_34                   REG_34;
	struct VREG_ISP_CFA_38                   REG_38;
	struct VREG_ISP_CFA_3C                   REG_3C;
	struct VREG_ISP_CFA_40                   REG_40;
	struct VREG_ISP_CFA_44                   REG_44;
	struct VREG_ISP_CFA_48                   REG_48;
	struct VREG_ISP_CFA_4C                   REG_4C;
	struct VREG_RESV                        _resv_0x50[8];
	struct VREG_ISP_CFA_70                   REG_70;
	struct VREG_ISP_CFA_74                   REG_74;
	struct VREG_ISP_CFA_78                   REG_78;
	struct VREG_ISP_CFA_7C                   REG_7C;
	struct VREG_ISP_CFA_80                   REG_80;
	struct VREG_RESV                        _resv_0x84[3];
	struct VREG_ISP_CFA_90                   REG_90;
	struct VREG_ISP_CFA_94                   REG_94;
	struct VREG_ISP_CFA_98                   REG_98;
	struct VREG_ISP_CFA_9C                   REG_9C;
	struct VREG_ISP_CFA_A0                   REG_A0;
	struct VREG_ISP_CFA_A4                   REG_A4;
	struct VREG_ISP_CFA_A8                   REG_A8;
	struct VREG_RESV                        _resv_0xac[25];
	struct VREG_ISP_CFA_110                  REG_110;
	struct VREG_RESV                        _resv_0x114[3];
	struct VREG_ISP_CFA_120                  REG_120;
	struct VREG_ISP_CFA_124                  REG_124;
	struct VREG_ISP_CFA_128                  REG_128;
	struct VREG_ISP_CFA_12C                  REG_12C;
	struct VREG_ISP_CFA_130                  REG_130;
	struct VREG_ISP_CFA_134                  REG_134;
	struct VREG_ISP_CFA_138                  REG_138;
	struct VREG_ISP_CFA_13C                  REG_13C;
	struct VREG_ISP_CFA_140                  REG_140;
	struct VREG_ISP_CFA_144                  REG_144;
	struct VREG_ISP_CFA_148                  REG_148;
	struct VREG_ISP_CFA_14C                  REG_14C;
	struct VREG_ISP_CFA_150                  REG_150;
	struct VREG_ISP_CFA_154                  REG_154;
	struct VREG_ISP_CFA_158                  REG_158;
	struct VREG_ISP_CFA_15C                  REG_15C;
	struct VREG_ISP_CFA_160                  REG_160;
	struct VREG_ISP_CFA_164                  REG_164;
	struct VREG_ISP_CFA_168                  REG_168;
	struct VREG_ISP_CFA_16C                  REG_16C;
	struct VREG_ISP_CFA_170                  REG_170;
	struct VREG_ISP_CFA_174                  REG_174;
	struct VREG_ISP_CFA_178                  REG_178;
	struct VREG_ISP_CFA_17C                  REG_17C;
	struct VREG_ISP_CFA_180                  REG_180;
	struct VREG_ISP_CFA_184                  REG_184;
	struct VREG_ISP_CFA_188                  REG_188;
	struct VREG_ISP_CFA_18C                  REG_18C;
	struct VREG_ISP_CFA_190                  REG_190;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_RGB_TOP_PATGEN_01 {
	union REG_RGB_TOP_PATGEN_01             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RGB_TOP_PATGEN_02 {
	union REG_RGB_TOP_PATGEN_02             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RGB_TOP_PATGEN_03 {
	union REG_RGB_TOP_PATGEN_03             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RGB_TOP_PATGEN_04 {
	union REG_RGB_TOP_PATGEN_04             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RGB_TOP_DUMMY {
	union REG_RGB_TOP_DUMMY                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RGB_TOP_PATGEN_05 {
	union REG_RGB_TOP_PATGEN_05             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_RGB_TOP_T {
	struct VREG_RGB_TOP_PATGEN_01            PATGEN_01;
	struct VREG_RGB_TOP_PATGEN_02            PATGEN_02;
	struct VREG_RGB_TOP_PATGEN_03            PATGEN_03;
	struct VREG_RGB_TOP_PATGEN_04            PATGEN_04;
	struct VREG_RGB_TOP_DUMMY                DUMMY;
	struct VREG_RGB_TOP_PATGEN_05            PATGEN_05;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_EE_EXT_REG0 {
	union REG_EE_EXT_REG0                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_GAMMA_0 {
	union REG_EE_EXT_GAMMA_0                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_GAMMA_1 {
	union REG_EE_EXT_GAMMA_1                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_GAMMA_2 {
	union REG_EE_EXT_GAMMA_2                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_GAMMA_3 {
	union REG_EE_EXT_GAMMA_3                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_GAMMA_4 {
	union REG_EE_EXT_GAMMA_4                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_GAMMA_5 {
	union REG_EE_EXT_GAMMA_5                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_GAMMA_6 {
	union REG_EE_EXT_GAMMA_6                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_GAMMA_7 {
	union REG_EE_EXT_GAMMA_7                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_GAMMA_8 {
	union REG_EE_EXT_GAMMA_8                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_GAMMA_9 {
	union REG_EE_EXT_GAMMA_9                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_GAMMA_10 {
	union REG_EE_EXT_GAMMA_10               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_GAMMA_11 {
	union REG_EE_EXT_GAMMA_11               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_GAMMA_12 {
	union REG_EE_EXT_GAMMA_12               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_GAMMA_13 {
	union REG_EE_EXT_GAMMA_13               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_GAMMA_14 {
	union REG_EE_EXT_GAMMA_14               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_GAMMA_15 {
	union REG_EE_EXT_GAMMA_15               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_GAMMA_16 {
	union REG_EE_EXT_GAMMA_16               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_GAMMA_17 {
	union REG_EE_EXT_GAMMA_17               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_GAMMA_18 {
	union REG_EE_EXT_GAMMA_18               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_GAMMA_19 {
	union REG_EE_EXT_GAMMA_19               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_GAMMA_20 {
	union REG_EE_EXT_GAMMA_20               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_E5C_0 {
	union REG_EE_EXT_E5C_0                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_E5C_1 {
	union REG_EE_EXT_E5C_1                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_E5C_2 {
	union REG_EE_EXT_E5C_2                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_E5A_0 {
	union REG_EE_EXT_E5A_0                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_E5A_1 {
	union REG_EE_EXT_E5A_1                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_E5A_2 {
	union REG_EE_EXT_E5A_2                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_E5B_0 {
	union REG_EE_EXT_E5B_0                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_E5B_1 {
	union REG_EE_EXT_E5B_1                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_E5B_2 {
	union REG_EE_EXT_E5B_2                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_E7_0 {
	union REG_EE_EXT_E7_0                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_E7_1 {
	union REG_EE_EXT_E7_1                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_E7_2 {
	union REG_EE_EXT_E7_2                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_E7_3 {
	union REG_EE_EXT_E7_3                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_E7_4 {
	union REG_EE_EXT_E7_4                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_NORM {
	union REG_EE_EXT_NORM                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_LUMA_BLEND {
	union REG_EE_EXT_LUMA_BLEND             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_REGION_0 {
	union REG_EE_EXT_REGION_0               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_REGION_1 {
	union REG_EE_EXT_REGION_1               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_REGION_2 {
	union REG_EE_EXT_REGION_2               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_REGION_3 {
	union REG_EE_EXT_REGION_3               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_REGION_4 {
	union REG_EE_EXT_REGION_4               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_REGION_5 {
	union REG_EE_EXT_REGION_5               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_REGION_6 {
	union REG_EE_EXT_REGION_6               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_SHADOW_RD_SEL {
	union REG_EE_EXT_SHADOW_RD_SEL          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_EE_EXT_T {
	struct VREG_EE_EXT_REG0                  REG0;
	struct VREG_EE_EXT_GAMMA_0               GAMMA_0;
	struct VREG_EE_EXT_GAMMA_1               GAMMA_1;
	struct VREG_EE_EXT_GAMMA_2               GAMMA_2;
	struct VREG_EE_EXT_GAMMA_3               GAMMA_3;
	struct VREG_EE_EXT_GAMMA_4               GAMMA_4;
	struct VREG_EE_EXT_GAMMA_5               GAMMA_5;
	struct VREG_EE_EXT_GAMMA_6               GAMMA_6;
	struct VREG_EE_EXT_GAMMA_7               GAMMA_7;
	struct VREG_EE_EXT_GAMMA_8               GAMMA_8;
	struct VREG_EE_EXT_GAMMA_9               GAMMA_9;
	struct VREG_EE_EXT_GAMMA_10              GAMMA_10;
	struct VREG_EE_EXT_GAMMA_11              GAMMA_11;
	struct VREG_EE_EXT_GAMMA_12              GAMMA_12;
	struct VREG_EE_EXT_GAMMA_13              GAMMA_13;
	struct VREG_EE_EXT_GAMMA_14              GAMMA_14;
	struct VREG_EE_EXT_GAMMA_15              GAMMA_15;
	struct VREG_EE_EXT_GAMMA_16              GAMMA_16;
	struct VREG_EE_EXT_GAMMA_17              GAMMA_17;
	struct VREG_EE_EXT_GAMMA_18              GAMMA_18;
	struct VREG_EE_EXT_GAMMA_19              GAMMA_19;
	struct VREG_EE_EXT_GAMMA_20              GAMMA_20;
	struct VREG_EE_EXT_E5C_0                 E5C_0;
	struct VREG_EE_EXT_E5C_1                 E5C_1;
	struct VREG_EE_EXT_E5C_2                 E5C_2;
	struct VREG_EE_EXT_E5A_0                 E5A_0;
	struct VREG_EE_EXT_E5A_1                 E5A_1;
	struct VREG_EE_EXT_E5A_2                 E5A_2;
	struct VREG_EE_EXT_E5B_0                 E5B_0;
	struct VREG_EE_EXT_E5B_1                 E5B_1;
	struct VREG_EE_EXT_E5B_2                 E5B_2;
	struct VREG_EE_EXT_E7_0                  E7_0;
	struct VREG_EE_EXT_E7_1                  E7_1;
	struct VREG_EE_EXT_E7_2                  E7_2;
	struct VREG_EE_EXT_E7_3                  E7_3;
	struct VREG_EE_EXT_E7_4                  E7_4;
	struct VREG_EE_EXT_NORM                  NORM;
	struct VREG_EE_EXT_LUMA_BLEND            LUMA_BLEND;
	struct VREG_EE_EXT_REGION_0              REGION_0;
	struct VREG_EE_EXT_REGION_1              REGION_1;
	struct VREG_RESV                        _resv_0xa0[24];
	struct VREG_EE_EXT_REGION_2              REGION_2;
	struct VREG_EE_EXT_REGION_3              REGION_3;
	struct VREG_EE_EXT_REGION_4              REGION_4;
	struct VREG_EE_EXT_REGION_5              REGION_5;
	struct VREG_EE_EXT_REGION_6              REGION_6;
	struct VREG_EE_EXT_SHADOW_RD_SEL         SHADOW_RD_SEL;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_PFR_REG0 {
	union REG_ISP_PFR_REG0                      write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_PFR_LUMA_SET0 {
	union REG_ISP_PFR_LUMA_SET0                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_PFR_LUMA_LEVEL0 {
	union REG_ISP_PFR_LUMA_LEVEL0               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_PFR_LUMA_LEVEL1 {
	union REG_ISP_PFR_LUMA_LEVEL1               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_PFR_LUMA_LEVEL2 {
	union REG_ISP_PFR_LUMA_LEVEL2               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_PFR_LUMA_LEVEL3 {
	union REG_ISP_PFR_LUMA_LEVEL3               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_PFR_LUMA_LEVEL4 {
	union REG_ISP_PFR_LUMA_LEVEL4               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_PFR_COLOR_UV {
	union REG_ISP_PFR_COLOR_UV                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_PFR_UV_DIFF_TH {
	union REG_ISP_PFR_UV_DIFF_TH                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_PFR_UV_DIFF_LUT0 {
	union REG_ISP_PFR_UV_DIFF_LUT0              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_PFR_UV_DIFF_LUT1 {
	union REG_ISP_PFR_UV_DIFF_LUT1              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_PFR_RB_WET {
	union REG_ISP_PFR_RB_WET                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_PFR_HUESET {
	union REG_ISP_PFR_HUESET                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_PFR_HUE_RANGE {
	union REG_ISP_PFR_HUE_RANGE                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_PFR_HUE_TH {
	union REG_ISP_PFR_HUE_TH                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_PFR_G_DIFF_TH0 {
	union REG_ISP_PFR_G_DIFF_TH0                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_PFR_G_DIFF_TH1 {
	union REG_ISP_PFR_G_DIFF_TH1                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_PFR_EDGE_TH {
	union REG_ISP_PFR_EDGE_TH                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_PFR_FCS_SET {
	union REG_ISP_PFR_FCS_SET                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_PFR_SHADOW_RD_SEL {
	union REG_ISP_PFR_SHADOW_RD_SEL             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_PFR_T {
	struct VREG_ISP_PFR_REG0                     REG0;
	struct VREG_ISP_PFR_LUMA_SET0                LUMA_SET0;
	struct VREG_ISP_PFR_LUMA_LEVEL0              LUMA_LEVEL0;
	struct VREG_RESV                        _resv_0xc[1];
	struct VREG_ISP_PFR_LUMA_LEVEL1              LUMA_LEVEL1;
	struct VREG_ISP_PFR_LUMA_LEVEL2              LUMA_LEVEL2;
	struct VREG_ISP_PFR_LUMA_LEVEL3              LUMA_LEVEL3;
	struct VREG_ISP_PFR_LUMA_LEVEL4              LUMA_LEVEL4;
	struct VREG_ISP_PFR_COLOR_UV                 COLOR_UV;
	struct VREG_ISP_PFR_UV_DIFF_TH               UV_DIFF_TH;
	struct VREG_ISP_PFR_UV_DIFF_LUT0             UV_DIFF_LUT0;
	struct VREG_RESV                        _resv_0x2c[1];
	struct VREG_ISP_PFR_UV_DIFF_LUT1             UV_DIFF_LUT1;
	struct VREG_ISP_PFR_RB_WET                   RB_WET;
	struct VREG_ISP_PFR_HUESET                   HUESET;
	struct VREG_RESV                        _resv_0x3c[1];
	struct VREG_ISP_PFR_HUE_RANGE                HUE_RANGE;
	struct VREG_ISP_PFR_HUE_TH                   HUE_TH;
	struct VREG_ISP_PFR_G_DIFF_TH0               G_DIFF_TH0;
	struct VREG_ISP_PFR_G_DIFF_TH1               G_DIFF_TH1;
	struct VREG_ISP_PFR_EDGE_TH                  EDGE_TH;
	struct VREG_ISP_PFR_FCS_SET                  FCS_SET;
	struct VREG_ISP_PFR_SHADOW_RD_SEL            SHADOW_RD_SEL;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_CCM_00 {
	union REG_ISP_CCM_00                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CCM_01 {
	union REG_ISP_CCM_01                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CCM_02 {
	union REG_ISP_CCM_02                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CCM_10 {
	union REG_ISP_CCM_10                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CCM_11 {
	union REG_ISP_CCM_11                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CCM_12 {
	union REG_ISP_CCM_12                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CCM_20 {
	union REG_ISP_CCM_20                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CCM_21 {
	union REG_ISP_CCM_21                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CCM_22 {
	union REG_ISP_CCM_22                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CCM_CTRL {
	union REG_ISP_CCM_CTRL                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CCM_DBG {
	union REG_ISP_CCM_DBG                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CCM_DMY0 {
	union REG_ISP_CCM_DMY0                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CCM_DMY1 {
	union REG_ISP_CCM_DMY1                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CCM_DMY_R {
	union REG_ISP_CCM_DMY_R                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CCM_OVEREXP_STR {
	union REG_ISP_CCM_OVEREXP_STR           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CCM_OVEREXP_THR {
	union REG_ISP_CCM_OVEREXP_THR           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CCM_T {
	struct VREG_ISP_CCM_00                   CCM_00;
	struct VREG_ISP_CCM_01                   CCM_01;
	struct VREG_ISP_CCM_02                   CCM_02;
	struct VREG_ISP_CCM_10                   CCM_10;
	struct VREG_ISP_CCM_11                   CCM_11;
	struct VREG_ISP_CCM_12                   CCM_12;
	struct VREG_ISP_CCM_20                   CCM_20;
	struct VREG_ISP_CCM_21                   CCM_21;
	struct VREG_ISP_CCM_22                   CCM_22;
	struct VREG_ISP_CCM_CTRL                 CCM_CTRL;
	struct VREG_ISP_CCM_DBG                  CCM_DBG;
	struct VREG_RESV                        _resv_0x2c[1];
	struct VREG_ISP_CCM_DMY0                 DMY0;
	struct VREG_ISP_CCM_DMY1                 DMY1;
	struct VREG_ISP_CCM_DMY_R                DMY_R;
	struct VREG_ISP_CCM_OVEREXP_STR          CCM_OVEREXP_STR;
	struct VREG_ISP_CCM_OVEREXP_THR          CCM_OVEREXP_THR;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_GAMMA_CTRL {
	union REG_ISP_GAMMA_CTRL                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GAMMA_PROG_CTRL {
	union REG_ISP_GAMMA_PROG_CTRL           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GAMMA_PROG_ST_ADDR {
	union REG_ISP_GAMMA_PROG_ST_ADDR        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GAMMA_PROG_DATA {
	union REG_ISP_GAMMA_PROG_DATA           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GAMMA_PROG_MAX {
	union REG_ISP_GAMMA_PROG_MAX            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GAMMA_MEM_SW_RADDR {
	union REG_ISP_GAMMA_MEM_SW_RADDR        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GAMMA_MEM_SW_RDATA {
	union REG_ISP_GAMMA_MEM_SW_RDATA        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GAMMA_MEM_SW_RDATA_BG {
	union REG_ISP_GAMMA_MEM_SW_RDATA_BG     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GAMMA_DBG {
	union REG_ISP_GAMMA_DBG                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GAMMA_DMY0 {
	union REG_ISP_GAMMA_DMY0                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GAMMA_DMY1 {
	union REG_ISP_GAMMA_DMY1                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GAMMA_DMY_R {
	union REG_ISP_GAMMA_DMY_R               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_GAMMA_T {
	struct VREG_ISP_GAMMA_CTRL               GAMMA_CTRL;
	struct VREG_ISP_GAMMA_PROG_CTRL          GAMMA_PROG_CTRL;
	struct VREG_ISP_GAMMA_PROG_ST_ADDR       GAMMA_PROG_ST_ADDR;
	struct VREG_ISP_GAMMA_PROG_DATA          GAMMA_PROG_DATA;
	struct VREG_ISP_GAMMA_PROG_MAX           GAMMA_PROG_MAX;
	struct VREG_ISP_GAMMA_MEM_SW_RADDR       GAMMA_SW_RADDR;
	struct VREG_ISP_GAMMA_MEM_SW_RDATA       GAMMA_SW_RDATA;
	struct VREG_ISP_GAMMA_MEM_SW_RDATA_BG    GAMMA_SW_RDATA_BG;
	struct VREG_ISP_GAMMA_DBG                GAMMA_DBG;
	struct VREG_ISP_GAMMA_DMY0               GAMMA_DMY0;
	struct VREG_ISP_GAMMA_DMY1               GAMMA_DMY1;
	struct VREG_ISP_GAMMA_DMY_R              GAMMA_DMY_R;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_DEHAZE_DHZ_SMOOTH {
	union REG_ISP_DEHAZE_DHZ_SMOOTH         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_DHZ_SKIN {
	union REG_ISP_DEHAZE_DHZ_SKIN           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_DHZ_WGT {
	union REG_ISP_DEHAZE_DHZ_WGT            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_DHZ_BYPASS {
	union REG_ISP_DEHAZE_DHZ_BYPASS         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_0 {
	union REG_ISP_DEHAZE_0                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_1 {
	union REG_ISP_DEHAZE_1                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_2 {
	union REG_ISP_DEHAZE_2                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_28 {
	union REG_ISP_DEHAZE_28                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_2C {
	union REG_ISP_DEHAZE_2C                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_3 {
	union REG_ISP_DEHAZE_3                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_5 {
	union REG_ISP_DEHAZE_5                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_6 {
	union REG_ISP_DEHAZE_6                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_7 {
	union REG_ISP_DEHAZE_7                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_8 {
	union REG_ISP_DEHAZE_8                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_9 {
	union REG_ISP_DEHAZE_9                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_10 {
	union REG_ISP_DEHAZE_10                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_11 {
	union REG_ISP_DEHAZE_11                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_12 {
	union REG_ISP_DEHAZE_12                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_17 {
	union REG_ISP_DEHAZE_17                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_18 {
	union REG_ISP_DEHAZE_18                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_19 {
	union REG_ISP_DEHAZE_19                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_20 {
	union REG_ISP_DEHAZE_20                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_25 {
	union REG_ISP_DEHAZE_25                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_26 {
	union REG_ISP_DEHAZE_26                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_00 {
	union REG_ISP_DEHAZE_TMAP_00            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_01 {
	union REG_ISP_DEHAZE_TMAP_01            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_02 {
	union REG_ISP_DEHAZE_TMAP_02            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_03 {
	union REG_ISP_DEHAZE_TMAP_03            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_04 {
	union REG_ISP_DEHAZE_TMAP_04            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_05 {
	union REG_ISP_DEHAZE_TMAP_05            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_06 {
	union REG_ISP_DEHAZE_TMAP_06            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_07 {
	union REG_ISP_DEHAZE_TMAP_07            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_08 {
	union REG_ISP_DEHAZE_TMAP_08            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_09 {
	union REG_ISP_DEHAZE_TMAP_09            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_10 {
	union REG_ISP_DEHAZE_TMAP_10            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_11 {
	union REG_ISP_DEHAZE_TMAP_11            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_12 {
	union REG_ISP_DEHAZE_TMAP_12            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_13 {
	union REG_ISP_DEHAZE_TMAP_13            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_14 {
	union REG_ISP_DEHAZE_TMAP_14            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_15 {
	union REG_ISP_DEHAZE_TMAP_15            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_16 {
	union REG_ISP_DEHAZE_TMAP_16            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_17 {
	union REG_ISP_DEHAZE_TMAP_17            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_18 {
	union REG_ISP_DEHAZE_TMAP_18            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_19 {
	union REG_ISP_DEHAZE_TMAP_19            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_20 {
	union REG_ISP_DEHAZE_TMAP_20            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_21 {
	union REG_ISP_DEHAZE_TMAP_21            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_22 {
	union REG_ISP_DEHAZE_TMAP_22            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_23 {
	union REG_ISP_DEHAZE_TMAP_23            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_24 {
	union REG_ISP_DEHAZE_TMAP_24            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_25 {
	union REG_ISP_DEHAZE_TMAP_25            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_26 {
	union REG_ISP_DEHAZE_TMAP_26            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_27 {
	union REG_ISP_DEHAZE_TMAP_27            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_28 {
	union REG_ISP_DEHAZE_TMAP_28            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_29 {
	union REG_ISP_DEHAZE_TMAP_29            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_30 {
	union REG_ISP_DEHAZE_TMAP_30            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_31 {
	union REG_ISP_DEHAZE_TMAP_31            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_TMAP_32 {
	union REG_ISP_DEHAZE_TMAP_32            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_O_DC_TH_UP_MODE {
	union REG_ISP_DEHAZE_O_DC_TH_UP_MODE    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_DEHAZE_T {
	struct VREG_ISP_DEHAZE_DHZ_SMOOTH        DHZ_SMOOTH;
	struct VREG_ISP_DEHAZE_DHZ_SKIN          DHZ_SKIN;
	struct VREG_ISP_DEHAZE_DHZ_WGT           DHZ_WGT;
	struct VREG_RESV                        _resv_0xc[2];
	struct VREG_ISP_DEHAZE_DHZ_BYPASS        DHZ_BYPASS;
	struct VREG_ISP_DEHAZE_0                 REG_0;
	struct VREG_RESV                        _resv_0x1c[1];
	struct VREG_ISP_DEHAZE_1                 REG_1;
	struct VREG_ISP_DEHAZE_2                 REG_2;
	struct VREG_ISP_DEHAZE_28                REG_28;
	struct VREG_ISP_DEHAZE_2C                REG_2C;
	struct VREG_ISP_DEHAZE_3                 REG_3;
	struct VREG_ISP_DEHAZE_5                 REG_5;
	struct VREG_ISP_DEHAZE_6                 REG_6;
	struct VREG_ISP_DEHAZE_7                 REG_7;
	struct VREG_ISP_DEHAZE_8                 REG_8;
	struct VREG_RESV                        _resv_0x44[3];
	struct VREG_ISP_DEHAZE_9                 REG_9;
	struct VREG_ISP_DEHAZE_10                REG_10;
	struct VREG_ISP_DEHAZE_11                REG_11;
	struct VREG_ISP_DEHAZE_12                REG_12;
	struct VREG_ISP_DEHAZE_17                REG_17;
	struct VREG_ISP_DEHAZE_18                REG_18;
	struct VREG_ISP_DEHAZE_19                REG_19;
	struct VREG_ISP_DEHAZE_20                REG_20;
	struct VREG_ISP_DEHAZE_25                REG_25;
	struct VREG_ISP_DEHAZE_26                REG_26;
	struct VREG_ISP_DEHAZE_TMAP_00           TMAP_00;
	struct VREG_ISP_DEHAZE_TMAP_01           TMAP_01;
	struct VREG_ISP_DEHAZE_TMAP_02           TMAP_02;
	struct VREG_ISP_DEHAZE_TMAP_03           TMAP_03;
	struct VREG_ISP_DEHAZE_TMAP_04           TMAP_04;
	struct VREG_ISP_DEHAZE_TMAP_05           TMAP_05;
	struct VREG_ISP_DEHAZE_TMAP_06           TMAP_06;
	struct VREG_ISP_DEHAZE_TMAP_07           TMAP_07;
	struct VREG_ISP_DEHAZE_TMAP_08           TMAP_08;
	struct VREG_ISP_DEHAZE_TMAP_09           TMAP_09;
	struct VREG_ISP_DEHAZE_TMAP_10           TMAP_10;
	struct VREG_ISP_DEHAZE_TMAP_11           TMAP_11;
	struct VREG_ISP_DEHAZE_TMAP_12           TMAP_12;
	struct VREG_ISP_DEHAZE_TMAP_13           TMAP_13;
	struct VREG_ISP_DEHAZE_TMAP_14           TMAP_14;
	struct VREG_ISP_DEHAZE_TMAP_15           TMAP_15;
	struct VREG_ISP_DEHAZE_TMAP_16           TMAP_16;
	struct VREG_ISP_DEHAZE_TMAP_17           TMAP_17;
	struct VREG_ISP_DEHAZE_TMAP_18           TMAP_18;
	struct VREG_ISP_DEHAZE_TMAP_19           TMAP_19;
	struct VREG_ISP_DEHAZE_TMAP_20           TMAP_20;
	struct VREG_ISP_DEHAZE_TMAP_21           TMAP_21;
	struct VREG_ISP_DEHAZE_TMAP_22           TMAP_22;
	struct VREG_ISP_DEHAZE_TMAP_23           TMAP_23;
	struct VREG_ISP_DEHAZE_TMAP_24           TMAP_24;
	struct VREG_ISP_DEHAZE_TMAP_25           TMAP_25;
	struct VREG_ISP_DEHAZE_TMAP_26           TMAP_26;
	struct VREG_ISP_DEHAZE_TMAP_27           TMAP_27;
	struct VREG_ISP_DEHAZE_TMAP_28           TMAP_28;
	struct VREG_ISP_DEHAZE_TMAP_29           TMAP_29;
	struct VREG_ISP_DEHAZE_TMAP_30           TMAP_30;
	struct VREG_ISP_DEHAZE_TMAP_31           TMAP_31;
	struct VREG_ISP_DEHAZE_TMAP_32           TMAP_32;
	struct VREG_ISP_DEHAZE_O_DC_TH_UP_MODE   O_DC_TH_UP_MODE;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_RGB_DITHER_RGB_DITHER {
	union REG_ISP_RGB_DITHER_RGB_DITHER     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_RGB_DITHER_RGB_DITHER_DEBUG0 {
	union REG_ISP_RGB_DITHER_RGB_DITHER_DEBUG0  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_RGB_DITHER_T {
	struct VREG_ISP_RGB_DITHER_RGB_DITHER    RGB_DITHER;
	struct VREG_ISP_RGB_DITHER_RGB_DITHER_DEBUG0  RGB_DITHER_DEBUG0;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_CLUT_CTRL {
	union REG_ISP_CLUT_CTRL                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CLUT_PROG_ADDR {
	union REG_ISP_CLUT_PROG_ADDR            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CLUT_PROG_DATA {
	union REG_ISP_CLUT_PROG_DATA            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CLUT_PROG_RDATA {
	union REG_ISP_CLUT_PROG_RDATA           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CLUT_DBG {
	union REG_ISP_CLUT_DBG                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CLUT_DMY0 {
	union REG_ISP_CLUT_DMY0                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CLUT_DMY1 {
	union REG_ISP_CLUT_DMY1                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CLUT_DMY_R {
	union REG_ISP_CLUT_DMY_R                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CLUT_T {
	struct VREG_ISP_CLUT_CTRL                CLUT_CTRL;
	struct VREG_ISP_CLUT_PROG_ADDR           CLUT_PROG_ADDR;
	struct VREG_ISP_CLUT_PROG_DATA           CLUT_PROG_DATA;
	struct VREG_ISP_CLUT_PROG_RDATA          CLUT_PROG_RDATA;
	struct VREG_RESV                        _resv_0x10[4];
	struct VREG_ISP_CLUT_DBG                 CLUT_DBG;
	struct VREG_ISP_CLUT_DMY0                CLUT_DMY0;
	struct VREG_ISP_CLUT_DMY1                CLUT_DMY1;
	struct VREG_ISP_CLUT_DMY_R               CLUT_DMY_R;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_CSC_0 {
	union REG_ISP_CSC_0                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSC_1 {
	union REG_ISP_CSC_1                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSC_2 {
	union REG_ISP_CSC_2                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSC_3 {
	union REG_ISP_CSC_3                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSC_4 {
	union REG_ISP_CSC_4                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSC_5 {
	union REG_ISP_CSC_5                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSC_6 {
	union REG_ISP_CSC_6                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSC_7 {
	union REG_ISP_CSC_7                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSC_8 {
	union REG_ISP_CSC_8                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSC_9 {
	union REG_ISP_CSC_9                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CSC_T {
	struct VREG_ISP_CSC_0                    REG_0;
	struct VREG_ISP_CSC_1                    REG_1;
	struct VREG_ISP_CSC_2                    REG_2;
	struct VREG_ISP_CSC_3                    REG_3;
	struct VREG_ISP_CSC_4                    REG_4;
	struct VREG_ISP_CSC_5                    REG_5;
	struct VREG_ISP_CSC_6                    REG_6;
	struct VREG_ISP_CSC_7                    REG_7;
	struct VREG_ISP_CSC_8                    REG_8;
	struct VREG_ISP_CSC_9                    REG_9;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_YUV_TOP_YUV_0 {
	union REG_YUV_TOP_YUV_0                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_YUV_TOP_YUV_2 {
	union REG_YUV_TOP_YUV_2                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_YUV_TOP_YUV_3 {
	union REG_YUV_TOP_YUV_3                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_YUV_TOP_YUV_DEBUG_0 {
	union REG_YUV_TOP_YUV_DEBUG_0           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_YUV_TOP_YUV_4 {
	union REG_YUV_TOP_YUV_4                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_YUV_TOP_YUV_DEBUG_STATE {
	union REG_YUV_TOP_YUV_DEBUG_STATE       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_YUV_TOP_YUV_5 {
	union REG_YUV_TOP_YUV_5                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_YUV_TOP_YUV_CTRL {
	union REG_YUV_TOP_YUV_CTRL              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_YUV_TOP_IMGW_M1 {
	union REG_YUV_TOP_IMGW_M1               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_YUV_TOP_STVALID_STATUS {
	union REG_YUV_TOP_STVALID_STATUS        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_YUV_TOP_STREADY_STATUS {
	union REG_YUV_TOP_STREADY_STATUS        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_YUV_TOP_PATGEN1 {
	union REG_YUV_TOP_PATGEN1               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_YUV_TOP_PATGEN2 {
	union REG_YUV_TOP_PATGEN2               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_YUV_TOP_PATGEN3 {
	union REG_YUV_TOP_PATGEN3               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_YUV_TOP_PATGEN4 {
	union REG_YUV_TOP_PATGEN4               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_YUV_TOP_CHECK_SUM {
	union REG_YUV_TOP_CHECK_SUM             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_YUV_TOP_AI_ISP_RDMA_CTRL {
	union REG_YUV_TOP_AI_ISP_RDMA_CTRL      write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_YUV_TOP_AI_ISP_IMG_SIZE_Y {
	union REG_YUV_TOP_AI_ISP_IMG_SIZE_Y     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_YUV_TOP_AI_ISP_W_CROP_Y {
	union REG_YUV_TOP_AI_ISP_W_CROP_Y       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_YUV_TOP_AI_ISP_H_CROP_Y {
	union REG_YUV_TOP_AI_ISP_H_CROP_Y       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_YUV_TOP_AI_ISP_IMG_SIZE_UV {
	union REG_YUV_TOP_AI_ISP_IMG_SIZE_UV    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_YUV_TOP_AI_ISP_W_CROP_UV {
	union REG_YUV_TOP_AI_ISP_W_CROP_UV      write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_YUV_TOP_AI_ISP_H_CROP_UV {
	union REG_YUV_TOP_AI_ISP_H_CROP_UV      write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_YUV_TOP_YUV_CTRL_PHOBOS3 {
	union REG_YUV_TOP_YUV_CTRL_PHOBOS3        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_YUV_TOP_T {
	struct VREG_YUV_TOP_YUV_0                YUV_0;
	struct VREG_RESV                        _resv_0x4[1];
	struct VREG_YUV_TOP_YUV_2                YUV_2;
	struct VREG_YUV_TOP_YUV_3                YUV_3;
	struct VREG_YUV_TOP_YUV_DEBUG_0          YUV_DEBUG_0;
	struct VREG_YUV_TOP_YUV_4                YUV_4;
	struct VREG_YUV_TOP_YUV_DEBUG_STATE      YUV_DEBUG_STATE;
	struct VREG_RESV                        _resv_0x1c[1];
	struct VREG_YUV_TOP_YUV_5                YUV_5;
	struct VREG_RESV                        _resv_0x24[15];
	struct VREG_YUV_TOP_YUV_CTRL             YUV_CTRL;
	struct VREG_YUV_TOP_IMGW_M1              IMGW_M1;
	struct VREG_RESV                        _resv_0x68[1];
	struct VREG_YUV_TOP_STVALID_STATUS       STVALID_STATUS;
	struct VREG_YUV_TOP_STREADY_STATUS       STREADY_STATUS;
	struct VREG_YUV_TOP_PATGEN1              PATGEN1;
	struct VREG_YUV_TOP_PATGEN2              PATGEN2;
	struct VREG_YUV_TOP_PATGEN3              PATGEN3;
	struct VREG_YUV_TOP_PATGEN4              PATGEN4;
	struct VREG_YUV_TOP_CHECK_SUM            CHECK_SUM;
	struct VREG_YUV_TOP_AI_ISP_RDMA_CTRL     AI_ISP_RDMA_CTRL;
	struct VREG_YUV_TOP_AI_ISP_IMG_SIZE_Y    AI_ISP_IMG_SIZE_Y;
	struct VREG_YUV_TOP_AI_ISP_W_CROP_Y      AI_ISP_W_CROP_Y;
	struct VREG_YUV_TOP_AI_ISP_H_CROP_Y      AI_ISP_H_CROP_Y;
	struct VREG_YUV_TOP_AI_ISP_IMG_SIZE_UV   AI_ISP_IMG_SIZE_UV;
	struct VREG_YUV_TOP_AI_ISP_W_CROP_UV     AI_ISP_W_CROP_UV;
	struct VREG_YUV_TOP_AI_ISP_H_CROP_UV     AI_ISP_H_CROP_UV;
	struct VREG_YUV_TOP_YUV_CTRL_PHOBOS3       YUV_CTRL_PHOBOS3;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_CA_00 {
	union REG_ISP_CA_00                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CA_04 {
	union REG_ISP_CA_04                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CA_08 {
	union REG_ISP_CA_08                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CA_0C {
	union REG_ISP_CA_0C                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CA_10 {
	union REG_ISP_CA_10                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CA_T {
	struct VREG_ISP_CA_00                    REG_00;
	struct VREG_ISP_CA_04                    REG_04;
	struct VREG_ISP_CA_08                    REG_08;
	struct VREG_ISP_CA_0C                    REG_0C;
	struct VREG_ISP_CA_10                    REG_10;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_CA_LITE_00 {
	union REG_ISP_CA_LITE_00                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CA_LITE_04 {
	union REG_ISP_CA_LITE_04                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CA_LITE_08 {
	union REG_ISP_CA_LITE_08                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CA_LITE_0C {
	union REG_ISP_CA_LITE_0C                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CA_LITE_10 {
	union REG_ISP_CA_LITE_10                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CA_LITE_14 {
	union REG_ISP_CA_LITE_14                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CA_LITE_18 {
	union REG_ISP_CA_LITE_18                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CA_LITE_1C {
	union REG_ISP_CA_LITE_1C                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CA_LITE_20 {
	union REG_ISP_CA_LITE_20                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CA_LITE_24 {
	union REG_ISP_CA_LITE_24                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CA_LITE_T {
	struct VREG_ISP_CA_LITE_00               REG_00;
	struct VREG_ISP_CA_LITE_04               REG_04;
	struct VREG_ISP_CA_LITE_08               REG_08;
	struct VREG_ISP_CA_LITE_0C               REG_0C;
	struct VREG_ISP_CA_LITE_10               REG_10;
	struct VREG_ISP_CA_LITE_14               REG_14;
	struct VREG_ISP_CA_LITE_18               REG_18;
	struct VREG_ISP_CA_LITE_1C               REG_1C;
	struct VREG_ISP_CA_LITE_20               REG_20;
	struct VREG_ISP_CA_LITE_24               REG_24;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_EE_ADD_REG0 {
	union REG_ISP_EE_ADD_REG0                   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_OVERSHOOT_0 {
	union REG_ISP_EE_ADD_OVERSHOOT_0            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_OVERSHOOT_1 {
	union REG_ISP_EE_ADD_OVERSHOOT_1            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_OVERSHOOT_2 {
	union REG_ISP_EE_ADD_OVERSHOOT_2            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_OVERSHOOT_3 {
	union REG_ISP_EE_ADD_OVERSHOOT_3            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_OVERSHOOT_4 {
	union REG_ISP_EE_ADD_OVERSHOOT_4            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_REFINE_0 {
	union REG_ISP_EE_ADD_REFINE_0               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_REFINE_1 {
	union REG_ISP_EE_ADD_REFINE_1               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_REFINE_2 {
	union REG_ISP_EE_ADD_REFINE_2               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_REFINE_3 {
	union REG_ISP_EE_ADD_REFINE_3               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_REFINE_4 {
	union REG_ISP_EE_ADD_REFINE_4               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_REFINE_5 {
	union REG_ISP_EE_ADD_REFINE_5               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_REFINE_6 {
	union REG_ISP_EE_ADD_REFINE_6               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_REFINE_7 {
	union REG_ISP_EE_ADD_REFINE_7               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_REFINE_8 {
	union REG_ISP_EE_ADD_REFINE_8               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_REFINE_9 {
	union REG_ISP_EE_ADD_REFINE_9               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_CHROMA_0 {
	union REG_ISP_EE_ADD_CHROMA_0               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_CHROMA_1 {
	union REG_ISP_EE_ADD_CHROMA_1               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_CHROMA_2 {
	union REG_ISP_EE_ADD_CHROMA_2               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_CHROMA_3 {
	union REG_ISP_EE_ADD_CHROMA_3               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_CHROMA_4 {
	union REG_ISP_EE_ADD_CHROMA_4               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_CHROMA_5 {
	union REG_ISP_EE_ADD_CHROMA_5               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_CHROMA_6 {
	union REG_ISP_EE_ADD_CHROMA_6               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_CHROMA_7 {
	union REG_ISP_EE_ADD_CHROMA_7               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_CHROMA_8 {
	union REG_ISP_EE_ADD_CHROMA_8               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_CHROMA_9 {
	union REG_ISP_EE_ADD_CHROMA_9               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_SHADOW_RD_SEL {
	union REG_ISP_EE_ADD_SHADOW_RD_SEL          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_T {
	struct VREG_ISP_EE_ADD_REG0                  REG0;
	struct VREG_ISP_EE_ADD_OVERSHOOT_0           OVERSHOOT_0;
	struct VREG_ISP_EE_ADD_OVERSHOOT_1           OVERSHOOT_1;
	struct VREG_ISP_EE_ADD_OVERSHOOT_2           OVERSHOOT_2;
	struct VREG_ISP_EE_ADD_OVERSHOOT_3           OVERSHOOT_3;
	struct VREG_ISP_EE_ADD_OVERSHOOT_4           OVERSHOOT_4;
	struct VREG_ISP_EE_ADD_REFINE_0              REFINE_0;
	struct VREG_ISP_EE_ADD_REFINE_1              REFINE_1;
	struct VREG_ISP_EE_ADD_REFINE_2              REFINE_2;
	struct VREG_ISP_EE_ADD_REFINE_3              REFINE_3;
	struct VREG_ISP_EE_ADD_REFINE_4              REFINE_4;
	struct VREG_ISP_EE_ADD_REFINE_5              REFINE_5;
	struct VREG_ISP_EE_ADD_REFINE_6              REFINE_6;
	struct VREG_ISP_EE_ADD_REFINE_7              REFINE_7;
	struct VREG_ISP_EE_ADD_REFINE_8              REFINE_8;
	struct VREG_ISP_EE_ADD_REFINE_9              REFINE_9;
	struct VREG_ISP_EE_ADD_CHROMA_0              CHROMA_0;
	struct VREG_ISP_EE_ADD_CHROMA_1              CHROMA_1;
	struct VREG_ISP_EE_ADD_CHROMA_2              CHROMA_2;
	struct VREG_ISP_EE_ADD_CHROMA_3              CHROMA_3;
	struct VREG_ISP_EE_ADD_CHROMA_4              CHROMA_4;
	struct VREG_ISP_EE_ADD_CHROMA_5              CHROMA_5;
	struct VREG_ISP_EE_ADD_CHROMA_6              CHROMA_6;
	struct VREG_ISP_EE_ADD_CHROMA_7              CHROMA_7;
	struct VREG_ISP_EE_ADD_CHROMA_8              CHROMA_8;
	struct VREG_ISP_EE_ADD_CHROMA_9              CHROMA_9;
	struct VREG_ISP_EE_ADD_SHADOW_RD_SEL         SHADOW_RD_SEL;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_CNR_CTRL_HW_ONLY {
	union REG_ISP_CNR_CTRL_HW_ONLY          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_MMP_SCL_IN_SIZE {
	union REG_ISP_CNR_MMP_SCL_IN_SIZE       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_SCL_DOWN_CTRL {
	union REG_ISP_CNR_SCL_DOWN_CTRL         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_CTRL_SW_HW {
	union REG_ISP_CNR_CTRL_SW_HW            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_SUBIM_OUTSEL {
	union REG_ISP_CNR_SUBIM_OUTSEL          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_CMF_KSIZE {
	union REG_ISP_CNR_CMF_KSIZE             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_FSIZE_SEL {
	union REG_ISP_CNR_IFE2_FSIZE_SEL        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_EKSIZE_SEL {
	union REG_ISP_CNR_IFE2_EKSIZE_SEL       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_Y_RCTH {
	union REG_ISP_CNR_IFE2_Y_RCTH           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_Y_CWT {
	union REG_ISP_CNR_IFE2_Y_CWT            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_Y_RCWT {
	union REG_ISP_CNR_IFE2_Y_RCWT           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_Y_OUTL_TH {
	union REG_ISP_CNR_IFE2_Y_OUTL_TH        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_UV_RCTH {
	union REG_ISP_CNR_IFE2_UV_RCTH          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_UV_CWT {
	union REG_ISP_CNR_IFE2_UV_CWT           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_UV_RCWT {
	union REG_ISP_CNR_IFE2_UV_RCWT          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_UV_OUTL_TH {
	union REG_ISP_CNR_IFE2_UV_OUTL_TH       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_Y_OUTL_DTH {
	union REG_ISP_CNR_IFE2_Y_OUTL_DTH       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_U_OUTL_DTH {
	union REG_ISP_CNR_IFE2_U_OUTL_DTH       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_V_OUTL_DTH {
	union REG_ISP_CNR_IFE2_V_OUTL_DTH       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_ED_PN_TH {
	union REG_ISP_CNR_IFE2_ED_PN_TH         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_ED_HV_TH {
	union REG_ISP_CNR_IFE2_ED_HV_TH         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_Y_FTH {
	union REG_ISP_CNR_IFE2_Y_FTH            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_Y_FTH_4 {
	union REG_ISP_CNR_IFE2_Y_FTH_4          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_Y_FWT {
	union REG_ISP_CNR_IFE2_Y_FWT            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_U_FTH {
	union REG_ISP_CNR_IFE2_U_FTH            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_U_FTH_4 {
	union REG_ISP_CNR_IFE2_U_FTH_4          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_U_FWT {
	union REG_ISP_CNR_IFE2_U_FWT            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_V_FTH {
	union REG_ISP_CNR_IFE2_V_FTH            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_V_FTH_4 {
	union REG_ISP_CNR_IFE2_V_FTH_4          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_V_FWT {
	union REG_ISP_CNR_IFE2_V_FWT            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_YFTR_EN {
	union REG_ISP_CNR_IFE2_YFTR_EN          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_EGD_EN {
	union REG_ISP_CNR_IFE2_EGD_EN           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_IFE2_RC_EN {
	union REG_ISP_CNR_IFE2_RC_EN            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_CHRA_REFY_WT {
	union REG_ISP_CNR_CHRA_REFY_WT          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_CHRA_REFC_WT {
	union REG_ISP_CNR_CHRA_REFC_WT          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_CHRA_OUT_WT_LUT {
	union REG_ISP_CNR_CHRA_OUT_WT_LUT       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_CHRA_Y_RNG {
	union REG_ISP_CNR_CHRA_Y_RNG            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_CHRA_Y_WTPRC {
	union REG_ISP_CNR_CHRA_Y_WTPRC          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_CHRA_Y_TH {
	union REG_ISP_CNR_CHRA_Y_TH             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_CHRA_Y_WTS {
	union REG_ISP_CNR_CHRA_Y_WTS            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_CHRA_Y_WTE {
	union REG_ISP_CNR_CHRA_Y_WTE            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_CHRA_UV_RNG {
	union REG_ISP_CNR_CHRA_UV_RNG           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_CHRA_UV_WTPRC {
	union REG_ISP_CNR_CHRA_UV_WTPRC         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_CHRA_UV_TH {
	union REG_ISP_CNR_CHRA_UV_TH            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_CHRA_UV_WTS {
	union REG_ISP_CNR_CHRA_UV_WTS           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_CHRA_UV_WTE {
	union REG_ISP_CNR_CHRA_UV_WTE           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_CHRA_SAT_RNG {
	union REG_ISP_CNR_CHRA_SAT_RNG          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_CHRA_SAT_WTPRC {
	union REG_ISP_CNR_CHRA_SAT_WTPRC        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_CHRA_SAT_TH {
	union REG_ISP_CNR_CHRA_SAT_TH           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_CHRA_SAT_WTS {
	union REG_ISP_CNR_CHRA_SAT_WTS          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_CHRA_SAT_WTE {
	union REG_ISP_CNR_CHRA_SAT_WTE          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_CHRA_SAT_OUTBLD_CORING {
	union REG_ISP_CNR_CHRA_SAT_OUTBLD_CORING  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_LCA_H_SFACT {
	union REG_ISP_CNR_LCA_H_SFACT           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_LCA_V_SFACT {
	union REG_ISP_CNR_LCA_V_SFACT           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_LCA_H_SFACT_INIT_OFS {
	union REG_ISP_CNR_LCA_H_SFACT_INIT_OFS  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_LCA_V_SFACT_INIT_OFS {
	union REG_ISP_CNR_LCA_V_SFACT_INIT_OFS  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_LCA_SRC_IMG_SIZE_H {
	union REG_ISP_CNR_LCA_SRC_IMG_SIZE_H    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_LCA_SRC_IMG_SIZE_V {
	union REG_ISP_CNR_LCA_SRC_IMG_SIZE_V    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_LCA_SUB_IMG_SIZE_H {
	union REG_ISP_CNR_LCA_SUB_IMG_SIZE_H    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_LCA_SUB_IMG_SIZE_V {
	union REG_ISP_CNR_LCA_SUB_IMG_SIZE_V    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_LCA_HV_FILTMODE {
	union REG_ISP_CNR_LCA_HV_FILTMODE       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_LCA_HV_COEF {
	union REG_ISP_CNR_LCA_HV_COEF           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_UPSCL_H_SFACT {
	union REG_ISP_CNR_UPSCL_H_SFACT         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_UPSCL_V_SFACT {
	union REG_ISP_CNR_UPSCL_V_SFACT         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_UPSCL_H_SFACT_INIT_OFS {
	union REG_ISP_CNR_UPSCL_H_SFACT_INIT_OFS  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_UPSCL_V_SFACT_INIT_OFS {
	union REG_ISP_CNR_UPSCL_V_SFACT_INIT_OFS  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_UPSCL_HV_FILTMODE {
	union REG_ISP_CNR_UPSCL_HV_FILTMODE     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_UPSCL_HV_COEF {
	union REG_ISP_CNR_UPSCL_HV_COEF         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_SHDW_READ_SEL {
	union REG_ISP_CNR_SHDW_READ_SEL         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_CNR_T {
	struct VREG_ISP_CNR_CTRL_HW_ONLY         CNR_CTRL_HW_ONLY;
	struct VREG_ISP_CNR_MMP_SCL_IN_SIZE      MMP_SCL_IN_SIZE;
	struct VREG_ISP_CNR_SCL_DOWN_CTRL        SCL_DOWN_CTRL;
	struct VREG_ISP_CNR_CTRL_SW_HW           CNR_CTRL_SW_HW;
	struct VREG_ISP_CNR_SUBIM_OUTSEL         CNR_SUBIM_OUTSEL;
	struct VREG_ISP_CNR_CMF_KSIZE            CNR_CMF_KSIZE;
	struct VREG_ISP_CNR_IFE2_FSIZE_SEL       CNR_IFE2_FSIZE_SEL;
	struct VREG_ISP_CNR_IFE2_EKSIZE_SEL      CNR_IFE2_EKSIZE_SEL;
	struct VREG_ISP_CNR_IFE2_Y_RCTH          CNR_IFE2_Y_RCTH;
	struct VREG_ISP_CNR_IFE2_Y_CWT           CNR_IFE2_Y_CWT;
	struct VREG_ISP_CNR_IFE2_Y_RCWT          CNR_IFE2_Y_RCWT;
	struct VREG_ISP_CNR_IFE2_Y_OUTL_TH       CNR_IFE2_Y_OUTL_TH;
	struct VREG_ISP_CNR_IFE2_UV_RCTH         CNR_IFE2_UV_RCTH;
	struct VREG_ISP_CNR_IFE2_UV_CWT          CNR_IFE2_UV_CWT;
	struct VREG_ISP_CNR_IFE2_UV_RCWT         CNR_IFE2_UV_RCWT;
	struct VREG_ISP_CNR_IFE2_UV_OUTL_TH      CNR_IFE2_UV_OUTL_TH;
	struct VREG_ISP_CNR_IFE2_Y_OUTL_DTH      CNR_IFE2_Y_OUTL_DTH;
	struct VREG_ISP_CNR_IFE2_U_OUTL_DTH      CNR_IFE2_U_OUTL_DTH;
	struct VREG_ISP_CNR_IFE2_V_OUTL_DTH      CNR_IFE2_V_OUTL_DTH;
	struct VREG_ISP_CNR_IFE2_ED_PN_TH        CNR_IFE2_ED_PN_TH;
	struct VREG_ISP_CNR_IFE2_ED_HV_TH        CNR_IFE2_ED_HV_TH;
	struct VREG_ISP_CNR_IFE2_Y_FTH           CNR_IFE2_Y_FTH;
	struct VREG_ISP_CNR_IFE2_Y_FTH_4         CNR_IFE2_Y_FTH_4;
	struct VREG_ISP_CNR_IFE2_Y_FWT           CNR_IFE2_Y_FWT;
	struct VREG_ISP_CNR_IFE2_U_FTH           CNR_IFE2_U_FTH;
	struct VREG_ISP_CNR_IFE2_U_FTH_4         CNR_IFE2_U_FTH_4;
	struct VREG_ISP_CNR_IFE2_U_FWT           CNR_IFE2_U_FWT;
	struct VREG_ISP_CNR_IFE2_V_FTH           CNR_IFE2_V_FTH;
	struct VREG_ISP_CNR_IFE2_V_FTH_4         CNR_IFE2_V_FTH_4;
	struct VREG_ISP_CNR_IFE2_V_FWT           CNR_IFE2_V_FWT;
	struct VREG_ISP_CNR_IFE2_YFTR_EN         CNR_IFE2_YFTR_EN;
	struct VREG_ISP_CNR_IFE2_EGD_EN          CNR_IFE2_EGD_EN;
	struct VREG_ISP_CNR_IFE2_RC_EN           CNR_IFE2_RC_EN;
	struct VREG_ISP_CNR_CHRA_REFY_WT         CNR_CHRA_REFY_WT;
	struct VREG_ISP_CNR_CHRA_REFC_WT         CNR_CHRA_REFC_WT;
	struct VREG_ISP_CNR_CHRA_OUT_WT_LUT      CNR_CHRA_OUT_WT_LUT;
	struct VREG_ISP_CNR_CHRA_Y_RNG           CNR_CHRA_Y_RNG;
	struct VREG_ISP_CNR_CHRA_Y_WTPRC         CNR_CHRA_Y_WTPRC;
	struct VREG_ISP_CNR_CHRA_Y_TH            CNR_CHRA_Y_TH;
	struct VREG_ISP_CNR_CHRA_Y_WTS           CNR_CHRA_Y_WTS;
	struct VREG_ISP_CNR_CHRA_Y_WTE           CNR_CHRA_Y_WTE;
	struct VREG_ISP_CNR_CHRA_UV_RNG          CNR_CHRA_UV_RNG;
	struct VREG_ISP_CNR_CHRA_UV_WTPRC        CNR_CHRA_UV_WTPRC;
	struct VREG_ISP_CNR_CHRA_UV_TH           CNR_CHRA_UV_TH;
	struct VREG_ISP_CNR_CHRA_UV_WTS          CNR_CHRA_UV_WTS;
	struct VREG_ISP_CNR_CHRA_UV_WTE          CNR_CHRA_UV_WTE;
	struct VREG_ISP_CNR_CHRA_SAT_RNG         CNR_CHRA_SAT_RNG;
	struct VREG_ISP_CNR_CHRA_SAT_WTPRC       CNR_CHRA_SAT_WTPRC;
	struct VREG_ISP_CNR_CHRA_SAT_TH          CNR_CHRA_SAT_TH;
	struct VREG_ISP_CNR_CHRA_SAT_WTS         CNR_CHRA_SAT_WTS;
	struct VREG_ISP_CNR_CHRA_SAT_WTE         CNR_CHRA_SAT_WTE;
	struct VREG_ISP_CNR_CHRA_SAT_OUTBLD_CORING  CNR_CHRA_SAT_OUTBLD_CORING;
	struct VREG_RESV                        _resv_0xd0[12];
	struct VREG_ISP_CNR_LCA_H_SFACT          CNR_LCA_H_SFACT;
	struct VREG_ISP_CNR_LCA_V_SFACT          CNR_LCA_V_SFACT;
	struct VREG_ISP_CNR_LCA_H_SFACT_INIT_OFS  CNR_LCA_H_SFACT_INIT_OFS;
	struct VREG_ISP_CNR_LCA_V_SFACT_INIT_OFS  CNR_LCA_V_SFACT_INIT_OFS;
	struct VREG_ISP_CNR_LCA_SRC_IMG_SIZE_H   CNR_LCA_SRC_IMG_SIZE_H;
	struct VREG_ISP_CNR_LCA_SRC_IMG_SIZE_V   CNR_LCA_SRC_IMG_SIZE_V;
	struct VREG_ISP_CNR_LCA_SUB_IMG_SIZE_H   CNR_LCA_SUB_IMG_SIZE_H;
	struct VREG_ISP_CNR_LCA_SUB_IMG_SIZE_V   CNR_LCA_SUB_IMG_SIZE_V;
	struct VREG_ISP_CNR_LCA_HV_FILTMODE      CNR_LCA_HV_FILTMODE;
	struct VREG_ISP_CNR_LCA_HV_COEF          CNR_LCA_HV_COEF;
	struct VREG_ISP_CNR_UPSCL_H_SFACT        UPSCL_H_SFACT;
	struct VREG_ISP_CNR_UPSCL_V_SFACT        UPSCL_V_SFACT;
	struct VREG_ISP_CNR_UPSCL_H_SFACT_INIT_OFS  UPSCL_H_SFACT_INIT_OFS;
	struct VREG_ISP_CNR_UPSCL_V_SFACT_INIT_OFS  UPSCL_V_SFACT_INIT_OFS;
	struct VREG_ISP_CNR_UPSCL_HV_FILTMODE    UPSCL_HV_FILTMODE;
	struct VREG_ISP_CNR_UPSCL_HV_COEF        UPSCL_HV_COEF;
	struct VREG_ISP_CNR_SHDW_READ_SEL        SHDW_READ_SEL;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_EE_ADD_BACK_EE_ADD_B_REG0 {
	union REG_ISP_EE_ADD_BACK_EE_ADD_B_REG0     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_BACK_SHADOW_RD_SEL {
	union REG_ISP_EE_ADD_BACK_SHADOW_RD_SEL     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_ADD_BACK_T {
	struct VREG_ISP_EE_ADD_BACK_EE_ADD_B_REG0    EE_ADD_B_REG0;
	struct VREG_ISP_EE_ADD_BACK_SHADOW_RD_SEL    SHADOW_RD_SEL;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_YCURV_YCUR_CTRL {
	union REG_ISP_YCURV_YCUR_CTRL           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_YCURV_YCUR_PROG_CTRL {
	union REG_ISP_YCURV_YCUR_PROG_CTRL      write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_YCURV_YCUR_PROG_ST_ADDR {
	union REG_ISP_YCURV_YCUR_PROG_ST_ADDR   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_YCURV_YCUR_PROG_DATA {
	union REG_ISP_YCURV_YCUR_PROG_DATA      write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_YCURV_YCUR_PROG_MAX {
	union REG_ISP_YCURV_YCUR_PROG_MAX       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_YCURV_YCUR_MEM_SW_MODE {
	union REG_ISP_YCURV_YCUR_MEM_SW_MODE    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_YCURV_YCUR_MEM_SW_RDATA {
	union REG_ISP_YCURV_YCUR_MEM_SW_RDATA   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_YCURV_YCUR_DBG {
	union REG_ISP_YCURV_YCUR_DBG            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_YCURV_YCUR_DMY0 {
	union REG_ISP_YCURV_YCUR_DMY0           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_YCURV_YCUR_DMY1 {
	union REG_ISP_YCURV_YCUR_DMY1           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_YCURV_YCUR_DMY_R {
	union REG_ISP_YCURV_YCUR_DMY_R          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_YCURV_T {
	struct VREG_ISP_YCURV_YCUR_CTRL          YCUR_CTRL;
	struct VREG_ISP_YCURV_YCUR_PROG_CTRL     YCUR_PROG_CTRL;
	struct VREG_ISP_YCURV_YCUR_PROG_ST_ADDR  YCUR_PROG_ST_ADDR;
	struct VREG_ISP_YCURV_YCUR_PROG_DATA     YCUR_PROG_DATA;
	struct VREG_ISP_YCURV_YCUR_PROG_MAX      YCUR_PROG_MAX;
	struct VREG_ISP_YCURV_YCUR_MEM_SW_MODE   YCUR_SW_MODE;
	struct VREG_ISP_YCURV_YCUR_MEM_SW_RDATA  YCUR_SW_RDATA;
	struct VREG_RESV                        _resv_0x1c[1];
	struct VREG_ISP_YCURV_YCUR_DBG           YCUR_DBG;
	struct VREG_ISP_YCURV_YCUR_DMY0          YCUR_DMY0;
	struct VREG_ISP_YCURV_YCUR_DMY1          YCUR_DMY1;
	struct VREG_ISP_YCURV_YCUR_DMY_R         YCUR_DMY_R;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_444_422_0 {
	union REG_ISP_444_422_0                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_1 {
	union REG_ISP_444_422_1                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_2 {
	union REG_ISP_444_422_2                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_3 {
	union REG_ISP_444_422_3                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_4 {
	union REG_ISP_444_422_4                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_5 {
	union REG_ISP_444_422_5                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_6 {
	union REG_ISP_444_422_6                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_7 {
	union REG_ISP_444_422_7                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_8 {
	union REG_ISP_444_422_8                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_9 {
	union REG_ISP_444_422_9                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_10 {
	union REG_ISP_444_422_10                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_11 {
	union REG_ISP_444_422_11                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_12 {
	union REG_ISP_444_422_12                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_13 {
	union REG_ISP_444_422_13                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_14 {
	union REG_ISP_444_422_14                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_15 {
	union REG_ISP_444_422_15                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_16 {
	union REG_ISP_444_422_16                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_17 {
	union REG_ISP_444_422_17                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_18 {
	union REG_ISP_444_422_18                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_19 {
	union REG_ISP_444_422_19                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_20 {
	union REG_ISP_444_422_20                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_21 {
	union REG_ISP_444_422_21                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_22 {
	union REG_ISP_444_422_22                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_23 {
	union REG_ISP_444_422_23                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_24 {
	union REG_ISP_444_422_24                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_25 {
	union REG_ISP_444_422_25                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_26 {
	union REG_ISP_444_422_26                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_27 {
	union REG_ISP_444_422_27                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_28 {
	union REG_ISP_444_422_28                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_29 {
	union REG_ISP_444_422_29                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_30 {
	union REG_ISP_444_422_30                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_31 {
	union REG_ISP_444_422_31                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_32 {
	union REG_ISP_444_422_32                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_33 {
	union REG_ISP_444_422_33                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_34 {
	union REG_ISP_444_422_34                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_35 {
	union REG_ISP_444_422_35                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_36 {
	union REG_ISP_444_422_36                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_37 {
	union REG_ISP_444_422_37                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_38 {
	union REG_ISP_444_422_38                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_39 {
	union REG_ISP_444_422_39                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_40 {
	union REG_ISP_444_422_40                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_41 {
	union REG_ISP_444_422_41                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_42 {
	union REG_ISP_444_422_42                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_43 {
	union REG_ISP_444_422_43                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_44 {
	union REG_ISP_444_422_44                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_45 {
	union REG_ISP_444_422_45                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_46 {
	union REG_ISP_444_422_46                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_47 {
	union REG_ISP_444_422_47                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_48 {
	union REG_ISP_444_422_48                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_49 {
	union REG_ISP_444_422_49                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_50 {
	union REG_ISP_444_422_50                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_51 {
	union REG_ISP_444_422_51                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_52 {
	union REG_ISP_444_422_52                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_53 {
	union REG_ISP_444_422_53                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_54 {
	union REG_ISP_444_422_54                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_55 {
	union REG_ISP_444_422_55                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_56 {
	union REG_ISP_444_422_56                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_57 {
	union REG_ISP_444_422_57                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_58 {
	union REG_ISP_444_422_58                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_59 {
	union REG_ISP_444_422_59                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_60 {
	union REG_ISP_444_422_60                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_61 {
	union REG_ISP_444_422_61                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_62 {
	union REG_ISP_444_422_62                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_63 {
	union REG_ISP_444_422_63                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_64 {
	union REG_ISP_444_422_64                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_65 {
	union REG_ISP_444_422_65                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_66 {
	union REG_ISP_444_422_66                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_67 {
	union REG_ISP_444_422_67                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_68 {
	union REG_ISP_444_422_68                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_69 {
	union REG_ISP_444_422_69                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_70 {
	union REG_ISP_444_422_70                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_71 {
	union REG_ISP_444_422_71                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_72 {
	union REG_ISP_444_422_72                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_73 {
	union REG_ISP_444_422_73                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_74 {
	union REG_ISP_444_422_74                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_75 {
	union REG_ISP_444_422_75                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_76 {
	union REG_ISP_444_422_76                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_77 {
	union REG_ISP_444_422_77                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_78 {
	union REG_ISP_444_422_78                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_79 {
	union REG_ISP_444_422_79                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_80 {
	union REG_ISP_444_422_80                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_81 {
	union REG_ISP_444_422_81                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_82 {
	union REG_ISP_444_422_82                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_83 {
	union REG_ISP_444_422_83                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_84 {
	union REG_ISP_444_422_84                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_85 {
	union REG_ISP_444_422_85                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_86 {
	union REG_ISP_444_422_86                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_87 {
	union REG_ISP_444_422_87                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_88 {
	union REG_ISP_444_422_88                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_89 {
	union REG_ISP_444_422_89                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_90 {
	union REG_ISP_444_422_90                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_91 {
	union REG_ISP_444_422_91                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_92 {
	union REG_ISP_444_422_92                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_93 {
	union REG_ISP_444_422_93                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_94 {
	union REG_ISP_444_422_94                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_95 {
	union REG_ISP_444_422_95                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_96 {
	union REG_ISP_444_422_96                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_97 {
	union REG_ISP_444_422_97                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_98 {
	union REG_ISP_444_422_98                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_99 {
	union REG_ISP_444_422_99                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_100 {
	union REG_ISP_444_422_100               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_101 {
	union REG_ISP_444_422_101               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_102 {
	union REG_ISP_444_422_102               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_103 {
	union REG_ISP_444_422_103               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_104 {
	union REG_ISP_444_422_104               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_105 {
	union REG_ISP_444_422_105               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_106 {
	union REG_ISP_444_422_106               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_107 {
	union REG_ISP_444_422_107               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_108 {
	union REG_ISP_444_422_108               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_109 {
	union REG_ISP_444_422_109               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_110 {
	union REG_ISP_444_422_110               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_111 {
	union REG_ISP_444_422_111               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_112 {
	union REG_ISP_444_422_112               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_113 {
	union REG_ISP_444_422_113               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_114 {
	union REG_ISP_444_422_114               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_115 {
	union REG_ISP_444_422_115               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_116 {
	union REG_ISP_444_422_116               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_444_422_T {
	struct VREG_ISP_444_422_0                REG_0;
	struct VREG_ISP_444_422_1                REG_1;
	struct VREG_ISP_444_422_2                REG_2;
	struct VREG_ISP_444_422_3                REG_3;
	struct VREG_ISP_444_422_4                REG_4;
	struct VREG_ISP_444_422_5                REG_5;
	struct VREG_ISP_444_422_6                REG_6;
	struct VREG_ISP_444_422_7                REG_7;
	struct VREG_ISP_444_422_8                REG_8;
	struct VREG_ISP_444_422_9                REG_9;
	struct VREG_ISP_444_422_10               REG_10;
	struct VREG_ISP_444_422_11               REG_11;
	struct VREG_ISP_444_422_12               REG_12;
	struct VREG_ISP_444_422_13               REG_13;
	struct VREG_ISP_444_422_14               REG_14;
	struct VREG_ISP_444_422_15               REG_15;
	struct VREG_ISP_444_422_16               REG_16;
	struct VREG_ISP_444_422_17               REG_17;
	struct VREG_ISP_444_422_18               REG_18;
	struct VREG_ISP_444_422_19               REG_19;
	struct VREG_ISP_444_422_20               REG_20;
	struct VREG_ISP_444_422_21               REG_21;
	struct VREG_ISP_444_422_22               REG_22;
	struct VREG_ISP_444_422_23               REG_23;
	struct VREG_ISP_444_422_24               REG_24;
	struct VREG_ISP_444_422_25               REG_25;
	struct VREG_ISP_444_422_26               REG_26;
	struct VREG_ISP_444_422_27               REG_27;
	struct VREG_ISP_444_422_28               REG_28;
	struct VREG_ISP_444_422_29               REG_29;
	struct VREG_ISP_444_422_30               REG_30;
	struct VREG_ISP_444_422_31               REG_31;
	struct VREG_ISP_444_422_32               REG_32;
	struct VREG_ISP_444_422_33               REG_33;
	struct VREG_ISP_444_422_34               REG_34;
	struct VREG_ISP_444_422_35               REG_35;
	struct VREG_ISP_444_422_36               REG_36;
	struct VREG_ISP_444_422_37               REG_37;
	struct VREG_ISP_444_422_38               REG_38;
	struct VREG_ISP_444_422_39               REG_39;
	struct VREG_ISP_444_422_40               REG_40;
	struct VREG_ISP_444_422_41               REG_41;
	struct VREG_ISP_444_422_42               REG_42;
	struct VREG_ISP_444_422_43               REG_43;
	struct VREG_ISP_444_422_44               REG_44;
	struct VREG_ISP_444_422_45               REG_45;
	struct VREG_ISP_444_422_46               REG_46;
	struct VREG_ISP_444_422_47               REG_47;
	struct VREG_ISP_444_422_48               REG_48;
	struct VREG_ISP_444_422_49               REG_49;
	struct VREG_ISP_444_422_50               REG_50;
	struct VREG_ISP_444_422_51               REG_51;
	struct VREG_ISP_444_422_52               REG_52;
	struct VREG_ISP_444_422_53               REG_53;
	struct VREG_ISP_444_422_54               REG_54;
	struct VREG_ISP_444_422_55               REG_55;
	struct VREG_ISP_444_422_56               REG_56;
	struct VREG_ISP_444_422_57               REG_57;
	struct VREG_ISP_444_422_58               REG_58;
	struct VREG_ISP_444_422_59               REG_59;
	struct VREG_ISP_444_422_60               REG_60;
	struct VREG_ISP_444_422_61               REG_61;
	struct VREG_ISP_444_422_62               REG_62;
	struct VREG_ISP_444_422_63               REG_63;
	struct VREG_ISP_444_422_64               REG_64;
	struct VREG_ISP_444_422_65               REG_65;
	struct VREG_ISP_444_422_66               REG_66;
	struct VREG_ISP_444_422_67               REG_67;
	struct VREG_ISP_444_422_68               REG_68;
	struct VREG_ISP_444_422_69               REG_69;
	struct VREG_ISP_444_422_70               REG_70;
	struct VREG_ISP_444_422_71               REG_71;
	struct VREG_ISP_444_422_72               REG_72;
	struct VREG_ISP_444_422_73               REG_73;
	struct VREG_ISP_444_422_74               REG_74;
	struct VREG_ISP_444_422_75               REG_75;
	struct VREG_ISP_444_422_76               REG_76;
	struct VREG_ISP_444_422_77               REG_77;
	struct VREG_ISP_444_422_78               REG_78;
	struct VREG_ISP_444_422_79               REG_79;
	struct VREG_ISP_444_422_80               REG_80;
	struct VREG_ISP_444_422_81               REG_81;
	struct VREG_ISP_444_422_82               REG_82;
	struct VREG_ISP_444_422_83               REG_83;
	struct VREG_ISP_444_422_84               REG_84;
	struct VREG_ISP_444_422_85               REG_85;
	struct VREG_ISP_444_422_86               REG_86;
	struct VREG_ISP_444_422_87               REG_87;
	struct VREG_ISP_444_422_88               REG_88;
	struct VREG_ISP_444_422_89               REG_89;
	struct VREG_ISP_444_422_90               REG_90;
	struct VREG_ISP_444_422_91               REG_91;
	struct VREG_ISP_444_422_92               REG_92;
	struct VREG_ISP_444_422_93               REG_93;
	struct VREG_ISP_444_422_94               REG_94;
	struct VREG_ISP_444_422_95               REG_95;
	struct VREG_ISP_444_422_96               REG_96;
	struct VREG_ISP_444_422_97               REG_97;
	struct VREG_ISP_444_422_98               REG_98;
	struct VREG_ISP_444_422_99               REG_99;
	struct VREG_ISP_444_422_100              REG_100;
	struct VREG_ISP_444_422_101              REG_101;
	struct VREG_ISP_444_422_102              REG_102;
	struct VREG_ISP_444_422_103              REG_103;
	struct VREG_ISP_444_422_104              REG_104;
	struct VREG_ISP_444_422_105              REG_105;
	struct VREG_ISP_444_422_106              REG_106;
	struct VREG_ISP_444_422_107              REG_107;
	struct VREG_ISP_444_422_108              REG_108;
	struct VREG_ISP_444_422_109              REG_109;
	struct VREG_ISP_444_422_110              REG_110;
	struct VREG_ISP_444_422_111              REG_111;
	struct VREG_ISP_444_422_112              REG_112;
	struct VREG_ISP_444_422_113              REG_113;
	struct VREG_ISP_444_422_114              REG_114;
	struct VREG_ISP_444_422_115              REG_115;
	struct VREG_ISP_444_422_116              REG_116;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_YUV_DITHER_Y_DITHER {
	union REG_ISP_YUV_DITHER_Y_DITHER       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_YUV_DITHER_UV_DITHER {
	union REG_ISP_YUV_DITHER_UV_DITHER      write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_YUV_DITHER_DEBUG_00 {
	union REG_ISP_YUV_DITHER_DEBUG_00       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_YUV_DITHER_DEBUG_01 {
	union REG_ISP_YUV_DITHER_DEBUG_01       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_YUV_DITHER_T {
	struct VREG_ISP_YUV_DITHER_Y_DITHER      Y_DITHER;
	struct VREG_ISP_YUV_DITHER_UV_DITHER     UV_DITHER;
	struct VREG_ISP_YUV_DITHER_DEBUG_00      DEBUG_00;
	struct VREG_ISP_YUV_DITHER_DEBUG_01      DEBUG_01;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_LDCI_CTL {
	union REG_ISP_LDCI_CTL                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LDCI_RECIPROCAL_0 {
	union REG_ISP_LDCI_RECIPROCAL_0         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LDCI_LUMA_PROT_LUT0 {
	union REG_ISP_LDCI_LUMA_PROT_LUT0       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LDCI_LUMA_PROT_LUT1 {
	union REG_ISP_LDCI_LUMA_PROT_LUT1       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LDCI_LUMA_PROT_LUT2 {
	union REG_ISP_LDCI_LUMA_PROT_LUT2       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LDCI_LUMA_PROT_LUT3 {
	union REG_ISP_LDCI_LUMA_PROT_LUT3       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LDCI_LUMA_PROT_LUT4 {
	union REG_ISP_LDCI_LUMA_PROT_LUT4       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LDCI_BLK_NUM {
	union REG_ISP_LDCI_BLK_NUM              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LDCI_BLK_SIZE {
	union REG_ISP_LDCI_BLK_SIZE             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LDCI_RECIPROCAL_1 {
	union REG_ISP_LDCI_RECIPROCAL_1         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LDCI_LUMA_GAIN_LUT0 {
	union REG_ISP_LDCI_LUMA_GAIN_LUT0       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LDCI_LUMA_GAIN_LUT1 {
	union REG_ISP_LDCI_LUMA_GAIN_LUT1       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LDCI_LUMA_GAIN_LUT2 {
	union REG_ISP_LDCI_LUMA_GAIN_LUT2       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LDCI_THR {
	union REG_ISP_LDCI_THR                  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LDCI_DIFF_GAIN_LUT_L0 {
	union REG_ISP_LDCI_DIFF_GAIN_LUT_L0     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LDCI_DIFF_GAIN_LUT_L1 {
	union REG_ISP_LDCI_DIFF_GAIN_LUT_L1     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LDCI_DIFF_GAIN_LUT_R0 {
	union REG_ISP_LDCI_DIFF_GAIN_LUT_R0     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LDCI_DIFF_GAIN_LUT_R1 {
	union REG_ISP_LDCI_DIFF_GAIN_LUT_R1     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LDCI_ROI_CTL {
	union REG_ISP_LDCI_ROI_CTL              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LDCI_ROI_AXIS_0 {
	union REG_ISP_LDCI_ROI_AXIS_0           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LDCI_ROI_AXIS_1 {
	union REG_ISP_LDCI_ROI_AXIS_1           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LDCI_HW_CTL {
	union REG_ISP_LDCI_HW_CTL               write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LDCI_DMA_STAT {
	union REG_ISP_LDCI_DMA_STAT             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LDCI_CROP_WIDTH {
	union REG_ISP_LDCI_CROP_WIDTH           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LDCI_CROP_HEIGHT {
	union REG_ISP_LDCI_CROP_HEIGHT          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LDCI_CROP_ENABLE {
	union REG_ISP_LDCI_CROP_ENABLE          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_LDCI_T {
	struct VREG_ISP_LDCI_CTL                 LDCI_CTL;
	struct VREG_ISP_LDCI_RECIPROCAL_0        LDCI_RECIPROCAL_0;
	struct VREG_ISP_LDCI_LUMA_PROT_LUT0      LDCI_LUMA_PROT_LUT0;
	struct VREG_ISP_LDCI_LUMA_PROT_LUT1      LDCI_LUMA_PROT_LUT1;
	struct VREG_ISP_LDCI_LUMA_PROT_LUT2      LDCI_LUMA_PROT_LUT2;
	struct VREG_ISP_LDCI_LUMA_PROT_LUT3      LDCI_LUMA_PROT_LUT3;
	struct VREG_ISP_LDCI_LUMA_PROT_LUT4      LDCI_LUMA_PROT_LUT4;
	struct VREG_ISP_LDCI_BLK_NUM             LDCI_BLK_NUM;
	struct VREG_ISP_LDCI_BLK_SIZE            LDCI_BLK_SIZE;
	struct VREG_ISP_LDCI_RECIPROCAL_1        LDCI_RECIPROCAL_1;
	struct VREG_RESV                        _resv_0x28[1];
	struct VREG_ISP_LDCI_LUMA_GAIN_LUT0      LDCI_LUMA_GAIN_LUT0;
	struct VREG_ISP_LDCI_LUMA_GAIN_LUT1      LDCI_LUMA_GAIN_LUT1;
	struct VREG_ISP_LDCI_LUMA_GAIN_LUT2      LDCI_LUMA_GAIN_LUT2;
	struct VREG_ISP_LDCI_THR                 LDCI_THR;
	struct VREG_ISP_LDCI_DIFF_GAIN_LUT_L0    LDCI_DIFF_GAIN_LUT_L0;
	struct VREG_ISP_LDCI_DIFF_GAIN_LUT_L1    LDCI_DIFF_GAIN_LUT_L1;
	struct VREG_ISP_LDCI_DIFF_GAIN_LUT_R0    LDCI_DIFF_GAIN_LUT_R0;
	struct VREG_ISP_LDCI_DIFF_GAIN_LUT_R1    LDCI_DIFF_GAIN_LUT_R1;
	struct VREG_ISP_LDCI_ROI_CTL             LDCI_ROI_CTL;
	struct VREG_ISP_LDCI_ROI_AXIS_0          LDCI_ROI_AXIS_0;
	struct VREG_ISP_LDCI_ROI_AXIS_1          LDCI_ROI_AXIS_1;
	struct VREG_ISP_LDCI_HW_CTL              LDCI_HW_CTL;
	struct VREG_ISP_LDCI_DMA_STAT            LDCI_DMA_STAT;
	struct VREG_ISP_LDCI_CROP_WIDTH          LDCI_CROP_WIDTH;
	struct VREG_ISP_LDCI_CROP_HEIGHT         LDCI_CROP_HEIGHT;
	struct VREG_ISP_LDCI_CROP_ENABLE         LDCI_CROP_ENABLE;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_LDCI_MAP_LUT_LDCI_MAP_CTL {
	union REG_LDCI_MAP_LUT_LDCI_MAP_CTL     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_LDCI_MAP_LUT_LDCI_RW_CTL {
	union REG_LDCI_MAP_LUT_LDCI_RW_CTL      write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_LDCI_MAP_LUT_LDCI_MAP_SW_CTL {
	union REG_LDCI_MAP_LUT_LDCI_MAP_SW_CTL  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_LDCI_MAP_LUT_T {
	struct VREG_LDCI_MAP_LUT_LDCI_MAP_CTL    LDCI_MAP_CTL;
	struct VREG_LDCI_MAP_LUT_LDCI_RW_CTL     LDCI_RW_CTL;
	struct VREG_LDCI_MAP_LUT_LDCI_MAP_SW_CTL  LDCI_MAP_SW_CTL;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_EE_00 {
	union REG_ISP_EE_00                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_04 {
	union REG_ISP_EE_04                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_0C {
	union REG_ISP_EE_0C                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_10 {
	union REG_ISP_EE_10                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_A4 {
	union REG_ISP_EE_A4                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_A8 {
	union REG_ISP_EE_A8                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_AC {
	union REG_ISP_EE_AC                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_B0 {
	union REG_ISP_EE_B0                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_B4 {
	union REG_ISP_EE_B4                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_B8 {
	union REG_ISP_EE_B8                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_BC {
	union REG_ISP_EE_BC                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_C0 {
	union REG_ISP_EE_C0                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_C4 {
	union REG_ISP_EE_C4                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_C8 {
	union REG_ISP_EE_C8                     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_HCC {
	union REG_ISP_EE_HCC                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_HD0 {
	union REG_ISP_EE_HD0                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_HD4 {
	union REG_ISP_EE_HD4                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_HD8 {
	union REG_ISP_EE_HD8                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_178 {
	union REG_ISP_EE_178                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_17C {
	union REG_ISP_EE_17C                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_180 {
	union REG_ISP_EE_180                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_184 {
	union REG_ISP_EE_184                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_188 {
	union REG_ISP_EE_188                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_18C {
	union REG_ISP_EE_18C                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_190 {
	union REG_ISP_EE_190                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_194 {
	union REG_ISP_EE_194                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_198 {
	union REG_ISP_EE_198                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_19C {
	union REG_ISP_EE_19C                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_1A0 {
	union REG_ISP_EE_1A0                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_1A4 {
	union REG_ISP_EE_1A4                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_1A8 {
	union REG_ISP_EE_1A8                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_EE_T {
	struct VREG_ISP_EE_00                    REG_00;
	struct VREG_ISP_EE_04                    REG_04;
	struct VREG_RESV                        _resv_0x8[1];
	struct VREG_ISP_EE_0C                    REG_0C;
	struct VREG_ISP_EE_10                    REG_10;
	struct VREG_RESV                        _resv_0x14[36];
	struct VREG_ISP_EE_A4                    REG_A4;
	struct VREG_ISP_EE_A8                    REG_A8;
	struct VREG_ISP_EE_AC                    REG_AC;
	struct VREG_ISP_EE_B0                    REG_B0;
	struct VREG_ISP_EE_B4                    REG_B4;
	struct VREG_ISP_EE_B8                    REG_B8;
	struct VREG_ISP_EE_BC                    REG_BC;
	struct VREG_ISP_EE_C0                    REG_C0;
	struct VREG_ISP_EE_C4                    REG_C4;
	struct VREG_ISP_EE_C8                    REG_C8;
	struct VREG_ISP_EE_HCC                   REG_HCC;
	struct VREG_ISP_EE_HD0                   REG_HD0;
	struct VREG_ISP_EE_HD4                   REG_HD4;
	struct VREG_ISP_EE_HD8                   REG_HD8;
	struct VREG_RESV                        _resv_0xdc[39];
	struct VREG_ISP_EE_178                   REG_178;
	struct VREG_ISP_EE_17C                   REG_17C;
	struct VREG_ISP_EE_180                   REG_180;
	struct VREG_ISP_EE_184                   REG_184;
	struct VREG_ISP_EE_188                   REG_188;
	struct VREG_ISP_EE_18C                   REG_18C;
	struct VREG_ISP_EE_190                   REG_190;
	struct VREG_ISP_EE_194                   REG_194;
	struct VREG_ISP_EE_198                   REG_198;
	struct VREG_ISP_EE_19C                   REG_19C;
	struct VREG_ISP_EE_1A0                   REG_1A0;
	struct VREG_ISP_EE_1A4                   REG_1A4;
	struct VREG_ISP_EE_1A8                   REG_1A8;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_ISP_TOP_INT_EVENT0 {
	union REG_ISP_TOP_INT_EVENT0            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_INT_EVENT1 {
	union REG_ISP_TOP_INT_EVENT1            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_INT_EVENT2 {
	union REG_ISP_TOP_INT_EVENT2            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_ERROR_STS {
	union REG_ISP_TOP_ERROR_STS             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_INT_EVENT0_EN {
	union REG_ISP_TOP_INT_EVENT0_EN         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_INT_EVENT1_EN {
	union REG_ISP_TOP_INT_EVENT1_EN         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_INT_EVENT2_EN {
	union REG_ISP_TOP_INT_EVENT2_EN         write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_SW_CTRL_0 {
	union REG_ISP_TOP_SW_CTRL_0             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_SW_CTRL_1 {
	union REG_ISP_TOP_SW_CTRL_1             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_CTRL_MODE_SEL0 {
	union REG_ISP_TOP_CTRL_MODE_SEL0        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_CTRL_MODE_SEL1 {
	union REG_ISP_TOP_CTRL_MODE_SEL1        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_SCENARIOS_CTRL {
	union REG_ISP_TOP_SCENARIOS_CTRL        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_SW_RST {
	union REG_ISP_TOP_SW_RST                write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_BLK_IDLE {
	union REG_ISP_TOP_BLK_IDLE              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_BLK_IDLE_ENABLE {
	union REG_ISP_TOP_BLK_IDLE_ENABLE       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_DBUS0 {
	union REG_ISP_TOP_DBUS0                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_DBUS1 {
	union REG_ISP_TOP_DBUS1                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_DBUS2 {
	union REG_ISP_TOP_DBUS2                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_DBUS3 {
	union REG_ISP_TOP_DBUS3                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_FORCE_INT {
	union REG_ISP_TOP_FORCE_INT             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_DUMMY {
	union REG_ISP_TOP_DUMMY                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_DBUS4 {
	union REG_ISP_TOP_DBUS4                 write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_IP_ENABLE1 {
	union REG_ISP_TOP_IP_ENABLE1            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_IP_ENABLE2 {
	union REG_ISP_TOP_IP_ENABLE2            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_IP_ENABLE3 {
	union REG_ISP_TOP_IP_ENABLE3            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_CMDQ_CTRL {
	union REG_ISP_TOP_CMDQ_CTRL             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_CMDQ_TRIG {
	union REG_ISP_TOP_CMDQ_TRIG             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_TRIG_CNT {
	union REG_ISP_TOP_TRIG_CNT              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_SVN_VERSION {
	union REG_ISP_TOP_SVN_VERSION           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_TIMESTAMP {
	union REG_ISP_TOP_TIMESTAMP             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_SCLIE_ENABLE {
	union REG_ISP_TOP_SCLIE_ENABLE          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_W_SLICE_THRESH_MAIN {
	union REG_ISP_TOP_W_SLICE_THRESH_MAIN   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_R_SLICE_THRESH_MAIN {
	union REG_ISP_TOP_R_SLICE_THRESH_MAIN   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_VI_SEL_FRAME_VALID {
	union REG_ISP_TOP_VI_SEL_FRAME_VALID    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_FIRST_FRAME {
	union REG_ISP_TOP_FIRST_FRAME           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_INT_EVENT0_LINE_SPLITER {
	union REG_ISP_TOP_INT_EVENT0_LINE_SPLITER  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_INT_EVENT0_EN_LINE_SPLITER {
	union REG_ISP_TOP_INT_EVENT0_EN_LINE_SPLITER  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_BLK_IDLE_1 {
	union REG_ISP_TOP_BLK_IDLE_1            write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_ISP_TOP_T {
	struct VREG_ISP_TOP_INT_EVENT0           INT_EVENT0;
	struct VREG_ISP_TOP_INT_EVENT1           INT_EVENT1;
	struct VREG_ISP_TOP_INT_EVENT2           INT_EVENT2;
	struct VREG_ISP_TOP_ERROR_STS            ERROR_STS;
	struct VREG_ISP_TOP_INT_EVENT0_EN        INT_EVENT0_EN;
	struct VREG_ISP_TOP_INT_EVENT1_EN        INT_EVENT1_EN;
	struct VREG_ISP_TOP_INT_EVENT2_EN        INT_EVENT2_EN;
	struct VREG_RESV                        _resv_0x1c[1];
	struct VREG_ISP_TOP_SW_CTRL_0            SW_CTRL_0;
	struct VREG_ISP_TOP_SW_CTRL_1            SW_CTRL_1;
	struct VREG_ISP_TOP_CTRL_MODE_SEL0       CTRL_MODE_SEL0;
	struct VREG_ISP_TOP_CTRL_MODE_SEL1       CTRL_MODE_SEL1;
	struct VREG_ISP_TOP_SCENARIOS_CTRL       SCENARIOS_CTRL;
	struct VREG_ISP_TOP_SW_RST               SW_RST;
	struct VREG_ISP_TOP_BLK_IDLE             BLK_IDLE;
	struct VREG_ISP_TOP_BLK_IDLE_ENABLE      BLK_IDLE_ENABLE;
	struct VREG_ISP_TOP_DBUS0                DBUS0;
	struct VREG_ISP_TOP_DBUS1                DBUS1;
	struct VREG_ISP_TOP_DBUS2                DBUS2;
	struct VREG_ISP_TOP_DBUS3                DBUS3;
	struct VREG_ISP_TOP_FORCE_INT            FORCE_INT;
	struct VREG_ISP_TOP_DUMMY                DUMMY;
	struct VREG_ISP_TOP_DBUS4                DBUS4;
	struct VREG_ISP_TOP_IP_ENABLE1           IP_ENABLE1;
	struct VREG_ISP_TOP_IP_ENABLE2           IP_ENABLE2;
	struct VREG_ISP_TOP_IP_ENABLE3           IP_ENABLE3;
	struct VREG_ISP_TOP_CMDQ_CTRL            CMDQ_CTRL;
	struct VREG_ISP_TOP_CMDQ_TRIG            CMDQ_TRIG;
	struct VREG_ISP_TOP_TRIG_CNT             TRIG_CNT;
	struct VREG_ISP_TOP_SVN_VERSION          SVN_VERSION;
	struct VREG_ISP_TOP_TIMESTAMP            TIMESTAMP;
	struct VREG_RESV                        _resv_0x7c[1];
	struct VREG_ISP_TOP_SCLIE_ENABLE         SCLIE_ENABLE;
	struct VREG_ISP_TOP_W_SLICE_THRESH_MAIN  W_SLICE_THRESH_MAIN;
	struct VREG_RESV                        _resv_0x88[2];
	struct VREG_ISP_TOP_R_SLICE_THRESH_MAIN  R_SLICE_THRESH_MAIN;
	struct VREG_RESV                        _resv_0x94[2];
	struct VREG_ISP_TOP_VI_SEL_FRAME_VALID   VI_SEL_FRAME_VALID;
	struct VREG_ISP_TOP_FIRST_FRAME          FIRST_FRAME;
	struct VREG_ISP_TOP_INT_EVENT0_LINE_SPLITER  INT_EVENT0_LINE_SPLITER;
	struct VREG_RESV                        _resv_0xa8[3];
	struct VREG_ISP_TOP_INT_EVENT0_EN_LINE_SPLITER  INT_EVENT0_EN_LINE_SPLITER;
	struct VREG_RESV                        _resv_0xb8[7];
	struct VREG_ISP_TOP_BLK_IDLE_1           BLK_IDLE_1;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_RDMA_CORE_SHADOW_RD_SEL   {
	union REG_RDMA_CORE_SHADOW_RD_SEL       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_IP_DISABLE {
	union REG_RDMA_CORE_IP_DISABLE          write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_UP_RING_BASE {
	union REG_RDMA_CORE_UP_RING_BASE        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NORM_STATUS0 {
	union REG_RDMA_CORE_NORM_STATUS0        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NORM_STATUS1 {
	union REG_RDMA_CORE_NORM_STATUS1        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_EN {
	union REG_RDMA_CORE_RING_BUFFER_EN      write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NORM_PERF  {
	union REG_RDMA_CORE_NORM_PERF           write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_AR_PRIORITY_SEL {
	union REG_RDMA_CORE_AR_PRIORITY_SEL     write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_PATCH_ENABLE {
	union REG_RDMA_CORE_RING_PATCH_ENABLE   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_SET_RING_BASE {
	union REG_RDMA_CORE_SET_RING_BASE       write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BASE_ADDR_L {
	union REG_RDMA_CORE_RING_BASE_ADDR_L    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BASE_ADDR_H {
	union REG_RDMA_CORE_RING_BASE_ADDR_H    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE0 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE0   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE1 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE1   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE2 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE2   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE3 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE3   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE4 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE4   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE5 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE5   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE6 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE6   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE7 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE7   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE8 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE8   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE9 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE9   write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE10 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE10  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE11 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE11  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE12 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE12  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE13 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE13  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE14 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE14  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE15 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE15  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE16 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE16  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE17 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE17  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE18 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE18  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE19 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE19  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE20 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE20  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE21 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE21  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE22 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE22  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE23 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE23  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE24 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE24  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE25 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE25  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE26 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE26  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE27 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE27  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE28 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE28  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE29 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE29  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE30 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE30  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_RING_BUFFER_SIZE31 {
	union REG_RDMA_CORE_RING_BUFFER_SIZE31  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS0 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS0  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS1 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS1  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS2 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS2  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS3 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS3  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS4 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS4  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS5 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS5  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS6 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS6  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS7 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS7  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS8 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS8  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS9 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS9  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS10 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS10  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS11 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS11  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS12 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS12  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS13 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS13  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS14 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS14  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS15 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS15  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS16 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS16  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS17 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS17  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS18 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS18  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS19 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS19  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS20 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS20  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS21 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS21  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS22 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS22  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS23 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS23  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS24 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS24  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS25 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS25  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS26 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS26  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS27 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS27  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS28 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS28  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS29 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS29  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS30 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS30  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS31 {
	union REG_RDMA_CORE_NEXT_DMA_ADDR_STS31  write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_RDMA_CORE_T {
	struct VREG_RDMA_CORE_SHADOW_RD_SEL      SHADOW_RD_SEL;
	struct VREG_RDMA_CORE_IP_DISABLE         IP_DISABLE;
	struct VREG_RESV                        _resv_0x8[1];
	struct VREG_RDMA_CORE_UP_RING_BASE       UP_RING_BASE;
	struct VREG_RDMA_CORE_NORM_STATUS0       NORM_STATUS0;
	struct VREG_RDMA_CORE_NORM_STATUS1       NORM_STATUS1;
	struct VREG_RDMA_CORE_RING_BUFFER_EN     RING_BUFFER_EN;
	struct VREG_RESV                        _resv_0x1c[1];
	struct VREG_RDMA_CORE_NORM_PERF          NORM_PERF;
	struct VREG_RDMA_CORE_AR_PRIORITY_SEL    AR_PRIORITY_SEL;
	struct VREG_RDMA_CORE_RING_PATCH_ENABLE  RING_PATCH_ENABLE;
	struct VREG_RDMA_CORE_SET_RING_BASE      SET_RING_BASE;
	struct VREG_RDMA_CORE_RING_BASE_ADDR_L   RING_BASE_ADDR_L;
	struct VREG_RDMA_CORE_RING_BASE_ADDR_H   RING_BASE_ADDR_H;
	struct VREG_RESV                        _resv_0x38[18];
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE0  RING_BUFFER_SIZE0;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE1  RING_BUFFER_SIZE1;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE2  RING_BUFFER_SIZE2;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE3  RING_BUFFER_SIZE3;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE4  RING_BUFFER_SIZE4;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE5  RING_BUFFER_SIZE5;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE6  RING_BUFFER_SIZE6;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE7  RING_BUFFER_SIZE7;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE8  RING_BUFFER_SIZE8;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE9  RING_BUFFER_SIZE9;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE10  RING_BUFFER_SIZE10;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE11  RING_BUFFER_SIZE11;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE12  RING_BUFFER_SIZE12;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE13  RING_BUFFER_SIZE13;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE14  RING_BUFFER_SIZE14;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE15  RING_BUFFER_SIZE15;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE16  RING_BUFFER_SIZE16;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE17  RING_BUFFER_SIZE17;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE18  RING_BUFFER_SIZE18;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE19  RING_BUFFER_SIZE19;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE20  RING_BUFFER_SIZE20;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE21  RING_BUFFER_SIZE21;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE22  RING_BUFFER_SIZE22;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE23  RING_BUFFER_SIZE23;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE24  RING_BUFFER_SIZE24;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE25  RING_BUFFER_SIZE25;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE26  RING_BUFFER_SIZE26;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE27  RING_BUFFER_SIZE27;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE28  RING_BUFFER_SIZE28;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE29  RING_BUFFER_SIZE29;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE30  RING_BUFFER_SIZE30;
	struct VREG_RDMA_CORE_RING_BUFFER_SIZE31  RING_BUFFER_SIZE31;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS0  NEXT_DMA_ADDR_STS0;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS1  NEXT_DMA_ADDR_STS1;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS2  NEXT_DMA_ADDR_STS2;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS3  NEXT_DMA_ADDR_STS3;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS4  NEXT_DMA_ADDR_STS4;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS5  NEXT_DMA_ADDR_STS5;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS6  NEXT_DMA_ADDR_STS6;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS7  NEXT_DMA_ADDR_STS7;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS8  NEXT_DMA_ADDR_STS8;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS9  NEXT_DMA_ADDR_STS9;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS10  NEXT_DMA_ADDR_STS10;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS11  NEXT_DMA_ADDR_STS11;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS12  NEXT_DMA_ADDR_STS12;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS13  NEXT_DMA_ADDR_STS13;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS14  NEXT_DMA_ADDR_STS14;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS15  NEXT_DMA_ADDR_STS15;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS16  NEXT_DMA_ADDR_STS16;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS17  NEXT_DMA_ADDR_STS17;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS18  NEXT_DMA_ADDR_STS18;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS19  NEXT_DMA_ADDR_STS19;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS20  NEXT_DMA_ADDR_STS20;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS21  NEXT_DMA_ADDR_STS21;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS22  NEXT_DMA_ADDR_STS22;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS23  NEXT_DMA_ADDR_STS23;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS24  NEXT_DMA_ADDR_STS24;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS25  NEXT_DMA_ADDR_STS25;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS26  NEXT_DMA_ADDR_STS26;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS27  NEXT_DMA_ADDR_STS27;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS28  NEXT_DMA_ADDR_STS28;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS29  NEXT_DMA_ADDR_STS29;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS30  NEXT_DMA_ADDR_STS30;
	struct VREG_RDMA_CORE_NEXT_DMA_ADDR_STS31  NEXT_DMA_ADDR_STS31;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_PRE_RAW_VI_SEL_0 {
	union REG_PRE_RAW_VI_SEL_0              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_PRE_RAW_VI_SEL_1 {
	union REG_PRE_RAW_VI_SEL_1              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_PRE_RAW_VI_SEL_2 {
	union REG_PRE_RAW_VI_SEL_2              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_PRE_RAW_VI_SEL_3 {
	union REG_PRE_RAW_VI_SEL_3              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_PRE_RAW_VI_SEL_4 {
	union REG_PRE_RAW_VI_SEL_4              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_PRE_RAW_VI_SEL_5 {
	union REG_PRE_RAW_VI_SEL_5              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_PRE_RAW_VI_SEL_6 {
	union REG_PRE_RAW_VI_SEL_6              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_PRE_RAW_VI_SEL_7 {
	union REG_PRE_RAW_VI_SEL_7              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_PRE_RAW_VI_SEL_8 {
	union REG_PRE_RAW_VI_SEL_8              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_PRE_RAW_VI_SEL_9 {
	union REG_PRE_RAW_VI_SEL_9              write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_PRE_RAW_VI_SEL_10 {
	union REG_PRE_RAW_VI_SEL_10             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_PRE_RAW_VI_SEL_11 {
	union REG_PRE_RAW_VI_SEL_11             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_PRE_RAW_VI_SEL_12 {
	union REG_PRE_RAW_VI_SEL_12             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_PRE_RAW_VI_SEL_13 {
	union REG_PRE_RAW_VI_SEL_13             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_PRE_RAW_VI_SEL_14 {
	union REG_PRE_RAW_VI_SEL_14             write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_PRE_RAW_VI_SEL_T {
	struct VREG_PRE_RAW_VI_SEL_0             REG_0;
	struct VREG_PRE_RAW_VI_SEL_1             REG_1;
	struct VREG_RESV                        _resv_0x8[2];
	struct VREG_PRE_RAW_VI_SEL_2             REG_2;
	struct VREG_PRE_RAW_VI_SEL_3             REG_3;
	struct VREG_RESV                        _resv_0x18[2];
	struct VREG_PRE_RAW_VI_SEL_4             REG_4;
	struct VREG_PRE_RAW_VI_SEL_5             REG_5;
	struct VREG_PRE_RAW_VI_SEL_6             REG_6;
	struct VREG_PRE_RAW_VI_SEL_7             REG_7;
	struct VREG_PRE_RAW_VI_SEL_8             REG_8;
	struct VREG_PRE_RAW_VI_SEL_9             REG_9;
	struct VREG_PRE_RAW_VI_SEL_10            REG_10;
	struct VREG_PRE_RAW_VI_SEL_11            REG_11;
	struct VREG_PRE_RAW_VI_SEL_12            REG_12;
	struct VREG_PRE_RAW_VI_SEL_13            REG_13;
	struct VREG_PRE_RAW_VI_SEL_14            REG_14;
};

/******************************************/
/*           Module Definition            */
/******************************************/
struct VREG_CROP_0 {
	union REG_CROP_0                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_CROP_1 {
	union REG_CROP_1                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_CROP_2 {
	union REG_CROP_2                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_CROP_3 {
	union REG_CROP_3                        write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_CROP_DUMMY {
	union REG_CROP_DUMMY                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_CROP_DEBUG {
	union REG_CROP_DEBUG                    write;
	union CMDSET_FIELD                      ctrl;
};

struct VREG_CROP_T {
	struct VREG_CROP_0                       REG_0;
	struct VREG_CROP_1                       REG_1;
	struct VREG_CROP_2                       REG_2;
	struct VREG_CROP_3                       REG_3;
	struct VREG_CROP_DUMMY                   DUMMY;
	struct VREG_CROP_DEBUG                   DEBUG;
};

#ifdef __cplusplus
}
#endif

#endif /* _VI_VREG_BLOCKS_H_ */
