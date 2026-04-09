#include "NCC.h"
#include "sample_assist.h"
#include "sample_file.h"
#include "sample_define.h"
#include "mpi_ive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

CVI_S32 NCCSample(CVI_VOID)
{
	IVE_SRC_IMAGE_S stSrc1;
	IVE_SRC_IMAGE_S stSrc2;
	IVE_DST_MEM_INFO_S stDst;
	IVE_DST_MEM_INFO_S stRef;
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
extern CVI_CHAR _binary_test_ive_data_result_sample_NCC_Mem_bin_start;
	const CVI_CHAR *resultFileName = &_binary_test_ive_data_result_sample_NCC_Mem_bin_start;

#else
	const CVI_CHAR *fileName1 = "../data/y/00_352x288_y.yuv";
	const CVI_CHAR *fileName2 = "../data/y/01_352x288_y.yuv";
	const CVI_CHAR *resultFileName = "../data/result/sample_NCC_Mem.bin";

#endif
	CVI_U64 *numerator;
	CVI_U64 *quadSum1;
	CVI_U64 *quadSum2;
	CVI_FLOAT fR;

	memset(&stSrc1,0,sizeof(IVE_SRC_IMAGE_S));
	memset(&stSrc2,0,sizeof(IVE_SRC_IMAGE_S));
	memset(&stDst,0,sizeof(IVE_DST_MEM_INFO_S));
	memset(&stRef,0,sizeof(IVE_DST_MEM_INFO_S));

	fp1 = NULL;
	fp2 = NULL;
	fp3 = NULL;
	u16Width = CVI_IMAGE_WIDTH_352;
	u16Height = CVI_IMAGE_HEIGHT_288;

	s32Result = CVI_CreateIveImage(&stSrc1,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stSrc2,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	stDst.u32Size = sizeof(IVE_NCC_DST_MEM_S);
	stDst.u64VirAddr = (CVI_U64)malloc(stDst.u32Size);
	CVI_CHECK_ET_GOTO(stDst.u64VirAddr,0,FAILURE);
	stDst.u64PhyAddr = (CVI_U64)stDst.u64VirAddr;

	stRef.u32Size = sizeof(IVE_NCC_DST_MEM_S);
	stRef.u64VirAddr = (CVI_U64)malloc(stRef.u32Size);
	CVI_CHECK_ET_GOTO(stRef.u64VirAddr,0,FAILURE);
	stRef.u64PhyAddr = (CVI_U64)stRef.u64VirAddr;

	s32Result = fopen_s(&fp1,fileName1,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
	s32Result = fopen_s(&fp2,fileName2,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
	s32Result = fopen_s(&fp3,resultFileName,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);

	s32Result = CVI_ReadFile(&stSrc1,fp1);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_ReadFile(&stSrc2,fp2);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_MemReadFile(&stRef,fp3);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	dTime = (CVI_DOUBLE)CVI_GetTickCount();
	s32Result = CVI_MPI_IVE_NCC(&handle,&stSrc1,&stSrc2,&stDst,CVI_FALSE);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	dTime = (CVI_DOUBLE)CVI_GetTickCount() - dTime;

	numerator = (CVI_U64*)stDst.u64VirAddr;
	quadSum1 = (CVI_U64*)(stDst.u64VirAddr + sizeof(CVI_U64));
	quadSum2 = quadSum1 + 1;

	fR = (CVI_FLOAT)((CVI_DOUBLE)*numerator/(sqrt((CVI_DOUBLE)*quadSum1) * sqrt((CVI_DOUBLE)*quadSum2)));

	printf("NCC,r = %f,time = %g ms\n",fR,dTime/((CVI_DOUBLE)CVI_GetTickFrequency()*1000.));

	// reset the reserved field for both buffer
	memset((CVI_U8*)stDst.u64VirAddr + 24, 0, 8);
	memset((CVI_U8*)stRef.u64VirAddr + 24, 0, 8);
	s32Result = CVI_CompareIveMem(&stDst, &stRef);

FAILURE:

	CVI_FCLOSE(fp1);
	CVI_FCLOSE(fp2);
	CVI_FCLOSE(fp3);
	CVI_DestroyIveImage(&stSrc1);
	CVI_DestroyIveImage(&stSrc2);
	CVI_FREE_64(stDst.u64VirAddr);
	CVI_FREE_64(stRef.u64VirAddr);

	return s32Result;
}
