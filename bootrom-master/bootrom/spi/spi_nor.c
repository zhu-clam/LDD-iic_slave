/*
 * (Copyright 2018) Verisilicon Inc. All Rights Reserved
 * Company confidential and Proprietary information.
 * This information may not be disclosed to unauthorized individual.
 * 
 * System Application Team <Jianheng.Zhang@verisilicon.com>
 *
 */

#include "datatype.h"
#include "platform.h"
//#include "sys.h"
#include "common.h"
//#include "printf.h"
#include "spi.h"
#include "spi_nor.h"
#include "timer.h"
#include "boot.h"

/**
 * struct spi_flash_params - SPI/QSPI flash device params structure
 *
 * @name:       Device name ([MANUFLETTER][DEVTYPE][DENSITY][EXTRAINFO])
 * @jedec:      Device jedec ID (0x[1byte_manuf_id][2byte_dev_id])
 * @ext_jedec:      Device ext_jedec ID
 * @sector_size:    Isn't necessarily a sector size from vendor,
 *          the size listed here is what works with CMD_ERASE_64K
 * @nr_sectors:     No.of sectors on this device
 * @e_rd_cmd:       Enum list for read commands
 * @flags:      Important param, for flash specific behaviour
 */
struct spi_flash_params {
    const char *name;
    u32 jedec;
    u32 chip_size;
};
/* SPI/QSPI flash device params structure */
const struct spi_flash_params spi_flash_params_table[] = {

    /* GIGADEVICE */
    {"GD25Q64",        0xc84017, SZ_8M},
    {"GD25Q127c",      0xc84018, SZ_16M},
    {"GD25Q256",       0xc86019, SZ_32M},
    /* WINBOND */
    {"W25Q64",    0xef4017, SZ_8M},
    {"W25Q128",   0xef4018, SZ_16M},
    {"W25Q256",   0xef4019, SZ_32M},
    {}, /* Empty entry to terminate the list */
};
#define NOR_DB
#ifdef NOR_DB
#define nor_debug(format, ...)  debug(format,##__VA_ARGS__)
#else
#define nor_debug(format, ...)  do {} while (0)
#endif

void spi_nor_read_id(spi_nor *nor, u32 *id)
{
    u8 cmd[1];
    u8 buff[4];

    //cmd
    cmd[0]=FLS_CMD_RDID;//Flash_CMD_REMS;

    spi_chip_select(nor->spi);//chip select
    spi_master_transmit(nor->spi, cmd,1);
    spi_master_receive(nor->spi, buff,3);
    spi_chip_deselect(nor->spi);//deselect
    *id = (u32)buff[0]<<16 | (u32)buff[1]<<8 | (u32)buff[2];
    debug("spi nor: M7_0= 0x%x, ID15_8= 0x%x  ID7_0=0x%x \n",buff[0],buff[1],buff[2]);
}
int spi_nor_reset(spi_nor *nor)
{
    int ret = 0;
    u8 cmd[1] = {0};
    //select the device
    spi_chip_select(nor->spi);
    //send command
    cmd[0] = FLS_CMD_ENRST;
    spi_master_transmit(nor->spi, cmd,1);
    //de-select device
    spi_chip_deselect(nor->spi);//deselect

    /* may need delay xx us ?*/
    //select the device
    spi_chip_select(nor->spi);
    //send command
    cmd[0] = FLS_CMD_DORST;
    spi_master_transmit(nor->spi, cmd,1);
    //de-select device
    spi_chip_deselect(nor->spi);//deselect

    ret = spi_nor_checkbusy(nor, SPI_NOR_RESET_TIMEOUT);
    return ret;
}
int spi_nor_scan(spi_nor *nor)
{
    u32 id = 0;
    struct spi_flash_params *type = (struct spi_flash_params *)&spi_flash_params_table[0];
    spi_nor_read_id(nor, &id);
    for( ; type->name != NULL; type ++)
    {
        if(id == type->jedec)
        {
            debug("find spi nor :%s size: %d bytes\n", type->name, type->chip_size);
            return OK;
        }
    }
    debug("SF nor: Unsupported flash:id = 0x%x\n", id);
    return ENODEV;
}
int spi_nor_init(spi_nor *nor)
{
    int ret = 0;

    /* spi int */

    ret = spi_nor_reset(nor);
    if (ret)
    {
        return ret;
    }
    ret = spi_nor_scan(nor);
    return ret;
}


int spi_nor_checkbusy(spi_nor *nor, u32 timeout)
{

    u32 timebase;
    int ret;

    timebase = get_timer_count();

    while ((timebase - get_timer_count()) < timeout) {
        ret = spi_nor_ready(nor);
        if (ret)
            return 0;
    }
    nor_debug("SF: Timeout!\n");
    return ETIMEDOUT;
}
/*
 * return 1 if nor is ready
 * */
