#include <common.h>
#include "module_testcase.h"
#include "system_common.h"
#include "mmio.h"
#include "timer.h"
#include <stdlib.h>
#include "testcase_sw_irq.h"
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include "command.h"
#include "cli.h"

#ifndef BUILD_TEST_CASE_all

#define ARRAY_SIZE(arr)           (sizeof(arr) / sizeof((arr)[0]))


int get_param_str(int argc, char **argv, char *index, char *content)
{
	int i=0;
	for(i=0; i<argc-1; i++){
		if(strcmp(argv[i], index) == 0){
			content = argv[i+1];
			return 0;
		}
	}
	return -1;
}

/*
* argv[0]:cmd
* argv[1]:--xxx
* argv[2]:val
*/
int get_param_uint32(int argc, char **argv, char *index, uint32_t *content)
{
	int i=0;
	if(argc<3 || argv==NULL || index==NULL) return -1;
	for(i=1; i<argc-1; i++){
		if(strcmp(argv[i], index) == 0){
			*content = atoi(argv[i+1]);
			return 0;
		}
	}
	return -1;
}


extern void memset_dword(void *buff, u32 val, u32 size);
extern void memset_dword_inc(void *buff, u32 start, u32 size);


static int sw_int_0_cpu2_irq_handler(int irqn, void *priv)
{
	mmio_write_32(BASE_TOP + 0xA0, 0xffff0000);
	printf("%s, SW_INT_0_CPU2, get irq=%d\n", __func__, irqn);
	// disable_irq(irqn);

    return 0;
}

static int sw_int_1_cpu2_irq_handler(int irqn, void *priv)
{
	mmio_write_32(BASE_TOP + 0xA4, 0xffff0000);
	printf("%s, SW_INT_1_CPU2 get irq=%d\n", __func__, irqn);
	// disable_irq(irqn);

    return 0;
}

static int trigger_sw_int_0_cpu2(int argc, char **argv)
{
	mmio_setbits_32(BASE_TOP + 0xA0, 0xf000f);
	printf("0x%x = 0x%x\n", BASE_TOP + 0x98, mmio_read_32(BASE_TOP + 0x98));
	
	return 0;
}

static int trigger_sw_int_1_cpu2(int argc, char **argv)
{
	mmio_setbits_32(BASE_TOP + 0xA4, 0xf000f);
	printf("0x%x = 0x%x\n", BASE_TOP + 0x9c, mmio_read_32(BASE_TOP + 0x9c));

	return 0;
}

static struct cmd_entry test_cmd_list[] __attribute__ ((unused)) = {
	{.name = "tc_000", .func = trigger_sw_int_0_cpu2, .arg_count = 0, .usage = "trigger_sw_int_0_cpu2" },
	{.name = "tc_001", .func = trigger_sw_int_1_cpu2, .arg_count = 0, .usage = "trigger_sw_int_1_cpu2" },
};

int testcase_sw_irq(void)
{
	int i, ret;

	printf("\n------------ test start ------------\n\n");

	ret = request_irq(SW_INT_0_CPU2, sw_int_0_cpu2_irq_handler, 0, "SW_INT_0_CPU2", NULL);
	if(0 == ret){
		printf("request_irq %d success\n", SW_INT_0_CPU2);
	}
	ret = request_irq(SW_INT_1_CPU2, sw_int_1_cpu2_irq_handler, 0, "SW_INT_0_CPU2", NULL);
	if(0 == ret){
		printf("request_irq %d success\n", SW_INT_1_CPU2);
	}

	printf("Commands:\nname\targ_count\tusage:\n");
	for(i = 0;i < ARRAY_SIZE(test_cmd_list);i++) {
		command_add(&test_cmd_list[i]);
		printf("%s\t%d\t%s\n",test_cmd_list[i].name, test_cmd_list[i].arg_count, test_cmd_list[i].usage);
	}

	char param[4];
	while(1){
		printf("run all test? input y or n\n");
		cli_scanf("%s", param);
		if(0 == strcmp("y", param)){
			for(i = 0;i < ARRAY_SIZE(test_cmd_list);i++) {
				printf("%s\t%s\n",test_cmd_list[i].name, test_cmd_list[i].usage);
				test_cmd_list[i].func(0, 0);
			}
		}
		cli_simple_loop();
	}
	return 0;
}

int testcase_main()
{
	return testcase_sw_irq();
}
#endif

