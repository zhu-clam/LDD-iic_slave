/*
 * (Copyright 2017) Verisilicon Inc. All Rights Reserved
 * Company confidential and Proprietary information.
 * This information may not be disclosed to unauthorized individual.
 * 
 * System Application Team <fengyin.wu@verisilicon.com>
 *
 */

#ifndef __SDHCI_HW_H
#define __SDHCI_HW_H

#include "mmc.h"

#define MMC_MAX_BLK_COUNT  8
//#define CONFIG_MMC_SDHCI_SDMA

#define CONFIG_SYS_CACHELINE_SIZE  64
#define ALIGN(x, a)	(((x) + (a) - 1) & ~((a) - 1))
#define BIT(nr)			(1UL << (nr))
#define BIT_ULL(nr)		(1ULL << (nr))

/*
 * Controller registers
 */

#define SDHCI_DMA_ADDRESS	0x00

#define SDHCI_BLOCK_SIZE	0x04
#define  SDHCI_MAKE_BLKSZ(dma, blksz) (((dma & 0x7) << 12) | (blksz & 0xFFF))

#define SDHCI_BLOCK_COUNT	0x06

#define SDHCI_ARGUMENT		0x08

#define SDHCI_TRANSFER_MODE	0x0C
#define  SDHCI_TRNS_DMA		BIT(0)
#define  SDHCI_TRNS_BLK_CNT_EN	BIT(1)
#define  SDHCI_TRNS_ACMD12	BIT(2)
#define  SDHCI_TRNS_READ	BIT(4)
#define  SDHCI_TRNS_MULTI	BIT(5)

#define SDHCI_COMMAND		0x0E
#define  SDHCI_CMD_RESP_MASK	0x03
#define  SDHCI_CMD_CRC		0x08
#define  SDHCI_CMD_INDEX	0x10
#define  SDHCI_CMD_DATA		0x20
#define  SDHCI_CMD_ABORTCMD	0xC0

#define  SDHCI_CMD_RESP_NONE	0x00
#define  SDHCI_CMD_RESP_LONG	0x01
#define  SDHCI_CMD_RESP_SHORT	0x02
#define  SDHCI_CMD_RESP_SHORT_BUSY 0x03

#define SDHCI_MAKE_CMD(c, f) (((c & 0xff) << 8) | (f & 0xff))
#define SDHCI_GET_CMD(c) ((c>>8) & 0x3f)

#define SDHCI_RESPONSE		0x10

#define SDHCI_BUFFER		0x20

#define SDHCI_PRESENT_STATE	0x24
#define  SDHCI_CMD_INHIBIT	BIT(0)
#define  SDHCI_DATA_INHIBIT	BIT(1)
#define  SDHCI_DOING_WRITE	BIT(8)
#define  SDHCI_DOING_READ	BIT(9)
#define  SDHCI_SPACE_AVAILABLE	BIT(10)
#define  SDHCI_DATA_AVAILABLE	BIT(11)
#define  SDHCI_CARD_PRESENT	BIT(16)
#define  SDHCI_CARD_STATE_STABLE	BIT(17)
#define  SDHCI_CARD_DETECT_PIN_LEVEL	BIT(18)
#define  SDHCI_WRITE_PROTECT	BIT(19)

#define SDHCI_HOST_CONTROL	0x28
#define  SDHCI_CTRL_LED		BIT(0)
#define  SDHCI_CTRL_4BITBUS	BIT(1)
#define  SDHCI_CTRL_HISPD	BIT(2)
#define  SDHCI_CTRL_DMA_MASK	0x18
#define   SDHCI_CTRL_SDMA	0x00
#define   SDHCI_CTRL_ADMA1	0x08
#define   SDHCI_CTRL_ADMA32	0x10
#define   SDHCI_CTRL_ADMA64	0x18
#define  SDHCI_CTRL_8BITBUS	BIT(5)
#define  SDHCI_CTRL_CD_TEST_INS	BIT(6)
#define  SDHCI_CTRL_CD_TEST	BIT(7)

#define SDHCI_POWER_CONTROL	0x29
#define  SDHCI_POWER_ON		0x01
#define  SDHCI_POWER_180	0x0A
#define  SDHCI_POWER_300	0x0C
#define  SDHCI_POWER_330	0x0E