int spi_nor_ready(spi_nor *nor)
{
    u8 cmd[1];
    spi_chip_select(nor->spi);
    //send command
    cmd[0] = FLS_CMD_RSRL;//0x05
    spi_master_transmit(nor->spi, cmd,1);
    spi_master_receive(nor->spi, cmd,1);
    //de-select device
    spi_chip_deselect(nor->spi);//deselect
    return !(cmd[0] & FLS_STS_WIP);
}

void spi_nor_read_flash(spi_nor *nor, u32 src_flash_addr, u8* dest_buff, u32 len)
{

    u8 cmd[4];	
    u8 *buff_temp=0;
    u32 read_len=0;
    u32 src_flash_addr_temp=src_flash_addr;
    u32 remain = len;
    u32 first_page_rw_reamain = (FLS_PAGE_SIZE- src_flash_addr%FLS_PAGE_SIZE)%FLS_PAGE_SIZE;
    buff_temp=dest_buff;
    nor_debug("Read data from spi-0x%x  to  mem-0x%x len is 0x%x.\n",src_flash_addr,dest_buff,len);

    do{
        if(first_page_rw_reamain==0)
            read_len = MIN(remain,(u32)FLS_SECTOR_SIZE);	//MIN(remain,FIFO_DEPTH);
        else
        {
            read_len=MIN(remain,first_page_rw_reamain);
            first_page_rw_reamain=0;
        }		
        //select the device
        spi_chip_select(nor->spi);
        //cmd
        cmd[0]=FLS_CMD_READ;
        //address
        cmd[1]=((src_flash_addr_temp>>16)&0xFF);
        cmd[2]=((src_flash_addr_temp>>8)&0xFF);
        cmd[3]=((src_flash_addr_temp>>0)&0xFF);
        spi_master_transmit(nor->spi, cmd,4);
        spi_master_receive(nor->spi, buff_temp,read_len);
        spi_chip_deselect(nor->spi);
        //CommonDelay(100);
        remain =remain -read_len;
        src_flash_addr_temp+=read_len;
        buff_temp+=read_len;

//		debug(".");
//		if(i++%32 == 0)
//			debug("\n");

    }while(remain != 0);
//	debug("\n");

    nor_debug("nor read done.\n");
}

int spi_nor_boot(spi_nor *nor)
{
    int read_addr = NOR_BOOT_ADDR;
    LOAD_ENTRY enter_jump_func;
    sb_header header;
    u16 crc16;

    debug("boot from spi nor\n");

    spi_nor_read_flash(nor, read_addr, (u8*)&header, sizeof(sb_header));

    nor_debug("magic=0x%x size=0x%x load_addr=0x%x entry_addr=0x%x crc16=0x%x \n",\
        header.magic_num,header.data_size,header.load_addr,header.entry_point,header.crc16);

    //check flag,error return -1
    if(header.magic_num != MAGIC_NUM)
    {
        debug("nor magic_num error.\n");
        return -BOOT_FAILED;//boot failed
    }
    //get data len, may be need check len (max) error return -1
    if(header.data_size > SPL_MAX_LEN)
    {
        debug("nor data_size error.\n");
        return -BOOT_FAILED;//boot failed
    }
    //check load address
    if(header.load_addr < SRAM_START_ADDRESS)
    {
        debug("load_addr error.\n");
        return -BOOT_FAILED;//boot failed
    }
    if((header.load_addr+header.data_size)>SPL_MAX_ADDRESS)
    {
        debug("the data is out of bounds.\n");
        return -BOOT_FAILED;//boot failed
    }

    //read data
    read_addr +=  sizeof(sb_header);

    spi_nor_read_flash(nor, read_addr, (u8*)header.load_addr, header.data_size);
    info_debug("nor read done.\n");


    //check checksum,error return -1
    crc16 = check_sum((u8*)header.load_addr, header.data_size);
    if(crc16 != (u16)header.crc16)
    {
        debug("checksum error.\n");
        return -BOOT_FAILED;//boot failed
    }

    //start boot second_boot
    enter_jump_func = (LOAD_ENTRY)0xf0000180;
    enter_jump_func();

    return -BOOT_FAILED;//boot failed
}
#if CONFIG_EN_SPI_NOR_WRITE
void spi_nor_enable_write(spi_nor *nor)
{
    u8 cmd[1];

    spi_chip_select(nor->spi);//chip select
    cmd[0] = FLS_CMD_WREN;
    spi_master_transmit(nor->spi, cmd, 0x01);
    spi_chip_deselect(nor->spi);//deselect
    //CommonDelay(100);
}

