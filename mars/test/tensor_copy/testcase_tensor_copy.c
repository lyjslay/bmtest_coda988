#include "boot_test.h"
#include "1880_test_util.h"

static void tl_copy_ref(int8_t *a, int8_t *res, uint64_t size)
{
  for (uint64_t i = 0; i < size; i++)
    res[i] = a[i];
}

static void test_tl_copy(bmctx_t *ctx)
{
  shape_t shape = shape_t4(2, 32, 1, 17);
  uint64_t size = shape_size(shape);

  int8_t *a_data = (int8_t *)xmalloc(size);
  for (int i = 0; i < size; i++)
    a_data[i] = (int8_t)(i % 256);

  int8_t *ref_data = (int8_t *)xmalloc(size);
  tl_copy_ref(a_data, ref_data, size);

  tensor_lmem *tl_a = tl_alloc(shape, FMT_I8, CTRL_AL);
  tensor_lmem *tl_res = tl_alloc(shape, FMT_I8, CTRL_AL);
  put_tensor(ctx, tl_a, (uint8_t *)a_data);
  tl_copy_fixed(tl_res, tl_a);
  uint8_t *res_data = get_tensor(ctx, tl_res);

  for (int i = 0; i < size; i++) {
    if ((int8_t)res_data[i] != ref_data[i]) {
      printf("comparing failed at ofmap_data[%d], got %x, exp %x\n",
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

int testcase_tensor_copy()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_tl_copy(&ctx);
  test_exit(&ctx);
  return 0;
}
//module_testcase("0", testcase_tensor_copy);

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  return testcase_tensor_copy();
}
#endif

