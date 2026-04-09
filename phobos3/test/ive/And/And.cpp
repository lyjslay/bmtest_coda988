#include "And.h"
#include "sample_assist.h"
#include "sample_file.h"
#include "sample_define.h"
#include "mpi_ive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CVI_S32 AndSample(CVI_VOID)
{
	IVE_SRC_IMAGE_S stSrc1;
	IVE_SRC_IMAGE_S stSrc2;
	IVE_SRC_IMAGE_S stRef;
	IVE_DST_IMAGE_S stDst;
	CVI_S32 s32Result;
	CVI_U32 u32Width;
	CVI_U32 u32Height;
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
extern CVI_CHAR _binary_test_ive_data_result_sample_And_yuv_start;
	const CVI_CHAR *fileName3 = &_binary_test_ive_data_result_sample_And_yuv_start;

#else
	const CVI_CHAR *fileName1 = "../data/y/00_352x288_y.yuv";
	const CVI_CHAR *fileName2 = "../data/y/01_352x288_y.yuv";
	const CVI_CHAR *fileName3 = "../data/result/sample_And.yuv";

#endif
	memset(&stSrc1,0,sizeof(IVE_SRC_IMAGE_S));
	memset(&stSrc2,0,sizeof(IVE_SRC_IMAGE_S));
	memset(&stDst,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stRef,0,sizeof(IVE_DST_IMAGE_S));

	fp1 = NULL;
	fp2 = NULL;
	fp3 = NULL;
	u32Width = CVI_IMAGE_WIDTH_352;
	u32Height = CVI_IMAGE_HEIGHT_288;

	s32Result = CVI_CreateIveImage(&stSrc1,IVE_IMAGE_TYPE_U8C1,u32Width,u32Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stSrc2,IVE_IMAGE_TYPE_U8C1,u32Width,u32Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stDst,IVE_IMAGE_TYPE_U8C1,u32Width,u32Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stRef,IVE_IMAGE_TYPE_U8C1,u32Width,u32Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
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

	dTime = (CVI_DOUBLE)CVI_GetTickCount();
	s32Result = CVI_MPI_IVE_And(&handle,&stSrc1,&stSrc2,&stDst,CVI_FALSE);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	dTime = (CVI_DOUBLE)CVI_GetTickCount() - dTime;

	printf("And,time = %g ms\n",dTime/((CVI_DOUBLE)CVI_GetTickFrequency()*1000.));

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
