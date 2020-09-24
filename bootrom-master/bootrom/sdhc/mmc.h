/*
 * (Copyright 2017) Verisilicon Inc. All Rights Reserved
 * Company confidential and Proprietary information.
 * This information may not be disclosed to unauthorized individual.
 * 
 * System Application Team <fengyin.wu@verisilicon.com>
 *
 */


#ifndef _MMC_H_
#define _MMC_H_

#include "datatype.h"
#include "platform.h"
#include "common.h"


#ifdef CONFIG_MMC_SPI
#define mmc_host_is_spi(mmc)	((mmc)->host_caps & MMC_MODE_SPI)
#else
#define mmc_host_is_spi(mmc)	0
#endif


/* SD/MMC version bits; 8 flags, 8 major, 8 minor, 8 change */
#define SD_VERSION_SD	(1U << 31)
#define MMC_VERSION_MMC	(1U << 30)

#define MAKE_SDMMC_VERSION(a, b, c)	\
	((((u32)(a)) << 16) | ((u32)(b) << 8) | (u32)(c))
#define MAKE_SD_VERSION(a, b, c)	\
	(SD_VERSION_SD | MAKE_SDMMC_VERSION(a, b, c))
#define MAKE_MMC_VERSION(a, b, c)	\
	(MMC_VERSION_MMC | MAKE_SDMMC_VERSION(a, b, c))

#define EXTRACT_SDMMC_MAJOR_VERSION(x)	\
	(((u32)(x) >> 16) & 0xff)
#define EXTRACT_SDMMC_MINOR_VERSION(x)	\
	(((u32)(x) >> 8) & 0xff)
#define EXTRACT_SDMMC_CHANGE_VERSION(x)	\
	((u32)(x) & 0xff)

#define SD_VERSION_3		MAKE_SD_VERSION(3, 0, 0)
#define SD_VERSION_2		MAKE_SD_VERSION(2, 0, 0)
#define SD_VERSION_1_0		MAKE_SD_VERSION(1, 0, 0)
#define SD_VERSION_1_10		MAKE_SD_VERSION(1, 10, 0)

#define MMC_VERSION_UNKNOWN	MAKE_MMC_VERSION(0, 0, 0)
#define MMC_VERSION_1_2		MAKE_MMC_VERSION(1, 2, 0)
#define MMC_VERSION_1_4		MAKE_MMC_VERSION(1, 4, 0)
#define MMC_VERSION_2_2		MAKE_MMC_VERSION(2, 2, 0)
#define MMC_VERSION_3		MAKE_MMC_VERSION(3, 0, 0)
#define MMC_VERSION_4		MAKE_MMC_VERSION(4, 0, 0)
#define MMC_VERSION_4_1		MAKE_MMC_VERSION(4, 1, 0)
#define MMC_VERSION_4_2		MAKE_MMC_VERSION(4, 2, 0)
#define MMC_VERSION_4_3		MAKE_MMC_VERSION(4, 3, 0)
#define MMC_VERSION_4_4		MAKE_MMC_VERSION(4, 4, 0)
#define MMC_VERSION_4_41	MAKE_MMC_VERSION(4, 4, 1)
#define MMC_VERSION_4_5		MAKE_MMC_VERSION(4, 5, 0)
#define MMC_VERSION_5_0		MAKE_MMC_VERSION(5, 0, 0)
#define MMC_VERSION_5_1		MAKE_MMC_VERSION(5, 1, 0)

#define MMC_MODE_HS		(1 << 0)
#define MMC_MODE_HS_52MHz	(1 << 1)
#define MMC_MODE_4BIT		(1 << 2)
#define MMC_MODE_8BIT		(1 << 3)
#define MMC_MODE_SPI		(1 << 4)
#define MMC_MODE_DDR_52MHz	(1 << 5)

#define SD_DATA_4BIT	0x00040000

#define IS_SD(x)	((x)->version & SD_VERSION_SD)
#define IS_MMC(x)	((x)->version & MMC_VERSION_MMC)

#define MMC_DATA_READ		1
#define MMC_DATA_WRITE		2

