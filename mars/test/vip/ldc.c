#ifdef ENV_CVITEST
#include <common.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "system_common.h"
#include "timer.h"
#elif defined(ENV_EMU)
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "emu/command.h"
#else
#include <linux/types.h>
#include <linux/delay.h>
#endif  // ENV_CVITEST

#include "vip_common.h"
#include "ldc_reg.h"
#include "ldc_cfg.h"
#include "ldc.h"
#include "cmdq.h"
#include "reg.h"
#include "reg_ldc.h"

/****************************************************************************
 * Global parameters
 ***************************************************************************/
static uintptr_t reg_base;

/****************************************************************************
 * Initial info
 ***************************************************************************/

/****************************************************************************
 * Interfaces
 ****************************************************************************/

void ldc_set_base_addr(void *base)
{
	reg_base = (uintptr_t)base;
}

/**
 * ldc_init - setup ldc, mainly interpolation settings.
 *
 * @param cfg: settings for this ldc's operation
 */
void ldc_init(void)
{
}

/**
 * ldc_reset - do reset. This can be activated only if dma stop to avoid hang
 *	       fabric.
 *
 */
void ldc_reset(void)
{
	union vip_sys_reset mask;

	mask.raw = 0;
	mask.b.ldc = 1;
	vip_toggle_reset(mask);
}

/**
 * ldc_intr_ctrl - ldc's interrupt on(1)/off(0)
 *                 bit0: frame_done, bit1: mesh_id axi read err,
 *                 bit2: mesh_table axi read err
 *
 * @param intr_mask: On/Off ctrl of the interrupt.
 */
void ldc_intr_ctrl(u8 intr_mask)
{
	_reg_write(reg_base + REG_LDC_IRQEN, intr_mask);
}

/**
 * ldc_intr_clr - clear ldc's interrupt
 *                 bit0: frame_done, bit1: mesh_id axi read err,
 *                 bit2: mesh_table axi read err
 *
 * @param intr_mask: On/Off ctrl of the interrupt.
 */
void ldc_intr_clr(u8 intr_mask)
{
	_reg_write(reg_base + REG_LDC_IRQCLR, intr_mask);
}

/**
 * ldc_intr_status - ldc's interrupt status
 *                 bit0: frame_done, bit1: mesh_id axi read err,
 *                 bit2: mesh_table axi read err
 *
 * @return: The interrupt's status. 1 if active.
 */
u8 ldc_intr_status(void)
{
	return _reg_read(reg_base + REG_LDC_IRQSTAT);
}

/**
 * ldc_intr_sel - ldc's interrupt selection
 *
 * @param sel
 *      0: ldc_interrupt
 *      1: cmdq interrupt
 */
void ldc_intr_sel(u8 sel)
{
	_reg_write(reg_base + REG_LDC_INT_SEL, sel);
}

/**
 * ldc_check_param - check if config param is valid.
 *
 * @param cfg: settings for this ldc's operation
 * @return: true for valid.
 */
bool ldc_check_param(const struct ldc_cfg *cfg)
{
	if ((cfg->src_width > LDC_MAX_WIDTH) || (cfg->src_height > LDC_MAX_HEIGHT)) {
		return false;
	}
	if ((cfg->src_width & (LDC_SIZE_ALIGN - 1)) ||
	    (cfg->src_height & (LDC_SIZE_ALIGN - 1))) {
		return false;
	}
	if ((cfg->src_xstart > cfg->src_xend) ||
	    (cfg->src_xend - cfg->src_xstart < 32) ||
	    (cfg->src_xstart > cfg->src_width - 1) ||
	    (cfg->src_xend > cfg->src_width - 1)) {
		return false;
	}
	if (cfg->map_base & (LDC_ADDR_ALIGN - 1)) {
		return false;
	}
	if ((cfg->src_y_base & (LDC_ADDR_ALIGN - 1)) ||
	    (cfg->src_c_base & (LDC_ADDR_ALIGN - 1))) {
		return false;
	}
	if ((cfg->dst_y_base & (LDC_ADDR_ALIGN - 1)) ||
	    (cfg->dst_c_base & (LDC_ADDR_ALIGN - 1))) {
		return false;
	}
	return true;
}

/**
 * ldc_engine - start a ldc operation, wait frame_done intr after this.
 *              If output target is scaler, scaler's config should be done
 *              before this.
 *
 * @param cfg: settings for this ldc's operation
 */
