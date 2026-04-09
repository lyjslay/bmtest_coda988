/*
 * libbmruntime/test/test_1880_pooling.cpp
 *
 * Copyright Bitmain Technologies Inc.
 * Written by:
 *   Wanwei CAI <wanwei.cai@bitmain.com>
 * Created Time: 2018-02-22 14:36
 */
#include "boot_test.h"
#include "1880_test_util.h"
#include "timer.h"

#define nullptr NULL
#define NPU_SHIFT 5

typedef enum {
    POOLING_MAX,
    POOLING_AVG,
    POOLING_DEPTHWISE,
} POOLING_TYPE;


typedef struct {
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
  int stride_h;
  int stride_w;
  int ins_w;
  int ins_h;
  int ins_w_last;
  int ins_h_last;
  POOLING_TYPE pooling_type;
  uint8_t avg_pooling_const;
  int r_shift_width;
  int start_local_mem_idx;
  int using_bias;
  int opd0_sign;
  int opd1_sign;
  int opd2_sign;
  int do_relu;
} pooling_param_t;

static int8_t *insert_value(
    int8_t *before, int val,
    int pad_l, int pad_r, int pad_t, int pad_b,
    int ins_h, int ins_w, int ins_h_last, int ins_w_last,
    int h_before, int w_before)
{
  int w_after = (w_before - 1) * (ins_w + 1) + ins_w_last + 1 + pad_l + pad_r;
  int h_after = (h_before - 1) * (ins_h + 1) + ins_h_last + 1 + pad_t + pad_b;
  int8_t *after = xmalloc(w_after * h_after);

  memset(after, val, w_after * h_after);
  for (int h = 0; h < h_before; h++) {
    for (int w = 0; w < w_before; w++) {
      int i = (h * (ins_h + 1) + pad_t) * w_after + w * (ins_w + 1) + pad_l;
      after[i] = before[h * w_before + w];
    }
  }
  return after;
}

static void mac_fix(int* a, int* b, int len, int *c)
{
  *c = 0;
  for(int ii=0; ii<len; ii++) {
    *c += (a[ii]*b[ii]);
  }
}

static void satu_2_8bit(
    int* pBuff, int len, int8_t* pByteOut, int rshiftbits,
    int round_floor, int sign_unsign)
{
  assert(rshiftbits>=0);
  int temp;
  int satu_max = (sign_unsign == 0) ? 127 : 255;
  int satu_min = (sign_unsign == 0) ? -128 : 0;
  if(rshiftbits == 0)  {
    for (int ii=0; ii<len; ii++)  {
      temp = (pBuff[ii]>satu_max) ? satu_max : ((pBuff[ii]<satu_min) ? satu_min : pBuff[ii]);
      memcpy(pByteOut+ii, &temp, 1);
    }
  } else {  // rshiftbits>0
    for (int ii=0; ii<len; ii++)  {
      if(round_floor == 1)
        temp = ((pBuff[ii]>>(rshiftbits-1))+1)>>1;
      else
        temp = pBuff[ii]>>rshiftbits;
      temp = (temp>satu_max) ? satu_max : ((temp<satu_min) ? satu_min : temp);
      memcpy(pByteOut+ii, &temp, 1);
    }
  }
  return;
}

