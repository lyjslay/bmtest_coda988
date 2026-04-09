/*
 * libbmruntime/test/test_1880_fc.cpp
 *
 * Copyright Bitmain Technologies Inc.
 * Written by:
 *   Wanwei CAI <wanwei.cai@bitmain.com>
 * Created Time: 2018-02-23 14:25
 */

#include "1880_test_util.h"
#include "timer.h"

#define nullptr NULL

typedef struct fc_param
{
  int L_row_num;
  int L_col_num;
  int R_col_num;
  bool with_bias;
  bool add_result;
  int start_local_mem_idx_L;
  int start_local_mem_idx_R;
  bool opd0_sign;
  bool opd1_sign;
  bool opd2_sign;
  bool res0_sign;
  bool do_relu;
  int r_shift_width;
  int l_shift_width;
  bool is_result_int8;
} fc_param_t;

static void satu_2_8bit(
    int* pBuff, int len, char* pByteOut,
    int rshiftbits, int round_floor, int sign_unsign)
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
  } else {  // rshiftbits>0
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

static void satu_2_16bit(
    int* pBuff, int len, short* pByteOut,
    int rshiftbits, int round_floor, int sign_unsign)
{
  ASSERT(rshiftbits>=0);
  int ii;
  int temp;
  int satu_max = (sign_unsign==0) ? 32767 : 65535;
  int satu_min = (sign_unsign==0) ? -32768 : 0;
  if(rshiftbits==0)  {
    for(ii=0; ii<len; ii++)  {
      temp = (pBuff[ii]>satu_max) ? satu_max : ((pBuff[ii]<satu_min) ? satu_min : pBuff[ii]);
      memcpy(pByteOut+ii, &temp, 2);
    }
  } else {  // rshiftbits>0
    for(ii=0; ii<len; ii++)  {
      if(round_floor==1)
        temp = ((pBuff[ii]>>(rshiftbits-1))+1)>>1;
      else
        temp = pBuff[ii]>>rshiftbits;
      temp = (temp>satu_max) ? satu_max : ((temp<satu_min) ? satu_min : temp);
      memcpy(pByteOut+ii, &temp, 2);
    }
  }
  return;
}

static void fill_int_with_char(int* pdest, char* psrc, int len)
{
  for(int ii=0; ii<len; ii++)
    pdest[ii] = (int)psrc[ii];
}

static void fill_int_with_short(int* pdest, short* psrc, int len)
{
  for(int ii=0; ii<len; ii++)
    pdest[ii] = (int)psrc[ii];
}

static void native_fc(
    const fc_param_t p_param, char *L, char *R, short *B, short *Y, int *Y_ref)
{
  unsigned char* uL = (unsigned char*)L;
  unsigned char* uR = (unsigned char*)R;
  unsigned short* uB = (unsigned short*)B;
  int opd0, opd1, opd2;

  for (int hidx = 0; hidx < p_param.L_row_num; hidx++) {
    for (int widx = 0; widx < p_param.R_col_num; widx++) {
      int Y1 = 0;
      int Y2 = 0;
      int sum_idx = 0;
      for (sum_idx = 0; sum_idx < p_param.L_col_num; sum_idx++) {
        opd0 = (p_param.opd0_sign) ? (int)L[hidx * p_param.L_col_num + sum_idx] :
            (int)uL[hidx * p_param.L_col_num + sum_idx];
        opd1 = (p_param.opd1_sign) ? (int)R[sum_idx * p_param.R_col_num + widx] :
            (int)uR[sum_idx * p_param.R_col_num + widx];
        if((sum_idx % 2 == 0 && sum_idx != 2) || sum_idx == 1){
          Y1 += opd0 * opd1;
        } else {
          Y2 += opd0 * opd1;
        }
      }
      sum_idx++;

      if (p_param.with_bias){
        opd2 = (p_param.opd2_sign) ? (int)B[widx] : (int)uB[widx];
        if((sum_idx % 2 == 0 && sum_idx != 2) || sum_idx == 1){
          Y1 += opd2;
        } else {
          Y2 += opd2;
        }
        sum_idx++;
      }

      if (p_param.add_result){
        if((sum_idx % 2 == 0 && sum_idx != 2) || sum_idx == 1){
          Y1 += (Y[hidx * p_param.R_col_num + widx] << p_param.l_shift_width);
        } else {
          Y2 += (Y[hidx * p_param.R_col_num + widx] << p_param.l_shift_width);
        }
      }

      Y_ref[hidx * p_param.R_col_num + widx] = Y1 + Y2;
    }
  }
  char* Yout_int8 = xmalloc(p_param.L_row_num * p_param.R_col_num);
  short* Yout_int16 = xmalloc(p_param.L_row_num * p_param.R_col_num * sizeof(short));

  if(p_param.is_result_int8)  {
    satu_2_8bit(Y_ref, p_param.L_row_num * p_param.R_col_num, Yout_int8, p_param.r_shift_width, 1, p_param.do_relu);
    fill_int_with_char(Y_ref, Yout_int8, p_param.L_row_num * p_param.R_col_num);
  }
  else  {
    satu_2_16bit(Y_ref, p_param.L_row_num * p_param.R_col_num, Yout_int16, p_param.r_shift_width, 1, p_param.do_relu);
    fill_int_with_short(Y_ref, Yout_int16, p_param.L_row_num * p_param.R_col_num);
  }

  free(Yout_int8);
  free(Yout_int16);
  Yout_int8 = NULL;
  Yout_int16 = NULL;
}

void print_fc_param(const fc_param_t p_param)
{
  printf("  %s\n", "FC parameters:");
  printf("  %s%d;\n", "p_param.L_row_num = ", p_param.L_row_num);
  printf("  %s%d;\n", "p_param.L_col_num = ", p_param.L_col_num);
  printf("  %s%d;\n", "p_param.R_col_num = ", p_param.R_col_num);
  printf("  %s%d;\n", "p_param.with_bias = ", p_param.with_bias);
  printf("  %s%d;\n", "p_param.do_relu = ", p_param.do_relu);
  printf("  %s%d;\n", "p_param.l_shift_width = ", p_param.l_shift_width);
  printf("  %s%d;\n", "p_param.r_shift_width = ", p_param.r_shift_width);
  printf("  %s%d;\n", "p_param.opd0_sign = ", p_param.opd0_sign);
  printf("  %s%d;\n", "p_param.opd1_sign = ", p_param.opd1_sign);
  printf("  %s%d;\n", "p_param.opd2_sign = ", p_param.opd2_sign);
  printf("  %s%d;\n", "p_param.is_result_int8 = ", p_param.is_result_int8);
  printf("  %s%d;\n", "p_param.add_result = ", p_param.add_result);
  printf("  %s%d;\n", "p_param.start_local_mem_idx_L = ", p_param.start_local_mem_idx_L);
  printf("  %s%d;\n", "p_param.start_local_mem_idx_R = ", p_param.start_local_mem_idx_R);
}

int array_cmp_int(const char * const info, int *p_exp, int *p_got, int count)
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

int test_fc(const fc_param_t p_param, bmctx_t *ctx)
{
  char* L = (char*)malloc(p_param.L_row_num * p_param.L_col_num * sizeof(char));
  char* R = (char*)malloc(p_param.L_col_num * p_param.R_col_num * sizeof(char));
  short* B = (short*)malloc(p_param.R_col_num * sizeof(short));
  short* Y = (short*)malloc(p_param.L_row_num * p_param.R_col_num * sizeof(short));
  char* int8_Y = (char *)Y;
  uint8_t *B_lo_hi = (uint8_t*)malloc(2*p_param.R_col_num * sizeof(char));
  uint8_t *Y_lo_hi = (uint8_t*)malloc(2*p_param.L_row_num * p_param.R_col_num * sizeof(char));

  for (int idx = 0; idx < p_param.L_row_num * p_param.L_col_num; idx++){
    L[idx] = (idx % 256) - 128;
  }

  for (int idx = 0; idx < p_param.L_col_num * p_param.R_col_num; idx++){
    R[idx] = (idx % 256) - 128;
  }
  for (int idx = 0; idx < p_param.R_col_num; idx++){
    B[idx] = (idx % 65536) - 32768;
    B_lo_hi[idx] = B[idx] & 0xff;
    B_lo_hi[idx + p_param.R_col_num] = (B[idx] >> 8) & 0xff;
  }
  for (int idx = 0; idx < p_param.L_row_num * p_param.R_col_num; idx++){
    Y[idx] = (idx % 65536) - 32768;
    Y_lo_hi[idx] = Y[idx] & 0xff;
    Y_lo_hi[idx + p_param.L_row_num * p_param.R_col_num] = (Y[idx] >> 8) & 0xff;
  }

  int* Y_ref = (int*)malloc(p_param.L_row_num * p_param.R_col_num * sizeof(int));
  native_fc(p_param, L, R, B, Y, Y_ref);

  gaddr_t gaddr_L = test_gmem_alloc(ctx, (p_param.L_row_num * p_param.L_col_num));
  gaddr_t gaddr_R = test_gmem_alloc(ctx, (p_param.L_col_num * p_param.R_col_num));
  gaddr_t gaddr_Y = test_gmem_alloc(ctx, (2 * p_param.L_row_num * p_param.R_col_num));
  gaddr_t gaddr_B = 0;

  // copy to device memory
  memcpy((uint8_t *)gaddr_L, (uint8_t*)L, (p_param.L_row_num * p_param.L_col_num));
  memcpy((uint8_t *)gaddr_R, (uint8_t*)R, (p_param.L_col_num * p_param.R_col_num));
  memcpy((uint8_t *)gaddr_Y, (uint8_t*)Y_lo_hi, (2 * p_param.L_row_num * p_param.R_col_num));

  printf("alloc L: %d %d\n", p_param.L_row_num, p_param.L_col_num);
  printf("alloc R: %d %d\n", p_param.L_col_num, p_param.R_col_num);
  printf("alloc Y: %d %d\n", p_param.L_row_num, p_param.R_col_num);
  printf("alloc B: %d %d\n", 1, p_param.R_col_num);

  if (p_param.with_bias) {
    gaddr_B = test_gmem_alloc(ctx, (2 * p_param.R_col_num));
    memcpy((uint8_t *)gaddr_B, (uint8_t*)B_lo_hi, (2 * p_param.R_col_num));
  }

  ctrl_t ctrls = CTRL_NULL;

  shape_t shape_L = shape_t2(p_param.L_row_num, p_param.L_col_num);
  shape_t shape_R = shape_t2(p_param.L_col_num, p_param.R_col_num);
  shape_t shape_Y = shape_t2(2 * p_param.L_row_num, p_param.R_col_num);
  tensor_lmem *tl_L = tl_alloc(shape_L, p_param.opd0_sign ? FMT_I8 : FMT_U8, CTRL_AL);
  tensor_lmem *tl_R = tl_alloc(shape_R, p_param.opd1_sign ? FMT_I8 : FMT_U8, CTRL_AL);
  tensor_lmem *tl_Y = tl_alloc(shape_Y, p_param.res0_sign ? FMT_I8 : FMT_U8, CTRL_AL);

  bool tl_alloc_success = (tl_L != nullptr) && (tl_R != nullptr) && (tl_Y != nullptr);
  tensor_lmem *tl_B = nullptr;
  if (p_param.with_bias) {
    shape_t shape_B = shape_t2(2, p_param.R_col_num);
    tl_B = tl_alloc(shape_B, p_param.opd2_sign ? FMT_I8 : FMT_U8, CTRL_AL);
    if (tl_B == nullptr) {
      tl_alloc_success = false;
    }
  }

  if (tl_alloc_success) {
    tl_load_fixed(tl_L, gaddr_L, CTRL_NULL);
    tl_load_fixed(tl_R, gaddr_R, CTRL_NULL);
    if (p_param.add_result) {
      tl_load_fixed(tl_Y, gaddr_Y, CTRL_NULL);
      ctrls |= CTRL_RA;
    }
    if (p_param.with_bias)
      tl_load_fixed(tl_B, gaddr_B, CTRL_NULL);
    if (p_param.do_relu)
      ctrls |= CTRL_RELU;
    test_bmkernel_submit();
    tl_Y->shape = shape_t2(p_param.L_row_num, p_param.R_col_num);
    if (tl_B)
      tl_B->shape = shape_t2(1, p_param.R_col_num);
    tl_matrix_mac_fixed(
        tl_Y, tl_L, tl_R, tl_B,
        p_param.l_shift_width,
        p_param.r_shift_width,
        p_param.is_result_int8,
        ctrls);

    tl_Y->shape = shape_t2(2 * p_param.L_row_num, p_param.R_col_num);
    tl_store_fixed(tl_Y, gaddr_Y, CTRL_NULL);
    test_bmkernel_submit();

    // copy from device memory
    memcpy((uint8_t*)Y_lo_hi, (uint8_t *)gaddr_Y, (2 * p_param.L_row_num * p_param.R_col_num));

    if(p_param.is_result_int8)  {
      for (int idx = 0; idx < p_param.L_row_num * p_param.R_col_num; idx++) {
        int8_Y[idx] = Y_lo_hi[idx]; // low 8bit
      }
    } else {
      for (int idx = 0; idx < p_param.L_row_num * p_param.R_col_num; idx++) {
        Y[idx] = Y_lo_hi[idx + p_param.L_row_num * p_param.R_col_num]; // high 8bit
        Y[idx] = Y_lo_hi[idx] + (Y[idx] << 8); // low 8bit + (high 8bit << 8)
      }
    }

    int *Ycmp = (int *)malloc(p_param.L_row_num * p_param.R_col_num * sizeof(int));
    if(p_param.is_result_int8)  {
      fill_int_with_char(Ycmp,  (char *)int8_Y, p_param.L_row_num * p_param.R_col_num);
    }  else  {
      fill_int_with_short(Ycmp,  (short *)Y, p_param.L_row_num * p_param.R_col_num);
    }

    int cmp_res = array_cmp_int(
        "Comparing results of native_fc and atomic_fc_with_stride ...\n",
        Y_ref, Ycmp, p_param.L_row_num * p_param.R_col_num);

    free(Ycmp);

    if(cmp_res != 0) {
      printf("Comparison FAILED for native_fc and atomic_fc_with_stride!!!\n");
      print_fc_param(p_param);
      exit(-1);
    }
  }
  if (tl_Y)
    tl_free(tl_Y);
  if (tl_R)
    tl_free(tl_R);
  if (tl_L)
    tl_free(tl_L);

  test_gmem_free(ctx, gaddr_Y);
  test_gmem_free(ctx, gaddr_R);
  test_gmem_free(ctx, gaddr_L);
  if (p_param.with_bias) {
    test_gmem_free(ctx, gaddr_B);
  }

  free(L);
  free(R);
  free(B);
  free(Y);
  free(Y_ref);

  printf("Comparison done for native_fc and atomic_fc_with_stride!\n");
  return tl_alloc_success ? 1 : 0;
}

static bool fc_res0_sign(fc_param_t *p)
{
  if (p->with_bias) {
    if (p->opd0_sign || p->opd1_sign || p->opd2_sign) {
      if (p->do_relu)
        return false;
      else
        return true;
    } else {
      return false;
    }
  } else {
    if (p->opd0_sign || p->opd1_sign) {
      if (p->do_relu)
        return false;
      else
        return true;
    } else {
      return false;
    }
  }
}

void init_fc_param(fc_param_t *p_param)
{
  srand((int)timer_get_tick());

  p_param->start_local_mem_idx_L = 0;
  p_param->start_local_mem_idx_R = 0;
  p_param->L_row_num = rand() % 64 + 1;
  p_param->L_col_num = rand() % 64 + 1;
  p_param->R_col_num = rand() % 64 + 1;
  p_param->with_bias = rand() % 2;
  p_param->l_shift_width = rand() % 15;
  p_param->r_shift_width = rand() % 27 + 5;

  p_param->is_result_int8 = rand() % 2;
  p_param->add_result = rand() % 2;
  if (p_param->add_result)
    p_param->do_relu = 0;
  else
    p_param->do_relu = rand() % 2;

  p_param->opd0_sign = (bool)rand() % 2;
  p_param->opd1_sign = (bool)rand() % 2;
  p_param->opd2_sign = (bool)rand() % 2;
  p_param->res0_sign = fc_res0_sign(p_param);
}

int testcase_fc_main(bmctx_t *ctx)
{
  int test_iteration_num = 2;
  int test_finished_num = 0;

  for (int i = 0; i < test_iteration_num; i++) {
    printf("random_test_fc iteration: %d\n", i);
    fc_param_t param;
    init_fc_param(&param);

    for (int with_bias = 0; with_bias < 2; with_bias++) {
      for (int res_int8 = 0; res_int8 < 2; res_int8++) {
        param.with_bias = with_bias;
        param.is_result_int8 = res_int8;
        test_finished_num += test_fc(param, ctx);
      }
    }
  }
  printf("Test finished %d/%d\n", test_finished_num, test_iteration_num);
  return 0;
}

int testcase_fc()
{
  bmctx_t ctx;
  test_init(&ctx);
  testcase_fc_main(&ctx);
  test_exit(&ctx);
  return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  bmctx_t ctx;
  test_init(&ctx);
  testcase_fc_main(&ctx);
  test_exit(&ctx);
  return 0;
}
#endif
