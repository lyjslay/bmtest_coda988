#include "FilterAndCSC.h"
#include "sample_assist.h"
#include "sample_file.h"
#include "sample_define.h"
#include "mpi_ive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CVI_S32 FilterAndCSCSample(IVE_IMAGE_TYPE_E enSrcType,IVE_CSC_MODE_E enMode,IVE_IMAGE_TYPE_E enDstType,CVI_S8 *ps8Mask,CVI_U8 u8Norm,const CVI_CHAR *fileName,const CVI_CHAR *fileName2)
{
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_DST_IMAGE_S stRef;
	IVE_FILTER_AND_CSC_CTRL_S stCtrl;
	CVI_S32 s32Result;
	CVI_U16 u16Width;
	CVI_U16 u16Height;
	FILE *fp;
	FILE *fp2;
	IVE_HANDLE handle;
	CVI_DOUBLE dTime;


	memset(&stSrc,0,sizeof(IVE_SRC_IMAGE_S));
	memset(&stDst,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stRef,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stCtrl,0,sizeof(IVE_FILTER_AND_CSC_CTRL_S));

	fp = NULL;
	fp2 = NULL;
	u16Width = CVI_IMAGE_WIDTH_352;
	u16Height = CVI_IMAGE_HEIGHT_288;

	s32Result = CVI_CreateIveImage(&stSrc,enSrcType,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stDst,enDstType,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stRef,enDstType,u16Width,u16Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	s32Result = fopen_s(&fp,fileName,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
	s32Result = fopen_s(&fp2,fileName2,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);

	s32Result = CVI_ReadFile(&stSrc,fp);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_ReadFile(&stRef,fp2);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	stCtrl.enMode = enMode;
	memcpy(stCtrl.as8Mask,ps8Mask,sizeof(CVI_S8) * 25);
	stCtrl.u8Norm = u8Norm;

	dTime = (CVI_DOUBLE)CVI_GetTickCount();
	s32Result = CVI_MPI_IVE_FilterAndCSC(&handle,&stSrc,&stDst,&stCtrl,CVI_FALSE);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	dTime = (CVI_DOUBLE)CVI_GetTickCount() - dTime;

	printf("FilterAndCSC mode %d,time = %g ms\n",enMode, dTime/((CVI_DOUBLE)CVI_GetTickFrequency()*1000.));

	s32Result = CVI_CompareIveImage(&stDst,&stRef);

FAILURE:

	CVI_FCLOSE(fp);
	CVI_FCLOSE(fp2);
	CVI_DestroyIveImage(&stSrc);
	CVI_DestroyIveImage(&stDst);
	CVI_DestroyIveImage(&stRef);

	return s32Result;
}

CVI_S32 FilterAndCSCSample420SPToVideoPackage3x3(CVI_VOID)
{
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_sp420_00_352x288_SP420_yuv_start;
	const CVI_CHAR *fileName = &_binary_test_ive_data_sp420_00_352x288_SP420_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_FilterAndCSC_420SPToVideoPackage3x3_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_FilterAndCSC_420SPToVideoPackage3x3_yuv_start;

#else
	const CVI_CHAR *fileName = "../data/sp420/00_352x288_SP420.yuv";
	const CVI_CHAR *fileName2 = "../data/result/sample_FilterAndCSC_420SPToVideoPackage3x3.yuv";

#endif
	CVI_S8 mask[25]={0,0,0,0,0,
		0,1,2,1,0,
		0,2,4,2,0,
		0,1,2,1,0,
		0,0,0,0,0};

	return FilterAndCSCSample(IVE_IMAGE_TYPE_YUV420SP,IVE_CSC_MODE_VIDEO_BT601_YUV2RGB,IVE_IMAGE_TYPE_U8C3_PACKAGE,mask,4,fileName,fileName2);
}
CVI_S32 FilterAndCSCSample420SPToVideoPackage5x5(CVI_VOID)
{
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_sp420_00_352x288_SP420_yuv_start;
	const CVI_CHAR *fileName = &_binary_test_ive_data_sp420_00_352x288_SP420_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_FilterAndCSC_420SPToVideoPackage5x5_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_FilterAndCSC_420SPToVideoPackage5x5_yuv_start;

#else
	const CVI_CHAR *fileName = "../data/sp420/00_352x288_SP420.yuv";
	const CVI_CHAR *fileName2 = "../data/result/sample_FilterAndCSC_420SPToVideoPackage5x5.yuv";

#endif
	CVI_S8 mask[25]={1,2,3,2,1,
					2,5,6,5,2,
					3,6,8,6,3,
					2,5,6,5,2,
					1,2,3,2,1};

	return FilterAndCSCSample(IVE_IMAGE_TYPE_YUV420SP,IVE_CSC_MODE_VIDEO_BT601_YUV2RGB,IVE_IMAGE_TYPE_U8C3_PACKAGE,mask,7,fileName,fileName2);
}
CVI_S32 FilterAndCSCSample420SPToVideoPlanar3x3(CVI_VOID)
{
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_sp420_00_352x288_SP420_yuv_start;
	const CVI_CHAR *fileName = &_binary_test_ive_data_sp420_00_352x288_SP420_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_FilterAndCSC_420SPToVideoPlanar3x3_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_FilterAndCSC_420SPToVideoPlanar3x3_yuv_start;

#else
	const CVI_CHAR *fileName = "../data/sp420/00_352x288_SP420.yuv";
	const CVI_CHAR *fileName2 = "../data/result/sample_FilterAndCSC_420SPToVideoPlanar3x3.yuv";

#endif
	CVI_S8 mask[25]={0,0,0,0,0,
		0,1,2,1,0,
		0,2,4,2,0,
		0,1,2,1,0,
		0,0,0,0,0};

	return FilterAndCSCSample(IVE_IMAGE_TYPE_YUV420SP,IVE_CSC_MODE_VIDEO_BT601_YUV2RGB,IVE_IMAGE_TYPE_U8C3_PLANAR,mask,4,fileName,fileName2);
}
CVI_S32 FilterAndCSCSample420SPToVideoPlanar5x5(CVI_VOID)
{
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_sp420_00_352x288_SP420_yuv_start;
	const CVI_CHAR *fileName = &_binary_test_ive_data_sp420_00_352x288_SP420_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_FilterAndCSC_420SPToVideoPlanar5x5_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_FilterAndCSC_420SPToVideoPlanar5x5_yuv_start;

#else
	const CVI_CHAR *fileName = "../data/sp420/00_352x288_SP420.yuv";
	const CVI_CHAR *fileName2 = "../data/result/sample_FilterAndCSC_420SPToVideoPlanar5x5.yuv";

#endif
	CVI_S8 mask[25]={1,2,3,2,1,
		2,5,6,5,2,
		3,6,8,6,3,
		2,5,6,5,2,
		1,2,3,2,1};

	return FilterAndCSCSample(IVE_IMAGE_TYPE_YUV420SP,IVE_CSC_MODE_VIDEO_BT601_YUV2RGB,IVE_IMAGE_TYPE_U8C3_PLANAR,mask,7,fileName,fileName2);
}
CVI_S32 FilterAndCSCSample422SPToVideoPackage3x3(CVI_VOID)
{
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_sp422_00_352x288_SP422_yuv_start;
	const CVI_CHAR *fileName = &_binary_test_ive_data_sp422_00_352x288_SP422_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_FilterAndCSC_422SPToVideoPackage3x3_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_FilterAndCSC_422SPToVideoPackage3x3_yuv_start;

#else
	const CVI_CHAR *fileName = "../data/sp422/00_352x288_SP422.yuv";
	const CVI_CHAR *fileName2 = "../data/result/sample_FilterAndCSC_422SPToVideoPackage3x3.yuv";

#endif
	CVI_S8 mask[25]={0,0,0,0,0,
		0,1,2,1,0,
		0,2,4,2,0,
		0,1,2,1,0,
		0,0,0,0,0};

	return FilterAndCSCSample(IVE_IMAGE_TYPE_YUV422SP,IVE_CSC_MODE_VIDEO_BT601_YUV2RGB,IVE_IMAGE_TYPE_U8C3_PACKAGE,mask,4,fileName,fileName2);
}
CVI_S32 FilterAndCSCSample422SPToVideoPackage5x5(CVI_VOID)
{
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_sp422_00_352x288_SP422_yuv_start;
	const CVI_CHAR *fileName = &_binary_test_ive_data_sp422_00_352x288_SP422_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_FilterAndCSC_422SPToVideoPackage5x5_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_FilterAndCSC_422SPToVideoPackage5x5_yuv_start;

#else
	const CVI_CHAR *fileName = "../data/sp422/00_352x288_SP422.yuv";
	const CVI_CHAR *fileName2 = "../data/result/sample_FilterAndCSC_422SPToVideoPackage5x5.yuv";

#endif
	CVI_S8 mask[25]={1,2,3,2,1,
					2,5,6,5,2,
					3,6,8,6,3,
					2,5,6,5,2,
					1,2,3,2,1};

	return FilterAndCSCSample(IVE_IMAGE_TYPE_YUV422SP,IVE_CSC_MODE_VIDEO_BT601_YUV2RGB,IVE_IMAGE_TYPE_U8C3_PACKAGE,mask,7,fileName,fileName2);
}
CVI_S32 FilterAndCSCSample422SPToVideoPlanar3x3(CVI_VOID)
{
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_sp422_00_352x288_SP422_yuv_start;
	const CVI_CHAR *fileName = &_binary_test_ive_data_sp422_00_352x288_SP422_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_FilterAndCSC_422SPToVideoPlanar3x3_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_FilterAndCSC_422SPToVideoPlanar3x3_yuv_start;

#else
	const CVI_CHAR *fileName = "../data/sp422/00_352x288_SP422.yuv";
	const CVI_CHAR *fileName2 = "../data/result/sample_FilterAndCSC_422SPToVideoPlanar3x3.yuv";

#endif
	CVI_S8 mask[25]={0,0,0,0,0,
		0,1,2,1,0,
		0,2,4,2,0,
		0,1,2,1,0,
		0,0,0,0,0};

	return FilterAndCSCSample(IVE_IMAGE_TYPE_YUV422SP,IVE_CSC_MODE_VIDEO_BT601_YUV2RGB,IVE_IMAGE_TYPE_U8C3_PLANAR,mask,4,fileName,fileName2);
}
CVI_S32 FilterAndCSCSample422SPToVideoPlanar5x5(CVI_VOID)
{
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_sp422_00_352x288_SP422_yuv_start;
	const CVI_CHAR *fileName = &_binary_test_ive_data_sp422_00_352x288_SP422_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_FilterAndCSC_422SPToVideoPlanar5x5_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_FilterAndCSC_422SPToVideoPlanar5x5_yuv_start;

#else
	const CVI_CHAR *fileName = "../data/sp422/00_352x288_SP422.yuv";
	const CVI_CHAR *fileName2 = "../data/result/sample_FilterAndCSC_422SPToVideoPlanar5x5.yuv";

#endif
	CVI_S8 mask[25]={1,2,3,2,1,
		2,5,6,5,2,
		3,6,8,6,3,
		2,5,6,5,2,
		1,2,3,2,1};

	return FilterAndCSCSample(IVE_IMAGE_TYPE_YUV422SP,IVE_CSC_MODE_VIDEO_BT601_YUV2RGB,IVE_IMAGE_TYPE_U8C3_PLANAR,mask,7,fileName,fileName2);
}

CVI_S32 FilterAndCSCSample420SPToPicPackage3x3(CVI_VOID)
{
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_sp420_00_352x288_SP420_yuv_start;
	const CVI_CHAR *fileName = &_binary_test_ive_data_sp420_00_352x288_SP420_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_FilterAndCSC_420SPToPicPackage3x3_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_FilterAndCSC_420SPToPicPackage3x3_yuv_start;

#else
	const CVI_CHAR *fileName = "../data/sp420/00_352x288_SP420.yuv";
	const CVI_CHAR *fileName2 = "../data/result/sample_FilterAndCSC_420SPToPicPackage3x3.yuv";

#endif
	CVI_S8 mask[25]={0,0,0,0,0,
		0,1,2,1,0,
		0,2,4,2,0,
		0,1,2,1,0,
		0,0,0,0,0};

	return FilterAndCSCSample(IVE_IMAGE_TYPE_YUV420SP,IVE_CSC_MODE_PIC_BT601_YUV2RGB,IVE_IMAGE_TYPE_U8C3_PACKAGE,mask,4,fileName,fileName2);
}
CVI_S32 FilterAndCSCSample420SPToPicPackage5x5(CVI_VOID)
{
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_sp420_00_352x288_SP420_yuv_start;
	const CVI_CHAR *fileName = &_binary_test_ive_data_sp420_00_352x288_SP420_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_FilterAndCSC_420SPToPicPackage5x5_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_FilterAndCSC_420SPToPicPackage5x5_yuv_start;

#else
	const CVI_CHAR *fileName = "../data/sp420/00_352x288_SP420.yuv";
	const CVI_CHAR *fileName2 = "../data/result/sample_FilterAndCSC_420SPToPicPackage5x5.yuv";

#endif
	CVI_S8 mask[25]={1,2,3,2,1,
		2,5,6,5,2,
		3,6,8,6,3,
		2,5,6,5,2,
		1,2,3,2,1};

	return FilterAndCSCSample(IVE_IMAGE_TYPE_YUV420SP,IVE_CSC_MODE_PIC_BT601_YUV2RGB,IVE_IMAGE_TYPE_U8C3_PACKAGE,mask,7,fileName,fileName2);
}
CVI_S32 FilterAndCSCSample420SPToPicPlanar3x3(CVI_VOID)
{
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_sp420_00_352x288_SP420_yuv_start;
	const CVI_CHAR *fileName = &_binary_test_ive_data_sp420_00_352x288_SP420_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_FilterAndCSC_420SPToPicPlanar3x3_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_FilterAndCSC_420SPToPicPlanar3x3_yuv_start;

#else
	const CVI_CHAR *fileName = "../data/sp420/00_352x288_SP420.yuv";
	const CVI_CHAR *fileName2 = "../data/result/sample_FilterAndCSC_420SPToPicPlanar3x3.yuv";

#endif
	CVI_S8 mask[25]={0,0,0,0,0,
		0,1,2,1,0,
		0,2,4,2,0,
		0,1,2,1,0,
		0,0,0,0,0};

	return FilterAndCSCSample(IVE_IMAGE_TYPE_YUV420SP,IVE_CSC_MODE_PIC_BT601_YUV2RGB,IVE_IMAGE_TYPE_U8C3_PLANAR,mask,4,fileName,fileName2);
}
CVI_S32 FilterAndCSCSample420SPToPicPlanar5x5(CVI_VOID)
{
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_sp420_00_352x288_SP420_yuv_start;
	const CVI_CHAR *fileName = &_binary_test_ive_data_sp420_00_352x288_SP420_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_FilterAndCSC_420SPToPicPlanar5x5_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_FilterAndCSC_420SPToPicPlanar5x5_yuv_start;

#else
	const CVI_CHAR *fileName = "../data/sp420/00_352x288_SP420.yuv";
	const CVI_CHAR *fileName2 = "../data/result/sample_FilterAndCSC_420SPToPicPlanar5x5.yuv";

#endif
	CVI_S8 mask[25]={1,2,3,2,1,
		2,5,6,5,2,
		3,6,8,6,3,
		2,5,6,5,2,
		1,2,3,2,1};

	return FilterAndCSCSample(IVE_IMAGE_TYPE_YUV420SP,IVE_CSC_MODE_PIC_BT601_YUV2RGB,IVE_IMAGE_TYPE_U8C3_PLANAR,mask,7,fileName,fileName2);
}
CVI_S32 FilterAndCSCSample422SPToPicPackage3x3(CVI_VOID)
{
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_sp422_00_352x288_SP422_yuv_start;
	const CVI_CHAR *fileName = &_binary_test_ive_data_sp422_00_352x288_SP422_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_FilterAndCSC_422SPToPicPackage3x3_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_FilterAndCSC_422SPToPicPackage3x3_yuv_start;

#else
	const CVI_CHAR *fileName = "../data/sp422/00_352x288_SP422.yuv";
	const CVI_CHAR *fileName2 = "../data/result/sample_FilterAndCSC_422SPToPicPackage3x3.yuv";

#endif
	CVI_S8 mask[25]={0,0,0,0,0,
		0,1,2,1,0,
		0,2,4,2,0,
		0,1,2,1,0,
		0,0,0,0,0};

	return FilterAndCSCSample(IVE_IMAGE_TYPE_YUV422SP,IVE_CSC_MODE_PIC_BT601_YUV2RGB,IVE_IMAGE_TYPE_U8C3_PACKAGE,mask,4,fileName,fileName2);
}
CVI_S32 FilterAndCSCSample422SPToPicPackage5x5(CVI_VOID)
{
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_sp422_00_352x288_SP422_yuv_start;
	const CVI_CHAR *fileName = &_binary_test_ive_data_sp422_00_352x288_SP422_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_FilterAndCSC_422SPToPicPackage5x5_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_FilterAndCSC_422SPToPicPackage5x5_yuv_start;

#else
	const CVI_CHAR *fileName = "../data/sp422/00_352x288_SP422.yuv";
	const CVI_CHAR *fileName2 = "../data/result/sample_FilterAndCSC_422SPToPicPackage5x5.yuv";

#endif
	CVI_S8 mask[25]={1,2,3,2,1,
		2,5,6,5,2,
		3,6,8,6,3,
		2,5,6,5,2,
		1,2,3,2,1};

	return FilterAndCSCSample(IVE_IMAGE_TYPE_YUV422SP,IVE_CSC_MODE_PIC_BT601_YUV2RGB,IVE_IMAGE_TYPE_U8C3_PACKAGE,mask,7,fileName,fileName2);
}
CVI_S32 FilterAndCSCSample422SPToPicPlanar3x3(CVI_VOID)
{
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_sp422_00_352x288_SP422_yuv_start;
	const CVI_CHAR *fileName = &_binary_test_ive_data_sp422_00_352x288_SP422_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_FilterAndCSC_422SPToPicPlanar3x3_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_FilterAndCSC_422SPToPicPlanar3x3_yuv_start;

#else
	const CVI_CHAR *fileName = "../data/sp422/00_352x288_SP422.yuv";
	const CVI_CHAR *fileName2 = "../data/result/sample_FilterAndCSC_422SPToPicPlanar3x3.yuv";

#endif
	CVI_S8 mask[25]={0,0,0,0,0,
		0,1,2,1,0,
		0,2,4,2,0,
		0,1,2,1,0,
		0,0,0,0,0};

	return FilterAndCSCSample(IVE_IMAGE_TYPE_YUV422SP,IVE_CSC_MODE_PIC_BT601_YUV2RGB,IVE_IMAGE_TYPE_U8C3_PLANAR,mask,4,fileName,fileName2);
}
CVI_S32 FilterAndCSCSample422SPToPicPlanar5x5(CVI_VOID)
{
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_sp422_00_352x288_SP422_yuv_start;
	const CVI_CHAR *fileName = &_binary_test_ive_data_sp422_00_352x288_SP422_yuv_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_FilterAndCSC_422SPToPicPlanar5x5_yuv_start;
	const CVI_CHAR *fileName2 = &_binary_test_ive_data_result_sample_FilterAndCSC_422SPToPicPlanar5x5_yuv_start;

#else
	const CVI_CHAR *fileName = "../data/sp422/00_352x288_SP422.yuv";
	const CVI_CHAR *fileName2 = "../data/result/sample_FilterAndCSC_422SPToPicPlanar5x5.yuv";

#endif
	CVI_S8 mask[25]={1,2,3,2,1,
		2,5,6,5,2,
		3,6,8,6,3,
		2,5,6,5,2,
		1,2,3,2,1};

	return FilterAndCSCSample(IVE_IMAGE_TYPE_YUV422SP,IVE_CSC_MODE_PIC_BT601_YUV2RGB,IVE_IMAGE_TYPE_U8C3_PLANAR,mask,7,fileName,fileName2);
}
