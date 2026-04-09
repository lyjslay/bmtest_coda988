#include "Erode.h"
#include "sample_assist.h"
#include "sample_file.h"
#include "sample_define.h"
#include "mpi_ive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CVI_S32 ErodeSample(CVI_U8 *pu8Mask,CVI_U8 u8LowThr,const CVI_CHAR *fileName2,const CVI_CHAR *fileName3, const CVI_CHAR *fileName4)
{
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst1;
	IVE_DST_IMAGE_S stDst2;
	IVE_DST_IMAGE_S stRef1;
	IVE_DST_IMAGE_S stRef2;
	IVE_DST_IMAGE_S stRef3;
	IVE_ERODE_CTRL_S stCtrlErode;
	IVE_THRESH_CTRL_S stCtrlThresh;

	CVI_S32 s32Result ;
	CVI_U32 u32Width;
	CVI_U32 u32Height;
	FILE *fp;
	FILE *fp2;
	FILE *fp3;
	FILE *fp4;
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
	memset(&stRef3,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stCtrlErode,0,sizeof(IVE_ERODE_CTRL_S));
	memset(&stCtrlThresh,0,sizeof(IVE_THRESH_CTRL_S));

	fp = NULL;
	fp2 = NULL;
	fp3 = NULL;
	u32Width = CVI_IMAGE_WIDTH_352;
	u32Height = CVI_IMAGE_HEIGHT_288;

	s32Result = CVI_CreateIveImage(&stSrc,IVE_IMAGE_TYPE_U8C1,u32Width,u32Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stDst1,IVE_IMAGE_TYPE_U8C1,u32Width,u32Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stDst2,IVE_IMAGE_TYPE_U8C1,u32Width,u32Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stRef1,IVE_IMAGE_TYPE_U8C1,u32Width,u32Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stRef2,IVE_IMAGE_TYPE_U8C1,u32Width,u32Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stRef3,IVE_IMAGE_TYPE_U8C1,u32Width,u32Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);


	s32Result = fopen_s(&fp,fileName,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
	s32Result = fopen_s(&fp2,fileName2,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
	s32Result = fopen_s(&fp3,fileName3,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
	s32Result = fopen_s(&fp4,fileName4,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);

	s32Result = CVI_ReadFile(&stSrc,fp);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_ReadFile(&stRef1,fp2);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_ReadFile(&stRef2,fp3);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_ReadFile(&stRef3,fp4);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

        {
          printf("directly test erode\n");
          memcpy(stCtrlErode.au8Mask,pu8Mask,sizeof(CVI_U8) * 25);
          s32Result = CVI_MPI_IVE_Erode(&handle,&stSrc,&stDst1,&
              stCtrlErode,CVI_TRUE);
          CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
          s32Result = CVI_CompareIveImage(&stDst1,&stRef3);
          CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
        }

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

	memcpy(stCtrlErode.au8Mask,pu8Mask,sizeof(CVI_U8) * 25);

	dTime = (CVI_DOUBLE)CVI_GetTickCount();
	s32Result = CVI_MPI_IVE_Erode(&handle,&stDst1,&stDst2,&stCtrlErode,CVI_FALSE);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	dTime = (CVI_DOUBLE)CVI_GetTickCount() - dTime;

	printf("Erode time = %g ms\n", dTime/((CVI_DOUBLE)CVI_GetTickFrequency()*1000.));

	s32Result = CVI_CompareIveImage(&stDst2,&stRef2);

FAILURE:

	CVI_FCLOSE(fp);
	CVI_FCLOSE(fp2);
	CVI_FCLOSE(fp3);
	CVI_FCLOSE(fp4);
	CVI_DestroyIveImage(&stSrc);
	CVI_DestroyIveImage(&stDst1);
	CVI_DestroyIveImage(&stDst2);
	CVI_DestroyIveImage(&stRef1);
	CVI_DestroyIveImage(&stRef2);
	CVI_DestroyIveImage(&stRef3);

	return s32Result;
}


CVI_S32 ErodeSample3x3(CVI_VOID)
{
	CVI_U8 mask[25]={0,	0,	0,	0,	0,
					0,	0,	255,0,	0,
					0,  255,255,255,  0,
					0,  0,  255, 0,  0,
					0,  0,  0,   0,  0};
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Erode_3x3_Thresh_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_Erode_3x3_Thresh_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_Erode_3x3_bin_start;
	const CVI_CHAR *fileName3 = &_binary_test_ive_data_result_sample_Erode_3x3_bin_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_Erode_3x3_bin_only_erode_start;
	const CVI_CHAR *fileName4 = &_binary_test_ive_data_result_sample_Erode_3x3_bin_only_erode_start;

#else
	const CVI_CHAR *fileName2 = "../data/result/sample_Erode_3x3_Thresh.yuv";
	const CVI_CHAR *fileName3 = "../data/result/sample_Erode_3x3.bin";
	const CVI_CHAR *fileName4 = "../data/result/sample_Erode_3x3.bin.only_erode";

#endif
	return ErodeSample(mask,41,fileName2,fileName3, fileName4);
}
CVI_S32 ErodeSample5x5(CVI_VOID)
{
	CVI_U8 mask[25]={0,	0,	255,	0,	0,
					0,	0,	255,	0,	0,
					255, 255,255,255,  255,
					0,  0,  255,	0,  0,
					0,  0,  255,	0,  0};
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Erode_5x5_Thresh_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_Erode_5x5_Thresh_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_Erode_5x5_bin_start;
	const CVI_CHAR *fileName3 = &_binary_test_ive_data_result_sample_Erode_5x5_bin_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_Erode_5x5_bin_only_erode_start;
	const CVI_CHAR *fileName4 = &_binary_test_ive_data_result_sample_Erode_5x5_bin_only_erode_start;

#else
	const CVI_CHAR *fileName2 = "../data/result/sample_Erode_5x5_Thresh.yuv";
	const CVI_CHAR *fileName3 = "../data/result/sample_Erode_5x5.bin";
	const CVI_CHAR *fileName4 = "../data/result/sample_Erode_5x5.bin.only_erode";

#endif
	return ErodeSample(mask,35,fileName2,fileName3, fileName4);
}
