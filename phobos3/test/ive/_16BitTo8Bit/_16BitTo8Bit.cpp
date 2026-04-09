#include "_16BitTo8Bit.h"
#include "sample_assist.h"
#include "sample_file.h"
#include "sample_define.h"
#include "mpi_ive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CVI_S32 _16BitTo8BitSample(IVE_IMAGE_TYPE_E enSrcType,IVE_IMAGE_TYPE_E enDstType,
		IVE_16BIT_TO_8BIT_MODE_E enMode,CVI_U8 u8Numerator, CVI_U16 u16Denominator, CVI_S8 s8Bias,
		const CVI_CHAR *fileName,const CVI_CHAR *resultFileName)
{
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_SRC_IMAGE_S stRef;
	IVE_16BIT_TO_8BIT_CTRL_S stCtrl;
	CVI_S32 s32Result;
	CVI_U32 u32Width;
	CVI_U32 u32Height;
	FILE *fp;
	FILE *fp2;
	IVE_HANDLE handle;
	CVI_DOUBLE dTime;

	memset(&stSrc,0,sizeof(IVE_SRC_IMAGE_S));
	memset(&stDst,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stRef,0,sizeof(IVE_DST_IMAGE_S));
	memset(&stCtrl,0,sizeof(IVE_16BIT_TO_8BIT_CTRL_S));

	fp = NULL;
	fp2 = NULL;
	u32Width = CVI_IMAGE_WIDTH_704;
	u32Height = CVI_IMAGE_HEIGHT_576;

	s32Result = CVI_CreateIveImage(&stSrc,enSrcType,u32Width,u32Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stDst,enDstType,u32Width,u32Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_CreateIveImage(&stRef,enDstType,u32Width,u32Height);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	CVI_ResetIveImage(&stDst,0);

	s32Result = fopen_s(&fp,fileName,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);
	s32Result = fopen_s(&fp2,resultFileName,"rb");
	CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);

	s32Result = CVI_ReadFile(&stSrc,fp);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	s32Result = CVI_ReadFile(&stRef,fp2);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

	stCtrl.enMode = enMode;
	//stCtrl.u8Numerator = CVI_GenRand(CVI_MAX_U8,0);
	//stCtrl.u16Denominator = CVI_GenRand(CVI_MAX_U16,stCtrl.u8Numerator);
	//stCtrl.s8Bias == IVE_16BIT_TO_8BIT_MODE_S16_TO_U8_BIAS == enMode ? CVI_GenRand(CVI_MAX_U8,0):0;
	stCtrl.u8Numerator = u8Numerator;
	stCtrl.u16Denominator = u16Denominator;
	stCtrl.s8Bias = s8Bias;

	dTime = (CVI_DOUBLE)CVI_GetTickCount();
	s32Result = CVI_MPI_IVE_16BitTo8Bit(&handle,&stSrc,&stDst,&stCtrl,CVI_FALSE);
	CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
	dTime = (CVI_DOUBLE)CVI_GetTickCount() - dTime;

	printf("16BitTo8Bit mode %d,time = %g ms\n",enMode, dTime/((CVI_DOUBLE)CVI_GetTickFrequency()*1000.));

	s32Result = CVI_CompareIveImage(&stDst, &stRef);

FAILURE:

	CVI_FCLOSE(fp);
	CVI_FCLOSE(fp2);
	CVI_DestroyIveImage(&stSrc);
	CVI_DestroyIveImage(&stDst);
	CVI_DestroyIveImage(&stRef);

	return s32Result;
}


