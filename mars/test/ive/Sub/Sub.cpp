#include "Sub.h"
#include "sample_assist.h"
#include "sample_file.h"
#include "sample_define.h"
#include "mpi_ive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CVI_S32 SubSample(IVE_SUB_MODE_E enMode)
{
	IVE_SRC_IMAGE_S stSrc1;
	IVE_SRC_IMAGE_S stSrc2;
	IVE_SRC_IMAGE_S stRef;
	IVE_DST_IMAGE_S stDst;
	IVE_SUB_CTRL_S stCtrl;
	CVI_S32 s32Result;
	CVI_U16 u16Width;
	CVI_U16 u16Height;
	FILE *fp1;
	FILE *fp2;
	FILE *fp3;
	IVE_HANDLE handle;
	CVI_DOUBLE dTime;
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_y_00_352x288_y_yuv_start;
	const CVI_CHAR *fileName1 = &_binary_test_ive_data_y_00_352x288_y_yuv_start;
extern CVI_CHAR _binary_test_ive_data_y_01_352x288_y_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_y_01_352x288_y_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_Sub_Shift_yuv_start;
	const CVI_CHAR *fileName3_shift = &_binary_test_ive_data_result_sample_Sub_Shift_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_Sub_Abs_yuv_start;
	const CVI_CHAR *fileName3_abs = &_binary_test_ive_data_result_sample_Sub_Abs_yuv_start;

#else
	const CVI_CHAR *fileName1 = "../data/y/00_352x288_y.yuv";
	const CVI_CHAR *fileName2 = "../data/y/01_352x288_y.yuv";
	const CVI_CHAR *fileName3_shift = "../data/result/sample_Sub_Shift.yuv";
	const CVI_CHAR *fileName3_abs = "../data/result/sample_Sub_Abs.yuv";

#endif
	const CVI_CHAR *fileName3 = NULL;

	memset(&stSrc1,0,sizeof(IVE_SRC_IMAGE_S));
	memset(&stSrc2,0,sizeof(IVE_SRC_IMAGE_S));
	memset(&stDst,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stRef,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stCtrl,0,sizeof(IVE_SUB_CTRL_S));

	fp1 = NULL;
	fp2 = NULL;
	fp3 = NULL;
	u16Width = CVI_IMAGE_WIDTH_352;
	u16Height = CVI_IMAGE_HEIGHT_288;

	s32Result = CVI_CreateIveImage(&stSrc1,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stSrc2,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	if(IVE_SUB_MODE_SHIFT == enMode)
	{
		s32Result = CVI_CreateIveImage(&stDst,IVE_IMAGE_TYPE_S8C1,u16Width,u16Height);
		CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
		s32Result = CVI_CreateIveImage(&stRef,IVE_IMAGE_TYPE_S8C1,u16Width,u16Height);
		CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
		fileName3 = fileName3_shift;
	}
	else
	{
		s32Result = CVI_CreateIveImage(&stDst,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
		CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
		s32Result = CVI_CreateIveImage(&stRef,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
		CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
		fileName3 = fileName3_abs;
	}
	CVI_ResetIveImage(&stDst,0);

	s32Result = fopen_s(&fp1,fileName1,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
	s32Result = fopen_s(&fp2,fileName2,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
	s32Result = fopen_s(&fp3,fileName3,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);

	s32Result = CVI_ReadFile(&stSrc1,fp1);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_ReadFile(&stSrc2,fp2);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_ReadFile(&stRef,fp3);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	stCtrl.enMode = enMode;

	dTime = (CVI_DOUBLE)CVI_GetTickCount();
	s32Result = CVI_MPI_IVE_Sub(&handle,&stSrc1,&stSrc2,&stDst,&stCtrl,CVI_FALSE);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	dTime = (CVI_DOUBLE)CVI_GetTickCount() - dTime;

	printf("Sub,mode %d,time = %g ms\n",enMode,dTime/((CVI_DOUBLE)CVI_GetTickFrequency()*1000.));

	s32Result = CVI_CompareIveImage(&stDst, &stRef);

FAILURE:

	CVI_FCLOSE(fp1);
	CVI_FCLOSE(fp2);
	CVI_FCLOSE(fp3);
	CVI_DestroyIveImage(&stSrc1);
	CVI_DestroyIveImage(&stSrc2);
	CVI_DestroyIveImage(&stDst);
	CVI_DestroyIveImage(&stRef);

	return s32Result;
}

CVI_S32 SubSampleABS(CVI_VOID)
{
	IVE_SUB_MODE_E enMode = IVE_SUB_MODE_ABS;

	return SubSample(enMode);
}
CVI_S32 SubSampleSHIFT(CVI_VOID)
{
	IVE_SUB_MODE_E enMode = IVE_SUB_MODE_SHIFT;

	return SubSample(enMode);
}
