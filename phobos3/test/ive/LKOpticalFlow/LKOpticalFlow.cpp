#include "LKOpticalFlow.h"
#include "sample_assist.h"
#include "sample_define.h"
#include "sample_file.h"
#include "opencv2/opencv.hpp"
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

using namespace cv;
using namespace std;

CVI_VOID Mat2IveImage(cv::Mat *src, IVE_DST_IMAGE_S *pstDst)
{
	if(src->cols != (int)pstDst->u32Width || src->rows != (int)pstDst->u32Height)
	{
		printf("width or height didn't equal,line:%d\n",__LINE__);
		return;
	}

	for(int r=0; r<src->rows; r++)
	{
		CVI_U8 *ptm = src->ptr(r);
		CVI_U8 *pti = (CVI_U8*)(pstDst->au64VirAddr[0] + r * pstDst->au32Stride[0]);
		{
			for(int c=0; c<src->cols; c++)
			{
				pti[c] = ptm[c];
			}
		}
	}
}

CVI_VOID IveImage2Mat(IVE_SRC_IMAGE_S *pstSrc, Mat *dst)
{
	if(dst->cols != (int)pstSrc->u32Width || dst->rows != (int)pstSrc->u32Height)
	{
		printf("width or height didn't equal,line:%d\n",__LINE__);
		return;
	}

	for(CVI_U32 r=0; r<pstSrc->u32Height; r++)
	{
		CVI_U8 *ptm = dst->ptr(r);
		CVI_U8 *pti = (CVI_U8*)(pstSrc->au64VirAddr[0] + r * pstSrc->au32Stride[0]);
		{
			for(CVI_U32 c=0; c<pstSrc->u32Width; c++)
			{
				ptm[c] = pti[c];
			}
		}
	}
}

CVI_VOID InitLKOpticalFlowPyr(CVI_U16 u16Width, CVI_U16 u16Height, CVI_U8 u8MaxLevel, IVE_MEM_INFO_S *pstMem,
	IVE_IMAGE_S astPrevPyr[], IVE_IMAGE_S astNextPyr[], IVE_MEM_INFO_S *pstPrevPts,
	IVE_MEM_INFO_S *pstNextPts, IVE_MEM_INFO_S *pstStatus, IVE_MEM_INFO_S *pstErr)
{
	//CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32Size, i;
	CVI_U32 u32ImgSize[4];
	CVI_U8 *pu8Mem;

	u32Size = 0;
	for(i=0; i<=u8MaxLevel; i++)
	{
		astPrevPyr[i].enType    = IVE_IMAGE_TYPE_U8C1;
		astPrevPyr[i].u32Width  = u16Width  >> i;
		astPrevPyr[i].u32Height = u16Height >> i;
		astPrevPyr[i].au32Stride[0] = CVI_CalcStride(astPrevPyr[i].u32Width, IVE_STRIDE_ALIGN);

		astNextPyr[i].enType    = IVE_IMAGE_TYPE_U8C1;
		astNextPyr[i].u32Width  = astPrevPyr[i].u32Width;
		astNextPyr[i].u32Height = astPrevPyr[i].u32Height;
		astNextPyr[i].au32Stride[0] = astPrevPyr[i].au32Stride[0];

		u32ImgSize[i] = astPrevPyr[i].au32Stride[0] * astPrevPyr[i].u32Height;
		u32Size += 2 * u32ImgSize[i];
	}

	pstPrevPts->u32Size = CVI_CalcStride(sizeof(IVE_POINT_S25Q7_S) * LK_OPTICAL_FLOW_MAX_POINT_NUM, IVE_STRIDE_ALIGN);
	pstNextPts->u32Size = pstPrevPts->u32Size;
	u32Size += 2 * pstPrevPts->u32Size;

	pstStatus->u32Size = CVI_CalcStride(sizeof(CVI_U8) * LK_OPTICAL_FLOW_MAX_POINT_NUM, IVE_STRIDE_ALIGN);
	u32Size += pstStatus->u32Size;

	pstErr->u32Size = CVI_CalcStride(sizeof(CVI_U9Q7) * LK_OPTICAL_FLOW_MAX_POINT_NUM, IVE_STRIDE_ALIGN);
	u32Size += pstErr->u32Size;

	pstMem->u32Size    = u32Size;
	pstMem->u64VirAddr = (CVI_U64)malloc(u32Size);
	if(!pstMem->u64VirAddr)
	{
		printf("malloc failed\n");
		return;
	}
	pstMem->u64PhyAddr = (CVI_U64)pstMem->u64VirAddr;

	pu8Mem = (CVI_U8*)pstMem->u64VirAddr;
	for(i=0; i<=u8MaxLevel; i++)
	{
		astPrevPyr[i].au64VirAddr[0] = (CVI_U64)pu8Mem;
		astPrevPyr[i].au64PhyAddr[0] = (CVI_U64)pu8Mem;
		pu8Mem += u32ImgSize[i];

		astNextPyr[i].au64VirAddr[0] = (CVI_U64)pu8Mem;
		astNextPyr[i].au64PhyAddr[0] = (CVI_U64)pu8Mem;
		pu8Mem += u32ImgSize[i];
	}

	pstPrevPts->u64VirAddr = (CVI_U64)pu8Mem;
	pstPrevPts->u64PhyAddr = (CVI_U64)pu8Mem;
	pu8Mem += pstPrevPts->u32Size;

	pstNextPts->u64VirAddr = (CVI_U64)pu8Mem;
	pstNextPts->u64PhyAddr = (CVI_U64)pu8Mem;
	pu8Mem += pstNextPts->u32Size;

	pstStatus->u64VirAddr = (CVI_U64)pu8Mem;
	pstStatus->u64PhyAddr = (CVI_U64)pu8Mem;
	pu8Mem += pstStatus->u32Size;

	pstErr->u64VirAddr = (CVI_U64)pu8Mem;
	pstErr->u64PhyAddr = (CVI_U64)pu8Mem;
}

