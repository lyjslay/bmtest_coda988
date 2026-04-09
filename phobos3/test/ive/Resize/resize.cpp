//#define useOpenCV (1)
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include "resize.h"
#include "sample_define.h"
#include "sample_assist.h"
#include "sample_file.h"
#include "mpi_ive.h"

#include <stdlib.h>     /* malloc, free, rand */
#include <string.h> /* memset */
#include <fstream> // std::ofstream
#include <sstream>      // std::ostringstream
using namespace std;


#ifdef useOpenCV
#include <cv.h>
#include <highgui.h>
#include <opencv2/opencv.hpp>
using namespace cv;
#endif

typedef struct IVE_IMAGE_FILL_U8C3_PLANAR_S
{
	CVI_U8  u8Type;
	CVI_U32  u32SrcPhyAddr[3];
	CVI_U32  u32DstPhyAddr[3];
	CVI_U16  u16SrcStride[3];
	CVI_U16  u16DstStride[3];
	CVI_U16  u16SrcWidth;
	CVI_U16  u16SrcHeight;
	CVI_U16  u16DstWidth;
	CVI_U16  u16DstHeight;
	CVI_U16  u16XScale;
	CVI_U16  u16YScale;
}IVE_IMAGE_FILL_U8C3_PLANAR_S;

