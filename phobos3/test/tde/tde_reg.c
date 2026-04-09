#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "timer.h"
#include "system_common.h"
#include "bitwise_ops.h"
#include "tde_reg.h"
#include "tde_reg_tab.h"



static uintptr_t reg_base;

uint32_t _reg_read(uintptr_t addr)
{
    return (*(volatile uint32_t *)(addr));
};

void _reg_write(uintptr_t addr, uint32_t data)
{
    (*(volatile uint32_t *)(addr) = (data));
}

void tde_set_base_addr(void *base_addr)
{
	reg_base = (uintptr_t)base_addr;
}

void tde_set_mode(enum tde_mode mode)
{
	u32 value;

	value = BIT(0); //enable
	value |= mode << 16; //mode

	_reg_write(reg_base + TDE_REG_SURFACE_REG0, value);
}

void tde_set_src_cfg(struct tde_surface_mem *cfg)
{
	_reg_write(reg_base + TDE_REG_SRC_SURFACE_ADDR_L, cfg->addr);
	_reg_write(reg_base + TDE_REG_SRC_SURFACE_ADDR_H, cfg->addr >> 32);
	_reg_write(reg_base + TDE_REG_SRC_SURFACE_STRIDE, cfg->stride);
	_reg_write(reg_base + TDE_REG_SRC_SURFACE_WIDTH, cfg->width);
	_reg_write(reg_base + TDE_REG_SRC_SURFACE_HEIGHT, cfg->height);

	_reg_write(reg_base + TDE_REG_SRC_SURFACE_PIXEL_4B, 0);
	_reg_write(reg_base + TDE_REG_SRC_SURFACE_PIXEL_2B, 0);
}

void tde_set_dst_cfg(struct tde_surface_mem *cfg)
{
	_reg_write(reg_base + TDE_REG_DST_SURFACE_ADDR_L, cfg->addr);
	_reg_write(reg_base + TDE_REG_DST_SURFACE_ADDR_H, cfg->addr >> 32);
	_reg_write(reg_base + TDE_REG_DST_SURFACE_STRIDE, cfg->stride);
	_reg_write(reg_base + TDE_REG_DST_SURFACE_WIDTH, cfg->width);
	_reg_write(reg_base + TDE_REG_DST_SURFACE_HEIGHT, cfg->height);
}

void tde_set_draw_line_cfg(struct tde_line_cfg *cfg)
{
	_reg_write(reg_base + TDE_REG_DRAW_LINE_CTRL0, cfg->start_x);
	_reg_write(reg_base + TDE_REG_DRAW_LINE_CTRL1, cfg->start_y);
	_reg_write(reg_base + TDE_REG_DRAW_LINE_CTRL2, cfg->end_x);
	_reg_write(reg_base + TDE_REG_DRAW_LINE_CTRL3, cfg->end_y);
	_reg_write(reg_base + TDE_REG_DRAW_LINE_CTRL4, cfg->color);
	_reg_write(reg_base + TDE_REG_DRAW_LINE_CTRL5, cfg->thick);
}

void tde_start(void)
{
	_reg_write(reg_base + TDE_REG_START_CTRL, 0x1);
}

void tde_clk_gate(bool enable)
{
	u32 value = enable ? 0x1 : 0x0;

	_reg_write(reg_base + TDE_REG_CLK_GATE, value);
}

u32 tde_intr_status(void)
{
	return _reg_read(reg_base + TDE_REG_INTR_STATUS) & 0x1;
}

void tde_clear_intr(void)
{
	_reg_write(reg_base + TDE_REG_INTR_CLEAR, 0x1);
}

void tde_toggle_reset(void)
{
	uintptr_t vi_sys_addr = (uintptr_t)REG_VI_SYS_BASE;
	u32 value = BIT(21);

	_reg_write((uintptr_t)vi_sys_addr, value);
	udelay(20);
	_reg_write((uintptr_t)vi_sys_addr, 0);
}

