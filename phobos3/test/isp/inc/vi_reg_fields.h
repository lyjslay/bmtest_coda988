/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name:vi_reg_fields.h
 * Description:HW register description
 */

#ifndef _VI_REG_FIELDS_H_
#define _VI_REG_FIELDS_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_PRE_RAW_FE_PRE_RAW_CTRL {
	uint32_t raw;
	struct {
		uint32_t BAYER_TYPE_LE                   : 4;
		uint32_t BAYER_TYPE_SE                   : 4;
		uint32_t _rsv_8                          : 10;
		uint32_t UP_PQ_EN                        : 1;
		uint32_t _rsv_19                         : 12;
		uint32_t SHDW_READ_SEL                   : 1;
	} bits;
};

union REG_PRE_RAW_FE_PRE_RAW_FRAME_SIZE {
	uint32_t raw;
	struct {
		uint32_t FRAME_WIDTHM1                   : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t FRAME_HEIGHTM1                  : 14;
	} bits;
};

union REG_PRE_RAW_FE_PRE_RAW_POST_NO_RSPD_CYC {
	uint32_t raw;
	struct {
		uint32_t POST_NO_RSPD_CYC                : 32;
	} bits;
};

union REG_PRE_RAW_FE_PRE_RAW_FRAME_VLD {
	uint32_t raw;
	struct {
		uint32_t FE_FRAME_VLD_CH0                : 1;
		uint32_t FE_FRAME_VLD_CH1                : 1;
		uint32_t FE_FRAME_VLD_CH2                : 1;
		uint32_t FE_FRAME_VLD_CH3                : 1;
		uint32_t FE_PQ_VLD_CH0                   : 1;
		uint32_t FE_PQ_VLD_CH1                   : 1;
		uint32_t FE_PQ_VLD_CH2                   : 1;
		uint32_t FE_PQ_VLD_CH3                   : 1;
		uint32_t _rsv_8                          : 8;
		uint32_t POST_RAW_IDLE                   : 1;
	} bits;
};

union REG_PRE_RAW_FE_PRE_RAW_DEBUG_STATE {
	uint32_t raw;
	struct {
		uint32_t PRE_RAW_FE_IDLE                 : 32;
	} bits;
};

union REG_PRE_RAW_FE_PRE_RAW_DUMMY {
	uint32_t raw;
	struct {
		uint32_t DUMMY_RW                        : 16;
		uint32_t DUMMY_RO                        : 16;
	} bits;
};

union REG_PRE_RAW_FE_PRE_RAW_DEBUG_INFO {
	uint32_t raw;
	struct {
		uint32_t IP_FRAME_DONE_STS               : 32;
	} bits;
};

union REG_PRE_RAW_FE_IDLE_INFO {
	uint32_t raw;
	struct {
		uint32_t IP_DMA_IDLE                     : 32;
	} bits;
};

union REG_PRE_RAW_FE_CHECK_SUM {
	uint32_t raw;
	struct {
		uint32_t LEXP_CHKSUM_ENABLE              : 1;
		uint32_t SEXP_CHKSUM_ENABLE              : 1;
	} bits;
};

union REG_PRE_RAW_FE_CHECK_SUM_VALUE {
	uint32_t raw;
	struct {
		uint32_t LEXP_CHKSUM_VALUE               : 8;
		uint32_t _rsv_8                          : 8;
		uint32_t SEXP_CHKSUM_VALUE               : 8;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_CSI_BDG_TOP_CTRL {
	uint32_t raw;
	struct {
		uint32_t CSI_MODE                        : 2;
		uint32_t CSI_IN_FORMAT                   : 1;
		uint32_t CSI_IN_YUV_FORMAT               : 1;
		uint32_t CH_NUM                          : 2;
		uint32_t CH0_DMA_WR_ENABLE               : 1;
		uint32_t CH1_DMA_WR_ENABLE               : 1;
		uint32_t CH2_DMA_WR_ENABLE               : 1;
		uint32_t Y_ONLY                          : 1;
		uint32_t PXL_DATA_SEL                    : 1;
		uint32_t VS_POL                          : 1;
		uint32_t HS_POL                          : 1;
		uint32_t RESET_MODE                      : 1;
		uint32_t VS_MODE                         : 1;
		uint32_t ABORT_MODE                      : 1;
		uint32_t RESET                           : 1;
		uint32_t ABORT                           : 1;
		uint32_t CH3_DMA_WR_ENABLE               : 1;
		uint32_t CH3_DMA_420_WR_ENABLE           : 1;
		uint32_t YUV_PACK_MODE                   : 1;
		uint32_t MULTI_CH_FRAME_SYNC_EN          : 1;
		uint32_t CH0_DMA_420_WR_ENABLE           : 1;
		uint32_t CH1_DMA_420_WR_ENABLE           : 1;
		uint32_t CSI_ENABLE                      : 1;
		uint32_t TGEN_ENABLE                     : 1;
		uint32_t YUV2BAY_ENABLE                  : 1;
		uint32_t CH2_DMA_420_WR_ENABLE           : 1;
		uint32_t SHDW_READ_SEL                   : 1;
		uint32_t _rsv_29                         : 2;
		uint32_t CSI_UP_REG                      : 1;
	} bits;
};

union REG_ISP_CSI_BDG_INTERRUPT_CTRL {
	uint32_t raw;
	struct {
		uint32_t CH0_VS_INT_EN                   : 1;
		uint32_t CH0_TRIG_INT_EN                 : 1;
		uint32_t CH0_DROP_INT_EN                 : 1;
		uint32_t CH0_SIZE_ERROR_INT_EN           : 1;
		uint32_t CH1_VS_INT_EN                   : 1;
		uint32_t CH1_TRIG_INT_EN                 : 1;
		uint32_t CH1_DROP_INT_EN                 : 1;
		uint32_t CH1_SIZE_ERROR_INT_EN           : 1;
		uint32_t CH2_VS_INT_EN                   : 1;
		uint32_t CH2_TRIG_INT_EN                 : 1;
		uint32_t CH2_DROP_INT_EN                 : 1;
		uint32_t CH2_SIZE_ERROR_INT_EN           : 1;
		uint32_t CH3_VS_INT_EN                   : 1;
		uint32_t CH3_TRIG_INT_EN                 : 1;
		uint32_t CH3_DROP_INT_EN                 : 1;
		uint32_t CH3_SIZE_ERROR_INT_EN           : 1;
		uint32_t _rsv_16                         : 12;
		uint32_t SLICE_LINE_INTP_EN              : 1;
		uint32_t DMA_ERROR_INTP_EN               : 1;
		uint32_t LINE_INTP_EN                    : 1;
		uint32_t FIFO_OVERFLOW_INT_EN            : 1;
	} bits;
};

union REG_ISP_CSI_BDG_DMA_DPCM_MODE {
	uint32_t raw;
	struct {
		uint32_t DMA_ST_DPCM_MODE                : 3;
		uint32_t _rsv_3                          : 1;
		uint32_t DPCM_MIPI_OPT                   : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t DROP_MODE                       : 4;
		uint32_t AVG_MODE                        : 4;
		uint32_t DPCM_XSTR                       : 13;
	} bits;
};

union REG_ISP_CSI_BDG_DMA_LD_DPCM_MODE {
	uint32_t raw;
	struct {
		uint32_t DMA_LD_DPCM_MODE                : 3;
		uint32_t _rsv_3                          : 13;
		uint32_t DPCM_RX_XSTR                    : 13;
	} bits;
};

union REG_ISP_CSI_BDG_CH0_SIZE {
	uint32_t raw;
	struct {
		uint32_t CH0_FRAME_WIDTHM1               : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t CH0_FRAME_HEIGHTM1              : 14;
	} bits;
};

union REG_ISP_CSI_BDG_CH1_SIZE {
	uint32_t raw;
	struct {
		uint32_t CH1_FRAME_WIDTHM1               : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t CH1_FRAME_HEIGHTM1              : 14;
	} bits;
};

union REG_ISP_CSI_BDG_CH2_SIZE {
	uint32_t raw;
	struct {
		uint32_t CH2_FRAME_WIDTHM1               : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t CH2_FRAME_HEIGHTM1              : 14;
	} bits;
};

union REG_ISP_CSI_BDG_CH3_SIZE {
	uint32_t raw;
	struct {
		uint32_t CH3_FRAME_WIDTHM1               : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t CH3_FRAME_HEIGHTM1              : 14;
	} bits;
};

union REG_ISP_CSI_BDG_CH0_CROP_EN {
	uint32_t raw;
	struct {
		uint32_t CH0_CROP_EN                     : 1;
	} bits;
};

union REG_ISP_CSI_BDG_CH0_HORZ_CROP {
	uint32_t raw;
	struct {
		uint32_t CH0_HORZ_CROP_START             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t CH0_HORZ_CROP_END               : 14;
	} bits;
};

union REG_ISP_CSI_BDG_CH0_VERT_CROP {
	uint32_t raw;
	struct {
		uint32_t CH0_VERT_CROP_START             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t CH0_VERT_CROP_END               : 14;
	} bits;
};

union REG_ISP_CSI_BDG_CH0_BLC_SUM {
	uint32_t raw;
	struct {
		uint32_t CH0_BLC_SUM                     : 32;
	} bits;
};

union REG_ISP_CSI_BDG_CH1_CROP_EN {
	uint32_t raw;
	struct {
		uint32_t CH1_CROP_EN                     : 1;
	} bits;
};

union REG_ISP_CSI_BDG_CH1_HORZ_CROP {
	uint32_t raw;
	struct {
		uint32_t CH1_HORZ_CROP_START             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t CH1_HORZ_CROP_END               : 14;
	} bits;
};

union REG_ISP_CSI_BDG_CH1_VERT_CROP {
	uint32_t raw;
	struct {
		uint32_t CH1_VERT_CROP_START             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t CH1_VERT_CROP_END               : 14;
	} bits;
};

union REG_ISP_CSI_BDG_CH1_BLC_SUM {
	uint32_t raw;
	struct {
		uint32_t CH1_BLC_SUM                     : 32;
	} bits;
};

union REG_ISP_CSI_BDG_CH2_CROP_EN {
	uint32_t raw;
	struct {
		uint32_t CH2_CROP_EN                     : 1;
	} bits;
};

union REG_ISP_CSI_BDG_CH2_HORZ_CROP {
	uint32_t raw;
	struct {
		uint32_t CH2_HORZ_CROP_START             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t CH2_HORZ_CROP_END               : 14;
	} bits;
};

union REG_ISP_CSI_BDG_CH2_VERT_CROP {
	uint32_t raw;
	struct {
		uint32_t CH2_VERT_CROP_START             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t CH2_VERT_CROP_END               : 14;
	} bits;
};

union REG_ISP_CSI_BDG_CH2_BLC_SUM {
	uint32_t raw;
	struct {
		uint32_t CH2_BLC_SUM                     : 32;
	} bits;
};

union REG_ISP_CSI_BDG_CH3_CROP_EN {
	uint32_t raw;
	struct {
		uint32_t CH3_CROP_EN                     : 1;
	} bits;
};

union REG_ISP_CSI_BDG_CH3_HORZ_CROP {
	uint32_t raw;
	struct {
		uint32_t CH3_HORZ_CROP_START             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t CH3_HORZ_CROP_END               : 14;
	} bits;
};

union REG_ISP_CSI_BDG_CH3_VERT_CROP {
	uint32_t raw;
	struct {
		uint32_t CH3_VERT_CROP_START             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t CH3_VERT_CROP_END               : 14;
	} bits;
};

union REG_ISP_CSI_BDG_CH3_BLC_SUM {
	uint32_t raw;
	struct {
		uint32_t CH3_BLC_SUM                     : 32;
	} bits;
};

union REG_ISP_CSI_BDG_PAT_GEN_CTRL {
	uint32_t raw;
	struct {
		uint32_t PAT_EN                          : 1;
		uint32_t GRA_INV                         : 1;
		uint32_t AUTO_EN                         : 1;
		uint32_t DITH_EN                         : 1;
		uint32_t SNOW_EN                         : 1;
		uint32_t FIX_MC                          : 1;
		uint32_t _rsv_6                          : 2;
		uint32_t DITH_MD                         : 3;
		uint32_t _rsv_11                         : 1;
		uint32_t BAYER_ID                        : 2;
	} bits;
};

union REG_ISP_CSI_BDG_PAT_IDX_CTRL {
	uint32_t raw;
	struct {
		uint32_t PAT_PRD                         : 8;
		uint32_t PAT_IDX                         : 5;
	} bits;
};

union REG_ISP_CSI_BDG_PAT_COLOR_0 {
	uint32_t raw;
	struct {
		uint32_t PAT_R                           : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t PAT_G                           : 12;
	} bits;
};

union REG_ISP_CSI_BDG_PAT_COLOR_1 {
	uint32_t raw;
	struct {
		uint32_t PAT_B                           : 12;
	} bits;
};

union REG_ISP_CSI_BDG_BACKGROUND_COLOR_0 {
	uint32_t raw;
	struct {
		uint32_t FDE_R                           : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t FDE_G                           : 12;
	} bits;
};

union REG_ISP_CSI_BDG_BACKGROUND_COLOR_1 {
	uint32_t raw;
	struct {
		uint32_t FDE_B                           : 12;
	} bits;
};

union REG_ISP_CSI_BDG_FIX_COLOR_0 {
	uint32_t raw;
	struct {
		uint32_t MDE_R                           : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t MDE_G                           : 12;
	} bits;
};

union REG_ISP_CSI_BDG_FIX_COLOR_1 {
	uint32_t raw;
	struct {
		uint32_t MDE_B                           : 12;
	} bits;
};

union REG_ISP_CSI_BDG_MDE_V_SIZE {
	uint32_t raw;
	struct {
		uint32_t VMDE_STR                        : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t VMDE_STP                        : 14;
	} bits;
};

union REG_ISP_CSI_BDG_MDE_H_SIZE {
	uint32_t raw;
	struct {
		uint32_t HMDE_STR                        : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t HMDE_STP                        : 14;
	} bits;
};

union REG_ISP_CSI_BDG_FDE_V_SIZE {
	uint32_t raw;
	struct {
		uint32_t VFDE_STR                        : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t VFDE_STP                        : 14;
	} bits;
};

union REG_ISP_CSI_BDG_FDE_H_SIZE {
	uint32_t raw;
	struct {
		uint32_t HFDE_STR                        : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t HFDE_STP                        : 14;
	} bits;
};

union REG_ISP_CSI_BDG_HSYNC_CTRL {
	uint32_t raw;
	struct {
		uint32_t HS_STR                          : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t HS_STP                          : 14;
	} bits;
};

union REG_ISP_CSI_BDG_VSYNC_CTRL {
	uint32_t raw;
	struct {
		uint32_t VS_STR                          : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t VS_STP                          : 14;
	} bits;
};

union REG_ISP_CSI_BDG_TGEN_TT_SIZE {
	uint32_t raw;
	struct {
		uint32_t HTT                             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t VTT                             : 14;
	} bits;
};

union REG_ISP_CSI_BDG_LINE_INTP_HEIGHT_0 {
	uint32_t raw;
	struct {
		uint32_t CH0_LINE_INTP_HEIGHTM1          : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t CH1_LINE_INTP_HEIGHTM1          : 14;
	} bits;
};

union REG_ISP_CSI_BDG_CH0_DEBUG_0 {
	uint32_t raw;
	struct {
		uint32_t CH0_PXL_CNT                     : 32;
	} bits;
};

union REG_ISP_CSI_BDG_CH0_DEBUG_1 {
	uint32_t raw;
	struct {
		uint32_t CH0_LINE_CNT                    : 16;
	} bits;
};

union REG_ISP_CSI_BDG_CH0_DEBUG_2 {
	uint32_t raw;
	struct {
		uint32_t CH0_VS_CNT                      : 16;
		uint32_t CH0_TRIG_CNT                    : 16;
	} bits;
};

union REG_ISP_CSI_BDG_CH0_DEBUG_3 {
	uint32_t raw;
	struct {
		uint32_t CH0_TOT_BLK_IDLE                : 1;
		uint32_t CH0_TOT_DMA_IDLE                : 1;
		uint32_t CH0_BDG_DMA_IDLE                : 1;
	} bits;
};

union REG_ISP_CSI_BDG_CH1_DEBUG_0 {
	uint32_t raw;
	struct {
		uint32_t CH1_PXL_CNT                     : 32;
	} bits;
};

union REG_ISP_CSI_BDG_CH1_DEBUG_1 {
	uint32_t raw;
	struct {
		uint32_t CH1_LINE_CNT                    : 16;
	} bits;
};

union REG_ISP_CSI_BDG_CH1_DEBUG_2 {
	uint32_t raw;
	struct {
		uint32_t CH1_VS_CNT                      : 16;
		uint32_t CH1_TRIG_CNT                    : 16;
	} bits;
};

union REG_ISP_CSI_BDG_CH1_DEBUG_3 {
	uint32_t raw;
	struct {
		uint32_t CH1_TOT_BLK_IDLE                : 1;
		uint32_t CH1_TOT_DMA_IDLE                : 1;
		uint32_t CH1_BDG_DMA_IDLE                : 1;
	} bits;
};

union REG_ISP_CSI_BDG_CH2_DEBUG_0 {
	uint32_t raw;
	struct {
		uint32_t CH2_PXL_CNT                     : 32;
	} bits;
};

union REG_ISP_CSI_BDG_CH2_DEBUG_1 {
	uint32_t raw;
	struct {
		uint32_t CH2_LINE_CNT                    : 16;
	} bits;
};

union REG_ISP_CSI_BDG_CH2_DEBUG_2 {
	uint32_t raw;
	struct {
		uint32_t CH2_VS_CNT                      : 16;
		uint32_t CH2_TRIG_CNT                    : 16;
	} bits;
};

union REG_ISP_CSI_BDG_CH2_DEBUG_3 {
	uint32_t raw;
	struct {
		uint32_t CH2_TOT_BLK_IDLE                : 1;
		uint32_t CH2_TOT_DMA_IDLE                : 1;
		uint32_t CH2_BDG_DMA_IDLE                : 1;
	} bits;
};

union REG_ISP_CSI_BDG_CH3_DEBUG_0 {
	uint32_t raw;
	struct {
		uint32_t CH3_PXL_CNT                     : 32;
	} bits;
};

union REG_ISP_CSI_BDG_CH3_DEBUG_1 {
	uint32_t raw;
	struct {
		uint32_t CH3_LINE_CNT                    : 16;
	} bits;
};

union REG_ISP_CSI_BDG_CH3_DEBUG_2 {
	uint32_t raw;
	struct {
		uint32_t CH3_VS_CNT                      : 16;
		uint32_t CH3_TRIG_CNT                    : 16;
	} bits;
};

union REG_ISP_CSI_BDG_CH3_DEBUG_3 {
	uint32_t raw;
	struct {
		uint32_t CH3_TOT_BLK_IDLE                : 1;
		uint32_t CH3_TOT_DMA_IDLE                : 1;
		uint32_t CH3_BDG_DMA_IDLE                : 1;
	} bits;
};

union REG_ISP_CSI_BDG_INTERRUPT_STATUS_0 {
	uint32_t raw;
	struct {
		uint32_t CH0_FRAME_DROP_INT              : 1;
		uint32_t CH0_VS_INT                      : 1;
		uint32_t CH0_TRIG_INT                    : 1;
		uint32_t _rsv_3                          : 1;
		uint32_t CH0_FRAME_WIDTH_GT_INT          : 1;
		uint32_t CH0_FRAME_WIDTH_LS_INT          : 1;
		uint32_t CH0_FRAME_HEIGHT_GT_INT         : 1;
		uint32_t CH0_FRAME_HEIGHT_LS_INT         : 1;
		uint32_t CH1_FRAME_DROP_INT              : 1;
		uint32_t CH1_VS_INT                      : 1;
		uint32_t CH1_TRIG_INT                    : 1;
		uint32_t _rsv_11                         : 1;
		uint32_t CH1_FRAME_WIDTH_GT_INT          : 1;
		uint32_t CH1_FRAME_WIDTH_LS_INT          : 1;
		uint32_t CH1_FRAME_HEIGHT_GT_INT         : 1;
		uint32_t CH1_FRAME_HEIGHT_LS_INT         : 1;
		uint32_t CH2_FRAME_DROP_INT              : 1;
		uint32_t CH2_VS_INT                      : 1;
		uint32_t CH2_TRIG_INT                    : 1;
		uint32_t _rsv_19                         : 1;
		uint32_t CH2_FRAME_WIDTH_GT_INT          : 1;
		uint32_t CH2_FRAME_WIDTH_LS_INT          : 1;
		uint32_t CH2_FRAME_HEIGHT_GT_INT         : 1;
		uint32_t CH2_FRAME_HEIGHT_LS_INT         : 1;
		uint32_t CH3_FRAME_DROP_INT              : 1;
		uint32_t CH3_VS_INT                      : 1;
		uint32_t CH3_TRIG_INT                    : 1;
		uint32_t _rsv_27                         : 1;
		uint32_t CH3_FRAME_WIDTH_GT_INT          : 1;
		uint32_t CH3_FRAME_WIDTH_LS_INT          : 1;
		uint32_t CH3_FRAME_HEIGHT_GT_INT         : 1;
		uint32_t CH3_FRAME_HEIGHT_LS_INT         : 1;
	} bits;
};

union REG_ISP_CSI_BDG_INTERRUPT_STATUS_1 {
	uint32_t raw;
	struct {
		uint32_t FIFO_OVERFLOW_INT               : 1;
		uint32_t FRAME_RESOLUTION_OVER_MAX_INT   : 1;
		uint32_t _rsv_2                          : 2;
		uint32_t DMA_ERROR_INT                   : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t CH0_LINE_INTP_INT               : 1;
		uint32_t CH1_LINE_INTP_INT               : 1;
		uint32_t CH2_LINE_INTP_INT               : 1;
		uint32_t CH3_LINE_INTP_INT               : 1;
		uint32_t CH0_SLICE_LINE_INTP_INT         : 1;
		uint32_t CH1_SLICE_LINE_INTP_INT         : 1;
		uint32_t CH2_SLICE_LINE_INTP_INT         : 1;
		uint32_t CH3_SLICE_LINE_INTP_INT         : 1;
		uint32_t CH3_PRECROP_LINE_INTP_INT       : 1;
		uint32_t CH2_PRECROP_LINE_INTP_INT       : 1;
		uint32_t CH1_PRECROP_LINE_INTP_INT       : 1;
		uint32_t CH0_PRECROP_LINE_INTP_INT       : 1;
		uint32_t CLSC_SE_FETCH_DONE_INTP_INT     : 1;
		uint32_t CLSC_LE_FETCH_DONE_INTP_INT     : 1;
		uint32_t GAIN_DMA_MODE_EN                : 1;
	} bits;
};

union REG_ISP_CSI_BDG_DEBUG {
	uint32_t raw;
	struct {
		uint32_t RING_BUFF_IDLE                  : 1;
	} bits;
};

union REG_ISP_CSI_BDG_OUT_VSYNC_LINE_DELAY {
	uint32_t raw;
	struct {
		uint32_t OUT_VSYNC_LINE_DELAY            : 12;
	} bits;
};

union REG_ISP_CSI_BDG_WR_URGENT_CTRL {
	uint32_t raw;
	struct {
		uint32_t WR_NEAR_OVERFLOW_THRESHOLD      : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t WR_SAFE_THRESHOLD               : 13;
	} bits;
};

union REG_ISP_CSI_BDG_RD_URGENT_CTRL {
	uint32_t raw;
	struct {
		uint32_t RD_NEAR_OVERFLOW_THRESHOLD      : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t RD_SAFE_THRESHOLD               : 13;
	} bits;
};

union REG_ISP_CSI_BDG_DUMMY {
	uint32_t raw;
	struct {
		uint32_t DUMMY_IN                        : 16;
		uint32_t DUMMY_OUT                       : 16;
	} bits;
};

union REG_ISP_CSI_BDG_LINE_INTP_HEIGHT_1 {
	uint32_t raw;
	struct {
		uint32_t CH2_LINE_INTP_HEIGHTM1          : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t CH3_LINE_INTP_HEIGHTM1          : 14;
	} bits;
};

union REG_ISP_CSI_BDG_SLICE_LINE_INTP_HEIGHT_0 {
	uint32_t raw;
	struct {
		uint32_t CH0_SLICE_LINE_INTP_HEIGHTM1    : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t CH1_SLICE_LINE_INTP_HEIGHTM1    : 14;
	} bits;
};

union REG_ISP_CSI_BDG_SLICE_LINE_INTP_HEIGHT_1 {
	uint32_t raw;
	struct {
		uint32_t CH2_SLICE_LINE_INTP_HEIGHTM1    : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t CH3_SLICE_LINE_INTP_HEIGHTM1    : 14;
	} bits;
};

union REG_ISP_CSI_BDG_BAYER_TYPE_CLK_GATE_YUV_SWAP {
	uint32_t raw;
	struct {
		uint32_t BAYER_TYPE_POST_CROP_MULTICH    : 2;
		uint32_t CLK_GATING_EN                   : 1;
		uint32_t YUV_CH_SWAP_EN                  : 1;
		uint32_t HDR_MODE_EN                     : 1;
		uint32_t CLSC_BYPASS_LE                  : 1;
		uint32_t CLSC_BYPASS_SE                  : 1;
		uint32_t PRE_CROP_LINE_INTP_EN           : 1;
		uint32_t CLSC_FETCH_DONE_INTP_EN         : 1;
		uint32_t SW_DMA_TRIG_STR_1T              : 1;
		uint32_t _rsv_10                         : 1;
		uint32_t HW_FF_THRESHOLD                 : 9;
		uint32_t RING_FIFO_USAGE                 : 9;
		uint32_t CLSC_HW_EN_STATUS               : 2;
	} bits;
};

union REG_ISP_CSI_BDG_WDMA_CH0_CROP_EN {
	uint32_t raw;
	struct {
		uint32_t ST_CH0_CROP_EN                  : 1;
	} bits;
};

union REG_ISP_CSI_BDG_WDMA_CH0_HORZ_CROP {
	uint32_t raw;
	struct {
		uint32_t ST_CH0_HORZ_CROP_START          : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ST_CH0_HORZ_CROP_END            : 14;
	} bits;
};

union REG_ISP_CSI_BDG_WDMA_CH0_VERT_CROP {
	uint32_t raw;
	struct {
		uint32_t ST_CH0_VERT_CROP_START          : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ST_CH0_VERT_CROP_END            : 14;
	} bits;
};

union REG_ISP_CSI_BDG_WDMA_CH1_CROP_EN {
	uint32_t raw;
	struct {
		uint32_t ST_CH1_CROP_EN                  : 1;
	} bits;
};

union REG_ISP_CSI_BDG_WDMA_CH1_HORZ_CROP {
	uint32_t raw;
	struct {
		uint32_t ST_CH1_HORZ_CROP_START          : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ST_CH1_HORZ_CROP_END            : 14;
	} bits;
};

union REG_ISP_CSI_BDG_WDMA_CH1_VERT_CROP {
	uint32_t raw;
	struct {
		uint32_t ST_CH1_VERT_CROP_START          : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ST_CH1_VERT_CROP_END            : 14;
	} bits;
};

union REG_ISP_CSI_BDG_WDMA_CH2_CROP_EN {
	uint32_t raw;
	struct {
		uint32_t ST_CH2_CROP_EN                  : 1;
	} bits;
};

union REG_ISP_CSI_BDG_WDMA_CH2_HORZ_CROP {
	uint32_t raw;
	struct {
		uint32_t ST_CH2_HORZ_CROP_START          : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ST_CH2_HORZ_CROP_END            : 14;
	} bits;
};

union REG_ISP_CSI_BDG_WDMA_CH2_VERT_CROP {
	uint32_t raw;
	struct {
		uint32_t ST_CH2_VERT_CROP_START          : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ST_CH2_VERT_CROP_END            : 14;
	} bits;
};

union REG_ISP_CSI_BDG_WDMA_CH3_CROP_EN {
	uint32_t raw;
	struct {
		uint32_t ST_CH3_CROP_EN                  : 1;
	} bits;
};

union REG_ISP_CSI_BDG_WDMA_CH3_HORZ_CROP {
	uint32_t raw;
	struct {
		uint32_t ST_CH3_HORZ_CROP_START          : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ST_CH3_HORZ_CROP_END            : 14;
	} bits;
};

union REG_ISP_CSI_BDG_WDMA_CH3_VERT_CROP {
	uint32_t raw;
	struct {
		uint32_t ST_CH3_VERT_CROP_START          : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t ST_CH3_VERT_CROP_END            : 14;
	} bits;
};

union REG_ISP_CSI_BDG_TRIG_DLY_CONTROL_0 {
	uint32_t raw;
	struct {
		uint32_t TRIG_DLY_EN                     : 1;
	} bits;
};

union REG_ISP_CSI_BDG_TRIG_DLY_CONTROL_1 {
	uint32_t raw;
	struct {
		uint32_t TRIG_DLY_VALUE                  : 32;
	} bits;
};

union REG_ISP_CSI_BDG_BAYER_TYPE {
	uint32_t raw;
	struct {
		uint32_t BAYER_TYPE                      : 2;
	} bits;
};

union REG_ISP_CSI_BDG_CH0_AI_ISP_CONTROL {
	uint32_t raw;
	struct {
		uint32_t BYPASS_CH0                      : 1;
		uint32_t _rsv_1                          : 1;
		uint32_t HW_AUTO_CG_EN_CH0               : 1;
		uint32_t GUARD_CNT_CH0                   : 8;
	} bits;
};

union REG_ISP_CSI_BDG_CH0_AI_ISP_TRANSFORM {
	uint32_t raw;
	struct {
		uint32_t AI_ISP_TRANSFORM_EN_CH0         : 1;
		uint32_t IN_FORMAT_CH0                   : 1;
		uint32_t OUT_FORMAT_CH0                  : 2;
		uint32_t ROUND_MODE_CH0                  : 3;
	} bits;
};

union REG_ISP_CSI_BDG_AI_ISP_DEBUG_STATUS_CH0 {
	uint32_t raw;
	struct {
		uint32_t AI_ISP_DEBUG_STATUS_CH0         : 32;
	} bits;
};

union REG_ISP_CSI_BDG_CH1_AI_ISP_CONTROL {
	uint32_t raw;
	struct {
		uint32_t BYPASS_CH1                      : 1;
		uint32_t _rsv_1                          : 1;
		uint32_t HW_AUTO_CG_EN_CH1               : 1;
		uint32_t GUARD_CNT_CH1                   : 8;
	} bits;
};

union REG_ISP_CSI_BDG_CH1_AI_ISP_TRANSFORM {
	uint32_t raw;
	struct {
		uint32_t AI_ISP_TRANSFORM_EN_CH1         : 1;
		uint32_t IN_FORMAT_CH1                   : 1;
		uint32_t OUT_FORMAT_CH1                  : 2;
		uint32_t ROUND_MODE_CH1                  : 3;
	} bits;
};

union REG_ISP_CSI_BDG_AI_ISP_DEBUG_STATUS_CH1 {
	uint32_t raw;
	struct {
		uint32_t AI_ISP_DEBUG_STATUS_CH1         : 32;
	} bits;
};

union REG_ISP_CSI_BDG_CH2_AI_ISP_CONTROL {
	uint32_t raw;
	struct {
		uint32_t BYPASS_CH2                      : 1;
		uint32_t _rsv_1                          : 1;
		uint32_t HW_AUTO_CG_EN_CH2               : 1;
		uint32_t GUARD_CNT_CH2                   : 8;
	} bits;
};

union REG_ISP_CSI_BDG_CH2_AI_ISP_TRANSFORM {
	uint32_t raw;
	struct {
		uint32_t AI_ISP_TRANSFORM_EN_CH2         : 1;
		uint32_t IN_FORMAT_CH2                   : 1;
		uint32_t OUT_FORMAT_CH2                  : 2;
		uint32_t ROUND_MODE_CH2                  : 3;
	} bits;
};

union REG_ISP_CSI_BDG_AI_ISP_DEBUG_STATUS_CH2 {
	uint32_t raw;
	struct {
		uint32_t AI_ISP_DEBUG_STATUS_CH2         : 32;
	} bits;
};

union REG_ISP_CSI_BDG_CH3_AI_ISP_CONTROL {
	uint32_t raw;
	struct {
		uint32_t BYPASS_CH3                      : 1;
		uint32_t _rsv_1                          : 1;
		uint32_t HW_AUTO_CG_EN_CH3               : 1;
		uint32_t GUARD_CNT_CH3                   : 8;
	} bits;
};

union REG_ISP_CSI_BDG_CH3_AI_ISP_TRANSFORM {
	uint32_t raw;
	struct {
		uint32_t AI_ISP_TRANSFORM_EN_CH3         : 1;
		uint32_t IN_FORMAT_CH3                   : 1;
		uint32_t OUT_FORMAT_CH3                  : 2;
		uint32_t ROUND_MODE_CH3                  : 3;
	} bits;
};

union REG_ISP_CSI_BDG_AI_ISP_DEBUG_STATUS_CH3 {
	uint32_t raw;
	struct {
		uint32_t AI_ISP_DEBUG_STATUS_CH3         : 32;
	} bits;
};

union REG_ISP_CSI_BDG_CH_DMA_420_CFG {
	uint32_t raw;
	struct {
		uint32_t CH0_DMA_420_CFG                 : 1;
		uint32_t CH1_DMA_420_CFG                 : 1;
		uint32_t CH2_DMA_420_CFG                 : 1;
		uint32_t CH3_DMA_420_CFG                 : 1;
	} bits;
};

/******************************************/
/*           MODULE DEFINITION            */
/******************************************/
union REG_ISP_DMA_CTL_SYS_CONTROL {
	uint32_t raw;
	struct {
		uint32_t QOS_SEL                         : 1;
		uint32_t SW_QOS                          : 1;
		uint32_t ENABLE_INV                      : 1;
		uint32_t _rsv_3                          : 1;
		uint32_t BASEH                           : 16;
		uint32_t BASE_SEL                        : 1;
		uint32_t STRIDE_SEL                      : 1;
		uint32_t SEGLEN_SEL                      : 1;
		uint32_t SEGNUM_SEL                      : 1;
		uint32_t SLICE_ENABLE                    : 1;
		uint32_t UPDATE_BASE_ADDR                : 1;
		uint32_t INV_SEL                         : 1;
		uint32_t _rsv_27                         : 1;
		uint32_t DBG_SEL                         : 3;
	} bits;
};

union REG_ISP_DMA_CTL_BASE_ADDR {
	uint32_t raw;
	struct {
		uint32_t BASEL                           : 32;
	} bits;
};

union REG_ISP_DMA_CTL_DMA_SEGLEN {
	uint32_t raw;
	struct {
		uint32_t SEGLEN                          : 28;
	} bits;
};

union REG_ISP_DMA_CTL_DMA_STRIDE {
	uint32_t raw;
	struct {
		uint32_t STRIDE                          : 28;
	} bits;
};

union REG_ISP_DMA_CTL_DMA_SEGNUM {
	uint32_t raw;
	struct {
		uint32_t SEGNUM                          : 14;
	} bits;
};

union REG_ISP_DMA_CTL_DMA_STATUS {
	uint32_t raw;
	struct {
		uint32_t STATUS                          : 32;
	} bits;
};

union REG_ISP_DMA_CTL_DMA_SLICESIZE {
	uint32_t raw;
	struct {
		uint32_t SLICE_SIZE                      : 6;
	} bits;
};

union REG_ISP_DMA_CTL_DMA_DUMMY {
	uint32_t raw;
	struct {
		uint32_t DUMMY                           : 16;
		uint32_t PERF_PATCH_ENABLE               : 1;
		uint32_t SEGLEN_LESS16_ENABLE            : 1;
		uint32_t SYNC_PATCH_ENABLE               : 1;
		uint32_t TRIG_PATCH_ENABLE               : 1;
	} bits;
};


/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_LSC_SC_WRAP_0 {
	uint32_t raw;
	struct {
		uint32_t BYPASS                          : 1;
		uint32_t HW_AUTO_CG_EN                   : 1;
		uint32_t LSC_BAYER_STARTING              : 2;
		uint32_t TRIG_STR_1T                     : 1;
		uint32_t LSC_ENABLE                      : 1;
		uint32_t _rsv_6                          : 10;
		uint32_t LSC_DEBUG                       : 16;
	} bits;
};

union REG_ISP_LSC_SC_WRAP_1 {
	uint32_t raw;
	struct {
		uint32_t SW_RST                          : 1;
		uint32_t SHDW_UPDATE_1T                  : 1;
		uint32_t SHDW_READ_SEL                   : 1;
	} bits;
};

union REG_ISP_LSC_SC_WRAP_2 {
	uint32_t raw;
	struct {
		uint32_t IMG_WIDTH                       : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t IMG_HEIGHT                      : 14;
	} bits;
};

union REG_ISP_LSC_SC_WRAP_3 {
	uint32_t raw;
	struct {
		uint32_t LSC_BOUNDARY_INTERPOLATION_LF_RANGE: 6;
		uint32_t _rsv_6                          : 2;
		uint32_t LSC_BOUNDARY_INTERPOLATION_RT_RANGE: 6;
		uint32_t _rsv_14                         : 2;
		uint32_t LSC_BOUNDARY_INTERPOLATION_UP_RANGE: 6;
		uint32_t _rsv_22                         : 2;
		uint32_t LSC_BOUNDARY_INTERPOLATION_DN_RANGE: 6;
	} bits;
};

union REG_ISP_LSC_SC_WRAP_4 {
	uint32_t raw;
	struct {
		uint32_t LSC_BOUNDARY_INTERPOLATION_MODE : 1;
		uint32_t LSC_GAIN_BICUBIC_0_BILINEAR_1   : 1;
		uint32_t LSC_GAIN_3P9_0_4P8_1            : 1;
		uint32_t LSC_OVERFLOW_RST                : 1;
		uint32_t _rsv_4                          : 12;
		uint32_t LSC_STRENGTH                    : 12;
	} bits;
};

union REG_ISP_LSC_SC_WRAP_5 {
	uint32_t raw;
	struct {
		uint32_t LSC_IMGX0                       : 22;
	} bits;
};

union REG_ISP_LSC_SC_WRAP_6 {
	uint32_t raw;
	struct {
		uint32_t LSC_IMGY0                       : 22;
	} bits;
};

union REG_ISP_LSC_SC_WRAP_7 {
	uint32_t raw;
	struct {
		uint32_t LSC_XSTEP                       : 15;
		uint32_t _rsv_15                         : 1;
		uint32_t LSC_YSTEP                       : 15;
	} bits;
};

union REG_ISP_LSC_SC_WRAP_8 {
	uint32_t raw;
	struct {
		uint32_t LSC_INTP_GAIN_MIN               : 26;
	} bits;
};

union REG_ISP_LSC_SC_WRAP_9 {
	uint32_t raw;
	struct {
		uint32_t LSC_INTP_GAIN_MAX               : 26;
	} bits;
};

union REG_ISP_LSC_SC_WRAP_10 {
	uint32_t raw;
	struct {
		uint32_t LSC_SRC_WIDTH                   : 1;
	} bits;
};

union REG_ISP_LSC_SC_WRAP_11 {
	uint32_t raw;
	struct {
		uint32_t LSC_LAUNCH_ADDR                 : 11;
		uint32_t DMA_ENABLE_LSC                  : 1;
		uint32_t _rsv_12                         : 4;
		uint32_t LSC_DMA_ADDR_HIGH8              : 8;
	} bits;
};

union REG_ISP_LSC_SC_WRAP_12 {
	uint32_t raw;
	struct {
		uint32_t LSC_DMA_ADDR_LOW32              : 32;
	} bits;
};

union REG_ISP_LSC_SC_WRAP_13 {
	uint32_t raw;
	struct {
		uint32_t BLC_OFFSET_R                    : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t BLC_OFFSET_GR                   : 12;
	} bits;
};

union REG_ISP_LSC_SC_WRAP_14 {
	uint32_t raw;
	struct {
		uint32_t BLC_OFFSET_GB                   : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t BLC_OFFSET_B                    : 12;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_AE_HIST_AE_HIST_STATUS {
	uint32_t raw;
	struct {
		uint32_t AE_HIST_STATUS                  : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_HIST_GRACE_RESET {
	uint32_t raw;
	struct {
		uint32_t AE_HIST_GRACE_RESET             : 1;
	} bits;
};

union REG_ISP_AE_HIST_AE_HIST_MONITOR {
	uint32_t raw;
	struct {
		uint32_t AE_HIST_MONITOR                 : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_HIST_BYPASS {
	uint32_t raw;
	struct {
		uint32_t AE_HIST_BYPASS                  : 1;
		uint32_t _rsv_1                          : 19;
		uint32_t HIST_ZEROING_ENABLE             : 1;
		uint32_t _rsv_21                         : 7;
		uint32_t FORCE_CLK_ENABLE                : 1;
	} bits;
};

union REG_ISP_AE_HIST_AE_KICKOFF {
	uint32_t raw;
	struct {
		uint32_t AE_ZERO_AE_SUM                  : 1;
		uint32_t _rsv_1                          : 1;
		uint32_t AE_WBGAIN_APPLY                 : 1;
		uint32_t _rsv_3                          : 1;
		uint32_t LOADSHADOWREG                   : 1;
		uint32_t _rsv_5                          : 1;
		uint32_t HIST_ZEROHISTOGRAM              : 1;
		uint32_t _rsv_7                          : 1;
		uint32_t HIST_WBGAIN_APPLY               : 1;
		uint32_t _rsv_9                          : 1;
		uint32_t AE_HIST_SHADOW_SELECT           : 1;
		uint32_t _rsv_11                         : 5;
		uint32_t AE_FACE_ENABLE                  : 4;
	} bits;
};

union REG_ISP_AE_HIST_STS_AE0_HIST_ENABLE {
	uint32_t raw;
	struct {
		uint32_t STS_AE0_HIST_ENABLE             : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t AE0_GAIN_ENABLE                 : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t HIST0_ENABLE                    : 1;
		uint32_t _rsv_9                          : 3;
		uint32_t HIST0_GAIN_ENABLE               : 1;
		uint32_t _rsv_13                         : 3;
		uint32_t IR_AE_ENABLE                    : 1;
		uint32_t _rsv_17                         : 3;
		uint32_t IR_AE_GAIN_ENABLE               : 1;
		uint32_t _rsv_21                         : 3;
		uint32_t IR_HIST_ENABLE                  : 1;
		uint32_t _rsv_25                         : 3;
		uint32_t IR_HIST_GAIN_ENABLE             : 1;
	} bits;
};

union REG_ISP_AE_HIST_STS_AE_OFFSETX {
	uint32_t raw;
	struct {
		uint32_t STS_AE0_OFFSETX                 : 13;
	} bits;
};

union REG_ISP_AE_HIST_STS_AE_OFFSETY {
	uint32_t raw;
	struct {
		uint32_t STS_AE0_OFFSETY                 : 13;
	} bits;
};

union REG_ISP_AE_HIST_STS_AE_NUMXM1 {
	uint32_t raw;
	struct {
		uint32_t STS_AE0_NUMXM1                  : 6;
	} bits;
};

union REG_ISP_AE_HIST_STS_AE_NUMYM1 {
	uint32_t raw;
	struct {
		uint32_t STS_AE0_NUMYM1                  : 5;
	} bits;
};

union REG_ISP_AE_HIST_STS_AE_WIDTH {
	uint32_t raw;
	struct {
		uint32_t STS_AE0_WIDTH                   : 8;
	} bits;
};

union REG_ISP_AE_HIST_STS_AE_HEIGHT {
	uint32_t raw;
	struct {
		uint32_t STS_AE0_HEIGHT                  : 8;
	} bits;
};

union REG_ISP_AE_HIST_STS_AE_STS_DIV {
	uint32_t raw;
	struct {
		uint32_t STS_AE0_STS_DIV                 : 13;
	} bits;
};

union REG_ISP_AE_HIST_STS_HIST_MODE {
	uint32_t raw;
	struct {
		uint32_t STS_HIST0_MODE                  : 2;
	} bits;
};

union REG_ISP_AE_HIST_SHDW_READ_SEL {
	uint32_t raw;
	struct {
		uint32_t SHDW_READ_SEL                   : 1;
	} bits;
};

union REG_ISP_AE_HIST_AE_HIST_MONITOR_SELECT {
	uint32_t raw;
	struct {
		uint32_t AE_HIST_MONITOR_SELECT          : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_HIST_LOCATION {
	uint32_t raw;
	struct {
		uint32_t AE_HIST_LOCATION                : 32;
	} bits;
};

union REG_ISP_AE_HIST_HW_AUTO_CG_EN {
	uint32_t raw;
	struct {
		uint32_t HW_AUTO_CG_EN                   : 1;
	} bits;
};

union REG_ISP_AE_HIST_STS_IR_AE_OFFSETX {
	uint32_t raw;
	struct {
		uint32_t STS_IR_AE_OFFSETX               : 13;
	} bits;
};

union REG_ISP_AE_HIST_STS_IR_AE_OFFSETY {
	uint32_t raw;
	struct {
		uint32_t STS_IR_AE_OFFSETY               : 13;
	} bits;
};

union REG_ISP_AE_HIST_STS_IR_AE_NUMXM1 {
	uint32_t raw;
	struct {
		uint32_t STS_IR_AE_NUMXM1                : 5;
	} bits;
};

union REG_ISP_AE_HIST_STS_IR_AE_NUMYM1 {
	uint32_t raw;
	struct {
		uint32_t STS_IR_AE_NUMYM1                : 5;
	} bits;
};

union REG_ISP_AE_HIST_STS_IR_AE_WIDTH {
	uint32_t raw;
	struct {
		uint32_t STS_IR_AE_WIDTH                 : 10;
	} bits;
};

union REG_ISP_AE_HIST_STS_IR_AE_HEIGHT {
	uint32_t raw;
	struct {
		uint32_t STS_IR_AE_HEIGHT                : 10;
	} bits;
};

union REG_ISP_AE_HIST_STS_IR_AE_STS_DIV {
	uint32_t raw;
	struct {
		uint32_t STS_IR_AE_STS_DIV               : 3;
	} bits;
};

union REG_ISP_AE_HIST_AE_HIST_BAYER_STARTING {
	uint32_t raw;
	struct {
		uint32_t AE_HIST_BAYER_STARTING          : 4;
		uint32_t _rsv_4                          : 12;
		uint32_t FORCE_BAYER_ENABLE              : 1;
	} bits;
};

union REG_ISP_AE_HIST_AE_HIST_DUMMY {
	uint32_t raw;
	struct {
		uint32_t AE_HIST_DUMMY                   : 16;
	} bits;
};

union REG_ISP_AE_HIST_AE_HIST_CHECKSUM {
	uint32_t raw;
	struct {
		uint32_t AE_HIST_CHECKSUM                : 32;
	} bits;
};

union REG_ISP_AE_HIST_WBG_4 {
	uint32_t raw;
	struct {
		uint32_t AE0_WBG_RGAIN                   : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t AE0_WBG_GGAIN                   : 14;
	} bits;
};

union REG_ISP_AE_HIST_WBG_5 {
	uint32_t raw;
	struct {
		uint32_t AE0_WBG_BGAIN                   : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t AE1_WBG_BGAIN                   : 14;
	} bits;
};

union REG_ISP_AE_HIST_WBG_6 {
	uint32_t raw;
	struct {
		uint32_t AE1_WBG_RGAIN                   : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t AE1_WBG_GGAIN                   : 14;
	} bits;
};

union REG_ISP_AE_HIST_WBG_7 {
	uint32_t raw;
	struct {
		uint32_t AE0_WBG_VGAIN                   : 14;
	} bits;
};

union REG_ISP_AE_HIST_DMI_ENABLE {
	uint32_t raw;
	struct {
		uint32_t DMI_ENABLE                      : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t DMI_QOS                         : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t FORCE_DMA_DISABLE               : 1;
		uint32_t _rsv_9                          : 7;
		uint32_t IR_DMI_ENABLE                   : 1;
		uint32_t _rsv_17                         : 3;
		uint32_t IR_DMI_QOS                      : 1;
		uint32_t _rsv_21                         : 3;
		uint32_t IR_FORCE_DMA_DISABLE            : 1;
	} bits;
};

union REG_ISP_AE_HIST_AE_FACE0_LOCATION {
	uint32_t raw;
	struct {
		uint32_t AE_FACE0_OFFSET_X               : 13;
		uint32_t AE_FACE0_OFFSET_Y               : 13;
	} bits;
};

union REG_ISP_AE_HIST_AE_FACE1_LOCATION {
	uint32_t raw;
	struct {
		uint32_t AE_FACE1_OFFSET_X               : 13;
		uint32_t AE_FACE1_OFFSET_Y               : 13;
	} bits;
};

union REG_ISP_AE_HIST_AE_FACE2_LOCATION {
	uint32_t raw;
	struct {
		uint32_t AE_FACE2_OFFSET_X               : 13;
		uint32_t AE_FACE2_OFFSET_Y               : 13;
	} bits;
};

union REG_ISP_AE_HIST_AE_FACE3_LOCATION {
	uint32_t raw;
	struct {
		uint32_t AE_FACE3_OFFSET_X               : 13;
		uint32_t AE_FACE3_OFFSET_Y               : 13;
	} bits;
};

union REG_ISP_AE_HIST_AE_FACE0_SIZE {
	uint32_t raw;
	struct {
		uint32_t AE_FACE0_SIZE_MINUS1_X          : 8;
		uint32_t _rsv_8                          : 8;
		uint32_t AE_FACE0_SIZE_MINUS1_Y          : 8;
	} bits;
};

union REG_ISP_AE_HIST_AE_FACE1_SIZE {
	uint32_t raw;
	struct {
		uint32_t AE_FACE1_SIZE_MINUS1_X          : 8;
		uint32_t _rsv_8                          : 8;
		uint32_t AE_FACE1_SIZE_MINUS1_Y          : 8;
	} bits;
};

union REG_ISP_AE_HIST_AE_FACE2_SIZE {
	uint32_t raw;
	struct {
		uint32_t AE_FACE2_SIZE_MINUS1_X          : 8;
		uint32_t _rsv_8                          : 8;
		uint32_t AE_FACE2_SIZE_MINUS1_Y          : 8;
	} bits;
};

union REG_ISP_AE_HIST_AE_FACE3_SIZE {
	uint32_t raw;
	struct {
		uint32_t AE_FACE3_SIZE_MINUS1_X          : 8;
		uint32_t _rsv_8                          : 8;
		uint32_t AE_FACE3_SIZE_MINUS1_Y          : 8;
	} bits;
};

union REG_ISP_AE_HIST_IR_AE_FACE0_LOCATION {
	uint32_t raw;
	struct {
		uint32_t IR_AE_FACE0_OFFSET_X            : 16;
		uint32_t IR_AE_FACE0_OFFSET_Y            : 16;
	} bits;
};

union REG_ISP_AE_HIST_IR_AE_FACE1_LOCATION {
	uint32_t raw;
	struct {
		uint32_t IR_AE_FACE1_OFFSET_X            : 16;
		uint32_t IR_AE_FACE1_OFFSET_Y            : 16;
	} bits;
};

union REG_ISP_AE_HIST_IR_AE_FACE2_LOCATION {
	uint32_t raw;
	struct {
		uint32_t IR_AE_FACE2_OFFSET_X            : 16;
		uint32_t IR_AE_FACE2_OFFSET_Y            : 16;
	} bits;
};

union REG_ISP_AE_HIST_IR_AE_FACE3_LOCATION {
	uint32_t raw;
	struct {
		uint32_t IR_AE_FACE3_OFFSET_X            : 16;
		uint32_t IR_AE_FACE3_OFFSET_Y            : 16;
	} bits;
};

union REG_ISP_AE_HIST_IR_AE_FACE0_SIZE {
	uint32_t raw;
	struct {
		uint32_t IR_AE_FACE0_SIZE_MINUS1_X       : 7;
		uint32_t _rsv_7                          : 9;
		uint32_t IR_AE_FACE0_SIZE_MINUS1_Y       : 7;
	} bits;
};

union REG_ISP_AE_HIST_IR_AE_FACE1_SIZE {
	uint32_t raw;
	struct {
		uint32_t IR_AE_FACE1_SIZE_MINUS1_X       : 7;
		uint32_t _rsv_7                          : 9;
		uint32_t IR_AE_FACE1_SIZE_MINUS1_Y       : 7;
	} bits;
};

union REG_ISP_AE_HIST_IR_AE_FACE2_SIZE {
	uint32_t raw;
	struct {
		uint32_t IR_AE_FACE2_SIZE_MINUS1_X       : 7;
		uint32_t _rsv_7                          : 9;
		uint32_t IR_AE_FACE2_SIZE_MINUS1_Y       : 7;
	} bits;
};

union REG_ISP_AE_HIST_IR_AE_FACE3_SIZE {
	uint32_t raw;
	struct {
		uint32_t IR_AE_FACE3_SIZE_MINUS1_X       : 7;
		uint32_t _rsv_7                          : 9;
		uint32_t IR_AE_FACE3_SIZE_MINUS1_Y       : 7;
	} bits;
};

union REG_ISP_AE_HIST_AE_FACE_ENABLE_CTRL {
	uint32_t raw;
	struct {
		uint32_t AE_FACE0_ENABLE                 : 1;
		uint32_t AE_FACE1_ENABLE                 : 1;
		uint32_t AE_FACE2_ENABLE                 : 1;
		uint32_t AE_FACE3_ENABLE                 : 1;
	} bits;
};

union REG_ISP_AE_HIST_AE_FACE0_STS_DIV {
	uint32_t raw;
	struct {
		uint32_t AE_FACE0_STS_DIV                : 13;
	} bits;
};

union REG_ISP_AE_HIST_AE_FACE1_STS_DIV {
	uint32_t raw;
	struct {
		uint32_t AE_FACE1_STS_DIV                : 13;
	} bits;
};

union REG_ISP_AE_HIST_AE_FACE2_STS_DIV {
	uint32_t raw;
	struct {
		uint32_t AE_FACE2_STS_DIV                : 13;
	} bits;
};

union REG_ISP_AE_HIST_AE_FACE3_STS_DIV {
	uint32_t raw;
	struct {
		uint32_t AE_FACE3_STS_DIV                : 13;
	} bits;
};

union REG_ISP_AE_HIST_STS_ENABLE {
	uint32_t raw;
	struct {
		uint32_t STS_AWB_ENABLE                  : 1;
	} bits;
};

union REG_ISP_AE_HIST_AE_ALGO_ENABLE {
	uint32_t raw;
	struct {
		uint32_t _rsv_0                          : 1;
		uint32_t AE_ALGO_ENABLE                  : 1;
	} bits;
};

union REG_ISP_AE_HIST_AE_HIST_LOW {
	uint32_t raw;
	struct {
		uint32_t AE_HIST_LOW                     : 8;
	} bits;
};

union REG_ISP_AE_HIST_AE_HIST_HIGH {
	uint32_t raw;
	struct {
		uint32_t _rsv_0                          : 8;
		uint32_t AE_HIST_HIGH                    : 8;
	} bits;
};

union REG_ISP_AE_HIST_AE_TOP {
	uint32_t raw;
	struct {
		uint32_t AE_AWB_TOP                      : 12;
	} bits;
};

union REG_ISP_AE_HIST_AE_BOT {
	uint32_t raw;
	struct {
		uint32_t _rsv_0                          : 12;
		uint32_t AE_AWB_BOT                      : 12;
	} bits;
};

union REG_ISP_AE_HIST_AE_OVEREXP_THR {
	uint32_t raw;
	struct {
		uint32_t AE_OVEREXP_THR                  : 10;
	} bits;
};

union REG_ISP_AE_HIST_AE_NUM_GAPLINE {
	uint32_t raw;
	struct {
		uint32_t AE_NUM_GAPLINE                  : 1;
	} bits;
};

union REG_ISP_AE_HIST_AE_CHOOSE_GR_EN {
	uint32_t raw;
	struct {
		uint32_t AE_CHOOSE_GR_EN                 : 1;
	} bits;
};

union REG_ISP_AE_HIST_FRAME_COUNTER_AWB {
	uint32_t raw;
	struct {
		uint32_t E_COUNTER_AWB_LE                : 16;
		uint32_t E_COUNTER_AWB_SE                : 16;
	} bits;
};

union REG_ISP_AE_HIST_AE_SIMPLE2A_RESULT_LUMA_SE {
	uint32_t raw;
	struct {
		uint32_t SIMPLE2A_RESULT_LUMA_SE         : 10;
	} bits;
};

union REG_ISP_AE_HIST_AE_SIMPLE2A_RESULT_RGAIN_SE {
	uint32_t raw;
	struct {
		uint32_t SIMPLE2A_RESULT_RGAIN_SE        : 10;
	} bits;
};

union REG_ISP_AE_HIST_AE_SIMPLE2A_RESULT_BGAIN_SE {
	uint32_t raw;
	struct {
		uint32_t SIMPLE2A_RESULT_BGAIN_SE        : 10;
	} bits;
};

union REG_ISP_AE_HIST_AE_SIMPLE2A_RESULT_LUMA_LE {
	uint32_t raw;
	struct {
		uint32_t SIMPLE2A_RESULT_LUMA_LE         : 10;
	} bits;
};

union REG_ISP_AE_HIST_AE_SIMPLE2A_RESULT_RGAIN_LE {
	uint32_t raw;
	struct {
		uint32_t SIMPLE2A_RESULT_RGAIN_LE        : 10;
	} bits;
};

union REG_ISP_AE_HIST_AE_SIMPLE2A_RESULT_BGAIN_LE {
	uint32_t raw;
	struct {
		uint32_t SIMPLE2A_RESULT_BGAIN_LE        : 10;
	} bits;
};

union REG_ISP_AE_HIST_AE_FACE0_RESULT_SE {
	uint32_t raw;
	struct {
		uint32_t AE_FACE0_RESULT_R_SUM_SE        : 10;
		uint32_t _rsv_10                         : 1;
		uint32_t AE_FACE0_RESULT_G_SUM_SE        : 10;
		uint32_t _rsv_21                         : 1;
		uint32_t AE_FACE0_RESULT_B_SUM_SE        : 10;
	} bits;
};

union REG_ISP_AE_HIST_AE_FACE1_RESULT_SE {
	uint32_t raw;
	struct {
		uint32_t AE_FACE1_RESULT_R_SUM_SE        : 10;
		uint32_t _rsv_10                         : 1;
		uint32_t AE_FACE1_RESULT_G_SUM_SE        : 10;
		uint32_t _rsv_21                         : 1;
		uint32_t AE_FACE1_RESULT_B_SUM_SE        : 10;
	} bits;
};

union REG_ISP_AE_HIST_AE_FACE2_RESULT_SE {
	uint32_t raw;
	struct {
		uint32_t AE_FACE2_RESULT_R_SUM_SE        : 10;
		uint32_t _rsv_10                         : 1;
		uint32_t AE_FACE2_RESULT_G_SUM_SE        : 10;
		uint32_t _rsv_21                         : 1;
		uint32_t AE_FACE2_RESULT_B_SUM_SE        : 10;
	} bits;
};

union REG_ISP_AE_HIST_AE_FACE3_RESULT_SE {
	uint32_t raw;
	struct {
		uint32_t AE_FACE3_RESULT_R_SUM_SE        : 10;
		uint32_t _rsv_10                         : 1;
		uint32_t AE_FACE3_RESULT_G_SUM_SE        : 10;
		uint32_t _rsv_21                         : 1;
		uint32_t AE_FACE3_RESULT_B_SUM_SE        : 10;
	} bits;
};

union REG_ISP_AE_HIST_AE_FACE0_RESULT_LE {
	uint32_t raw;
	struct {
		uint32_t AE_FACE0_RESULT_R_SUM_LE        : 10;
		uint32_t _rsv_10                         : 1;
		uint32_t AE_FACE0_RESULT_G_SUM_LE        : 10;
		uint32_t _rsv_21                         : 1;
		uint32_t AE_FACE0_RESULT_B_SUM_LE        : 10;
	} bits;
};

union REG_ISP_AE_HIST_AE_FACE1_RESULT_LE {
	uint32_t raw;
	struct {
		uint32_t AE_FACE1_RESULT_R_SUM_LE        : 10;
		uint32_t _rsv_10                         : 1;
		uint32_t AE_FACE1_RESULT_G_SUM_LE        : 10;
		uint32_t _rsv_21                         : 1;
		uint32_t AE_FACE1_RESULT_B_SUM_LE        : 10;
	} bits;
};

union REG_ISP_AE_HIST_AE_FACE2_RESULT_LE {
	uint32_t raw;
	struct {
		uint32_t AE_FACE2_RESULT_R_SUM_LE        : 10;
		uint32_t _rsv_10                         : 1;
		uint32_t AE_FACE2_RESULT_G_SUM_LE        : 10;
		uint32_t _rsv_21                         : 1;
		uint32_t AE_FACE2_RESULT_B_SUM_LE        : 10;
	} bits;
};

union REG_ISP_AE_HIST_AE_FACE3_RESULT_LE {
	uint32_t raw;
	struct {
		uint32_t AE_FACE3_RESULT_R_SUM_LE        : 10;
		uint32_t _rsv_10                         : 1;
		uint32_t AE_FACE3_RESULT_G_SUM_LE        : 10;
		uint32_t _rsv_21                         : 1;
		uint32_t AE_FACE3_RESULT_B_SUM_LE        : 10;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_00 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_00                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_01 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_01                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_02 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_02                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_03 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_03                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_04 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_04                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_05 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_05                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_06 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_06                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_07 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_07                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_08 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_08                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_09 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_09                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_10 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_10                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_11 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_11                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_12 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_12                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_13 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_13                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_14 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_14                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_15 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_15                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_16 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_16                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_17 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_17                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_18 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_18                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_19 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_19                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_20 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_20                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_21 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_21                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_22 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_22                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_23 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_23                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_24 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_24                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_25 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_25                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_26 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_26                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_27 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_27                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_28 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_28                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_29 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_29                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_30 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_30                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_AE_WGT_31 {
	uint32_t raw;
	struct {
		uint32_t AE_WGT_31                       : 32;
	} bits;
};

union REG_ISP_AE_HIST_LE_AE_BLC_OFFSET_R {
	uint32_t raw;
	struct {
		uint32_t EG_AE_BLC_OFFSET_R              : 12;
	} bits;
};

union REG_ISP_AE_HIST_LE_AE_BLC_OFFSET_GR {
	uint32_t raw;
	struct {
		uint32_t EG_AE_BLC_OFFSET_GR             : 12;
	} bits;
};

union REG_ISP_AE_HIST_LE_AE_BLC_OFFSET_GB {
	uint32_t raw;
	struct {
		uint32_t EG_AE_BLC_OFFSET_GB             : 12;
	} bits;
};

union REG_ISP_AE_HIST_LE_AE_BLC_OFFSET_B {
	uint32_t raw;
	struct {
		uint32_t EG_AE_BLC_OFFSET_B              : 12;
	} bits;
};

union REG_ISP_AE_HIST_LE_AE_BLC_OFFSET_IR {
	uint32_t raw;
	struct {
		uint32_t EG_AE_BLC_OFFSET_IR             : 12;
	} bits;
};

union REG_ISP_AE_HIST_LE_AE_BLC_GAIN_R {
	uint32_t raw;
	struct {
		uint32_t EG_AE_BLC_GAIN_R                : 16;
	} bits;
};

union REG_ISP_AE_HIST_LE_AE_BLC_GAIN_GR {
	uint32_t raw;
	struct {
		uint32_t EG_AE_BLC_GAIN_GR               : 16;
	} bits;
};

union REG_ISP_AE_HIST_LE_AE_BLC_GAIN_GB {
	uint32_t raw;
	struct {
		uint32_t EG_AE_BLC_GAIN_GB               : 16;
	} bits;
};

union REG_ISP_AE_HIST_LE_AE_BLC_GAIN_B {
	uint32_t raw;
	struct {
		uint32_t EG_AE_BLC_GAIN_B                : 16;
	} bits;
};

union REG_ISP_AE_HIST_LE_AE_BLC_GAIN_IR {
	uint32_t raw;
	struct {
		uint32_t EG_AE_BLC_GAIN_IR               : 16;
	} bits;
};

union REG_ISP_AE_HIST_LE_AE_BLC_ENABLE {
	uint32_t raw;
	struct {
		uint32_t EG_AE_BLC_ENABLE                : 1;
	} bits;
};

union REG_ISP_AE_HIST_SE_AE_BLC_OFFSET_R {
	uint32_t raw;
	struct {
		uint32_t EG_AE_BLC_OFFSET_R              : 12;
	} bits;
};

union REG_ISP_AE_HIST_SE_AE_BLC_OFFSET_GR {
	uint32_t raw;
	struct {
		uint32_t EG_AE_BLC_OFFSET_GR             : 12;
	} bits;
};

union REG_ISP_AE_HIST_SE_AE_BLC_OFFSET_GB {
	uint32_t raw;
	struct {
		uint32_t EG_AE_BLC_OFFSET_GB             : 12;
	} bits;
};

union REG_ISP_AE_HIST_SE_AE_BLC_OFFSET_B {
	uint32_t raw;
	struct {
		uint32_t EG_AE_BLC_OFFSET_B              : 12;
	} bits;
};

union REG_ISP_AE_HIST_SE_AE_BLC_OFFSET_IR {
	uint32_t raw;
	struct {
		uint32_t EG_AE_BLC_OFFSET_IR             : 12;
	} bits;
};

union REG_ISP_AE_HIST_SE_AE_BLC_GAIN_R {
	uint32_t raw;
	struct {
		uint32_t EG_AE_BLC_GAIN_R                : 16;
	} bits;
};

union REG_ISP_AE_HIST_SE_AE_BLC_GAIN_GR {
	uint32_t raw;
	struct {
		uint32_t EG_AE_BLC_GAIN_GR               : 16;
	} bits;
};

union REG_ISP_AE_HIST_SE_AE_BLC_GAIN_GB {
	uint32_t raw;
	struct {
		uint32_t EG_AE_BLC_GAIN_GB               : 16;
	} bits;
};

union REG_ISP_AE_HIST_SE_AE_BLC_GAIN_B {
	uint32_t raw;
	struct {
		uint32_t EG_AE_BLC_GAIN_B                : 16;
	} bits;
};

union REG_ISP_AE_HIST_SE_AE_BLC_GAIN_IR {
	uint32_t raw;
	struct {
		uint32_t EG_AE_BLC_GAIN_IR               : 16;
	} bits;
};

union REG_ISP_AE_HIST_SE_AE_BLC_ENABLE {
	uint32_t raw;
	struct {
		uint32_t EG_AE_BLC_ENABLE                : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_WDMA_CORE_SHADOW_RD_SEL   {
	uint32_t raw;
	struct {
		uint32_t SHADOW_RD_SEL                   : 1;
		uint32_t ABORT_MODE                      : 1;
	} bits;
};

union REG_WDMA_CORE_IP_DISABLE {
	uint32_t raw;
	struct {
		uint32_t IP_DISABLE                      : 32;
	} bits;
};

union REG_WDMA_CORE_DISABLE_SEGLEN {
	uint32_t raw;
	struct {
		uint32_t SEGLEN_DISABLE                  : 32;
	} bits;
};

union REG_WDMA_CORE_UP_RING_BASE {
	uint32_t raw;
	struct {
		uint32_t UP_RING_BASE                    : 32;
	} bits;
};

union REG_WDMA_CORE_NORM_STATUS0 {
	uint32_t raw;
	struct {
		uint32_t ABORT_DONE                      : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t ERROR_AXI                       : 1;
		uint32_t ERROR_DMI                       : 1;
		uint32_t SLOT_FULL                       : 1;
		uint32_t _rsv_7                          : 1;
		uint32_t ERROR_ID                        : 5;
		uint32_t _rsv_13                         : 3;
		uint32_t DMA_VERSION                     : 16;
	} bits;
};

union REG_WDMA_CORE_NORM_STATUS1 {
	uint32_t raw;
	struct {
		uint32_t ID_IDLE                         : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_EN {
	uint32_t raw;
	struct {
		uint32_t RING_ENABLE                     : 32;
	} bits;
};

union REG_WDMA_CORE_NORM_PERF  {
	uint32_t raw;
	struct {
		uint32_t BWLWIN                          : 10;
		uint32_t BWLTXN                          : 6;
		uint32_t QOSO_TH                         : 4;
		uint32_t QOSO_EN                         : 1;
	} bits;
};

union REG_WDMA_CORE_RING_PATCH_ENABLE {
	uint32_t raw;
	struct {
		uint32_t RING_PATCH_ENABLE               : 32;
	} bits;
};

union REG_WDMA_CORE_SET_RING_BASE {
	uint32_t raw;
	struct {
		uint32_t SET_RING_BASE                   : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BASE_ADDR_L {
	uint32_t raw;
	struct {
		uint32_t RING_BASE_L                     : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BASE_ADDR_H {
	uint32_t raw;
	struct {
		uint32_t RING_BASE_H                     : 16;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE0 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE0                      : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE1 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE1                      : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE2 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE2                      : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE3 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE3                      : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE4 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE4                      : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE5 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE5                      : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE6 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE6                      : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE7 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE7                      : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE8 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE8                      : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE9 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE9                      : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE10 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE10                     : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE11 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE11                     : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE12 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE12                     : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE13 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE13                     : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE14 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE14                     : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE15 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE15                     : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE16 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE16                     : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE17 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE17                     : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE18 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE18                     : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE19 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE19                     : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE20 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE20                     : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE21 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE21                     : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE22 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE22                     : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE23 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE23                     : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE24 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE24                     : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE25 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE25                     : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE26 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE26                     : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE27 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE27                     : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE28 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE28                     : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE29 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE29                     : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE30 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE30                     : 32;
	} bits;
};

union REG_WDMA_CORE_RING_BUFFER_SIZE31 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE31                     : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS0 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR0                  : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS1 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR1                  : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS2 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR2                  : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS3 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR3                  : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS4 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR4                  : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS5 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR5                  : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS6 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR6                  : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS7 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR7                  : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS8 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR8                  : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS9 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR9                  : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS10 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR10                 : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS11 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR11                 : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS12 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR12                 : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS13 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR13                 : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS14 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR14                 : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS15 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR15                 : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS16 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR16                 : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS17 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR17                 : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS18 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR18                 : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS19 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR19                 : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS20 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR20                 : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS21 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR21                 : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS22 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR22                 : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS23 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR23                 : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS24 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR24                 : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS25 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR25                 : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS26 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR26                 : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS27 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR27                 : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS28 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR28                 : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS29 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR29                 : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS30 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR30                 : 32;
	} bits;
};

union REG_WDMA_CORE_NEXT_DMA_ADDR_STS31 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR31                 : 32;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_RAW_TOP0_RAW_TOP_READ_SEL {
	uint32_t raw;
	struct {
		uint32_t SHDW_READ_SEL_2X                : 1;
	} bits;
};

union REG_RAW_TOP0_CHK_SUM_EN {
	uint32_t raw;
	struct {
		uint32_t UP_PQ_EN_2X                     : 1;
	} bits;
};

union REG_RAW_TOP0_RAW_2 {
	uint32_t raw;
	struct {
		uint32_t IMG_WIDTHM_1_2X                 : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t IMG_HEIGHTM_1_2X                : 14;
	} bits;
};

union REG_RAW_TOP0_RAW_3 {
	uint32_t raw;
	struct {
		uint32_t IMG_WIDTHM_0                    : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t IMG_HEIGHTM_0                   : 14;
	} bits;
};

union REG_RAW_TOP0_RAW_BAYER_TYPE_TOPLEFT {
	uint32_t raw;
	struct {
		uint32_t BAYER_TYPE_PRECROP              : 2;
		uint32_t BAYER_TYPE_2X                   : 2;
	} bits;
};

union REG_RAW_TOP0_PASS_SEL {
	uint32_t raw;
	struct {
		uint32_t FIFO_REVERSE_2X                 : 1;
		uint32_t PASS_SEL                        : 1;
		uint32_t PASS_CNT_M1                     : 8;
		uint32_t PASS_SEL_SE_BNR                 : 1;
		uint32_t PASS_CNT_M1_BNR                 : 8;
	} bits;
};

union REG_RAW_TOP0_IP_CFG_GCLK_EN {
	uint32_t raw;
	struct {
		uint32_t CFG_GCLK_EN_CLSC                : 1;
	} bits;
};

union REG_RAW_TOP0_IP_BYPASS {
	uint32_t raw;
	struct {
		uint32_t BYPASS_SE_BNR                   : 1;
		uint32_t _rsv_1                          : 2;
		uint32_t BYPASS_CLSC_LE                  : 1;
		uint32_t BYPASS_CLSC_SE                  : 1;
	} bits;
};

union REG_RAW_TOP0_PG_CFG0 {
	uint32_t raw;
	struct {
		uint32_t PG_ENABLE                       : 1;
		uint32_t CURSER_EN                       : 1;
		uint32_t X_CURSER                        : 14;
		uint32_t Y_CURSER                        : 14;
	} bits;
};

union REG_RAW_TOP0_PG_CFG1 {
	uint32_t raw;
	struct {
		uint32_t CURSER_VALUE                    : 16;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_RAW_TOP1_RAW_TOP_READ_SEL {
	uint32_t raw;
	struct {
		uint32_t SHDW_READ_SEL_1X                : 1;
	} bits;
};

union REG_RAW_TOP1_CHK_SUM_EN {
	uint32_t raw;
	struct {
		uint32_t UP_PQ_EN_1X                     : 1;
		uint32_t _rsv_1                          : 15;
		uint32_t CHK_SUM_EN                      : 1;
	} bits;
};

union REG_RAW_TOP1_RAW_2 {
	uint32_t raw;
	struct {
		uint32_t IMG_WIDTHM_1_1X                 : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t IMG_HEIGHTM_1_1X                : 14;
	} bits;
};

union REG_RAW_TOP1_RAW_4 {
	uint32_t raw;
	struct {
		uint32_t YUV_PATH                        : 1;
		uint32_t FRAME_DONE_SEL_TAIL0_ALL1       : 1;
	} bits;
};

union REG_RAW_TOP1_RAW_TOP_STATUS {
	uint32_t raw;
	struct {
		uint32_t RAW_TOP_STATUS                  : 32;
	} bits;
};

union REG_RAW_TOP1_RAW_TOP_DEBUG {
	uint32_t raw;
	struct {
		uint32_t RAW_TOP_DEBUG                   : 32;
	} bits;
};

union REG_RAW_TOP1_RAW_BAYER_TYPE {
	uint32_t raw;
	struct {
		uint32_t BAYER_TYPE_1X                   : 2;
	} bits;
};

union REG_RAW_TOP1_FIFO_REVERSE {
	uint32_t raw;
	struct {
		uint32_t FIFO_REVERSE_1X                 : 1;
	} bits;
};

union REG_RAW_TOP1_RAW_TOP_DMA_IDLE {
	uint32_t raw;
	struct {
		uint32_t RAW_TOP_DMA_IDLE                : 32;
	} bits;
};

union REG_RAW_TOP1_RAW_TOP_CHECKSUM {
	uint32_t raw;
	struct {
		uint32_t RAW_TOP_CHECKSUM                : 32;
	} bits;
};

union REG_RAW_TOP1_IP_CFG_GCLK_EN {
	uint32_t raw;
	struct {
		uint32_t _rsv_0                          : 1;
		uint32_t CFG_GCLK_EN_LLSC                : 1;
		uint32_t CFG_GCLK_EN_DRC                 : 1;

	} bits;
};

union REG_RAW_TOP1_IP_BYPASS {
	uint32_t raw;
	struct {
		uint32_t _rsv_0                          : 1;
		uint32_t BYPASS_MAP_CURVE                : 1;
		uint32_t BYPASS_LLSC                     : 1;
	} bits;
};

union REG_RAW_TOP1_AF_BLC_CFG0 {
	uint32_t raw;
	struct {
		uint32_t BLC_OFFSET_B                    : 12;
		uint32_t BLC_OFFSET_GB                   : 12;
	} bits;
};

union REG_RAW_TOP1_AF_BLC_CFG1 {
	uint32_t raw;
	struct {
		uint32_t BLC_OFFSET_GR                   : 12;
		uint32_t BLC_OFFSET_R                    : 12;
	} bits;
};

union REG_RAW_TOP1_AF_BLC_CFG2 {
	uint32_t raw;
	struct {
		uint32_t BLC_GAIN_B                      : 16;
		uint32_t BLC_GAIN_GB                     : 16;
	} bits;
};

union REG_RAW_TOP1_AF_BLC_CFG3 {
	uint32_t raw;
	struct {
		uint32_t BLC_GAIN_GR                     : 16;
		uint32_t BLC_GAIN_R                      : 16;
	} bits;
};

union REG_RAW_TOP1_AF_BLC_CFG4 {
	uint32_t raw;
	struct {
		uint32_t BLC_ENABLE                      : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_AF_STATUS {
	uint32_t raw;
	struct {
		uint32_t AF_STATUS                       : 32;
	} bits;
};

union REG_ISP_AF_GRACE_RESET {
	uint32_t raw;
	struct {
		uint32_t AF_GRACE_RESET                  : 1;
	} bits;
};

union REG_ISP_AF_MONITOR {
	uint32_t raw;
	struct {
		uint32_t AF_MONITOR                      : 32;
	} bits;
};

union REG_ISP_AF_BYPASS {
	uint32_t raw;
	struct {
		uint32_t AF_BYPASS                       : 1;
	} bits;
};

union REG_ISP_AF_KICKOFF {
	uint32_t raw;
	struct {
		uint32_t AF_ENABLE                       : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t AF_WBGAIN_APPLY                 : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t AF_REVERT_EXPOSURE              : 1;
		uint32_t _rsv_9                          : 7;
		uint32_t AF_GAIN_ENABLE                  : 1;
	} bits;
};

union REG_ISP_AF_ENABLES {
	uint32_t raw;
	struct {
		uint32_t AF_HORIZON_0_ENABLE             : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t AF_HORIZON_1_ENABLE             : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t AF_VERTICAL_0_ENABLE            : 1;
		uint32_t _rsv_9                          : 3;
		uint32_t AF_GAMMA_ENABLE                 : 1;
		uint32_t _rsv_13                         : 3;
		uint32_t AF_DPC_ENABLE                   : 1;
		uint32_t _rsv_17                         : 3;
		uint32_t AF_HLC_ENABLE                   : 1;
		uint32_t _rsv_21                         : 3;
		uint32_t AF_SHADOW_SELECT                : 1;
		uint32_t _rsv_25                         : 3;
		uint32_t FORCE_CLK_ENABLE                : 1;
	} bits;
};

union REG_ISP_AF_OFFSET_X {
	uint32_t raw;
	struct {
		uint32_t AF_OFFSET_X                     : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t AF_OFFSET_Y                     : 13;
	} bits;
};

union REG_ISP_AF_MXN_IMAGE_WIDTH_M1 {
	uint32_t raw;
	struct {
		uint32_t AF_MXN_IMAGE_WIDTH              : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t AF_MXN_IMAGE_HEIGHT             : 13;
	} bits;
};

union REG_ISP_AF_BLOCK_WIDTH {
	uint32_t raw;
	struct {
		uint32_t AF_BLOCK_WIDTH                  : 8;
	} bits;
};

union REG_ISP_AF_BLOCK_HEIGHT {
	uint32_t raw;
	struct {
		uint32_t AF_BLOCK_HEIGHT                 : 8;
	} bits;
};

union REG_ISP_AF_BLOCK_NUM_X {
	uint32_t raw;
	struct {
		uint32_t AF_BLOCK_NUM_X                  : 5;
	} bits;
};

union REG_ISP_AF_BLOCK_NUM_Y {
	uint32_t raw;
	struct {
		uint32_t AF_BLOCK_NUM_Y                  : 4;
		uint32_t _rsv_4                          : 12;
		uint32_t FORCE_BAYER_ENABLE              : 1;
	} bits;
};

union REG_ISP_AF_HOR_LOW_PASS_VALUE_SHIFT {
	uint32_t raw;
	struct {
		uint32_t AF_HOR_LOW_PASS_VALUE_SHIFT     : 4;
	} bits;
};

union REG_ISP_AF_CORNING_OFFSET_HORIZONTAL_0 {
	uint32_t raw;
	struct {
		uint32_t AF_CORNING_OFFSET_HORIZONTAL_0  : 8;
	} bits;
};

union REG_ISP_AF_CORNING_OFFSET_HORIZONTAL_1 {
	uint32_t raw;
	struct {
		uint32_t AF_CORNING_OFFSET_HORIZONTAL_1  : 8;
	} bits;
};

union REG_ISP_AF_CORNING_OFFSET_VERTICAL {
	uint32_t raw;
	struct {
		uint32_t AF_CORNING_OFFSET_VERTICAL      : 8;
	} bits;
};

union REG_ISP_AF_HIGH_Y_THRE {
	uint32_t raw;
	struct {
		uint32_t AF_HIGH_Y_THRE                  : 8;
	} bits;
};

union REG_ISP_AF_LOW_PASS_HORIZON {
	uint32_t raw;
	struct {
		uint32_t AF_LOW_PASS_HORIZON_0           : 6;
		uint32_t AF_LOW_PASS_HORIZON_1           : 6;
		uint32_t AF_LOW_PASS_HORIZON_2           : 6;
		uint32_t AF_LOW_PASS_HORIZON_3           : 6;
		uint32_t AF_LOW_PASS_HORIZON_4           : 6;
	} bits;
};

union REG_ISP_AF_LOCATION {
	uint32_t raw;
	struct {
		uint32_t AF_LOCATION                     : 32;
	} bits;
};

union REG_ISP_AF_HIGH_PASS_HORIZON_0 {
	uint32_t raw;
	struct {
		uint32_t AF_HIGH_PASS_HORIZON_0_0        : 6;
		uint32_t AF_HIGH_PASS_HORIZON_0_1        : 6;
		uint32_t AF_HIGH_PASS_HORIZON_0_2        : 6;
		uint32_t AF_HIGH_PASS_HORIZON_0_3        : 6;
		uint32_t AF_HIGH_PASS_HORIZON_0_4        : 6;
	} bits;
};

union REG_ISP_AF_HIGH_PASS_HORIZON_1 {
	uint32_t raw;
	struct {
		uint32_t AF_HIGH_PASS_HORIZON_1_0        : 6;
		uint32_t AF_HIGH_PASS_HORIZON_1_1        : 6;
		uint32_t AF_HIGH_PASS_HORIZON_1_2        : 6;
		uint32_t AF_HIGH_PASS_HORIZON_1_3        : 6;
		uint32_t AF_HIGH_PASS_HORIZON_1_4        : 6;
	} bits;
};

union REG_ISP_AF_HIGH_PASS_VERTICAL_0 {
	uint32_t raw;
	struct {
		uint32_t AF_HIGH_PASS_VERTICAL_0_0       : 6;
		uint32_t AF_HIGH_PASS_VERTICAL_0_1       : 6;
		uint32_t AF_HIGH_PASS_VERTICAL_0_2       : 6;
	} bits;
};

union REG_ISP_AF_MEM_SW_MODE {
	uint32_t raw;
	struct {
		uint32_t AF_MEM_SW_MODE                  : 1;
		uint32_t AF_R_MEM_SEL                    : 1;
		uint32_t AF_G_MEM_SEL                    : 1;
		uint32_t AF_B_MEM_SEL                    : 1;
		uint32_t AF_BLK_DIV_MEM_SEL              : 1;
		uint32_t AF_GAMMA_G_MEM_SEL              : 1;
		uint32_t AF_MAGFACTOR_MEM_SEL            : 1;
		uint32_t AF_BLK_DIV_DFF_SEL              : 1;
		uint32_t AF_GAMMA_G_DFF_SEL              : 1;
		uint32_t AF_MAGFACTOR_DFF_SEL            : 1;
	} bits;
};

union REG_ISP_AF_MONITOR_SELECT {
	uint32_t raw;
	struct {
		uint32_t AF_MONITOR_SELECT               : 32;
	} bits;
};

union REG_ISP_AF_IMAGE_WIDTH {
	uint32_t raw;
	struct {
		uint32_t AF_IMAGE_WIDTH                  : 16;
	} bits;
};

union REG_ISP_AF_DUMMY {
	uint32_t raw;
	struct {
		uint32_t AF_DUMMY                        : 16;
	} bits;
};

union REG_ISP_AF_MEM_SW_RADDR {
	uint32_t raw;
	struct {
		uint32_t AF_SW_RADDR                     : 7;
	} bits;
};

union REG_ISP_AF_MEM_SW_RDATA {
	uint32_t raw;
	struct {
		uint32_t AF_RDATA                        : 31;
		uint32_t AF_SW_READ                      : 1;
	} bits;
};

union REG_ISP_AF_MXN_BORDER {
	uint32_t raw;
	struct {
		uint32_t AF_MXN_BORDER                   : 2;
	} bits;
};

union REG_ISP_AF_TH_LOW    {
	uint32_t raw;
	struct {
		uint32_t AF_TH_LOW                       : 8;
		uint32_t _rsv_8                          : 8;
		uint32_t AF_TH_HIGH                      : 8;
	} bits;
};

union REG_ISP_AF_GAIN_LOW  {
	uint32_t raw;
	struct {
		uint32_t AF_GAIN_LOW                     : 8;
		uint32_t _rsv_8                          : 8;
		uint32_t AF_GAIN_HIGH                    : 8;
	} bits;
};

union REG_ISP_AF_SLOP_LOW {
	uint32_t raw;
	struct {
		uint32_t AF_SLOP_LOW                     : 4;
		uint32_t AF_SLOP_HIGH                    : 4;
	} bits;
};

union REG_ISP_AF_DMI_ENABLE {
	uint32_t raw;
	struct {
		uint32_t DMI_ENABLE                      : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t DMI_QOS                         : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t FORCE_DMA_DISABLE               : 1;
	} bits;
};

union REG_ISP_AF_SQUARE_ENABLE {
	uint32_t raw;
	struct {
		uint32_t AF_SQUARE_ENABLE                : 1;
	} bits;
};

union REG_ISP_AF_OUTSHIFT {
	uint32_t raw;
	struct {
		uint32_t _rsv_0                          : 4;
		uint32_t AF_OUTSHIFT                     : 3;
	} bits;
};

union REG_ISP_AF_NUM_GAPLINE {
	uint32_t raw;
	struct {
		uint32_t AF_NUM_GAPLINE                  : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_GMS_STATUS {
	uint32_t raw;
	struct {
		uint32_t GMS_STATUS                      : 32;
	} bits;
};

union REG_ISP_GMS_GRACE_RESET {
	uint32_t raw;
	struct {
		uint32_t GMS_GRACE_RESET                 : 1;
	} bits;
};

union REG_ISP_GMS_MONITOR {
	uint32_t raw;
	struct {
		uint32_t GMS_MONITOR                     : 32;
	} bits;
};

union REG_ISP_GMS_ENABLE {
	uint32_t raw;
	struct {
		uint32_t GMS_ENABLE                      : 1;
		uint32_t OUT_SHIFTBIT                    : 3;
		uint32_t _rsv_4                          : 12;
		uint32_t FORCE_BAYER_ENABLE              : 1;
		uint32_t _rsv_17                         : 11;
		uint32_t FORCE_CLK_ENABLE                : 1;
	} bits;
};

union REG_ISP_GMS_FLOW {
	uint32_t raw;
	struct {
		uint32_t GMS_ZEROGMSOGRAM                : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t GMS_SHADOW_SELECT               : 1;
	} bits;
};

union REG_ISP_GMS_START_X {
	uint32_t raw;
	struct {
		uint32_t GMS_START_X                     : 13;
	} bits;
};

union REG_ISP_GMS_START_Y {
	uint32_t raw;
	struct {
		uint32_t GMS_START_Y                     : 13;
	} bits;
};

union REG_ISP_GMS_LOCATION {
	uint32_t raw;
	struct {
		uint32_t GMS_LOCATION                    : 32;
	} bits;
};

union REG_ISP_GMS_X_SIZEM1 {
	uint32_t raw;
	struct {
		uint32_t GMS_X_SIZEM1                    : 10;
	} bits;
};

union REG_ISP_GMS_Y_SIZEM1 {
	uint32_t raw;
	struct {
		uint32_t GMS_Y_SIZEM1                    : 10;
	} bits;
};

union REG_ISP_GMS_X_GAP {
	uint32_t raw;
	struct {
		uint32_t GMS_X_GAP                       : 10;
	} bits;
};

union REG_ISP_GMS_Y_GAP {
	uint32_t raw;
	struct {
		uint32_t GMS_Y_GAP                       : 10;
	} bits;
};

union REG_ISP_GMS_DUMMY {
	uint32_t raw;
	struct {
		uint32_t GMS_DUMMY                       : 16;
	} bits;
};

union REG_ISP_GMS_MEM_SW_MODE {
	uint32_t raw;
	struct {
		uint32_t GMS_MEM_SW_MODE                 : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t GMS_MEM_SEL                     : 6;
	} bits;
};

union REG_ISP_GMS_MEM_SW_RADDR {
	uint32_t raw;
	struct {
		uint32_t GMS_SW_RADDR                    : 11;
	} bits;
};

union REG_ISP_GMS_MEM_SW_RDATA {
	uint32_t raw;
	struct {
		uint32_t GMS_RDATA_R                     : 31;
		uint32_t GMS_SW_R                        : 1;
	} bits;
};

union REG_ISP_GMS_MONITOR_SELECT {
	uint32_t raw;
	struct {
		uint32_t GMS_MONITOR_SELECT              : 32;
	} bits;
};

union REG_ISP_GMS_DMI_ENABLE {
	uint32_t raw;
	struct {
		uint32_t DMI_ENABLE                      : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t DMI_QOS                         : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t FORCE_DMA_DISABLE               : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_SE_BNR_U1_SE_BNR_ENABLE {
	uint32_t raw;
	struct {
		uint32_t U1_SE_BNR_ENABLE                : 1;
	} bits;
};

union REG_ISP_SE_BNR_U4_SE_BNR_SPATIAL_STR {
	uint32_t raw;
	struct {
		uint32_t U4_SE_BNR_SPATIAL_STR           : 4;
	} bits;
};

union REG_ISP_SE_BNR_U10_SE_BNR_INTENSITY_STR_LUT_0 {
	uint32_t raw;
	struct {
		uint32_t U10_SE_BNR_INTENSITY_STR_LUT_0  : 10;
	} bits;
};

union REG_ISP_SE_BNR_U10_SE_BNR_INTENSITY_STR_LUT_1 {
	uint32_t raw;
	struct {
		uint32_t U10_SE_BNR_INTENSITY_STR_LUT_1  : 10;
	} bits;
};

union REG_ISP_SE_BNR_U10_SE_BNR_INTENSITY_STR_LUT_2 {
	uint32_t raw;
	struct {
		uint32_t U10_SE_BNR_INTENSITY_STR_LUT_2  : 10;
	} bits;
};

union REG_ISP_SE_BNR_U10_SE_BNR_INTENSITY_STR_LUT_3 {
	uint32_t raw;
	struct {
		uint32_t U10_SE_BNR_INTENSITY_STR_LUT_3  : 10;
	} bits;
};

union REG_ISP_SE_BNR_U10_SE_BNR_INTENSITY_STR_LUT_4 {
	uint32_t raw;
	struct {
		uint32_t U10_SE_BNR_INTENSITY_STR_LUT_4  : 10;
	} bits;
};

union REG_ISP_SE_BNR_U10_SE_BNR_INTENSITY_STR_LUT_5 {
	uint32_t raw;
	struct {
		uint32_t U10_SE_BNR_INTENSITY_STR_LUT_5  : 10;
	} bits;
};

union REG_ISP_SE_BNR_SHDW_READ_SEL {
	uint32_t raw;
	struct {
		uint32_t SHDW_READ_SEL                   : 1;
	} bits;
};

union REG_ISP_SE_BNR_HW_AUTO_CG_EN {
	uint32_t raw;
	struct {
		uint32_t HW_AUTO_CG_EN                   : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_BLC_DG_WB_BASE_CONFIG {
	uint32_t raw;
	struct {
		uint32_t SHIFT_MODE                      : 1;
		uint32_t BLC_LE_ENABLE                   : 1;
		uint32_t BLC_SE_ENABLE                   : 1;
		uint32_t WBG_LE_ENABLE                   : 1;
		uint32_t WBG_SE_ENABLE                   : 1;
		uint32_t CG_ENABLE                       : 1;
	} bits;
};

union REG_ISP_BLC_DG_WBG_LE_GAIN0 {
	uint32_t raw;
	struct {
		uint32_t WBG_LE_RGAIN                    : 14;
		uint32_t WBG_LE_GGAIN                    : 14;
	} bits;
};

union REG_ISP_BLC_DG_WBG_LE_GAIN1 {
	uint32_t raw;
	struct {
		uint32_t WBG_LE_BGAIN                    : 14;
	} bits;
};

union REG_ISP_BLC_DG_WB_BLC_LE_OFFSET0 {
	uint32_t raw;
	struct {
		uint32_t BLC_LE_OFFSET_R                 : 12;
		uint32_t BLC_LE_OFFSET_GR                : 12;
	} bits;
};

union REG_ISP_BLC_DG_WB_BLC_LE_OFFSET1 {
	uint32_t raw;
	struct {
		uint32_t BLC_LE_OFFSET_GB                : 12;
		uint32_t BLC_LE_OFFSET_B                 : 12;
	} bits;
};

union REG_ISP_BLC_DG_WB_BLC_LE_OFFSET_2ND0 {
	uint32_t raw;
	struct {
		uint32_t BLC_LE_2NDOFFSET_R              : 12;
		uint32_t BLC_LE_2NDOFFSET_GR             : 12;
	} bits;
};

union REG_ISP_BLC_DG_WB_BLC_LE_OFFSET_2ND1 {
	uint32_t raw;
	struct {
		uint32_t BLC_LE_2NDOFFSET_GB             : 12;
		uint32_t BLC_LE_2NDOFFSET_B              : 12;
	} bits;
};

union REG_ISP_BLC_DG_WB_BLC_LE_GAIN0 {
	uint32_t raw;
	struct {
		uint32_t BLC_LE_GAIN_R                   : 16;
		uint32_t BLC_LE_GAIN_GR                  : 16;
	} bits;
};

union REG_ISP_BLC_DG_WB_BLC_LE_GAIN1 {
	uint32_t raw;
	struct {
		uint32_t BLC_LE_GAIN_GB                  : 16;
		uint32_t BLC_LE_GAIN_B                   : 16;
	} bits;
};

union REG_ISP_BLC_DG_WB_BLC_LE_NORMGAIN0 {
	uint32_t raw;
	struct {
		uint32_t BLC_LE_NORMGAIN_R               : 16;
		uint32_t BLC_LE_NORMGAIN_GR              : 16;
	} bits;
};

union REG_ISP_BLC_DG_WB_BLC_LE_NORMGAIN1 {
	uint32_t raw;
	struct {
		uint32_t BLC_LE_NORMGAIN_GB              : 16;
		uint32_t BLC_LE_NORMGAIN_B               : 16;
	} bits;
};

union REG_ISP_BLC_DG_WBG_SE_GAIN0 {
	uint32_t raw;
	struct {
		uint32_t WBG_SE_RGAIN                    : 14;
		uint32_t WBG_SE_GGAIN                    : 14;
	} bits;
};

union REG_ISP_BLC_DG_WBG_SE_GAIN1 {
	uint32_t raw;
	struct {
		uint32_t WBG_SE_BGAIN                    : 14;
	} bits;
};

union REG_ISP_BLC_DG_WB_BLC_SE_OFFSET0 {
	uint32_t raw;
	struct {
		uint32_t BLC_SE_OFFSET_R                 : 12;
		uint32_t BLC_SE_OFFSET_GR                : 12;
	} bits;
};

union REG_ISP_BLC_DG_WB_BLC_SE_OFFSET1 {
	uint32_t raw;
	struct {
		uint32_t BLC_SE_OFFSET_GB                : 12;
		uint32_t BLC_SE_OFFSET_B                 : 12;
	} bits;
};

union REG_ISP_BLC_DG_WB_BLC_SE_OFFSET_2ND0 {
	uint32_t raw;
	struct {
		uint32_t BLC_SE_2NDOFFSET_R              : 12;
		uint32_t BLC_SE_2NDOFFSET_GR             : 12;
	} bits;
};

union REG_ISP_BLC_DG_WB_BLC_SE_OFFSET_2ND1 {
	uint32_t raw;
	struct {
		uint32_t BLC_SE_2NDOFFSET_GB             : 12;
		uint32_t BLC_SE_2NDOFFSET_B              : 12;
	} bits;
};

union REG_ISP_BLC_DG_WB_BLC_SE_GAIN0 {
	uint32_t raw;
	struct {
		uint32_t BLC_SE_GAIN_R                   : 16;
		uint32_t BLC_SE_GAIN_GR                  : 16;
	} bits;
};

union REG_ISP_BLC_DG_WB_BLC_SE_GAIN1 {
	uint32_t raw;
	struct {
		uint32_t BLC_SE_GAIN_GB                  : 16;
		uint32_t BLC_SE_GAIN_B                   : 16;
	} bits;
};

union REG_ISP_BLC_DG_WB_BLC_SE_NORMGAIN0 {
	uint32_t raw;
	struct {
		uint32_t BLC_SE_NORMGAIN_R               : 16;
		uint32_t BLC_SE_NORMGAIN_GR              : 16;
	} bits;
};

union REG_ISP_BLC_DG_WB_BLC_SE_NORMGAIN1 {
	uint32_t raw;
	struct {
		uint32_t BLC_SE_NORMGAIN_GB              : 16;
		uint32_t BLC_SE_NORMGAIN_B               : 16;
	} bits;
};

union REG_ISP_BLC_DG_WB_SHDW_READ_SEL {
	uint32_t raw;
	struct {
		uint32_t SHDW_READ_SEL                   : 1;
	} bits;
};

union REG_ISP_BLC_DG_WB_AUTO_CG_EN {
	uint32_t raw;
	struct {
		uint32_t HW_AUTO_CG_EN                   : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_FUSION_CTRL_REG {
	uint32_t raw;
	struct {
		uint32_t U1_FUSION_EN                    : 1;
		uint32_t U2_FUSION_FNUM                  : 2;
		uint32_t U2_FUSION_MODE                  : 2;
		uint32_t U4_FUSION_DBG_MODE              : 4;
		uint32_t U2_FUSION_YSEL                  : 2;
		uint32_t U2_FUSION_NSEL                  : 2;
		uint32_t U2_FUSION_DSEL                  : 2;
		uint32_t U13_FUSION_EVRATIO              : 13;
		uint32_t U4_FUSION_EV_FMT                : 4;
	} bits;
};

union REG_FUSION_DS1 {
	uint32_t raw;
	struct {
		uint32_t U12_DS1_TH                      : 12;
		uint32_t U8_DS1_STEP                     : 8;
		uint32_t U8_DS1_LB                       : 8;
	} bits;
};

union REG_FUSION_DS2 {
	uint32_t raw;
	struct {
		uint32_t U12_DS2_TH                      : 12;
		uint32_t U8_DS2_STEP                     : 8;
		uint32_t U8_DS2_LB                       : 8;
	} bits;
};

union REG_FUSION_BCL_RANGE {
	uint32_t raw;
	struct {
		uint32_t U12_BCNL_RANGE                  : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t U12_BCDL_RANGE                  : 12;
	} bits;
};

union REG_FUSION_BCL_P0 {
	uint32_t raw;
	struct {
		uint32_t U12_BCNL_P0                     : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t U12_BCDL_P0                     : 12;
	} bits;
};

union REG_FUSION_BCL_SLOP {
	uint32_t raw;
	struct {
		uint32_t U16_BCNL_SLOPE                  : 16;
		uint32_t U16_BCDL_SLOPE                  : 16;
	} bits;
};

union REG_FUSION_BCS_RANGE {
	uint32_t raw;
	struct {
		uint32_t U12_BCNS_RANGE                  : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t U12_BCDS_RANGE                  : 12;
	} bits;
};

union REG_FUSION_BCS_P0 {
	uint32_t raw;
	struct {
		uint32_t U12_BCNS_P0                     : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t U12_BCDS_P0                     : 12;
	} bits;
};

union REG_FUSION_BCS_SLOP {
	uint32_t raw;
	struct {
		uint32_t U16_BCNS_SLOPE                  : 16;
		uint32_t U16_BCDS_SLOPE                  : 16;
	} bits;
};

union REG_FUSION_DIFF {
	uint32_t raw;
	struct {
		uint32_t U4_DIFF_LUMSHFTR                : 4;
		uint32_t U12_DIFF_LUMTHR                 : 12;
		uint32_t U3_DIFF_RATIO                   : 3;
		uint32_t U5_DIFF_DW_D                    : 5;
	} bits;
};

union REG_FUSION_LUT_0 {
	uint32_t raw;
	struct {
		uint32_t U5_DIFF_DW16_0                  : 5;
		uint32_t U5_DIFF_DW16_1                  : 5;
		uint32_t U5_DIFF_DW16_2                  : 5;
		uint32_t U5_DIFF_DW16_3                  : 5;
		uint32_t U5_DIFF_DW16_4                  : 5;
		uint32_t U5_DIFF_DW16_5                  : 5;
	} bits;
};

union REG_FUSION_LUT_1 {
	uint32_t raw;
	struct {
		uint32_t U5_DIFF_DW16_6                  : 5;
		uint32_t U5_DIFF_DW16_7                  : 5;
		uint32_t U5_DIFF_DW16_8                  : 5;
		uint32_t U5_DIFF_DW16_9                  : 5;
		uint32_t U5_DIFF_DW16_10                 : 5;
		uint32_t U5_DIFF_DW16_11                 : 5;
	} bits;
};

union REG_FUSION_LUT_2 {
	uint32_t raw;
	struct {
		uint32_t U5_DIFF_DW16_12                 : 5;
		uint32_t U5_DIFF_DW16_13                 : 5;
		uint32_t U5_DIFF_DW16_14                 : 5;
		uint32_t U5_DIFF_DW16_15                 : 5;
	} bits;
};

union REG_FUSION_HW_AUTO {
	uint32_t raw;
	struct {
		uint32_t HW_AUTO_CG_EN                   : 1;
	} bits;
};

union REG_FUSION_SHADOW_RD_SEL {
	uint32_t raw;
	struct {
		uint32_t SHDW_READ_SEL                   : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_MAP_CURVE_01 {
	uint32_t raw;
	struct {
		uint32_t U1_FCURVE16_EN                  : 1;
		uint32_t U2_FCURVE16_YSEL                : 2;
		uint32_t _rsv_3                          : 5;
		uint32_t U4_FCURVE16_YVWET               : 4;
		uint32_t _rsv_12                         : 4;
		uint32_t U4_IFE_F_FCURVE_EV_FMT          : 4;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_FLUMW_LUT0 {
	uint32_t raw;
	struct {
		uint32_t U8_FCURVE16_FLUMW_LUT17_0       : 8;
		uint32_t U8_FCURVE16_FLUMW_LUT17_1       : 8;
		uint32_t U8_FCURVE16_FLUMW_LUT17_2       : 8;
		uint32_t U8_FCURVE16_FLUMW_LUT17_3       : 8;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_FLUMW_LUT1 {
	uint32_t raw;
	struct {
		uint32_t U8_FCURVE16_FLUMW_LUT17_4       : 8;
		uint32_t U8_FCURVE16_FLUMW_LUT17_5       : 8;
		uint32_t U8_FCURVE16_FLUMW_LUT17_6       : 8;
		uint32_t U8_FCURVE16_FLUMW_LUT17_7       : 8;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_FLUMW_LUT2 {
	uint32_t raw;
	struct {
		uint32_t U8_FCURVE16_FLUMW_LUT17_8       : 8;
		uint32_t U8_FCURVE16_FLUMW_LUT17_9       : 8;
		uint32_t U8_FCURVE16_FLUMW_LUT17_10      : 8;
		uint32_t U8_FCURVE16_FLUMW_LUT17_11      : 8;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_FLUMW_LUT3 {
	uint32_t raw;
	struct {
		uint32_t U8_FCURVE16_FLUMW_LUT17_12      : 8;
		uint32_t U8_FCURVE16_FLUMW_LUT17_13      : 8;
		uint32_t U8_FCURVE16_FLUMW_LUT17_14      : 8;
		uint32_t U8_FCURVE16_FLUMW_LUT17_15      : 8;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_FLUMW_LUT4 {
	uint32_t raw;
	struct {
		uint32_t U8_FCURVE16_FLUMW_LUT17_16      : 8;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L0 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_0            : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L1 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_1            : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L2 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_2            : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L3 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_3            : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L4 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_4            : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L5 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_5            : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L6 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_6            : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L7 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_7            : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L8 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_8            : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L9 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_9            : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L10 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_10           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L11 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_11           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L12 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_12           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L13 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_13           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L14 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_14           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L15 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_15           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L16 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_16           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L17 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_17           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L18 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_18           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L19 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_19           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L20 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_20           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L21 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_21           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L22 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_22           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L23 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_23           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L24 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_24           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L25 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_25           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L26 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_26           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L27 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_27           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L28 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_28           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L29 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_29           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L30 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_30           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L31 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_31           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L32 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_32           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L33 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_33           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L34 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_34           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L35 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_35           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L36 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_36           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L37 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_37           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L38 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_38           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L39 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_39           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L40 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_40           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L41 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_41           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L42 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_42           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L43 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_43           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L44 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_44           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L45 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_45           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L46 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_46           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L47 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_47           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L48 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_48           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L49 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_49           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L50 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_50           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L51 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_51           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L52 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_52           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L53 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_53           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L54 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_54           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L55 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_55           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L56 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_56           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L57 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_57           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L58 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_58           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L59 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_59           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L60 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_60           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L61 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_61           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L62 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_62           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L63 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_63           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_L64 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_L_64           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_R0 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_R_0            : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_R1 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_R_1            : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_R2 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_R_2            : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_R3 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_R_3            : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_R4 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_R_4            : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_R5 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_R_5            : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_R6 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_R_6            : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_R7 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_R_7            : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_R8 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_R_8            : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_R9 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_R_9            : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_R10 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_R_10           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_R11 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_R_11           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_R12 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_R_12           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_R13 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_R_13           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_R14 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_R_14           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_R15 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_R_15           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_R16 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_R_16           : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_END0 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_END_0          : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_END1 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_END_1          : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_END2 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_END_2          : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_END3 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_END_3          : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_END4 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_END_4          : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_END5 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_END_5          : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_END6 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_END_6          : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_END7 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_END_7          : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_END8 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_END_8          : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_END9 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_END_9          : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_END10 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_END_10         : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_END11 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_END_11         : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_END12 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_END_12         : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_END13 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_END_13         : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_END14 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_END_14         : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_END15 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_END_15         : 24;
	} bits;
};

union REG_ISP_MAP_CURVE_FCURVE_END16 {
	uint32_t raw;
	struct {
		uint32_t U24_IFE_F_FCURVE_END_16         : 24;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_DPC_BASE_CONFIG {
	uint32_t raw;
	struct {
		uint32_t DPC_ENABLE                      : 1;
		uint32_t SPC_ENABLE                      : 1;
		uint32_t GE_ENABLE                       : 1;
		uint32_t CG_ENABLE                       : 1;
	} bits;
};

union REG_ISP_DPC_DARK_THRESHOLD0 {
	uint32_t raw;
	struct {
		uint32_t DPC_OUTLIER_DARK_TH0            : 12;
		uint32_t DPC_OUTLIER_DARK_TH1            : 12;
	} bits;
};

union REG_ISP_DPC_DARK_THRESHOLD1 {
	uint32_t raw;
	struct {
		uint32_t DPC_OUTLIER_DARK_TH2            : 12;
		uint32_t DPC_OUTLIER_DARK_TH3            : 12;
	} bits;
};

union REG_ISP_DPC_DARK_THRESHOLD2 {
	uint32_t raw;
	struct {
		uint32_t DPC_OUTLIER_DARK_TH4            : 12;
	} bits;
};

union REG_ISP_DPC_BRIGHT_THRESHOLD0 {
	uint32_t raw;
	struct {
		uint32_t DPC_OUTLIER_BRIT_TH0            : 12;
		uint32_t DPC_OUTLIER_BRIT_TH1            : 12;
	} bits;
};

union REG_ISP_DPC_BRIGHT_THRESHOLD1 {
	uint32_t raw;
	struct {
		uint32_t DPC_OUTLIER_BRIT_TH2            : 12;
		uint32_t DPC_OUTLIER_BRIT_TH3            : 12;
	} bits;
};

union REG_ISP_DPC_BRIGHT_THRESHOLD2 {
	uint32_t raw;
	struct {
		uint32_t DPC_OUTLIER_BRIT_TH4            : 12;
	} bits;
};

union REG_ISP_DPC_THRESHOLD_OFFSET {
	uint32_t raw;
	struct {
		uint32_t DPC_OUTLIER_DARK_TH_OFS         : 12;
		uint32_t DPC_OUTLIER_BRIT_TH_OFS         : 12;
	} bits;
};

union REG_ISP_DPC_OUTLIER_CNT {
	uint32_t raw;
	struct {
		uint32_t DPC_OUTLIER_CNT1                : 5;
		uint32_t DPC_OUTLIER_CNT2                : 5;
	} bits;
};

union REG_ISP_DPC_OUTLIER_CONFIG {
	uint32_t raw;
	struct {
		uint32_t DPC_OUTLIER_ADV_MODE            : 1;
		uint32_t DPC_OUTLIER_AVG_MODE            : 1;
		uint32_t DPC_OUTLIER_WEIGHT              : 8;
	} bits;
};

union REG_ISP_DPC_GE_CONFIG {
	uint32_t raw;
	struct {
		uint32_t GE_STRENGTH                     : 9;
		uint32_t GE_COMBINEWEIGHT                : 4;
	} bits;
};

union REG_ISP_DPC_GE_THRESHOLD0 {
	uint32_t raw;
	struct {
		uint32_t GE_THRE1                        : 12;
		uint32_t GE_THRE2                        : 12;
	} bits;
};

union REG_ISP_DPC_GE_THRESHOLD1 {
	uint32_t raw;
	struct {
		uint32_t GE_THRE3                        : 12;
		uint32_t GE_THRE4                        : 12;
	} bits;
};

union REG_ISP_DPC_GE_THRESHOLD1_0 {
	uint32_t raw;
	struct {
		uint32_t GE_THRE11                       : 12;
		uint32_t GE_THRE21                       : 12;
	} bits;
};

union REG_ISP_DPC_GE_THRESHOLD1_1 {
	uint32_t raw;
	struct {
		uint32_t GE_THRE31                       : 12;
		uint32_t GE_THRE41                       : 12;
	} bits;
};

union REG_ISP_DPC_GE_THRESHOLD2_0 {
	uint32_t raw;
	struct {
		uint32_t GE_THRE12                       : 12;
		uint32_t GE_THRE22                       : 12;
	} bits;
};

union REG_ISP_DPC_GE_THRESHOLD2_1 {
	uint32_t raw;
	struct {
		uint32_t GE_THRE32                       : 12;
		uint32_t GE_THRE42                       : 12;
	} bits;
};

union REG_ISP_DPC_SHDW_READ_SEL {
	uint32_t raw;
	struct {
		uint32_t SHDW_READ_SEL                   : 1;
	} bits;
};

union REG_ISP_DPC_SPC_CONFIG {
	uint32_t raw;
	struct {
		uint32_t DPC_MEM0_ADDR                   : 10;
		uint32_t _rsv_10                         : 21;
		uint32_t DPC_MEM_SW_MODE                 : 1;
	} bits;
};

union REG_ISP_DPC_SW_READ_0 {
	uint32_t raw;
	struct {
		uint32_t DPC_RDATA_R                     : 26;
		uint32_t _rsv_26                         : 5;
		uint32_t DPC_SW_R                        : 1;
	} bits;
};

union REG_ISP_DPC_WRITE_MEM {
	uint32_t raw;
	struct {
		uint32_t DPC_BP_MEM_D                    : 26;
		uint32_t _rsv_26                         : 5;
		uint32_t DPC_BP_MEM_W                    : 1;
	} bits;
};

union REG_ISP_DPC_WRITE_MEM_ST_ADDR {
	uint32_t raw;
	struct {
		uint32_t DPC_BP_MEM_ST_ADDR              : 10;
		uint32_t _rsv_10                         : 2;
		uint32_t DPC_BP_MEM_ST_ADDR_W            : 1;
		uint32_t BP_NUM                          : 13;
	} bits;
};

union REG_ISP_DPC_AUTO_CG_EN {
	uint32_t raw;
	struct {
		uint32_t HW_AUTO_CG_EN                   : 1;
	} bits;
};

union REG_ISP_DPC_SW_READ_1 {
	uint32_t raw;
	struct {
		uint32_t DPC_SW_RADDR                    : 10;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_BNR_00 {
	uint32_t raw;
	struct {
		uint32_t U1_BNR_ENABLE                   : 1;
		uint32_t SHD_RD                          : 1;
		uint32_t BYPASS                          : 1;
		uint32_t SOFT_CLR                        : 1;
		uint32_t HW_AUTO_CG_EN                   : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t U2_BNR_DEBUGMODE                : 2;
		uint32_t _rsv_10                         : 14;
		uint32_t DUMMY                           : 8;
	} bits;
};

union REG_ISP_BNR_04 {
	uint32_t raw;
	struct {
		uint32_t OUT_ENABLE                      : 2;
	} bits;
};

union REG_ISP_BNR_10 {
	uint32_t raw;
	struct {
		uint32_t U1_BNR_FILTMODE                 : 1;
		uint32_t U1_BNR_BILAT_TH_EN              : 1;
		uint32_t U1_BNR_BILAT_CENTER_SEL         : 1;
		uint32_t _rsv_3                          : 5;
		uint32_t U5_BNR_BILAT_BLEND_W            : 5;
	} bits;
};

union REG_ISP_BNR_14 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_BILAT_TH1               : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_BILAT_TH2               : 10;
	} bits;
};

union REG_ISP_BNR_18 {
	uint32_t raw;
	struct {
		uint32_t U5_BNR_SWEIGHT_0                : 5;
		uint32_t _rsv_5                          : 1;
		uint32_t U5_BNR_SWEIGHT_1                : 5;
		uint32_t _rsv_11                         : 1;
		uint32_t U5_BNR_SWEIGHT_2                : 5;
		uint32_t _rsv_17                         : 1;
		uint32_t U5_BNR_SWEIGHT_3                : 5;
		uint32_t _rsv_23                         : 1;
		uint32_t U5_BNR_SWEIGHT_4                : 5;
	} bits;
};

union REG_ISP_BNR_1C {
	uint32_t raw;
	struct {
		uint32_t U5_BNR_SWEIGHT_5                : 5;
		uint32_t _rsv_5                          : 1;
		uint32_t U5_BNR_SWEIGHT_6                : 5;
		uint32_t _rsv_11                         : 1;
		uint32_t U5_BNR_SWEIGHT_7                : 5;
		uint32_t _rsv_17                         : 1;
		uint32_t U5_BNR_SWEIGHT_8                : 5;
		uint32_t _rsv_23                         : 1;
		uint32_t U5_BNR_SWEIGHT_9                : 5;
	} bits;
};

union REG_ISP_BNR_20 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_R_LUT_BASE_0     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_R_LUT_BASE_1     : 10;
	} bits;
};

union REG_ISP_BNR_24 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_R_LUT_BASE_2     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_R_LUT_BASE_3     : 10;
	} bits;
};

union REG_ISP_BNR_28 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_R_LUT_BASE_4     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_R_LUT_BASE_5     : 10;
	} bits;
};

union REG_ISP_BNR_2C {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_GR_LUT_BASE_0    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_GR_LUT_BASE_1    : 10;
	} bits;
};

union REG_ISP_BNR_30 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_GR_LUT_BASE_2    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_GR_LUT_BASE_3    : 10;
	} bits;
};

union REG_ISP_BNR_34 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_GR_LUT_BASE_4    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_GR_LUT_BASE_5    : 10;
	} bits;
};

union REG_ISP_BNR_38 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_GB_LUT_BASE_0    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_GB_LUT_BASE_1    : 10;
	} bits;
};

union REG_ISP_BNR_3C {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_GB_LUT_BASE_2    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_GB_LUT_BASE_3    : 10;
	} bits;
};

union REG_ISP_BNR_40 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_GB_LUT_BASE_4    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_GB_LUT_BASE_5    : 10;
	} bits;
};

union REG_ISP_BNR_44 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_B_LUT_BASE_0     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_B_LUT_BASE_1     : 10;
	} bits;
};

union REG_ISP_BNR_48 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_B_LUT_BASE_2     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_B_LUT_BASE_3     : 10;
	} bits;
};

union REG_ISP_BNR_4C {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_B_LUT_BASE_4     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_B_LUT_BASE_5     : 10;
	} bits;
};

union REG_ISP_BNR_50 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_R_LUT_LUMA_0     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_R_LUT_LUMA_1     : 10;
	} bits;
};

union REG_ISP_BNR_54 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_R_LUT_LUMA_2     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_R_LUT_LUMA_3     : 10;
	} bits;
};

union REG_ISP_BNR_58 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_R_LUT_LUMA_4     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_R_LUT_LUMA_5     : 10;
	} bits;
};

union REG_ISP_BNR_5C {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_R_LUT_LUMA_6     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_R_LUT_LUMA_7     : 10;
	} bits;
};

union REG_ISP_BNR_60 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_R_LUT_LUMA_8     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_R_LUT_LUMA_9     : 10;
	} bits;
};

union REG_ISP_BNR_64 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_R_LUT_LUMA_10    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_R_LUT_LUMA_11    : 10;
	} bits;
};

union REG_ISP_BNR_68 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_R_LUT_LUMA_12    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_R_LUT_LUMA_13    : 10;
	} bits;
};

union REG_ISP_BNR_6C {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_R_LUT_LUMA_14    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_R_LUT_LUMA_15    : 10;
	} bits;
};

union REG_ISP_BNR_70 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_R_LUT_LUMA_16    : 10;
	} bits;
};

union REG_ISP_BNR_74 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_GR_LUT_LUMA_0    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_GR_LUT_LUMA_1    : 10;
	} bits;
};

union REG_ISP_BNR_78 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_GR_LUT_LUMA_2    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_GR_LUT_LUMA_3    : 10;
	} bits;
};

union REG_ISP_BNR_7C {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_GR_LUT_LUMA_4    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_GR_LUT_LUMA_5    : 10;
	} bits;
};

union REG_ISP_BNR_80 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_GR_LUT_LUMA_6    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_GR_LUT_LUMA_7    : 10;
	} bits;
};

union REG_ISP_BNR_84 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_GR_LUT_LUMA_8    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_GR_LUT_LUMA_9    : 10;
	} bits;
};

union REG_ISP_BNR_88 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_GR_LUT_LUMA_10   : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_GR_LUT_LUMA_11   : 10;
	} bits;
};

union REG_ISP_BNR_8C {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_GR_LUT_LUMA_12   : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_GR_LUT_LUMA_13   : 10;
	} bits;
};

union REG_ISP_BNR_90 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_GR_LUT_LUMA_14   : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_GR_LUT_LUMA_15   : 10;
	} bits;
};

union REG_ISP_BNR_94 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_GR_LUT_LUMA_16   : 10;
	} bits;
};

union REG_ISP_BNR_98 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_GB_LUT_LUMA_0    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_GB_LUT_LUMA_1    : 10;
	} bits;
};

union REG_ISP_BNR_9C {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_GB_LUT_LUMA_2    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_GB_LUT_LUMA_3    : 10;
	} bits;
};

union REG_ISP_BNR_A0 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_GB_LUT_LUMA_4    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_GB_LUT_LUMA_5    : 10;
	} bits;
};

union REG_ISP_BNR_A4 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_GB_LUT_LUMA_6    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_GB_LUT_LUMA_7    : 10;
	} bits;
};

union REG_ISP_BNR_A8 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_GB_LUT_LUMA_8    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_GB_LUT_LUMA_9    : 10;
	} bits;
};

union REG_ISP_BNR_AC {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_GB_LUT_LUMA_10   : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_GB_LUT_LUMA_11   : 10;
	} bits;
};

union REG_ISP_BNR_B0 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_GB_LUT_LUMA_12   : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_GB_LUT_LUMA_13   : 10;
	} bits;
};

union REG_ISP_BNR_B4 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_GB_LUT_LUMA_14   : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_GB_LUT_LUMA_15   : 10;
	} bits;
};

union REG_ISP_BNR_B8 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_GB_LUT_LUMA_16   : 10;
	} bits;
};

union REG_ISP_BNR_BC {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_B_LUT_LUMA_0     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_B_LUT_LUMA_1     : 10;
	} bits;
};

union REG_ISP_BNR_C0 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_B_LUT_LUMA_2     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_B_LUT_LUMA_3     : 10;
	} bits;
};

union REG_ISP_BNR_C4 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_B_LUT_LUMA_4     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_B_LUT_LUMA_5     : 10;
	} bits;
};

union REG_ISP_BNR_C8 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_B_LUT_LUMA_6     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_B_LUT_LUMA_7     : 10;
	} bits;
};

union REG_ISP_BNR_CC {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_B_LUT_LUMA_8     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_B_LUT_LUMA_9     : 10;
	} bits;
};

union REG_ISP_BNR_D0 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_B_LUT_LUMA_10    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_B_LUT_LUMA_11    : 10;
	} bits;
};

union REG_ISP_BNR_D4 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_B_LUT_LUMA_12    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_B_LUT_LUMA_13    : 10;
	} bits;
};

union REG_ISP_BNR_D8 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_B_LUT_LUMA_14    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHNLM_B_LUT_LUMA_15    : 10;
	} bits;
};

union REG_ISP_BNR_DC {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHNLM_B_LUT_LUMA_16    : 10;
	} bits;
};

union REG_ISP_BNR_E0 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_R_LUT_BASE_0      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_R_LUT_BASE_1      : 10;
	} bits;
};

union REG_ISP_BNR_E4 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_R_LUT_BASE_2      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_R_LUT_BASE_3      : 10;
	} bits;
};

union REG_ISP_BNR_E8 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_R_LUT_BASE_4      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_R_LUT_BASE_5      : 10;
	} bits;
};

union REG_ISP_BNR_EC {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_GR_LUT_BASE_0     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_GR_LUT_BASE_1     : 10;
	} bits;
};

union REG_ISP_BNR_F0 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_GR_LUT_BASE_2     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_GR_LUT_BASE_3     : 10;
	} bits;
};

union REG_ISP_BNR_F4 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_GR_LUT_BASE_4     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_GR_LUT_BASE_5     : 10;
	} bits;
};

union REG_ISP_BNR_F8 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_GB_LUT_BASE_0     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_GB_LUT_BASE_1     : 10;
	} bits;
};

union REG_ISP_BNR_FC {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_GB_LUT_BASE_2     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_GB_LUT_BASE_3     : 10;
	} bits;
};

union REG_ISP_BNR_100 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_GB_LUT_BASE_4     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_GB_LUT_BASE_5     : 10;
	} bits;
};

union REG_ISP_BNR_104 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_B_LUT_BASE_0      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_B_LUT_BASE_1      : 10;
	} bits;
};

union REG_ISP_BNR_108 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_B_LUT_BASE_2      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_B_LUT_BASE_3      : 10;
	} bits;
};

union REG_ISP_BNR_10C {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_B_LUT_BASE_4      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_B_LUT_BASE_5      : 10;
	} bits;
};

union REG_ISP_BNR_110 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_R_LUT_LUMA_0      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_R_LUT_LUMA_1      : 10;
	} bits;
};

union REG_ISP_BNR_114 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_R_LUT_LUMA_2      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_R_LUT_LUMA_3      : 10;
	} bits;
};

union REG_ISP_BNR_118 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_R_LUT_LUMA_4      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_R_LUT_LUMA_5      : 10;
	} bits;
};

union REG_ISP_BNR_11C {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_R_LUT_LUMA_6      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_R_LUT_LUMA_7      : 10;
	} bits;
};

union REG_ISP_BNR_120 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_R_LUT_LUMA_8      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_R_LUT_LUMA_9      : 10;
	} bits;
};

union REG_ISP_BNR_124 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_R_LUT_LUMA_10     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_R_LUT_LUMA_11     : 10;
	} bits;
};

union REG_ISP_BNR_128 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_R_LUT_LUMA_12     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_R_LUT_LUMA_13     : 10;
	} bits;
};

union REG_ISP_BNR_12C {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_R_LUT_LUMA_14     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_R_LUT_LUMA_15     : 10;
	} bits;
};

union REG_ISP_BNR_130 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_R_LUT_LUMA_16     : 10;
	} bits;
};

union REG_ISP_BNR_134 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_GR_LUT_LUMA_0     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_GR_LUT_LUMA_1     : 10;
	} bits;
};

union REG_ISP_BNR_138 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_GR_LUT_LUMA_2     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_GR_LUT_LUMA_3     : 10;
	} bits;
};

union REG_ISP_BNR_13C {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_GR_LUT_LUMA_4     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_GR_LUT_LUMA_5     : 10;
	} bits;
};

union REG_ISP_BNR_140 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_GR_LUT_LUMA_6     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_GR_LUT_LUMA_7     : 10;
	} bits;
};

union REG_ISP_BNR_144 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_GR_LUT_LUMA_8     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_GR_LUT_LUMA_9     : 10;
	} bits;
};

union REG_ISP_BNR_148 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_GR_LUT_LUMA_10    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_GR_LUT_LUMA_11    : 10;
	} bits;
};

union REG_ISP_BNR_14C {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_GR_LUT_LUMA_12    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_GR_LUT_LUMA_13    : 10;
	} bits;
};

union REG_ISP_BNR_150 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_GR_LUT_LUMA_14    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_GR_LUT_LUMA_15    : 10;
	} bits;
};

union REG_ISP_BNR_154 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_GR_LUT_LUMA_16    : 10;
	} bits;
};

union REG_ISP_BNR_158 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_GB_LUT_LUMA_0     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_GB_LUT_LUMA_1     : 10;
	} bits;
};

union REG_ISP_BNR_15C {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_GB_LUT_LUMA_2     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_GB_LUT_LUMA_3     : 10;
	} bits;
};

union REG_ISP_BNR_160 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_GB_LUT_LUMA_4     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_GB_LUT_LUMA_5     : 10;
	} bits;
};

union REG_ISP_BNR_164 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_GB_LUT_LUMA_6     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_GB_LUT_LUMA_7     : 10;
	} bits;
};

union REG_ISP_BNR_168 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_GB_LUT_LUMA_8     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_GB_LUT_LUMA_9     : 10;
	} bits;
};

union REG_ISP_BNR_16C {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_GB_LUT_LUMA_10    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_GB_LUT_LUMA_11    : 10;
	} bits;
};

union REG_ISP_BNR_170 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_GB_LUT_LUMA_12    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_GB_LUT_LUMA_13    : 10;
	} bits;
};

union REG_ISP_BNR_174 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_GB_LUT_LUMA_14    : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_GB_LUT_LUMA_15    : 10;
	} bits;
};

union REG_ISP_BNR_178 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_GB_LUT_LUMA_16    : 10;
	} bits;
};

union REG_ISP_BNR_17C {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_B_LUT_LUMA_0      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_B_LUT_LUMA_1      : 10;
	} bits;
};

union REG_ISP_BNR_180 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_B_LUT_LUMA_2      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_B_LUT_LUMA_3      : 10;
	} bits;
};

union REG_ISP_BNR_184 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_B_LUT_LUMA_4      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_B_LUT_LUMA_5      : 10;
	} bits;
};

union REG_ISP_BNR_188 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_B_LUT_LUMA_6      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_B_LUT_LUMA_7      : 10;
	} bits;
};

union REG_ISP_BNR_18C {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_B_LUT_LUMA_8      : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_B_LUT_LUMA_9      : 10;
	} bits;
};

union REG_ISP_BNR_190 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_B_LUT_LUMA_10     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_B_LUT_LUMA_11     : 10;
	} bits;
};

union REG_ISP_BNR_194 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_B_LUT_LUMA_12     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_B_LUT_LUMA_13     : 10;
	} bits;
};

union REG_ISP_BNR_198 {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_B_LUT_LUMA_14     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t U10_BNR_RTHBF_B_LUT_LUMA_15     : 10;
	} bits;
};

union REG_ISP_BNR_19C {
	uint32_t raw;
	struct {
		uint32_t U10_BNR_RTHBF_B_LUT_LUMA_16     : 10;
	} bits;
};

union REG_ISP_BNR_1A0 {
	uint32_t raw;
	struct {
		uint32_t DUMMY_SHD                       : 32;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_LSCR_SC_WRAP_0 {
	uint32_t raw;
	struct {
		uint32_t BYPASS                          : 1;
		uint32_t HW_AUTO_CG_EN                   : 1;
		uint32_t LSC_BAYER_STARTING              : 2;
		uint32_t TRIG_STR_1T                     : 1;
	} bits;
};

union REG_ISP_LSCR_SC_WRAP_1 {
	uint32_t raw;
	struct {
		uint32_t SW_RST                          : 1;
		uint32_t SHDW_UPDATE_1T                  : 1;
		uint32_t LSCR_ENABLE                     : 1;
		uint32_t SHDW_READ_SEL                   : 1;
		uint32_t _rsv_4                          : 4;
		uint32_t LSCR_DY_GAIN                    : 8;
		uint32_t BLC_ENABLE                      : 1;
	} bits;
};

union REG_ISP_LSCR_SC_WRAP_2 {
	uint32_t raw;
	struct {
		uint32_t IMG_WIDTH                       : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t IMG_HEIGHT                      : 14;
	} bits;
};

union REG_ISP_LSCR_SC_WRAP_3 {
	uint32_t raw;
	struct {
		uint32_t LSCR_CENTERX                    : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t LSCR_CENTERY                    : 13;
	} bits;
};

union REG_ISP_LSCR_SC_WRAP_4 {
	uint32_t raw;
	struct {
		uint32_t LSCR_NORM                       : 15;
		uint32_t _rsv_15                         : 1;
		uint32_t LSCR_STRNTH                     : 12;
	} bits;
};

union REG_ISP_LSCR_SC_WRAP_5 {
	uint32_t raw;
	struct {
		uint32_t LSCR_ND_THR                     : 8;
		uint32_t LSCR_ND_STR                     : 8;
	} bits;
};

union REG_ISP_LSCR_SC_WRAP_6 {
	uint32_t raw;
	struct {
		uint32_t LSC_RADIUS_GGAIN0               : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t LSC_RADIUS_GGAIN1               : 12;
	} bits;
};

union REG_ISP_LSCR_SC_WRAP_7 {
	uint32_t raw;
	struct {
		uint32_t LSC_RADIUS_GGAIN2               : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t LSC_RADIUS_GGAIN3               : 12;
	} bits;
};

union REG_ISP_LSCR_SC_WRAP_8 {
	uint32_t raw;
	struct {
		uint32_t LSC_RADIUS_GGAIN4               : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t LSC_RADIUS_GGAIN5               : 12;
	} bits;
};

union REG_ISP_LSCR_SC_WRAP_9 {
	uint32_t raw;
	struct {
		uint32_t LSC_RADIUS_GGAIN6               : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t LSC_RADIUS_GGAIN7               : 12;
	} bits;
};

union REG_ISP_LSCR_SC_WRAP_10 {
	uint32_t raw;
	struct {
		uint32_t LSC_RADIUS_GGAIN8               : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t LSC_RADIUS_GGAIN9               : 12;
	} bits;
};

union REG_ISP_LSCR_SC_WRAP_11 {
	uint32_t raw;
	struct {
		uint32_t LSC_RADIUS_GGAIN10              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t LSC_RADIUS_GGAIN11              : 12;
	} bits;
};

union REG_ISP_LSCR_SC_WRAP_12 {
	uint32_t raw;
	struct {
		uint32_t LSC_RADIUS_GGAIN12              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t LSC_RADIUS_GGAIN13              : 12;
	} bits;
};

union REG_ISP_LSCR_SC_WRAP_13 {
	uint32_t raw;
	struct {
		uint32_t LSC_RADIUS_GGAIN14              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t LSC_RADIUS_GGAIN15              : 12;
	} bits;
};

union REG_ISP_LSCR_SC_WRAP_14 {
	uint32_t raw;
	struct {
		uint32_t LSC_RADIUS_GGAIN16              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t LSC_RADIUS_GGAIN17              : 12;
	} bits;
};

union REG_ISP_LSCR_SC_WRAP_15 {
	uint32_t raw;
	struct {
		uint32_t LSC_RADIUS_GGAIN18              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t LSC_RADIUS_GGAIN19              : 12;
	} bits;
};

union REG_ISP_LSCR_SC_WRAP_16 {
	uint32_t raw;
	struct {
		uint32_t LSC_RADIUS_GGAIN20              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t LSC_RADIUS_GGAIN21              : 12;
	} bits;
};

union REG_ISP_LSCR_SC_WRAP_17 {
	uint32_t raw;
	struct {
		uint32_t LSC_RADIUS_GGAIN22              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t LSC_RADIUS_GGAIN23              : 12;
	} bits;
};

union REG_ISP_LSCR_SC_WRAP_18 {
	uint32_t raw;
	struct {
		uint32_t LSC_RADIUS_GGAIN24              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t LSC_RADIUS_GGAIN25              : 12;
	} bits;
};

union REG_ISP_LSCR_SC_WRAP_19 {
	uint32_t raw;
	struct {
		uint32_t LSC_RADIUS_GGAIN26              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t LSC_RADIUS_GGAIN27              : 12;
	} bits;
};

union REG_ISP_LSCR_SC_WRAP_20 {
	uint32_t raw;
	struct {
		uint32_t LSC_RADIUS_GGAIN28              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t LSC_RADIUS_GGAIN29              : 12;
	} bits;
};

union REG_ISP_LSCR_SC_WRAP_21 {
	uint32_t raw;
	struct {
		uint32_t LSC_RADIUS_GGAIN30              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t LSC_RADIUS_GGAIN31              : 12;
	} bits;
};

union REG_ISP_LSCR_SC_WRAP_22 {
	uint32_t raw;
	struct {
		uint32_t BLC_OFFSET_R                    : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t BLC_OFFSET_GR                   : 12;
	} bits;
};

union REG_ISP_LSCR_SC_WRAP_23 {
	uint32_t raw;
	struct {
		uint32_t BLC_OFFSET_GB                   : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t BLC_OFFSET_B                    : 12;
	} bits;
};

union REG_ISP_LSCR_SC_WRAP_24 {
	uint32_t raw;
	struct {
		uint32_t LSCR_DEBUG                      : 32;
	} bits;
};

union REG_ISP_LSCR_SC_WRAP_25 {
	uint32_t raw;
	struct {
		uint32_t LSCR_DMA_ENABLE                 : 1;
		uint32_t _rsv_1                          : 7;
		uint32_t LSCR_DMA_IDLE                   : 1;
		uint32_t _rsv_9                          : 7;
		uint32_t LSCR_DMA_ADDR_HIGH8             : 8;
	} bits;
};

union REG_ISP_LSCR_SC_WRAP_26 {
	uint32_t raw;
	struct {
		uint32_t LSCR_DMA_ADDR_LOW32             : 32;
	} bits;
};
/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_DRC_FRAME_DONE {
	uint32_t raw;
	struct {
		uint32_t FRAME_START_W1P                 : 1;
		uint32_t _rsv_1                          : 1;
		uint32_t FRAME_DONE                      : 1;
	} bits;
};

union REG_ISP_DRC_ENABLE {
	uint32_t raw;
	struct {
		uint32_t _rsv_0                          : 1;
		uint32_t DRC_ENABLE                      : 1;
	} bits;
};

union REG_ISP_DRC_BLK_NUM {
	uint32_t raw;
	struct {
		uint32_t DRC_SUBIMG_WIDTH                : 6;
		uint32_t _rsv_6                          : 10;
		uint32_t DRC_SUBIMG_HEIGHT               : 6;
	} bits;
};

union REG_ISP_DRC_BLK_SIZE {
	uint32_t raw;
	struct {
		uint32_t DRC_SUBIMG_RATIO_HORI           : 16;
		uint32_t DRC_SUBIMG_RATIO_VERT           : 16;
	} bits;
};

union REG_ISP_DRC_BLK_DIV {
	uint32_t raw;
	struct {
		uint32_t DRC_SUBIMG_RATIO_HORI_DIV       : 16;
		uint32_t DRC_SUBIMG_RATIO_VERT_DIV       : 16;
	} bits;
};

union REG_ISP_DRC_LPF_MODE {
	uint32_t raw;
	struct {
		uint32_t DRC_SUBIMG_LPF_MODE             : 2;
	} bits;
};

union REG_ISP_DRC_HIST {
	uint32_t raw;
	struct {
		uint32_t DRC_HIST_ENABLE                 : 1;
		uint32_t _rsv_1                          : 15;
		uint32_t DRC_HIST_MODE                   : 1;
	} bits;
};

union REG_ISP_DRC_HIST_STEP {
	uint32_t raw;
	struct {
		uint32_t DRC_HIST_STEP_X                 : 5;
		uint32_t _rsv_5                          : 11;
		uint32_t DRC_HIST_STEP_Y                 : 5;
	} bits;
};

union REG_ISP_DRC_INTENSIT_MODE {
	uint32_t raw;
	struct {
		uint32_t DRC_INTENSITY_MODE              : 2;
		uint32_t _rsv_2                          : 14;
		uint32_t DRC_TONE_CURVE_ENABLE           : 1;
	} bits;
};

union REG_ISP_DRC_HORI_LPF_TH12 {
	uint32_t raw;
	struct {
		uint32_t DRC_HORI_LPF_TH1                : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_HORI_LPF_TH2                : 12;
	} bits;
};

union REG_ISP_DRC_HORI_LPF_TH34 {
	uint32_t raw;
	struct {
		uint32_t DRC_HORI_LPF_TH3                : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_HORI_LPF_TH4                : 12;
	} bits;
};

union REG_ISP_DRC_HORI_LPF_TH56 {
	uint32_t raw;
	struct {
		uint32_t DRC_HORI_LPF_TH5                : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_HORI_LPF_TH6                : 12;
	} bits;
};

union REG_ISP_DRC_Q_COEFF12 {
	uint32_t raw;
	struct {
		uint32_t DRC_Q_COEFF1                    : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t DRC_Q_COEFF2                    : 13;
	} bits;
};

union REG_ISP_DRC_Q_COEFF34 {
	uint32_t raw;
	struct {
		uint32_t DRC_Q_COEFF3                    : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t DRC_Q_COEFF4                    : 13;
	} bits;
};

union REG_ISP_DRC_B2Q_TH {
	uint32_t raw;
	struct {
		uint32_t DRC_B2Q_TH                      : 12;
	} bits;
};

union REG_ISP_DRC_FBC {
	uint32_t raw;
	struct {
		uint32_t DRC_FBC_ENABLE                  : 1;
		uint32_t _rsv_1                          : 15;
		uint32_t DRC_FBC_RAITO                   : 8;
	} bits;
};

union REG_ISP_DRC_ANTI_HALO {
	uint32_t raw;
	struct {
		uint32_t DRC_ANTI_HALO_MODE              : 2;
		uint32_t _rsv_2                          : 6;
		uint32_t DRC_ANTI_HALO_RATIO             : 8;
		uint32_t DRC_ANTI_HALO_SLOPE             : 8;
	} bits;
};

union REG_ISP_DRC_STRENGTH {
	uint32_t raw;
	struct {
		uint32_t DRC_STRENGTH                    : 8;
	} bits;
};

union REG_ISP_DRC_LTM_EN {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_ENABLE         : 1;
		uint32_t _rsv_1                          : 15;
		uint32_t DRC_GAIN_CTRL_ENABLE            : 1;
	} bits;
};

union REG_ISP_DRC_GAIN {
	uint32_t raw;
	struct {
		uint32_t DRC_MAX_GAIN                    : 8;
		uint32_t _rsv_8                          : 8;
		uint32_t DRC_MIN_GAIN                    : 8;
	} bits;
};

union REG_ISP_DRC_SAT_TH {
	uint32_t raw;
	struct {
		uint32_t DRC_GAIN_PROT_STR               : 8;
		uint32_t _rsv_8                          : 8;
		uint32_t DRC_SAT_TH                      : 12;
	} bits;
};

union REG_ISP_DRC_SAT_DELTA {
	uint32_t raw;
	struct {
		uint32_t DRC_SAT_DELTA                   : 8;
		uint32_t _rsv_8                          : 8;
		uint32_t DRC_SAT_WT_LOW                  : 8;
	} bits;
};

union REG_ISP_DRC_YV_BLD_LUT0_4 {
	uint32_t raw;
	struct {
		uint32_t DRC_YV_BLD_LUT_0                : 6;
		uint32_t DRC_YV_BLD_LUT_1                : 6;
		uint32_t DRC_YV_BLD_LUT_2                : 6;
		uint32_t DRC_YV_BLD_LUT_3                : 6;
		uint32_t DRC_YV_BLD_LUT_4                : 6;
	} bits;
};

union REG_ISP_DRC_YV_BLD_LUT5_8 {
	uint32_t raw;
	struct {
		uint32_t DRC_YV_BLD_LUT_5                : 6;
		uint32_t DRC_YV_BLD_LUT_6                : 6;
		uint32_t DRC_YV_BLD_LUT_7                : 6;
		uint32_t DRC_YV_BLD_LUT_8                : 6;
	} bits;
};

union REG_ISP_DRC_INPUT_BLD_LUT_0_3 {
	uint32_t raw;
	struct {
		uint32_t DRC_INPUT_BLD_LUT_0             : 8;
		uint32_t DRC_INPUT_BLD_LUT_1             : 8;
		uint32_t DRC_INPUT_BLD_LUT_2             : 8;
		uint32_t DRC_INPUT_BLD_LUT_3             : 8;
	} bits;
};

union REG_ISP_DRC_INPUT_BLD_LUT_4_7 {
	uint32_t raw;
	struct {
		uint32_t DRC_INPUT_BLD_LUT_4             : 8;
		uint32_t DRC_INPUT_BLD_LUT_5             : 8;
		uint32_t DRC_INPUT_BLD_LUT_6             : 8;
		uint32_t DRC_INPUT_BLD_LUT_7             : 8;
	} bits;
};

union REG_ISP_DRC_INPUT_BLD_LUT_8_11 {
	uint32_t raw;
	struct {
		uint32_t DRC_INPUT_BLD_LUT_8             : 8;
		uint32_t DRC_INPUT_BLD_LUT_9             : 8;
		uint32_t DRC_INPUT_BLD_LUT_10            : 8;
		uint32_t DRC_INPUT_BLD_LUT_11            : 8;
	} bits;
};

union REG_ISP_DRC_INPUT_BLD_LUT_12_15 {
	uint32_t raw;
	struct {
		uint32_t DRC_INPUT_BLD_LUT_12            : 8;
		uint32_t DRC_INPUT_BLD_LUT_13            : 8;
		uint32_t DRC_INPUT_BLD_LUT_14            : 8;
		uint32_t DRC_INPUT_BLD_LUT_15            : 8;
	} bits;
};

union REG_ISP_DRC_INPUT_BLD_LUT_16 {
	uint32_t raw;
	struct {
		uint32_t DRC_INPUT_BLD_LUT_16            : 8;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_0_1 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_0          : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_1          : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_2_3 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_2          : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_3          : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_4_5 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_4          : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_5          : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_6_7 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_6          : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_7          : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_8_9 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_8          : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_9          : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_10_11 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_10         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_11         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_12_13 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_12         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_13         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_14_15 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_14         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_15         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_16_17 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_16         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_17         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_18_19 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_18         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_19         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_20_21 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_20         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_21         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_22_23 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_22         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_23         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_24_25 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_24         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_25         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_26_27 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_26         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_27         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_28_29 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_28         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_29         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_30_31 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_30         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_31         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_32_33 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_32         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_33         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_34_35 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_34         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_35         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_36_37 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_36         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_37         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_38_39 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_38         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_39         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_40_41 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_40         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_41         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_42_43 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_42         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_43         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_44_45 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_44         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_45         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_46_47 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_46         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_47         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_48_49 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_48         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_49         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_50_51 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_50         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_51         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_52_53 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_52         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_53         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_54_55 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_54         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_55         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_56_57 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_56         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_57         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_58_59 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_58         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_59         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_60_61 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_60         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_61         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_62_63 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_62         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_L_LUT_63         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_L_LUT_64 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_L_LUT_64         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_R_LUT_0_1 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_R_LUT_0          : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_R_LUT_1          : 12;
	} bits;
};

union REG_ISP_DRC_GTM_R_LUT_2_3 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_R_LUT_2          : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_R_LUT_3          : 12;
	} bits;
};

union REG_ISP_DRC_GTM_R_LUT_4_5 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_R_LUT_4          : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_R_LUT_5          : 12;
	} bits;
};

union REG_ISP_DRC_GTM_R_LUT_6_7 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_R_LUT_6          : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_R_LUT_7          : 12;
	} bits;
};

union REG_ISP_DRC_GTM_R_LUT_8_9 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_R_LUT_8          : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_R_LUT_9          : 12;
	} bits;
};

union REG_ISP_DRC_GTM_R_LUT_10_11 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_R_LUT_10         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_R_LUT_11         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_R_LUT_12_13 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_R_LUT_12         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_R_LUT_13         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_R_LUT_14_15 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_R_LUT_14         : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_TONE_CURVE_R_LUT_15         : 12;
	} bits;
};

union REG_ISP_DRC_GTM_R_LUT_16 {
	uint32_t raw;
	struct {
		uint32_t DRC_TONE_CURVE_R_LUT_16         : 12;
	} bits;
};

union REG_ISP_DRC_LUMA_PROT_LUT_0_1 {
	uint32_t raw;
	struct {
		uint32_t DRC_LUMA_PROT_LUT_0             : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_LUMA_PROT_LUT_1             : 12;
	} bits;
};

union REG_ISP_DRC_LUMA_PROT_LUT_2_3 {
	uint32_t raw;
	struct {
		uint32_t DRC_LUMA_PROT_LUT_2             : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_LUMA_PROT_LUT_3             : 12;
	} bits;
};

union REG_ISP_DRC_LUMA_PROT_LUT_4_5 {
	uint32_t raw;
	struct {
		uint32_t DRC_LUMA_PROT_LUT_4             : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_LUMA_PROT_LUT_5             : 12;
	} bits;
};

union REG_ISP_DRC_LUMA_PROT_LUT_6_7 {
	uint32_t raw;
	struct {
		uint32_t DRC_LUMA_PROT_LUT_6             : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_LUMA_PROT_LUT_7             : 12;
	} bits;
};

union REG_ISP_DRC_LUMA_PROT_LUT_8_9 {
	uint32_t raw;
	struct {
		uint32_t DRC_LUMA_PROT_LUT_8             : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_LUMA_PROT_LUT_9             : 12;
	} bits;
};

union REG_ISP_DRC_LUMA_PROT_LUT_10_11 {
	uint32_t raw;
	struct {
		uint32_t DRC_LUMA_PROT_LUT_10            : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_LUMA_PROT_LUT_11            : 12;
	} bits;
};

union REG_ISP_DRC_LUMA_PROT_LUT_12_13 {
	uint32_t raw;
	struct {
		uint32_t DRC_LUMA_PROT_LUT_12            : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_LUMA_PROT_LUT_13            : 12;
	} bits;
};

union REG_ISP_DRC_LUMA_PROT_LUT_14_15 {
	uint32_t raw;
	struct {
		uint32_t DRC_LUMA_PROT_LUT_14            : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_LUMA_PROT_LUT_15            : 12;
	} bits;
};

union REG_ISP_DRC_LUMA_PROT_LUT_16 {
	uint32_t raw;
	struct {
		uint32_t DRC_LUMA_PROT_LUT_16            : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_0_1 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_0        : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_1        : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_2_3 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_2        : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_3        : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_4_5 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_4        : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_5        : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_6_7 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_6        : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_7        : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_8_9 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_8        : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_9        : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_10_11 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_10       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_11       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_12_13 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_12       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_13       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_14_15 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_14       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_15       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_16_17 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_16       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_17       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_18_19 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_18       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_19       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_20_21 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_20       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_21       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_22_23 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_22       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_23       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_24_25 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_24       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_25       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_26_27 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_26       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_27       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_28_29 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_28       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_29       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_30_31 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_30       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_31       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_32_33 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_32       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_33       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_34_35 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_34       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_35       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_36_37 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_36       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_37       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_38_39 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_38       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_39       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_40_41 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_40       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_41       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_42_43 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_42       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_43       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_44_45 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_44       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_45       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_46_47 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_46       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_47       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_48_49 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_48       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_49       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_50_51 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_50       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_51       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_52_53 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_52       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_53       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_54_55 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_54       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_55       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_56_57 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_56       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_57       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_58_59 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_58       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_59       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_60_61 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_60       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_61       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_62_63 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_62       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_L_LUT_63       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_L_LUT_64 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_L_LUT_64       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_R_LUT_0_1 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_R_LUT_0        : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_R_LUT_1        : 12;
	} bits;
};

union REG_ISP_DRC_LTM_R_LUT_2_3 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_R_LUT_2        : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_R_LUT_3        : 12;
	} bits;
};

union REG_ISP_DRC_LTM_R_LUT_4_5 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_R_LUT_4        : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_R_LUT_5        : 12;
	} bits;
};

union REG_ISP_DRC_LTM_R_LUT_6_7 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_R_LUT_6        : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_R_LUT_7        : 12;
	} bits;
};

union REG_ISP_DRC_LTM_R_LUT_8_9 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_R_LUT_8        : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_R_LUT_9        : 12;
	} bits;
};

union REG_ISP_DRC_LTM_R_LUT_10_11 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_R_LUT_10       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_R_LUT_11       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_R_LUT_12_13 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_R_LUT_12       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_R_LUT_13       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_R_LUT_14_15 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_R_LUT_14       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DRC_OUTBLD_CURVE_R_LUT_15       : 12;
	} bits;
};

union REG_ISP_DRC_LTM_R_LUT_16 {
	uint32_t raw;
	struct {
		uint32_t DRC_OUTBLD_CURVE_R_LUT_16       : 12;
	} bits;
};

union REG_ISP_DRC_IMG_SIZE {
	uint32_t raw;
	struct {
		uint32_t IMG_WIDTH_M1                    : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t IMG_HEIGHT_M1                   : 14;
	} bits;
};

union REG_ISP_DRC_SHDW_SEL {
	uint32_t raw;
	struct {
		uint32_t DRC_SHDW_SEL                    : 1;
	} bits;
};

union REG_ISP_DRC_PIPECTRL_CG {
	uint32_t raw;
	struct {
		uint32_t HW_AUTO_CG_EN                   : 1;
		uint32_t _rsv_1                          : 7;
		uint32_t GUARD_CNT                       : 8;
		uint32_t BYPASS                          : 1;
	} bits;
};

union REG_ISP_DRC_BAYERID {
	uint32_t raw;
	struct {
		uint32_t PRE_IMG_BAYERID                 : 2;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_FBCD_00 {
	uint32_t raw;
	struct {
		uint32_t FBCD_EN                         : 1;
		uint32_t FF_DROP                         : 1;
		uint32_t FBCD_AUTO                       : 1;
		uint32_t FORCE_CKE                       : 1;
	} bits;
};

union REG_FBCD_0C {
	uint32_t raw;
	struct {
		uint32_t Y_BIT_STREAM_SIZE               : 32;
	} bits;
};

union REG_FBCD_10 {
	uint32_t raw;
	struct {
		uint32_t C_BIT_STREAM_SIZE               : 32;
	} bits;
};

union REG_FBCD_14 {
	uint32_t raw;
	struct {
		uint32_t DEBUG_SEL                       : 4;
	} bits;
};

union REG_FBCD_18 {
	uint32_t raw;
	struct {
		uint32_t DEBUG_BUS                       : 32;
	} bits;
};

union REG_FBCD_20 {
	uint32_t raw;
	struct {
		uint32_t DUMMY                           : 8;
		uint32_t SHD_RD                          : 1;
	} bits;
};

union REG_FBCD_24 {
	uint32_t raw;
	struct {
		uint32_t Y_LOSSLESS                      : 1;
		uint32_t Y_BASE_QDPCM_Q                  : 1;
		uint32_t _rsv_2                          : 2;
		uint32_t Y_BASE_PCM_BD_MINUS2            : 3;
		uint32_t _rsv_7                          : 1;
		uint32_t Y_DEFAULT_GR_K                  : 3;
	} bits;
};

union REG_FBCD_28 {
	uint32_t raw;
	struct {
		uint32_t C_LOSSLESS                      : 1;
		uint32_t C_BASE_QDPCM_Q                  : 1;
		uint32_t _rsv_2                          : 2;
		uint32_t C_BASE_PCM_BD_MINUS2            : 3;
		uint32_t _rsv_7                          : 1;
		uint32_t C_DEFAULT_GR_K                  : 3;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_FBCE_00 {
	uint32_t raw;
	struct {
		uint32_t FBCE_EN                         : 1;
		uint32_t SHD_RD                          : 1;
		uint32_t FORCE_CKE                       : 1;
		uint32_t _rsv_3                          : 5;
		uint32_t DEBUG                           : 16;
		uint32_t DUMMY                           : 8;
	} bits;
};

union REG_FBCE_10 {
	uint32_t raw;
	struct {
		uint32_t Y_LOSSLESS                      : 1;
		uint32_t Y_BASE_QDPCM_Q                  : 1;
		uint32_t _rsv_2                          : 2;
		uint32_t Y_BASE_PCM_BD_MINUS2            : 3;
		uint32_t _rsv_7                          : 1;
		uint32_t Y_DEFAULT_GR_K                  : 3;
		uint32_t _rsv_11                         : 1;
		uint32_t Y_CPLX_SHIFT                    : 3;
		uint32_t _rsv_15                         : 1;
		uint32_t Y_PEN_POS_SHIFT                 : 3;
	} bits;
};

union REG_FBCE_14 {
	uint32_t raw;
	struct {
		uint32_t Y_MIN_CU_BIT                    : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t Y_MAX_CU_BIT                    : 7;
	} bits;
};

union REG_FBCE_18 {
	uint32_t raw;
	struct {
		uint32_t _rsv_0                          : 6;
		uint32_t Y_BIT_STREAM_SIZE               : 22;
	} bits;
};

union REG_FBCE_1C {
	uint32_t raw;
	struct {
		uint32_t Y_TOTAL_LINE_BIT_BUDGET         : 17;
	} bits;
};

union REG_FBCE_20 {
	uint32_t raw;
	struct {
		uint32_t C_LOSSLESS                      : 1;
		uint32_t C_BASE_QDPCM_Q                  : 1;
		uint32_t _rsv_2                          : 2;
		uint32_t C_BASE_PCM_BD_MINUS2            : 3;
		uint32_t _rsv_7                          : 1;
		uint32_t C_DEFAULT_GR_K                  : 3;
		uint32_t _rsv_11                         : 1;
		uint32_t C_CPLX_SHIFT                    : 3;
		uint32_t _rsv_15                         : 1;
		uint32_t C_PEN_POS_SHIFT                 : 3;
	} bits;
};

union REG_FBCE_24 {
	uint32_t raw;
	struct {
		uint32_t C_MIN_CU_BIT                    : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t C_MAX_CU_BIT                    : 7;
	} bits;
};

union REG_FBCE_28 {
	uint32_t raw;
	struct {
		uint32_t _rsv_0                          : 6;
		uint32_t C_BIT_STREAM_SIZE               : 22;
	} bits;
};

union REG_FBCE_2C {
	uint32_t raw;
	struct {
		uint32_t C_TOTAL_LINE_BIT_BUDGET         : 17;
	} bits;
};

union REG_FBCE_30 {
	uint32_t raw;
	struct {
		uint32_t Y_TOTAL_FIRST_LINE_BIT_BUDGET   : 17;
	} bits;
};

union REG_FBCE_34 {
	uint32_t raw;
	struct {
		uint32_t C_TOTAL_FIRST_LINE_BIT_BUDGET   : 17;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_CFA_00 {
	uint32_t raw;
	struct {
		uint32_t CFA_SHDW_SEL                    : 1;
		uint32_t CFA_ENABLE                      : 1;
		uint32_t _rsv_2                          : 1;
		uint32_t CFA_YMOIRE_ENABLE               : 1;
		uint32_t DELAY                           : 1;
		uint32_t FORCE_CLK_ENABLE                : 1;
		uint32_t CFA_FORCE_DIR_ENABLE            : 1;
		uint32_t _rsv_7                          : 1;
		uint32_t CFA_FORCE_DIR_SEL               : 2;
	} bits;
};

union REG_ISP_CFA_04 {
	uint32_t raw;
	struct {
		uint32_t CFA_OUT_SEL                     : 1;
		uint32_t CONT_EN                         : 1;
		uint32_t _rsv_2                          : 1;
		uint32_t SOFTRST                         : 1;
		uint32_t _rsv_4                          : 1;
		uint32_t DBG_EN                          : 1;
		uint32_t _rsv_6                          : 10;
		uint32_t CFA_EDGEE_THD2                  : 12;
	} bits;
};

union REG_ISP_CFA_0C {
	uint32_t raw;
	struct {
		uint32_t CFA_EDGEE_THD                   : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t CFA_SIGE_THD                    : 12;
	} bits;
};

union REG_ISP_CFA_10 {
	uint32_t raw;
	struct {
		uint32_t CFA_GSIG_TOL                    : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t CFA_RBSIG_TOL                   : 12;
	} bits;
};

union REG_ISP_CFA_14 {
	uint32_t raw;
	struct {
		uint32_t CFA_EDGE_TOL                    : 12;
	} bits;
};

union REG_ISP_CFA_18 {
	uint32_t raw;
	struct {
		uint32_t CFA_GHP_THD                     : 16;
	} bits;
};

union REG_ISP_CFA_1C {
	uint32_t raw;
	struct {
		uint32_t CFA_RB_VT_ENABLE                : 1;
	} bits;
};

union REG_ISP_CFA_20 {
	uint32_t raw;
	struct {
		uint32_t CFA_RBSIG_LUMA_THD              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t CFA_VW_THD                      : 12;
	} bits;
};

union REG_ISP_CFA_30 {
	uint32_t raw;
	struct {
		uint32_t CFA_GHP_LUT00                   : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t CFA_GHP_LUT01                   : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t CFA_GHP_LUT02                   : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t CFA_GHP_LUT03                   : 6;
	} bits;
};

union REG_ISP_CFA_34 {
	uint32_t raw;
	struct {
		uint32_t CFA_GHP_LUT04                   : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t CFA_GHP_LUT05                   : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t CFA_GHP_LUT06                   : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t CFA_GHP_LUT07                   : 6;
	} bits;
};

union REG_ISP_CFA_38 {
	uint32_t raw;
	struct {
		uint32_t CFA_GHP_LUT08                   : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t CFA_GHP_LUT09                   : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t CFA_GHP_LUT10                   : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t CFA_GHP_LUT11                   : 6;
	} bits;
};

union REG_ISP_CFA_3C {
	uint32_t raw;
	struct {
		uint32_t CFA_GHP_LUT12                   : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t CFA_GHP_LUT13                   : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t CFA_GHP_LUT14                   : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t CFA_GHP_LUT15                   : 6;
	} bits;
};

union REG_ISP_CFA_40 {
	uint32_t raw;
	struct {
		uint32_t CFA_GHP_LUT16                   : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t CFA_GHP_LUT17                   : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t CFA_GHP_LUT18                   : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t CFA_GHP_LUT19                   : 6;
	} bits;
};

union REG_ISP_CFA_44 {
	uint32_t raw;
	struct {
		uint32_t CFA_GHP_LUT20                   : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t CFA_GHP_LUT21                   : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t CFA_GHP_LUT22                   : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t CFA_GHP_LUT23                   : 6;
	} bits;
};

union REG_ISP_CFA_48 {
	uint32_t raw;
	struct {
		uint32_t CFA_GHP_LUT24                   : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t CFA_GHP_LUT25                   : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t CFA_GHP_LUT26                   : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t CFA_GHP_LUT27                   : 6;
	} bits;
};

union REG_ISP_CFA_4C {
	uint32_t raw;
	struct {
		uint32_t CFA_GHP_LUT28                   : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t CFA_GHP_LUT29                   : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t CFA_GHP_LUT30                   : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t CFA_GHP_LUT31                   : 6;
	} bits;
};

union REG_ISP_CFA_70 {
	uint32_t raw;
	struct {
		uint32_t DIR_READCNT_FROM_LINE0          : 5;
	} bits;
};

union REG_ISP_CFA_74 {
	uint32_t raw;
	struct {
		uint32_t PROB_OUT_SEL                    : 4;
		uint32_t PROB_PERFMT                     : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t PROB_FMT                        : 6;
	} bits;
};

union REG_ISP_CFA_78 {
	uint32_t raw;
	struct {
		uint32_t PROB_LINE                       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t PROB_PIX                        : 12;
	} bits;
};

union REG_ISP_CFA_7C {
	uint32_t raw;
	struct {
		uint32_t CFA_DBG0                        : 32;
	} bits;
};

union REG_ISP_CFA_80 {
	uint32_t raw;
	struct {
		uint32_t CFA_DBG1                        : 32;
	} bits;
};

union REG_ISP_CFA_90 {
	uint32_t raw;
	struct {
		uint32_t CFA_YMOIRE_REF_MAXG_ONLY        : 1;
		uint32_t _rsv_1                          : 7;
		uint32_t CFA_YMOIRE_NP                   : 8;
	} bits;
};

union REG_ISP_CFA_94 {
	uint32_t raw;
	struct {
		uint32_t CFA_YMOIRE_DETAIL_TH            : 8;
		uint32_t CFA_YMOIRE_DETAIL_LOW           : 9;
	} bits;
};

union REG_ISP_CFA_98 {
	uint32_t raw;
	struct {
		uint32_t CFA_YMOIRE_DETAIL_HIGH          : 9;
		uint32_t _rsv_9                          : 7;
		uint32_t CFA_YMOIRE_DETAIL_SLOPE         : 15;
	} bits;
};

union REG_ISP_CFA_9C {
	uint32_t raw;
	struct {
		uint32_t CFA_YMOIRE_EDGE_TH              : 8;
		uint32_t CFA_YMOIRE_EDGE_LOW             : 9;
	} bits;
};

union REG_ISP_CFA_A0 {
	uint32_t raw;
	struct {
		uint32_t CFA_YMOIRE_EDGE_HIGH            : 9;
		uint32_t _rsv_9                          : 7;
		uint32_t CFA_YMOIRE_EDGE_SLOPE           : 15;
	} bits;
};

union REG_ISP_CFA_A4 {
	uint32_t raw;
	struct {
		uint32_t CFA_YMOIRE_LUT_TH               : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t CFA_YMOIRE_LUT_LOW              : 9;
	} bits;
};

union REG_ISP_CFA_A8 {
	uint32_t raw;
	struct {
		uint32_t CFA_YMOIRE_LUT_HIGH             : 9;
		uint32_t _rsv_9                          : 7;
		uint32_t CFA_YMOIRE_LUT_SLOPE            : 15;
	} bits;
};

union REG_ISP_CFA_110 {
	uint32_t raw;
	struct {
		uint32_t CFA_YMOIRE_LPF_W                : 8;
		uint32_t CFA_YMOIRE_DC_W                 : 8;
	} bits;
};

union REG_ISP_CFA_120 {
	uint32_t raw;
	struct {
		uint32_t CFA_CMOIRE_ENABLE               : 1;
		uint32_t _rsv_1                          : 7;
		uint32_t CFA_CMOIRE_STRTH                : 8;
	} bits;
};

union REG_ISP_CFA_124 {
	uint32_t raw;
	struct {
		uint32_t CFA_CMOIRE_SAT_X0               : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t CFA_CMOIRE_SAT_Y0               : 12;
	} bits;
};

union REG_ISP_CFA_128 {
	uint32_t raw;
	struct {
		uint32_t CFA_CMOIRE_SAT_SLP0             : 18;
	} bits;
};

union REG_ISP_CFA_12C {
	uint32_t raw;
	struct {
		uint32_t CFA_CMOIRE_SAT_X1               : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t CFA_CMOIRE_SAT_Y1               : 12;
	} bits;
};

union REG_ISP_CFA_130 {
	uint32_t raw;
	struct {
		uint32_t CFA_CMOIRE_PTCLR_X0             : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t CFA_CMOIRE_PTCLR_Y0             : 12;
	} bits;
};

union REG_ISP_CFA_134 {
	uint32_t raw;
	struct {
		uint32_t CFA_CMOIRE_PTCLR_SLP0           : 18;
	} bits;
};

union REG_ISP_CFA_138 {
	uint32_t raw;
	struct {
		uint32_t CFA_CMOIRE_PTCLR_X1             : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t CFA_CMOIRE_PTCLR_Y1             : 12;
	} bits;
};

union REG_ISP_CFA_13C {
	uint32_t raw;
	struct {
		uint32_t CFA_CMOIRE_PROTCLR1_ENABLE      : 1;
		uint32_t CFA_CMOIRE_PROTCLR2_ENABLE      : 1;
		uint32_t CFA_CMOIRE_PROTCLR3_ENABLE      : 1;
	} bits;
};

union REG_ISP_CFA_140 {
	uint32_t raw;
	struct {
		uint32_t CFA_CMOIRE_PROTCLR1             : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t CFA_CMOIRE_PROTCLR2             : 12;
	} bits;
};

union REG_ISP_CFA_144 {
	uint32_t raw;
	struct {
		uint32_t CFA_CMOIRE_PROTCLR3             : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t CFA_CMOIRE_PD_X0                : 12;
	} bits;
};

union REG_ISP_CFA_148 {
	uint32_t raw;
	struct {
		uint32_t CFA_CMOIRE_PD_Y0                : 12;
	} bits;
};

union REG_ISP_CFA_14C {
	uint32_t raw;
	struct {
		uint32_t CFA_CMOIRE_PD_SLP0              : 18;
	} bits;
};

union REG_ISP_CFA_150 {
	uint32_t raw;
	struct {
		uint32_t CFA_CMOIRE_PD_X1                : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t CFA_CMOIRE_PD_Y1                : 12;
	} bits;
};

union REG_ISP_CFA_154 {
	uint32_t raw;
	struct {
		uint32_t CFA_CMOIRE_EDGE_X0              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t CFA_CMOIRE_EDGE_Y0              : 12;
	} bits;
};

union REG_ISP_CFA_158 {
	uint32_t raw;
	struct {
		uint32_t CFA_CMOIRE_EDGE_SLP0            : 18;
	} bits;
};

union REG_ISP_CFA_15C {
	uint32_t raw;
	struct {
		uint32_t CFA_CMOIRE_EDGE_X1              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t CFA_CMOIRE_EDGE_Y1              : 12;
	} bits;
};

union REG_ISP_CFA_160 {
	uint32_t raw;
	struct {
		uint32_t CFA_CMOIRE_LUMAGAIN_ENABLE      : 1;
	} bits;
};

union REG_ISP_CFA_164 {
	uint32_t raw;
	struct {
		uint32_t CFA_CMOIRE_LUMATG               : 12;
	} bits;
};

union REG_ISP_CFA_168 {
	uint32_t raw;
	struct {
		uint32_t CFA_CMOIRE_EDGE_D0C0            : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t CFA_CMOIRE_EDGE_D0C1            : 13;
	} bits;
};

union REG_ISP_CFA_16C {
	uint32_t raw;
	struct {
		uint32_t CFA_CMOIRE_EDGE_D0C2            : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t CFA_CMOIRE_EDGE_D45C0           : 13;
	} bits;
};

union REG_ISP_CFA_170 {
	uint32_t raw;
	struct {
		uint32_t CFA_CMOIRE_EDGE_D45C1           : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t CFA_CMOIRE_EDGE_D45C2           : 13;
	} bits;
};

union REG_ISP_CFA_174 {
	uint32_t raw;
	struct {
		uint32_t CFA_CMOIRE_EDGE_D45C3           : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t CFA_CMOIRE_EDGE_D45C4           : 13;
	} bits;
};

union REG_ISP_CFA_178 {
	uint32_t raw;
	struct {
		uint32_t CFA_CMOIRE_EDGE_D45C5           : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t CFA_CMOIRE_EDGE_D45C6           : 13;
	} bits;
};

union REG_ISP_CFA_17C {
	uint32_t raw;
	struct {
		uint32_t CFA_CMOIRE_EDGE_D45C7           : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t CFA_CMOIRE_EDGE_D45C8           : 13;
	} bits;
};

union REG_ISP_CFA_180 {
	uint32_t raw;
	struct {
		uint32_t _CFA_SHPN_ENABLE                : 1;
		uint32_t _CFA_SHPN_PRE_PROC_ENABLE       : 1;
		uint32_t _rsv_2                          : 6;
		uint32_t _CFA_SHPN_MIN_Y                 : 12;
	} bits;
};

union REG_ISP_CFA_184 {
	uint32_t raw;
	struct {
		uint32_t _CFA_SHPN_MIN_GAIN              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t _CFA_SHPN_MAX_GAIN              : 12;
	} bits;
};

union REG_ISP_CFA_188 {
	uint32_t raw;
	struct {
		uint32_t CFA_SHPN_MF_CORE_GAIN           : 8;
		uint32_t CFA_SHPN_HF_BLEND_WGT           : 8;
		uint32_t CFA_SHPN_MF_BLEND_WGT           : 8;
	} bits;
};

union REG_ISP_CFA_18C {
	uint32_t raw;
	struct {
		uint32_t CFA_SHPN_CORE_VALUE             : 12;
	} bits;
};

union REG_ISP_CFA_190 {
	uint32_t raw;
	struct {
		uint32_t CFA_SHPN_GAIN_VALUE             : 8;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_RGB_TOP_PATGEN_01 {
	uint32_t raw;
	struct {
		uint32_t PG_ENABLE                       : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t CURSER_EN                       : 1;
		uint32_t _rsv_5                          : 11;
		uint32_t CURSER_VALUE                    : 16;
	} bits;
};

union REG_RGB_TOP_PATGEN_02 {
	uint32_t raw;
	struct {
		uint32_t X_CURSER                        : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t Y_CURSER                        : 14;
	} bits;
};

union REG_RGB_TOP_PATGEN_03 {
	uint32_t raw;
	struct {
		uint32_t VALUE_REPORT                    : 32;
	} bits;
};

union REG_RGB_TOP_PATGEN_04 {
	uint32_t raw;
	struct {
		uint32_t XCNT_RPT                        : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t YCNT_RPT                        : 14;
	} bits;
};

union REG_RGB_TOP_DUMMY {
	uint32_t raw;
	struct {
		uint32_t DUMMY                           : 32;
	} bits;
};

union REG_RGB_TOP_PATGEN_05 {
	uint32_t raw;
	struct {
		uint32_t RGBTOP_IMGH                     : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t RGBTOP_IMGW                     : 14;
		uint32_t _rsv_30                         : 1;
		uint32_t PFR_BYPASS_RGBTOP               : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_EE_EXT_REG0 {
	uint32_t raw;
	struct {
		uint32_t EE_ENABLE                       : 1;
		uint32_t EE_DEBUG_MODE                   : 4;
		uint32_t HW_AUTO_CG_EN                   : 1;
		uint32_t EE_CH_SELECTION                 : 1;
		uint32_t EE_GAMMA_SELECTION              : 1;
		uint32_t EE_GAMMA_LUT_U10_0              : 10;
		uint32_t EE_GAMMA_LUT_U10_1              : 10;
	} bits;
};

union REG_EE_EXT_GAMMA_0 {
	uint32_t raw;
	struct {
		uint32_t EE_GAMMA_LUT_U10_2              : 10;
		uint32_t EE_GAMMA_LUT_U10_3              : 10;
		uint32_t EE_GAMMA_LUT_U10_4              : 10;
	} bits;
};

union REG_EE_EXT_GAMMA_1 {
	uint32_t raw;
	struct {
		uint32_t EE_GAMMA_LUT_U10_5              : 10;
		uint32_t EE_GAMMA_LUT_U10_6              : 10;
		uint32_t EE_GAMMA_LUT_U10_7              : 10;
	} bits;
};

union REG_EE_EXT_GAMMA_2 {
	uint32_t raw;
	struct {
		uint32_t EE_GAMMA_LUT_U10_8              : 10;
		uint32_t EE_GAMMA_LUT_U10_9              : 10;
		uint32_t EE_GAMMA_LUT_U10_10             : 10;
	} bits;
};

union REG_EE_EXT_GAMMA_3 {
	uint32_t raw;
	struct {
		uint32_t EE_GAMMA_LUT_U10_11             : 10;
		uint32_t EE_GAMMA_LUT_U10_12             : 10;
		uint32_t EE_GAMMA_LUT_U10_13             : 10;
	} bits;
};

union REG_EE_EXT_GAMMA_4 {
	uint32_t raw;
	struct {
		uint32_t EE_GAMMA_LUT_U10_14             : 10;
		uint32_t EE_GAMMA_LUT_U10_15             : 10;
		uint32_t EE_GAMMA_LUT_U10_16             : 10;
	} bits;
};

union REG_EE_EXT_GAMMA_5 {
	uint32_t raw;
	struct {
		uint32_t EE_GAMMA_LUT_U10_17             : 10;
		uint32_t EE_GAMMA_LUT_U10_18             : 10;
		uint32_t EE_GAMMA_LUT_U10_19             : 10;
	} bits;
};

union REG_EE_EXT_GAMMA_6 {
	uint32_t raw;
	struct {
		uint32_t EE_GAMMA_LUT_U10_20             : 10;
		uint32_t EE_GAMMA_LUT_U10_21             : 10;
		uint32_t EE_GAMMA_LUT_U10_22             : 10;
	} bits;
};

union REG_EE_EXT_GAMMA_7 {
	uint32_t raw;
	struct {
		uint32_t EE_GAMMA_LUT_U10_23             : 10;
		uint32_t EE_GAMMA_LUT_U10_24             : 10;
		uint32_t EE_GAMMA_LUT_U10_25             : 10;
	} bits;
};

union REG_EE_EXT_GAMMA_8 {
	uint32_t raw;
	struct {
		uint32_t EE_GAMMA_LUT_U10_26             : 10;
		uint32_t EE_GAMMA_LUT_U10_27             : 10;
		uint32_t EE_GAMMA_LUT_U10_28             : 10;
	} bits;
};

union REG_EE_EXT_GAMMA_9 {
	uint32_t raw;
	struct {
		uint32_t EE_GAMMA_LUT_U10_29             : 10;
		uint32_t EE_GAMMA_LUT_U10_30             : 10;
		uint32_t EE_GAMMA_LUT_U10_31             : 10;
	} bits;
};

union REG_EE_EXT_GAMMA_10 {
	uint32_t raw;
	struct {
		uint32_t EE_GAMMA_LUT_U10_32             : 10;
		uint32_t EE_GAMMA_LUT_U10_33             : 10;
		uint32_t EE_GAMMA_LUT_U10_34             : 10;
	} bits;
};

union REG_EE_EXT_GAMMA_11 {
	uint32_t raw;
	struct {
		uint32_t EE_GAMMA_LUT_U10_35             : 10;
		uint32_t EE_GAMMA_LUT_U10_36             : 10;
		uint32_t EE_GAMMA_LUT_U10_37             : 10;
	} bits;
};

union REG_EE_EXT_GAMMA_12 {
	uint32_t raw;
	struct {
		uint32_t EE_GAMMA_LUT_U10_38             : 10;
		uint32_t EE_GAMMA_LUT_U10_39             : 10;
		uint32_t EE_GAMMA_LUT_U10_40             : 10;
	} bits;
};

union REG_EE_EXT_GAMMA_13 {
	uint32_t raw;
	struct {
		uint32_t EE_GAMMA_LUT_U10_41             : 10;
		uint32_t EE_GAMMA_LUT_U10_42             : 10;
		uint32_t EE_GAMMA_LUT_U10_43             : 10;
	} bits;
};

union REG_EE_EXT_GAMMA_14 {
	uint32_t raw;
	struct {
		uint32_t EE_GAMMA_LUT_U10_44             : 10;
		uint32_t EE_GAMMA_LUT_U10_45             : 10;
		uint32_t EE_GAMMA_LUT_U10_46             : 10;
	} bits;
};

union REG_EE_EXT_GAMMA_15 {
	uint32_t raw;
	struct {
		uint32_t EE_GAMMA_LUT_U10_47             : 10;
		uint32_t EE_GAMMA_LUT_U10_48             : 10;
		uint32_t EE_GAMMA_LUT_U10_49             : 10;
	} bits;
};

union REG_EE_EXT_GAMMA_16 {
	uint32_t raw;
	struct {
		uint32_t EE_GAMMA_LUT_U10_50             : 10;
		uint32_t EE_GAMMA_LUT_U10_51             : 10;
		uint32_t EE_GAMMA_LUT_U10_52             : 10;
	} bits;
};

union REG_EE_EXT_GAMMA_17 {
	uint32_t raw;
	struct {
		uint32_t EE_GAMMA_LUT_U10_53             : 10;
		uint32_t EE_GAMMA_LUT_U10_54             : 10;
		uint32_t EE_GAMMA_LUT_U10_55             : 10;
	} bits;
};

union REG_EE_EXT_GAMMA_18 {
	uint32_t raw;
	struct {
		uint32_t EE_GAMMA_LUT_U10_56             : 10;
		uint32_t EE_GAMMA_LUT_U10_57             : 10;
		uint32_t EE_GAMMA_LUT_U10_58             : 10;
	} bits;
};

union REG_EE_EXT_GAMMA_19 {
	uint32_t raw;
	struct {
		uint32_t EE_GAMMA_LUT_U10_59             : 10;
		uint32_t EE_GAMMA_LUT_U10_60             : 10;
		uint32_t EE_GAMMA_LUT_U10_61             : 10;
	} bits;
};

union REG_EE_EXT_GAMMA_20 {
	uint32_t raw;
	struct {
		uint32_t EE_GAMMA_LUT_U10_62             : 10;
		uint32_t EE_GAMMA_LUT_U10_63             : 10;
		uint32_t EE_GAMMA_LUT_U10_64             : 10;
	} bits;
};

union REG_EE_EXT_E5C_0 {
	uint32_t raw;
	struct {
		uint32_t EE_E5C_COEF_S11_0               : 11;
		uint32_t EE_E5C_COEF_S11_1               : 11;
	} bits;
};

union REG_EE_EXT_E5C_1 {
	uint32_t raw;
	struct {
		uint32_t EE_E5C_COEF_S11_2               : 11;
		uint32_t EE_E5C_COEF_S11_3               : 11;
	} bits;
};

union REG_EE_EXT_E5C_2 {
	uint32_t raw;
	struct {
		uint32_t EE_E5C_COEF_S11_4               : 11;
		uint32_t EE_E5C_COEF_S11_5               : 11;
		uint32_t EE_E5C_ENH                      : 10;
	} bits;
};

union REG_EE_EXT_E5A_0 {
	uint32_t raw;
	struct {
		uint32_t EE_E5A_COEF_S11_0               : 11;
		uint32_t EE_E5A_COEF_S11_1               : 11;
	} bits;
};

union REG_EE_EXT_E5A_1 {
	uint32_t raw;
	struct {
		uint32_t EE_E5A_COEF_S11_2               : 11;
		uint32_t EE_E5A_COEF_S11_3               : 11;
	} bits;
};

union REG_EE_EXT_E5A_2 {
	uint32_t raw;
	struct {
		uint32_t EE_E5A_COEF_S11_4               : 11;
		uint32_t EE_E5A_COEF_S11_5               : 11;
		uint32_t EE_E5A_ENH                      : 10;
	} bits;
};

union REG_EE_EXT_E5B_0 {
	uint32_t raw;
	struct {
		uint32_t EE_E5B_COEF_S11_0               : 11;
		uint32_t EE_E5B_COEF_S11_1               : 11;
	} bits;
};

union REG_EE_EXT_E5B_1 {
	uint32_t raw;
	struct {
		uint32_t EE_E5B_COEF_S11_2               : 11;
		uint32_t EE_E5B_COEF_S11_3               : 11;
	} bits;
};

union REG_EE_EXT_E5B_2 {
	uint32_t raw;
	struct {
		uint32_t EE_E5B_COEF_S11_4               : 11;
		uint32_t EE_E5B_COEF_S11_5               : 11;
		uint32_t EE_E5B_ENH                      : 10;
	} bits;
};

union REG_EE_EXT_E7_0 {
	uint32_t raw;
	struct {
		uint32_t EE_E7_COEF_S11_0                : 11;
		uint32_t EE_E7_COEF_S11_1                : 11;
	} bits;
};

union REG_EE_EXT_E7_1 {
	uint32_t raw;
	struct {
		uint32_t EE_E7_COEF_S11_2                : 11;
		uint32_t EE_E7_COEF_S11_3                : 11;
	} bits;
};

union REG_EE_EXT_E7_2 {
	uint32_t raw;
	struct {
		uint32_t EE_E7_COEF_S11_4                : 11;
		uint32_t EE_E7_COEF_S11_5                : 11;
	} bits;
};

union REG_EE_EXT_E7_3 {
	uint32_t raw;
	struct {
		uint32_t EE_E7_COEF_S11_6                : 11;
		uint32_t EE_E7_COEF_S11_7                : 11;
	} bits;
};

union REG_EE_EXT_E7_4 {
	uint32_t raw;
	struct {
		uint32_t EE_E7_COEF_S11_8                : 11;
		uint32_t EE_E7_COEF_S11_9                : 11;
		uint32_t EE_E7_ENH                       : 10;
	} bits;
};

union REG_EE_EXT_NORM {
	uint32_t raw;
	struct {
		uint32_t EE_E5C_NORM                     : 4;
		uint32_t EE_E5A_NORM                     : 4;
		uint32_t EE_E5B_NORM                     : 4;
		uint32_t EE_E7_NORM                      : 4;
		uint32_t EE_ENG_NORM                     : 5;
		uint32_t EE_CON_NORM                     : 5;
		uint32_t EE_CON_ENG_WT                   : 4;
	} bits;
};

union REG_EE_EXT_LUMA_BLEND {
	uint32_t raw;
	struct {
		uint32_t EE_BLEND_TH                     : 8;
		uint32_t EE_BLEND_SLOPE                  : 6;
		uint32_t EE_BLEND_W1                     : 8;
		uint32_t EE_BLEND_W2                     : 8;
	} bits;
};

union REG_EE_EXT_REGION_0 {
	uint32_t raw;
	struct {
		uint32_t EE_TH_FLAT                      : 10;
		uint32_t EE_TH_EDGE                      : 10;
		uint32_t EE_TH_FLAT_HLD                  : 10;
	} bits;
};

union REG_EE_EXT_REGION_1 {
	uint32_t raw;
	struct {
		uint32_t EE_TH_EDGE_HLD                  : 10;
		uint32_t EE_TH_LUM_HLD                   : 10;
	} bits;
};

union REG_EE_EXT_REGION_2 {
	uint32_t raw;
	struct {
		uint32_t EE_THIN_WT_FOR_FLAT             : 5;
		uint32_t EE_THIN_WT_FOR_EDGE             : 5;
		uint32_t EE_THIN_WT_HLD_FLAT             : 5;
		uint32_t EE_THIN_WT_HLD_EDGE             : 5;
		uint32_t EE_REGION_STR_ENABLE            : 1;
	} bits;
};

union REG_EE_EXT_REGION_3 {
	uint32_t raw;
	struct {
		uint32_t EE_STR_FLAT_REGION              : 8;
		uint32_t EE_STR_EDGE_REGION              : 8;
		uint32_t EE_THIN_ENH                     : 8;
		uint32_t EE_ROBUST_ENH                   : 8;
	} bits;
};

union REG_EE_EXT_REGION_4 {
	uint32_t raw;
	struct {
		uint32_t EE_SLOPE_CON_ENG_HLD            : 16;
		uint32_t EE_SLOPE_CON_ENG                : 16;
	} bits;
};

union REG_EE_EXT_REGION_5 {
	uint32_t raw;
	struct {
		uint32_t EE_SLOPE_FLAT_REGION            : 16;
		uint32_t EE_SLOPE_EDGE_REGION            : 16;
	} bits;
};

union REG_EE_EXT_REGION_6 {
	uint32_t raw;
	struct {
		uint32_t EE_THIN_WT_UHF_HF               : 4;
		uint32_t EE_ROBUST_WT_HF_SHF             : 4;
		uint32_t EE_THIN_WT_AGAINST_SHF          : 4;
		uint32_t EE_ROBUST_WT_AGAINST_MF         : 4;
		uint32_t EE_THIN_HLD_WT_UHF_HF           : 4;
		uint32_t EE_ROBUST_HLD_WT_HF_SHF         : 4;
		uint32_t EE_THIN_HLD_WT_AGAINST_SHF      : 4;
		uint32_t EE_ROBUST_HLD_WT_AGAINST_MF     : 4;
	} bits;
};

union REG_EE_EXT_SHADOW_RD_SEL {
	uint32_t raw;
	struct {
		uint32_t SHDW_READ_SEL                   : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_PFR_REG0 {
	uint32_t raw;
	struct {
		uint32_t PFR_EN                          : 1;
		uint32_t FCS_EN                          : 1;
		uint32_t HW_AUTO_CG_EN                   : 1;
		uint32_t PFR_LUMA_LEVEL_EN               : 1;
		uint32_t PFR_UVSET_EN_0                  : 1;
		uint32_t PFR_UVSET_EN_1                  : 1;
		uint32_t PFR_HUESET_EN_0                 : 1;
		uint32_t PFR_HUESET_EN_1                 : 1;
	} bits;
};

union REG_ISP_PFR_LUMA_SET0 {
	uint32_t raw;
	struct {
		uint32_t PFR_LUMA_LEVEL_TH               : 8;
		uint32_t PFR_LUMA_INIVALUE               : 8;
	} bits;
};

union REG_ISP_PFR_LUMA_LEVEL0 {
	uint32_t raw;
	struct {
		uint32_t PFR_LUMA_LEVEL_0                : 8;
		uint32_t PFR_LUMA_LEVEL_1                : 8;
		uint32_t PFR_LUMA_LEVEL_2                : 8;
		uint32_t PFR_LUMA_LEVEL_3                : 8;
	} bits;
};

union REG_ISP_PFR_LUMA_LEVEL1 {
	uint32_t raw;
	struct {
		uint32_t PFR_LUMA_LEVEL_4                : 8;
		uint32_t PFR_LUMA_LEVEL_5                : 8;
		uint32_t PFR_LUMA_LEVEL_6                : 8;
		uint32_t PFR_LUMA_LEVEL_7                : 8;
	} bits;
};

union REG_ISP_PFR_LUMA_LEVEL2 {
	uint32_t raw;
	struct {
		uint32_t PFR_LUMA_LEVEL_8                : 8;
		uint32_t PFR_LUMA_LEVEL_9                : 8;
		uint32_t PFR_LUMA_LEVEL_10               : 8;
		uint32_t PFR_LUMA_LEVEL_11               : 8;
	} bits;
};

union REG_ISP_PFR_LUMA_LEVEL3 {
	uint32_t raw;
	struct {
		uint32_t PFR_LUMA_LEVEL_12               : 8;
		uint32_t PFR_LUMA_LEVEL_13               : 8;
		uint32_t PFR_LUMA_LEVEL_14               : 8;
		uint32_t PFR_LUMA_LEVEL_15               : 8;
	} bits;
};

union REG_ISP_PFR_LUMA_LEVEL4 {
	uint32_t raw;
	struct {
		uint32_t PFR_LUMA_LEVEL_16               : 8;
	} bits;
};

union REG_ISP_PFR_COLOR_UV {
	uint32_t raw;
	struct {
		uint32_t PFR_COLOR_U_0                   : 8;
		uint32_t PFR_COLOR_U_1                   : 8;
		uint32_t PFR_COLOR_V_0                   : 8;
		uint32_t PFR_COLOR_V_1                   : 8;
	} bits;
};

union REG_ISP_PFR_UV_DIFF_TH {
	uint32_t raw;
	struct {
		uint32_t PFR_COLOR_DIFF_TH_SET_0         : 8;
		uint32_t PFR_COLOR_DIFF_TH_SET_1         : 8;
		uint32_t PFR_COLOR_DIFF_LUT_SET0_0       : 8;
		uint32_t PFR_COLOR_DIFF_LUT_SET0_1       : 8;
	} bits;
};

union REG_ISP_PFR_UV_DIFF_LUT0 {
	uint32_t raw;
	struct {
		uint32_t PFR_COLOR_DIFF_LUT_SET0_2       : 8;
		uint32_t PFR_COLOR_DIFF_LUT_SET0_3       : 8;
		uint32_t PFR_COLOR_DIFF_LUT_SET0_4       : 8;
		uint32_t PFR_COLOR_DIFF_LUT_SET1_0       : 8;
	} bits;
};

union REG_ISP_PFR_UV_DIFF_LUT1 {
	uint32_t raw;
	struct {
		uint32_t PFR_COLOR_DIFF_LUT_SET1_1       : 8;
		uint32_t PFR_COLOR_DIFF_LUT_SET1_2       : 8;
		uint32_t PFR_COLOR_DIFF_LUT_SET1_3       : 8;
		uint32_t PFR_COLOR_DIFF_LUT_SET1_4       : 8;
	} bits;
};

union REG_ISP_PFR_RB_WET {
	uint32_t raw;
	struct {
		uint32_t PFR_R_WET_0                     : 8;
		uint32_t PFR_R_WET_1                     : 8;
		uint32_t PFR_B_WET_0                     : 8;
		uint32_t PFR_B_WET_1                     : 8;
	} bits;
};

union REG_ISP_PFR_HUESET {
	uint32_t raw;
	struct {
		uint32_t PFR_HUE_0                       : 9;
		uint32_t PFR_HUE_1                       : 9;
	} bits;
};

union REG_ISP_PFR_HUE_RANGE {
	uint32_t raw;
	struct {
		uint32_t PFR_HUE_RANGE_0                 : 8;
		uint32_t PFR_HUE_RANGE_1                 : 8;
	} bits;
};

union REG_ISP_PFR_HUE_TH {
	uint32_t raw;
	struct {
		uint32_t PFR_HUE_TH                      : 8;
	} bits;
};

union REG_ISP_PFR_G_DIFF_TH0 {
	uint32_t raw;
	struct {
		uint32_t PFR_G_DIFF_TH_SET_0             : 8;
		uint32_t PFR_G_DIFF_TH_SET_1             : 8;
		uint32_t PFR_G_DIFF_TH_SET_2             : 8;
		uint32_t PFR_G_DIFF_TH_SET_3             : 8;
	} bits;
};

union REG_ISP_PFR_G_DIFF_TH1 {
	uint32_t raw;
	struct {
		uint32_t PFR_G_DIFF_TH_SET_4             : 8;
	} bits;
};

union REG_ISP_PFR_EDGE_TH {
	uint32_t raw;
	struct {
		uint32_t PFR_EDGE_TH_0                   : 8;
		uint32_t PFR_EDGE_TH_1                   : 8;
		uint32_t PFR_EDGE_TH_2                   : 8;
		uint32_t PFR_EDGE_TH_3                   : 8;
	} bits;
};

union REG_ISP_PFR_FCS_SET {
	uint32_t raw;
	struct {
		uint32_t FCS_EDGE_TH                     : 7;
		uint32_t FCS_KERNEL_SZ                   : 1;
		uint32_t FCS_BLEND                       : 8;
	} bits;
};

union REG_ISP_PFR_SHADOW_RD_SEL {
	uint32_t raw;
	struct {
		uint32_t SHDW_READ_SEL                   : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_CCM_00 {
	uint32_t raw;
	struct {
		uint32_t CCM_00                          : 14;
	} bits;
};

union REG_ISP_CCM_01 {
	uint32_t raw;
	struct {
		uint32_t CCM_01                          : 14;
	} bits;
};

union REG_ISP_CCM_02 {
	uint32_t raw;
	struct {
		uint32_t CCM_02                          : 14;
	} bits;
};

union REG_ISP_CCM_10 {
	uint32_t raw;
	struct {
		uint32_t CCM_10                          : 14;
	} bits;
};

union REG_ISP_CCM_11 {
	uint32_t raw;
	struct {
		uint32_t CCM_11                          : 14;
	} bits;
};

union REG_ISP_CCM_12 {
	uint32_t raw;
	struct {
		uint32_t CCM_12                          : 14;
	} bits;
};

union REG_ISP_CCM_20 {
	uint32_t raw;
	struct {
		uint32_t CCM_20                          : 14;
	} bits;
};

union REG_ISP_CCM_21 {
	uint32_t raw;
	struct {
		uint32_t CCM_21                          : 14;
	} bits;
};

union REG_ISP_CCM_22 {
	uint32_t raw;
	struct {
		uint32_t CCM_22                          : 14;
	} bits;
};

union REG_ISP_CCM_CTRL {
	uint32_t raw;
	struct {
		uint32_t CCM_SHDW_SEL                    : 1;
		uint32_t _rsv_1                          : 1;
		uint32_t CCM_ENABLE                      : 1;
	} bits;
};

union REG_ISP_CCM_DBG {
	uint32_t raw;
	struct {
		uint32_t FORCE_CLK_ENABLE                : 1;
		uint32_t SOFTRST                         : 1;
	} bits;
};

union REG_ISP_CCM_DMY0 {
	uint32_t raw;
	struct {
		uint32_t DMY_DEF0                        : 32;
	} bits;
};

union REG_ISP_CCM_DMY1 {
	uint32_t raw;
	struct {
		uint32_t DMY_DEF1                        : 32;
	} bits;
};

union REG_ISP_CCM_DMY_R {
	uint32_t raw;
	struct {
		uint32_t DMY_RO                          : 32;
	} bits;
};

union REG_ISP_CCM_OVEREXP_STR {
	uint32_t raw;
	struct {
		uint32_t CCM_OVEREXP_STR                 : 8;
	} bits;
};

union REG_ISP_CCM_OVEREXP_THR {
	uint32_t raw;
	struct {
		uint32_t CCM_OVEREXP_THR                 : 8;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_GAMMA_CTRL {
	uint32_t raw;
	struct {
		uint32_t GAMMA_ENABLE                    : 1;
		uint32_t GAMMA_SHDW_SEL                  : 1;
		uint32_t FORCE_CLK_ENABLE                : 1;
	} bits;
};

union REG_ISP_GAMMA_PROG_CTRL {
	uint32_t raw;
	struct {
		uint32_t GAMMA_WSEL                      : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t GAMMA_RSEL                      : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t GAMMA_PROG_EN                   : 1;
		uint32_t _rsv_9                          : 3;
		uint32_t GAMMA_PROG_1TO3_EN              : 1;
		uint32_t _rsv_13                         : 3;
		uint32_t GAMMA_PROG_MODE                 : 2;
	} bits;
};

union REG_ISP_GAMMA_PROG_ST_ADDR {
	uint32_t raw;
	struct {
		uint32_t GAMMA_ST_ADDR                   : 8;
		uint32_t _rsv_8                          : 23;
		uint32_t GAMMA_ST_W                      : 1;
	} bits;
};

union REG_ISP_GAMMA_PROG_DATA {
	uint32_t raw;
	struct {
		uint32_t GAMMA_DATA_E                    : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t GAMMA_DATA_O                    : 12;
		uint32_t _rsv_28                         : 3;
		uint32_t GAMMA_W                         : 1;
	} bits;
};

union REG_ISP_GAMMA_PROG_MAX {
	uint32_t raw;
	struct {
		uint32_t GAMMA_MAX                       : 13;
	} bits;
};

union REG_ISP_GAMMA_MEM_SW_RADDR {
	uint32_t raw;
	struct {
		uint32_t GAMMA_SW_RADDR                  : 8;
		uint32_t _rsv_8                          : 4;
		uint32_t GAMMA_SW_R_MEM_SEL              : 1;
	} bits;
};

union REG_ISP_GAMMA_MEM_SW_RDATA {
	uint32_t raw;
	struct {
		uint32_t GAMMA_RDATA_R                   : 12;
		uint32_t _rsv_12                         : 19;
		uint32_t GAMMA_SW_R                      : 1;
	} bits;
};

union REG_ISP_GAMMA_MEM_SW_RDATA_BG {
	uint32_t raw;
	struct {
		uint32_t GAMMA_RDATA_G                   : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t GAMMA_RDATA_B                   : 12;
	} bits;
};

union REG_ISP_GAMMA_DBG {
	uint32_t raw;
	struct {
		uint32_t PROG_HDK_DIS                    : 1;
		uint32_t SOFTRST                         : 1;
	} bits;
};

union REG_ISP_GAMMA_DMY0 {
	uint32_t raw;
	struct {
		uint32_t DMY_DEF0                        : 32;
	} bits;
};

union REG_ISP_GAMMA_DMY1 {
	uint32_t raw;
	struct {
		uint32_t DMY_DEF1                        : 32;
	} bits;
};

union REG_ISP_GAMMA_DMY_R {
	uint32_t raw;
	struct {
		uint32_t DMY_RO                          : 32;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_DEHAZE_DHZ_SMOOTH {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_W                        : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TH_SMOOTH                : 10;
	} bits;
};

union REG_ISP_DEHAZE_DHZ_SKIN {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_SKIN_CB                  : 8;
		uint32_t DEHAZE_SKIN_CR                  : 8;
	} bits;
};

union REG_ISP_DEHAZE_DHZ_WGT {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_A_LUMA_WGT               : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t DEHAZE_BLEND_WGT                : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t DEHAZE_TMAP_SCALE               : 8;
		uint32_t DEHAZE_D_WGT                    : 5;
	} bits;
};

union REG_ISP_DEHAZE_DHZ_BYPASS {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_ENABLE                   : 1;
		uint32_t DEHAZE_LUMA_LUT_ENABLE          : 1;
		uint32_t DEHAZE_SKIN_LUT_ENABLE          : 1;
		uint32_t DEHAZE_SHDW_SEL                 : 1;
		uint32_t FORCE_CLK_ENABLE                : 1;
	} bits;
};

union REG_ISP_DEHAZE_0 {
	uint32_t raw;
	struct {
		uint32_t SOFTRST                         : 1;
		uint32_t _rsv_1                          : 4;
		uint32_t DBG_EN                          : 1;
		uint32_t _rsv_6                          : 10;
		uint32_t CHECK_SUM                       : 16;
	} bits;
};

union REG_ISP_DEHAZE_1 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_CUM_TH                   : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t DEHAZE_HIST_TH                  : 14;
	} bits;
};

union REG_ISP_DEHAZE_2 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_SW_DC_TH                 : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t DEHAZE_SW_AGLOBAL_R             : 12;
		uint32_t DEHAZE_SW_DC_TRIG               : 1;
		uint32_t _rsv_29                         : 2;
		uint32_t DEHAZE_SW_DC_AGLOBAL_TRIG       : 1;
	} bits;
};

union REG_ISP_DEHAZE_28 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_SW_AGLOBAL_G             : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DEHAZE_SW_AGLOBAL_B             : 12;
	} bits;
};

union REG_ISP_DEHAZE_2C {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_AGLOBAL_MAX              : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DEHAZE_AGLOBAL_MIN              : 12;
	} bits;
};

union REG_ISP_DEHAZE_3 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_MIN                 : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t DEHAZE_TMAP_MAX                 : 13;
	} bits;
};

union REG_ISP_DEHAZE_5 {
	uint32_t raw;
	struct {
		uint32_t FMT_ST                          : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t FMT_END                         : 12;
		uint32_t TILE_NM                         : 4;
	} bits;
};

union REG_ISP_DEHAZE_6 {
	uint32_t raw;
	struct {
		uint32_t DBG_SEL                         : 3;
	} bits;
};

union REG_ISP_DEHAZE_7 {
	uint32_t raw;
	struct {
		uint32_t DHZ_DBG0                        : 32;
	} bits;
};

union REG_ISP_DEHAZE_8 {
	uint32_t raw;
	struct {
		uint32_t DHZ_DBG1                        : 32;
	} bits;
};

union REG_ISP_DEHAZE_9 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_LUMA_LUT00               : 8;
		uint32_t DEHAZE_LUMA_LUT01               : 8;
		uint32_t DEHAZE_LUMA_LUT02               : 8;
		uint32_t DEHAZE_LUMA_LUT03               : 8;
	} bits;
};

union REG_ISP_DEHAZE_10 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_LUMA_LUT04               : 8;
		uint32_t DEHAZE_LUMA_LUT05               : 8;
		uint32_t DEHAZE_LUMA_LUT06               : 8;
		uint32_t DEHAZE_LUMA_LUT07               : 8;
	} bits;
};

union REG_ISP_DEHAZE_11 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_LUMA_LUT08               : 8;
		uint32_t DEHAZE_LUMA_LUT09               : 8;
		uint32_t DEHAZE_LUMA_LUT10               : 8;
		uint32_t DEHAZE_LUMA_LUT11               : 8;
	} bits;
};

union REG_ISP_DEHAZE_12 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_LUMA_LUT12               : 8;
		uint32_t DEHAZE_LUMA_LUT13               : 8;
		uint32_t DEHAZE_LUMA_LUT14               : 8;
		uint32_t DEHAZE_LUMA_LUT15               : 8;
	} bits;
};

union REG_ISP_DEHAZE_17 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_SKIN_LUT00               : 8;
		uint32_t DEHAZE_SKIN_LUT01               : 8;
		uint32_t DEHAZE_SKIN_LUT02               : 8;
		uint32_t DEHAZE_SKIN_LUT03               : 8;
	} bits;
};

union REG_ISP_DEHAZE_18 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_SKIN_LUT04               : 8;
		uint32_t DEHAZE_SKIN_LUT05               : 8;
		uint32_t DEHAZE_SKIN_LUT06               : 8;
		uint32_t DEHAZE_SKIN_LUT07               : 8;
	} bits;
};

union REG_ISP_DEHAZE_19 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_SKIN_LUT08               : 8;
		uint32_t DEHAZE_SKIN_LUT09               : 8;
		uint32_t DEHAZE_SKIN_LUT10               : 8;
		uint32_t DEHAZE_SKIN_LUT11               : 8;
	} bits;
};

union REG_ISP_DEHAZE_20 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_SKIN_LUT12               : 8;
		uint32_t DEHAZE_SKIN_LUT13               : 8;
		uint32_t DEHAZE_SKIN_LUT14               : 8;
		uint32_t DEHAZE_SKIN_LUT15               : 8;
	} bits;
};

union REG_ISP_DEHAZE_25 {
	uint32_t raw;
	struct {
		uint32_t AGLOBAL_R                       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t AGLOBAL_G                       : 12;
	} bits;
};

union REG_ISP_DEHAZE_26 {
	uint32_t raw;
	struct {
		uint32_t AGLOBAL_B                       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t DC_TH                           : 10;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_00 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT000         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT001         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT002         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT003         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_01 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT004         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT005         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT006         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT007         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_02 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT008         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT009         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT010         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT011         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_03 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT012         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT013         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT014         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT015         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_04 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT016         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT017         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT018         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT019         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_05 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT020         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT021         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT022         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT023         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_06 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT024         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT025         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT026         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT027         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_07 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT028         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT029         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT030         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT031         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_08 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT032         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT033         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT034         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT035         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_09 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT036         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT037         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT038         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT039         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_10 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT040         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT041         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT042         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT043         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_11 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT044         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT045         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT046         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT047         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_12 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT048         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT049         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT050         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT051         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_13 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT052         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT053         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT054         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT055         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_14 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT056         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT057         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT058         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT059         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_15 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT060         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT061         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT062         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT063         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_16 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT064         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT065         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT066         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT067         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_17 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT068         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT069         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT070         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT071         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_18 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT072         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT073         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT074         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT075         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_19 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT076         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT077         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT078         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT079         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_20 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT080         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT081         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT082         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT083         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_21 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT084         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT085         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT086         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT087         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_22 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT088         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT089         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT090         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT091         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_23 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT092         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT093         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT094         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT095         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_24 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT096         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT097         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT098         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT099         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_25 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT100         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT101         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT102         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT103         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_26 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT104         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT105         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT106         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT107         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_27 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT108         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT109         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT110         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT111         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_28 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT112         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT113         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT114         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT115         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_29 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT116         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT117         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT118         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT119         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_30 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT120         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT121         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT122         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT123         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_31 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT124         : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT125         : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT126         : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t DEHAZE_TMAP_GAIN_LUT127         : 7;
	} bits;
};

union REG_ISP_DEHAZE_TMAP_32 {
	uint32_t raw;
	struct {
		uint32_t DEHAZE_TMAP_GAIN_LUT128         : 7;
	} bits;
};

union REG_ISP_DEHAZE_O_DC_TH_UP_MODE {
	uint32_t raw;
	struct {
		uint32_t O_DC_TH_UP_MODE                 : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_RGB_DITHER_RGB_DITHER {
	uint32_t raw;
	struct {
		uint32_t RGB_DITHER_ENABLE               : 1;
		uint32_t RGB_DITHER_MOD_EN               : 1;
		uint32_t RGB_DITHER_HISTIDX_EN           : 1;
		uint32_t RGB_DITHER_FMNUM_EN             : 1;
		uint32_t RGB_DITHER_SHDW_SEL             : 1;
		uint32_t RGB_DITHER_SOFTRST              : 1;
		uint32_t _rsv_6                          : 2;
		uint32_t CROP_WIDTHM1                    : 12;
		uint32_t CROP_HEIGHTM1                   : 12;
	} bits;
};

union REG_ISP_RGB_DITHER_RGB_DITHER_DEBUG0 {
	uint32_t raw;
	struct {
		uint32_t RGB_DITHER_DEBUG0               : 32;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_CLUT_CTRL {
	uint32_t raw;
	struct {
		uint32_t CLUT_ENABLE                     : 1;
		uint32_t CLUT_SHDW_SEL                   : 1;
		uint32_t FORCE_CLK_ENABLE                : 1;
		uint32_t PROG_EN                         : 1;
	} bits;
};

union REG_ISP_CLUT_PROG_ADDR {
	uint32_t raw;
	struct {
		uint32_t SRAM_R_IDX                      : 5;
		uint32_t _rsv_5                          : 3;
		uint32_t SRAM_G_IDX                      : 5;
		uint32_t _rsv_13                         : 3;
		uint32_t SRAM_B_IDX                      : 5;
	} bits;
};

union REG_ISP_CLUT_PROG_DATA {
	uint32_t raw;
	struct {
		uint32_t SRAM_WDATA                      : 30;
		uint32_t _rsv_30                         : 1;
		uint32_t SRAM_WR                         : 1;
	} bits;
};

union REG_ISP_CLUT_PROG_RDATA {
	uint32_t raw;
	struct {
		uint32_t SRAM_RDATA                      : 30;
		uint32_t _rsv_30                         : 1;
		uint32_t SRAM_RD                         : 1;
	} bits;
};

union REG_ISP_CLUT_DBG {
	uint32_t raw;
	struct {
		uint32_t PROG_HDK_DIS                    : 1;
	} bits;
};

union REG_ISP_CLUT_DMY0 {
	uint32_t raw;
	struct {
		uint32_t DMY_DEF0                        : 32;
	} bits;
};

union REG_ISP_CLUT_DMY1 {
	uint32_t raw;
	struct {
		uint32_t DMY_DEF1                        : 32;
	} bits;
};

union REG_ISP_CLUT_DMY_R {
	uint32_t raw;
	struct {
		uint32_t DMY_RO                          : 32;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_CSC_0 {
	uint32_t raw;
	struct {
		uint32_t CSC_ENABLE                      : 1;
		uint32_t R2Y4_SHDW_SEL                   : 1;
	} bits;
};

union REG_ISP_CSC_1 {
	uint32_t raw;
	struct {
		uint32_t OP_START                        : 1;
		uint32_t CONT_EN                         : 1;
		uint32_t R2Y4_BYPASS                     : 1;
		uint32_t SOFTRST                         : 1;
		uint32_t AUTO_UPDATE_EN                  : 1;
		uint32_t DBG_EN                          : 1;
		uint32_t _rsv_6                          : 10;
		uint32_t CHECK_SUM                       : 16;
	} bits;
};

union REG_ISP_CSC_2 {
	uint32_t raw;
	struct {
		uint32_t SHDW_UPDATE_REQ                 : 1;
	} bits;
};

union REG_ISP_CSC_3 {
	uint32_t raw;
	struct {
		uint32_t DMY0                            : 32;
	} bits;
};

union REG_ISP_CSC_4 {
	uint32_t raw;
	struct {
		uint32_t COEFF_00                        : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t COEFF_01                        : 14;
	} bits;
};

union REG_ISP_CSC_5 {
	uint32_t raw;
	struct {
		uint32_t COEFF_02                        : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t COEFF_10                        : 14;
	} bits;
};

union REG_ISP_CSC_6 {
	uint32_t raw;
	struct {
		uint32_t COEFF_11                        : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t COEFF_12                        : 14;
	} bits;
};

union REG_ISP_CSC_7 {
	uint32_t raw;
	struct {
		uint32_t COEFF_20                        : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t COEFF_21                        : 14;
	} bits;
};

union REG_ISP_CSC_8 {
	uint32_t raw;
	struct {
		uint32_t COEFF_22                        : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t OFFSET_0                        : 11;
	} bits;
};

union REG_ISP_CSC_9 {
	uint32_t raw;
	struct {
		uint32_t OFFSET_1                        : 11;
		uint32_t _rsv_11                         : 5;
		uint32_t OFFSET_2                        : 11;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_YUV_TOP_YUV_0 {
	uint32_t raw;
	struct {
		uint32_t _rsv_0                          : 8;
		uint32_t YUV_TOP_SEL                     : 1;
	} bits;
};

union REG_YUV_TOP_YUV_2 {
	uint32_t raw;
	struct {
		uint32_t FD_INT                          : 1;
		uint32_t DMA_INT                         : 1;
		uint32_t FRAME_OVERFLOW                  : 1;
	} bits;
};

union REG_YUV_TOP_YUV_3 {
	uint32_t raw;
	struct {
		uint32_t _rsv_0                          : 2;
		uint32_t YONLY_EN                        : 1;
	} bits;
};

union REG_YUV_TOP_YUV_DEBUG_0 {
	uint32_t raw;
	struct {
		uint32_t DEBUG_BUS                       : 32;
	} bits;
};

union REG_YUV_TOP_YUV_4 {
	uint32_t raw;
	struct {
		uint32_t DUMMY                           : 32;
	} bits;
};

union REG_YUV_TOP_YUV_DEBUG_STATE {
	uint32_t raw;
	struct {
		uint32_t MA_IDLE                         : 24;
		uint32_t _rsv_24                         : 7;
		uint32_t IDLE                            : 1;
	} bits;
};

union REG_YUV_TOP_YUV_5 {
	uint32_t raw;
	struct {
		uint32_t DIS_UV2DRAM                     : 1;
		uint32_t LINE_THRES_EN                   : 1;
		uint32_t _rsv_2                          : 6;
		uint32_t LINE_THRES                      : 14;
		uint32_t _rsv_22                         : 2;
		uint32_t PG2_ENABLE                      : 1;
	} bits;
};

union REG_YUV_TOP_YUV_CTRL {
	uint32_t raw;
	struct {
		uint32_t CHECKSUM_ENABLE                 : 1;
		uint32_t SC_DMA_SWITCH                   : 1;
		uint32_t _rsv_2                          : 5;
		uint32_t CURSER2_EN                      : 1;
		uint32_t GUARD_CNT                       : 8;
	} bits;
};

union REG_YUV_TOP_IMGW_M1 {
	uint32_t raw;
	struct {
		uint32_t YUV_TOP_IMGW_M1                 : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t YUV_TOP_IMGH_M1                 : 14;
	} bits;
};

union REG_YUV_TOP_STVALID_STATUS {
	uint32_t raw;
	struct {
		uint32_t STVALID_STATUS                  : 32;
	} bits;
};

union REG_YUV_TOP_STREADY_STATUS {
	uint32_t raw;
	struct {
		uint32_t STREADY_STATUS                  : 32;
	} bits;
};

union REG_YUV_TOP_PATGEN1 {
	uint32_t raw;
	struct {
		uint32_t X_CURSER                        : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t Y_CURSER                        : 14;
		uint32_t CURSER_EN                       : 1;
		uint32_t PG_ENABLE                       : 1;
	} bits;
};

union REG_YUV_TOP_PATGEN2 {
	uint32_t raw;
	struct {
		uint32_t CURSER_VALUE                    : 16;
	} bits;
};

union REG_YUV_TOP_PATGEN3 {
	uint32_t raw;
	struct {
		uint32_t VALUE_REPORT                    : 32;
	} bits;
};

union REG_YUV_TOP_PATGEN4 {
	uint32_t raw;
	struct {
		uint32_t XCNT_RPT                        : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t YCNT_RPT                        : 14;
	} bits;
};

union REG_YUV_TOP_CHECK_SUM {
	uint32_t raw;
	struct {
		uint32_t K_SUM                           : 32;
	} bits;
};

union REG_YUV_TOP_AI_ISP_RDMA_CTRL {
	uint32_t raw;
	struct {
		uint32_t AI_ISP_RDMA_ENABLE              : 3;
		uint32_t AI_ISP_CROP_ENABLE              : 1;
		uint32_t AI_ISP_ENABLE                   : 1;
		uint32_t AI_ISP_MASK                     : 1;
	} bits;
};

union REG_YUV_TOP_AI_ISP_IMG_SIZE_Y {
	uint32_t raw;
	struct {
		uint32_t AI_ISP_IMG_WIDTH_CROP_0         : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t AI_ISP_IMG_HEIGHT_CROP_0        : 14;
	} bits;
};

union REG_YUV_TOP_AI_ISP_W_CROP_Y {
	uint32_t raw;
	struct {
		uint32_t AI_ISP_CROP_W_STR_0             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t AI_ISP_CROP_W_END_0             : 14;
	} bits;
};

union REG_YUV_TOP_AI_ISP_H_CROP_Y {
	uint32_t raw;
	struct {
		uint32_t AI_ISP_CROP_H_STR_0             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t AI_ISP_CROP_H_END_0             : 14;
	} bits;
};

union REG_YUV_TOP_AI_ISP_IMG_SIZE_UV {
	uint32_t raw;
	struct {
		uint32_t AI_ISP_IMG_WIDTH_CROP_1         : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t AI_ISP_IMG_HEIGHT_CROP_1        : 14;
	} bits;
};

union REG_YUV_TOP_AI_ISP_W_CROP_UV {
	uint32_t raw;
	struct {
		uint32_t AI_ISP_CROP_W_STR_1             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t AI_ISP_CROP_W_END_1             : 14;
	} bits;
};

union REG_YUV_TOP_AI_ISP_H_CROP_UV {
	uint32_t raw;
	struct {
		uint32_t AI_ISP_CROP_H_STR_1             : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t AI_ISP_CROP_H_END_1             : 14;
	} bits;
};

union REG_YUV_TOP_YUV_CTRL_PHOBOS3 {
	uint32_t raw;
	struct {
		uint32_t BYPASS_H_0                      : 1;
		uint32_t AVG_MODE_0                      : 1;
		uint32_t NODELAY_DUPLICATE_MODE_0        : 1;
		uint32_t SCALE_DISABLE_0                 : 1;
		uint32_t REG_422_444_EN_0                : 1;
		uint32_t SWAP_EN_0                       : 2;
		uint32_t _rsv_7                          : 4;
		uint32_t BYPASS_H_1                      : 1;
		uint32_t AVG_MODE_2                      : 1;
		uint32_t NODELAY_DUPLICATE_MODE_2        : 1;
		uint32_t SCALE_DISABLE_2                 : 1;
		uint32_t BYPASS_V_1                      : 1;
		uint32_t AVG_MODE_3                      : 1;
		uint32_t DROP_MODE                       : 1;
		uint32_t BYPASS_H_2                      : 1;
		uint32_t BYPASS_V_2                      : 1;
		uint32_t AVG_MODE_4                      : 1;
		uint32_t NODELAY_DUPLICATE_MODE_3        : 1;
		uint32_t SCALE_DISABLE_3                 : 1;
		uint32_t REG_422_444_DITHER              : 1;
		uint32_t YV_SWAP_EN                      : 1;
		uint32_t UV_SWAP_EN                      : 1;
		uint32_t YU_SWAP_EN                      : 1;
		uint32_t MCTF_UV_SWAP_EN                 : 1;
		uint32_t HW_AUTO_CG_EN                   : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_CA_00 {
	uint32_t raw;
	struct {
		uint32_t CACP_ENABLE                     : 1;
		uint32_t CACP_MODE                       : 1;
		uint32_t CACP_DBG_MODE                   : 1;
		uint32_t CACP_MEM_SW_MODE                : 1;
		uint32_t CACP_SHDW_READ_SEL              : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t PROG_HDK_DIS                    : 1;
		uint32_t _rsv_9                          : 7;
		uint32_t CACP_ISO_RATIO                  : 11;
	} bits;
};

union REG_ISP_CA_04 {
	uint32_t raw;
	struct {
		uint32_t CACP_MEM_D                      : 24;
		uint32_t _rsv_24                         : 7;
		uint32_t CACP_MEM_W                      : 1;
	} bits;
};

union REG_ISP_CA_08 {
	uint32_t raw;
	struct {
		uint32_t CACP_MEM_ST_ADDR                : 8;
		uint32_t _rsv_8                          : 23;
		uint32_t CACP_MEM_ST_ADDR_W              : 1;
	} bits;
};

union REG_ISP_CA_0C {
	uint32_t raw;
	struct {
		uint32_t CACP_MEM_SW_RADDR               : 8;
		uint32_t _rsv_8                          : 23;
		uint32_t CACP_MEM_SW_R                   : 1;
	} bits;
};

union REG_ISP_CA_10 {
	uint32_t raw;
	struct {
		uint32_t CACP_MEM_SW_RDATA_R             : 24;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_CA_LITE_00 {
	uint32_t raw;
	struct {
		uint32_t CA_LITE_ENABLE                  : 1;
		uint32_t CA_LITE_SHDW_READ_SEL           : 1;
	} bits;
};

union REG_ISP_CA_LITE_04 {
	uint32_t raw;
	struct {
		uint32_t CA_LITE_LUT_IN_0                : 9;
		uint32_t _rsv_9                          : 7;
		uint32_t CA_LITE_LUT_IN_1                : 9;
	} bits;
};

union REG_ISP_CA_LITE_08 {
	uint32_t raw;
	struct {
		uint32_t CA_LITE_LUT_IN_2                : 9;
		uint32_t _rsv_9                          : 7;
		uint32_t CA_LITE_LUT_IN_3                : 9;
	} bits;
};

union REG_ISP_CA_LITE_0C {
	uint32_t raw;
	struct {
		uint32_t CA_LITE_LUT_IN_4                : 9;
		uint32_t _rsv_9                          : 7;
		uint32_t CA_LITE_LUT_IN_5                : 9;
	} bits;
};

union REG_ISP_CA_LITE_10 {
	uint32_t raw;
	struct {
		uint32_t CA_LITE_LUT_OUT_0               : 11;
		uint32_t _rsv_11                         : 5;
		uint32_t CA_LITE_LUT_OUT_1               : 11;
	} bits;
};

union REG_ISP_CA_LITE_14 {
	uint32_t raw;
	struct {
		uint32_t CA_LITE_LUT_OUT_2               : 11;
		uint32_t _rsv_11                         : 5;
		uint32_t CA_LITE_LUT_OUT_3               : 11;
	} bits;
};

union REG_ISP_CA_LITE_18 {
	uint32_t raw;
	struct {
		uint32_t CA_LITE_LUT_OUT_4               : 11;
		uint32_t _rsv_11                         : 5;
		uint32_t CA_LITE_LUT_OUT_5               : 11;
	} bits;
};

union REG_ISP_CA_LITE_1C {
	uint32_t raw;
	struct {
		uint32_t CA_LITE_LUT_SLP_0               : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t CA_LITE_LUT_SLP_1               : 12;
	} bits;
};

union REG_ISP_CA_LITE_20 {
	uint32_t raw;
	struct {
		uint32_t CA_LITE_LUT_SLP_2               : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t CA_LITE_LUT_SLP_3               : 12;
	} bits;
};

union REG_ISP_CA_LITE_24 {
	uint32_t raw;
	struct {
		uint32_t CA_LITE_LUT_SLP_4               : 12;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_EE_ADD_REG0 {
	uint32_t raw;
	struct {
		uint32_t EE_ENABLE                       : 1;
		uint32_t EE_DEBUG_MODE                   : 4;
		uint32_t HW_AUTO_CG_EN                   : 1;
		uint32_t EE_EXT_BYPASS_EN                : 1;
	} bits;
};

union REG_ISP_EE_ADD_OVERSHOOT_0 {
	uint32_t raw;
	struct {
		uint32_t EE_TH_OVERSHOOT                 : 8;
		uint32_t EE_TH_UNDERSHOOT_LUM            : 8;
		uint32_t EE_TH_UNDERSHOOT_ENG            : 8;
		uint32_t EE_TH_UNDERSHOOT                : 8;
	} bits;
};

union REG_ISP_EE_ADD_OVERSHOOT_1 {
	uint32_t raw;
	struct {
		uint32_t EE_OVER_UNDER_SHOOT_ENABLE      : 1;
		uint32_t EE_WT_OVERSHOOT                 : 9;
		uint32_t EE_WT_UNDERSHOOT                : 9;
	} bits;
};

union REG_ISP_EE_ADD_OVERSHOOT_2 {
	uint32_t raw;
	struct {
		uint32_t EE_SLOPE_OVERSHOOT              : 15;
		uint32_t EE_SLOPE_UNDERSHOOT             : 15;
	} bits;
};

union REG_ISP_EE_ADD_OVERSHOOT_3 {
	uint32_t raw;
	struct {
		uint32_t EE_SLOPE_UNDERSHOOT_LUM         : 15;
		uint32_t EE_SLOPE_UNDERSHOOT_ENG         : 15;
	} bits;
};

union REG_ISP_EE_ADD_OVERSHOOT_4 {
	uint32_t raw;
	struct {
		uint32_t EE_USHOOT_CLAMPWT_LUM           : 8;
		uint32_t EE_USHOOT_CLAMPWT_ENG           : 8;
		uint32_t EE_USHOOT_STR_LUM_ENG           : 8;
		uint32_t EE_USHOOT_NORM_LUM_ENG          : 5;
	} bits;
};

union REG_ISP_EE_ADD_REFINE_0 {
	uint32_t raw;
	struct {
		uint32_t EE_REFINE_BY_LUM_TH_LOW         : 10;
		uint32_t EE_REFINE_BY_LUM_TH_HIGH        : 10;
		uint32_t EE_REFINE_BY_LUM_STEP_LOW       : 3;
		uint32_t EE_REFINE_BY_LUM_STEP_HIGH      : 3;
	} bits;
};

union REG_ISP_EE_ADD_REFINE_1 {
	uint32_t raw;
	struct {
		uint32_t EE_REFINE_EDGE_TYPE_SEL         : 2;
		uint32_t EE_REFINE_BY_EDGE_TH_LOW        : 10;
		uint32_t EE_REFINE_BY_EDGE_TH_HIGH       : 10;
		uint32_t EE_REFINE_BY_EDGE_STEP_LOW      : 3;
		uint32_t EE_REFINE_BY_EDGE_STEP_HIGH     : 3;
	} bits;
};

union REG_ISP_EE_ADD_REFINE_2 {
	uint32_t raw;
	struct {
		uint32_t EE_REFINE_BY_LUM_LUT_LOW_U8_0   : 8;
		uint32_t EE_REFINE_BY_LUM_LUT_LOW_U8_1   : 8;
		uint32_t EE_REFINE_BY_LUM_LUT_LOW_U8_2   : 8;
		uint32_t EE_REFINE_BY_LUM_LUT_LOW_U8_3   : 8;
	} bits;
};

union REG_ISP_EE_ADD_REFINE_3 {
	uint32_t raw;
	struct {
		uint32_t EE_REFINE_BY_LUM_LUT_LOW_U8_4   : 8;
		uint32_t EE_REFINE_BY_LUM_LUT_LOW_U8_5   : 8;
		uint32_t EE_REFINE_BY_LUM_LUT_LOW_U8_6   : 8;
		uint32_t EE_REFINE_BY_LUM_LUT_LOW_U8_7   : 8;
	} bits;
};

union REG_ISP_EE_ADD_REFINE_4 {
	uint32_t raw;
	struct {
		uint32_t EE_REFINE_BY_LUM_LUT_HIGH_U8_0  : 8;
		uint32_t EE_REFINE_BY_LUM_LUT_HIGH_U8_1  : 8;
		uint32_t EE_REFINE_BY_LUM_LUT_HIGH_U8_2  : 8;
		uint32_t EE_REFINE_BY_LUM_LUT_HIGH_U8_3  : 8;
	} bits;
};

union REG_ISP_EE_ADD_REFINE_5 {
	uint32_t raw;
	struct {
		uint32_t EE_REFINE_BY_LUM_LUT_HIGH_U8_4  : 8;
		uint32_t EE_REFINE_BY_LUM_LUT_HIGH_U8_5  : 8;
		uint32_t EE_REFINE_BY_LUM_LUT_HIGH_U8_6  : 8;
		uint32_t EE_REFINE_BY_LUM_LUT_HIGH_U8_7  : 8;
	} bits;
};

union REG_ISP_EE_ADD_REFINE_6 {
	uint32_t raw;
	struct {
		uint32_t EE_REFINE_BY_EDGE_LUT_LOW_U8_0  : 8;
		uint32_t EE_REFINE_BY_EDGE_LUT_LOW_U8_1  : 8;
		uint32_t EE_REFINE_BY_EDGE_LUT_LOW_U8_2  : 8;
		uint32_t EE_REFINE_BY_EDGE_LUT_LOW_U8_3  : 8;
	} bits;
};

union REG_ISP_EE_ADD_REFINE_7 {
	uint32_t raw;
	struct {
		uint32_t EE_REFINE_BY_EDGE_LUT_LOW_U8_4  : 8;
		uint32_t EE_REFINE_BY_EDGE_LUT_LOW_U8_5  : 8;
		uint32_t EE_REFINE_BY_EDGE_LUT_LOW_U8_6  : 8;
		uint32_t EE_REFINE_BY_EDGE_LUT_LOW_U8_7  : 8;
	} bits;
};

union REG_ISP_EE_ADD_REFINE_8 {
	uint32_t raw;
	struct {
		uint32_t EE_REFINE_BY_EDGE_LUT_HIGH_U8_0 : 8;
		uint32_t EE_REFINE_BY_EDGE_LUT_HIGH_U8_1 : 8;
		uint32_t EE_REFINE_BY_EDGE_LUT_HIGH_U8_2 : 8;
		uint32_t EE_REFINE_BY_EDGE_LUT_HIGH_U8_3 : 8;
	} bits;
};

union REG_ISP_EE_ADD_REFINE_9 {
	uint32_t raw;
	struct {
		uint32_t EE_REFINE_BY_EDGE_LUT_HIGH_U8_4 : 8;
		uint32_t EE_REFINE_BY_EDGE_LUT_HIGH_U8_5 : 8;
		uint32_t EE_REFINE_BY_EDGE_LUT_HIGH_U8_6 : 8;
		uint32_t EE_REFINE_BY_EDGE_LUT_HIGH_U8_7 : 8;
	} bits;
};

union REG_ISP_EE_ADD_CHROMA_0 {
	uint32_t raw;
	struct {
		uint32_t EE_CHROMA_ADPTCTRL_LUT_00       : 6;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_01       : 6;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_02       : 6;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_03       : 6;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_04       : 6;
	} bits;
};

union REG_ISP_EE_ADD_CHROMA_1 {
	uint32_t raw;
	struct {
		uint32_t EE_CHROMA_ADPTCTRL_LUT_05       : 6;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_06       : 6;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_07       : 6;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_08       : 6;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_09       : 6;
	} bits;
};

union REG_ISP_EE_ADD_CHROMA_2 {
	uint32_t raw;
	struct {
		uint32_t EE_CHROMA_ADPTCTRL_LUT_10       : 6;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_11       : 6;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_12       : 6;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_13       : 6;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_14       : 6;
	} bits;
};

union REG_ISP_EE_ADD_CHROMA_3 {
	uint32_t raw;
	struct {
		uint32_t EE_CHROMA_ADPTCTRL_LUT_15       : 6;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_16       : 6;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_17       : 6;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_18       : 6;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_19       : 6;
	} bits;
};

union REG_ISP_EE_ADD_CHROMA_4 {
	uint32_t raw;
	struct {
		uint32_t EE_CHROMA_ADPTCTRL_LUT_20       : 6;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_21       : 6;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_22       : 6;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_23       : 6;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_24       : 6;
	} bits;
};

union REG_ISP_EE_ADD_CHROMA_5 {
	uint32_t raw;
	struct {
		uint32_t EE_CHROMA_ADPTCTRL_LUT_25       : 6;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_26       : 6;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_27       : 6;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_28       : 6;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_29       : 6;
	} bits;
};

union REG_ISP_EE_ADD_CHROMA_6 {
	uint32_t raw;
	struct {
		uint32_t EE_CHROMA_ADPTCTRL_LUT_30       : 6;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_31       : 6;
		uint32_t EE_CHROMA_ADPTCTRL_EN           : 1;
		uint32_t EE_CHROMA_AMP_LUT_SLOPE_0       : 12;
	} bits;
};

union REG_ISP_EE_ADD_CHROMA_7 {
	uint32_t raw;
	struct {
		uint32_t EE_CHROMA_AMP_LUT_SLOPE_1       : 12;
		uint32_t EE_CHROMA_AMP_LUT_SLOPE_2       : 12;
	} bits;
};

union REG_ISP_EE_ADD_CHROMA_8 {
	uint32_t raw;
	struct {
		uint32_t EE_CHROMA_AMP_LUT_IN_0          : 8;
		uint32_t EE_CHROMA_AMP_LUT_IN_1          : 8;
		uint32_t EE_CHROMA_AMP_LUT_IN_2          : 8;
		uint32_t EE_CHROMA_AMP_LUT_IN_3          : 8;
	} bits;
};

union REG_ISP_EE_ADD_CHROMA_9 {
	uint32_t raw;
	struct {
		uint32_t EE_CHROMA_AMP_LUT_OUT_0         : 8;
		uint32_t EE_CHROMA_AMP_LUT_OUT_1         : 8;
		uint32_t EE_CHROMA_AMP_LUT_OUT_2         : 8;
		uint32_t EE_CHROMA_AMP_LUT_OUT_3         : 8;
	} bits;
};

union REG_ISP_EE_ADD_SHADOW_RD_SEL {
	uint32_t raw;
	struct {
		uint32_t SHDW_READ_SEL                   : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_CNR_CTRL_HW_ONLY {
	uint32_t raw;
	struct {
		uint32_t CNR_ENABLE                      : 1;
		uint32_t HW_AUTO_CG_EN                   : 1;
		uint32_t TILE_MODE                       : 2;
		uint32_t _rsv_4                          : 4;
		uint32_t FIRST_FRAME_RESET               : 1;
	} bits;
};

union REG_ISP_CNR_MMP_SCL_IN_SIZE {
	uint32_t raw;
	struct {
		uint32_t MMP_SCL_IN_IMGW                 : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t MMP_SCL_IN_IMGH                 : 14;
	} bits;
};

union REG_ISP_CNR_SCL_DOWN_CTRL {
	uint32_t raw;
	struct {
		uint32_t SCL_DOWN_CTRL_EN                : 1;
		uint32_t SCL_DOWN_MODE                   : 2;
	} bits;
};

union REG_ISP_CNR_CTRL_SW_HW {
	uint32_t raw;
	struct {
		uint32_t CNR_CMF_EN                      : 1;
		uint32_t CNR_LCA_ENABLE                  : 1;
		uint32_t CNR_IFE2_FILTER_EN              : 1;
		uint32_t CNR_CHRA_EN                     : 1;
		uint32_t CNR_M_CHRA_BYPASS               : 1;
		uint32_t CNR_CHRA_DBGMODE                : 3;
		uint32_t CNR_CHRA_SAT_OUTBLD_EN          : 1;
	} bits;
};

union REG_ISP_CNR_SUBIM_OUTSEL {
	uint32_t raw;
	struct {
		uint32_t CNR_SUBIM_OUTSEL                : 1;
	} bits;
};

union REG_ISP_CNR_CMF_KSIZE {
	uint32_t raw;
	struct {
		uint32_t CNR_CMF_KSIZE                   : 2;
	} bits;
};

union REG_ISP_CNR_IFE2_FSIZE_SEL {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_FSIZE_SEL              : 2;
	} bits;
};

union REG_ISP_CNR_IFE2_EKSIZE_SEL {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_EKSIZE_SEL             : 2;
	} bits;
};

union REG_ISP_CNR_IFE2_Y_RCTH {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_Y_RCTH_0               : 8;
		uint32_t CNR_IFE2_Y_RCTH_1               : 8;
		uint32_t CNR_IFE2_Y_RCTH_2               : 8;
	} bits;
};

union REG_ISP_CNR_IFE2_Y_CWT {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_Y_CWT                  : 5;
	} bits;
};

union REG_ISP_CNR_IFE2_Y_RCWT {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_Y_RCWT_0               : 4;
		uint32_t CNR_IFE2_Y_RCWT_1               : 4;
		uint32_t CNR_IFE2_Y_RCWT_2               : 4;
		uint32_t CNR_IFE2_Y_RCWT_3               : 4;
	} bits;
};

union REG_ISP_CNR_IFE2_Y_OUTL_TH {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_Y_OUTL_TH              : 3;
	} bits;
};

union REG_ISP_CNR_IFE2_UV_RCTH {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_UV_RCTH_0              : 8;
		uint32_t CNR_IFE2_UV_RCTH_1              : 8;
		uint32_t CNR_IFE2_UV_RCTH_2              : 8;
	} bits;
};

union REG_ISP_CNR_IFE2_UV_CWT {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_UV_CWT                 : 5;
	} bits;
};

union REG_ISP_CNR_IFE2_UV_RCWT {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_UV_RCWT_0              : 4;
		uint32_t CNR_IFE2_UV_RCWT_1              : 4;
		uint32_t CNR_IFE2_UV_RCWT_2              : 4;
		uint32_t CNR_IFE2_UV_RCWT_3              : 4;
	} bits;
};

union REG_ISP_CNR_IFE2_UV_OUTL_TH {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_UV_OUTL_TH             : 3;
	} bits;
};

union REG_ISP_CNR_IFE2_Y_OUTL_DTH {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_Y_OUTL_DTH             : 8;
	} bits;
};

union REG_ISP_CNR_IFE2_U_OUTL_DTH {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_U_OUTL_DTH             : 8;
	} bits;
};

union REG_ISP_CNR_IFE2_V_OUTL_DTH {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_V_OUTL_DTH             : 8;
	} bits;
};

union REG_ISP_CNR_IFE2_ED_PN_TH {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_ED_PN_TH               : 8;
	} bits;
};

union REG_ISP_CNR_IFE2_ED_HV_TH {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_ED_HV_TH               : 8;
	} bits;
};

union REG_ISP_CNR_IFE2_Y_FTH {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_Y_FTH_0                : 8;
		uint32_t CNR_IFE2_Y_FTH_1                : 8;
		uint32_t CNR_IFE2_Y_FTH_2                : 8;
		uint32_t CNR_IFE2_Y_FTH_3                : 8;
	} bits;
};

union REG_ISP_CNR_IFE2_Y_FTH_4 {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_Y_FTH_4                : 8;
	} bits;
};

union REG_ISP_CNR_IFE2_Y_FWT {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_Y_FWT_0                : 5;
		uint32_t CNR_IFE2_Y_FWT_1                : 5;
		uint32_t CNR_IFE2_Y_FWT_2                : 5;
		uint32_t CNR_IFE2_Y_FWT_3                : 5;
		uint32_t CNR_IFE2_Y_FWT_4                : 5;
		uint32_t CNR_IFE2_Y_FWT_5                : 5;
	} bits;
};

union REG_ISP_CNR_IFE2_U_FTH {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_U_FTH_0                : 8;
		uint32_t CNR_IFE2_U_FTH_1                : 8;
		uint32_t CNR_IFE2_U_FTH_2                : 8;
		uint32_t CNR_IFE2_U_FTH_3                : 8;
	} bits;
};

union REG_ISP_CNR_IFE2_U_FTH_4 {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_U_FTH_4                : 8;
	} bits;
};

union REG_ISP_CNR_IFE2_U_FWT {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_U_FWT_0                : 5;
		uint32_t CNR_IFE2_U_FWT_1                : 5;
		uint32_t CNR_IFE2_U_FWT_2                : 5;
		uint32_t CNR_IFE2_U_FWT_3                : 5;
		uint32_t CNR_IFE2_U_FWT_4                : 5;
		uint32_t CNR_IFE2_U_FWT_5                : 5;
	} bits;
};

union REG_ISP_CNR_IFE2_V_FTH {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_V_FTH_0                : 8;
		uint32_t CNR_IFE2_V_FTH_1                : 8;
		uint32_t CNR_IFE2_V_FTH_2                : 8;
		uint32_t CNR_IFE2_V_FTH_3                : 8;
	} bits;
};

union REG_ISP_CNR_IFE2_V_FTH_4 {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_V_FTH_4                : 8;
	} bits;
};

union REG_ISP_CNR_IFE2_V_FWT {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_V_FWT_0                : 5;
		uint32_t CNR_IFE2_V_FWT_1                : 5;
		uint32_t CNR_IFE2_V_FWT_2                : 5;
		uint32_t CNR_IFE2_V_FWT_3                : 5;
		uint32_t CNR_IFE2_V_FWT_4                : 5;
		uint32_t CNR_IFE2_V_FWT_5                : 5;
	} bits;
};

union REG_ISP_CNR_IFE2_YFTR_EN {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_YFTR_EN                : 1;
	} bits;
};

union REG_ISP_CNR_IFE2_EGD_EN {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_EGD_EN                 : 1;
	} bits;
};

union REG_ISP_CNR_IFE2_RC_EN {
	uint32_t raw;
	struct {
		uint32_t CNR_IFE2_RC_EN                  : 1;
	} bits;
};

union REG_ISP_CNR_CHRA_REFY_WT {
	uint32_t raw;
	struct {
		uint32_t CNR_CHRA_REFY_WT                : 5;
	} bits;
};

union REG_ISP_CNR_CHRA_REFC_WT {
	uint32_t raw;
	struct {
		uint32_t CNR_CHRA_REFC_WT                : 5;
	} bits;
};

union REG_ISP_CNR_CHRA_OUT_WT_LUT {
	uint32_t raw;
	struct {
		uint32_t CNR_CHRA_OUT_WT_LUT_0           : 5;
		uint32_t _rsv_5                          : 3;
		uint32_t CNR_CHRA_OUT_WT_LUT_1           : 5;
		uint32_t _rsv_13                         : 3;
		uint32_t CNR_CHRA_OUT_WT_LUT_2           : 5;
	} bits;
};

union REG_ISP_CNR_CHRA_Y_RNG {
	uint32_t raw;
	struct {
		uint32_t CNR_CHRA_Y_RNG                  : 2;
	} bits;
};

union REG_ISP_CNR_CHRA_Y_WTPRC {
	uint32_t raw;
	struct {
		uint32_t CNR_CHRA_Y_WTPRC                : 2;
	} bits;
};

union REG_ISP_CNR_CHRA_Y_TH {
	uint32_t raw;
	struct {
		uint32_t CNR_CHRA_Y_TH                   : 5;
	} bits;
};

union REG_ISP_CNR_CHRA_Y_WTS {
	uint32_t raw;
	struct {
		uint32_t CNR_CHRA_Y_WTS                  : 6;
	} bits;
};

union REG_ISP_CNR_CHRA_Y_WTE {
	uint32_t raw;
	struct {
		uint32_t CNR_CHRA_Y_WTE                  : 6;
	} bits;
};

union REG_ISP_CNR_CHRA_UV_RNG {
	uint32_t raw;
	struct {
		uint32_t CNR_CHRA_UV_RNG                 : 3;
	} bits;
};

union REG_ISP_CNR_CHRA_UV_WTPRC {
	uint32_t raw;
	struct {
		uint32_t CNR_CHRA_UV_WTPRC               : 2;
	} bits;
};

union REG_ISP_CNR_CHRA_UV_TH {
	uint32_t raw;
	struct {
		uint32_t CNR_CHRA_UV_TH                  : 6;
	} bits;
};

union REG_ISP_CNR_CHRA_UV_WTS {
	uint32_t raw;
	struct {
		uint32_t CNR_CHRA_UV_WTS                 : 7;
	} bits;
};

union REG_ISP_CNR_CHRA_UV_WTE {
	uint32_t raw;
	struct {
		uint32_t CNR_CHRA_UV_WTE                 : 7;
	} bits;
};

union REG_ISP_CNR_CHRA_SAT_RNG {
	uint32_t raw;
	struct {
		uint32_t CNR_CHRA_SAT_RNG                : 3;
	} bits;
};

union REG_ISP_CNR_CHRA_SAT_WTPRC {
	uint32_t raw;
	struct {
		uint32_t CNR_CHRA_SAT_WTPRC              : 2;
	} bits;
};

union REG_ISP_CNR_CHRA_SAT_TH {
	uint32_t raw;
	struct {
		uint32_t CNR_CHRA_SAT_TH                 : 5;
	} bits;
};

union REG_ISP_CNR_CHRA_SAT_WTS {
	uint32_t raw;
	struct {
		uint32_t CNR_CHRA_SAT_WTS                : 6;
	} bits;
};

union REG_ISP_CNR_CHRA_SAT_WTE {
	uint32_t raw;
	struct {
		uint32_t CNR_CHRA_SAT_WTE                : 6;
	} bits;
};

union REG_ISP_CNR_CHRA_SAT_OUTBLD_CORING {
	uint32_t raw;
	struct {
		uint32_t CNR_CHRA_SAT_OUTBLD_CORING      : 6;
	} bits;
};

union REG_ISP_CNR_LCA_H_SFACT {
	uint32_t raw;
	struct {
		uint32_t CNR_LCA_H_SFACT                 : 16;
	} bits;
};

union REG_ISP_CNR_LCA_V_SFACT {
	uint32_t raw;
	struct {
		uint32_t CNR_LCA_V_SFACT                 : 16;
	} bits;
};

union REG_ISP_CNR_LCA_H_SFACT_INIT_OFS {
	uint32_t raw;
	struct {
		uint32_t CNR_LCA_H_SFACT_INIT_OFS        : 17;
	} bits;
};

union REG_ISP_CNR_LCA_V_SFACT_INIT_OFS {
	uint32_t raw;
	struct {
		uint32_t CNR_LCA_V_SFACT_INIT_OFS        : 17;
	} bits;
};

union REG_ISP_CNR_LCA_SRC_IMG_SIZE_H {
	uint32_t raw;
	struct {
		uint32_t CNR_LCA_SRC_IMG_SIZE_H          : 14;
	} bits;
};

union REG_ISP_CNR_LCA_SRC_IMG_SIZE_V {
	uint32_t raw;
	struct {
		uint32_t CNR_LCA_SRC_IMG_SIZE_V          : 14;
	} bits;
};

union REG_ISP_CNR_LCA_SUB_IMG_SIZE_H {
	uint32_t raw;
	struct {
		uint32_t CNR_LCA_SUB_IMG_SIZE_H          : 14;
	} bits;
};

union REG_ISP_CNR_LCA_SUB_IMG_SIZE_V {
	uint32_t raw;
	struct {
		uint32_t CNR_LCA_SUB_IMG_SIZE_V          : 14;
	} bits;
};

union REG_ISP_CNR_LCA_HV_FILTMODE {
	uint32_t raw;
	struct {
		uint32_t CNR_LCA_HV_FILTMODE             : 1;
	} bits;
};

union REG_ISP_CNR_LCA_HV_COEF {
	uint32_t raw;
	struct {
		uint32_t CNR_LCA_HV_COEF                 : 6;
	} bits;
};

union REG_ISP_CNR_UPSCL_H_SFACT {
	uint32_t raw;
	struct {
		uint32_t UPSCL_H_SFACT                   : 16;
	} bits;
};

union REG_ISP_CNR_UPSCL_V_SFACT {
	uint32_t raw;
	struct {
		uint32_t UPSCL_V_SFACT                   : 16;
	} bits;
};

union REG_ISP_CNR_UPSCL_H_SFACT_INIT_OFS {
	uint32_t raw;
	struct {
		uint32_t UPSCL_H_SFACT_INIT_OFS          : 17;
	} bits;
};

union REG_ISP_CNR_UPSCL_V_SFACT_INIT_OFS {
	uint32_t raw;
	struct {
		uint32_t UPSCL_V_SFACT_INIT_OFS          : 17;
	} bits;
};

union REG_ISP_CNR_UPSCL_HV_FILTMODE {
	uint32_t raw;
	struct {
		uint32_t UPSCL_HV_FILTMODE               : 1;
	} bits;
};

union REG_ISP_CNR_UPSCL_HV_COEF {
	uint32_t raw;
	struct {
		uint32_t UPSCL_HV_COEF                   : 6;
	} bits;
};

union REG_ISP_CNR_SHDW_READ_SEL {
	uint32_t raw;
	struct {
		uint32_t SHDW_READ_SEL                   : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_EE_ADD_BACK_EE_ADD_B_REG0 {
	uint32_t raw;
	struct {
		uint32_t EE_ENABLE                       : 1;
		uint32_t EE_DEBUG_MODE                   : 4;
		uint32_t HW_AUTO_CG_EN                   : 1;
	} bits;
};

union REG_ISP_EE_ADD_BACK_SHADOW_RD_SEL {
	uint32_t raw;
	struct {
		uint32_t SHDW_READ_SEL                   : 1;
	} bits;
};


/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_YCURV_YCUR_CTRL {
	uint32_t raw;
	struct {
		uint32_t YCUR_ENABLE                     : 1;
		uint32_t YCUR_SHDW_SEL                   : 1;
		uint32_t FORCE_CLK_ENABLE                : 1;
	} bits;
};

union REG_ISP_YCURV_YCUR_PROG_CTRL {
	uint32_t raw;
	struct {
		uint32_t YCUR_WSEL                       : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t YCUR_RSEL                       : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t YCUR_PROG_EN                    : 1;
	} bits;
};

union REG_ISP_YCURV_YCUR_PROG_ST_ADDR {
	uint32_t raw;
	struct {
		uint32_t YCUR_ST_ADDR                    : 6;
		uint32_t _rsv_6                          : 25;
		uint32_t YCUR_ST_W                       : 1;
	} bits;
};

union REG_ISP_YCURV_YCUR_PROG_DATA {
	uint32_t raw;
	struct {
		uint32_t YCUR_DATA_E                     : 8;
		uint32_t _rsv_8                          : 8;
		uint32_t YCUR_DATA_O                     : 8;
		uint32_t _rsv_24                         : 7;
		uint32_t YCUR_W                          : 1;
	} bits;
};

union REG_ISP_YCURV_YCUR_PROG_MAX {
	uint32_t raw;
	struct {
		uint32_t YCUR_MAX                        : 9;
	} bits;
};

union REG_ISP_YCURV_YCUR_MEM_SW_MODE {
	uint32_t raw;
	struct {
		uint32_t YCUR_SW_RADDR                   : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t YCUR_SW_R_MEM_SEL               : 1;
	} bits;
};

union REG_ISP_YCURV_YCUR_MEM_SW_RDATA {
	uint32_t raw;
	struct {
		uint32_t YCUR_RDATA_R                    : 8;
		uint32_t _rsv_8                          : 23;
		uint32_t YCUR_SW_R                       : 1;
	} bits;
};

union REG_ISP_YCURV_YCUR_DBG {
	uint32_t raw;
	struct {
		uint32_t PROG_HDK_DIS                    : 1;
		uint32_t SOFTRST                         : 1;
	} bits;
};

union REG_ISP_YCURV_YCUR_DMY0 {
	uint32_t raw;
	struct {
		uint32_t DMY_DEF0                        : 32;
	} bits;
};

union REG_ISP_YCURV_YCUR_DMY1 {
	uint32_t raw;
	struct {
		uint32_t DMY_DEF1                        : 32;
	} bits;
};

union REG_ISP_YCURV_YCUR_DMY_R {
	uint32_t raw;
	struct {
		uint32_t DMY_RO                          : 32;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_444_422_0 {
	uint32_t raw;
	struct {
		uint32_t FIRST_FRAME_RESET               : 1;
		uint32_t TDNR_ENABLE                     : 2;
		uint32_t _rsv_3                          : 1;
		uint32_t DMA_CROP_ENABLE                 : 1;
		uint32_t FORCE_MONO_ENABLE               : 1;
		uint32_t DEBUG_STATUS_EN                 : 1;
		uint32_t DMA_ENABLE                      : 11;
		uint32_t BYPASS                          : 1;
		uint32_t SHDW_READ_SEL                   : 1;
		uint32_t _rsv_20                         : 1;
		uint32_t HW_AUTO_CG_EN_BLEND             : 1;
		uint32_t HW_AUTO_CG_EN_ALL               : 1;
		uint32_t _rsv_23                         : 1;
		uint32_t GUARD_CNT                       : 8;
	} bits;
};

union REG_ISP_444_422_1 {
	uint32_t raw;
	struct {
		uint32_t TDNR_DEBUG_STATUS               : 32;
	} bits;
};

union REG_ISP_444_422_2 {
	uint32_t raw;
	struct {
		uint32_t TILE_MODE_EN                    : 1;
		uint32_t DMA_SEL                         : 1;
		uint32_t BYPASS_V                        : 1;
		uint32_t BYPASS_H                        : 1;
		uint32_t _rsv_4                          : 1;
		uint32_t AVG_MODE_READ                   : 1;
		uint32_t AVG_MODE_WRITE                  : 1;
		uint32_t DROP_MODE_WRITE                 : 1;
		uint32_t _rsv_8                          : 1;
		uint32_t CHROMA_OFF_FOR_FBCD             : 1;
		uint32_t _rsv_10                         : 1;
		uint32_t TDNR_SEED_RESET_EN              : 1;
		uint32_t _rsv_12                         : 4;
		uint32_t TDNR_DEBUG_CNT                  : 16;
	} bits;
};

union REG_ISP_444_422_3 {
	uint32_t raw;
	struct {
		uint32_t IMG_WIDTH_CROP                  : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t IMG_HEIGHT_CROP                 : 14;
	} bits;
};

union REG_ISP_444_422_4 {
	uint32_t raw;
	struct {
		uint32_t CROP_W_STR                      : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t CROP_W_END                      : 14;
	} bits;
};

union REG_ISP_444_422_5 {
	uint32_t raw;
	struct {
		uint32_t CROP_H_STR                      : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t CROP_H_END                      : 14;
	} bits;
};

union REG_ISP_444_422_6 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_DEBUG_MODE             : 4;
		uint32_t REG_3DNR_SUBPIXEL_ENABLE        : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t REG_3DNR_Y_L0_PSS_BLUR          : 8;
		uint32_t _rsv_16                         : 1;
		uint32_t REG_3DNR_LUMA_JND_RATIO         : 8;
		uint32_t _rsv_25                         : 1;
		uint32_t REG_3DNR_SAD_LPF_MODE           : 2;
	} bits;
};

union REG_ISP_444_422_7 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_JND_STD_U14_0          : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t REG_3DNR_JND_STD_U14_1          : 14;
	} bits;
};

union REG_ISP_444_422_8 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_JND_STD_U14_2          : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t REG_3DNR_JND_STD_U14_3          : 14;
	} bits;
};

union REG_ISP_444_422_9 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_JND_STD_U14_4          : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t REG_3DNR_JND_STD_U14_5          : 14;
	} bits;
};

union REG_ISP_444_422_10 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_JND_STD_U14_6          : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t REG_3DNR_JND_STD_U14_7          : 14;
	} bits;
};

union REG_ISP_444_422_11 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_JND_EDGE_K_U6_0        : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t REG_3DNR_JND_EDGE_K_U6_1        : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t REG_3DNR_JND_EDGE_K_U6_2        : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t REG_3DNR_JND_EDGE_K_U6_3        : 6;
	} bits;
};

union REG_ISP_444_422_12 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_JND_EDGE_K_U6_4        : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t REG_3DNR_JND_EDGE_K_U6_5        : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t REG_3DNR_JND_EDGE_K_U6_6        : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t REG_3DNR_JND_EDGE_K_U6_7        : 6;
	} bits;
};

union REG_ISP_444_422_13 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_JND_B_U14_0            : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t REG_3DNR_JND_B_U14_1            : 14;
	} bits;
};

union REG_ISP_444_422_14 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_JND_B_U14_2            : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t REG_3DNR_JND_B_U14_3            : 14;
	} bits;
};

union REG_ISP_444_422_15 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_JND_B_U14_4            : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t REG_3DNR_JND_B_U14_5            : 14;
	} bits;
};

union REG_ISP_444_422_16 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_JND_B_U14_6            : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t REG_3DNR_JND_B_U14_7            : 14;
	} bits;
};

union REG_ISP_444_422_17 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_WARP_STD_U14_0         : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t REG_3DNR_WARP_STD_U14_1         : 14;
	} bits;
};

union REG_ISP_444_422_18 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_WARP_STD_U14_2         : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t REG_3DNR_WARP_STD_U14_3         : 14;
	} bits;
};

union REG_ISP_444_422_19 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_WARP_STD_U14_4         : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t REG_3DNR_WARP_STD_U14_5         : 14;
	} bits;
};

union REG_ISP_444_422_20 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_WARP_STD_U14_6         : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t REG_3DNR_WARP_STD_U14_7         : 14;
	} bits;
};

union REG_ISP_444_422_21 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_WARP_B0_U14_0          : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t REG_3DNR_WARP_B0_U14_1          : 14;
	} bits;
};

union REG_ISP_444_422_22 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_WARP_B0_U14_2          : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t REG_3DNR_WARP_B0_U14_3          : 14;
	} bits;
};

union REG_ISP_444_422_23 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_WARP_B0_U14_4          : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t REG_3DNR_WARP_B0_U14_5          : 14;
	} bits;
};

union REG_ISP_444_422_24 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_WARP_B0_U14_6          : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t REG_3DNR_WARP_B0_U14_7          : 14;
	} bits;
};

union REG_ISP_444_422_25 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_WARP_EDG_K_U6_0        : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t REG_3DNR_WARP_EDG_K_U6_1        : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t REG_3DNR_WARP_EDG_K_U6_2        : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t REG_3DNR_WARP_EDG_K_U6_3        : 6;
	} bits;
};

union REG_ISP_444_422_26 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_WARP_EDG_K_U6_4        : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t REG_3DNR_WARP_EDG_K_U6_5        : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t REG_3DNR_WARP_EDG_K_U6_6        : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t REG_3DNR_WARP_EDG_K_U6_7        : 6;
	} bits;
};

union REG_ISP_444_422_27 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_WARP_B_U14_0           : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t REG_3DNR_WARP_B_U14_1           : 14;
	} bits;
};

union REG_ISP_444_422_28 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_WARP_B_U14_2           : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t REG_3DNR_WARP_B_U14_3           : 14;
	} bits;
};

union REG_ISP_444_422_29 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_WARP_B_U14_4           : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t REG_3DNR_WARP_B_U14_5           : 14;
	} bits;
};

union REG_ISP_444_422_30 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_WARP_B_U14_6           : 14;
		uint32_t _rsv_14                         : 3;
		uint32_t REG_3DNR_WARP_B_U14_7           : 14;
	} bits;
};

union REG_ISP_444_422_31 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_ACBD_SAD_ADJ_U10_0     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t REG_3DNR_ACBD_SAD_ADJ_U10_1     : 10;
	} bits;
};

union REG_ISP_444_422_32 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_ACBD_SAD_ADJ_U10_2     : 10;
		uint32_t _rsv_10                         : 6;
		uint32_t REG_3DNR_ACBD_SAD_ADJ_U10_3     : 10;
	} bits;
};

union REG_ISP_444_422_33 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_LUMA_JND_TH_U8_0       : 8;
		uint32_t REG_3DNR_LUMA_JND_TH_U8_1       : 8;
		uint32_t REG_3DNR_LUMA_JND_TH_U8_2       : 8;
		uint32_t REG_3DNR_LUMA_JND_TH_U8_3       : 8;
	} bits;
};

union REG_ISP_444_422_34 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_LUMA_JND_TH_U8_4       : 8;
		uint32_t REG_3DNR_LUMA_JND_TH_U8_5       : 8;
		uint32_t REG_3DNR_LUMA_JND_TH_U8_6       : 8;
		uint32_t REG_3DNR_LUMA_JND_TH_U8_7       : 8;
	} bits;
};

union REG_ISP_444_422_35 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_JND_SAD_WT             : 4;
		uint32_t REG_3DNR_EDGE_TH                : 14;
		uint32_t REG_3DNR_A_MUTUAL_TH            : 14;
	} bits;
};

union REG_ISP_444_422_36 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_ACBD_EDGE_ADJ_U4_0     : 4;
		uint32_t REG_3DNR_ACBD_EDGE_ADJ_U4_1     : 4;
		uint32_t REG_3DNR_ACBD_EDGE_ADJ_U4_2     : 4;
		uint32_t REG_3DNR_ACBD_EDGE_ADJ_U4_3     : 4;
		uint32_t _rsv_16                         : 1;
		uint32_t REG_3DNR_ME_BOUNDARY_SET        : 1;
		uint32_t REG_3DNR_RAND_SEARCH_MODE       : 1;
		uint32_t _rsv_19                         : 1;
		uint32_t REG_3DNR_ME_RAND_BIT_Y          : 3;
		uint32_t REG_3DNR_ME_RAND_BIT_X          : 3;
		uint32_t _rsv_26                         : 1;
		uint32_t REG_3DNR_DBG_RANDOM0            : 1;
	} bits;
};

union REG_ISP_444_422_37 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_STABLE_INTENSITY_U1    : 8;
		uint32_t REG_3DNR_FLOW_JND_MODE          : 2;
		uint32_t REG_3DNR_FLOW_NORM_MODE         : 2;
		uint32_t _rsv_12                         : 1;
		uint32_t REG_3DNR_DYNAMIC_BLUR_ENABLE    : 1;
		uint32_t _rsv_14                         : 1;
		uint32_t REG_3DNR_DYNAMIC_BLUR_JND_TH    : 5;
		uint32_t _rsv_20                         : 2;
		uint32_t REG_3DNR_DYNAMIC_BLUR_ST        : 4;
		uint32_t _rsv_26                         : 2;
		uint32_t REG_3DNR_DYNAMIC_BLUR_STEP      : 4;
	} bits;
};

union REG_ISP_444_422_38 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L0_ST_JND_SIGMA_U8_0 : 8;
		uint32_t REG_3DNR_Y_L0_ST_JND_SIGMA_U8_1 : 8;
		uint32_t REG_3DNR_Y_L0_ST_JND_SIGMA_U8_2 : 8;
	} bits;
};

union REG_ISP_444_422_39 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L0_SS_JND_BLUR_U8_0  : 8;
		uint32_t REG_3DNR_Y_L0_SS_JND_BLUR_U8_1  : 8;
		uint32_t REG_3DNR_Y_L0_SS_JND_BLUR_U8_2  : 8;
	} bits;
};

union REG_ISP_444_422_40 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L0_SS_GRAD_BLUR_TH   : 16;
		uint32_t REG_3DNR_Y_L0_SS_GRAD_BLUR_STR  : 8;
		uint32_t REG_3DNR_Y_L0_ST_CENTER_ZERO_EN : 1;
	} bits;
};

union REG_ISP_444_422_41 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_SAD_NEIGHBOR_TH        : 14;
		uint32_t REG_3DNR_NEIGHBOR_RATIO_MODE    : 1;
		uint32_t REG_3DNR_NEIGHBOR_BLEND_RATIO_U8_0: 8;
		uint32_t REG_3DNR_NEIGHBOR_BLEND_RATIO_U8_1: 8;
	} bits;
};

union REG_ISP_444_422_42 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_FLOW_NEIGHBOR_TH_U14_0 : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t REG_3DNR_FLOW_NEIGHBOR_TH_U14_1 : 14;
	} bits;
};

union REG_ISP_444_422_43 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_RATIO_SLOPE_U16_0      : 16;
		uint32_t REG_3DNR_RATIO_SLOPE_U16_1      : 16;
	} bits;
};

union REG_ISP_444_422_44 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_STILL_REGION_TH_U14_0  : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t REG_3DNR_STILL_REGION_TH_U14_1  : 14;
	} bits;
};

union REG_ISP_444_422_45 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_STILL_MIX_BASE         : 8;
		uint32_t REG_3DNR_STILL_MIX_SLOPE        : 16;
		uint32_t _rsv_24                         : 1;
		uint32_t REG_3DNR_NULL_FLOW_MODE         : 1;
	} bits;
};

union REG_ISP_444_422_46 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_FS_GLOBAL_STR     : 16;
		uint32_t REG_3DNR_Y_L1_FT_GLOBAL_STR     : 16;
	} bits;
};

union REG_ISP_444_422_47 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_LUMA_ADJ_U8_0     : 8;
		uint32_t REG_3DNR_Y_L1_LUMA_ADJ_U8_1     : 8;
		uint32_t REG_3DNR_Y_L1_LUMA_ADJ_U8_2     : 8;
		uint32_t REG_3DNR_Y_L1_LUMA_ADJ_U8_3     : 8;
	} bits;
};

union REG_ISP_444_422_48 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_LUMA_ADJ_U8_4     : 8;
		uint32_t REG_3DNR_Y_L1_LUMA_ADJ_U8_5     : 8;
		uint32_t REG_3DNR_Y_L1_LUMA_ADJ_U8_6     : 8;
		uint32_t REG_3DNR_Y_L1_LUMA_ADJ_U8_7     : 8;
	} bits;
};

union REG_ISP_444_422_49 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_FS_JND_ADJ_U8_0   : 8;
		uint32_t REG_3DNR_Y_L1_FS_JND_ADJ_U8_1   : 8;
		uint32_t REG_3DNR_Y_L1_FS_JND_ADJ_U8_2   : 8;
	} bits;
};

union REG_ISP_444_422_50 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_FT_JND_ADJ_U8_0   : 8;
		uint32_t REG_3DNR_Y_L1_FT_JND_ADJ_U8_1   : 8;
		uint32_t REG_3DNR_Y_L1_FT_JND_ADJ_U8_2   : 8;
		uint32_t _rsv_24                         : 4;
		uint32_t REG_3DNR_DYNAMIC_BLUR_UV_WT     : 4;
	} bits;
};

union REG_ISP_444_422_51 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_UV_L0_ST_JND_SIGMA_U8_0: 8;
		uint32_t REG_3DNR_UV_L0_ST_JND_SIGMA_U8_1: 8;
		uint32_t REG_3DNR_UV_L0_ST_JND_SIGMA_U8_2: 8;
		uint32_t _rsv_24                         : 1;
		uint32_t REG_3DNR_DC_ENABLE              : 1;
	} bits;
};

union REG_ISP_444_422_52 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_FS_FREQ_ADJ_U8_0  : 8;
		uint32_t REG_3DNR_Y_L1_FS_FREQ_ADJ_U8_1  : 8;
		uint32_t REG_3DNR_Y_L1_FS_FREQ_ADJ_U8_2  : 8;
		uint32_t REG_3DNR_Y_L1_FS_FREQ_ADJ_U8_3  : 8;
	} bits;
};

union REG_ISP_444_422_53 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_FT_FREQ_ADJ_U8_0  : 8;
		uint32_t REG_3DNR_Y_L1_FT_FREQ_ADJ_U8_1  : 8;
		uint32_t REG_3DNR_Y_L1_FT_FREQ_ADJ_U8_2  : 8;
		uint32_t REG_3DNR_Y_L1_FT_FREQ_ADJ_U8_3  : 8;
	} bits;
};

union REG_ISP_444_422_54 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_0        : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_1        : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_2        : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_3        : 8;
	} bits;
};

union REG_ISP_444_422_55 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_4        : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_5        : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_6        : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_7        : 8;
	} bits;
};

union REG_ISP_444_422_56 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_8        : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_9        : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_10       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_11       : 8;
	} bits;
};

union REG_ISP_444_422_57 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_12       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_13       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_14       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_15       : 8;
	} bits;
};

union REG_ISP_444_422_58 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_16       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_17       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_18       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_19       : 8;
	} bits;
};

union REG_ISP_444_422_59 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_20       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_21       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_22       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_23       : 8;
	} bits;
};

union REG_ISP_444_422_60 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_24       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_25       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_26       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_27       : 8;
	} bits;
};

union REG_ISP_444_422_61 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_28       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_29       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_30       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_31       : 8;
	} bits;
};

union REG_ISP_444_422_62 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_32       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_33       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_34       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_35       : 8;
	} bits;
};

union REG_ISP_444_422_63 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_36       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_37       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_38       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_39       : 8;
	} bits;
};

union REG_ISP_444_422_64 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_40       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_41       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_42       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_43       : 8;
	} bits;
};

union REG_ISP_444_422_65 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_44       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_45       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_46       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_47       : 8;
	} bits;
};

union REG_ISP_444_422_66 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_48       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_49       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_50       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_51       : 8;
	} bits;
};

union REG_ISP_444_422_67 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_52       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_53       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_54       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_55       : 8;
	} bits;
};

union REG_ISP_444_422_68 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_56       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_57       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_58       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_59       : 8;
	} bits;
};

union REG_ISP_444_422_69 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_60       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_61       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_62       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_S_U8_63       : 8;
	} bits;
};

union REG_ISP_444_422_70 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_0        : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_1        : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_2        : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_3        : 8;
	} bits;
};

union REG_ISP_444_422_71 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_4        : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_5        : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_6        : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_7        : 8;
	} bits;
};

union REG_ISP_444_422_72 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_8        : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_9        : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_10       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_11       : 8;
	} bits;
};

union REG_ISP_444_422_73 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_12       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_13       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_14       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_15       : 8;
	} bits;
};

union REG_ISP_444_422_74 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_16       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_17       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_18       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_19       : 8;
	} bits;
};

union REG_ISP_444_422_75 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_20       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_21       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_22       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_23       : 8;
	} bits;
};

union REG_ISP_444_422_76 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_24       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_25       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_26       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_27       : 8;
	} bits;
};

union REG_ISP_444_422_77 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_28       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_29       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_30       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_31       : 8;
	} bits;
};

union REG_ISP_444_422_78 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_32       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_33       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_34       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_35       : 8;
	} bits;
};

union REG_ISP_444_422_79 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_36       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_37       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_38       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_39       : 8;
	} bits;
};

union REG_ISP_444_422_80 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_40       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_41       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_42       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_43       : 8;
	} bits;
};

union REG_ISP_444_422_81 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_44       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_45       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_46       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_47       : 8;
	} bits;
};

union REG_ISP_444_422_82 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_48       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_49       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_50       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_51       : 8;
	} bits;
};

union REG_ISP_444_422_83 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_52       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_53       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_54       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_55       : 8;
	} bits;
};

union REG_ISP_444_422_84 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_56       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_57       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_58       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_59       : 8;
	} bits;
};

union REG_ISP_444_422_85 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_60       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_61       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_62       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_T_U8_63       : 8;
	} bits;
};

union REG_ISP_444_422_86 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_0        : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_1        : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_2        : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_3        : 8;
	} bits;
};

union REG_ISP_444_422_87 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_4        : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_5        : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_6        : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_7        : 8;
	} bits;
};

union REG_ISP_444_422_88 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_8        : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_9        : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_10       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_11       : 8;
	} bits;
};

union REG_ISP_444_422_89 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_12       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_13       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_14       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_15       : 8;
	} bits;
};

union REG_ISP_444_422_90 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_16       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_17       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_18       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_19       : 8;
	} bits;
};

union REG_ISP_444_422_91 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_20       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_21       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_22       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_23       : 8;
	} bits;
};

union REG_ISP_444_422_92 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_24       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_25       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_26       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_27       : 8;
	} bits;
};

union REG_ISP_444_422_93 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_28       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_29       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_30       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_31       : 8;
	} bits;
};

union REG_ISP_444_422_94 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_32       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_33       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_34       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_35       : 8;
	} bits;
};

union REG_ISP_444_422_95 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_36       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_37       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_38       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_39       : 8;
	} bits;
};

union REG_ISP_444_422_96 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_40       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_41       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_42       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_43       : 8;
	} bits;
};

union REG_ISP_444_422_97 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_44       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_45       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_46       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_47       : 8;
	} bits;
};

union REG_ISP_444_422_98 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_48       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_49       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_50       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_51       : 8;
	} bits;
};

union REG_ISP_444_422_99 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_52       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_53       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_54       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_55       : 8;
	} bits;
};

union REG_ISP_444_422_100 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_56       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_57       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_58       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_59       : 8;
	} bits;
};

union REG_ISP_444_422_101 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_60       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_61       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_62       : 8;
		uint32_t REG_3DNR_Y_L1_PFT_M_U8_63       : 8;
	} bits;
};

union REG_ISP_444_422_102 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L2_S_LUT_U7_0        : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t REG_3DNR_Y_L2_S_LUT_U7_1        : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t REG_3DNR_Y_L2_S_LUT_U7_2        : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t REG_3DNR_Y_L2_S_LUT_U7_3        : 7;
	} bits;
};

union REG_ISP_444_422_103 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L2_S_LUT_U7_4        : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t REG_3DNR_Y_L2_S_LUT_U7_5        : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t REG_3DNR_Y_L2_S_LUT_U7_6        : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t REG_3DNR_Y_L2_S_LUT_U7_7        : 7;
	} bits;
};

union REG_ISP_444_422_104 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L2_T_LUT_U7_0        : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t REG_3DNR_Y_L2_T_LUT_U7_1        : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t REG_3DNR_Y_L2_T_LUT_U7_2        : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t REG_3DNR_Y_L2_T_LUT_U7_3        : 7;
	} bits;
};

union REG_ISP_444_422_105 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L2_T_LUT_U7_4        : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t REG_3DNR_Y_L2_T_LUT_U7_5        : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t REG_3DNR_Y_L2_T_LUT_U7_6        : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t REG_3DNR_Y_L2_T_LUT_U7_7        : 7;
	} bits;
};

union REG_ISP_444_422_106 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L2_M_LUT_U7_0        : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t REG_3DNR_Y_L2_M_LUT_U7_1        : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t REG_3DNR_Y_L2_M_LUT_U7_2        : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t REG_3DNR_Y_L2_M_LUT_U7_3        : 7;
	} bits;
};

union REG_ISP_444_422_107 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_L2_M_LUT_U7_4        : 7;
		uint32_t _rsv_7                          : 1;
		uint32_t REG_3DNR_Y_L2_M_LUT_U7_5        : 7;
		uint32_t _rsv_15                         : 1;
		uint32_t REG_3DNR_Y_L2_M_LUT_U7_6        : 7;
		uint32_t _rsv_23                         : 1;
		uint32_t REG_3DNR_Y_L2_M_LUT_U7_7        : 7;
	} bits;
};

union REG_ISP_444_422_108 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_DYNAMIC_BLUR_Y_JND_WT_U4_0: 4;
		uint32_t REG_3DNR_DYNAMIC_BLUR_Y_JND_WT_U4_1: 4;
		uint32_t REG_3DNR_DYNAMIC_BLUR_Y_JND_WT_U4_2: 4;
		uint32_t REG_3DNR_Y_L1_FT_LUMA_STR       : 8;
	} bits;
};

union REG_ISP_444_422_109 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_UV_L2_ST_DELTA_DAMP_U7_0: 7;
		uint32_t _rsv_7                          : 1;
		uint32_t REG_3DNR_UV_L2_ST_DELTA_DAMP_U7_1: 7;
		uint32_t _rsv_15                         : 1;
		uint32_t REG_3DNR_UV_L2_ST_DELTA_DAMP_U7_2: 7;
		uint32_t _rsv_23                         : 1;
		uint32_t REG_3DNR_UV_L2_ST_DELTA_DAMP_U7_3: 7;
	} bits;
};

union REG_ISP_444_422_110 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_UV_L2_ST_DELTA_DAMP_U7_4: 7;
		uint32_t _rsv_7                          : 1;
		uint32_t REG_3DNR_UV_L2_ST_DELTA_DAMP_U7_5: 7;
		uint32_t _rsv_15                         : 1;
		uint32_t REG_3DNR_UV_L2_ST_DELTA_DAMP_U7_6: 7;
		uint32_t _rsv_23                         : 1;
		uint32_t REG_3DNR_UV_L2_ST_DELTA_DAMP_U7_7: 7;
	} bits;
};

union REG_ISP_444_422_111 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_UV_L2_ST_DELTA_DAMP_M_U7_0: 7;
		uint32_t _rsv_7                          : 1;
		uint32_t REG_3DNR_UV_L2_ST_DELTA_DAMP_M_U7_1: 7;
		uint32_t _rsv_15                         : 1;
		uint32_t REG_3DNR_UV_L2_ST_DELTA_DAMP_M_U7_2: 7;
		uint32_t _rsv_23                         : 1;
		uint32_t REG_3DNR_UV_L2_ST_DELTA_DAMP_M_U7_3: 7;
	} bits;
};

union REG_ISP_444_422_112 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_UV_L2_ST_DELTA_DAMP_M_U7_4: 7;
		uint32_t _rsv_7                          : 1;
		uint32_t REG_3DNR_UV_L2_ST_DELTA_DAMP_M_U7_5: 7;
		uint32_t _rsv_15                         : 1;
		uint32_t REG_3DNR_UV_L2_ST_DELTA_DAMP_M_U7_6: 7;
		uint32_t _rsv_23                         : 1;
		uint32_t REG_3DNR_UV_L2_ST_DELTA_DAMP_M_U7_7: 7;
	} bits;
};

union REG_ISP_444_422_113 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_UV_L2_ST_DELTA_DAMP_T_U7_0: 7;
		uint32_t _rsv_7                          : 1;
		uint32_t REG_3DNR_UV_L2_ST_DELTA_DAMP_T_U7_1: 7;
		uint32_t _rsv_15                         : 1;
		uint32_t REG_3DNR_UV_L2_ST_DELTA_DAMP_T_U7_2: 7;
		uint32_t _rsv_23                         : 1;
		uint32_t REG_3DNR_UV_L2_ST_DELTA_DAMP_T_U7_3: 7;
	} bits;
};

union REG_ISP_444_422_114 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_UV_L2_ST_DELTA_DAMP_T_U7_4: 7;
		uint32_t _rsv_7                          : 1;
		uint32_t REG_3DNR_UV_L2_ST_DELTA_DAMP_T_U7_5: 7;
		uint32_t _rsv_15                         : 1;
		uint32_t REG_3DNR_UV_L2_ST_DELTA_DAMP_T_U7_6: 7;
		uint32_t _rsv_23                         : 1;
		uint32_t REG_3DNR_UV_L2_ST_DELTA_DAMP_T_U7_7: 7;
	} bits;
};

union REG_ISP_444_422_115 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_SAD_DEBIAS_ENABLE      : 1;
		uint32_t REG_3DNR_WARP_STD_RANGE_U6_0    : 6;
		uint32_t REG_3DNR_WARP_STD_RANGE_U6_1    : 6;
		uint32_t REG_3DNR_JND_STD_RANGE_U6_0     : 6;
		uint32_t REG_3DNR_JND_STD_RANGE_U6_1     : 6;
		uint32_t REG_3DNR_FLOW_CHECK             : 1;
		uint32_t REG_3DNR_FLOW_JND_TH            : 6;
	} bits;
};

union REG_ISP_444_422_116 {
	uint32_t raw;
	struct {
		uint32_t REG_3DNR_Y_BUNDLE_NR_ENABLE     : 1;
		uint32_t _rsv_1                          : 1;
		uint32_t REG_3DNR_UV_BUNDLE_NR_ENABLE    : 1;
		uint32_t _rsv_3                          : 1;
		uint32_t REG_3DNR_UV_MONO_ENABLE         : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t REG_3DNR_UV_MONO_VAL            : 8;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_YUV_DITHER_Y_DITHER {
	uint32_t raw;
	struct {
		uint32_t Y_DITHER_ENABLE                 : 1;
		uint32_t Y_DITHER_MOD_ENABLE             : 1;
		uint32_t Y_DITHER_HISTIDX_ENABLE         : 1;
		uint32_t Y_DITHER_FMNUM_ENABLE           : 1;
		uint32_t Y_DITHER_SHDW_SEL               : 1;
		uint32_t Y_DITHER_SOFTRST                : 1;
		uint32_t _rsv_6                          : 2;
		uint32_t Y_DITHER_HEIGHTM1               : 12;
		uint32_t Y_DITHER_WIDTHM1                : 12;
	} bits;
};

union REG_ISP_YUV_DITHER_UV_DITHER {
	uint32_t raw;
	struct {
		uint32_t UV_DITHER_ENABLE                : 1;
		uint32_t UV_DITHER_MOD_ENABLE            : 1;
		uint32_t UV_DITHER_HISTIDX_ENABLE        : 1;
		uint32_t UV_DITHER_FMNUM_ENABLE          : 1;
		uint32_t _rsv_4                          : 4;
		uint32_t UV_DITHER_HEIGHTM1              : 12;
		uint32_t UV_DITHER_WIDTHM1               : 12;
	} bits;
};

union REG_ISP_YUV_DITHER_DEBUG_00 {
	uint32_t raw;
	struct {
		uint32_t UV_DITHER_DEBUG0                : 32;
	} bits;
};

union REG_ISP_YUV_DITHER_DEBUG_01 {
	uint32_t raw;
	struct {
		uint32_t Y_DITHER_DEBUG0                 : 32;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_LDCI_CTL {
	uint32_t raw;
	struct {
		uint32_t LDCI_ENABLE                     : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t DCI_ENABLE                      : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t LDCI_LUMA_PROT_EN               : 1;
		uint32_t _rsv_9                          : 7;
		uint32_t BYPASS_PIPECTRL                 : 1;
		uint32_t _rsv_17                         : 7;
		uint32_t LDCI_PIPE_RST                   : 1;
		uint32_t _rsv_25                         : 3;
		uint32_t LDCI_PIPE_GCLK_EN               : 1;
	} bits;
};

union REG_ISP_LDCI_RECIPROCAL_0 {
	uint32_t raw;
	struct {
		uint32_t LDCI_RECIPROCAL_V               : 16;
		uint32_t LDCI_RECIPROCAL_H               : 16;
	} bits;
};

union REG_ISP_LDCI_LUMA_PROT_LUT0 {
	uint32_t raw;
	struct {
		uint32_t LDCI_LUMA_PROT_LUT_0            : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t LDCI_LUMA_PROT_LUT_1            : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t LDCI_LUMA_PROT_LUT_2            : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t LDCI_LUMA_PROT_LUT_3            : 6;
	} bits;
};

union REG_ISP_LDCI_LUMA_PROT_LUT1 {
	uint32_t raw;
	struct {
		uint32_t LDCI_LUMA_PROT_LUT_4            : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t LDCI_LUMA_PROT_LUT_5            : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t LDCI_LUMA_PROT_LUT_6            : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t LDCI_LUMA_PROT_LUT_7            : 6;
	} bits;
};

union REG_ISP_LDCI_LUMA_PROT_LUT2 {
	uint32_t raw;
	struct {
		uint32_t LDCI_LUMA_PROT_LUT_8            : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t LDCI_LUMA_PROT_LUT_9            : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t LDCI_LUMA_PROT_LUT_10           : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t LDCI_LUMA_PROT_LUT_11           : 6;
	} bits;
};

union REG_ISP_LDCI_LUMA_PROT_LUT3 {
	uint32_t raw;
	struct {
		uint32_t LDCI_LUMA_PROT_LUT_12           : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t LDCI_LUMA_PROT_LUT_13           : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t LDCI_LUMA_PROT_LUT_14           : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t LDCI_LUMA_PROT_LUT_15           : 6;
	} bits;
};

union REG_ISP_LDCI_LUMA_PROT_LUT4 {
	uint32_t raw;
	struct {
		uint32_t LDCI_LUMA_PROT_LUT_16           : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t LDCI_LUMA_PROT_CW               : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t LDCI_LUMA_PROT_BW               : 4;
	} bits;
};

union REG_ISP_LDCI_BLK_NUM {
	uint32_t raw;
	struct {
		uint32_t LDCI_BLK_NUM_H                  : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t LDCI_BLK_NUM_V                  : 6;
	} bits;
};

union REG_ISP_LDCI_BLK_SIZE {
	uint32_t raw;
	struct {
		uint32_t LDCI_BLK_WIDTH                  : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t LDCI_BLK_HEIGHT                 : 12;
	} bits;
};

union REG_ISP_LDCI_RECIPROCAL_1 {
	uint32_t raw;
	struct {
		uint32_t LDCI_RECIPROCAL                 : 27;
	} bits;
};

union REG_ISP_LDCI_LUMA_GAIN_LUT0 {
	uint32_t raw;
	struct {
		uint32_t LDCI_LUMA_GAIN_LUT_0            : 8;
		uint32_t LDCI_LUMA_GAIN_LUT_1            : 8;
		uint32_t LDCI_LUMA_GAIN_LUT_2            : 8;
		uint32_t LDCI_LUMA_GAIN_LUT_3            : 8;
	} bits;
};

union REG_ISP_LDCI_LUMA_GAIN_LUT1 {
	uint32_t raw;
	struct {
		uint32_t LDCI_LUMA_GAIN_LUT_4            : 8;
		uint32_t LDCI_LUMA_GAIN_LUT_5            : 8;
		uint32_t LDCI_LUMA_GAIN_LUT_6            : 8;
		uint32_t LDCI_LUMA_GAIN_LUT_7            : 8;
	} bits;
};

union REG_ISP_LDCI_LUMA_GAIN_LUT2 {
	uint32_t raw;
	struct {
		uint32_t LDCI_LUMA_GAIN_LUT_8            : 8;
	} bits;
};

union REG_ISP_LDCI_THR {
	uint32_t raw;
	struct {
		uint32_t LDCI_THRESHOLD1                 : 8;
	} bits;
};

union REG_ISP_LDCI_DIFF_GAIN_LUT_L0 {
	uint32_t raw;
	struct {
		uint32_t LDCI_DIFF_GAIN_LUT_L_0          : 8;
		uint32_t LDCI_DIFF_GAIN_LUT_L_1          : 8;
		uint32_t LDCI_DIFF_GAIN_LUT_L_2          : 8;
		uint32_t LDCI_DIFF_GAIN_LUT_L_3          : 8;
	} bits;
};

union REG_ISP_LDCI_DIFF_GAIN_LUT_L1 {
	uint32_t raw;
	struct {
		uint32_t LDCI_DIFF_GAIN_LUT_L_4          : 8;
	} bits;
};

union REG_ISP_LDCI_DIFF_GAIN_LUT_R0 {
	uint32_t raw;
	struct {
		uint32_t LDCI_DIFF_GAIN_LUT_R_0          : 8;
		uint32_t LDCI_DIFF_GAIN_LUT_R_1          : 8;
		uint32_t LDCI_DIFF_GAIN_LUT_R_2          : 8;
		uint32_t LDCI_DIFF_GAIN_LUT_R_3          : 8;
	} bits;
};

union REG_ISP_LDCI_DIFF_GAIN_LUT_R1 {
	uint32_t raw;
	struct {
		uint32_t LDCI_DIFF_GAIN_LUT_R_4          : 8;
	} bits;
};

union REG_ISP_LDCI_ROI_CTL {
	uint32_t raw;
	struct {
		uint32_t DCI_HISTSAMPLE_STEP             : 5;
		uint32_t _rsv_5                          : 3;
		uint32_t DCI_ROI_ENABLE                  : 1;
	} bits;
};

union REG_ISP_LDCI_ROI_AXIS_0 {
	uint32_t raw;
	struct {
		uint32_t DCI_ROI_START_X                 : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t DCI_ROI_START_Y                 : 13;
	} bits;
};

union REG_ISP_LDCI_ROI_AXIS_1 {
	uint32_t raw;
	struct {
		uint32_t DCI_ROI_WIDTH                   : 13;
		uint32_t _rsv_13                         : 3;
		uint32_t DCI_ROI_HEIGHT                  : 13;
	} bits;
};

union REG_ISP_LDCI_HW_CTL {
	uint32_t raw;
	struct {
		uint32_t LDCI_FORCE_CK_EN                : 1;
		uint32_t _rsv_1                          : 7;
		uint32_t LDCI_SHDW_SEL                   : 1;
	} bits;
};

union REG_ISP_LDCI_DMA_STAT {
	uint32_t raw;
	struct {
		uint32_t LDCI_LMAP_RDMA_IDLE             : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t LDCI_LMAP_WDMA_IDLE             : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t LDCI_HIST_WDMA_IDLE             : 1;
	} bits;
};

union REG_ISP_LDCI_CROP_WIDTH {
	uint32_t raw;
	struct {
		uint32_t LDCI_CROP_W_STR                 : 16;
		uint32_t LDCI_CROP_W_END                 : 16;
	} bits;
};

union REG_ISP_LDCI_CROP_HEIGHT {
	uint32_t raw;
	struct {
		uint32_t LDCI_CROP_H_STR                 : 16;
		uint32_t LDCI_CROP_H_END                 : 16;
	} bits;
};

union REG_ISP_LDCI_CROP_ENABLE {
	uint32_t raw;
	struct {
		uint32_t LDCI_CROP_ENABLE                : 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_LDCI_MAP_LUT_LDCI_MAP_CTL {
	uint32_t raw;
	struct {
		uint32_t SHDW_SEL                        : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t MAP_EN                          : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t MAP_PROG_HDK_DIS                : 1;
		uint32_t _rsv_9                          : 3;
		uint32_t CK_ENABLE                       : 1;
		uint32_t _rsv_13                         : 3;
		uint32_t LUT_PROG_EN                     : 1;
	} bits;
};

union REG_LDCI_MAP_LUT_LDCI_RW_CTL {
	uint32_t raw;
	struct {
		uint32_t LUT_WSEL                        : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t LUT_RSEL                        : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t LUT_W1T                         : 1;
		uint32_t _rsv_9                          : 3;
		uint32_t LUT_ST_W1T                      : 1;
		uint32_t _rsv_13                         : 3;
		uint32_t LUT_WDATA                       : 8;
		uint32_t LUT_WADDR                       : 8;
	} bits;
};

union REG_LDCI_MAP_LUT_LDCI_MAP_SW_CTL {
	uint32_t raw;
	struct {
		uint32_t SW_LUT_R_W1T                    : 1;
		uint32_t _rsv_1                          : 7;
		uint32_t SW_LUT_RADDR                    : 8;
		uint32_t SW_LUT_RSEL                     : 1;
		uint32_t _rsv_17                         : 7;
		uint32_t SW_LUT_RDATA                    : 8;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_EE_00 {
	uint32_t raw;
	struct {
		uint32_t EE_ENABLE                       : 1;
		uint32_t EE_SHADOW_SEL                   : 1;
		uint32_t _rsv_2                          : 2;
		uint32_t EE_DEBUG_MODE                   : 4;
		uint32_t EE_CORING_TH                    : 8;
		uint32_t EE_CHROMA_ADPTCTRL_EN           : 1;
		uint32_t HW_AUTO_CG_EN                   : 1;
		uint32_t CBCR_SWAP_EN                    : 1;
	} bits;
};

union REG_ISP_EE_04 {
	uint32_t raw;
	struct {
		uint32_t EE_OVERSHOOT_CLIP_RATIO         : 8;
		uint32_t EE_UNDERSHOOT_CLIP_RATIO        : 8;
	} bits;
};

union REG_ISP_EE_0C {
	uint32_t raw;
	struct {
		uint32_t EE_DELTA_WT_SRC_OPT             : 1;
		uint32_t EE_DELTA_WT_CORING_TH           : 8;
		uint32_t EE_DELTA_WT_GAIN                : 8;
		uint32_t EE_NOISE_LEVEL                  : 8;
	} bits;
};

union REG_ISP_EE_10 {
	uint32_t raw;
	struct {
		uint32_t EE_BLEND_DEGAMMA                : 8;
		uint32_t EE_SHARP_STR                    : 8;
	} bits;
};

union REG_ISP_EE_A4 {
	uint32_t raw;
	struct {
		uint32_t EE_NOISE_LUT_U8_0               : 8;
		uint32_t EE_NOISE_LUT_U8_1               : 8;
		uint32_t EE_NOISE_LUT_U8_2               : 8;
		uint32_t EE_NOISE_LUT_U8_3               : 8;
	} bits;
};

union REG_ISP_EE_A8 {
	uint32_t raw;
	struct {
		uint32_t EE_NOISE_LUT_U8_4               : 8;
		uint32_t EE_NOISE_LUT_U8_5               : 8;
		uint32_t EE_NOISE_LUT_U8_6               : 8;
		uint32_t EE_NOISE_LUT_U8_7               : 8;
	} bits;
};

union REG_ISP_EE_AC {
	uint32_t raw;
	struct {
		uint32_t EE_NOISE_LUT_U8_8               : 8;
		uint32_t EE_NOISE_LUT_U8_9               : 8;
		uint32_t EE_NOISE_LUT_U8_10              : 8;
		uint32_t EE_NOISE_LUT_U8_11              : 8;
	} bits;
};

union REG_ISP_EE_B0 {
	uint32_t raw;
	struct {
		uint32_t EE_NOISE_LUT_U8_12              : 8;
		uint32_t EE_NOISE_LUT_U8_13              : 8;
		uint32_t EE_NOISE_LUT_U8_14              : 8;
		uint32_t EE_NOISE_LUT_U8_15              : 8;
	} bits;
};

union REG_ISP_EE_B4 {
	uint32_t raw;
	struct {
		uint32_t EE_NOISE_LUT_U8_16              : 8;
	} bits;
};

union REG_ISP_EE_B8 {
	uint32_t raw;
	struct {
		uint32_t EE_CON_ENG_WT                   : 4;
		uint32_t EE_FLAT_TH                      : 11;
		uint32_t EE_EDGE_TH                      : 11;
	} bits;
};

union REG_ISP_EE_BC {
	uint32_t raw;
	struct {
		uint32_t EE_CON_ENG_SLOPE                : 12;
		uint32_t EE_FLAT_REGION_STR              : 8;
		uint32_t EE_EDGE_REGION_STR              : 8;
	} bits;
};

union REG_ISP_EE_C0 {
	uint32_t raw;
	struct {
		uint32_t EE_MOTION_ENABLE                : 1;
		uint32_t EE_MOTION_DELTA_WT_STR          : 8;
	} bits;
};

union REG_ISP_EE_C4 {
	uint32_t raw;
	struct {
		uint32_t EE_STATIC_DELTA_WT_STR          : 8;
		uint32_t EE_TRANS_DELTA_WT_STR           : 8;
		uint32_t EE_FILTER_SIZE_OPT              : 1;
	} bits;
};

union REG_ISP_EE_C8 {
	uint32_t raw;
	struct {
		uint32_t EE_DELTA_WT_GAIN_LUT_U11_0      : 11;
		uint32_t EE_DELTA_WT_GAIN_LUT_U11_1      : 11;
	} bits;
};

union REG_ISP_EE_HCC {
	uint32_t raw;
	struct {
		uint32_t EE_DELTA_WT_GAIN_LUT_U11_2      : 11;
		uint32_t EE_DELTA_WT_GAIN_LUT_U11_3      : 11;
	} bits;
};

union REG_ISP_EE_HD0 {
	uint32_t raw;
	struct {
		uint32_t EE_DELTA_WT_GAIN_LUT_U11_4      : 11;
		uint32_t EE_DELTA_WT_GAIN_LUT_U11_5      : 11;
	} bits;
};

union REG_ISP_EE_HD4 {
	uint32_t raw;
	struct {
		uint32_t EE_DELTA_WT_GAIN_LUT_U11_6      : 11;
		uint32_t EE_DELTA_WT_GAIN_LUT_U11_7      : 11;
	} bits;
};

union REG_ISP_EE_HD8 {
	uint32_t raw;
	struct {
		uint32_t EE_DELTA_WT_GAIN_LUT_U11_8      : 11;
	} bits;
};

union REG_ISP_EE_178 {
	uint32_t raw;
	struct {
		uint32_t EE_CHROMA_ADPTCTRL_LUT_00       : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_01       : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_02       : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_03       : 6;
	} bits;
};

union REG_ISP_EE_17C {
	uint32_t raw;
	struct {
		uint32_t EE_CHROMA_ADPTCTRL_LUT_04       : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_05       : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_06       : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_07       : 6;
	} bits;
};

union REG_ISP_EE_180 {
	uint32_t raw;
	struct {
		uint32_t EE_CHROMA_ADPTCTRL_LUT_08       : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_09       : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_10       : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_11       : 6;
	} bits;
};

union REG_ISP_EE_184 {
	uint32_t raw;
	struct {
		uint32_t EE_CHROMA_ADPTCTRL_LUT_12       : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_13       : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_14       : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_15       : 6;
	} bits;
};

union REG_ISP_EE_188 {
	uint32_t raw;
	struct {
		uint32_t EE_CHROMA_ADPTCTRL_LUT_16       : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_17       : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_18       : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_19       : 6;
	} bits;
};

union REG_ISP_EE_18C {
	uint32_t raw;
	struct {
		uint32_t EE_CHROMA_ADPTCTRL_LUT_20       : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_21       : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_22       : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_23       : 6;
	} bits;
};

union REG_ISP_EE_190 {
	uint32_t raw;
	struct {
		uint32_t EE_CHROMA_ADPTCTRL_LUT_24       : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_25       : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_26       : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_27       : 6;
	} bits;
};

union REG_ISP_EE_194 {
	uint32_t raw;
	struct {
		uint32_t EE_CHROMA_ADPTCTRL_LUT_28       : 6;
		uint32_t _rsv_6                          : 2;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_29       : 6;
		uint32_t _rsv_14                         : 2;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_30       : 6;
		uint32_t _rsv_22                         : 2;
		uint32_t EE_CHROMA_ADPTCTRL_LUT_31       : 6;
	} bits;
};

union REG_ISP_EE_198 {
	uint32_t raw;
	struct {
		uint32_t EE_CHROMA_ADPTCTRL_LUT_32       : 6;
	} bits;
};

union REG_ISP_EE_19C {
	uint32_t raw;
	struct {
		uint32_t EE_CHROMA_AMP_LUT_IN_0          : 8;
		uint32_t EE_CHROMA_AMP_LUT_IN_1          : 8;
		uint32_t EE_CHROMA_AMP_LUT_IN_2          : 8;
		uint32_t EE_CHROMA_AMP_LUT_IN_3          : 8;
	} bits;
};

union REG_ISP_EE_1A0 {
	uint32_t raw;
	struct {
		uint32_t EE_CHROMA_AMP_LUT_OUT_0         : 8;
		uint32_t EE_CHROMA_AMP_LUT_OUT_1         : 8;
		uint32_t EE_CHROMA_AMP_LUT_OUT_2         : 8;
		uint32_t EE_CHROMA_AMP_LUT_OUT_3         : 8;
	} bits;
};

union REG_ISP_EE_1A4 {
	uint32_t raw;
	struct {
		uint32_t EE_CHROMA_AMP_LUT_SLOPE_0       : 12;
		uint32_t _rsv_12                         : 4;
		uint32_t EE_CHROMA_AMP_LUT_SLOPE_1       : 12;
	} bits;
};

union REG_ISP_EE_1A8 {
	uint32_t raw;
	struct {
		uint32_t EE_CHROMA_AMP_LUT_SLOPE_2       : 12;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_ISP_TOP_INT_EVENT0 {
	uint32_t raw;
	struct {
		uint32_t FRAME_DONE_FE0                  : 4;
		uint32_t FRAME_DONE_FE1                  : 4;
		uint32_t FRAME_DONE_FE2                  : 2;
		uint32_t FRAME_DONE_BE                   : 2;
		uint32_t FRAME_DONE_RAW                  : 1;
		uint32_t FRAME_DONE_RGB                  : 1;
		uint32_t FRAME_DONE_YUV                  : 1;
		uint32_t FRAME_DONE_POST                 : 1;
		uint32_t SHAW_DONE_FE0                   : 4;
		uint32_t SHAW_DONE_FE1                   : 4;
		uint32_t SHAW_DONE_FE2                   : 2;
		uint32_t SHAW_DONE_BE                    : 2;
		uint32_t SHAW_DONE_RAW                   : 1;
		uint32_t SHAW_DONE_RGB                   : 1;
		uint32_t SHAW_DONE_YUV                   : 1;
		uint32_t SHAW_DONE_POST                  : 1;
	} bits;
};

union REG_ISP_TOP_INT_EVENT1 {
	uint32_t raw;
	struct {
		uint32_t PQ_DONE_FE0                     : 4;
		uint32_t PQ_DONE_FE1                     : 4;
		uint32_t PQ_DONE_FE2                     : 2;
		uint32_t PQ_DONE_BE                      : 2;
		uint32_t PQ_DONE_RAW                     : 1;
		uint32_t PQ_DONE_RGB                     : 1;
		uint32_t PQ_DONE_YUV                     : 1;
		uint32_t PQ_DONE_POST                    : 1;
	} bits;
};

union REG_ISP_TOP_INT_EVENT2 {
	uint32_t raw;
	struct {
		uint32_t FRAME_START_FE0                 : 4;
		uint32_t FRAME_START_FE1                 : 4;
		uint32_t FRAME_START_FE2                 : 2;
		uint32_t FRAME_ERR                       : 1;
		uint32_t PCHK_ERR                        : 1;
		uint32_t CMDQ_INT                        : 1;
		uint32_t LINE_INTP_FE0                   : 1;
		uint32_t LINE_INTP_FE1                   : 1;
		uint32_t LINE_INTP_FE2                   : 1;
		uint32_t LINE_INTP_POST                  : 1;
		uint32_t INT_BDG0_LITE                   : 1;
		uint32_t _rsv_18                         : 1;
		uint32_t INT_DMA_ERR                     : 1;
		uint32_t FRAME_ERR_LINE_SPLITER_FE0      : 1;
		uint32_t FRAME_ERR_LINE_SPLITER_FE1      : 1;
	} bits;
};

union REG_ISP_TOP_ERROR_STS {
	uint32_t raw;
	struct {
		uint32_t PCHK0_ERR_FE0                   : 1;
		uint32_t PCHK0_ERR_FE1                   : 1;
		uint32_t PCHK0_ERR_FE2                   : 1;
		uint32_t PCHK0_ERR_BE                    : 1;
		uint32_t PCHK0_ERR_RAW                   : 1;
		uint32_t PCHK0_ERR_RGB                   : 1;
		uint32_t PCHK0_ERR_YUV                   : 1;
		uint32_t PCHK1_ERR_FE0                   : 1;
		uint32_t PCHK1_ERR_FE1                   : 1;
		uint32_t PCHK1_ERR_FE2                   : 1;
		uint32_t PCHK1_ERR_BE                    : 1;
		uint32_t PCHK1_ERR_RAW                   : 1;
		uint32_t PCHK1_ERR_RGB                   : 1;
		uint32_t PCHK1_ERR_YUV                   : 1;
	} bits;
};

union REG_ISP_TOP_INT_EVENT0_EN {
	uint32_t raw;
	struct {
		uint32_t FRAME_DONE_ENABLE_FE0           : 4;
		uint32_t FRAME_DONE_ENABLE_FE1           : 4;
		uint32_t FRAME_DONE_ENABLE_FE2           : 2;
		uint32_t FRAME_DONE_ENABLE_BE            : 2;
		uint32_t FRAME_DONE_ENABLE_RAW           : 1;
		uint32_t FRAME_DONE_ENABLE_RGB           : 1;
		uint32_t FRAME_DONE_ENABLE_YUV           : 1;
		uint32_t FRAME_DONE_ENABLE_POST          : 1;
		uint32_t SHAW_DONE_ENABLE_FE0            : 4;
		uint32_t SHAW_DONE_ENABLE_FE1            : 4;
		uint32_t SHAW_DONE_ENABLE_FE2            : 2;
		uint32_t SHAW_DONE_ENABLE_BE             : 2;
		uint32_t SHAW_DONE_ENABLE_RAW            : 1;
		uint32_t SHAW_DONE_ENABLE_RGB            : 1;
		uint32_t SHAW_DONE_ENABLE_YUV            : 1;
		uint32_t SHAW_DONE_ENABLE_POST           : 1;
	} bits;
};

union REG_ISP_TOP_INT_EVENT1_EN {
	uint32_t raw;
	struct {
		uint32_t PQ_DONE_ENABLE_FE0              : 4;
		uint32_t PQ_DONE_ENABLE_FE1              : 4;
		uint32_t PQ_DONE_ENABLE_FE2              : 2;
		uint32_t PQ_DONE_ENABLE_BE               : 2;
		uint32_t PQ_DONE_ENABLE_RAW              : 1;
		uint32_t PQ_DONE_ENABLE_RGB              : 1;
		uint32_t PQ_DONE_ENABLE_YUV              : 1;
		uint32_t PQ_DONE_ENABLE_POST             : 1;
	} bits;
};

union REG_ISP_TOP_INT_EVENT2_EN {
	uint32_t raw;
	struct {
		uint32_t FRAME_START_ENABLE_FE0          : 4;
		uint32_t FRAME_START_ENABLE_FE1          : 4;
		uint32_t FRAME_START_ENABLE_FE2          : 2;
		uint32_t FRAME_ERR_ENABLE                : 1;
		uint32_t PCHK_ERR_ENABLE                 : 1;
		uint32_t CMDQ_INT_ENABLE                 : 1;
		uint32_t LINE_INTP_ENABLE_FE0            : 1;
		uint32_t LINE_INTP_ENABLE_FE1            : 1;
		uint32_t LINE_INTP_ENABLE_FE2            : 1;
		uint32_t LINE_INTP_ENABLE_POST           : 1;
		uint32_t INT_BDG0_LITE_ENABLE            : 1;
		uint32_t _rsv_18                         : 1;
		uint32_t INT_DMA_ERR_ENABLE              : 1;
		uint32_t FRAME_ERR_LINE_SPLITER_ENABLE_FE0: 1;
		uint32_t FRAME_ERR_LINE_SPLITER_ENABLE_FE1: 1;
	} bits;
};

union REG_ISP_TOP_SW_CTRL_0 {
	uint32_t raw;
	struct {
		uint32_t TRIG_STR_FE0                    : 4;
		uint32_t TRIG_STR_FE1                    : 4;
		uint32_t TRIG_STR_FE2                    : 2;
		uint32_t TRIG_STR_BE                     : 2;
		uint32_t TRIG_STR_RAW                    : 1;
		uint32_t TRIG_STR_POST                   : 1;
		uint32_t _rsv_14                         : 2;
		uint32_t SHAW_UP_FE0                     : 4;
		uint32_t SHAW_UP_FE1                     : 4;
		uint32_t SHAW_UP_FE2                     : 2;
		uint32_t SHAW_UP_BE                      : 2;
		uint32_t SHAW_UP_RAW                     : 1;
		uint32_t SHAW_UP_POST                    : 1;
	} bits;
};

union REG_ISP_TOP_SW_CTRL_1 {
	uint32_t raw;
	struct {
		uint32_t PQ_UP_FE0                       : 4;
		uint32_t PQ_UP_FE1                       : 4;
		uint32_t PQ_UP_FE2                       : 2;
		uint32_t PQ_UP_BE                        : 2;
		uint32_t PQ_UP_RAW                       : 1;
		uint32_t PQ_UP_POST                      : 1;
	} bits;
};

union REG_ISP_TOP_CTRL_MODE_SEL0 {
	uint32_t raw;
	struct {
		uint32_t TRIG_STR_SEL_FE0                : 4;
		uint32_t TRIG_STR_SEL_FE1                : 4;
		uint32_t TRIG_STR_SEL_FE2                : 2;
		uint32_t TRIG_STR_SEL_BE                 : 2;
		uint32_t TRIG_STR_SEL_RAW                : 1;
		uint32_t TRIG_STR_SEL_POST               : 1;
		uint32_t _rsv_14                         : 2;
		uint32_t SHAW_UP_SEL_FE0                 : 4;
		uint32_t SHAW_UP_SEL_FE1                 : 4;
		uint32_t SHAW_UP_SEL_FE2                 : 2;
		uint32_t SHAW_UP_SEL_BE                  : 2;
		uint32_t SHAW_UP_SEL_RAW                 : 1;
		uint32_t SHAW_UP_SEL_POST                : 1;
	} bits;
};

union REG_ISP_TOP_CTRL_MODE_SEL1 {
	uint32_t raw;
	struct {
		uint32_t PQ_UP_SEL_FE0                   : 4;
		uint32_t PQ_UP_SEL_FE1                   : 4;
		uint32_t PQ_UP_SEL_FE2                   : 2;
		uint32_t PQ_UP_SEL_BE                    : 2;
		uint32_t PQ_UP_SEL_RAW                   : 1;
		uint32_t PQ_UP_SEL_POST                  : 1;
	} bits;
};

union REG_ISP_TOP_SCENARIOS_CTRL {
	uint32_t raw;
	struct {
		uint32_t DST2SC                          : 1;
		uint32_t DST2DMA                         : 1;
		uint32_t PRE2BE_L_ENABLE                 : 1;
		uint32_t PRE2BE_S_ENABLE                 : 1;
		uint32_t PRE2YUV_422_ENABLE              : 1;
		uint32_t BE2RAW_L_ENABLE                 : 1;
		uint32_t BE2RAW_S_ENABLE                 : 1;
		uint32_t BE_RDMA_L_ENABLE                : 1;
		uint32_t BE_RDMA_S_ENABLE                : 1;
		uint32_t BE_WDMA_L_ENABLE                : 1;
		uint32_t BE_WDMA_S_ENABLE                : 1;
		uint32_t MULTI_SENSOR_ENABLE             : 1;
		uint32_t _rsv_12                         : 5;
		uint32_t HDR_ENABLE                      : 1;
		uint32_t HW_AUTO_ENABLE                  : 1;
		uint32_t HW_AUTO_ISO                     : 2;
		uint32_t _rsv_21                         : 1;
		uint32_t FE_DMA_SHARE_ENABLE             : 1;
		uint32_t FE2_DMA_SHARE_ENABLE            : 1;
		uint32_t BE_SRC_SEL                      : 3;
		uint32_t YUV_EDGE_EN                     : 1;
		uint32_t YUV_IN_SEL_MODE                 : 2;
		uint32_t YUV_FORMAT                      : 1;
		uint32_t YUV420_REPLAY_ENABLE            : 1;
	} bits;
};

union REG_ISP_TOP_SW_RST {
	uint32_t raw;
	struct {
		uint32_t ISP_RST                         : 1;
		uint32_t CSI0_RST                        : 1;
		uint32_t CSI1_RST                        : 1;
		uint32_t CSI_BE_RST                      : 1;
		uint32_t CSI2_RST                        : 1;
		uint32_t BDG0_LITE_RST                   : 1;
		uint32_t _rsv_6                          : 1;
		uint32_t AXI_RST                         : 1;
		uint32_t CMDQ_RST                        : 1;
		uint32_t APB_RST                         : 1;
		uint32_t RAW_RST                         : 1;
	} bits;
};

union REG_ISP_TOP_BLK_IDLE {
	uint32_t raw;
	struct {
		uint32_t FE0_BLK_IDLE                    : 1;
		uint32_t FE1_BLK_IDLE                    : 1;
		uint32_t FE2_BLK_IDLE                    : 1;
		uint32_t BE_BLK_IDLE                     : 1;
		uint32_t RAW_BLK_IDLE                    : 1;
		uint32_t RGB_BLK_IDLE                    : 1;
		uint32_t YUV_BLK_IDLE                    : 1;
		uint32_t RDMA0_IDLE                      : 1;
		uint32_t WDMA0_IDLE                      : 1;
		uint32_t WDMA1_IDLE                      : 1;
	} bits;
};

union REG_ISP_TOP_BLK_IDLE_ENABLE {
	uint32_t raw;
	struct {
		uint32_t BLK_IDLE_CSI0_EN                : 1;
		uint32_t BLK_IDLE_CSI1_EN                : 1;
		uint32_t BLK_IDLE_CSI2_EN                : 1;
		uint32_t BLK_IDLE_BDG0_LITE_EN           : 1;
		uint32_t _rsv_4                          : 1;
		uint32_t BLK_IDLE_BE_EN                  : 1;
		uint32_t BLK_IDLE_POST_EN                : 1;
		uint32_t BLK_IDLE_APB_EN                 : 1;
		uint32_t BLK_IDLE_AXI_EN                 : 1;
		uint32_t BLK_IDLE_CMDQ_EN                : 1;
		uint32_t BLK_IDLE_RAW_EN                 : 1;
		uint32_t BLK_IDLE_RGB_EN                 : 1;
		uint32_t BLK_IDLE_YUV_EN                 : 1;
	} bits;
};

union REG_ISP_TOP_DBUS0 {
	uint32_t raw;
	struct {
		uint32_t DBUS0                           : 32;
	} bits;
};

union REG_ISP_TOP_DBUS1 {
	uint32_t raw;
	struct {
		uint32_t DBUS1                           : 32;
	} bits;
};

union REG_ISP_TOP_DBUS2 {
	uint32_t raw;
	struct {
		uint32_t DBUS2                           : 32;
	} bits;
};

union REG_ISP_TOP_DBUS3 {
	uint32_t raw;
	struct {
		uint32_t DBUS3                           : 32;
	} bits;
};

union REG_ISP_TOP_FORCE_INT {
	uint32_t raw;
	struct {
		uint32_t FORCE_ISP_INT                   : 1;
		uint32_t FORCE_ISP_INT_EN                : 1;
	} bits;
};

union REG_ISP_TOP_DUMMY {
	uint32_t raw;
	struct {
		uint32_t DUMMY                           : 28;
		uint32_t DBUS_SEL                        : 4;
	} bits;
};

union REG_ISP_TOP_DBUS4 {
	uint32_t raw;
	struct {
		uint32_t DBUS4                           : 32;
	} bits;
};

union REG_ISP_TOP_IP_ENABLE1 {
	uint32_t raw;
	struct {
		uint32_t RAW_CROP_L_ENABLE               : 1;
		uint32_t RAW_CROP_S_ENABLE               : 1;
		uint32_t RAW_BNR_ENABLE                  : 1;
		uint32_t RAW_BNR_LITE_ENABLE             : 1;
		uint32_t RAW_CFA_ENABLE                  : 1;
		uint32_t RAW_LLSC_ENABLE                 : 1;
		uint32_t RAW_CLSC_ENABLE                 : 1;
		uint32_t RAW_BLC_0_ENABLE                : 1;
		uint32_t RAW_BLC_1_ENABLE                : 1;
		uint32_t RAW_FUSION_ENABLE               : 1;
		uint32_t RAW_MAP_CURVE_ENABLE            : 1;
		uint32_t RAW_AE_1_ENABLE                 : 1;
		uint32_t RAW_AF_ENABLE                   : 1;
		uint32_t RAW_GMS_ENABLE                  : 1;
		uint32_t RAW_DPC_ENABLE                  : 1;
		uint32_t RAW_DRC_ENABLE                  : 1;
		uint32_t _rsv_16                         : 4;
		uint32_t PFR_ENABLE                      : 1;
		uint32_t RGB_CCM_ENABLE                  : 1;
		uint32_t RGB_GAMMA_ENABLE                : 1;
		uint32_t RGB_DHZ_ENABLE                  : 1;
		uint32_t RGB_RGBDITHER_ENABLE            : 1;
		uint32_t RGB_CLUT_ENABLE                 : 1;
		uint32_t RGB_R2Y4_ENABLE                 : 1;
		uint32_t SRAM_EE_EXT_ENABLE              : 1;
		uint32_t SRAM_PFR_ENABLE                 : 1;
		uint32_t EE_EXT_ENABLE                   : 1;
		uint32_t RAW_AE_0_ENABLE                 : 2;
	} bits;
};

union REG_ISP_TOP_IP_ENABLE2 {
	uint32_t raw;
	struct {
		uint32_t YUV_EE_F_ENABLE                 : 1;
		uint32_t YUV_EE_B_ENABLE                 : 1;
		uint32_t YUV_3DNR_ENABLE                 : 1;
		uint32_t YUV_CNR_ENABLE                  : 1;
		uint32_t YUV_POSTEE_ENABLE               : 1;
		uint32_t YUV_YCROP_ENABLE                : 1;
		uint32_t YUV_UVCROP_ENABLE               : 1;
		uint32_t YUV_YCURVE_ENABLE               : 1;
		uint32_t YUV_CA_LITE_ENABLE              : 1;
		uint32_t YUV_CA_ENABLE                   : 1;
		uint32_t YUV_LDCI_ENABLE                 : 1;
	} bits;
};

union REG_ISP_TOP_IP_ENABLE3 {
	uint32_t raw;
	struct {
		uint32_t CSI0_AE_L_ENABLE                : 1;
		uint32_t CSI0_AE_S_ENABLE                : 1;
		uint32_t CSI0_CLSC_L_ENABLE              : 1;
		uint32_t CSI0_CLSC_S_ENABLE              : 1;
		uint32_t CSI1_AE_L_ENABLE                : 1;
		uint32_t CSI1_AE_S_ENABLE                : 1;
		uint32_t CSI1_CLSC_L_ENABLE              : 1;
		uint32_t CSI1_CLSC_S_ENABLE              : 1;
		uint32_t CSI2_AE_L_ENABLE                : 1;
		uint32_t CSI2_AE_S_ENABLE                : 1;
		uint32_t CSI2_CLSC_L_ENABLE              : 1;
		uint32_t CSI2_CLSC_S_ENABLE              : 1;
	} bits;
};

union REG_ISP_TOP_CMDQ_CTRL {
	uint32_t raw;
	struct {
		uint32_t CMDQ_TSK_EN                     : 8;
		uint32_t CMDQ_FLAG_SEL                   : 2;
		uint32_t CMDQ_TASK_SEL                   : 2;
	} bits;
};

union REG_ISP_TOP_CMDQ_TRIG {
	uint32_t raw;
	struct {
		uint32_t CMDQ_TSK_TRIG                   : 8;
	} bits;
};

union REG_ISP_TOP_TRIG_CNT {
	uint32_t raw;
	struct {
		uint32_t TRIG_STR_CNT                    : 4;
		uint32_t VSYNC_DELAY                     : 4;
	} bits;
};

union REG_ISP_TOP_SVN_VERSION {
	uint32_t raw;
	struct {
		uint32_t SVN_REVISION                    : 32;
	} bits;
};

union REG_ISP_TOP_TIMESTAMP {
	uint32_t raw;
	struct {
		uint32_t UNIX_TIMESTAMP                  : 32;
	} bits;
};

union REG_ISP_TOP_SCLIE_ENABLE {
	uint32_t raw;
	struct {
		uint32_t SLICE_ENABLE_MAIN_LEXP          : 1;
		uint32_t SLICE_ENABLE_MAIN_SEXP          : 1;
	} bits;
};

union REG_ISP_TOP_W_SLICE_THRESH_MAIN {
	uint32_t raw;
	struct {
		uint32_t W_SLICE_THR_MAIN_LEXP           : 16;
		uint32_t W_SLICE_THR_MAIN_SEXP           : 16;
	} bits;
};

union REG_ISP_TOP_R_SLICE_THRESH_MAIN {
	uint32_t raw;
	struct {
		uint32_t R_SLICE_THR_MAIN_LEXP           : 16;
		uint32_t R_SLICE_THR_MAIN_SEXP           : 16;
	} bits;
};

union REG_ISP_TOP_VI_SEL_FRAME_VALID {
	uint32_t raw;
	struct {
		uint32_t VI_SEL_FRAME_VLD_LE             : 1;
		uint32_t VI_SEL_PQ_VLD_LE                : 1;
		uint32_t VI_SEL_FRAME_VLD_SE             : 1;
		uint32_t VI_SEL_PQ_VLD_SE                : 1;
	} bits;
};

union REG_ISP_TOP_FIRST_FRAME {
	uint32_t raw;
	struct {
		uint32_t FIRST_FRAME_SW                  : 4;
		uint32_t FIRST_FRAME_TOP                 : 4;
	} bits;
};

union REG_ISP_TOP_INT_EVENT0_LINE_SPLITER {
	uint32_t raw;
	struct {
		uint32_t LINE_SPLITER_DMA_DONE_FE0       : 4;
		uint32_t LINE_SPLITER_DMA_DONE_FE1       : 4;
	} bits;
};

union REG_ISP_TOP_INT_EVENT0_EN_LINE_SPLITER {
	uint32_t raw;
	struct {
		uint32_t LINE_SPLITER_DMA_DONE_ENABLE_FE0: 4;
		uint32_t LINE_SPLITER_DMA_DONE_ENABLE_FE1: 4;
	} bits;
};

union REG_ISP_TOP_BLK_IDLE_1 {
	uint32_t raw;
	struct {
		uint32_t RDMA1_IDLE                      : 1;
		uint32_t WDMA2_IDLE                      : 1;
		uint32_t RDMA2_IDLE                      : 1;
		uint32_t LINE_SPLITER_FE0_DMA_IDLE       : 4;
		uint32_t LINE_SPLITER_FE1_DMA_IDLE       : 4;
		uint32_t LINE_SPLITER_FE0_FIFO_OVERFLOW_STATUS: 1;
		uint32_t LINE_SPLITER_FE1_FIFO_OVERFLOW_STATUS: 1;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_RDMA_CORE_SHADOW_RD_SEL   {
	uint32_t raw;
	struct {
		uint32_t SHADOW_RD_SEL                   : 1;
		uint32_t ABORT_MODE                      : 1;
		uint32_t _rsv_2                          : 6;
		uint32_t MAX_OSTD                        : 8;
		uint32_t OSTD_SW_EN                      : 1;
	} bits;
};

union REG_RDMA_CORE_IP_DISABLE {
	uint32_t raw;
	struct {
		uint32_t IP_DISABLE                      : 32;
	} bits;
};

union REG_RDMA_CORE_UP_RING_BASE {
	uint32_t raw;
	struct {
		uint32_t UP_RING_BASE                    : 32;
	} bits;
};

union REG_RDMA_CORE_NORM_STATUS0 {
	uint32_t raw;
	struct {
		uint32_t ABORT_DONE                      : 1;
		uint32_t _rsv_1                          : 3;
		uint32_t ERROR_AXI                       : 1;
		uint32_t _rsv_5                          : 3;
		uint32_t ERROR_ID                        : 5;
		uint32_t _rsv_13                         : 3;
		uint32_t DMA_VERSION                     : 16;
	} bits;
};

union REG_RDMA_CORE_NORM_STATUS1 {
	uint32_t raw;
	struct {
		uint32_t ID_IDLE                         : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_EN {
	uint32_t raw;
	struct {
		uint32_t RING_ENABLE                     : 32;
	} bits;
};

union REG_RDMA_CORE_NORM_PERF  {
	uint32_t raw;
	struct {
		uint32_t BWLWIN                          : 10;
		uint32_t BWLTXN                          : 6;
	} bits;
};

union REG_RDMA_CORE_AR_PRIORITY_SEL {
	uint32_t raw;
	struct {
		uint32_t AR_PRIORITY_SEL                 : 1;
		uint32_t QOS_PRIORITY_SEL                : 1;
		uint32_t ARB_HIST_DISABLE                : 1;
		uint32_t _rsv_3                          : 1;
		uint32_t ARB_USAGE_TH                    : 4;
	} bits;
};

union REG_RDMA_CORE_RING_PATCH_ENABLE {
	uint32_t raw;
	struct {
		uint32_t RING_PATCH_ENABLE               : 32;
	} bits;
};

union REG_RDMA_CORE_SET_RING_BASE {
	uint32_t raw;
	struct {
		uint32_t SET_RING_BASE                   : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BASE_ADDR_L {
	uint32_t raw;
	struct {
		uint32_t RING_BASE_L                     : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BASE_ADDR_H {
	uint32_t raw;
	struct {
		uint32_t RING_BASE_H                     : 16;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE0 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE0                      : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE1 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE1                      : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE2 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE2                      : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE3 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE3                      : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE4 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE4                      : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE5 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE5                      : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE6 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE6                      : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE7 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE7                      : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE8 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE8                      : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE9 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE9                      : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE10 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE10                     : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE11 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE11                     : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE12 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE12                     : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE13 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE13                     : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE14 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE14                     : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE15 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE15                     : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE16 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE16                     : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE17 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE17                     : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE18 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE18                     : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE19 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE19                     : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE20 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE20                     : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE21 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE21                     : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE22 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE22                     : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE23 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE23                     : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE24 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE24                     : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE25 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE25                     : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE26 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE26                     : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE27 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE27                     : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE28 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE28                     : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE29 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE29                     : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE30 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE30                     : 32;
	} bits;
};

union REG_RDMA_CORE_RING_BUFFER_SIZE31 {
	uint32_t raw;
	struct {
		uint32_t RBUF_SIZE31                     : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS0 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR0                  : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS1 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR1                  : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS2 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR2                  : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS3 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR3                  : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS4 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR4                  : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS5 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR5                  : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS6 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR6                  : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS7 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR7                  : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS8 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR8                  : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS9 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR9                  : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS10 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR10                 : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS11 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR11                 : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS12 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR12                 : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS13 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR13                 : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS14 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR14                 : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS15 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR15                 : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS16 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR16                 : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS17 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR17                 : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS18 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR18                 : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS19 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR19                 : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS20 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR20                 : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS21 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR21                 : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS22 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR22                 : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS23 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR23                 : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS24 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR24                 : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS25 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR25                 : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS26 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR26                 : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS27 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR27                 : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS28 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR28                 : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS29 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR29                 : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS30 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR30                 : 32;
	} bits;
};

union REG_RDMA_CORE_NEXT_DMA_ADDR_STS31 {
	uint32_t raw;
	struct {
		uint32_t NEXT_DMA_ADDR31                 : 32;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_PRE_RAW_VI_SEL_0 {
	uint32_t raw;
	struct {
		uint32_t RING_BUFF_RESET                 : 1;
		uint32_t RING_BUFF_MONITOR_EN            : 1;
		uint32_t DMA_LD_DPCM_MODE                : 3;
		uint32_t CSI_IN_FORMAT                   : 1;
		uint32_t YUV_SWAP_EN                     : 1;
		uint32_t _rsv_7                          : 9;
		uint32_t DPCM_RX_XSTR                    : 13;
	} bits;
};

union REG_PRE_RAW_VI_SEL_1 {
	uint32_t raw;
	struct {
		uint32_t FRAME_WIDTHM1                   : 14;
		uint32_t _rsv_14                         : 2;
		uint32_t FRAME_HEIGHTM1                  : 14;
	} bits;
};

union REG_PRE_RAW_VI_SEL_2 {
	uint32_t raw;
	struct {
		uint32_t RING_BUFF_THRESHOLD_0           : 16;
		uint32_t RING_BUFF_THRESHOLD_1           : 16;
	} bits;
};

union REG_PRE_RAW_VI_SEL_3 {
	uint32_t raw;
	struct {
		uint32_t RING_BUFF_THRESHOLD_2           : 16;
		uint32_t RING_BUFF_THRESHOLD_3           : 16;
	} bits;
};

union REG_PRE_RAW_VI_SEL_4 {
	uint32_t raw;
	struct {
		uint32_t RING_BUFF_ABOVE_TH_0            : 32;
	} bits;
};

union REG_PRE_RAW_VI_SEL_5 {
	uint32_t raw;
	struct {
		uint32_t RING_BUFF_ABOVE_TH_1            : 32;
	} bits;
};

union REG_PRE_RAW_VI_SEL_6 {
	uint32_t raw;
	struct {
		uint32_t RING_BUFF_ABOVE_TH_2            : 32;
	} bits;
};

union REG_PRE_RAW_VI_SEL_7 {
	uint32_t raw;
	struct {
		uint32_t RING_BUFF_ABOVE_TH_3            : 32;
	} bits;
};

union REG_PRE_RAW_VI_SEL_8 {
	uint32_t raw;
	struct {
		uint32_t IP_DMA_IDLE                     : 32;
	} bits;
};

union REG_PRE_RAW_VI_SEL_9 {
	uint32_t raw;
	struct {
		uint32_t CROP_ENABLE_LE                  : 1;
		uint32_t CROP_ENABLE_SE                  : 1;
		uint32_t CROP_ENABLE_YUV                 : 1;
	} bits;
};

union REG_PRE_RAW_VI_SEL_10 {
	uint32_t raw;
	struct {
		uint32_t VI_SEL_SHADOW_SEL               : 1;
	} bits;
};

union REG_PRE_RAW_VI_SEL_11 {
	uint32_t raw;
	struct {
		uint32_t AI_ISP_TRANSFORM_EN             : 1;
		uint32_t BAYER_TYPE                      : 2;
		uint32_t BYPASS_LE                       : 1;
		uint32_t BYPASS_SE                       : 1;
		uint32_t HW_AUTO_CG_EN_LE                : 1;
		uint32_t HW_AUTO_CG_EN_SE                : 1;
		uint32_t IN_FORMAT_LE                    : 2;
		uint32_t IN_FORMAT_SE                    : 2;
		uint32_t ROUND_MODE_LE                   : 3;
		uint32_t ROUND_MODE_SE                   : 3;
	} bits;
};

union REG_PRE_RAW_VI_SEL_12 {
	uint32_t raw;
	struct {
		uint32_t GUARD_CNT_LE                    : 8;
		uint32_t GUARD_CNT_SE                    : 8;
	} bits;
};

union REG_PRE_RAW_VI_SEL_13 {
	uint32_t raw;
	struct {
		uint32_t AI_ISP_DEBUG_STATUS_LE          : 32;
	} bits;
};

union REG_PRE_RAW_VI_SEL_14 {
	uint32_t raw;
	struct {
		uint32_t AI_ISP_DEBUG_STATUS_SE          : 32;
	} bits;
};

/******************************************/
/*           Module Definition            */
/******************************************/
union REG_CROP_0 {
	uint32_t raw;
	struct {
		uint32_t CROP_ENABLE                     : 1;
		uint32_t DMA_ENABLE                      : 1;
		uint32_t SHAW_READ_SEL                   : 1;
		uint32_t DMI_QOS                         : 1;
		uint32_t _rsv_4                          : 4;
		uint32_t DPCM_MODE                       : 3;
		uint32_t _rsv_11                         : 5;
		uint32_t DPCM_XSTR                       : 13;
		uint32_t _rsv_29                         : 2;
		uint32_t DMI16B_EN                       : 1;
	} bits;
};

union REG_CROP_1 {
	uint32_t raw;
	struct {
		uint32_t CROP_START_Y                    : 14;
		uint32_t _rsv_13                         : 2;
		uint32_t CROP_END_Y                      : 14;
	} bits;
};

union REG_CROP_2 {
	uint32_t raw;
	struct {
		uint32_t CROP_START_X                    : 14;
		uint32_t _rsv_13                         : 2;
		uint32_t CROP_END_X                      : 14;
	} bits;
};

union REG_CROP_3 {
	uint32_t raw;
	struct {
		uint32_t IN_WIDTHM1                      : 16;
		uint32_t IN_HEIGHTM1                     : 16;
	} bits;
};

union REG_CROP_DUMMY {
	uint32_t raw;
	struct {
		uint32_t DUMMY                           : 32;
	} bits;
};

union REG_CROP_DEBUG {
	uint32_t raw;
	struct {
		uint32_t FORCE_CLK_ENABLE                : 1;
		uint32_t FORCE_DMA_DISABLE               : 1;
	} bits;
};

#ifdef __cplusplus
}
#endif

#endif /* _VI_REG_FIELDS_H_ */