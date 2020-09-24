/*****************************************************************************
 *  File: syns_mmc.c
 *
 *  Descirption: contains the functions support Synopsys DesignWare Cores
 *               Mobile Storage Host Controller - DWC_mshc – Product Code: A555-0.
 *
 *  Copyright (C) : 2018 VeriSilicon.
 *
 *  Author: Chunming Li
 *  Mail:   chunming.li@verisilicon.com
 *  Date:   Mar 5 2018
 *
 *****************************************************************************/

/* UG: DesignWare Cores Mobile Storage Host Controller 1.70a */

#include "syns_mmc.h"
#include "datatype.h"
#include "misc.h"

void sdhci_writel(struct sdhci_host *host, u32 val, int reg)
{
    write_mreg32(host->ioaddr + reg, val);
}

void sdhci_writew(struct sdhci_host *host, u16 val, int reg)
{
    write_mreg16(host->ioaddr + reg, val);
}

void sdhci_writeb(struct sdhci_host *host, u8 val, int reg)
{
    write_mreg8(host->ioaddr + reg, val);
}

u32 sdhci_readl(struct sdhci_host *host, int reg)
{
    return read_mreg32(host->ioaddr + reg);
}

u16 sdhci_readw(struct sdhci_host *host, int reg)
{
    return read_mreg16(host->ioaddr + reg);
}

u8 sdhci_readb(struct sdhci_host *host, int reg)
{
    return read_mreg8(host->ioaddr + reg);
}

void sdhci_isr() {
    // TODO
}

void sdhci_reset(struct sdhci_host *host, u8 mask)
{
	unsigned long timeout;

	/* Wait max 100 ms */
	timeout = 100;
	sdhci_writeb(host, mask, SDHCI_SOFTWARE_RESET);
	while (sdhci_readb(host, SDHCI_SOFTWARE_RESET) & mask) {
		if (timeout == 0) {
			printf("%s: Reset 0x%x never completed.\n",
			       __func__, (int)mask);
			return;
		}
		timeout--;
		udelay(1000);
	}
}

void sdhci_cmd_done(struct sdhci_host *host, struct mmc_cmd *cmd)
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

/*
 * No command will be sent by driver if card is busy, so driver must wait
 * for card ready state.
 * Every time when card is busy after timeout then (last) timeout value will be
 * increased twice but only if it doesn't exceed global defined maximum.
 * Each function call will use last timeout value. Max timeout can be redefined
 * in board config file.
 */
#ifndef CONFIG_SDHCI_CMD_MAX_TIMEOUT
#define CONFIG_SDHCI_CMD_MAX_TIMEOUT		3200
#endif
#define CONFIG_SDHCI_CMD_DEFAULT_TIMEOUT	100
#define SDHCI_READ_STATUS_TIMEOUT		1000

int sdhci_send_command(struct sdhci_host *host, struct mmc_cmd *cmd,
		       u32 flags)
{
	unsigned int stat = 0;
	int ret = 0;
	u32 mask;
	unsigned int time = 0;
	unsigned start;

	/* Timeout unit - ms */
	unsigned int cmd_timeout = CONFIG_SDHCI_CMD_DEFAULT_TIMEOUT;

	sdhci_writel(host, SDHCI_INT_ALL_MASK, SDHCI_INT_STATUS);
	mask = SDHCI_CMD_INHIBIT | SDHCI_DATA_INHIBIT;

	/* We shouldn't wait for data inihibit for stop commands, even
	   though they might use busy signaling */
	if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION)
		mask &= ~SDHCI_DATA_INHIBIT;

	while (sdhci_readl(host, SDHCI_PRESENT_STATE) & mask) {
		if (time >= cmd_timeout) {
			printf("%s: MMC: %d busy ", __func__, host->id);
			if (2 * cmd_timeout <= CONFIG_SDHCI_CMD_MAX_TIMEOUT) {
				cmd_timeout += cmd_timeout;
				printf("timeout increasing to: %u ms.\n",
				       cmd_timeout);
			} else {
				puts("timeout.\n");
				return COMM_ERR;
			}
		}
		time++;
		udelay(100);
	}

	mask = SDHCI_INT_RESPONSE;
	if (!(cmd->resp_type & MMC_RSP_PRESENT))
		flags |= SDHCI_CMD_RESP_NONE;
	else if (cmd->resp_type & MMC_RSP_136)
		flags |= SDHCI_CMD_RESP_LONG;
	else if (cmd->resp_type & MMC_RSP_BUSY) {
		flags |= SDHCI_CMD_RESP_SHORT_BUSY;
		mask |= SDHCI_INT_DATA_END;
	} else
		flags |= SDHCI_CMD_RESP_SHORT;

	if (cmd->resp_type & MMC_RSP_CRC)
		flags |= SDHCI_CMD_CRC;
	if (cmd->resp_type & MMC_RSP_OPCODE)
		flags |= SDHCI_CMD_INDEX;

	sdhci_writel(host, cmd->cmdarg, SDHCI_ARGUMENT);

