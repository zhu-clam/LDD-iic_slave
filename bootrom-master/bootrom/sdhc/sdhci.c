/*
 * (Copyright 2017) Verisilicon Inc. All Rights Reserved
 * Company confidential and Proprietary information.
 * This information may not be disclosed to unauthorized individual.
 * 
 * System Application Team <fengyin.wu@verisilicon.com>
 *
 */


#include "datatype.h"
#include "uart.h"
#include "platform.h"
#include "common.h"
#include "spi.h"
#include "sdhci.h"

//#define SDHCI_DB
#ifdef SDHCI_DB
#define sdhci_db(format, ...)  debug(format,##__VA_ARGS__)
#else
#define sdhci_db(format, ...)  do {} while (0)
#endif

static inline void sdhci_writel(struct sdhci_host *host, u32 val, int reg)
{
	write_mreg32(host->ioaddr + reg, val);
}

static inline void sdhci_writew(struct sdhci_host *host, u16 val, int reg)
{
	write_mreg16(host->ioaddr + reg, val);
}

static inline void sdhci_writeb(struct sdhci_host *host, u8 val, int reg)
{
	write_mreg8(host->ioaddr + reg, val);
}
static inline u32 sdhci_readl(struct sdhci_host *host, int reg)
{
	return read_mreg32(host->ioaddr + reg);
}

static inline u16 sdhci_readw(struct sdhci_host *host, int reg)
{
	return read_mreg16(host->ioaddr + reg);
}

static inline u8 sdhci_readb(struct sdhci_host *host, int reg)
{
	return read_mreg8(host->ioaddr + reg);
}

static void sdhci_reset(struct sdhci_host *host, u8 mask)
{
	unsigned long timeout;

	/* Wait max 100 ms */
	timeout = 100;
	sdhci_writeb(host, mask, SDHCI_SOFTWARE_RESET);
	while (sdhci_readb(host, SDHCI_SOFTWARE_RESET) & mask) {
		if (timeout == 0) {
			debug("%s: Reset 0x%x never completed.\n",
			       __func__, (int)mask);
			return;
		}
		timeout--;
		usleep(1000);
	}
}

static void sdhci_cmd_done(struct sdhci_host *host, struct mmc_cmd *cmd)
{
	int i;
	if (cmd->resp_type & MMC_RSP_136) {
		/* CRC is stripped so we need to do some shifting. */
		for (i = 0; i < 4; i++) {
			cmd->response[i] = sdhci_readl(host,
					SDHCI_RESPONSE + (3-i)*4) << 8;
			if (i != 3)
				cmd->response[i] |= sdhci_readb(host,
						SDHCI_RESPONSE + (3-i)*4-1);
		}
	} else {
		cmd->response[0] = sdhci_readl(host, SDHCI_RESPONSE);
	}
}

static void sdhci_transfer_pio(struct sdhci_host *host, struct mmc_data *data)
{
	int i;
	char *offs;
	for (i = 0; i < data->blocksize; i += 4) {
		if (data->flags == MMC_DATA_READ){
			offs = data->dest + i;
			*(u32 *)offs = sdhci_readl(host, SDHCI_BUFFER);
		}	
		else{
			offs = data->src + i;
			sdhci_writel(host, *(u32 *)offs, SDHCI_BUFFER);
		}
	}
}

static int sdhci_transfer_data(struct sdhci_host *host, struct mmc_data *data,
				unsigned int start_addr)
{
	unsigned int stat, rdy, mask, timeout, block = 0;
	bool transfer_done = false;
#ifdef CONFIG_MMC_SDHCI_SDMA
	unsigned char ctrl;
	ctrl = sdhci_readb(host, SDHCI_HOST_CONTROL);
	ctrl &= ~SDHCI_CTRL_DMA_MASK;
	sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);
