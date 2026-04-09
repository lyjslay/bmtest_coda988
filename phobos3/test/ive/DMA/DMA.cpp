#include "DMA.h"
#include "sample_assist.h"
#include "sample_file.h"
#include "sample_define.h"
#include "mpi_ive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CVI_S32 DMASample(IVE_DMA_MODE_E enMode,const CVI_CHAR *fileName2)
{
	IVE_DATA_S stSrc1;
	IVE_SRC_IMAGE_S stSrc2;
	IVE_DATA_S stDst;
	IVE_DATA_S stRef;
	IVE_DMA_CTRL_S stCtrl;
	CVI_S32 s32Result;
	CVI_U16 u16Width;
	CVI_U16 u16Height;
	FILE *fp;
	FILE *fp2;
	IVE_HANDLE handle;
	CVI_DOUBLE dTime;
	const CVI_CHAR *fileName = "../data/y/00_352x288_y.yuv";
	CVI_BOOL bUse =   CVI_FALSE;

	memset(&stSrc1,0,sizeof(IVE_DATA_S));
	memset(&stSrc2,0,sizeof(IVE_SRC_IMAGE_S));
	memset(&stDst,0,sizeof(IVE_DATA_S));
	memset(&stRef,0,sizeof(IVE_DATA_S));
	memset(&stCtrl,0,sizeof(IVE_DMA_CTRL_S));

	fp = NULL;
	fp2 = NULL;
	u16Width = CVI_IMAGE_WIDTH_352;
	u16Height = CVI_IMAGE_HEIGHT_288;

	s32Result = CVI_CreateIveImage(&stSrc2,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	s32Result = fopen_s(&fp,fileName,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
	s32Result = fopen_s(&fp2,fileName2,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);

	s32Result = CVI_ReadFile(&stSrc2,fp);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	stSrc1.u64VirAddr = stSrc2.au64VirAddr[0];
	stSrc1.u64PhyAddr = stSrc2.au64PhyAddr[0];
	stSrc1.u32Stride = stSrc2.au32Stride[0];
	stSrc1.u32Width = stSrc2.u32Width;
	stSrc1.u32Height = stSrc2.u32Height;

	stCtrl.enMode = enMode;

	switch(enMode)
	{
	case IVE_DMA_MODE_DIRECT_COPY:
		{
			stDst.u64VirAddr = (CVI_U64)malloc(stSrc1.u32Stride * stSrc1.u32Height);
			CVI_CHECK_ET_GOTO(stDst.u64VirAddr,0,FAILURE);
			stDst.u64PhyAddr = (CVI_U64)stDst.u64VirAddr;
			stDst.u32Stride = stSrc1.u32Stride;
			stDst.u32Width = stSrc1.u32Width;
			stDst.u32Height = stSrc1.u32Height;

			bUse = CVI_TRUE;
		}
		break;
	case IVE_DMA_MODE_INTERVAL_COPY:
		{
			stCtrl.u8ElemSize = 1;
			stCtrl.u8HorSegSize = 4;
			stCtrl.u8VerSegRows = 1;

			stDst.u32Width = (stSrc1.u32Width / stCtrl.u8HorSegSize) * stCtrl.u8ElemSize;
			stDst.u32Height = stSrc1.u32Height/stCtrl.u8VerSegRows;
			stDst.u32Stride = CVI_CalcStride(stDst.u32Width,CVI_IVE2_STRIDE_ALIGN);
			stDst.u64VirAddr = (CVI_U64)malloc(stDst.u32Stride * stDst.u32Height);
			CVI_CHECK_ET_GOTO(stDst.u64VirAddr,0,FAILURE);
			stDst.u64PhyAddr = (CVI_U64)stDst.u64VirAddr;

			bUse = CVI_TRUE;
		}
		break;
	case IVE_DMA_MODE_SET_3BYTE:
	case IVE_DMA_MODE_SET_8BYTE:
		{
			stCtrl.u64Val = 0;
		}
		break;
	default:
		break;

	}

	if (enMode == IVE_DMA_MODE_INTERVAL_COPY)
	{
		stRef.u32Width = (stSrc1.u32Width / stCtrl.u8HorSegSize) * stCtrl.u8ElemSize;
		stRef.u32Height = stSrc1.u32Height/stCtrl.u8VerSegRows;
		stRef.u32Stride = CVI_CalcStride(stRef.u32Width,CVI_IVE2_STRIDE_ALIGN);
		stRef.u64VirAddr = (CVI_U64)malloc(stRef.u32Stride * stRef.u32Height);
		CVI_CHECK_ET_GOTO(stRef.u64VirAddr,0,FAILURE);
		stRef.u64PhyAddr = (CVI_U64)stRef.u64VirAddr;
	}
	else
	{
		stRef.u64VirAddr = (CVI_U64)malloc(stSrc1.u32Stride * stSrc1.u32Height);
		CVI_CHECK_ET_GOTO(stRef.u64VirAddr,0,FAILURE);
		stRef.u64PhyAddr = (CVI_U64)stRef.u64VirAddr;
		stRef.u32Stride = stSrc1.u32Stride;
		stRef.u32Width = stSrc1.u32Width;
		stRef.u32Height = stSrc1.u32Height;
	}

	dTime = (CVI_DOUBLE)CVI_GetTickCount();
	s32Result = CVI_MPI_IVE_DMA(&handle,&stSrc1,CVI_TRUE == bUse ? &stDst : CVI_NULL,&stCtrl,CVI_FALSE);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	dTime = (CVI_DOUBLE)CVI_GetTickCount() - dTime;

	printf("DMA mode %d,time = %g ms\n",enMode, dTime/((CVI_DOUBLE)CVI_GetTickFrequency()*1000.));

	s32Result = CVI_DataReadFile(&stRef,fp2);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CompareIveData(CVI_TRUE == bUse ? &stDst : &stSrc1, &stRef);

FAILURE:

	CVI_FCLOSE(fp);
	CVI_FCLOSE(fp2);
	CVI_DestroyIveImage(&stSrc2);
	CVI_FREE_64(stDst.u64VirAddr);
	CVI_FREE_64(stRef.u64VirAddr);

	return s32Result;
}

CVI_S32 DMASampleDirectCopy(CVI_VOID)
{
	IVE_DMA_MODE_E enMode = IVE_DMA_MODE_DIRECT_COPY;
	const CVI_CHAR *fileName2 = "../data/result/sample_DMA_Direct.bin";

	return DMASample(enMode,fileName2);
}
CVI_S32 DMASampleIntervalCopy(CVI_VOID)
{
	IVE_DMA_MODE_E enMode = IVE_DMA_MODE_INTERVAL_COPY;
	const CVI_CHAR *fileName2 = "../data/result/sample_DMA_Interval.bin";

	return DMASample(enMode,fileName2);
}
CVI_S32 DMASampleSet3Byte(CVI_VOID)
{
	IVE_DMA_MODE_E enMode = IVE_DMA_MODE_SET_3BYTE;
	const CVI_CHAR *fileName2 = "../data/result/sample_DMA_Set3Byte.bin";

	return DMASample(enMode,fileName2);
}
CVI_S32 DMASampleSet8Byte(CVI_VOID)
{
	IVE_DMA_MODE_E enMode = IVE_DMA_MODE_SET_8BYTE;
	const CVI_CHAR *fileName2 = "../data/result/sample_DMA_Set8Byte.bin";

	return DMASample(enMode,fileName2);
}