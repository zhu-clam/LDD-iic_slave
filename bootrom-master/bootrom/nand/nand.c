/*
 * (Copyright 2018) Verisilicon Inc. All Rights Reserved
 * Company confidential and Proprietary information.
 * This information may not be disclosed to unauthorized individual.
 *
 * System Application Team <Jianheng.Zhang@verisilicon.com>
 *
 */

#include "datatype.h"
#include "common.h"
#include "nand.h"
#include <string.h>
#include "boot.h"

#define NAND_DB
#ifdef NAND_DB
#define nand_debug(format, ...)  debug(format,##__VA_ARGS__)
#else
#define nand_debug(format, ...)  do {} while (0)
#endif
#define CONFIG_NAND_EN_WRITE 0
const nand_flash_info flash_info_table[] = {
        {"K9F4G08U0E", 0xec, 0xdc109554, SZ_2K, 64},
        {"K9K8G08U0E", 0xec, 0xd3519559, SZ_2K, 64},
        {"MT29F16G08M", 0x2c, 0xd5943e74, SZ_4K, 218},
        {"MT29F64G08C", 0x2c, 0x88044ba9, SZ_4K, 218},
        {}//null

};

#define nand_writel(nfc, off, val)  \
        write_mreg32((nfc)->base + (off), (val))

#define nand_readl(nfc, off)        \
        read_mreg32((nfc)->base + (off))
/*
u8 nand_read8(u32 reg_adr)
{
    u32 reg32_val;
    reg32_val = *((u32*) reg_adr);
    reg32_val &= 0xFF;
    return (u8) reg32_val;
}
*/

/*******************************************************************************************************
 *	when bitmask of reg set/unset just wait
 *******************************************************************************************************/
int nand_wait(nand_flash *nfc, u32 reg, u32 bit_mask, enum BIT_SET bitset)
{
    int delay_number = 100000;
    if(SET_BIT == bitset)
    {
        while (delay_number > 0)
        {
            if (nand_readl(nfc, reg) & bit_mask)
            {
                return 0;
            }
            usleep(1);
            delay_number--;
        }
    }
    else
    {
        while (delay_number > 0)
        {
            if (!(nand_readl(nfc, reg) & bit_mask))
            {
                return 0;
            }
            usleep(1);
            delay_number--;
        }
    }
    nand_debug("vsi nfc wait timer out! reg :%x mask:%x bitset: %d /n", reg, bit_mask, bitset);
    return -1;
}

u8 nand_read_status(nand_flash *nfc)
{
    u8 reg8_val;

    nand_writel(nfc, NAND_FACOMM_1, NAND_STATUS); //read status command 0x70

    nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_L); //set CE to low-level;

    nand_writel(nfc, NAND_FSPEC, FSPEC_RDSTAT_TRIG); //trig to read flash status

    nand_wait(nfc, NAND_FSPEC, FSPEC_RDSTAT_TRIG, UNSET_BIT);

    reg8_val = (u8)nand_readl(nfc, NAND_FSTAT);

    nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_H); //set CE to high-level

    return reg8_val;
}

/*******************************************************************************************************
 *	read nand id data to parameter id_data
 *******************************************************************************************************/
void nand_read_id(nand_flash *nfc, u8 *m_id, u32 *d_id)
{
    u32 id;
    //read ID command
    nand_writel(nfc, NAND_FACOMM_1, NAND_READ_ID);
    //read ID, address 0x00
    nand_writel(nfc, NAND_FA1, 0x00);
    // flash channel index. flash chip select using the decoding style.
    nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_L);
    //trig to read flash ID
    nand_writel(nfc, NAND_FSPEC, FSPEC_RDID_TRIG);
    /*wait command send out*/
    nand_wait(nfc, NAND_FSPEC, FSPEC_RDID_TRIG, UNSET_BIT);
    nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_H);

    *m_id = (u8)nand_readl(nfc, NAND_FMAK);

    id = (u8)nand_readl(nfc, NAND_FDEV);
    id = (id << 8) | (u8)nand_readl(nfc, NAND_Flash_ID3);
    id = (id << 8) | (u8)nand_readl(nfc, NAND_Flash_ID4);
    *d_id = (id << 8) | (u8)nand_readl(nfc, NAND_Flash_ID5);

}

