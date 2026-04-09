#ifndef __DEC_TEST_H__
#define __DEC_TEST_H__

#ifndef MAX_FILE_PATH
#define	MAX_FILE_PATH	256
#endif

#ifndef MAX_NUM_INSTANCE
#define MAX_NUM_INSTANCE	8
#endif

#ifndef _EncConfigParam_
#define _EncConfigParam_
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
#endif

#ifndef _DecConfigParam_
#define _DecConfigParam_
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
#endif


#ifndef _MultiConfigParam_
#define _MultiConfigParam_
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
#endif

#if defined (__cplusplus)
extern "C" {
#endif

int jpeg_main(DecConfigParam *pDec, EncConfigParam *pEnc, MultiConfigParam *pMulti );

#if defined (__cplusplus)
}
#endif


#endif
