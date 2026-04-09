#include <string.h>
#ifdef ENV_CVITEST
#include <common.h>
#include <stdbool.h>
#include "system_common.h"
#else
#include <linux/types.h>
#endif

#include "vip_common.h"
#include "vip_sys_reg.h"
#include "reg.h"

#include <string.h>

static uintptr_t reg_base;

void vip_set_base_addr(void *base)
{
	reg_base = (uintptr_t)base;
}

union vip_sys_clk vip_get_clk_lp(void)
{
	union vip_sys_clk clk;

	clk.raw = _reg_read(reg_base + REG_VIP_CLK_LP);
	return clk;
}

void vip_set_clk_lp(union vip_sys_clk clk)
{
	_reg_write(reg_base + REG_VIP_CLK_LP, clk.raw);
}

union vip_sys_clk_ctrl0 vip_get_clk_ctrl0(void)
{
	union vip_sys_clk_ctrl0 cfg;

	cfg.raw = _reg_read(reg_base + REG_VIP_CLK_CTRL0);
	return cfg;
}

void vip_set_clk_ctrl0(union vip_sys_clk_ctrl0 cfg)
{
	_reg_write(reg_base + REG_VIP_CLK_CTRL0, cfg.raw);
}

union vip_sys_reset vip_get_reset(void)
{
	union vip_sys_reset reset;

	reset.raw = _reg_read(reg_base + REG_VIP_RESETS);
	return reset;
}

void vip_set_reset(union vip_sys_reset reset)
{
	_reg_write(reg_base + REG_VIP_RESETS, reset.raw);
	//uartlog("00: %#x\n", reg0.raw);
}

/**
 * vip_toggle_reset - enable/disable reset specified in mask. Lock protected.
 *
 * @param mask: resets want to be toggled.
 */
void vip_toggle_reset(union vip_sys_reset mask)
{
	union vip_sys_reset value;

	value = vip_get_reset();
	value.raw |= mask.raw;
	vip_set_reset(value);
	value.raw &= ~mask.raw;
	vip_set_reset(value);
}

union vip_sys_intr vip_get_intr_status(void)
{
	union vip_sys_intr intr;

	intr.raw = _reg_read(reg_base + REG_VIP_INT_STS);
	return intr;
}

union vip_sys_intr vip_get_intr_mask(void)
{
	union vip_sys_intr intr;

	intr.raw = _reg_read(reg_base + REG_VIP_INT_EN);
	return intr;
}

void vip_set_intr_mask(union vip_sys_intr intr)
{
	_reg_write(reg_base + REG_VIP_INT_EN, intr.raw);
}
