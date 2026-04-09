#include "Thresh_U16.h"
#include "sample_assist.h"
#include "sample_file.h"
#include "sample_define.h"
#include "mpi_ive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CVI_S32 Thresh_U16Sample(IVE_THRESH_U16_MODE_E enMode, const CVI_CHAR *resultFileName)
{
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_SRC_IMAGE_S stRef;
	IVE_THRESH_U16_CTRL_S stCtrl;
	CVI_S32 s32Result;
	CVI_U16 u16Width;
	CVI_U16 u16Height;
	FILE *fp;
	FILE *fp2;
	IVE_HANDLE handle;
	CVI_DOUBLE dTime;
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_u16_00_704x576_u16_start;
	const CVI_CHAR *fileName = &_binary_test_ive_data_u16_00_704x576_u16_start;

#else
	const CVI_CHAR *fileName = "../data/u16/00_704x576.u16";

#endif
	memset(&stSrc,0,sizeof(IVE_SRC_IMAGE_S));
	memset(&stDst,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stRef,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stCtrl,0,sizeof(IVE_THRESH_U16_CTRL_S));

	fp = NULL;
	fp2 = NULL;
	u16Width = CVI_IMAGE_WIDTH_704;
	u16Height = CVI_IMAGE_HEIGHT_576;

	s32Result = CVI_CreateIveImage(&stSrc,IVE_IMAGE_TYPE_U16C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stDst,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stRef,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	CVI_ResetIveImage(&stDst,0);

	s32Result = fopen_s(&fp,fileName,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
	s32Result = fopen_s(&fp2,resultFileName,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);

	s32Result = CVI_ReadFile(&stSrc,fp);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_ReadFile(&stRef,fp2);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	stCtrl.enMode = enMode;
#if 0
	stCtrl.u16LowThr = CVI_GenRand(CVI_MAX_U8,CVI_MIN_U8);
	stCtrl.u16HighThr = CVI_GenRand(CVI_MAX_U8,stCtrl.u16LowThr + 1);
	stCtrl.u8MinVal = CVI_GenRand(CVI_MAX_U8,CVI_MIN_U8);
	stCtrl.u8MidVal = CVI_GenRand(CVI_MAX_U8,CVI_MIN_U8);
	stCtrl.u8MaxVal = CVI_GenRand(CVI_MAX_U8,CVI_MIN_U8);
#else
	switch (stCtrl.enMode)
	{
	case 0:
		// enMode = 0: u16LowThr =  41, u16HighThr = 105, u8Min Mid Max (190, 132, 225)
		stCtrl.u16LowThr = 41;
		stCtrl.u16HighThr = 105;
		stCtrl.u8MinVal = 190;
		stCtrl.u8MidVal = 132;
		stCtrl.u8MaxVal = 225;
		break;
	case 1:
		// enMode = 1: u16LowThr = 108, u16HighThr = 121, u8Min Mid Max (174, 82, 144)
		stCtrl.u16LowThr = 108;
		stCtrl.u16HighThr = 121;
		stCtrl.u8MinVal = 174;
		stCtrl.u8MidVal = 82;
		stCtrl.u8MaxVal = 144;
		break;
	default:
		CVI_ASSERT(0);
		break;
	}
#endif
	printf("enMode = %d: u16LowThr = %d, u16HighThr = %d, u8Min Mid Max (%d, %d, %d)\n",
			enMode, stCtrl.u16LowThr, stCtrl.u16HighThr,
			stCtrl.u8MinVal, stCtrl.u8MidVal, stCtrl.u8MaxVal);

	dTime = (CVI_DOUBLE)CVI_GetTickCount();
	s32Result = CVI_MPI_IVE_Thresh_U16(&handle,&stSrc,&stDst,&stCtrl,CVI_FALSE);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	dTime = (CVI_DOUBLE)CVI_GetTickCount() - dTime;

	printf("Thresh_U16 mode %d,time = %g ms\n",enMode, dTime/((CVI_DOUBLE)CVI_GetTickFrequency()*1000.));

	s32Result = CVI_CompareIveImage(&stDst, &stRef);

FAILURE:

	CVI_FCLOSE(fp);
	CVI_FCLOSE(fp2);
	CVI_DestroyIveImage(&stSrc);
	CVI_DestroyIveImage(&stDst);
	CVI_DestroyIveImage(&stRef);

	return s32Result;
}

CVI_S32 Thresh_U16SampleToU8MinMidMax(CVI_VOID)
{
	IVE_THRESH_U16_MODE_E enMode = IVE_THRESH_U16_MODE_U16_TO_U8_MIN_MID_MAX;
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Thresh_U16_To_U8_MinMidMax_yuv_start;
	const CVI_CHAR *resultFileName = &_binary_test_ive_data_result_sample_Thresh_U16_To_U8_MinMidMax_yuv_start;

#else
	const CVI_CHAR *resultFileName = "../data/result/sample_Thresh_U16_To_U8_MinMidMax.yuv";

#endif
	return Thresh_U16Sample(enMode, resultFileName);
}
CVI_S32 Thresh_U16SampleToU8MinOriMax(CVI_VOID)
{
	IVE_THRESH_U16_MODE_E enMode = IVE_THRESH_U16_MODE_U16_TO_U8_MIN_ORI_MAX;
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Thresh_U16_To_U8_MinOriMax_yuv_start;
	const CVI_CHAR *resultFileName = &_binary_test_ive_data_result_sample_Thresh_U16_To_U8_MinOriMax_yuv_start;

#else
	const CVI_CHAR *resultFileName = "../data/result/sample_Thresh_U16_To_U8_MinOriMax.yuv";

#endif
	return Thresh_U16Sample(enMode, resultFileName);
}

