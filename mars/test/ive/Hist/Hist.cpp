#include "Hist.h"
#include "sample_assist.h"
#include "sample_file.h"
#include "sample_define.h"
#include "mpi_ive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


CVI_S32 HistSample(CVI_VOID)
{
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_MEM_INFO_S stDst;
	IVE_DST_MEM_INFO_S stRef;
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
extern CVI_CHAR _binary_test_ive_data_result_sample_Hist_bin_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_Hist_bin_start;

#else
	const CVI_CHAR *fileName = "../data/y/00_352x288_y.yuv";
	const CVI_CHAR *fileName2 = "../data/result/sample_Hist.bin";

#endif
	memset(&stSrc,0,sizeof(IVE_SRC_IMAGE_S));
	memset(&stDst,0,sizeof(IVE_DST_MEM_INFO_S));

	fp = NULL;
	fp2 = NULL;
	u16Width = CVI_IMAGE_WIDTH_352;
	u16Height = CVI_IMAGE_HEIGHT_288;

	s32Result = CVI_CreateIveImage(&stSrc,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	stDst.u32Size = 256 * sizeof(CVI_U32);
	stDst.u64VirAddr = (CVI_U64)malloc(stDst.u32Size);
	CVI_CHECK_ET_GOTO(stDst.u64VirAddr,0,FAILURE);
	stDst.u64PhyAddr = (CVI_U64)stDst.u64VirAddr;

	stRef.u32Size = 256 * sizeof(CVI_U32);
	stRef.u64VirAddr = (CVI_U64)malloc(stRef.u32Size);
	CVI_CHECK_ET_GOTO(stRef.u64VirAddr,0,FAILURE);
	stRef.u64PhyAddr = (CVI_U64)stRef.u64VirAddr;

	s32Result = fopen_s(&fp,fileName,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
	s32Result = fopen_s(&fp2,fileName2,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);

	s32Result = CVI_ReadFile(&stSrc,fp);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_MemReadFile(&stRef,fp2);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	dTime = (CVI_DOUBLE)CVI_GetTickCount();
	s32Result = CVI_MPI_IVE_Hist(&handle,&stSrc,&stDst,CVI_FALSE);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	dTime = (CVI_DOUBLE)CVI_GetTickCount() - dTime;

	printf( "Histogram time = %g ms\n", dTime/((CVI_DOUBLE)CVI_GetTickFrequency()*1000.));

	s32Result = CVI_CompareIveMem(&stDst,&stRef);

FAILURE:

	CVI_FCLOSE(fp);
	CVI_FCLOSE(fp2);
	CVI_DestroyIveImage(&stSrc);
	CVI_FREE_64(stDst.u64VirAddr);

	return s32Result;
}
