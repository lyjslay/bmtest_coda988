/*
 * libbmruntime/test/test_1880_winograd.c
 *
 * Copyright Bitmain Technologies Inc.
 * Written by:
 *   Min Chen <min.chen@bitmain.com>
 * Created Time: 2018-03-08 12:07
 */

#include "boot_test.h"
#include "1880_test_util.h"
#include "timer.h"

#define nullptr NULL
#define WINOGRAD3_KH 4
#define WINOGRAD3_KW 4

#define NODECHIP_NUM 1
#define NPU_SHIFT 5
#define EU_NUM 16
#define LMEM_BANK_SIZE 0x10000

typedef struct {
  int RndSeed;
  int input_n;
  int input_c;
  int input_h;
  int input_w;
  int kw;
  int kh;
  int pad_h_t;
  int pad_h_b;
  int pad_w_l;
  int pad_w_r;
  int ins_h;
  int ins_h_last;
  int ins_w;
  int ins_w_last;
  int stride_h;
  int stride_w;
  int output_c;
  int output_w;
  int output_h;
  int using_bias;
  int rshiftbits;
  int ifrelu;
  int opd0_sign;
  int opd1_sign;
  int opd2_sign;
} winograd_param_t;

#if 0
static inline char *img2col_fix8b(
    char* img, int input_c, int input_h, int input_w,
    int pad_h_t, int pad_h_b, int pad_w_l, int pad_w_r,
    int ins_h, int ins_h_last, int ins_w, int ins_w_last,
    int stride_h, int stride_w, int kh, int kw)
{
  int ih_ext = pad_h_t + pad_h_b + (input_h - 1) * (ins_h + 1) + 1 + ins_h_last;
  int iw_ext = pad_w_l + pad_w_r + (input_w - 1) * (ins_w + 1) + 1 + ins_w_last;
  int oh = (ih_ext - kh) / stride_h + 1;
  int ow = (iw_ext - kw) / stride_w + 1;
  int conv_mac_size = input_c * kh * kw;
  char *img_col = xmalloc(oh * ow * conv_mac_size*4);
  for(int cidx = 0; cidx < input_c; cidx++){
    for(int ohidx = 0; ohidx < oh; ohidx++){
      for(int owidx = 0; owidx < ow; owidx++){
        for(int khidx = 0; khidx < kh; khidx++){
          for(int kwidx = 0; kwidx < kw; kwidx++){
            int ih_pos = ohidx * stride_h + khidx - pad_h_t;
            int iw_pos = owidx * stride_w + kwidx - pad_w_l;
            bool pos_ins0 = (ih_pos % (ins_h + 1)) > 0;
            pos_ins0 = pos_ins0 || ( (iw_pos % (ins_w + 1)) > 0 );
            bool pos_pad_0 = (ih_pos < 0) || ( iw_pos < 0);
            ih_pos = ih_pos / (ins_h + 1);
            iw_pos = iw_pos / (ins_w + 1);
            int col_idx = ohidx * (ow * conv_mac_size) + owidx * conv_mac_size +
                cidx * kh * kw + khidx * kw + kwidx;
            if( !pos_ins0 && !pos_pad_0 && (ih_pos < input_h) && (iw_pos < input_w) ){
              img_col[col_idx] = img[(cidx * input_h + ih_pos) * input_w + iw_pos];
            }
          }
        }
      }
    }
  }
  return img_col;
}
#else
static inline char *img2col_fix8b(
    char* img, int input_c, int input_h, int input_w,
    int pad_h_t, int pad_h_b, int pad_w_l, int pad_w_r,
    int ins_h, int ins_h_last, int ins_w, int ins_w_last,
    int stride_h, int stride_w)
{
  int ih_ext = pad_h_t + pad_h_b + (input_h - 1) * (ins_h + 1) + 1 + ins_h_last;
  int iw_ext = pad_w_l + pad_w_r + (input_w - 1) * (ins_w + 1) + 1 + ins_w_last;
  int oh = (ih_ext - 3) / stride_h + 1;
  int ow = (iw_ext - 3) / stride_w + 1;
  int conv_mac_size = input_c * 4 * 4;
  char *img_col = xmalloc(oh * ow * conv_mac_size*4);
  for(int cidx = 0; cidx < input_c; cidx++){
    for(int ohidx = 0; ohidx < oh; ohidx++){
      for(int owidx = 0; owidx < ow; owidx++){
        for(int khidx = 0; khidx < 4; khidx++){
          for(int kwidx = 0; kwidx < 4; kwidx++){
            int ih_pos = ohidx * stride_h + khidx - pad_h_t;
            int iw_pos = owidx * stride_w + kwidx - pad_w_l;
            bool pos_ins0 = (ih_pos % (ins_h + 1)) > 0;
            pos_ins0 = pos_ins0 || ( (iw_pos % (ins_w + 1)) > 0 );
            bool pos_pad_0 = (ih_pos < 0) || ( iw_pos < 0);
            ih_pos = ih_pos / (ins_h + 1);
            iw_pos = iw_pos / (ins_w + 1);
            int col_idx = ohidx * (ow * conv_mac_size) + owidx * conv_mac_size +
                cidx * 4 * 4 + khidx * 4 + kwidx;
            if( !pos_ins0 && !pos_pad_0 && (ih_pos < input_h) && (iw_pos < input_w) ){
              img_col[col_idx] = img[(cidx * input_h + ih_pos) * input_w + iw_pos];
            }
          }
        }
      }
    }
  }
  return img_col;
}
#endif

