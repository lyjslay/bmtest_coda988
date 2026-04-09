//#define useOpenCV (1)
#include "BgModel.h"
#include <mpi_ive.h>
#include <sample_define.h>
#include <sample_assist.h>
#include <sample_file.h>

#include <stdlib.h>     /* malloc, free, rand */
#include <string.h> /* memset */
#include <fstream> // std::ofstream
#include <sstream>      // std::ostringstream

#ifdef useOpenCV
#include <opencv2/opencv.hpp>
using namespace cv;
#endif

using namespace std;

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

CVI_S32 InitMatchBgModel(IVE_SRC_IMAGE_S *pstCurImg, IVE_DATA_S *pstBgModel,	IVE_IMAGE_S *pstFgFlag,
	IVE_DST_IMAGE_S *pstBgDiffFg, IVE_DST_IMAGE_S *pstFrmDiffFg, IVE_DST_MEM_INFO_S *pstStatData,
	IVE_MATCH_BG_MODEL_CTRL_S *pstMatchBgModelCtrl,CVI_U32 u32Width,CVI_U32 u32Height)
{
	CVI_S32 s32Result = CVI_SUCCESS;

	//malloc pstCurImg
	s32Result = CVI_CreateIveImage(pstCurImg,IVE_IMAGE_TYPE_U8C1,u32Width,u32Height);
	CVI_CHECK_ET_RET(s32Result,CVI_FAILURE,s32Result);

	//malloc pstBgModel
	pstBgModel->u32Height = u32Height;
	pstBgModel->u32Width = u32Width * sizeof(IVE_BG_MODEL_PIX_S);
	pstBgModel->u32Stride = CVI_CalcStride(pstBgModel->u32Width,CVI_IVE2_STRIDE_ALIGN);
	pstBgModel->u64VirAddr = (CVI_U64)malloc(pstBgModel->u32Height * pstBgModel->u32Stride);
	CVI_CHECK_ET_RET(pstBgModel->u64VirAddr,0,CVI_FAILURE);
	memset((CVI_VOID*)pstBgModel->u64VirAddr,0,pstBgModel->u32Height * pstBgModel->u32Stride);
	pstBgModel->u64PhyAddr = (CVI_U64) pstBgModel->u64VirAddr;

	//malloc pstFgFlag
	s32Result = CVI_CreateIveImage(pstFgFlag,IVE_IMAGE_TYPE_U8C1,u32Width,u32Height);
	CVI_CHECK_ET_RET(s32Result,CVI_FAILURE,s32Result);
	memset((CVI_VOID*)pstFgFlag->au64VirAddr[0],0,pstFgFlag->u32Height * pstFgFlag->au32Stride[0]);

	//malloc pstMatchFg
	s32Result = CVI_CreateIveImage(pstBgDiffFg,IVE_IMAGE_TYPE_S8C1,u32Width,u32Height);
	CVI_CHECK_ET_RET(s32Result,CVI_FAILURE,s32Result);

	//malloc pstDiffFg
	s32Result = CVI_CreateIveImage(pstFrmDiffFg,IVE_IMAGE_TYPE_S8C1,u32Width,u32Height);
	CVI_CHECK_ET_RET(s32Result,CVI_FAILURE,s32Result);

	//malloc pstStatData
	pstStatData->u32Size =  sizeof(IVE_BG_STAT_DATA_S) ;
	pstStatData->u64VirAddr = (CVI_U64)malloc(pstStatData->u32Size);
	CVI_CHECK_ET_RET(pstStatData->u64VirAddr,0,CVI_FAILURE);
	pstStatData->u64PhyAddr = (CVI_U64)pstStatData->u64VirAddr;

	//init ctrl info
	pstMatchBgModelCtrl->u32CurFrmNum = 1;
	pstMatchBgModelCtrl->u32PreFrmNum = 1;
	pstMatchBgModelCtrl->u16TimeThr = 20;
	pstMatchBgModelCtrl->u8DiffThrCrlCoef = 0;
	pstMatchBgModelCtrl->u8DiffMaxThr = 6;
	pstMatchBgModelCtrl->u8DiffMinThr = 4;
	pstMatchBgModelCtrl->u8DiffThrInc = 0;
	pstMatchBgModelCtrl->u8FastLearnRate = 1;
	pstMatchBgModelCtrl->u8DetChgRegion = 0;

	return s32Result;
}

