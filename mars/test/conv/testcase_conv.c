/*
 * libbmruntime/test/test_bmkernel_1880.c
 *
 * Copyright Bitmain Technologies Inc.
 * Written by:
 *   Wanwei CAI <wanwei.cai@bitmain.com>
 * Created Time: 2018-02-09 18:01
 */

#include "1880_test_util.h"
#include "timer.h"

#define nullptr NULL

typedef struct {
  int random_seed;
  int input_n;
  int input_c;
  int input_h;
  int input_w;
  int kw;
  int kh;
  int dh;
  int dw;
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
  int using_bias;
  int bReLU_EN;
  int r_shift_m;
  int input_start_npu_idx;
  int output_start_npu_idx;
  int opd0_sign;
  int opd1_sign;
  int opd2_sign;
} conv_param_t;

static char *kernel_dilation(int kh,int kw,int dh,int dw,char *kernel_before)
{
  int kh_ext = (kh-1)*dh + 1;
  int kw_ext = (kw-1)*dw + 1;
  char* kernel_after = (char *)xmalloc(kh_ext * kw_ext * sizeof(char));

  for(int h = 0; h < kh_ext; h++)
    for(int w = 0; w < kw_ext; w++){
      kernel_after[h*kw_ext+w] = 0;
    }

  for(int h = 0; h < kh; h++)
    for(int w = 0; w < kw; w++){
      kernel_after[h*dh*kw_ext + w*dw] = kernel_before[h*kw + w];
    }
  return kernel_after;
}

static int matrix_dot_mult(
    int dim_n, int dim_m, char *A, char *B,
    int opd0_sign, int opd1_sign)
{
  int sum = 0;
  for (int i=0; i<dim_n; i++){
    for (int j=0; j<dim_m; j++){
      if(opd0_sign) {
        sum += A[i*dim_m+j] * B [i*dim_m+j];
      } else {
        sum += (int)((uint8_t)A[i*dim_m+j]) * B [i*dim_m+j];
      }
    }
  }
  return sum;
}

static int8_t *insert_zero(
    int8_t *p_before_insert,
    int padw_left,
    int padw_right,
    int padh_top,
    int padh_bottom,
    int ins0_h,
    int ins0_w,
    int ins0_last_h,
    int ins0_last_w,
    int height_before,
    int width_before)
{
  int width_after_insert =
      (width_before - 1) * (ins0_w + 1) + ins0_last_w + 1 + padw_left + padw_right;
  int height_after_insert =
      (height_before - 1) * (ins0_h + 1) + ins0_last_h + 1 + padh_top + padh_bottom;

  int8_t *p_after_insert = (int8_t *) xmalloc(width_after_insert * height_after_insert * sizeof(int8_t));
  memset(p_after_insert, 0,
         width_after_insert * height_after_insert * sizeof(int8_t));

  for (int h = 0; h < height_before; h++) {
    for (int w = 0; w < width_before; w++) {
      p_after_insert[(h * (ins0_h + 1) + padh_top) * width_after_insert +
                     w * (ins0_w + 1) + padw_left] =
          p_before_insert[h * width_before + w];
    }
  }
  return p_after_insert;
}

