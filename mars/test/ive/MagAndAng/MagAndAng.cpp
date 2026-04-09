#include "MagAndAng.h"
#include "sample_assist.h"
#include "sample_file.h"
#include "sample_define.h"
#include "mpi_ive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CVI_S32 MagAndAngSample(IVE_MAG_AND_ANG_OUT_CTRL_E enOutCtrl,CVI_S8 *ps8Mask,CVI_BOOL bUseThresh,CVI_U16 u16Thr,
		const CVI_CHAR *fileName2,const CVI_CHAR *fileName3)
{
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDstMag;
	IVE_DST_IMAGE_S stDstAng;
	IVE_DST_IMAGE_S stRefMag;
	IVE_DST_IMAGE_S stRefAng;
	IVE_MAG_AND_ANG_CTRL_S stCtrl;
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
	memset(&stDstMag,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stDstAng,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stRefMag,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stRefAng,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stCtrl,0,sizeof(IVE_MAG_AND_ANG_CTRL_S));

	fp = NULL;
	fp2 = NULL;
	fp3 = NULL;
	u16Width = CVI_IMAGE_WIDTH_352;
	u16Height = CVI_IMAGE_HEIGHT_288;

	s32Result = CVI_CreateIveImage(&stSrc,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stDstMag,IVE_IMAGE_TYPE_U16C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stRefMag,IVE_IMAGE_TYPE_U16C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	if(IVE_MAG_AND_ANG_OUT_CTRL_MAG_AND_ANG == enOutCtrl)
	{
		s32Result = CVI_CreateIveImage(&stDstAng,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
		CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
		s32Result = CVI_CreateIveImage(&stRefAng,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
		CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	}

	s32Result = fopen_s(&fp,fileName,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
	s32Result = fopen_s(&fp2,fileName2,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
	if(IVE_MAG_AND_ANG_OUT_CTRL_MAG_AND_ANG == enOutCtrl)
	{
		s32Result = fopen_s(&fp3,fileName3,"rb");
		CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
	}

	s32Result = CVI_ReadFile(&stSrc,fp);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_ReadFile(&stRefMag,fp2);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	if(IVE_MAG_AND_ANG_OUT_CTRL_MAG_AND_ANG == enOutCtrl)
	{
		s32Result = CVI_ReadFile(&stRefAng,fp3);
		CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	}

	//stCtrl.u16Thr = CVI_TRUE == bUseThresh ? CVI_GenRand(CVI_MAX_U16,CVI_MIN_U16 + 1) : 0;
	stCtrl.u16Thr = CVI_TRUE == bUseThresh ? u16Thr : 0;
	printf("u16Thr = %d\n", stCtrl.u16Thr);
	stCtrl.enOutCtrl = enOutCtrl;
	memcpy(stCtrl.as8Mask,ps8Mask,sizeof(CVI_S8) * 25);

	dTime = (CVI_DOUBLE)CVI_GetTickCount();
	s32Result = CVI_MPI_IVE_MagAndAng(&handle,&stSrc,
			&stDstMag,
			IVE_MAG_AND_ANG_OUT_CTRL_MAG_AND_ANG == enOutCtrl ? &stDstAng : CVI_NULL,
			&stCtrl,CVI_FALSE);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	dTime = (CVI_DOUBLE)CVI_GetTickCount() - dTime;

	printf("MagAndAng outCtrl %d,time = %g ms\n",enOutCtrl, dTime/((CVI_DOUBLE)CVI_GetTickFrequency()*1000.));

	s32Result = CVI_CompareIveImage(&stDstMag,&stRefMag);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	if(IVE_MAG_AND_ANG_OUT_CTRL_MAG_AND_ANG == enOutCtrl)
	{
		s32Result = CVI_CompareIveImage(&stDstAng,&stRefAng);
		CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	}

FAILURE:

	CVI_FCLOSE(fp);
	CVI_FCLOSE(fp2);
	if(IVE_MAG_AND_ANG_OUT_CTRL_MAG_AND_ANG == enOutCtrl)
	{
		CVI_FCLOSE(fp3);
	}
	CVI_DestroyIveImage(&stSrc);
	CVI_DestroyIveImage(&stDstMag);
	CVI_DestroyIveImage(&stRefMag);
	if(IVE_MAG_AND_ANG_OUT_CTRL_MAG_AND_ANG == enOutCtrl)
	{
		CVI_DestroyIveImage(&stDstAng);
		CVI_DestroyIveImage(&stRefAng);
	}

	return s32Result;
}


CVI_S32 MagAndAngSampleMag3x3(CVI_VOID)
{
	IVE_MAG_AND_ANG_OUT_CTRL_E enOutCtrl = IVE_MAG_AND_ANG_OUT_CTRL_MAG;
	CVI_S8 mask[25]={0,0,0,0,0,
					0,-1,0,1,0,
					0,-2,0,2,0,
					0,-1,0,1,0,
					0,0,0,0,0};
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_MagAndAng_Mag3x3_Mag_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_MagAndAng_Mag3x3_Mag_yuv_start;

#else
	const CVI_CHAR *fileName2 = "../data/result/sample_MagAndAng_Mag3x3_Mag.yuv";

#endif
	return MagAndAngSample(enOutCtrl,mask,CVI_FALSE,0,fileName2,NULL);
}
CVI_S32 MagAndAngSampleMag5x5(CVI_VOID)
{
	IVE_MAG_AND_ANG_OUT_CTRL_E enOutCtrl = IVE_MAG_AND_ANG_OUT_CTRL_MAG;
	CVI_S8 mask[25]={-1,-2,0,2,1,
					-4,-8,0,8,4,
					-6,-12,0,12,6,
					-4,-8,0,8,4,
					-1,-2,0,2,1};
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_MagAndAng_Mag5x5_Mag_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_MagAndAng_Mag5x5_Mag_yuv_start;

#else
	const CVI_CHAR *fileName2 = "../data/result/sample_MagAndAng_Mag5x5_Mag.yuv";

#endif
	return MagAndAngSample(enOutCtrl,mask,CVI_FALSE,0,fileName2,NULL);
}
CVI_S32 MagAndAngSampleMagAndAng3x3(CVI_VOID)
{
	IVE_MAG_AND_ANG_OUT_CTRL_E enOutCtrl = IVE_MAG_AND_ANG_OUT_CTRL_MAG_AND_ANG;
	CVI_S8 mask[25]={0,0,0,0,0,
					0,-1,0,1,0,
					0,-2,0,2,0,
					0,-1,0,1,0,
					0,0,0,0,0};
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_MagAndAng_MagAndAng3x3_Mag_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_MagAndAng_MagAndAng3x3_Mag_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_MagAndAng_MagAndAng3x3_Ang_yuv_start;
	const CVI_CHAR *fileName3 = &_binary_test_ive_data_result_sample_MagAndAng_MagAndAng3x3_Ang_yuv_start;

#else
	const CVI_CHAR *fileName2 = "../data/result/sample_MagAndAng_MagAndAng3x3_Mag.yuv";
	const CVI_CHAR *fileName3 = "../data/result/sample_MagAndAng_MagAndAng3x3_Ang.yuv";

#endif
	return MagAndAngSample(enOutCtrl,mask,CVI_FALSE,0,fileName2,fileName3);
}
CVI_S32 MagAndAngSampleMagAndAng5x5(CVI_VOID)
{
	IVE_MAG_AND_ANG_OUT_CTRL_E enOutCtrl = IVE_MAG_AND_ANG_OUT_CTRL_MAG_AND_ANG;
	CVI_S8 mask[25]={-1,-2,0,2,1,
					-4,-8,0,8,4,
					-6,-12,0,12,6,
					-4,-8,0,8,4,
					-1,-2,0,2,1};
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_MagAndAng_MagAndAng5x5_Mag_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_MagAndAng_MagAndAng5x5_Mag_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_MagAndAng_MagAndAng5x5_Ang_yuv_start;
	const CVI_CHAR *fileName3 = &_binary_test_ive_data_result_sample_MagAndAng_MagAndAng5x5_Ang_yuv_start;

#else
	const CVI_CHAR *fileName2 = "../data/result/sample_MagAndAng_MagAndAng5x5_Mag.yuv";
	const CVI_CHAR *fileName3 = "../data/result/sample_MagAndAng_MagAndAng5x5_Ang.yuv";

#endif
	return MagAndAngSample(enOutCtrl,mask,CVI_FALSE,0,fileName2,fileName3);
}
CVI_S32 MagAndAngSampleThresh3x3(CVI_VOID)
{
	IVE_MAG_AND_ANG_OUT_CTRL_E enOutCtrl = IVE_MAG_AND_ANG_OUT_CTRL_MAG;
	CVI_S8 mask[25]={0,0,0,0,0,
		0,-1,0,1,0,
		0,-2,0,2,0,
		0,-1,0,1,0,
		0,0,0,0,0};
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_MagAndAng_Thresh3x3_Mag_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_MagAndAng_Thresh3x3_Mag_yuv_start;

#else
	const CVI_CHAR *fileName2 = "../data/result/sample_MagAndAng_Thresh3x3_Mag.yuv";

#endif
	return MagAndAngSample(enOutCtrl,mask,CVI_TRUE,42,fileName2,NULL);
}
CVI_S32 MagAndAngSampleThresh5x5(CVI_VOID)
{
	IVE_MAG_AND_ANG_OUT_CTRL_E enOutCtrl = IVE_MAG_AND_ANG_OUT_CTRL_MAG;
	CVI_S8 mask[25]={-1,-2,0,2,1,
					-4,-8,0,8,4,
					-6,-12,0,12,6,
					-4,-8,0,8,4,
					-1,-2,0,2,1};
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_MagAndAng_Thresh5x5_Mag_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_MagAndAng_Thresh5x5_Mag_yuv_start;

#else
	const CVI_CHAR *fileName2 = "../data/result/sample_MagAndAng_Thresh5x5_Mag.yuv";

#endif
	return MagAndAngSample(enOutCtrl,mask,CVI_TRUE,18468,fileName2,NULL);
}