CVI_S32 InitUpdateBgModel(IVE_DST_IMAGE_S *pstBgImg, IVE_DST_IMAGE_S *pstChgStaImg,
	IVE_DST_IMAGE_S *pstChgStaFg, IVE_DST_IMAGE_S *pstChgStaLife,
	IVE_UPDATE_BG_MODEL_CTRL_S *pstUpdateBgModelCtrl,CVI_U16 u16Width,CVI_U16 u16Height)
{
	CVI_S32 s32Result = CVI_SUCCESS;

	//malloc pstBgImg
	s32Result = CVI_CreateIveImage(pstBgImg,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_RET(s32Result,CVI_FAILURE,s32Result);

	//malloc pstChgStaImg
	s32Result = CVI_CreateIveImage(pstChgStaImg,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_RET(s32Result,CVI_FAILURE,s32Result);

	//malloc pstChgStaFg
	s32Result = CVI_CreateIveImage(pstChgStaFg,IVE_IMAGE_TYPE_S8C1,u16Width,u16Height);
	CVI_CHECK_ET_RET(s32Result,CVI_FAILURE,s32Result);

	//malloc pstChgStaLife
	s32Result = CVI_CreateIveImage(pstChgStaLife,IVE_IMAGE_TYPE_U16C1,u16Width,u16Height);
	CVI_CHECK_ET_RET(s32Result,CVI_FAILURE,s32Result);

	//init ctrl info
	pstUpdateBgModelCtrl->u32CurFrmNum = 1;
	pstUpdateBgModelCtrl->u32PreChkTime = 1;
	pstUpdateBgModelCtrl->u32FrmChkPeriod = 100;
	pstUpdateBgModelCtrl->u32InitMinTime = 200;
	pstUpdateBgModelCtrl->u32StyBgMinBlendTime = 200;
	pstUpdateBgModelCtrl->u32StyBgMaxBlendTime = 1500;
	pstUpdateBgModelCtrl->u32DynBgMinBlendTime = 0;
	pstUpdateBgModelCtrl->u32StaticDetMinTime = 50;
	pstUpdateBgModelCtrl->u16FgMaxFadeTime = 15;
	pstUpdateBgModelCtrl->u16BgMaxFadeTime= 15;
	pstUpdateBgModelCtrl->u8StyBgAccTimeRateThr = 80;
	pstUpdateBgModelCtrl->u8ChgBgAccTimeRateThr = 60;
	pstUpdateBgModelCtrl->u8DynBgAccTimeThr = 0;
	pstUpdateBgModelCtrl->u8DynBgDepth = 3;
	pstUpdateBgModelCtrl->u8BgEffStaRateThr = 90;
	pstUpdateBgModelCtrl->u8AcceBgLearn = 1;
	pstUpdateBgModelCtrl->u8DetChgRegion = 0;

	return s32Result;
}
CVI_S32 InitSub(IVE_DST_IMAGE_S *pstFgImg,IVE_SUB_CTRL_S  *pstSubCtrl,CVI_U16 u16Width,CVI_U16 u16Height)
{
	CVI_S32 s32Result = CVI_SUCCESS;
	//malloc pstFgImg
	s32Result = CVI_CreateIveImage(pstFgImg,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_RET(s32Result,CVI_FAILURE,s32Result);

	pstSubCtrl->enMode = IVE_SUB_MODE_ABS;

	return s32Result;
}

CVI_VOID UninitMatchBgModel(IVE_SRC_IMAGE_S *pstCurImg, IVE_DATA_S *pstBgModel,	IVE_IMAGE_S *pstFgFlag,
	IVE_DST_IMAGE_S *pstMatchFg, IVE_DST_IMAGE_S *pstDiffFg, IVE_DST_MEM_INFO_S *pstStatData)
{
	CVI_DestroyIveImage(pstCurImg);
	CVI_FREE_64(pstBgModel->u64VirAddr);
	CVI_DestroyIveImage(pstFgFlag);
	CVI_DestroyIveImage(pstMatchFg);
	CVI_DestroyIveImage(pstDiffFg);
	CVI_FREE_64(pstStatData->u64VirAddr);
}

CVI_VOID UninitUpdateBgModel(IVE_DST_IMAGE_S *pstBgImg, IVE_DST_IMAGE_S *pstChgStaImg,
	IVE_DST_IMAGE_S *pstChgStaFg, IVE_DST_IMAGE_S *pstChgStaLife)
{
	CVI_DestroyIveImage(pstBgImg);
	CVI_DestroyIveImage(pstChgStaImg);
	CVI_DestroyIveImage(pstChgStaFg);
	CVI_DestroyIveImage(pstChgStaLife);
}

CVI_VOID UninitSub(IVE_DST_IMAGE_S *pstDst)
{
	CVI_DestroyIveImage(pstDst);
}

static CVI_S32 CompareImage(IVE_IMAGE_S *img, CVI_CHAR *fileName)
{
	FILE *fp;
	IVE_IMAGE_S ref;
	CVI_S32 cmp, open;
	CVI_S32 result = 0;

	open = fopen_s(&fp,fileName,"rb");
	if (open == CVI_SUCCESS)
	{
		CVI_CreateIveImage(&ref, img->enType, img->u32Width, img->u32Height);
		CVI_ReadFile(&ref, fp);
		CVI_FCLOSE(fp);
		cmp = CVI_CompareIveImage(img, &ref);
		if (cmp)
		{
			printf("Compare against %s failed\n", fileName);
			result ++;
		}
		else
		{
			printf("Compare against %s passed\n", fileName);
		}
		CVI_DestroyIveImage(&ref);
	}
	else
	{
		printf("%s not exist\n", fileName);
	}
	return result;
}

CVI_S32 BgModelSample1(int show, int compare)
{
	IVE_HANDLE hIveHandle;
	CVI_BOOL bInstant = CVI_TRUE;
	CVI_S32 s32Result;
	CVI_S32 s32CompareError = 0;

	IVE_SRC_IMAGE_S stCurImg;
	IVE_DATA_S stBgModel;
	IVE_IMAGE_S stFgFlag;
	IVE_DST_IMAGE_S stBgDiffFg;
	IVE_DST_IMAGE_S stFrmDiffFg;
	IVE_DST_MEM_INFO_S stStatData;
	IVE_MATCH_BG_MODEL_CTRL_S stMatchBgModelCtrl;

	IVE_DST_IMAGE_S stBgImg;
	IVE_DST_IMAGE_S stChgStaImg;
	IVE_DST_IMAGE_S stChgStaFg;
	IVE_DST_IMAGE_S stChgStaLife;
	IVE_UPDATE_BG_MODEL_CTRL_S stUpdateBgModelCtrl;

	IVE_DST_IMAGE_S stFgImg;
	IVE_SUB_CTRL_S  stSubCtrl;

	const CVI_CHAR *pchFileName = "../data/avi/hall.avi";
	const CVI_CHAR *pchRes = "../data/avi/BgModel_Sample1.avi";
	CVI_U16 u16Width;
	CVI_U16 u16Height;
	CVI_S32 s32FrmCnt = 0;
	CVI_U32 u32FrameNum;
    CVI_U32 u32FrameNumMax = -1;

#ifdef useOpenCV
	// for opencv
	Mat cvImg, cvFg, cvBg;
	Mat cvFgBGR, cvBgBGR;
	//Mat cvImgGrayOrg;
	Mat cvImgGray, cvDispImg;
	VideoCapture cvCap;
	VideoWriter  cvWte;
	int cvWidth, cvHeight, cvGap;
	Size cvSz;
#endif

	FILE *fpInput = NULL;
	IVE_DST_MEM_INFO_S stInput;
    u16Height = 576;
    u16Width = 720;
	CVI_CHAR resultImgName[_MAX_FNAME];
    stInput.u64VirAddr = CVI_NULL;

    UNUSED(pchFileName);
    UNUSED(pchRes);
    UNUSED(u32FrameNumMax);
    UNUSED(resultImgName);
    UNUSED(fpInput);

#ifdef useOpenCV
	// ori w & h
	cvCap.open(pchFileName);
	cvWidth  = (CVI_S32)cvCap.get(CV_CAP_PROP_FRAME_WIDTH);
	cvHeight = (CVI_S32)cvCap.get(CV_CAP_PROP_FRAME_HEIGHT);

	// even w & h
	u16Width  = cvWidth & (~1);
	u16Height = cvHeight & (~1);
    printf("use opencv\n");
#endif
	//u16Width = CVI_IMAGE_WIDTH_352;
	//u16Height = CVI_IMAGE_HEIGHT_288;

	//init match bg model info
	s32Result = InitMatchBgModel(&stCurImg, &stBgModel,&stFgFlag,
		&stBgDiffFg, &stFrmDiffFg, &stStatData,&stMatchBgModelCtrl,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	//init update bg model info
	s32Result = InitUpdateBgModel(&stBgImg, &stChgStaImg,&stChgStaFg, &stChgStaLife,
		&stUpdateBgModelCtrl,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	//init sub info
	s32Result = InitSub(&stFgImg,&stSubCtrl,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	//malloc opencv source
#ifdef useOpenCV
	cvFg	= Mat::zeros(u16Height, u16Width, CV_8UC1);
	cvFgBGR = Mat::zeros(u16Height, u16Width, CV_8UC3);
	cvBg	= Mat::zeros(u16Height, u16Width, CV_8UC1);
	cvBgBGR = Mat::zeros(u16Height, u16Width, CV_8UC3);

	cvGap = 10;
	cvSz.width  = stCurImg.u32Width + stBgImg.u32Width + stFgImg.u32Width  + 4 * cvGap;
	cvSz.height = max(max(stCurImg.u32Height, stBgImg.u32Height), stFgImg.u32Height) + 2 * cvGap;
	cvWte.open(pchRes, CV_FOURCC('X','V','I','D'), cvCap.get(CV_CAP_PROP_FPS), cvSz);

#else
    u32FrameNumMax = 5;
    stInput.u32Size = u16Height * u16Width;
    stInput.u64VirAddr =  (CVI_U64)malloc(stInput.u32Size);
    stInput.u64PhyAddr = (CVI_U64)stInput.u64VirAddr;

#endif
    for(;;)
	{
#ifdef useOpenCV
		vector<Mat *> cvImgVec;
		s32FrmCnt = (CVI_S32)cvCap.get(CV_CAP_PROP_POS_FRAMES);
		u32FrameNum = s32FrmCnt + 1;

		cvCap >> cvImg;
		if( cvImg.empty())
			break;

		if(cvImg.rows != (int)stCurImg.u32Height || cvImg.cols != (int)stCurImg.u32Width)
		{
			printf("Size not match, quit\n");
			break;
		}

        if (0)
        {
            // debug use
            std::ostringstream filename;
            std::ofstream outfile;
            filename.str("");
            filename << "../data/avi/BgModel_Sample" << s32FrmCnt << ".raw";
            outfile.open (filename.str().c_str(), ios::out | ios::binary);
			cvtColor(cvImg, cvImgGray, CV_BGR2GRAY);
            outfile.write ((char*)(cvImgGray.data), cvImgGray.total());  // In byte so frame.total() should be enough ?
            outfile.close();
        }
#else
		u32FrameNum = s32FrmCnt + 1;
        
		snprintf(resultImgName, _MAX_FNAME,
                "../data/avi/BgModel_Sample%d.raw", s32FrmCnt);
		s32Result = fopen_s(&fpInput, resultImgName,"rb");
		CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
		s32Result = CVI_MemReadFile(&stInput,fpInput);
		CVI_CHECK_NET_GOTO(s32Result, 0, FAILURE);
		CVI_FCLOSE(fpInput);

        if (u32FrameNum > u32FrameNumMax) {
            break;
        }
#endif

        if ((stMatchBgModelCtrl.u32CurFrmNum - stMatchBgModelCtrl.u32PreFrmNum) == 3)
		{
			//IVE_BG_STAT_DATA_S *stat = (IVE_BG_STAT_DATA_S *)stStatData.u64VirAddr;
			//cvResize(pstCvGrayImgOrg,pstCvGrayImg,CV_INTER_LINEAR);

			//get gray img
#ifdef useOpenCV
			cvtColor(cvImg, cvImgGray, CV_BGR2GRAY);
			mat2IveImg(&cvImgGray, &stCurImg, CVGRAY_IVEU8C1);
#else
            for(int r=0; r<u16Height; r++)
            {
                CVI_U8 *ptm = (CVI_U8*)(stInput.u64VirAddr + r * stCurImg.au32Stride[0]);
                CVI_U8 *pti = (CVI_U8*)(stCurImg.au64VirAddr[0] + r * stCurImg.au32Stride[0]);

                memcpy(pti, ptm, u16Width);
            }
#endif

			//call match bg model
			s32Result = CVI_MPI_IVE_MatchBgModel(&hIveHandle,&stCurImg, &stBgModel,
				&stFgFlag, &stBgDiffFg, &stFrmDiffFg, &stStatData,&stMatchBgModelCtrl,bInstant);
			CVI_CHECK_NET_GOTO(s32Result,CVI_SUCCESS,FAILURE);
			//printf("frm %d, stat u32PixNum = %d, u32SumLum = %d\n", s32FrmCnt, stat->u32PixNum, stat->u32SumLum);

			//call update bg model
			s32Result = CVI_MPI_IVE_UpdateBgModel(&hIveHandle,&stBgModel, &stFgFlag,
				&stBgImg, &stChgStaImg, &stChgStaFg, &stChgStaLife,
				&stStatData, &stUpdateBgModelCtrl,bInstant);
			CVI_CHECK_NET_GOTO(s32Result,CVI_SUCCESS,FAILURE);
			//printf("frm %d, stat u32PixNum = %d, u32SumLum = %d\n", s32FrmCnt, stat->u32PixNum, stat->u32SumLum);

			//call sub to get fg image is not very good method,as you see in result window
			s32Result = CVI_MPI_IVE_Sub(&hIveHandle,&stCurImg,&stBgImg,&stFgImg,&stSubCtrl,bInstant);
			CVI_CHECK_NET_GOTO(s32Result,CVI_SUCCESS,FAILURE);

			stMatchBgModelCtrl.u32PreFrmNum = u32FrameNum;
			stUpdateBgModelCtrl.u32PreChkTime = u32FrameNum;

			if (compare && (s32FrmCnt == 4 || s32FrmCnt == 8 || s32FrmCnt == 512))
			{
				CVI_CHAR fileName[_MAX_FNAME];

#if 0
				snprintf(fileName, _MAX_FNAME, "../data/result/sample_BgModel_Sample1_fgFlag_%d.yuv", s32FrmCnt);
				s32CompareError += CompareImage(&stFgFlag, fileName);

				snprintf(fileName, _MAX_FNAME, "../data/result/sample_BgModel_Sample1_bgDiffFg_%d.yuv", s32FrmCnt);
				s32CompareError += CompareImage(&stBgDiffFg, fileName);

				snprintf(fileName, _MAX_FNAME, "../data/result/sample_BgModel_Sample1_frmDiffFg_%d.yuv", s32FrmCnt);
				s32CompareError += CompareImage(&stFrmDiffFg, fileName);

				//snprintf(fileName, _MAX_FNAME, "../data/result/sample_BgModel_Sample1_chgStaFg_%d.yuv", s32FrmCnt);
				//s32CompareError += CompareImage(&stChgStaImg, fileName);

				//snprintf(fileName, _MAX_FNAME, "../data/result/sample_BgModel_Sample1_chgStaFg_%d.yuv", s32FrmCnt);
				//s32CompareError += CompareImage(&stChgStaFg, fileName);

				snprintf(fileName, _MAX_FNAME, "./sample_BgModel_Sample1_Model_%d.bin", s32FrmCnt);
				FILE *fp;
				CVI_S32 open = fopen_s(&fp,fileName,"wb");
				if (open == CVI_SUCCESS)
				{
					CVI_DataWriteFile(&stBgModel, fp);
					CVI_FCLOSE(fp);
				}
#endif

				snprintf(fileName, _MAX_FNAME, "../data/result/sample_BgModel_Sample1_bg_%d.yuv", s32FrmCnt);
				s32CompareError += CompareImage(&stBgImg, fileName);

				snprintf(fileName, _MAX_FNAME, "../data/result/sample_BgModel_Sample1_fg_%d.yuv", s32FrmCnt);
				s32CompareError += CompareImage(&stFgImg, fileName);
			}

			if (show)
			{
				//show result image
				//ShowImg(pstCvGrayImgOrg,pstCvGrayImg,pstCvBgImg,&stBgImg,pstCvFgImg,&stFgImg);
#ifdef useOpenCV
				iveImage2Mat(&stBgImg, &cvBg, IVEU8C1_CVGRAY);
				iveImage2Mat(&stFgImg, &cvFg, IVEU8C1_CVGRAY);
				cvtColor(cvBg, cvBgBGR, CV_GRAY2BGR);
				cvtColor(cvFg, cvFgBGR, CV_GRAY2BGR);

				putText(cvImg, "srcImg", Point(5,15), FONT_HERSHEY_COMPLEX_SMALL, 1, CV_RGB(255,0,0));
				cvImgVec.push_back(&cvImg);

				putText(cvBgBGR, "bgImg", Point(5,15), FONT_HERSHEY_COMPLEX_SMALL, 1, CV_RGB(255,0,0));
				cvImgVec.push_back(&cvBgBGR);

				putText(cvFgBGR, "fgImg", Point(5,15), FONT_HERSHEY_COMPLEX_SMALL, 1, CV_RGB(255,0,0));
				cvImgVec.push_back(&cvFgBGR);

				cvShowMultiImg("BgModel_Sample1", 1, 3, cvImgVec, cvGap, &cvDispImg);
				waitKey(10);

				cvWte.write(cvDispImg);
#endif
			}
		}

		stMatchBgModelCtrl.u32CurFrmNum = u32FrameNum;
		stUpdateBgModelCtrl.u32CurFrmNum = u32FrameNum;
        s32FrmCnt++;
	}

	//release source
FAILURE:

	UninitMatchBgModel(&stCurImg, &stBgModel,&stFgFlag, &stBgDiffFg, &stFrmDiffFg, &stStatData);
	UninitUpdateBgModel(&stBgImg, &stChgStaImg,	&stChgStaFg, &stChgStaLife);
	UninitSub(&stFgImg);
    if (stInput.u64VirAddr != CVI_NULL) {
        free ((CVI_U8*)stInput.u64VirAddr);
    }

#ifdef useOpenCV
    if (show)
		cvDestroyAllWindows();
#endif
	return (s32CompareError == 0) ? s32Result : CVI_FAILURE;
}

CVI_S32 BgModelSample2(int show, int compare)
{
#ifdef useOpenCV
	IVE_HANDLE hIveHandle = 0;
	CVI_BOOL bInstant = CVI_TRUE;
	CVI_S32 s32Result;
	CVI_S32 s32CompareError = 0;

	IVE_SRC_IMAGE_S stCurImg;
	IVE_DATA_S stBgModel;
	IVE_IMAGE_S stFgFlag;
	IVE_DST_IMAGE_S stBgDiffFg;
	IVE_DST_IMAGE_S stFrmDiffFg;
	IVE_DST_MEM_INFO_S stStatData;
	IVE_MATCH_BG_MODEL_CTRL_S stMatchBgModelCtrl;

	IVE_DST_IMAGE_S stBgImg;
	IVE_DST_IMAGE_S stChgStaImg;
	IVE_DST_IMAGE_S stChgStaFg;
	IVE_DST_IMAGE_S stChgStaLife;
	IVE_UPDATE_BG_MODEL_CTRL_S stUpdateBgModelCtrl;

	IVE_DST_IMAGE_S stFgImg;
	IVE_THRESH_CTRL_S stThrCtrl = {IVE_THRESH_MODE_BINARY, 0, 0, 0, 0, 255};

	const CVI_CHAR *pchFileName = "../data/avi/hall.avi";
	const CVI_CHAR *pchRes = "../data/avi/BgModel_Sample2.avi";
	CVI_U16 u16Width;
	CVI_U16 u16Height;
	CVI_S32 s32FrmCnt = 0;
	CVI_U32 u32FrameNum;

	CVI_U32 u32UpdCnt = 0;
	CVI_U32 u32PreUpdTime = 0;
	CVI_U32 u32FrmUpdPeriod = 15;
	CVI_U32 u32PreChkTime = 0;
	CVI_U32 u32FrmChkPeriod = 50;

	// for opencv
	Mat cvImg, cvFg, cvBg;
	Mat cvFgBGR, cvBgBGR;
	//Mat cvImgGrayOrg;
	Mat cvImgGray, cvDispImg;
	VideoCapture cvCap;
	VideoWriter  cvWte;
	int cvWidth, cvHeight, cvGap;
	Size cvSz;

	// ori w & h
	cvCap.open(pchFileName);
	cvWidth  = (CVI_S32)cvCap.get(CV_CAP_PROP_FRAME_WIDTH);
	cvHeight = (CVI_S32)cvCap.get(CV_CAP_PROP_FRAME_HEIGHT);

	// even w & h
	u16Width  = cvWidth & (~1);
	u16Height = cvHeight & (~1);

	memset(&stBgModel, 0, sizeof(IVE_DATA_S));
	memset(&stStatData, 0, sizeof(IVE_DST_MEM_INFO_S));

	//
	//init match bg model info, InitMatchBgModel()
	//
	//malloc pstCurImg
	s32Result = CVI_CreateIveImage(&stCurImg,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	//malloc pstBgModel
	stBgModel.u32Height = u16Height;
	stBgModel.u32Width = u16Width * sizeof(IVE_BG_MODEL_PIX_S);
	stBgModel.u32Stride = CVI_CalcStride(stBgModel.u32Width, CVI_IVE2_STRIDE_ALIGN);
	stBgModel.u64VirAddr = (CVI_U64)malloc( stBgModel.u32Height * stBgModel.u32Stride );
	memset((CVI_VOID*) stBgModel.u64VirAddr, 0, stBgModel.u32Height * stBgModel.u32Stride );
	stBgModel.u64PhyAddr = (CVI_U64)stBgModel.u64VirAddr;

	//malloc pstFgFlag
	s32Result = CVI_CreateIveImage(&stFgFlag,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	memset( (CVI_VOID*)stFgFlag.au64VirAddr[0], 0, u16Width * u16Height );

	//malloc pstMatchFg
	s32Result = CVI_CreateIveImage(&stBgDiffFg,IVE_IMAGE_TYPE_S8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	memset( (CVI_VOID*)stBgDiffFg.au64VirAddr[0], 0, u16Width * u16Height );

	//malloc pstDiffFg
	s32Result = CVI_CreateIveImage(&stFrmDiffFg,IVE_IMAGE_TYPE_S8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	memset( (CVI_VOID*)stFrmDiffFg.au64VirAddr[0], 0, u16Width * u16Height );

	//malloc pstStatData
	stStatData.u32Size =  sizeof(IVE_BG_STAT_DATA_S) ;
	stStatData.u64VirAddr = (CVI_U64)malloc(stStatData.u32Size);
	memset((CVI_VOID*) stStatData.u64VirAddr, 0, stStatData.u32Size );
	stStatData.u64PhyAddr = (CVI_U64)stStatData.u64VirAddr;

	//
	//init update bg model info, InitUpdateBgModel()
	//
	//malloc pstBgImg
	s32Result = CVI_CreateIveImage(&stBgImg,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	memset( (CVI_VOID*)stBgImg.au64VirAddr[0], 0, u16Width * u16Height );

	//malloc pstChgStaImg
	s32Result = CVI_CreateIveImage(&stChgStaImg,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	memset( (CVI_VOID*)stChgStaImg.au64VirAddr[0], 0, u16Width * u16Height );

	//malloc pstChgStaFg
	s32Result = CVI_CreateIveImage(&stChgStaFg,IVE_IMAGE_TYPE_S8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	memset((CVI_VOID*) stChgStaFg.au64VirAddr[0], 0, u16Width * u16Height );

	//malloc pstChgStaLife
	s32Result = CVI_CreateIveImage(&stChgStaLife,IVE_IMAGE_TYPE_U16C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	memset((CVI_VOID*) stChgStaLife.au64VirAddr[0], 0, sizeof(CVI_U16) * u16Width * u16Height );

	//
	//init Thresh
	//
	//malloc pstFgImg
	s32Result = CVI_CreateIveImage(&stFgImg,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	memset((CVI_VOID*) stFgImg.au64VirAddr[0], 0, u16Width * u16Height );
	//stFgImg.enType = IVE_IMAGE_TYPE_U8C1;
	//stFgImg.au64VirAddr[0] = stBgDiffFg.au64VirAddr[0];
	//stFgImg.au64PhyAddr[0] = stBgDiffFg.au64PhyAddr[0];
	//stFgImg.u32Width = stBgDiffFg.u32Width;
	//stFgImg.u32Height = stBgDiffFg.u32Height;
	//stFgImg.au32Stride[0] = stBgDiffFg.au32Stride[0];


	stMatchBgModelCtrl.u32CurFrmNum = 0;
	stMatchBgModelCtrl.u32PreFrmNum = 0;
	stMatchBgModelCtrl.u16TimeThr = 20;
	stMatchBgModelCtrl.u8DiffThrCrlCoef = 0;
	stMatchBgModelCtrl.u8DiffMaxThr = 10;
	stMatchBgModelCtrl.u8DiffMinThr = 10;
	stMatchBgModelCtrl.u8DiffThrInc = 0;
	stMatchBgModelCtrl.u8FastLearnRate = 4;
	stMatchBgModelCtrl.u8DetChgRegion = 0;

	stUpdateBgModelCtrl.u32CurFrmNum = 0;
	stUpdateBgModelCtrl.u32PreChkTime = 0;
	stUpdateBgModelCtrl.u32FrmChkPeriod = 50;
	stUpdateBgModelCtrl.u32InitMinTime = 25;
	stUpdateBgModelCtrl.u32StyBgMinBlendTime = 100;
	stUpdateBgModelCtrl.u32StyBgMaxBlendTime = 1500;
	stUpdateBgModelCtrl.u32DynBgMinBlendTime = 0;
	stUpdateBgModelCtrl.u32StaticDetMinTime = 80;
	stUpdateBgModelCtrl.u16FgMaxFadeTime = 15;
	stUpdateBgModelCtrl.u16BgMaxFadeTime = 60;
	stUpdateBgModelCtrl.u8StyBgAccTimeRateThr = 80;
	stUpdateBgModelCtrl.u8ChgBgAccTimeRateThr = 60;
	stUpdateBgModelCtrl.u8DynBgAccTimeThr = 0;
	stUpdateBgModelCtrl.u8DynBgDepth = 3;
	stUpdateBgModelCtrl.u8BgEffStaRateThr = 90;
	stUpdateBgModelCtrl.u8AcceBgLearn = 0;
	stUpdateBgModelCtrl.u8DetChgRegion = 0;

	//malloc opencv source
	cvFg	= Mat::zeros(u16Height, u16Width, CV_8UC1);
	cvFgBGR = Mat::zeros(u16Height, u16Width, CV_8UC3);
	cvBg	= Mat::zeros(u16Height, u16Width, CV_8UC1);
	cvBgBGR = Mat::zeros(u16Height, u16Width, CV_8UC3);

	cvGap = 10;
	cvSz.width  = stCurImg.u32Width + stBgImg.u32Width + stFgImg.u32Width  + 4 * cvGap;
	cvSz.height = max(max(stCurImg.u32Height, stBgImg.u32Height), stFgImg.u32Height) + 2 * cvGap;
	cvWte.open(pchRes, CV_FOURCC('X','V','I','D'), cvCap.get(CV_CAP_PROP_FPS), cvSz);

	for(;;)
	{
		vector<Mat *> cvImgVec;
		s32FrmCnt = (CVI_S32)cvCap.get(CV_CAP_PROP_POS_FRAMES);
		u32FrameNum = s32FrmCnt + 1;

		cvCap >> cvImg;
		if( cvImg.empty())
			break;

		if(cvImg.rows != (int)stCurImg.u32Height || cvImg.cols != (int)stCurImg.u32Width)
		{
			printf("Size not match, quit\n");
			break;
		}

		cvtColor(cvImg, cvImgGray, CV_BGR2GRAY);
		mat2IveImg(&cvImgGray, &stCurImg, CVGRAY_IVEU8C1);

		stMatchBgModelCtrl.u32PreFrmNum = stMatchBgModelCtrl.u32CurFrmNum;
		stMatchBgModelCtrl.u32CurFrmNum = u32FrameNum;
		s32Result = CVI_MPI_IVE_MatchBgModel( &hIveHandle, &stCurImg, &stBgModel,
				&stFgFlag, &stBgDiffFg, &stFrmDiffFg, &stStatData, &stMatchBgModelCtrl, bInstant);
		CVI_CHECK_NET_GOTO(s32Result,CVI_SUCCESS,FAILURE);
		if( u32UpdCnt==0 || u32FrameNum>=u32PreUpdTime+u32FrmUpdPeriod )
		{
			u32UpdCnt++;
			u32PreUpdTime = u32FrameNum;
			stUpdateBgModelCtrl.u32CurFrmNum = u32FrameNum;
			stUpdateBgModelCtrl.u32PreChkTime = u32PreChkTime;
			stUpdateBgModelCtrl.u32FrmChkPeriod = 0;
			if( u32FrameNum>=u32PreChkTime+u32FrmChkPeriod )
			{
				stUpdateBgModelCtrl.u32FrmChkPeriod = u32FrmChkPeriod;
				u32PreChkTime = u32FrameNum;
			}

			s32Result = CVI_MPI_IVE_UpdateBgModel( &hIveHandle, &stBgModel, &stFgFlag,
					&stBgImg, &stChgStaImg, &stChgStaFg, &stChgStaLife,
					&stStatData, &stUpdateBgModelCtrl, bInstant);
			CVI_CHECK_NET_GOTO(s32Result,CVI_SUCCESS,FAILURE);
		}

		//extract fg
		CVI_MPI_IVE_Thresh(&hIveHandle, &stBgDiffFg, &stFgImg, &stThrCtrl, bInstant);

		if (compare && (s32FrmCnt == 0 || s32FrmCnt == 1 || s32FrmCnt == 512))
		{
			CVI_CHAR fileName[_MAX_FNAME];

			snprintf(fileName, _MAX_FNAME, "../data/result/sample_BgModel_Sample2_bg_%d.yuv", s32FrmCnt);
			s32CompareError += CompareImage(&stBgImg, fileName);

			snprintf(fileName, _MAX_FNAME, "../data/result/sample_BgModel_Sample2_fg_%d.yuv", s32FrmCnt);
			s32CompareError += CompareImage(&stFgImg, fileName);
		}

		if (show)
		{
			//show result image
			iveImage2Mat(&stBgImg, &cvBg, IVEU8C1_CVGRAY);
			iveImage2Mat(&stFgImg, &cvFg, IVEU8C1_CVGRAY);
			cvtColor(cvBg, cvBgBGR, CV_GRAY2BGR);
			cvtColor(cvFg, cvFgBGR, CV_GRAY2BGR);

			putText(cvImg, "srcImg", Point(5,15), FONT_HERSHEY_COMPLEX_SMALL, 1, CV_RGB(255,0,0));
			cvImgVec.push_back(&cvImg);

			putText(cvBgBGR, "bgImg", Point(5,15), FONT_HERSHEY_COMPLEX_SMALL, 1, CV_RGB(255,0,0));
			cvImgVec.push_back(&cvBgBGR);

			putText(cvFgBGR, "fgImg", Point(5,15), FONT_HERSHEY_COMPLEX_SMALL, 1, CV_RGB(255,0,0));
			cvImgVec.push_back(&cvFgBGR);

			cvShowMultiImg("BgModel_Sample2", 1, 3, cvImgVec, cvGap, &cvDispImg);
			waitKey(10);

			cvWte.write(cvDispImg);
		}
	}

FAILURE:
	CVI_DestroyIveImage(&stCurImg);
	CVI_DestroyIveImage(&stFgFlag);
	CVI_DestroyIveImage(&stBgDiffFg);
	CVI_DestroyIveImage(&stFrmDiffFg);
	CVI_DestroyIveImage(&stBgImg);
	CVI_DestroyIveImage(&stChgStaImg);
	CVI_DestroyIveImage(&stChgStaFg);
	CVI_DestroyIveImage(&stChgStaLife);
	CVI_DestroyIveImage(&stFgImg);
	if (stBgModel.u64VirAddr)
		CVI_FREE_64(stBgModel.u64VirAddr);
	if (stStatData.u64VirAddr)
		CVI_FREE_64(stStatData.u64VirAddr);

	if (show)
		cvDestroyAllWindows();
	return (s32CompareError == 0) ? s32Result : CVI_FAILURE;
#else
    UNUSED(show);
    UNUSED(compare);
	return CVI_FAILURE;
#endif
}
