#include "1880_test_util.h"

static void get_tensor_stride_ref(uint8_t *ref, uint8_t *a, shape_t lmem_shape,
                                  stride_t gmem_stride)
{
  int n, c, h, w;
  tl_shape_to_nchw(lmem_shape, &n, &c, &h, &w);

  int n_str = gmem_stride.n;
  int c_str = gmem_stride.c;
  int h_str = gmem_stride.h;
  int w_str = gmem_stride.w;

  /*
   * Same as in get_tensor_stride().
   */
  int stride_size = n * gmem_stride.n;
  for (int i = 0; i < stride_size; i++)
    ref[i] = 0xcf;

  for (int ni = 0; ni < n; ni++) {
    for (int ci = 0; ci < c; ci++) {
      for (int hi = 0; hi < h; hi++) {
        for (int wi = 0; wi < w; wi++) {
          u64 src_i = ni * c * h * w + ci * h * w + hi * w + wi;
          u64 dst_i = ni * n_str + ci * c_str + hi * h_str + wi * w_str;
          ref[dst_i] = a[src_i];
        }
      }
    }
  }
}

static void test_get_tensor_stride(bmctx_t *ctx)
{
  int n = 2;
  int c = 15;
  int h = 10;
  int w = 8;
  shape_t lmem_shape = shape_t4(n, c, h, w);

  int size = n * c * h * w;
  uint8_t *data_x = (uint8_t *)xmalloc(size);
  for (int i = 0; i < size; i++)
    data_x[i] = i;

  stride_t gmem_stride;
  gmem_stride.w = 1;
  gmem_stride.h = w * 2;
  gmem_stride.c = gmem_stride.h * h * 2;
  gmem_stride.n = gmem_stride.c * c * 2;

  tensor_lmem *tl_x = tl_alloc(lmem_shape, FMT_I8, CTRL_AL);
  put_tensor(ctx, tl_x, data_x);
  uint8_t *result_x = get_tensor_stride(ctx, tl_x, gmem_stride);

  int stride_size = n * gmem_stride.n;
  uint8_t *ref_x = (uint8_t *)xmalloc(stride_size);
  get_tensor_stride_ref(ref_x, data_x, lmem_shape, gmem_stride);

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

int testcase_get_tensor_stride()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_get_tensor_stride(&ctx);
  test_exit(&ctx);
  return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_get_tensor_stride(&ctx);
  test_exit(&ctx);
  return 0;
}
#endif