CVI_S32 _16BitTo8BitSampleS16ToS8(CVI_VOID)
{
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_s16_00_704x576_s16_start;
	const CVI_CHAR *fileName = &_binary_test_ive_data_s16_00_704x576_s16_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_16BitTo8Bit_S16ToS8_yuv_start;
	const CVI_CHAR *resultFileName = &_binary_test_ive_data_result_sample_16BitTo8Bit_S16ToS8_yuv_start;

#else
	const CVI_CHAR *fileName = "../data/s16/00_704x576.s16";
	const CVI_CHAR *resultFileName = "../data/result/sample_16BitTo8Bit_S16ToS8.yuv";

#endif
	IVE_IMAGE_TYPE_E enSrcType = IVE_IMAGE_TYPE_S16C1;
	IVE_IMAGE_TYPE_E enDstType = IVE_IMAGE_TYPE_S8C1;
	IVE_16BIT_TO_8BIT_MODE_E enMode = IVE_16BIT_TO_8BIT_MODE_S16_TO_S8;
	//CVI_U8 u8Numerator = CVI_GenRand(CVI_MAX_U8,0);
	//CVI_U16 u16Denominator = CVI_GenRand(CVI_MAX_U16,u8Numerator);
	//CVI_S8 s8Bias = IVE_16BIT_TO_8BIT_MODE_S16_TO_U8_BIAS == enMode ? CVI_GenRand(CVI_MAX_U8,0):0;
	CVI_U8 u8Numerator = 41;
	CVI_U16 u16Denominator = 18508;
	CVI_S8 s8Bias = 0;
	printf("S16ToS8: u8Numerator = %d, u16Denominator = %d, s8Bias = %d\n",
			u8Numerator,u16Denominator,s8Bias);

	return _16BitTo8BitSample(enSrcType,enDstType,enMode,u8Numerator,u16Denominator,s8Bias,
			fileName,resultFileName);
}
CVI_S32 _16BitTo8BitSampleS16ToU8ABS(CVI_VOID)
{
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_s16_00_704x576_s16_start;
	const CVI_CHAR *fileName = &_binary_test_ive_data_s16_00_704x576_s16_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_16BitTo8Bit_Abs_yuv_start;
	const CVI_CHAR *resultFileName = &_binary_test_ive_data_result_sample_16BitTo8Bit_Abs_yuv_start;

#else
	const CVI_CHAR *fileName = "../data/s16/00_704x576.s16";
	const CVI_CHAR *resultFileName = "../data/result/sample_16BitTo8Bit_Abs.yuv";

#endif
	IVE_IMAGE_TYPE_E enSrcType = IVE_IMAGE_TYPE_S16C1;
	IVE_IMAGE_TYPE_E enDstType = IVE_IMAGE_TYPE_U8C1;
	IVE_16BIT_TO_8BIT_MODE_E enMode = IVE_16BIT_TO_8BIT_MODE_S16_TO_U8_ABS;
	//CVI_U8 u8Numerator = CVI_GenRand(CVI_MAX_U8,0);
	//CVI_U16 u16Denominator = CVI_GenRand(CVI_MAX_U16,u8Numerator);
	//CVI_S8 s8Bias = IVE_16BIT_TO_8BIT_MODE_S16_TO_U8_BIAS == enMode ? CVI_GenRand(CVI_MAX_U8,0):0;
	CVI_U8 u8Numerator = 190;
	CVI_U16 u16Denominator = 26690;
	CVI_S8 s8Bias = 0;
	printf("S16ToU8ABS: u8Numerator = %d, u16Denominator = %d, s8Bias = %d\n",
			u8Numerator,u16Denominator,s8Bias);

	return _16BitTo8BitSample(enSrcType,enDstType,enMode,u8Numerator,u16Denominator,s8Bias,
			fileName,resultFileName);
}
CVI_S32 _16BitTo8BitSampleS16ToU8SHIFT(CVI_VOID)
{
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_s16_00_704x576_s16_start;
	const CVI_CHAR *fileName = &_binary_test_ive_data_s16_00_704x576_s16_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_16BitTo8Bit_Shift_yuv_start;
	const CVI_CHAR *resultFileName = &_binary_test_ive_data_result_sample_16BitTo8Bit_Shift_yuv_start;

#else
	const CVI_CHAR *fileName = "../data/s16/00_704x576.s16";
	const CVI_CHAR *resultFileName = "../data/result/sample_16BitTo8Bit_Shift.yuv";

#endif
	IVE_IMAGE_TYPE_E enSrcType = IVE_IMAGE_TYPE_S16C1;
	IVE_IMAGE_TYPE_E enDstType = IVE_IMAGE_TYPE_U8C1;
	IVE_16BIT_TO_8BIT_MODE_E enMode = IVE_16BIT_TO_8BIT_MODE_S16_TO_U8_BIAS;
	//CVI_U8 u8Numerator = CVI_GenRand(CVI_MAX_U8,0);
	//CVI_U16 u16Denominator = CVI_GenRand(CVI_MAX_U16,u8Numerator);
	//CVI_S8 s8Bias = IVE_16BIT_TO_8BIT_MODE_S16_TO_U8_BIAS == enMode ? CVI_GenRand(CVI_MAX_U8,0):0;
	CVI_U8 u8Numerator = 225;
	CVI_U16 u16Denominator = 15949;
	CVI_S8 s8Bias = -42;
	printf("S16ToU8SHIFT: u8Numerator = %d, u16Denominator = %d, s8Bias = %d\n",
			u8Numerator,u16Denominator,s8Bias);

	return _16BitTo8BitSample(enSrcType,enDstType,enMode,u8Numerator,u16Denominator,s8Bias,
			fileName,resultFileName);
}
CVI_S32 _16BitTo8BitSampleU16ToU8(CVI_VOID)
{
#if defined BOARD_FPGA  || defined BOARD_ASIC
extern CVI_CHAR _binary_test_ive_data_u16_00_704x576_u16_start;
	const CVI_CHAR *fileName = &_binary_test_ive_data_u16_00_704x576_u16_start;
extern CVI_CHAR _binary_test_ive_data_result_sample_16BitTo8Bit_U16ToU8_yuv_start;
	const CVI_CHAR *resultFileName = &_binary_test_ive_data_result_sample_16BitTo8Bit_U16ToU8_yuv_start;

#else
	const CVI_CHAR *fileName = "../data/u16/00_704x576.u16";
	const CVI_CHAR *resultFileName = "../data/result/sample_16BitTo8Bit_U16ToU8.yuv";

#endif
	IVE_IMAGE_TYPE_E enSrcType = IVE_IMAGE_TYPE_U16C1;
	IVE_IMAGE_TYPE_E enDstType = IVE_IMAGE_TYPE_U8C1;
	IVE_16BIT_TO_8BIT_MODE_E enMode = IVE_16BIT_TO_8BIT_MODE_U16_TO_U8;
	//CVI_U8 u8Numerator = CVI_GenRand(CVI_MAX_U8,0);
	//CVI_U16 u16Denominator = CVI_GenRand(CVI_MAX_U16,u8Numerator);
	//CVI_S8 s8Bias = IVE_16BIT_TO_8BIT_MODE_S16_TO_U8_BIAS == enMode ? CVI_GenRand(CVI_MAX_U8,0):0;
	CVI_U8 u8Numerator = 174;
	CVI_U16 u16Denominator = 27136;
	CVI_S8 s8Bias = 0;
	printf("U16ToU8: u8Numerator = %d, u16Denominator = %d, s8Bias = %d\n",
			u8Numerator,u16Denominator,s8Bias);

	return _16BitTo8BitSample(enSrcType,enDstType,enMode,u8Numerator,u16Denominator,s8Bias,
			fileName,resultFileName);
}