static void native_pooling(
    const pooling_param_t p_param,
    int8_t* i_fmap,
    int8_t* weight,
    short *bias,
    int8_t* o_fmap)
{
  int pooling_type = p_param.pooling_type;
  int input_n = p_param.input_n;
  int input_c = p_param.input_c;
  int input_h = p_param.input_h;
  int input_w = p_param.input_w;
  int kh = p_param.kh;
  int kw = p_param.kw;
  int pad_h_t = p_param.pad_h_t;
  int pad_h_b = p_param.pad_h_b;
  int pad_w_l = p_param.pad_w_l;
  int pad_w_r = p_param.pad_w_r;
  int stride_h = p_param.stride_h;
  int stride_w = p_param.stride_w;
  int ins_w = p_param.ins_w;
  int ins_h = p_param.ins_h;
  int ins_w_last = p_param.ins_w_last;
  int ins_h_last = p_param.ins_h_last;
  if (pooling_type == POOLING_MAX) {
    assert(ins_w == 0);
    assert(ins_h == 0);
    assert(ins_w_last == 0);
    assert(ins_h_last == 0);
  }
  uint8_t avg_pooling_const = p_param.avg_pooling_const;
  int8_t *weight_orig = weight;

  int *avg_pooling_mac_a = (int *)xmalloc(kh * kw * sizeof(int));
  int *avg_pooling_mac_b = (int *)xmalloc(kh * kw * sizeof(int));

  int h_after = (input_h - 1) * (ins_h + 1) + ins_h_last + 1 + pad_h_t + pad_h_b;
  int w_after  = (input_w - 1) * (ins_w + 1) + ins_w_last + 1 + pad_w_l + pad_w_r;

  int output_h = (h_after - kh)/stride_h + 1;
  int output_w = (w_after - kw)/stride_w + 1;
  int opd0_sign = p_param.opd0_sign;
  int opd1_sign = p_param.opd1_sign;
  int opd2_sign = p_param.opd2_sign;

  if (pooling_type == POOLING_AVG)
    assert(opd1_sign==0);
  if (pooling_type == POOLING_DEPTHWISE)
    assert(opd1_sign==1);
  assert(opd2_sign==1);
  assert(p_param.do_relu ==0);

  if (POOLING_MAX == pooling_type) {
    int max_init = opd0_sign? -128: 0;
    for (int n = 0; n < input_n; n++) {
      for (int c = 0; c < input_c; ++c) {
        int8_t *i_fmap_pad = insert_value(
            i_fmap, max_init, pad_w_l, pad_w_r, pad_h_t, pad_h_b,
            0, 0, 0, 0, input_h, input_w);
        for (int ph = 0; ph < output_h; ++ph) {
          for (int pw = 0; pw < output_w; ++pw) {
            int hstart = ph * stride_h;
            int wstart = pw * stride_w;
            int pool_index = ph * output_w + pw;
            int max = max_init;
            for (int h = 0; h < kh; h++) {
              for (int w = 0; w < kw; w++) {
                int index = (hstart + h) * (input_w + pad_w_l + pad_w_r) + (w + wstart);
                int val = opd0_sign? i_fmap_pad[index]: (uint8_t)i_fmap_pad[index];
                if (val > max)
                  max = val;
              }
            }
            o_fmap[pool_index] = max;
          }
        }
        i_fmap += input_w * input_h;
        o_fmap += output_w * output_h;

        free(i_fmap_pad);
      }
    }
  } else {
    for (int idx = 0; idx < input_n * input_c * output_h * output_w; idx++) {
      o_fmap[idx] = 0;
    }

    for (int n = 0; n < input_n; n++) {
      weight = weight_orig;
      for (int c = 0; c < input_c; ++c) {
        int8_t *i_fmap_pad = insert_value(
            i_fmap, 0, pad_w_l, pad_w_r, pad_h_t, pad_h_b,
            ins_h, ins_w, ins_h_last, ins_w_last, input_h, input_w);
        for (int ph = 0; ph < output_h; ++ph) {
          for (int pw = 0; pw < output_w; ++pw) {

            int hstart = ph * stride_h;
            int wstart = pw * stride_w;
            const int pool_index = ph * output_w + pw;
            int mac_index = 0;
            int avg_pool_result;

            for (int h = 0; h < kh; h++) {
              for (int w = 0; w < kw; w++) {
                const int index = (hstart + h) * (w_after) + (w + wstart);
                mac_index = h * kw + w;
                if (opd0_sign == 0) {
                  avg_pooling_mac_a[mac_index] =(int)((uint8_t)(i_fmap_pad[index]));
                } else {
                  avg_pooling_mac_a[mac_index] = i_fmap_pad[index];
                }
                if (pooling_type == POOLING_AVG){
                  //unsigned weight for average
                  avg_pooling_mac_b[mac_index] = avg_pooling_const;
                } else if(pooling_type == POOLING_DEPTHWISE){
                  //signed weight for depthwise
                  avg_pooling_mac_b[mac_index] = weight[mac_index];
                }
              }
            }

            if (kh * kw >= 2) {
              mac_fix(avg_pooling_mac_a, avg_pooling_mac_b, kh * kw, &avg_pool_result);
            } else if(kh * kw == 1){
              avg_pool_result = avg_pooling_mac_a[0] * avg_pooling_mac_b[0];
            } else{
              printf("not support ... pooling");
              exit(-1);
            }
            // add bias
            if((pooling_type == POOLING_DEPTHWISE) && p_param.using_bias) {
              avg_pool_result += bias[c] ; //bias+c ;signed
            }
            if(pooling_type == POOLING_AVG) {
              satu_2_8bit(&avg_pool_result, sizeof(int8_t), o_fmap + pool_index,
                          p_param.r_shift_width, 1, opd0_sign== 0);
            }
            if(pooling_type == POOLING_DEPTHWISE) {
              satu_2_8bit(&avg_pool_result, sizeof(int8_t), o_fmap + pool_index,
                          p_param.r_shift_width, 1, p_param.do_relu);
            }
          }
        }

        i_fmap += input_w * input_h;
        if(pooling_type == POOLING_DEPTHWISE) {
          weight += kh * kw;
        }
        o_fmap += output_w * output_h;
        free(i_fmap_pad);
      }
    }
  }

  free(avg_pooling_mac_a);
  free(avg_pooling_mac_b);
}

