#include "1880_test_util.h"

static void get_tensor_tp_ref(uint8_t *ref, uint8_t *a, shape_t lmem_shape)
{
  int n, c, h, w;
  tl_shape_to_nchw(lmem_shape, &n, &c, &h, &w);

  for (int ni = 0; ni < n; ni++) {
    for (int ci = 0; ci < c; ci++) {
      for (int hi = 0; hi < h; hi++) {
        for (int wi = 0; wi < w; wi++) {
          u64 src_i = ni * c * h * w + ci * h * w + hi * w + wi;
          u64 dst_i = ci * n * h * w + ni * h * w + hi * w + wi;
          ref[dst_i] = a[src_i];
        }
      }
    }
  }
}

static void test_get_tensor_tp(bmctx_t *ctx)
{
  int n = 2;
  int c = 66;
  int h = 1;
  int w = 15;
  int size = n * c * h * w;
  shape_t lmem_shape = shape_t4(n, c, h, w);

  uint8_t *data_x = (uint8_t *)xmalloc(size);
  for (int i = 0; i < size; i++)
    data_x[i] = i;

  tensor_lmem *tl_x = tl_alloc(lmem_shape, FMT_I8, CTRL_AL);
  put_tensor(ctx, tl_x, data_x);
  uint8_t *result_x = get_tensor_tp(ctx, tl_x);

  uint8_t *ref_x = (uint8_t *)xmalloc(size);
  get_tensor_tp_ref(ref_x, data_x, lmem_shape);

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

int testcase_get_tensor_tp()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_get_tensor_tp(&ctx);
  test_exit(&ctx);
  return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_get_tensor_tp(&ctx);
  test_exit(&ctx);
  return 0;
}
#endif