#ifdef CONFIG_MMC_SDMA
	flush_cache(start_addr, trans_bytes);
#endif
	sdhci_writew(host, SDHCI_MAKE_CMD(cmd->cmdidx, flags), SDHCI_COMMAND);

	start = 0;
	do {
		stat = sdhci_readl(host, SDHCI_INT_STATUS);
		if (stat & SDHCI_INT_ERROR)
			break;
        start += 1;
        udelay(1000);
	} while (((stat & mask) != mask) &&
		 (start < SDHCI_READ_STATUS_TIMEOUT));

	if (start >= SDHCI_READ_STATUS_TIMEOUT) {
		if (host->quirks & SDHCI_QUIRK_BROKEN_R1B)
			return 0;
		else {
			printf("%s: Timeout for status update!\n", __func__);
			return TIMEOUT;
		}
	}
#if CK_SDIO_DEBUG
    printf("[%s:%d] stat=0x%x, mask=0x%x\n", __FUNCTION__, __LINE__, stat, mask);
#endif
	if ((stat & (SDHCI_INT_ERROR | mask)) == mask) {
		sdhci_cmd_done(host, cmd);
		sdhci_writel(host, mask, SDHCI_INT_STATUS);
	} else
		ret = -1;

	if (host->quirks & SDHCI_QUIRK_WAIT_SEND_CMD)
		udelay(1000);

	if (!ret) {
		return 0;
	}

	sdhci_reset(host, SDHCI_RESET_CMD);
	sdhci_reset(host, SDHCI_RESET_DATA);
	if (stat & SDHCI_INT_TIMEOUT)
		return TIMEOUT;
	else
		return COMM_ERR;
}

int sdhci_set_clock(struct sdhci_host *host, unsigned int clock)
{
	unsigned int div, clk, timeout, reg;

	/* Wait max 20 ms */
	timeout = 200;
	while (sdhci_readl(host, SDHCI_PRESENT_STATE) &
			   (SDHCI_CMD_INHIBIT | SDHCI_DATA_INHIBIT)) {
		if (timeout == 0) {
			printf("%s: Timeout to wait cmd & data inhibit\n",
			       __func__);
			return -1;
		}

		timeout--;
		udelay(100);
	}

	reg = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	reg &= ~SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, reg, SDHCI_CLOCK_CONTROL);

	if (clock == 0)
		return 0;

	if (host->version >= SDHCI_SPEC_300) {
		/* Version 3.00 divisors must be a multiple of 2. */
		if (host->f_max <= clock)
			div = 1;
		else {
			for (div = 2; div < SDHCI_MAX_DIV_SPEC_300; div += 2) {
				if ((host->f_max / div) <= clock)
					break;
			}
		}
	} else {
		/* Version 2.00 divisors must be a power of 2. */
		for (div = 1; div < SDHCI_MAX_DIV_SPEC_200; div *= 2) {
			if ((host->f_max / div) <= clock)
				break;
		}
	}
	div >>= 1;

	clk = (div & SDHCI_DIV_MASK) << SDHCI_DIVIDER_SHIFT;
	clk |= ((div & SDHCI_DIV_HI_MASK) >> SDHCI_DIV_MASK_LEN)
		<< SDHCI_DIVIDER_HI_SHIFT;
	clk |= SDHCI_CLOCK_INT_EN;
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

	/* Wait max 150 ms */
	timeout = 150;
	while (!((clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL))
		& SDHCI_CLOCK_INT_STABLE)) {
		if (timeout == 0) {
            printf("JJJ_DEBUG 0x00 CLOCK_CONTROL=0x%x\n",
			       sdhci_readw(host, SDHCI_CLOCK_CONTROL));
			printf("%s: Internal clock never stabilized.\n",
			       __func__);
			//return -1;
            break;
		}
		timeout--;
		udelay(1000);
	}

	clk |= SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

    // Set MAX timeout value
    sdhci_writeb(host, 0xe, SDHCI_TIMEOUT_CONTROL);

	return 0;
}