#define NO_CARD_ERR		-16 /* No SD/MMC card inserted */
#define UNUSABLE_ERR		-17 /* Unusable Card */
#define COMM_ERR		-18 /* Communications Error */
#define TIMEOUT			-19
#define SWITCH_ERR		-20 /* Card reports failure to switch mode */

#define MMC_CMD_GO_IDLE_STATE		0
#define MMC_CMD_SEND_OP_COND		1
#define MMC_CMD_ALL_SEND_CID		2
#define MMC_CMD_SET_RELATIVE_ADDR	3
#define MMC_CMD_SET_DSR			4
#define MMC_CMD_SWITCH			6
#define MMC_CMD_SELECT_CARD		7
#define MMC_CMD_SEND_EXT_CSD		8
#define MMC_CMD_SEND_CSD		9
#define MMC_CMD_SEND_CID		10
#define MMC_CMD_STOP_TRANSMISSION	12
#define MMC_CMD_SEND_STATUS		13
#define MMC_CMD_SET_BLOCKLEN		16
#define MMC_CMD_READ_SINGLE_BLOCK	17
#define MMC_CMD_READ_MULTIPLE_BLOCK	18
#define MMC_CMD_SET_BLOCK_COUNT         23
#define MMC_CMD_WRITE_SINGLE_BLOCK	24
#define MMC_CMD_WRITE_MULTIPLE_BLOCK	25
#define MMC_CMD_ERASE_GROUP_START	35
#define MMC_CMD_ERASE_GROUP_END		36
#define MMC_CMD_ERASE			38
#define MMC_CMD_APP_CMD			55
#define MMC_CMD_SPI_READ_OCR		58
#define MMC_CMD_SPI_CRC_ON_OFF		59
#define MMC_CMD_RES_MAN			62

#define MMC_CMD62_ARG1			0xefac62ec
#define MMC_CMD62_ARG2			0xcbaea7


#define SD_CMD_SEND_RELATIVE_ADDR	3
#define SD_CMD_SWITCH_FUNC		6
#define SD_CMD_SEND_IF_COND		8
#define SD_CMD_SWITCH_UHS18V		11

#define SD_CMD_APP_SET_BUS_WIDTH	6
#define SD_CMD_ERASE_WR_BLK_START	32
#define SD_CMD_ERASE_WR_BLK_END		33
#define SD_CMD_APP_SEND_OP_COND		41
#define SD_CMD_APP_SEND_SCR		51

/* SCR definitions in different words */
#define SD_HIGHSPEED_BUSY	0x00020000
#define SD_HIGHSPEED_SUPPORTED	0x00020000

#define OCR_BUSY		0x80000000
#define OCR_HCS			0x40000000
#define OCR_VOLTAGE_MASK	0x007FFF80
#define OCR_ACCESS_MODE		0x60000000

#define MMC_ERASE_ARG		0x00000000
#define MMC_SECURE_ERASE_ARG	0x80000000
#define MMC_TRIM_ARG		0x00000001
#define MMC_DISCARD_ARG		0x00000003
#define MMC_SECURE_TRIM1_ARG	0x80000001
#define MMC_SECURE_TRIM2_ARG	0x80008000

#define MMC_STATUS_MASK		(~0x0206BF7F)
#define MMC_STATUS_SWITCH_ERROR	(1 << 7)
#define MMC_STATUS_RDY_FOR_DATA (1 << 8)
#define MMC_STATUS_CURR_STATE	(0xf << 9)
#define MMC_STATUS_ERROR	(1 << 19)

#define MMC_STATE_PRG		(7 << 9)

