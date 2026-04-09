#include "boot_test.h"
#include "1880_test_util.h"

static void tl_arith_shift_ref(
    uint8_t *ref_high, uint8_t *ref_low,
    uint8_t *a_high, uint8_t *a_low,
    uint8_t *bits, uint64_t size)
{
  for (uint64_t i = 0; i < size; i++) {
    int32_t ta = ((int8_t)a_high[i] << 8) + a_low[i];
    int32_t tbits = (int8_t)bits[i];

    /*
     * Yes, a @tbits bigger than zero means shifting LEFT,
     * no matter whether the shift type is arithmetic
     * RIGHT shift or logic RIGHT shift.
     */
    int32_t res;
    if (tbits >= 0)
      res = ta << tbits;
    else
      res = ta >> -tbits;

    ref_high[i] = (res >> 8) & 0xff;
    ref_low[i] = res & 0xff;
  }
}

static void test_tl_arith_shift(bmctx_t *ctx)
{
  shape_t shape = shape_t4(2, 32, 1, 17);
  uint64_t size = shape_size(shape);

  uint8_t *a_high_data = (uint8_t *)xmalloc(size);
  uint8_t *a_low_data = (uint8_t *)xmalloc(size);
  uint8_t *bits_data = (uint8_t *)xmalloc(size);
  for (int i = 0; i < size; i++) {
    a_high_data[i] = 240 + i;
    a_low_data[i] = 200 + i;
    bits_data[i] = (i % 33) - 16;
  }

  uint8_t *ref_high_data = (uint8_t *)xmalloc(size);
  uint8_t *ref_low_data = (uint8_t *)xmalloc(size);
  tl_arith_shift_ref(
      ref_high_data, ref_low_data,
      a_high_data, a_low_data,
      bits_data, size);

  tensor_lmem *tl_a_low = tl_alloc(shape, FMT_I8, CTRL_AL);
  tensor_lmem *tl_a_high = tl_alloc(shape, FMT_I8, CTRL_AL);
  tensor_lmem *tl_bits = tl_alloc(shape, FMT_I8, CTRL_AL);
  tensor_lmem *tl_res_low = tl_alloc(shape, FMT_I8, CTRL_AL);
  tensor_lmem *tl_res_high = tl_alloc(shape, FMT_I8, CTRL_AL);
  put_tensor(ctx, tl_a_low, a_low_data);
  put_tensor(ctx, tl_a_high, a_high_data);
  put_tensor(ctx, tl_bits, bits_data);
  tl_arith_shift_fixed(
      tl_res_high, tl_res_low,
      tl_a_high, tl_a_low,
      tl_bits);
  uint8_t *res_high_data = get_tensor(ctx, tl_res_high);
  uint8_t *res_low_data = get_tensor(ctx, tl_res_low);

  for (int i = 0; i < size; i++) {
    if (res_high_data[i] != ref_high_data[i]) {
      printf("comparing failed at res_high_data[%d], got %d, exp %d\n",
             i, res_high_data[i], ref_high_data[i]);
      exit(-1);
    }
    if (res_low_data[i] != ref_low_data[i]) {
      printf("comparing failed at res_low_data[%d], got %d, exp %d\n",
             i, res_low_data[i], ref_low_data[i]);
      exit(-1);
    }
  }

  tl_free(tl_res_high);
  tl_free(tl_res_low);
  tl_free(tl_bits);
  tl_free(tl_a_high);
  tl_free(tl_a_low);

  free(a_high_data);
  free(a_low_data);
  free(bits_data);
  free(ref_high_data);
  free(ref_low_data);
  free(res_high_data);
  free(res_low_data);
}

int testcase_tensor_arith_shift()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_tl_arith_shift(&ctx);
  test_exit(&ctx);
  return 0;
}

//module_testcase("1", testcase_tensor_arith_shift);

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  return testcase_tensor_arith_shift();
}
#endif