static void native_conv_1880(
    const conv_param_t p_param, char *ifmap, char *weight,
    short *bias, char *ofmap)
{
  int in = p_param.input_n;
  int ic = p_param.input_c;
  int ih = p_param.input_h;
  int iw = p_param.input_w;
  int oc = p_param.output_c;

  int kh = p_param.kh;
  int kw = p_param.kw;
  int dh = p_param.dh;
  int dw = p_param.dw;

  int ins_h = p_param.ins_h;
  int ins_h_last = p_param.ins_h_last;
  int ins_w = p_param.ins_w;
  int ins_w_last = p_param.ins_w_last;
  int opd0_sign = p_param.opd0_sign;
  int opd1_sign = p_param.opd1_sign;
  int opd2_sign = p_param.opd2_sign;
  assert(opd1_sign == 1);
  assert(opd2_sign == 1);
  int ih_ext = (ih - 1) * (ins_h + 1) + ins_h_last + 1 + p_param.pad_h_t + p_param.pad_h_b ;
  int iw_ext = (iw - 1) * (ins_w + 1) + ins_w_last + 1 + p_param.pad_w_l + p_param.pad_w_r ;
  int kh_ext = (kh - 1) * dh + 1;
  int kw_ext = (kw - 1) * dw + 1;

  int oh = (ih_ext - kh_ext) / p_param.stride_h + 1;
  int ow = (iw_ext - kw_ext) / p_param.stride_w + 1;

  int *result = (int *) xmalloc(in * oc * oh * ow * sizeof(int));
  // Note(wwcai): initialzation is important, or the value is random value
  for (int i = 0; i < in * oc * oh * ow; i++) {
    result[i] = 0;
  }

  int total_out_range_cnt = 0;
  int total_zero_cnt = 0;
  for (int n = 0; n < in; ++n) {
    for (int c = 0; c < oc; ++c) {
      for (int cc = 0; cc < ic; ++cc) {
        char *i_fmap_pad_ker = (char *) xmalloc(kh_ext * kw_ext * sizeof(char));
        // the image tensor after padding and zero insertion.
        int8_t *i_fmap_pad = insert_zero(
            (int8_t*)ifmap + n*ic*ih*iw + cc*ih*iw,
            p_param.pad_w_l,
            p_param.pad_w_r,
            p_param.pad_h_t,
            p_param.pad_h_b,
            ins_h, ins_w, ins_h_last, ins_w_last, ih, iw);
        char *kernel_after = kernel_dilation(
            p_param.kh, p_param.kw, p_param.dh, p_param.dw,
            (weight + c*ic*kh*kw + cc*kh*kw));

        for (int ph = 0; ph < oh; ++ph) {
          for (int pw = 0; pw < ow; ++pw) {
            for (int idxh = 0; idxh < kh_ext; ++idxh)
              for (int idxw = 0; idxw < kw_ext; ++idxw){
                i_fmap_pad_ker[idxh * kw_ext + idxw] =
                    i_fmap_pad[(idxh+ph*p_param.stride_h) * iw_ext +
                               idxw + pw*p_param.stride_w];
              }
            result[n*oc*oh*ow + c*oh*ow + ph*ow + pw] +=
                matrix_dot_mult(kh_ext, kw_ext, i_fmap_pad_ker,
                                kernel_after, opd0_sign, opd1_sign);
          }
        }
        free(i_fmap_pad_ker);
        free(i_fmap_pad);
        free(kernel_after);
      }

      if(p_param.using_bias){
        for (int ph = 0; ph < oh; ++ph) {
          for (int pw = 0; pw < ow; ++pw) {
            result[n*oc*oh*ow + c*oh*ow + ph*ow + pw] += bias[c]; //bias+c ;
          }
        }
      }

      for (int ph = 0; ph < oh; ++ph) {
        for (int pw = 0; pw < ow; ++pw) {
          int temp = result[n*oc*oh*ow + c*oh*ow + ph*ow + pw];
          if (0==p_param.bReLU_EN) {
            int satu_max = 127;
            int satu_min = -128;
            if(p_param.r_shift_m > 0){
              temp = ((temp>>(p_param.r_shift_m-1))+1)>>1;
              if (temp == 0)  total_zero_cnt++;
              if (temp > satu_max || temp < satu_min)  total_out_range_cnt++;
            }
            temp = (temp>satu_max) ? satu_max : ((temp<satu_min) ? satu_min : temp);
          } else {
            int satu_max = 255;
            temp = (temp>0) ? temp : 0;
            if(p_param.r_shift_m > 0){
              temp = ((temp>>(p_param.r_shift_m-1))+1)>>1;
            }
            temp = (temp>satu_max) ? satu_max : temp;
          }
          ofmap[n*oc*oh*ow + c*oh*ow + ph*ow + pw] = temp;
        }
      }
    } //end for (int c = 0; c < oc; ++c)
  } //end for (int n = 0; n < in; n++)

  printf("Total Saturation rate: %f(%d/%d), %d zeros found.\n",
         (float)total_out_range_cnt/(float)(in*oc*oh*ow),
         total_out_range_cnt, in*oc*oh*ow, total_zero_cnt);
  free(result);
}

