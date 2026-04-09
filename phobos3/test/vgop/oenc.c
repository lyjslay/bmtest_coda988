#ifdef ENV_CVITEST
#include <common.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include "system_common.h"
#include "timer.h"
#elif defined(ENV_EMU)
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "emu/command.h"
#else
#include <linux/types.h>
#include <linux/delay.h>

#endif  // ENV_CVITEST

#include "vip_common.h"
#include "vip_sys_reg.h"

#include "oenc_reg.h"
#include "reg.h"
#include "oenc.h"

#define DEFAULT_W 1920
#define DEFAULT_H 1080
#define INT_CTRL_MASK 59

/****************************************************************************
 * Global parameters
 ****************************************************************************/

static struct oenc_cfg g_oenc_cfg[MAX_OSD_ENC_INST];
// static uintptr_t reg_base = 0;

/**
 * oenc_intr_clr - clear oenc's interrupt
 *				   check flag
 *
 * @param intr_mask: On/Off ctrl of the interrupt.
 */
void oenc_intr_clr(u8 inst, u32 flag)
{
	// uartlog("(0x4 & (flag<<2):%x\n",(0x4 & (flag<<2)));
	_reg_write(REG_VO_SYS_OENC_INT_GO(inst), (0x8000 & (flag<<15)));
}

/**
 * oenc_intr_status - oenc's interrupt status
 *					  check 'union oenc_intr' for each bit mask
 *
 * @return: The interrupt's status
 */

int oenc_intr_status(u8 inst)
{
	struct oenc_int oenc_int;

	oenc_int.go_intr.raw = _reg_read(REG_VO_SYS_OENC_INT_GO(inst));
    int flag = oenc_int.go_intr.b.done;
	return flag;
}

/****************************************************************************
 * OSD Encoder
 ****************************************************************************/
/**
 * oenc_set_cfg - set compression configurations.
 *
 * @param oenc_cfg: compression's settings.
 */
void oenc_set_cfg(struct oenc_cfg *oenc_cfg, u8 oenc_inst)
{
	if(oenc_inst < MAX_OSD_ENC_INST){
		//reset
		// uartlog("REG_VO_SYS_OENC_RST(%d) is %#x ", oenc_inst, REG_VO_SYS_OENC_RST(oenc_inst));
		_reg_write_mask(REG_VO_SYS_OENC_RST(oenc_inst), BIT(0), 1);
		_reg_write_mask(REG_VO_SYS_OENC_RST(oenc_inst), BIT(0), 0);

		//Compression Format
		//4'b0000: ARGB8888
		//4'b0100: ARGB4444
		//4'b0101: ARGB1555
		//4'b1000: 256LUT-ARGB4444
		//4'b1010: 16-LUT-ARGB4444
		static u8 reg_map_fmt[OENC_GOP_FMT_MAX] = {0, 0x4, 0x5, 0x8, 0xa};

		//reset
		// uartlog("REG_VO_SYS_OENC_RST(%d) is %#x ", oenc_inst, REG_VO_SYS_OENC_RST(oenc_inst));
		_reg_write_mask(REG_VO_SYS_OENC_RST(oenc_inst), BIT(0), 1);
		_reg_write_mask(REG_VO_SYS_OENC_RST(oenc_inst), BIT(0), 0);

		// uartlog("REG_VO_SYS_AXI_EXT_OW is %#x ", oenc_inst, REG_VO_SYS_AXI_EXT_OW);
		// //extern to 36 bit
		// u32 axi_ext_ow = _reg_read(REG_VO_SYS_AXI_EXT_OW);
		// _reg_write(REG_VO_SYS_AXI_EXT_OW, axi_ext_ow | 0xf0);
		// uartlog("REG_VO_SYS_AXI_ADDR_EXT0 is %#x ", oenc_inst, REG_VO_SYS_AXI_ADDR_EXT0);
		// u32 axi_ext0 = _reg_read(REG_VO_SYS_AXI_ADDR_EXT0);
		// _reg_write(REG_VO_SYS_AXI_ADDR_EXT0, axi_ext0 | 0x11110000);

		oenc_cfg[oenc_inst].cfg.b.fmt = reg_map_fmt[oenc_cfg[oenc_inst].fmt];
		oenc_cfg[oenc_inst].cfg.b.intr_en = 1;
		// uartlog("REG_VO_SYS_OENC_CFG(%d) is %#x ", oenc_inst, REG_VO_SYS_OENC_CFG(oenc_inst));
		_reg_write(REG_VO_SYS_OENC_CFG(oenc_inst), oenc_cfg[oenc_inst].cfg.raw);
		_reg_write(REG_VO_SYS_OENC_RANGE(oenc_inst),
				((oenc_cfg[oenc_inst].src_picture_size.h - 1) << 16) | (oenc_cfg[oenc_inst].src_picture_size.w - 1));
		_reg_write(REG_VO_SYS_OENC_PITCH(oenc_inst), oenc_cfg[oenc_inst].src_pitch);
		_reg_write(REG_VO_SYS_OENC_SRC_ADDR(oenc_inst), oenc_cfg[oenc_inst].src_adr);
		_reg_write(REG_VO_SYS_OENC_BSO_ADDR(oenc_inst), oenc_cfg[oenc_inst].bso_adr);

		if (oenc_cfg[oenc_inst].cfg.b.wprot_en) {
			_reg_write(REG_VO_SYS_OENC_WPROT_LADDR(oenc_inst), oenc_cfg[oenc_inst].wprot_laddr);
			_reg_write(REG_VO_SYS_OENC_WPROT_UADDR(oenc_inst), oenc_cfg[oenc_inst].wprot_uaddr);
		}

		if (oenc_cfg[oenc_inst].cfg.b.limit_bsz_en)
			_reg_write(REG_VO_SYS_OENC_LIMIT_BSZ(oenc_inst), oenc_cfg[oenc_inst].limit_bsz);

		//disp_oenc_trig
		_reg_write_mask(REG_VO_SYS_OENC_INT_GO(oenc_inst), BIT(0), 1);
	} else{
		uartlog("[bm-vip][disp] %s: only 0 ~ 1 oenc_inst, no such oenc_inst(%d). ", __func__, oenc_inst);
	}
}

