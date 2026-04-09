//#define useOpenCV (1)
#include "CCL.h"
#include "sample_assist.h"
#include "sample_file.h"
#include "sample_define.h"
#include "mpi_ive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream> // std::ofstream
#include <sstream>      // std::ostringstream

#ifdef useOpenCV
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;
#endif

using namespace std;

CVI_S32 CCL_Sample01()
{
	IVE_HANDLE hIveHandle;
	CVI_BOOL bInstant = CVI_TRUE;
	CVI_S32 s32Result;

	IVE_IMAGE_S stIveImg;
	IVE_CCL_CTRL_S stCCLCtrl;
	IVE_DST_MEM_INFO_S stDst;
	IVE_DST_MEM_INFO_S stRef;
	IVE_CCBLOB_S *pstCCBlob = CVI_NULL;
	CVI_U16 u16Width,u16Height,u16Stride;
	CVI_U8 *showImg = CVI_NULL;
	CVI_CHAR achImgName[_MAX_FNAME];
	FILE *fp = NULL;
	FILE *fpInput = NULL;
    CVI_U8* inputData = NULL;
    std::ostringstream filename;
    std::ofstream outfile;
	IVE_DST_MEM_INFO_S stInput;
	CVI_CHAR resultImgName[_MAX_FNAME];
#ifdef useOpenCV
	Mat cvGrayImg, cvBwImg;
    printf("use opencv\n");
#endif
    int inputHW[] = { 720, 1280, 576, 720};

    UNUSED(achImgName);
    UNUSED(fpInput);
    UNUSED(stInput);
    UNUSED(inputData);
    UNUSED(filename);
    UNUSED(outfile);
    UNUSED(inputHW);

	stDst.u32Size = sizeof(IVE_CCBLOB_S);
	pstCCBlob = (IVE_CCBLOB_S *)malloc(stDst.u32Size);
	CVI_CHECK_ET_RET(pstCCBlob, NULL, CVI_FAILURE);
	stDst.u64VirAddr =  (CVI_U64)pstCCBlob;
	stDst.u64PhyAddr = (CVI_U64)stDst.u64VirAddr;

	stRef.u32Size = sizeof(IVE_CCBLOB_S);
	stRef.u64VirAddr =  (CVI_U64)malloc(stRef.u32Size);
	stRef.u64PhyAddr = (CVI_U64)stRef.u64VirAddr;

	for(int i=0; i<2; i++)
	{
        stInput.u64VirAddr = CVI_NULL; 
#ifdef useOpenCV
		snprintf(achImgName, _MAX_FNAME, "../data/img/ccl/ccl_%d.jpg", i);
		cvGrayImg = imread(achImgName, IMREAD_GRAYSCALE);
        if( cvGrayImg.empty() )
        {
            printf("read image failed!\n");
            return CVI_FAILURE;
        }
  #if 0
        filename.str("");
        filename << "Frame_" << i << ".raw";
        outfile.open (filename.str().c_str(), ios::out | ios::binary);
        outfile.write ((char*)(cvGrayImg.data), cvGrayImg.total());  // In byte so frame.total() should be enough ?
        outfile.close();
  #endif
        u16Width = cvGrayImg.cols & (~1);
		u16Height = cvGrayImg.rows & (~1);
#else
        u16Width = inputHW[i * 2 + 1];
		u16Height = inputHW[i * 2];
#endif

		s32Result = CVI_CreateIveImage(&stIveImg, IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
		CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
		u16Stride = stIveImg.au32Stride[0];

#ifdef useOpenCV
		threshold(cvGrayImg, cvBwImg, 128, 255, THRESH_BINARY);

		for(int j = 0;j < u16Height;j++)
		{
			memcpy(((CVI_U8*)stIveImg.au64VirAddr[0]) + j * stIveImg.au32Stride[0], cvBwImg.data + j * cvBwImg.step, u16Width);
		}
        inputData = (CVI_U8*)cvBwImg.data;
#else
        stInput.u32Size = u16Width * u16Height;
        stInput.u64VirAddr =  (CVI_U64)malloc(stInput.u32Size);
        stInput.u64PhyAddr = (CVI_U64)stInput.u64VirAddr;
		snprintf(resultImgName, _MAX_FNAME, "../data/img/ccl/ccl_raw_%d.raw", i);
		s32Result = fopen_s(&fpInput, resultImgName,"rb");
		CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
		s32Result = CVI_MemReadFile(&stInput,fpInput);
		CVI_CHECK_NET_GOTO(s32Result, 0, FAILURE);
		CVI_FCLOSE(fpInput);

        inputData = (CVI_U8*)stInput.u64VirAddr;
        for (int i = 0; i < u16Width * u16Height; i++) {
            inputData[i] = inputData[i] <= 128 ? 0: 255;
        }

        for(int j = 0;j < u16Height;j++)
		{
			memcpy(((CVI_U8*)stIveImg.au64VirAddr[0]) + j * stIveImg.au32Stride[0], inputData + j * stIveImg.au32Stride[0], u16Width);
		}
#endif
		stCCLCtrl.enMode = (IVE_CCL_MODE_E)i;
		stCCLCtrl.u16InitAreaThr = 4;
		stCCLCtrl.u16Step = 2;

		s32Result = CVI_MPI_IVE_CCL(&hIveHandle, &stIveImg, &stDst, &stCCLCtrl, bInstant);
		CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

		showImg	= (CVI_U8*) malloc(u16Width * u16Height);
		memcpy(showImg, inputData, u16Width * u16Height);

		for(int k = 0; k < IVE_MAX_REGION_NUM;k++)
		{
			int left, right, top, bottom,m,n;
			if(pstCCBlob->astRegion[k].u32Area == 0)
				continue;
			left = pstCCBlob->astRegion[k].u16Left;
			right = pstCCBlob->astRegion[k].u16Right;
			top = pstCCBlob->astRegion[k].u16Top;
			bottom = pstCCBlob->astRegion[k].u16Bottom;
			for(n = top; n<= bottom; n++)
			{
				for(m = left; m <= right; m++)
				{
					if(m == left || m == right || n == top || n == bottom)
					{
						showImg[n * u16Width + m] = 255;
					}
				}
			}
		}

		//memcpy(cvBwImg.data,showImg,u16Width * u16Height);
		//sprintf_s(achImgName, "../data/result/sample_CCL_%d.jpg", i);
		//imwrite(achImgName, cvBwImg);
		printf("RegionNum = %d, CurAreaThr = %d\n", pstCCBlob->u8RegionNum, pstCCBlob->u16CurAreaThr);
		//namedWindow("result");
		//imshow("result", cvBwImg);
		//waitKey(2000);

		fp = NULL;
		snprintf(resultImgName, _MAX_FNAME, "../data/result/sample_CCL_%d.bin", i);
		s32Result = fopen_s(&fp, resultImgName,"rb");
		CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
		s32Result = CVI_MemReadFile(&stRef,fp);
		CVI_CHECK_NET_GOTO(s32Result, 0, FAILURE);
		CVI_FCLOSE(fp);
		s32Result = CVI_CompareIveMem(&stDst, &stRef);
		CVI_CHECK_NET_GOTO(s32Result, 0, FAILURE);
		UNUSED(u16Stride);

FAILURE:
		CVI_DestroyIveImage(&stIveImg);
        if (stInput.u64VirAddr != CVI_NULL) {
            free ((CVI_U8*)stInput.u64VirAddr);
        }
		//destroyWindow("result");
	}

	if(CVI_NULL != pstCCBlob)
	{
		free(pstCCBlob);
	}

    free ((CVI_U8*)stRef.u64VirAddr);
	return s32Result;
}
