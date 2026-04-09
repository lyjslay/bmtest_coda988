#include "1880_test_util.h"

static void tl_matrix_mac_2_ref(
    uint8_t *ref, uint8_t *left, uint8_t *right,
    int left_row, int left_col, int right_col)
{
  /*
   * @left and @ref is row-major, @right is column-major.
   */
  int right_row = left_col;
  int res_row = left_row;
  int res_col = right_col;

  for (int row = 0; row < res_row; row++) {
    for (int col = 0; col < res_col; col++) {
      int32_t tmp = 0;
      for (int i = 0; i < left_col; i++) {
        int32_t l = (int8_t)left[row * left_col + i];
        int32_t r = (int8_t)right[col * right_row + i];
        tmp += l * r;
      }

      int32_t low_8 = tmp & 0xff;
      int32_t high_8 = (tmp >> 8) & 0xff;
      ref[row * res_col + col] = (low_8 + high_8) & 0xff;
    }
  }
}

static void test_tl_matrix_mac_2(bmctx_t *ctx)
{
  /*
   * Hardware supports 255x256 matrice only.
   */
  int left_row = 256;
  int left_col = 256;
  int right_col = 256;
  int res_row = 256;
  int res_col = 256;

  shape_t left_shape = shape_t4(1, left_row, 1, left_col);
  shape_t right_shape = shape_t4(right_col, 16, 1, 16);
  shape_t res_shape = shape_t4(1, res_row, 1, res_col);
  uint64_t left_size = shape_size(left_shape);
  uint64_t right_size = shape_size(right_shape);
  uint64_t res_size = shape_size(res_shape);

  uint8_t *left_data = (uint8_t *)xmalloc(left_size);
  for (int i = 0; i < left_size; i++)
    left_data[i] = (i % 3) - 1;

  uint8_t *right_data = (uint8_t *)xmalloc(right_size);
  for (int i = 0; i < right_size; i++)
    right_data[i] = 47 - i % 97;

  uint8_t *ref_data = (uint8_t *)xmalloc(res_size);
  tl_matrix_mac_2_ref(ref_data, left_data, right_data,
                      left_row, left_col, right_col);
  tensor_lmem *tl_left = tl_alloc(left_shape, FMT_I8, CTRL_AL);
  tensor_lmem *tl_right = tl_alloc(right_shape, FMT_I8, CTRL_AL);
  tensor_lmem *tl_res = tl_alloc(res_shape, FMT_I8, CTRL_AL);
  put_tensor(ctx, tl_left, left_data);
  put_tensor(ctx, tl_right, right_data);
  tl_matrix_mac_2_fixed(tl_res, tl_left, tl_right);
  uint8_t *res_data = get_tensor(ctx, tl_res);
  for (int i = 0; i < res_size; i++) {
    if (res_data[i] != ref_data[i]) {
      printf("comparing failed at res_data[%d], got %d, exp %d\n",
             i, res_data[i], ref_data[i]);
      exit(-1);
    }
  }
  printf("comparing passed\n");

  tl_free(tl_res);
  tl_free(tl_right);
  tl_free(tl_left);

  free(res_data);
  free(left_data);
  free(right_data);
}

int testcase_matrix_mac_2()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_tl_matrix_mac_2(&ctx);
  test_exit(&ctx);
  return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_tl_matrix_mac_2(&ctx);
  test_exit(&ctx);
  return 0;
}
#endif

