/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "mmio.h"
#include "bm_sdhci.h"
#include "mmc.h"
#include "system_common.h"
#include "debug.h"

static void bm_mmc_hw_init(struct mmc_host *host);
static int bm_mmc_send_cmd(struct mmc_host *host, mmc_cmd_t *cmd);
static int bm_mmc_read(int lba, uintptr_t buf, size_t size);
static int bm_mmc_write(int lba, uintptr_t buf, size_t size);
static int bm_mmc_start_signal_voltage_switch(struct mmc_host *mmc, struct mmc_ios *ios);
static int bm_mmc_card_busy(struct mmc_host *mmc);
static int bm_mmc_set_ios(struct mmc_host *host, struct mmc_ios *ios);
static int bm_mmc_select_drive_strength(struct mmc_card *card,
				       unsigned int max_dtr, int host_drv,
				       int card_drv, int *drv_type);
static int bm_mmc_execute_tuning(struct mmc_host *host, u32 opcode);

struct mmc_ops common_ops = {
	.init           = bm_mmc_hw_init,
	.send_cmd       = bm_mmc_send_cmd,
	.set_ios        = bm_mmc_set_ios,
	.read           = bm_mmc_read,
	.write          = bm_mmc_write,
	.execute_tuning = bm_mmc_execute_tuning,
	.start_signal_voltage_switch = bm_mmc_start_signal_voltage_switch,
	.select_drive_strength = bm_mmc_select_drive_strength,
	.card_busy		= bm_mmc_card_busy,
};

static void bm_sdhci_adma_write_cmd_desc(struct mmc_host *host, void *desc,
				u32 reg, unsigned int attr)
{
	struct sdhci_adma3_cmd_desc *cmd_desc = desc;

	cmd_desc->attr = attr;
	cmd_desc->reg = reg;
	cmd_desc->reserved = 0;
	cmd_desc->reserved2 = 0;
}

static void bm_sdhci_desc_reg_set(struct mmc_host *host, int reg, u32 value)
{
	struct sdhci_adma3_cmd_desc *cmd_desc = host->adma_table;

	if (SDHCI_DMA_ADDRESS == reg) {
		cmd_desc[0].reg = value;
	} else if (SDHCI_BLOCK_SIZE == reg) {
		cmd_desc[1].reg |= value & 0xffff;
	} else if (SDHCI_BLOCK_COUNT == reg) {
		cmd_desc[1].reg |= (value & 0xffff) << 16;
	} else if (SDHCI_ARGUMENT == reg) {
		cmd_desc[2].reg = value;
	} else if (SDHCI_TRANSFER_MODE == reg) {
		cmd_desc[3].reg |=  value & 0xffff;
	} else if (SDHCI_COMMAND == reg) {
		cmd_desc[3].reg |= (value & 0xffff) << 16;
	}
}

static void bm_sdhci_adma_write_desc(struct mmc_host *host, void *desc,
				uint64_t addr, unsigned int len, unsigned int attr)
{
	struct bm_mmc_params *param = host->priv;
	uintptr_t base = param->reg_base;
	struct sdhci_adma2_64_desc *adma_desc = desc;

	adma_desc->attr = attr;
	if (mmio_read_16(base + SDHCI_HOST_CONTROL2) & SDHCI_HOST_ADMA2_LEN_MODE) {
		adma_desc->len_10 = len;
		adma_desc->len_16 = len >> 10;
	} else {
		adma_desc->len_16 = len;
	}

	adma_desc->addr = addr;
}

static void bm_sdhci_cmd_desc_init(struct mmc_host *host)
{
	void *desc = host->adma_table;

	bm_sdhci_adma_write_cmd_desc(host, desc, 0, ADMA3_CMD_VALID);
	desc += 8;
	bm_sdhci_adma_write_cmd_desc(host, desc, 0, ADMA3_CMD_VALID);
	desc += 8;
	bm_sdhci_adma_write_cmd_desc(host, desc, 0, ADMA3_CMD_VALID);
	desc += 8;
	bm_sdhci_adma_write_cmd_desc(host, desc, 0, ADMA3_CMD_VALID);
}

#define min(x, y) ((x) > (y) ? (y) : (x))
static void bm_sdhci_adma_table_pre(struct mmc_host *host, struct mmc_data *data)
{
	u32 remain_size, buf_size;
	void *desc;
	uint64_t addr;

	remain_size = data->blksz * data->blocks;
	addr = (uint64_t)data->buf;
	desc = host->adma_table;
	if (host->flags & SDHCI_USE_ADMA3)
		desc += 32;

	while (remain_size) {
		buf_size = min(host->adma_buf_size, remain_size);
		bm_sdhci_adma_write_desc(host, desc, addr, buf_size, ADMA2_TRAN_VALID);

		addr += buf_size;
		desc += host->desc_sz;
		remain_size -= buf_size;

		if ((desc - host->adma_table) >= host->adma_table_sz) {
			printf("Exceed adma table capacity!!!\n");
			assert(0);
		}
	}

	bm_sdhci_adma_write_desc(host, desc, 0, 0, ADMA2_NOP_END_VALID);
}

static int bm_mmc_prepare_data(struct mmc_host *host, mmc_cmd_t *cmd)
{
	struct bm_mmc_params *param = host->priv;
	uintptr_t base = param->reg_base;
	struct mmc_data *data = cmd->data;
	uint64_t load_addr;
	u32 block_cnt, blksz;
	uint8_t tmp;

	assert(((uint64_t)data->buf & SD_BLOCK_MASK) == 0);

	blksz = data->blksz;
	block_cnt = data->blocks;

	if (host->flags & (SDHCI_USE_ADMA2 | SDHCI_USE_ADMA3)) {
		bm_sdhci_adma_table_pre(host, data);

		if (host->flags & SDHCI_USE_ADMA3) {
			bm_sdhci_cmd_desc_init(host);
			bm_sdhci_desc_reg_set(host, SDHCI_DMA_ADDRESS, block_cnt);
			bm_sdhci_desc_reg_set(host, SDHCI_BLOCK_COUNT, 0);
			bm_sdhci_desc_reg_set(host, SDHCI_BLOCK_SIZE, SDHCI_MAKE_BLKSZ(7, blksz));

			tmp = mmio_read_8(base + SDHCI_HOST_CONTROL);
			tmp &= ~SDHCI_CTRL_DMA_MASK;
			tmp |= SDHCI_CTRL_ADMA3;
			mmio_write_8(base + SDHCI_HOST_CONTROL, tmp);

			return 0;
		}

		load_addr = (uint64_t)host->adma_table;
	} else {
		load_addr = (uint64_t)data->buf;
	}

	if (mmio_read_16(base + SDHCI_HOST_CONTROL2) & SDHCI_HOST_VER4_ENABLE) {
		if (host->flags & SDHCI_REQ_USE_DMA) {
			mmio_write_32(base + SDHCI_ADMA_SA_LOW, load_addr);
			mmio_write_32(base + SDHCI_ADMA_SA_HIGH, (load_addr >> 32));
		}
		mmio_write_32(base + SDHCI_DMA_ADDRESS, block_cnt);
		mmio_write_16(base + SDHCI_BLOCK_COUNT, 0);
	} else {
		assert((load_addr >> 32) == 0);

		if (host->flags & SDHCI_REQ_USE_DMA) {
			mmio_write_32(base + SDHCI_DMA_ADDRESS, load_addr);
		}
		mmio_write_16(base + SDHCI_BLOCK_COUNT, block_cnt);
	}

	mmio_write_16(base + SDHCI_BLOCK_SIZE, SDHCI_MAKE_BLKSZ(7, blksz));

	// select SDMA
	tmp = mmio_read_8(base + SDHCI_HOST_CONTROL);
	tmp &= ~SDHCI_CTRL_DMA_MASK;

	if (host->flags & SDHCI_REQ_USE_DMA) {
		if (host->flags & SDHCI_USE_ADMA2)
			tmp |= SDHCI_CTRL_ADMA2;
		else
			tmp |= SDHCI_CTRL_SDMA;
	}
	mmio_write_8(base + SDHCI_HOST_CONTROL, tmp);

	return 0;
}

static void bm_sdhci_read_block_pio(struct mmc_host *host)
{
	struct bm_mmc_params *param = host->priv;
	uintptr_t base = param->reg_base;
	size_t blksize, chunk;
	u32 scratch;
	uint8_t *buf;
	u32 block_cnt;

	pr_debug("PIO reading\n");

	blksize = host->data->blksz;
	block_cnt = host->data->blocks;

	buf = (uint8_t *)((uint64_t)host->data->buf + ((block_cnt - 1) * blksize));
	chunk = 0;
	scratch = 0;

	while (blksize) {
		if (chunk == 0) {
			scratch = mmio_read_32(base + SDHCI_BUFFER);
			chunk = 4;
		}

		*buf = scratch & 0xFF;

		buf++;
		scratch >>= 8;
		chunk--;
		blksize--;
	}
}

static void bm_sdhci_write_block_pio(struct mmc_host *host)
{
	struct bm_mmc_params *param = host->priv;
	uintptr_t base = param->reg_base;
	size_t blksize, chunk;
	u32 scratch;
	uint8_t *buf;
	u32 block_cnt;

	pr_debug("PIO writing\n");

	blksize = host->data->blksz;
	block_cnt = host->data->blocks;

	buf = (uint8_t *)((uint64_t)host->data->buf + ((block_cnt - 1) * blksize));
	chunk = 0;
	scratch = 0;

	while (blksize) {
		scratch |= (u32)*buf << (chunk * 8);

		buf++;
		chunk++;
		blksize--;

		if ((chunk == 4) || (blksize == 0)) {
			mmio_write_32(base + SDHCI_BUFFER, scratch);
			chunk = 0;
			scratch = 0;
		}
	}
}