void ldc_engine(const struct ldc_cfg *cfg)
{
	u8 ras_mode = (cfg->dst_mode == LDC_DST_FLAT) ? 0 : 1;
	u32 val;
	u32 reg_prob_ti_x = 0;
	u32 reg_prob_ti_y = 1;
	u32 reg_prob_row = 0;
	u32 reg_prob_col = 0;
	u32 reg_prob_en = 1;

	_reg_write(reg_base + REG_LDC_DATA_FORMAT, cfg->pixel_fmt);
	_reg_write(reg_base + REG_LDC_RAS_MODE, ras_mode);
	_reg_write(reg_base + REG_LDC_RAS_XSIZE, cfg->ras_width);
	_reg_write(reg_base + REG_LDC_RAS_YSIZE, cfg->ras_height);

	_reg_write(reg_base + REG_LDC_MAP_BASE, cfg->map_base >> LDC_BASE_ADDR_SHIFT);
	_reg_write(reg_base + REG_LDC_MAP_BYPASS, cfg->map_bypass);

	_reg_write(reg_base + REG_LDC_SRC_BASE_Y, cfg->src_y_base >> LDC_BASE_ADDR_SHIFT);
	_reg_write(reg_base + REG_LDC_SRC_BASE_C, cfg->src_c_base >> LDC_BASE_ADDR_SHIFT);
	_reg_write(reg_base + REG_LDC_SRC_XSIZE, cfg->src_width);
	_reg_write(reg_base + REG_LDC_SRC_YSIZE, cfg->src_height);
	_reg_write(reg_base + REG_LDC_SRC_XSTART, cfg->src_xstart);
	_reg_write(reg_base + REG_LDC_SRC_XEND, cfg->src_xend);
	_reg_write(reg_base + REG_LDC_SRC_BG, cfg->bgcolor);

	_reg_write(reg_base + REG_LDC_DST_BASE_Y, cfg->dst_y_base >> LDC_BASE_ADDR_SHIFT);
	_reg_write(reg_base + REG_LDC_DST_BASE_C, cfg->dst_c_base >> LDC_BASE_ADDR_SHIFT);
	_reg_write(reg_base + REG_LDC_DST_MODE, cfg->dst_mode);

	_reg_write(reg_base + REG_LDC_INT_SEL, 0); // 0: ldc intr, 1: cmd intr

	// Write 0a0c00a8
	// Read  0a0c00ac
	// reg_prob_ti_x = 0
	// reg_prob_ti_y = 1
	// reg_prob_row = 0
	// reg_prob_col = 0
	// reg_prob_en = 1
	val = ((reg_prob_ti_x << LDC_REG_PROB_TI_X_OFFSET) & LDC_REG_PROB_TI_X_MASK)
	    | ((reg_prob_ti_y << LDC_REG_PROB_TI_Y_OFFSET) & LDC_REG_PROB_TI_Y_MASK)
	    | ((reg_prob_row << LDC_REG_PROB_ROW_OFFSET) & LDC_REG_PROB_ROW_MASK)
	    | ((reg_prob_col << LDC_REG_PROB_COL_OFFSET) & LDC_REG_PROB_COL_MASK)
	    | ((reg_prob_en << LDC_REG_PROB_EN_OFFSET) & LDC_REG_PROB_EN_MASK);
	_reg_write(reg_base + REG_LDC_PROB, val);

	val = _reg_read(reg_base + REG_LDC_PROB);
	uartlog("LDC_LDC_PROB=0x%08x\n", val);
	uartlog("  reg_prob_ti_x=%d, reg_prob_ti_y=%d, reg_prob_row=%d, reg_prob_col=%d, reg_prob_en=%d\n",
		(val & LDC_REG_PROB_TI_X_MASK) >> LDC_REG_PROB_TI_X_OFFSET,
		(val & LDC_REG_PROB_TI_Y_MASK) >> LDC_REG_PROB_TI_Y_OFFSET,
		(val & LDC_REG_PROB_ROW_MASK) >> LDC_REG_PROB_ROW_OFFSET,
		(val & LDC_REG_PROB_COL_MASK) >> LDC_REG_PROB_COL_OFFSET,
		(val & LDC_REG_PROB_EN_MASK) >> LDC_REG_PROB_EN_OFFSET);

	// start ldc
	_reg_write(reg_base + REG_LDC_START, 1);

	// There should be a frame_down intr afterwards.
#ifdef ENV_EMU
	for (int i = 0 ; i < 0x400; i += 4) {
		if (i == 0x280)
			i = 0x300;
		else if (i == 0x380)
			break;
		printf("0x%x: 0x%x\n", i,
		       _reg_read(reg_base + i + REG_ldc_BASE));
	}
#endif
}