void sdhci_set_ios(struct sdhci_host *host)
{
	u32 ctrl;

    sdhci_set_clock(host, host->clock);

	/* Set default bus width to 1bit*/
	ctrl = sdhci_readb(host, SDHCI_HOST_CONTROL);
    ctrl &= ~SDHCI_CTRL_4BITBUS;

	if (host->clock > 26000000)
		ctrl |= SDHCI_CTRL_HISPD;
	else
		ctrl &= ~SDHCI_CTRL_HISPD;

	if (host->quirks & SDHCI_QUIRK_NO_HISPD_BIT)
		ctrl &= ~SDHCI_CTRL_HISPD;

	sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);

#if CK_SDIO_DEBUG
        printf("[%s:%d] ctrl = 0x%x.\n", __FUNCTION__, __LINE__,
                sdhci_readb(host, SDHCI_HOST_CONTROL));
#endif
}

int sdhci_init(struct sdhci_host *host)
{
    /* Step 4 UG - P127 Figure 4-22 SD Bus Power Control Sequence */
    if ((host->caps & SDHCI_CAN_VDD_330) == SDHCI_CAN_VDD_330) {
#if CK_SDIO_DEBUG
        printf("Set VDD1 as 3.3V.\n");
#endif
        sdhci_writeb(host, SDHCI_POWER_330 | SDHCI_POWER_ON, SDHCI_POWER_CONTROL);
    } else if ((host->caps & SDHCI_CAN_VDD_300) == SDHCI_CAN_VDD_300) {
#if CK_SDIO_DEBUG
        printf("Set VDD1 as 3.0V.\n");
#endif
        sdhci_writeb(host, SDHCI_POWER_300 | SDHCI_POWER_ON, SDHCI_POWER_CONTROL);
    } else if ((host->caps & SDHCI_CAN_VDD_180) == SDHCI_CAN_VDD_180) {
#if CK_SDIO_DEBUG
        printf("Set VDD1 as 1.8V.\n");
#endif
        sdhci_writeb(host, SDHCI_POWER_180 | SDHCI_POWER_ON, SDHCI_POWER_CONTROL);
    } else {
        printf("No support Voltage.\n");
        return FAILURE;
    }

	/* Enable only interrupts status served by the SD controller */
	sdhci_writel(host, SDHCI_INT_DATA_MASK | SDHCI_INT_CMD_MASK | SDHCI_INT_CARD_REMOVE | SDHCI_INT_CARD_INSERT,
		     SDHCI_INT_ENABLE);
	/* Mask all sdhci interrupt sources except Card insert and remove*/
	sdhci_writel(host, SDHCI_INT_CARD_INSERT | SDHCI_INT_CARD_REMOVE, SDHCI_SIGNAL_ENABLE);

	return 0;
}

