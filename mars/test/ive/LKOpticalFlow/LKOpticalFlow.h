#ifndef  LKOPTICALFLOW_H_
#define  LKOPTICALFLOW_H_

#include "cvi_type.h"

#define MAX_POINT_NUM 100
#define LK_OPTICAL_FLOW_MAX_POINT_NUM 500

CVI_S32 LKOpticalFlowSample(int show, int compare);
CVI_S32 LKOpticalFlowPyrSample(int show, int compare);

#endif