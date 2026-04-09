#ifndef __CNM_VENC_TESTARGV_H__
#define __CNM_VENC_TESTARGV_H__

#define MAX_ROI_NUM 64
typedef struct _TestArgV_t {
    int argc;
    int showVersion;
    int n; // num of frames
    int picWidth;
    int picHeight;
    int enNv21;
    int rotate; // 0:no rotate, 1:rotate 90 degree, 2:rotate 180 degree, 3:rotate 270 degree
    int mirror; // 0:no mirror, 1:vertical mirror, 2:horizontal mirror, 3:both directions

    int gopSize;
    int avcProfile; // 0:baseline, 1:main, 2:high
    int rcEnable;  // 0: CQP, 1:CBR, 2:VBR 4: picture level RC
    int targetBitrate; // kbps for coda988, if 0 , no rate control
    int iqpmax;
    int iqpmin;
    int pqpmax;
    int pqpmin;

    int sliceMode; // 0: 1 slice 1：multi slice
    int sliceSizeMode;
    int mbLineNum;

    int roiEnable;
    // following 4 var is sync from `AvcRoiParam` in vpuapi.h
    int roiParamMode;
    int roiParamNumber;
    int roiPicAvgQp;
    int nonRoiQp;
    int roiParamRegion[MAX_ROI_NUM][4];
    int roiParamQP[MAX_ROI_NUM];
    unsigned int roiQPMapAddr;

    // addrRemap
    int addrRemapEn;
    int ARMode; // 0 = original mode, 1 = offset mode
    int numExtraLine;

    //single luma buffer
    int singleLumaBuf; // 0: disable, 1: enable

    int virtualIPeriod; //smart-p case

    // Secondary AXI
    int secondaryAxi; // bit mask for secondary AXI usage
                      // bit0: useBitEnable, bit1: useIpEnable
                      // bit2: useDbkYEnable, bit3: useDbkCEnable
                      // bit4: useBtpEnable, bit5: useOvlEnable
                      // default: 0xf (enable bit0-3)

    // SVC (Scalable Video Coding)
    int svcEnable;    // 0: disable, 1: enable SVC-T (temporal scalability)
    int mvcEnable;    // 0: disable, 1: enable MVC (spatial scalability)

    // TODO: 添加另外的參數，需要在SyncParamFromTestArgV中同步給EncOpenparam
} TestArgV_t;




// typedef struct _SrcFrameInfo_t {
//     unsigned int phys_addr;
//     unsigned int frame_num; // 帧序号
//     unsigned int stride;
//     unsigned int height;
// } SrcFrameInfo_t;





#endif