static void bm_sdhci_transfer_pio(struct mmc_host *host)
{
	struct bm_mmc_params *param = host->priv;
	uintptr_t base = param->reg_base;
	u32 mask;

	if (host->data->blocks == 0)
		return;

	if (host->data->flags & MMC_DATA_READ)
		mask = SDHCI_DATA_AVAILABLE;
	else
		mask = SDHCI_SPACE_AVAILABLE;

	while (mmio_read_32(base + SDHCI_PRESENT_STATE) & mask) {
		if (host->data->flags & MMC_DATA_READ)
			bm_sdhci_read_block_pio(host);
		else
			bm_sdhci_write_block_pio(host);

		host->data->blocks--;
		if (host->data->blocks == 0)
			break;
	}

	pr_debug("PIO transfer complete.\n");
}

static int bm_mmc_check_cmd_stats(struct mmc_host *host)
{
	int i;
	volatile mmc_cmd_t *cmd = host->cmd;
	struct bm_mmc_params *param = host->priv;
	uintptr_t base = param->reg_base;
	u32 stat;

	// check cmd complete if necessary
	if ((mmio_read_16(base + SDHCI_TRANSFER_MODE)
				& SDHCI_TRNS_RESP_INT) == 0) {
		while (1) {
			stat = mmio_read_16(base + SDHCI_INT_STATUS);
			if (stat & SDHCI_INT_ERROR)
			{
				printf("Status 30 : 0x:%x\n", mmio_read_16(base + SDHCI_INT_STATUS));
				printf("Error status32: 0x:%x\n", mmio_read_16(base + SDHCI_ERR_INT_STATUS));
				return -EIO;
			}
			if (stat & SDHCI_INT_RESPONSE) {
				mmio_write_16(base + SDHCI_INT_STATUS, SDHCI_INT_RESPONSE);
				break;
			}
		}

		if (cmd->resp_type & MMC_RSP_PRESENT) {
			if (cmd->resp_type & MMC_RSP_136) {
				/* CRC is stripped so we need to do some shifting. */
				for (i = 0; i < 4; i++) {
					cmd->resp_data[i] = mmio_read_32(base + SDHCI_RESPONSE_01 + (3-i)*4) << 8;
					if (i != 3)
						cmd->resp_data[i] |= mmio_read_8(base + SDHCI_RESPONSE_01 + (3-i)*4-1);
				}
			} else {
				cmd->resp_data[0] = mmio_read_32(base + SDHCI_RESPONSE_01);
			}
		}
	}

	return 0;
}

static void bm_mmc_dump_int_stats(struct mmc_host *host)
{
	struct bm_mmc_params *param = host->priv;
	uintptr_t base = param->reg_base;
	uint32_t reg = SDHCI_INT_STATUS;

	for (; reg <= SDHCI_ERROR_SIGNAL_ENABLE; reg += 2) {
		printf("reg%x : 0x:%x\n", reg, mmio_read_16(base + reg));
	}

	return;
}

static int bm_mmc_check_data_stats(struct mmc_host *host)
{
	struct bm_mmc_params *param = host->priv;
	uintptr_t base = param->reg_base;
	u32 stat, dma_addr;

	pr_debug("%s\n", __func__);

	if (host->flags & SDHCI_REQ_USE_DMA) {
		pr_debug("%s SDHCI_REQ_USE_DMA\n", __func__);

		// check dma/transfer complete
		while (1) {
			stat = mmio_read_16(base + SDHCI_INT_STATUS);
			if (stat & SDHCI_INT_ERROR)
			{
				printf("Status2 30 : 0x:%x\n", mmio_read_16(base + SDHCI_INT_STATUS));
				bm_mmc_dump_int_stats(host);
				return -EIO;
			}

			if (stat & SDHCI_INT_DATA_END) {
				pr_debug("%s SDHCI_INT_STATUS\n", __func__);
				mmio_write_16(base + SDHCI_INT_STATUS, stat);
				break;
			}

			if ((host->flags & SDHCI_USE_SDMA) && (stat & SDHCI_INT_DMA_END)) {
				mmio_write_16(base + SDHCI_INT_STATUS, stat);
				if (mmio_read_16(base + SDHCI_HOST_CONTROL2) & SDHCI_HOST_VER4_ENABLE) {
					dma_addr = mmio_read_32(base + SDHCI_ADMA_SA_LOW);
					mmio_write_32(base + SDHCI_ADMA_SA_LOW, dma_addr);
					mmio_write_32(base + SDHCI_ADMA_SA_HIGH, 0);
				} else {
					dma_addr = mmio_read_32(base + SDHCI_DMA_ADDRESS);
					mmio_write_32(base + SDHCI_DMA_ADDRESS, dma_addr);
				}
			}
		}
	} else {
		pr_debug("%s no use DMA\n", __func__);
		while (1) {

			stat = mmio_read_16(base + SDHCI_INT_STATUS);
			if (stat & (SDHCI_INT_DATA_AVAIL | SDHCI_INT_SPACE_AVAIL)) {
				mmio_write_16(base + SDHCI_INT_STATUS, SDHCI_INT_DATA_AVAIL | SDHCI_INT_SPACE_AVAIL);
				bm_sdhci_transfer_pio(host);
			}

			if (stat & SDHCI_INT_ERROR)
			{
				printf("Status3 30 : 0x:%x\n", mmio_read_16(base + SDHCI_INT_STATUS));
				return -EIO;
			}

			if (stat & SDHCI_INT_DATA_END) {
				mmio_write_16(base + SDHCI_INT_STATUS, stat);
				break;
			}

		}
	}


	return 0;
}

static void bm_sdhci_host_init_for_irq(struct mmc_host *host)
{
	host->cmd_completion = 0;
	host->data_completion = 0;
	host->cmd->error = 0;

	if (host->data) {
		host->data->error = 0;
	}
}

static void bm_sdhci_wait_cmd_complete(struct mmc_host *host)
{
	while (!host->cmd_completion) {
		pr_debug("%s\n", __func__);
	}
	host->cmd_completion = 0;
}

static void bm_sdhci_wait_data_complete(struct mmc_host *host)
{
	while (!host->data_completion) {
		pr_debug("%s\n", __func__);
	}
	host->data_completion = 0;
}

static int bm_mmc_send_data_cmd(struct mmc_host *host, mmc_cmd_t *cmd)
{
	struct bm_mmc_params *param = host->priv;
	uintptr_t base = param->reg_base;
	struct mmc_data *data = cmd->data;
	u32 mode = 0, flags = 0;

	pr_debug("%s: cmd->cmd_idx %d, host->flags 0x%x\n", __func__, cmd->cmd_idx, host->flags);

	//make sure cmd line is clear
	while (1) {
		if (!(mmio_read_32(base + SDHCI_PRESENT_STATE) & SDHCI_CMD_INHIBIT))
			break;
	}

	bm_mmc_prepare_data(host, cmd);

	if (host->flags & SDHCI_REQ_USE_DMA)
		mode = SDHCI_TRNS_DMA;

	if (mmc_op_multi(cmd->cmd_idx) || data->blocks > 1) {
		mode |= SDHCI_TRNS_MULTI | SDHCI_TRNS_BLK_CNT_EN;
	}

	if (data->flags & MMC_DATA_READ) {
		mode |= SDHCI_TRNS_READ;
	} else {
		mode &= ~SDHCI_TRNS_READ;
	}

	// set cmd flags
	if (cmd->cmd_idx == 0)
		flags |= SDHCI_CMD_RESP_NONE;
	else if (cmd->resp_type == MMC_RSP_R2)
		flags |= SDHCI_CMD_RESP_LONG;
	else
		flags |= SDHCI_CMD_RESP_SHORT;
	flags |= SDHCI_CMD_CRC;
	flags |= SDHCI_CMD_INDEX;
	flags |= SDHCI_CMD_DATA;

	if (host->flags & SDHCI_SDIO_IRQ_ENABLED) {
		bm_sdhci_host_init_for_irq(host);
	}

	if (host->flags & SDHCI_USE_ADMA3) {
		bm_sdhci_desc_reg_set(host, SDHCI_TRANSFER_MODE, mode);
		bm_sdhci_desc_reg_set(host, SDHCI_ARGUMENT, cmd->cmd_arg);
		bm_sdhci_desc_reg_set(host, SDHCI_COMMAND, SDHCI_MAKE_CMD(cmd->cmd_idx, flags));
		mmio_write_64(base + SDHCI_ADMA_ID_LOW_R, (uint64_t)host->intgr_table);
	} else {
		mmio_write_16(base + SDHCI_TRANSFER_MODE, mode);
		mmio_write_32(base + SDHCI_ARGUMENT, cmd->cmd_arg);
		// issue the cmd
		mmio_write_16(base + SDHCI_COMMAND,
				SDHCI_MAKE_CMD(cmd->cmd_idx, flags));
	}

	if (host->flags & SDHCI_SDIO_IRQ_ENABLED) {
		bm_sdhci_wait_cmd_complete(host);
		bm_sdhci_wait_data_complete(host);

		if (cmd->error || cmd->data->error) {
			printf("%s command transfer failed\n", __func__);
			return -1;
		}
	} else {
		if (bm_mmc_check_cmd_stats(host)) {
			printf("%s cmd stat err\n", __func__);
			return -1;
		}

		if (bm_mmc_check_data_stats(host)) {
			printf("%s data stat err\n", __func__);
			return -1;
		}
	}

	return 0;
}