/**
 * disp_oenc_get_cfg - set compression configurations.
 *
 * @return oenc_cfg: compression's settings.
 *
 */
struct oenc_cfg * oenc_get_cfg(u8 oenc_inst)
{
	if(oenc_inst < MAX_OSD_ENC_INST){

		struct oenc_int oenc_trig;

		oenc_trig.go_intr.raw = _reg_read(REG_VO_SYS_OENC_INT_GO(oenc_inst));

		if (oenc_trig.go_intr.b.done)
			uartlog("[bm-vip][disp] DISP OSD Compression done!!\n");

		uartlog("[bm-vip][disp] DISP OSD Compression INTR vector:");
		if (oenc_trig.go_intr.b.intr_vec & BIT(0))
			uartlog("Successful!!\n");
		if (oenc_trig.go_intr.b.intr_vec & BIT(1))
			uartlog("Fail, Bitstream size great than limiter bypass!!\n");
		if (oenc_trig.go_intr.b.intr_vec & BIT(2))
			uartlog("Fail, Bitstream size great than limiter!!\n");
		if (oenc_trig.go_intr.b.intr_vec & BIT(3))
			uartlog("Fail, Watch Dog time-out!!\n");
		if (oenc_trig.go_intr.b.intr_vec & BIT(4))
			uartlog("Fail, Out of Dram Write protection region!!\n");

		g_oenc_cfg[oenc_inst].cfg.raw = _reg_read(REG_VO_SYS_OENC_CFG(oenc_inst));
		g_oenc_cfg[oenc_inst].bso_adr = _reg_read(REG_VO_SYS_OENC_BSO_ADDR(oenc_inst));
		g_oenc_cfg[oenc_inst].bso_sz  = _reg_read(REG_VO_SYS_OENC_BSO_SZ(oenc_inst)) + 64; //count from 0
		// g_oenc_cfg[oenc_inst].bso_sz  = _reg_read(REG_VO_SYS_OENC_BSO_SZ(oenc_inst));
		g_oenc_cfg[oenc_inst].bso_mem_size.w = ALIGN(g_oenc_cfg[oenc_inst].bso_sz, 16) & 0x7fff;
		g_oenc_cfg[oenc_inst].bso_mem_size.h = ALIGN(g_oenc_cfg[oenc_inst].bso_sz, 16) >> 15;
		uartlog("[bm-vip][disp] g_oenc_cfg[oenc_inst].bso_adr:0x%lx\n",g_oenc_cfg[oenc_inst].bso_adr);
		uartlog("[bm-vip][disp] g_oenc_cfg[oenc_inst].bso_sz:%d\n",g_oenc_cfg[oenc_inst].bso_sz);


	} else{
		uartlog("[bm-vip][disp] %s: only 0 ~ 1 oenc_inst, no such oenc_inst(%d). ", __func__, oenc_inst);
		return NULL;
	}

	return g_oenc_cfg;
}

