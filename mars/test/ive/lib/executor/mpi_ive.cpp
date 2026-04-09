#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi_ive.h"
#include "cvi_ive_hw.h"

#include "cvi_mpi_ive.h"

CVI_S32 CVI_MPI_IVE_Add(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc1,
	IVE_SRC_IMAGE_S *pstSrc2, IVE_DST_IMAGE_S *pstDst, IVE_ADD_CTRL_S *pstAddCtrl, CVI_BOOL bInstant)
{
	printf("CVI_MPI_IVE_Add\n");

    IVEReg _iveReg(pIveHandle, bInstant, pstSrc1->u32Height, pstSrc1->u32Width);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableTop();
    // align isp
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstSrc1->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstSrc1->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;

    // setting
    // FIXME: check reg_frame2op_op_mode/reg_frame2op_fg_op_mode settings
    iveReg->ive_top_c->REG_20.reg_frame2op_op_mode = 3;
    // "2 frame operation_mode 3'd0: And 3'd1: Or 3'd2: Xor 3'd3: Add 3'd4: Sub 3'd5: Bypass mode output =frame source 0 3'd6: Bypass mode output =frame source 1 default: And"
    iveReg->ive_top_c->REG_h80.reg_frame2op_fg_op_mode = 3;

    // TODO: test that set rounding and clip
    iveReg->ive_top_c->REG_20.reg_frame2op_add_mode_rounding = 1;
    iveReg->ive_top_c->REG_20.reg_frame2op_add_mode_clipping = 1;
    iveReg->ive_top_c->REG_21.reg_fram2op_x_u0q16 = (CVI_U0Q16)pstAddCtrl->u0q16X;
    iveReg->ive_top_c->REG_21.reg_fram2op_y_u0q16 = (CVI_U0Q16)pstAddCtrl->u0q16Y;

    // TODO: set dma
    iveReg->srcEnType = pstSrc1->enType;
    iveReg->dstEnType = pstDst->enType;
    iveReg->setDMA(pstSrc1, pstDst, pstSrc2);

    return CVI_MPI_IVE_Add(iveReg);
}

CVI_S32 CVI_MPI_IVE_And(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc1,
	IVE_SRC_IMAGE_S *pstSrc2, IVE_DST_IMAGE_S *pstDst, CVI_BOOL bInstant)
{
	printf("CVI_MPI_IVE_And\n");

    IVEReg _iveReg(pIveHandle, bInstant, pstSrc1->u32Height, pstSrc1->u32Width);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableTop();
    // align isp
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstSrc1->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstSrc1->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;
    // setting
    // FIXME: check reg_frame2op_op_mode/reg_frame2op_fg_op_mode settings
    iveReg->ive_top_c->REG_20.reg_frame2op_op_mode = 0;
    iveReg->ive_top_c->REG_h80.reg_frame2op_fg_op_mode = 0;

    // TODO: set dma
    iveReg->srcEnType = pstSrc1->enType;
    iveReg->dstEnType = pstDst->enType;
    iveReg->setDMA(pstSrc1, pstDst, pstSrc2);

    return CVI_MPI_IVE_And(iveReg);
}

CVI_S32 CVI_MPI_IVE_Or(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc1,
	IVE_SRC_IMAGE_S *pstSrc2, IVE_DST_IMAGE_S *pstDst, CVI_BOOL bInstant)
{
	printf("CVI_MPI_IVE_Or\n");

    IVEReg _iveReg(pIveHandle, bInstant, pstSrc1->u32Height, pstSrc1->u32Width);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableTop();
    // align isp
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstSrc1->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstSrc1->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;
    // setting
    // FIXME: check reg_frame2op_op_mode/reg_frame2op_fg_op_mode settings
    iveReg->ive_top_c->REG_20.reg_frame2op_op_mode = 1;
    iveReg->ive_top_c->REG_h80.reg_frame2op_fg_op_mode = 1;

    // TODO: set dma
    iveReg->srcEnType = pstSrc1->enType;
    iveReg->dstEnType = pstDst->enType;
    iveReg->setDMA(pstSrc1, pstDst, pstSrc2);

    return CVI_MPI_IVE_Or(iveReg);

}

CVI_S32 CVI_MPI_IVE_Sub(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc1,
  IVE_SRC_IMAGE_S *pstSrc2, IVE_DST_IMAGE_S *pstDst, IVE_SUB_CTRL_S *pstSubCtrl, CVI_BOOL bInstant)
{
	printf("CVI_MPI_IVE_Sub\n");

    IVEReg _iveReg(pIveHandle, bInstant, pstSrc1->u32Height, pstSrc1->u32Width);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableTop();
    // align isp
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstSrc1->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstSrc1->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;
    // setting
    // FIXME: check reg_frame2op_op_mode/reg_frame2op_fg_op_mode settings
    iveReg->ive_top_c->REG_20.reg_frame2op_op_mode = 4;
    iveReg->ive_top_c->REG_h80.reg_frame2op_fg_op_mode = 4;
    // TODO: need to verify?
    iveReg->ive_top_c->REG_20.reg_frame2op_sub_change_order = 0;
    iveReg->ive_top_c->REG_20.reg_frame2op_sub_switch_src = 0;

    iveReg->ive_top_c->REG_20.reg_frame2op_sub_mode = (CVI_U8)pstSubCtrl->enMode;

    // TODO: set dma
    iveReg->srcEnType = pstSrc1->enType;
    iveReg->dstEnType = pstDst->enType;
    iveReg->setDMA(pstSrc1, pstDst, pstSrc2);

    return CVI_MPI_IVE_Sub(iveReg);
}

CVI_S32 CVI_MPI_IVE_Xor(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc1,
	IVE_SRC_IMAGE_S *pstSrc2, IVE_DST_IMAGE_S *pstDst, CVI_BOOL bInstant)
{
	printf("CVI_MPI_IVE_Xor\n");

    IVEReg _iveReg(pIveHandle, bInstant, pstSrc1->u32Height, pstSrc1->u32Width);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableTop();
    // align isp
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstSrc1->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstSrc1->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;
    // setting
    // FIXME: check reg_frame2op_op_mode/reg_frame2op_fg_op_mode settings
    iveReg->ive_top_c->REG_20.reg_frame2op_op_mode = 2;
    iveReg->ive_top_c->REG_h80.reg_frame2op_fg_op_mode = 2;

    // TODO: set dma
    iveReg->srcEnType = pstSrc1->enType;
    iveReg->dstEnType = pstDst->enType;
    iveReg->setDMA(pstSrc1, pstDst, pstSrc2);

    return CVI_MPI_IVE_Xor(iveReg);
}

CVI_S32 CVI_MPI_IVE_Thresh(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
	IVE_DST_IMAGE_S *pstDst, IVE_THRESH_CTRL_S *pstThrCtrl, CVI_BOOL bInstant)
{
	printf("CVI_MPI_IVE_Thresh\n");

    IVEReg _iveReg(pIveHandle, bInstant, pstDst->u32Height, pstDst->u32Width);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableTop();

    // top
    // settign later
    //iveReg->ive_top_c->REG_h130.reg_thresh_thresh_en = bInstant;
    // align isp
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstDst->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstDst->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;
    // setting
    iveReg->ive_top_c->REG_h14c.reg_thresh_u8bit_thr_l = pstThrCtrl->u8LowThr;
    iveReg->ive_top_c->REG_h14c.reg_thresh_u8bit_thr_h = pstThrCtrl->u8HighThr;
    iveReg->ive_top_c->REG_h14c.reg_thresh_enmode      = pstThrCtrl->enMode;
    iveReg->ive_top_c->REG_h150.reg_thresh_u8bit_min   = pstThrCtrl->u8MinVal;
    iveReg->ive_top_c->REG_h150.reg_thresh_u8bit_mid   = pstThrCtrl->u8MidVal;
    iveReg->ive_top_c->REG_h150.reg_thresh_u8bit_max   = pstThrCtrl->u8MaxVal;

    // TODO: set dma
    iveReg->srcEnType = pstSrc->enType;
    iveReg->dstEnType = pstDst->enType;
    iveReg->setDMA(pstSrc, pstDst);

    return CVI_MPI_IVE_Thresh(iveReg);
}

CVI_S32 CVI_MPI_IVE_Thresh_S16(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
	IVE_DST_IMAGE_S *pstDst, IVE_THRESH_S16_CTRL_S *pstThrS16Ctrl, CVI_BOOL bInstant)
{
	printf("CVI_MPI_IVE_Thresh_S16\n");

    IVEReg _iveReg(pIveHandle, bInstant, pstDst->u32Height, pstDst->u32Width);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableTop();

    // top
    iveReg->ive_top_c->REG_h130.reg_thresh_top_mod = 2;
    // settign later
    // align isp
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstDst->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstDst->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;
    // setting
    iveReg->ive_top_c->REG_h13c.reg_thresh_s16_enmode = pstThrS16Ctrl->enMode;
    iveReg->ive_top_c->REG_h13c.reg_thresh_s16_u8bit_min = pstThrS16Ctrl->un8MinVal.u8Val;
    iveReg->ive_top_c->REG_h13c.reg_thresh_s16_u8bit_mid = pstThrS16Ctrl->un8MidVal.u8Val;
    iveReg->ive_top_c->REG_h13c.reg_thresh_s16_u8bit_max = pstThrS16Ctrl->un8MaxVal.u8Val;
    iveReg->ive_top_c->REG_h140.reg_thresh_s16_bit_thr_l = pstThrS16Ctrl->s16LowThr;
    iveReg->ive_top_c->REG_h140.reg_thresh_s16_bit_thr_h = pstThrS16Ctrl->s16HighThr;

    // TODO: set dma
    iveReg->srcEnType = pstSrc->enType;
    iveReg->dstEnType = pstDst->enType;
    iveReg->setDMA(pstSrc, pstDst);

    return CVI_MPI_IVE_Thresh_S16(iveReg);
}

CVI_S32 CVI_MPI_IVE_Thresh_U16(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
	IVE_DST_IMAGE_S *pstDst, IVE_THRESH_U16_CTRL_S *pstThrU16Ctrl, CVI_BOOL bInstant)
{
	printf("CVI_MPI_IVE_Thresh_U16\n");
    IVEReg _iveReg(pIveHandle, bInstant, pstDst->u32Height, pstDst->u32Width);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableTop();

    // top
    iveReg->ive_top_c->REG_h130.reg_thresh_top_mod = 3;
    // align isp
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstDst->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstDst->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;
    // setting
    iveReg->ive_top_c->REG_h144.reg_thresh_u16_enmode    = pstThrU16Ctrl->enMode;
    iveReg->ive_top_c->REG_h144.reg_thresh_u16_u8bit_min = pstThrU16Ctrl->u8MinVal;
    iveReg->ive_top_c->REG_h144.reg_thresh_u16_u8bit_mid = pstThrU16Ctrl->u8MidVal;
    iveReg->ive_top_c->REG_h144.reg_thresh_u16_u8bit_max = pstThrU16Ctrl->u8MaxVal;
    iveReg->ive_top_c->REG_h148.reg_thresh_u16_bit_thr_l = pstThrU16Ctrl->u16LowThr;
    iveReg->ive_top_c->REG_h148.reg_thresh_u16_bit_thr_h = pstThrU16Ctrl->u16HighThr;

    // TODO: set dma
    iveReg->srcEnType = pstSrc->enType;
    iveReg->dstEnType = pstDst->enType;
    iveReg->setDMA(pstSrc, pstDst);

    return CVI_MPI_IVE_Thresh_U16(iveReg);
}

CVI_S32 CVI_MPI_IVE_DMA(IVE_HANDLE *pIveHandle, IVE_DATA_S *pstSrc,
	IVE_DST_DATA_S *pstDst, IVE_DMA_CTRL_S *pstDmaCtrl,CVI_BOOL bInstant)
{
	printf("CVI_MPI_IVE_DMA\n");

	if (pstDst == NULL) {
		CVI_ASSERT(IVE_DMA_MODE_SET_3BYTE == pstDmaCtrl->enMode ||
					IVE_DMA_MODE_SET_8BYTE == pstDmaCtrl->enMode);
		pstDst = pstSrc;
	}

    IVEReg _iveReg(pIveHandle, bInstant, pstSrc->u32Height, pstSrc->u32Width);
    IVEReg* iveReg = &_iveReg;

    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstSrc->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstSrc->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;

    iveReg->ive_dma_c->REG_0.reg_ive_dma_mode = pstDmaCtrl->enMode;
    iveReg->ive_dma_c->REG_4.reg_ive_dma_horsegsize = pstDmaCtrl->u8HorSegSize;
    iveReg->ive_dma_c->REG_4.reg_ive_dma_versegrow = pstDmaCtrl->u8VerSegRows;
    iveReg->ive_dma_c->REG_4.reg_ive_dma_elemsize = pstDmaCtrl->u8ElemSize;
    iveReg->ive_dma_c->REG_1.reg_ive_dma_src_stride = (CVI_U16)pstSrc->u32Stride;
    iveReg->ive_dma_c->REG_1.reg_ive_dma_dst_stride = (CVI_U16)pstDst->u32Stride;
    iveReg->ive_dma_c->REG_5.reg_ive_dma_u64_val[0] = (CVI_U32)(pstDmaCtrl->u64Val & 0xffffffff);
    iveReg->ive_dma_c->REG_5.reg_ive_dma_u64_val[1] = (CVI_U32)((pstDmaCtrl->u64Val >> 32) & 0xffffffff);

    IVE_SRC_IMAGE_S _pstSrc;
    IVE_SRC_IMAGE_S _pstDst;
    for (int i = 0; i < 3; i++) {
        // avoid -Werror=uninitialized
        _pstSrc.au64PhyAddr[i] = pstSrc->u64PhyAddr;
        _pstSrc.au64VirAddr[i] = pstSrc->u64VirAddr;
        _pstDst.au64PhyAddr[i] = pstDst->u64PhyAddr;
        _pstDst.au64VirAddr[i] = pstDst->u64VirAddr;
        _pstSrc.au32Stride[i] = pstSrc->u32Stride;
        _pstDst.au32Stride[i] = pstDst->u32Stride;
    }

    iveReg->srcEnType = _pstSrc.enType;
    iveReg->dstEnType = _pstDst.enType;
    iveReg->setDMA(&_pstSrc, &_pstDst);

	return CVI_MPI_IVE_DMA(iveReg);
}

CVI_S32 CVI_MPI_IVE_16BitTo8Bit(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
	IVE_DST_IMAGE_S *pstDst, IVE_16BIT_TO_8BIT_CTRL_S *pst16BitTo8BitCtrl, CVI_BOOL bInstant)
{
    IVEReg _iveReg(pIveHandle, bInstant, pstDst->u32Height, pstDst->u32Width);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableTop();

	printf("CVI_MPI_IVE_16BitTo8Bit\n");

	CVI_U32 tmp = (((CVI_U32)pst16BitTo8BitCtrl->u8Numerator << 16) + (pst16BitTo8BitCtrl->u16Denominator >> 1))
				 / pst16BitTo8BitCtrl->u16Denominator;
	CVI_U16 u8Num_div_u16Den = (CVI_U16)(tmp & 0xffff);

    //  Abstract :  1. 16to8 / thresh_s16 /thresh_u16 + thresh
    //              reg_thresh_top_mod = 0 : bypass
    //              reg_thresh_top_mod = 1 : 16to8                   reg_thresh_thresh_en = 1 : thresh
    //              reg_thresh_top_mod = 2 : thresh_s16     +   reg_thresh_thresh_en = 0 : bypass
    //              reg_thresh_top_mod = 3 : thresh_u16"
    // top
    iveReg->ive_top_c->REG_h130.reg_thresh_top_mod = 1;
    // settign later
    //iveReg->ive_top_c->REG_h130.reg_thresh_thresh_en = bInstant;
    // TODO: need to set?
    iveReg->ive_top_c->REG_h138.reg_thresh_st_16to8_en = bInstant;
    // align isp
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstDst->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstDst->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;
    // setting
    iveReg->ive_top_c->REG_h138.reg_thresh_st_16to8_u8Numerator = pst16BitTo8BitCtrl->u8Numerator;
    iveReg->ive_top_c->REG_h134.reg_thresh_16to8_mod = pst16BitTo8BitCtrl->enMode;
    iveReg->ive_top_c->REG_h134.reg_thresh_16to8_u8Num_div_u16Den = u8Num_div_u16Den;
    iveReg->ive_top_c->REG_h134.reg_thresh_16to8_s8bias = pst16BitTo8BitCtrl->s8Bias;

    // TODO: set dma
    iveReg->srcEnType = pstSrc->enType;
    iveReg->dstEnType = pstDst->enType;
    iveReg->setDMA(pstSrc, pstDst);

    return CVI_MPI_IVE_16BitTo8Bit(iveReg);
}

