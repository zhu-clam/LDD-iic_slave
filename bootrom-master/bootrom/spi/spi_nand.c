/*
 * (Copyright 2018) Verisilicon Inc. All Rights Reserved
 * Company confidential and Proprietary information.
 * This information may not be disclosed to unauthorized individual.
 *
 * System Application Team <Jianheng.Zhang@verisilicon.com>
 *
 */

#include "spi_nand.h"
#include "common.h"
#include "timer.h"
#include "spi.h"
#include <string.h>
#include "boot.h"

#define SPI_NAND_DB
#ifdef SPI_NAND_DB
#define spi_nand_debug(format, ...)  debug(format,##__VA_ARGS__)
#else
#define spi_nand_debug(format, ...)  do {} while (0)
#endif

const spi_nand_info spi_chip_table[] = {
    {"GD5F1GQ4UB", 2, {0xc8, 0xd1}, 2048, 128},
    {"GD5F2GQ4UB", 2, {0xc8, 0xd2}, 2048, 128},
    {"GD5F4GQ4UB", 2, {0xc8, 0xd4}, 4096, 256},
    {"W25N01GV", 3, {0xef, 0xaa, 0x21}, 2048, 64},
    {"W25M02GV", 3, {0xef, 0xab, 0x21}, 2048, 64},
    {}/* Empty entry to terminate the list */
};


static int _spi_nand_get_feature(spi_nand *nand, u8 addr, u8 *status)
{
    u8  cmd[2] = {0};

    spi_chip_select(nand->spi);//chip select
    cmd[0] = SPINAND_CMD_GET_FEATURE;
    cmd[1] = addr;
    spi_master_transmit(nand->spi, cmd, 0x02);
    spi_master_receive(nand->spi, status, 0x01);
    spi_chip_deselect(nand->spi);//deselect

    return 0;
}

static int _spi_nand_set_feature(spi_nand *nand, u8 addr, u8 status)
{
    u8  cmd[3] = {0};

    spi_chip_select(nand->spi);//chip select
    cmd[0] = SPINAND_CMD_SET_FEATURE;
    cmd[1] = addr;
    cmd[2] = status;
    spi_master_transmit(nand->spi, cmd, 0x03);
    spi_chip_deselect(nand->spi);//deselect

    return 0;
}
/*
 * return true if nand is ready
 * */
static int _spi_nand_ready(spi_nand *nand)
{
    u8 status;
    _spi_nand_get_feature(nand, REG_STATUS, &status);
    return ((status & STATUS_OIP_MASK) == STATUS_READY);
}

static int _spi_nand_wait_ready(spi_nand *nand)
{
    u32 timebase;
    int ret;

    timebase = get_timer_count();

    while ((timebase - get_timer_count()) < SPI_NAND_TIMEOUT) {
        ret = _spi_nand_ready(nand);

        if (ret)
            return 0;
    }

    spi_nand_debug("SPI NAND: Timeout!\n");
    return ENODEV;
}

int spi_nand_read_id(spi_nand *nand, u8 *id)
{
    u8  nand_cmd[2] = {0};
    int i = 0;
    spi_chip_select(nand->spi);//chip select
    nand_cmd[0] = SPINAND_CMD_READ_ID;
    nand_cmd[1] = 0x00;  //dummy byte
    spi_master_transmit(nand->spi, nand_cmd, 0x02);
    spi_master_receive(nand->spi, id, SPI_NAND_MAX_IDLEN);
    spi_chip_deselect(nand->spi);//deselect
    spi_nand_debug("spi nand:ID :");

    for (i = 0; i < SPI_NAND_MAX_IDLEN; i ++)
    {
        spi_nand_debug("0x%x ", id[i]);
    }
    spi_nand_debug("\n");
    return 0;
}

int spi_nand_ident(spi_nand *nand)
{
    const spi_nand_info *type = spi_chip_table;

    spi_nand_read_id(nand, nand->info.id);
    for( ; type->name != NULL; type ++)
    {
        if (0 == memcmp(nand->info.id, type->id, type->id_len))
        {
            memcpy(&nand->info, type, sizeof(spi_nand_info));
            nand->page_shift = ffs(nand->info.page_size) - 1;
            spi_nand_debug("ident spi nand: %s\n", nand->info.name);
            return 0;
        }
    }
    spi_nand_debug("Spi nand: Unknown flash\n");
    return ENODEV;
}
int spi_nand_reset(spi_nand *nand)
{
    u8  cmd[1] = {0};

    spi_chip_select(nand->spi);//chip select
    cmd[0] = SPINAND_CMD_RESET;
    spi_master_transmit(nand->spi, cmd, 0x01);
    spi_chip_deselect(nand->spi);//deselect

    return _spi_nand_wait_ready(nand);
}

int spi_nand_init(spi_nand *nand)
{
    u8 cfg = 0;
    int ret = 0;

    /* chip reset */
    spi_nand_reset(nand);
    /* set buffer read mode for winbond chips*/
    _spi_nand_get_feature(nand, REG_CFG, &cfg);
    cfg |= CONFIG_READ_MODE_BUFF;
    _spi_nand_set_feature(nand, REG_CFG, cfg);

    ret = spi_nand_ident(nand);
    if (ret)
    {
        return ret;
    }


    return 0;
}