#define MMC_VDD_165_195		0x00000080	/* VDD voltage 1.65 - 1.95 */
#define MMC_VDD_20_21		0x00000100	/* VDD voltage 2.0 ~ 2.1 */
#define MMC_VDD_21_22		0x00000200	/* VDD voltage 2.1 ~ 2.2 */
#define MMC_VDD_22_23		0x00000400	/* VDD voltage 2.2 ~ 2.3 */
#define MMC_VDD_23_24		0x00000800	/* VDD voltage 2.3 ~ 2.4 */
#define MMC_VDD_24_25		0x00001000	/* VDD voltage 2.4 ~ 2.5 */
#define MMC_VDD_25_26		0x00002000	/* VDD voltage 2.5 ~ 2.6 */
#define MMC_VDD_26_27		0x00004000	/* VDD voltage 2.6 ~ 2.7 */
#define MMC_VDD_27_28		0x00008000	/* VDD voltage 2.7 ~ 2.8 */
#define MMC_VDD_28_29		0x00010000	/* VDD voltage 2.8 ~ 2.9 */
#define MMC_VDD_29_30		0x00020000	/* VDD voltage 2.9 ~ 3.0 */
#define MMC_VDD_30_31		0x00040000	/* VDD voltage 3.0 ~ 3.1 */
#define MMC_VDD_31_32		0x00080000	/* VDD voltage 3.1 ~ 3.2 */
#define MMC_VDD_32_33		0x00100000	/* VDD voltage 3.2 ~ 3.3 */
#define MMC_VDD_33_34		0x00200000	/* VDD voltage 3.3 ~ 3.4 */
#define MMC_VDD_34_35		0x00400000	/* VDD voltage 3.4 ~ 3.5 */
#define MMC_VDD_35_36		0x00800000	/* VDD voltage 3.5 ~ 3.6 */

#define MMC_SWITCH_MODE_CMD_SET		0x00 /* Change the command set */
#define MMC_SWITCH_MODE_SET_BITS	0x01 /* Set bits in EXT_CSD byte
						addressed by index which are
						1 in value field */
#define MMC_SWITCH_MODE_CLEAR_BITS	0x02 /* Clear bits in EXT_CSD byte
						addressed by index, which are
						1 in value field */
#define MMC_SWITCH_MODE_WRITE_BYTE	0x03 /* Set target byte to value */

#define SD_SWITCH_CHECK		0
#define SD_SWITCH_SWITCH	1

/*
 * EXT_CSD fields
 */
#define EXT_CSD_ENH_START_ADDR		136	/* R/W */
#define EXT_CSD_ENH_SIZE_MULT		140	/* R/W */
#define EXT_CSD_GP_SIZE_MULT		143	/* R/W */
#define EXT_CSD_PARTITION_SETTING	155	/* R/W */
#define EXT_CSD_PARTITIONS_ATTRIBUTE	156	/* R/W */
#define EXT_CSD_MAX_ENH_SIZE_MULT	157	/* R */
#define EXT_CSD_PARTITIONING_SUPPORT	160	/* RO */
#define EXT_CSD_RST_N_FUNCTION		162	/* R/W */
#define EXT_CSD_BKOPS_EN		163	/* R/W & R/W/E */
#define EXT_CSD_WR_REL_PARAM		166	/* R */
#define EXT_CSD_WR_REL_SET		167	/* R/W */
#define EXT_CSD_RPMB_MULT		168	/* RO */
#define EXT_CSD_ERASE_GROUP_DEF		175	/* R/W */
#define EXT_CSD_BOOT_BUS_WIDTH		177
#define EXT_CSD_PART_CONF		179	/* R/W */
#define EXT_CSD_BUS_WIDTH		183	/* R/W */
#define EXT_CSD_HS_TIMING		185	/* R/W */
#define EXT_CSD_REV			192	/* RO */
#define EXT_CSD_CARD_TYPE		196	/* RO */
#define EXT_CSD_SEC_CNT			212	/* RO, 4 bytes */
#define EXT_CSD_HC_WP_GRP_SIZE		221	/* RO */
#define EXT_CSD_HC_ERASE_GRP_SIZE	224	/* RO */
#define EXT_CSD_BOOT_MULT		226	/* RO */
#define EXT_CSD_BKOPS_SUPPORT		502	/* RO */

/*
 * EXT_CSD field definitions
 */

#define EXT_CSD_CMD_SET_NORMAL		(1 << 0)
#define EXT_CSD_CMD_SET_SECURE		(1 << 1)
#define EXT_CSD_CMD_SET_CPSECURE	(1 << 2)