int ldc_engine_dv(const struct ldc_cfg *cfg)
{
	// Data.Set PM:0a0c0090 %long 0x00000000 ; LDC_CMDQ_IRQ_EN
	_reg_write(reg_base + REG_LDC_INT_SEL, 0); // 0: ldc intr, 1: cmd intr

	// Data.Set PM:0a0c0000 %long 0x00000001 ; FORMAT
	_reg_write(reg_base + REG_LDC_DATA_FORMAT, 1);

	// Data.Set PM:0a0c0004 %long 0x00000001 ; RAS_MODE
	_reg_write(reg_base + REG_LDC_RAS_MODE, 1);

	// Data.Set PM:0a0c0008 %long 0x00000140 ; RAS_XSIZE
	_reg_write(reg_base + REG_LDC_RAS_XSIZE, 0x00000140);

	// Data.Set PM:0a0c000c %long 0x00000100 ; RAS_YSIZE
	_reg_write(reg_base + REG_LDC_RAS_YSIZE, 0x00000100);

	// Data.Set PM:0a0c008c %long 0x00000001 ; LDC_DIR
	_reg_write(reg_base + REG_LDC_DIR, 1);

	// Data.Set PM:0a0c0010 %long 0x02040000 ; MAP_BASE
	_reg_write(reg_base + REG_LDC_MAP_BASE, 0x02040000);

	// Data.Set PM:0a0c0014 %long 0x00000001 ; MAP_BYPASS
	_reg_write(reg_base + REG_LDC_MAP_BYPASS, 1);

	// Data.Set PM:0a0c0020 %long 0x02080000 ; SRC_BASE_Y
	_reg_write(reg_base + REG_LDC_SRC_BASE_Y, cfg->src_y_base >> LDC_BASE_ADDR_SHIFT);

	// Data.Set PM:0a0c0024 %long 0x020c0000 ; SRC_BASE_C
	_reg_write(reg_base + REG_LDC_SRC_BASE_C, cfg->src_c_base >> LDC_BASE_ADDR_SHIFT);

	// Data.Set PM:0a0c0028 %long 0x00000140 ; SRC_XSIZE
	_reg_write(reg_base + REG_LDC_SRC_XSIZE, cfg->src_width);

	// Data.Set PM:0a0c002c %long 0x00000100 ; SRC_Y_SIZE
	_reg_write(reg_base + REG_LDC_SRC_YSIZE, cfg->src_height);

	// Data.Set PM:0a0c0030 %long 0x00000000 ; SRC_XSTR
	_reg_write(reg_base + REG_LDC_SRC_XSTART, cfg->src_xstart);

	// Data.Set PM:0a0c0034 %long 0x000000ff ; SRC_XEND
	_reg_write(reg_base + REG_LDC_SRC_XEND, cfg->src_xend);

	// Data.Set PM:0a0c0038 %long 0x0033f17e ; SRC_BG
	// Data.Set PM:0a0c0040 %long 0x02000000 ; DST_BASE_Y
	// Data.Set PM:0a0c0044 %long 0x02008000 ; DST_BASE_C
	// Data.Set PM:0a0c0048 %long 0x00000003 ; DST_MODE
	// Data.Set PM:0a0c0060 %long 0x00000002 ; SB_MODE
	// Data.Set PM:0a0c0064 %long 0x00000001 ; SB_NB
	// Data.Set PM:0a0c0068 %long 0x00000000 ; SB_SIZE
	// Data.Set PM:0a0c0074 %long 0x00000001 ; SB_SW_CLR
	// Data.Set PM:0a0c0070 %long 0x00000000 ; SB_SW_WPTR
	// Data.Set PM:0a0c006c %long 0x00000001 ; SB_SET_STR
	// Data.Set PM:0a0c0050 %long 0x00000000 ; IRQEN
	// Data.Set PM:0a0c0090 %long 0x00000000 ; LDC_CMDQ_IRQ_EN
	// Data.Set PM:0a0c0054 %long 0x00000001 ; START
	// Data.Set PM:0a0c005c %long 0x00000001 ; IRQCLR

	return 0;
}