int mmc_send_cmd(struct sdhci_host *host, struct mmc_cmd *cmd, u32 flags)
{
	int ret;

#ifdef CONFIG_MMC_TRACE
	int i;
	u8 *ptr;

	printf("CMD_SEND:%d\n", cmd->cmdidx);
	printf("\t\tARG\t\t\t 0x%x\n", cmd->cmdarg);
	ret = sdhci_send_command(host, cmd, flags);
	if (ret) {
		printf("\t\tRET\t\t\t %d\n", ret);
	} else {
		switch (cmd->resp_type) {
		case MMC_RSP_NONE:
			printf("\t\tMMC_RSP_NONE\n");
			break;
		case MMC_RSP_R1:
			printf("\t\tMMC_RSP_R1,5,6,7 \t 0x%x \n",
				cmd->response[0]);
			break;
		case MMC_RSP_R1b:
			printf("\t\tMMC_RSP_R1b\t\t 0x%x \n",
				cmd->response[0]);
			break;
		case MMC_RSP_R2:
			printf("\t\tMMC_RSP_R2\t\t 0x%x \n",
				cmd->response[0]);
			printf("\t\t          \t\t 0x%x \n",
				cmd->response[1]);
			printf("\t\t          \t\t 0x%x \n",
				cmd->response[2]);
			printf("\t\t          \t\t 0x%x \n",
				cmd->response[3]);
			printf("\n");
			printf("\t\t\t\t\tDUMPING DATA\n");
			for (i = 0; i < 4; i++) {
				int j;
				printf("\t\t\t\t\t%03d - ", i*4);
				ptr = (u8 *)&cmd->response[i];
				ptr += 3;
				for (j = 0; j < 4; j++)
					printf("%02X ", *ptr--);
				printf("\n");
			}
			break;
		case MMC_RSP_R3:
			printf("\t\tMMC_RSP_R3,4\t\t 0x%x \n",
				cmd->response[0]);
			break;
		default:
			printf("\t\tERROR MMC rsp not supported\n");
			break;
		}
	}
#else
	ret = sdhci_send_command(host, cmd, flags);
#endif
	return ret;
}

int mmc_go_idle(struct sdhci_host *host)
{
	struct mmc_cmd cmd;
	int err;

	udelay(1000);

	cmd.cmdidx = MMC_CMD_GO_IDLE_STATE;
	cmd.cmdarg = 0;
	cmd.resp_type = MMC_RSP_NONE;

	err = mmc_send_cmd(host, &cmd, 0);

	if (err)
		return err;

	udelay(2000);

	return 0;
}

int mmc_send_op_cond(struct sdhci_host *host)
{
	int err, i;

    /* Some cards seem to need this */
	//mmc_go_idle(host);

 	/* Asking to the card its capabilities */
	for (i = 0; i < 1; i++) {
		err = mmc_send_op_cond_iter(host, i != 0);
		if (err)
			return err;

		/* exit if not busy (flag seems to be inverted) */
		if (host->ocr & OCR_BUSY)
			break;
	}
	host->op_cond_pending = 1;
	return 0;
}

int mmc_send_op_cond_iter(struct sdhci_host *host, int use_arg)
{
	struct mmc_cmd cmd;
	int err;

	cmd.cmdidx = MMC_CMD_SEND_OP_COND;
	cmd.resp_type = MMC_RSP_R3;
	cmd.cmdarg = 0;
	if (use_arg)
		cmd.cmdarg = OCR_HCS |
			(host->voltages &
			(host->ocr & OCR_VOLTAGE_MASK)) |
			(host->ocr & OCR_ACCESS_MODE);

	err = mmc_send_cmd(host, &cmd, 0);
	if (err)
		return err;
	host->ocr = cmd.response[0];
	return 0;
}

int sd_send_op_cond(struct sdhci_host *host)
{
	int timeout = 1000;
	int err;
	struct mmc_cmd cmd;

	while (1) {
		cmd.cmdidx = MMC_CMD_APP_CMD;
		cmd.resp_type = MMC_RSP_R1;
		cmd.cmdarg = 0;

		err = mmc_send_cmd(host, &cmd, 0);

		if (err)
			return err;

		cmd.cmdidx = SD_CMD_APP_SEND_OP_COND;
        cmd.cmdarg = 0x40008000;
		cmd.resp_type = MMC_RSP_R3;

		if (host->version == SDHCI_SPEC_200)
			cmd.cmdarg |= OCR_HCS;

		err = mmc_send_cmd(host, &cmd, 0);

		if (err)
			return err;

		if (cmd.response[0] & OCR_BUSY)
			break;

		if (timeout-- <= 0)
			return UNUSABLE_ERR;

		udelay(1000);
	}

	if (host->version != SDHCI_SPEC_200)
		host->version = SDHCI_SPEC_100;

	host->ocr = cmd.response[0];

	host->high_capacity = ((host->ocr & OCR_HCS) == OCR_HCS);
	host->rca = 0;

	return 0;
}