static int bm_mmc_send_nondata_cmd(struct mmc_host *host, mmc_cmd_t *cmd)
{
	struct bm_mmc_params *param = host->priv;
	uintptr_t base = param->reg_base;
	u32 flags = 0x0;

	pr_debug("%s: cmd->cmd_idx %d, host->flags 0x%x\n", __func__, cmd->cmd_idx, host->flags);

	//make sure cmd line is clear
	while (1) {
		if (!(mmio_read_32(base + SDHCI_PRESENT_STATE) & SDHCI_CMD_INHIBIT))
			break;
	}

	// set cmd flags
	if (cmd->cmd_idx == MMC_CMD0)
		flags |= SDHCI_CMD_RESP_NONE;
	else if (cmd->cmd_idx == MMC_CMD1)
		flags |= SDHCI_CMD_RESP_SHORT;
	else if (cmd->cmd_idx == SD_ACMD41)
		flags |= SDHCI_CMD_RESP_SHORT;
	else if (cmd->resp_type == MMC_RSP_R2) {
		flags |= SDHCI_CMD_RESP_LONG;
		flags |= SDHCI_CMD_CRC;
	} else {
		flags |= SDHCI_CMD_RESP_SHORT;
		flags |= SDHCI_CMD_CRC;
		flags |= SDHCI_CMD_INDEX;
	}

	// make sure dat line is clear if necessary
	if (flags & SDHCI_CMD_RESP_SHORT_BUSY) {
		while (1) {
			if (!(mmio_read_32(base + SDHCI_PRESENT_STATE) & SDHCI_CMD_INHIBIT_DAT))
				break;
		}
	}

	if (cmd->cmd_idx == MMC_CMD19 || cmd->cmd_idx == MMC_CMD21)
		flags |= SDHCI_CMD_DATA;

	if (host->flags & SDHCI_SDIO_IRQ_ENABLED) {
		bm_sdhci_host_init_for_irq(host);
	}

	// issue the cmd
	mmio_write_32(base + SDHCI_ARGUMENT, cmd->cmd_arg);
	mmio_write_16(base + SDHCI_COMMAND,
			SDHCI_MAKE_CMD(cmd->cmd_idx, flags));

	if (cmd->cmd_idx == MMC_CMD19 || cmd->cmd_idx == MMC_CMD21)
		return 0;

	if (host->flags & SDHCI_SDIO_IRQ_ENABLED) {
		bm_sdhci_wait_cmd_complete(host);

		if (cmd->error) {
			printf("%s command transfer failed\n", __func__);
			return -1;
		}
	} else {
		if (bm_mmc_check_cmd_stats(host)) {
			printf("%s cmd stat err\n", __func__);
			return -1;
		}
	}

	return 0;
}

static int bm_mmc_send_cmd(struct mmc_host *host, mmc_cmd_t *cmd)
{
	pr_debug("send cmd, cmd_idx=%d, cmd_arg=0x%x, resp_type=0x%x\n",
		 cmd->cmd_idx, cmd->cmd_arg, cmd->resp_type);

	host->cmd = cmd;
	if (cmd->data) {
		host->data_cmd = cmd;
		host->data = cmd->data;
		return bm_mmc_send_data_cmd(host, cmd);
	} else {
		host->data_cmd = NULL;
		host->data = NULL;
		return bm_mmc_send_nondata_cmd(host, cmd);
	}
}

static void bm_mmc_set_clk(struct mmc_host *host, int clk)
{
	int i, div;
	struct bm_mmc_params *param = host->priv;
	uintptr_t base = param->reg_base;

	assert(clk > 0);

	printf("%s: clk %d\n", __func__, clk);

	if (param->f_max <= clk) {
		div = 0;
	} else {
		for (div=0x1; div<0x3FF; div++) {
			if (param->f_max / (2 * div) <= clk)
				break;
		}
		if (div == 0x3FF) {
			pr_debug("Warning: Can't set the freq to %d, divider is filled!!!\n", clk);
		}
	}
	assert(div <= 0x3FF);

	if (mmio_read_16(base + SDHCI_HOST_CONTROL2) & 1<<15) {
		printf("Use SDCLK Preset Value\n");
	} else {
		printf("Set SDCLK by driver. div=0x%x(%d)\n", div, div);
		mmio_write_16(base + SDHCI_CLK_CTRL,
				mmio_read_16(base + SDHCI_CLK_CTRL) & ~0x9); // disable INTERNAL_CLK_EN and PLL_ENABLE
		mmio_write_16(base + SDHCI_CLK_CTRL,
				(mmio_read_16(base + SDHCI_CLK_CTRL) & 0x3F) | ((div & 0xff) << 8) | ((div & 0x300) >> 2)); // set clk div

		mmio_write_16(base + SDHCI_CLK_CTRL,
				mmio_read_16(base + SDHCI_CLK_CTRL) | 0x1); // set INTERNAL_CLK_EN

		for (i = 0; i <= 150000; i += 100) {
			if (mmio_read_16(base + SDHCI_CLK_CTRL) & 0x2) {
				break;
			}
			udelay(100);
		}

		printf("mmio_read_16(base + SDHCI_CLK_CTRL) = 0x%x\n", mmio_read_16(base + SDHCI_CLK_CTRL));

		if (i > 150000) {
			ERROR("SD INTERNAL_CLK_EN seting FAILED!\n");
			assert(0);
		}

		mmio_write_16(base + SDHCI_CLK_CTRL,
				mmio_read_16(base + SDHCI_CLK_CTRL) | 0x8); // set PLL_ENABLE

		for (i = 0; i <= 150000; i += 100) {
			if (mmio_read_16(base + SDHCI_CLK_CTRL) & 0x2) {
				return;
			}
			udelay(100);
		}
	}

	ERROR("SD PLL seting FAILED!\n");
	return;
}

static void bm_mmc_change_clk(struct mmc_host *host, int clk)
{
	int div;
	int i;
	struct bm_mmc_params *param = host->priv;
	uintptr_t base = param->reg_base;

	if (clk <= 0) {
		mmio_write_16(base + SDHCI_CLK_CTRL,
				mmio_read_32(base + SDHCI_CLK_CTRL) & ~(0x1<<2)); // stop SD clock
//		mmio_write_16(base + 0x31e, 0x00);
		return;
	}

	printf("%s: param->f_max %d, clk %d\n", __func__, param->f_max, clk);

	if (param->f_max <= clk) {
		div = 0;
	} else {
		for (div=0x1; div<0x3FF; div++) {
			if (param->f_max / (2 * div) <= clk)
				break;
		}
		if (div == 0x3FF) {
			pr_debug("Warning: Can't set the freq to %d, divider is filled!!!\n", clk);
		}
	}
	assert(div <= 0x3FF);

	mmio_write_16(base + SDHCI_CLK_CTRL,
			mmio_read_16(base + SDHCI_CLK_CTRL) & ~(0x1<<2)); // stop SD clock

//	mmio_write_16(base + 0x31e, 0x10);
	mmio_write_16(base + SDHCI_CLK_CTRL,
			mmio_read_16(base + SDHCI_CLK_CTRL) & ~0x8); // disable  PLL_ENABLE

	if (mmio_read_16(base + SDHCI_HOST_CONTROL2) & (1 << 15)) {
		printf("Use SDCLK Preset Value\n");
		// 4 need recheck?
		mmio_write_16(base + SDHCI_HOST_CONTROL2,
				mmio_read_16(base + SDHCI_HOST_CONTROL2) & ~0x7); // clr UHS_MODE_SEL
	} else {
		printf("Set SDCLK by driver. div=0x%x(%d)\n", div, div);
		// 5 need recheck?

		// 6
		mmio_write_16(base + SDHCI_CLK_CTRL,
				(mmio_read_16(base + SDHCI_CLK_CTRL) & 0x3F) | ((div & 0xff) << 8) | ((div & 0x300) >> 2)); // set clk div
		mmio_write_16(base + SDHCI_CLK_CTRL,
				mmio_read_16(base + SDHCI_CLK_CTRL) & ~(0x1 << 5)); // CLK_GEN_SELECT
	}
	mmio_write_16(base + SDHCI_CLK_CTRL,
			mmio_read_16(base + SDHCI_CLK_CTRL) | 0xc); // enable  PLL_ENABLE

	printf("mmio_read_16(base + SDHCI_CLK_CTRL) = 0x%x\n", mmio_read_16(base + SDHCI_CLK_CTRL));

	for (i = 0; i <= 150000; i += 100) {
		if (mmio_read_16(base + SDHCI_CLK_CTRL) & 0x2) {
			return;
		}
		udelay(100);
	}

	ERROR("SD PLL seting FAILED!\n");
	return;
}

static int bm_mmc_send_tuning(struct mmc_host *host, u32 opcode)
{
	struct bm_mmc_params *param = host->priv;
	uintptr_t base = param->reg_base;
	mmc_cmd_t cmd;
	u32 norm_stat, err_stat;

	MMC_PACK_CMD(cmd, opcode, 0, MMC_RSP_R1);
	mmio_write_16(base + SDHCI_BLOCK_SIZE, SDHCI_MAKE_BLKSZ(7, 64));
	mmio_write_16(base + SDHCI_TRANSFER_MODE, SDHCI_TRNS_READ);

	host->ops->send_cmd(host, &cmd);

	while (1) {
		norm_stat = mmio_read_16(base + SDHCI_INT_STATUS);
		err_stat = mmio_read_16(base + SDHCI_ERR_INT_STATUS);
		if (norm_stat & SDHCI_INT_BUF_RD_READY)
			break;
		if (err_stat & 0x7f)
			break;
	}
	mmio_write_16(base + SDHCI_INT_STATUS, 0xff);
	mmio_write_16(base + SDHCI_ERR_INT_STATUS, 0xff);

	mmio_write_8(base + SDHCI_SOFTWARE_RESET, 0x6);
	while (mmio_read_8(base + SDHCI_SOFTWARE_RESET))
		;

	// dump tuning status and lead_lag_flag value
	printf("%s, err_stat %d, h312:0x%x\n", __func__, err_stat, mmio_read_16(base + 0x312));

	if (err_stat == 0) {
//		printf("%s tuning pass\n", __func__);
		return 0;
	}

//	printf("%s tuning failed\n", __func__);
	return -1;
}

