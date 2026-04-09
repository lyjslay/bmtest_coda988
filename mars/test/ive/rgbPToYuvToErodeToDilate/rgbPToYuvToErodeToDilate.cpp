#include "rgbPToYuvToErodeToDilate.h"
#include "sample_assist.h"
#include "sample_file.h"
#include "sample_define.h"
#include "mpi_ive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CVI_S32 rgbPToYuvToErodeToDilateSample(IVE_IMAGE_TYPE_E enType,CVI_S8 *ps8Mask,CVI_U8 u8Norm,
		const CVI_CHAR *fileName,const CVI_CHAR *resultFileName)
{
  IVE_SRC_IMAGE_S stSrc;
  IVE_DST_IMAGE_S stDst;
  IVE_DST_IMAGE_S stDst2;
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
  memset(&stDst2,0,sizeof(IVE_DST_IMAGE_S));
  memset(&stRef,0,sizeof(IVE_DST_IMAGE_S));
  memset(&stCtrl,0,sizeof(IVE_FILTER_CTRL_S));

  fp = NULL;
  fp2 = NULL;
  u16Width = 512;
  u16Height = 512;

  s32Result = CVI_CreateIveImage(&stSrc,IVE_IMAGE_TYPE_U8C3_PLANAR,u16Width,u16Height);
  CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
  s32Result = CVI_CreateIveImage(&stDst,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
  CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
  s32Result = CVI_CreateIveImage(&stDst2,IVE_IMAGE_TYPE_U8C3_PLANAR,u16Width,u16Height);
  CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
  s32Result = CVI_CreateIveImage(&stRef,enType,u16Width,u16Height);
  CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
#if 1
printf("skip set dst to 0\n");
#else
  CVI_ResetIveImage(&stDst,0);
  CVI_ResetIveImage(&stDst2,0);
#endif

  s32Result = fopen_s(&fp,fileName,"rb");
  CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);

  s32Result = fopen_s(&fp2,resultFileName,"rb");
  CVI_CHECK_NET_GOTO(s32Result,0,FAILURE);

  printf("make sure img_in memory align with 16bytes\n");
  if ((CVI_U64)fileName % 16 != 0) {
    printf("copy aligned addr...\n");
    memcpy((char*)stSrc.au64PhyAddr[0], fileName, 512*512*3);
    printf("copy done\n");
  }
  else {
    s32Result = CVI_ReadFile(&stSrc,fp);
    CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
  }

  s32Result = CVI_ReadFile(&stRef,fp2);
  CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);

  dTime = (CVI_DOUBLE)CVI_GetTickCount();

  s32Result = CVI_MPI_IVE_rgbPToYuvToErodeToDilate(&handle,&stSrc,&stDst,
      &stDst2,&stCtrl,
      CVI_TRUE
      /*CVI_FALSE*/
      );

  CVI_CHECK_ET_GOTO(s32Result,CVI_FAILURE,FAILURE);
  dTime = (CVI_DOUBLE)CVI_GetTickCount() - dTime;

  printf("rgbPToYuvToErodeToDilate image type %d,time = %g ms\n",enType, dTime/((CVI_DOUBLE)CVI_GetTickFrequency()*1000.));

  printf("compare...\n");
  //stDst.enType = stRef.enType;
  s32Result = CVI_CompareIveImage(&stDst, &stRef);
  //s32Result = CVI_CompareIveImage(&stDst2, &stRef);

FAILURE:

  CVI_FCLOSE(fp);
  CVI_FCLOSE(fp2);
  CVI_DestroyIveImage(&stSrc);
  CVI_DestroyIveImage(&stDst);
  CVI_DestroyIveImage(&stRef);

  return s32Result;
}

CVI_S32 rgbPToYuvToErodeToDilateSampleY3x3(CVI_VOID)
{
#if defined BOARD_FPGA  || defined BOARD_ASIC
  extern char _binary_test_ive_data_rgb_lene2rgbplanner_rgb_start;
  extern char _binary_test_ive_data_result_ive_erode_dilate_env_scoreboard_0_golden_rgb_start;
  const CVI_CHAR *fileName =
    &_binary_test_ive_data_rgb_lene2rgbplanner_rgb_start;
  const CVI_CHAR *resultFileName =
    &_binary_test_ive_data_result_ive_erode_dilate_env_scoreboard_0_golden_rgb_start;
#else
  const CVI_CHAR *fileName = "../data/rgb/lene2rgbplanner.rgb"
  const CVI_CHAR *resultFileName = "../data/result/ive_erode_dilate_env_scoreboard_0_golden.rgb";
#endif
  CVI_S8 mask[25]={0,0,0,0,0,
    0,1,2,1,0,
    0,2,4,2,0,
    0,1,2,1,0,
    0,0,0,0,0};

  return rgbPToYuvToErodeToDilateSample(IVE_IMAGE_TYPE_U8C1,mask,4,fileName,resultFileName);
}
