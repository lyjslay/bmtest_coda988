#include "boot_test.h"
#include "1880_test_util.h"

static void tg_copy_gdma_ref(
    uint8_t *ref, uint8_t *src, shape_t shape,
    stride_t dst_stride, stride_t src_stride)
{
  int n, c, h, w;
  tl_shape_to_nchw(shape, &n, &c, &h, &w);

  for (int ni = 0; ni < n; ni++) {
    for (int ci = 0; ci < c; ci++) {
      for (int hi = 0; hi < h; hi++) {
        for (int wi = 0; wi < w; wi++) {
          int src_i = ni * src_stride.n + ci * src_stride.c +
              hi * src_stride.h + wi;
          int dst_i = ni * dst_stride.n + ci * dst_stride.c +
              hi * dst_stride.h + wi;
          ref[dst_i] = src[src_i];
        }
      }
    }
  }
}

static void test_tg_copy_gdma(bmctx_t *ctx)
{
  int n = 2;
  int c = 15;
  int h = 10;
  int w = 8;
  shape_t shape = shape_t4(n, c, h, w);

  stride_t src_stride;
  src_stride.w = 1;
  src_stride.h = w * 2;
  src_stride.c = w * 2 * h * 2;
  src_stride.n = w * 2 * h * 2 * c * 2;

  stride_t dst_stride;
  dst_stride.w = 1;
  dst_stride.h = w;
  dst_stride.c = h * w;
  dst_stride.n = h * w * c;

  int src_size = src_stride.n * n;
  int dst_size = n * c * h * w;

  uint8_t *src_data = (uint8_t *)xmalloc(src_size);
  for (int i = 0; i < src_size; i++)
    src_data[i] = i;

  shape_t src_stride_shape = shape_t4(src_size, 1, 1, 1);
  gaddr_t src_mem = bm_tensor_s2d(ctx, src_stride_shape, src_data);
  tensor_gmem tg_src;
  tg_src.addr = src_mem;
  tg_src.shape = shape;
  tg_src.stride = src_stride;

  gaddr_t dst_mem = test_gmem_alloc(ctx, dst_size);
  tensor_gmem tg_dst;
  tg_dst.addr = dst_mem;
  tg_dst.shape = shape;
  tg_dst.stride = dst_stride;

  tg_copy_gdma_fixed(&tg_dst, &tg_src, CTRL_NULL);
  test_bmkernel_submit();

  uint8_t *dst_data = (uint8_t *)xmalloc(dst_size);
  memcpy(dst_data, (void *)dst_mem, dst_size);

  uint8_t *ref_data = (uint8_t *)xmalloc(dst_size);
  tg_copy_gdma_ref(ref_data, src_data, shape, dst_stride, src_stride);

  for (int i = 0; i < dst_size; i++) {
    if (dst_data[i] != ref_data[i]) {
      printf("compare failed at result_x[%d], got %d, exp %d\n",
             i, dst_data[i], ref_data[i]);
      exit(-1);
    }
  }

  test_gmem_free(ctx, src_mem);
  test_gmem_free(ctx, dst_mem);
  free(src_data);
  free(dst_data);
  free(ref_data);
}

int testcase_tg_copy_gdma()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_tg_copy_gdma(&ctx);
  test_exit(&ctx);
  return 0;
}
//module_testcase("1", testcase_tg_copy_gdma);

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
    return testcase_tg_copy_gdma();
}
#endif


