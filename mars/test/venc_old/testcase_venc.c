#include <stdio.h>
#include <boot_test.h>
#include <system_common.h>
#include <command.h>
#include "string.h"
#include "cli.h"
#include "mmio.h"
#include "coda988_test.h"


#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

int case0_get_vpu_info(){
	TestArgV_t stArgv = {0};
	int ret = 0;
	char *name = "case0_get_vpu_info";
	stArgv.argc = 1;
	stArgv.showVersion = 1;
	uartlog("----------------%s START!-------------------------\n", name);
	ret = coda988_test(&stArgv);
	if(ret) {
		uartlog("----------------%s  FAIL!-------------------------\n", name);
	} else {
		uartlog("----------------%s  SUCCESS!-----------------------\n", name);
	}
	return ret;
}

/************************************************************
//>>> 全I帧测试， IntraPeroid = 1，不同分辨率
************************************************************/
int case1_352_288(int argc, char **argv)
{
	TestArgV_t stArgv = {0};
	char *name = "case1_352_288";
	int ret = 0;
	stArgv.argc = 1;
	stArgv.showVersion = 0;
	stArgv.n = 5;
	stArgv.picWidth = 352;
	stArgv.picHeight = 288;
	stArgv.enNv21 = 0;
	stArgv.rotate = 0;
	stArgv.mirror = 0;
	stArgv.gopSize = 1; // 全I帧
	stArgv.idrInterval = 1; // I都是IDR
	stArgv.rcEnable = 0; //CQP
	stArgv.avcProfile = 2;
	uartlog("----------------%s START!-------------------------\n", name);
	ret = coda988_test(&stArgv);
	if(ret) {
		uartlog("----------------%s  SUCCESS!-----------------------\n", name);
	} else {
		uartlog("----------------%s  SUCCESS!-----------------------\n", name);
	}
	return 0;
}

int case1_1920_1080(int argc, char **argv)
{
	TestArgV_t stArgv = {0};
	char *name = "case1_1920_1080";
	int ret = 0;
	stArgv.argc = 1;
	stArgv.n = 5;
	stArgv.picWidth = 1920;
	stArgv.picHeight = 1080;
	stArgv.enNv21 = 0;
	stArgv.rotate = 0;
	stArgv.mirror = 0;
	stArgv.gopSize = 1; // 全I帧
	stArgv.avcProfile = 0;
	uartlog("----------------%s START!-------------------------\n", name);
	ret = coda988_test(&stArgv);
	if(ret) {
		uartlog("----------------%s  SUCCESS!-----------------------\n", name);
	} else {
		uartlog("----------------%s  SUCCESS!-----------------------\n", name);
	}
	return 0;
}

int case1_2304_1296(int argc, char **argv)
{
	TestArgV_t stArgv = {0};
	char *name = "case1_2304_1296";
	int ret = 0;
	stArgv.argc = 1;
	stArgv.n = 5;
	stArgv.picWidth = 2304;
	stArgv.picHeight = 1296;
	stArgv.enNv21 = 0;
	stArgv.rotate = 0;
	stArgv.mirror = 0;
	stArgv.gopSize = 1; // 全I帧
	stArgv.avcProfile = 0;
	uartlog("----------------%s START!-------------------------\n", name);
	ret = coda988_test(&stArgv);
	if(ret) {
		uartlog("----------------%s  SUCCESS!-----------------------\n", name);
	} else {
		uartlog("----------------%s  SUCCESS!-----------------------\n", name);
	}
	return 0;
}

