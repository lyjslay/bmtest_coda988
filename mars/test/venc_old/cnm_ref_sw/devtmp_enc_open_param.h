typedef struct {
    PhysicalAddress bitstreamBuffer;        /**< The start address of bitstream buffer into which encoder puts bitstream. This address must be aligned to AXI bus width. */
    Uint32          bitstreamBufferSize;    /**< The size of the buffer in bytes pointed by bitstreamBuffer. This value must be a multiple of 1024. The maximum size is 16383 x 1024 bytes. */
    CodStd          bitstreamFormat;        /**< The standard type of bitstream in encoder operation. It is one of STD_MPEG4, STD_H263, STD_AVC and STD_MJPG. */
/**
@verbatim
@* 0 : disable
@* 1 : enable

This flag declares the streaming mode for the current encoder instance. Two
streaming modes, packet-based streaming with ring-buffer (buffer-reset mode) and
frame-based streaming with line buffer (buffer-flush mode), can be configured by
using this flag.

When this field is set, packet-based streaming with ring-buffer is used.
And when this field is not set, frame-based streaming with line-buffer is used.

@endverbatim
*/
    int             ringBufferEnable;
    int             picWidth;           /**< The width of a picture to be encoded in pixels. */
    int             picHeight;          /**< The height of a picture to be encoded in pixels. */
/**
@verbatim
It is a linear to tiled enable mode.
The map type can be changed from linear to tiled format while reading source frame
in the PrP (Pre-Processing) unit.

@* 0 : disable linear to tiled-map conversion for getting source image
@* 1 : enable linear to tiled-map conversion for getting source image
@endverbatim
*/
    int             linear2TiledEnable;
/**
@verbatim
It can specify the map type of source frame buffer when linear2TiledEnable is enabled. (CODA980 only)

@* 1 : source frame buffer is in linear frame map.
@* 2 : source frame buffer is in linear field map.

@endverbatim
*/
    int             linear2TiledMode;
/**
@verbatim
The 16 LSB bits, [15:0], is a numerator and 16 MSB bits, [31:16], is a
denominator for calculating frame rate. The numerator means clock ticks per
second, and the denominator is clock ticks between frames minus 1.

So the frame rate can be defined by (numerator/(denominator + 1)),
which equals to (frameRateInfo & 0xffff) /((frameRateInfo >> 16) + 1).

For example, the value 30 of frameRateInfo represents 30 frames/sec, and the
value 0x3e87530 represents 29.97 frames/sec.
@endverbatim
*/
    int             frameRateInfo;
/**
@verbatim
The horizontal search range mode for Motion Estimation

@* 0 : Horizontal(-64 ~ 63)
@* 1 : Horizontal(-48 ~ 47)
@* 2 : Horizontal(-32 ~ 31)
@* 3 : Horizontal(-16 ~ 15)
@endverbatim
*/
    // [START] CODA980 & WAVE320
    int             MESearchRangeX;
/**
@verbatim
The vertical search range mode for Motion Estimation

@* 0 : Vertical(-48 ~ 47)
@* 1 : Vertical(-32 ~ 31)
@* 2 : Vertical(-16 ~ 15)
@endverbatim
*/
    int             MESearchRangeY;
/**
@verbatim
An initial QP offset for I picture in GOP.

This setting can lower an I picture QP value
down to a user-defined value ranging -4 to 4.
This value is valid for AvcEnc and ignored
when when RcEnable is 0 or RcGopIQpOff-setEn is 0.
@endverbatim
*/
    int             rcGopIQpOffsetEn;
/**
@verbatim
An enable flag for initial QP offset for I picture in GOP.

@* 0 : disable (default)
@* 1 : disable

This value is valid for AvcEnc and ignored when when RcEnable is 0.
@endverbatim
*/
    int             rcGopIQpOffset;
/**
@verbatim
The search range mode for Motion Estimation

@* 0 : Horizontal(-128 ~ 127), Vertical(-64 ~ 63)
@* 1 : Horizontal(-64 ~ 63), Vertical(-32 ~ 31)
@* 2 : Horizontal(-32 ~ 31), Vertical(-16 ~ 15)
@* 3 : Horizontal(-16 ~ 15), Vertical(-16 ~ 15)
@endverbatim
*/
    int             MESearchRange;
    // [END] CODA980 & WAVE320
/**
@verbatim
vbv_buffer_size in bits

This value is ignored if rate control is disabled or
initialDelay is 0. The value 0 means the encoder does not check for reference
decoder buffer size constraints.
@endverbatim
*/
    // [START] CODA9xx & WAVE320
    int             vbvBufferSize;
/**
@verbatim
Frame skip indicates that encoder can skip frame encoding automatically when
bitstream has been generated much so far considering the given target bitrate. This parameter is
ignored if rate control is not used (bitRate is 0).

@* 0 : enables frame skip function in encoder.
@* 1 : disables frame skip function in encoder.
@endverbatim
*/
    int             frameSkipDisable;

/**
@verbatim
This is the GOP size.

@* 0 : only first picture is I
@* 1 : all I pictures
@* 2 : IPIP...
@* 3 : IPPIPP...

The maximum value is 32767, but in practice, a smaller value should be
chosen by HOST application for proper error concealment operations. This value is
ignored in case of STD_MJPG
@endverbatim
*/
    int             gopSize;
/**
An interval of adding an IDR picture
*/
    int idrInterval;
/**
@verbatim
A block mode enable flag for Motion Estimation. (H.264/AVC only).
A HOST can use some combination (bitwise or-ing) of each value under below.

@* 4'b0000 or 4'b1111 : Use all block mode
@* 4'b0001 : Enable 16x16 block mode
@* 4'b0010 : Enable 16x8 block mode
@* 4'b0100 : Enable 8x16 block mode
@* 4'b1000 : Enable 8x8 block mode
@endverbatim
*/
    int             meBlkMode;
    EncSliceMode    sliceMode;      /**< The parameter for slice mode */
/**
@verbatim
@* 0 : intra MB refresh is not used.
@* Otherwise : at least N MBs in every P-frame is encoded as intra MBs.

This value is ignored in case of STD_MJPG
@endverbatim
*/
    int             intraRefresh;
/**
@verbatim
Consecutive intra MB refresh mode

This option is valid only when IntraMbRefresh-Num[15:0] is not 0.

@* 0 - ConscIntraRefreshEn is disabled. IntraMbRefreshNumnumber of MBs are encoded
as an intra MB at the defined interval by picture
size.
@* 1 - IntraMbRefreshNumnumber of consec-utive MBs are encoded as an intra MB.
@endverbatim
*/
    int             ConscIntraRefreshEnable;
/**
@verbatim
Intra MB count enable

If it is enabled, VPU does not include intra MBs
from mode decision when counting IntraMbRefreshNum. This option is valid only when
IntraMbRefreshNum[15:0] is not 0.

@* 0 - intra mb counting is disabled.
@* 1 - intra mb counting is enabled.
@endverbatim
*/
    int             CountIntraMbEnable;
/**
@verbatim
Field sequence intra refresh enable

This option is valid only when IntraMbRefresh-Num[15:0] is not 0 and interlaced encoding.

@* 0 - FieldSeqIntraRefreshEn is disabled.
@* 1 - Intra mb is sequentially refreshed at each
field.
@endverbatim
*/
    int             FieldSeqIntraRefreshEnable;
/**
@verbatim
The maximum quantized step parameter for encoding process

In MPEG4/H.263 mode, the maximum value is 31.
In H.264 mode, allowed maximum value is 51.
@endverbatim
*/
    int             userQpMax;

    //h.264 only
    int             maxIntraSize;   /**< The maximum bit size for intra frame or encoding process. It works only in H.264/AVC encoder. */
    int             userMaxDeltaQp; /**< The maximum delta QP for encoding process. It works only in the H.264/AVC mode. */
    int             userMinDeltaQp; /**< The minimum delta QP for encoding process. It works only in the H.264/AVC mode. */
    int             userQpMinI;      /**< The minimum quantized step parameter for Intra frame. (H.264/AVC only) */
    int             userQpMinP;      /**< The minimum quantized step parameter for Inter frame. (H.264/AVC only) */
    int             userQpMaxI;    /**< The maximum quantized step parameter for Intra frame. (H.264/AVC only) */
    int             userQpMaxP;    /**< The maximum quantized step parameter for Inter frame. (H.264/AVC only) */
/**
@verbatim
The PMV option for Motion Estimation.
If this field is 1, encoding quality could be worse than when it was zero.

@* 0 : Motion Estimation engine uses PMV that was derived from neighbor MV
@* 1 : Motion Estimation engine uses Zero PMV
@endverbatim
*/
    int             MEUseZeroPmv;
/**
@verbatim
Additional weight of intra cost for mode decision to reduce intra MB density

By default, it could be zero.
If this register have some value W,
and the cost of best intra mode that was decided by Refine-Intra-Mode-Decision is ICOST,
the Final Intra Cost FIC is like below,

FIC = ICOST + W

So, if this field is not zero,
the Final Intra Cost have additional weight. Then the Intra/Inter mode decision logic tend to make more Inter-Macroblock.
@endverbatim
*/
    int             intraCostWeight;
/**
@verbatim
The quantization parameter for I frame

When this value is -1, the quantization
parameter for I frames is automatically determined by VPU. This value is ignored
in case of STD_MJPG
@endverbatim
*/
    //mp4 only
    int             rcIntraQp;
/**
@verbatim
A gamma is a smoothing factor in motion estimation. A value for gamma is
factor * 32768, the factor value is selected from the range 0 &le; factor &ge; 1.

@* If the factor value getting close to 0, Qp changes slowly.
@* If the factor value getting close to 1, Qp changes quickly.

The default gamma value is 0.75 * 32768
@endverbatim
*/
    int             userGamma;
/**
@verbatim
Encoder Rate Control Mode setting

@* 0 : Normal mode rate control
@* 1 : FRAME_LEVEL rate control
@* 2 : SLICE_LEVEL rate control
@* 3 : USER DEFINED MB LEVEL rate control
@endverbatim
*/
    int             rcIntervalMode;
/**
@verbatim
The user defined MB interval value

This value is used only when rcIntervalMode is 3.
@endverbatim
*/
    int             mbInterval;
/**
@verbatim
Target bit rate in kbps

If 0, there is no rate control, and pictures are encoded with a quantization parameter equal to quantParam in EncParam.
@endverbatim
*/
    // [END] CODA9xx & WAVE320
    int             bitRate;
/**
@verbatim
Time delay (in mili-seconds)

It takes for the bitstream to reach initial occupancy of the vbv buffer from zero level.

This value is ignored if rate
control is disabled. The value 0 means the encoder does not check for reference
decoder buffer delay constraints.
@endverbatim
*/
    int             initialDelay;
/**
@verbatim
@* WAVE420
@** 0 : Rate control is off.
@** 1 : Rate control is on.

@* CODA9
@** 0 : Constant QP (VBR, rate control off)
@** 1 : Constant Bit-Rate (CBR)
@** 2 : Average Bit-Rate (ABR)
@** 4 : Picture level rate control
@endverbatim
*/
    int             rcEnable;

    union {
        EncMp4Param     mp4Param;       /**< The parameters for MPEG4 part 2 Visual  */
        EncH263Param    h263Param;      /**< The parameters for ITU-T H.263 */
        EncAvcParam     avcParam;       /**< The parameters for ITU-T H.263 */
        EncHevcParam    hevcParam;      /**< The parameters for ITU-T H.265/HEVC */
    } EncStdParam;

    // Maverick-II Cache Configuration
/**
@verbatim
Enable or disable frame buffer cache

@* 0 : ME/MC CACHE
@* 1 : ME BYPASS/MC CACHE
@* 2 : MC BYPASS/ME CACHE
@* 3 : ME/MC BYPASS
@endverbatim
*/
    int             frameCacheBypass;
/**
@verbatim
Cache burst value

@* 0 : Burst 4
@* 1 : Burst 8
@endverbatim
*/
    int             frameCacheBurst;
/**
@verbatim
@* 0 : Horizontal no merge
@* 1 : Horizontal merge
@* 2 : Vertical no merge
@* 3 : Vertical merge
@endverbatim
*/
    int             frameCacheMerge;
/**
@verbatim
Cache way size of luma and chroma.
This value range is from 0 to 15.

[0-15] {chroma[1:0], luma[1:0]}

@* Luma   [0,1 : 64x64, 2,3 : 128x32]
@* Chroma Separated   [0,1: 32x32, 2,3: 64x16]
@* Chroma Interleaved [0,1: 32x64, 2: 64x32, 3: 128x16]
@endverbatim
*/
    int             frameCacheWayShape;
/**
@verbatim
@* 0 : CbCr data is written in separate frame memories (chroma separate mode)
@* 1 : CbCr data is interleaved in chroma memory. (chroma interleave mode)
@endverbatim
*/
    int             cbcrInterleave;
/**
@verbatim
CbCr order in planar mode (YV12 format)

@* 0 : Cb data are written first and then Cr written in their separate plane.
@* 1 : Cr data are written first and then Cb written in their separate plane.
@endverbatim
*/
    int             cbcrOrder;
/**
@verbatim
Frame buffer endianness

@* 0 : little endian format
@* 1 : big endian format
@* 2 : 32 bit little endian format
@* 3 : 32 bit big endian format
@* 16 ~ 31 : 128 bit endian format

NOTE: For setting specific values of 128 bit endiness, please refer to the 'WAVE4 Datasheet'.
@endverbatim
*/
    int             frameEndian;
/**
@verbatim
Bistream buffer endianness

@* 0 : little endian format
@* 1 : big endian format
@* 2 : 32 bit little endian format
@* 3 : 32 bit big endian format
@* 16 ~ 31 : 128 bit endian format

NOTE: For setting specific values of 128 bit endiness, please refer to the 'WAVE4 Datasheet'.
@endverbatim
*/
    int             streamEndian;
/**
@verbatim
Endianness of source YUV

@* 0 : Little endian format
@* 1 : Big endian format
@* 2 : 32 bit little endian format
@* 3 : 32 bit big endian format
@* 16 ~ 31 : 128 bit endian format

NOTE: For setting specific values of 128 bit endiness, please refer to the 'WAVE4 Datasheet'.
@endverbatim
*/
    int             sourceEndian;
/**
@verbatim
It writes output with 8 burst in linear map mode.

@* 0 : Burst write back is disabled
@* 1 : Burst write back is enabled.

NOTE: WAVE4 does not support it.
@endverbatim
*/
    int             bwbEnable;
/**
@verbatim
@* 0 : Disable
@* 1 : Enable

This flag is used for AvcEnc frame-based streaming with line buffer.
If coding standard is not AvcEnc or ringBufferEnable is 1, this flag is ignored.

If this field is set, VPU sends a buffer full interrupt when line buffer is full, and waits until the interrupt is cleared.
HOST should read the bitstream in line buffer and clear the interrupt.
If this field is not set, VPU does not send a buffer full interrupt when line buffer is full.
In both cases, VPU sets RET_ENC_PIC_FLAG register to 1 if line buffer is full.
@endverbatim
*/
    int             lineBufIntEn;
/**
@verbatim
This flag indicates whether source images are in packed format.

@* 0 : Not packed mode
@* 1 : Packed mode
@endverbatim
*/
    int             packedFormat;
/**
This flag indicates a color format of source image which is one among <<vpuapi_h_FrameBufferFormat>>.
*/
    int             srcFormat;
    int             srcBitDepth;
    ParamChange paramChange;

/**
@verbatim
VPU core index number

* 0 ~ number of VPU core - 1
@endverbatim
*/
    Uint32          coreIdx;
/**
@verbatim
@* 0 : CbCr data is interleaved in chroma source frame memory. (NV12)
@* 1 : CrCb data is interleaved in chroma source frame memory. (NV21)
@endverbatim
*/
    int             nv21;



    Uint32 encodeHrdRbspInVPS;        /**< It encodes the HRD syntax into VPS. */
    Uint32 encodeHrdRbspInVUI;        /**< It encodes the HRD syntax into VUI. */
    Uint32 hrdRbspDataSize;           /**< The bit size of the HRD rbsp data */
    PhysicalAddress hrdRbspDataAddr;  /**< The address of the HRD rbsp data */

    Uint32 encodeVuiRbsp;     /**< A flag to encode the VUI syntax in rbsp which is given by HOST application */
    Uint32 vuiRbspDataSize;   /**< The bit size of the VUI rbsp data */
    PhysicalAddress vuiRbspDataAddr;   /**< The address of the VUI rbsp data */

    Uint32          virtAxiID;
    BOOL            enablePTS; /**< An enable flag to report PTS(Presentation Timestamp. */
    int             coda9RoiEnable;  /**< It enables ROI for CODA980. */
    int             RoiPicAvgQp;     /**< It specifies an ROI average QP. */
    char            RoiFile[1024];   /**< It specifies the ROI file. */

    int             set_dqp_pic_num; /**< It is the number of cyclic pictures. */

    gop_entry_t     gopEntry[MAX_GOP_SIZE *2];  /**< The entire structure of cyclic pictures  */
    int             gopPreset;                  /**< GOP preset value (0~5) */

    int             LongTermPeriod;  /**< It specifies an interval value of longterm period. */
    int             LongTermDeltaQp; /**< It specifies a delta QP value between base(intra) picture and longterm inter picture.   */

    int             dqp[8];          /**< A delta QP value of frame# within the cyclic pictures  */
    int             HvsQpScaleDiv2;  /**< It specifies the scale for MB QP derivation. It is only avilable when EN_HVS_QP is 1. */
    int             EnHvsQp;         /**< It enables HVS QP. */
    int             EnRowLevelRc;    /**< It enables row-level rate control. */
    int             RcInitialQp;     /**< An initial QP for rate control */
    int             RcHvsMaxDeltaQp; /**< A maximum delta QP value for HVS considered rate control */
#ifdef ROI_MB_RC
    int             roi_max_delta_qp_minus;
    int             roi_max_delta_qp_plus;
#endif
#ifdef AUTO_FRM_SKIP_DROP
    int             enAutoFrmSkip; /**< It enables frame skip automatically according to threshold */
    int             enAutoFrmDrop; /**< It enables frame drop automatically according to threshold */
    int             vbvThreshold;  /**< Vbv buffer threshold value */
    int             qpThreshold;   /**< Qp buffer threshold value */
    int             maxContinuosFrameSkipNum;  /**< It specifies the maximum number of continusous frame skip. */
    int             maxContinuosFrameDropNum;  /**< It specifies the maximum number of continusous frame drop. */
#endif
    int             rcWeightFactor; /**< Adjusts the speed of updating parameters to the rate control model. If RcWeightFactor is set with a large value, the RC parameters are updated slowly. (Min: 1, Max: 32, default: 2) */

    /**
    @verbatim
    Time delay in mili-seconds (CODA9 only)

    It is the amount of time in ms taken for bitstream to reach initial occupancy of the vbv buffer from zero level.

    This value is ignored if rate
    control is disabled. The value 0 means VPU does not check for reference
    decoder buffer delay constraints.
    @endverbatim
    */
    int             rcInitDelay;

    /**
    @verbatim
    The number of intra MB to be inserted in picture (CODA9 only)

    @* 0 : intra MB refresh is not used.
    @* Other value : intraRefreshNum of MBs are encoded as intra MBs in every P frame.

    @endverbatim
    */
    int             intraRefreshNum;

    //>>> addrRemap begin
    BOOL addrRemapEn;
    //>>> addrRemap end
} EncOpenParam;