static CVI_S32 _CVI_MPI_IVE_CSC(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
    IVE_DST_IMAGE_S *pstDst, IVE_CSC_CTRL_S *pstCscCtrl, CVI_BOOL bInstant,
    IVEReg* iveReg, bool isEmit)
{
    IVEReg _iveReg(pIveHandle, bInstant, pstDst->u32Height, pstDst->u32Width);
    if (!iveReg) {
        iveReg = &_iveReg;
    }
    iveReg->setEnableTop();

    if (isEmit) {
      iveReg->srcEnType = pstSrc->enType;
      iveReg->dstEnType = pstDst->enType;
      iveReg->setDMA(pstSrc, pstDst);
    }
    else {
        // NOTICE:FilterAndCSC case ONLY
    }

    // top
    // align isp
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstDst->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstDst->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;
    iveReg->ive_top_c->REG_14.reg_csc_enmode = pstCscCtrl->enMode;

    // setting

    {
        // NOTICE: hs not native support rgb2hsv/rgb2lab table by user
        if (0) {
            iveReg->ive_top_c->REG_12.reg_csc_tab_sw_update   = 1;
            //unsigned 15 bit, update table value of inv_h_tab(rgb2hsv) or xyz_tab(rgb2lab) when reg_csc_tab_sw_update == 1
            iveReg->ive_top_c->REG_11.reg_csc_tab_sw_1 = iveReg->getSrcLHSAu64VirAddr(2);

            // unsigned 12 bit, update table value of inv_v_tab(rgb2hsv) or gamma_tab(rgb2lab) when reg_csc_tab_sw_update == 1
            iveReg->ive_top_c->REG_11.reg_csc_tab_sw_0 = iveReg->getSrcLHSAu64VirAddr(2) + (6144 / 2);
        }

        switch(iveReg->ive_top_c->REG_14.reg_csc_enmode) {
            case IVE_CSC_MODE_PIC_BT601_YUV2HSV:
            case IVE_CSC_MODE_PIC_BT709_YUV2HSV:
            case IVE_CSC_MODE_PIC_BT601_YUV2LAB:
            case IVE_CSC_MODE_PIC_BT709_YUV2LAB:
                // "update rgb2hsv/rgb2lab table value by software
                //0:use const, 1:update table by reg_csc_tab_sw_0 and reg_csc_tab_sw_1"
                if (iveReg->getSrcLHSAu64VirAddr(2)) {
                    iveReg->ive_top_c->REG_12.reg_csc_tab_sw_update   = 1;
                    //unsigned 15 bit, update table value of inv_h_tab(rgb2hsv) or xyz_tab(rgb2lab) when reg_csc_tab_sw_update == 1
                    iveReg->ive_top_c->REG_11.reg_csc_tab_sw_1 = iveReg->getSrcLHSAu64VirAddr(2);

                    // unsigned 12 bit, update table value of inv_v_tab(rgb2hsv) or gamma_tab(rgb2lab) when reg_csc_tab_sw_update == 1
                    iveReg->ive_top_c->REG_11.reg_csc_tab_sw_0 = iveReg->getSrcLHSAu64VirAddr(2) + (6144 / 2);
                }
                break;
            default:
                break;
        }
    }

    {
        // default table align cmodel
        // TODO: export to user
        CVI_S32 coef_BT601_to_GBR_16_235[12] = {1024, 0, 1404, 179188, 1024, 344, 715, 136040, 1024, 1774, 0, 226505};  // 0
        CVI_S32 coef_BT709_to_GBR_16_235[12] = {1024, 0, 1577, 201339, 1024, 187, 470, 84660, 1024, 1860, 0, 237515};   // 1
        CVI_S32 coef_BT601_to_GBR_0_255[12]  = {1192, 0, 1634, 227750, 1192, 400, 833, 139252, 1192, 2066, 0, 283062};  // 2
        CVI_S32 coef_BT709_to_GBR_0_255[12]  = {1192, 0, 1836, 253571, 1192, 218, 547, 79352, 1192, 2166, 0, 295776};   // 3
        CVI_S32 coef_RGB_to_BT601_0_255[12] = {306, 601, 117, 512, 176, 347, 523, 131584, 523, 438, 85, 131584};
        CVI_S32 coef_RGB_to_BT709_0_255[12] = {218, 732, 74, 512, 120, 403, 523, 131584, 523, 475, 48, 131584};     // 9
        CVI_S32 coef_RGB_to_BT601_16_235[12] = {263, 516, 100, 16896, 152, 298, 450, 131584, 450, 377, 73, 131584}; // 10
        CVI_S32 coef_RGB_to_BT709_16_235[12] = {187, 629, 63, 16896, 103, 346, 450, 131584, 450, 409, 41, 131584};  // 11

        CVI_S32* tbl = NULL;
        switch(iveReg->ive_top_c->REG_14.reg_csc_enmode) {
            case IVE_CSC_MODE_VIDEO_BT601_YUV2RGB: 
                tbl = coef_BT601_to_GBR_16_235;
                break;
            case IVE_CSC_MODE_VIDEO_BT709_YUV2RGB:
                tbl = coef_BT709_to_GBR_16_235;
                break;
            case IVE_CSC_MODE_PIC_BT601_YUV2RGB:
                tbl = coef_BT601_to_GBR_0_255;
                break;
            case IVE_CSC_MODE_PIC_BT709_YUV2RGB:
                tbl = coef_BT709_to_GBR_0_255;
                break;
            case IVE_CSC_MODE_VIDEO_BT601_RGB2YUV:
                tbl = coef_RGB_to_BT601_0_255;
                break;
            case IVE_CSC_MODE_VIDEO_BT709_RGB2YUV:
                tbl = coef_RGB_to_BT709_0_255;
                break;
            case IVE_CSC_MODE_PIC_BT601_RGB2YUV:
                tbl = coef_RGB_to_BT601_16_235;
                break;
            case IVE_CSC_MODE_PIC_BT709_RGB2YUV:
                tbl = coef_RGB_to_BT709_16_235;
                break;
            default:
                break;
        }

        if (tbl) {
            iveReg->ive_top_c->REG_12.reg_csc_coeff_sw_update = 1;
            iveReg->ive_top_c->REG_CSC_COEFF_0.reg_csc_coeff_sw_00 = tbl[0];
            iveReg->ive_top_c->REG_CSC_COEFF_1.reg_csc_coeff_sw_01 = tbl[1];
            iveReg->ive_top_c->REG_CSC_COEFF_2.reg_csc_coeff_sw_02 = tbl[2];
            iveReg->ive_top_c->REG_CSC_COEFF_3.reg_csc_coeff_sw_03 = tbl[3];
            iveReg->ive_top_c->REG_CSC_COEFF_4.reg_csc_coeff_sw_04 = tbl[4];
            iveReg->ive_top_c->REG_CSC_COEFF_5.reg_csc_coeff_sw_05 = tbl[5];
            iveReg->ive_top_c->REG_CSC_COEFF_6.reg_csc_coeff_sw_06 = tbl[6];
            iveReg->ive_top_c->REG_CSC_COEFF_7.reg_csc_coeff_sw_07 = tbl[7];
            iveReg->ive_top_c->REG_CSC_COEFF_8.reg_csc_coeff_sw_08 = tbl[8];
            iveReg->ive_top_c->REG_CSC_COEFF_9.reg_csc_coeff_sw_09 = tbl[9];
            iveReg->ive_top_c->REG_CSC_COEFF_A.reg_csc_coeff_sw_10 = tbl[10];
            iveReg->ive_top_c->REG_CSC_COEFF_B.reg_csc_coeff_sw_11 = tbl[11];
        }
    }

    // TODO: need to set enType?
    iveReg->srcEnType = pstSrc->enType;
    iveReg->dstEnType = pstDst->enType;
	CVI_S32 r = CVI_SUCCESS;

    if (isEmit) {
        r = CVI_MPI_IVE_CSC(iveReg);
    }

    return r;
}

CVI_S32 CVI_MPI_IVE_CSC(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
    IVE_DST_IMAGE_S *pstDst, IVE_CSC_CTRL_S *pstCscCtrl, CVI_BOOL bInstant)
{
    return _CVI_MPI_IVE_CSC(pIveHandle, pstSrc,
            pstDst, pstCscCtrl, bInstant,
            /*iveReg=*/NULL, /*isEmit=*/true);
}

static CVI_S32 _CVI_MPI_IVE_Filter(IVE_HANDLE *pIveHandle,
        IVE_SRC_IMAGE_S *pstSrc, IVE_DST_IMAGE_S *pstDst,
        IVE_FILTER_CTRL_S *pstFltCtrl,CVI_BOOL bInstant,
        IVEReg* iveReg, bool isEmit)
{
    IVEReg _iveReg(pIveHandle, bInstant, pstDst->u32Height, pstDst->u32Width);
    if (!iveReg) {
        iveReg = &_iveReg;
    }
    iveReg->setEnableFilterop();
    iveReg->setEnableTop();

    // top
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstDst->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstDst->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;
    
    // TODO: check 1 channels setting
    iveReg->ive_filterop_c->REG_h10.reg_filterop_mode = 0;
    iveReg->ive_filterop_c->REG_h14.reg_filterop_op1_cmd = 1;
    iveReg->ive_filterop_c->REG_h14.reg_filterop_sw_ovw_op = 1;

    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef00 = pstFltCtrl->as8Mask[0] ;
    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef01 = pstFltCtrl->as8Mask[1] ;
    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef02 = pstFltCtrl->as8Mask[2] ;
    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef03 = pstFltCtrl->as8Mask[3] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef04 = pstFltCtrl->as8Mask[4] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef10 = pstFltCtrl->as8Mask[5] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef11 = pstFltCtrl->as8Mask[6] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef12 = pstFltCtrl->as8Mask[7] ;
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef13 = pstFltCtrl->as8Mask[8] ;
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef14 = pstFltCtrl->as8Mask[9] ;
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef20 = pstFltCtrl->as8Mask[10];
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef21 = pstFltCtrl->as8Mask[11];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef22 = pstFltCtrl->as8Mask[12];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef23 = pstFltCtrl->as8Mask[13];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef24 = pstFltCtrl->as8Mask[14];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef30 = pstFltCtrl->as8Mask[15];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef31 = pstFltCtrl->as8Mask[16];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef32 = pstFltCtrl->as8Mask[17];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef33 = pstFltCtrl->as8Mask[18];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef34 = pstFltCtrl->as8Mask[19];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef40 = pstFltCtrl->as8Mask[20];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef41 = pstFltCtrl->as8Mask[21];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef42 = pstFltCtrl->as8Mask[22];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef43 = pstFltCtrl->as8Mask[23];
    iveReg->ive_filterop_c->REG_10.reg_filterop_h_coef44 = pstFltCtrl->as8Mask[24];
    iveReg->ive_filterop_c->REG_10.reg_filterop_h_norm   = pstFltCtrl->u8Norm;

    iveReg->srcEnType = pstSrc->enType;
    iveReg->dstEnType = pstDst->enType;
    iveReg->setDMA(pstSrc, pstDst);

    switch (pstSrc->enType) {
        case IVE_IMAGE_TYPE_U8C1:
            // pass, filter case
            break;
        case IVE_IMAGE_TYPE_YUV420SP:
        case IVE_IMAGE_TYPE_YUV422SP:
            iveReg->ive_filterop_c->REG_h10.reg_filterop_mode = 1;
            break;
        default:
            printf("Invalid Image enType %d\n", pstSrc->enType);
            return CVI_FAILURE;
            break;
    }

	CVI_S32 r = CVI_SUCCESS;
    if (isEmit) {
        r = CVI_MPI_IVE_Filter(iveReg);
    }

    return r;
}
CVI_S32 CVI_MPI_IVE_Filter(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
	IVE_DST_IMAGE_S *pstDst, IVE_FILTER_CTRL_S *pstFltCtrl,CVI_BOOL bInstant)
{
    return _CVI_MPI_IVE_Filter(pIveHandle,
            pstSrc, pstDst,
            pstFltCtrl, bInstant,
            /*iveReg=*/NULL, /*isEmit=*/true);
}

#ifdef CMODEL
// TODO: to remove, used by CVI_MPI_IVE_FilterAndCSC for filter Dst creation
static CVI_U32 _CalcStride(CVI_U32 u32Width, CVI_U32 u32Align)
{
	CVI_U32 u16stride = u32Width + (u32Align - u32Width%u32Align)%u32Align;
	return u16stride;
}
#define _IVE2_STRIDE_ALIGN 16
static CVI_S32 _CreateIveImage(IVE_IMAGE_S *pstImage,IVE_IMAGE_TYPE_E enType, CVI_U32 u32Width, CVI_U32 u32Height)
{
	CVI_U32 u32Stride;
	CVI_S32 s32Succ;

	pstImage->enType = enType;
	pstImage->u32Width = u32Width;
	pstImage->u32Height = u32Height;

	u32Stride = _CalcStride(u32Width, _IVE2_STRIDE_ALIGN);
	s32Succ = CVI_SUCCESS;

	switch(enType)
	{
	case IVE_IMAGE_TYPE_U8C1:
	case IVE_IMAGE_TYPE_S8C1:
		{
			pstImage->au64VirAddr[0] = (CVI_U64)malloc(u32Stride * u32Height);

			pstImage->au64PhyAddr[0] = (CVI_U64)pstImage->au64VirAddr[0];
			pstImage->au32Stride[0]  = u32Stride;
		}
		break;
	case IVE_IMAGE_TYPE_YUV420SP:
		{
			pstImage->au64VirAddr[0] = (CVI_U64)malloc(u32Stride * u32Height * 3/2);

			pstImage->au64VirAddr[1] = pstImage->au64VirAddr[0] + u32Stride * u32Height;
			pstImage->au64PhyAddr[0] = (CVI_U64)pstImage->au64VirAddr[0];
			pstImage->au64PhyAddr[1] = (CVI_U64)pstImage->au64VirAddr[1];
			pstImage->au32Stride[0]  = u32Stride;
			pstImage->au32Stride[1]  = pstImage->au32Stride[0];
		}
		break;
	case IVE_IMAGE_TYPE_YUV422SP:
		{
			pstImage->au64VirAddr[0] = (CVI_U64)malloc(u32Stride * u32Height * 2);

			pstImage->au64VirAddr[1] = pstImage->au64VirAddr[0] + u32Stride * u32Height;
			pstImage->au64PhyAddr[0] = (CVI_U64)pstImage->au64VirAddr[0];
			pstImage->au64PhyAddr[1] = (CVI_U64)pstImage->au64VirAddr[1];
			pstImage->au32Stride[0] = u32Stride;
			pstImage->au32Stride[1]  = pstImage->au32Stride[0];
		}
		break;
	case IVE_IMAGE_TYPE_YUV420P:
		{
			CVI_U32 u32Stride2;

			u32Stride2= _CalcStride(u32Width/2, _IVE2_STRIDE_ALIGN);
			pstImage->au64VirAddr[0] = (CVI_U64)malloc(u32Stride * u32Height + u32Stride2 * u32Height);
			pstImage->au64VirAddr[1] = pstImage->au64VirAddr[0] + u32Stride * u32Height;
			pstImage->au64VirAddr[2] = pstImage->au64VirAddr[1] + u32Stride2 * u32Height/2;
			pstImage->au64PhyAddr[0] = (CVI_U64)pstImage->au64VirAddr[0];
			pstImage->au64PhyAddr[1] = (CVI_U64)pstImage->au64VirAddr[1];
			pstImage->au64PhyAddr[2] = (CVI_U64)pstImage->au64VirAddr[2];
			pstImage->au32Stride[0] = u32Stride;
			pstImage->au32Stride[1]  = u32Stride2;
			pstImage->au32Stride[2]  = pstImage->au32Stride[1];
		}
		break;
	case IVE_IMAGE_TYPE_YUV422P:
		{
			CVI_U32 u32Stride2;

			u32Stride2 = _CalcStride(u32Width/2, _IVE2_STRIDE_ALIGN);
			pstImage->au64VirAddr[0] = (CVI_U64)malloc(u32Stride * u32Height + u32Stride2 * u32Height*2);
			pstImage->au64VirAddr[1] = pstImage->au64VirAddr[0] + u32Stride * u32Height;
			pstImage->au64VirAddr[2] = pstImage->au64VirAddr[1] + u32Stride2 * u32Height;
			pstImage->au64PhyAddr[0] = (CVI_U64)pstImage->au64VirAddr[0];
			pstImage->au64PhyAddr[1] = (CVI_U64)pstImage->au64VirAddr[1];
			pstImage->au64PhyAddr[2] = (CVI_U64)pstImage->au64VirAddr[2];
			pstImage->au32Stride[0]  = u32Stride;
			pstImage->au32Stride[1]  = u32Stride2;
			pstImage->au32Stride[2]  = pstImage->au32Stride[1];
		}
		break;
	case IVE_IMAGE_TYPE_S8C2_PACKAGE:
		{
			pstImage->au64VirAddr[0] = (CVI_U64)malloc(u32Stride * u32Height * 2);
			pstImage->au64VirAddr[1] = pstImage->au64VirAddr[0] + 1;
			pstImage->au64PhyAddr[0] = (CVI_U64)pstImage->au64VirAddr[0];
			pstImage->au64PhyAddr[1] = (CVI_U64)pstImage->au64VirAddr[1];
			pstImage->au32Stride[0] = u32Stride;
			pstImage->au32Stride[1]  = pstImage->au32Stride[0];
		}
		break;
	case IVE_IMAGE_TYPE_S8C2_PLANAR:
		{
			pstImage->au64VirAddr[0] = (CVI_U64)malloc(u32Stride * u32Height * 2);
			pstImage->au64VirAddr[1] = pstImage->au64VirAddr[0] + u32Stride * u32Height;
			pstImage->au64PhyAddr[0] = (CVI_U64)pstImage->au64VirAddr[0];
			pstImage->au64PhyAddr[1] = (CVI_U64)pstImage->au64VirAddr[1];
			pstImage->au32Stride[0]  = u32Stride;
			pstImage->au32Stride[1]  = pstImage->au32Stride[0];
		}
		break;
	case IVE_IMAGE_TYPE_S16C1:
	case IVE_IMAGE_TYPE_U16C1:
		{
			pstImage->au64VirAddr[0] = (CVI_U64)malloc(u32Stride * u32Height * 2);
			pstImage->au64PhyAddr[0] = (CVI_U64)pstImage->au64VirAddr[0];
			pstImage->au32Stride[0] = u32Stride;
		}
		break;
	case IVE_IMAGE_TYPE_U8C3_PACKAGE:
		{
			pstImage->au64VirAddr[0] = (CVI_U64)malloc(u32Stride * u32Height * 3);
			pstImage->au64VirAddr[1] = pstImage->au64VirAddr[0] + 1;
			pstImage->au64VirAddr[2] = pstImage->au64VirAddr[1] + 1;
			pstImage->au64PhyAddr[0] = (CVI_U64)pstImage->au64VirAddr[0];
			pstImage->au64PhyAddr[1] = (CVI_U64)pstImage->au64VirAddr[1];
			pstImage->au64PhyAddr[2] = (CVI_U64)pstImage->au64VirAddr[2];
			pstImage->au32Stride[0] = u32Stride;
			pstImage->au32Stride[1] = pstImage->au32Stride[0];
			pstImage->au32Stride[2] = pstImage->au32Stride[0];
		}
		break;
	case IVE_IMAGE_TYPE_U8C3_PLANAR:
		{
			pstImage->au64VirAddr[0] = (CVI_U64)malloc(u32Stride * u32Height * 3);
			pstImage->au64VirAddr[1] = pstImage->au64VirAddr[0] + u32Stride * u32Height;
			pstImage->au64VirAddr[2] = pstImage->au64VirAddr[1] + u32Stride * u32Height;
			pstImage->au64PhyAddr[0] = (CVI_U64)pstImage->au64VirAddr[0];
			pstImage->au64PhyAddr[1] = (CVI_U64)pstImage->au64VirAddr[1];
			pstImage->au64PhyAddr[2] = (CVI_U64)pstImage->au64VirAddr[2];
			pstImage->au32Stride[0] = u32Stride;
			pstImage->au32Stride[1] = pstImage->au32Stride[0];
			pstImage->au32Stride[2] = pstImage->au32Stride[0];
		}
		break;
	case IVE_IMAGE_TYPE_S32C1:
	case IVE_IMAGE_TYPE_U32C1:
		{
			pstImage->au64VirAddr[0] = (CVI_U64)malloc(u32Stride * u32Height * 4);
			pstImage->au64PhyAddr[0] = (CVI_U64)pstImage->au64VirAddr[0];
			pstImage->au32Stride[0] = u32Stride;
		}
		break;
	case IVE_IMAGE_TYPE_S64C1:
	case IVE_IMAGE_TYPE_U64C1:
		{
			pstImage->au64VirAddr[0] = (CVI_U64)malloc(u32Stride * u32Height * 8);
			pstImage->au64PhyAddr[0] = (CVI_U64)pstImage->au64VirAddr[0];
			pstImage->au32Stride[0] = u32Stride;
		}
		break;
	default:
		{
			s32Succ = CVI_FAILURE;
		}
		break;
	}

	return s32Succ;
}

