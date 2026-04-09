//#define useOpenCV (1)
#include "GMM2.h"
#include "sample_assist.h"
#include "sample_file.h"
#include "sample_define.h"
#include "mpi_ive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>    // std::max

using namespace std;
#ifdef useOpenCV
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;
#endif

enum
{
	CVGRAY_IVEU8C1 = 0,
	CVBGR_IVEU8C3PACKAGE = 1,

	IVEU8C1_CVGRAY = 3,
	IVEU8C3PACKAGE_CVBGR = 4
};

#ifdef useOpenCV
// only support CVBGR2IVEU8C3PACKAGE and CVGRAY2IVEU8C1
static void mat2IveImg(Mat *src, IVE_IMAGE_S *pstDst, int type)
{
	if(src->cols != (int)pstDst->u32Width || src->rows != (int)pstDst->u32Height)
	{
		printf("width or height didn't equal, line:%d\n",__LINE__);
	}

	switch(type)
	{
	case CVGRAY_IVEU8C1:
		{
			for(int r=0; r<src->rows; r++)
			{
				CVI_U8 *ptm = src->ptr(r);
				CVI_U8 *pti = (CVI_U8*)(pstDst->au64VirAddr[0] + r * pstDst->au32Stride[0]);

				memcpy(pti, ptm, pstDst->u32Width);
			}
			break;
		}
	case CVBGR_IVEU8C3PACKAGE:
		{
			for(int r=0; r<src->rows; r++)
			{
				CVI_U8 *ptm = src->ptr(r);
				CVI_U8 *pti = (CVI_U8*)(pstDst->au64VirAddr[0] + 3 * r * pstDst->au32Stride[0]);

				memcpy(pti, ptm, pstDst->u32Width * 3);
			}
			break;
		}
	default:
		printf("not support type for mat2IveImg!\n");
	}
}

// only support IVEU8C1_CVGRAY and IVEU8C3PACKAGE_CVBGR
static void iveImage2Mat(IVE_IMAGE_S *pstSrc, Mat *dst, int type)
{
	if(dst->cols != (int)pstSrc->u32Width || dst->rows != (int)pstSrc->u32Height)
	{
		printf("width or height didn't equal,line:%d\n",__LINE__);
		return;
	}

	switch(type)
	{
	case IVEU8C1_CVGRAY:
		{
			for(int r=0; r<dst->rows; r++)
			{
				CVI_U8 *ptm = dst->ptr(r);
				CVI_U8 *pti = (CVI_U8*)(pstSrc->au64VirAddr[0] + r * pstSrc->au32Stride[0]);

				memcpy(ptm, pti, dst->cols);
			}
			break;
		}
	case IVEU8C3PACKAGE_CVBGR:
		{
			for(int r=0; r<dst->rows; r++)
			{
				CVI_U8 *ptm = dst->ptr(r);
				CVI_U8 *pti = (CVI_U8*)(pstSrc->au64VirAddr[0] + 3 * r * pstSrc->au32Stride[0]);
				memcpy(ptm, pti, dst->cols * 3);
			}
			break;
		}
	default:
		printf("not support type for iveImage2Mat!\n");
	}
}

static void cvShowMultiImg(string title, int winsInCol, int winInRow, std::vector<Mat*> &imgVec, int gap=10,  Mat *dispImg=NULL)
{
	// the image in which input images are to be copied
	Mat showImg;

	int i, j, idx;
	int winW, winH;
	int realCellW, realCellH;
	int offX, offY;

	// If the number of arguments is lesser than 0 or greater than 12
	// return without displaying
	if(winInRow<1 || winsInCol<1)
	{
		printf("cellW(%d) and cellH(%d) can't be less than 1.\n", winInRow, winsInCol);
		return;
	}

	if((int)imgVec.size() > winsInCol *winInRow)
	{
		printf("There not enough window for showing imgVec, will be showing by default!\n");
	}

	winW = winH = 0;
	for(i=0; i<(int)imgVec.size(); i++)
	{
		Mat *img = imgVec[i];
		if(img->cols>winW)
			winW = img->cols;
		if(img->rows>winH)
			winH = img->rows;
	}

	realCellW = winInRow;
	realCellH = (CVI_S32)(imgVec.size() + realCellW - 1)/realCellW;

	showImg = cv::Mat::zeros(realCellH * winH + gap * realCellH + gap, realCellW * winW + gap * realCellW + gap, CV_8UC3);
	//showImg.create(realCellH * winH + gap * realCellH + gap, realCellW * winW + gap * realCellW + gap, CV_8UC3);
	//showImg.setTo(Scalar(255,255,255));

	offY = offX = idx = 0;
	for(i=0; i<realCellH; i++)
	{
		offY = gap + (winH + gap ) * i;
		for(j=0; j<realCellW; j++)
		{
			offX = gap + (winW + gap) * j;
			if(idx < (int)imgVec.size())
			{
				Mat *img = imgVec[idx];
				Mat rgb;
				Mat *roi = new Mat(showImg, Rect(offX, offY, winW, winH));

				if(img->type()==CV_8UC1)
				{
					cvtColor(*img, rgb, CV_GRAY2BGR);
					resize(rgb, *roi, roi->size());
				}
				else
				{
					resize(*img, *roi, roi->size());
				}
				delete roi;
			}
			idx++;
		}
	}

	imshow(title, showImg);

	if(dispImg)
	{
		dispImg->release();
		*dispImg = showImg.clone();
	}
}
#endif