#define EXT_CSD_CARD_TYPE_26	(1 << 0)	/* Card can run at 26MHz */
#define EXT_CSD_CARD_TYPE_52	(1 << 1)	/* Card can run at 52MHz */
#define EXT_CSD_CARD_TYPE_DDR_1_8V	(1 << 2)
#define EXT_CSD_CARD_TYPE_DDR_1_2V	(1 << 3)
#define EXT_CSD_CARD_TYPE_DDR_52	(EXT_CSD_CARD_TYPE_DDR_1_8V \
					| EXT_CSD_CARD_TYPE_DDR_1_2V)

#define EXT_CSD_BUS_WIDTH_1	0	/* Card is in 1 bit mode */
#define EXT_CSD_BUS_WIDTH_4	1	/* Card is in 4 bit mode */
#define EXT_CSD_BUS_WIDTH_8	2	/* Card is in 8 bit mode */
#define EXT_CSD_DDR_BUS_WIDTH_4	5	/* Card is in 4 bit DDR mode */
#define EXT_CSD_DDR_BUS_WIDTH_8	6	/* Card is in 8 bit DDR mode */

#define EXT_CSD_BOOT_ACK_ENABLE			(1 << 6)
#define EXT_CSD_BOOT_PARTITION_ENABLE		(1 << 3)
#define EXT_CSD_PARTITION_ACCESS_ENABLE		(1 << 0)
#define EXT_CSD_PARTITION_ACCESS_DISABLE	(0 << 0)

#define EXT_CSD_BOOT_ACK(x)		(x << 6)
#define EXT_CSD_BOOT_PART_NUM(x)	(x << 3)
#define EXT_CSD_PARTITION_ACCESS(x)	(x << 0)

#define EXT_CSD_BOOT_BUS_WIDTH_MODE(x)	(x << 3)
#define EXT_CSD_BOOT_BUS_WIDTH_RESET(x)	(x << 2)
#define EXT_CSD_BOOT_BUS_WIDTH_WIDTH(x)	(x)

#define EXT_CSD_PARTITION_SETTING_COMPLETED	(1 << 0)

#define EXT_CSD_ENH_USR		(1 << 0)	/* user data area is enhanced */
#define EXT_CSD_ENH_GP(x)	(1 << ((x)+1))	/* GP part (x+1) is enhanced */

#define EXT_CSD_HS_CTRL_REL	(1 << 0)	/* host controlled WR_REL_SET */

#define EXT_CSD_WR_DATA_REL_USR		(1 << 0)	/* user data area WR_REL */
#define EXT_CSD_WR_DATA_REL_GP(x)	(1 << ((x)+1))	/* GP part (x+1) WR_REL */

#define R1_ILLEGAL_COMMAND		(1 << 22)
#define R1_APP_CMD			(1 << 5)

#define MMC_RSP_PRESENT (1 << 0)
#define MMC_RSP_136	(1 << 1)		/* 136 bit response */
#define MMC_RSP_CRC	(1 << 2)		/* expect valid crc */
#define MMC_RSP_BUSY	(1 << 3)		/* card may send busy */
#define MMC_RSP_OPCODE	(1 << 4)		/* response contains opcode */

#define MMC_RSP_NONE	(0)
#define MMC_RSP_R1	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R1b	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE| \
			MMC_RSP_BUSY)
#define MMC_RSP_R2	(MMC_RSP_PRESENT|MMC_RSP_136|MMC_RSP_CRC)
#define MMC_RSP_R3	(MMC_RSP_PRESENT)
#define MMC_RSP_R4	(MMC_RSP_PRESENT)
#define MMC_RSP_R5	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R6	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R7	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)

#define MMCPART_NOAVAILABLE	(0xff)
#define PART_ACCESS_MASK	(0x7)
#define PART_SUPPORT		(0x1)
#define ENHNCD_SUPPORT		(0x2)
#define PART_ENH_ATTRIB		(0x1f)

/* Maximum block size for MMC */
#define MMC_MAX_BLOCK_LEN	512

/* The number of MMC physical partitions.  These consist of:
 * boot partitions (2), general purpose partitions (4) in MMC v4.4.
 */
#define MMC_NUM_BOOT_PARTITION	2
#define MMC_PART_RPMB           3       /* RPMB partition number */


struct mmc_cmd {
	u16 cmdidx;
	u32 resp_type;
	u32 cmdarg;
	u32 response[4];
};