static void fill_int_with_char(int* pdest, char* psrc, int len)
{
  for(int ii=0; ii<len; ii++)
    pdest[ii] = (int)psrc[ii];
}

#if 0
static void fill_int_with_short(int* pdest, short* psrc, int len)
{
  for(int ii=0; ii<len; ii++)
    pdest[ii] = (int)psrc[ii];
}
#endif

static void fill_int_with_uchar(int *pdest, u8 *psrc, int len)
{
  for(int ii=0; ii<len; ii++)  {
    pdest[ii] = psrc[ii];
  }
}

static void matrix_mul(int* Y, int* A, int* B, int M, int N, int K)
{
  int ii, jj, kk;
  for(ii=0; ii<M; ii++)  {
    for(jj=0; jj<N; jj++)  {
      Y[ii*N+jj] = 0;
      for(kk=0; kk<K; kk++)  {
        Y[ii*N+jj] += A[ii*K+kk]*B[kk*N+jj];
      }
    }
  }
  return;
}

static void satu_2_8bit(
    int* pBuff, int len, char* pByteOut, int rshiftbits,
    int round_floor, int sign_unsign)
{
  ASSERT(rshiftbits>=0);
  int ii;
  int temp;
  int satu_max = (sign_unsign==0) ? 127 : 255;
  int satu_min = (sign_unsign==0) ? -128 : 0;
  if(rshiftbits==0)  {
    for(ii=0; ii<len; ii++)  {
      temp = (pBuff[ii]>satu_max) ? satu_max : ((pBuff[ii]<satu_min) ? satu_min : pBuff[ii]);
      memcpy(pByteOut+ii, &temp, 1);
    }
  } else {
    for(ii=0; ii<len; ii++)  {
      if(round_floor==1)
        temp = ((pBuff[ii]>>(rshiftbits-1))+1)>>1;
      else
        temp = pBuff[ii]>>rshiftbits;
      temp = (temp>satu_max) ? satu_max : ((temp<satu_min) ? satu_min : temp);
      memcpy(pByteOut+ii, &temp, 1);
    }
  }
  return;
}