void spi_nor_disable_write(spi_nor *nor)
{
    u8 cmd[1];

    spi_chip_select(nor->spi);//chip select
    cmd[0] = FLS_CMD_WRDIS;
    spi_master_transmit(nor->spi, cmd, 0x01);
    spi_chip_deselect(nor->spi);//deselect
    //CommonDelay(100);
}

void spi_nor_disable_protect(spi_nor *nor)
{
    u8 cmd[3];

    spi_chip_select(nor->spi);//chip select
    cmd[0] = FLS_CMD_WRSR;
    cmd[1] = 0x00;//diable all protect
    cmd[2] = 0x00;//diable all protect
    spi_master_transmit(nor->spi, cmd, 0x02);
    spi_chip_deselect(nor->spi);//deselect
}
void spi_nor_erase_chip(spi_nor *nor)
{
    u8 cmd[1];
    //enable flash write
    spi_nor_enable_write(nor);
    spi_nor_disable_protect(nor);//disable  all block protect
    spi_nor_checkbusy(nor, SPI_NOR_RESET_TIMEOUT);
    //
    spi_nor_enable_write(nor);
    spi_chip_select(nor->spi);//chip select
    cmd[0] = FLS_CMD_CE;
    spi_master_transmit(nor->spi, cmd, 0x01);
    spi_chip_deselect(nor->spi);//deselect
    //CommonDelay(100);
    spi_nor_checkbusy(nor, SPI_NOR_CHIP_ERASE_TIMEOUT);
}

void spi_nor_erase_sector(spi_nor *nor, u32 addr)
{
    u8 cmd[4];
    //enable flash write
    spi_nor_enable_write(nor);
    spi_nor_disable_protect(nor);//disable  all block protect
    spi_nor_checkbusy(nor, SPI_NOR_RESET_TIMEOUT);
    //
    spi_nor_enable_write(nor);
    spi_chip_select(nor->spi);//chip select
    addr=addr&0xfffff000;
    cmd[0] = FLS_CMD_SE;
    //address
    cmd[1]=((addr>>16)&0xFF);
    cmd[2]=((addr>>8)&0xFF);
    cmd[3]=((addr>>0)&0xFF);
    spi_master_transmit(nor->spi, cmd, 0x04);
    spi_chip_deselect(nor->spi);//deselect
    //CommonDelay(100);
    spi_nor_checkbusy(nor, SPI_NOR_SECTOR_ERASE_TIMEOUT);
}

void spi_nor_erase_flash(spi_nor *nor, u32 flash_addr, u32 len)
{
    u32 sectors=0;
    u32 i = 0;
    sectors = (len+FLS_SECTOR_SIZE-1)/FLS_SECTOR_SIZE;
    for(i=0;i<sectors;i++)
    {
        spi_nor_erase_sector(nor->spi, flash_addr + i*FLS_SECTOR_SIZE);

        debug(".");
        if((i+1)%32 == 0)
            debug("\n");

    }
    debug("\n");
}
void spi_nor_write_flash(spi_nor *nor, u8* src_buff, u32 dest_flash_addr, u32 len)
{
    int i=1;
    u8 cmd[4];
    u32 write_len;
    u8 *buff_temp;
    u32 dest_flash_addr_temp=dest_flash_addr;
    u32 remain = len;
    u32 first_page_rw_reamain =(FLS_PAGE_SIZE- dest_flash_addr%FLS_PAGE_SIZE)%FLS_PAGE_SIZE;
    buff_temp=src_buff;
    //enable write
    //spi_nor_enable_write(spi_inst);
    do{
        spi_nor_enable_write(nor);
        if(first_page_rw_reamain==0)
            write_len = MIN(remain,(u32)FLS_MAX_WRITE_SIZE);    //MIN(remain,FIFO_DEPTH);
        else
        {
            write_len =MIN(remain,first_page_rw_reamain);
            first_page_rw_reamain=0;
        }
        //
        spi_chip_select(nor->spi);//chip select
        cmd[0] = FLS_CMD_PP;
        cmd[1] = (dest_flash_addr_temp>>16)&0xFF;
        cmd[2] = (dest_flash_addr_temp>>8)&0xFF;
        cmd[3] = (dest_flash_addr_temp>>0)&0xFF;
        spi_master_transmit(nor->spi, cmd, 0x04);
        spi_master_transmit(nor->spi, buff_temp,write_len);
        spi_chip_deselect(nor->spi);//deselect
        //CommonDelay(100);
        spi_nor_checkbusy(nor, SPI_NOR_PROG_TIMEOUT);
        remain =remain - write_len;
        dest_flash_addr_temp+=write_len;
        buff_temp+=write_len;

        debug(".");
        if(i++%32 == 0)
            debug("\n");

    }while(remain != 0);
    debug("\n");
    spi_nor_disable_write(nor);

    nor_debug("nor write done.\n");
}
#endif