int nand_ident(nand_flash *nfc)
{
    const nand_flash_info *type = flash_info_table;

    nand_read_id(nfc, &nfc->info.M_ID, &nfc->info.D_ID);
    for( ; type->name != NULL; type ++)
    {
        if ((type->M_ID == nfc->info.M_ID) && (type->D_ID == nfc->info.D_ID))
        {
            memcpy(&nfc->info, type, sizeof(nand_flash_info));
            nand_debug("ident nand flash: %s\n", nfc->info.name);
            return 0;
        }
    }
    nand_debug("NAND: unknown flash: M_ID 0x%x D_ID 0x%x\n", nfc->info.M_ID, nfc->info.D_ID);

    return ENODEV;

}
/*******************************************************************************************************
 *	send reset command
 *******************************************************************************************************/
void nand_reset(nand_flash *nfc)
{
    nand_writel(nfc, NAND_INT_CLR, 0x7f); //clear all interrupt
    nand_writel(nfc, NAND_INT_MSK, (INT_ECCERR_BIT|INT_FLSRDY_BIT));
    nand_writel(nfc, NAND_FACOMM_1, NAND_RESET);
    nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_L);
    nand_writel(nfc, NAND_FCMDCTL, FCMDCTL_SINGLETRIG_ST|FCMDCTL_NORMALTRIG_ST);
    /*wait interrupt*/
    nand_wait(nfc, NAND_INT_MSK, INT_FLSRDY_BIT, SET_BIT);
    /*set CE to high-level*/
    nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_H);
    return;
}
void nfc_set_page_size(nand_flash *nfc, u32 page_size, u32 oob_size)
{

    u32 val = 0;
    switch (page_size)
    {
        case SZ_2K:
            nfc->sector_cnt = 4;
            nfc->sector_size = 512;
            val = FUNCCTL_PGDTBLANKCHK_EN |
                  FUNCCTL_ECC8_SEC512;
            nfc->ecc_code_size = 13;
            break;
        case SZ_4K:
            nfc->sector_cnt = 8;
            nfc->sector_size = 512;

            val = FUNCCTL_PGDTBLANKCHK_EN |
                  FUNCCTL_ECC8_SEC512;
            nfc->ecc_code_size = 13;
            break;
        case SZ_8K:
            nfc->sector_cnt = 8;
            nfc->sector_size = 1024;
            val = FUNCCTL_PGDTBLANKCHK_EN |
                  FUNCCTL_ECC24_SEC1K;
            nfc->ecc_code_size = 42;
            break;
        default: return;
    }

    nand_writel(nfc,NAND_FUNC_CTL, val);

    /* Spare Count = (oob_size / s_sector_cnt) */
    nand_writel(nfc,NAND_FSPR_CNT, FSPR_CNT_X(oob_size / nfc->sector_cnt));

    /* The sector number of one page */
    nand_writel(nfc,NAND_SECTOR_NUM, nfc->sector_cnt);
}
void nfc_init(nand_flash *nfc)
{
    u32 val;

    /* default 8bit data width */
    nand_writel(nfc, NAND_FTYPE, (FTYPE_8BS_DTWIDTH));

    val=(FCTL_DEWRITEPROTECT_ST)| /* cancel write protect */
        (FCTL_DEVSELMOD_BITMAP)|  /* '1' bit-mapped mode for ce pin */
        (FCTL_MODE_AUTO)|         /* flash read/write auto mode */
        (FCTL_ECCHWAUTOCLR_ST);   /* clear ECC circuit as starting a FDBA transfer */

    nand_writel(nfc,NAND_FCTL,val);
    /*set CE to high-level*/
    nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_H);
    //nand_writel(nfc,NAND_ECCTL,ECCCTL_ECC_EN); //enable ECC
    nand_writel(nfc,NAND_ECCTL,0); //enable ECC
    /*change nand R/W speed*/
    nand_writel(nfc, NAND_STB_LWIDTH, 0x06);
    nand_writel(nfc, NAND_STB_HWIDTH, 0x06);
    //clear all interrupt
    nand_writel(nfc, NAND_INT_CLR, 0x7f);
    /* enable below interrupts then them can set
     * NFC Interrupt Status Register (NFC_INT) corresponding bits.
     * but only INT_ECCERR_BIT can trigger cpu int buy
     * NAND_INT_MSK setting */
    nand_writel(nfc, NAND_INT_EN,
                    (INT_ECCERR_BIT|
                    INT_FLSRDY_BIT|
                    INT_BUFSZDATARDY_BIT|
                    INT_FIFOEMPTY_BIT|
                    INT_FIFOFULL_BIT |
                    INT_page_done));
    nand_writel(nfc, NAND_INT_MSK, (INT_ECCERR_BIT));
    nand_writel(nfc, NAND_BUSY_CNT, 10);

    /*
     * reset this parameters after nand chip ident
     *
     * */
    nfc_set_page_size(nfc, SZ_2K, 64);
}
/*******************************************************************************************************
 *	init nand flash: init controller register->reset nand flash->read id->set controller register
 *	all nand use 8 bit ecc,io mode and standard command
 *******************************************************************************************************/