#endif

	timeout = 1000000;
	rdy = SDHCI_INT_SPACE_AVAIL | SDHCI_INT_DATA_AVAIL;
	mask = SDHCI_DATA_AVAILABLE | SDHCI_SPACE_AVAILABLE;
	do {
		stat = sdhci_readl(host, SDHCI_INT_STATUS);
		if (stat & SDHCI_INT_ERROR) {
			debug("%s: Error detected in status(0x%X)!\n",
				 __func__, stat);
			return -EIO;
		}
//		stat = 0xffffffff;
		if (!transfer_done && (stat & rdy)) {
			if (!(sdhci_readl(host, SDHCI_PRESENT_STATE) & mask))
				continue;
			sdhci_writel(host, rdy, SDHCI_INT_STATUS);
			sdhci_transfer_pio(host, data);
			if (data->flags == MMC_DATA_READ)
				data->dest += data->blocksize;
			else if (data->flags == MMC_DATA_WRITE)
				data->src += data->blocksize;
			if (++block >= data->blocks) {
				/* Keep looping until the SDHCI_INT_DATA_END is
				 * cleared, even if we finished sending all the
				 * blocks.
				 */
				transfer_done = true;
				continue;
			}
		}
#ifdef CONFIG_MMC_SDHCI_SDMA
		if (!transfer_done && (stat & SDHCI_INT_DMA_END)) {
			sdhci_writel(host, SDHCI_INT_DMA_END, SDHCI_INT_STATUS);
			start_addr &= ~(SDHCI_DEFAULT_BOUNDARY_SIZE - 1);
			start_addr += SDHCI_DEFAULT_BOUNDARY_SIZE;
			sdhci_writel(host, start_addr, SDHCI_DMA_ADDRESS);
		}
#endif
		if (timeout-- > 0)
			usleep(10);
		else {
			debug("%s: Transfer data timeout\n", __func__);
			return -ETIMEDOUT;
		}
	} while (!(stat & SDHCI_INT_DATA_END));
	return 0;
}

/*
 * No command will be sent by driver if card is busy, so driver must wait
 * for card ready state.
 * Every time when card is busy after timeout then (last) timeout value will be
 * increased twice but only if it doesn't exceed global defined maximum.
 * Each function call will use last timeout value.
 */
#define SDHCI_CMD_MAX_TIMEOUT			3200
#define SDHCI_CMD_DEFAULT_TIMEOUT		1000
#define SDHCI_READ_STATUS_TIMEOUT		1000
int sdhci_send_command(struct mmc *mmc, struct mmc_cmd *cmd,
			      struct mmc_data *data)
{
	struct sdhci_host *host = mmc->priv;
	unsigned int stat = 0;
	int ret = 0;
	int trans_bytes = 0;
//	int is_aligned = 1;
	u32 mask, flags, mode;
	unsigned int time = 0, start_addr = 0;
	int mmc_dev = 0;//mmc_get_blk_desc(mmc)->devnum;
	unsigned start = 0;//get_timer(0);

	/* Timeout unit - ms */
	unsigned int cmd_timeout = SDHCI_CMD_DEFAULT_TIMEOUT;

	mask = SDHCI_CMD_INHIBIT | SDHCI_DATA_INHIBIT;

	/* We shouldn't wait for data inihibit for stop commands, even
	   though they might use busy signaling */
	if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION)
		mask &= ~SDHCI_DATA_INHIBIT;

	while (sdhci_readl(host, SDHCI_PRESENT_STATE) & mask) {
		if (time >= cmd_timeout) {
			debug("%s: MMC: %d busy ", __func__, mmc_dev);
			if (2 * cmd_timeout <= SDHCI_CMD_MAX_TIMEOUT) {
				cmd_timeout += cmd_timeout;
				debug("timeout increasing to: %u ms.\n",
				       cmd_timeout);
			} else {
				debug("timeout.\n");
				return -ECOMM;
			}
		}
		time++;
		usleep(1000);
	}

	sdhci_writel(host, SDHCI_INT_ALL_MASK, SDHCI_INT_STATUS);

	mask = SDHCI_INT_RESPONSE;
	if (!(cmd->resp_type & MMC_RSP_PRESENT))
		flags = SDHCI_CMD_RESP_NONE;
	else if (cmd->resp_type & MMC_RSP_136)
		flags = SDHCI_CMD_RESP_LONG;
	else if (cmd->resp_type & MMC_RSP_BUSY) {
		flags = SDHCI_CMD_RESP_SHORT_BUSY;
		if (data)
			mask |= SDHCI_INT_DATA_END;
	} else
		flags = SDHCI_CMD_RESP_SHORT;

	if (cmd->resp_type & MMC_RSP_CRC)
		flags |= SDHCI_CMD_CRC;
	if (cmd->resp_type & MMC_RSP_OPCODE)
		flags |= SDHCI_CMD_INDEX;
	if (data)
		flags |= SDHCI_CMD_DATA;

	/* Set Transfer mode regarding to data flag */
	if (data) {
		sdhci_writeb(host, 0xe, SDHCI_TIMEOUT_CONTROL);
		mode = SDHCI_TRNS_BLK_CNT_EN;
		trans_bytes = data->blocks * data->blocksize;
		if (data->blocks > 1)
			mode |= SDHCI_TRNS_MULTI;

		if (data->flags == MMC_DATA_READ)
			mode |= SDHCI_TRNS_READ;

#ifdef CONFIG_MMC_SDHCI_SDMA
		if (data->flags == MMC_DATA_READ)
			start_addr = (unsigned long)data->dest;
		else
			start_addr = (unsigned long)data->src;

		sdhci_writel(host, start_addr, SDHCI_DMA_ADDRESS);
		mode |= SDHCI_TRNS_DMA;
#endif
		sdhci_writew(host, SDHCI_MAKE_BLKSZ(SDHCI_DEFAULT_BOUNDARY_ARG,
				data->blocksize),
				SDHCI_BLOCK_SIZE);
		sdhci_writew(host, data->blocks, SDHCI_BLOCK_COUNT);
		sdhci_writew(host, mode, SDHCI_TRANSFER_MODE);
	} else if (cmd->resp_type & MMC_RSP_BUSY) {
		sdhci_writeb(host, 0xe, SDHCI_TIMEOUT_CONTROL);
	}

	sdhci_writel(host, cmd->cmdarg, SDHCI_ARGUMENT);