#define SDHCI_BLOCK_GAP_CONTROL	0x2A

#define SDHCI_WAKE_UP_CONTROL	0x2B
#define  SDHCI_WAKE_ON_INT	BIT(0)
#define  SDHCI_WAKE_ON_INSERT	BIT(1)
#define  SDHCI_WAKE_ON_REMOVE	BIT(2)

#define SDHCI_CLOCK_CONTROL	0x2C
#define  SDHCI_DIVIDER_SHIFT	8
#define  SDHCI_DIVIDER_HI_SHIFT	6
#define  SDHCI_DIV_MASK	0xFF
#define  SDHCI_DIV_MASK_LEN	8
#define  SDHCI_DIV_HI_MASK	0x300
#define  SDHCI_PROG_CLOCK_MODE  BIT(5)
#define  SDHCI_CLOCK_CARD_EN	BIT(2)
#define  SDHCI_CLOCK_INT_STABLE	BIT(1)
#define  SDHCI_CLOCK_INT_EN	BIT(0)

#define SDHCI_TIMEOUT_CONTROL	0x2E

#define SDHCI_SOFTWARE_RESET	0x2F
#define  SDHCI_RESET_ALL	0x01
#define  SDHCI_RESET_CMD	0x02
#define  SDHCI_RESET_DATA	0x04

#define SDHCI_INT_STATUS	0x30
#define SDHCI_INT_ENABLE	0x34
#define SDHCI_SIGNAL_ENABLE	0x38
#define  SDHCI_INT_RESPONSE	BIT(0)
#define  SDHCI_INT_DATA_END	BIT(1)
#define  SDHCI_INT_DMA_END	BIT(3)
#define  SDHCI_INT_SPACE_AVAIL	BIT(4)
#define  SDHCI_INT_DATA_AVAIL	BIT(5)
#define  SDHCI_INT_CARD_INSERT	BIT(6)
#define  SDHCI_INT_CARD_REMOVE	BIT(7)
#define  SDHCI_INT_CARD_INT	BIT(8)
#define  SDHCI_INT_ERROR	BIT(15)
#define  SDHCI_INT_TIMEOUT	BIT(16)
#define  SDHCI_INT_CRC		BIT(17)
#define  SDHCI_INT_END_BIT	BIT(18)
#define  SDHCI_INT_INDEX	BIT(19)
#define  SDHCI_INT_DATA_TIMEOUT	BIT(20)
#define  SDHCI_INT_DATA_CRC	BIT(21)
#define  SDHCI_INT_DATA_END_BIT	BIT(22)
#define  SDHCI_INT_BUS_POWER	BIT(23)
#define  SDHCI_INT_ACMD12ERR	BIT(24)
#define  SDHCI_INT_ADMA_ERROR	BIT(25)

#define  SDHCI_INT_NORMAL_MASK	0x00007FFF
#define  SDHCI_INT_ERROR_MASK	0xFFFF8000

#define  SDHCI_INT_CMD_MASK	(SDHCI_INT_RESPONSE | SDHCI_INT_TIMEOUT | \
		SDHCI_INT_CRC | SDHCI_INT_END_BIT | SDHCI_INT_INDEX)
#define  SDHCI_INT_DATA_MASK	(SDHCI_INT_DATA_END | SDHCI_INT_DMA_END | \
		SDHCI_INT_DATA_AVAIL | SDHCI_INT_SPACE_AVAIL | \
		SDHCI_INT_DATA_TIMEOUT | SDHCI_INT_DATA_CRC | \
		SDHCI_INT_DATA_END_BIT | SDHCI_INT_ADMA_ERROR)
#define SDHCI_INT_ALL_MASK	((unsigned int)-1)

#define SDHCI_ACMD12_ERR	0x3C

/* 3E-3F reserved */

