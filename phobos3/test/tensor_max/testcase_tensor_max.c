#include "1880_test_util.h"

static void tl_max_ref(int8_t *a, int8_t *b, int8_t *max, uint64_t size)
{
  for (uint64_t i = 0; i < size; i++) {
    if (a[i] > b[i])
      max[i] = a[i];
    else
      max[i] = b[i];
  }
}

static void test_tl_max(bmctx_t *ctx)
{
  shape_t shape = shape_t4(2, 32, 1, 17);
  uint64_t size = shape_size(shape);

  int8_t *a_data = (int8_t *)xmalloc(size);
  for (int i = 0; i < size; i++)
    a_data[i] = (int8_t)(i % 256);

  int8_t *b_data = (int8_t *)xmalloc(size);
  for (int i = 0; i < size; i++)
    b_data[i] = (int8_t)(100 - i % 256);

  int8_t *ref_data = (int8_t *)xmalloc(size);
  tl_max_ref(a_data, b_data, ref_data, size);

  tensor_lmem *tl_a = tl_alloc(shape, FMT_I8, CTRL_AL);
  tensor_lmem *tl_b = tl_alloc(shape, FMT_I8, CTRL_AL);
  tensor_lmem *tl_max = tl_alloc(shape, FMT_I8, CTRL_AL);
  put_tensor(ctx, tl_a, (uint8_t *)a_data);
  put_tensor(ctx, tl_b, (uint8_t *)b_data);
  tl_max_fixed(tl_max, tl_a, tl_b);
  uint8_t *max_data = get_tensor(ctx, tl_max);

  for (int i = 0; i < size; i++) {
    if ((int8_t)max_data[i] != ref_data[i]) {
      printf("comparing failed at ofmap_data[%d], got %x, exp %x\n",
             i, max_data[i], ref_data[i]);
      exit(-1);
    }
  }

  tl_free(tl_max);
  tl_free(tl_b);
  tl_free(tl_a);

  free(a_data);
  free(b_data);
  free(ref_data);
  free(max_data);
}

int testcase_tensor_max()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_tl_max(&ctx);
  test_exit(&ctx);
  return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_tl_max(&ctx);
  test_exit(&ctx);
  return 0;
}
#endif