static void bm_sdhci_set_tap(struct mmc_host *host, u16 tap)
{
	struct bm_mmc_params *param = host->priv;
	uintptr_t base = param->reg_base;

	/* printf("%s tap:%d\n", __func__, tap); */

	mmio_write_16(base + 0x200, mmio_read_16(base+0x200)|0x2);

	mmio_write_8(base + 0x240, tap);
}

#define NUMBER_OF_STEP 128
int bm_mmc_execute_tuning(struct mmc_host *host, u32 opcode)
{
	struct bm_mmc_params *param = host->priv;
	uintptr_t base = param->reg_base;
	u32 ctl2;
	u16 min, max;
	u32 norm_stat_en_b, err_stat_en_b;
	u32 norm_signal_en_b;



	norm_stat_en_b = mmio_read_16(base + SDHCI_INT_STATUS_EN);
	err_stat_en_b = mmio_read_16(base + SDHCI_ERR_INT_STATUS_EN);
	norm_signal_en_b = mmio_read_32(base + SDHCI_SIGNAL_ENABLE);

	mmio_write_32(base + SDHCI_SIGNAL_ENABLE, 0);
	mmio_write_16(base + SDHCI_INT_STATUS_EN, SDHCI_INT_BUF_RD_READY);
	mmio_write_16(base + SDHCI_ERR_INT_STATUS_EN, 0x7f);

	mmio_write_16(base + SDHCI_INT_STATUS, 0xff);
	mmio_write_16(base + SDHCI_ERR_INT_STATUS, 0xff);

	ctl2 = mmio_read_16(base + SDHCI_HOST_CONTROL2);
	ctl2 |= SDHCI_CTRL_EXEC_TUNING;
	mmio_write_16(base + SDHCI_HOST_CONTROL2, ctl2);

	mmio_write_8(base + SDHCI_SOFTWARE_RESET, 0x6);
	while (mmio_read_8(base + SDHCI_SOFTWARE_RESET))
		;
	min = 0;
	while (min < NUMBER_OF_STEP) {
		bm_sdhci_set_tap(host, min);
		if (!bm_mmc_send_tuning(host, opcode)) {
			printf("%s break1\n", __func__);
			break;
		}
		min++;
	}

	if (min >= NUMBER_OF_STEP) {
		printf("tuning failed!!!\n");
		return -1;
	}

	max = min + 1;
	while (max < NUMBER_OF_STEP) {
		bm_sdhci_set_tap(host, max);

		if (bm_mmc_send_tuning(host, opcode)) {
			max--;
			printf("%s break2\n", __func__);
			break;
		}
		max++;
	}

	bm_sdhci_set_tap(host, min + ((max - min) / 2));

	mmio_write_8(base + SDHCI_SOFTWARE_RESET, 0x6);
	while (mmio_read_8(base + SDHCI_SOFTWARE_RESET))
		;

	ctl2 = mmio_read_16(base + SDHCI_HOST_CONTROL2);
	ctl2 &= ~SDHCI_CTRL_EXEC_TUNING;
	mmio_write_16(base + SDHCI_HOST_CONTROL2, ctl2);

	mmio_write_16(base + SDHCI_INT_STATUS_EN, norm_stat_en_b);
	mmio_write_32(base + SDHCI_SIGNAL_ENABLE, norm_signal_en_b);
	mmio_write_16(base + SDHCI_ERR_INT_STATUS_EN, err_stat_en_b);

	printf("%s finished tuning, code:%d\n", __func__,
	       min + ((max - min) / 2));

	return 0;
}

static int bm_mmc_select_drive_strength(struct mmc_card *card,
				       unsigned int max_dtr, int host_drv,
				       int card_drv, int *drv_type)
{
// 1822 Remove PHY
	return 0;
}

#define DEFAULT_DIV_SD_INIT_CLOCK 0x2

#define SDHCI_RX_DELAY_LINE		0x40 // P_VERDOR_SPECIFIC_AREA + 0x40(0x240) PHY_TX_RX_DLY
#define REG_RX_SRC_SEL_MASK     (0xFCFFFFFF) //	0x240 PHY_RX_SRC [25:24]
#define REG_RX_SRC_SEL_CLK_TX			(0) // clk_tx
#define REG_RX_SRC_SEL_CLK_TX_INV		(1) // clk_tx_inv
#define REG_RX_SRC_SEL_RES			(2) // resvd
#define REG_RX_SRC_SEL_PAD_CLK			(1) // pad_clk
#define REG_RX_SRC_SEL_SHIFT		(24) // 0x240 PHY_RX_SRC [25:24]

#define SDHCI_TX_DELAY_LINE		0x40 // P_VERDOR_SPECIFIC_AREA + 0x40(0x240) PHY_TX_RX_DLY
#define REG_TX_SRC_SEL_MASK		(0xFFFFFCFF) // 0x240  PHY_TX_SRC[9:8]
#define REG_TX_SRC_SEL_CLK_TX			(0) // clk_tx
#define REG_TX_SRC_SEL_CLK_TX_INV		(1) // clk_tx_inv
#define REG_TX_SRC_SEL_RES			(2) // resvd
#define REG_TX_SRC_SEL_PAD_CLK			(1) // pad_clk
#define REG_TX_SRC_SEL_SHIFT		(8) // 0x240  PHY_TX_SRC[9:8]

static void bm_mmc_hw_reset(struct mmc_host *host)
{
	struct bm_mmc_params *param = host->priv;
	uintptr_t base = param->reg_base;
	printf("base 0x%lx\n", base);

	uintptr_t vendor_base = base + (mmio_read_16(base + P_VENDOR_SPECIFIC_AREA) & ((1<<12)-1));
	printf("vendor base 0x%lx\n", vendor_base);
	// set default clk div before reset IP to avoid CLK glitch
	mmio_write_16(base + SDHCI_CLK_CTRL,
		(mmio_read_16(base + SDHCI_CLK_CTRL) & 0x3F) | DEFAULT_DIV_SD_INIT_CLOCK << 8);

	mdelay(1);

	// reset IP
	mmio_write_8(base + SDHCI_SOFTWARE_RESET, 0x7);
	while (mmio_read_8(base + SDHCI_SOFTWARE_RESET))
		;

	/* mmio_write_32(base + SDHCI_RX_DELAY_LINE, 0); */
	/* mmio_write_32(base + SDHCI_TX_DELAY_LINE, 0); */
	mmio_write_32(vendor_base + SDHCI_RX_DELAY_LINE,
		(mmio_read_32(vendor_base + SDHCI_RX_DELAY_LINE) & REG_RX_SRC_SEL_MASK) |
		REG_RX_SRC_SEL_CLK_TX_INV << REG_RX_SRC_SEL_SHIFT);

	// set tx src sel to 1
	mmio_write_32(vendor_base + SDHCI_TX_DELAY_LINE,
		(mmio_read_32(vendor_base + SDHCI_TX_DELAY_LINE) & REG_TX_SRC_SEL_MASK) |
		REG_TX_SRC_SEL_CLK_TX_INV << REG_TX_SRC_SEL_SHIFT);

}

/*
 * https://info.bitmain.vip:8443/display/BM1882/V2_07_EMMC_SD
 *
 * PAD NAME	reg name		address/bit	reset value		emmc mode	Note
 * EMMC_RSTN	REG_EMMC_RSTN_PU	0x0300_1C24[2]		0		1		PU
 *		REG_EMMC_RSTN_PD	0x0300_1C24[3]		1		0
 *
 * EMMC_CLK	REG_EMMC_CLK_PU		0x0300_1C20[2]		0		0		PD
 *		REG_EMMC_CLK_PD		0x0300_1C20[3]		1		1
 *
 * EMMC_CMD	REG_EMMC_CMD_PU		0x0300_1C28[2]		0		1		PU
 *		REG_EMMC_CMD_PD		0x0300_1C28[3]		1		0
 *
 * EMMC_DAT0	REG_EMMC_DAT0_PU	0x0300_1C30[2]		0		1		PU
 *		REG_EMMC_DAT0_PD	0x0300_1C30[3]		1		0
 *
 * EMMC_DAT1	REG_EMMC_DAT1_PU	0x0300_1C2C[2]		0		1		PU
 *		REG_EMMC_DAT1_PD	0x0300_1C2C[3]		1		0
 *
 * EMMC_DAT2	REG_EMMC_DAT2_PU	0x0300_1C34[2]		0		1		PU
 *		REG_EMMC_DAT2_PD	0x0300_1C34[3]		1		0
 *
 * EMMC_DAT3	REG_EMMC_DAT3_PU	0x0300_1C38[2]		0		1		PU
 *		REG_EMMC_DAT3_PD	0x0300_1C38[3]		1		0
 */

#define REG_EMMC_PAD_MASK	(0xFFFFFFF3)
#define REG_EMMC_PAD_SHIFT	(2)

