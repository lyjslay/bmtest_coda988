#ifdef ENV_CVITEST
#include <common.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <timer.h>
#include "system_common.h"
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
#include <linux/log2.h>
#endif  // ENV_CVITEST

#include "vip_common.h"
#include "scaler_reg.h"
#include "reg.h"
#include "dsi_phy.h"

/****************************************************************************
 * Global parameters
 ****************************************************************************/
static uintptr_t reg_base;
static u8 data_0_lane;

/****************************************************************************
 * Interfaces
 ****************************************************************************/
void dphy_set_base_addr(void *base)
{
	reg_base = (uintptr_t)base;
}

/**
 * dphy_dsi_lane_en - set dsi-lanes enable control.
 *                    setup before dphy_dsi_init().
 *
 * @param clk_en: clk lane enable
 * @param data_en: data lane[0-3] enable
 * @param preamble_en: preeamble enable
 */
void dphy_dsi_lane_en(bool clk_en, bool *data_en, bool preamble_en)
{
	u8 val = 0, i = 0;

	val |= clk_en;
	for (i = 0; i < 4; ++i)
		val |= (data_en[i] << (i+1));
	if (preamble_en)
		val |= 0x20;
	_reg_write_mask(reg_base + REG_DSI_PHY_EN, 0x3f, val);
}

/**
 * dphy_dsi_set_lane - dsi-lanes control.
 *                     setup before dphy_dsi_lane_en().
 *
 * @param lane_num: lane[0-4].
 * @param lane: the role of this lane.
 * @param pn_swap: if this lane positive/negative swap.
 * @return: 0 for success.
 */
int dphy_dsi_set_lane(u8 lane_num, enum lane_id lane, bool pn_swap)
{
	if ((lane_num > 4) || (lane >= DSI_LANE_MAX))
		return -1;

	_reg_write_mask(reg_base + REG_DSI_PHY_LANE_SEL, 0x7 << (4 * lane_num), lane << (4 * lane_num));
	_reg_write_mask(reg_base + REG_DSI_PHY_LANE_PN_SWAP, BIT(lane_num), pn_swap << lane_num);

	if (lane == DSI_LANE_CLK)
		_reg_write_mask(reg_base + REG_DSI_PHY_LANE_SEL, 0x1f << 24, (1 << 24) << lane_num);
	if (lane == DSI_LANE_0)
		data_0_lane = lane_num;

	return 0;
}

void dphy_lvds_enable(bool en)
{
	_reg_write(reg_base + REG_DSI_PHY_LVDS_EN, en);
}

void dphy_i80_init(void)
{
	_reg_write(reg_base + REG_DSI_PHY_PD, 0x1f1f);
	_reg_write(reg_base + REG_DSI_PHY_ESC_INIT, 0x100);
	_reg_write(reg_base + REG_DSI_PHY_ESC_WAKE, 0x100);

	_reg_write(reg_base + REG_DSI_PHY_EXT_GPIO, 0x000fffff);

	_reg_write(reg_base + REG_DSI_PHY_LVDS_EN, 0x0);

}


/**
 * dphy_dsi_init - dphy init.
 *                 Invoked after dphy_dsi_set_lane() and dphy_dsi_lane_en().
 *
 */
void dphy_dsi_init(void)
{
	_reg_write(reg_base + REG_DSI_PHY_PD, 0x0);
	_reg_write(reg_base + REG_DSI_PHY_ESC_INIT, 0x100);
	_reg_write(reg_base + REG_DSI_PHY_ESC_WAKE, 0x100);

	dphy_lvds_enable(0);
}

void dphy_lvds_init(void)
{
	_reg_write(reg_base + REG_DSI_PHY_PD, 0x1f1f);
	dphy_lvds_enable(1);
}

#ifdef ENV_CVITEST
static s16 ilog2(u32 n)
{
        s16 val = -1;

        if (n == 0)
                return val;
        while (n) {
                ++val;
                n >>= 1;
        }

        return val;
}
#endif

