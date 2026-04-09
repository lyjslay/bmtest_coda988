#include "boot_test.h"
#include "1880_test_util.h"

#define G_N   8*256
#define G_C   2
#define G_H   8
#define G_W   8

void gdma_g2g(u64 ga_out, u64 ga_in)
{
  test_bmkernel_init();
  shape_t shape = shape_t4(G_N, G_C * 4, G_H, G_W); // *4 is a temp solution to align GDMA size and test step size
#if 0
  g_copy(ga_out, ga_in, shape, FMT_F32);
#else
  stride_t stride;
  stride.w = 1;
  stride.h = G_W;
  stride.c = G_H * G_W;
  stride.n = G_H * G_W * G_C * 4;

  tensor_gmem src;
  tensor_gmem dst;

  src.addr = ga_in;
  src.shape = shape;
  src.stride = stride;

  dst.addr = ga_out;
  dst.shape = shape;
  dst.stride = stride;

  tg_copy_gdma_fixed(&dst, &src, CTRL_NULL);
#endif
  test_bmkernel_submit();
  bmk_exit();
  test_bmkernel_exit();
}

int test_gdma_g2g_stress(u8 s_level, u32 d_pattern)
{
  int err = 0;
  u64 ga_in = 0, ga_out = 0;
  u64 start_addr = 0x001000000;
  u64 end_addr   = 0x010000000;
  u64 step_count = (G_N * G_C * G_H * G_W);
  u64 step_size = (G_N * G_C * G_H * G_W * 4);

  switch (s_level) {
    case 0:
      globalmem_fill_rand_ival(start_addr, step_count);
      break;
    case 1:
      globalmem_fill_u32_const(start_addr, d_pattern, step_count);
      break;
    default:
      return 1;
      break;
  }
  //uartlog("fill done\n");
  u64 last_block = end_addr - step_size - step_size;
  for (int iter = 0; iter < 10; iter++) {
    for (u64 i = start_addr; i <= last_block; i += step_size) {
      ga_in = i;
      ga_out = i + step_size;
      //uartlog("test iter %d in 0x%lx, out 0x%lx\n", iter, ga_in, ga_out);
      gdma_g2g(ga_out, ga_in);
      //uartlog("test iter %d in 0x%lx, out 0x%lx done\n", iter, ga_in, ga_out);
    }
    switch (s_level) {
      case 0:
        err += globalmem_cmp_u32_rand(ga_out, step_count);
        break;
      case 1:
        err += globalmem_cmp_u32_const(ga_out, d_pattern, step_count);
        break;
      default:
        break;
    }
    if (s_level == 0)
    {
      uartlog("ddr gdma test,stress:%d, iter:%d, datapattern random %s \n", s_level, iter, err?"failed":"passed");
    }else
    {
      uartlog("ddr gdma test,stress:%d, iter:%d, datapattern %08x %s \n", s_level, iter, d_pattern, err?"failed":"passed");
    }
  }

  return err;
}

int testcase_gdma_stress(void)
{
  uartlog("%s\n", __func__);

  int err = 0;
  for (int i = 0; i < 1000000; i++) {
    err += test_gdma_g2g_stress(0, 0);
    err += test_gdma_g2g_stress(1, 0xffffffff);
    err += test_gdma_g2g_stress(1, 0x00000000);
    err += test_gdma_g2g_stress(1, 0x55aa55aa);
    err += test_gdma_g2g_stress(1, 0xaa55aa55);
    err += test_gdma_g2g_stress(1, 0x55555555);
    err += test_gdma_g2g_stress(1, 0xaaaaaaaa);
    err += test_gdma_g2g_stress(1, 0xff00ff00);
    err += test_gdma_g2g_stress(1, 0x00ff00ff);
    err += test_gdma_g2g_stress(1, 0x12345678);
    err += test_gdma_g2g_stress(1, 0x87654321);
    uartlog("stress iter:%d, err:%d\n", i, err);
    if (err)
      return err;
  }

  uartlog("%s err:%d, %s\n", __func__, err, err?"failed":"passed");

  return err;
}
//module_testcase("1", testcase_gdma_stress);

#ifndef BUILD_TEST_CASE_all
int testcase_main(void)
{
  return testcase_gdma_stress();
}
#endif
