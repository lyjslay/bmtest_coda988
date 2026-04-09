#include "OrdStaFilter.h"
#include "sample_assist.h"
#include "sample_file.h"
#include "sample_define.h"
#include "mpi_ive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CVI_S32 OrdStaFilterSample(IVE_ORD_STAT_FILTER_MODE_E enMode, const CVI_CHAR *resultFileName)
{
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_SRC_IMAGE_S stRef;
	IVE_ORD_STAT_FILTER_CTRL_S stCtrl;
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
	memset(&stCtrl,0,sizeof(IVE_ORD_STAT_FILTER_CTRL_S));

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

	dTime = (CVI_DOUBLE)CVI_GetTickCount();
	s32Result = CVI_MPI_IVE_OrdStatFilter(&handle,&stSrc,&stDst,&stCtrl,CVI_FALSE);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	dTime = (CVI_DOUBLE)CVI_GetTickCount() - dTime;

	printf("OrdStaFilter mode %d,time = %g ms\n",enMode, dTime/((CVI_DOUBLE)CVI_GetTickFrequency()*1000.));

	s32Result = CVI_CompareIveImage(&stDst, &stRef);

FAILURE:

	CVI_FCLOSE(fp);
	CVI_FCLOSE(fp2);
	CVI_DestroyIveImage(&stSrc);
	CVI_DestroyIveImage(&stDst);
	CVI_DestroyIveImage(&stRef);

	return s32Result;
}

CVI_S32 OrdStaFilterSampleMeidan(CVI_VOID)
{
	IVE_ORD_STAT_FILTER_MODE_E enMode = IVE_ORD_STAT_FILTER_MODE_MEDIAN;
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_OrdStaFilter_Median_yuv_start;
	const CVI_CHAR *resultFileName = &_binary_test_ive_data_result_sample_OrdStaFilter_Median_yuv_start;

#else
	const CVI_CHAR *resultFileName = "../data/result/sample_OrdStaFilter_Median.yuv";

#endif
	return OrdStaFilterSample(enMode, resultFileName);
}
CVI_S32 OrdStaFilterSampleMax(CVI_VOID)
{
	IVE_ORD_STAT_FILTER_MODE_E enMode = IVE_ORD_STAT_FILTER_MODE_MAX;
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_OrdStaFilter_Max_yuv_start;
	const CVI_CHAR *resultFileName = &_binary_test_ive_data_result_sample_OrdStaFilter_Max_yuv_start;

#else
	const CVI_CHAR *resultFileName = "../data/result/sample_OrdStaFilter_Max.yuv";

#endif
	return OrdStaFilterSample(enMode, resultFileName);
}
CVI_S32 OrdStaFilterSampleMin(CVI_VOID)
{
	IVE_ORD_STAT_FILTER_MODE_E enMode = IVE_ORD_STAT_FILTER_MODE_MIN;
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_OrdStaFilter_Min_yuv_start;
	const CVI_CHAR *resultFileName = &_binary_test_ive_data_result_sample_OrdStaFilter_Min_yuv_start;

#else
	const CVI_CHAR *resultFileName = "../data/result/sample_OrdStaFilter_Min.yuv";

#endif
	return OrdStaFilterSample(enMode, resultFileName);
}
