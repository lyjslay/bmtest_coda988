#include "1880_test_util.h"

static void tl_mul_const_ref(int8_t *ofmap, int8_t *ifmap, uint64_t size,
                             int8_t mul_const, int shift_bits)
{
  for (uint64_t i = 0; i < size; i++) {
    int32_t tmp = ifmap[i] * mul_const;
    tmp += 1 << (shift_bits - 1);
    tmp >>= shift_bits;
    if (tmp > 127)
      tmp = 127;
    else if (tmp < -128)
      tmp = -128;
    ofmap[i] = tmp;
  }
}

static void test_tl_mul_const(bmctx_t *ctx)
{
  shape_t ifmap_shape = shape_t4(1, 32, 1, 256);
  shape_t ofmap_shape = ifmap_shape;
  uint64_t size = shape_size(ifmap_shape);

  int8_t *ifmap_data = (int8_t *)xmalloc(size);
  for (int i = 0; i < size; i++)
    ifmap_data[i] = (uint8_t)(i % 256);

  int8_t mul_const = 20;
  int shift_bits = 1;

  int8_t *ref_data = (int8_t *)xmalloc(size);
  tl_mul_const_ref(ref_data, ifmap_data, size, mul_const, shift_bits);

  tensor_lmem *tl_ifmap = tl_alloc(ifmap_shape, FMT_I8, CTRL_AL);
  tensor_lmem *tl_ofmap = tl_alloc(ofmap_shape, FMT_I8, CTRL_AL);
  put_tensor(ctx, tl_ifmap, (uint8_t *)ifmap_data);
  tl_mul_const_fixed(NULL, tl_ofmap, tl_ifmap, mul_const, 1, shift_bits);
  uint8_t *ofmap_data = get_tensor(ctx, tl_ofmap);

  for (int i = 0; i < size; i++) {
    if ((int8_t)ofmap_data[i] != ref_data[i]) {
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

int testcase_tensor_mul_const()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_tl_mul_const(&ctx);
  test_exit(&ctx);
  return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_tl_mul_const(&ctx);
  test_exit(&ctx);
  return 0;
}
#endif
