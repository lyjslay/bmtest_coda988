#ifndef _DPU_COMMON_H_
#define _DPU_COMMON_H_
/// \file common.h
/// \brief common types/enums/structures/macros/functions
///

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>

#ifndef __TYPEDEFS__
#define __TYPEDEFS__

#ifndef BITACCURATE_TYPES
typedef signed char     int8_t;
typedef signed short    int16_t;
typedef signed int      int32_t;
//typedef long long 		int64_t;
typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;
//typedef unsigned long long uint64_t;
#else
#include <ac_int.h>
typedef ac_int<8, true>   int8_t;
typedef ac_int<16,true>   int16_t;
typedef ac_int<32,true>   int32_t;
typedef ac_int<8, false>  uint8_t;
typedef ac_int<16,false>  uint16_t;
typedef ac_int<32,false>  uint32_t;
#endif
#endif

typedef uint8_t           CVI_U8;
typedef uint8_t           CVI_UCHAR;
typedef uint16_t          CVI_U16;
typedef uint32_t          CVI_U32;
typedef int8_t            CVI_S8;
typedef int16_t           CVI_S16;
typedef int32_t           CVI_S32;
//typedef uint64_t          CVI_U64;
//typedef int64_t           CVI_S64;
typedef char              CVI_CHAR;

typedef enum FRAME_TYPE {
     TOP_FIELD = 0,
     BOT_FIELD = 1,
     PROGRESSIVE = 2,
     RAW = 3,
} FRAME_TYPE;

typedef enum COLOR_RGB {
     R = 0,
     G = 1,
     B = 2,
} COLOR_RGB;

typedef enum COLOR_YCBCR {
     Y  = 0,
     CB = 1,
     CR = 2
} COLOR_YCBCR;

typedef enum COLOR_FORMAT {
     RGB = 0,
     YCBCR = 1,
     GRAY = 2,
     RAW_RGB = 3,
     RAW_RGBIR = 4
} COLOR_FORMAT;

typedef enum BAYER_RGB {
     BAYER_BG = 0,
     BAYER_GB = 1,
     BAYER_GR = 2,
     BAYER_RG = 3
} BAYER_RGB;

typedef enum HIST_CH {
     HIST_RAW_R     = 0,
     HIST_RAW_G     = 1,
     HIST_RAW_B     = 2,
     HIST_RGB_V     = 3,
     HIST_RGB_Y     = 4,
     HIST_RAW_IR    = 5,
} HIST_CH;

typedef struct FRAME {
     FRAME_TYPE     type;
     COLOR_FORMAT   format;
     int            index;
     int            width;
     int            height;
     int            channel;
     int            depth;
     BAYER_RGB      bayer_id;
     int            *data;
} FRAME;

typedef struct HIST {
     int            nbins;
     int            depth;
     int            channel;
     int            *data;
} HIST;



//Motion Vector structure
typedef struct
{
  short mv_x;
  short mv_y;
} MotionVector;

// Bayer ID
typedef enum {
    BAYER_ID_BG = 0,
    BAYER_ID_GB = 1,
    BAYER_ID_GR = 2,
    BAYER_ID_RG = 3,
    BAYER_ID_RGB = 4,
    BAYER_ID_YUV = 5,
    BAYER_ID_IR  = 6,
    BAYER_ID_DUMMY = 7,
    IRBAYER_ID_GRGBI = 8,  // Convert to "RGGB"
    IRBAYER_ID_RGBGI = 9,  // Convert to "RGGB"
    IRBAYER_ID_GBGRI = 10, // Convert to BGGR
    IRBAYER_ID_BGRGI = 11, // Convert to BGGR
    IRBAYER_ID_IGRGB = 12, // Convert to "RGGB"
    IRBAYER_ID_IRGBG = 13, // Convert to "RGGB"
    IRBAYER_ID_IBGRG = 14, // Convert to BGGR
    IRBAYER_ID_IGBGR = 15  // Convert to BGGR
} BAYER_ID;




//-----------------------------------------------------------------------------------------
// MACRO definitions
//-----------------------------------------------------------------------------------------

#define ASR(x,n)              ((x)<0 ? ~((~(x)) >> (n)) : (x) >> (n))
#define ABS(x)                ( (x) < 0 ? -(x) : (x) )
#define MIN(x,y)              ( ((x) < (y)) ? (x) : (y) )
#define MIN3(a,b,c)           (MIN(a, MIN(b, c)))
#define MAX(x,y)              ( ((x) < (y)) ? (y) : (x) )
#define MAX3(a,b,c)           (MAX(a, MAX(b, c)))
#define MEDIAN(a,b,c)         ( MIN(MAX( MIN((a),(b)), (c)), MAX((a),(b))) )
#define CLIP(x,xl,xh)         ( MIN(MAX((x),(xl)),(xh)))
#define MINMAX(x,min,max)     ( ((x)<min)? min:(((x)>max)? max:(x)) )
#define SIGN(x)               (((x) < 0) ? -1 : 1)
#define LIMIT(a, up, low)     (MIN(MAX((a), (low)), (up)))