#define REG_EMMC_CLK_PAD_REG	(PINMUX_BASE + 0xC20)
#define REG_EMMC_CLK_PAD_VALUE	(2)

#define REG_EMMC_RSTN_PAD_REG	(PINMUX_BASE + 0xC24)
#define REG_EMMC_RSTN_PAD_VALUE	(1)

#define REG_EMMC_CMD_PAD_REG	(PINMUX_BASE + 0xC28)
#define REG_EMMC_CMD_PAD_VALUE	(1)

#define REG_EMMC_DAT1_PAD_REG	(PINMUX_BASE + 0xC2C)
#define REG_EMMC_DAT1_PAD_VALUE	(1)

#define REG_EMMC_DAT0_PAD_REG	(PINMUX_BASE + 0xC30)
#define REG_EMMC_DAT0_PAD_VALUE	(1)

#define REG_EMMC_DAT2_PAD_REG	(PINMUX_BASE + 0xC34)
#define REG_EMMC_DAT2_PAD_VALUE	(1)

#define REG_EMMC_DAT3_PAD_REG	(PINMUX_BASE + 0xC38)
#define REG_EMMC_DAT3_PAD_VALUE	(1)

static void bm_emmc_pad_setting(void)
{
	mmio_write_32(REG_EMMC_CLK_PAD_REG,
		(mmio_read_32(REG_EMMC_CLK_PAD_REG) & REG_EMMC_PAD_MASK) |
		REG_EMMC_CLK_PAD_VALUE << REG_EMMC_PAD_SHIFT);

	mmio_write_32(REG_EMMC_RSTN_PAD_REG,
		(mmio_read_32(REG_EMMC_RSTN_PAD_REG) & REG_EMMC_PAD_MASK) |
		REG_EMMC_RSTN_PAD_VALUE << REG_EMMC_PAD_SHIFT);

	mmio_write_32(REG_EMMC_CMD_PAD_REG,
		(mmio_read_32(REG_EMMC_CMD_PAD_REG) & REG_EMMC_PAD_MASK) |
		REG_EMMC_CMD_PAD_VALUE << REG_EMMC_PAD_SHIFT);

	mmio_write_32(REG_EMMC_DAT1_PAD_REG,
		(mmio_read_32(REG_EMMC_DAT1_PAD_REG) & REG_EMMC_PAD_MASK) |
		REG_EMMC_DAT1_PAD_VALUE << REG_EMMC_PAD_SHIFT);

	mmio_write_32(REG_EMMC_DAT0_PAD_REG,
		(mmio_read_32(REG_EMMC_DAT0_PAD_REG) & REG_EMMC_PAD_MASK) |
		REG_EMMC_DAT0_PAD_VALUE << REG_EMMC_PAD_SHIFT);

	mmio_write_32(REG_EMMC_DAT2_PAD_REG,
		(mmio_read_32(REG_EMMC_DAT2_PAD_REG) & REG_EMMC_PAD_MASK) |
		REG_EMMC_DAT2_PAD_VALUE << REG_EMMC_PAD_SHIFT);

	mmio_write_32(REG_EMMC_DAT3_PAD_REG,
		(mmio_read_32(REG_EMMC_DAT3_PAD_REG) & REG_EMMC_PAD_MASK) |
		REG_EMMC_DAT3_PAD_VALUE << REG_EMMC_PAD_SHIFT);
}


#define REG_SDIO0_PAD_MASK		(0xFFFFFFF3)
#define REG_SDIO0_PAD_SHIFT		(2)

#define REG_SDIO0_CD_PAD_REG		(PINMUX_BASE + 0x900)
#define REG_SDIO0_CD_PAD_VALUE		(1)

#define REG_SDIO0_PWR_EN_PAD_REG	(PINMUX_BASE + 0x904)
#define REG_SDIO0_PWR_EN_PAD_VALUE	(1)

#define REG_SDIO0_CLK_PAD_REG		(PINMUX_BASE + 0xA00)
#define REG_SDIO0_CLK_PAD_VALUE		(1)

#define REG_SDIO0_CMD_PAD_REG		(PINMUX_BASE + 0xA04)
#define REG_SDIO0_CMD_PAD_VALUE		(1)

#define REG_SDIO0_DAT0_PAD_REG		(PINMUX_BASE + 0xA08)
#define REG_SDIO0_DAT0_PAD_VALUE	(1)

#define REG_SDIO0_DAT1_PAD_REG		(PINMUX_BASE + 0xA0C)
#define REG_SDIO0_DAT1_PAD_VALUE	(1)

#define REG_SDIO0_DAT2_PAD_REG		(PINMUX_BASE + 0xA10)
#define REG_SDIO0_DAT2_PAD_VALUE	(1)

#define REG_SDIO0_DAT3_PAD_REG		(PINMUX_BASE + 0xA14)
#define REG_SDIO0_DAT3_PAD_VALUE	(1)

#define RTCIO_BASE (0x5027000)
#define REG_SDIO1_PAD_MASK		(0xFFFFFFF3)
#define REG_SDIO1_PAD_SHIFT		(2)

#define REG_SDIO1_CLK_PAD_REG		(RTCIO_BASE + 0x6C)
#define REG_SDIO1_CLK_PAD_VALUE		(1)

#define REG_SDIO1_CMD_PAD_REG		(RTCIO_BASE + 0x68)
#define REG_SDIO1_CMD_PAD_VALUE		(1)

#define REG_SDIO1_DAT0_PAD_REG		(RTCIO_BASE + 0x64)
#define REG_SDIO1_DAT0_PAD_VALUE	(1)

#define REG_SDIO1_DAT1_PAD_REG		(RTCIO_BASE + 0x60)
#define REG_SDIO1_DAT1_PAD_VALUE	(1)

#define REG_SDIO1_DAT2_PAD_REG		(RTCIO_BASE + 0x5C)
#define REG_SDIO1_DAT2_PAD_VALUE	(1)

#define REG_SDIO1_DAT3_PAD_REG		(RTCIO_BASE + 0x58)
#define REG_SDIO1_DAT3_PAD_VALUE	(1)

void bm_sdio_pad_setting(void)
{
	printf("%s\n", __func__);
#if defined(TEST_SD0)
	mmio_write_32(REG_SDIO0_CD_PAD_REG,
		(mmio_read_32(REG_SDIO0_CD_PAD_REG) & REG_SDIO0_PAD_MASK) |
		REG_SDIO0_CD_PAD_VALUE << REG_SDIO0_PAD_SHIFT);

	mmio_write_32(REG_SDIO0_CLK_PAD_REG,
		(mmio_read_32(REG_SDIO0_CLK_PAD_REG) & REG_SDIO0_PAD_MASK) |
		REG_SDIO0_CLK_PAD_VALUE << REG_SDIO0_PAD_SHIFT);

	mmio_write_32(REG_SDIO0_CMD_PAD_REG,
		(mmio_read_32(REG_SDIO0_CMD_PAD_REG) & REG_SDIO0_PAD_MASK) |
		REG_SDIO0_CMD_PAD_VALUE << REG_SDIO0_PAD_SHIFT);

	mmio_write_32(REG_SDIO0_DAT1_PAD_REG,
		(mmio_read_32(REG_SDIO0_DAT1_PAD_REG) & REG_SDIO0_PAD_MASK) |
		REG_SDIO0_DAT1_PAD_VALUE << REG_SDIO0_PAD_SHIFT);

	mmio_write_32(REG_SDIO0_DAT0_PAD_REG,
		(mmio_read_32(REG_SDIO0_DAT0_PAD_REG) & REG_SDIO0_PAD_MASK) |
		REG_SDIO0_DAT0_PAD_VALUE << REG_SDIO0_PAD_SHIFT);

	mmio_write_32(REG_SDIO0_DAT2_PAD_REG,
		(mmio_read_32(REG_SDIO0_DAT2_PAD_REG) & REG_SDIO0_PAD_MASK) |
		REG_SDIO0_DAT2_PAD_VALUE << REG_SDIO0_PAD_SHIFT);

	mmio_write_32(REG_SDIO0_DAT3_PAD_REG,
		(mmio_read_32(REG_SDIO0_DAT3_PAD_REG) & REG_SDIO0_PAD_MASK) |
		REG_SDIO0_DAT3_PAD_VALUE << REG_SDIO0_PAD_SHIFT);
#elif defined(TEST_SD1)
	mmio_write_32(REG_SDIO1_CLK_PAD_REG,
		(mmio_read_32(REG_SDIO1_CLK_PAD_REG) & REG_SDIO1_PAD_MASK) |
		REG_SDIO1_CLK_PAD_VALUE << REG_SDIO1_PAD_SHIFT);

	mmio_write_32(REG_SDIO1_CMD_PAD_REG,
		(mmio_read_32(REG_SDIO1_CMD_PAD_REG) & REG_SDIO1_PAD_MASK) |
		REG_SDIO1_CMD_PAD_VALUE << REG_SDIO1_PAD_SHIFT);

	mmio_write_32(REG_SDIO1_DAT1_PAD_REG,
		(mmio_read_32(REG_SDIO1_DAT1_PAD_REG) & REG_SDIO1_PAD_MASK) |
		REG_SDIO1_DAT1_PAD_VALUE << REG_SDIO1_PAD_SHIFT);

	mmio_write_32(REG_SDIO1_DAT0_PAD_REG,
		(mmio_read_32(REG_SDIO1_DAT0_PAD_REG) & REG_SDIO1_PAD_MASK) |
		REG_SDIO1_DAT0_PAD_VALUE << REG_SDIO1_PAD_SHIFT);

	mmio_write_32(REG_SDIO1_DAT2_PAD_REG,
		(mmio_read_32(REG_SDIO1_DAT2_PAD_REG) & REG_SDIO1_PAD_MASK) |
		REG_SDIO1_DAT2_PAD_VALUE << REG_SDIO1_PAD_SHIFT);

	mmio_write_32(REG_SDIO1_DAT3_PAD_REG,
		(mmio_read_32(REG_SDIO1_DAT3_PAD_REG) & REG_SDIO1_PAD_MASK) |
		REG_SDIO1_DAT3_PAD_VALUE << REG_SDIO1_PAD_SHIFT);
#endif
}