static CVI_S32 Resize_Sample(int show, int compare,
        IVE_RESIZE_MODE_E enMode,
        const CVI_CHAR *fileName1,
        const CVI_CHAR *fileName2,
        CVI_U16 au16ResizeWidth[],
        CVI_U16 au16ResizeHeight[])
{
	IVE_HANDLE  hIveHnd;
	IVE_SRC_IMAGE_S astSrc[2];
	IVE_DST_IMAGE_S astDst[2];
	IVE_DST_IMAGE_S astRef[2];
	IVE_RESIZE_CTRL_S stCtrl;
	CVI_BOOL bInstant = CVI_TRUE;
	CVI_S32 s32Ret = CVI_FAILURE, s32Key;
	CVI_U32 i, j;
	FILE *fp1=NULL, *fp2=NULL;
#ifdef useOpenCV
    IplImage *pstColorImg = CVI_NULL, *pstGrayImg = CVI_NULL;
#endif

	FILE *fpInput = NULL;
	IVE_DST_MEM_INFO_S stInput[2];
    CVI_U16 u16Height = 576;
    CVI_U16 u16Width = 720;
    CVI_U16 widthStep = u16Width;
	CVI_CHAR resultImgName[_MAX_FNAME];
    CVI_U8* inputData[] = {NULL, NULL};

    stInput[0].u64VirAddr = CVI_NULL;
    stInput[1].u64VirAddr = CVI_NULL;

    UNUSED(resultImgName);
    UNUSED(fpInput);
    UNUSED(inputData);
    UNUSED(stInput);
    UNUSED(s32Key);

#ifdef useOpenCV
	//1. Load Image use opencv cvLoadImage function
	pstColorImg = cvLoadImage("../data/img/shitomasi/penguin.jpg", CV_LOAD_IMAGE_COLOR);
	CVI_ASSERT(CVI_NULL != pstColorImg);
    // fake for avoid unused-label warning
    CVI_CHECK_NET_GOTO(0,0,FAILURE);
	pstGrayImg= cvLoadImage("../data/img/shitomasi/penguin.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	CVI_ASSERT(CVI_NULL != pstGrayImg);
    printf("use opencv\n");
    if (0)
    {
        // debug use
        std::ostringstream filename;
        std::ofstream outfile;
        Mat cvImg;
        filename.str("");
        filename << "../data/img/shitomasi/penguin.rgb.raw";
        outfile.open (filename.str().c_str(), ios::out | ios::binary);
        cvImg = imread("../data/img/shitomasi/penguin.jpg");
        printf("penguin.rgb.raw size %d, depth %d\n", (int)cvImg.total(),
                cvImg.channels());
        outfile.write ((char*)(cvImg.data), u16Height * u16Width * 3);  // In byte so frame.total() should be enough ?
        outfile.close();

        filename.str("");
        filename << "../data/img/shitomasi/penguin.gray.raw";
        outfile.open (filename.str().c_str(), ios::out | ios::binary);
        cvImg = imread("../data/img/shitomasi/penguin.jpg", IMREAD_GRAYSCALE);
        printf("penguin..raw size %d, depth %d\n", (int)cvImg.total(), cvImg.channels());
        outfile.write ((char*)(cvImg.data), u16Height * u16Width);  // In byte so frame.total() should be enough ?
        outfile.close();
    }
    assert(pstColorImg->width == pstGrayImg->width);
    assert(pstColorImg->height == pstGrayImg->height);
    assert(pstColorImg->width == u16Width);
    assert(pstColorImg->height == u16Height);
    u16Height = pstColorImg->height;
    u16Width = pstColorImg->width;
    inputData[0] = (CVI_U8* )pstColorImg->imageData;
    inputData[1] = (CVI_U8* )pstGrayImg->imageData;
#else
    // rgb
    stInput[0].u32Size = u16Height * u16Width * 3;
    stInput[0].u64VirAddr =  (CVI_U64)malloc(stInput[0].u32Size);
    stInput[0].u64PhyAddr = (CVI_U64)stInput[0].u64VirAddr;
    // gray
    stInput[1].u32Size = u16Height * u16Width;
    stInput[1].u64VirAddr =  (CVI_U64)malloc(stInput[1].u32Size);
    stInput[1].u64PhyAddr = (CVI_U64)stInput[1].u64VirAddr;
    inputData[0] = (CVI_U8* )stInput[0].u64VirAddr;
    inputData[1] = (CVI_U8* )stInput[1].u64VirAddr;

    snprintf(resultImgName, _MAX_FNAME, "../data/img/shitomasi/penguin.rgb.raw");
    s32Ret = fopen_s(&fpInput, resultImgName,"rb");
    CVI_CHECK_NET_GOTO(s32Ret,0,FAILURE);
    s32Ret = CVI_MemReadFile(&stInput[0],fpInput);
    CVI_CHECK_NET_GOTO(s32Ret, 0, FAILURE);
    CVI_FCLOSE(fpInput);

    snprintf(resultImgName, _MAX_FNAME, "../data/img/shitomasi/penguin.gray.raw");
    s32Ret = fopen_s(&fpInput, resultImgName,"rb");
    CVI_CHECK_NET_GOTO(s32Ret,0,FAILURE);
    s32Ret = CVI_MemReadFile(&stInput[1],fpInput);
    CVI_CHECK_NET_GOTO(s32Ret, 0, FAILURE);
    CVI_FCLOSE(fpInput);
#endif

	//2.set src
	CVI_CreateIveImage(&astSrc[0], IVE_IMAGE_TYPE_U8C3_PLANAR, u16Width, u16Height);
	CVI_CreateIveImage(&astSrc[1], IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);

	//Change the image format from opencv rgb package to rgb planner
	for(i=0; i<astSrc[0].u32Height;i++)
	{
		for(j=0; j<astSrc[0].u32Width; j++)
		{
			*((CVI_U8*)astSrc[0].au64VirAddr[0]+ i * astSrc[0].au32Stride[0]+j) = inputData[0][i* widthStep*3 + 3* j];
			*((CVI_U8*)astSrc[0].au64VirAddr[1]+ i * astSrc[0].au32Stride[1]+j) = inputData[0][i* widthStep*3 + 3* j +1];
			*((CVI_U8*)astSrc[0].au64VirAddr[2]+ i * astSrc[0].au32Stride[2]+j) = inputData[0][i* widthStep*3 + 3* j +2];
		}
	}

	for(i=0; i<astSrc[1].u32Height; i++)
	{
		memcpy((CVI_U8*)astSrc[1].au64VirAddr[0]+ i* astSrc[1].au32Stride[0],
			inputData[1] + i* widthStep, u16Width);
	}

	//3. set dst

	CVI_CreateIveImage(&astDst[0], IVE_IMAGE_TYPE_U8C3_PLANAR,  au16ResizeWidth[0], au16ResizeHeight[0]);
	CVI_CreateIveImage(&astDst[1], IVE_IMAGE_TYPE_U8C1, au16ResizeWidth[1], au16ResizeHeight[1] );
	CVI_CreateIveImage(&astRef[0], IVE_IMAGE_TYPE_U8C3_PLANAR,  au16ResizeWidth[0], au16ResizeHeight[0]);
	CVI_CreateIveImage(&astRef[1], IVE_IMAGE_TYPE_U8C1, au16ResizeWidth[1], au16ResizeHeight[1] );

	//4.set ctrl
	stCtrl.u16Num=2;
	stCtrl.stMem.u32Size = stCtrl.u16Num * sizeof(IVE_IMAGE_FILL_U8C3_PLANAR_S)*2;
	stCtrl.stMem.u64VirAddr = (CVI_U64)malloc(stCtrl.stMem.u32Size);
	stCtrl.stMem.u64PhyAddr = (CVI_U64)stCtrl.stMem.u64VirAddr;
	stCtrl.enMode = enMode;

	//5.resize
	s32Ret = CVI_MPI_IVE_Resize(&hIveHnd, astSrc, astDst, &stCtrl, bInstant);
	CVI_ASSERT(CVI_SUCCESS == s32Ret);

	//6. check result
	if (compare)
	{
		CVI_S32 s32RetItem;

		s32Ret = fopen_s(&fp1,fileName1,"rb");
		s32Ret = fopen_s(&fp2,fileName2,"rb");

		s32RetItem = CVI_ReadFile(&astRef[0],fp1);
		CVI_ASSERT(CVI_SUCCESS == s32Ret);
		s32RetItem = CVI_ReadFile(&astRef[1],fp2);
		CVI_ASSERT(CVI_SUCCESS == s32Ret);

		s32Ret = CVI_SUCCESS;
		s32RetItem = CVI_CompareIveImage(&astDst[0],&astRef[0]);
		if (s32RetItem != CVI_SUCCESS) {
			printf("astDst[0] compare failed\n");
			s32Ret = CVI_FAILURE;
		}
		s32RetItem = CVI_CompareIveImage(&astDst[1],&astRef[1]);
		if (s32RetItem != CVI_SUCCESS) {
			printf("astDst[1] compare failed\n");
			s32Ret = CVI_FAILURE;
		}

		CVI_FCLOSE(fp1);
		CVI_FCLOSE(fp2);
	}

	if (show)
    {
#ifdef useOpenCV
        //show resize result(option)
        IplImage* pstDstColorImg= cvCreateImage(cvSize(au16ResizeWidth[0], au16ResizeHeight[0]), pstColorImg->depth, pstColorImg->nChannels);
        IplImage* pstDstGrayImg= cvCreateImage(cvSize(au16ResizeWidth[1], au16ResizeHeight[1]), pstGrayImg->depth, pstGrayImg->nChannels);
        for(i=0; i<astDst[0].u32Height;i++)
        {
            for(j=0; j<astDst[0].u32Width; j++)
            {
                pstDstColorImg->imageData[i* pstDstColorImg->widthStep + 3* j]=*((CVI_U8*)astDst[0].au64VirAddr[0]+ i * astDst[0].au32Stride[0]+j);
                pstDstColorImg->imageData[i* pstDstColorImg->widthStep + 3* j +1]=*((CVI_U8*)astDst[0].au64VirAddr[1]+ i * astDst[0].au32Stride[1]+j);
                pstDstColorImg->imageData[i* pstDstColorImg->widthStep + 3* j +2]=*((CVI_U8*)astDst[0].au64VirAddr[2]+ i * astDst[0].au32Stride[2]+j);
            }
        }

        for(i=0; i<astDst[1].u32Height; i++)
        {
            memcpy(pstDstGrayImg->imageData + i* pstDstGrayImg->widthStep,
                    (CVI_U8*)astDst[1].au64VirAddr[0]+ i* astDst[1].au32Stride[0], pstDstGrayImg->width);
        }

        cvNamedWindow("Orgin image1");
        cvNamedWindow("Orgin image2");
        cvNamedWindow("BilinearResized image1");
        cvNamedWindow("BilinearResized image2");

        cvShowImage("Orgin image1",pstColorImg);
        cvShowImage("Orgin image2",pstGrayImg);
        cvShowImage("BilinearResized image1",pstDstColorImg);
        cvShowImage("BilinearResized image2",pstDstGrayImg);
        //press ��q�� key to exit  image show
        s32Key = cvWaitKey(0);
        if ( s32Key == 'q')
        {
            cvDestroyAllWindows();
        }

        cvReleaseImage(&pstDstColorImg);
        cvReleaseImage(&pstDstGrayImg);
#endif
    }

FAILURE:
	//6. release
#ifdef useOpenCV
    if (pstColorImg != CVI_NULL)
        cvReleaseImage(&pstColorImg);
    if (pstGrayImg != CVI_NULL)
        cvReleaseImage(&pstGrayImg);
#endif

    CVI_DestroyIveImage(&astSrc[0]);
    CVI_DestroyIveImage(&astSrc[1]);
    CVI_DestroyIveImage(&astDst[0]);
    CVI_DestroyIveImage(&astDst[1]);
    CVI_FREE_64(stCtrl.stMem.u64VirAddr);
    for (int i = 0; i < 2; i++) {
        if (stInput[i].u64VirAddr != CVI_NULL)
            CVI_FREE_64(stInput[i].u64VirAddr);
    }

	return s32Ret;
}

CVI_S32 Resize_Sample_Bilinear(int show, int compare) {
    CVI_U16 au16ResizeWidth[2];
    CVI_U16 au16ResizeHeight[2];
    CVI_U16 u16Height = 576;
    CVI_U16 u16Width = 720;

    au16ResizeWidth[0] = u16Width/2;
    au16ResizeHeight[0] = u16Height/3;

    au16ResizeWidth[1] = u16Width/3;
    au16ResizeHeight[1] = u16Height/2;

#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Resize_Bilinear_rgb_rgb_start;
    const CVI_CHAR *fileName1 = &_binary_test_ive_data_result_sample_Resize_Bilinear_rgb_rgb_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_Resize_Bilinear_gray_yuv_start;
    const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_Resize_Bilinear_gray_yuv_start;

#else
    const CVI_CHAR *fileName1 = "../data/result/sample_Resize_Bilinear_rgb.rgb";
    const CVI_CHAR *fileName2 = "../data/result/sample_Resize_Bilinear_gray.yuv";

#endif
    return Resize_Sample(show, compare,
            IVE_RESIZE_MODE_LINEAR,
            fileName1, fileName2,
            au16ResizeWidth, au16ResizeHeight);
}

CVI_S32 Resize_Sample_Area(int show, int compare)
{
    CVI_U16 au16ResizeWidth[2];
    CVI_U16 au16ResizeHeight[2];
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_result_sample_Resize_Area_rgb_rgb_start;
    const CVI_CHAR *fileName1 = &_binary_test_ive_data_result_sample_Resize_Area_rgb_rgb_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_Resize_Area_gray_yuv_start;
    const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_Resize_Area_gray_yuv_start;

#else
    const CVI_CHAR *fileName1 = "../data/result/sample_Resize_Area_rgb.rgb";
    const CVI_CHAR *fileName2 = "../data/result/sample_Resize_Area_gray.yuv";

#endif
    CVI_U16 u16Height = 576;
    CVI_U16 u16Width = 720;

    au16ResizeWidth[0] = u16Width/3;
    au16ResizeHeight[0] = u16Height/2;

    au16ResizeWidth[1] = u16Width/2;
    au16ResizeHeight[1] = u16Height/3;
    return Resize_Sample(show, compare, IVE_RESIZE_MODE_AREA,
            fileName1, fileName2,
            au16ResizeWidth, au16ResizeHeight);
}
