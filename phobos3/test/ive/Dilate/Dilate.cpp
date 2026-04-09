#include "Dilate.h"
#include "sample_assist.h"
#include "sample_file.h"
#include "sample_define.h"
#include "mpi_ive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CVI_S32 DilateSample(CVI_U8 *pu8Mask,CVI_U8 u8LowThr,const CVI_CHAR *fileName2,const CVI_CHAR *fileName3)
{
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst1;
	IVE_DST_IMAGE_S stDst2;
	IVE_DST_IMAGE_S stRef1;
	IVE_DST_IMAGE_S stRef2;
	IVE_DILATE_CTRL_S stCtrlDilate;
	IVE_THRESH_CTRL_S stCtrlThresh;

	CVI_S32 s32Result ;
	CVI_U16 u16Width;
	CVI_U16 u16Height;
	FILE *fp;
	FILE *fp2;
	FILE *fp3;
	IVE_HANDLE handle;
	CVI_DOUBLE dTime;
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_y_bin_352x288_y_yuv_start;
	const CVI_CHAR *fileName = &_binary_test_ive_data_y_bin_352x288_y_yuv_start;

#else
	const CVI_CHAR *fileName = "../data/y/bin_352x288_y.yuv";

#endif
	memset(&stSrc,0,sizeof(IVE_SRC_IMAGE_S));
	memset(&stDst1,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stDst2,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stRef1,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stRef2,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stCtrlDilate,0,sizeof(IVE_DILATE_CTRL_S));
	memset(&stCtrlThresh,0,sizeof(IVE_THRESH_CTRL_S));

	fp = NULL;
	fp2 = NULL;
	fp3 = NULL;
	u16Width = CVI_IMAGE_WIDTH_352;
	u16Height = CVI_IMAGE_HEIGHT_288;

	s32Result = CVI_CreateIveImage(&stSrc,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stDst1,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stDst2,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stRef1,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stRef2,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);


	s32Result = fopen_s(&fp,fileName,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
	s32Result = fopen_s(&fp2,fileName2,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
	s32Result = fopen_s(&fp3,fileName3,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);

	s32Result = CVI_ReadFile(&stSrc,fp);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_ReadFile(&stRef1,fp2);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_ReadFile(&stRef2,fp3);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	stCtrlThresh.enMode = IVE_THRESH_MODE_BINARY;
	stCtrlThresh.u8MaxVal = 255;
	stCtrlThresh.u8MinVal = 0;
	// stCtrlThresh.u8LowThr = CVI_GenRand(CVI_MAX_U8,0);
	stCtrlThresh.u8LowThr = u8LowThr;
	printf("u8LowThr = %d\n", stCtrlThresh.u8LowThr);

	s32Result = CVI_MPI_IVE_Thresh(&handle,&stSrc,&stDst1,&stCtrlThresh,CVI_FALSE);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	s32Result = CVI_CompareIveImage(&stDst1,&stRef1);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	memcpy(stCtrlDilate.au8Mask,pu8Mask,sizeof(CVI_U8) * 25);

	dTime = (CVI_DOUBLE)CVI_GetTickCount();
	s32Result = CVI_MPI_IVE_Dilate(&handle,&stDst1,&stDst2,&stCtrlDilate,CVI_FALSE);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	dTime = (CVI_DOUBLE)CVI_GetTickCount() - dTime;

	printf("Dilate time = %g ms\n", dTime/((CVI_DOUBLE)CVI_GetTickFrequency()*1000.));

	s32Result = CVI_CompareIveImage(&stDst2,&stRef2);

FAILURE:

	CVI_FCLOSE(fp);
	CVI_FCLOSE(fp2);
	CVI_FCLOSE(fp3);
	CVI_DestroyIveImage(&stSrc);
	CVI_DestroyIveImage(&stDst1);
	CVI_DestroyIveImage(&stDst2);
	CVI_DestroyIveImage(&stRef1);
	CVI_DestroyIveImage(&stRef2);

	return s32Result;
}


CVI_S32 DilateSample3x3(CVI_VOID)
{
	CVI_U8 mask[25]={0,	0,	0,	0,	0,
					0,	0,	255,0,	0,
					0,  255,255,255,  0,
					0,  0,  255, 0,  0,
					0,  0,  0,   0,  0};
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Dilate_3x3_Thresh_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_Dilate_3x3_Thresh_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_Dilate_3x3_bin_start;
	const CVI_CHAR *fileName3 = &_binary_test_ive_data_result_sample_Dilate_3x3_bin_start;

#else
	const CVI_CHAR *fileName2 = "../data/result/sample_Dilate_3x3_Thresh.yuv";
	const CVI_CHAR *fileName3 = "../data/result/sample_Dilate_3x3.bin";

#endif
	return DilateSample(mask,41,fileName2,fileName3);
}
CVI_S32 DilateSample5x5(CVI_VOID)
{
	CVI_U8 mask[25]={0,	0,	255,	0,	0,
					0,	0,	255,	0,	0,
					255, 255,255,255,  255,
					0,  0,  255,	0,  0,
					0,  0,  255,	0,  0};
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Dilate_5x5_Thresh_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_Dilate_5x5_Thresh_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_Dilate_5x5_bin_start;
	const CVI_CHAR *fileName3 = &_binary_test_ive_data_result_sample_Dilate_5x5_bin_start;

#else
	const CVI_CHAR *fileName2 = "../data/result/sample_Dilate_5x5_Thresh.yuv";
	const CVI_CHAR *fileName3 = "../data/result/sample_Dilate_5x5.bin";

#endif
	return DilateSample(mask,35,fileName2,fileName3);
}