/************************************************************
//>>> IPPP帧测试(baseline intra_period=4)
************************************************************/
int case2_352_288(int argc, char **argv)
{
	TestArgV_t stArgv = {0};
	char *name = "case2_352_288";
	int ret = 0;
	stArgv.argc = 1;
	stArgv.n = 5;
	stArgv.picWidth = 352;
	stArgv.picHeight = 288;
	stArgv.enNv21 = 0;
	stArgv.rotate = 0;
	stArgv.mirror = 0;
	stArgv.gopSize = 4; // IPPP帧
	stArgv.avcProfile = 0; // baseline
	uartlog("----------------%s START!-------------------------\n", name);
	ret = coda988_test(&stArgv);
	if(ret) {
		uartlog("----------------%s  SUCCESS!-----------------------\n", name);
	} else {
		uartlog("----------------%s  SUCCESS!-----------------------\n", name);
	}
	return 0;
}
int case2_1920_1080(int argc, char **argv)
{
	TestArgV_t stArgv = {0};
	char *name = "case1_1920_1080";
	int ret = 0;
	stArgv.argc = 1;
	stArgv.n = 5;
	stArgv.picWidth = 1920;
	stArgv.picHeight = 1080;
	stArgv.enNv21 = 0;
	stArgv.rotate = 0;
	stArgv.mirror = 0;
	stArgv.gopSize = 4; // IPPP帧
	stArgv.avcProfile = 0; // baseline
	uartlog("----------------%s START!-------------------------\n", name);
	ret = coda988_test(&stArgv);
	if(ret) {
		uartlog("----------------%s  SUCCESS!-----------------------\n", name);
	} else {
		uartlog("----------------%s  SUCCESS!-----------------------\n", name);
	}
	return 0;
}

/************************************************************
//>>> 码率控制测试 intra_period=10 highprofile
************************************************************/
int case3_1920_1080_500k(int argc, char **argv)
{
	TestArgV_t stArgv = {0};
	char *name = "case3_1920_1080_500k";
	int ret = 0;
	stArgv.argc = 1;
	stArgv.n = 10;
	stArgv.picWidth = 1920;
	stArgv.picHeight = 1080;
	stArgv.enNv21 = 0;
	stArgv.rotate = 0;
	stArgv.mirror = 0;
	stArgv.gopSize = 10; //
	stArgv.avcProfile = 2; // highprofile
	stArgv.rcEnable = 1;
	stArgv.targetBitrate = 500; // kbps for coda988
	stArgv.iqpmax = 42;
	stArgv.iqpmin = 16;
	stArgv.pqpmax = 42;
	stArgv.pqpmin = 16;

	uartlog("----------------%s START!-------------------------\n", name);
	ret = coda988_test(&stArgv);
	if(ret) {
		uartlog("----------------%s  SUCCESS!-----------------------\n", name);
	} else {
		uartlog("----------------%s  SUCCESS!-----------------------\n", name);
	}
	return 0;
}
int case3_1920_1080_800k(int argc, char **argv)
{
	TestArgV_t stArgv = {0};
	char *name = "case3_1920_1080_800k";
	int ret = 0;
	stArgv.argc = 1;
	stArgv.n = 10;
	stArgv.picWidth = 1920;
	stArgv.picHeight = 1080;
	stArgv.enNv21 = 0;
	stArgv.rotate = 0;
	stArgv.mirror = 0;
	stArgv.gopSize = 10; //
	stArgv.avcProfile = 2; // highprofile
	stArgv.rcEnable = 1;
	stArgv.targetBitrate = 800; // kbps for coda988
	stArgv.iqpmax = 42;
	stArgv.iqpmin = 16;
	stArgv.pqpmax = 42;
	stArgv.pqpmin = 16;

	uartlog("----------------%s START!-------------------------\n", name);
	ret = coda988_test(&stArgv);
	if(ret) {
		uartlog("----------------%s  SUCCESS!-----------------------\n", name);
	} else {
		uartlog("----------------%s  SUCCESS!-----------------------\n", name);
	}
	return 0;
}
int case3_1920_1080_4m(int argc, char **argv)
{
	TestArgV_t stArgv = {0};
	char *name = "case3_1920_1080_4m";
	int ret = 0;
	stArgv.argc = 1;
	stArgv.n = 10;
	stArgv.picWidth = 1920;
	stArgv.picHeight = 1080;
	stArgv.enNv21 = 0;
	stArgv.rotate = 0;
	stArgv.mirror = 0;
	stArgv.gopSize = 10; //
	stArgv.avcProfile = 2; // highprofile
	stArgv.rcEnable = 1;
	stArgv.targetBitrate = 4000; // kbps for coda988
	stArgv.iqpmax = 42;
	stArgv.iqpmin = 16;
	stArgv.pqpmax = 42;
	stArgv.pqpmin = 16;

	uartlog("----------------%s START!-------------------------\n", name);
	ret = coda988_test(&stArgv);
	if(ret) {
		uartlog("----------------%s  SUCCESS!-----------------------\n", name);
	} else {
		uartlog("----------------%s  SUCCESS!-----------------------\n", name);
	}
	return 0;
}
int case3_1920_1080_max(int argc, char **argv)
{
	TestArgV_t stArgv = {0};
	char *name = "case3_1920_1080_max";
	int ret = 0;
	stArgv.argc = 1;
	stArgv.n = 10;
	stArgv.picWidth = 1920;
	stArgv.picHeight = 1080;
	stArgv.enNv21 = 0;
	stArgv.rotate = 0;
	stArgv.mirror = 0;
	stArgv.gopSize = 10; //
	stArgv.avcProfile = 2; // highprofile
	stArgv.targetBitrate = 0; // close rate control, max bitrate
	stArgv.rcEnable = 0;
	stArgv.iqpmax = 1;
	stArgv.iqpmin = 1;
	stArgv.pqpmax = 1;
	stArgv.pqpmin = 1;
	uartlog("----------------%s START!-------------------------\n", name);
	ret = coda988_test(&stArgv);
	if(ret) {
		uartlog("----------------%s  SUCCESS!-----------------------\n", name);
	} else {
		uartlog("----------------%s  SUCCESS!-----------------------\n", name);
	}
	return 0;
}