static int array_cmp_atomic_int8(
    char *p_exp, char *p_got, int len,
    const char *info_label)
{
  for (int idx = 0; idx < len; idx++) {
    if (p_exp[idx] != p_got[idx]){
      printf("%s error at index %d exp %d got %d\n",
             info_label, idx, p_exp[idx], p_got[idx]);
      return -1;
    }
  }
  return 0;
}

static void print_conv_param(const conv_param_t p_param)
{
  printf("%s\n", "Conv parameters:");
  printf("  %s%d;\n", "p_param.random_seed = ", p_param.random_seed);

  printf("  %s%d;\n", "p_param.input_n = ", p_param.input_n);
  printf("  %s%d;\n", "p_param.input_c = ", p_param.input_c);
  printf("  %s%d;\n", "p_param.input_h = ", p_param.input_h);
  printf("  %s%d;\n", "p_param.input_w = ", p_param.input_w);
  printf("  %s%d;\n", "p_param.output_c = ", p_param.output_c);

  printf("  %s%d;\n", "p_param.kh = ", p_param.kh);
  printf("  %s%d;\n", "p_param.kw = ", p_param.kw);
  printf("  %s%d;\n", "p_param.dh = ", p_param.dh);
  printf("  %s%d;\n", "p_param.dw = ", p_param.dw);
  printf("  %s%d;\n", "p_param.pad_h_t = ", p_param.pad_h_t);
  printf("  %s%d;\n", "p_param.pad_h_b = ", p_param.pad_h_b);
  printf("  %s%d;\n", "p_param.pad_w_l = ", p_param.pad_w_l);
  printf("  %s%d;\n", "p_param.pad_w_r = ", p_param.pad_w_r);
  printf("  %s%d;\n", "p_param.stride_h = ", p_param.stride_h);
  printf("  %s%d;\n", "p_param.stride_w = ", p_param.stride_w);
  printf("  %s%d;\n", "p_param.ins_w = ", p_param.ins_w);
  printf("  %s%d;\n", "p_param.ins_h = ", p_param.ins_h);
  printf("  %s%d;\n", "p_param.ins_w_last = ", p_param.ins_w_last);
  printf("  %s%d;\n", "p_param.ins_h_last = ", p_param.ins_h_last);

  printf("  %s%d;\n", "p_param.r_shift_m = ", p_param.r_shift_m);
  printf("  %s%d;\n", "p_param.opd0_sign = ", p_param.opd0_sign);
  printf("  %s%d;\n", "p_param.opd1_sign = ", p_param.opd1_sign);
  printf("  %s%d;\n", "p_param.opd2_sign = ", p_param.opd2_sign);
  printf("  %s%d;\n", "p_param.bReLU_EN = ", p_param.bReLU_EN);
  printf("  %s%d;\n", "p_param.using_bias = ", p_param.using_bias);

  printf("  %s%d;\n", "p_param.input_start_npu_idx = ", p_param.input_start_npu_idx);
  printf("  %s%d;\n", "p_param.output_start_npu_idx = ", p_param.output_start_npu_idx);

  int ih = p_param.input_h;
  int iw = p_param.input_w;
  int kh = p_param.kh;
  int kw = p_param.kw;
  int dh = p_param.dh;
  int dw = p_param.dw;
  int ins_h = p_param.ins_h;
  int ins_h_last = p_param.ins_h_last;
  int ins_w = p_param.ins_w;
  int ins_w_last = p_param.ins_w_last;
  int kh_ext = (kh - 1) * dh + 1;
  int kw_ext = (kw - 1) * dw + 1;
  int ih_ext = (ih - 1) * (ins_h + 1) + ins_h_last + 1 + p_param.pad_h_t + p_param.pad_h_b ;
  int iw_ext = (iw - 1) * (ins_w + 1) + ins_w_last + 1 + p_param.pad_w_l + p_param.pad_w_r ;
  int oh = (ih_ext - kh_ext) / p_param.stride_h + 1;
  int ow = (iw_ext - kw_ext) / p_param.stride_w + 1;

  printf("  %s%d\n", "kh_ext = ",kh_ext);
  printf("  %s%d\n", "kw_ext = ",kw_ext);
  printf("  %s%d\n", "ih_ext = ",ih_ext);
  printf("  %s%d\n", "iw_ext = ",iw_ext);
  printf("  %s%d\n", "output_h = ",oh);
  printf("  %s%d\n", "output_w = ",ow);
}

