#include "Thresh.h"
#include "sample_assist.h"
#include "sample_file.h"
#include "sample_define.h"
#include "mpi_ive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CVI_S32 ThreshSample(IVE_THRESH_MODE_E enMode, const CVI_CHAR *resultFileName)
{
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_SRC_IMAGE_S stRef;
	IVE_THRESH_CTRL_S stCtrl;
	CVI_S32 s32Result;
	CVI_U16 u16Width;
	CVI_U16 u16Height;
	FILE *fp;
	FILE *fp2;
	IVE_HANDLE handle;
	CVI_DOUBLE dTime;
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_y_00_352x288_y_yuv_start;
	const CVI_CHAR *fileName = &_binary_test_ive_data_y_00_352x288_y_yuv_start;

#else
	const CVI_CHAR *fileName = "../data/y/00_352x288_y.yuv";

#endif
	memset(&stSrc,0,sizeof(IVE_SRC_IMAGE_S));
	memset(&stDst,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stRef,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stCtrl,0,sizeof(IVE_THRESH_CTRL_S));

	fp = NULL;
	fp2 = NULL;
	u16Width = CVI_IMAGE_WIDTH_352;
	u16Height = CVI_IMAGE_HEIGHT_288;

	s32Result = CVI_CreateIveImage(&stSrc,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
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
	stCtrl.u8LowThr = CVI_GenRand(CVI_MAX_U8 - 1,CVI_MIN_U8);
	stCtrl.u8HighThr = CVI_GenRand(CVI_MAX_U8,stCtrl.u8LowThr + 1);
	stCtrl.u8MinVal = CVI_GenRand(CVI_MAX_U8,CVI_MIN_U8);
	stCtrl.u8MidVal = CVI_GenRand(CVI_MAX_U8,CVI_MIN_U8);
	stCtrl.u8MaxVal = CVI_GenRand(CVI_MAX_U8,CVI_MIN_U8);
#else
	switch (stCtrl.enMode)
	{
	case 0:
		// enMode = 0: u8LowThr =  41, u8HighThr = 105, u8Min Mid Max (190, 132, 225)
		stCtrl.u8LowThr = 41;
		stCtrl.u8HighThr = 105;
		stCtrl.u8MinVal = 190;
		stCtrl.u8MidVal = 132;
		stCtrl.u8MaxVal = 225;
		break;
	case 1:
		// enMode = 1: u8LowThr = 169, u8HighThr = 210, u8Min Mid Max (174, 82, 144)
		stCtrl.u8LowThr = 169;
		stCtrl.u8HighThr = 210;
		stCtrl.u8MinVal = 174;
		stCtrl.u8MidVal = 82;
		stCtrl.u8MaxVal = 144;
		break;
	case 2:
		// enMode = 2: u8LowThr =  95, u8HighThr = 241, u8Min Mid Max (241, 187, 233)
		stCtrl.u8LowThr = 95;
		stCtrl.u8HighThr = 241;
		stCtrl.u8MinVal = 241;
		stCtrl.u8MidVal = 187;
		stCtrl.u8MaxVal = 233;
		break;
	case 3:
		// enMode = 3: u8LowThr = 236, u8HighThr = 249, u8Min Mid Max (166, 219, 60)
		stCtrl.u8LowThr = 236;
		stCtrl.u8HighThr = 249;
		stCtrl.u8MinVal = 166;
		stCtrl.u8MidVal = 219;
		stCtrl.u8MaxVal = 60;
		break;
	case 4:
		// enMode = 4: u8LowThr =   6, u8HighThr = 169, u8Min Mid Max (62, 153, 36)
		stCtrl.u8LowThr = 6;
		stCtrl.u8HighThr = 169;
		stCtrl.u8MinVal = 62;
		stCtrl.u8MidVal = 153;
		stCtrl.u8MaxVal = 36;
		break;
	case 5:
		// enMode = 5: u8LowThr = 142, u8HighThr = 162, u8Min Mid Max (28, 6, 183)
		stCtrl.u8LowThr = 142;
		stCtrl.u8HighThr = 162;
		stCtrl.u8MinVal = 28;
		stCtrl.u8MidVal = 6;
		stCtrl.u8MaxVal = 183;
		break;
	case 6:
		// enMode = 6: u8LowThr =  92, u8HighThr = 140, u8Min Mid Max (179, 18, 77)
		stCtrl.u8LowThr = 92;
		stCtrl.u8HighThr = 140;
		stCtrl.u8MinVal = 179;
		stCtrl.u8MidVal = 18;
		stCtrl.u8MaxVal = 77;
		break;
	case 7:
		// enMode = 7: u8LowThr =  22, u8HighThr =  60, u8Min Mid Max (187, 139, 166)
		stCtrl.u8LowThr = 22;
		stCtrl.u8HighThr = 60;
		stCtrl.u8MinVal = 187;
		stCtrl.u8MidVal = 139;
		stCtrl.u8MaxVal = 166;
		break;
	default:
		CVI_ASSERT(0);
		break;
	}
#endif
	printf("enMode = %d: u8LowThr = %d, u8HighThr = %d, u8Min Mid Max (%d, %d, %d)\n",
			enMode, stCtrl.u8LowThr, stCtrl.u8HighThr, stCtrl.u8MinVal, stCtrl.u8MidVal, stCtrl.u8MaxVal);

	dTime = (CVI_DOUBLE)CVI_GetTickCount();
	s32Result = CVI_MPI_IVE_Thresh(&handle,&stSrc,&stDst,&stCtrl,CVI_FALSE);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	dTime = (CVI_DOUBLE)CVI_GetTickCount() - dTime;

	printf("Thresh mode %d,time = %g ms\n",enMode, dTime/((CVI_DOUBLE)CVI_GetTickFrequency()*1000.));

	s32Result = CVI_CompareIveImage(&stDst, &stRef);

FAILURE:

	CVI_FCLOSE(fp);
	CVI_FCLOSE(fp2);
	CVI_DestroyIveImage(&stSrc);
	CVI_DestroyIveImage(&stDst);
	CVI_DestroyIveImage(&stRef);

	return s32Result;
}

CVI_S32 ThreshSampleBinary(CVI_VOID)
{
	IVE_THRESH_MODE_E enMode = IVE_THRESH_MODE_BINARY;
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Thresh_Binary_yuv_start;
	const CVI_CHAR *resultFileName = &_binary_test_ive_data_result_sample_Thresh_Binary_yuv_start;

#else
	const CVI_CHAR *resultFileName = "../data/result/sample_Thresh_Binary.yuv";

#endif
	return ThreshSample(enMode, resultFileName);
}
CVI_S32 ThreshSampleTrunc(CVI_VOID)
{
	IVE_THRESH_MODE_E enMode = IVE_THRESH_MODE_TRUNC;
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Thresh_Trunc_yuv_start;
	const CVI_CHAR *resultFileName = &_binary_test_ive_data_result_sample_Thresh_Trunc_yuv_start;

#else
	const CVI_CHAR *resultFileName = "../data/result/sample_Thresh_Trunc.yuv";

#endif
	return ThreshSample(enMode, resultFileName);
}
CVI_S32 ThreshSampleToMinval (CVI_VOID)
{
	IVE_THRESH_MODE_E enMode = IVE_THRESH_MODE_TO_MINVAL;
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Thresh_ToMinVal_yuv_start;
	const CVI_CHAR *resultFileName = &_binary_test_ive_data_result_sample_Thresh_ToMinVal_yuv_start;

#else
	const CVI_CHAR *resultFileName = "../data/result/sample_Thresh_ToMinVal.yuv";

#endif
	return ThreshSample(enMode, resultFileName);
}
CVI_S32 ThreshSampleMinMidMax(CVI_VOID)
{
	IVE_THRESH_MODE_E enMode = IVE_THRESH_MODE_MIN_MID_MAX;
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Thresh_MinMidMax_yuv_start;
	const CVI_CHAR *resultFileName = &_binary_test_ive_data_result_sample_Thresh_MinMidMax_yuv_start;

#else
	const CVI_CHAR *resultFileName = "../data/result/sample_Thresh_MinMidMax.yuv";

#endif
	return ThreshSample(enMode, resultFileName);
}
CVI_S32 ThreshSampleOriMidMax(CVI_VOID)
{
	IVE_THRESH_MODE_E enMode = IVE_THRESH_MODE_ORI_MID_MAX;
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Thresh_OriMidMax_yuv_start;
	const CVI_CHAR *resultFileName = &_binary_test_ive_data_result_sample_Thresh_OriMidMax_yuv_start;

#else
	const CVI_CHAR *resultFileName = "../data/result/sample_Thresh_OriMidMax.yuv";

#endif
	return ThreshSample(enMode, resultFileName);
}
CVI_S32 ThreshSampleMinMidOri(CVI_VOID)
{
	IVE_THRESH_MODE_E enMode = IVE_THRESH_MODE_MIN_MID_ORI;
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Thresh_MinMidOri_yuv_start;
	const CVI_CHAR *resultFileName = &_binary_test_ive_data_result_sample_Thresh_MinMidOri_yuv_start;

#else
	const CVI_CHAR *resultFileName = "../data/result/sample_Thresh_MinMidOri.yuv";

#endif
	return ThreshSample(enMode, resultFileName);
}
CVI_S32 ThreshSampleMinOriMax(CVI_VOID)
{
	IVE_THRESH_MODE_E enMode = IVE_THRESH_MODE_MIN_ORI_MAX;
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Thresh_MinOriMax_yuv_start;
	const CVI_CHAR *resultFileName = &_binary_test_ive_data_result_sample_Thresh_MinOriMax_yuv_start;

#else
	const CVI_CHAR *resultFileName = "../data/result/sample_Thresh_MinOriMax.yuv";

#endif
	return ThreshSample(enMode, resultFileName);
}
CVI_S32 ThreshSampleOriMinOri(CVI_VOID)
{
	IVE_THRESH_MODE_E enMode = IVE_THRESH_MODE_ORI_MID_ORI;
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Thresh_OriMidOri_yuv_start;
	const CVI_CHAR *resultFileName = &_binary_test_ive_data_result_sample_Thresh_OriMidOri_yuv_start;

#else
	const CVI_CHAR *resultFileName = "../data/result/sample_Thresh_OriMidOri.yuv";

#endif
	return ThreshSample(enMode, resultFileName);
}