struct mmc_data {
	///union {
	char *dest;
	char *src; /* src buffers don't get written to */
	///};
	u32 flags;
	u32 blocks;
	u32 blocksize;
};

struct mmc_ios {
	unsigned int	clock;			/* clock rate */
	unsigned short	vdd;
/* vdd stores the bit number of the selected voltage range from below. */
	unsigned char	bus_mode;		/* command output mode */
#define MMC_BUSMODE_OPENDRAIN	1
#define MMC_BUSMODE_PUSHPULL	2

	unsigned char	chip_select;		/* SPI chip select */
#define MMC_CS_DONTCARE		0
#define MMC_CS_HIGH		1
#define MMC_CS_LOW		2
	unsigned char	power_mode;		/* power supply mode */
#define MMC_POWER_OFF		0
#define MMC_POWER_UP		1
#define MMC_POWER_ON		2
#define MMC_POWER_UNDEFINED	3
	unsigned char	bus_width;		/* data bus width */
#define MMC_BUS_WIDTH_1		0
#define MMC_BUS_WIDTH_4		2
#define MMC_BUS_WIDTH_8		3
	
	unsigned char	timing;			/* timing specification used */
#define MMC_TIMING_LEGACY	0
#define MMC_TIMING_MMC_HS	1
#define MMC_TIMING_SD_HS	2
#define MMC_TIMING_UHS_SDR12	3
#define MMC_TIMING_UHS_SDR25	4
#define MMC_TIMING_UHS_SDR50	5
#define MMC_TIMING_UHS_SDR104	6
#define MMC_TIMING_UHS_DDR50	7
#define MMC_TIMING_MMC_DDR52	8
#define MMC_TIMING_MMC_HS200	9
#define MMC_TIMING_MMC_HS400	10
};
struct mmc_config {
	const char *name;
//#if !CONFIG_IS_ENABLED(DM_MMC)
	const struct mmc_ops *ops;
//#endif
	u32 host_caps;
	u32 voltages;
	u32 f_min;
	u32 f_max;
	u32 b_max;
	unsigned char part_type;
};
struct mmc {
	const struct mmc_config *cfg;	/* provided configuration */
	u32 version;
	u32 voltages;
	void *priv;
	u32 has_init;
	int high_capacity;
	u32 bus_width;
	u32 clock;
	u32 card_caps;
	u32 ocr;
	u32 dsr;
	u32 dsr_imp;
	u32 scr[2];
	u32 csd[4];
	u32 cid[4];
	u16 rca;
	u8 part_support;
	u8 part_attr;
	u8 wr_rel_set;
	char part_config;
	char part_num;
	u32 tran_speed;
	u32 read_bl_len;
	u32 write_bl_len;
	u32 erase_grp_size;	/* in 512-byte sectors */
	u32 hc_wp_grp_size;	/* in 512-byte sectors */
	u64 capacity;
	u64 capacity_user;
	u64 capacity_boot;
	u64 capacity_rpmb;
	u64 capacity_gp[4];
	u64 enh_user_start;
	u64 enh_user_size;
	char op_cond_pending;	/* 1 if we are waiting on an op_cond command */
	char init_in_progress;	/* 1 if we have done mmc_start_init() */
	char preinit;		/* start init as early as possible */
	int ddr_mode;
	
	u32 f_max; //trans max clock
	u32 f_min; //trans min clock
	u32 b_max; //The maximum blocks of a read.
	u32 host_caps;
	u32 mmc_inited;
};

struct mmc_ops {
	int (*send_cmd)(struct mmc *mmc,
			struct mmc_cmd *cmd, struct mmc_data *data);
	int (*set_ios)(struct mmc *mmc);
	int (*init)(struct mmc *mmc);
	int (*getcd)(struct mmc *mmc);
	int (*getwp)(struct mmc *mmc);
};

u64 mmc_berase(struct mmc *mmc, u64 start, u64 blkcnt);
u64 mmc_bread(struct mmc *mmc,u64 start, u64 blkcnt, u32 dst);
u64 mmc_bwrite(struct mmc *mmc, u64 start, u64 blkcnt, const void *src);

int mmc_init(struct mmc *mmc);

#endif /* _MMC_H_ */
