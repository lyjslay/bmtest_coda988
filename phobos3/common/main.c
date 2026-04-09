#include "boot_test.h"
#include "mmio.h"
#include "system_common.h"
#include "cli.h"

extern module_func_t __module_testcase_start[];
extern module_func_t __module_testcase_end[];

//#define BUILD_TEST_CASE_all

#ifdef BUILD_TEST_CASE_all
int testcase_main(void)
{
    int ret;
    module_func_t *func;

    for(func = __module_testcase_start; func < __module_testcase_end; func++)
    {
        ret = (*func)();
        if(ret)
            return ret;
    }

    return ret;
}
#else
extern int testcase_main();
#endif

int main(void)
{
  int r;
  uartlog("%s %s\n", __DATE__, __TIME__);
  uartlog("phobos bmtest start\n");

#ifdef INIT_DDR
  plat_bm_ddr_init();
#endif

  r = testcase_main();
  uartlog("test done\n");

  cli_simple_loop();

  return r;
}