static CVI_S32 _DestroyIveImage(IVE_IMAGE_S *pstImage)
{
	if (0 != pstImage->au64VirAddr[0])
	{
		free((CVI_VOID*)pstImage->au64VirAddr[0]);
		pstImage->au64VirAddr[0] = 0;
	}

	return CVI_SUCCESS;
}
#endif

CVI_S32 CVI_MPI_IVE_FilterAndCSC(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
        IVE_DST_IMAGE_S *pstDst, IVE_FILTER_AND_CSC_CTRL_S *pstFltCscCtrl, CVI_BOOL bInstant)
{
	printf("CVI_MPI_IVE_FilterAndCSC\n");

	CVI_S32 s32Result = CVI_SUCCESS;
	IVE_FILTER_CTRL_S stFltCtrl;
	IVE_CSC_CTRL_S stCscCtrl;

    if (pstSrc->enType == IVE_IMAGE_TYPE_YUV420SP
            || pstSrc->enType == IVE_IMAGE_TYPE_YUV422SP) {
        // only support YUV420SP、YUV422SP as input
    }
    else {
		CVI_ASSERT(0 && "only support input fmt YUV420SP/YUV422SP\n");
        return CVI_FAILURE;
    }

#ifdef CMODEL
	IVE_DST_IMAGE_S stFilterDst;

	s32Result = _CreateIveImage(&stFilterDst,pstSrc->enType,pstSrc->u32Width,pstSrc->u32Height);
	if (s32Result != CVI_SUCCESS)
	{
		printf("_CreateIveImage failed\n");
		return s32Result;
	}

	memcpy(stFltCtrl.as8Mask, pstFltCscCtrl->as8Mask, sizeof(stFltCtrl.as8Mask));
	stFltCtrl.u8Norm = pstFltCscCtrl->u8Norm;
	s32Result = CVI_MPI_IVE_Filter(pIveHandle,pstSrc,&stFilterDst,&stFltCtrl,bInstant);
	if (s32Result != CVI_SUCCESS)
	{
		printf("CVI_MPI_IVE_Filter failed\n");
		goto FAILURE;
	}

	stCscCtrl.enMode = pstFltCscCtrl->enMode;
	s32Result = CVI_MPI_IVE_CSC(pIveHandle,&stFilterDst,pstDst, &stCscCtrl,bInstant);
	if (s32Result != CVI_SUCCESS)
	{
		printf("CVI_MPI_IVE_CSC failed\n");
		goto FAILURE;
	}
FAILURE:
	_DestroyIveImage(&stFilterDst);
#else
    // set at once
    IVEReg _iveReg(pIveHandle, bInstant, pstSrc->u32Height, pstSrc->u32Width);
    IVEReg* iveReg = &_iveReg;

// FIXME: combine it
    // set filter
	memcpy(stFltCtrl.as8Mask, pstFltCscCtrl->as8Mask, sizeof(stFltCtrl.as8Mask));
	stFltCtrl.u8Norm = pstFltCscCtrl->u8Norm;
    _CVI_MPI_IVE_Filter(pIveHandle,
            pstSrc, pstDst,
            &stFltCtrl, bInstant,
            iveReg, /*isEmit=*/false);

    // set csc
	stCscCtrl.enMode = pstFltCscCtrl->enMode;
    _CVI_MPI_IVE_CSC(pIveHandle, pstSrc,
            pstDst, &stCscCtrl, bInstant,
            iveReg, /*isEmit=*/false);

    // emit
    s32Result = CVI_MPI_IVE_FilterAndCSC(iveReg);
#endif

	return s32Result;
}

CVI_S32 CVI_MPI_IVE_Hist(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
	IVE_DST_MEM_INFO_S *pstDst, CVI_BOOL bInstant)
{
  printf("CVI_MPI_IVE_Hist\n");

  IVEReg _iveReg(pIveHandle, bInstant, pstSrc->u32Height, pstSrc->u32Width);
  IVEReg* iveReg = &_iveReg;
  iveReg->setEnableFilterop();
  iveReg->setEnableTop();

  // top
  iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstSrc->u32Width - 1;
  iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstSrc->u32Height - 1;
  iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
  iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;

  iveReg->ive_hist_c->REG_1.reg_ive_hist_stride = (CVI_U16)pstSrc->au32Stride[0];

  iveReg->srcEnType = pstSrc->enType;
  //iveReg->dstEnType = pstDst->enType;

  IVE_DST_IMAGE_S _pstDst;
  _pstDst.au64VirAddr[0] = pstDst->u64VirAddr;
  iveReg->setDMA(pstSrc, &_pstDst);

  return CVI_MPI_IVE_Hist(iveReg);
}

CVI_S32 CVI_MPI_IVE_EqualizeHist(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
	IVE_DST_IMAGE_S *pstDst, IVE_EQUALIZE_HIST_CTRL_S *pstEqualizeHistCtrl, CVI_BOOL bInstant)
{
	printf("CVI_MPI_IVE_Hist\n");

	//
	// TODO: Here shall be real MPI IVE driver code, which fill in register command, then pass to hw/cmodel
	// FIXME: call CVI_HW_XXX() directly for now
	//
    IVEReg _iveReg(pIveHandle, bInstant, pstSrc->u32Height, pstSrc->u32Width);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableTop();

    // FIXME: need to review
    // top
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstSrc->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstSrc->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;

    IVE_DST_IMAGE_S _pstSrc;
    _pstSrc.au64VirAddr[0] = pstEqualizeHistCtrl->stMem.u64VirAddr;

    // TODO: set dma
    iveReg->srcEnType = pstSrc->enType;
    iveReg->dstEnType = pstDst->enType;
    iveReg->setDMA(pstSrc, pstDst, &_pstSrc);

    return CVI_MPI_IVE_EqualizeHist(iveReg);
}

CVI_S32 CVI_MPI_IVE_Map(IVE_HANDLE *pIveHandle,IVE_SRC_IMAGE_S *pstSrc,
	IVE_SRC_MEM_INFO_S *pstMap, IVE_DST_IMAGE_S *pstDst,IVE_MAP_CTRL_S *pstMapCtrl,CVI_BOOL bInstant)
{

	printf("CVI_MPI_IVE_Map\n");

    IVEReg _iveReg(pIveHandle, bInstant, pstDst->u32Height, pstDst->u32Width);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableFilterop();
    iveReg->setEnableTop();

    // top
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstDst->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstDst->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;

    iveReg->ive_filterop_c->REG_h10.reg_filterop_mode = 11;
    // TODO: neet to set?
    iveReg->ive_filterop_c->REG_h14.reg_filterop_op1_cmd = 0;
    // TODO: neet to set?
    //iveReg->ive_filterop_c->REG_h14.reg_filterop_sw_ovw_op = 11;
    iveReg->ive_filterop_c->REG_110.reg_filterop_map_enmode = pstMapCtrl->enMode;
    IVE_SRC_IMAGE_S _pstMap;
    _pstMap.au64VirAddr[0] = pstMap->u64VirAddr;

    iveReg->srcEnType = pstSrc->enType;
    iveReg->dstEnType = pstDst->enType;
    iveReg->setDMA(pstSrc, pstDst, &_pstMap);

    return CVI_MPI_IVE_Map(iveReg);
}

CVI_S32 CVI_MPI_IVE_NCC(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc1,
	IVE_SRC_IMAGE_S *pstSrc2, IVE_DST_MEM_INFO_S *pstDst, CVI_BOOL bInstant)
{
	printf("CVI_MPI_IVE_NCC\n");
    IVEReg _iveReg(pIveHandle, bInstant, pstSrc1->u32Height, pstSrc1->u32Width);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableTop();

    // top
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstSrc1->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstSrc1->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;

    // output
    IVE_DST_IMAGE_S _pstDst;
    _pstDst.au64VirAddr[0] = pstDst->u64VirAddr;

    // TODO: set dma
    iveReg->srcEnType = pstSrc1->enType;
    iveReg->dstEnType = _pstDst.enType;
    iveReg->setDMA(pstSrc1, &_pstDst, pstSrc2);

    return CVI_MPI_IVE_NCC(iveReg);
}

CVI_S32 CVI_MPI_IVE_OrdStatFilter(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
	IVE_DST_IMAGE_S *pstDst, IVE_ORD_STAT_FILTER_CTRL_S *pstOrdStatFltCtrl, CVI_BOOL bInstant)
{
	printf("CVI_MPI_IVE_OrdStatFilter\n");

    IVEReg _iveReg(pIveHandle, bInstant, pstDst->u32Height, pstDst->u32Width);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableFilterop();
    iveReg->setEnableTop();

    // top
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstDst->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstDst->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;

    // TODO: check set to 0
    iveReg->ive_filterop_c->REG_h10.reg_filterop_mode = 0;
    iveReg->ive_filterop_c->REG_h14.reg_filterop_op1_cmd = 4;
    //iveReg->ive_filterop_c->REG_h14.reg_filterop_sw_ovw_op = 4;
    iveReg->ive_filterop_c->REG_18.reg_filterop_order_enmode   = pstOrdStatFltCtrl->enMode;
    iveReg->srcEnType = pstSrc->enType;
    iveReg->dstEnType = pstDst->enType;
    iveReg->setDMA(pstSrc, pstDst);

    return CVI_MPI_IVE_OrdStatFilter(iveReg);
}

CVI_S32 CVI_MPI_IVE_Resize(IVE_HANDLE *pIveHandle,IVE_SRC_IMAGE_S astSrc[],
	IVE_DST_IMAGE_S astDst[],IVE_RESIZE_CTRL_S *pstResizeCtrl,CVI_BOOL bInstant)
{
	printf("CVI_MPI_IVE_Resize\n");
	if (pstResizeCtrl->enMode != IVE_RESIZE_MODE_AREA && pstResizeCtrl->enMode != IVE_RESIZE_MODE_LINEAR)
	{
		printf("Invalid Resize enMode %d\n", pstResizeCtrl->enMode);
		return CVI_FAILURE;
	}
	for(int i=0; i < pstResizeCtrl->u16Num; i++)
    {
        CVI_U16 scale_x = (astSrc[i].u32Width << 11) / astDst[i].u32Width;
        CVI_U16 scale_y = (astSrc[i].u32Height << 11) / astDst[i].u32Height;

        if(astSrc[i].enType != IVE_IMAGE_TYPE_U8C3_PLANAR && astSrc[i].enType != IVE_IMAGE_TYPE_U8C1)
        {
            printf("Invalid IMAGE TYPE %d\n", astSrc[0].enType);
            return CVI_FAILURE;
        }

        IVEReg _iveReg(pIveHandle, bInstant, astSrc[i].u32Height, astSrc[i].u32Width);
        IVEReg* iveReg = &_iveReg;
        iveReg->setEnableTop();

        // top
        // setting
        // TODO: need softrst?
        iveReg->ive_top_c->REG_RS_SRC_SIZE.reg_resize_src_wd = astSrc[i].u32Width - 1;
        iveReg->ive_top_c->REG_RS_SRC_SIZE.reg_resize_src_ht = astSrc[i].u32Height - 1;
        iveReg->ive_top_c->REG_RS_DST_SIZE.reg_resize_dst_wd = astDst[i].u32Width - 1;
        iveReg->ive_top_c->REG_RS_DST_SIZE.reg_resize_dst_ht = astDst[i].u32Height - 1;
        iveReg->ive_top_c->REG_RS_H_SC.reg_resize_h_sc_fac = scale_y;
        iveReg->ive_top_c->REG_RS_V_SC.reg_resize_v_sc_fac = scale_x;
        // TODO: need to setting?
        iveReg->ive_top_c->REG_RS_PH_INI.reg_resize_h_ini_ph = 0;
        iveReg->ive_top_c->REG_RS_PH_INI.reg_resize_v_ini_ph = 0;
        iveReg->ive_top_c->REG_RS_NOR.reg_resize_h_nor = 0;
        iveReg->ive_top_c->REG_RS_NOR.reg_resize_v_nor = 0;
        iveReg->ive_top_c->REG_RS_CTRL.reg_resize_ip_en = bInstant;
        iveReg->ive_top_c->REG_RS_CTRL.reg_resize_dbg_en = 0;
        if (pstResizeCtrl->enMode == IVE_RESIZE_MODE_AREA) {
            iveReg->ive_top_c->REG_RS_CTRL.reg_resize_area_fast = 1;
        }
        else if (pstResizeCtrl->enMode == IVE_RESIZE_MODE_LINEAR) {
            iveReg->ive_top_c->REG_RS_CTRL.reg_resize_blnr_mode = 1;
        }
        else {
            printf("Invalid ResizeCtrl->enMode %d\n", pstResizeCtrl->enMode);
            return CVI_FAILURE;
        }
        // TODO: need to setting?
        iveReg->ive_top_c->REG_RS_dBG_H1.reg_resize_sc_dbg_h1 = 0;
        iveReg->ive_top_c->REG_RS_dBG_H2.reg_resize_sc_dbg_h2 = 0;
        iveReg->ive_top_c->REG_RS_dBG_V1.reg_resize_sc_dbg_v1 = 0;
        iveReg->ive_top_c->REG_RS_dBG_V2.reg_resize_sc_dbg_v2 = 0;

        // TODO: set dma
        iveReg->setDMA(&astSrc[i], &astDst[i]);
        iveReg->srcEnType = astSrc[i].enType;
        iveReg->dstEnType = astDst[i].enType;

        CVI_MPI_IVE_Resize(iveReg);

    }
    return CVI_SUCCESS;
}

