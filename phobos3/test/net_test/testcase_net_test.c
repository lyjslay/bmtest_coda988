#include "boot_test.h"
#include "net_test.h"
#include "testcase_net_test.h"

int testcase_net_test(void)
{
  return do_net_test(&net_test_param);
}

//module_testcase("1", testcase_net_test);

#ifndef BUILD_TEST_CASE_all
int testcase_main(void)
{

  return testcase_net_test();
}
#endif