void tde_dump_reg(void)
{
    u32 reg;

    uartlog("[Print] Print tde Reg configurations ...\n");

    reg = _reg_read(reg_base + TDE_REG_SURFACE_REG0);
    uartlog("[TDE_REG_SURFACE_REG0]:\t0x%08x\n", reg);

	//src
    reg = _reg_read(reg_base + TDE_REG_SRC_SURFACE_ADDR_L);
    uartlog("[TDE_REG_SRC_SURFACE_ADDR_L]:\t0x%08x\n", reg);

    reg = _reg_read(reg_base + TDE_REG_SRC_SURFACE_ADDR_H);
    uartlog("[TDE_REG_SRC_SURFACE_ADDR_H]:\t0x%08x\n", reg);

    reg = _reg_read(reg_base + TDE_REG_SRC_SURFACE_PIXEL_4B);
    uartlog("[TDE_REG_SRC_SURFACE_PIXEL_4B]:\t0x%08x\n", reg);

    reg = _reg_read(reg_base + TDE_REG_SRC_SURFACE_PIXEL_2B);
    uartlog("[TDE_REG_SRC_SURFACE_PIXEL_2B]:\t0x%08x\n", reg);

    reg = _reg_read(reg_base + TDE_REG_SRC_SURFACE_STRIDE);
    uartlog("[TDE_REG_SRC_SURFACE_STRIDE]:\t0x%08x\n", reg);

    reg = _reg_read(reg_base + TDE_REG_SRC_SURFACE_WIDTH);
    uartlog("[TDE_REG_SRC_SURFACE_WIDTH]:\t0x%08x\n", reg);

    reg = _reg_read(reg_base + TDE_REG_SRC_SURFACE_HEIGHT);
    uartlog("[TDE_REG_SRC_SURFACE_HEIGHT]:\t0x%08x\n", reg);

	//dst
    reg = _reg_read(reg_base + TDE_REG_DST_SURFACE_ADDR_L);
    uartlog("[TDE_REG_DST_SURFACE_ADDR_L]:\t0x%08x\n", reg);

    reg = _reg_read(reg_base + TDE_REG_DST_SURFACE_ADDR_H);
    uartlog("[TDE_REG_DST_SURFACE_ADDR_H]:\t0x%08x\n", reg);

    reg = _reg_read(reg_base + TDE_REG_DST_SURFACE_WIDTH);
    uartlog("[TDE_REG_DST_SURFACE_WIDTH]:\t0x%08x\n", reg);

    reg = _reg_read(reg_base + TDE_REG_DST_SURFACE_HEIGHT);
    uartlog("[TDE_REG_DST_SURFACE_HEIGHT]:\t0x%08x\n", reg);

    reg = _reg_read(reg_base + TDE_REG_DST_SURFACE_STRIDE);
    uartlog("[TDE_REG_DST_SURFACE_STRIDE]:\t0x%08x\n", reg);

	//line ctrl
    reg = _reg_read(reg_base + TDE_REG_DRAW_LINE_CTRL0);
    uartlog("[TDE_REG_DRAW_LINE_CTRL0]:\t0x%08x\n", reg);

    reg = _reg_read(reg_base + TDE_REG_DRAW_LINE_CTRL1);
    uartlog("[TDE_REG_DRAW_LINE_CTRL1]:\t0x%08x\n", reg);

    reg = _reg_read(reg_base + TDE_REG_DRAW_LINE_CTRL2);
    uartlog("[TDE_REG_DRAW_LINE_CTRL2]:\t0x%08x\n", reg);

    reg = _reg_read(reg_base + TDE_REG_DRAW_LINE_CTRL3);
    uartlog("[TDE_REG_DRAW_LINE_CTRL3]:\t0x%08x\n", reg);

    reg = _reg_read(reg_base + TDE_REG_DRAW_LINE_CTRL4);
    uartlog("[TDE_REG_DRAW_LINE_CTRL4]:\t0x%08x\n", reg);

    reg = _reg_read(reg_base + TDE_REG_DRAW_LINE_CTRL5);
    uartlog("[TDE_REG_DRAW_LINE_CTRL5]:\t0x%08x\n", reg);

	//other
    reg = _reg_read(reg_base + TDE_REG_SHADOW_RD_SEL);
    uartlog("[TDE_REG_SHADOW_RD_SEL]:\t0x%08x\n", reg);

    reg = _reg_read(reg_base + TDE_REG_DBG_BUS);
    uartlog("[TDE_REG_DBG_BUS]:\t0x%08x\n", reg);

    reg = _reg_read(reg_base + TDE_REG_CLK_GATE);
    uartlog("[TDE_REG_CLK_GATE]:\t0x%08x\n", reg);

    reg = _reg_read(reg_base + TDE_REG_INTR_STATUS);
    uartlog("[TDE_REG_INTR_STATUS]:\t0x%08x\n", reg);
}