static void native_winograd(
    winograd_param_t * p,
    char* IfMap,
    char* Weight,
    short* Bias,
    int* OfMap,
    char* OfMap_int8)
{
  int kh=4, kw=4;
  int pad_l = p->pad_w_l;
  int pad_r = p->pad_w_r;
  int pad_u = p->pad_h_t;
  int pad_d = p->pad_h_b;
  int Y[16];
  int R[4];
  int matrix_for_BIBT[256] = {
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    -1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 1, -1, 1, 0, -1, 1, -1, 0, 1, -1,
    1, 0, 0, 0, 0, -1, 1, 1, 0, 1, -1, -1, 0, -1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0,
    0, -1, -1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, -1, 1, -1, 0, 1, -1, 1, 0, 1, -1, 1, 0,
    0, 0, 0, 1, -1, -1, 0, -1, 1, 1, 0, -1, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, -1, 0, 0, 0, -1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1,
    1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, -1, -1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1
  };

  int len = p->input_w*p->input_h*p->input_c*p->input_n;
  int *p_nIfMap = xmalloc(len * sizeof(int));
  for(int ii=0; ii<len; ii++)
    p_nIfMap[ii] = IfMap[ii];

  len = kw*kh*p->input_c*p->output_c;
  int *p_nWeight = xmalloc(len * sizeof(int));
  for(int ii=0; ii<len; ii++)
    p_nWeight[ii] = Weight[ii];

  int with_bias = p->using_bias;
  int opd2_sign = p->opd2_sign;
  short *p_nBias = xmalloc(p->output_c * sizeof(short));
  u16 *p_uBias  = (u16*)p_nBias;
  for(int ii=0; ii<p->output_c; ii++)
    p_nBias[ii] = Bias[ii];

  int stride_h=2, stride_w=2;
  int ins_h=p->ins_h;
  int ins_last_h=p->ins_h_last;
  int ins_w=p->ins_w;
  int ins_last_w=p->ins_w_last;

  int output_h =(p->output_h + 1) /2;
  int output_w =(p->output_w + 1) /2;
  int *BIB = xmalloc(p->input_c*kh*kw*output_h*output_w * sizeof(int));
  int *rearrange_I = xmalloc(p->input_c*kh*kw*output_h*output_w * sizeof(int));

  for(int nind = 0; nind < p->input_n; nind++){
#if 0
    char *input_col = img2col_fix8b(
        IfMap + nind * p->input_c * p->input_h * p->input_w,
        p->input_c, p->input_h, p->input_w,
        pad_u, pad_d, pad_l, pad_r,
        ins_h, ins_last_h, ins_w, ins_last_w,
        stride_h, stride_w, kh, kw);
#else
    char *input_col = img2col_fix8b(
        IfMap + nind * p->input_c * p->input_h * p->input_w,
        p->input_c, p->input_h, p->input_w,
        pad_u, pad_d, pad_l, pad_r,
        ins_h, ins_last_h, ins_w, ins_last_w,
        stride_h, stride_w);
#endif

    if(p->opd0_sign)
      fill_int_with_char(rearrange_I, input_col, p->input_c*kh*kw*output_h*output_w);
    else
      fill_int_with_uchar(rearrange_I, (u8 *)input_col, p->input_c*kh*kw*output_h*output_w);

    free(input_col);

    matrix_mul(BIB, rearrange_I, matrix_for_BIBT, p->input_c*output_h*output_w, kh*kw, kh*kw);

    for (int oc = 0; oc < p->output_c; oc++) {
      for (int oh = 0; oh < output_h; oh++) {
        for (int ow = 0; ow < output_w; ow++) {
          for (int ii = 0; ii < kh*kw; ii++)
            Y[ii]=0;

          int addr1=p->input_c*kh*kw*(ow+oh*output_w);
          int addr2=p->input_c*kh*kw*oc;
          for (int idx = 0; idx < kh*kw; idx++) {
            for (int ic = 0; ic < p->input_c; ic++) {
              Y[idx]+=BIB[addr1+ic*kh*kw+idx]*p_nWeight[addr2+ic*kh*kw+idx];
            }
          }
          R[0] = Y[0]+Y[1]+Y[2]+Y[4]+Y[5]+Y[6]+Y[8]+Y[9]+Y[10];
          R[1] = Y[1]-Y[2]-Y[3]+Y[5]-Y[6]-Y[7]+Y[9]-Y[10]-Y[11];
          R[2] = Y[4]+Y[5]+Y[6]-Y[8]-Y[9]-Y[10]-Y[12]-Y[13]-Y[14];
          R[3] = Y[5]-Y[6]-Y[7]-Y[9]+Y[10]+Y[11]-Y[13]+Y[14]+Y[15];

          addr1 = nind*output_h*output_w*p->output_c+output_h*output_w*oc + oh*output_w + ow;
          short nBias = 0;
          if (with_bias)  {
            nBias = opd2_sign ? p_nBias[oc] : p_uBias[oc];
          }
          OfMap[4*addr1] = R[0]+nBias;
          OfMap[4*addr1+1] = R[1]+nBias;
          OfMap[4*addr1+2] = R[2]+nBias;
          OfMap[4*addr1+3] = R[3]+nBias;
        }
      }
    }
  }
  int output_h_div2 = output_h;
  int output_w_div2 = output_w;
  int *result = xmalloc(p->input_n*p->output_c*(p->output_h+1)*(p->output_w) * sizeof(int));
  int *Row0 = xmalloc(output_w_div2*2 * sizeof(int));
  int *Row1 = xmalloc(output_w_div2*2 * sizeof(int));
  output_w = p->output_w;
  output_h = p->output_h;
  int cnt=0;
  int hidx, widx, cidx, nidx;
  int cstr = p->output_h * p->output_w;
  int nstr = cstr * p->output_c;

  for(nidx=0; nidx<p->input_n; nidx++)  {
    for(cidx=0; cidx<p->output_c; cidx++)  {
      for(hidx=0; hidx<output_h_div2; hidx++)  {
        for(widx=0; widx<output_w_div2; widx++)  {
          Row0[2*widx] = OfMap[cnt++];
          Row0[2*widx+1] = OfMap[cnt++];
          Row1[2*widx] = OfMap[cnt++];
          Row1[2*widx+1] = OfMap[cnt++];
        }
        memcpy(result+hidx*2*output_w+cstr*cidx+nstr*nidx, Row0, output_w*sizeof(int));
        if(hidx*2+1<p->output_h)
          memcpy(result+(hidx*2+1)*output_w+cstr*cidx+nstr*nidx, Row1, output_w*sizeof(int));
      }
    }
  }

  satu_2_8bit(result, p->input_n*p->output_c*output_h*output_w, OfMap_int8, p->rshiftbits, 1, p->ifrelu);

  memcpy(OfMap, result, p->input_n*p->output_c*output_h*output_w*sizeof(int));

  free(rearrange_I);
  free(BIB);
  free(p_nWeight);
  free(p_nIfMap);
  free(p_nBias);
  free(result);
  free(Row0);
  free(Row1);
  return;
}

