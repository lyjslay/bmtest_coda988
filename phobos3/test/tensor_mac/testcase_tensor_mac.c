#include "boot_test.h"
#include "1880_test_util.h"

static void tl_mac_ref(
    uint8_t *ref_high, uint8_t *ref_low,
    uint8_t *a, uint8_t *b, uint8_t *c_high, uint8_t *c_low,
    int lshift_width, int rshift_width, uint64_t size)
{
  for (uint64_t i = 0; i < size; i++) {
    int32_t ta = (int8_t)a[i];
    int32_t tb = (int8_t)b[i];
    int32_t tc = ((int8_t)c_high[i] << 8) + c_low[i];
    tc <<= lshift_width;
    int32_t res = ta * tb + tc;

    res += 1 << (rshift_width - 1);
    res >>= rshift_width;

    if (res > 32767)
      res = 32767;
    else if (res < -32768)
      res = -32768;
    ref_high[i] = (res >> 8) & 0xff;
    ref_low[i] = res & 0xff;
  }
}

static void test_tl_mac(bmctx_t *ctx)
{
  int lshift_width = 1;
  int rshift_width = 3;
  shape_t shape = shape_t4(1, 32, 1, 256);
  uint64_t size = shape_size(shape);

  uint8_t *a_data = (uint8_t *)xmalloc(size);
  uint8_t *b_data = (uint8_t *)xmalloc(size);
  uint8_t *c_high_data = (uint8_t *)xmalloc(size);
  uint8_t *c_low_data = (uint8_t *)xmalloc(size);

  for (int i = 0; i < size; i++) {
    a_data[i] = i % 256;
    b_data[i] = 100 - i;
    c_high_data[i] = 200 - i;
    c_low_data[i] = 200 + 2 * i;
  }

  uint8_t *ref_high_data = (uint8_t *)xmalloc(size);
  uint8_t *ref_low_data = (uint8_t *)xmalloc(size);
  tl_mac_ref(ref_high_data, ref_low_data,
             a_data, b_data, c_high_data, c_low_data,
             lshift_width, rshift_width, size);

  tensor_lmem *tl_a = tl_alloc(shape, FMT_I8, CTRL_AL);
  tensor_lmem *tl_b = tl_alloc(shape, FMT_I8, CTRL_AL);
  tensor_lmem *tl_c_low = tl_alloc(shape, FMT_I8, CTRL_AL);
  tensor_lmem *tl_c_high = tl_alloc(shape, FMT_I8, CTRL_AL);
  put_tensor(ctx, tl_a, a_data);
  put_tensor(ctx, tl_b, b_data);
  put_tensor(ctx, tl_c_low, c_low_data);
  put_tensor(ctx, tl_c_high, c_high_data);
  tl_mac_fixed(tl_c_high, tl_c_low, 0, tl_a, tl_b, lshift_width, rshift_width);
  uint8_t *mac_high_data = get_tensor(ctx, tl_c_high);
  uint8_t *mac_low_data = get_tensor(ctx, tl_c_low);

  for (int i = 0; i < size; i++) {
    if (mac_high_data[i] != ref_high_data[i]) {
      printf("comparing failed at mac_high_data[%d], got %d, exp %d\n",
             i, mac_high_data[i], ref_high_data[i]);
      exit(-1);
    }
    if (mac_low_data[i] != ref_low_data[i]) {
      printf("comparing failed at mac_low_data[%d], got %d, exp %d\n",
             i, mac_low_data[i], ref_low_data[i]);
      exit(-1);
    }
  }

  tl_free(tl_c_high);
  tl_free(tl_c_low);
  tl_free(tl_b);
  tl_free(tl_a);

  free(a_data);
  free(b_data);
  free(c_high_data);
  free(c_low_data);
  free(ref_high_data);
  free(ref_low_data);
  free(mac_high_data);
  free(mac_low_data);
}

int testcase_tensor_mac()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_tl_mac(&ctx);
  test_exit(&ctx);
  return 0;
}
//module_testcase("1", testcase_tensor_mac);

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  return testcase_tensor_mac();
}
#endif

