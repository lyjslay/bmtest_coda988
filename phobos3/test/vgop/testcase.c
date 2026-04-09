#include <stdbool.h>
#ifdef ENV_EMU
#include "emu/command.h"
#include "emu/cli.h"
#else
#include "system_common.h"
#include "command.h"
#include "cli.h"
#endif

#include "testcase_vgop.h"

#ifndef BUILD_TEST_CASE_all
#ifdef ENV_EMU
int main(int argc, char *argv[])
#else
int testcase_main(void)
#endif
{
	int ret;

	uartlog("Enter into testcase_main function successfully !\n");
	ret = testcase_vgop();

	do {
		ret = cli_simple_loop();
	} while (ret == 0);

	return ret;
}
#endif