void _cal_pll_reg(u32 clkkHz, u32 VCORx10000, u32 *reg_txpll, u32 *reg_set)
{
	u8 gain = 1 << ilog2(MAX(1, 25000000UL / VCORx10000));
	u32 VCOCx1000 = VCORx10000 * gain / 10;
	u8 reg_disp_div_sel = VCOCx1000 / clkkHz;
	u8 dig_dig = ilog2(gain);
	u8 reg_divout_sel = MIN(3, dig_dig);
	u8 reg_div_sel = dig_dig - reg_divout_sel;
	u8 loop_gain = (((VCOCx1000 / 266000) + 7) >> 3) << 3;
	*reg_set = ((u64)(1200000 * loop_gain) << 26) / VCOCx1000;
	*reg_txpll = (reg_div_sel << 10) | (reg_divout_sel << 8) | reg_disp_div_sel;

#if 1
	uartlog("clkkHz(%d) VCORx10000(%d) gain(%d)\n", clkkHz, VCORx10000, gain);
	uartlog("VCOCx1000(%d) dig_dig(%d) loop_gain(%d)\n", VCOCx1000, dig_dig, loop_gain);
	uartlog("regs: disp_div_sel(%d), divout_sel(%d), set(%#x)\n", reg_disp_div_sel, reg_divout_sel, *reg_set);
#endif
}

void dphy_lvds_set_pll(u32 clkkHz, u8 link)
{
	u32 VCORx10000 = clkkHz * 70 / link;
	u32 reg_txpll, reg_set;

	_cal_pll_reg(clkkHz, VCORx10000, &reg_txpll, &reg_set);

	_reg_write_mask(reg_base + REG_DSI_PHY_TXPLL, 0x7ff, reg_txpll);
	_reg_write(reg_base + REG_DSI_PHY_REG_SET, reg_set);
}

void dphy_dsi_set_pll(u32 clkkHz, u8 lane, u8 bits)
{
	u32 VCORx10000 = clkkHz * bits * 10 / lane;
	u32 reg_txpll, reg_set;

	_cal_pll_reg(clkkHz, VCORx10000, &reg_txpll, &reg_set);

	_reg_write_mask(reg_base + REG_DSI_PHY_TXPLL, 0x7ff, reg_txpll);
	_reg_write(reg_base + REG_DSI_PHY_REG_SET, reg_set);

	// update
	_reg_write_mask(reg_base + REG_DSI_PHY_REG_8C, BIT(0), 0);
	_reg_write_mask(reg_base + REG_DSI_PHY_REG_8C, BIT(0), 1);
}

#define dcs_delay 1

static void _esc_entry(void)
{
	_reg_write(reg_base + REG_DSI_PHY_DATA_OV, 0x01010101 << data_0_lane);
	udelay(dcs_delay);
	_reg_write(reg_base + REG_DSI_PHY_DATA_OV, 0x01010001 << data_0_lane);
	udelay(dcs_delay);
	_reg_write(reg_base + REG_DSI_PHY_DATA_OV, 0x00010001 << data_0_lane);
	udelay(dcs_delay);
	_reg_write(reg_base + REG_DSI_PHY_DATA_OV, 0x00010101 << data_0_lane);
	udelay(dcs_delay);
	_reg_write(reg_base + REG_DSI_PHY_DATA_OV, 0x00010001 << data_0_lane);
	udelay(dcs_delay);
}

static void _esc_exit(void)
{
	_reg_write(reg_base + REG_DSI_PHY_DATA_OV, 0x00010001 << data_0_lane);
	udelay(dcs_delay);
	_reg_write(reg_base + REG_DSI_PHY_DATA_OV, 0x01010001 << data_0_lane);
	udelay(dcs_delay);
	_reg_write(reg_base + REG_DSI_PHY_DATA_OV, 0x01010101 << data_0_lane);
	udelay(dcs_delay);
}

static void _esc_data(u8 data)
{
	u8 i = 0;

	for (i = 0; i < 8; ++i) {
		_reg_write(reg_base + REG_DSI_PHY_DATA_OV,
			   ((data & (1 << i)) ? 0x01010001 : 0x00010101) << data_0_lane);
		udelay(dcs_delay);
		_reg_write(reg_base + REG_DSI_PHY_DATA_OV, 0x00010001 << data_0_lane);
		udelay(dcs_delay);
	}
}

void dpyh_mipi_tx_manual_packet(const u8 *data, u8 count)
{
	u8 i = 0;

	_esc_entry();
	_esc_data(0x87); // LPDT
	for (i = 0; i < count; ++i)
		_esc_data(data[i]);
	_esc_exit();
	_reg_write(reg_base + REG_DSI_PHY_DATA_OV, 0x0);
}