void GKGt_int8(char* Y, char* X, int num_connection,char *scale_f)  {
  short fX[9];
  short fY[16*num_connection];
  float Coef[13] = {0.5, 0.625, 0.75, 0.875, 1, 1.125, 1.25, 1.375, 1.5, 1.625, 1.75, 1.875, 2.0};
  float C[16][9] = {{1.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},
                    {0.50, 0.50, 0.50, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},
                    {0.50, -0.50, 0.50, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},
                    {0.00, 0.00, 1.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},
                    {0.50, 0.00, 0.00, 0.50, 0.00, 0.00, 0.50, 0.00, 0.00},
                    {0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25},
                    {0.25, -0.25, 0.25, 0.25, -0.25, 0.25, 0.25, -0.25, 0.25},
                    {0.00, 0.00, 0.50, 0.00, 0.00, 0.50, 0.00, 0.00, 0.50},
                    {0.50, 0.00, 0.00, -0.50, 0.00, 0.00, 0.50, 0.00, 0.00},
                    {0.25, 0.25, 0.25, -0.25, -0.25, -0.25, 0.25, 0.25, 0.25},
                    {0.25, -0.25, 0.25, -0.25, 0.25, -0.25, 0.25, -0.25, 0.25},
                    {0.00, 0.00, 0.50, 0.00, 0.00, -0.50, 0.00, 0.00, 0.50},
                    {0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 1.00, 0.00, 0.00},
                    {0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.50, 0.50, 0.50},
                    {0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.50, -0.50, 0.50},
                    {0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 1.00}};
  float fMax = 0;
  for(int conn=0; conn<num_connection; conn++)  {
    for(int ii=0; ii<9; ii++)  {
      fX[ii] = (short)X[9*conn+ii];
    }
    for(int ii=0; ii<16; ii++)  {
      fY[16*conn+ii] = 0;
      char right_shift_bit = -1;
      for(int jj=0; jj<9; jj++) {
        if (C[ii][jj] == 1) {
          right_shift_bit = 0;
        }
        else if (C[ii][jj] == 0.5 || C[ii][jj] == -0.5) {
          right_shift_bit = 1;
        }
        else if (C[ii][jj] == 0.25 || C[ii][jj] == -0.25) {
          right_shift_bit = 2;
        }
        if(right_shift_bit >= 0)  break;
      }
      for(int jj=0; jj<9; jj++)  {
        if (C[ii][jj] > 0) {
          fY[16*conn+ii]+=fX[jj];
        }
        else if (C[ii][jj] < 0) {
          fY[16*conn+ii]-=fX[jj];
        }
      }
      if (right_shift_bit > 0) {
        fY[16*conn+ii] = (fY[16*conn+ii] + (1 << (right_shift_bit - 1)))  >> right_shift_bit;
      }
      fMax = (fMax<fabs(fY[16*conn+ii])) ? fabs(fY[16*conn+ii]) : fMax;
    }
  }

  float coef = 0.0;
  if(fMax>255)
    coef = 0.5;
  else if(fMax<64)
    coef = 2;
  else  {
    for(int ii=0; ii<12; ii++)  {
      float Table[13] = {256.0000,  204.8000,  170.6667,  146.2857,  128.0000,  113.7778,  102.4000,   93.0909,   85.3333,   78.7692,   73.1429,   68.2667,   64.0000};
      if(fMax>=Table[ii + 1] && fMax<Table[ii])  {
        coef = Coef[ii];
        break;
      }
    }
  }
  *scale_f = coef * 8 - 1;

  for(int ii=0; ii<num_connection*16; ii++)  {
    //Y[ii] = ((fY[ii]*(*scale_f + 1)  >>  2) + 1) >> 1;//right shit 3bit and round_floor
    //Y[ii] = (Y[ii]>127) ? 127 : ((Y[ii]<-128) ? -128 : Y[ii]);
    fY[ii] = ((fY[ii]*(*scale_f + 1)  >>  2) + 1) >> 1;//right shit 3bit and round_floor
    Y[ii] = (fY[ii]>127) ? 127 : ((fY[ii]<-128) ? -128 : fY[ii]);
  }
  return;
}

