#include <stdint.h>
#include "boot_test.h"
#include "system_common.h"
#include "module_testcase.h"
#include "mmio.h"

extern const char fw_version_info[];
extern int dhrystone_test ();

// program starts here
int testcase_dhrystone()
{
    int result = 0;
    printf("fw_version_info: %s\n", fw_version_info);
    printf("built time: %s %s\n", __DATE__, __TIME__);
    result = dhrystone_test ();
    return result;
}

#if 0
#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
    return testcase_dhrystone();
}
#endif
#endif

module_testcase("1", testcase_dhrystone);