/// MIRROR:
///   -1 to 1, -2 to 2, ...
///   width to width-2, width+1 to width-3,...
#define MIRROR(x, width)      ( ((x) < 0)? (-(x)):(((x) > (width)-1)? (2*(width)-2 -(x)):(x)) )
///   -1 to 0, -2 to 1, ...
///   width to width-1, width+1 to width-2,...
#define MIRROR_SYMMETRIC(x, width)      ( ((x) < 0)? (-(x)-1:(((x) > (width)-1)? (2*(width)-1 -(x)):(x)) )
#define REPEAT(x, width)      ( ((x) < 0)? (0):(((x) > (width)-1)? ((width) - 1):(x)) )

/// Access PIXEL(x,y,ch) in a Frame:
///   frame:    pointer of a frame struct
///   x,y:      pixel location
///   ch:       pixel channel
#define SETPIXEL(frame,x,y,ch,val)     (*(frame->data + (MIRROR((y), frame->height) * frame->width + MIRROR((x),frame->width)) * frame->channel + (ch)) = val)
#define PIXEL(frame,x,y,ch)        (*(frame->data + (MIRROR((y), frame->height) * frame->width + MIRROR((x),frame->width)) * frame->channel + (ch)))
#define BLK_PIXEL(frame, blk_w, blk_h, x, y, ch)  PIXEL(frame, (x/blk_w), (y/blk_h), ch)
#define HIST_P(hist,x,ch)     (*(hist->data + MIRROR((x), hist->nbins) * hist->channel + (ch)))
#define PIXEL_REPEAT(frame,x,y,ch)        (*(frame->data + (REPEAT((y), frame->height) * frame->width + REPEAT((x),frame->width)) * frame->channel + (ch)))

/// Unsigned floor - should not be used in hw.c
#define UFLOOR(x)   (floor(x))
#define UROUND(x)   (floor(x + 0.5))
/// Signed floor - should not be used in hw.c
#define SFLOOR(x)   (((x) >= 0)? UFLOOR(x):(-UFLOOR(-(x))))
#define SROUND(x)   (((x) >= 0)? UROUND(x):(-UROUND(-(x))))

#define BASEUNIT_HALF(x,y)    (x<<(y-1))
#define UROUND_BITWISE(x,y)   ((x+BASEUNIT_HALF(1,y))>>y)
#define SROUND_BITWISE(x,y,depth_out)   (UROUND_BITWISE(ABS(x-(1<< (depth_out-1+y))),y) * ((x >= (1<< (depth_out-1+y)))? 1:-1) + (1<< (depth_out-1)))

/// Bit mask - provide a bit mask of x bits, ex: BIT_MASK(8) = 0xFF
///  - this macro which will work for arbitrary positive inputs
///  - do NOT use function as its inputs (a double evaluation macro)
///  Max input: 32
#define BIT_MASK(x)             (((x) >= sizeof(unsigned) * CHAR_BIT) ?  (unsigned) -1 : (1U << (x)) - 1)
///  Max input: 64 but mind casting needs for this long
#define BIT_MASK64BIT(x)        (((x) >= sizeof(unsigned long) * CHAR_BIT) ?  (unsigned long) -1 : (1UL << (x)) - 1)
/// Bit mask with input - apply a bit mask of x bits and then apply it to input i (you can use sizeof(i) as x if its size are correct)
#define SET_MASK(i,x)           (i & BIT_MASK(x))

/// \def CM_INTERPOLATE(top,bottom,diff,base)
/// \brief A macro for common interpolation of the position \a diff in a range of (\a top, \a bottom) with a floating point at \a base \n
///         EXAMPLE: original - u4RGamma[idx] = ((idx - X1) * (Y2_R - Y1_R) + (Y1_R << GMA_LUT_BASE) + GMA_LUT_BASE_D2) >> GMA_LUT_BASE; \n
///                  macro'ed - u4RGamma[idx] = CM_INTERPOLATE(Y2_R, Y1_R, (idx - X1), (GMA_LUT_BASE)); \n
/// \param top      the topp element of interpolation
/// \param bottom   the bottom element of interpolation
/// \param diff     the difference bettwen the interpolated point and the bottom element
/// \param base     the position of floating point
#define CM_INTERPOLATE(top,bottom,diff,base) (((diff) * (top - bottom) + (bottom << base) + BASEUNIT_HALF(1,base)) >> base)

#define PAUSE printf("Press Enter key to continue..."); fgetc(stdin);


#define STR(name) #name
#define TOSTR(name) STR(name)
#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
#define READ_REG(section, var, item, value) if (MATCH(TOSTR(section), TOSTR(item))) { var.item = atoi(value);return 1;}
#define READ_ARRAY(reg) \
		tok = strtok((char *)value,","); \
		pconfig->hw.reg[tok_idx] = atoi(tok);\
		while(tok!=NULL) {\
			tok_idx++;\
			tok =strtok(NULL," ,");\
			if(tok!= NULL)\
			  pconfig->hw.reg[tok_idx] = atoi(tok);\
		}

