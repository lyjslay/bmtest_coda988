#include "Thresh_S16.h"
#include "sample_assist.h"
#include "sample_file.h"
#include "sample_define.h"
#include "mpi_ive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CVI_S32 Thresh_S16Sample(IVE_IMAGE_TYPE_E enType,IVE_THRESH_S16_MODE_E enMode,
		const CVI_CHAR *resultFileName)
{
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_SRC_IMAGE_S stRef;
	IVE_THRESH_S16_CTRL_S stCtrl;
	CVI_S32 s32Result;
	CVI_U16 u16Width;
	CVI_U16 u16Height;
	FILE *fp;
	FILE *fp2;
	IVE_HANDLE handle;
	CVI_DOUBLE dTime;
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_s16_00_704x576_s16_start;
	const CVI_CHAR *fileName = &_binary_test_ive_data_s16_00_704x576_s16_start;

#else
	const CVI_CHAR *fileName = "../data/s16/00_704x576.s16";

#endif
	memset(&stSrc,0,sizeof(IVE_SRC_IMAGE_S));
	memset(&stDst,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stRef,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stCtrl,0,sizeof(IVE_THRESH_S16_CTRL_S));

	fp = NULL;
	fp2 = NULL;
	u16Width = CVI_IMAGE_WIDTH_704;
	u16Height = CVI_IMAGE_HEIGHT_576;

	s32Result = CVI_CreateIveImage(&stSrc,IVE_IMAGE_TYPE_S16C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stDst,enType,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stRef,enType,u16Width,u16Height);
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
	if (( IVE_THRESH_S16_MODE_S16_TO_S8_MIN_MID_MAX == enMode)
		||(IVE_THRESH_S16_MODE_S16_TO_S8_MIN_ORI_MAX == enMode))
	{
		stCtrl.s16LowThr = CVI_GenRand(CVI_MAX_S8,CVI_MIN_S8);
		stCtrl.s16HighThr = CVI_GenRand(CVI_MAX_S8,stCtrl.s16LowThr + 1);
		stCtrl.un8MinVal.s8Val = CVI_GenRand(CVI_MAX_S8,CVI_MIN_S8);
		stCtrl.un8MidVal.s8Val = CVI_GenRand(CVI_MAX_S8,CVI_MIN_S8);
		stCtrl.un8MaxVal.s8Val = CVI_GenRand(CVI_MAX_S8,CVI_MIN_S8);
		printf("enMode = %d: s16LowThr = %d, s16HighThr = %d, s8Min Mid Max (%d, %d, %d)\n",
			enMode, stCtrl.s16LowThr, stCtrl.s16HighThr,
			stCtrl.un8MinVal.s8Val, stCtrl.un8MidVal.s8Val, stCtrl.un8MaxVal.s8Val);

	}else
	{
		stCtrl.s16LowThr = CVI_GenRand(CVI_MAX_U8,CVI_MIN_U8);
		stCtrl.s16HighThr = CVI_GenRand(CVI_MAX_U8,stCtrl.s16LowThr + 1);
		stCtrl.un8MinVal.u8Val = CVI_GenRand(CVI_MAX_U8,CVI_MIN_U8);
		stCtrl.un8MidVal.u8Val = CVI_GenRand(CVI_MAX_U8,CVI_MIN_U8);
		stCtrl.un8MaxVal.u8Val = CVI_GenRand(CVI_MAX_U8,CVI_MIN_U8);
		printf("enMode = %d: s16LowThr = %d, s16HighThr = %d, u8Min Mid Max (%d, %d, %d)\n",
			enMode, stCtrl.s16LowThr, stCtrl.s16HighThr,
			stCtrl.un8MinVal.u8Val, stCtrl.un8MidVal.u8Val, stCtrl.un8MaxVal.u8Val);
	}
#else
	switch (stCtrl.enMode)
	{
	case 0:
		// enMode = 0: s16LowThr =  41, s16HighThr = 105, s8Min Mid Max (-63, -5, -98)
		stCtrl.s16LowThr = 41;
		stCtrl.s16HighThr = 105;
		stCtrl.un8MinVal.s8Val = -63;
		stCtrl.un8MidVal.s8Val = -5;
		stCtrl.un8MaxVal.s8Val = -98;
		printf("enMode = %d: s16LowThr = %d, s16HighThr = %d, s8Min Mid Max (%d, %d, %d)\n",
			enMode, stCtrl.s16LowThr, stCtrl.s16HighThr,
			stCtrl.un8MinVal.s8Val, stCtrl.un8MidVal.s8Val, stCtrl.un8MaxVal.s8Val);
		break;
	case 1:
		// enMode = 1: s16LowThr = 108, s16HighThr = 111, s8Min Mid Max (-47, 82, -17)
		stCtrl.s16LowThr = 108;
		stCtrl.s16HighThr = 111;
		stCtrl.un8MinVal.s8Val = -47;
		stCtrl.un8MidVal.s8Val = 82;
		stCtrl.un8MaxVal.s8Val = -17;
		printf("enMode = %d: s16LowThr = %d, s16HighThr = %d, s8Min Mid Max (%d, %d, %d)\n",
			enMode, stCtrl.s16LowThr, stCtrl.s16HighThr,
			stCtrl.un8MinVal.s8Val, stCtrl.un8MidVal.s8Val, stCtrl.un8MaxVal.s8Val);
		break;
	case 2:
		// enMode = 2: s16LowThr =  73, s16HighThr = 191, u8Min Mid Max (241, 187, 233)
		stCtrl.s16LowThr = 73;
		stCtrl.s16HighThr = 191;
		stCtrl.un8MinVal.u8Val = 241;
		stCtrl.un8MidVal.u8Val = 187;
		stCtrl.un8MaxVal.u8Val = 233;
		printf("enMode = %d: s16LowThr = %d, s16HighThr = %d, u8Min Mid Max (%d, %d, %d)\n",
			enMode, stCtrl.s16LowThr, stCtrl.s16HighThr,
			stCtrl.un8MinVal.u8Val, stCtrl.un8MidVal.u8Val, stCtrl.un8MaxVal.u8Val);
		break;
	case 3:
		// enMode = 3: s16LowThr = 235, s16HighThr = 251, u8Min Mid Max (166, 219, 60)
		stCtrl.s16LowThr = 235;
		stCtrl.s16HighThr = 251;
		stCtrl.un8MinVal.u8Val = 166;
		stCtrl.un8MidVal.u8Val = 219;
		stCtrl.un8MaxVal.u8Val = 60;
		printf("enMode = %d: s16LowThr = %d, s16HighThr = %d, u8Min Mid Max (%d, %d, %d)\n",
			enMode, stCtrl.s16LowThr, stCtrl.s16HighThr,
			stCtrl.un8MinVal.u8Val, stCtrl.un8MidVal.u8Val, stCtrl.un8MaxVal.u8Val);
		break;
	default:
		CVI_ASSERT(0);
		break;
	}
#endif

	dTime = (CVI_DOUBLE)CVI_GetTickCount();
	s32Result = CVI_MPI_IVE_Thresh_S16(&handle,&stSrc,&stDst,&stCtrl,CVI_FALSE);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	dTime = (CVI_DOUBLE)CVI_GetTickCount() - dTime;

	printf("Thresh_S16 mode %d,time = %g ms\n",enMode, dTime/((CVI_DOUBLE)CVI_GetTickFrequency()*1000.));

	s32Result = CVI_CompareIveImage(&stDst, &stRef);

FAILURE:

	CVI_FCLOSE(fp);
	CVI_FCLOSE(fp2);
	CVI_DestroyIveImage(&stSrc);
	CVI_DestroyIveImage(&stDst);
	CVI_DestroyIveImage(&stRef);

	return s32Result;
}