// Calculate the right shift value, m
// Steps:
//  1. Get the abs() of each weight;
//  2. Summary all the abs() in one kernel;
//  3. Get Log2 of each sum;
//  4. Downward rounding;
// After every r_shift value got, sort and find the middle one.
static int calc_rshift_m(char* weight, int kernel_cnt, int kernel_size)
{
  int sum = 0;
  int mid = 0;
  int rshift_m = 0;
  int tag[32] = {0};
  int *kernel_shifts = (int *) xmalloc(kernel_cnt * sizeof(int));

  for (int idx = 0; idx < kernel_cnt; idx++) {
    kernel_shifts[idx] = 0;
  }

  // Part 1:
  // Get right shift value for each kernel
  for (int i = 0; i < kernel_cnt; i++) {
    // Step 1 & 2: Get the sum of abs()
    for (int j = 0; j < kernel_size; j++) {
      sum += (int)(*weight < 0 ? -(*weight) : (*weight));
      weight++;
    }
    // Step 3 & 4: log2 and downward rounding
    sum >>= 1;
    while (sum) {
      sum >>= 1;
      kernel_shifts[i]++;
    }
  }

  // Part 2:
  // Find the middle of all the values
  for (int cnt = 0; cnt < kernel_cnt; cnt++) {
    tag[kernel_shifts[cnt]]++;
  }

  do {
    mid += tag[rshift_m++];
  } while(mid < (kernel_cnt - 1) >> 1);

  free(kernel_shifts);

  return rshift_m - 1;
}