#define REG_TOP_SD_PWRSW_CTRL		(0x1F4)

void bm_mmc_phy_init(struct mmc_host *host)
{
	struct bm_mmc_params *param = host->priv;
	uintptr_t base = param->reg_base;

	uintptr_t vendor_base = base + (mmio_read_16(base + P_VENDOR_SPECIFIC_AREA) & ((1<<12)-1));
	bm_mmc_hw_reset(host);

	// set rx/tx delay line
	if (host->flags & SDHCI_TYPE_EMMC) {
		bm_emmc_pad_setting();
	}
	else {
		// https://info.bitmain.vip:8443/display/BM1882/SDCard+initialization+software+flow
		// 3. Wait 3ms to let VDD3V_SD to settle (assume 10ohm x 100uF x 3 RC = 3000us = 3ms)
		mdelay(3);
		// 4. sd_pwrsw_ctrl (0x030001F4) = 0x00000009
		// (reg_pwrsw_auto=1, reg_pwrsw_disc=0, reg_pwrsw_vsel=0(3.0v), reg_en_pwrsw=1)
		mmio_write_32(TOP_BASE + REG_TOP_SD_PWRSW_CTRL, 0x9);
		// 5. Wait 1ms (1ohm x 10uF x 3RC = 30us , x2 : 0->1.8->3.3)
		mdelay(1);
		// 6. Enable SDIO io pull up
		bm_sdio_pad_setting();

		mmio_write_32(vendor_base + SDHCI_RX_DELAY_LINE,
		(mmio_read_32(vendor_base + SDHCI_RX_DELAY_LINE) & REG_RX_SRC_SEL_MASK) |
		REG_RX_SRC_SEL_CLK_TX_INV << REG_RX_SRC_SEL_SHIFT);

		// set tx src sel to 1
		mmio_write_32(vendor_base + SDHCI_TX_DELAY_LINE,
				(mmio_read_32(vendor_base + SDHCI_TX_DELAY_LINE) & REG_TX_SRC_SEL_MASK) |
				REG_TX_SRC_SEL_CLK_TX_INV << REG_TX_SRC_SEL_SHIFT);

	}

}

static void bm_sdhci_finish_command(struct mmc_host *host)
{
	int i;
	struct bm_mmc_params *param = host->priv;
	uintptr_t base = param->reg_base;
	volatile mmc_cmd_t *cmd = host->cmd;

	if (cmd->resp_type & MMC_RSP_PRESENT) {
		if (cmd->resp_type & MMC_RSP_136) {
			/* CRC is stripped so we need to do some shifting. */
			for (i = 0; i < 4; i++) {
				cmd->resp_data[i] = mmio_read_32(base + SDHCI_RESPONSE_01 + (3-i)*4) << 8;
				if (i != 3)
					cmd->resp_data[i] |= mmio_read_8(base + SDHCI_RESPONSE_01 + (3-i)*4-1);
			}
		} else {
			cmd->resp_data[0] = mmio_read_32(base + SDHCI_RESPONSE_01);
		}
	}
}

static void bm_sdhci_cmd_irq(struct mmc_host *host, u32 intmask)
{
	if (!host->cmd) {
		pr_debug("%s Got command interrupt 0x%08x even through no command operation was in progress.\n", __func__, intmask);

		return;
	}

	if (intmask & (SDHCI_INT_TIMEOUT | SDHCI_INT_CRC |
		       SDHCI_INT_END_BIT | SDHCI_INT_INDEX)) {
		if (intmask & SDHCI_INT_TIMEOUT)
			host->cmd->error = -ETIMEDOUT;
		else
			host->cmd->error = -EILSEQ;

		host->cmd = NULL;
		return;
	}

	if (intmask & SDHCI_INT_RESPONSE)
		bm_sdhci_finish_command(host);
}

static void bm_sdhci_data_irq(struct mmc_host *host, u32 intmask)
{
	volatile mmc_cmd_t *data_cmd = host->data_cmd;
	struct bm_mmc_params *param = host->priv;
	uintptr_t base = param->reg_base;
	u32 command;
	u32 dma_addr;

	/* CMD19 generates _only_ Buffer Read Ready interrupt */
	if (intmask & SDHCI_INT_DATA_AVAIL) {
		command = SDHCI_GET_CMD(mmio_read_16(base + SDHCI_COMMAND));
		if (command == MMC_CMD19 ||
		    command == MMC_CMD19) {
			//host->tuning_done = 1;
			return;
		}
	}

	if (data_cmd && (data_cmd->data->flags & MMC_RSP_BUSY)) {
		if (intmask & SDHCI_INT_DATA_TIMEOUT) {
			host->data_cmd = NULL;
			data_cmd->error = -ETIMEDOUT;
			return;
		}
		if (intmask & SDHCI_INT_DATA_END) {
			host->data_cmd = NULL;
			return;
		}
	}

	if (intmask & SDHCI_INT_DATA_TIMEOUT)
		host->data->error = -ETIMEDOUT;
	else if (intmask & SDHCI_INT_DATA_END_BIT)
		host->data->error = -EILSEQ;
	else if (intmask & SDHCI_INT_DATA_CRC) {
		host->data->error = -EILSEQ;
	}
	else if (intmask & SDHCI_INT_ADMA_ERROR) {
		host->data->error = -EIO;
	}

	if (host->data->error) {
		host->data = NULL;
		host->data_cmd = NULL;
	} else {
		if (intmask & (SDHCI_INT_DATA_AVAIL | SDHCI_INT_SPACE_AVAIL))
			bm_sdhci_transfer_pio(host);

		if (intmask & SDHCI_INT_DATA_END) {
			host->data = NULL;
			host->data_cmd = NULL;
			return;
		}

		if ((host->flags & SDHCI_USE_SDMA) && (intmask & SDHCI_INT_DMA_END)) {
			if (mmio_read_16(base + SDHCI_HOST_CONTROL2) & SDHCI_HOST_VER4_ENABLE) {
				dma_addr = mmio_read_32(base + SDHCI_ADMA_SA_LOW);
				mmio_write_32(base + SDHCI_ADMA_SA_LOW, dma_addr);
				mmio_write_32(base + SDHCI_ADMA_SA_HIGH, 0);
			} else {
				dma_addr = mmio_read_32(base + SDHCI_DMA_ADDRESS);
				mmio_write_32(base + SDHCI_DMA_ADDRESS, dma_addr);
			}
		}
	}

	return;
}

static int bm_sdhci_irq_handler(int irqn, void *priv)
{
	struct mmc_host *host = priv;
	struct bm_mmc_params *param = host->priv;
	uintptr_t base = param->reg_base;
	u32 intmask, mask, unexpected;
	int max_loop = 16;

	intmask = mmio_read_32(base + SDHCI_INT_STATUS);

	if (!intmask || intmask == 0xffffffff) {
		printf("%s never be here!\n", __func__);
		return -1;
	}

	pr_debug("%s got interrupt: 0x%08x\n", __func__, intmask);

	do {
		mask = intmask & (SDHCI_INT_CMD_MASK | SDHCI_INT_DATA_MASK |
				SDHCI_INT_BUS_POWER);

		mmio_write_32(base + SDHCI_INT_STATUS, mask);

		if (intmask & SDHCI_INT_CMD_MASK) {
			pr_debug("%s: cmd->cmd_idx %d, intmask 0x%x\n", __func__, host->cmd->cmd_idx, intmask);
			bm_sdhci_cmd_irq(host, intmask & SDHCI_INT_CMD_MASK);	
			host->cmd_completion = 1;
		}

		if (intmask & SDHCI_INT_DATA_MASK) {
			pr_debug("%s: data_cmd->cmd_idx %d, intmask 0x%x\n", __func__, host->data_cmd->cmd_idx, intmask);
			bm_sdhci_data_irq(host, intmask & SDHCI_INT_DATA_MASK);
			host->data_completion = 1;
		}

		intmask &= ~(SDHCI_INT_CARD_INSERT | SDHCI_INT_CARD_REMOVE |
				SDHCI_INT_CMD_MASK | SDHCI_INT_DATA_MASK |
				SDHCI_INT_ERROR | SDHCI_INT_BUS_POWER |
				SDHCI_INT_RETUNE | SDHCI_INT_CARD_INT);

		if (intmask) {
			unexpected = intmask;
			mmio_write_32(base + SDHCI_INT_STATUS, intmask);
			printf("%s unexpected interrupt: 0x%08x\n", __func__, unexpected);
		}

		intmask = mmio_read_32(base + SDHCI_INT_STATUS);
	} while (intmask && --max_loop);

	return 0;
}

