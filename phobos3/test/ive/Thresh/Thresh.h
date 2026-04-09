#ifndef THRESH_H_
#define THRESH_H_

#include "cvi_type.h"

CVI_S32 ThreshSampleBinary(CVI_VOID);
CVI_S32 ThreshSampleTrunc(CVI_VOID);
CVI_S32 ThreshSampleToMinval (CVI_VOID);
CVI_S32 ThreshSampleMinMidMax(CVI_VOID);
CVI_S32 ThreshSampleOriMidMax(CVI_VOID);
CVI_S32 ThreshSampleMinMidOri(CVI_VOID);
CVI_S32 ThreshSampleMinOriMax(CVI_VOID);
CVI_S32 ThreshSampleOriMinOri(CVI_VOID);

#endif