int array_cmp_atomic_int8(
    char      *p_exp,
    char      *p_got,
    int         len,
    const char *info_label,
    int       delta)
{
  int idx = 0;
  for(idx = 0; idx < len; idx++){
    if(p_exp[idx]!=p_got[idx])  {
      printf("%s error at index %d exp %d got %d\n",
             info_label, idx, p_exp[idx], p_got[idx]);
      return -1;
    }
  }
  return 0;
}

int FindMedian(int*pData,int Count)
{
  int iTemp;  // step1: bubble sort
  for(int i=0;i<Count-1;i++)
  {
    for(int j=Count-1;j>i;j--)
    {
      if(pData[j]<pData[j-1])
      {
        iTemp=pData[j-1];
        pData[j-1]=pData[j];
        pData[j]=iTemp;
      }
    }
  }
  return pData[Count>>1];
}

static void nc_transpose(char out[], char in[], int n, int c, int h, int w)
{
  int hw_size = h * w;

  for (int ni = 0; ni < n; ni++) {
    for (int ci = 0; ci < c; ci++) {
      int src = (ni * c + ci) * hw_size;
      int dst = (ci * n + ni) * hw_size;
      for (int i = 0; i < hw_size; i++)
        out[dst + i] = in[src + i];
    }
  }
}

