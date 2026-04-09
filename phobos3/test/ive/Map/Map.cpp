#include "Map.h"
#include "sample_assist.h"
#include "sample_file.h"
#include "sample_define.h"
#include "mpi_ive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CVI_VOID GetMap(CVI_U32 *pu32Hist,CVI_U8 *pu8Table,CVI_U16 u16Width, CVI_U16 u16Height)
{
	CVI_S32 s32Sum;
	CVI_FLOAT fScale ;
	CVI_U32 u32Total  = u16Width * u16Height;
	CVI_S32 s32Index = 0;
	CVI_S32 s32Tmp;

	while (0 == pu32Hist[s32Index]) ++s32Index;

	if (pu32Hist[s32Index] == u32Total)
	{
		memset(pu8Table,s32Index,256 * sizeof(CVI_U8));
	}else
	{
		fScale = (256 - 1.f)/( u16Width * u16Height - pu32Hist[s32Index]);

		for (pu8Table[s32Index++] = 0,s32Sum = 0; s32Index < 256; ++s32Index)
		{
			s32Sum += pu32Hist[s32Index];
			s32Tmp = (CVI_S32)(s32Sum * fScale + 0.5);
			pu8Table[s32Index] = CVI_CLIP(s32Tmp,CVI_MAX_U8,CVI_MIN_U8);
		}
	}
	dump_data_u8(pu8Table, 256, "GetMap");
}

CVI_S32 MapSample(CVI_VOID)
{
	IVE_SRC_IMAGE_S stSrc1;
	IVE_SRC_MEM_INFO_S stSrc2;
	IVE_DST_MEM_INFO_S stDst1;
	IVE_DST_IMAGE_S stDst2;
	IVE_DST_MEM_INFO_S stRef1;
	IVE_SRC_IMAGE_S stRef2;
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
extern CVI_CHAR _binary_test_ive_data_result_sample_Map_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_Map_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_Map_Mem_bin_start;
	const CVI_CHAR *fileNameHist = &_binary_test_ive_data_result_sample_Map_Mem_bin_start;

#else
	const CVI_CHAR *fileName = "../data/y/00_352x288_y.yuv";
	const CVI_CHAR *fileName2 = "../data/result/sample_Map.yuv";
	const CVI_CHAR *fileNameHist = "../data/result/sample_Map_Mem.bin";

#endif
	IVE_MAP_CTRL_S stMapCtrl={IVE_MAP_MODE_U8};

	memset(&stSrc1,0,sizeof(IVE_SRC_IMAGE_S));
	memset(&stSrc2,0,sizeof(IVE_SRC_MEM_INFO_S));
	memset(&stDst1,0,sizeof(IVE_DST_MEM_INFO_S));
	memset(&stDst2,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stRef1,0,sizeof(IVE_DST_MEM_INFO_S));
	memset(&stRef2,0,sizeof(IVE_DST_IMAGE_S));

	fp = NULL;
	fp2 = NULL;
	fp3 = NULL;
	u16Width = CVI_IMAGE_WIDTH_352;
	u16Height = CVI_IMAGE_HEIGHT_288;

	s32Result = CVI_CreateIveImage(&stSrc1,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stDst2,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stRef2,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	CVI_ResetIveImage(&stDst2,0);

	switch(stDst2.enType)
	{
	case IVE_IMAGE_TYPE_U8C1:
		{
			stMapCtrl.enMode = IVE_MAP_MODE_U8;
			stSrc2.u32Size = 256* sizeof(CVI_U8);
		}
		break;
	case  IVE_IMAGE_TYPE_S16C1:
		{
			stMapCtrl.enMode  = IVE_MAP_MODE_S16;
			stSrc2.u32Size = 256* sizeof(CVI_S16);
		}
		break;
	case  IVE_IMAGE_TYPE_U16C1:
		{
			stMapCtrl.enMode = IVE_MAP_MODE_U16;
			stSrc2.u32Size = 256* sizeof(CVI_U16);
		}
		break;
	default:
		{
			printf("stMapCtrl.enMode(%d) must be in [0, %d)!\n", stMapCtrl.enMode, IVE_MAP_MODE_BUTT);
			return CVI_FAILURE;
		}
	}

	stSrc2.u64VirAddr = (CVI_U64)malloc(stSrc2.u32Size);
	CVI_CHECK_ET_GOTO(stSrc2.u64VirAddr,0,FAILURE);
	stSrc2.u64PhyAddr = (CVI_U64)stSrc2.u64VirAddr;

	stDst1.u32Size = 256 * sizeof(CVI_U32) + 100;
	stDst1.u64VirAddr = (CVI_U64)malloc(stDst1.u32Size);
	CVI_CHECK_ET_GOTO(stDst1.u64VirAddr,0,FAILURE);
	stDst1.u64PhyAddr = (CVI_U64)stDst1.u64VirAddr;

	stRef1.u32Size = 256 * sizeof(CVI_U32) + 100;
	stRef1.u64VirAddr = (CVI_U64)malloc(stRef1.u32Size);
	CVI_CHECK_ET_GOTO(stRef1.u64VirAddr,0,FAILURE);
	stRef1.u64PhyAddr = (CVI_U64)stRef1.u64VirAddr;

	s32Result = fopen_s(&fp,fileName,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
	s32Result = fopen_s(&fp2,fileName2,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
	s32Result = fopen_s(&fp3,fileNameHist,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);

	s32Result = CVI_ReadFile(&stSrc1,fp);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_ReadFile(&stRef2,fp2);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_MemReadFile(&stRef1,fp3);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	s32Result = CVI_MPI_IVE_Hist(&handle,&stSrc1,&stDst1,CVI_FALSE);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	// clear the last 100 bytes for both buffer
	memset((CVI_U8*)stDst1.u64VirAddr + 256 * sizeof(CVI_U32), 0, 100);
	memset((CVI_U8*)stRef1.u64VirAddr + 256 * sizeof(CVI_U32), 0, 100);
	s32Result = CVI_CompareIveMem(&stDst1,&stRef1);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	GetMap((CVI_U32*)stDst1.u64VirAddr,(CVI_U8*)stSrc2.u64VirAddr,u16Width,u16Height);

	dTime = (CVI_DOUBLE)CVI_GetTickCount();
	s32Result = CVI_MPI_IVE_Map(&handle,&stSrc1,&stSrc2,&stDst2, &stMapCtrl , CVI_FALSE);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	dTime = (CVI_DOUBLE)CVI_GetTickCount() - dTime;

	printf("Map time = %g ms\n", dTime/((CVI_DOUBLE)CVI_GetTickFrequency()*1000.));

	s32Result = CVI_CompareIveImage(&stDst2, &stRef2);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

FAILURE:

	CVI_FCLOSE(fp);
	CVI_FCLOSE(fp2);
	CVI_FCLOSE(fp3);
	CVI_DestroyIveImage(&stSrc1);
	CVI_DestroyIveImage(&stDst2);
	CVI_DestroyIveImage(&stRef2);
	CVI_FREE_64(stSrc2.u64VirAddr);
	CVI_FREE_64(stDst1.u64VirAddr);
	CVI_FREE_64(stRef1.u64VirAddr);

	return s32Result;
}
