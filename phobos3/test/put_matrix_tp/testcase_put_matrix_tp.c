#include "1880_test_util.h"

static void matrix_tp_ref(uint8_t *ref, uint8_t *a, shape_t lmem_shape)
{
  int col = lmem_shape.row;
  int row = lmem_shape.col;

  for (int ri = 0; ri < row; ri++) {
    for (int ci = 0; ci < col; ci++) {
      ref[ci * row + ri] = a[ri * col + ci];
    }
  }
}

static void test_put_matrix_tp(bmctx_t *ctx)
{
  int row = 80;
  int col = 70;
  int size = row * col;
  shape_t lmem_shape = shape_t2(col, row);

  uint8_t *data_x = (uint8_t *)xmalloc(size);
  for (int i = 0; i < size; i++)
    data_x[i] = i;

  tensor_lmem *tl_x = put_matrix_tp(ctx, lmem_shape, data_x);
  tl_x->shape = shape_t2(col, row);
  uint8_t *result_x = get_matrix(ctx, tl_x);
  tl_x->shape = lmem_shape;

  uint8_t *ref_x = (uint8_t *)xmalloc(size);
  matrix_tp_ref(ref_x, data_x, lmem_shape);

  for (int i = 0; i < size; i++) {
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

int testcase_put_matrix_tp()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_put_matrix_tp(&ctx);
  test_exit(&ctx);
  return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_put_matrix_tp(&ctx);
  test_exit(&ctx);
  return 0;
}
#endif
