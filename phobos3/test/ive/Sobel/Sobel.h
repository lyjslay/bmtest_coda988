#ifndef SOBEL_H_
#define SOBEL_H_

#include "cvi_type.h"

CVI_S32 SobelSampleBoth3x3(CVI_VOID);
CVI_S32 SobelSampleBoth5x5(CVI_VOID);
CVI_S32 SobelSampleHor3x3(CVI_VOID);
CVI_S32 SobelSampleHor5x5(CVI_VOID);
CVI_S32 SobelSampleVer3x3(CVI_VOID);
CVI_S32 SobelSampleVer5x5(CVI_VOID);

#endif