static int test_conv(conv_param_t p_param, bmctx_t *ctx)
{
  print_conv_param(p_param);
  int in = p_param.input_n;
  int ic = p_param.input_c;
  int ih = p_param.input_h;
  int iw = p_param.input_w;
  int oc = p_param.output_c;
  int kh = p_param.kh;
  int kw = p_param.kw;
  int dh = p_param.dh;
  int dw = p_param.dw;
  int ins_h = p_param.ins_h;
  int ins_h_last = p_param.ins_h_last;
  int ins_w = p_param.ins_w;
  int ins_w_last = p_param.ins_w_last;

  int kh_ext = (kh - 1) * dh + 1;
  int kw_ext = (kw - 1) * dw + 1;
  int ih_ext = (ih - 1) * (ins_h + 1) + ins_h_last + 1 + p_param.pad_h_t + p_param.pad_h_b ;
  int iw_ext = (iw - 1) * (ins_w + 1) + ins_w_last + 1 + p_param.pad_w_l + p_param.pad_w_r ;
  int oh = (ih_ext - kh_ext) / p_param.stride_h + 1;
  int ow = (iw_ext - kw_ext) / p_param.stride_w + 1;

  assert(dh > 0 && dw > 0);
  assert(ih_ext >= kh_ext);
  assert(iw_ext >= kw_ext);
  assert(kh_ext > p_param.pad_h_t);
  assert(kh_ext > p_param.pad_h_b);
  assert(kw_ext > p_param.pad_w_l);
  assert(kw_ext > p_param.pad_w_r);

  char *input_data = (char *) xmalloc(in * ic * ih * iw * sizeof(char));
  char *weight_data = (char *) xmalloc(oc * ic * kh * kw * sizeof(char));
  char *output_data = (char *) xmalloc(in * oc * oh * ow * sizeof(char));
  char *output_data_ref = (char *) xmalloc(in * oc * oh * ow * sizeof(char));
  short *bias_data = (short *) xmalloc(oc * sizeof(short));
  uint8_t *bias_data_lo_hi = (uint8_t *)xmalloc(2 * oc * sizeof(uint8_t));

  ctrl_t conv_ctrls = CTRL_NULL;
  if (p_param.bReLU_EN) {
    conv_ctrls |= CTRL_RELU;
  }

  for (int idx = 0; idx < in * ic * ih * iw; idx++) {
    input_data[idx] = rand() % 256 - 128;
  }

  for (int idx = 0;idx < oc * ic * kh * kw;idx++){
    weight_data[idx] = rand() % 256 - 128;
  }

  // Calculate the right shift value per weight's values
  p_param.r_shift_m = calc_rshift_m(weight_data, oc * ic, kh * kw);

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

  native_conv_1880(p_param, input_data, weight_data, bias_data, output_data_ref);

  // alloc memory
  gaddr_t gaddr_input = test_gmem_alloc(ctx, (in * ic * ih * iw));
  gaddr_t gaddr_weight = test_gmem_alloc(ctx, (oc * ic * kh * kw));
  gaddr_t gaddr_bias = 0;
  if (p_param.using_bias) {
    gaddr_bias = test_gmem_alloc(ctx, (2 * oc * 1 * 1));
    memcpy((uint8_t *)gaddr_bias, (uint8_t*)bias_data_lo_hi, (2 * oc * 1 * 1));
  }
  gaddr_t gaddr_output = test_gmem_alloc(ctx, (in * oc * oh * ow));

  // copy to device memory
  memcpy((uint8_t *)gaddr_input, (uint8_t*)input_data, (in * ic * ih * iw));

  char *weight_data_1880 = (char *) xmalloc(oc* kh * kw * ic);
  //oc,ic,kh,kw->1,oc,kh*kw,ic
  for (int oc_idx = 0;oc_idx < oc; oc_idx++){
    for (int ic_idx = 0;ic_idx < ic; ic_idx++){
      for (int kh_idx = 0;kh_idx < kh; kh_idx++){
        for (int kw_idx = 0;kw_idx < kw; kw_idx++){
          weight_data_1880[oc_idx * kh * kw * ic + kh_idx * kw * ic + kw_idx * ic + ic_idx]
              = weight_data[oc_idx* ic * kh * kw + ic_idx * kh * kw + kh_idx * kw + kw_idx];
        }
      }
    }
  }

  memcpy((uint8_t *)gaddr_weight, (uint8_t*)weight_data_1880, (oc * ic * kh * kw));

  fmt_t if_fmt_t = p_param.opd0_sign? FMT_I8: FMT_U8;
  fmt_t weight_fmt_t = p_param.opd1_sign? FMT_I8: FMT_U8;

  tensor_lmem *tl_ifmap = tl_alloc(shape_t4(in, ic, ih, iw), if_fmt_t, CTRL_AL);
  tensor_lmem *tl_weight = tl_alloc(shape_t4(ic, oc, kh, kw), weight_fmt_t, CTRL_NULL);
  tensor_lmem *tl_ofmap = tl_alloc(shape_t4(in, oc, oh, ow), FMT_I8, CTRL_AL);
  bool tl_alloc_success = (tl_ifmap != nullptr) &&
      (tl_weight != nullptr) && (tl_ofmap != nullptr);

  tensor_lmem * tl_weight_gdma = tl_prealloc(
      tl_address(tl_weight),
      shape_t4(1, oc, kh * kw, ic),
      weight_fmt_t);

  tensor_lmem *tl_bias = nullptr;
  if (p_param.using_bias) {
    fmt_t bias_fmt_t;
    if (p_param.opd2_sign) {
      bias_fmt_t = FMT_I8;
    }
    else {
      bias_fmt_t = FMT_U8;
    }
    tl_bias = tl_alloc(shape_t4(2, oc, 1, 1), bias_fmt_t, CTRL_NULL);
    if ((tl_bias == nullptr)) {
      tl_alloc_success = false;
    }
  }

  if (tl_alloc_success) {
    tl_load_fixed(tl_ifmap, gaddr_input, CTRL_NULL);
    tl_load_fixed(tl_weight_gdma, gaddr_weight, CTRL_NULL);
    if (p_param.using_bias) {
      tl_load_fixed(tl_bias, gaddr_bias, CTRL_NULL);
    }

    if (p_param.using_bias) {
      tl_bias->shape = shape_t4(1, oc, 1, 1);
      if (tl_bias->fmt == FMT_I8)
        tl_bias->fmt = FMT_I16;
      else
        tl_bias->fmt = FMT_U16;
    }
    tl_conv_fixed(
        tl_ifmap, tl_ofmap, tl_weight, tl_bias,
        p_param.ins_h,
        p_param.ins_h_last,
        p_param.ins_w,
        p_param.ins_w_last,
        p_param.pad_h_t,
        p_param.pad_h_b,
        p_param.pad_w_l,
        p_param.pad_w_r,
        p_param.stride_h,
        p_param.stride_w,
        p_param.dh,
        p_param.dw,
        p_param.r_shift_m,
        conv_ctrls);

    tl_store_fixed(tl_ofmap, gaddr_output, CTRL_NULL);

    test_bmkernel_submit();

    // copy from device memory
    memcpy((uint8_t*)output_data, (uint8_t *)gaddr_output, (in * oc * oh * ow));

    int ofmap_size = in*oc*oh*ow;
    int cmp_res = array_cmp_atomic_int8(
        output_data_ref, output_data, ofmap_size,
        "Comparing int8 results of native_conv and atomic_conv_neuron ...\n");

    if(cmp_res != 0){
      printf("Comparison FAILED for native_conv and atomic_conv_neuron!!!\n");
      //print_conv_param(p_param);
      exit(-1);
    }
  } else {
    printf("tl_alloc failed and goto exit\n");
  }

  if (p_param.using_bias && tl_bias) {
    tl_free(tl_bias);
  }
  if (tl_weight_gdma) {
    tl_free(tl_weight_gdma);
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

  // free device memory
  test_gmem_free(ctx, gaddr_input);
  test_gmem_free(ctx, gaddr_output);
  test_gmem_free(ctx, gaddr_weight);
  if (p_param.using_bias) {
    test_gmem_free(ctx, gaddr_bias);
  }

  free(input_data);
  free(weight_data);
  free(weight_data_1880);
  free(bias_data);
  free(bias_data_lo_hi);
  free(output_data);
  free(output_data_ref);

  printf("Comparison PASS!! for native_conv and atomic_conv_neuron!\n");
  return tl_alloc_success ? 1 : 0;
}

static void init_conv_param(conv_param_t *p_param)
{
  printf("init_conv_param\n");
  p_param->random_seed = (int)timer_get_tick();
  srand(p_param->random_seed);

retry:
#ifdef BUILD_TEST_CASE_SLT_conv
  p_param->input_n = rand() % 2 + 1;
  p_param->input_c = rand() % 16 + 1;
  p_param->kh = rand() % 5 + 1;
  p_param->kw = rand() % 5 + 1;
  p_param->input_h = rand() % 16 + p_param->kh;
  p_param->input_w = rand() % 16 + p_param->kw;
#else
  p_param->input_n = rand() % 5 + 1;
  p_param->input_c = rand() % 48 + 1;
  p_param->kh = rand() % 7 + 1;
  p_param->kw = rand() % 7 + 1;
  p_param->input_h = rand() % 40 + p_param->kh;
  p_param->input_w = rand() % 40 + p_param->kw;
#endif
  p_param->output_c = rand() % 10 + 3;
  p_param->stride_h = rand() % (p_param->kh) + 1;
  p_param->stride_w = rand() % (p_param->kw) + 1;
  p_param->ins_h = rand() % p_param->kh;
  p_param->ins_w = rand() % p_param->kw;
  p_param->ins_h_last = rand() % p_param->kh;;
  p_param->ins_w_last = rand() % p_param->kw;;
  p_param->dh = rand() % 3 + 1;
  p_param->dw = rand() % 3 + 1;

  int kh_ext = (p_param->kh - 1) * p_param->dh + 1;
  int kw_ext = (p_param->kw - 1) * p_param->dw + 1;
  p_param->pad_h_t = rand() % kh_ext;
  p_param->pad_h_b = rand() % kh_ext;
  p_param->pad_w_l = rand() % kw_ext;
  p_param->pad_w_r = rand() % kw_ext;

  int ih_ext = (p_param->input_h - 1) * (p_param->ins_h + 1) +
      p_param->ins_h_last + 1 + p_param->pad_h_t + p_param->pad_h_b;
  int iw_ext = (p_param->input_w - 1) * (p_param->ins_w + 1) +
      p_param->ins_w_last + 1 + p_param->pad_w_l + p_param->pad_w_r;

  if ((kh_ext > ih_ext)
      || (kw_ext > iw_ext)
      || (kh_ext <= p_param->pad_h_t)
      || (kh_ext <= p_param->pad_h_b)
      || (kw_ext <= p_param->pad_w_l)
      || (kw_ext <= p_param->pad_w_r)
      || (p_param->pad_h_t >= (1 << 4)) // 4bit
      || (p_param->pad_h_b >= (1 << 4)) // 4bit
      || (p_param->pad_w_l >= (1 << 4)) // 4bit
      || (p_param->pad_w_r >= (1 << 4)) // 4bit
      )
  {
    printf("retry init_conv_param\n");
    goto retry;
  }

  p_param->using_bias = rand() % 2;
  p_param->r_shift_m = rand() % 8;
  p_param->bReLU_EN = rand() % 2;

  p_param->opd0_sign = rand() % 2;
  p_param->opd1_sign = 1;
  p_param->opd2_sign = 1;

  p_param->input_start_npu_idx = 0;
  p_param->output_start_npu_idx = 0;
}

int testcase_conv_main(bmctx_t *ctx)
{
#ifdef BUILD_TEST_CASE_SLT_conv
  const int TEST_ITERATION_NUM = 2;
#else
  const int TEST_ITERATION_NUM = 4;
#endif

  int test_finished_num = 0;
  for (int i = 0; i < TEST_ITERATION_NUM; i++) {
    printf("random_test_conv iteration: %d\n", i);
    conv_param_t test_conv_param;
    init_conv_param(&test_conv_param);
    test_finished_num += test_conv(test_conv_param, ctx);
  }
  printf("Test finished %d/%d\n", test_finished_num, TEST_ITERATION_NUM);
  return 0;
}

int testcase_conv()
{
  bmctx_t ctx;
  test_init(&ctx);
  testcase_conv_main(&ctx);
  test_exit(&ctx);
  return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  bmctx_t ctx;
  test_init(&ctx);
  testcase_conv_main(&ctx);
  test_exit(&ctx);
  return 0;
}
#endif

