#include "boot_test.h"
#include "1880_test_util.h"

static void test_put_and_get_matrix(bmctx_t *ctx)
{
  int row = 5;
  int col = 16 * 5 + 2;
  int size = row * col;
  shape_t s = shape_t2(row, col);
  uint8_t *data_x = (uint8_t *)xmalloc(size);
  uint8_t *data_y = (uint8_t *)xmalloc(size);

  for (int i = 0; i < size; i++)
    data_x[i] = i - 100;

  for (int i = 0; i < size; i++)
    data_y[i] = -i;

  /*
   * Interleave two matrice in case the same devmem is reused between
   * put_matrix() and get_matrix(), in which case the content of
   * devmem is already what is expected before tl_store_matrix_fixed().
   */
  tensor_lmem *tl_x = put_matrix(ctx, s, data_x);
  tensor_lmem *tl_y = put_matrix(ctx, s, data_y);

  uint8_t *result_x = get_matrix(ctx, tl_x);
  uint8_t *result_y = get_matrix(ctx, tl_y);
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
  result_y = get_matrix(ctx, tl_y);
  result_x = get_matrix(ctx, tl_x);
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

int testcase_matrix_transfer()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_put_and_get_matrix(&ctx);
  test_exit(&ctx);
  return 0;
}
//module_testcase("1", testcase_matrix_transfer);

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  return testcase_matrix_transfer();
}
#endif