CVI_S32 CVI_MPI_IVE_CannyHysEdge(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc, IVE_DST_IMAGE_S *pstEdge,
	   IVE_DST_MEM_INFO_S *pstStack, IVE_CANNY_HYS_EDGE_CTRL_S *pstCannyHysEdgeCtrl, CVI_BOOL bInstant)
{
	printf("CVI_MPI_IVE_CannyHysEdge\n");

    IVEReg _iveReg(pIveHandle, bInstant, pstSrc->u32Height, pstSrc->u32Width);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableFilterop();
    iveReg->setEnableTop();

    // top
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstSrc->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstSrc->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;
    iveReg->ive_filterop_c->REG_h10.reg_filterop_mode = 4;
    // TODO: need to setting?
    //iveReg->ive_filterop_c->REG_h14.reg_filterop_op1_cmd = 1;
    // TODO: need to setting?
    //iveReg->ive_filterop_c->REG_h14.reg_filterop_sw_ovw_op = 1;
    iveReg->ive_filterop_c->REG_CANNY_0.reg_canny_lowthr = pstCannyHysEdgeCtrl->u16LowThr;
    iveReg->ive_filterop_c->REG_CANNY_0.reg_canny_hithr = pstCannyHysEdgeCtrl->u16HighThr;
    iveReg->ive_filterop_c->REG_CANNY_1.reg_canny_en = bInstant;
    // TODO: need to properly setting
    //iveReg->ive_filterop_c->REG_CANNY_1.reg_canny_strong_point_cnt;
    //iveReg->ive_filterop_c->REG_CANNY_1.reg_canny_strong_point_cnt_en;
    //iveReg->ive_filterop_c->REG_CANNY_1.reg_canny_non_or_weak;
    //iveReg->ive_filterop_c->REG_CANNY_1.reg_canny_eof;
    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef00 = pstCannyHysEdgeCtrl->as8Mask[0] ;
    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef01 = pstCannyHysEdgeCtrl->as8Mask[1] ;
    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef02 = pstCannyHysEdgeCtrl->as8Mask[2] ;
    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef03 = pstCannyHysEdgeCtrl->as8Mask[3] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef04 = pstCannyHysEdgeCtrl->as8Mask[4] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef10 = pstCannyHysEdgeCtrl->as8Mask[5] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef11 = pstCannyHysEdgeCtrl->as8Mask[6] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef12 = pstCannyHysEdgeCtrl->as8Mask[7] ;
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef13 = pstCannyHysEdgeCtrl->as8Mask[8] ;
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef14 = pstCannyHysEdgeCtrl->as8Mask[9] ;
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef20 = pstCannyHysEdgeCtrl->as8Mask[10];
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef21 = pstCannyHysEdgeCtrl->as8Mask[11];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef22 = pstCannyHysEdgeCtrl->as8Mask[12];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef23 = pstCannyHysEdgeCtrl->as8Mask[13];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef24 = pstCannyHysEdgeCtrl->as8Mask[14];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef30 = pstCannyHysEdgeCtrl->as8Mask[15];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef31 = pstCannyHysEdgeCtrl->as8Mask[16];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef32 = pstCannyHysEdgeCtrl->as8Mask[17];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef33 = pstCannyHysEdgeCtrl->as8Mask[18];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef34 = pstCannyHysEdgeCtrl->as8Mask[19];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef40 = pstCannyHysEdgeCtrl->as8Mask[20];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef41 = pstCannyHysEdgeCtrl->as8Mask[21];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef42 = pstCannyHysEdgeCtrl->as8Mask[22];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef43 = pstCannyHysEdgeCtrl->as8Mask[23];
    iveReg->ive_filterop_c->REG_10.reg_filterop_h_coef44 = pstCannyHysEdgeCtrl->as8Mask[24];

    IVE_DST_IMAGE_S _pstStack;
    _pstStack.au64VirAddr[0] = pstStack->u64VirAddr;
    // NOTICE: we leverage stride as size
    _pstStack.au32Stride[0] = pstStack->u32Size;
    iveReg->srcEnType = pstSrc->enType;
    iveReg->dstEnType = pstEdge->enType;
    iveReg->setDMA(pstSrc, pstEdge, NULL, &_pstStack);

    return CVI_MPI_IVE_CannyHysEdge(iveReg);
}

CVI_S32 CVI_MPI_IVE_CannyEdge(IVE_IMAGE_S *pstEdge, IVE_MEM_INFO_S *pstStack)
{
    // link edge and draw it
    printf("CVI_MPI_IVE_CannyEdge\n");

    // TODO: pass pIveHandle
    IVE_HANDLE *pIveHandle = NULL;
    CVI_BOOL bInstant = CVI_TRUE;

    IVEReg _iveReg(pIveHandle, bInstant, pstEdge->u32Height, pstEdge->u32Width);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableFilterop();
    iveReg->setEnableTop();

    // top
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstEdge->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstEdge->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;
    iveReg->ive_filterop_c->REG_h10.reg_filterop_mode = 4;
    // TODO: need to setting?
    //iveReg->ive_filterop_c->REG_h14.reg_filterop_op1_cmd = 1;
    // TODO: need to setting?
    //iveReg->ive_filterop_c->REG_h14.reg_filterop_sw_ovw_op = 1;
    iveReg->ive_filterop_c->REG_CANNY_1.reg_canny_en = bInstant;
    // TODO: need to properly setting

    IVE_DST_IMAGE_S _pstStack, _pstStackSize;
    _pstStack.au64VirAddr[0] = pstStack->u64VirAddr;

    CVI_U32 *u32StackSize = (CVI_U32 *)(pstStack->u64VirAddr + pstStack->u32Size - 16);
    _pstStackSize.au64VirAddr[0] = (CVI_U64)u32StackSize;

    // in/out
    iveReg->srcEnType = pstEdge->enType;
    iveReg->setDMA(pstEdge, NULL, &_pstStack, NULL, &_pstStackSize);

    return CVI_MPI_IVE_CannyEdge(iveReg);
}

CVI_S32 CVI_MPI_IVE_Integ(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
	IVE_DST_IMAGE_S *pstDst, IVE_INTEG_CTRL_S *pstIntegCtrl, CVI_BOOL bInstant)
{
	printf("CVI_MPI_IVE_Integ\n");

    IVEReg _iveReg(pIveHandle, bInstant, pstSrc->u32Height, pstSrc->u32Width);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableFilterop();
    iveReg->setEnableTop();

    // top
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstDst->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstDst->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;

    // FIXME: support tile
    iveReg->ive_intg_c->REG_0.reg_ive_intg_tile_nm = 0;
    iveReg->ive_intg_c->REG_0.reg_ive_intg_ctrl = (int) pstIntegCtrl->enOutCtrl;
    iveReg->ive_intg_c->REG_1.reg_ive_intg_stride = (CVI_U16)pstSrc->au32Stride[0];
    // TODO: set dma
    iveReg->srcEnType = pstSrc->enType;
    iveReg->dstEnType = pstDst->enType;
    iveReg->setDMA(pstSrc, pstDst);

    return CVI_MPI_IVE_Integ(iveReg);

	CVI_HW_Integ(
			(CVI_U8 *)pstSrc->au64VirAddr[0], (CVI_U16)pstSrc->au32Stride[0],
			(CVI_U32 *)pstDst->au64VirAddr[0], (CVI_U16)pstDst->au32Stride[0],
			(CVI_U16)pstDst->u32Width, (CVI_U16)pstDst->u32Height,
			pstIntegCtrl->enOutCtrl);
	return CVI_SUCCESS;
}

CVI_S32 CVI_MPI_IVE_LBP(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
	IVE_DST_IMAGE_S *pstDst, IVE_LBP_CTRL_S *pstLbpCtrl, CVI_BOOL bInstant)
{
	printf("CVI_MPI_IVE_LBP\n");
    IVEReg _iveReg(pIveHandle, bInstant, pstDst->u32Height, pstDst->u32Width);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableTop();
    iveReg->setEnableFilterop();

    // align isp
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstDst->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstDst->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;
    // setting
    iveReg->ive_top_c->REG_15.reg_lbp_u8bit_thr = pstLbpCtrl->un8BitThr.u8Val;
    iveReg->ive_top_c->REG_15.reg_lbp_s8bit_thr = pstLbpCtrl->un8BitThr.s8Val;
    iveReg->ive_top_c->REG_15.reg_lbp_enmode = pstLbpCtrl->enMode;

    // TODO: set dma
    iveReg->srcEnType = pstSrc->enType;
    iveReg->dstEnType = pstDst->enType;
    iveReg->setDMA(pstSrc, pstDst);

    return CVI_MPI_IVE_LBP(iveReg);
}