#define SDHCI_CAPABILITIES	0x40
#define  SDHCI_TIMEOUT_CLK_MASK	0x0000003F
#define  SDHCI_TIMEOUT_CLK_SHIFT 0
#define  SDHCI_TIMEOUT_CLK_UNIT	0x00000080
#define  SDHCI_CLOCK_BASE_MASK	0x00003F00
#define  SDHCI_CLOCK_V3_BASE_MASK	0x0000FF00
#define  SDHCI_CLOCK_BASE_SHIFT	8
#define  SDHCI_MAX_BLOCK_MASK	0x00030000
#define  SDHCI_MAX_BLOCK_SHIFT  16
#define  SDHCI_CAN_DO_8BIT	BIT(18)
#define  SDHCI_CAN_DO_ADMA2	BIT(19)
#define  SDHCI_CAN_DO_ADMA1	BIT(20)
#define  SDHCI_CAN_DO_HISPD	BIT(21)
#define  SDHCI_CAN_DO_SDMA	BIT(22)
#define  SDHCI_CAN_VDD_330	BIT(24)
#define  SDHCI_CAN_VDD_300	BIT(25)
#define  SDHCI_CAN_VDD_180	BIT(26)
#define  SDHCI_CAN_64BIT	BIT(28)

#define SDHCI_CAPABILITIES_1	0x44
#define  SDHCI_CLOCK_MUL_MASK	0x00FF0000
#define  SDHCI_CLOCK_MUL_SHIFT	16

#define SDHCI_MAX_CURRENT	0x48

/* 4C-4F reserved for more max current */

#define SDHCI_SET_ACMD12_ERROR	0x50
#define SDHCI_SET_INT_ERROR	0x52

#define SDHCI_ADMA_ERROR	0x54

/* 55-57 reserved */

#define SDHCI_ADMA_ADDRESS	0x58

/* 60-FB reserved */

#define SDHCI_SLOT_INT_STATUS	0xFC

#define SDHCI_HOST_VERSION	0xFE
#define  SDHCI_VENDOR_VER_MASK	0xFF00
#define  SDHCI_VENDOR_VER_SHIFT	8
#define  SDHCI_SPEC_VER_MASK	0x00FF
#define  SDHCI_SPEC_VER_SHIFT	0
#define   SDHCI_SPEC_100	0
#define   SDHCI_SPEC_200	1
#define   SDHCI_SPEC_300	2

#define SDHCI_GET_VERSION(x) (x->version & SDHCI_SPEC_VER_MASK)

/*
 * End of controller registers.
 */

#define SDHCI_MAX_DIV_SPEC_200	256
#define SDHCI_MAX_DIV_SPEC_300	2046

/*
 * quirks
 */
#define SDHCI_QUIRK_32BIT_DMA_ADDR	(1 << 0)
#define SDHCI_QUIRK_REG32_RW		(1 << 1)
#define SDHCI_QUIRK_BROKEN_R1B		(1 << 2)
#define SDHCI_QUIRK_NO_HISPD_BIT	(1 << 3)
#define SDHCI_QUIRK_BROKEN_VOLTAGE	(1 << 4)
#define SDHCI_QUIRK_WAIT_SEND_CMD	(1 << 6)
#define SDHCI_QUIRK_USE_WIDE8		(1 << 8)


#define SDHCI_DEFAULT_BOUNDARY_SIZE	(512 * 1024)
#define SDHCI_DEFAULT_BOUNDARY_ARG	(7)

struct sdhci_ops {
	int		(*get_cd)(struct sdhci_host *host);
	void	(*set_control_reg)(struct sdhci_host *host);
	void	(*set_ios_post)(struct sdhci_host *host);
	void	(*set_clock)(struct sdhci_host *host, u32 div);
};

struct sdhci_host {
	const char *name;
	void *ioaddr;
	unsigned int quirks;
	unsigned int host_caps;
	unsigned int version;
	unsigned int max_clk;   /* Maximum Base Clock frequency */
	unsigned int clk_mul;   /* Clock Multiplier value */
	unsigned int clock;
	struct mmc *mmc;
	const struct sdhci_ops *ops;
	int index;

	int bus_width;
//	struct gpio_desc pwr_gpio;	/* Power GPIO */
//	struct gpio_desc cd_gpio;		/* Card Detect GPIO */

	u32	voltages;

	struct mmc_config cfg;
};

int sdhci_send_command(struct mmc *mmc, struct mmc_cmd *cmd,struct mmc_data *data);
int sdhci_set_ios(struct mmc *mmc);
int sdhci_init(struct mmc *mmc);
int sdhci_setup_cfg(struct mmc_config *cfg, struct sdhci_host *host,
        u32 f_max, u32 f_min);


#endif