static void bm_mmc_hw_init(struct mmc_host *host)
{
	struct bm_mmc_params *param = host->priv;
	uintptr_t base = param->reg_base;

	// Wait for phy power good

	// reset data & cmd
	mmio_write_8(base + SDHCI_SOFTWARE_RESET, 0x6);

	// init common parameters
	mmio_write_8(base + SDHCI_PWR_CONTROL, (0x7 << 1));
	mmio_write_8(base + SDHCI_TOUT_CTRL, 0xe);  // for TMCLK 50Khz
	mmio_write_16(base + SDHCI_HOST_CONTROL2,
			mmio_read_16(base + SDHCI_HOST_CONTROL2) | 1<<11);  // set cmd23 support

	mmio_write_16(base + SDHCI_CLK_CTRL, mmio_read_16(base + SDHCI_CLK_CTRL) & ~(0x1 << 5));  // divided clock mode

	if (host->flags & SDHCI_USE_VERSION4) {
		// set host version 4 parameters
		mmio_write_16(base + SDHCI_HOST_CONTROL2,
				mmio_read_16(base + SDHCI_HOST_CONTROL2) | SDHCI_HOST_VER4_ENABLE); // set HOST_VER4_ENABLE
	}
	if (mmio_read_32(base + SDHCI_CAPABILITIES1) & (0x1 << 27)) {
		mmio_write_16(base + SDHCI_HOST_CONTROL2,
				mmio_read_16(base + SDHCI_HOST_CONTROL2) | 0x1<<13); // set 64bit addressing
	}

	// if support asynchronous int
	if (mmio_read_32(base + SDHCI_CAPABILITIES1) & (0x1<<29)) {
		mmio_write_16(base + SDHCI_HOST_CONTROL2, mmio_read_16(base + SDHCI_HOST_CONTROL2) | (0x1<<14)); // enable async int
	}

	// give some time to power down card
#ifndef BOARD_PALLADIUM
	mdelay(20);
#endif

	// 12
	mmio_write_16(base + SDHCI_HOST_CONTROL2,
			mmio_read_16(base + SDHCI_HOST_CONTROL2) & ~(0x1<<8)); // clr UHS2_IF_ENABLE
	mmio_write_8(base + SDHCI_PWR_CONTROL,
			mmio_read_8(base + SDHCI_PWR_CONTROL) | 0x1); // set SD_BUS_PWR_VDD1
	mmio_write_16(base + SDHCI_HOST_CONTROL2,
			mmio_read_16(base + SDHCI_HOST_CONTROL2) & ~0x7); // clr UHS_MODE_SEL

	// 13
	bm_mmc_set_clk(host, host->ios.clock);

#ifndef BOARD_PALLADIUM
	mdelay(50);
#endif

	// 15
	mmio_write_16(base + SDHCI_CLK_CTRL,
			mmio_read_16(base + SDHCI_CLK_CTRL) | (0x1<<2)); // supply SD clock

	// 16
	udelay(400); // wait for voltage ramp up time at least 74 cycle, 400us is 80 cycles for 200Khz

	mmio_write_16(base + SDHCI_INT_STATUS, mmio_read_16(base + SDHCI_INT_STATUS) | (0x1 << 6));

	//we enable all interrupt status here for testing
	mmio_write_16(base + SDHCI_INT_STATUS_EN, mmio_read_16(base + SDHCI_INT_STATUS_EN) | 0xFFFF);
	mmio_write_16(base + SDHCI_ERR_INT_STATUS_EN,mmio_read_16(base + SDHCI_ERR_INT_STATUS_EN) | 0xFFFF);

	if (host->ier) {
		if (host->flags & SDHCI_TYPE_EMMC)
			request_irq(EMMC_INTR, bm_sdhci_irq_handler, 0, "software int", host);
		else {
#if defined(TEST_SD0)
			request_irq(SD0_INTR, bm_sdhci_irq_handler, 0, "software int", host);
#elif defined(TEST_SD1)
			request_irq(SD1_INTR, bm_sdhci_irq_handler, 0, "software int", host);
#endif
		}
		mmio_write_32(base + SDHCI_SIGNAL_ENABLE, host->ier);
	} else {
		mmio_write_32(base + SDHCI_SIGNAL_ENABLE, 0);
		disable_irq(SD0_INTR);
	}


	INFO("bm_mmc_hw_init done\n");
}

static void bm_mmc_set_uhs_signaling(struct mmc_host *host, unsigned int uhs)
{
	struct bm_mmc_params *param = host->priv;
	uintptr_t base = param->reg_base;
	u16 ctrl_2;

	ctrl_2 = mmio_read_16(base + SDHCI_HOST_CONTROL2);
	/* Select Bus Speed Mode for host */
	ctrl_2 &= ~SDHCI_CTRL_UHS_MASK;
	switch (uhs) {
	case MMC_TIMING_SD_HS:
		ctrl_2 |= SDHCI_CTRL_UHS_SDR25;
		break;
	case MMC_TIMING_UHS_SDR12:
		ctrl_2 |= SDHCI_CTRL_UHS_SDR12;
		break;
	case MMC_TIMING_UHS_SDR25:
		ctrl_2 |= SDHCI_CTRL_UHS_SDR25;
		break;
	case MMC_TIMING_UHS_SDR50:
		ctrl_2 |= SDHCI_CTRL_UHS_SDR50;
		break;
	case MMC_TIMING_MMC_HS200:
	case MMC_TIMING_UHS_SDR104:
		ctrl_2 |= SDHCI_CTRL_UHS_SDR104;
		break;
	case MMC_TIMING_UHS_DDR50:
	case MMC_TIMING_MMC_DDR52:
		ctrl_2 |= SDHCI_CTRL_UHS_DDR50;
		break;
	}

	/*
	 * When clock frequency is less than 100MHz, the feedback clock must be
	 * provided and DLL must not be used so that tuning can be skipped. To
	 * provide feedback clock, the mode selection can be any value less
	 * than 3'b011 in bits [2:0] of HOST CONTROL2 register.
	 */
	if (host->clock <= 100000000 &&
	    (uhs == MMC_TIMING_MMC_HS400 ||
	     uhs == MMC_TIMING_MMC_HS200 ||
	     uhs == MMC_TIMING_UHS_SDR104))
		ctrl_2 &= ~SDHCI_CTRL_UHS_MASK;

	printf("%s: clock=%u uhs=%u ctrl_2=0x%x ken\n",	__func__, host->clock, uhs, ctrl_2);
	mmio_write_16(base + SDHCI_HOST_CONTROL2, ctrl_2);
}

static int bm_mmc_set_ios(struct mmc_host *host, struct mmc_ios *ios)
{
	struct bm_mmc_params *param = host->priv;
	uintptr_t base = param->reg_base;
	u32 ctrl;

	printf("%s ios->bus_width:%d, ios->clock:%d, ios->timing:0x%x\n", __func__, ios->bus_width, ios->clock, ios->timing);

	if (!ios->clock || ios->clock != host->clock) {
		bm_mmc_change_clk(host, ios->clock);
		host->clock = ios->clock;
	}

	ctrl = mmio_read_8(base + SDHCI_HOST_CONTROL);
	switch (ios->bus_width) {
		case MMC_BUS_WIDTH_1:
			ctrl &= ~SDHCI_DAT_XFER_WIDTH;
			break;
		case MMC_BUS_WIDTH_4:
			ctrl |= SDHCI_DAT_XFER_WIDTH;
			break;
		default:
			assert(0);
	}

	if ((ios->timing == MMC_TIMING_MMC_HS200) ||
		(ios->timing == MMC_TIMING_MMC_DDR52) ||
		(ios->timing == MMC_TIMING_SD_HS) ||
		(ios->timing == MMC_TIMING_UHS_SDR50) ||
		(ios->timing == MMC_TIMING_UHS_SDR104) ||
		(ios->timing == MMC_TIMING_UHS_DDR50) ||
		(ios->timing == MMC_TIMING_UHS_SDR25)) {
		ctrl |= SDHCI_CTRL_HISPD;
		bm_mmc_set_uhs_signaling(host, ios->timing);
		host->timing = ios->timing;
	}
	else
		ctrl &= ~SDHCI_CTRL_HISPD;


	mmio_write_8(base + SDHCI_HOST_CONTROL, ctrl);

	return 0;
}

static int bm_mmc_start_signal_voltage_switch(struct mmc_host *host, struct mmc_ios *ios)
{

	printf("%s %d\n", __func__, ios->signal_voltage);

	switch (ios->signal_voltage) {
	case MMC_SIGNAL_VOLTAGE_330:
		printf("%s: 3.3v switch\n", __func__);

		if (host->flags & SDHCI_TYPE_SDCARD) {
			/*
			 * Voltage switching flow (1.8->3.3)
			 * set SDIO0 PAD to 3.0V mode
			 *  =>  0x03000018[5:4] = 2'b01  ,( reg_sd0_ms_ow=1. reg_sd0_ms_sw=0) => MS overwrite to 0 (3.0v mode)
			 * Set PWRSW to 3.3V
			 *  => sd_pwrsw_ctrl (0x030001F4) = 0x00000009 (reg_pwrsw_auto=1,  reg_pwrsw_disc=0,  reg_pwrsw_vsel=0(3.0v),  reg_en_pwrsw=1)
			 * wait 1ms
			*/

			// 0x03000018[5:4] = 2'b01
			// reg_sd0_ms_ow=1
			mmio_setbits_32(0x03000018, 4);
			// reg_sd0_ms_sw=0
			mmio_clrbits_32(0x03000018, 5);

			// sd_pwrsw_ctrl (0x030001F4) = 0x00000009
			mmio_write_32(0x030001F4, 0x00000009);
		}
		else {
			printf("%s emmc only 1.8v\n", __func__);
		}
		/* Wait for 5ms */
#ifndef BOARD_PALLADIUM
		mdelay(5);
#endif
		return 0;
	case MMC_SIGNAL_VOLTAGE_180:
		printf("%s: 1.8v switch\n", __func__);

		if (host->flags & SDHCI_TYPE_SDCARD) {
			/*
			 * Voltage switching flow (3.3→1.8)
			 * Set PWRSW to 1.8V
			 *  => sd_pwrsw_ctrl (0x030001F4) = 0x0000000B (reg_pwrsw_auto=1,  reg_pwrsw_disc=0,  reg_pwrsw_vsel=1(1.8v),  reg_en_pwrsw=1)
			 * set SDIO0 PAD to 1.8V mode
			 *  =>	0x03000018[5:4] = 2'b11  ,( reg_sd0_ms_ow=1. reg_sd0_ms_sw=0) => MS overwrite to 1 (1.8v mode)
			 * wait 1ms
			 */
			// sd_pwrsw_ctrl (0x030001F4) = 0x0000000B
			mmio_write_32(0x030001F4, 0x0000000B);

			// 0x03000018[5:4] = 2'b11
			// reg_sd0_ms_ow=1
			mmio_setbits_32(0x03000018, 4);
			// reg_sd0_ms_sw=1
			mmio_setbits_32(0x03000018, 5);
		}
		else {
			printf("%s emmc only 1.8v\n", __func__);
		}

		return 0;

	case MMC_SIGNAL_VOLTAGE_120:
		printf("%s: 1.2v switch?\n", __func__);
		return -1;
	default:
		/* No signal voltage switch required */
		return 0;
	}
}