int array_cmp_int8(const char * const info, int8_t *p_exp, int8_t *p_got, int count)
{
  int idx;
  for (idx = 0; idx < count; idx++) {
    if (p_exp[idx] != p_got[idx]) {
      printf("%s error at index %d exp %d got %d\n",
          info, idx, p_exp[idx], p_got[idx]);
      return -1;
    }
  }
  return 0;
}

static void print_pooling_param(const pooling_param_t p_param)
{
  printf("  %s\n", "Pooling parameters:");
  printf("  %s%d;\n",  "input_n = ", p_param.input_n);
  printf("  %s%d;\n",  "input_c = ", p_param.input_c);
  printf("  %s%d;\n",  "input_h = ", p_param.input_h);
  printf("  %s%d;\n",  "input_w = ", p_param.input_w);
  printf("  %s%d;\n",  "kh = ", p_param.kh);
  printf("  %s%d;\n",  "kw = ", p_param.kw);
  printf("  %s%d;\n",  "pad_h_t = ", p_param.pad_h_t);
  printf("  %s%d;\n",  "pad_h_b = ", p_param.pad_h_b);
  printf("  %s%d;\n",  "pad_w_l = ", p_param.pad_w_l);
  printf("  %s%d;\n",  "pad_w_r = ", p_param.pad_w_r);
  printf("  %s%d;\n",  "stride_h = ", p_param.stride_h);
  printf("  %s%d;\n",  "stride_w = ", p_param.stride_w);
  printf("  %s%d;\n",  "ins_w = ", p_param.ins_w);
  printf("  %s%d;\n",  "ins_h = ", p_param.ins_h);
  printf("  %s%d;\n",  "ins_w_last = ", p_param.ins_w_last);
  printf("  %s%d;\n",  "ins_h_last = ", p_param.ins_h_last);
  printf("  %s%d;\n",  "pooling_type = ", p_param.pooling_type);
  printf("  %s%d;\n",  "avg_pooling_const = ", p_param.avg_pooling_const);
  printf("  %s%d;\n",  "r_shift_width = ", p_param.r_shift_width);
  printf("  %s%d;\n",  "start_local_mem_idx = ", p_param.start_local_mem_idx);
  printf("  %s%d;\n",  "using_bias = ", p_param.using_bias);
  printf("  %s%d;\n",  "opd0_sign = ", p_param.opd0_sign);
  printf("  %s%d;\n",  "opd1_sign = ", p_param.opd1_sign);
  printf("  %s%d;\n",  "opd2_sign = ", p_param.opd2_sign);
  printf("  %s%d;\n",  "do_relu = ", p_param.do_relu);
}