int mmc_send_if_cond(struct sdhci_host *host)
{
	struct mmc_cmd cmd;
	int err;

	cmd.cmdidx = SD_CMD_SEND_IF_COND;
	/* We set the bit if the host supports voltages between 2.7 and 3.6 V */
	cmd.cmdarg = 0x1aa;
	cmd.resp_type = MMC_RSP_R7;

	err = mmc_send_cmd(host, &cmd, 0);

	if (err)
		return err;

	if ((cmd.response[0] & 0xff) != 0xaa)
		return UNUSABLE_ERR;
	else
		host->version = SDHCI_SPEC_200;

	return 0;
}

int mmc_complete_op_cond(struct sdhci_host *host)
{
	int timeout = 1000;
	u32 start;
	int err;

    start = 0;
	host->op_cond_pending = 0;
	if (!(host->ocr & OCR_BUSY)) {
		start = 0;
		while (1) {
			err = mmc_send_op_cond_iter(host, 1);
			if (err)
				return err;
			if (host->ocr & OCR_BUSY)
				break;
			if (start > timeout)
				return UNUSABLE_ERR;
			udelay(100);
            start += 1;
		}
	}

	//JJJ_DEBUGhost->version = MMC_VERSION_UNKNOWN;

	host->high_capacity = ((host->ocr & OCR_HCS) == OCR_HCS);
	host->rca = 1;

	return 0;
}

/* board-specific MMC power initializations. */
void board_mmc_power_init(void)
{
}

int mmc_start_init(struct sdhci_host *host)
{
	int err;
    int present;
    int freq_sel;
    int clk_mul;
    int clk_sel;
    int M;

    present = sdhci_readl(host, SDHCI_PRESENT_STATE);
    host->version = (sdhci_readw(host, SDHCI_HOST_VERSION) & SDHCI_SPEC_VER_MASK) >> SDHCI_SPEC_VER_SHIFT;
    host->caps = sdhci_readl(host, SDHCI_CAPABILITIES);

    if (host->version >= SDHCI_SPEC_300) {
        host->caps1 = sdhci_readl(host, SDHCI_CAPABILITIES_2);
    }

	/* we pretend there should be card inserted */
	if ((present & SDHCI_CARD_PRESENT) != SDHCI_CARD_PRESENT) {
		host->has_init = 0;
		printf("host: no card present\n");
		return NO_CARD_ERR;
	}

	if (host->has_init) {
        printf("host already initialized\n");
		return 0;
    }

	board_mmc_power_init();

    sdhci_reset(host, SDHCI_RESET_ALL);

	err = sdhci_init(host);

	if (err)
		return err;

    sdhci_set_ios(host);

    freq_sel = (sdhci_readw(host, SDHCI_CLOCK_CONTROL) >> 8) & 0xff;
    clk_sel = (sdhci_readw(host, SDHCI_CLOCK_CONTROL) >> 5) & 0x1;
    clk_mul = (sdhci_readw(host, SDHCI_CAPABILITIES_2) >> 16) & 0xff;
    M = clk_mul == 0 ? 1 : (clk_mul + 1);
    printf("\t\tSDCLK output=%dHz, freq_sel=0x%x, M=0x%x\n",
           clk_sel ? host->f_max * M  / (freq_sel + 1) :
                   host->f_max / (freq_sel ? (freq_sel * 2) : 1), freq_sel, M);

	/* Reset the Card */
	err = mmc_go_idle(host);

    if (err) {
        printf("Reset the Card fail, err=0x%x", err);
        return err;
    }

    if (host->card_type == SDCARD) {
        /* Test for SD version 2 */
        err = mmc_send_if_cond(host);
        if (err)
			printf("SD CMD8 SEND_IF_COND fail.\n");

        /* Now try to get the SD card's operating condition */
        err = sd_send_op_cond(host);
        if (err)
			printf("SD ACMD41 SEND_OP_COND fail.\n");

    } else if (host->card_type == EMMCCARD) {
        err = mmc_send_op_cond(host);

		if (err) {
			printf("eMMC CMD1 SEND_OP_COND fail\n");
			return err;
		}
    }

	if (!err)
		host->init_in_progress = 1;

	return err;
}

