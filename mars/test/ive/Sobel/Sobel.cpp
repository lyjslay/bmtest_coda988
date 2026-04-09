#include "Sobel.h"
#include "sample_assist.h"
#include "sample_file.h"
#include "sample_define.h"
#include "mpi_ive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CVI_S32 SobelSample(IVE_SOBEL_OUT_CTRL_E enOutCtrl,CVI_S8 *ps8Mask,
		const CVI_CHAR *fileName2,const CVI_CHAR *fileName3)
{
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDstH;
	IVE_DST_IMAGE_S stDstV;
	IVE_DST_IMAGE_S stRefH;
	IVE_DST_IMAGE_S stRefV;
	IVE_SOBEL_CTRL_S stCtrl;
	CVI_S32 s32Result;
	CVI_U16 u16Width;
	CVI_U16 u16Height;
	FILE *fp;
	FILE *fp2;
	FILE *fp3;
	IVE_HANDLE handle;
	CVI_DOUBLE dTime;
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_y_00_352x288_y_yuv_start;
	const CVI_CHAR *fileName = &_binary_test_ive_data_y_00_352x288_y_yuv_start;

#else
	const CVI_CHAR *fileName = "../data/y/00_352x288_y.yuv";

#endif
	memset(&stSrc,0,sizeof(IVE_SRC_IMAGE_S));
	memset(&stDstH,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stDstV,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stRefH,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stRefV,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stCtrl,0,sizeof(IVE_MAG_AND_ANG_CTRL_S));

	fp = NULL;
	fp2 = NULL;
	fp3 = NULL;
	u16Width = CVI_IMAGE_WIDTH_352;
	u16Height = CVI_IMAGE_HEIGHT_288;

	s32Result = CVI_CreateIveImage(&stSrc,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	if (IVE_SOBEL_OUT_CTRL_VER != enOutCtrl )
	{
		s32Result = CVI_CreateIveImage(&stDstH,IVE_IMAGE_TYPE_S16C1,u16Width,u16Height);
		CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
		s32Result = CVI_CreateIveImage(&stRefH,IVE_IMAGE_TYPE_S16C1,u16Width,u16Height);
		CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	}
	if (IVE_SOBEL_OUT_CTRL_HOR != enOutCtrl )
	{
		s32Result = CVI_CreateIveImage(&stDstV,IVE_IMAGE_TYPE_S16C1,u16Width,u16Height);
		CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
		s32Result = CVI_CreateIveImage(&stRefV,IVE_IMAGE_TYPE_S16C1,u16Width,u16Height);
		CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	}

	s32Result = fopen_s(&fp,fileName,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
	s32Result = CVI_ReadFile(&stSrc,fp);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	if (IVE_SOBEL_OUT_CTRL_VER != enOutCtrl )
	{
		s32Result = fopen_s(&fp2,fileName2,"rb");
		CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);

		s32Result = CVI_ReadFile(&stRefH,fp2);
		CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	}

	if (IVE_SOBEL_OUT_CTRL_HOR != enOutCtrl )
	{
		s32Result = fopen_s(&fp3,fileName3,"rb");
		CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);

		s32Result = CVI_ReadFile(&stRefV,fp3);
		CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	}

	stCtrl.enOutCtrl = enOutCtrl;
	memcpy(stCtrl.as8Mask,ps8Mask,sizeof(CVI_S8) * 25);

	dTime = (CVI_DOUBLE)CVI_GetTickCount();
	s32Result = CVI_MPI_IVE_Sobel(&handle,&stSrc,
		IVE_SOBEL_OUT_CTRL_VER != enOutCtrl ? &stDstH : CVI_NULL,
		IVE_SOBEL_OUT_CTRL_HOR != enOutCtrl ? &stDstV : CVI_NULL,
		&stCtrl,CVI_FALSE);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	dTime = (CVI_DOUBLE)CVI_GetTickCount() - dTime;

	printf("Sobel outCtrl %d,time = %g ms\n",enOutCtrl, dTime/((CVI_DOUBLE)CVI_GetTickFrequency()*1000.));

	if (IVE_SOBEL_OUT_CTRL_VER != enOutCtrl )
	{
		s32Result = CVI_CompareIveImage(&stDstH,&stRefH);
		CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	}
	if (IVE_SOBEL_OUT_CTRL_HOR != enOutCtrl )
	{
		s32Result = CVI_CompareIveImage(&stDstV,&stRefV);
		CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	}

FAILURE:

	CVI_FCLOSE(fp);
	CVI_DestroyIveImage(&stSrc);
	if (IVE_SOBEL_OUT_CTRL_VER != enOutCtrl )
	{
		CVI_FCLOSE(fp2);
		CVI_DestroyIveImage(&stDstH);
		CVI_DestroyIveImage(&stRefH);
	}
	if (IVE_SOBEL_OUT_CTRL_HOR != enOutCtrl )
	{
		CVI_FCLOSE(fp3);
		CVI_DestroyIveImage(&stDstV);
		CVI_DestroyIveImage(&stRefV);
	}

	return s32Result;
}


