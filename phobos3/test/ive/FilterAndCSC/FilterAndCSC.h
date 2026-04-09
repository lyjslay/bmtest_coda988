#ifndef FILTERANDCSC_H_
#define FILTERANDCSC_H_

#include "cvi_type.h"

CVI_S32 FilterAndCSCSample420SPToVideoPackage3x3(CVI_VOID);
CVI_S32 FilterAndCSCSample420SPToVideoPackage5x5(CVI_VOID);
CVI_S32 FilterAndCSCSample420SPToVideoPlanar3x3(CVI_VOID);
CVI_S32 FilterAndCSCSample420SPToVideoPlanar5x5(CVI_VOID);
CVI_S32 FilterAndCSCSample422SPToVideoPackage3x3(CVI_VOID);
CVI_S32 FilterAndCSCSample422SPToVideoPackage5x5(CVI_VOID);
CVI_S32 FilterAndCSCSample422SPToVideoPlanar3x3(CVI_VOID);
CVI_S32 FilterAndCSCSample422SPToVideoPlanar5x5(CVI_VOID);

CVI_S32 FilterAndCSCSample420SPToPicPackage3x3(CVI_VOID);
CVI_S32 FilterAndCSCSample420SPToPicPackage5x5(CVI_VOID);
CVI_S32 FilterAndCSCSample420SPToPicPlanar3x3(CVI_VOID);
CVI_S32 FilterAndCSCSample420SPToPicPlanar5x5(CVI_VOID);
CVI_S32 FilterAndCSCSample422SPToPicPackage3x3(CVI_VOID);
CVI_S32 FilterAndCSCSample422SPToPicPackage5x5(CVI_VOID);
CVI_S32 FilterAndCSCSample422SPToPicPlanar3x3(CVI_VOID);
CVI_S32 FilterAndCSCSample422SPToPicPlanar5x5(CVI_VOID);

#endif