CVI_S32 GMM2_Sample_U8C3_PACKAGE(int show, int compare)
{
	CVI_S32  s32Ret;
	CVI_S32 s32CompareError = 0;
	IVE_HANDLE hIveHandle;
	CVI_BOOL bInstant = CVI_TRUE;
	const CVI_CHAR *pchAvi = "../data/avi/campus.avi";
	const CVI_CHAR *pchRes = "../data/avi/GMM2_Sample_U8C3_PACKAGE.avi";

	IVE_SRC_IMAGE_S stIveImg;
	IVE_DST_IMAGE_S stIveFg;
	IVE_DST_IMAGE_S stIveBg;
	IVE_MEM_INFO_S stModel;
	IVE_GMM2_CTRL_S stGMM2Ctrl;
	IVE_SRC_IMAGE_S	stFactor;
	IVE_DST_IMAGE_S	stMatchModelInfo;
	CVI_U16 u16Width, u16Height;
	CVI_S32 s32FrmCnt = 0;
	CVI_U32 u32FrmNum;

    CVI_U32 u32FrameNumMax = -1;
    FILE *fpInput = NULL;
	IVE_DST_MEM_INFO_S stInput;
	CVI_CHAR resultImgName[_MAX_FNAME];
    stInput.u64VirAddr = CVI_NULL;
	u16Width  = 352;
	u16Height = 288;

    UNUSED(u32FrameNumMax);
    UNUSED(resultImgName);
    UNUSED(fpInput);
    UNUSED(pchAvi);
    UNUSED(pchRes);

#ifdef useOpenCV
	// for opencv
	Mat cvImg, cvFg;
	Mat cvFgBGR,cvBgBGR;
	Mat cvDispImg;
	VideoCapture cvCap;
	VideoWriter  cvWte;
	int cvWidth, cvHeight, cvGap;
	Size cvSz;

	// ori w & h
	cvCap.open(pchAvi);
	cvWidth  = (CVI_S32)cvCap.get(CV_CAP_PROP_FRAME_WIDTH);
	cvHeight = (CVI_S32)cvCap.get(CV_CAP_PROP_FRAME_HEIGHT);

	// even w & h
	u16Width  = cvWidth & (~1);
	u16Height = cvHeight & (~1);

	//for images
	cvFg    = Mat::zeros(u16Height, u16Width, CV_8UC1);
	cvFgBGR = Mat::zeros(u16Height, u16Width, CV_8UC3);
	cvBgBGR = Mat::zeros(u16Height, u16Width, CV_8UC3);
#endif

	//Initialization--------------------------------------------------------------------
	//set default parameters
	stGMM2Ctrl.u16VarRate			  =	1;
	stGMM2Ctrl.u8ModelNum			  =	1;
	stGMM2Ctrl.u9q7MaxVar			  =	(16 * 16)<<7;
	stGMM2Ctrl.u9q7MinVar			  =	( 8 *  8)<<7;
	stGMM2Ctrl.u8GlbSnsFactor		  =	8;
	stGMM2Ctrl.enSnsFactorMode		  =	IVE_GMM2_SNS_FACTOR_MODE_GLB;
	stGMM2Ctrl.u16FreqThr			  =	12000;
	stGMM2Ctrl.u16FreqInitVal		  =	20000;
	stGMM2Ctrl.u16FreqAddFactor		  =	0xEF;
	stGMM2Ctrl.u16FreqReduFactor	  =	0xFF00;
	stGMM2Ctrl.u16LifeThr			  =	5000;
	stGMM2Ctrl.enLifeUpdateFactorMode = IVE_GMM2_LIFE_UPDATE_FACTOR_MODE_GLB;

	//for input image
	memset(&stIveImg, 0, sizeof(IVE_SRC_IMAGE_S));
	memset(&stIveFg,  0, sizeof(IVE_DST_IMAGE_S));
	memset(&stIveBg,  0, sizeof(IVE_DST_IMAGE_S));
	memset(&stFactor, 0, sizeof(IVE_SRC_IMAGE_S));
	memset(&stMatchModelInfo, 0, sizeof(IVE_DST_IMAGE_S));
	memset(&stModel,0,sizeof(IVE_MEM_INFO_S));

	//for image
	s32Ret = CVI_CreateIveImage(&stIveImg,IVE_IMAGE_TYPE_U8C3_PACKAGE, u16Width, u16Height);
	CVI_CHECK_NET_GOTO_PRT(s32Ret,CVI_SUCCESS,FAILURE,"CVI_CreateIveImage Image fail\n");

	//for foreground
	s32Ret = CVI_CreateIveImage(&stIveFg,IVE_IMAGE_TYPE_U8C1,u16Width, u16Height);
	CVI_CHECK_NET_GOTO_PRT(s32Ret,CVI_SUCCESS,FAILURE,"CVI_CreateIveImage FG fail\n");

	//for background
	s32Ret = CVI_CreateIveImage(&stIveBg,IVE_IMAGE_TYPE_U8C3_PACKAGE, u16Width, u16Height);
	CVI_CHECK_NET_GOTO_PRT(s32Ret,CVI_SUCCESS,FAILURE,"CVI_CreateIveImage BG fail\n");

	//for sensitivity factor
	s32Ret = CVI_CreateIveImage(&stFactor,IVE_IMAGE_TYPE_U16C1, u16Width, u16Height);
	CVI_CHECK_NET_GOTO_PRT(s32Ret,CVI_SUCCESS,FAILURE,"CVI_CreateIveImage fail\n");

	//for matched model information
	s32Ret = CVI_CreateIveImage(&stMatchModelInfo,IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
	CVI_CHECK_NET_GOTO_PRT(s32Ret,CVI_SUCCESS,FAILURE,"CVI_CreateIveImage fail\n");

	//for models
	stModel.u32Size = stGMM2Ctrl.u8ModelNum * 12 * u16Width * u16Height;
	stModel.u64VirAddr = (CVI_U64)malloc(stModel.u32Size);
	CVI_CHECK_ET_GOTO_PRT(stModel.u64VirAddr,0,FAILURE,"malloc fail\n");
	memset((CVI_U8*)stModel.u64VirAddr,0,stModel.u32Size);
	stModel.u64PhyAddr = (CVI_U64)stModel.u64VirAddr;
#ifdef useOpenCV
	//for video writer
	cvGap = 10;
	cvSz.width  = stIveImg.u32Width  + stIveBg.u32Width  + stIveFg.u32Width  + 4 * cvGap;
	cvSz.height = max(max(stIveImg.u32Height, stIveBg.u32Height), stIveFg.u32Height) + 2 * cvGap;
	cvWte.open(pchRes, CV_FOURCC('X','V','I','D'), cvCap.get(CV_CAP_PROP_FPS), cvSz);
#else
    u32FrameNumMax = 2;
    stInput.u32Size = u16Height * u16Width * 3;
    stInput.u64VirAddr =  (CVI_U64)malloc(stInput.u32Size);
    stInput.u64PhyAddr = (CVI_U64)stInput.u64VirAddr;
#endif

	for(;;)
	{
#ifdef useOpenCV
		vector<Mat *> cvImgVec;
		s32FrmCnt = (CVI_S32)cvCap.get(CV_CAP_PROP_POS_FRAMES);

		cvCap >> cvImg;
		if( cvImg.empty())
			break;

		if(cvImg.rows != (int)stIveImg.u32Height || cvImg.cols != (int)stIveImg.u32Width)
		{
			printf("Size not match, quit\n");
			break;
		}

		if (s32FrmCnt == 0)
		{
			imwrite("./sample_GMM2_frm_U8C3_0.jpg", cvImg);
		}
		if (s32FrmCnt == 1)
		{
			imwrite("./sample_GMM2_frm__U8C3_1.jpg", cvImg);
		}
		mat2IveImg(&cvImg, &stIveImg, CVBGR_IVEU8C3PACKAGE);
#else
        if ((CVI_U32)s32FrmCnt > u32FrameNumMax) {
            break;
        }

        snprintf(resultImgName, _MAX_FNAME,
                "../data/avi/campus.u8c3.%d.raw", s32FrmCnt);
		s32Ret = fopen_s(&fpInput, resultImgName,"rb");
		CVI_CHECK_NET_GOTO(s32Ret,0,FAILURE);
		s32Ret = CVI_MemReadFile(&stInput,fpInput);
		CVI_CHECK_NET_GOTO(s32Ret, 0, FAILURE);
		CVI_FCLOSE(fpInput);

        for(int r=0; r<u16Height; r++)
        {
            CVI_U8 *ptm = (CVI_U8*)(stInput.u64VirAddr + 3 * r * stIveImg.au32Stride[0]);
            CVI_U8 *pti = (CVI_U8*)(stIveImg.au64VirAddr[0] + 3 * r * stIveImg.au32Stride[0]);


            memcpy(pti, ptm, 3 * u16Width);
        }
#endif
		u32FrmNum = s32FrmCnt + 1;

		//To building a stable background model quickly at the begin, some parameters are set specially.
		if (1 == stGMM2Ctrl.u8ModelNum)
		{
			//If the parameter u8ModelNum is set to 1, the parameter u16FreqReduFactor
			//is usually set to a small value at the first N frames. Here, N = 500.
			stGMM2Ctrl.u16FreqReduFactor = (u32FrmNum >= 500) ? 0xFFA0 : 0xFC00;
		}
		else
		{
			//If the parameter u8ModelNum is more than 1, the global life mode should be used at the first N frames,
			//and the parameter u16GlbLifeUpdateFactor is usually set to a big value. Here, N = 500.
			stGMM2Ctrl.u16GlbLifeUpdateFactor = (u32FrmNum >= 500) ? 4 : 0xFFFF / u32FrmNum;
		}

		s32Ret = CVI_MPI_IVE_GMM2(&hIveHandle, &stIveImg, &stFactor, &stIveFg, &stIveBg,
			&stMatchModelInfo, &stModel, &stGMM2Ctrl, bInstant);
		CVI_CHECK_NET_GOTO_PRT(s32Ret,CVI_SUCCESS,FAILURE,"CVI_MPI_IVE_GMM2 fail,Error(%#x)\n",s32Ret);


		if (compare && (s32FrmCnt == 0 || s32FrmCnt == 1 || s32FrmCnt == 512))
		{
			FILE *fp;
			CVI_CHAR fileName[_MAX_FNAME];
			IVE_IMAGE_S stIveBgRef;
			IVE_IMAGE_S stIveFgRef;
			CVI_S32 cmp, open;

			snprintf(fileName, _MAX_FNAME, "../data/result/sample_GMM2_U8C3_PACKAGE_bg_%d.rgb", s32FrmCnt);
			open = fopen_s(&fp,fileName,"rb");
			if (open == CVI_SUCCESS)
			{
				CVI_CreateIveImage(&stIveBgRef, stIveBg.enType, u16Width, u16Height);
				CVI_ReadFile(&stIveBgRef, fp);
				CVI_FCLOSE(fp);
				cmp = CVI_CompareIveImage(&stIveBg, &stIveBgRef);
				if (cmp)
				{
					printf("Compare against %s failed\n", fileName);
					s32CompareError ++;
				}
				else
				{
					printf("Compare against %s passed\n", fileName);
				}
				CVI_DestroyIveImage(&stIveBgRef);
			}
			else
			{
				printf("%s not exist\n", fileName);
			}

			snprintf(fileName, _MAX_FNAME, "../data/result/sample_GMM2_U8C3_PACKAGE_fg_%d.yuv", s32FrmCnt);
			open = fopen_s(&fp,fileName,"rb");
			if (open == CVI_SUCCESS)
			{
				CVI_CreateIveImage(&stIveFgRef, stIveFg.enType, u16Width, u16Height);
				CVI_ReadFile(&stIveFgRef, fp);
				CVI_FCLOSE(fp);
				cmp = CVI_CompareIveImage(&stIveFg, &stIveFgRef);
				if (cmp)
				{
					printf("Compare against %s failed\n", fileName);
					s32CompareError ++;
				}
				else
				{
					printf("Compare against %s passed\n", fileName);
				}
				CVI_DestroyIveImage(&stIveFgRef);
			}
			else
			{
				printf("%s not exist\n", fileName);
			}
		}

		if (show)
		{
#ifdef useOpenCV
			iveImage2Mat(&stIveBg, &cvBgBGR, IVEU8C3PACKAGE_CVBGR);
			iveImage2Mat(&stIveFg, &cvFg, IVEU8C1_CVGRAY);
			cvtColor(cvFg, cvFgBGR, CV_GRAY2BGR);

			putText(cvImg, "srcImg", Point(5,15), FONT_HERSHEY_COMPLEX_SMALL, 1, CV_RGB(255,0,0));
			cvImgVec.push_back(&cvImg);

			putText(cvBgBGR, "bgImg", Point(5,15), FONT_HERSHEY_COMPLEX_SMALL, 1, CV_RGB(255,0,0));
			cvImgVec.push_back(&cvBgBGR);

			putText(cvFgBGR, "fgImg", Point(5,15), FONT_HERSHEY_COMPLEX_SMALL, 1, CV_RGB(255,0,0));
			cvImgVec.push_back(&cvFgBGR);

			cvShowMultiImg("GMM_U8C3_PACKAGE", 1, 3, cvImgVec, cvGap, &cvDispImg);
			waitKey(10);

			cvWte.write(cvDispImg);
#endif
		}
        s32FrmCnt++;
	}

FAILURE:
    if (stInput.u64VirAddr != CVI_NULL) {
        free ((CVI_U8*)stInput.u64VirAddr);
    }

	CVI_DestroyIveImage(&stIveImg);
	CVI_DestroyIveImage(&stIveFg);
	CVI_DestroyIveImage(&stIveBg);
	CVI_DestroyIveImage(&stFactor);
	CVI_DestroyIveImage(&stMatchModelInfo);
	CVI_FREE_64(stModel.u64VirAddr);

#ifdef useOpenCV
	if (show)
		cvDestroyAllWindows();
#endif
	return (s32CompareError == 0) ? s32Ret : CVI_FAILURE;
}

CVI_S32 GMM2_Sample_U8C1(int show, int compare)
{
	CVI_S32  s32Ret;
	CVI_S32 s32CompareError = 0;
	IVE_HANDLE hIveHandle;
	CVI_BOOL bInstant = CVI_TRUE;
	const CVI_CHAR *pchAvi = "../data/avi/campus.avi";
	const CVI_CHAR *pchRes = "../data/avi/GMM2_Sample_U8C1.avi";

	IVE_SRC_IMAGE_S stIveImg;
	IVE_DST_IMAGE_S stIveFg;
	IVE_DST_IMAGE_S stIveBg;
	IVE_MEM_INFO_S  stModel;
	IVE_GMM2_CTRL_S stGmm2Ctrl;
	IVE_SRC_IMAGE_S	stFactor;
	IVE_DST_IMAGE_S	stMatchModelInfo;
	CVI_U16 u16Width, u16Height;
	CVI_S32 s32FrmCnt = 0;
	CVI_U32 u32FrmNum;

    CVI_U32 u32FrameNumMax = -1;
    FILE *fpInput = NULL;
	IVE_DST_MEM_INFO_S stInput;
	CVI_CHAR resultImgName[_MAX_FNAME];
    stInput.u64VirAddr = CVI_NULL;
	u16Width  = 352;
	u16Height = 288;

    UNUSED(u32FrameNumMax);
    UNUSED(resultImgName);
    UNUSED(fpInput);
    UNUSED(pchAvi);
    UNUSED(pchRes);

#ifdef useOpenCV
	// for opencv
	Mat cvImg, cvFg, cvBg;
	Mat cvImgGray, cvFgBGR, cvBgBGR, cvDispImg;
	VideoCapture cvCap;
	VideoWriter  cvWte;
	int cvWidth, cvHeight, cvGap;
	Size cvSz;

	// ori w & h
	cvCap.open(pchAvi);
	cvWidth  = (CVI_S32)cvCap.get(CV_CAP_PROP_FRAME_WIDTH);
	cvHeight = (CVI_S32)cvCap.get(CV_CAP_PROP_FRAME_HEIGHT);

	// even w & h
	u16Width  = cvWidth & (~1);
	u16Height = cvHeight & (~1);

	cvFg    = Mat::zeros(u16Height, u16Width, CV_8UC1);
	cvFgBGR = Mat::zeros(u16Height, u16Width, CV_8UC3);
	cvBg    = Mat::zeros(u16Height, u16Width, CV_8UC1);
	cvBgBGR = Mat::zeros(u16Height, u16Width, CV_8UC3);
#endif

	//Initialization--------------------------------------------------------------------
	//set default parameters
	stGmm2Ctrl.u16VarRate			  =	1;
	stGmm2Ctrl.u8ModelNum			  =	3;
	stGmm2Ctrl.u9q7MaxVar			  =	(16 * 16)<<7;
	stGmm2Ctrl.u9q7MinVar			  =	( 8 *  8)<<7;
	stGmm2Ctrl.u8GlbSnsFactor		  =	8;
	stGmm2Ctrl.enSnsFactorMode		  =	IVE_GMM2_SNS_FACTOR_MODE_GLB;
	stGmm2Ctrl.u16FreqThr			  =	12000;
	stGmm2Ctrl.u16FreqInitVal		  =	20000;
	stGmm2Ctrl.u16FreqAddFactor		  =	0xEF;
	stGmm2Ctrl.u16FreqReduFactor	  =	0xFF00;
	stGmm2Ctrl.u16LifeThr			  =	5000;
	stGmm2Ctrl.enLifeUpdateFactorMode = IVE_GMM2_LIFE_UPDATE_FACTOR_MODE_GLB;

	//for input image
	memset(&stIveImg, 0, sizeof(IVE_SRC_IMAGE_S));
	memset(&stIveFg,  0, sizeof(IVE_DST_IMAGE_S));
	memset(&stIveBg,  0, sizeof(IVE_DST_IMAGE_S));
	memset(&stFactor, 0, sizeof(IVE_SRC_IMAGE_S));
	memset(&stMatchModelInfo, 0, sizeof(IVE_DST_IMAGE_S));
	memset(&stModel,0,sizeof(IVE_MEM_INFO_S));

	//for src
	s32Ret = CVI_CreateIveImage(&stIveImg,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_NET_GOTO_PRT(s32Ret,CVI_SUCCESS,FAILURE,"CVI_CreateIveImage fail\n");

	//for foreground
	s32Ret = CVI_CreateIveImage(&stIveFg,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_NET_GOTO_PRT(s32Ret,CVI_SUCCESS,FAILURE,"CVI_CreateIveImage fail\n");

	//for background
	s32Ret = CVI_CreateIveImage(&stIveBg,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_NET_GOTO_PRT(s32Ret,CVI_SUCCESS,FAILURE,"CVI_CreateIveImage fail\n");

	//for sensitivity factor
	s32Ret = CVI_CreateIveImage(&stFactor,IVE_IMAGE_TYPE_U16C1,u16Width,u16Height);
	CVI_CHECK_NET_GOTO_PRT(s32Ret,CVI_SUCCESS,FAILURE,"CVI_CreateIveImage fail\n");

	//for matched model information
	s32Ret = CVI_CreateIveImage(&stMatchModelInfo,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_NET_GOTO_PRT(s32Ret,CVI_SUCCESS,FAILURE,"CVI_CreateIveImage fail\n");

	//for models
	stModel.u32Size = stGmm2Ctrl.u8ModelNum * 8 * u16Width * u16Height;
	stModel.u64VirAddr = (CVI_U64)malloc(stModel.u32Size);
	CVI_CHECK_ET_GOTO_PRT(stModel.u64VirAddr,0,FAILURE,"malloc fail\n");
	memset((CVI_U8*)stModel.u64VirAddr,0,stModel.u32Size);
	stModel.u64PhyAddr = (CVI_U64)stModel.u64VirAddr;

#ifdef useOpenCV
	cvGap = 10;
	cvSz.width  = stIveImg.u32Width  + stIveBg.u32Width  + stIveFg.u32Width  + 4 * cvGap;
	cvSz.height = max(max(stIveImg.u32Height, stIveBg.u32Height), stIveFg.u32Height) + 2 * cvGap;
	cvWte.open(pchRes, CV_FOURCC('X','V','I','D'), cvCap.get(CV_CAP_PROP_FPS), cvSz);
#else
    u32FrameNumMax = 2;
    stInput.u32Size = u16Height * u16Width;
    stInput.u64VirAddr =  (CVI_U64)malloc(stInput.u32Size);
    stInput.u64PhyAddr = (CVI_U64)stInput.u64VirAddr;
#endif

	for(;;)
	{
#ifdef useOpenCV
		vector<Mat *> cvImgVec;
		s32FrmCnt = (CVI_S32)cvCap.get(CV_CAP_PROP_POS_FRAMES);

		cvCap >> cvImg;
		if( cvImg.empty())
			break;

		if(cvImg.rows != (int)stIveImg.u32Height || cvImg.cols != (int)stIveImg.u32Width)
		{
			printf("Size not match, quit\n");
			break;
		}

		cvtColor(cvImg, cvImgGray, CV_BGR2GRAY);
		if (s32FrmCnt == 0)
		{
			imwrite("./sample_GMM2_frm_0.jpg", cvImg);
			imwrite("./sample_GMM2_frmGray_0.jpg", cvImgGray);
		}
		if (s32FrmCnt == 1)
		{
			imwrite("./sample_GMM2_frm_1.jpg", cvImg);
			imwrite("./sample_GMM2_frmGray_1.jpg", cvImgGray);
		}
		mat2IveImg(&cvImgGray, &stIveImg, CVGRAY_IVEU8C1);
#else
        if ((CVI_U32)s32FrmCnt > u32FrameNumMax) {
            break;
        }

        snprintf(resultImgName, _MAX_FNAME,
                "../data/avi/campus.u8c1.%d.raw", s32FrmCnt);
		s32Ret = fopen_s(&fpInput, resultImgName,"rb");
		CVI_CHECK_NET_GOTO(s32Ret,0,FAILURE);
		s32Ret = CVI_MemReadFile(&stInput,fpInput);
		CVI_CHECK_NET_GOTO(s32Ret, 0, FAILURE);
		CVI_FCLOSE(fpInput);

        for(int r=0; r<u16Height; r++)
        {
            CVI_U8 *ptm = (CVI_U8*)(stInput.u64VirAddr + r * stIveImg.au32Stride[0]);
            CVI_U8 *pti = (CVI_U8*)(stIveImg.au64VirAddr[0] + r * stIveImg.au32Stride[0]);


            memcpy(pti, ptm, u16Width);
        }
#endif
		u32FrmNum = s32FrmCnt + 1;

		//To building a stable background model quickly at the begin, some parameters are set specially.
		if (1 == stGmm2Ctrl.u8ModelNum)
		{
			//If the parameter u8ModelNum is set to 1, the parameter u16FreqReduFactor
			//is usually set to a small value at the first N frames. Here, N = 500.
			stGmm2Ctrl.u16FreqReduFactor = (u32FrmNum >= 500) ? 0xFFA0 : 0xFC00;
		}
		else
		{
			//If the parameter u8ModelNum is more than 1, the global life mode should be used at the first N frames,
			//and the parameter u16GlbLifeUpdateFactor is usually set to a big value. Here, N = 500.
			stGmm2Ctrl.u16GlbLifeUpdateFactor = (u32FrmNum >= 500) ? 4 : 0xFFFF / u32FrmNum;
		}
		s32Ret = CVI_MPI_IVE_GMM2(&hIveHandle,&stIveImg,&stFactor, &stIveFg,&stIveBg, &stMatchModelInfo,&stModel,&stGmm2Ctrl,bInstant);
		CVI_CHECK_NET_GOTO_PRT(s32Ret,CVI_SUCCESS,FAILURE,"CVI_MPI_IVE_GMM2 fail,Error(%#x)\n",s32Ret);

		if (compare && (s32FrmCnt == 0 || s32FrmCnt == 1 || s32FrmCnt == 512))
		{
			FILE *fp;
			CVI_CHAR fileName[_MAX_FNAME];
			IVE_IMAGE_S stIveBgRef;
			IVE_IMAGE_S stIveFgRef;
			CVI_S32 cmp, open;

			snprintf(fileName, _MAX_FNAME, "../data/result/sample_GMM2_U8C1_bg_%d.yuv", s32FrmCnt);
			open = fopen_s(&fp,fileName,"rb");
			if (open == CVI_SUCCESS)
			{
				CVI_CreateIveImage(&stIveBgRef, stIveBg.enType, u16Width, u16Height);
				CVI_ReadFile(&stIveBgRef, fp);
				CVI_FCLOSE(fp);
				cmp = CVI_CompareIveImage(&stIveBg, &stIveBgRef);
				if (cmp)
				{
					printf("Compare against %s failed\n", fileName);
					s32CompareError ++;
				}
				else
				{
					printf("Compare against %s passed\n", fileName);
				}
				CVI_DestroyIveImage(&stIveBgRef);
			}
			else
			{
				printf("%s not exist\n", fileName);
			}

			snprintf(fileName, _MAX_FNAME, "../data/result/sample_GMM2_U8C1_fg_%d.yuv", s32FrmCnt);
			open = fopen_s(&fp,fileName,"rb");
			if (open == CVI_SUCCESS)
			{
				CVI_CreateIveImage(&stIveFgRef, stIveFg.enType, u16Width, u16Height);
				CVI_ReadFile(&stIveFgRef, fp);
				CVI_FCLOSE(fp);
				cmp = CVI_CompareIveImage(&stIveFg, &stIveFgRef);
				if (cmp)
				{
					printf("Compare against %s failed\n", fileName);
					s32CompareError ++;
				}
				else
				{
					printf("Compare against %s passed\n", fileName);
				}
				CVI_DestroyIveImage(&stIveFgRef);
			}
			else
			{
				printf("%s not exist\n", fileName);
			}
		}

		if (show)
		{
#ifdef useOpenCV
			iveImage2Mat(&stIveBg, &cvBg, IVEU8C1_CVGRAY);
			iveImage2Mat(&stIveFg, &cvFg, IVEU8C1_CVGRAY);
			cvtColor(cvBg, cvBgBGR, CV_GRAY2BGR);
			cvtColor(cvFg, cvFgBGR, CV_GRAY2BGR);

			putText(cvImg, "srcImg", Point(5,15), FONT_HERSHEY_COMPLEX_SMALL, 1, CV_RGB(255,0,0));
			cvImgVec.push_back(&cvImg);

			putText(cvBgBGR, "bgImg", Point(5,15), FONT_HERSHEY_COMPLEX_SMALL, 1, CV_RGB(255,0,0));
			cvImgVec.push_back(&cvBgBGR);

			putText(cvFgBGR, "fgImg", Point(5,15), FONT_HERSHEY_COMPLEX_SMALL, 1, CV_RGB(255,0,0));
			cvImgVec.push_back(&cvFgBGR);

			cvShowMultiImg("GMM_U8C1", 1, 3, cvImgVec, cvGap, &cvDispImg);
			waitKey(10);

			cvWte.write(cvDispImg);
#endif
		}
        s32FrmCnt++;
	}

FAILURE:
    if (stInput.u64VirAddr != CVI_NULL) {
        free ((CVI_U8*)stInput.u64VirAddr);
    }

	CVI_DestroyIveImage(&stIveImg);
	CVI_DestroyIveImage(&stIveFg);
	CVI_DestroyIveImage(&stIveBg);
	CVI_DestroyIveImage(&stFactor);
	CVI_DestroyIveImage(&stMatchModelInfo);
	CVI_FREE_64(stModel.u64VirAddr);

#ifdef useOpenCV
	if (show)
		cvDestroyAllWindows();
#endif
	return (s32CompareError == 0) ? s32Ret : CVI_FAILURE;
}

static CVI_S32 GMM2_Sample_GenFgMask(IVE_SRC_IMAGE_S *pstFg,IVE_DST_IMAGE_S *pstFgMask)
{
	CVI_S32 s32Ret;
	IVE_HANDLE IveHandle;
	IVE_THRESH_CTRL_S stCtrl;

	stCtrl.enMode	= IVE_THRESH_MODE_BINARY;
	stCtrl.u8MinVal = 0;
	stCtrl.u8MaxVal = 1;
	stCtrl.u8LowThr = 5;
	s32Ret = CVI_MPI_IVE_Thresh(&IveHandle, pstFg, pstFgMask, &stCtrl, CVI_FALSE);
	CVI_CHECK_NET_RET_PRT(s32Ret,CVI_SUCCESS,s32Ret,"CVI_MPI_IVE_Thresh fail,Error(%#x)\n",s32Ret);

	return s32Ret;
}
/*
*Reduce the factor gradually to the default value
*/
static CVI_VOID GMM2_Sample_ReduFactor(IVE_IMAGE_S *pstFactor)
{
	CVI_U16 i,j;
	CVI_U8 *pu8Tmp;

	pu8Tmp = (CVI_U8*)pstFactor->au64VirAddr[0];
	for (i = 0; i < pstFactor->u32Height; i++)
	{
		for (j = 0; j < pstFactor->u32Width; j++)
		{
			pu8Tmp[j<<1]		= std::max(8, pu8Tmp[i<<1] - 1);
			pu8Tmp[(i<<1) + 1]	= std::max(8, pu8Tmp[(i<<1) + 1] - 2);
		}
		pu8Tmp += pstFactor->au32Stride[0] * sizeof(CVI_U16);
	}
}
/*
*Change factor by difference frame
*/
static CVI_BOOL GMM2_Sample_ChgFactorByDiffFrm(IVE_SRC_IMAGE_S *pstSrc,IVE_SRC_IMAGE_S *pstLastImg,
	IVE_DST_IMAGE_S *pstDiffImg,IVE_DST_IMAGE_S *pstFactor)
{
	CVI_S32 s32Ret;
	IVE_HANDLE IveHandle;
	CVI_U16 i,j;
	CVI_S32 s32AbsTh = 10;
	CVI_U32 u32PointSum = 0;
	IVE_SUB_CTRL_S stSubCtrl;
	CVI_U8 *pu8Tmp,*pu8Factor;

	stSubCtrl.enMode = IVE_SUB_MODE_ABS;
	s32Ret = CVI_MPI_IVE_Sub(&IveHandle, pstSrc, pstLastImg, pstDiffImg, &stSubCtrl, CVI_TRUE);
	CVI_CHECK_NET_RET_PRT(s32Ret,CVI_SUCCESS,CVI_FALSE,"CVI_MPI_IVE_Sub fail,Error(%#x)\n",s32Ret);

	pu8Tmp = (CVI_U8*)pstDiffImg->au64VirAddr[0];
	for (i = 0; i < pstSrc->u32Height; i++)
	{
		for (j = 0; j < pstSrc->u32Width; j++)
		{
			u32PointSum += pu8Tmp[j] > s32AbsTh;
		}
		pu8Tmp += pstDiffImg->au32Stride[0];
	}

	if ((u32PointSum * 3) > (CVI_U16)(pstSrc->u32Width * pstSrc->u32Height* 2))
	{
		pu8Tmp   = (CVI_U8*)pstDiffImg->au64VirAddr[0];
		pu8Factor = (CVI_U8*)pstFactor->au64VirAddr[0];
		for (i = 0; i < pstSrc->u32Height; i++)
		{
			for (j = 0; j < pstSrc->u32Width; j++)
			{
				if (pu8Tmp[j] > s32AbsTh)
				{
					pu8Factor[j<<1] 	 	= 20;
					pu8Factor[(j<<1) + 1] 	= 200;

				}
			}
			pu8Tmp 		+= pstDiffImg->au32Stride[0];
			pu8Factor 	+= pstFactor->au32Stride[0] * sizeof(CVI_U16);
		}
		return CVI_TRUE;
	}
	return CVI_FALSE;
}

/*
*Change factor by gradient
*/
static CVI_VOID GMM2_Sample_ChgFactorByGrad(IVE_SRC_IMAGE_S *pstFgMask,
	IVE_SRC_IMAGE_S *pstSrc,IVE_SRC_IMAGE_S *pstLastImg,IVE_SRC_IMAGE_S *pstLastNormMag,
	IVE_DST_IMAGE_S *pstMagImg,IVE_DST_IMAGE_S *pstCurNormMag,IVE_DST_MEM_INFO_S *pstBlob,
	IVE_DST_IMAGE_S *pstDiffImg,IVE_DST_IMAGE_S *pstFactor)
{
	CVI_S32 s32Ret;
	IVE_HANDLE IveHandle;
	CVI_U16 i,j,k;
	CVI_U16 u16Top;
	CVI_U16 u16Left;
	CVI_U16 u16Right;
	CVI_U16 u16Bottom;
	CVI_U8 *pu8FgCur;
	CVI_U8 *pu8GradDiff;
	CVI_U8 *pu8Factor;
	CVI_S32 s32FgSum    = 0;
	CVI_S32 s32GradSum  = 0;

	CVI_S8 au8Mask[25] = {0, 0,0,0,0,
		0,-1,0,1,0,
		0,-2,0,2,0,
		0,-1,0,1,0,
		0, 0,0,0,0};
	IVE_CCL_CTRL_S stCclCtrl;
	IVE_CCBLOB_S   *pstCclBlob;
	IVE_MAG_AND_ANG_CTRL_S   stMagAndAngCtrl;
	IVE_16BIT_TO_8BIT_CTRL_S st16To8Ctrl;
	IVE_SUB_CTRL_S stSubCtrl;

	stCclCtrl.enMode   = IVE_CCL_MODE_8C;
	stCclCtrl.u16Step  = 2;
	stCclCtrl.u16InitAreaThr = 4;
	s32Ret = CVI_MPI_IVE_CCL(&IveHandle, pstFgMask, pstBlob, &stCclCtrl, CVI_TRUE);
	CVI_CHECK_NET_RET_VOID_PRT(s32Ret,CVI_SUCCESS,"CVI_MPI_IVE_CCL fail,Error(%#x)\n",s32Ret);

	pstCclBlob = (IVE_CCBLOB_S *)pstBlob->u64VirAddr;
	if (pstCclBlob->u8RegionNum > 0)
	{
		//Calc the gradient difference of the current image and the last image
		memcpy(stMagAndAngCtrl.as8Mask, au8Mask, 25);
		stMagAndAngCtrl.u16Thr		 = 0;
		stMagAndAngCtrl.enOutCtrl	 = IVE_MAG_AND_ANG_OUT_CTRL_MAG;
		s32Ret = CVI_MPI_IVE_MagAndAng(&IveHandle, pstSrc, pstMagImg, CVI_NULL, &stMagAndAngCtrl, CVI_FALSE);
		CVI_CHECK_NET_RET_VOID_PRT(s32Ret,CVI_SUCCESS,"CVI_MPI_IVE_MagAndAng fail,Error(%#x)\n",s32Ret);

		st16To8Ctrl.s8Bias		 	= 0;
		st16To8Ctrl.enMode		 	= IVE_16BIT_TO_8BIT_MODE_U16_TO_U8;
		st16To8Ctrl.u8Numerator	 	= 255;
		st16To8Ctrl.u16Denominator 	= 255 * 4;
		s32Ret = CVI_MPI_IVE_16BitTo8Bit(&IveHandle, pstMagImg, pstCurNormMag, &st16To8Ctrl, CVI_FALSE);
		CVI_CHECK_NET_RET_VOID_PRT(s32Ret,CVI_SUCCESS,"CVI_MPI_IVE_16BitTo8Bit fail,Error(%#x)\n",s32Ret);

		s32Ret = CVI_MPI_IVE_MagAndAng(&IveHandle, pstLastImg, pstMagImg, CVI_NULL, &stMagAndAngCtrl, CVI_FALSE);
		CVI_CHECK_NET_RET_VOID_PRT(s32Ret,CVI_SUCCESS,"CVI_MPI_IVE_MagAndAng fail,Error(%#x)\n",s32Ret);

		s32Ret = CVI_MPI_IVE_16BitTo8Bit(&IveHandle, pstMagImg, pstLastNormMag, &st16To8Ctrl, CVI_FALSE);
		CVI_CHECK_NET_RET_VOID_PRT(s32Ret,CVI_SUCCESS,"CVI_MPI_IVE_16BitTo8Bit fail,Error(%#x)\n",s32Ret);

		stSubCtrl.enMode = IVE_SUB_MODE_ABS;
		s32Ret = CVI_MPI_IVE_Sub(&IveHandle, pstCurNormMag, pstLastNormMag, pstDiffImg, &stSubCtrl, CVI_TRUE);
		CVI_CHECK_NET_RET_VOID_PRT(s32Ret,CVI_SUCCESS,"CVI_MPI_IVE_Sub fail,Error(%#x)\n",s32Ret);
	}

	//for each blob, analyze the gradient change
	for(k = 0; k < IVE_MAX_REGION_NUM; k++)
	{
		if(0 == pstCclBlob->astRegion[k].u32Area)
		{
			continue;
		}

		u16Top    =  pstCclBlob->astRegion[k].u16Top;
		u16Left   =  pstCclBlob->astRegion[k].u16Left;
		u16Right  =  pstCclBlob->astRegion[k].u16Right;
		u16Bottom =  pstCclBlob->astRegion[k].u16Bottom;
		if((u16Right - u16Left) * (u16Bottom - u16Top) < 60)
		{
			continue;
		}

		s32FgSum    = 0;
		s32GradSum  = 0;
		pu8FgCur    = (CVI_U8*)(pstFgMask->au64VirAddr[0] + u16Top * pstFgMask->au32Stride[0]);
		pu8GradDiff = (CVI_U8*)(pstDiffImg->au64VirAddr[0] + u16Top * pstDiffImg->au32Stride[0]);
		for (i = u16Top; i < u16Bottom; i++)
		{
			for (j = u16Left; j < u16Right; j++)
			{
				if (pu8FgCur[j])
				{
					s32FgSum++;
					if (pu8GradDiff[j])
					{
						s32GradSum++;
					}
				}
			}

			pu8FgCur    += pstFgMask->au32Stride[0];
			pu8GradDiff += pstDiffImg->au32Stride[0];
		}

		if ((s32GradSum < 10) || (s32GradSum * 100 < s32FgSum))
		{
			pu8Factor = (CVI_U8*)(pstFactor->au64VirAddr[0] + 2 * u16Top * pstFactor->au32Stride[0]);
			pu8FgCur = (CVI_U8*)(pstFgMask->au64VirAddr[0] + u16Top * pstFgMask->au32Stride[0]);
			for (i = u16Top; i < u16Bottom; i++)
			{
				for (j = u16Left; j < u16Right; j++)
				{
					if (pu8FgCur[j])
					{
						pu8Factor[j<<1] = 16;
						pu8Factor[(j<<1) + 1] = 200;
					}
				}
				pu8FgCur     += pstFgMask->au32Stride[0];
				pu8Factor    += pstFactor->au32Stride[0] * sizeof(CVI_U16);
			}
		}
	}

}
/*
*Adjustment factor
*/
static CVI_VOID GMM2_Sample_AdjustmentFactor(IVE_SRC_IMAGE_S *pstFactor,
	IVE_SRC_IMAGE_S *pstFgMask, IVE_SRC_IMAGE_S *pstSrc,
	IVE_SRC_IMAGE_S *pstLastImg, IVE_DST_IMAGE_S *pstDiffImg,
	IVE_DST_IMAGE_S *pstMagImg, IVE_DST_IMAGE_S *pstCurNormMag,
	IVE_DST_IMAGE_S *pstLastNormMag, IVE_DST_MEM_INFO_S *pstBlob)
{

	CVI_BOOL bChg;
	//First, reduce the factor gradually to the default value------------------------------
	GMM2_Sample_ReduFactor(pstFactor);

	//Second, analyze the frame difference-------------------------------------------------
	//When the number of changed pixels is more than the threshold, there maybe a light switch.
	//And then, we should set a big factor to adapt to it quickly.
	bChg = GMM2_Sample_ChgFactorByDiffFrm(pstSrc,pstLastImg,pstDiffImg,pstFactor);
	if (CVI_TRUE == bChg)
	{
		return;
	}

	//Third, analyze the gradient for foreground blobs------------------------------------
	//When gradient change of a foreground blob is very small, it maybe a local illumination change,
	//a ghost, or a static object.
	//Here we try to reduce the influence by a local illumination change or a ghost only.
	GMM2_Sample_ChgFactorByGrad(pstFgMask,pstSrc,pstLastImg,pstLastNormMag,
		pstMagImg,pstCurNormMag,pstBlob,
		pstDiffImg,	pstFactor);
}

CVI_S32 GMM2_Sample_U8C1_PixelCtrl(int show, int compare)
{
	CVI_S32  s32Ret;
	CVI_S32 s32CompareError = 0;
	IVE_HANDLE hIveHandle;
	CVI_BOOL bInstant = CVI_TRUE;
	//const CVI_CHAR *pchAvi = "../data/avi/GMMtest1.avi";
	const CVI_CHAR *pchAvi = "../data/avi/campus.avi";
	const CVI_CHAR *pchRes = "../data/avi/GMM2_Sample_U8C1_PixelCtrl.avi";

	IVE_SRC_IMAGE_S stIveImg;
	IVE_DST_IMAGE_S stIveFg;
	IVE_DST_IMAGE_S stIveBg;
	IVE_MEM_INFO_S  stModel;
	IVE_GMM2_CTRL_S stGmm2Ctrl;
	IVE_SRC_IMAGE_S	stFactor;
	IVE_DST_IMAGE_S	stMatchModelInfo;
	IVE_SRC_IMAGE_S stFgMask;
	IVE_SRC_IMAGE_S stLastImg;
	IVE_DST_IMAGE_S stDiffImg;
	IVE_DST_IMAGE_S stMagImg;
	IVE_DST_IMAGE_S stCurNormMag;
	IVE_DST_IMAGE_S stLastNormMag;
	IVE_DST_MEM_INFO_S stBlob;
	CVI_U16 u16Width, u16Height;
	CVI_S32 s32FrmCnt = 0;
	CVI_U32 u32FrmNum;

    CVI_U32 u32FrameNumMax = -1;
    FILE *fpInput = NULL;
	IVE_DST_MEM_INFO_S stInput;
	CVI_CHAR resultImgName[_MAX_FNAME];
    stInput.u64VirAddr = CVI_NULL;
	u16Width  = 352;
	u16Height = 288;

    UNUSED(u32FrameNumMax);
    UNUSED(resultImgName);
    UNUSED(fpInput);
    UNUSED(pchAvi);
    UNUSED(pchRes);

#ifdef useOpenCV
	// for opencv
	Mat cvImg, cvFg, cvBg;
	Mat cvImgGray, cvFgBGR, cvBgBGR, cvDispImg;
	VideoCapture cvCap;
	VideoWriter  cvWte;
	int cvWidth, cvHeight, cvGap;
	Size cvSz;

	// ori w & h
	cvCap.open(pchAvi);
	cvWidth  = (CVI_S32)cvCap.get(CV_CAP_PROP_FRAME_WIDTH);
	cvHeight = (CVI_S32)cvCap.get(CV_CAP_PROP_FRAME_HEIGHT);

	// even w & h
	u16Width  = cvWidth & (~1);
	u16Height = cvHeight & (~1);

	//for images
	cvFg    = Mat::zeros(u16Height, u16Width, CV_8UC1);
	cvFgBGR = Mat::zeros(u16Height, u16Width, CV_8UC3);
	cvBg    = Mat::zeros(u16Height, u16Width, CV_8UC1);
	cvBgBGR = Mat::zeros(u16Height, u16Width, CV_8UC3);
#endif

	//Initialization--------------------------------------------------------------------
	//set default parameters
	stGmm2Ctrl.u16VarRate			  =	1;
	stGmm2Ctrl.u8ModelNum			  =	3;
	stGmm2Ctrl.u9q7MaxVar			  =	(16 * 16)<<7;
	stGmm2Ctrl.u9q7MinVar			  =	( 8 *  8)<<7;
	stGmm2Ctrl.u8GlbSnsFactor		  =	8;
	stGmm2Ctrl.enSnsFactorMode		  =	IVE_GMM2_SNS_FACTOR_MODE_PIX;
	stGmm2Ctrl.u16FreqThr			  =	12000;
	stGmm2Ctrl.u16FreqInitVal		  =	20000;
	stGmm2Ctrl.u16FreqAddFactor		  =	0xEF;
	stGmm2Ctrl.u16FreqReduFactor	  =	0xFF00;
	stGmm2Ctrl.u16LifeThr			  =	5000;
	stGmm2Ctrl.enLifeUpdateFactorMode = IVE_GMM2_LIFE_UPDATE_FACTOR_MODE_GLB;

	memset(&stIveImg, 0, sizeof(IVE_SRC_IMAGE_S));
	memset(&stIveFg,  0, sizeof(IVE_DST_IMAGE_S));
	memset(&stIveBg,  0, sizeof(IVE_DST_IMAGE_S));
	memset(&stFactor, 0, sizeof(IVE_SRC_IMAGE_S));
	memset(&stMatchModelInfo, 0, sizeof(IVE_DST_IMAGE_S));
	memset(&stModel,0,sizeof(IVE_MEM_INFO_S));
	memset(&stFgMask,  0, sizeof(IVE_DST_IMAGE_S));
	memset(&stLastImg,  0, sizeof(IVE_DST_IMAGE_S));
	memset(&stDiffImg,  0, sizeof(IVE_DST_IMAGE_S));
	memset(&stMagImg,  0, sizeof(IVE_DST_IMAGE_S));
	memset(&stCurNormMag,  0, sizeof(IVE_DST_IMAGE_S));
	memset(&stLastNormMag,  0, sizeof(IVE_DST_IMAGE_S));
	memset(&stBlob,0,sizeof(IVE_MEM_INFO_S));

	//for input image
	s32Ret = CVI_CreateIveImage(&stIveImg,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_NET_GOTO_PRT(s32Ret,CVI_SUCCESS,FAILURE,"CVI_CreateIveImage fail\n");
	memset((CVI_U8*)stIveImg.au64VirAddr[0], 0, sizeof(CVI_U8) * u16Width * u16Height);

	//for foreground
	s32Ret = CVI_CreateIveImage(&stIveFg,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_NET_GOTO_PRT(s32Ret,CVI_SUCCESS,FAILURE,"CVI_CreateIveImage fail\n");
	memset((CVI_U8*)stIveFg.au64VirAddr[0], 0, sizeof(CVI_U8) * u16Width * u16Height);

	//for background
	s32Ret = CVI_CreateIveImage(&stIveBg,IVE_IMAGE_TYPE_U8C1, u16Width,u16Height);
	CVI_CHECK_NET_GOTO_PRT(s32Ret,CVI_SUCCESS,FAILURE,"CVI_CreateIveImage fail\n");
	memset((CVI_U8*)stIveBg.au64VirAddr[0], 0, sizeof(CVI_U8) * u16Width * u16Height);

	//for sensitivity factor
	s32Ret = CVI_CreateIveImage(&stFactor,IVE_IMAGE_TYPE_U16C1, u16Width,u16Height);
	CVI_CHECK_NET_GOTO_PRT(s32Ret,CVI_SUCCESS,FAILURE,"CVI_CreateIveImage fail\n");
	for (int i = 0; i < u16Width * u16Height; i++)
	{
		((CVI_U8*)stFactor.au64VirAddr[0])[i<<1]     = 8;
		((CVI_U8*)stFactor.au64VirAddr[0])[(i<<1)+1] = 4;
	}

	//for matched model information
	s32Ret = CVI_CreateIveImage(&stMatchModelInfo,IVE_IMAGE_TYPE_U8C1, u16Width,u16Height);
	CVI_CHECK_NET_GOTO_PRT(s32Ret,CVI_SUCCESS,FAILURE,"CVI_CreateIveImage fail\n");
	memset((CVI_U8*)stMatchModelInfo.au64VirAddr[0], 0, sizeof(CVI_U8) * u16Width * u16Height);

	//for models
	stModel.u32Size = stGmm2Ctrl.u8ModelNum * 8 * u16Width * u16Height;
	stModel.u64VirAddr = (CVI_U64)malloc(stModel.u32Size);
	CVI_CHECK_ET_GOTO_PRT(stModel.u64VirAddr,0,FAILURE,"malloc fail\n");
	memset((CVI_U8*)stModel.u64VirAddr,0,stModel.u32Size);
	stModel.u64PhyAddr = (CVI_U64)stModel.u64VirAddr;

	//for pixel factor adjustment
	s32Ret = CVI_CreateIveImage(&stFgMask,IVE_IMAGE_TYPE_U8C1, u16Width,u16Height);
	CVI_CHECK_NET_GOTO_PRT(s32Ret,CVI_SUCCESS,FAILURE,"CVI_CreateIveImage fail\n");
	memset((CVI_U8*)stFgMask.au64VirAddr[0], 0, sizeof(CVI_U8) * u16Width * u16Height);

	s32Ret = CVI_CreateIveImage(&stLastImg,IVE_IMAGE_TYPE_U8C1, u16Width,u16Height);
	CVI_CHECK_NET_GOTO_PRT(s32Ret,CVI_SUCCESS,FAILURE,"CVI_CreateIveImage fail\n");
	memset((CVI_U8*)stLastImg.au64VirAddr[0], 0, sizeof(CVI_U8) * u16Width * u16Height);

	s32Ret = CVI_CreateIveImage(&stDiffImg,IVE_IMAGE_TYPE_U8C1, u16Width,u16Height);
	CVI_CHECK_NET_GOTO_PRT(s32Ret,CVI_SUCCESS,FAILURE,"CVI_CreateIveImage fail\n");
	memset((CVI_U8*)stDiffImg.au64VirAddr[0], 0, sizeof(CVI_U8) * u16Width * u16Height);

	s32Ret = CVI_CreateIveImage(&stMagImg,IVE_IMAGE_TYPE_U16C1, u16Width,u16Height);
	CVI_CHECK_NET_GOTO_PRT(s32Ret,CVI_SUCCESS,FAILURE,"CVI_CreateIveImage fail\n");
	memset((CVI_U8*)stMagImg.au64VirAddr[0], 0, sizeof(CVI_U16) * u16Width * u16Height);

	s32Ret = CVI_CreateIveImage(&stCurNormMag,IVE_IMAGE_TYPE_U8C1, u16Width,u16Height);
	CVI_CHECK_NET_GOTO_PRT(s32Ret,CVI_SUCCESS,FAILURE,"CVI_CreateIveImage fail\n");
	memset((CVI_U8*)stCurNormMag.au64VirAddr[0], 0, sizeof(CVI_U8) * u16Width * u16Height);

	s32Ret = CVI_CreateIveImage(&stLastNormMag,IVE_IMAGE_TYPE_U8C1, u16Width,u16Height);
	CVI_CHECK_NET_GOTO_PRT(s32Ret,CVI_SUCCESS,FAILURE,"CVI_CreateIveImage fail\n");
	memset((CVI_U8*)stLastNormMag.au64VirAddr[0], 0, sizeof(CVI_U8) * u16Width * u16Height);
	//for models
	stBlob.u32Size = sizeof(IVE_CCBLOB_S);
	stBlob.u64VirAddr = (CVI_U64)malloc(stBlob.u32Size);
	CVI_CHECK_ET_GOTO_PRT(stBlob.u64VirAddr,0,FAILURE,"malloc fail\n");
	stBlob.u64PhyAddr = (CVI_U64)stBlob.u64VirAddr;

#ifdef useOpenCV
	cvGap = 10;
	cvSz.width  = stIveImg.u32Width  + stIveBg.u32Width  + stIveFg.u32Width  + 4 * cvGap;
	cvSz.height = max(max(stIveImg.u32Height, stIveBg.u32Height), stIveFg.u32Height) + 2 * cvGap;
	cvWte.open(pchRes, CV_FOURCC('X','V','I','D'), cvCap.get(CV_CAP_PROP_FPS), cvSz);
#else
    u32FrameNumMax = 2;
    stInput.u32Size = u16Height * u16Width;
    stInput.u64VirAddr =  (CVI_U64)malloc(stInput.u32Size);
    stInput.u64PhyAddr = (CVI_U64)stInput.u64VirAddr;
#endif

	for(;;)
	{
#ifdef useOpenCV
		vector<Mat *> cvImgVec;
		s32FrmCnt = (CVI_S32)cvCap.get(CV_CAP_PROP_POS_FRAMES);

		cvCap >> cvImg;
		if( cvImg.empty())
			break;

		if(cvImg.rows != (int)stIveImg.u32Height || cvImg.cols != (int)stIveImg.u32Width)
		{
			printf("Size not match, quit\n");
			break;
		}

		cvtColor(cvImg, cvImgGray, CV_BGR2GRAY);
		mat2IveImg(&cvImgGray, &stIveImg, CVGRAY_IVEU8C1);
#else
        if ((CVI_U32)s32FrmCnt > u32FrameNumMax) {
            break;
        }

        snprintf(resultImgName, _MAX_FNAME,
                "../data/avi/campus.u8c1.%d.raw", s32FrmCnt);
		s32Ret = fopen_s(&fpInput, resultImgName,"rb");
		CVI_CHECK_NET_GOTO(s32Ret,0,FAILURE);
		s32Ret = CVI_MemReadFile(&stInput,fpInput);
		CVI_CHECK_NET_GOTO(s32Ret, 0, FAILURE);
		CVI_FCLOSE(fpInput);

        for(int r=0; r<u16Height; r++)
        {
            CVI_U8 *ptm = (CVI_U8*)(stInput.u64VirAddr + r * stIveImg.au32Stride[0]);
            CVI_U8 *pti = (CVI_U8*)(stIveImg.au64VirAddr[0] + r * stIveImg.au32Stride[0]);


            memcpy(pti, ptm, u16Width);
        }
#endif
		u32FrmNum = s32FrmCnt + 1;


		//To building a stable background model quickly at the begin, some parameters are set specially.
		if (1 == stGmm2Ctrl.u8ModelNum)
		{
			//If the parameter u8ModelNum is set to 1, the parameter u16FreqReduFactor
			//is usually set to a small value at the first N frames. Here, N = 500.
			stGmm2Ctrl.u16FreqReduFactor = (u32FrmNum >= 500) ? 0xFFA0 : 0xFC00;
		}
		else
		{
			//If the parameter u8ModelNum is more than 1, the global life mode should be used at the first N frames,
			//and the parameter u16GlbLifeUpdateFactor is usually set to a big value. Here, N = 500.
			if(u32FrmNum >= 500)
			{
				stGmm2Ctrl.enLifeUpdateFactorMode = IVE_GMM2_LIFE_UPDATE_FACTOR_MODE_PIX;
			}
			else
			{
				stGmm2Ctrl.u16GlbLifeUpdateFactor = 0xFFFF / u32FrmNum;
			}
		}

		//GMM2
		s32Ret = CVI_MPI_IVE_GMM2(&hIveHandle,&stIveImg,&stFactor,&stIveFg,&stIveBg,
			&stMatchModelInfo,&stModel,&stGmm2Ctrl,bInstant);
		CVI_CHECK_NET_GOTO_PRT(s32Ret,CVI_SUCCESS,FAILURE,"CVI_MPI_IVE_GMM2 fail,Error(%#x)\n",s32Ret);

		//factor adjustment
		if (u32FrmNum > 1)
		{
			//gen fg mask
			GMM2_Sample_GenFgMask(&stIveFg,&stFgMask);
			//adjustment  factor
			GMM2_Sample_AdjustmentFactor(&stFactor,
				&stFgMask, &stIveImg,
				&stLastImg, &stDiffImg,
				 &stMagImg, &stCurNormMag,
				&stLastNormMag, &stBlob);
		}
#ifdef useOpenCV
		mat2IveImg(&cvImgGray, &stLastImg, CVGRAY_IVEU8C1);
#endif

		if (compare && (s32FrmCnt == 0 || s32FrmCnt == 1 || s32FrmCnt == 512))
		{
			FILE *fp;
			CVI_CHAR fileName[_MAX_FNAME];
			IVE_IMAGE_S stIveBgRef;
			IVE_IMAGE_S stIveFgRef;
			CVI_S32 cmp, open;

			snprintf(fileName, _MAX_FNAME, "../data/result/sample_GMM2_U8C1_PixelCtrl_bg_%d.yuv", s32FrmCnt);
			open = fopen_s(&fp,fileName,"rb");
			if (open == CVI_SUCCESS)
			{
				CVI_CreateIveImage(&stIveBgRef, stIveBg.enType, u16Width, u16Height);
				CVI_ReadFile(&stIveBgRef, fp);
				CVI_FCLOSE(fp);
				cmp = CVI_CompareIveImage(&stIveBg, &stIveBgRef);
				if (cmp)
				{
					printf("Compare against %s failed\n", fileName);
					s32CompareError ++;
				}
				else
				{
					printf("Compare against %s passed\n", fileName);
				}
				CVI_DestroyIveImage(&stIveBgRef);
			}
			else
			{
				printf("%s not exist\n", fileName);
			}

			snprintf(fileName, _MAX_FNAME, "../data/result/sample_GMM2_U8C1_PixelCtrl_fg_%d.yuv", s32FrmCnt);
			open = fopen_s(&fp,fileName,"rb");
			if (open == CVI_SUCCESS)
			{
				CVI_CreateIveImage(&stIveFgRef, stIveFg.enType, u16Width, u16Height);
				CVI_ReadFile(&stIveFgRef, fp);
				CVI_FCLOSE(fp);
				cmp = CVI_CompareIveImage(&stIveFg, &stIveFgRef);
				if (cmp)
				{
					printf("Compare against %s failed\n", fileName);
					s32CompareError ++;
				}
				else
				{
					printf("Compare against %s passed\n", fileName);
				}
				CVI_DestroyIveImage(&stIveFgRef);
			}
			else
			{
				printf("%s not exist\n", fileName);
			}
		}

		if (show)
		{
#ifdef useOpenCV
			//show the results
			iveImage2Mat(&stIveBg, &cvBg, IVEU8C1_CVGRAY);
			iveImage2Mat(&stIveFg, &cvFg, IVEU8C1_CVGRAY);
			cvtColor(cvBg, cvBgBGR, CV_GRAY2BGR);
			cvtColor(cvFg, cvFgBGR, CV_GRAY2BGR);

			putText(cvImg, "srcImg", Point(5,15), FONT_HERSHEY_COMPLEX_SMALL, 1, CV_RGB(255,0,0));
			cvImgVec.push_back(&cvImg);

			putText(cvBgBGR, "bgImg", Point(5,15), FONT_HERSHEY_COMPLEX_SMALL, 1, CV_RGB(255,0,0));
			cvImgVec.push_back(&cvBgBGR);

			putText(cvFgBGR, "fgImg", Point(5,15), FONT_HERSHEY_COMPLEX_SMALL, 1, CV_RGB(255,0,0));
			cvImgVec.push_back(&cvFgBGR);

			cvShowMultiImg("GMM_U8C1", 1, 3, cvImgVec, cvGap, &cvDispImg);
			waitKey(10);

			cvWte.write(cvDispImg);
#endif
		}
		s32FrmCnt++;
	}

FAILURE:
    if (stInput.u64VirAddr != CVI_NULL) {
        free ((CVI_U8*)stInput.u64VirAddr);
    }

	CVI_DestroyIveImage(&stIveImg);
	CVI_DestroyIveImage(&stIveFg);
	CVI_DestroyIveImage(&stIveBg);
	CVI_DestroyIveImage(&stFactor);
	CVI_DestroyIveImage(&stMatchModelInfo);
	CVI_DestroyIveImage(&stFgMask);
	CVI_DestroyIveImage(&stLastImg);
	CVI_DestroyIveImage(&stDiffImg);
	CVI_DestroyIveImage(&stMagImg);
	CVI_DestroyIveImage(&stCurNormMag);
	CVI_DestroyIveImage(&stLastNormMag);
	CVI_FREE_64(stModel.u64VirAddr);
	CVI_FREE_64(stBlob.u64VirAddr);

#ifdef useOpenCV
	if (show)
		cvDestroyAllWindows();
#endif
	return (s32CompareError == 0) ? s32Ret : CVI_FAILURE;
}
