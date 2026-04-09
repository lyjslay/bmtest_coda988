#include "Integ.h"
#include "sample_assist.h"
#include "sample_file.h"
#include "sample_define.h"
#include "mpi_ive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CVI_S32 Integ(IVE_INTEG_OUT_CTRL_E enOutCtrl,const CVI_CHAR *fileName2)
{
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_DST_IMAGE_S stRef;
	IVE_INTEG_CTRL_S stCtrl;
	CVI_S32 s32Result;
	CVI_U16 u16Width;
	CVI_U16 u16Height;
	FILE *fp;
	FILE *fp2;
	IVE_HANDLE handle;
	CVI_DOUBLE dTime;
	CVI_CHAR info[CVI_INFO_LEN];
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_y_00_352x288_y_yuv_start;
	const CVI_CHAR *fileName = &_binary_test_ive_data_y_00_352x288_y_yuv_start;

#else
	const CVI_CHAR *fileName = "../data/y/00_352x288_y.yuv";

#endif
	memset(&stSrc,0,sizeof(IVE_SRC_IMAGE_S));
	memset(&stDst,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stCtrl,0,sizeof(IVE_INTEG_CTRL_S));

	fp = NULL;
	fp2 = NULL;
	u16Width = CVI_IMAGE_WIDTH_352;
	u16Height = CVI_IMAGE_HEIGHT_288;

	s32Result = CVI_CreateIveImage(&stSrc,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	switch (enOutCtrl)
	{
	case IVE_INTEG_OUT_CTRL_COMBINE:
		strcpy_s(info,CVI_INFO_LEN,"Histogram combine time =");
		s32Result = CVI_CreateIveImage(&stDst,IVE_IMAGE_TYPE_U64C1,u16Width,u16Height);
		CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
		s32Result = CVI_CreateIveImage(&stRef,IVE_IMAGE_TYPE_U64C1,u16Width,u16Height);
		CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
		break;
	case IVE_INTEG_OUT_CTRL_SUM:
		strcpy_s(info,CVI_INFO_LEN,"Histogram sum time =");
		s32Result = CVI_CreateIveImage(&stDst,IVE_IMAGE_TYPE_U32C1,u16Width,u16Height);
		CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
		s32Result = CVI_CreateIveImage(&stRef,IVE_IMAGE_TYPE_U32C1,u16Width,u16Height);
		CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
		break;
	case IVE_INTEG_OUT_CTRL_SQSUM:
		strcpy_s(info,CVI_INFO_LEN,"Histogram sqsum time =");
		s32Result = CVI_CreateIveImage(&stDst,IVE_IMAGE_TYPE_U64C1,u16Width,u16Height);
		CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
		s32Result = CVI_CreateIveImage(&stRef,IVE_IMAGE_TYPE_U64C1,u16Width,u16Height);
		CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
		break;
	default:
		s32Result = CVI_FAILURE;
		break;
	}

	stCtrl.enOutCtrl = enOutCtrl;

	s32Result = fopen_s(&fp,fileName,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
	s32Result = fopen_s(&fp2,fileName2,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);

	s32Result = CVI_ReadFile(&stSrc,fp);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_ReadFile(&stRef,fp2);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	dTime = (CVI_DOUBLE)CVI_GetTickCount();
	s32Result = CVI_MPI_IVE_Integ(&handle,&stSrc,&stDst,&stCtrl,CVI_FALSE);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	dTime = (CVI_DOUBLE)CVI_GetTickCount() - dTime;

	printf("%s %g ms\n",info, dTime/((CVI_DOUBLE)CVI_GetTickFrequency()*1000.));

	s32Result = CVI_CompareIveImage(&stDst,&stRef);

FAILURE:

	CVI_FCLOSE(fp);
	CVI_FCLOSE(fp2);
	CVI_DestroyIveImage(&stSrc);
	CVI_DestroyIveImage(&stDst);
	CVI_DestroyIveImage(&stRef);

	return s32Result;
}

CVI_S32 IntegSampleCombine(CVI_VOID)
{
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Integ_Combine_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_Integ_Combine_yuv_start;

#else
	const CVI_CHAR *fileName2 = "../data/result/sample_Integ_Combine.yuv";

#endif
	return Integ(IVE_INTEG_OUT_CTRL_COMBINE,fileName2);
}
CVI_S32 IntegSampleSum(CVI_VOID)
{
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Integ_Sum_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_Integ_Sum_yuv_start;

#else
	const CVI_CHAR *fileName2 = "../data/result/sample_Integ_Sum.yuv";

#endif
	return Integ(IVE_INTEG_OUT_CTRL_SUM,fileName2);
}
CVI_S32 IntegSampleSqsum(CVI_VOID)
{
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Integ_Sqsum_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_Integ_Sqsum_yuv_start;

#else
	const CVI_CHAR *fileName2 = "../data/result/sample_Integ_Sqsum.yuv";

#endif
	return Integ(IVE_INTEG_OUT_CTRL_SQSUM,fileName2);
}