#ifdef CONFIG_MMC_SDHCI_SDMA
	if (data) {
		trans_bytes = ALIGN(trans_bytes, CONFIG_SYS_CACHELINE_SIZE);
//		flush_cache(start_addr, trans_bytes);
	}
#endif
	sdhci_writew(host, SDHCI_MAKE_CMD(cmd->cmdidx, flags), SDHCI_COMMAND);
	start = 0;//get_timer(0);
	do {
		stat = sdhci_readl(host, SDHCI_INT_STATUS);
		if (stat & SDHCI_INT_ERROR)
			break;

		if (start >= SDHCI_READ_STATUS_TIMEOUT) {
			if (host->quirks & SDHCI_QUIRK_BROKEN_R1B) {
				return 0;
			} else {
				debug("%s: Timeout for status update!\n",
				       __func__);
				return -ETIMEDOUT;
			}
		}
		
		start++;
		usleep(1000);
	} while ((stat & mask) != mask);

	if ((stat & (SDHCI_INT_ERROR | mask)) == mask) {
		sdhci_cmd_done(host, cmd);
		sdhci_writel(host, mask, SDHCI_INT_STATUS);
	} else
		ret = -1;

	if (!ret && data)
		ret = sdhci_transfer_data(host, data, start_addr);

	if (host->quirks & SDHCI_QUIRK_WAIT_SEND_CMD)
		usleep(1000);

	stat = sdhci_readl(host, SDHCI_INT_STATUS);
	sdhci_writel(host, SDHCI_INT_ALL_MASK, SDHCI_INT_STATUS);
	if (!ret) {
		return 0;
	}

	sdhci_reset(host, SDHCI_RESET_CMD);
	sdhci_reset(host, SDHCI_RESET_DATA);
	if (stat & SDHCI_INT_TIMEOUT)
		return -ETIMEDOUT;
	else
		return -ECOMM;
}

