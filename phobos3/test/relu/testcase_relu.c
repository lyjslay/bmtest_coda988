#include "boot_test.h"
#include "1880_test_util.h"

static void tl_relu_ref(uint8_t *ofmap, uint8_t *ifmap, shape_t shape)
{
  for (int i = 0; i < shape_size(shape); i++)
    ofmap[i] = (int8_t)ifmap[i] < 0? 0: ifmap[i];
}

static void test_tl_relu(bmctx_t *ctx)
{
  shape_t ifmap_shape = shape_t4(1, 32, 1, 256);
  shape_t ofmap_shape = ifmap_shape;

  int ifmap_size = shape_size(ifmap_shape);
  int ofmap_size = shape_size(ofmap_shape);

  uint8_t *ifmap_data = (uint8_t *)xmalloc(ifmap_size);
  for (int i = 0; i < ifmap_size; i++)
    ifmap_data[i] = (uint8_t)(i % 256);

  uint8_t *ref_data = (uint8_t *)xmalloc(ofmap_size);
  tl_relu_ref(ref_data, ifmap_data, ifmap_shape);

  tensor_lmem *tl_ifmap = tl_alloc(ifmap_shape, FMT_I8, CTRL_AL);
  tensor_lmem *tl_ofmap = tl_alloc(ofmap_shape, FMT_I8, CTRL_AL);
  put_tensor(ctx, tl_ifmap, ifmap_data);
  tl_relu_fixed(tl_ofmap, tl_ifmap);
  uint8_t *ofmap_data = get_tensor(ctx, tl_ofmap);

  for (int i = 0; i < ofmap_size; i++) {
    if (ofmap_data[i] != ref_data[i]) {
      printf("comparing failed at ofmap_data[%d], got %x, exp %x\n",
             i, ofmap_data[i], ref_data[i]);
      exit(-1);
    }
  }

  tl_free(tl_ofmap);
  tl_free(tl_ifmap);

  free(ifmap_data);
  free(ref_data);
  free(ofmap_data);
}

int testcase_relu()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_tl_relu(&ctx);
  test_exit(&ctx);
  return 0;
}
//module_testcase("0", testcase_relu);

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  return testcase_relu();
}
#endif
