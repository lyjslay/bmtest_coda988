#include "1880_test_util.h"

static void put_tensor_stride_unalign_ref(
    uint8_t *ref, uint8_t *a, shape_t lmem_shape,
    stride_t gmem_stride)
{
  int n, c, h, w;
  tl_shape_to_nchw(lmem_shape, &n, &c, &h, &w);

  int n_str = gmem_stride.n;
  int c_str = gmem_stride.c;
  int h_str = gmem_stride.h;
  int w_str = gmem_stride.w;

  for (int ni = 0; ni < n; ni++) {
    for (int ci = 0; ci < c; ci++) {
      for (int hi = 0; hi < h; hi++) {
        for (int wi = 0; wi < w; wi++) {
          u64 src_i = ni * n_str + ci * c_str + hi * h_str + wi * w_str;
          u64 dst_i = ci * n * h * w + ni * h * w + hi * w + wi;
          ref[dst_i] = a[src_i];
        }
      }
    }
  }
}

static void test_put_tensor_stride_unalign(bmctx_t *ctx)
{
  int n = 6;
  int c = 17;
  int h = 1;
  int w = 8;
  shape_t lmem_shape = shape_t4(n, c, h, w);

  stride_t gmem_stride;
  gmem_stride.w = 1;
  gmem_stride.h = w * 2;
  gmem_stride.c = gmem_stride.h * h * 2;
  gmem_stride.n = gmem_stride.c * c * 2;

  int size = n * c * h * w;
  int stride_size = gmem_stride.n * n;

  uint8_t *data_x = (uint8_t *)xmalloc(stride_size);
  for (int i = 0; i < stride_size; i++)
    data_x[i] = i;

  tensor_lmem *tl_x = tl_alloc(lmem_shape, FMT_I8, CTRL_NULL);
  put_tensor_stride(ctx, tl_x, gmem_stride, data_x);
  tl_x->shape = shape_t4(1, c, n * h, w);
  uint8_t *result_x = get_tensor(ctx, tl_x);
  tl_x->shape = lmem_shape;

  uint8_t *ref_x = (uint8_t *)xmalloc(size);
  put_tensor_stride_unalign_ref(ref_x, data_x, lmem_shape, gmem_stride);

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

int testcase_put_tensor_stride_unalign()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_put_tensor_stride_unalign(&ctx);
  test_exit(&ctx);
  return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_put_tensor_stride_unalign(&ctx);
  test_exit(&ctx);
  return 0;
}
#endif
