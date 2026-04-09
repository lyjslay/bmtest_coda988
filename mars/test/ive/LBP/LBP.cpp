#include "LBP.h"
#include "sample_assist.h"
#include "sample_file.h"
#include "sample_define.h"
#include "mpi_ive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CVI_S32 LBPSample(IVE_LBP_CMP_MODE_E enMode,const CVI_CHAR *fileName2)
{
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_DST_IMAGE_S stRef;
	IVE_LBP_CTRL_S stCtrl;
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
	memset(&stCtrl,0,sizeof(IVE_LBP_CTRL_S));

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

	s32Result = fopen_s(&fp,fileName,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
	s32Result = fopen_s(&fp2,fileName2,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);

	s32Result = CVI_ReadFile(&stSrc,fp);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_ReadFile(&stRef,fp2);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	stCtrl.enMode = enMode;
	// stCtrl.un8BitThr.s8Val = (IVE_LBP_CMP_MODE_ABS == enMode ? CVI_GenRand(CVI_MAX_S8,0) : CVI_GenRand(CVI_MAX_S8,CVI_MIN_S8));
	stCtrl.un8BitThr.s8Val = (IVE_LBP_CMP_MODE_ABS == enMode ? 35 : 41);
	printf("enMOde = %d, un8BitThr = %d\n", enMode, stCtrl.un8BitThr.s8Val);

	dTime = (CVI_DOUBLE)CVI_GetTickCount();
	s32Result = CVI_MPI_IVE_LBP(&handle,&stSrc,&stDst,&stCtrl,CVI_FALSE);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	dTime = (CVI_DOUBLE)CVI_GetTickCount() - dTime;

	printf("LBP mode %d,time = %g ms\n",enMode,dTime/((CVI_DOUBLE)CVI_GetTickFrequency()*1000.));

	s32Result = CVI_CompareIveImage(&stDst,&stRef);

FAILURE:

	CVI_FCLOSE(fp);
	CVI_FCLOSE(fp2);
	CVI_DestroyIveImage(&stSrc);
	CVI_DestroyIveImage(&stDst);
	CVI_DestroyIveImage(&stRef);

	return s32Result;
}

CVI_S32 LBPSampleNormal(CVI_VOID)
{
	IVE_LBP_CMP_MODE_E enMode = IVE_LBP_CMP_MODE_NORMAL;
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_LBP_Normal_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_LBP_Normal_yuv_start;

#else
	const CVI_CHAR *fileName2 = "../data/result/sample_LBP_Normal.yuv";

#endif
	return LBPSample(enMode,fileName2);
}
CVI_S32 LBPSampleABS(CVI_VOID)
{
	IVE_LBP_CMP_MODE_E enMode = IVE_LBP_CMP_MODE_ABS;
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_LBP_Abs_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_LBP_Abs_yuv_start;

#else
	const CVI_CHAR *fileName2 = "../data/result/sample_LBP_Abs.yuv";

#endif
	return LBPSample(enMode,fileName2);
}
