#include "1880_test_util.h"

static void
put_tensor_tp_unalign_ref(uint8_t *ref, uint8_t *a, shape_t lmem_shape)
{
  /*
   * (c, n, h, w) => (n, c, h, w) => (1, c, n * h, w)
   */
  int size = shape_size(lmem_shape);
  for (int i = 0; i < size; i++)
    ref[i] = a[i];
}

static void test_put_tensor_tp_unalign(bmctx_t *ctx)
{
  int n = 2;
  int c = 15;
  int h = 1;
  int w = 8;
  int size = n * c * h * w;
  shape_t lmem_shape = shape_t4(n, c, h, w);

  uint8_t *data_x = (uint8_t *)xmalloc(size);
  for (int i = 0; i < size; i++)
    data_x[i] = i;

  tensor_lmem *tl_x = tl_alloc(lmem_shape, FMT_I8, CTRL_NULL);
  put_tensor_tp(ctx, tl_x, data_x);
  tl_x->shape = shape_t4(1, c, n * h, w);
  uint8_t *result_x = get_tensor(ctx, tl_x);
  tl_x->shape = lmem_shape;

  uint8_t *ref_x = (uint8_t *)xmalloc(size);
  put_tensor_tp_unalign_ref(ref_x, data_x, lmem_shape);

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

int testcase_put_tensor_tp_unalign()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_put_tensor_tp_unalign(&ctx);
  test_exit(&ctx);
  return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_put_tensor_tp_unalign(&ctx);
  test_exit(&ctx);
  return 0;
}
#endif
