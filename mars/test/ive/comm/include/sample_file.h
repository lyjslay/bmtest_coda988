#ifndef SAMPLE_FILE_H_
#define SAMPLE_FILE_H_

#include "cvi_comm_ive.h"
#include <stdio.h>

#if defined BOARD_FPGA  || defined BOARD_ASIC
// 1 == 1 fake for make sure return as true
#define fopen_s(pFile,filename,mode) (1 == 0); FILE _##filename; (*(pFile)) = &_##filename;  (*(pFile))->_p = (CVI_U8*)filename;
#elif defined __unix
#define fopen_s(pFile,filename,mode) ((*(pFile))=fopen((filename),  (mode)))==NULL
#endif

CVI_S32 CVI_ReadFile(IVE_SRC_IMAGE_S *pstImage,FILE *fp);
CVI_S32 CVI_WriteFile(IVE_SRC_IMAGE_S *pstImage,FILE *fp);
CVI_S32 CVI_MemReadFile(IVE_MEM_INFO_S *pstMem,FILE *fp);
CVI_S32 CVI_MemWriteFile(IVE_MEM_INFO_S *pstMem,FILE *fp);
CVI_S32 CVI_DataReadFile(IVE_DATA_S *pstData,FILE *fp);
CVI_S32 CVI_DataWriteFile(IVE_DATA_S *pstData,FILE *fp);

#endif