static int bm_mmc_card_busy(struct mmc_host *host)
{
	struct bm_mmc_params *param = host->priv;
	uintptr_t base = param->reg_base;
	u32 present_state;

	present_state = mmio_read_32(base + SDHCI_PRESENT_STATE);

	return !(present_state & SDHCI_DATA_0_LVL_MASK);
}

static int bm_mmc_read(int lba, uintptr_t buf, size_t size)
{
	return 0;
}

static int bm_mmc_write(int lba, uintptr_t buf, size_t size)
{
	return 0;
}

int bm_sd_card_detect(struct mmc_host *host)
{
	struct bm_mmc_params *param = host->priv;
	uintptr_t base = param->reg_base;
	u32 reg;

	reg = mmio_read_32(base + SDHCI_PRESENT_STATE);

	if (reg & SDHCI_CARD_INSERTED)
		return 1;
	else
		return 0;
}

int bm_sd_card_lock(struct mmc_host *host)
{
	struct bm_mmc_params *param = host->priv;
	uintptr_t base = param->reg_base;
	u32 reg;

	reg = mmio_read_32(base + SDHCI_PRESENT_STATE);

	if (reg & SDHCI_WR_PROTECT_SW_LVL)
		return 1;
	else
		return 0;
}

static uint8_t g_adma_table[SDHCI_ADMA2_64_DESC_SZ * SDHCI_ADMA2_MAX_DESC_COUNT] __attribute__((aligned (SDHCI_ADMA_ALIGN)));
static void bm_sdhci_adma2_init(struct mmc_host *host)
{
	host->desc_sz = SDHCI_ADMA2_64_DESC_SZ;
	host->adma_table_sz = host->desc_sz * SDHCI_ADMA2_MAX_DESC_COUNT;
	//host->adma_table = malloc(host->adma_table_sz);
	host->adma_table = g_adma_table;
	host->adma_buf_size = 65536;

	if (((uint64_t)host->adma_table) & SDHCI_ADMA_MASK)
		assert(0);

}

static uint8_t g_adma_intgr_tbl[SDHCI_ADMA2_64_DESC_SZ] __attribute__((aligned (SDHCI_ADMA_ALIGN)));
static void bm_sdhci_adma3_init(struct mmc_host *host)
{
	host->intgr_table = g_adma_intgr_tbl;
	bm_sdhci_adma2_init(host);
	bm_sdhci_adma_write_desc(host, host->intgr_table, (uint64_t)host->adma_table, 0, ADMA3_INTEGRATED_END_VALID);

	if (((uint64_t)host->adma_table) & SDHCI_ADMA_MASK)
		assert(0);
}

int bm_mmc_setup_host(struct bm_mmc_params *param, struct mmc_host *host)
{
	u32 pio_irqs = SDHCI_INT_DATA_AVAIL | SDHCI_INT_SPACE_AVAIL;
	u32 dma_irqs = SDHCI_INT_DMA_END | SDHCI_INT_ADMA_ERROR;

	host->priv = param;
	host->ocr_avail = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;
	host->clock = (u32)0xffffffff;
	host->timing = 0xff;
	host->card = NULL;

	host->ios.clock = param->f_min;
	/* Inititate to 1 line */
	host->ios.bus_width = MMC_BUS_WIDTH_1;

	host->ops = &common_ops;
	host->flags = SDHCI_USE_VERSION4;
	if (param->flags & MMC_FLAG_PIO_TRANS) {
		host->ops = &common_ops;
	} else if (param->flags & MMC_FLAG_SDMA_TRANS) {
		host->flags |= SDHCI_USE_SDMA | SDHCI_REQ_USE_DMA;
		host->ops = &common_ops;
	} else if (param->flags & MMC_FLAG_ADMA2_TRANS) {
		host->flags |= SDHCI_USE_ADMA2 | SDHCI_REQ_USE_DMA;
		host->ops = &common_ops;
		bm_sdhci_adma2_init(host);
	} else if (param->flags & MMC_FLAG_ADMA3_TRANS) {
		host->flags |= SDHCI_USE_ADMA3| SDHCI_USE_VERSION4 | SDHCI_REQ_USE_DMA;
		host->ops = &common_ops;
		bm_sdhci_adma3_init(host);
	}

	host->bus_mode = 0;
	host->force_1_8v = 0;
	host->force_bus_width_1 = 0;
	host->force_bus_clock = param->force_trans_freq;
	printf("host->force_bus_clock %d\n", host->force_bus_clock);

	if (param->flags & MMC_FLAG_SDCARD) {
		host->flags |= SDHCI_TYPE_SDCARD;

		if (param->flags & MMC_FLAG_SD_DFLTSPEED) {
			host->bus_mode |= SD_MODE_DFLT_SPEED;
		}
		if (param->flags & MMC_FLAG_SD_HIGHSPEED) {
			host->bus_mode |= SD_MODE_HIGH_SPEED;
		}
		if (param->flags & MMC_FLAG_SD_SDR12) {
			host->bus_mode |= SD_MODE_UHS_SDR12;
			host->force_1_8v = 1;
		}
		if (param->flags & MMC_FLAG_SD_SDR25) {
			host->bus_mode |= SD_MODE_UHS_SDR25;
			host->force_1_8v = 1;
		}
		if (param->flags & MMC_FLAG_SD_SDR50) {
			host->bus_mode |= SD_MODE_UHS_SDR25;
			host->force_1_8v = 1;
		}
		if (param->flags & MMC_FLAG_SD_SDR104) {
			host->bus_mode |= SD_MODE_UHS_SDR104;
			host->force_1_8v = 1;
		}
		if (param->flags & MMC_FLAG_SD_DDR50) {
			host->bus_mode |= SD_MODE_UHS_DDR50;
			host->force_1_8v = 1;
		}

		if (!host->bus_mode) {
			host->bus_mode = SD_MODE_MASK;
		}
	}

	if (param->flags & MMC_FLAG_EMMC) {
		host->flags |= SDHCI_TYPE_EMMC;

		if (param->flags & MMC_FLAG_EMMC_HS200) {
			host->bus_mode |= EXT_CSD_CARD_TYPE_HS200_1_8V;
			host->force_1_8v = 1;
		}
		if (param->flags & MMC_FLAG_EMMC_DDR52) {
			host->bus_mode |= EXT_CSD_CARD_TYPE_DDR_52 | EXT_CSD_CARD_TYPE_HS_52;
			host->force_1_8v = 1;
		}
		if (param->flags & MMC_FLAG_EMMC_SDR52) {
			host->bus_mode |= EXT_CSD_CARD_TYPE_HS_52;
			host->force_1_8v = 1;
		}
		if (param->flags & MMC_FLAG_EMMC_SDR26) {
			host->bus_mode |= EXT_CSD_CARD_TYPE_HS_26;
			host->force_1_8v = 1;
		}

		if (!host->bus_mode) {
			host->bus_mode = EXT_CSD_CARD_TYPE_HS_26 | EXT_CSD_CARD_TYPE_HS_52
					| EXT_CSD_CARD_TYPE_DDR_52 | EXT_CSD_CARD_TYPE_HS200_1_8V;
		}
	}

	if (param->flags & MMC_FLAG_BUS_WIDTH_1) {
		host->force_bus_width_1 = 1;
	}

	if (param->flags & MMC_FLAG_SUPPORT_IRQ) {
		host->flags |= SDHCI_SDIO_IRQ_ENABLED;
		host->ier = SDHCI_INT_BUS_POWER | SDHCI_INT_DATA_END_BIT |
			SDHCI_INT_DATA_CRC | SDHCI_INT_DATA_TIMEOUT |
			SDHCI_INT_INDEX | SDHCI_INT_END_BIT | SDHCI_INT_CRC |
			SDHCI_INT_TIMEOUT | SDHCI_INT_DATA_END | SDHCI_INT_RESPONSE;

		if (host->flags & SDHCI_REQ_USE_DMA)
			host->ier = (host->ier & ~pio_irqs) | dma_irqs;
		else
			host->ier = (host->ier & ~dma_irqs) | pio_irqs;

	} else {
		host->ier = 0;
	}

	return 0;
}