static int sdhci_set_clock(struct mmc *mmc, unsigned int clock)
{
	struct sdhci_host *host = mmc->priv;
	unsigned int div, clk = 0, timeout;

	/* Wait max 20 ms */
	timeout = 200;
	while (sdhci_readl(host, SDHCI_PRESENT_STATE) &
			   (SDHCI_CMD_INHIBIT | SDHCI_DATA_INHIBIT)) {
		if (timeout == 0) {
			debug("%s: Timeout to wait cmd & data inhibit\n",
			       __func__);
			return -EBUSY;
		}

		timeout--;
		usleep(100);
	}

	sdhci_writew(host, 0, SDHCI_CLOCK_CONTROL);

	if (clock == 0)
		return 0;

	if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300) {
		/*
		 * Check if the Host Controller supports Programmable Clock
		 * Mode.
		 */
		if (host->clk_mul) {
			for (div = 1; div <= 1024; div++) {
				if ((host->max_clk / div) <= clock)
					break;
			}

			/*
			 * Set Programmable Clock Mode in the Clock
			 * Control register.
			 */
			clk = SDHCI_PROG_CLOCK_MODE;
			div--;
		} else {
			/* Version 3.00 divisors must be a multiple of 2. */
			if (host->max_clk <= clock) {
				div = 1;
			} else {
				for (div = 2;
				     div < SDHCI_MAX_DIV_SPEC_300;
				     div += 2) {
					if ((host->max_clk / div) <= clock)
						break;
				}
			}
			div >>= 1;
		}
	} else {
		/* Version 2.00 divisors must be a power of 2. */
		for (div = 1; div < SDHCI_MAX_DIV_SPEC_200; div *= 2) {
			if ((host->max_clk / div) <= clock)
				break;
		}
		div >>= 1;
	}

	if (host->ops && host->ops->set_clock)//////////////////////////maybe  need
		host->ops->set_clock(host, div);

	clk |= (div & SDHCI_DIV_MASK) << SDHCI_DIVIDER_SHIFT;
	clk |= ((div & SDHCI_DIV_HI_MASK) >> SDHCI_DIV_MASK_LEN)
		<< SDHCI_DIVIDER_HI_SHIFT;
	clk |= SDHCI_CLOCK_INT_EN;
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

	/* Wait max 20 ms */
	timeout = 20;
	while (!((clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL))
		& SDHCI_CLOCK_INT_STABLE)) {
		if (timeout == 0) {
			debug("%s: Internal clock never stabilised.\n",
			       __func__);
#if 0
			return -EBUSY;
#else
			break;
#endif
		}
		timeout--;
		usleep(1000);
	}

	clk |= SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);
	clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);

	return 0;
}

static void sdhci_set_power(struct sdhci_host *host, unsigned short power)
{
	u8 pwr = 0;

	if (power != (unsigned short)-1) {
		switch (1 << power) {
		case MMC_VDD_165_195:
			pwr = SDHCI_POWER_180;
//			debug("set power in 1.8V \n");
			break;
		case MMC_VDD_29_30:
		case MMC_VDD_30_31:
			pwr = SDHCI_POWER_300;
//			debug("set power in 3.0V \n");
			break;
		case MMC_VDD_32_33:
		case MMC_VDD_33_34:
			pwr = SDHCI_POWER_330;
//			debug("set power in 3.3V \n");
			break;
		}
	}

	if (pwr == 0) {
		sdhci_writeb(host, 0, SDHCI_POWER_CONTROL);
		return;
	}

	pwr |= SDHCI_POWER_ON;

	sdhci_writeb(host, pwr, SDHCI_POWER_CONTROL);
}

int sdhci_set_ios(struct mmc *mmc)
{
	u32 ctrl;
	struct sdhci_host *host = mmc->priv;

	if (host->ops && host->ops->set_control_reg)
		host->ops->set_control_reg(host);

	sdhci_db("clock = %d \n",mmc->clock);
	if (mmc->clock != host->clock)
		sdhci_set_clock(mmc, mmc->clock);

	/* Set bus width */
	ctrl = sdhci_readb(host, SDHCI_HOST_CONTROL);
	sdhci_db("bus_width = %d \n",mmc->bus_width);
	if (mmc->bus_width == 8) {
		ctrl &= ~SDHCI_CTRL_4BITBUS;
		if ((SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300) ||
				(host->quirks & SDHCI_QUIRK_USE_WIDE8))
			ctrl |= SDHCI_CTRL_8BITBUS;
	} else {
		if ((SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300) ||
				(host->quirks & SDHCI_QUIRK_USE_WIDE8))
			ctrl &= ~SDHCI_CTRL_8BITBUS;
		if (mmc->bus_width == 4)
			ctrl |= SDHCI_CTRL_4BITBUS;
		else
			ctrl &= ~SDHCI_CTRL_4BITBUS;
	}

	if (mmc->clock > 26000000)
		ctrl |= SDHCI_CTRL_HISPD;
	else
		ctrl &= ~SDHCI_CTRL_HISPD;

	if (host->quirks & SDHCI_QUIRK_NO_HISPD_BIT)
		ctrl &= ~SDHCI_CTRL_HISPD;

	sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);
	
	/* If available, call the driver specific "post" set_ios() function */
	if (host->ops && host->ops->set_ios_post)
		host->ops->set_ios_post(host);

	return 0;
}

static  int fls(int x)
{
	int r = 32;

	if (!x)
		return 0;
	if (!(x & 0xffff0000u)) {
		x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff000000u)) {
		x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf0000000u)) {
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u)) {
		x <<= 2;
		r -= 2;
	}
	if (!(x & 0x80000000u)) {
		x <<= 1;
		r -= 1;
	}
	return r;
}

