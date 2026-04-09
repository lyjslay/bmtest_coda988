#include "1880_test_util.h"

static void tl_mdsum_ref(int8_t *a, int8_t *res, int n, int c, int h, int w)
{
  s32 *tmp_res = (s32 *)xmalloc(c * sizeof(s32));
  for (int i = 0; i < c; i++)
    tmp_res[i] = 0;

  for (int ni = 0; ni < n; ni++) {
    for (int ci = 0; ci < c; ci++) {
      for (int hi = 0; hi < h; hi++) {
        for (int wi = 0; wi < w; wi++) {
          uint64_t h_size = w;
          uint64_t c_size = h * h_size;
          uint64_t n_size = c * c_size;
          uint64_t i = ni * n_size + ci * c_size + hi * h_size + wi;
          tmp_res[ci] += a[i];
        }
      }
    }
  }

  for (uint64_t i = 0; i < c; i++) {
    if (tmp_res[i] > 127)
      tmp_res[i] = 127;
    else if (tmp_res[i] < -128)
      tmp_res[i] = -128;
 }

  for (uint64_t i = 0; i < c; i++)
    res[i] = tmp_res[i];

  free(tmp_res);
}

static void test_tl_mdsum(bmctx_t *ctx)
{
  int n = 4;
  int c = 32;
  int h = 1;
  int w = 17;

  shape_t shape = shape_t4(n, c, h, w);
  shape_t res_shape = shape_t4(1, c, 1, 1);
  uint64_t size = shape_size(shape);

  int8_t *a_data = (int8_t *)xmalloc(size);
  for (uint64_t i = 0; i < size; i++)
    a_data[i] = 1 + (i % 3) - (i % 5);

  int8_t *ref_data = (int8_t *)xmalloc(c);
  tl_mdsum_ref(a_data, ref_data, n, c, h, w);

  tensor_lmem *tl_a = tl_alloc(shape, FMT_I8, CTRL_AL);
  tensor_lmem *tl_res = tl_alloc(res_shape, FMT_I8, CTRL_AL);
  put_tensor(ctx, tl_a, (uint8_t *)a_data);
  tl_mdsum_fixed(NULL, tl_res, tl_a);
  uint8_t *res_data = get_tensor(ctx, tl_res);

  for (int i = 0; i < c; i++) {
    if ((int8_t)res_data[i] != ref_data[i]) {
      printf("comparing failed at ofmap_data[%d], got %d, exp %d\n",
             i, res_data[i], ref_data[i]);
      exit(-1);
    }
  }

  tl_free(tl_res);
  tl_free(tl_a);

  free(a_data);
  free(ref_data);
  free(res_data);
}

int testcase_tensor_mdsum()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_tl_mdsum(&ctx);
  test_exit(&ctx);
  return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_tl_mdsum(&ctx);
  test_exit(&ctx);
  return 0;
}
#endif
