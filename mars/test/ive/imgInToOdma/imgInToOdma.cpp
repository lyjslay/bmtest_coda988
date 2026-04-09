#include "imgInToOdma.h"
#include "sample_assist.h"
#include "sample_file.h"
#include "sample_define.h"
#include "mpi_ive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CVI_S32 imgInToOdmaSample(IVE_IMAGE_TYPE_E enType,CVI_S8 *ps8Mask,CVI_U8 u8Norm,
		const CVI_CHAR *fileName,const CVI_CHAR *resultFileName)
{
  IVE_SRC_IMAGE_S stSrc;
  IVE_DST_IMAGE_S stDst;
  IVE_SRC_IMAGE_S stRef;
  IVE_FILTER_CTRL_S stCtrl;
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
  memset(&stCtrl,0,sizeof(IVE_FILTER_CTRL_S));

  fp = NULL;
  fp2 = NULL;
  u16Width = CVI_IMAGE_WIDTH_352;
  u16Height = CVI_IMAGE_HEIGHT_288;

  s32Result = CVI_CreateIveImage(&stSrc,enType,u16Width,u16Height);
  CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
  s32Result = CVI_CreateIveImage(&stDst,enType,u16Width,u16Height);
  CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
  s32Result = CVI_CreateIveImage(&stRef,enType,u16Width,u16Height);
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

  memcpy(stCtrl.as8Mask,ps8Mask,sizeof(CVI_S8) * 25);
  stCtrl.u8Norm = u8Norm;

  dTime = (CVI_DOUBLE)CVI_GetTickCount();
  s32Result = CVI_MPI_IVE_imgInToOdma(&handle,&stSrc,&stDst,&stCtrl,
      CVI_TRUE
      /*CVI_FALSE*/
      );
  CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
  dTime = (CVI_DOUBLE)CVI_GetTickCount() - dTime;

  printf("imgInToOdma image type %d,time = %g ms\n",enType, dTime/((CVI_DOUBLE)CVI_GetTickFrequency()*1000.));

  printf("compare itself\n");
  s32Result = CVI_CompareIveImage(&stDst, &stSrc);

FAILURE:

  CVI_FCLOSE(fp);
  CVI_FCLOSE(fp2);
  CVI_DestroyIveImage(&stSrc);
  CVI_DestroyIveImage(&stDst);
  CVI_DestroyIveImage(&stRef);

  return s32Result;
}

CVI_S32 imgInToOdmaSampleY3x3(CVI_VOID)
{
#if defined BOARD_FPGA  || defined BOARD_ASIC
  extern char _binary_test_ive_data_y_00_352x288_y_yuv_start;
  extern char _binary_test_ive_data_result_sample_Filter_Y3x3_yuv_start;
  const CVI_CHAR *fileName =
    &_binary_test_ive_data_y_00_352x288_y_yuv_start;
  const CVI_CHAR *resultFileName =
    &_binary_test_ive_data_result_sample_Filter_Y3x3_yuv_start;
#else
  const CVI_CHAR *fileName = "../data/y/00_352x288_y.yuv";
  const CVI_CHAR *resultFileName = "../data/result/sample_Filter_Y3x3.yuv";
#endif
  CVI_S8 mask[25]={0,0,0,0,0,
    0,1,2,1,0,
    0,2,4,2,0,
    0,1,2,1,0,
    0,0,0,0,0};

  return imgInToOdmaSample(IVE_IMAGE_TYPE_U8C1,mask,4,fileName,resultFileName);
}
