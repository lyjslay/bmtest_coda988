#ifndef FILTER_H_
#define FILTER_H_

#include "cvi_type.h"

CVI_S32 FilterSampleY3x3(CVI_VOID);
CVI_S32 FilterSample420SP3x3(CVI_VOID);
CVI_S32 FilterSample422SP3x3(CVI_VOID);
CVI_S32 FilterSampleY5x5(CVI_VOID);
CVI_S32 FilterSample420SP5x5(CVI_VOID);
CVI_S32 FilterSample422SP5x5(CVI_VOID);

#endif