CVI_S32 SobelSampleBoth3x3(CVI_VOID)
{
	IVE_SOBEL_OUT_CTRL_E enOutCtrl = IVE_SOBEL_OUT_CTRL_BOTH;
	CVI_S8 mask[25]={0,0,0,0,0,
					0,-1,0,1,0,
					0,-2,0,2,0,
					0,-1,0,1,0,
					0,0,0,0,0};
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Sobel_Both3x3_H_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_Sobel_Both3x3_H_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_Sobel_Both3x3_V_yuv_start;
	const CVI_CHAR *fileName3 = &_binary_test_ive_data_result_sample_Sobel_Both3x3_V_yuv_start;

#else
	const CVI_CHAR *fileName2 = "../data/result/sample_Sobel_Both3x3_H.yuv";
	const CVI_CHAR *fileName3 = "../data/result/sample_Sobel_Both3x3_V.yuv";

#endif
	return SobelSample(enOutCtrl,mask,fileName2,fileName3);
}
CVI_S32 SobelSampleBoth5x5(CVI_VOID)
{
	IVE_SOBEL_OUT_CTRL_E enOutCtrl = IVE_SOBEL_OUT_CTRL_BOTH;
	CVI_S8 mask[25]={-1,-2,0,2,1,
					-4,-8,0,8,4,
					-6,-12,0,12,6,
					-4,-8,0,8,4,
					-1,-2,0,2,1};
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Sobel_Both5x5_H_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_Sobel_Both5x5_H_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_Sobel_Both5x5_V_yuv_start;
	const CVI_CHAR *fileName3 = &_binary_test_ive_data_result_sample_Sobel_Both5x5_V_yuv_start;

#else
	const CVI_CHAR *fileName2 = "../data/result/sample_Sobel_Both5x5_H.yuv";
	const CVI_CHAR *fileName3 = "../data/result/sample_Sobel_Both5x5_V.yuv";

#endif
	return SobelSample(enOutCtrl,mask,fileName2,fileName3);
}
CVI_S32 SobelSampleHor3x3(CVI_VOID)
{
	IVE_SOBEL_OUT_CTRL_E enOutCtrl = IVE_SOBEL_OUT_CTRL_HOR;
	CVI_S8 mask[25]={0,0,0,0,0,
					0,-1,0,1,0,
					0,-2,0,2,0,
					0,-1,0,1,0,
					0,0,0,0,0};
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Sobel_Hor3x3_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_Sobel_Hor3x3_yuv_start;

#else
	const CVI_CHAR *fileName2 = "../data/result/sample_Sobel_Hor3x3.yuv";

#endif
	return SobelSample(enOutCtrl,mask,fileName2,NULL);
}
CVI_S32 SobelSampleHor5x5(CVI_VOID)
{
	IVE_SOBEL_OUT_CTRL_E enOutCtrl = IVE_SOBEL_OUT_CTRL_HOR;
	CVI_S8 mask[25]={-1,-2,0,2,1,
					-4,-8,0,8,4,
					-6,-12,0,12,6,
					-4,-8,0,8,4,
					-1,-2,0,2,1};
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Sobel_Hor5x5_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_Sobel_Hor5x5_yuv_start;

#else
	const CVI_CHAR *fileName2 = "../data/result/sample_Sobel_Hor5x5.yuv";

#endif
	return SobelSample(enOutCtrl,mask,fileName2,NULL);
}
CVI_S32 SobelSampleVer3x3(CVI_VOID)
{
	IVE_SOBEL_OUT_CTRL_E enOutCtrl = IVE_SOBEL_OUT_CTRL_VER;
	CVI_S8 mask[25]={0,0,0,0,0,
					0,-1,0,1,0,
					0,-2,0,2,0,
					0,-1,0,1,0,
					0,0,0,0,0};
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Sobel_Ver3x3_yuv_start;
	const CVI_CHAR *fileName3 = &_binary_test_ive_data_result_sample_Sobel_Ver3x3_yuv_start;

#else
	const CVI_CHAR *fileName3 = "../data/result/sample_Sobel_Ver3x3.yuv";

#endif
	return SobelSample(enOutCtrl,mask,NULL,fileName3);
}
CVI_S32 SobelSampleVer5x5(CVI_VOID)
{
	IVE_SOBEL_OUT_CTRL_E enOutCtrl = IVE_SOBEL_OUT_CTRL_VER;
	CVI_S8 mask[25]={-1,-2,0,2,1,
					-4,-8,0,8,4,
					-6,-12,0,12,6,
					-4,-8,0,8,4,
					-1,-2,0,2,1};
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Sobel_Ver5x5_yuv_start;
	const CVI_CHAR *fileName3 = &_binary_test_ive_data_result_sample_Sobel_Ver5x5_yuv_start;

#else
	const CVI_CHAR *fileName3 = "../data/result/sample_Sobel_Ver5x5.yuv";

#endif
	return SobelSample(enOutCtrl,mask,NULL,fileName3);
}
