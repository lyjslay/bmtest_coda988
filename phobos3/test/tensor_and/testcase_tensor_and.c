#include "boot_test.h"
#include "1880_test_util.h"

static void tl_and_int8_ref(int8_t *a, int8_t *b, int8_t *res, uint64_t size)
{
  for (uint64_t i = 0; i < size; i++)
    res[i] = a[i] & b[i];
}

static void tl_and_int16_ref(
    uint8_t *ref_high, uint8_t *ref_low,
    uint8_t *a_high, uint8_t *a_low,
    uint8_t *b_high, uint8_t *b_low,
    uint64_t size)
{
  for (uint64_t i = 0; i < size; i++) {
    int32_t ta = ((int8_t)a_high[i] << 8) + a_low[i];
    int32_t tb = ((int8_t)b_high[i] << 8) + b_low[i];
    int32_t res = ta & tb;
    ref_high[i] = (res >> 8) & 0xff;
    ref_low[i] = res & 0xff;
  }
}

static void test_tl_and_int8(bmctx_t *ctx)
{
  shape_t shape = shape_t4(2, 32, 1, 17);
  uint64_t size = shape_size(shape);

  int8_t *a_data = (int8_t *)xmalloc(size);
  for (int i = 0; i < size; i++)
    a_data[i] = (int8_t)(i % 256);

  int8_t *b_data = (int8_t *)xmalloc(size);
  for (int i = 0; i < size; i++)
    b_data[i] = (int8_t)(100 - i % 256);

  int8_t *ref_data = (int8_t *)xmalloc(size);
  tl_and_int8_ref(a_data, b_data, ref_data, size);

  tensor_lmem *tl_a = tl_alloc(shape, FMT_I8, CTRL_AL);
  tensor_lmem *tl_b = tl_alloc(shape, FMT_I8, CTRL_AL);
  tensor_lmem *tl_res = tl_alloc(shape, FMT_I8, CTRL_AL);
  put_tensor(ctx, tl_a, (uint8_t *)a_data);
  put_tensor(ctx, tl_b, (uint8_t *)b_data);
  tl_and_int8_fixed(tl_res, tl_a, tl_b);
  uint8_t *res_data = get_tensor(ctx, tl_res);

  for (int i = 0; i < size; i++) {
    if ((int8_t)res_data[i] != ref_data[i]) {
      printf("comparing failed at ofmap_data[%d], got %x, exp %x\n",
             i, res_data[i], ref_data[i]);
      exit(-1);
    }
  }

  tl_free(tl_res);
  tl_free(tl_b);
  tl_free(tl_a);

  free(a_data);
  free(b_data);
  free(ref_data);
  free(res_data);
}

static void test_tl_and_int16(bmctx_t *ctx)
{
  shape_t shape = shape_t4(2, 32, 1, 17);
  uint64_t size = shape_size(shape);

  uint8_t *a_high_data = (uint8_t *)xmalloc(size);
  uint8_t *a_low_data = (uint8_t *)xmalloc(size);
  uint8_t *b_high_data = (uint8_t *)xmalloc(size);
  uint8_t *b_low_data = (uint8_t *)xmalloc(size);
  for (int i = 0; i < size; i++) {
    a_high_data[i] = i / 10;
    a_low_data[i] = i;
    b_high_data[i] = (i + 250) / 20;
    b_low_data[i] = 100 - i;
  }

  uint8_t *ref_high_data = (uint8_t *)xmalloc(size);
  uint8_t *ref_low_data = (uint8_t *)xmalloc(size);
  tl_and_int16_ref(
      ref_high_data, ref_low_data,
      a_high_data, a_low_data,
      b_high_data, b_low_data,
      size);

  tensor_lmem *tl_a_low = tl_alloc(shape, FMT_I8, CTRL_AL);
  tensor_lmem *tl_a_high = tl_alloc(shape, FMT_I8, CTRL_AL);
  tensor_lmem *tl_b_low = tl_alloc(shape, FMT_I8, CTRL_AL);
  tensor_lmem *tl_b_high = tl_alloc(shape, FMT_I8, CTRL_AL);
  tensor_lmem *tl_res_low = tl_alloc(shape, FMT_I8, CTRL_AL);
  tensor_lmem *tl_res_high = tl_alloc(shape, FMT_I8, CTRL_AL);
  put_tensor(ctx, tl_a_low, a_low_data);
  put_tensor(ctx, tl_a_high, a_high_data);
  put_tensor(ctx, tl_b_low, b_low_data);
  put_tensor(ctx, tl_b_high, b_high_data);
  tl_and_int16_fixed(
      tl_res_high, tl_res_low,
      tl_a_high, tl_a_low,
      tl_b_high, tl_b_low);
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
  tl_free(tl_b_high);
  tl_free(tl_b_low);
  tl_free(tl_a_high);
  tl_free(tl_a_low);

  free(a_high_data);
  free(a_low_data);
  free(b_high_data);
  free(b_low_data);
  free(ref_high_data);
  free(ref_low_data);
  free(res_high_data);
  free(res_low_data);
}

int testcase_tensor_and()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_tl_and_int8(&ctx);
  test_tl_and_int16(&ctx);
  test_exit(&ctx);
  return 0;
}
//module_testcase("1", testcase_tensor_and);

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  return testcase_tensor_and();
}
#endif

