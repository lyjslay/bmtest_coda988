#include "boot_test.h"
#include "1880_test_util.h"

static void test_put_and_get_tensor(bmctx_t *ctx)
{
  int n = 2;
  int c = 66;
  int h = 1;
  int w = 15;
  int size = n * c * h * w;
  shape_t s = shape_t4(n, c, h, w);
  uint8_t *data_x = (uint8_t *)xmalloc(size);
  uint8_t *data_y = (uint8_t *)xmalloc(size);

  for (int i = 0; i < size; i++)
    data_x[i] = i - 100;

  for (int i = 0; i < size; i++)
    data_y[i] = -i;

  /*
   * Interleave two tensors in case the same devmem is reused between
   * put_tensor() and get_tensor(), in which case the content of
   * devmem is already what is expected before tl_store_fixed().
   */
  tensor_lmem *tl_x = tl_alloc(s, FMT_I8, CTRL_AL);
  tensor_lmem *tl_y = tl_alloc(s, FMT_I8, CTRL_AL);
  put_tensor(ctx, tl_x, data_x);
  put_tensor(ctx, tl_y, data_y);

  uint8_t *result_x = get_tensor(ctx, tl_x);
  uint8_t *result_y = get_tensor(ctx, tl_y);
  for (int i = 0; i < size; i++) {
    if (result_x[i] != data_x[i]) {
      printf("compare failed at result_x[%d]\n", i);
      exit(-1);
    }
    if (result_y[i] != data_y[i]) {
      printf("compare failed at result_y[%d]\n", i);
      exit(-1);
    }
  }
  free(result_x);
  free(result_y);

  /*
   * Get result_y before result_x.
   */
  result_y = get_tensor(ctx, tl_y);
  result_x = get_tensor(ctx, tl_x);
  for (int i = 0; i < size; i++) {
    if (result_x[i] != data_x[i]) {
      printf("compare failed at result_x[%d]\n", i);
      exit(-1);
    }
    if (result_y[i] != data_y[i]) {
      printf("compare failed at result_y[%d]\n", i);
      exit(-1);
    }
  }
  free(result_x);
  free(result_y);

  tl_free(tl_y);
  tl_free(tl_x);
  free(data_x);
  free(data_y);
}

int testcase_tensor_transfer()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_put_and_get_tensor(&ctx);
  test_exit(&ctx);
  return 0;
}

//module_testcase("1", testcase_tensor_transfer);


#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  return testcase_tensor_transfer();
}
#endif


