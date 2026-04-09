#include <stdbool.h>
#ifdef ENV_EMU
#include "emu/command.h"
#include "emu/cli.h"
#else
#include "system_common.h"
#include "command.h"
#include "cli.h"
#endif

#include "testcase_ldc.h"
#include "testcase_sclr.h"

extern int testcase_isp(void);

#ifndef BUILD_TEST_CASE_all
#ifdef ENV_EMU
int main(int argc, char *argv[])
#else
int testcase_main(void)
#endif
{
    int ret;

    ret = testcase_ldc();
    ret = testcase_sclr();
#ifdef VIP_INC_ISP
    ret = testcase_isp();
#endif

    do {
        ret = cli_simple_loop();
    } while (ret == 0);

    return ret;
}
#endif