int test_winograd(winograd_param_t * p, bmctx_t *ctx)
{
  printf("%s\n", "Conv parameters:");
  printf("%s%d\n", "input_n=",p->input_n);
  printf("%s%d\n", "input_c=",p->input_c);
  printf("%s%d\n", "input_h=",p->input_h);
  printf("%s%d\n", "input_w=",p->input_w);
  printf("%s%d\n", "output_c=",p->output_c);

  printf("%s%d\n", "kh=",p->kh);
  printf("%s%d\n", "kw=",p->kw);
  printf("%s%d\n", "pad_h_t=",p->pad_h_t);
  printf("%s%d\n", "pad_h_b=",p->pad_h_b);
  printf("%s%d\n", "pad_w_l=",p->pad_w_l);
  printf("%s%d\n", "pad_w_r=",p->pad_w_r);
  printf("%s%d\n", "stride_h=",p->stride_h);
  printf("%s%d\n", "stride_w=",p->stride_w);
  printf("%s%d\n", "ins_w=",p->ins_w);
  printf("%s%d\n", "ins_h=",p->ins_h);
  printf("%s%d\n", "ins_w_last=",p->ins_w_last);
  printf("%s%d\n", "ins_h_last=",p->ins_h_last);

  printf("%s%d\n", "using_bias=",p->using_bias);
  printf("%s%d\n", "ReLU=",p->ifrelu);

  int in = p->input_n           ;
  int ic = p->input_c           ;
  int ih = p->input_h           ;
  int iw = p->input_w           ;
  int oc = p->output_c          ;

  int oh=p->output_h;
  int ow=p->output_w;

  printf("%s%d\n", "output_h=",oh);
  printf("%s%d\n", "output_w=",ow);

  char *input_data = xmalloc(in * ic * ih * iw);
  char *weight_data = xmalloc(oc * ic * WINOGRAD3_KH * WINOGRAD3_KW);
  char *output_data = xmalloc(in * oc * oh * ow);
  int *ofmap_ref = xmalloc(in * oc * (oh+1)/2*2 * (ow+1)/2*2 * sizeof(int));
  char *ofmap_ref_int8 = xmalloc(in * oc * (oh+1)/2*2 * (ow+1)/2*2);
  char* K = xmalloc(oc * ic * 9);
  short *bias_data = xmalloc(oc * sizeof(short));
  uint8_t *bias_data_lo_hi = (uint8_t *)xmalloc(2 * oc);

  memset(input_data, 0, (in * ic * ih * iw));
  srand(50);
  for (int idx = 0; idx < in * ic * ih * iw; idx++) {
    input_data[idx] = (rand()%21)-10;  //(rand()%256)-128;
  }

  memset(weight_data, 0, oc * ic * WINOGRAD3_KH * WINOGRAD3_KW);
  memset(K, 0, oc * ic * 9);
  memset(bias_data, 0, oc * sizeof(short));
  memset(bias_data_lo_hi, 0, 2 * oc);
  memset(output_data, 0, (in * oc * oh * ow));
  memset(ofmap_ref, 0, (in * oc * (oh+1)/2*2 * (ow+1)/2*2 * sizeof(int)));
  memset(ofmap_ref_int8, 0, (in * oc * (oh+1)/2*2 * (ow+1)/2*2));

  int* p_rshiftbits = xmalloc(oc * ic * sizeof(int));
  for (int idx = 0; idx < oc * ic; idx++){
    int sum=0;
    for(int idx2=0; idx2<9; idx2++) {
      // limit K value small and K(i)%4==0, to keep GKG'<256 and all char,
      // and equivalent to conventional convolution
      // K[9*idx+idx2] = (char)(rand()%256)-128;
      K[9*idx+idx2] = (char)(rand()%120)-60;
      K[9*idx+idx2] = (K[9*idx+idx2]/4)*4;
      sum += ((K[9*idx+idx2] > 0) ? K[9*idx+idx2] : -K[9*idx+idx2]);
    }
    p_rshiftbits[idx] = (int)floor(log2(sum));
  }
  p->rshiftbits = FindMedian(p_rshiftbits, oc*ic);
  free(p_rshiftbits);
  char winograd_scale_factor;
  GKGt_int8(weight_data, K, oc * ic,&winograd_scale_factor);

  for (int idx = 0;idx < oc;idx++){
    bias_data[idx] = (rand()%2)+1;  //(rand()%65536)-32768;
    //bias_data[idx] = idx % 128;
    bias_data_lo_hi[idx] = bias_data[idx] & 0xff; // low 8bit
    bias_data_lo_hi[idx + oc] = (bias_data[idx] >> 8) & 0xff; // high 8bit
  }

  for (int idx = 0;idx < in * oc * oh * ow; idx++){
    ofmap_ref[idx] = 0;
    ofmap_ref_int8[idx] = 0;
  }

  native_winograd(p, input_data, weight_data, bias_data, ofmap_ref, ofmap_ref_int8);

  gaddr_t gaddr_input = test_gmem_alloc(ctx, (in * ic * ih * iw));
  gaddr_t gaddr_weight = test_gmem_alloc(ctx, (1 * oc * 1 * (ic*9)));
  gaddr_t gaddr_output = test_gmem_alloc(ctx, (in * oc * oh * ow));
  gaddr_t gaddr_bias = 0;
  if (p->using_bias) {
    gaddr_bias = test_gmem_alloc(ctx, (2 * oc * 1 * 1));
  }

  // copy to device memory
  memcpy((uint8_t *)gaddr_input, (uint8_t*)input_data, (in * ic * ih * iw));
  if (p->using_bias) {
    memcpy((uint8_t *)gaddr_bias, (uint8_t*)bias_data_lo_hi, (2 * oc * 1 * 1));
  }
  char* K_tp = xmalloc(oc * ic * 9);
  nc_transpose(K_tp, K, oc, ic, 3, 3);
  memcpy((uint8_t *)gaddr_weight, (uint8_t*)K_tp, (1 * oc * 1 * (ic*9)));

  tensor_lmem *tl_ifmap = tl_alloc(shape_t4(in, ic, ih, iw), FMT_I8, CTRL_AL);
  tensor_lmem *tl_weight = tl_alloc(shape_t4(ic, oc, 1, 16), FMT_I8, CTRL_NULL);
  tensor_lmem *tl_ofmap = tl_alloc(shape_t4(in, oc, oh, ow), FMT_I8, CTRL_AL);
  bool tl_alloc_success = (tl_ifmap != nullptr) && (tl_weight != nullptr) && (tl_ofmap != nullptr);

  tensor_lmem * tl_weight_npu = tl_prealloc( tl_address(tl_weight),shape_t4(ic, oc, 4, 4),FMT_I8);

  tensor_lmem *tl_bias = nullptr;
  if (p->using_bias) {
    fmt_t bias_fmt_t = p->opd2_sign? FMT_I8: FMT_U8;
    tl_bias = tl_alloc(shape_t4(2, oc, 1, 1), bias_fmt_t, CTRL_NULL);
    if ((tl_bias == nullptr)) {
      tl_alloc_success = false;
    }
  }

  if (tl_alloc_success) {
    tl_load_fixed(tl_ifmap, gaddr_input, CTRL_NULL);
    tl_load_fixed(tl_weight, gaddr_weight, CTRL_WINOGRAD |(winograd_scale_factor * CTRL_WINOGRAD_SCALE_FACTOR));
    if (p->using_bias) {
      tl_load_fixed(tl_bias, gaddr_bias, CTRL_NULL);
    }
    test_bmkernel_submit();

    tl_winograd_fixed(
        tl_ifmap,
        tl_ofmap,
        tl_weight_npu,
        tl_bias,
        p->ins_h,
        p->ins_h_last,
        p->ins_w,
        p->ins_w_last,
        !p->ifrelu,
        p->opd0_sign,
        p->opd1_sign,
        p->opd2_sign,
        p->pad_h_t,
        p->pad_h_b,
        p->pad_w_l,
        p->pad_w_r,
//        p->stride_h,
//        p->stride_w,
        p->rshiftbits, // shift_width
        p->ifrelu); // relu_enable

    tl_store_fixed(tl_ofmap, gaddr_output, CTRL_NULL);

    test_bmkernel_submit();
  }else {
    printf("tl_alloc failed and goto exit\n");
  }
  if (p->using_bias) {
    tl_free(tl_bias);
  }
  if (tl_weight_npu) {
    tl_free(tl_weight_npu);
  }
  if (tl_ofmap) {
    tl_free(tl_ofmap);
  }
  if (tl_weight) {
    tl_free(tl_weight);
  }
  if (tl_ifmap) {
    tl_free(tl_ifmap);
  }

  // copy from device memory
  memcpy((uint8_t*)output_data, (uint8_t *)gaddr_output, (in * oc * oh * ow));

  int ofmap_size = in*oc*oh*ow;
  int cmp_res = array_cmp_atomic_int8(
      ofmap_ref_int8, output_data, ofmap_size,
      "Comparing results of native_winograd and atomic_winograd_neuron ...\n", 0);
  if(cmp_res != 0){
    printf("Comparison FAILED for native_winograd and atomic_winograd_neuron!!!\n");
    exit(-1);
  }

  // free device memory
  test_gmem_free(ctx, gaddr_input);
  test_gmem_free(ctx, gaddr_weight);
  test_gmem_free(ctx, gaddr_output);
  if (p->using_bias) {
    test_gmem_free(ctx, gaddr_bias);
  }

  free(input_data);
  free(weight_data);
  free(bias_data);
  free(output_data);
  free(ofmap_ref);
  free(ofmap_ref_int8);
  free(K);
  free(K_tp);

  printf("Comparison done for native_winograd and atomic_winograd_neuron!\n");
  return tl_alloc_success ? 1 : 0;
}