int sdhci_init(struct mmc *mmc)
{
	struct sdhci_host *host = mmc->priv;
	
	sdhci_reset(host, SDHCI_RESET_ALL);

	sdhci_set_power(host, fls(mmc->cfg->voltages) - 1);//************************************

	if (host->ops && host->ops->get_cd)
		host->ops->get_cd(host);

	/* Enable only interrupts served by the SD controller */
	sdhci_writel(host, SDHCI_INT_DATA_MASK | SDHCI_INT_CMD_MASK,
		     SDHCI_INT_ENABLE);
	
	/* Mask all sdhci interrupt sources */
	sdhci_writel(host, 0x0, SDHCI_SIGNAL_ENABLE);

	return 0;
}

#if 0
static const struct mmc_ops sdhci_ops = {
	.send_cmd	= sdhci_send_command,
	.set_ios	= sdhci_set_ios,
	.init		= sdhci_init,
};
#endif

int sdhci_setup_cfg(struct mmc_config *cfg, struct sdhci_host *host,
		u32 f_max, u32 f_min)
{
	u32 caps, caps_1;

	caps = sdhci_readl(host, SDHCI_CAPABILITIES);

#ifdef CONFIG_MMC_SDHCI_SDMA
	if (!(caps & SDHCI_CAN_DO_SDMA)) {
		debug("%s: Your controller doesn't support SDMA!!\n",
		       __func__);
		return -EINVAL;
	}
#endif
	if (host->quirks & SDHCI_QUIRK_REG32_RW)
		host->version =
			sdhci_readl(host, SDHCI_HOST_VERSION - 2) >> 16;
	else
		host->version = sdhci_readw(host, SDHCI_HOST_VERSION);

	cfg->name = host->name;
#ifndef CONFIG_DM_MMC
//	cfg->ops = &sdhci_ops;
#endif

	/* Check whether the clock multiplier is supported or not */
	if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300) {
		caps_1 = sdhci_readl(host, SDHCI_CAPABILITIES_1);
		host->clk_mul = (caps_1 & SDHCI_CLOCK_MUL_MASK) >>
				SDHCI_CLOCK_MUL_SHIFT;
	}

	if (host->max_clk == 0) {
		if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300)
			host->max_clk = (caps & SDHCI_CLOCK_V3_BASE_MASK) >>
				SDHCI_CLOCK_BASE_SHIFT;
		else
			host->max_clk = (caps & SDHCI_CLOCK_BASE_MASK) >>
				SDHCI_CLOCK_BASE_SHIFT;
		host->max_clk *= 1000000;
		if (host->clk_mul)
			host->max_clk *= host->clk_mul;
	}
	if (host->max_clk == 0) {
		debug("%s: Hardware doesn't specify base clock frequency\n",
		       __func__);
		return -EINVAL;
	}
	if (f_max && (f_max < host->max_clk))
		cfg->f_max = f_max;
	else
		cfg->f_max = host->max_clk;
	if (f_min)
		cfg->f_min = f_min;
	else {
		if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300)
			cfg->f_min = cfg->f_max / SDHCI_MAX_DIV_SPEC_300;
		else
			cfg->f_min = cfg->f_max / SDHCI_MAX_DIV_SPEC_200;
	}
	cfg->voltages = 0;
	if (caps & SDHCI_CAN_VDD_330)
		cfg->voltages |= MMC_VDD_32_33 | MMC_VDD_33_34;
	if (caps & SDHCI_CAN_VDD_300)
		cfg->voltages |= MMC_VDD_29_30 | MMC_VDD_30_31;
	if (caps & SDHCI_CAN_VDD_180)
		cfg->voltages |= MMC_VDD_165_195;

	if (host->quirks & SDHCI_QUIRK_BROKEN_VOLTAGE)
		cfg->voltages |= host->voltages;

	cfg->host_caps |= MMC_MODE_HS ;
	
	/* Since Host Controller Version3.0 */
	if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300) {
		if (!(caps & SDHCI_CAN_DO_8BIT))
			cfg->host_caps &= ~MMC_MODE_8BIT;
	}

	if (host->host_caps)
		cfg->host_caps |= host->host_caps;

	cfg->b_max = MMC_MAX_BLK_COUNT;

	return 0;
}