int nand_init(nand_flash *nfc)
{
    int ret = 0;
    nfc_init(nfc);

    nand_reset(nfc);

    ret = nand_ident(nfc);
    nfc->page_shift = ffs(nfc->info.page_size) - 1;
    nfc_set_page_size(nfc, nfc->info.page_size, nfc->info.oob_size);
    return ret;
}

/*******************************************************************************************************
 *	init NFC register for one sector read
 *	parameter:
 *		p_adr - physical address for nand flash
 *******************************************************************************************************/
void nand_read_sector(nand_flash *nfc, u32 p_adr)
{
    nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_RD); //DMA read direction
    //to fill the command
    nand_writel(nfc, NAND_FACOMM_1, NAND_READ1);//flash read page command, cycle 1 -- 0x00
    nand_writel(nfc, NAND_FACOMM_2, NAND_READ2);//flash read page command, cycle 2 -- 0x30
    //to fill the address
    nand_writel(nfc, NAND_FA0, 0);
    nand_writel(nfc, NAND_FA1, 0);
    nand_writel(nfc, NAND_FA2, (u8 )(p_adr & 0xff));
    nand_writel(nfc, NAND_FA3, (u8 )((p_adr & 0xff00) >> 8));
    nand_writel(nfc, NAND_FA4, (u8 )((p_adr & 0xff0000) >> 16));

    nand_writel(nfc, NAND_FDBACTL, FDBACTL_MD_NORMALDTMV|FDBACTL_MD_ONLYDT|FDBACTL_RWIR_RD);//data+ECC+spare move
    nand_writel(nfc, NAND_FCMDCTL, FCMDCTL_FAS01234_ST);

    //debug("  value (0x%x )=0x%x   \r\n",NAND_FCMDCTL,nand_readl(nfc, NAND_FCMDCTL) );

    nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_L);//set CE to low-level

    //trigger the command flow use the cmd_auto_mode
    nand_writel(nfc, NAND_INT_MSK, (INT_ECCERR_BIT|INT_FLSRDY_BIT));
    //nand_writel(nfc, NAND_FATCTL, 0x09);
    nand_writel(nfc, NAND_FATCTL,(FATCTL_AUTOMD_CM1ADRCM2CHRDYDT|FATCTL_CMDDT_PROC_TRIG));//COMM1+FA0~FA4+COMM2
    return;
}
void nand_read_spare(nand_flash *nfc, u32 p_adr, u32 col_addr)
{
    nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_RD); //DMA read direction
    //to fill the command
    nand_writel(nfc, NAND_FACOMM_1, NAND_READ1);//flash read page command, cycle 1 -- 0x00
    nand_writel(nfc, NAND_FACOMM_2, NAND_READ2);//flash read page command, cycle 2 -- 0x30
    //to fill the address
    nand_writel(nfc, NAND_FA0, (u8 )(col_addr & 0xff));
    nand_writel(nfc, NAND_FA1, (u8 )((col_addr & 0xff00) >> 8));
    nand_writel(nfc, NAND_FA2, (u8 )(p_adr & 0xff));
    nand_writel(nfc, NAND_FA3, (u8 )((p_adr & 0xff00) >> 8));
    nand_writel(nfc, NAND_FA4, (u8 )((p_adr & 0xff0000) >> 16));

    nand_writel(nfc, NAND_FDBACTL, FDBACTL_MD_NORMALSPARMV|FDBACTL_MD_ONLYSP|FDBACTL_RWIR_RD);//only spare move
    nand_writel(nfc, NAND_FCMDCTL, FCMDCTL_FAS01234_ST);

    //debug("  value (0x%x )=0x%x   \r\n",NAND_FCMDCTL,nand_readl(nfc, NAND_FCMDCTL) );

    nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_L);//set CE to low-level

    //trigger the command flow use the cmd_auto_mode
    nand_writel(nfc, NAND_INT_MSK, (INT_ECCERR_BIT|INT_FLSRDY_BIT));
    //nand_writel(nfc, NAND_FATCTL, 0x09);
    nand_writel(nfc, NAND_FATCTL,(FATCTL_AUTOMD_CM1ADRCM2CHRDYDT|FATCTL_CMDDT_PROC_TRIG));//COMM1+FA0~FA4+COMM2
    return;
}
/*********************************************************************************
 *
 *   Note: to define the page read function
 *
 ********************************************************************************/