static bool check_local_size_enough_winograd(winograd_param_t *p)
{
  int in = p->input_n;
  int ic = p->input_c;
  int ih = p->input_h;
  int iw = p->input_w;
  int oc = p->output_c;
  int input_h_conv = p->pad_h_t + (p->ins_h + 1) * (ih - 1) +
      1 + p->ins_h_last + p->pad_h_b;
  int input_w_conv = p->pad_w_l + (p->ins_w + 1) * (iw - 1) +
      1 + p->ins_w_last + p->pad_w_r;
  int kh_ext = p->kh;
  int kw_ext = p->kw;
  int oh = (input_h_conv - kh_ext) / p->stride_h + 1;
  int ow = (input_w_conv - kw_ext) / p->stride_w + 1;
  int total_size;
  int isz;
  int num_per_npu;

  // input
  isz = ALIGN(ih*iw, EU_NUM);
  num_per_npu = ceiling_func_shift(ic, NPU_SHIFT) * in;
  total_size = isz*num_per_npu;

  // GKG
  isz = ALIGN(WINOGRAD3_KH*WINOGRAD3_KH, EU_NUM);
  num_per_npu = ceiling_func_shift(oc, NPU_SHIFT) * ic;
  total_size += isz*num_per_npu;

  // bias
  isz = ALIGN(1, EU_NUM);
  num_per_npu = ceiling_func_shift(oc, NPU_SHIFT);
  total_size += isz*num_per_npu*sizeof(short);

  // output
  isz = ALIGN(oh*ow, EU_NUM);
  num_per_npu = ceiling_func_shift(oc, NPU_SHIFT) * in;
  total_size += isz*num_per_npu;

  return total_size < LMEM_BANK_SIZE;
}