int spi_nand_read_page2cache(spi_nand *nand, u32 row_addr)
{
    u8 cmd[4] = {0};
    int ret = 0;
    u8 status;
    ret = _spi_nand_wait_ready(nand);
    if (ret)
    {
        return ret;
    }
    cmd[0] = SPINAND_CMD_PAGE_READ;
    cmd[1] = (row_addr >> 16) & 0xff;
    cmd[2] = (row_addr >> 8) & 0xff;
    cmd[3] = row_addr & 0xff;

    spi_chip_select(nand->spi);//chip select
    spi_master_transmit(nand->spi, cmd, 0x04);
    spi_chip_deselect(nand->spi);//deselect

    /* wait data read to cache */
    ret = _spi_nand_wait_ready(nand);
    if (ret)
    {
        return ret;
    }

    /* check ecc status */
    _spi_nand_get_feature(nand, REG_STATUS, &status);
    if ((status & STATUS_ECC_MASK) == STATUS_ECC_FAIL)
    {
        spi_nand_debug("read page: ecc failed ");
        return 1;
    }
    return 0;
}



int spi_nand_read_cache(spi_nand *nand, u8 *data, u32 column_addr, u32 len)
{
    u8 cmd[4] = {0};

    cmd[0] = SPINAND_CMD_READ_FROM_CACHE;
    cmd[1] = (column_addr >> 8) & 0xff;
    cmd[2] = column_addr & 0xff;
    cmd[3] = 0x00;//dummy

    spi_chip_select(nand->spi);//chip select
    spi_master_transmit(nand->spi, cmd, 0x04);
    spi_master_receive(nand->spi, data, len);
    spi_chip_deselect(nand->spi);//deselect

    return 0;
}

int spi_nand_read(spi_nand *nand, u8 *data, u32 addr, u32 len)
{
    u32 row_addr = addr >> nand->page_shift;
    u32 column_addr = addr & (nand->info.page_size - 1);
    u32 page_read_len = 0;
    int ret = 0;
    u32 remain = len;
    u8 *buff_temp = data;

    do
    {
        if ((remain + column_addr) <= nand->info.page_size)
            page_read_len = remain;
        else
            page_read_len = nand->info.page_size - column_addr;

        ret = spi_nand_read_page2cache(nand, row_addr);
        if (ret)
            return ret;
        spi_nand_read_cache(nand, buff_temp, column_addr, page_read_len);

        remain -= page_read_len;
        buff_temp += page_read_len;
        row_addr ++;
        column_addr = 0;
    }while(remain != 0);

    spi_nand_debug("nand read done.\n");
    return 0;
}
int spi_nand_boot(spi_nand *nand)
{
    int i;
    u32 read_addr;
    sb_header header;
    u16 crc16;
    LOAD_ENTRY enter_jump_func;
    debug("boot form spi nand \n");
    debug(" - page size: %d + %doob \n", nand->info.page_size, nand->info.oob_size);
    nand->page_shift = ffs(nand->info.page_size) - 1;
    for (i = 0; i < 8; i ++) //max  read 8 block
    {
        /* default page number of one block is 64
         * so : block size is -> (nand->info.page_size << 6)
         * */

        read_addr = NAND_BOOT_ADDR + (i * (nand->info.page_size << 6));
        debug("read header data from block %d offset: \n", i, read_addr);
        spi_nand_read(nand, &header, (u8*)read_addr, sizeof(sb_header));

        debug("magic=0x%x size=0x%x load_addr=0x%x entry_addr=0x%x crc16=0x%x  index=%d \n",\
            header.magic_num,header.data_size,header.load_addr,header.entry_point,header.crc16,i);

        //check flag,error continue
        if (header.magic_num != MAGIC_NUM)
        {
            debug("index=%d magic_num error.\n",i);
            continue;
        }

        //get data len, may be need check len (max) error continue
        if (header.data_size > SPL_MAX_LEN)
        {
            debug("index=%d data_size error.\n",i);
            continue;
        }

        //check load address
        if (header.load_addr < SRAM_START_ADDRESS)
        {
            debug("index=%d load_addr error.\n",i);
            continue;
        }
        if ((header.load_addr+header.data_size)>SPL_MAX_ADDRESS)
        {
            debug("index=%d the data is out of bounds.\n",i);
            continue;
        }

        //read data
        read_addr +=  sizeof(sb_header);
        spi_nand_read(nand, (u8*)header.load_addr, read_addr, header.data_size);

        //check checksum,error continue
        crc16 = check_sum((u8*)header.load_addr, header.data_size);
        if (crc16 != (u16)header.crc16)
        {
            debug("index=%d checksum error.\n",i);
            continue;
        }

        info_debug("Get second boot from %d block.\n",i);

        //start boot second_boot
        enter_jump_func = (LOAD_ENTRY)header.entry_point;
        enter_jump_func();
    }

    return -BOOT_FAILED; //boot failed

}