void nand_read_sector_dma(nand_flash *nfc, u32 p_adr)
{
    nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_RD);//DMA read direction
    //to fill the command
    nand_writel(nfc, NAND_FACOMM_1, NAND_READ1);//flash read page command, cycle 1 -- 0x00
    nand_writel(nfc, NAND_FACOMM_2, NAND_READ2);//flash read page command, cycle 2 -- 0x30
    //to fill the address
    nand_writel(nfc, NAND_FA0, 0);
    nand_writel(nfc, NAND_FA1, 0);
    nand_writel(nfc, NAND_FA2, (u8)(p_adr & 0xff));
    nand_writel(nfc, NAND_FA3, (u8)((p_adr & 0xff00) >> 8));
    nand_writel(nfc, NAND_FA4, (u8)((p_adr & 0xff0000) >> 16));

    nand_writel(nfc, NAND_FDBACTL, FDBACTL_MD_NORMALSPARMV|FDBACTL_RWIR_RD);//data+ECC+spare move
    //clear fifo
    nand_writel(nfc, NAND_CONFIG, CFG_FIFO_CLEAR);
    nand_writel(nfc, NAND_CONFIG,
            (CFG_DMADIR_RD | CFG_DMA_EN )); //enable dma,DMA read direction
    nand_writel(nfc, NAND_FCMDCTL, FCMDCTL_FAS01234_ST|FCMDCTL_NORMALTRIG_ST);
    nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_L);    //set CE to low-level
    //trigger the command flow use the cmd_auto_mode
    nand_writel(nfc, NAND_INT_MSK, (INT_ECCERR_BIT|INT_FLSRDY_BIT));
    nand_writel(nfc, NAND_FATCTL, 0x09);
    //nand_writel(nfc, NAND_FATCTL,(FATCTL_AUTOMD_CM1ADRCM2|FATCTL_CMDDT_PROC_TRIG));//COMM1+FA0~FA4+COMM2
    return;
}

/*******************************************************************************************************
 *	read nand page
 *	parameter:
 *		pageaddr - start page address
 *		pageCount - total number of page
 *		dst_mem_addr- destination addresss in memory for saving data
 *	return:
 *		data length
 *******************************************************************************************************/