void random_winograd_param(winograd_param_t * p)
{
  static int entry=0;
  entry++;
  p->RndSeed = (int)timer_get_tick();
  srand(p->RndSeed);

  printf("test winograd rand seed: %d\n", p->RndSeed);

  //int npu_num = NPU_NUM;
  int eu_num = EU_NUM;
  p->using_bias = rand() % 2;
  p->ins_h_last = 0;
  p->ins_w_last = 0;
  bool random_success = false;

  while(!random_success){
    p->input_n = rand() % 2 + 1;
    p->input_c = rand() % (4) + 1;
    p->output_c = rand() % (2) + 1;
    p->input_h = rand() % 16 + 3;  // kh_ext
    p->input_w = rand() % 16 + 3 + 3;  // kw_ext

    p->ifrelu = rand()%2;//rand() % 7 + 1;
    p->opd0_sign = rand()%2;
    p->opd1_sign = 1; // must be signed
    p->opd2_sign = rand()%2;

    p->kh = 3;
    p->kw = 3;
    int kh_ext = p->kh;
    int kw_ext = p->kw;
    p->ins_h = 0;
    p->ins_w = 0;
    p->stride_h = 1;
    p->stride_w = 1;

    p->pad_h_t = rand()%3;
    p->pad_h_b = rand()%3;
    p->pad_w_l = rand()%3;
    p->pad_w_r = rand()%3;
    if((p->input_h+p->pad_h_t+p->pad_h_b)%2==1)
      p->pad_h_b += 1;
    if((p->input_w+p->pad_w_l+p->pad_w_r)%2==1)
      p->pad_w_r += 1;

    p->output_h = (p->input_h - 1) * (p->ins_h + 1) + p->ins_h_last + 1 +
        p->pad_h_t + p->pad_h_b - 3 + 1;
    p->output_w = (p->input_w - 1) * (p->ins_w + 1) + p->ins_w_last + 1 +
        p->pad_w_l + p->pad_w_r - 3 + 1;

    int ih_ext = 2 * (p->kh - 1) + (p->ins_h + 1) * (p->input_h - 1) + 1;
    int iw_ext = 2 * (p->kw - 1) + (p->ins_w + 1) * (p->input_w - 1) + 1;
    int oh = (ih_ext - kh_ext) / p->stride_h + 1;
    int ow = (iw_ext - kw_ext) / p->stride_w + 1;
    int oc_per_NPU = ceiling_func_shift(p->output_c, NPU_SHIFT);
    int of_count = (oh * ow + eu_num - 1) / eu_num;
    if( oc_per_NPU * of_count * p->kh * p->kw * p->input_c > 100000 ){
      random_success = false;
    }else if(!check_local_size_enough_winograd(p)){
      random_success = false;
    }else{
      random_success = true;
    }
  }
  printf("input_(n,c,h,w)=(%d, %d, %d, %d), oc=%d\n", p->input_n,p->input_c,p->input_h,p->input_w, p->output_c);

  printf("random_success=%d\n", random_success);
}

static void WINOGRAD_RegChecker(winograd_param_t* p)
{
  ASSERT(p->input_n>0);
  ASSERT(p->input_c>0);
  ASSERT(p->input_h>0);
  ASSERT(p->input_w>0);
  ASSERT(p->kh>0);
  ASSERT(p->kw>0);

  ASSERT(p->output_c>0);
  ASSERT(p->output_h>0);
  ASSERT(p->output_w>0);
  ASSERT(p->opd1_sign==1);
}

int testcase_winograd_main(bmctx_t *ctx)
{
  const int TEST_ITERATION_NUM = 2;
  int test_finished_num = 0;

  for (int i = 0; i < TEST_ITERATION_NUM; i++) {
    winograd_param_t p;
    memset(&p, 0, sizeof(p));
    random_winograd_param(&p);

    WINOGRAD_RegChecker(&p);
    test_finished_num += test_winograd(&p, ctx);
  }
  printf("Test finished %d/%d .\n", test_finished_num, TEST_ITERATION_NUM);
  return 0;
}

int testcase_winograd()
{
  bmctx_t ctx;
  test_init(&ctx);
  testcase_winograd_main(&ctx);
  test_exit(&ctx);
  return 0;
}

//module_testcase("0", testcase_main_winograd);

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
    return testcase_winograd();
}
#endif