/**
 * ldc_is_busy - check if ldc's operation is finished.
 *              ldc_start can only be toggled if only dma done(frame_done intr),
 *              ow dma won't finished.
 */
bool ldc_is_busy(void)
{
	return false;
}

/**
 * ldc_engine_cmdq - start a ldc operation, wait frame_done intr after this.
 *                   If output target is scaler, scaler's config should be done
 *                   before this.
 *
 * @param cfg: settings for cmdq
 * @param cnt: the number of settings
 * @param cmdq_addr: memory-address to put cmdq
 */
void ldc_engine_cmdq(const struct ldc_cfg *cfg, u64 cmdq_addr)
{
	u8 ras_mode = (cfg->dst_mode == LDC_DST_FLAT) ? 0 : 1;
	u8 cmd_idx = 0;
	union cmdq_set *cmd_start = (union cmdq_set *)cmdq_addr;

	cmdQ_set_package(&cmd_start[cmd_idx++].reg, reg_base + REG_LDC_DATA_FORMAT, cfg->pixel_fmt);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, reg_base + REG_LDC_RAS_MODE, ras_mode);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, reg_base + REG_LDC_RAS_XSIZE, cfg->ras_width);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, reg_base + REG_LDC_RAS_YSIZE, cfg->ras_height);

	cmdQ_set_package(&cmd_start[cmd_idx++].reg, reg_base + REG_LDC_MAP_BASE, cfg->map_base >> LDC_BASE_ADDR_SHIFT);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, reg_base + REG_LDC_MAP_BYPASS, cfg->map_bypass);

	cmdQ_set_package(&cmd_start[cmd_idx++].reg, reg_base + REG_LDC_SRC_BASE_Y, cfg->src_y_base >> LDC_BASE_ADDR_SHIFT);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, reg_base + REG_LDC_SRC_BASE_C, cfg->src_c_base >> LDC_BASE_ADDR_SHIFT);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, reg_base + REG_LDC_SRC_XSIZE, cfg->src_width);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, reg_base + REG_LDC_SRC_YSIZE, cfg->src_height);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, reg_base + REG_LDC_SRC_XSTART, cfg->src_xstart);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, reg_base + REG_LDC_SRC_XEND, cfg->src_xend);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, reg_base + REG_LDC_SRC_BG, cfg->bgcolor);

	cmdQ_set_package(&cmd_start[cmd_idx++].reg, reg_base + REG_LDC_DST_BASE_Y, cfg->dst_y_base >> LDC_BASE_ADDR_SHIFT);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, reg_base + REG_LDC_DST_BASE_C, cfg->dst_c_base >> LDC_BASE_ADDR_SHIFT);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, reg_base + REG_LDC_DST_MODE, cfg->dst_mode);

	// Enable cmdq interrupt
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, reg_base + REG_LDC_INT_SEL,
			 1); // 0: ldc intr, 1: cmd intr
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, reg_base + REG_LDC_IRQEN,
			 1);

	// start ldc
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, reg_base + REG_LDC_START, 1);

#if 0
	s// wait end-inter(1)
	u32 flag_num = 1;   // 0: sc_str_flag, sc_stp_flag
	cmdQ_set_wait(&cmd_start[cmd_idx++], /*is_timer*/false, flag_num, 0);

	// clear intr
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, REG_LDC_IRQCLR, 1);
#endif

	cmd_start[cmd_idx-1].reg.intr_end = 1;
	cmd_start[cmd_idx-1].reg.intr_last = 1;

	cmdQ_intr_ctrl(REG_LDC_CMDQ_BASE, 0x02);
	cmdQ_engine(REG_LDC_CMDQ_BASE, (uintptr_t)cmdq_addr,
		    REG_LDC_TOP_BASE >> 22, true, false, cmd_idx);
}

void ldc_get_sb_default(struct ldc_sb_cfg *cfg)
{
	memset(cfg, 0, sizeof(*cfg));
}

