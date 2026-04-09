#include "boot_test.h"
#include "1880_test_util.h"

static void get_matrix_stride_ref(uint8_t *ref, uint8_t *a, shape_t lmem_shape,
                                  stride_t gmem_stride)
{
  int row = lmem_shape.row;
  int col = lmem_shape.col;
  int row_stride = gmem_stride.h;

  /*
   * Same as in get_matrix_stride().
   */
  int stride_size = row * row_stride;
  for (int i = 0; i < stride_size; i++)
    ref[i] = 0xaf;

  for (int ri = 0; ri < row; ri++) {
    for (int ci = 0; ci < col; ci++) {
      int src_i = ri * col + ci;
      int dst_i = ri * row_stride + ci;
      ref[dst_i] = a[src_i];
    }
  }
}

static void test_get_matrix_stride(bmctx_t *ctx)
{
  int row = 80;
  int col = 70;
  int size = row * col;
  shape_t lmem_shape = shape_t2(row, col);

  int row_stride = col * 2;
  stride_t gmem_stride;
  gmem_stride.h = row_stride;
  int stride_size = row * row_stride;

  uint8_t *data_x = (uint8_t *)xmalloc(size);
  for (int i = 0; i < size; i++)
    data_x[i] = i;

  tensor_lmem *tl_x = put_matrix(ctx, lmem_shape, data_x);
  uint8_t *result_x = get_matrix_stride(ctx, tl_x, gmem_stride);

  uint8_t *ref_x = (uint8_t *)xmalloc(stride_size);
  get_matrix_stride_ref(ref_x, data_x, lmem_shape, gmem_stride);

  for (int i = 0; i < stride_size; i++) {
    if (result_x[i] != ref_x[i]) {
      printf("compare failed at result_x[%d], got %d, exp %d\n",
             i, result_x[i], ref_x[i]);
      exit(-1);
    }
  }

  tl_free(tl_x);
  free(data_x);
  free(result_x);
  free(ref_x);
}

int testcase_get_matrix_stride()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_get_matrix_stride(&ctx);
  test_exit(&ctx);
  return 0;
}
module_testcase("1", testcase_get_matrix_stride);

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  return testcase_get_matrix_stride();
}
#endif