static int test_pooling(const pooling_param_t p_param, bmctx_t *ctx)
{
  int in = p_param.input_n;
  int ic = p_param.input_c;
  int ih = p_param.input_h;
  int iw = p_param.input_w;
  int kh = p_param.kh;
  int kw = p_param.kw;
  int oc = ic; // Note: oc == ic even POOLING_DEPTHWISE

  int ins_h      = p_param.ins_h;
  int ins_h_last = p_param.ins_h_last;
  int ins_w      = p_param.ins_w;
  int ins_w_last = p_param.ins_w_last;

  int h_after = (ih - 1) * (ins_h + 1) + ins_h_last + 1 + p_param.pad_h_t + p_param.pad_h_b;
  int w_after  = (iw - 1) * (ins_w + 1) + ins_w_last + 1 + p_param.pad_w_l + p_param.pad_w_r;

  int oh = (h_after - kh)/p_param.stride_h + 1;
  int ow = (w_after - kw)/p_param.stride_w + 1;

  int8_t *input_data = (int8_t *)xmalloc(in * ic * ih * iw);
  int8_t *weight_data = (int8_t *)xmalloc(1 * ic * kh * kw);
  int8_t *output_data = (int8_t *)xmalloc(in * oc * oh * ow);
  int8_t *output_data_ref = (int8_t *)xmalloc(in * oc * oh * ow);
  short *bias_data = (short *)xmalloc(oc * sizeof(short));
  uint8_t *bias_data_lo_hi = (uint8_t *)xmalloc(2 * oc);

  srand((int)timer_get_tick());
  for (int idx = 0; idx < in * ic * ih * iw; idx++) {
    input_data[idx] = rand() % 256 - 128;
  }

  for (int idx = 0;idx < 1 * ic * kh * kw;idx++){
    weight_data[idx] = rand() % 256 - 128;
  }

  if (p_param.using_bias) {
    for (int idx = 0;idx < oc;idx++){
      bias_data[idx] = rand() % 65536 - 32768;
      bias_data_lo_hi[idx] = bias_data[idx] & 0xff; // low 8bit
      bias_data_lo_hi[idx + oc] = (bias_data[idx] >> 8) & 0xff; // high 8bit
    }
  }

  for (int idx = 0;idx < in * oc * oh * ow; idx++){
    output_data_ref[idx] = 0;
  }

  native_pooling(p_param, input_data, weight_data, bias_data, output_data_ref);

  // alloc memory
  gaddr_t gaddr_input = test_gmem_alloc(ctx, (in * ic * ih * iw));
  gaddr_t gaddr_weight = test_gmem_alloc(ctx, (1 * ic * kh * kw));
  gaddr_t gaddr_output = test_gmem_alloc(ctx, (in * oc * oh * ow));
  gaddr_t gaddr_bias = 0;

  // copy to device memory
  memcpy((uint8_t *)gaddr_input, (uint8_t*)input_data, (in * ic * ih * iw));
  memcpy((uint8_t *)gaddr_weight, (uint8_t*)weight_data, (1 * ic * kh * kw));

  if (p_param.using_bias) {
    gaddr_bias = test_gmem_alloc(ctx, (2 * oc * 1 * 1));
    memcpy((uint8_t *)gaddr_bias, (uint8_t*)bias_data_lo_hi, (2 * oc * 1 * 1));
  }

  fmt_t input_fmt_t;
  if (p_param.opd0_sign) {
    input_fmt_t = FMT_I8;
  }
  else {
    input_fmt_t = FMT_U8;
  }

  tensor_lmem *tl_ifmap = tl_alloc(shape_t4(in, ic, ih, iw), input_fmt_t, CTRL_AL);
  tensor_lmem *tl_weight = tl_alloc(shape_t4(1, ic, kh, kw), FMT_I8, CTRL_AL);
  tensor_lmem *tl_ofmap = tl_alloc(shape_t4(in, oc, oh, ow), FMT_I8, CTRL_AL);

  bool tl_alloc_success = (tl_ifmap != nullptr) &&
      (tl_weight != nullptr) && (tl_ofmap != nullptr);

  tensor_lmem *tl_bias = nullptr;
  if (p_param.using_bias) {
    tl_bias = tl_alloc(shape_t4(2, oc, 1, 1), FMT_I8, CTRL_NULL);
    if ((tl_bias == nullptr)) {
      tl_alloc_success = false;
    }
  }

  if (tl_alloc_success) {
    tl_load_fixed(tl_ifmap, gaddr_input, CTRL_NULL);
    tl_load_fixed(tl_weight, gaddr_weight, CTRL_NULL);
    if (p_param.using_bias) {
      tl_load_fixed(tl_bias, gaddr_bias, CTRL_NULL);
    }

//      test_bmkernel_submit();
//      printf("bmk_submit end\n");

    switch (p_param.pooling_type) {

      case POOLING_MAX:
        tl_max_pooling_fixed(
                tl_ifmap, tl_ofmap,
                p_param.kh, p_param.kw,
                p_param.ins_h, p_param.ins_h_last, p_param.ins_w, p_param.ins_w_last,
                p_param.pad_h_t, p_param.pad_h_b, p_param.pad_w_l, p_param.pad_w_r,
                p_param.stride_h, p_param.stride_w);
            break;

      case POOLING_AVG:
        tl_avg_pooling_fixed(
                tl_ifmap, tl_ofmap,
                p_param.kh, p_param.kw,
                p_param.ins_h, p_param.ins_h_last, p_param.ins_w, p_param.ins_w_last,
                p_param.pad_h_t, p_param.pad_h_b, p_param.pad_w_l, p_param.pad_w_r,
                p_param.stride_h, p_param.stride_w,
                p_param.avg_pooling_const, p_param.r_shift_width);
            break;

      case POOLING_DEPTHWISE:
        tl_depthwise_fixed(
                tl_ifmap, tl_weight, tl_bias, tl_ofmap,
                p_param.kh, p_param.kw,
                p_param.ins_h, p_param.ins_h_last, p_param.ins_w, p_param.ins_w_last,
                p_param.pad_h_t, p_param.pad_h_b, p_param.pad_w_l, p_param.pad_w_r,
                p_param.stride_h, p_param.stride_w,
                p_param.r_shift_width);
            break;

    }

    tl_store_fixed(tl_ofmap, gaddr_output, CTRL_NULL);

    test_bmkernel_submit();

    // copy from device memory
    memcpy((uint8_t*)output_data, (uint8_t *)gaddr_output, (in * oc * oh * ow));
    int ofmap_size = in*oc*oh*ow;
    int cmp_res = array_cmp_int8(
        (const char*)"Comparing results of native_pooling and atomic_pooling ...\n",
        (int8_t*)output_data_ref, (int8_t*)output_data, ofmap_size);

    if(cmp_res != 0){
      printf("Comparison FAILED for native_pooling and atomic_pooling!!!\n");
      print_pooling_param(p_param);
      exit(-1);
    }
  }else {
    printf("tl_alloc failed and goto exit");
  }
  if (p_param.using_bias && tl_bias) {
    tl_free(tl_bias);
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

  test_gmem_free(ctx, gaddr_input);
  test_gmem_free(ctx, gaddr_weight);
  test_gmem_free(ctx, gaddr_output);
  if (p_param.using_bias) {
    test_gmem_free(ctx, gaddr_bias);
  }

  free(output_data);
  free(output_data_ref);
  free(input_data);
  free(weight_data);
  free(bias_data);
  free(bias_data_lo_hi);

  printf("Comparison done for native_pooling and atomic_pooling!\n");
  return tl_alloc_success ? 1 : 0;
}

static void init_pooling_param(pooling_param_t *p_param)
{
  srand((int)timer_get_tick());

  static int pooling_type = 0;
  p_param->pooling_type = pooling_type;
  pooling_type = (pooling_type + 1) % 3;

retry:
  if (p_param->pooling_type == POOLING_DEPTHWISE)
    p_param->using_bias = rand() % 2;
  else
    p_param->using_bias = 0;

  p_param->opd0_sign = rand() % 2;
  /* zhiqi wang: weight is usually signed for depthwise and unsigned for pooling average
     bias is 16bits, can be only considered signed to reduce complexity */
  p_param->opd1_sign = 0;
  if (p_param->pooling_type  == POOLING_AVG ) {
    p_param->opd1_sign = 0;
  }
  if (p_param->pooling_type  == POOLING_DEPTHWISE ) {
    p_param->opd1_sign = 1;
  }
  p_param->opd2_sign = 1;
  // Zhiqi Wang: depthwise, but pooling haven't relu interface now 3/26/2018
  p_param->do_relu = 0;

#ifdef BUILD_TEST_CASE_SLT_pooling
  p_param->input_n = rand() % 5 + 1;
  p_param->input_c = rand() % (8) + 1;
  p_param->input_h = rand() % 16 + 3;
  p_param->input_w = rand() % 16 + 3 + 3;
#else
  p_param->input_n = rand() % 5 + 1;
  p_param->input_c = rand() % (5 * NPU_NUM) + 1;
  p_param->input_h = rand() % 40 + 3;
  p_param->input_w = rand() % 40 + 3 + 3;
#endif

  //p_param->start_local_mem_idx = rand() % NPU_NUM;
  p_param->start_local_mem_idx = 0; // TODO(wwcai)

  p_param->kh = rand() % 7 + 1;
  p_param->kw = rand() % 7 + 1;
  if (p_param->pooling_type == POOLING_MAX) {
    p_param->ins_h = 0;
    p_param->ins_w = 0;
    p_param->ins_h_last = 0;
    p_param->ins_w_last = 0;
    p_param->r_shift_width = 0;
  } else {
    p_param->ins_h = rand() % p_param->kh;
    p_param->ins_w = rand() % p_param->kw;
    p_param->ins_h_last = rand() % p_param->kh;
    p_param->ins_w_last = rand() % p_param->kw;
    p_param->r_shift_width = rand() % 32;
  }
  p_param->stride_h = rand() % (p_param->kh) + 1;
  p_param->stride_w = rand() % (p_param->kw) + 1;

  p_param->pad_h_t = rand() % p_param->kh;
  p_param->pad_h_b = rand() % p_param->kh;
  p_param->pad_w_l = rand() % p_param->kw;
  p_param->pad_w_r = rand() % p_param->kw;

  int ih_ext = (p_param->input_h - 1) * (p_param->ins_h + 1) + p_param->ins_h_last
      + 1 + p_param->pad_h_t + p_param->pad_h_b;
  int iw_ext = (p_param->input_w - 1) * (p_param->ins_w + 1) + p_param->ins_w_last
      + 1 + p_param->pad_w_l + p_param->pad_w_r;

  int kh = p_param->kh;
  int kw = p_param->kw;

  p_param->avg_pooling_const = rand() % 256;

  // TODO(wwcai): check param scope
  // TODO(wwcai): if pooling_max, kh * kw != 1
  if((kh > ih_ext)
     || (kw > iw_ext)
     || (kh <= p_param->pad_h_t)
     || (kh <= p_param->pad_h_b)
     || (kw <= p_param->pad_w_l)
     || (kw <= p_param->pad_w_r)
     || (p_param->pad_h_t >= (1 << 4)) // 4bit
     || (p_param->pad_h_b >= (1 << 4)) // 4bit
     || (p_param->pad_w_l >= (1 << 4)) // 4bit
     || (p_param->pad_w_r >= (1 << 4)) // 4bit
     || ((p_param->pooling_type == 0) && (p_param->kh * p_param->kw == 1))
     )
  {
    printf("retry init_pooling_param\n");
    goto retry;
  }
}

#ifdef BUILD_TEST_CASE_SLT_pooling
#define TEST_ITERATION_NUM 4
#else
#define TEST_ITERATION_NUM 8
#endif
int testcae_pooling_main(bmctx_t *ctx)
{
  int test_finished_num = 0;
  for (int i = 0; i < TEST_ITERATION_NUM; i++) {
    printf("random_test_pooling iteration: %d\n", i);
    pooling_param_t test_pooling_param;
    init_pooling_param(&test_pooling_param);
    test_finished_num += test_pooling(test_pooling_param, ctx);
  }
  printf("Test finished %d/%d .\n", test_finished_num, TEST_ITERATION_NUM);
  return 0;
}

int testcase_pooling()
{
  bmctx_t ctx;
  test_init(&ctx);
  testcae_pooling_main(&ctx);
  test_exit(&ctx);
  return 0;
}
//module_testcase("1", testcase_pooling);

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  return testcase_pooling();
}
#endif