int mmc_complete_init(struct sdhci_host *host)
{
	int err = 0;
    u32 ctrl;
    struct mmc_cmd cmd;

	host->init_in_progress = 0;
	if (host->op_cond_pending) {
		err = mmc_complete_op_cond(host);
        if (err) {
            printf("eMMC COMPLETE_OP_COND fail\n");
            return err;

        }
    }

    /* Send CMD2 Put the Card in Identify Mode */
    cmd.cmdidx = MMC_CMD_ALL_SEND_CID;
    cmd.resp_type = MMC_RSP_R2;
    cmd.cmdarg = 0;
    err = mmc_send_cmd(host, &cmd, 0);

    if (err) {
        printf("CMD2 ALL_SEND_CID fail\n");
        return err;
    }

    /* Send CMD3 Puts the cards into Standby State*/
    /*
	 * For MMC cards, set the Relative Address.
	 * For SD cards, get the Relatvie Address.
	 */
    cmd.cmdidx = SD_CMD_SEND_RELATIVE_ADDR;
	cmd.cmdarg = 0;
	cmd.resp_type = MMC_RSP_R6;

	err = mmc_send_cmd(host, &cmd, 0);

    host->rca = (cmd.response[0] >> 16) & 0xffff;

	if (err) {
		printf("CMD3 SEND_RELATIVE_ADDR fail\n");
        return err;
    }

    /* Get the Card-Specific Data */
	cmd.cmdidx = MMC_CMD_SEND_CSD;
	cmd.resp_type = MMC_RSP_R2;
	cmd.cmdarg = host->rca << 16;

	err = mmc_send_cmd(host, &cmd, 0);
    if (err){
		printf("CMD9 CMD_SEND_CSD fail\n");
        return err;
    }
	host->csd[0] = cmd.response[0];
	host->csd[1] = cmd.response[1];
	host->csd[2] = cmd.response[2];
	host->csd[3] = cmd.response[3];

    //JJJ_DEBUG TODO: determine the type of device

    /* Select the card, and put it into Transfer Mode */
	cmd.cmdidx = MMC_CMD_SELECT_CARD;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = host->rca << 16;
	err = mmc_send_cmd(host, &cmd, 0);

    if (err)
		return err;

    /* Set card data bus width */
    if (host->card_type == SDCARD) {
        cmd.cmdidx = MMC_CMD_APP_CMD;
        cmd.resp_type = MMC_RSP_R1;
        cmd.cmdarg = host->rca << 16;

        err = mmc_send_cmd(host, &cmd, 0);

        if (err)
            return err;
    }

    cmd.cmdidx = SD_CMD_APP_SET_BUS_WIDTH;
    if (host->card_type == SDCARD) {
        cmd.resp_type = MMC_RSP_R1;
        if (host->bus_width == 4)
            cmd.cmdarg = SDHCI_CTRL_4BITBUS;
        else
            cmd.cmdarg = 0;
    } else if (host->card_type == EMMCCARD) {
        cmd.resp_type = MMC_RSP_R1b;
        // Access EXT_CSD byte BUS_WIDTH [183]
        // [31:26] Set to 0 [25:24] Access [23:16] Index [15:8] Value
        // [7:3] Set to 0 [2:0] Cmd Set
        if (host->bus_width == 4)
            cmd.cmdarg = 0x3b70100;
        else
            cmd.cmdarg = 0x3b70000;
    }
    err = mmc_send_cmd(host, &cmd, 0);

    if (err)
        return err;

    /* Set Controller Bus Width */
    ctrl = sdhci_readb(host, SDHCI_HOST_CONTROL);

	if (host->bus_width == 4)
		ctrl |= SDHCI_CTRL_4BITBUS;
	else
		ctrl &= ~SDHCI_CTRL_4BITBUS;

    sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);

    if (host->bus_width == 4 && host->card_type == EMMCCARD)
        sdhci_writew(host, SDHCI_CTRL_UHS_SDR25, SDHCI_HOST_CONTROL2);

    host->has_init = 1;
	return err;
}

int sdhci_setup_host(struct sdhci_host *host)
{
	int err = 0;

	if (host->has_init)
		return 0;

    err = mmc_start_init(host);

	if (!err)
		err = mmc_complete_init(host);
#if CK_SDIO_DEBUG
	printf("\n\t\t%s: %d\n", __func__, err);
#endif
	return err;

}
