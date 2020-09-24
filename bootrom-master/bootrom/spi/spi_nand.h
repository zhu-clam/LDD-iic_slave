/*
 * spi_nand.h
 *
 *  Created on: Sep 12, 2018
 *      Author: cn0971
 */

#ifndef DRIVERS_SPI_NAND_H_
#define DRIVERS_SPI_NAND_H_
#include "datatype.h"

/*
 * Standard SPI-NAND flash commands
 */
#define SPINAND_CMD_RESET               0xff
#define SPINAND_CMD_GET_FEATURE         0x0f
#define SPINAND_CMD_SET_FEATURE         0x1f
#define SPINAND_CMD_PAGE_READ           0x13
#define SPINAND_CMD_READ_PAGE_CACHE_RDM     0x30
#define SPINAND_CMD_READ_PAGE_CACHE_LAST    0x3f
#define SPINAND_CMD_READ_FROM_CACHE         0x03
#define SPINAND_CMD_READ_FROM_CACHE_FAST    0x0b
#define SPINAND_CMD_READ_FROM_CACHE_X2      0x3b
#define SPINAND_CMD_READ_FROM_CACHE_DUAL_IO 0xbb
#define SPINAND_CMD_READ_FROM_CACHE_X4      0x6b
#define SPINAND_CMD_READ_FROM_CACHE_QUAD_IO 0xeb
#define SPINAND_CMD_BLK_ERASE           0xd8
#define SPINAND_CMD_PROG_EXC            0x10
#define SPINAND_CMD_PROG_LOAD           0x02
#define SPINAND_CMD_PROG_LOAD_RDM_DATA      0x84
#define SPINAND_CMD_PROG_LOAD_X4        0x32
#define SPINAND_CMD_PROG_LOAD_RDM_DATA_X4   0x34
#define SPINAND_CMD_READ_ID         0x9f
#define SPINAND_CMD_WR_DISABLE          0x04
#define SPINAND_CMD_WR_ENABLE           0x06
#define SPINAND_CMD_END             0x0


/* feature registers */
#define REG_BLOCK_LOCK      0xa0
#define REG_CFG         0xb0
#define REG_STATUS      0xc0
#define REG_DIE_SELECT      0xd0

/* status */
#define STATUS_OIP_MASK     0x01
#define STATUS_CRBSY_MASK       0x80
#define STATUS_READY        (0 << 0)
#define STATUS_BUSY     (1 << 0)

#define STATUS_E_FAIL_MASK  0x04
#define STATUS_E_FAIL       (1 << 2)

#define STATUS_P_FAIL_MASK  0x08
#define STATUS_P_FAIL       (1 << 3)

#define STATUS_ECC_MASK  0x30
#define STATUS_ECC_FAIL       (2 << 4)
/* config */
#define CONFIG_READ_MODE_MASK   (1 << 3)
#define CONFIG_READ_MODE_BUFF   (1 << 3)

#define SPI_NAND_MAX_IDLEN  3
#define SPI_NAND_TIMEOUT  1000

typedef struct {
    const char *name;
    u8 id_len;
    u8 id[SPI_NAND_MAX_IDLEN];
    u32 page_size;
    u32 oob_size;
}spi_nand_info;

typedef struct {
    void *spi;
    spi_nand_info info;
    int page_shift;
}spi_nand;

int spi_nand_init(spi_nand *nand);
int spi_nand_read(spi_nand *nand, u8 *data, u32 addr, u32 len);
int spi_nand_boot(spi_nand *nand);
#endif /* DRIVERS_SPI_NAND_H_ */
