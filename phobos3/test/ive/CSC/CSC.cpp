#include "CSC.h"
#include "sample_assist.h"
#include "sample_file.h"
#include "sample_define.h"
#include "mpi_ive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CVI_S32 CSCSample(IVE_IMAGE_TYPE_E enSrcType,IVE_CSC_MODE_E enMode,IVE_IMAGE_TYPE_E enDstType,const CVI_CHAR *fileName,const CVI_CHAR *fileName2)
{
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_DST_IMAGE_S stRef;
	IVE_CSC_CTRL_S stCtrl;
	CVI_S32 s32Result;
	CVI_U16 u16Width;
	CVI_U16 u16Height;
	FILE *fp;
	FILE *fp2;
	IVE_HANDLE handle;
	CVI_DOUBLE dTime;


	memset(&stSrc,0,sizeof(IVE_SRC_IMAGE_S));
	memset(&stDst,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stCtrl,0,sizeof(IVE_CSC_CTRL_S));

	fp = NULL;
	fp2 = NULL;
	u16Width = CVI_IMAGE_WIDTH_352;
	u16Height = CVI_IMAGE_HEIGHT_288;

	s32Result = CVI_CreateIveImage(&stSrc,enSrcType,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stDst,enDstType,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stRef,enDstType,u16Width,u16Height);
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

	dTime = (CVI_DOUBLE)CVI_GetTickCount();
	s32Result = CVI_MPI_IVE_CSC(&handle,&stSrc,&stDst,&stCtrl,CVI_FALSE);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	dTime = (CVI_DOUBLE)CVI_GetTickCount() - dTime;

	printf("CSC mode %d,time = %g ms\n",enMode, dTime/((CVI_DOUBLE)CVI_GetTickFrequency()*1000.));

	s32Result = CVI_CompareIveImage(&stDst,&stRef);

FAILURE:

	CVI_FCLOSE(fp);
	CVI_FCLOSE(fp2);
	CVI_DestroyIveImage(&stSrc);
	CVI_DestroyIveImage(&stDst);

	return s32Result;
}


CVI_S32 CSCSampleVideoBT601YUV2RGB(CVI_VOID)
{
	IVE_IMAGE_TYPE_E enSrcType = IVE_IMAGE_TYPE_YUV420SP;
	IVE_IMAGE_TYPE_E enDstType = IVE_IMAGE_TYPE_U8C3_PACKAGE;
	IVE_CSC_MODE_E enMode = IVE_CSC_MODE_VIDEO_BT601_YUV2RGB;
	const CVI_CHAR *fileName = "../data/sp420/00_352x288_SP420.yuv";
	const CVI_CHAR *fileName2 = "../data/result/sample_CSC_YUV2RGB.rgb";

	return CSCSample(enSrcType,enMode,enDstType,fileName,fileName2);
}
