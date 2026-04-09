#include "1880_test_util.h"

static void tl_mul_ref(int8_t *ofmap, int8_t *a, int8_t *b, uint64_t size,
                       int shift_bits)
{
  for (uint64_t i = 0; i < size; i++) {
    int32_t tmp = a[i] * b[i];
    tmp += 1 << (shift_bits - 1);
    tmp >>= shift_bits;
    if (tmp > 127)
      tmp = 127;
    else if (tmp < -128)
      tmp = -128;
    ofmap[i] = tmp;
  }
}

static void test_tl_mul(bmctx_t *ctx)
{
  shape_t shape = shape_t4(1, 32, 1, 256);
  uint64_t size = shape_size(shape);
  int shift_bits = 1;

  int8_t *a_data = (int8_t *)xmalloc(size);
  int8_t *b_data = (int8_t *)xmalloc(size);
  for (int i = 0; i < size; i++) {
    a_data[i] = i;
    b_data[i] = 128 - i;
  }

  tensor_lmem *tl_a = tl_alloc(shape, FMT_I8, CTRL_AL);
  tensor_lmem *tl_b = tl_alloc(shape, FMT_I8, CTRL_AL);
  tensor_lmem *tl_res_low = tl_alloc(shape, FMT_I8, CTRL_AL);
  put_tensor(ctx, tl_a, (uint8_t *)a_data);
  put_tensor(ctx, tl_b, (uint8_t *)b_data);
  tl_mul_fixed(NULL, tl_res_low, tl_a, tl_b, shift_bits);
  uint8_t *res_low_data = get_tensor(ctx, tl_res_low);

  int8_t *ref_data = (int8_t *)xmalloc(size);
  tl_mul_ref(ref_data, a_data, b_data, size, shift_bits);

  for (int i = 0; i < size; i++) {
    if ((int8_t)res_low_data[i] != ref_data[i]) {
      printf("comparing failed at res_low_data[%d], got %x, exp %x\n",
             i, res_low_data[i], ref_data[i]);
      exit(-1);
    }
  }

  tl_free(tl_res_low);
  tl_free(tl_b);
  tl_free(tl_a);

  free(a_data);
  free(b_data);
  free(ref_data);
  free(res_low_data);
}

int testcase_tensor_mul()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_tl_mul(&ctx);
  test_exit(&ctx);
  return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_tl_mul(&ctx);
  test_exit(&ctx);
  return 0;
}
#endif