CVI_S32 Thresh_S16SampleToS8MinMidMax(CVI_VOID)
{
	IVE_IMAGE_TYPE_E enType = IVE_IMAGE_TYPE_S8C1;
	IVE_THRESH_S16_MODE_E enMode = IVE_THRESH_S16_MODE_S16_TO_S8_MIN_MID_MAX;
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Thresh_S16_To_S8_MinMidMax_yuv_start;
	const CVI_CHAR *resultFileName = &_binary_test_ive_data_result_sample_Thresh_S16_To_S8_MinMidMax_yuv_start;

#else
	const CVI_CHAR *resultFileName = "../data/result/sample_Thresh_S16_To_S8_MinMidMax.yuv";

#endif
	return Thresh_S16Sample(enType,enMode,resultFileName);
}
CVI_S32 Thresh_S16SampleToS8MinOriMax(CVI_VOID)
{
	IVE_IMAGE_TYPE_E enType = IVE_IMAGE_TYPE_S8C1;
	IVE_THRESH_S16_MODE_E enMode = IVE_THRESH_S16_MODE_S16_TO_S8_MIN_ORI_MAX;
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Thresh_S16_To_S8_MinOriMax_yuv_start;
	const CVI_CHAR *resultFileName = &_binary_test_ive_data_result_sample_Thresh_S16_To_S8_MinOriMax_yuv_start;

#else
	const CVI_CHAR *resultFileName = "../data/result/sample_Thresh_S16_To_S8_MinOriMax.yuv";

#endif
	return Thresh_S16Sample(enType,enMode,resultFileName);
}
CVI_S32 Thresh_S16SampleToU8MinMidMax(CVI_VOID)
{
	IVE_IMAGE_TYPE_E enType = IVE_IMAGE_TYPE_U8C1;
	IVE_THRESH_S16_MODE_E enMode = IVE_THRESH_S16_MODE_S16_TO_U8_MIN_MID_MAX;
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Thresh_S16_To_U8_MinMidMax_yuv_start;
	const CVI_CHAR *resultFileName = &_binary_test_ive_data_result_sample_Thresh_S16_To_U8_MinMidMax_yuv_start;

#else
	const CVI_CHAR *resultFileName = "../data/result/sample_Thresh_S16_To_U8_MinMidMax.yuv";

#endif
	return Thresh_S16Sample(enType,enMode,resultFileName);
}
CVI_S32 Thresh_S16SampleToU8MinOriMax(CVI_VOID)
{
	IVE_IMAGE_TYPE_E enType = IVE_IMAGE_TYPE_U8C1;
	IVE_THRESH_S16_MODE_E enMode = IVE_THRESH_S16_MODE_S16_TO_U8_MIN_ORI_MAX;
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Thresh_S16_To_U8_MinOriMax_yuv_start;
	const CVI_CHAR *resultFileName = &_binary_test_ive_data_result_sample_Thresh_S16_To_U8_MinOriMax_yuv_start;

#else
	const CVI_CHAR *resultFileName = "../data/result/sample_Thresh_S16_To_U8_MinOriMax.yuv";

#endif
	return Thresh_S16Sample(enType,enMode,resultFileName);
}