CVI_S32 BuildLKOpticalFlowPyr(CVI_U8 u8MaxLevel, IVE_IMAGE_S astPrevPyr[], IVE_IMAGE_S astNextPyr[])
{
	Mat matPyrPrev[4], matPyrNext[4];

	matPyrPrev[0].create(astPrevPyr[0].u32Height, astPrevPyr[0].u32Width, CV_8UC1);
	matPyrNext[0].create(astNextPyr[0].u32Height, astNextPyr[0].u32Width, CV_8UC1);

	IveImage2Mat(&astPrevPyr[0], &matPyrPrev[0]);
	IveImage2Mat(&astNextPyr[0], &matPyrNext[0]);

	for(int i = 0;i < u8MaxLevel; i++)
	{
		pyrDown(matPyrPrev[i], matPyrPrev[i+1]);
		pyrDown(matPyrNext[i], matPyrNext[i+1]);
	}

	for(int i=1; i<=u8MaxLevel; i++)
	{
		Mat2IveImage(&matPyrPrev[i], &astPrevPyr[i]);
		Mat2IveImage(&matPyrNext[i], &astNextPyr[i]);
	}
	return CVI_SUCCESS;
}

CVI_VOID DeinitLKOpticalFlowPyr(CVI_U8 u8MaxLevel, IVE_MEM_INFO_S *pstMem, IVE_IMAGE_S astPrevPyr[], IVE_IMAGE_S astNextPyr[],
	IVE_MEM_INFO_S *pstPrevPts, IVE_MEM_INFO_S *pstNextPts, IVE_MEM_INFO_S *pstStatus, IVE_MEM_INFO_S *pstErr)
{
	CVI_S32 i;
	UNUSED(pstNextPts);

	CVI_FREE_64(pstMem->u64VirAddr);
	memset(pstMem, 0, sizeof(IVE_MEM_INFO_S));
	for(i=0; i<=u8MaxLevel; i++)
	{
		memset((CVI_VOID*)&astPrevPyr[i], 0, sizeof(IVE_IMAGE_S));
		memset((CVI_VOID*)&astNextPyr[i], 0, sizeof(IVE_IMAGE_S));
	}

	memset(pstPrevPts, 0, sizeof(IVE_MEM_INFO_S));
	memset(pstStatus, 0, sizeof(IVE_MEM_INFO_S));
	memset(pstErr, 0, sizeof(IVE_MEM_INFO_S));
}