void ldc_set_sb(struct ldc_sb_cfg *cfg)
{
	_reg_write(reg_base + REG_SB_MODE,    cfg->sb_mode);
	_reg_write(reg_base + REG_SB_NB,      cfg->sb_nb);
	_reg_write(reg_base + REG_SB_SIZE,    cfg->sb_size);
	_reg_write(reg_base + REG_SB_SW_CLR,  1);
	_reg_write(reg_base + REG_SB_SW_WPTR, cfg->sb_sw_wptr);
	_reg_write(reg_base + REG_SB_SET_STR, cfg->sb_set_str);

	_reg_write(reg_base + REG_LDC_DIR, cfg->ldc_dir);
}

void ldc_dump_register(void)
{
	u32 val;
	u32 ldc_base = reg_base + REG_LDC_REG_BASE;

	uartlog("ldc base address=0x%08x\n", ldc_base);
	uartlog("LDC_FORMAT=0x%08x\n", _reg_read(ldc_base + LDC_FORMAT));
	uartlog("LDC_RAS_MODE=0x%08x\n", _reg_read(ldc_base + LDC_RAS_MODE));
	uartlog("LDC_RAS_XSIZE=0x%08x\n", _reg_read(ldc_base + LDC_RAS_XSIZE));
	uartlog("LDC_RAS_YSIZE=0x%08x\n", _reg_read(ldc_base + LDC_RAS_YSIZE));

	val = _reg_read(ldc_base + LDC_MAP_BASE);
	uartlog("LDC_MAP_BASE=0x%08x\n", val);
	uartlog("    addr=0x%08x\n", val << LDC_BASE_ADDR_SHIFT);

	uartlog("LDC_MAP_BYPASS=0x%08x\n", _reg_read(ldc_base + LDC_MAP_BYPASS));

	val = _reg_read(ldc_base + LDC_SRC_BASE_Y);
	uartlog("LDC_SRC_BASE_Y=0x%08x\n", val);
	uartlog("    addr=0x%08x\n", val << LDC_BASE_ADDR_SHIFT);

	val = _reg_read(ldc_base + LDC_SRC_BASE_C);
	uartlog("LDC_SRC_BASE_C=0x%08x\n", val);
	uartlog("    addr=0x%08x\n", val << LDC_BASE_ADDR_SHIFT);

	uartlog("LDC_SRC_XSIZE=0x%08x\n", _reg_read(ldc_base + LDC_SRC_XSIZE));
	uartlog("LDC_SRC_YSIZE=0x%08x\n", _reg_read(ldc_base + LDC_SRC_YSIZE));
	uartlog("LDC_SRC_XSTR=0x%08x\n", _reg_read(ldc_base + LDC_SRC_XSTR));
	uartlog("LDC_SRC_XEND=0x%08x\n", _reg_read(ldc_base + LDC_SRC_XEND));
	uartlog("LDC_SRC_BG=0x%08x\n", _reg_read(ldc_base + LDC_SRC_BG));

	val = _reg_read(ldc_base + LDC_DST_BASE_Y);
	uartlog("LDC_DST_BASE_Y=0x%08x\n", val);
	uartlog("    addr=0x%08x\n", val << LDC_BASE_ADDR_SHIFT);

	val = _reg_read(ldc_base + LDC_DST_BASE_C);
	uartlog("LDC_DST_BASE_C=0x%08x\n", val);
	uartlog("    addr=0x%08x\n", val << LDC_BASE_ADDR_SHIFT);

	uartlog("LDC_DST_MODE=0x%08x\n", _reg_read(ldc_base + LDC_DST_MODE));
	uartlog("LDC_IRQEN=0x%08x\n", _reg_read(ldc_base + LDC_IRQEN));
	uartlog("LDC_START=0x%08x\n", _reg_read(ldc_base + LDC_START));
	uartlog("LDC_IRQSTAT=0x%08x\n", _reg_read(ldc_base + LDC_IRQSTAT));
	uartlog("LDC_IRQCLR=0x%08x\n", _reg_read(ldc_base + LDC_IRQCLR));
	uartlog("LDC_SB_MODE=0x%08x\n", _reg_read(ldc_base + LDC_SB_MODE));
	uartlog("LDC_SB_NB=0x%08x\n", _reg_read(ldc_base + LDC_SB_NB));
	uartlog("LDC_SB_SIZE=0x%08x\n", _reg_read(ldc_base + LDC_SB_SIZE));
	uartlog("LDC_SB_SET_STR=0x%08x\n", _reg_read(ldc_base + LDC_SB_SET_STR));
	uartlog("LDC_SB_SW_WPTR=0x%08x\n", _reg_read(ldc_base + LDC_SB_SW_WPTR));
	uartlog("LDC_SB_SW_CLR=0x%08x\n", _reg_read(ldc_base + LDC_SB_SW_CLR));
	uartlog("LDC_SB_FULL=0x%08x\n", _reg_read(ldc_base + LDC_SB_FULL));
	uartlog("LDC_SB_EMPTY=0x%08x\n", _reg_read(ldc_base + LDC_SB_EMPTY));
	uartlog("LDC_SB_WPTR_RO=0x%08x\n", _reg_read(ldc_base + LDC_SB_WPTR_RO));
	uartlog("LDC_SB_DPTR_RO=0x%08x\n", _reg_read(ldc_base + LDC_SB_DPTR_RO));
	uartlog("LDC_SB_STRIDE=0x%08x\n", _reg_read(ldc_base + LDC_SB_STRIDE));
	uartlog("LDC_LDC_DIR=0x%08x\n", _reg_read(ldc_base + LDC_LDC_DIR));
	uartlog("LDC_CMDQ_IRQ_EN=0x%08x\n", _reg_read(ldc_base + LDC_REG_CMDQ_IRQ_EN));
	uartlog("LDC_FORCE_IN_RANGE=0x%08x\n", _reg_read(ldc_base + LDC_LDC_FORCE_IN_RANGE));
	uartlog("LDC_OUT_RANGE=0x%08x\n", _reg_read(ldc_base + LDC_LDC_OUT_RANGE));
	uartlog("LDC_OUT_RANGE_DST_X=0x%08x\n", _reg_read(ldc_base + LDC_LDC_OUT_RANGE_DST_X));
	uartlog("LDC_OUT_RANGE_DST_Y=0x%08x\n", _reg_read(ldc_base + LDC_LDC_OUT_RANGE_DST_Y));
	uartlog("LDC_OUT_RANGE_SRC_X=0x%08x\n", _reg_read(ldc_base + LDC_LDC_OUT_RANGE_SRC_X));
	uartlog("LDC_OUT_RANGE_SRC_Y=0x%08x\n", _reg_read(ldc_base + LDC_LDC_OUT_RANGE_SRC_Y));
	uartlog("LDC_DST_TI_CNT_X=0x%08x\n", _reg_read(ldc_base + LDC_LDC_DST_TI_CNT_X));
	uartlog("LDC_DST_TI_CNT_Y=0x%08x\n", _reg_read(ldc_base + LDC_LDC_DST_TI_CNT_Y));

	val = _reg_read(ldc_base + LDC_LDC_PROB_TI_X);
	uartlog("LDC_LDC_PROB=0x%08x\n", val);
	uartlog("  reg_prob_ti_x=%d, reg_prob_ti_y=%d, reg_prob_row=%d, reg_prob_col=%d, reg_prob_en=%d\n",
		(val & LDC_REG_PROB_TI_X_MASK) >> LDC_REG_PROB_TI_X_OFFSET,
		(val & LDC_REG_PROB_TI_Y_MASK) >> LDC_REG_PROB_TI_Y_OFFSET,
		(val & LDC_REG_PROB_ROW_MASK) >> LDC_REG_PROB_ROW_OFFSET,
		(val & LDC_REG_PROB_COL_MASK) >> LDC_REG_PROB_COL_OFFSET,
		(val & LDC_REG_PROB_EN_MASK) >> LDC_REG_PROB_EN_OFFSET);

	uartlog("LDC_LDC_PROB_DATA=0x%08x\n", _reg_read(ldc_base + LDC_LDC_PROB_DATA));
}

void ldc_dump_cmdq(u64 cmdq_addr, u32 num_cmd)
{
	u32 i;
	union cmdq_set *cmd_start = (union cmdq_set *)cmdq_addr;


	uartlog("cmdq addr=0x%08x, num=%d\n", cmdq_addr, num_cmd);
	for (i = 0; i < num_cmd; i++) {
		uartlog("[%02d] [0x%08x]=0x%08x\n",
			i, cmd_start[i].reg.addr << 2,
			cmd_start[i].reg.data);
	}
}
