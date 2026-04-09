#ifndef _JPU_RUN_H_
#define _JPU_RUN_H_
#include"../jpuapi/jpuconfig.h"


#define	MAX_FILE_PATH	256




typedef struct {
	unsigned char * buf;
	int size;
	int point;
	int count;
	int fillendbs;
} BufInfo;

typedef struct
{
	char* yuvFileName;
	char* bitstreamFileName;
	char* huffFileName;
	char* qMatFileName;
	char* qpFileName;
	char* cfgFileName;
	int picWidth;
	int picHeight;
	int rotAngle;
	int mirDir;
	int useRot;
	int mjpgChromaFormat;
	int outNum;
	int instNum;
	int roiEnable;

	int StreamEndian;
	int FrameEndian;
	int chromaInterleave;
    int bEnStuffByte;

	// altek requirement
	int encHeaderMode;


    char* strStmDir;
	char* strCfgDir;
    int usePartialMode;
    int partialBufNum;
    int partialHeight;
	int packedFormat;
	int RandRotMode;
    int compareJpg;
} EncConfigParam;

typedef struct
{
	char* yuvFileName;
	char* bitstreamFileName;
	int comparatorFlag;
    int rotAngle;
	int mirDir;
	int useRot;
	int outNum;
	int checkeos;
	int instNum;
	int StreamEndian;
	int FrameEndian;
	int chromaInterleave;
    int iHorScaleMode;
	int iVerScaleMode;
	
	//ROI
	int roiEnable;
	int roiWidth;
	int roiHeight;
	int roiOffsetX;
	int roiOffsetY;
	int roiWidthInMcu;
	int roiHeightInMcu;
	int roiOffsetXInMcu;
	int roiOffsetYInMcu;

	//packed
	int packedFormat;


	

    int usePartialMode;
    int partialBufNum;

    int partialHeight;
	int filePlay;
	
} DecConfigParam;


enum {
	STD_JPG_ENC
};

typedef struct {
	int codecMode;
	int numMulti;
	int saveYuv;
	int  multiMode[MAX_NUM_INSTANCE];
    char* multiFileName[MAX_NUM_INSTANCE];
	char* multiYuvFileName[MAX_NUM_INSTANCE];	
	EncConfigParam encConfig[MAX_NUM_INSTANCE];
	DecConfigParam decConfig[MAX_NUM_INSTANCE];
} MultiConfigParam;


#if defined (__cplusplus)
extern "C" {
#endif
	int DecodeTest(DecConfigParam *param);
	int EncodeTest(EncConfigParam *param);	
	int MultiInstanceTest(MultiConfigParam	*param);
#if defined (__cplusplus)
}
#endif
#endif	/* _JPU_RUN_H_ */