int nand_read(nand_flash *nfc, u8 *data, u32 addr, u32 len)
{
    u32 row_start = addr >> nfc->page_shift;
    u32 column_start = addr & (nfc->info.page_size - 1);
    u32 now_row = row_start;
    u32 now_column = 0;
    int en_data_copy = 0;
    int ret = 0;
    u32 remain = len;
    u8 *buff_temp = data;
    /////////////////////////////////////
    int i = 0;
    u32 tmp32;
    int j = 0;
    int k = 0;
    debug("row_start :%d. column_start :%d\n",row_start, column_start);
    nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_RD);
    /*use 8bit ecc*/
    //nand_writel(nfc, NAND_FUNC_CTL, 0x20); //8bit ecc
    //nand_writel(nfc, NAND_FSPR_CNT, 0x0f);
    nand_writel(nfc, NAND_INT_CLR, 0x7f); //clear all interrupt
    //rd secs
    do
    {
        j = 0;
        nand_read_sector(nfc, now_row);	//dmac master
        /*wait auto cmd findish*/
        nand_wait(nfc, NAND_FATCTL, FATCTL_CMDDT_PROC_TRIG, UNSET_BIT);

        /*trig read/write process*/
        for(i = 0; i < nfc->sector_cnt; i++)
        {

            if (j > 0)
            {
                /* ecc off
                 * triger sector_n data read only
                 * */
                nand_writel(nfc, NAND_FDBACTL, nand_readl(nfc,NAND_FDBACTL)
                                             | FDBACTL_DTAUTO_TRIG);
                nand_wait(nfc, NAND_FDBACTL, FDBACTL_DTAUTO_TRIG, UNSET_BIT);
            }
            nand_wait(nfc, NAND_INT, INT_FIFOFULL_BIT, SET_BIT);
            //nand_wait(nfc, NAND_FCMDCTL, FCMDCTL_NORMALTRIG_ST, UNSET_BIT);
            for (j = 0; j < (nfc->sector_size >> 2); j ++)
            {
                tmp32 = nand_readl(nfc, NAND_AHB_FiFo);

                for (k = 0; k < 4; k++)
                {
#if 0
                    debug("now_row :0x%x now_column :0x%x en_data_copy: %d remain:%d buff_temp:0x%x.\n",
                            now_row, now_column ,en_data_copy, remain, buff_temp);
#endif
                    if (0 == en_data_copy)
                    {
                        if ((now_row == row_start) && (now_column == column_start))
                        {
                            en_data_copy = 1;
                        }
                    }
                    else if (0 == remain)
                    {
                        en_data_copy = 0;
                    }

                    if (en_data_copy)
                    {
                        *buff_temp = (tmp32 >> (8 * k)) & 0xFF;
                        //debug("cp data :0x%x.\n",*buff_temp);

                        buff_temp ++;
                        remain --;
                    }
                    now_column ++;
                }
            }
            nand_wait(nfc, NAND_INT, INT_FIFOEMPTY_BIT, SET_BIT);
            nand_writel(nfc, NAND_INT_CLR, INT_FIFOEMPTY_BIT
                                         | INT_FIFOFULL_BIT
                                         | INT_BUFSZDATARDY_BIT);
        }

        /*after one page read*/
        now_row += 1;
        now_column = 0;
        nand_writel(nfc, NAND_INT_CLR, 0x7f);	//clear
    }while(remain != 0);
    nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_H);
    nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_RD);
    return ret;
}

