#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef ENV_HOSTPC
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include "cli/command.h"
#include "cli/cli.h"
#else
#include "command.h"
#include "cli.h"
#include "system_common.h"
#include "module_testcase.h"
#include "uart.h"
#include "timer.h"
#endif

#include "mmio.h"
#include "mars_reg_fmux_gpio.h"
#include "mars_pinlist_swconfig.h"

#include "../vip/reg.h"
#include "inc/isp_reg.h"

#include "sensorlist.h"
#include "vip_common.h"



#undef uartlog
#define uartlog(fmt, args...) \
{ \
    printf(fmt, ##args); \
    fflush(stdout); \
}

#define CIF_DPHY_ENABLE

static int do_nothing(void *priv);
static int i2cloop(void *);
//static int spiloop(void *);
static int sensor_verify_op(void *ctx);
int config_csibdg(uint32_t cam_id, uint32_t width, uint32_t height, pattern_ctx *ctx,
		uint8_t hdr, uint8_t yuv_sensor);

static pattern_ctx m_ptn_ctx = {
	.vreg_bases = NULL,
	.vreg_bases_pa = NULL,
	.phys_regs  = NULL,
	.mem_base   = DRAM_PHY_BASE,
	.alloc_size = 0,
	.alloc_cnt  = 0,
	.ptn        = NULL,
};

static struct isp_ctx m_ispctx[2];

static pattern_entry pattern_db[] = {
	{
		.name = "imx290_default",
		.priv = (void *)&imx290_info,
		.config_op = peri_up_0,
		.start_op = imx290_poweron,
		.stop_op = peri_down_0,
		.verify_op = sensor_verify_op,
	},
	{
		.name = "i2c_test_loop(imx290)",
		.priv = (void *)&imx290_info,
		.config_op = peri_up_0,
		.start_op = i2cloop,
		.stop_op = do_nothing,
		.verify_op = NULL,
	},
	{
		.name = "ar0237",
		.priv = (void *)&ar0237_info,
		.config_op = peri_up_2,
		.start_op = ar0237_poweron,
		.stop_op = peri_down_2,
		.verify_op = sensor_verify_op,
	},
	{
		.name = "i2c_test_loop(ar0237)",
		.priv = (void *)&ar0237_info,
		.config_op = peri_up_2,
		.start_op = i2cloop,
		.stop_op = do_nothing,
		.verify_op = NULL,
	},
	{
		.name = "ar0239",
		.priv = (void *)&ar0239_info,
		.config_op = peri_up_3,
		.start_op = ar0239_poweron,
		.stop_op = peri_down_3,
		.verify_op = sensor_verify_op,
	},
	{
		.name = "i2c_test_loop(ar0239)",
		.priv = (void *)&ar0239_info,
		.config_op = peri_up_3,
		.start_op = i2cloop,
		.stop_op = do_nothing,
		.verify_op = NULL,
	},
	{
		.name = "os08a10",
		.priv = (void *)&os08a10_info,
		.config_op = peri_up_4,
		.start_op = os08a10_poweron,
		.stop_op = peri_down_4,
		.intr_op = os08a10_intr,
		.verify_op = sensor_verify_op,
	},
	{
		.name = "i2c_test_loop(os08a10)",
		.priv = (void *)&os08a10_info,
		.config_op = peri_up_4,
		.start_op = i2cloop,
		.stop_op = do_nothing,
		.verify_op = NULL,
	},
	{
		.name = "as0142at",
		.priv = (void *)&as0142at_info,
		.config_op = peri_up_5,
		.start_op = as0142at_poweron,
		.stop_op = peri_down_5,
		.verify_op = sensor_verify_op,
	},
	{
		.name = "imx327",
		.priv = (void *)&imx327_info,
		.config_op = peri_up_6,
		.start_op = imx327_poweron,
		.stop_op = peri_down_6,
		.intr_op = imx327_intr,
		.verify_op = sensor_verify_op,
	},
	{
		.name = "os08a20",
		.priv = (void *)&os08a20_info,
		.config_op = peri_up_os08a20,
		.start_op = os08a20_poweron,
		.stop_op = peri_down_os08a20,
		.intr_op = os08a20_intr,
		.verify_op = sensor_verify_op,
	},
	{
		.name = "f35",
		.priv = (void *)&f35_info,
		.config_op = peri_up_f35,
		.start_op = f35_poweron,
		.stop_op = peri_down_f35,
		.intr_op = f35_intr,
		.verify_op = sensor_verify_op,
	},
	{
		.name = "f35(i2c)",
		.priv = (void *)&f35_info,
		.config_op = peri_up_f35,
		.start_op = i2cloop,
		.stop_op = peri_down_f35,
	},
	{
		.name = "f35_dvp",
		.priv = (void *)&f35_dvp_info,
		.config_op = peri_up_f35_dvp,
		.start_op = f35_dvp_poweron,
		.stop_op = peri_down_f35_dvp,
		.verify_op = sensor_verify_op,
	},
	{
		.name = "imx327_sublvds",
		.priv = (void *)&imx327lvds_info,
		.config_op = peri_up_imx327lvds,
		.start_op = imx327lvds_poweron,
		.stop_op = peri_down_imx327lvds,
		.verify_op = sensor_verify_op,
	},
	{
		.name = "imx327_slave",
		.priv = (void *)&imx327_slave_info,
		.config_op = peri_up_imx327_slave,
		.start_op = imx327_slave_poweron,
		.stop_op = peri_down_imx327_slave,
		.intr_op = imx327_slave_intr,
		.verify_op = sensor_verify_op,
	},
	{
		.name = "ov2736",
		.priv = (void *)&ov2736_info,
		.config_op = peri_up_ov2736,
		.start_op = ov2736_poweron,
		.stop_op = peri_down_ov2736,
		.intr_op = ov2736_intr,
		.verify_op = sensor_verify_op,
	},
	{
		.name = "f23",
		.priv = (void *)&f23_info,
		.config_op = peri_up_f23,
		.start_op = f23_poweron,
		.stop_op = peri_down_f23,
		.intr_op = f23_intr,
		.verify_op = sensor_verify_op,
	},
	{
		.name = "n5",
		.priv = (void *)&n5_info,
		.config_op = peri_up_n5,
		.start_op = n5_poweron,
		.stop_op = peri_down_n5,
		.intr_op = n5_intr,
		.verify_op = sensor_verify_op,
	},
	{
		.name = "n6",
		.priv = (void *)&n6_info,
		.config_op = peri_up_n6,
		.start_op = n6_poweron,
		.stop_op = peri_down_n6,
		.intr_op = n6_intr,
		.verify_op = sensor_verify_op,
	},
	{
		.name = "i2c_test_loop(n6)",
		.priv = (void *)&n6_info,
		.config_op = peri_up_n6,
		.start_op = i2cloop,
		.stop_op = do_nothing,
		.verify_op = NULL,
	},
	{
		.name = "END",
		.config_op = NULL,
	}
};

extern int cli_readline(char *prompt, char *cmdbuf);

/********************************************************************
 *   APIs to replace bmtest's standard APIs
 ********************************************************************/

int sensor_get_gain(pattern_entry *ptn, struct sensor_prop *prop)
{
	struct sensor_info *sinf;

	if (ptn == NULL) {
		uartlog("null sensor ptn\n");
		return -1;
	}
	sinf = (struct sensor_info *)ptn->priv;

	if (sinf->ops && sinf->ops->get_gain)
		sinf->ops->get_gain(sinf, prop);

	return 0;
}

int sensor_set_gain(pattern_entry *ptn, uint16_t gain)
{
	struct sensor_info *sinf;

	if (ptn == NULL) {
		uartlog("null sensor ptn\n");
		return -1;
	}
	sinf = (struct sensor_info *)ptn->priv;

	struct sensor_prop prop;
	if (sinf->ops && sinf->ops->get_gain)
		sinf->ops->get_gain(sinf, &prop);
	if (gain > prop.max || gain < prop.min)
		return -1;
	if (sinf->ops && sinf->ops->set_gain)
		sinf->ops->set_gain(sinf, gain);

	return 0;
}

int sensor_get_sgain(pattern_entry *ptn, struct sensor_prop *prop)
{
	struct sensor_info *sinf;

	if (ptn == NULL) {
		uartlog("null sensor ptn\n");
		return -1;
	}
	sinf = (struct sensor_info *)ptn->priv;

	if (sinf->ops && sinf->ops->get_sgain)
		sinf->ops->get_sgain(sinf, prop);

	return 0;
}

int sensor_set_sgain(pattern_entry *ptn, uint16_t gain)
{
	struct sensor_info *sinf;

	if (ptn == NULL) {
		uartlog("null sensor ptn\n");
		return -1;
	}
	sinf = (struct sensor_info *)ptn->priv;

	struct sensor_prop prop;
	if (sinf->ops && sinf->ops->get_sgain)
		sinf->ops->get_sgain(sinf, &prop);
	if (gain > prop.max || gain < prop.min)
		return -1;
	if (sinf->ops && sinf->ops->set_sgain)
		sinf->ops->set_sgain(sinf, gain);

	return 0;
}

int sensor_get_shsl(pattern_entry *ptn, struct sensor_prop *prop)
{
	struct sensor_info *sinf;

	if (ptn == NULL) {
		uartlog("null sensor ptn\n");
		return -1;
	}
	sinf = (struct sensor_info *)ptn->priv;

	if (sinf->ops && sinf->ops->get_shsl)
		sinf->ops->get_shsl(sinf, prop);

	return 0;
}

int sensor_set_shsl(pattern_entry *ptn, uint32_t shs)
{
	struct sensor_info *sinf;

	if (ptn == NULL) {
		uartlog("null sensor ptn\n");
		return -1;
	}
	sinf = (struct sensor_info *)ptn->priv;

	struct sensor_prop prop;
	if (sinf->ops && sinf->ops->get_shsl)
		sinf->ops->get_shsl(sinf, &prop);
	if (shs > prop.max || shs < prop.min)
		return -1;
	if (sinf->ops && sinf->ops->set_shsl)
		sinf->ops->set_shsl(sinf, shs);

	return 0;
}

int sensor_get_shss(pattern_entry *ptn, struct sensor_prop *prop)
{
	struct sensor_info *sinf;

	if (ptn == NULL) {
		uartlog("null sensor ptn\n");
		return -1;
	}
	sinf = (struct sensor_info *)ptn->priv;

	if (sinf->ops && sinf->ops->get_shss)
		sinf->ops->get_shss(sinf, prop);

	return 0;
}

int sensor_set_shss(pattern_entry *ptn, uint32_t shs)
{
	struct sensor_info *sinf;

	if (ptn == NULL) {
		uartlog("null sensor ptn\n");
		return -1;
	}
	sinf = (struct sensor_info *)ptn->priv;

	struct sensor_prop prop;
	if (sinf->ops && sinf->ops->get_shss)
		sinf->ops->get_shss(sinf, &prop);
	if (shs > prop.max || shs < prop.min)
		return -1;
	if (sinf->ops && sinf->ops->set_shss)
		sinf->ops->set_shss(sinf, shs);

	return 0;
}

static int do_nothing(void *priv)
{
	uartlog("do noting\n");
	return 0;
}

#define SNSR_TEST_CAPTURE_NUM	5

static int sensor_verify_op(void *priv)
{
	pattern_ctx *ctx = (pattern_ctx *)priv;
	struct isp_ctx     *ictx = &m_ispctx[0];
	struct sensor_info *sinf = (struct sensor_info *)ctx->ptn->priv;
	uint64_t    bayer_w_addr[SNSR_TEST_CAPTURE_NUM][2];
	int i;

#if 1
	for (i = 0; i < SNSR_TEST_CAPTURE_NUM; i++) {
		bayer_w_addr[i][0] = 0x86000000 + 0x2000000 * i;
		bayer_w_addr[i][1] = 0x87000000 + 0x2000000 * i;
		bayer_w_addr[i][0] = 0x88000000 + 0x2000000 * i;
		bayer_w_addr[i][1] = 0x89000000 + 0x2000000 * i;
	}
	while(1) {
		uint64_t    bufaddr;
		uint32_t    bufsize;
		uint32_t    tmp = 0, test_mode = 0, value = 0, value2 = 0;
		int cnt[4] = {0,0,0,0}, err_cnt[4] = {0,0,0,0};
		uint32_t    frm_start = 0x3, frm_done = 0x3;
		volatile uint32_t reg;

		bm_get_num("Leave test(0), GAIN(1), SGAIN(2), SHS(3), SHSS(4), Burning(5), Ref(6), Grp(7):", test_mode);

		if (!test_mode)
			break;

		if (test_mode != 6) {
			bm_get_num("Value:", value);
		} else {
			bm_get_num("shutter:", value);
			bm_get_num("gain:", value2);
		}

		/* capture the 1st default frame. */
		/* Polling Vsync */
		writel(0x0a070000, ictx->is_hdr_on ? 3 : 1);
		writel(0x0a0108e0, 0xFFFF);

		for(;;) {
			if (frm_start) {
				reg = readl(0x0a0108e0);
				
				for (i=0; i<2; i++) {
					uint32_t offset = i*8;

					if (frm_start & (1<<i)) {
						if ((reg & (0x2 << offset))) {
							writel(0x0a0108e0, 0xFF << offset);
							frm_start &= ~(1<<i);
						}
					}
				}
			}

			/* vsync handler */
			for (i=0; i<2; i++) {
				static int wr = 0;
				uint32_t offset = i*8;
				
				if ((frm_start & (1<<i)))
					continue;

				if (!(frm_done & (1<<i)))
					continue;

				frm_done &= ~(1<<i);

				/* set frame valid*/
				writel(0x0a010028, 1 << i);
				if (wr == 0) {
					writel(0x0a010028, 1 << i);
					wr++;
				} else if (wr == 1)
					writel(0x0a010028, 3);
				else
					writel(0x0a010028, 1 << i);

				if (cnt[i] == 1) {
					/* fire the i2c after the 1st frame is
					 * preraw frame done. */
					switch (test_mode) {
					case 1:
						/* set long gain */
						if (sinf->ops && sinf->ops->set_gain)
							sinf->ops->set_gain(sinf, value);
						if (readl(0x0a0108e0) & (0x2<<offset))
							uartlog("i2c cmd cross a fs\n");
						break;
					case 2:
						/* set short gain */
						if (sinf->ops && sinf->ops->set_sgain)
							sinf->ops->set_sgain(sinf, value);
						if (readl(0x0a0108e0) & (0x2<<offset))
							uartlog("i2c cmd cross a fs\n");
						break;
					case 3:
					case 6:
						/* Set long shutter */
						if (sinf->ops && sinf->ops->set_shsl)
							sinf->ops->set_shsl(sinf, value);
						if (readl(0x0a0108e0) & (0x2<<offset))
							uartlog("i2c cmd cross a fs\n");
						break;
					case 4:
						/* Set short shutter */
						if (sinf->ops && sinf->ops->set_shss)
							sinf->ops->set_shss(sinf, value);
						if (readl(0x0a0108e0) & (0x2<<offset))
							uartlog("i2c cmd cross a fs\n");
						break;
					case 5:
						//uartlog("burning %d frame, start!\n", value);
						break;
					case 7:
						if (sinf->ops && sinf->ops->test_mode)
							sinf->ops->test_mode(sinf, 0);
						break;
					default:
						return 0;
					}
				} else if (test_mode == 6 && cnt[i] == 2) {
					/* set long gain */
					if (sinf->ops && sinf->ops->set_gain)
						sinf->ops->set_gain(sinf, value2);
					if (readl(0x0a0108e0) & (0x2<<offset))
						uartlog("i2c cmd cross a fs\n");
				}

				writel(0x0a0108e0, 0xFF << (i*8));
			}

			/* error handler */
			reg = readl(0x0a0108e0);
			for (i=0; i<2; i++) {
				uint32_t offset = i*8;

				if ((frm_start & (1<<i)))
					continue;

				if (reg & (0xF0<<offset)) {
					uartlog("frm[%d][%d] err 0x%x\n",
							i, cnt[i], readl(0x0a0108e0));
					/* Stop if it is not burnin test.*/
					if (test_mode != 5)
						return -1;
					err_cnt[i]++;
				}
				writel(0x0a0108e0, 0xFF << (i*8));
			}

			/* frame done handler */
			reg = readl(0x0a070000);
			for (i=0; i<2; i++) {

				if ((frm_start & (1<<i)))
					continue;

				if (reg & (1<<i)) {
					
					/* Config the dma */
					if (!ictx->is_yuv_sensor) {
						bufaddr = bayer_w_addr[cnt[i]%SNSR_TEST_CAPTURE_NUM][i];
						writel((uintptr_t)(0x0A071404+i*0x20), bufaddr);
					} else {
						bufaddr = bayer_w_addr[cnt[i]%SNSR_TEST_CAPTURE_NUM][i];
						writel((uintptr_t)(0x0A071584+i*0x20), bufaddr);
					}
					writel(0x0a0108e0, 0xFF<<(i*8));
					cnt[i]++;
					frm_done |= (1<<i);
					writel(0x0a070000, 1 << i);
				}
			}

			/* leave loop*/
			if (cnt[0]>=(test_mode==5?value:SNSR_TEST_CAPTURE_NUM)) {
				if (!ictx->is_hdr_on || (cnt[1]>=(test_mode==5?value:SNSR_TEST_CAPTURE_NUM)))
					break;
			}
		}

		/* restore the dma */
		if (!ictx->is_yuv_sensor) {
			for (i = 0; i < 4; i++) {
				bufaddr = 0x82000000 + 0x01000000 * i;
				bufsize = VIP_ALIGN((ictx->src_width*12)/8) * ictx->src_height;
				uartlog("csi_bdg to dram 0x%lx size(%#x)\n", bufaddr, bufsize);
				writel(0x0A071404, bufaddr);
			}
		} else {
			bufaddr = 0x82000000;
			bufsize = VIP_ALIGN((ictx->src_width*2)) * ictx->src_height;
			uartlog("csi_bdg to dram 0x%lx size(%#x)\n", bufaddr, bufsize);
			writel(0x0A071584, bufaddr);
			if (ictx->is_hdr_on) {
				bufaddr = 0x83000000;
				writel(0x0A0715a4, bufaddr);
				uartlog("se csi_bdg to dram 0x%lx size(%#x)\n", bufaddr, bufsize);
			}
		}
		for (i=0; i<SNSR_TEST_CAPTURE_NUM-1; i++) {
			bufaddr = bayer_w_addr[i][0];
			uartlog("csi_bdg to dram 0x%lx size(%#x)\n", bufaddr, bufsize);
			if (ictx->is_hdr_on) {
				bufaddr = bayer_w_addr[i][1];
				uartlog("se csi_bdg to dram 0x%lx size(%#x)\n", bufaddr, bufsize);
			}
		}
		if (test_mode == 5)
			for (i=0; i<4; i++)
				uartlog("[%d] %d error(s) in %d frame\n", i, err_cnt[i], value);
	}

#else
	for (i = 0; i < SNSR_TEST_CAPTURE_NUM; i++) {
		bayer_w_addr[i][0] = 0x83000000 + 0x2000000 * i;
		bayer_w_addr[i][1] = 0x84000000 + 0x2000000 * i;
	}
	while(1) {
		uint64_t    bufaddr;
		uint32_t    bufsize;
		uint32_t    tmp = 0, test_mode = 0, value = 0, value2 = 0;
		int cnt = 0, err_cnt = 0;

		bm_get_num("Leave test(0), GAIN(1), SGAIN(2), SHS(3), SHSS(4), Burning(5), Ref(6), Grp(7):", test_mode);

		if (!test_mode)
			break;

		if (test_mode != 6) {
			bm_get_num("Value:", value);
		} else {
			bm_get_num("shutter:", value);
			bm_get_num("gain:", value2);
		}
		/* capture the 1st default frame. */
		/* Polling Vsync */
		writel(0x0a070000, ictx->is_hdr_on ? 3 : 1);
		writel(0x0a0108e0, 0xFFFF);
		for(;;) {
			if (ictx->is_hdr_on) {
				if ((readl(0x0a0108e0) & 0x202) == 0x202) {
					break;
				}
			} else {
				if ((readl(0x0a0108e0) & 0x2) == 0x2) {
					break;
				}
			}
		}
		writel(0x0a0108e0, 0xFFFF);

		while(cnt<(test_mode==5?value:SNSR_TEST_CAPTURE_NUM)) {

			/* set frame valid*/
			writel(0x0a010028, ictx->is_hdr_on ? 3 : 1);
			if (cnt == 1) {
				/* capture the 1st default frame. */
				/* Polling Vsync */
#if 0
				writel(0x0a070000, 1);
				writel(0x0a001064, 0xFFF);
				for(;;) {
					if (readl(0x0a001064) & 0xC0)
						break;
				}
				writel(0x0a001064, 0xFFF);
#endif
				/* fire the i2c after the 1st frame is
				 * preraw frame done. */
				switch (test_mode) {
				case 1:
					/* set long gain */
					if (sinf->ops && sinf->ops->set_gain)
						sinf->ops->set_gain(sinf, value);
					if (readl(0x0a0108e0) & 0x202)
						uartlog("i2c cmd cross a fs\n");
					break;
				case 2:
					/* set short gain */
					if (sinf->ops && sinf->ops->set_sgain)
						sinf->ops->set_sgain(sinf, value);
					if (readl(0x0a0108e0) & 0x202)
						uartlog("i2c cmd cross a fs\n");
					break;
				case 3:
				case 6:
					/* Set long shutter */
					if (sinf->ops && sinf->ops->set_shsl)
						sinf->ops->set_shsl(sinf, value);
					if (readl(0x0a0108e0) & 0x202)
						uartlog("i2c cmd cross a fs\n");
					break;
				case 4:
					/* Set short shutter */
					if (sinf->ops && sinf->ops->set_shss)
						sinf->ops->set_shss(sinf, value);
					if (readl(0x0a0108e0) & 0x202)
						uartlog("i2c cmd cross a fs\n");
					break;
				case 5:
					uartlog("burning %d frame, start!\n", value);
					break;
				case 7:
					if (sinf->ops && sinf->ops->test_mode)
						sinf->ops->test_mode(sinf, 0);
					break;
				default:
					return 0;
				}
			} else if (test_mode == 6 && cnt == 2) {
				/* set long gain */
				if (sinf->ops && sinf->ops->set_gain)
					sinf->ops->set_gain(sinf, value2);
				if (readl(0x0a0108e0) & 0x202)
					uartlog("i2c cmd cross a fs\n");
			}/* else if (test_mode == 7 && cnt == 3) {
					if (sinf->ops && sinf->ops->test_mode)
						sinf->ops->test_mode(sinf, 1);
			}*/
			/* polling frame done. */
			for(;;) {
				if (readl(0x0a0108e0) & 0xF1F1) {
					uartlog("frm[%d] err 0x%x\n",
							cnt, readl(0x0a0108e0));
					/* Stop if it is not burnin test.*/
					if (test_mode != 5)
						return -1;
					err_cnt++;
					writel(0x0a0108e0, 0xFFFF);
					break;
				}
				if (ictx->is_hdr_on) {
					if ((readl(0x0a070000) & 0x3) == 0x03) {
						writel(0x0a070000, 3);
						break;
					}
				} else {
					if ((readl(0x0a070000) & 0x1) == 0x01) {
						writel(0x0a070000, 1);
						break;
					}
				}
			}
			/* Config the dma */
			if (!ictx->is_yuv_sensor) {
				bufaddr = bayer_w_addr[cnt%SNSR_TEST_CAPTURE_NUM][0];
				writel(0x0A071404, bufaddr);
				if (ictx->is_hdr_on) {
					bufaddr = bayer_w_addr[cnt%SNSR_TEST_CAPTURE_NUM][1];
					writel(0x0A071424, bufaddr);
				}
			} else {
				bufaddr = bayer_w_addr[cnt%SNSR_TEST_CAPTURE_NUM][0];
				writel(0x0A071584, bufaddr);
				if (ictx->is_hdr_on) {
					bufaddr = bayer_w_addr[cnt%SNSR_TEST_CAPTURE_NUM][1];
					writel(0x0A0715a4, bufaddr);
				}
			}
			writel(0x0a0108e0, 0xFFFF);
			cnt++;
		}
		/* restore the dma */
		if (!ictx->is_yuv_sensor) {
			bufaddr = 0x82000000;
			bufsize = VIP_ALIGN((ictx->src_width*12)/8) * ictx->src_height;
			uartlog("ch0 csi_bdg to dram 0x%lx size(%#x)\n", bufaddr, bufsize);
			writel(0x0A071404, bufaddr);
			bufaddr = 0x83000000;
			writel(0x0A071424, bufaddr);
			uartlog("ch1 csi_bdg to dram 0x%lx size(%#x)\n", bufaddr, bufsize);
			bufaddr = 0x84000000;
			writel(0x0A071444, bufaddr);
			uartlog("ch2 csi_bdg to dram 0x%lx size(%#x)\n", bufaddr, bufsize);
			bufaddr = 0x85000000;
			writel(0x0A071464, bufaddr);
			uartlog("ch3 csi_bdg to dram 0x%lx size(%#x)\n", bufaddr, bufsize);
		} else {
			bufaddr = 0x82000000;
			bufsize = VIP_ALIGN((ictx->src_width*2)) * ictx->src_height;
			uartlog("csi_bdg to dram 0x%lx size(%#x)\n", bufaddr, bufsize);
			writel(0x0A071584, bufaddr);
			if (ictx->is_hdr_on) {
				bufaddr = 0x83000000;
				writel(0x0A0715a4, bufaddr);
				uartlog("se csi_bdg to dram 0x%lx size(%#x)\n", bufaddr, bufsize);
			}
		}
		for (cnt=0; cnt<SNSR_TEST_CAPTURE_NUM-1; cnt++) {
			bufaddr = bayer_w_addr[cnt][0];
			uartlog("csi_bdg to dram 0x%lx size(%#x)\n", bufaddr, bufsize);
			if (ictx->is_hdr_on) {
				bufaddr = bayer_w_addr[cnt][1];
				uartlog("se csi_bdg to dram 0x%lx size(%#x)\n", bufaddr, bufsize);
			}
		}
		if (test_mode == 5)
			uartlog("%d error(s) in %d frame\n", err_cnt, value);

	}
#endif
	uartlog("%s done\n", __func__);
	return 0;

}

void config_pinmux_raw0(void)
{
#if 0
	mmio_wr32(PINMUX_BASE + fmux_gpio_REG_IOCTRL_MIPIRX1_PAD0P, 0x5);
	mmio_wr32(PINMUX_BASE + fmux_gpio_REG_IOCTRL_MIPIRX1_PAD0N, 0x5);
	mmio_wr32(PINMUX_BASE + fmux_gpio_REG_IOCTRL_MIPIRX1_PAD1P, 0x5);
	mmio_wr32(PINMUX_BASE + fmux_gpio_REG_IOCTRL_MIPIRX1_PAD1N, 0x5);
	mmio_wr32(PINMUX_BASE + fmux_gpio_REG_IOCTRL_MIPIRX1_PAD2P, 0x5);
	mmio_wr32(PINMUX_BASE + fmux_gpio_REG_IOCTRL_MIPIRX1_PAD2N, 0x5);
	mmio_wr32(PINMUX_BASE + fmux_gpio_REG_IOCTRL_MIPIRX1_PAD3P, 0x5);
	mmio_wr32(PINMUX_BASE + fmux_gpio_REG_IOCTRL_MIPIRX1_PAD3N, 0x5);
	mmio_wr32(PINMUX_BASE + fmux_gpio_REG_IOCTRL_MIPIRX1_PAD4P, 0x5);
	mmio_wr32(PINMUX_BASE + fmux_gpio_REG_IOCTRL_MIPIRX1_PAD4N, 0x5);
	mmio_wr32(PINMUX_BASE + fmux_gpio_REG_IOCTRL_MIPIRX0_PAD4N, 0x1);
	mmio_wr32(PINMUX_BASE + fmux_gpio_REG_IOCTRL_MIPIRX0_PAD4P, 0x1);
	mmio_wr32(PINMUX_BASE + fmux_gpio_REG_IOCTRL_MIPIRX0_PAD3N, 0x1);
	mmio_wr32(PINMUX_BASE + fmux_gpio_REG_IOCTRL_MIPIRX0_PAD3P, 0x1);
	mmio_wr32(PINMUX_BASE + fmux_gpio_REG_IOCTRL_MIPIRX0_PAD2N, 0x1);
	mmio_wr32(PINMUX_BASE + fmux_gpio_REG_IOCTRL_MIPIRX0_PAD2P, 0x1);
	mmio_wr32(PINMUX_BASE + fmux_gpio_REG_IOCTRL_MIPIRX0_PAD1N, 0x1);
	mmio_wr32(PINMUX_BASE + fmux_gpio_REG_IOCTRL_MIPIRX0_PAD1P, 0x1);
	mmio_wr32(PINMUX_BASE + fmux_gpio_REG_IOCTRL_MIPIRX0_PAD0N, 0x1);
	mmio_wr32(PINMUX_BASE + fmux_gpio_REG_IOCTRL_MIPIRX0_PAD0P, 0x1);
	mmio_wr32(PINMUX_BASE + fmux_gpio_REG_IOCTRL_VI_DATA19    , 0x1);
	mmio_wr32(PINMUX_BASE + fmux_gpio_REG_IOCTRL_VI_DATA20    , 0x1);
	mmio_wr32(PINMUX_BASE + fmux_gpio_REG_IOCTRL_VI_DATA21    , 0x1);
	mmio_wr32(PINMUX_BASE + fmux_gpio_REG_IOCTRL_VI_DATA22    , 0x1);
	mmio_wr32(PINMUX_BASE + fmux_gpio_REG_IOCTRL_VI_DATA23    , 0x1);
	mmio_wr32(PINMUX_BASE + fmux_gpio_REG_IOCTRL_VI_DATA24    , 0x1);
#endif
}

int config_csibdg(uint32_t cam_id, uint32_t width, uint32_t height,
		pattern_ctx *ctx, uint8_t hdr, uint8_t yuv_sensor)
{
	struct	    isp_ctx     *ictx = &m_ispctx[0];
	uint32_t    data_per_line;
	uint64_t    bayer_w_addr[4] = {0x82000000, 0x83000000, 0x84000000, 0x85000000};

	// 16-alignment
	data_per_line = (width * 3 / 2 + 15) & (~0xF);
	ictx->is_hdr_on = !!hdr;
	ictx->is_yuv_sensor = !!yuv_sensor;
	switch(hdr) {
	case 1:
		// shadow update
		writel(0x0A070010, 0x30003);
		// csibdg size
		writel(0x0A010810, (height - 1) << 16 | (width - 1));
		writel(0x0A010814, (height - 1) << 16 | (width - 1));
		// config dma
		writel(0x0A071400, 0 << 4);
		writel(0x0A071404, bayer_w_addr[0]);
		writel(0x0A071408, data_per_line);
		writel(0x0A071410, height);
		writel(0x0A071420, 1 << 4);
		writel(0x0A071424, bayer_w_addr[1]);
		writel(0x0A071428, data_per_line);
		writel(0x0A071430, height);
		// csibdg configure
		writel(0x0A010800, 0x111000D1);
		// set frame valid, start streaming
		writel(0x0a010028, 3);

		break;
	case 2:
#if 0
		// shadow update
		writel(0x0A070010, 0x30003);
		// csibdg size
		writel(0x0A010810, (height - 1) << 16 | (width - 1));
		writel(0x0A010814, (height - 1) << 16 | (width - 1));
		// csibdg configure
		writel(0x0A010800, 0x11104011);
		// set frame valid, start streaming
		writel(0x0a010028, 3);
#else
		// shadow update
		writel(0x0A070010, 0x30003);
		// csibdg size
		writel(0x0A010810, (height - 1) << 16 | (width - 1));
		writel(0x0A010814, (height - 1) << 16 | (width - 1));
		// config dma
		writel(0x0A071400, 0 << 4);
		writel(0x0A071404, bayer_w_addr[0]);
		writel(0x0A071408, data_per_line);
		writel(0x0A071410, height);
		writel(0x0A071420, 1 << 4);
		writel(0x0A071424, bayer_w_addr[1]);
		writel(0x0A071428, data_per_line);
		writel(0x0A071430, height);
		//memset((void*)bayer_w_addr[0], 0, data_per_line * height);
		//memset((void*)bayer_w_addr[1], 0, data_per_line * height);
		// csibdg configure
		writel(0x0A010800, 0x111040D1);
		// set frame valid, start streaming
		writel(0x0a010028, 3);
#endif
		break;
	case 4:
		data_per_line = (width * 2 + 15) & (~0xF);
		// RGB stagger + YUV
		// shadow update
		writel(0x0A070010, 0xf000f);
		// csibdg size
		writel(0x0A010810, (height - 1) << 16 | (width - 1));
		writel(0x0A010814, (height - 1) << 16 | (width - 1));
		writel(0x0A010818, (height - 1) << 16 | (width - 1));
		writel(0x0A01081c, (height - 1) << 16 | (width - 1));
		// config dma
		writel(0x0A071400, 0 << 4);
		writel(0x0A071404, bayer_w_addr[0]);
		writel(0x0A071408, data_per_line);
		writel(0x0A071410, height);
		writel(0x0A071420, 1 << 4);
		writel(0x0A071424, bayer_w_addr[1]);
		writel(0x0A071428, data_per_line);
		writel(0x0A071430, height);
		writel(0x0A0714C0, 9 << 4);
		writel(0x0A0714C4, bayer_w_addr[2]);
		writel(0x0A0714C8, data_per_line);
		writel(0x0A0714D0, height);
		writel(0x0A0714E0, 10 << 4);
		writel(0x0A0714E4, bayer_w_addr[3]);
		writel(0x0A0714E8, data_per_line);
		writel(0x0A0714F0, height);
		// csibdg configure
		writel(0x0A010800, 0x111441F5);
		// set frame valid, start streaming
		writel(0x0a010028, 0x0F);

		break;

	case 3:
		data_per_line = (width * 2 + 15) & (~0xF);
		// csibdg lite size
		writel(0x0A073010, (height - 1) << 16 | (width - 1));
		writel(0x0A073014, (height - 1) << 16 | (width - 1));
		// config dma12, dma13
		writel(0x0A071580, 0x70);
		writel(0x0A071584, bayer_w_addr[0]);
		writel(0x0A071588, data_per_line);
		writel(0x0A071590, height);
		writel(0x0A0715A0, 0x80);
		writel(0x0A0715A4, bayer_w_addr[1]);
		writel(0x0A0715A8, data_per_line);
		writel(0x0A0715B0, height);
		// csibdg lite configure
		writel(0x0A073000, 0x110080D1);
		// set frame valid, start streaming
		writel(0x0A07300C, 3);

		break;
	case 0:
	default:
		if (!ictx->is_yuv_sensor) {
			// shadow update
			writel(0x0A070010, 0x10001);
			// csibdg size
			writel(0x0A010810, (height - 1) << 16 | (width - 1));
			// config dma
			writel(0x0A071404, bayer_w_addr[0]);
			writel(0x0A071408, data_per_line);
			writel(0x0A071410, height);
			// csibdg configure
			writel(0x0A010800, 0x11100041);
			// set frame valid, start streaming
			writel(0x0a010028, 1);
		} else {
			data_per_line = (width * 2 + 15) & (~0xF);
			// csibdg lite size
			writel(0x0A073010, (height - 1) << 16 | (width - 1));
			// config dma12, dma13
			writel(0x0A071580, 0x70);
			writel(0x0A071584, bayer_w_addr[0]);
			writel(0x0A071588, data_per_line);
			writel(0x0A071590, height);
			// csibdg lite configure
			writel(0x0A073000, 0x11008041);
			// set frame valid, start streaming
			writel(0x0A07300C, 1);
		}
		break;
	}

	return 0;
}

int list_sensor(void)
{
	int i;

	uartlog("=======================================\n");
	uartlog("            Sensor List\n");
	uartlog("=======================================\n");
	for (i = 0; i < ARRAY_SIZE(pattern_db); ++i) {
		if (pattern_db[i].config_op == NULL)
			break;

		uartlog("%02d. %s\n", i, pattern_db[i].name);
	}
	uartlog("---------------------------------------\n");

	return 0;
}

pattern_ctx* get_sensor_ctx(void)
{
	return &m_ptn_ctx;
}

pattern_ctx* create_sensor_ctx(int case_num)
{
	if ((case_num >= ARRAY_SIZE(pattern_db)) || (pattern_db[case_num].config_op == NULL)) {
		uartlog("%s:%d: error: invalid input %d\n", __func__, __LINE__, case_num);
		return NULL;
	}

	m_ptn_ctx.phys_regs = NULL;
	m_ptn_ctx.mem_base = DRAM_PHY_BASE;
	m_ptn_ctx.vreg_bases = NULL;
	m_ptn_ctx.vreg_bases_pa = NULL;
	m_ptn_ctx.alloc_size = 0;
	m_ptn_ctx.alloc_cnt = 0;

	m_ptn_ctx.ptn = &pattern_db[case_num];

	return &m_ptn_ctx;
}

void destroy_sensor_ctx(pattern_ctx *ctx)
{
	PTN_FREE_ALL(ctx);
}

static int i2cloop(void *priv)
{
	pattern_ctx *ctx = (pattern_ctx *)priv;
	struct sensor_info *sinf = (struct sensor_info *)ctx->ptn->priv;
	struct snr_i2c_adapter *adpt = NULL;
	uint32_t data = 0, wdat = 0, rdat = 0, addr;
	char cmdbuf[8];
	int r, cnt, loop = 1, val;
	unsigned int *reg;

	// i2c config here
	snr_bm_i2c_get_adap((void *)&adpt);
	sinf->adpt = adpt;

#if 1
	uartlog("current slave addr 0x%x\n", sinf->dev);

	cmdbuf[0] = cmdbuf[1] = '0';
	cli_readline("new slave addr? s for scan : ", cmdbuf);
	if (cmdbuf[0] == 's') {
		int devid;
		cli_readline("enter addr : ", cmdbuf);
		addr = _natoh(cmdbuf, 8);
		for (devid = 0x00; devid < 0x80; devid++) {
			uartlog("try read dev[0x%x] at 0x%x ===============\n", devid, addr)
			data = 0;
			r = sinf->adpt->read(sinf->adpt, devid, addr, sinf->alen, (uint8_t*)&(data), 1);
			if (r == 0) {
				sinf->dev = devid;
				uartlog("###########################################\n");
				uartlog("### matched slave addr: 0x%x reg[0x%x]=0x%x\n", devid, addr, data);
				uartlog("###########################################\n");
				break;
			}
		}
	} else {
		val = _natoh(cmdbuf, 2);
		if (val) {
			sinf->dev = val;
			uartlog("updated slave addr: 0x%x\n", sinf->dev);
		}
	}
	adpt->slaveaddr = sinf->dev;
#endif

#if 0
	reg = (unsigned int *)(CONFIG_SYS_I2C_BASE + 0xA0);
	val = readl(reg);
	uartlog("current offset %p: 0x%x\n", reg, val);

	cmdbuf[0] = cmdbuf[1] = '0';
	cli_readline("new offset val? ", cmdbuf);
	val = _natoh(cmdbuf, 2);
	if (val) {
		writel(reg, val);
		uartlog("updated offset %p set 0x%x get 0x%x\n", reg, val, readl(reg));
	}
#else
	(void)reg;
#endif

#if 1
	uartlog("loop count %d:\n", loop);
	cmdbuf[0] = cmdbuf[1] = '0';
	cli_readline("new loop count? ", cmdbuf);
	val = _natoi(cmdbuf, 6);
	if (val) {
		loop = val;
		uartlog("updated loop count %d:\n", loop);
	}
#endif

	snr_bm_i2c_init(adpt);

	do {
		char op;
		cli_readline("### i2c r/w loop; enter e[x]it loop. [w]rite reg. [r]ead reg. : ", cmdbuf);
		op = cmdbuf[0];

		if (op == 'x')
			break;

		cli_readline("enter addr, [g] 0x3008 : ", cmdbuf);
		switch (cmdbuf[0]) {
		case 'g':
			addr = 0x3008;
			break;
		case 'h':
			addr = 0x3020;
			break;
		default:
			addr = _natoh(cmdbuf, 8);
			break;
		}

		for (cnt = 0; cnt < loop; cnt++) {
			switch (op) {
			case 'w':
				cli_readline("enter hex val : ", cmdbuf);
				wdat = _natoh(cmdbuf, 8);
				rdat = 0x00;
				uartlog("dev[0x%x] write 0x%x=0x%x ===============\n", sinf->dev, addr, wdat);
				sinf->adpt->write(sinf->adpt, sinf->dev, addr, sinf->alen, (uint8_t*)&(wdat), sinf->dlen);
				r = sinf->adpt->read(sinf->adpt, sinf->dev, addr, sinf->alen, (uint8_t*)&(rdat), sinf->dlen);
				uartlog("dev[0x%x] read 0x%x %s 0x%x\n", sinf->dev, addr, (r ? "fail" : ":"), rdat);
				break;
			case 'r':
			default:
				uartlog("dev[0x%x] read 0x%x ===============\n", sinf->dev, addr)
				data = 0;
				r = sinf->adpt->read(sinf->adpt, sinf->dev, addr, sinf->alen, (uint8_t*)&(data), sinf->dlen);
				uartlog("dev[0x%x] read 0x%x %s 0x%x\n", sinf->dev, addr, (r ? "fail" : ":"), data);
				break;
			}
		}
	} while (1);

	return 0;
}

#if 0
static int spiloop(void *priv)
{
	pattern_ctx *ctx = (pattern_ctx *)priv;
	struct sensor_info *sinf = (struct sensor_info *)ctx->ptn->priv;
	uint32_t data = 0, wdat = 0, addr;
	char cmdbuf[8];
	int r, cnt, loop = 1, val;

	sinf->spi_adpt = snr_bm_spi_get_adpt(NULL);

	sinf->spi_adpt->init(sinf->spi_adpt, sinf->dev, sinf->alen, 1);

#if 1
	uartlog("current slave addr 0x%x\n", sinf->dev);

	cmdbuf[0] = cmdbuf[1] = '0';
	cli_readline("new slave addr? s for scan : ", cmdbuf);
	if (cmdbuf[0] == 's') {
		int devid;
		cli_readline("enter addr : ", cmdbuf);
		addr = _natoh(cmdbuf, 8);
		for (devid = 0x00; devid < 0x80; devid++) {
			uartlog("try read dev[0x%x] at 0x%x ===============\n", devid, addr)
			data = 0;
			sinf->spi_adpt->slaveaddr = devid;
			r = sinf->spi_adpt->read(sinf->spi_adpt, addr, (uint8_t*)&(data), 1);
			if (r == 0) {
				sinf->dev = devid;
				uartlog("###########################################\n");
				uartlog("### matched slave addr: 0x%x reg[0x%x]=0x%x\n", devid, addr, data);
				uartlog("###########################################\n");
				break;
			}
		}
	} else {
		val = _natoh(cmdbuf, 2);
		if (val) {
			sinf->dev = val;
			uartlog("updated slave addr: 0x%x\n", sinf->dev);
		}
	}
#endif

#if 1
	uartlog("loop count %d:\n", loop);
	cmdbuf[0] = cmdbuf[1] = '0';
	cli_readline("new loop count? ", cmdbuf);
	val = _natoh(cmdbuf, 2);
	if (val) {
		loop = val;
		uartlog("updated loop count %d:\n", loop);
	}
#endif

	do {
		char op;
		cli_readline("### spi r/w loop; enter e[x]it loop. [w]rite reg. [r]ead reg. : ", cmdbuf);
		op = cmdbuf[0];

		if (op == 'x')
			break;

		addr = 0x6;

		switch (op) {
		case 'w':
			cli_readline("enter wr data hex : ", cmdbuf);
			wdat = _natoh(cmdbuf, 8);
			for (cnt = 0; 1/*cnt < loop*/; cnt++) {
				wdat = (cnt >> 8) & 0xFF;
				uartlog("dev[0x%x] wr %d 0x%x=0x%x ===============\n", sinf->dev, cnt, addr, wdat);
				sinf->spi_adpt->write(sinf->spi_adpt, addr, (uint8_t*)&(wdat), 1);

				if (0) {
					cli_readline("enter write data hex : ", cmdbuf);
					wdat = _natoh(cmdbuf, 8);
				}
				udelay(100);
			}
			break;
		case 'r':
		default:
			for (cnt = 0; cnt < loop; cnt++) {
				uartlog("dev[0x%x] rd 0x%x ===============\n", sinf->dev, addr)
				data = 0;
				r = sinf->spi_adpt->read(sinf->spi_adpt, addr, (uint8_t*)&(data), 1);
				uartlog("dev[0x%x] rd 0x%x %s 0x%x\n", sinf->dev, addr, (r ? "fail" : ":"), data);
			}
			break;
		}
	} while (1);

	return 0;
}
#endif

