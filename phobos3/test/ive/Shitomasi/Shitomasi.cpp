//#define useOpenCV (1)
#include "Shitomasi.h"
#include "sample_assist.h"
#include "sample_define.h"
#include "sample_file.h"
#include "mpi_ive.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h> /* memset */
using namespace std;

#ifdef useOpenCV
#include "opencv2/opencv.hpp"
using namespace cv;
#endif

CVI_S32 ShitomasiSample(int show, int compare)
{
	IVE_IMAGE_S stSrcImg, stCondiCorner;
	IVE_IMAGE_S stCondiCornerRef;
	IVE_MEM_INFO_S stCorner;
	IVE_MEM_INFO_S stCornerRef;
	IVE_ST_CANDI_CORNER_CTRL_S stStCandiCornerCtrl;
	IVE_MEM_INFO_S stStCandiCornerCtrlMemRef;
	IVE_ST_CORNER_CTRL_S stStCornerCtrl;
	IVE_ST_CORNER_INFO_S *pstCornerInfo;

	IVE_HANDLE hIveHandle;
	CVI_BOOL bInstant = CVI_TRUE;
	CVI_S32 s32Result;
#ifdef useOpenCV
	Mat image, gray;
	Point2f pointf;
#endif
	FILE *fp;
	const CVI_CHAR *fileName1 = "../data/result/sample_Shitomasi_CandiCorner.yuv";
	//const CVI_CHAR *fileName2 = "../data/result/sample_Shitomasi_CandiCornerMem.bin";
	//const CVI_CHAR *fileName3 = "../data/result/sample_Shitomasi_Corner.bin";
	//const CVI_CHAR *fileName_tmp = "./sample_Shitomasi_CandiCornerMem.bin";

	//memset(&pSrcImage,0,sizeof(IVE_SRC_IMAGE_S));
	memset(&stCorner,0,sizeof(IVE_MEM_INFO_S));
	memset(&stCornerRef,0,sizeof(IVE_MEM_INFO_S));
	memset(&stStCandiCornerCtrl,0,sizeof(IVE_ST_CANDI_CORNER_CTRL_S));
	memset(&stStCandiCornerCtrlMemRef,0,sizeof(IVE_MEM_INFO_S));

    FILE *fpInput = NULL;
	IVE_DST_MEM_INFO_S stInput;
    CVI_U16 u16Height = 576;
    CVI_U16 u16Width = 720;
    CVI_U16 step = u16Width;
	CVI_CHAR resultImgName[_MAX_FNAME];
    stInput.u64VirAddr = CVI_NULL;
    CVI_U8* inputData = NULL;

    UNUSED(resultImgName);
    UNUSED(fpInput);
    UNUSED(inputData);
    UNUSED(stInput);
    UNUSED(pstCornerInfo);

#ifdef useOpenCV
	image = imread("../data/img/shitomasi/penguin.jpg", 1);
	if( image.empty() )
	{
		printf("image is Null");
		return CVI_FAILURE;
	}
	cvtColor( image, gray, CV_BGR2GRAY );
    inputData = gray.data;
#else
    // gray
    stInput.u32Size = u16Height * u16Width;
    stInput.u64VirAddr =  (CVI_U64)malloc(stInput.u32Size);
    stInput.u64PhyAddr = (CVI_U64)stInput.u64VirAddr;
    inputData = (CVI_U8* )stInput.u64VirAddr;

    snprintf(resultImgName, _MAX_FNAME, "../data/img/shitomasi/penguin.gray.shitomasi.raw");
    s32Result = fopen_s(&fpInput, resultImgName,"rb");
    CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
    s32Result = CVI_MemReadFile(&stInput,fpInput);
    CVI_CHECK_NET_GOTO(s32Result, 0, FAILURE);
    CVI_FCLOSE(fpInput);

#endif

	s32Result = CVI_CreateIveImage(&stSrcImg,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	for(CVI_S32 i = 0;i < u16Height;i++)
	{
		memcpy((CVI_U8*)stSrcImg.au64VirAddr[0] + i *  stSrcImg.au32Stride[0], inputData + i *  step, stSrcImg.u32Width);
	}

	s32Result = CVI_CreateIveImage(&stCondiCorner,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stCondiCornerRef,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	stCorner.u32Size = sizeof(IVE_ST_CORNER_INFO_S);
	stCorner.u64VirAddr = (CVI_U64)malloc(stCorner.u32Size);
	stCorner.u64PhyAddr = (CVI_U64)stCorner.u64VirAddr;

	stCornerRef.u32Size = sizeof(IVE_ST_CORNER_INFO_S);
	stCornerRef.u64VirAddr = (CVI_U64)malloc(stCornerRef.u32Size);
	stCornerRef.u64PhyAddr = (CVI_U64)stCornerRef.u64VirAddr;

	stStCandiCornerCtrl.u0q8QualityLevel = 25;
	stStCandiCornerCtrl.stMem.u32Size = 4 * stSrcImg.u32Height * stSrcImg.au32Stride[0] + sizeof(IVE_ST_MAX_EIG_S);
	//stStCandiCornerCtrl.stMem.u64VirAddr = (CVI_U64)malloc(2 * (2 * stSrcImg.u32Height + 1) * stSrcImg.au32Stride[0]);
	stStCandiCornerCtrl.stMem.u64VirAddr = (CVI_U64)malloc(stStCandiCornerCtrl.stMem.u32Size);
	stStCandiCornerCtrl.stMem.u64PhyAddr = (CVI_U64)stStCandiCornerCtrl.stMem.u64VirAddr;

	stStCandiCornerCtrlMemRef.u32Size = 4 * stSrcImg.u32Height * stSrcImg.au32Stride[0] + sizeof(IVE_ST_MAX_EIG_S);
	stStCandiCornerCtrlMemRef.u64VirAddr = (CVI_U64)malloc(stStCandiCornerCtrlMemRef.u32Size);
	stStCandiCornerCtrlMemRef.u64PhyAddr = (CVI_U64)stStCandiCornerCtrlMemRef.u64VirAddr;

	s32Result = CVI_MPI_IVE_STCandiCorner(&hIveHandle, &stSrcImg, &stCondiCorner, &stStCandiCornerCtrl, bInstant);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	if (compare)
	{
		s32Result = fopen_s(&fp, fileName1, "rb");
		CVI_CHECK_NET_GOTO(s32Result, 0, FAILURE);
		s32Result = CVI_ReadFile(&stCondiCornerRef, fp);
		CVI_FCLOSE(fp);
		s32Result = CVI_CompareIveImage(&stCondiCorner, &stCondiCornerRef);
		CVI_CHECK_NET_GOTO(s32Result, 0, FAILURE);

		//s32Result = fopen_s(&fp,fileName_tmp,"wb");
		//CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
		//s32Result = CVI_MemWriteFile(&stStCandiCornerCtrl.stMem, fp);
		//CVI_FCLOSE(fp);

		//s32Result = fopen_s(&fp,fileName2,"rb");
		//CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
		//s32Result = CVI_MemReadFile(&stStCandiCornerCtrlMemRef, fp);
		//CVI_FCLOSE(fp);
		//s32Result = CVI_CompareIveMem(&stStCandiCornerCtrl.stMem,&stStCandiCornerCtrlMemRef);
		//CVI_CHECK_NET_GOTO(s32Result, 0, FAILURE);
	}

	stStCornerCtrl.u16MaxCornerNum = 200;
	stStCornerCtrl.u16MinDist = 10;

	pstCornerInfo = (IVE_ST_CORNER_INFO_S *)stCorner.u64VirAddr;

	s32Result = CVI_MPI_IVE_STCorner(&stCondiCorner, &stCorner, &stStCornerCtrl);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	if (compare)
	{
		//s32Result = fopen_s(&fp,fileName3,"rb");
		//CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
		//s32Result = CVI_MemReadFile(&stCornerRef, fp);
		//CVI_FCLOSE(fp);
		//s32Result = CVI_CompareIveMem(&stCorner,&stCornerRef);
		//CVI_CHECK_NET_GOTO(s32Result, 0, FAILURE);
	}

	if (show)
	{
#ifdef useOpenCV
		printf("Total Corner Number %d\n", pstCornerInfo->u16CornerNum);
		for(CVI_U32 j=0; j<pstCornerInfo->u16CornerNum; j++)
		{
			pointf.x = pstCornerInfo->astCorner[j].u16X;
			pointf.y = pstCornerInfo->astCorner[j].u16Y;
			circle( image, pointf, 3, Scalar(0,255,0), -1, 8);
		}
		imwrite("./sample_Shitomasi.jpg",image);
		imshow("result", image);
		cvWaitKey(0);
#endif
	}
#ifdef useOpenCV
    printf("use opencv\n");
#endif

FAILURE:
	CVI_FREE_64(stStCandiCornerCtrl.stMem.u64VirAddr);
	CVI_FREE_64(stStCandiCornerCtrlMemRef.u64VirAddr);
	CVI_DestroyIveImage(&stCondiCorner);
	CVI_DestroyIveImage(&stSrcImg);
	CVI_DestroyIveImage(&stCondiCornerRef);
	CVI_FREE_64(stCorner.u64VirAddr);
	CVI_FREE_64(stCornerRef.u64VirAddr);
    if (stInput.u64VirAddr != CVI_NULL)
        CVI_FREE_64(stInput.u64VirAddr);
	//image = imread("../data/img/00_720x576.jpg", 1 );
	return s32Result;
}