u32 nand_read_oob(nand_flash *nfc, u32 PageAddr, u8* dst_mem_addr)
{
    int i, data_addr_cnt = 0;

    int one_move_size = 0;
    int ret = 0;

    nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_RD);
    /*use 8bit ecc*/
    //nand_writel(nfc, NAND_FUNC_CTL, 0x20); //8bit ecc
    //nand_writel(nfc, NAND_FSPR_CNT, 0x0f);
    nand_writel(nfc, NAND_INT_CLR, 0x7f); //clear all interrupt
    //rd secs


    if (NAND_SPARE_MAX >= nfc->info.oob_size)
    {
        one_move_size = nfc->info.oob_size;
    }
    else
    {
        one_move_size = NAND_SPARE_MAX;
    }

    nand_writel(nfc,NAND_FSPR_CNT, FSPR_CNT_X(one_move_size));
    nand_read_spare(nfc, PageAddr, nfc->info.page_size);    //dmac master
    /*wait auto cmd findish*/
    nand_wait(nfc, NAND_FATCTL, FATCTL_CMDDT_PROC_TRIG, UNSET_BIT);

    for (i = 0; i < one_move_size; i ++)
    {
        dst_mem_addr[data_addr_cnt] = nand_readl(nfc, NAND_FSPR_REG(i));
        data_addr_cnt++;
    }

    while (data_addr_cnt < nfc->info.oob_size)
    {
        one_move_size = nfc->info.oob_size - data_addr_cnt;
        if (one_move_size >= NAND_SPARE_MAX)
        {
            one_move_size = NAND_SPARE_MAX;
        }
        else
        {
            nand_writel(nfc,NAND_FSPR_CNT, FSPR_CNT_X(one_move_size));
        }


        nand_writel(nfc, NAND_FDBACTL,
                    nand_readl(nfc, NAND_FDBACTL)|FDBACTL_DTAUTO_TRIG);
        nand_wait(nfc, NAND_FDBACTL, FDBACTL_DTAUTO_TRIG, UNSET_BIT);

        nand_wait(nfc, NAND_FCMDCTL, FCMDCTL_NORMALTRIG_ST, UNSET_BIT);
        for (i = 0; i < one_move_size; i ++)
        {
            dst_mem_addr[data_addr_cnt] = nand_readl(nfc, NAND_FSPR_REG(i));
            data_addr_cnt++;
        }

    }
    //clear sector_cnt_reg!!!
    nand_writel(nfc,NAND_SECTOR_NUM, 0);
    nand_writel(nfc,NAND_SECTOR_NUM, nfc->sector_cnt);

    nand_writel(nfc, NAND_INT_CLR, 0x7f);   //clear

    /*after one page read*/
    nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_H);
    nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_RD);
    return ret;
}

int nand_boot(nand_flash *nand)
{
    int i;
    u32 read_addr;
    sb_header header;
    u16 crc16;
    LOAD_ENTRY enter_jump_func;
    debug("boot form nand \n");
    debug(" - page size: %d + %doob \n", nand->info.page_size, nand->info.oob_size);
    nand->page_shift = ffs(nand->info.page_size) - 1;
    for (i = 0; i < 8; i ++) //max  read 8 block
    {
        /* default page number of one block is 64
         * so : block size is -> (nand->info.page_size << 6)
         * */

        read_addr = NAND_BOOT_ADDR + (i * (nand->info.page_size << 6));
        debug("read header data from block %d offset: %d\n", i, read_addr);
        nand_read(nand, (u8 *)&header, read_addr, sizeof(sb_header));

        debug("magic=0x%x size=0x%x load_addr=0x%x ",\
            header.magic_num,header.data_size,header.load_addr);
        debug(" entry_addr=0x%x crc16=0x%x  index=%d \n",\
                    header.entry_point,header.crc16,i);
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
        nand_read(nand, (u8 *)header.load_addr, read_addr, header.data_size);

        //check checksum,error continue
        crc16 = check_sum((u8*)header.load_addr, header.data_size);
        if (crc16 != (u16)header.crc16)
        {
            debug("index=%d checksum error.\n",i);
            debug("check_crc: 0x%x. file_crc: 0x%x\n",crc16, header.crc16);
            continue;
        }

        info_debug("Get second boot from %d block.\n",i);

        //start boot second_boot
        enter_jump_func = (LOAD_ENTRY)header.entry_point;
        enter_jump_func();
    }

    return -BOOT_FAILED; //boot failed

}


