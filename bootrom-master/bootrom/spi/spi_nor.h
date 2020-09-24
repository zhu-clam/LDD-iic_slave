/*
 * (Copyright 2018) Verisilicon Inc. All Rights Reserved
 * Company confidential and Proprietary information.
 * This information may not be disclosed to unauthorized individual.
 * 
 * System Application Team <Jianheng.Zhang@verisilicon.com>
 *
 */
 
#ifndef __SPI_NOR_H__
#define __SPI_NOR_H__

//******************************************************
//Flash command
#define FLS_CMD_RDID		(0x9F)
#define Flash_CMD_REMS		(0x90)
#define Flash_MID_ADDR		(0x000000)
#define Flash_DID_ADDR		(0x000001)

#define FLS_CMD_RSRL		(0x05)	
#define FLS_CMD_RSRH		(0x35)

//please note those macro for the specify flash
#define FLS_CMD_READ		(0x03)
#define FLS_CMD_FastREAD	(0x0b)
#define FLS_CMD_WREN		(0x06)
#define FLS_CMD_WRDIS		(0x04)
#define FLS_CMD_SE			(0x20)
#define FLS_CMD_BE_32K		(0x52)//32K block
#define FLS_CMD_BE_64K		(0xD8)//32K block
#define FLS_CMD_CE			(0x60) //(0xC7)
#define FLS_CMD_PP			(0x02)
#define FLS_CMD_FSRD		(0xeb)
#define FLS_CMD_WRSR 		(0x01)
	
#define FLS_CMD_HPM 		(0xA3)
#define FLS_CMD_FRD_OUT		(0x3B)
#define FLS_CMD_FRD_IO		(0xBB)
#define FLS_CMD_FRQ_OUT		(0x6B)
#define FLS_CMD_FRQ_IO_2D	(0xEB)
#define FLS_CMD_FRQ_IO_1D	(0xE7)
#define FLS_CMD_CRMR		(0xFF)
#define FLS_CMD_ENRST       (0x66)
#define FLS_CMD_DORST       (0x99)
// flash status
#define FLS_STS_QE			(0x01<<9)
#define FLS_STS_SRP1		(0x01<<8)
#define FLS_STS_SRP0		(0x01<<7)
#define FLS_STS_BP4			(0x01<<6)
#define FLS_STS_BP3			(0x01<<5)
#define FLS_STS_BP2			(0x01<<4)
#define FLS_STS_BP1			(0x01<<3)
#define FLS_STS_BP0			(0x01<<2)
#define FLS_STS_WEL			(0x01<<1)
#define FLS_STS_WIP			(0x01<<0)

#define FLS_BLOCK_SIZE				(0x00010000)//64K
#define FLS_BASE_BLOCK(n)			(n*FLS_BLOCK_SIZE)
#define FLS_SECTOR_SIZE				(0x00001000)// 4k
#define FLS_BASE_SECTOR(n)			(n*FLS_SECTOR_SIZE)
#define FLS_PAGE_SIZE				(0x00000100)//256byte
#define FLS_BASE_PAGE(n)				(n*FLS_PAGE_SIZE)
#define FLS_MAX_WRITE_SIZE		(256)
#define FLS_MAX_READ_SIZE		(0x10000)

#define CONTINUES_MODE			0xa0
#define NON_CONTINUES_MODE		0x0


#define SPI_TIMEOUT_DEFAULT		0//500000
#define CONFIG_EN_SPI_NOR_WRITE 0
/* Flash timeout values */
#define SPI_NOR_PROG_TIMEOUT      (2 * CONFIG_SYS_HZ)
#define SPI_NOR_PAGE_ERASE_TIMEOUT    (5 * CONFIG_SYS_HZ)
#define SPI_NOR_SECTOR_ERASE_TIMEOUT  (10 * CONFIG_SYS_HZ)
#define SPI_NOR_CHIP_ERASE_TIMEOUT    (60 * CONFIG_SYS_HZ)
#define SPI_NOR_RESET_TIMEOUT  (1 * CONFIG_SYS_HZ)

typedef struct tag_spi_nor{
    void *spi;

}spi_nor;

//Flash acess function
void spi_nor_read_id(spi_nor *nor, u32 *id);
int spi_nor_reset(spi_nor *nor);
int spi_nor_scan(spi_nor *nor);
int spi_nor_init(spi_nor *nor);
int spi_nor_checkbusy(spi_nor *nor, u32 timeout);
int spi_nor_ready(spi_nor *nor);
void spi_nor_read_flash(spi_nor *nor, u32 src_flash_addr, u8* dest_buff, u32 len);


int spi_nor_boot(spi_nor *nor);

#endif