/************************************************************
//>>> addrRemap 测试
************************************************************/
int case4_addr_remap(int argc, char **argv)
{
	TestArgV_t stArgv = {0};
	char *name = "case4_addr_remap";
	int ret = 0;
	stArgv.argc = 1;
	stArgv.n = 10;
	stArgv.picWidth = 1920;
	stArgv.picHeight = 1080;
	stArgv.enNv21 = 0;
	stArgv.rotate = 0;
	stArgv.mirror = 0;
	stArgv.gopSize = 10; //
	stArgv.avcProfile = 2; // highprofile
	stArgv.rcEnable = 0;
	stArgv.iqpmax = 1;
	stArgv.iqpmin = 1;
	stArgv.pqpmax = 1;
	stArgv.pqpmin = 1;
	uartlog("----------------%s START!-------------------------\n", name);
	ret = coda988_test(&stArgv);
	if(ret) {
		uartlog("----------------%s  SUCCESS!-----------------------\n", name);
	} else {
		uartlog("----------------%s  SUCCESS!-----------------------\n", name);
	}
	return 0;
}




static struct cmd_entry test_cmd_list[] __attribute__((unused)) = {
	{"case0_get_vpu_info", case0_get_vpu_info, 0, NULL},
    {"case1_352_288", case1_352_288, 0, NULL},
	{"case1_1920_1080", case1_1920_1080, 0, NULL},
	{"case2_352_288", case2_352_288, 0, NULL},
	{"case2_1920_1080", case2_1920_1080, 0, NULL},
	{"case3_1920_1080_500k", case3_1920_1080_500k, 0, NULL},
	{"case3_1920_1080_800k", case3_1920_1080_800k, 0, NULL},
	{"case3_1920_1080_4m", case3_1920_1080_4m, 0, NULL},
	{"case3_1920_1080_max", case3_1920_1080_max, 0, NULL},
    {NULL, NULL, 0, NULL}
};

int testcase_venc(void)
{
	int i = 0;
	for (i = 0; i < ARRAY_SIZE(test_cmd_list) - 1; i++)
	{
		command_add(&test_cmd_list[i]);
	}
	uartlog("testcase_sclr: add command done\n");
	u32 ret;
	do
	{
		ret = cli_simple_loop();
	} while (ret == 0);
	return 0;
}

#ifndef BUILD_TEST_CASE_ALL
int testcase_main()
{
    return testcase_venc();

}
#endif

module_testcase("1", testcase_venc);