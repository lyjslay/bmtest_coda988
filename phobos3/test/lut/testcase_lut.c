#include "boot_test.h"
#include "1880_test_util.h"

static void tl_lut_ref(uint8_t *ofmap, uint8_t *ifmap, uint8_t *table,
                       shape_t ifmap_shape, shape_t table_shape)
{
  int in, ic, ih, iw;
  int tn, tc, th, tw;
  tl_shape_to_nchw(ifmap_shape, &in, &ic, &ih, &iw);
  tl_shape_to_nchw(table_shape, &tn, &tc, &th, &tw);
  assert(tn == 1);
  assert(th * tw == 256);

  for (int i = 0; i < shape_size(ifmap_shape); i++) {
    int ici = i / (ih * iw) % 32;
    ofmap[i] = table[ici * (th * tw) + ifmap[i]];
  }
}

static void test_tl_lut(bmctx_t *ctx)
{
  shape_t ifmap_shape = shape_t4(1, 32, 1, 224);
  shape_t table_shape = shape_t4(1, 32, 16, 16);
  shape_t ofmap_shape = ifmap_shape;

  int ifmap_size = shape_size(ifmap_shape);
  int table_size = shape_size(table_shape);
  int ofmap_size = shape_size(ofmap_shape);

  uint8_t *ifmap_data = (uint8_t *)xmalloc(ifmap_size);
  for (int i = 0; i < ifmap_size; i++)
    ifmap_data[i] = i - 20;

  uint8_t *table_data = (uint8_t *)xmalloc(table_size);
  for (int i = 0; i < table_size; i++)
    table_data[i] = i + i / 256 * 3;

  uint8_t *ref_data = (uint8_t *)xmalloc(ofmap_size);
  tl_lut_ref(ref_data, ifmap_data, table_data, ifmap_shape, table_shape);

  tensor_lmem *tl_ifmap = tl_alloc(ifmap_shape, FMT_I8, CTRL_AL);
  tensor_lmem *tl_table = tl_alloc(table_shape, FMT_I8, CTRL_AL);
  tensor_lmem *tl_ofmap = tl_alloc(ofmap_shape, FMT_I8, CTRL_AL);
  put_tensor(ctx, tl_ifmap, ifmap_data);
  put_tensor(ctx, tl_table, table_data);
  tl_lut_fixed(tl_ofmap, tl_ifmap, tl_table);
  uint8_t *ofmap_data = get_tensor(ctx, tl_ofmap);

  for (int i = 0; i < ofmap_size; i++) {
    if (ofmap_data[i] != ref_data[i]) {
      fprintf(stderr, "comparing failed at ofmap_data[%d], got %d, exp %d\n",
              i, ofmap_data[i], ref_data[i]);
      exit(-1);
    }
  }
  printf("comparing passed\n");

  tl_free(tl_ofmap);
  tl_free(tl_table);
  tl_free(tl_ifmap);

  free(ifmap_data);
  free(table_data);
  free(ref_data);
  free(ofmap_data);
}

int testcase_lut()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_tl_lut(&ctx);
  test_exit(&ctx);
  return 0;
}
//module_testcase("1", testcase_lut);

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  testcase_lut();
  return 0;
}
#endif