// e.g. READ_REG(demosaic, pconfig->hw, reg_cfa_dc_filter_mode, value)
#define MAKE_FN_PROTO(x,y,z,...) z x ## _ ## y ( __VA_ARGS__ )
#define FUNCTION_PROTO(mod, func, rt, parm) MAKE_FN_PROTO(mod, func, rt, parm)
#define MAKE_FN_NAME(x, y) x ## _ ## y
#define FUNCTION_NAME(mod, func) MAKE_FN_NAME(mod, func)
#define GET_SLOPE(x0,x1,y0,y1,unit) ((x1)!=(x0))?(((long long)unit)*(long long)((y1)-(y0))/(float)((x1)-(x0))):unit

//-----------------------------------------------------------------------------------------
// COMMON Functions
//-----------------------------------------------------------------------------------------
FRAME *CM_NewFRAME(FRAME_TYPE type, COLOR_FORMAT format, int width, int height, int channel, int depth);
FRAME *CM_CopyFRAME(FRAME *img);
FRAME *CM_CopySingleCh(FRAME *img, int ch);
FRAME *CM_Merge3Ch(FRAME *src0, FRAME *src1, FRAME *src2, COLOR_FORMAT format);
int CM_DelFRAME(FRAME *img);
FRAME *CM_ReadPPM(const char *filename, FRAME_TYPE type);      // read RGB image
FRAME *CM_ReadPPM5(const char *filename, FRAME_TYPE type);     // read gray scale image
int CM_WritePPM(const char *filename, FRAME *img);             // write RGB image
int CM_WritePPM5(const char *filename, FRAME *img);            // write gray scale image
void CM_PPM6toPPM3(const char *filename, const char *filename_w, FRAME_TYPE type, char *dbg_path, const int frame_idx);
int CM_WritePPM3(const char *filename, FRAME *img, char *dbg_path, const int frame_idx);             // write RGB image
FRAME *CM_RGB2YCbCr(FRAME *img);
FRAME *CM_YCbCr2RGB(FRAME *img);
FRAME *CM_GetField(FRAME *img, FRAME_TYPE field);
void Frame_Set_Constant(FRAME *frame_in, int constant);

/**
 * @brief alpha blending function
 *
 * @param lut_in the input value for horizontal axis
 * @param in_th_left the left node for horizontal axis
 * @param weight_l weighting for left node
 * @param weight_r weighting for right node
 * @param slope slope value, calculate with common_sw
 * @param slope_shift decimal place precision - numbr of shift bit
 * @return (int) the result of alpha blending, will between weight_l and weight_r
 */
int CM_LUT3(int lut_in, int in_th_left, int weight_l, int weight_r, int slope, int slope_shift);

int CM_BlendIIR(int A, int B, int Alpha, int Shift);                  // IIR for Y or unsigned channel
int CM_BlendIIR_C(int A, int B, int Alpha, int Shift, int AB_Depth);  // IIR for C or signed channel
int CM_BlendR(int A, int B, int Alpha, int Shift);
FRAME *CM_ReadRAW(const char *filename, COLOR_FORMAT format, int fidx, int hdr_mode, int width, int height, int depth);
int CM_WriteRAW(const char *filename, FRAME *raw);
int CM_RAW2PPM6(const char *filename, FRAME *raw, int target_depth);
int CM_RAW2PPM5(const char *filename, FRAME *raw, int target_depth);
int CM_RGB2PPM(const char *filename, FRAME *rgb, int target_depth);
HIST *CM_NewHIST(int nbins, int depth, int channel);
int CM_DelHIST(HIST *hist);
int CM_GET_COLOR_ID(int row, int col, int id);
int CM_YCbCr444to420(FRAME *YCbCr444, FRAME *Y420, FRAME *Cb420, FRAME *Cr420);
int CM_YCbCr420to444(FRAME *Y420, FRAME *Cb420, FRAME *Cr420, FRAME *YCbCr444);
int CM_YCbCr444to422(FRAME *YCbCr444, FRAME *Y422, FRAME *Cb422, FRAME *Cr422);
int CM_YCbCr422to444(FRAME *Y422, FRAME *Cb422, FRAME *Cr422, FRAME *YCbCr444);
int CM_interp1(int in_val, int x0, int y0, int y1, int slope, int sft) ;
int CM_LUT(int in_val, int *lut_in, int *lut_out, int *lut_slope, int lut_num, int sft_bit);
int CM_WriteHIST(const char *filename, HIST *hist);
char *CM_StringConcat(char *str1, char *str2);
int CM_SingleIntp(int x0, int x1, int y0, int y1, int x);
int CM_LutBilinearInterp(int* LUT, int x0, int nbits_t, int nbits_m);
#endif
