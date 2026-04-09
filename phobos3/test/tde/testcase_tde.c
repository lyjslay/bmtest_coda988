#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "bm_types.h"
#include "command.h"
#include "system_common.h"
#include "regconfig.h"
#include "timer.h"
#include "cli.h"
#include "tde_reg.h"
#include "tde_reg_tab.h"



#define TDE_IRQ_NUM               16
#define TDE_ALIGN                 16
#define TDE_WAIT_TIME             100

#define SRC_SURFACE_ADDR          0x82000000
#define DST_SURFACE_ADDR          0x83000000

u32 time_start;
u8 irq_flag;


int tde_check_irq_handler(int timeout)
{
	int i = 0;

	for (i = 0; i < timeout; ++i) {
		if (irq_flag > 0) {
			irq_flag = 0;
			uartlog("%s: check tick:%d\n", __func__, i);
			return 0;
		}
		mdelay(1);
	}

	return -1;
}

static void tde_ip_init(void)
{
	tde_clk_gate(true);
	tde_clear_intr();
}

static int tde_run_general(enum tde_mode mode, struct tde_surface_mem *src_buffer,
	struct tde_surface_mem *dst_buffer)
{
	int ret = 0;

	uartlog("--------------------\n");
	uartlog("mode(%d).\n", mode);
	uartlog("src: w(%d) h(%d) stirde(%d) addr(%lx)\n",
		src_buffer->width, src_buffer->height, src_buffer->stride, src_buffer->addr);
	uartlog("dst: w(%d) h(%d) stirde(%d) addr(%lx)\n",
		dst_buffer->width, dst_buffer->height, dst_buffer->stride, dst_buffer->addr);

	tde_ip_init();
	tde_set_mode(mode);
	tde_set_src_cfg(src_buffer);
	tde_set_dst_cfg(dst_buffer);

	irq_flag = 0;
	time_start = timer_meter_get_us();
	tde_start();

	if (tde_check_irq_handler(TDE_WAIT_TIME)) {
		uartlog("tde end intr doesn't happens after %d tick\n", TDE_WAIT_TIME);
		ret = -1;
	} else {
		uartlog("TEST CASE PASS!!!\n");
	}

	return ret;
}

void set_param_default(struct tde_surface_mem *src_mem, struct tde_surface_mem *dst_mem)
{
	u32 w = 640, h = 320;

	src_mem->addr = SRC_SURFACE_ADDR;
	src_mem->width = w;
	src_mem->height = h;
	src_mem->stride = ALIGN(w, TDE_ALIGN);

	dst_mem->addr = DST_SURFACE_ADDR;
	dst_mem->width = h;
	dst_mem->height = w;
	dst_mem->stride = ALIGN(h, TDE_ALIGN);

}

int tde_rotate90(int argc, char **argv)
{
	struct tde_surface_mem src_mem, dst_mem;

	uartlog("===%s===\n", __func__);

	set_param_default(&src_mem, &dst_mem);
	return tde_run_general(TDE_MODE_ROTATE_90, &src_mem, &dst_mem);
}

int tde_rotate270(int argc, char **argv)
{
	struct tde_surface_mem src_mem, dst_mem;

	uartlog("===%s===\n", __func__);

	set_param_default(&src_mem, &dst_mem);
	return tde_run_general(TDE_MODE_ROTATE_270, &src_mem, &dst_mem);
}


int tde_irq(int irq, void *_dev)
{
	u32 time_end = timer_meter_get_us();
	u32 status = tde_intr_status();

	if (irq != TDE_IRQ_NUM) {
		uartlog("irq(%d) error.\n", irq);
		return 0;
	}

	tde_clear_intr();
	uartlog("intr status(%x).\n", status);

	if (status) {
	    uartlog("TDE frame done, cost time(%d - %d = %d).\n",
			time_end, time_start, time_end - time_start);
		irq_flag = 1;
	}

    return 0;
}

static struct cmd_entry test_cmd_tde_list[] __attribute__ ((unused)) = {
    {"rotate90",               tde_rotate90,              0, "[rotate90]"},
    {"rotate270",              tde_rotate270,             0, "[rotate270]"},
    {NULL, NULL, 0, NULL}
};

#ifdef ENV_EMU
int main(int argc, char *argv[])
#else
int testcase_main(void)
#endif
{
    int ret;

    uartlog("TDE main start!\n");

	for(int i = 0; i < ARRAY_SIZE(test_cmd_tde_list) - 1; i++) {
		command_add(&test_cmd_tde_list[i]);
	}

	tde_set_base_addr((void *)TDE_REG_BASE_ADDR);
    ret = request_irq(TDE_IRQ_NUM, tde_irq, 0, "tde", NULL);
	if (ret) {
		uartlog("TDE request_irq failed!\n");
	}

    do {
        ret = cli_simple_loop();
    } while (0);

	return ret;
}

