#include "1880_test_util.h"

static void tl_copy_gdma_ref(
    uint8_t *ref, shape_t ref_shape,
    uint8_t *a, shape_t a_shape)
{
  int ref_n, ref_c, ref_h, ref_w;
  int a_n, a_c, a_h, a_w;
  tl_shape_to_nchw(ref_shape, &ref_n, &ref_c, &ref_h, &ref_w);
  tl_shape_to_nchw(a_shape, &a_n, &a_c, &a_h, &a_w);

  dim_t src_dim = { a_n, a_c, a_h, a_w };
  dim_t dst_dim = { ref_n, ref_c, ref_h, ref_w };

  for (int i = 0; i < dim_size(&src_dim); i++) {
    dim_t si = dim_of_ith_element(i, &src_dim, 0);
    dim_t di = dim_of_ith_element(i, &dst_dim, 0);
    int src = si.n * src_dim.c * src_dim.h * src_dim.w +
        si.c * src_dim.h * src_dim.w +
        si.h * src_dim.w +
        si.w;
    int dst = di.n * dst_dim.c * dst_dim.h * dst_dim.w +
        di.c * dst_dim.h * dst_dim.w +
        di.h * dst_dim.w +
        di.w;
    ref[dst] = a[src];
  }
}

static void test_tl_copy_gdma_fixed(bmctx_t *ctx)
{
  int n = 2;
  int c = 5;
  int h = 3;
  int w = 32;
  int size = n * c * h * w;

  uint8_t *data_x = (uint8_t *)xmalloc(size);
  for (int i = 0; i < size; i++)
    data_x[i] = i;

  shape_t shape_x = shape_t4(n, c, h, w);
  tensor_lmem *tl_x = tl_alloc(shape_x, FMT_I8, CTRL_AL);
  put_tensor(ctx, tl_x, data_x);

  shape_t shape_y = shape_t4(n, c * 2, h, w / 2);
  tensor_lmem *tl_y = tl_alloc(shape_y, FMT_I8, CTRL_AL);
  tl_copy_gdma_fixed(tl_y, tl_x, CTRL_NULL);
  uint8_t *result_y = get_tensor(ctx, tl_y);

  uint8_t *ref_y = (uint8_t *)xmalloc(size);
  tl_copy_gdma_ref(ref_y, shape_y, data_x, shape_x);

  for (int i = 0; i < size; i++) {
    if (result_y[i] != ref_y[i]) {
      printf("compare failed at [%d], got %d, exp %d\n",
             i, result_y[i], ref_y[i]);
      exit(-1);
    }
  }

  tl_free(tl_y);
  tl_free(tl_x);
  free(data_x);
  free(result_y);
  free(ref_y);
}

int testcase_tl_copy_gdma_fixed()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_tl_copy_gdma_fixed(&ctx);
  test_exit(&ctx);
  return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_tl_copy_gdma_fixed(&ctx);
  test_exit(&ctx);
  return 0;
}
#endif