CVI_S32 CVI_MPI_IVE_NormGrad(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
		IVE_DST_IMAGE_S *pstDstH, IVE_DST_IMAGE_S *pstDstV,
		IVE_DST_IMAGE_S *pstDstHV,
		IVE_NORM_GRAD_CTRL_S *pstNormGradCtrl, CVI_BOOL bInstant)
{
	printf("CVI_MPI_IVE_NormGrad\n");

    IVEReg _iveReg(pIveHandle, bInstant, pstSrc->u32Height, pstSrc->u32Width);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableFilterop();
    iveReg->setEnableTop();

    // top
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstSrc->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstSrc->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;
    iveReg->ive_filterop_c->REG_h10.reg_filterop_mode = 8;
    // TODO: check needs to set
    //iveReg->ive_filterop_c->REG_h14.reg_filterop_op1_cmd = 1;
    // TODO: check needs to set
    //iveReg->ive_filterop_c->REG_h14.reg_filterop_sw_ovw_op = 7;
    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef00 = pstNormGradCtrl->as8Mask[0] ;
    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef01 = pstNormGradCtrl->as8Mask[1] ;
    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef02 = pstNormGradCtrl->as8Mask[2] ;
    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef03 = pstNormGradCtrl->as8Mask[3] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef04 = pstNormGradCtrl->as8Mask[4] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef10 = pstNormGradCtrl->as8Mask[5] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef11 = pstNormGradCtrl->as8Mask[6] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef12 = pstNormGradCtrl->as8Mask[7] ;
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef13 = pstNormGradCtrl->as8Mask[8] ;
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef14 = pstNormGradCtrl->as8Mask[9] ;
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef20 = pstNormGradCtrl->as8Mask[10];
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef21 = pstNormGradCtrl->as8Mask[11];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef22 = pstNormGradCtrl->as8Mask[12];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef23 = pstNormGradCtrl->as8Mask[13];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef24 = pstNormGradCtrl->as8Mask[14];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef30 = pstNormGradCtrl->as8Mask[15];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef31 = pstNormGradCtrl->as8Mask[16];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef32 = pstNormGradCtrl->as8Mask[17];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef33 = pstNormGradCtrl->as8Mask[18];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef34 = pstNormGradCtrl->as8Mask[19];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef40 = pstNormGradCtrl->as8Mask[20];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef41 = pstNormGradCtrl->as8Mask[21];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef42 = pstNormGradCtrl->as8Mask[22];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef43 = pstNormGradCtrl->as8Mask[23];
    iveReg->ive_filterop_c->REG_10.reg_filterop_h_coef44 = pstNormGradCtrl->as8Mask[24];

    iveReg->ive_filterop_c->REG_10.reg_filterop_h_norm   = pstNormGradCtrl->u8Norm;
    iveReg->srcEnType = pstSrc->enType;


	if (pstNormGradCtrl->enOutCtrl == IVE_NORM_GRAD_OUT_CTRL_HOR_AND_VER)
	{
    iveReg->dstEnType = pstDstH->enType;
      iveReg->setDMA(pstSrc, pstDstH, NULL, pstDstV);
      iveReg->ive_filterop_c->REG_110.reg_filterop_norm_out_ctrl = 0;
      iveReg->ive_filterop_c->REG_h14.reg_op_y_wdma_en = 1; // h
      iveReg->ive_filterop_c->REG_h14.reg_op_c_wdma_en = 1; // v
      return CVI_MPI_IVE_NormGrad(iveReg);
	}
	else if (pstNormGradCtrl->enOutCtrl == IVE_NORM_GRAD_OUT_CTRL_HOR)
	{
    iveReg->dstEnType = pstDstH->enType;
      iveReg->setDMA(pstSrc, pstDstH);
      iveReg->ive_filterop_c->REG_110.reg_filterop_norm_out_ctrl = 1;
      iveReg->ive_filterop_c->REG_h14.reg_op_y_wdma_en = 1; // h
      iveReg->ive_filterop_c->REG_h14.reg_op_c_wdma_en = 0; // v
      return CVI_MPI_IVE_NormGrad(iveReg);
	}
	else if (pstNormGradCtrl->enOutCtrl == IVE_NORM_GRAD_OUT_CTRL_VER)
	{
    iveReg->dstEnType = pstDstV->enType;
      iveReg->setDMA(pstSrc, NULL, NULL, pstDstV);
      iveReg->ive_filterop_c->REG_110.reg_filterop_norm_out_ctrl = 2;
      iveReg->ive_filterop_c->REG_h14.reg_op_y_wdma_en = 0; // h
      iveReg->ive_filterop_c->REG_h14.reg_op_c_wdma_en = 1; // v
      return CVI_MPI_IVE_NormGrad(iveReg);
	}
    else if (pstNormGradCtrl->enOutCtrl == IVE_NORM_GRAD_OUT_CTRL_COMBINE)
	{
    iveReg->dstEnType = pstDstHV->enType;
      iveReg->setDMA(pstSrc, pstDstHV);
      iveReg->ive_filterop_c->REG_110.reg_filterop_norm_out_ctrl = 3;
      iveReg->ive_filterop_c->REG_h14.reg_op_y_wdma_en = 1; // h
      iveReg->ive_filterop_c->REG_h14.reg_op_c_wdma_en = 0; // v
      return CVI_MPI_IVE_NormGrad(iveReg);
	}
	else
	{
		printf("Invalid enOutCtrl %d\n", pstNormGradCtrl->enOutCtrl);
		return CVI_FAILURE;
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_MPI_IVE_MagAndAng(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
		IVE_DST_IMAGE_S *pstDstMag, IVE_DST_IMAGE_S *pstDstAng,
		IVE_MAG_AND_ANG_CTRL_S *pstMagAndAngCtrl, CVI_BOOL bInstant)
{

	printf("CVI_MPI_IVE_MagAndAng\n");
    IVEReg _iveReg(pIveHandle, bInstant, pstSrc->u32Height, pstSrc->u32Width);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableFilterop();
    iveReg->setEnableTop();

    // top
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstSrc->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstSrc->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;
    iveReg->ive_filterop_c->REG_h10.reg_filterop_mode = 7;
    // TODO: check 1 channels setting
    iveReg->ive_filterop_c->REG_h14.reg_filterop_op1_cmd = 0;
    // TODO: neet to set?
    //iveReg->ive_filterop_c->REG_h14.reg_filterop_sw_ovw_op = 8;
    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef00 = pstMagAndAngCtrl->as8Mask[0] ;
    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef01 = pstMagAndAngCtrl->as8Mask[1] ;
    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef02 = pstMagAndAngCtrl->as8Mask[2] ;
    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef03 = pstMagAndAngCtrl->as8Mask[3] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef04 = pstMagAndAngCtrl->as8Mask[4] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef10 = pstMagAndAngCtrl->as8Mask[5] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef11 = pstMagAndAngCtrl->as8Mask[6] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef12 = pstMagAndAngCtrl->as8Mask[7] ;
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef13 = pstMagAndAngCtrl->as8Mask[8] ;
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef14 = pstMagAndAngCtrl->as8Mask[9] ;
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef20 = pstMagAndAngCtrl->as8Mask[10];
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef21 = pstMagAndAngCtrl->as8Mask[11];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef22 = pstMagAndAngCtrl->as8Mask[12];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef23 = pstMagAndAngCtrl->as8Mask[13];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef24 = pstMagAndAngCtrl->as8Mask[14];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef30 = pstMagAndAngCtrl->as8Mask[15];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef31 = pstMagAndAngCtrl->as8Mask[16];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef32 = pstMagAndAngCtrl->as8Mask[17];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef33 = pstMagAndAngCtrl->as8Mask[18];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef34 = pstMagAndAngCtrl->as8Mask[19];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef40 = pstMagAndAngCtrl->as8Mask[20];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef41 = pstMagAndAngCtrl->as8Mask[21];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef42 = pstMagAndAngCtrl->as8Mask[22];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef43 = pstMagAndAngCtrl->as8Mask[23];
    iveReg->ive_filterop_c->REG_10.reg_filterop_h_coef44 = pstMagAndAngCtrl->as8Mask[24];

    iveReg->ive_filterop_c->REG_18.reg_filterop_mag_thr  = pstMagAndAngCtrl->u16Thr;

    iveReg->srcEnType = pstSrc->enType;
    iveReg->dstEnType = pstDstMag->enType;
	if(pstMagAndAngCtrl->enOutCtrl == IVE_MAG_AND_ANG_OUT_CTRL_MAG_AND_ANG)
	{
      iveReg->setDMA(pstSrc, pstDstMag, NULL, pstDstAng);
      //iveReg->ive_filterop_c->REG_h14.reg_op_y_wdma_en = 1;
      //iveReg->ive_filterop_c->REG_h14.reg_op_c_wdma_en = 1;
      // "0 : h , v  -> wdma_y wdma_c will be active 1 :h only -> wdma_y 2: v only -> wdma_c 3. h , v pack => {v ,h} -> wdma_y "
      iveReg->ive_filterop_c->REG_110.reg_filterop_magang_out_ctrl = 1;
      return CVI_MPI_IVE_MagAndAng(iveReg);
	}
	else
	{
      iveReg->setDMA(pstSrc, pstDstMag);
      //iveReg->ive_filterop_c->REG_h14.reg_op_c_wdma_en = 1;
      // "0 : h , v  -> wdma_y wdma_c will be active 1 :h only -> wdma_y 2: v only -> wdma_c 3. h , v pack => {v ,h} -> wdma_y "
      iveReg->ive_filterop_c->REG_110.reg_filterop_magang_out_ctrl = 0;
      return CVI_MPI_IVE_MagAndAng(iveReg);
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_MPI_IVE_STCandiCorner(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc, IVE_DST_IMAGE_S *pstCandiCorner,
	IVE_ST_CANDI_CORNER_CTRL_S *pstStCandiCornerCtrl, CVI_BOOL bInstant)
{
	UNUSED(pIveHandle);
	UNUSED(bInstant);

	//printf("CVI_MPI_IVE_STCandiCorner\n");

	if (pstSrc->enType != IVE_IMAGE_TYPE_U8C1)
	{
		printf("pstSrc->enType(%d) must be U8C1(%d)\n", pstSrc->enType, IVE_IMAGE_TYPE_U8C1);
		return CVI_FAILURE;
	}

	if (pstCandiCorner->enType != IVE_IMAGE_TYPE_U8C1)
	{
		printf("pstCandiCorner->enType(%d) must be U8C1(%d)\n", pstCandiCorner->enType, IVE_IMAGE_TYPE_U8C1);
		return CVI_FAILURE;
	}

	if (pstSrc->u32Height != pstCandiCorner->u32Height || pstSrc->u32Width != pstCandiCorner->u32Width)
	{
		printf("pstCandiCorner->u32Width(%d) and pstCandiCorner->u32Height(%d) "
				"must be equal to pstSrc->u32Width(%d) and pstSrc->u32Height(%d)\n",
				pstCandiCorner->u32Width, pstCandiCorner->u32Height,
				pstSrc->u32Width, pstSrc->u32Height);
		return CVI_FAILURE;
	}

	if (!pstStCandiCornerCtrl->stMem.u64VirAddr)
	{
		printf("pstStCandiCornerCtrl->stMem.u64VirAddr can't be 0!\n");
		return CVI_FAILURE;
	}

	if (!pstStCandiCornerCtrl->stMem.u64PhyAddr)
	{
		printf("pstStCandiCornerCtrl->stMem.u64PhyAddr can't be 0!\n");
		return CVI_FAILURE;
	}

	if (pstStCandiCornerCtrl->stMem.u32Size < 4 * pstSrc->u32Height * pstSrc->au32Stride[0] + sizeof(IVE_ST_MAX_EIG_S))
	{
		printf("pstStCandiCornerCtrl->stMem.u32Size must be greater than or equal to %zu!\n",
				4 * pstSrc->u32Height * pstSrc->au32Stride[0] + sizeof(IVE_ST_MAX_EIG_S));
		return CVI_FAILURE;
	}

	if (pstStCandiCornerCtrl->u0q8QualityLevel == 0)
	{
		printf("pstStCandiCornerCtrl->u0q8QualityLevel can't be 0!");
		return CVI_FAILURE;
	}

	CVI_U32 u32SizeS8C2 = pstSrc->u32Height * pstSrc->au32Stride[0] * 2;
	CVI_U32 u32SizeU8C1 = pstSrc->u32Height * pstSrc->au32Stride[0];

	IVE_NORM_GRAD_OUT_CTRL_E enOutCtrl = IVE_NORM_GRAD_OUT_CTRL_COMBINE;
	CVI_S8 as8Mask[25]={0,0,0,0,0,
						0,-1,0,1,0,
						0,-2,0,2,0,
						0,-1,0,1,0,
						0,0,0,0,0};
	CVI_U8 u8Norm = 3;
	CVI_HW_NormGrad(
			(CVI_U8 *)pstSrc->au64VirAddr[0], (CVI_U16)pstSrc->au32Stride[0],
			NULL, 0,
			NULL, 0,
			(CVI_S8 *)pstStCandiCornerCtrl->stMem.u64VirAddr, (CVI_U16)pstSrc->au32Stride[0],
			as8Mask,
			(CVI_U16)pstSrc->u32Width, (CVI_U16)pstSrc->u32Height,
			u8Norm, enOutCtrl);

    (void)u32SizeS8C2;
    (void)u32SizeU8C1;
	CVI_U16 *u16MaxEig = (CVI_U16 *)(pstStCandiCornerCtrl->stMem.u64VirAddr + u32SizeS8C2 * 2);
	CVI_HW_STBoxFltAndEigCalc(
			(CVI_S16 *)pstStCandiCornerCtrl->stMem.u64VirAddr, (CVI_U16)pstSrc->au32Stride[0],
			(CVI_U16 *)(pstStCandiCornerCtrl->stMem.u64VirAddr + u32SizeS8C2), (CVI_U16)pstSrc->au32Stride[0],
			(CVI_U16)pstSrc->u32Width, (CVI_U16)pstSrc->u32Height,
			u16MaxEig);
	//printf("u16MaxEig = %d\n", *u16MaxEig);

	CVI_U8 enMode_16BitTo8Bit = IVE_16BIT_TO_8BIT_MODE_U16_TO_U8;
	CVI_U16	u16Denominator 	= *u16MaxEig;
	CVI_U8 u8Numerator	 	= 255;
	CVI_S8 s8Bias = 0;
	CVI_U32 tmp = (((CVI_U32)u8Numerator << 16) + (u16Denominator >> 1)) / u16Denominator;
	CVI_U16 u8Num_div_u16Den = (CVI_U16)(tmp & 0xffff);
	CVI_HW_16BitTo8Bit(
			(CVI_U16 *)(pstStCandiCornerCtrl->stMem.u64VirAddr + u32SizeS8C2), (CVI_U16)pstSrc->au32Stride[0],
			(CVI_U8 *)pstStCandiCornerCtrl->stMem.u64VirAddr, (CVI_U16)pstSrc->au32Stride[0],
			(CVI_U16)pstSrc->u32Width, (CVI_U16)pstSrc->u32Height,
			enMode_16BitTo8Bit, u8Num_div_u16Den, s8Bias);

	IVE_THRESH_MODE_E enMode_Thresh = IVE_THRESH_MODE_TO_MINVAL;
	CVI_U8 u8LowThr = pstStCandiCornerCtrl->u0q8QualityLevel;
	CVI_HW_Thresh(
			(CVI_U8 *)pstStCandiCornerCtrl->stMem.u64VirAddr, (CVI_U16)pstSrc->au32Stride[0],
			(CVI_U8 *)(pstStCandiCornerCtrl->stMem.u64VirAddr + u32SizeU8C1), (CVI_U16)pstSrc->au32Stride[0],
			(CVI_U16)pstSrc->u32Width, (CVI_U16)pstSrc->u32Height,
			u8LowThr, 0, 0, 0, 0, enMode_Thresh);

	IVE_ORD_STAT_FILTER_MODE_E enMode_OrdStatFilter = IVE_ORD_STAT_FILTER_MODE_MAX;
	CVI_HW_OrdStatFilter(
			(CVI_U8 *)(pstStCandiCornerCtrl->stMem.u64VirAddr + u32SizeU8C1), (CVI_U16)pstSrc->au32Stride[0],
			(CVI_U8 *)pstStCandiCornerCtrl->stMem.u64VirAddr, (CVI_U16)pstSrc->au32Stride[0],
			(CVI_U16)pstSrc->u32Width, (CVI_U16)pstSrc->u32Height,
			enMode_OrdStatFilter);

	CVI_HW_STCandiCorner(
			(CVI_U8 *)(pstStCandiCornerCtrl->stMem.u64VirAddr + u32SizeU8C1), (CVI_U16)pstSrc->au32Stride[0],
			(CVI_U8 *)pstStCandiCornerCtrl->stMem.u64VirAddr, (CVI_U16)pstSrc->au32Stride[0],
			(CVI_U8 *)pstCandiCorner->au64VirAddr[0], (CVI_U16)pstCandiCorner->au32Stride[0],
			(CVI_U16)pstSrc->u32Width, (CVI_U16)pstSrc->u32Height);

	return CVI_SUCCESS;
}

static int _CVI_Comp(const void *a1, const void*a2)
{
	return **(CVI_U8 **)a2 - **(CVI_U8 **)a1;
}

typedef struct _IVE_ST_CORNER_CELL_S
{
    CVI_U16 u16PointNum;
	CVI_U16 Reserved;
    IVE_POINT_U16_S *pstPoint;
}IVE_ST_CORNER_CELL_S;

CVI_S32 CVI_MPI_IVE_STCorner(IVE_SRC_IMAGE_S *pstCandiCorner, IVE_DST_MEM_INFO_S *pstCorner,
    IVE_ST_CORNER_CTRL_S *pstStCornerCtrl)
{
	//printf("CVI_MPI_IVE_STCorner\n");

	// CVI_STCornerCollect()
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32Width = pstCandiCorner->u32Width;
	CVI_U32 u32Height = pstCandiCorner->u32Height;
	CVI_U32 u32Stride = pstCandiCorner->au32Stride[0];
	CVI_U8 *pCandiCorner = (CVI_U8 *)pstCandiCorner->au64VirAddr[0];
	CVI_U8 **Base = NULL;
	CVI_U8 *Block = NULL;
	CVI_U32 x, y, k, l, BlkX, BlkY, m, n, ii;
	CVI_U32 NumOfElements = 0;
	CVI_U32 u32BlkWidth;
	CVI_U32 u32BlkHeight;
	CVI_U16 u16MinDist = pstStCornerCtrl->u16MinDist;
	CVI_U16 u16MinDista = u16MinDist * u16MinDist;
	IVE_ST_CORNER_CELL_S *pstCells;
	IVE_ST_CORNER_CELL_S *cell;
	CVI_U32 v13, v14, v15, v16, v17, v18;
	CVI_U32 new_point;
	IVE_ST_CORNER_INFO_S *pstCornerInfo = (IVE_ST_CORNER_INFO_S *)pstCorner->u64VirAddr;
	pstCornerInfo->u16CornerNum = 0;

	if (!pstStCornerCtrl->u16MinDist)
	{
		printf("u16MinDist is zero\n");
		return CVI_FAILURE;
	}

	Base = (CVI_U8 **)malloc(sizeof(CVI_U8 *) * u32Width * u32Height);
	if (!Base) {
		printf("malloc Base failed\n");
		return CVI_FAILURE;
	}
	//memset(Base, 0, sizeof(CVI_U8 *) * u32Width * u32Height);

	u32BlkWidth = (u32Width + u16MinDist - 1) / u16MinDist;
	u32BlkHeight = (u32Height + u16MinDist - 1) / u16MinDist;
	Block = (CVI_U8 *)malloc(sizeof(IVE_ST_CORNER_CELL_S) * u32BlkWidth * u32BlkHeight);
	if (!Block) {
		printf("malloc Block failed\n");
		free(Base);
		return CVI_FAILURE;
	}
	//memset(Block, 0, sizeof(IVE_ST_CORNER_CELL_S) * u32BlkWidth * u32BlkHeight);

	for (y = 1; y < u32Height - 1; ++y)
	{
		for (x = 1; x < u32Width - 1; ++x)
		{
			if (pCandiCorner[y * u32Stride + x])
			{
				Base[NumOfElements++] = &pCandiCorner[y * u32Stride + x];
			}
		}
	}
	//printf("NumOfElements = %d\n", NumOfElements);

	qsort(Base, NumOfElements, sizeof(CVI_U8 *), _CVI_Comp);

	//for (l = 0; l < 300; ++l)
	//{
	//	y = (CVI_U32)((Base[l] - pCandiCorner) / u32Stride);
	//	x = (CVI_U32)((Base[l] - pCandiCorner) - u32Stride * y);
	//	printf("[%d] x = %04x, y = %04x, v = %03d\n", l, x, y, *Base[l]);
	//}

	memset(Block, 0, sizeof(IVE_ST_CORNER_CELL_S) * u32BlkWidth * u32BlkHeight);
	pstCells = (IVE_ST_CORNER_CELL_S *)Block;
	for (k = 0; k < u32BlkWidth * u32BlkHeight; ++k)
	{
		pstCells[k].pstPoint = (IVE_POINT_U16_S *)malloc(sizeof(IVE_POINT_U16_S) * u16MinDist * u16MinDist);
		if (!Block) {
			printf("malloc pstCells[%d].pstPoint failed\n", k);
			s32Ret = CVI_FAILURE;
			goto out;
		}
		memset(pstCells[k].pstPoint, 0, sizeof(IVE_POINT_U16_S) * u16MinDist * u16MinDist);
	}

	for (l = 0; l < NumOfElements; ++l)
	{
		y = (CVI_U32)((Base[l] - pCandiCorner) / u32Stride);
		x = (CVI_U32)((Base[l] - pCandiCorner) - u32Stride * y);

		new_point = 1;

		BlkY = y / u16MinDist;
		BlkX = x / u16MinDist;
		v13 = ((BlkX - 1) <= 0) ? 0 : (BlkX - 1);
		v14 = ((BlkY - 1) <= 0) ? 0 : (BlkY - 1);
		v15 = ((BlkX + 1) > (u32BlkWidth - 1)) ? (u32BlkWidth - 1) : (BlkX + 1);
		v16 = ((BlkY + 1) > (u32BlkHeight - 1)) ? (u32BlkHeight - 1) : (BlkY + 1);
		for (m = v14; m <= v16; ++m)
		{
			for (n = v13; n <= v15; ++n)
			{
				cell = &pstCells[u32BlkWidth * m + n];
				if (cell->u16PointNum)
				{
					for (ii = 0; ii < cell->u16PointNum; ++ii)
					{
						v18 = x - cell->pstPoint[ii].u16X;
						v17 = y - cell->pstPoint[ii].u16Y;
						if (v17 * v17 + v18 * v18 < u16MinDista)
						{
							new_point = 0;

							// break out the loops
							n = v15 + 1;
							m = v16 + 1;
							break;
						}
					}
				}
			}
		}

		if (new_point)
		{
			cell = &pstCells[u32BlkWidth * BlkY + BlkX];
			cell->pstPoint[cell->u16PointNum].u16X = x;
			cell->pstPoint[cell->u16PointNum].u16Y = y;
			cell->u16PointNum ++;
			pstCornerInfo->astCorner[pstCornerInfo->u16CornerNum].u16X = x;
			pstCornerInfo->astCorner[pstCornerInfo->u16CornerNum].u16Y = y;
			pstCornerInfo->u16CornerNum ++;

			if (pstCornerInfo->u16CornerNum >= pstStCornerCtrl->u16MaxCornerNum)
			{
				break;
			}
		}
	}

out:
	for (k = 0; k < u32BlkWidth * u32BlkHeight; ++k)
	{
		if (pstCells[k].pstPoint)
		{
			free(pstCells[k].pstPoint);
		}
	}
	free(Block);
	free(Base);

	return s32Ret;
}

CVI_S32 CVI_MPI_IVE_Sobel(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
		IVE_DST_IMAGE_S *pstDstH, IVE_DST_IMAGE_S *pstDstV,
		IVE_SOBEL_CTRL_S *pstSobelCtrl, CVI_BOOL bInstant)
{
	printf("CVI_MPI_IVE_Sobel\n");

    IVEReg _iveReg(pIveHandle, bInstant, pstSrc->u32Height, pstSrc->u32Width);
    IVEReg* iveReg = &_iveReg;

    iveReg->setEnableFilterop();
    iveReg->setEnableTop();

    // top
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstSrc->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 =pstSrc->u32Height - 1;
    iveReg->ive_filterop_c->REG_h10.reg_filterop_mode = 9;

    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef00 = pstSobelCtrl->as8Mask[0] ;
    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef01 = pstSobelCtrl->as8Mask[1] ;
    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef02 = pstSobelCtrl->as8Mask[2] ;
    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef03 = pstSobelCtrl->as8Mask[3] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef04 = pstSobelCtrl->as8Mask[4] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef10 = pstSobelCtrl->as8Mask[5] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef11 = pstSobelCtrl->as8Mask[6] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef12 = pstSobelCtrl->as8Mask[7] ;
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef13 = pstSobelCtrl->as8Mask[8] ;
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef14 = pstSobelCtrl->as8Mask[9] ;
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef20 = pstSobelCtrl->as8Mask[10];
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef21 = pstSobelCtrl->as8Mask[11];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef22 = pstSobelCtrl->as8Mask[12];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef23 = pstSobelCtrl->as8Mask[13];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef24 = pstSobelCtrl->as8Mask[14];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef30 = pstSobelCtrl->as8Mask[15];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef31 = pstSobelCtrl->as8Mask[16];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef32 = pstSobelCtrl->as8Mask[17];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef33 = pstSobelCtrl->as8Mask[18];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef34 = pstSobelCtrl->as8Mask[19];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef40 = pstSobelCtrl->as8Mask[20];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef41 = pstSobelCtrl->as8Mask[21];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef42 = pstSobelCtrl->as8Mask[22];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef43 = pstSobelCtrl->as8Mask[23];
    iveReg->ive_filterop_c->REG_10.reg_filterop_h_coef44 = pstSobelCtrl->as8Mask[24];

    iveReg->srcEnType = pstSrc->enType;
    IVE_DST_IMAGE_S* firstOut = pstDstH;
    IVE_DST_IMAGE_S* secondOut = pstDstV;
    if (pstSobelCtrl->enOutCtrl == IVE_SOBEL_OUT_CTRL_VER) {
      // FIXME: check only one output
      firstOut = pstDstV;
      secondOut = NULL;
    }

    iveReg->dstEnType = firstOut->enType;

    iveReg->setDMA(pstSrc, firstOut, NULL, secondOut);

    // "0 : h , v  -> wdma_y wdma_c will be active 1 :h only -> wdma_y 2: v only -> wdma_c 3. h , v pack => {v ,h} -> wdma_y "
    iveReg->ive_filterop_c->REG_110.reg_filterop_norm_out_ctrl = (int)pstSobelCtrl->enOutCtrl;

	if (pstSobelCtrl->enOutCtrl == IVE_SOBEL_OUT_CTRL_BOTH
            || pstSobelCtrl->enOutCtrl == IVE_SOBEL_OUT_CTRL_HOR
            || pstSobelCtrl->enOutCtrl == IVE_SOBEL_OUT_CTRL_VER) {
        // valid
    }
    else {
		CVI_ASSERT(0 && "not support enOutCtrl\n");
        return CVI_FAILURE;
    }

    return CVI_MPI_IVE_Sobel(iveReg);
}

CVI_S32 CVI_MPI_IVE_CCL(IVE_HANDLE *pIveHandle, IVE_IMAGE_S *pstSrcDst,
	IVE_DST_MEM_INFO_S *pstBlob, IVE_CCL_CTRL_S *pstCclCtrl, CVI_BOOL bInstant)
{
	printf("CVI_MPI_IVE_CCL, enMode = %d\n", pstCclCtrl->enMode);
#if 0
    IVEReg _iveReg(pIveHandle, bInstant);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableTop();

    // top
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstSrcDst->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstSrcDst->u32Height - 1;
reg_ccl_mode
reg_ccl_area_thr

    // setting

    IVE_SRC_IMAGE_S _pstModel;
    _pstModel.au64PhyAddr[0] = pstModel->u64PhyAddr;
    _pstModel.au64VirAddr[0] = pstModel->u64VirAddr;

    iveReg->setDMA(pstSrc, pstFg, pstFactor, pstBg, &_pstModel, pstMatchModelInfo);

    return CVI_MPI_IVE_CCL(iveReg);
#else
	UNUSED(pIveHandle);
	UNUSED(bInstant);
	IVE_CCBLOB_S *pstCCBlob = (IVE_CCBLOB_S *)pstBlob->u64VirAddr;

	CVI_HW_CCL(
			(CVI_U8 *)pstSrcDst->au64VirAddr[0], (CVI_U16)pstSrcDst->au32Stride[0],
			(CVI_U16)pstSrcDst->u32Width, (CVI_U16)pstSrcDst->u32Height,
			pstCCBlob->astRegion, &pstCCBlob->u16CurAreaThr,
			pstCclCtrl->u16InitAreaThr, pstCclCtrl->u16Step,
			&pstCCBlob->s8LabelStatus, &pstCCBlob->u8RegionNum, pstCclCtrl->enMode);
#endif
	return CVI_SUCCESS;
}

CVI_S32 CVI_MPI_IVE_Dilate(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
	IVE_DST_IMAGE_S *pstDst, IVE_DILATE_CTRL_S *pstDilateCtrl,CVI_BOOL bInstant)
{
	printf("CVI_MPI_IVE_Dilate\n");

    IVEReg _iveReg(pIveHandle, bInstant, pstSrc->u32Height, pstSrc->u32Width);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableFilterop();
    iveReg->setEnableTop();

    // top
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstSrc->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstSrc->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;
    // TODO: check 1 channels setting
    iveReg->ive_filterop_c->REG_h10.reg_filterop_mode = 2;
    iveReg->ive_filterop_c->REG_h14.reg_filterop_op1_cmd = 2;

    // TODO: need to set?
    //iveReg->ive_filterop_c->REG_h14.reg_filterop_sw_ovw_op = 3;
    // TODO: enable for second pass
    //iveReg->ive_filterop_c->REG_28.reg_filterop_op2_erodila_en = 1;
#if 1
    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef00 = pstDilateCtrl->au8Mask[0] ;
    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef01 = pstDilateCtrl->au8Mask[1] ;
    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef02 = pstDilateCtrl->au8Mask[2] ;
    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef03 = pstDilateCtrl->au8Mask[3] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef04 = pstDilateCtrl->au8Mask[4] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef10 = pstDilateCtrl->au8Mask[5] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef11 = pstDilateCtrl->au8Mask[6] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef12 = pstDilateCtrl->au8Mask[7] ;
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef13 = pstDilateCtrl->au8Mask[8] ;
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef14 = pstDilateCtrl->au8Mask[9] ;
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef20 = pstDilateCtrl->au8Mask[10];
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef21 = pstDilateCtrl->au8Mask[11];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef22 = pstDilateCtrl->au8Mask[12];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef23 = pstDilateCtrl->au8Mask[13];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef24 = pstDilateCtrl->au8Mask[14];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef30 = pstDilateCtrl->au8Mask[15];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef31 = pstDilateCtrl->au8Mask[16];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef32 = pstDilateCtrl->au8Mask[17];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef33 = pstDilateCtrl->au8Mask[18];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef34 = pstDilateCtrl->au8Mask[19];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef40 = pstDilateCtrl->au8Mask[20];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef41 = pstDilateCtrl->au8Mask[21];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef42 = pstDilateCtrl->au8Mask[22];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef43 = pstDilateCtrl->au8Mask[23];
    iveReg->ive_filterop_c->REG_10.reg_filterop_h_coef44 = pstDilateCtrl->au8Mask[24];
#else
#error "ONLY for second pass"

    iveReg->ive_filterop_c->REG_21.reg_filterop_op2_erodila_coef00 = pstDilateCtrl->au8Mask[0] ;
    iveReg->ive_filterop_c->REG_21.reg_filterop_op2_erodila_coef01 = pstDilateCtrl->au8Mask[1] ;
    iveReg->ive_filterop_c->REG_21.reg_filterop_op2_erodila_coef02 = pstDilateCtrl->au8Mask[2] ;
    iveReg->ive_filterop_c->REG_21.reg_filterop_op2_erodila_coef03 = pstDilateCtrl->au8Mask[3] ;
    iveReg->ive_filterop_c->REG_22.reg_filterop_op2_erodila_coef04 = pstDilateCtrl->au8Mask[4] ;
    iveReg->ive_filterop_c->REG_22.reg_filterop_op2_erodila_coef10 = pstDilateCtrl->au8Mask[5] ;
    iveReg->ive_filterop_c->REG_22.reg_filterop_op2_erodila_coef11 = pstDilateCtrl->au8Mask[6] ;
    iveReg->ive_filterop_c->REG_22.reg_filterop_op2_erodila_coef12 = pstDilateCtrl->au8Mask[7] ;
    iveReg->ive_filterop_c->REG_23.reg_filterop_op2_erodila_coef13 = pstDilateCtrl->au8Mask[8] ;
    iveReg->ive_filterop_c->REG_23.reg_filterop_op2_erodila_coef14 = pstDilateCtrl->au8Mask[9] ;
    iveReg->ive_filterop_c->REG_23.reg_filterop_op2_erodila_coef20 = pstDilateCtrl->au8Mask[10];
    iveReg->ive_filterop_c->REG_23.reg_filterop_op2_erodila_coef21 = pstDilateCtrl->au8Mask[11];
    iveReg->ive_filterop_c->REG_24.reg_filterop_op2_erodila_coef22 = pstDilateCtrl->au8Mask[12];
    iveReg->ive_filterop_c->REG_24.reg_filterop_op2_erodila_coef23 = pstDilateCtrl->au8Mask[13];
    iveReg->ive_filterop_c->REG_24.reg_filterop_op2_erodila_coef24 = pstDilateCtrl->au8Mask[14];
    iveReg->ive_filterop_c->REG_24.reg_filterop_op2_erodila_coef30 = pstDilateCtrl->au8Mask[15];
    iveReg->ive_filterop_c->REG_25.reg_filterop_op2_erodila_coef31 = pstDilateCtrl->au8Mask[16];
    iveReg->ive_filterop_c->REG_25.reg_filterop_op2_erodila_coef32 = pstDilateCtrl->au8Mask[17];
    iveReg->ive_filterop_c->REG_25.reg_filterop_op2_erodila_coef33 = pstDilateCtrl->au8Mask[18];
    iveReg->ive_filterop_c->REG_25.reg_filterop_op2_erodila_coef34 = pstDilateCtrl->au8Mask[19];
    iveReg->ive_filterop_c->REG_26.reg_filterop_op2_erodila_coef40 = pstDilateCtrl->au8Mask[20];
    iveReg->ive_filterop_c->REG_26.reg_filterop_op2_erodila_coef41 = pstDilateCtrl->au8Mask[21];
    iveReg->ive_filterop_c->REG_26.reg_filterop_op2_erodila_coef42 = pstDilateCtrl->au8Mask[22];
    iveReg->ive_filterop_c->REG_26.reg_filterop_op2_erodila_coef43 = pstDilateCtrl->au8Mask[23];
    iveReg->ive_filterop_c->REG_27.reg_filterop_op2_erodila_coef44 = pstDilateCtrl->au8Mask[24];
#endif

    iveReg->srcEnType = pstSrc->enType;
    iveReg->dstEnType = pstDst->enType;
    iveReg->setDMA(pstSrc, pstDst);

    return CVI_MPI_IVE_Dilate(iveReg);
}

CVI_S32 CVI_MPI_IVE_Erode(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
	IVE_DST_IMAGE_S *pstDst, IVE_ERODE_CTRL_S *pstErodeCtrl,CVI_BOOL bInstant)
{
	printf("CVI_MPI_IVE_Erode\n");

    IVEReg _iveReg(pIveHandle, bInstant, pstSrc->u32Height, pstSrc->u32Width,
        IVEModuleFILTEROP);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableFilterop();
    iveReg->setEnableTop();

    // top
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstSrc->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstSrc->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;
   
    // TODO: check 1 channels setting
    iveReg->ive_filterop_c->REG_h10.reg_filterop_mode = 3;
    iveReg->ive_filterop_c->REG_h14.reg_filterop_op1_cmd = 3;
    
#if 1
    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef00 = pstErodeCtrl->au8Mask[0] ;
    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef01 = pstErodeCtrl->au8Mask[1] ;
    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef02 = pstErodeCtrl->au8Mask[2] ;
    iveReg->ive_filterop_c->REG_4.reg_filterop_h_coef03 = pstErodeCtrl->au8Mask[3] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef04 = pstErodeCtrl->au8Mask[4] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef10 = pstErodeCtrl->au8Mask[5] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef11 = pstErodeCtrl->au8Mask[6] ;
    iveReg->ive_filterop_c->REG_5.reg_filterop_h_coef12 = pstErodeCtrl->au8Mask[7] ;
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef13 = pstErodeCtrl->au8Mask[8] ;
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef14 = pstErodeCtrl->au8Mask[9] ;
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef20 = pstErodeCtrl->au8Mask[10];
    iveReg->ive_filterop_c->REG_6.reg_filterop_h_coef21 = pstErodeCtrl->au8Mask[11];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef22 = pstErodeCtrl->au8Mask[12];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef23 = pstErodeCtrl->au8Mask[13];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef24 = pstErodeCtrl->au8Mask[14];
    iveReg->ive_filterop_c->REG_7.reg_filterop_h_coef30 = pstErodeCtrl->au8Mask[15];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef31 = pstErodeCtrl->au8Mask[16];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef32 = pstErodeCtrl->au8Mask[17];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef33 = pstErodeCtrl->au8Mask[18];
    iveReg->ive_filterop_c->REG_8.reg_filterop_h_coef34 = pstErodeCtrl->au8Mask[19];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef40 = pstErodeCtrl->au8Mask[20];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef41 = pstErodeCtrl->au8Mask[21];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef42 = pstErodeCtrl->au8Mask[22];
    iveReg->ive_filterop_c->REG_9.reg_filterop_h_coef43 = pstErodeCtrl->au8Mask[23];
    iveReg->ive_filterop_c->REG_10.reg_filterop_h_coef44 = pstErodeCtrl->au8Mask[24];
#else
#error "ONLY for second pass"
    iveReg->ive_filterop_c->REG_21.reg_filterop_op2_erodila_coef00 = pstErodeCtrl->au8Mask[0] ;
    iveReg->ive_filterop_c->REG_21.reg_filterop_op2_erodila_coef01 = pstErodeCtrl->au8Mask[1] ;
    iveReg->ive_filterop_c->REG_21.reg_filterop_op2_erodila_coef02 = pstErodeCtrl->au8Mask[2] ;
    iveReg->ive_filterop_c->REG_21.reg_filterop_op2_erodila_coef03 = pstErodeCtrl->au8Mask[3] ;
    iveReg->ive_filterop_c->REG_22.reg_filterop_op2_erodila_coef04 = pstErodeCtrl->au8Mask[4] ;
    iveReg->ive_filterop_c->REG_22.reg_filterop_op2_erodila_coef10 = pstErodeCtrl->au8Mask[5] ;
    iveReg->ive_filterop_c->REG_22.reg_filterop_op2_erodila_coef11 = pstErodeCtrl->au8Mask[6] ;
    iveReg->ive_filterop_c->REG_22.reg_filterop_op2_erodila_coef12 = pstErodeCtrl->au8Mask[7] ;
    iveReg->ive_filterop_c->REG_23.reg_filterop_op2_erodila_coef13 = pstErodeCtrl->au8Mask[8] ;
    iveReg->ive_filterop_c->REG_23.reg_filterop_op2_erodila_coef14 = pstErodeCtrl->au8Mask[9] ;
    iveReg->ive_filterop_c->REG_23.reg_filterop_op2_erodila_coef20 = pstErodeCtrl->au8Mask[10];
    iveReg->ive_filterop_c->REG_23.reg_filterop_op2_erodila_coef21 = pstErodeCtrl->au8Mask[11];
    iveReg->ive_filterop_c->REG_24.reg_filterop_op2_erodila_coef22 = pstErodeCtrl->au8Mask[12];
    iveReg->ive_filterop_c->REG_24.reg_filterop_op2_erodila_coef23 = pstErodeCtrl->au8Mask[13];
    iveReg->ive_filterop_c->REG_24.reg_filterop_op2_erodila_coef24 = pstErodeCtrl->au8Mask[14];
    iveReg->ive_filterop_c->REG_24.reg_filterop_op2_erodila_coef30 = pstErodeCtrl->au8Mask[15];
    iveReg->ive_filterop_c->REG_25.reg_filterop_op2_erodila_coef31 = pstErodeCtrl->au8Mask[16];
    iveReg->ive_filterop_c->REG_25.reg_filterop_op2_erodila_coef32 = pstErodeCtrl->au8Mask[17];
    iveReg->ive_filterop_c->REG_25.reg_filterop_op2_erodila_coef33 = pstErodeCtrl->au8Mask[18];
    iveReg->ive_filterop_c->REG_25.reg_filterop_op2_erodila_coef34 = pstErodeCtrl->au8Mask[19];
    iveReg->ive_filterop_c->REG_26.reg_filterop_op2_erodila_coef40 = pstErodeCtrl->au8Mask[20];
    iveReg->ive_filterop_c->REG_26.reg_filterop_op2_erodila_coef41 = pstErodeCtrl->au8Mask[21];
    iveReg->ive_filterop_c->REG_26.reg_filterop_op2_erodila_coef42 = pstErodeCtrl->au8Mask[22];
    iveReg->ive_filterop_c->REG_26.reg_filterop_op2_erodila_coef43 = pstErodeCtrl->au8Mask[23];
    iveReg->ive_filterop_c->REG_27.reg_filterop_op2_erodila_coef44 = pstErodeCtrl->au8Mask[24];
#endif

    iveReg->srcEnType = pstSrc->enType;
    iveReg->dstEnType = pstDst->enType;
    iveReg->setDMA(pstSrc, pstDst);

    return CVI_MPI_IVE_Erode(iveReg);
}

CVI_S32 CVI_MPI_IVE_MatchBgModel(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstCurImg, IVE_DATA_S *pstBgModel,
	IVE_IMAGE_S *pstFgFlag, IVE_DST_IMAGE_S *pstBgDiffFg, IVE_DST_IMAGE_S *pstFrmDiffFg, IVE_DST_MEM_INFO_S *pstStatData,
	IVE_MATCH_BG_MODEL_CTRL_S *pstMatchBgModelCtrl,	CVI_BOOL bInstant)
{
    printf("CVI_MPI_IVE_MatchBgModel\n");
    IVEReg _iveReg(pIveHandle, bInstant, pstCurImg->u32Height, pstCurImg->u32Width);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableTop();

    // top
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstCurImg->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstCurImg->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;

    // setting
    iveReg->ive_match_bg_c->REG_00.reg_matchbg_en = bInstant;
    // TODO: need to reset?
    iveReg->ive_match_bg_c->REG_00.reg_matchbg_softrst = 0;
    iveReg->ive_match_bg_c->REG_04.reg_matchbg_curfrmnum = pstMatchBgModelCtrl->u32CurFrmNum;

    iveReg->ive_match_bg_c->REG_08.reg_matchbg_prefrmnum = pstMatchBgModelCtrl->u32PreFrmNum;
    iveReg->ive_match_bg_c->REG_0C.reg_matchbg_timethr = pstMatchBgModelCtrl->u16TimeThr;
    iveReg->ive_match_bg_c->REG_0C.reg_matchbg_diffthrcrlcoef = pstMatchBgModelCtrl->u8DiffThrCrlCoef;
    iveReg->ive_match_bg_c->REG_0C.reg_matchbg_diffmaxthr = pstMatchBgModelCtrl->u8DiffMaxThr;
    iveReg->ive_match_bg_c->REG_0C.reg_matchbg_diffminthr = pstMatchBgModelCtrl->u8DiffMinThr;
    iveReg->ive_match_bg_c->REG_0C.reg_matchbg_diffthrinc = pstMatchBgModelCtrl->u8DiffThrInc;
    iveReg->ive_match_bg_c->REG_0C.reg_matchbg_fastlearnrate = pstMatchBgModelCtrl->u8FastLearnRate;
    iveReg->ive_match_bg_c->REG_0C.reg_matchbg_detchgregion = pstMatchBgModelCtrl->u8DetChgRegion;
    // TODO: debug use?
    //iveReg->ive_match_bg_c->REG_10.reg_matchbg_stat_pixnum;
    //iveReg->ive_match_bg_c->REG_14.reg_matchbg_stat_sumlum;
    // TODO: set dma
    IVE_DST_IMAGE_S _pstStatData;
    _pstStatData.au64VirAddr[0] = pstStatData->u64VirAddr;
    IVE_SRC_IMAGE_S _pstFgFlag;
    IVE_SRC_IMAGE_S _pstBgModel;
    for (int i = 0; i < 3; i++) {
        _pstFgFlag.au64PhyAddr[i] = pstFgFlag->au64PhyAddr[i];
        _pstFgFlag.au64VirAddr[i] = pstFgFlag->au64VirAddr[i];
        _pstFgFlag.au32Stride[i]  = pstFgFlag->au32Stride[i];
        _pstBgModel.au64PhyAddr[i] = pstBgModel->u64PhyAddr;
        _pstBgModel.au64VirAddr[i] = pstBgModel->u64VirAddr;
        _pstBgModel.au32Stride[i]  = pstBgModel->u32Stride;
    }

    iveReg->srcEnType = pstCurImg->enType;
    // FIXME: check output type
    iveReg->dstEnType = pstBgDiffFg->enType;
    iveReg->setDMA(pstCurImg, pstBgDiffFg, &_pstBgModel, pstFrmDiffFg,
            &_pstFgFlag, &_pstStatData);

    return CVI_MPI_IVE_MatchBgModel(iveReg);
}

CVI_S32 CVI_MPI_IVE_UpdateBgModel(IVE_HANDLE *pIveHandle,	IVE_DATA_S *pstBgModel, IVE_IMAGE_S *pstFgFlag,
	IVE_DST_IMAGE_S *pstBgImg, IVE_DST_IMAGE_S *pstChgStaImg, IVE_DST_IMAGE_S *pstChgStaFg, IVE_DST_IMAGE_S *pstChgStaLife,
	IVE_DST_MEM_INFO_S *pstStatData, IVE_UPDATE_BG_MODEL_CTRL_S *pstUpdateBgModelCtrl, CVI_BOOL bInstant)
{
	UNUSED(pIveHandle);
	UNUSED(bInstant);

	//printf("CVI_MPI_IVE_UpdateBgModel\n");

	//
	// TODO: Here shall be real MPI IVE driver code, which fill in register command, then pass to hw/cmodel
	// FIXME: call CVI_HW_XXX() directly for now
	//
	CVI_HW_UpdateBgModel(
			(CVI_U8 *)pstBgModel->u64VirAddr, (CVI_U16)pstBgModel->u32Stride,
			(CVI_U8 *)pstFgFlag->au64VirAddr[0], (CVI_U8 *)pstBgImg->au64VirAddr[0], (CVI_U8 *)pstChgStaImg->au64VirAddr[0],
			(CVI_S8 *)pstChgStaFg->au64VirAddr[0], (CVI_U16)pstFgFlag->au32Stride[0],
			(CVI_U16 *)pstChgStaLife->au64VirAddr[0], (CVI_U16)pstChgStaLife->au32Stride[0],
			(CVI_U16)pstFgFlag->u32Width, (CVI_U16)pstFgFlag->u32Height,
			(CVI_U32 *)pstStatData->u64VirAddr,
			pstUpdateBgModelCtrl->u32CurFrmNum,
			pstUpdateBgModelCtrl->u32PreChkTime,
			pstUpdateBgModelCtrl->u32FrmChkPeriod,
			pstUpdateBgModelCtrl->u32InitMinTime,
			pstUpdateBgModelCtrl->u32StyBgMinBlendTime,
			pstUpdateBgModelCtrl->u32StyBgMaxBlendTime,
			pstUpdateBgModelCtrl->u32DynBgMinBlendTime,
			pstUpdateBgModelCtrl->u16FgMaxFadeTime,
			pstUpdateBgModelCtrl->u16BgMaxFadeTime,
			pstUpdateBgModelCtrl->u32StaticDetMinTime,
			pstUpdateBgModelCtrl->u8StyBgAccTimeRateThr,
			pstUpdateBgModelCtrl->u8ChgBgAccTimeRateThr,
			pstUpdateBgModelCtrl->u8DynBgAccTimeThr,
			pstUpdateBgModelCtrl->u8DynBgDepth,
			pstUpdateBgModelCtrl->u8BgEffStaRateThr,
			pstUpdateBgModelCtrl->u8AcceBgLearn,
			pstUpdateBgModelCtrl->u8DetChgRegion);

	return CVI_SUCCESS;
}

CVI_S32 CVI_MPI_IVE_GMM(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc, IVE_DST_IMAGE_S *pstFg,
	IVE_DST_IMAGE_S *pstBg, IVE_MEM_INFO_S *pstModel, IVE_GMM_CTRL_S *pstGmmCtrl, CVI_BOOL bInstant)
{
	printf("CVI_MPI_IVE_GMM\n");
    IVEReg _iveReg(pIveHandle, bInstant, pstSrc->u32Height, pstSrc->u32Width);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableTop();

    // top
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstSrc->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstSrc->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;
    iveReg->ive_gmm_c->REG_GMM_13.reg_gmm_gmm2_enable = bInstant ? 0x1 : bInstant;

    // setting
    iveReg->ive_gmm_c->REG_GMM_0.reg_gmm_learn_rate = pstGmmCtrl->u0q16LearnRate;
    iveReg->ive_gmm_c->REG_GMM_0.reg_gmm_bg_ratio = pstGmmCtrl->u0q16BgRatio;
    iveReg->ive_gmm_c->REG_GMM_1.reg_gmm_var_thr = pstGmmCtrl->u8q8VarThr;
    iveReg->ive_gmm_c->REG_GMM_2.reg_gmm_noise_var = pstGmmCtrl->u22q10NoiseVar;
    iveReg->ive_gmm_c->REG_GMM_3.reg_gmm_max_var = pstGmmCtrl->u22q10MaxVar;
    iveReg->ive_gmm_c->REG_GMM_4.reg_gmm_min_var = pstGmmCtrl->u22q10MinVar;
    iveReg->ive_gmm_c->REG_GMM_5.reg_gmm_init_weight = pstGmmCtrl->u0q16InitWeight;
    iveReg->ive_gmm_c->REG_GMM_6.reg_gmm_detect_shadow = 0; // enDetectShadow
    iveReg->ive_gmm_c->REG_GMM_6.reg_gmm_shadow_thr = 0; // u0q8ShadowThr
    iveReg->ive_gmm_c->REG_GMM_6.reg_gmm_sns_factor = 8; // u8SnsFactor, as from GMM2 sample
    iveReg->ive_gmm_c->REG_GMM_13.reg_gmm_gmm2_model_num = pstGmmCtrl->u8ModelNum;
    iveReg->srcEnType = pstSrc->enType;


    IVE_SRC_IMAGE_S _pstModel;
    _pstModel.au64PhyAddr[0] = pstModel->u64PhyAddr;
    _pstModel.au64VirAddr[0] = pstModel->u64VirAddr;
    iveReg->srcEnType = pstSrc->enType;
    iveReg->dstEnType = pstFg->enType;
    iveReg->setDMA(pstSrc, pstFg, &_pstModel, pstBg);

    return CVI_MPI_IVE_GMM(iveReg);
}

CVI_S32 CVI_MPI_IVE_GMM2(IVE_HANDLE *pIveHandle,IVE_SRC_IMAGE_S *pstSrc,IVE_SRC_IMAGE_S *pstFactor,
	IVE_DST_IMAGE_S *pstFg,IVE_DST_IMAGE_S *pstBg,IVE_DST_IMAGE_S *pstMatchModelInfo,
	IVE_MEM_INFO_S  *pstModel,IVE_GMM2_CTRL_S *pstGmm2Ctrl,CVI_BOOL bInstant)
{
	printf("CVI_MPI_IVE_GMM2\n");

    IVEReg _iveReg(pIveHandle, bInstant, pstSrc->u32Height, pstSrc->u32Width);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableTop();

    // top
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstSrc->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstSrc->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;
    // TODO: need to set?
    iveReg->ive_gmm_c->REG_GMM_13.reg_gmm_gmm2_enable = bInstant ? 0x10 : bInstant;

    // setting
    iveReg->ive_gmm_c->REG_GMM_7.reg_gmm2_life_update_factor = pstGmm2Ctrl->u16GlbLifeUpdateFactor;
    iveReg->ive_gmm_c->REG_GMM_7.reg_gmm2_var_rate = pstGmm2Ctrl->u16VarRate;
    iveReg->ive_gmm_c->REG_GMM_8.reg_gmm2_freq_redu_factor = pstGmm2Ctrl->u16FreqReduFactor;
    iveReg->ive_gmm_c->REG_GMM_8.reg_gmm2_max_var = pstGmm2Ctrl->u9q7MaxVar;
    iveReg->ive_gmm_c->REG_GMM_9.reg_gmm2_min_var = pstGmm2Ctrl->u9q7MinVar;
    iveReg->ive_gmm_c->REG_GMM_9.reg_gmm2_freq_add_factor = pstGmm2Ctrl->u16FreqAddFactor;
    iveReg->ive_gmm_c->REG_GMM_10.reg_gmm2_freq_init = pstGmm2Ctrl->u16FreqInitVal;
    iveReg->ive_gmm_c->REG_GMM_10.reg_gmm2_freq_thr = pstGmm2Ctrl->u16FreqThr;
    iveReg->ive_gmm_c->REG_GMM_11.reg_gmm2_life_thr = pstGmm2Ctrl->u16LifeThr;
    iveReg->ive_gmm_c->REG_GMM_11.reg_gmm2_sns_factor = pstGmm2Ctrl->u8GlbSnsFactor;
    // TODO: need to set?
    //iveReg->ive_gmm_c->.reg_gmm2_factor = 0;
    iveReg->ive_gmm_c->REG_GMM_12.reg_gmm2_life_update_factor_mode = pstGmm2Ctrl->enLifeUpdateFactorMode;
    iveReg->ive_gmm_c->REG_GMM_12.reg_gmm2_sns_factor_mode = pstGmm2Ctrl->enSnsFactorMode;
    // NOTICE: cmodel that y only set to 0
    iveReg->ive_gmm_c->REG_GMM_13.reg_gmm_gmm2_yonly = (pstSrc->enType == IVE_IMAGE_TYPE_U8C3_PACKAGE) ? 0 : 1;
    // TODO: reuse it?
    iveReg->ive_gmm_c->REG_GMM_13.reg_gmm_gmm2_model_num = pstGmm2Ctrl->u8ModelNum;

    IVE_SRC_IMAGE_S _pstModel;
    _pstModel.au64PhyAddr[0] = pstModel->u64PhyAddr;
    _pstModel.au64VirAddr[0] = pstModel->u64VirAddr;

    iveReg->srcEnType = pstSrc->enType;
    iveReg->dstEnType = pstFg->enType;
    iveReg->setDMA(pstSrc, pstFg, pstFactor, pstBg, &_pstModel, pstMatchModelInfo);

    return CVI_MPI_IVE_GMM2(iveReg);
}

CVI_S32 CVI_MPI_IVE_LKOpticalFlowPyr(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S astSrcPrevPyr[], IVE_SRC_IMAGE_S astSrcNextPyr[],
    IVE_SRC_MEM_INFO_S *pstPrevPts, IVE_MEM_INFO_S *pstNextPts, IVE_DST_MEM_INFO_S *pstStatus, IVE_DST_MEM_INFO_S *pstErr,
    IVE_LK_OPTICAL_FLOW_PYR_CTRL_S *pstLkOptiFlowPyrCtrl, CVI_BOOL bInstant)
{
	UNUSED(pIveHandle);
	UNUSED(bInstant);

	//printf("CVI_MPI_IVE_LKOpticalFlowPyr\n");

	//
	// TODO: Here shall be real MPI IVE driver code, which fill in register command, then pass to hw/cmodel
	// FIXME: call CVI_HW_XXX() directly for now
	//
	int level_num = pstLkOptiFlowPyrCtrl->u8MaxLevel + 1;
	CVI_U8 **SrcPrevPyr_Address = (CVI_U8 **)malloc(sizeof(CVI_U8 *) * level_num);
	CVI_U16 *SrcPrevPyr_u16Stride = (CVI_U16 *)malloc(sizeof(CVI_U16) * level_num);
	CVI_U8 **SrcNextPyr_Address = (CVI_U8 **)malloc(sizeof(CVI_U8 *) * level_num);
	CVI_U16 *SrcNextPyr_u16Stride = (CVI_U16 *)malloc(sizeof(CVI_U16) * level_num);
	CVI_U32 i;

	for(i=0; i<=pstLkOptiFlowPyrCtrl->u8MaxLevel; i++)
	{
		SrcPrevPyr_Address[i] = (CVI_U8 *)astSrcPrevPyr[i].au64VirAddr[0];
		SrcPrevPyr_u16Stride[i] = (CVI_U16)astSrcPrevPyr[i].au32Stride[0];
		SrcNextPyr_Address[i] = (CVI_U8 *)astSrcNextPyr[i].au64VirAddr[0];
		SrcNextPyr_u16Stride[i] = (CVI_U16)astSrcNextPyr[i].au32Stride[0];
	}

	CVI_HW_LKOpticalFlowPyr(
			SrcPrevPyr_Address, SrcPrevPyr_u16Stride,
			SrcNextPyr_Address, SrcNextPyr_u16Stride,
			(CVI_U16)astSrcPrevPyr[0].u32Width, (CVI_U16)astSrcPrevPyr[0].u32Height,
			pstLkOptiFlowPyrCtrl->u8MaxLevel,
			(IVE_POINT_S25Q7_S *)pstPrevPts->u64VirAddr, (IVE_POINT_S25Q7_S *)pstNextPts->u64VirAddr,
			(CVI_U8 *)pstStatus->u64VirAddr, (CVI_U16 *)pstErr->u64VirAddr,
			pstLkOptiFlowPyrCtrl->u16PtsNum, pstLkOptiFlowPyrCtrl->u8IterCnt,
			pstLkOptiFlowPyrCtrl->u0q8Eps, pstLkOptiFlowPyrCtrl->u0q8MinEigThr,
			pstLkOptiFlowPyrCtrl->bUseInitFlow, pstLkOptiFlowPyrCtrl->enOutMode);

	free(SrcPrevPyr_Address);
	free(SrcPrevPyr_u16Stride);
	free(SrcNextPyr_Address);
	free(SrcNextPyr_u16Stride);
	return CVI_SUCCESS;
}

CVI_S32 CVI_MPI_IVE_SAD(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc1,
    IVE_SRC_IMAGE_S *pstSrc2, IVE_DST_IMAGE_S *pstSad, IVE_DST_IMAGE_S
    *pstThr, IVE_SAD_CTRL_S *pstSadCtrl, CVI_BOOL bInstant)
{
	printf("CVI_MPI_IVE_Sad\n");

    // check input
    if (pstSrc1->enType != IVE_IMAGE_TYPE_U8C1 || pstSrc2->enType != IVE_IMAGE_TYPE_U8C1) {
        printf("pstSrc1->enType %d and pstSrc2->enType %d should be %d(IVE_IMAGE_TYPE_U8C1)\n",
                pstSrc1->enType, pstSrc2->enType, IVE_IMAGE_TYPE_U8C1);
        return CVI_FAILURE;
    }

    // check output
    IVE_SAD_OUT_CTRL_E enOutCtrl = pstSadCtrl->enOutCtrl;
    switch (enOutCtrl) {
        case IVE_SAD_OUT_CTRL_16BIT_BOTH:
            if (pstSad->enType != IVE_IMAGE_TYPE_U16C1
                    || pstThr->enType != IVE_IMAGE_TYPE_U8C1
               ) {
                printf("pstSad->enType %d (should be IVE_IMAGE_TYPE_U16C1(%d)"
                        " pstThr->enType %d should be %d(IVE_IMAGE_TYPE_U8C1)\n"
                        , pstSad->enType, IVE_IMAGE_TYPE_U16C1
                        , pstThr->enType, IVE_IMAGE_TYPE_U8C1
                        );
                return CVI_FAILURE;
            }
            break;
        case IVE_SAD_OUT_CTRL_16BIT_SAD: // dont care thr
            if (pstSad->enType != IVE_IMAGE_TYPE_U16C1
               ) {
                printf("pstSad->enType %d (should be IVE_IMAGE_TYPE_U16C1(%d)"
                        , pstSad->enType, IVE_IMAGE_TYPE_U16C1
                        );
                return CVI_FAILURE;
            }
            break;
        case IVE_SAD_OUT_CTRL_THRESH: // only output thresh
            if (
                    pstThr->enType != IVE_IMAGE_TYPE_U8C1
               ) {
                printf(
                        " pstThr->enType %d should be %d(IVE_IMAGE_TYPE_U8C1)\n"
                        , pstThr->enType, IVE_IMAGE_TYPE_U8C1
                        );
                return CVI_FAILURE;
            }
            break;
        case IVE_SAD_OUT_CTRL_8BIT_BOTH:
            if (pstSad->enType != IVE_IMAGE_TYPE_U8C1
                    || pstThr->enType != IVE_IMAGE_TYPE_U8C1
               ) {
                printf("pstSad->enType %d (should be IVE_IMAGE_TYPE_U8C1(%d)"
                        " pstThr->enType %d should be %d(IVE_IMAGE_TYPE_U8C1)\n"
                        , pstSad->enType, IVE_IMAGE_TYPE_U8C1
                        , pstThr->enType, IVE_IMAGE_TYPE_U8C1
                        );
                return CVI_FAILURE;
            }
            break;
        case IVE_SAD_OUT_CTRL_8BIT_SAD:
            if (pstSad->enType != IVE_IMAGE_TYPE_U8C1
               ) {
                printf("pstSad->enType %d (should be IVE_IMAGE_TYPE_U8C1(%d)"
                        , pstSad->enType, IVE_IMAGE_TYPE_U8C1
                        );
                return CVI_FAILURE;
            }
            break;
        default:
            printf("not support output type %d, return\n", enOutCtrl);
            return CVI_FAILURE;
    }

    IVEReg _iveReg(pIveHandle, bInstant, pstSrc1->u32Height, pstSrc1->u32Width);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableTop();

    // top
    // TODO: need to set?
    iveReg->ive_top_c->REG_h10.reg_sad_top_enable = bInstant;
    // align isp
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstSrc1->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstSrc1->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;
    // setting
    iveReg->ive_sad_c->REG_SAD_00.reg_sad_enmode = pstSadCtrl->enMode;
    iveReg->ive_sad_c->REG_SAD_00.reg_sad_out_ctrl = pstSadCtrl->enOutCtrl;
    iveReg->ive_sad_c->REG_SAD_00.reg_sad_u16bit_thr = pstSadCtrl->u16Thr;
    iveReg->ive_sad_c->REG_SAD_01.reg_sad_u8bit_max = pstSadCtrl->u8MaxVal;
    iveReg->ive_sad_c->REG_SAD_01.reg_sad_u8bit_min = pstSadCtrl->u8MinVal;

    // TODO: set dma
    iveReg->srcEnType = pstSrc1->enType;
    iveReg->dstEnType = pstSad->enType;
    iveReg->setDMA(pstSrc1, pstSad, pstSrc2, pstThr);

    return CVI_MPI_IVE_SAD(iveReg);
}

CVI_S32 CVI_MPI_IVE_GradFg(IVE_HANDLE *pIveHandle,
        IVE_SRC_IMAGE_S *pstBgDiffFg, IVE_SRC_IMAGE_S *pstCurGrad,
        IVE_SRC_IMAGE_S *pstBgGrad, IVE_DST_IMAGE_S *pstGradFg,
        IVE_GRAD_FG_CTRL_S *pstGradFgCtrl, CVI_BOOL bInstant) {
	printf("CVI_MPI_IVE_GradFg\n");

    IVEReg _iveReg(pIveHandle, bInstant, pstBgDiffFg->u32Height, pstBgDiffFg->u32Width);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableTop();
    iveReg->setEnableFilterop();

    // top
    // TODO: need to set?
    iveReg->ive_filterop_c->REG_33.reg_filterop_op2_gradfg_en = bInstant;
    // align isp
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstBgDiffFg->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstBgDiffFg->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;
    // setting

    // TODO: need softrst?
    //iveReg->ive_filterop_c->REG_33.reg_filterop_op2_gradfg_softrst = 0;
    iveReg->ive_filterop_c->REG_33.reg_filterop_op2_gradfg_enmode = pstGradFgCtrl->enMode;
    iveReg->ive_filterop_c->REG_33.reg_filterop_op2_gradfg_edwdark = pstGradFgCtrl->u8EdwDark;
    iveReg->ive_filterop_c->REG_33.reg_filterop_op2_gradfg_edwfactor = pstGradFgCtrl->u16EdwFactor;
    iveReg->ive_filterop_c->REG_34.reg_filterop_op2_gradfg_crlcoefthr = pstGradFgCtrl->u8CrlCoefThr;
    iveReg->ive_filterop_c->REG_34.reg_filterop_op2_gradfg_magcrlthr = pstGradFgCtrl->u8MagCrlThr;
    iveReg->ive_filterop_c->REG_34.reg_filterop_op2_gradfg_minmagdiff = pstGradFgCtrl->u8MinMagDiff;
    iveReg->ive_filterop_c->REG_34.reg_filterop_op2_gradfg_noiseval = pstGradFgCtrl->u8NoiseVal;

    // TODO: set dma
    iveReg->srcEnType = pstBgDiffFg->enType;
    iveReg->dstEnType = pstGradFg->enType;
    iveReg->setDMA(pstBgDiffFg, pstGradFg, pstCurGrad, NULL, pstBgGrad);

    return CVI_MPI_IVE_GradFg(iveReg);
}

CVI_S32 CVI_MPI_IVE_Bernsen(IVE_HANDLE *pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
	IVE_DST_IMAGE_S *pstDst, IVE_BERNSEN_CTRL_S *pstBernsenCtrl, CVI_BOOL bInstant)
{
#if 1
	printf("CVI_MPI_IVE_Bernsen\n");

    if (pstBernsenCtrl->u8WinSize != 3 && pstBernsenCtrl->u8WinSize != 5) {
        printf("not support u8WinSize %d, currently only support 3 or 5\n",
                pstBernsenCtrl->u8WinSize);
        return CVI_FAILURE;
    }
    IVEReg _iveReg(pIveHandle, bInstant, pstSrc->u32Height, pstSrc->u32Width);
    IVEReg* iveReg = &_iveReg;
    iveReg->setEnableFilterop();
    iveReg->setEnableTop();

    // top
    iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstDst->u32Width - 1;
    iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstDst->u32Height - 1;
    iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
    iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;
    // TODO: need bypass?
    //iveReg->ive_filterop_c->REG_h10.reg_filterop_mode = 7;
    // TODO: check 1 channels setting
    iveReg->ive_filterop_c->REG_h14.reg_filterop_op1_cmd = 5;
    // TODO: neet to another field to set
    //iveReg->ive_filterop_c->.reg_filterop_lbp_u8bit_thr = 5;
    iveReg->ive_filterop_c->REG_19.reg_filterop_bernsen_mode= pstBernsenCtrl->enMode;
    iveReg->ive_filterop_c->REG_19.reg_filterop_bernsen_win5x5_en= pstBernsenCtrl->u8WinSize == 5 ;
    iveReg->ive_filterop_c->REG_19.reg_filterop_bernsen_thr= pstBernsenCtrl->u8Thr;
    // TODO: add more register to save it
    iveReg->ive_filterop_c->REG_20.reg_filterop_lbp_u8bit_thr = pstBernsenCtrl->u8ContrastThreshold;

    iveReg->srcEnType = pstSrc->enType;
    iveReg->dstEnType = pstDst->enType;
    iveReg->setDMA(pstSrc, pstDst);
    return CVI_MPI_IVE_Bernsen(iveReg);

#else
	UNUSED(pIveHandle);
	UNUSED(bInstant);

	CVI_HW_Bernsen(
			(CVI_U8 *)pstSrc->au64VirAddr[0], (CVI_U16)pstSrc->au32Stride[0],
			(CVI_U8 *)pstDst->au64VirAddr[0], (CVI_U16)pstDst->au32Stride[0],
			(CVI_U16)pstDst->u32Width, (CVI_U16)pstDst->u32Height,
			pstBernsenCtrl->u8WinSize, pstBernsenCtrl->u8Thr,
            pstBernsenCtrl->u8ContrastThreshold, pstBernsenCtrl->enMode);
	return CVI_SUCCESS;
    // FIXME:
#endif
}


// export to shift address
static uintptr_t reg_base = 0;
static uintptr_t dma_base = 0;

void setRegBase(uintptr_t _reg_base) {
    reg_base = _reg_base;
}

uintptr_t getRegBase(void) {
    return reg_base;
}

void setDmaBase(uintptr_t _dma_base) {
    dma_base = _dma_base;
}

uint64_t getDmaBase(void) {
    return dma_base;
}

uint64_t *ive_get_phys_reg_bases(void)
{
    static uint64_t m_ive_phys_base_list[IVE_BLK_ID_MAX] = {
        [IVE_BLK_ID_IVE_TOP]              = (IVE_BLK_BA_IVE_TOP),
        [IVE_BLK_ID_IMG_IN]               = (IVE_BLK_BA_IMG_IN),
        [IVE_BLK_ID_RDMA_IMG1]            = (IVE_BLK_BA_RDMA_IMG1),
        [IVE_BLK_ID_MAP]                  = (IVE_BLK_BA_MAP),
        [IVE_BLK_ID_HIST]                 = (IVE_BLK_BA_HIST),
        [IVE_BLK_ID_INTG]                 = (IVE_BLK_BA_INTG),
        [IVE_BLK_ID_SAD]                  = (IVE_BLK_BA_SAD),
        [IVE_BLK_ID_SAD_WDMA]             = (IVE_BLK_BA_SAD_WDMA),
        [IVE_BLK_ID_SAD_WDMA_THR]         = (IVE_BLK_BA_SAD_WDMA_THR),
        [IVE_BLK_ID_NCC]                  = (IVE_BLK_BA_NCC),
        [IVE_BLK_ID_GMM_MODEL_RDMA_0]     = (IVE_BLK_BA_GMM_MODEL_RDMA_0),
        [IVE_BLK_ID_GMM_MODEL_RDMA_1]     = (IVE_BLK_BA_GMM_MODEL_RDMA_1),
        [IVE_BLK_ID_GMM_MODEL_RDMA_2]     = (IVE_BLK_BA_GMM_MODEL_RDMA_2),
        [IVE_BLK_ID_GMM_MODEL_RDMA_3]     = (IVE_BLK_BA_GMM_MODEL_RDMA_3),
        [IVE_BLK_ID_GMM_MODEL_RDMA_4]     = (IVE_BLK_BA_GMM_MODEL_RDMA_4),
        [IVE_BLK_ID_GMM_MODEL_WDMA_0]     = (IVE_BLK_BA_GMM_MODEL_WDMA_0),
        [IVE_BLK_ID_GMM_MODEL_WDMA_1]     = (IVE_BLK_BA_GMM_MODEL_WDMA_1),
        [IVE_BLK_ID_GMM_MODEL_WDMA_2]     = (IVE_BLK_BA_GMM_MODEL_WDMA_2),
        [IVE_BLK_ID_GMM_MODEL_WDMA_3]     = (IVE_BLK_BA_GMM_MODEL_WDMA_3),
        [IVE_BLK_ID_GMM_MODEL_WDMA_4]     = (IVE_BLK_BA_GMM_MODEL_WDMA_4),
        [IVE_BLK_ID_GMM]                  = (IVE_BLK_BA_GMM),
        [IVE_BLK_ID_GMM2_MODEL_RDMA_0]    = (IVE_BLK_BA_GMM2_MODEL_RDMA_0),
        [IVE_BLK_ID_GMM2_MODEL_RDMA_1]    = (IVE_BLK_BA_GMM2_MODEL_RDMA_1),
        [IVE_BLK_ID_GMM2_MODEL_RDMA_2]    = (IVE_BLK_BA_GMM2_MODEL_RDMA_2),
        [IVE_BLK_ID_GMM2_MODEL_RDMA_3]    = (IVE_BLK_BA_GMM2_MODEL_RDMA_3),
        [IVE_BLK_ID_GMM2_MODEL_RDMA_4]    = (IVE_BLK_BA_GMM2_MODEL_RDMA_4),
        [IVE_BLK_ID_GMM2_MODEL_WDMA_0]    = (IVE_BLK_BA_GMM2_MODEL_WDMA_0),
        [IVE_BLK_ID_GMM2_MODEL_WDMA_1]    = (IVE_BLK_BA_GMM2_MODEL_WDMA_1),
        [IVE_BLK_ID_GMM2_MODEL_WDMA_2]    = (IVE_BLK_BA_GMM2_MODEL_WDMA_2),
        [IVE_BLK_ID_GMM2_MODEL_WDMA_3]    = (IVE_BLK_BA_GMM2_MODEL_WDMA_3),
        [IVE_BLK_ID_GMM2_MODEL_WDMA_4]    = (IVE_BLK_BA_GMM2_MODEL_WDMA_4),
        [IVE_BLK_ID_GMM2_MATCH_WDMA]      = (IVE_BLK_BA_GMM2_MATCH_WDMA),
        [IVE_BLK_ID_GMM2]                 = (IVE_BLK_BA_GMM2),
        [IVE_BLK_ID_BG_MATCH]             = (IVE_BLK_BA_BG_MATCH),
        [IVE_BLK_ID_BG_UPDATE]            = (IVE_BLK_BA_BG_UPDATE),
        [IVE_BLK_ID_FILTEROP]             = (IVE_BLK_BA_FILTEROP),
        [IVE_BLK_ID_CCL]                  = (IVE_BLK_BA_CCL),
        [IVE_BLK_ID_CCL_SRC_RDMA]         = (IVE_BLK_BA_CCL_SRC_RDMA),
        [IVE_BLK_ID_CCL_DST_WDMA]         = (IVE_BLK_BA_CCL_DST_WDMA),
        [IVE_BLK_ID_CCL_REGION_WDMA]      = (IVE_BLK_BA_CCL_REGION_WDMA),
        [IVE_BLK_ID_DMAF]                 = (IVE_BLK_BA_DMAF),
        [IVE_BLK_ID_LK]                   = (IVE_BLK_BA_LK),
        [IVE_BLK_ID_RDMA_EIGVAL]          = (IVE_BLK_BA_RDMA_EIGVAL),
        [IVE_BLK_ID_WDMA]                 = (IVE_BLK_BA_WDMA),
        [IVE_BLK_ID_RDMA]                 = (IVE_BLK_BA_RDMA),
    };
    return m_ive_phys_base_list;
}

void ive_set_base_addr(void *base)
{
	uint64_t *addr = ive_get_phys_reg_bases();
	int i = 0;

	for (i = 0; i < IVE_BLK_ID_MAX; ++i) {
		addr[i] -= getRegBase();
		addr[i] += (uintptr_t)base;
	}
	setRegBase((uintptr_t)base);
}


CVI_S32 CVI_MPI_IVE_imgInToOdma(IVE_HANDLE *pIveHandle,
        IVE_SRC_IMAGE_S *pstSrc, IVE_DST_IMAGE_S *pstDst,
        IVE_FILTER_CTRL_S *pstFltCtrl,CVI_BOOL bInstant)
{
  printf("CVI_MPI_IVE_imgInToOdma\n");

  IVEReg _iveReg(pIveHandle, bInstant, pstDst->u32Height, pstDst->u32Width,
      IVEModuleFILTEROP);
  IVEReg* iveReg = &_iveReg;

  iveReg->setEnableFilterop();
  iveReg->setEnableTop();

  // top
  iveReg->ive_top_c->REG_0.reg_img_in_widthm1 = pstDst->u32Width - 1;
  iveReg->ive_top_c->REG_0.reg_img_in_heightm1 = pstDst->u32Height - 1;
  iveReg->ive_top_c->REG_2.reg_img_heightm1 = iveReg->ive_top_c->REG_0.reg_img_in_heightm1;
  iveReg->ive_top_c->REG_2.reg_img_widthm1 = iveReg->ive_top_c->REG_0.reg_img_in_widthm1;

  iveReg->srcEnType = pstSrc->enType;
  iveReg->dstEnType = pstDst->enType;
  iveReg->setDMA(pstSrc, pstDst);

  return CVI_MPI_IVE_imgInToOdma(iveReg);
}

CVI_S32 CVI_MPI_IVE_rgbPToYuvToErodeToDilate(IVE_HANDLE *pIveHandle,
        IVE_SRC_IMAGE_S *pstSrc, IVE_DST_IMAGE_S *pstDst,
        IVE_DST_IMAGE_S *pstDst2,
        IVE_FILTER_CTRL_S *pstFltCtrl,CVI_BOOL bInstant)
{
  printf("CVI_MPI_IVE_rgbPToYuvToErodeToDilate\n");

  IVEReg _iveReg(pIveHandle, bInstant, pstDst->u32Height, pstDst->u32Width,
      IVEModuleFILTEROP);
  IVEReg* iveReg = &_iveReg;

  iveReg->setEnableFilterop();
  iveReg->setEnableTop();

  iveReg->srcEnType = pstSrc->enType;
  iveReg->dstEnType = pstDst->enType;
  iveReg->setDMA(pstSrc, pstDst, NULL, pstDst2);

  return CVI_MPI_IVE_rgbPToYuvToErodeToDilate(iveReg);
}