CVI_S32 LKOpticalFlowPyrSample( int show, int compare )
{
	string video = "../data/avi/lk_src.avi";
	string saveVideo = "lkPyr_result.avi";

	// for LK
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 s32CompareError = 0;
	IVE_HANDLE iveHandle;
	IVE_IMAGE_S astPrevPyr[4], astNextPyr[4];
	IVE_MEM_INFO_S stPrevPts, stNextPts, stStatus, stErr;
	IVE_LK_OPTICAL_FLOW_PYR_CTRL_S stLkPyrCtrl;
	CVI_BOOL bInstant = CVI_TRUE;

	IVE_MEM_INFO_S stMem;
	IVE_POINT_S25Q7_S *prevPts, *nextPts;
	CVI_U8   *statusTan;
	//CVI_U9Q7 *errTan;

	// for opencv
	VideoCapture cap;
	TermCriteria termcrit(TermCriteria::COUNT|TermCriteria::EPS,20,0.03);
	Size subPixWinSize(5,5), winSize(7,7);

	VideoWriter wte;
	Mat gray, prevGray, dispImg;
	vector<Point2f> points[2];
	int width, height;

	// ctrl of LK-tan
	stLkPyrCtrl.enOutMode    = IVE_LK_OPTICAL_FLOW_PYR_OUT_MODE_BOTH;
	stLkPyrCtrl.bUseInitFlow = CVI_FALSE;
	stLkPyrCtrl.u16PtsNum    = LK_OPTICAL_FLOW_MAX_POINT_NUM;
	stLkPyrCtrl.u8IterCnt    = 20;
	stLkPyrCtrl.u8MaxLevel   = 3;
	stLkPyrCtrl.u0q8MinEigThr= 100; //opencv: 0.001
	stLkPyrCtrl.u0q8Eps      = 2;   //opencv: (CVI_U0Q8)(0.03 * 256);


	cap.open(video);
	width  = (int)cap.get(CV_CAP_PROP_FRAME_WIDTH);
	height = (int)cap.get(CV_CAP_PROP_FRAME_HEIGHT);
	// wte.open(saveVideo, VideoWriter::fourcc('X','V','I','D'), cap.get(CV_CAP_PROP_FPS), Size(width, height));
	wte.open(saveVideo, CV_FOURCC('X','V','I','D'), cap.get(CV_CAP_PROP_FPS), Size(width, height));

	InitLKOpticalFlowPyr((CVI_U16)width, (CVI_U16)height, stLkPyrCtrl.u8MaxLevel, &stMem,
		astPrevPyr, astNextPyr, &stPrevPts, &stNextPts, &stStatus, &stErr);

	prevPts = (IVE_POINT_S25Q7_S *)stPrevPts.u64VirAddr;
	nextPts = (IVE_POINT_S25Q7_S *)stNextPts.u64VirAddr;
	statusTan  = (CVI_U8*)stStatus.u64VirAddr;
	//errTan     = (CVI_U9Q7 *)stErr.u64VirAddr;

	for(;;)
	{
		int frmCnt = (int)cap.get(CV_CAP_PROP_POS_FRAMES);

		Mat frame;
		cap >> frame;
		if( frame.empty() )
			break;

		frame.copyTo(dispImg);
		cvtColor(dispImg, gray, COLOR_BGR2GRAY);

		if( 0==frmCnt )
		{
			// automatic initialization
			goodFeaturesToTrack(gray, points[0], LK_OPTICAL_FLOW_MAX_POINT_NUM, 0.01, (double)10, Mat(), 3, (bool)0, 0.04);
			cornerSubPix(gray, points[0], subPixWinSize, Size(-1,-1), termcrit);

			for(size_t i=0; i<points[0].size(); i++)
			{
				prevPts[i].s25q7X = (CVI_S25Q7)(points[0][i].x * 128);
				prevPts[i].s25q7Y = (CVI_S25Q7)(points[0][i].y * 128);
			}

			if(prevGray.empty())
				gray.copyTo(prevGray);

			for( size_t i = 0; i < points[0].size(); i++)
			{
				Point2f ptf;
				ptf.x = prevPts[i].s25q7X/128.0f;
				ptf.y = prevPts[i].s25q7Y/128.0f;
				circle(dispImg, ptf, 3, Scalar(0,255,0), -1, 8);
			}
		}
		else
		{
			Mat2IveImage(&prevGray, &astPrevPyr[0]);
			Mat2IveImage(&gray, &astNextPyr[0]);
			BuildLKOpticalFlowPyr(stLkPyrCtrl.u8MaxLevel, astPrevPyr, astNextPyr);

			s32Ret = CVI_MPI_IVE_LKOpticalFlowPyr(&iveHandle, astPrevPyr, astNextPyr,
				&stPrevPts, &stNextPts, &stStatus, &stErr, &stLkPyrCtrl, bInstant);

			CVI_U16 i, k;
			for( i = k = 0; i < (CVI_U16)stLkPyrCtrl.u16PtsNum; i++)
			{
				Point2f ptf;
				if(!statusTan[i])
					continue;
				nextPts[k].s25q7X = nextPts[i].s25q7X;
				nextPts[k].s25q7Y = nextPts[i].s25q7Y;
				k++;

				ptf.x = nextPts[i].s25q7X/128.0f;
				ptf.y = nextPts[i].s25q7Y/128.0f;
				circle(dispImg, ptf, 3, Scalar(0,255,0), -1, 8);
			}
			stLkPyrCtrl.u16PtsNum = k;
			memcpy((CVI_U8*)stPrevPts.u64VirAddr, (CVI_U8*)stNextPts.u64VirAddr, stLkPyrCtrl.u16PtsNum * sizeof(IVE_POINT_S25Q7_S));
			CVI_U8 *pTem = (CVI_U8*)astPrevPyr[0].au64VirAddr[0];
			astPrevPyr[0].au64VirAddr[0] = astNextPyr[0].au64VirAddr[0];
			astPrevPyr[0].au64PhyAddr[0] = (CVI_U64)astPrevPyr[0].au64VirAddr[0];
			astNextPyr[0].au64VirAddr[0] = (CVI_U64)pTem;
			astNextPyr[0].au64PhyAddr[0] = (CVI_U64)astNextPyr[0].au64VirAddr[0];

			cv::swap(prevGray, gray);
		}

		if (compare && (frmCnt <= 20 || frmCnt == 200))
		{
			FILE *fp;
			CVI_CHAR fileName[_MAX_FNAME];
			IVE_MEM_INFO_S stPrevPtsRef;
			CVI_S32 cmp, open;

#if 0
			snprintf(fileName, _MAX_FNAME, "./sample_LKOpticalFlow_Status_%d.bin", frmCnt);
			open = fopen_s(&fp,fileName,"wb");
			if (open == CVI_SUCCESS)
			{
				CVI_MemWriteFile(&stStatus, fp);
				CVI_FCLOSE(fp);
			}

			snprintf(fileName, _MAX_FNAME, "./sample_LKOpticalFlow_Err_%d.bin", frmCnt);
			open = fopen_s(&fp,fileName,"wb");
			if (open == CVI_SUCCESS)
			{
				CVI_MemWriteFile(&stErr, fp);
				CVI_FCLOSE(fp);
			}
#endif
#if 0
			snprintf(fileName, _MAX_FNAME, "./sample_LKOpticalFlow_prevPts_%d.bin", frmCnt);
			open = fopen_s(&fp,fileName,"wb");
			if (open == CVI_SUCCESS)
			{
				CVI_MemWriteFile(&stPrevPts, fp);
				CVI_FCLOSE(fp);
			}
#endif

			snprintf(fileName, _MAX_FNAME, "../data/result/sample_LKOpticalFlow_prevPts_%d.bin", frmCnt);
			open = fopen_s(&fp,fileName,"rb");
			if (open == CVI_SUCCESS)
			{
				stPrevPtsRef.u32Size = stPrevPts.u32Size;
				stPrevPtsRef.u64VirAddr = (CVI_U64)malloc(stPrevPtsRef.u32Size);
				stPrevPtsRef.u64PhyAddr = (CVI_U64)stPrevPtsRef.u64VirAddr;
				CVI_MemReadFile(&stPrevPtsRef, fp);
				CVI_FCLOSE(fp);
				cmp = CVI_CompareIveMem(&stPrevPts, &stPrevPtsRef);
				if (cmp)
				{
					printf("Compare against %s failed\n", fileName);
					s32CompareError ++;
				}
				else
				{
					printf("Compare against %s passed\n", fileName);
				}
				CVI_FREE_64(stPrevPtsRef.u64VirAddr);
			}
			else
			{
				printf("%s not exist\n", fileName);
			}
		}

		if (show)
		{
			char text[_MAX_FNAME] = {'\0'};
			snprintf(text, sizeof(text), "FrmCnt: %d", frmCnt);
			putText(dispImg, string(text), Point(5,15), FONT_HERSHEY_COMPLEX_SMALL, 1, CV_RGB(255,0,0));
			imshow("LK_SAMPLE2", dispImg);
			waitKey(10);
			wte.write(dispImg);
		}
	}

	DeinitLKOpticalFlowPyr(stLkPyrCtrl.u8MaxLevel, &stMem, astPrevPyr, astNextPyr,
		&stPrevPts, &stNextPts, &stStatus, &stErr);

	destroyAllWindows();
	return (s32CompareError == 0) ? s32Ret : CVI_FAILURE;
}
