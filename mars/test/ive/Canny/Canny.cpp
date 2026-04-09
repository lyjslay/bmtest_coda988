#include "Canny.h"
#include "sample_assist.h"
#include "sample_file.h"
#include "sample_define.h"
#include "mpi_ive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CVI_S32 CannySample(CVI_S8 *ps8Mask,CVI_U16 u16LowThr,
		const CVI_CHAR *fileName2,const CVI_CHAR *fileNameMem,const CVI_CHAR *fileNameMem_0)
{
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stEdge;
	IVE_DST_IMAGE_S stEdgeRef;
	IVE_DST_MEM_INFO_S stStack;
	IVE_DST_MEM_INFO_S stStackRef;
	IVE_DST_MEM_INFO_S stStackRef_0;
	IVE_CANNY_HYS_EDGE_CTRL_S stCannyHysEdgeCtrl;

	CVI_S32 s32Result;
	CVI_U16 u16Width;
	CVI_U16 u16Height;
	FILE *fp;
	FILE *fp2;
	FILE *fp3;
	FILE *fp4;
	IVE_HANDLE handle;
	CVI_DOUBLE dTime;
	const CVI_CHAR *fileName = "../data/y/00_352x288_y.yuv";

	memset(&stSrc,0,sizeof(IVE_SRC_IMAGE_S));
	memset(&stEdge,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stEdgeRef,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stStack,0,sizeof(IVE_DST_MEM_INFO_S));
	memset(&stStackRef,0,sizeof(IVE_DST_MEM_INFO_S));
	memset(&stStackRef_0,0,sizeof(IVE_DST_MEM_INFO_S));
	memset(&stCannyHysEdgeCtrl,0,sizeof(IVE_CANNY_HYS_EDGE_CTRL_S));

	fp = NULL;
	fp2 = NULL;
	fp3 = NULL;
	fp4 = NULL;
	u16Width = CVI_IMAGE_WIDTH_352;
	u16Height = CVI_IMAGE_HEIGHT_288;

	s32Result = CVI_CreateIveImage(&stSrc,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stEdge,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stEdgeRef,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	stStack.u32Size = stSrc.u32Width * stSrc.u32Height * (sizeof(IVE_POINT_U16_S)) + sizeof(IVE_CANNY_STACK_SIZE_S);
	stStack.u64VirAddr = (CVI_U64)malloc(stStack.u32Size);
	stStack.u64PhyAddr = (CVI_U64)stStack.u64VirAddr;
	CVI_CHECK_ET_GOTO(stStack.u64VirAddr, 0, FAILURE);
	memset((CVI_U8 *)stStack.u64VirAddr,0,stStack.u32Size);

	stStackRef.u32Size = stStack.u32Size;
	stStackRef.u64VirAddr = (CVI_U64)malloc(stStackRef.u32Size);
	stStackRef.u64PhyAddr = (CVI_U64)stStackRef.u64VirAddr;
	CVI_CHECK_ET_GOTO(stStackRef.u64VirAddr, 0, FAILURE);
	memset((CVI_U8 *)stStackRef.u64VirAddr,0,stStackRef.u32Size);

	stStackRef_0.u32Size = stStack.u32Size;
	stStackRef_0.u64VirAddr = (CVI_U64)malloc(stStackRef_0.u32Size);
	stStackRef_0.u64PhyAddr = (CVI_U64)stStackRef_0.u64VirAddr;
	CVI_CHECK_ET_GOTO(stStackRef_0.u64VirAddr, 0, FAILURE);
	memset((CVI_U8 *)stStackRef_0.u64VirAddr,0,stStackRef_0.u32Size);

	stCannyHysEdgeCtrl.stMem.u32Size = CVI_IMAGE_WIDTH_704 * CVI_IMAGE_HEIGHT_576 *(sizeof(CVI_U16) + sizeof(CVI_U8) );
	stCannyHysEdgeCtrl.stMem.u64VirAddr = (CVI_U64)malloc(stCannyHysEdgeCtrl.stMem.u32Size);
	CVI_CHECK_ET_GOTO(stCannyHysEdgeCtrl.stMem.u64VirAddr,0,FAILURE);
	stCannyHysEdgeCtrl.stMem.u64PhyAddr = (CVI_U64)stCannyHysEdgeCtrl.stMem.u64VirAddr;
	//stCannyHysEdgeCtrl.u16LowThr = CVI_GenRand(CVI_MAX_U8,1);
	stCannyHysEdgeCtrl.u16LowThr = u16LowThr;
	printf("u16LowThr = %d\n", stCannyHysEdgeCtrl.u16LowThr);
	stCannyHysEdgeCtrl.u16HighThr= 3 * stCannyHysEdgeCtrl.u16LowThr;
	memcpy(stCannyHysEdgeCtrl.as8Mask,ps8Mask,25 * sizeof(CVI_S8));

	s32Result = fopen_s(&fp,fileName,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
	s32Result = fopen_s(&fp2,fileName2,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
	s32Result = fopen_s(&fp3,fileNameMem,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
	s32Result = fopen_s(&fp4,fileNameMem_0,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);

	s32Result = CVI_ReadFile(&stSrc,fp);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_ReadFile(&stEdgeRef,fp2);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_MemReadFile(&stStackRef,fp3);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_MemReadFile(&stStackRef_0,fp4);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	dTime = (CVI_DOUBLE)CVI_GetTickCount();
	s32Result = CVI_MPI_IVE_CannyHysEdge(&handle,&stSrc,&stEdge, &stStack, &stCannyHysEdgeCtrl,CVI_FALSE);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	s32Result = CVI_CompareIveMem(&stStack,&stStackRef_0);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	s32Result = CVI_MPI_IVE_CannyEdge(&stEdge, &stStack);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	dTime = (CVI_DOUBLE)CVI_GetTickCount() - dTime;

	printf("Canny time = %g ms\n", dTime/((CVI_DOUBLE)CVI_GetTickFrequency()*1000.));

	//s32Result = CVI_CompareIveMem(&stStack,&stStackRef);
	//CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	//s32Result = CVI_CompareIveImage(&stEdge,&stEdgeRef);
	//CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

FAILURE:

	CVI_FCLOSE(fp);
	CVI_FCLOSE(fp2);
	CVI_FCLOSE(fp3);
	CVI_DestroyIveImage(&stSrc);
	CVI_DestroyIveImage(&stEdge);
	CVI_DestroyIveImage(&stEdgeRef);
	CVI_FREE_64(stStack.u64VirAddr);
	CVI_FREE_64(stStackRef.u64VirAddr);
	CVI_FREE_64(stCannyHysEdgeCtrl.stMem.u64VirAddr);

	return s32Result;
}

CVI_S32 CannySample3x3(CVI_VOID)
{
	CVI_S8 mask[25]={0,0,0,0,0,
					0,-1,0,1,0,
					0,-2,0,2,0,
					0,-1,0,1,0,
					0,0,0,0,0};
	const CVI_CHAR *fileName2 = "../data/result/sample_Canny_3x3.yuv";
	const CVI_CHAR *fileNameMem = "../data/result/sample_Canny_Mem_3x3.bin";
	const CVI_CHAR *fileNameMem_0 = "../data/result/sample_Canny_Mem_3x3_0.bin";

	return CannySample(mask,42,fileName2,fileNameMem,fileNameMem_0);
}
CVI_S32 CannySample5x5(CVI_VOID)
{
	CVI_S8 mask[25]={-1,-2,0,2,1,
					-4,-8,0,8,4,
					-6,-12,0,12,6,
					-4,-8,0,8,4,
					-1,-2,0,2,1};
	const CVI_CHAR *fileName2 = "../data/result/sample_Canny_5x5.yuv";
	const CVI_CHAR *fileNameMem = "../data/result/sample_Canny_Mem_5x5.bin";
	const CVI_CHAR *fileNameMem_0 = "../data/result/sample_Canny_Mem_5x5_0.bin";

	return CannySample(mask,108,fileName2,fileNameMem,fileNameMem_0);
}