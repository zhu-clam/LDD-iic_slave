/*
 * (Copyright 2018) Verisilicon Inc. All Rights Reserved
 * Company confidential and Proprietary information.
 * This information may not be disclosed to unauthorized individual.
 *
 * System Application Team <Jianheng.Zhang@verisilicon.com>
 *
 */

#include "datatype.h"
#include "nand.h"
#include <string.h>
//#define NAND_DB
#ifdef NAND_DB
#define nand_debug(format, ...)  printf(format,##__VA_ARGS__)
#else
#define nand_debug(format, ...)  do {} while (0)
#endif
#define CONFIG_NAND_EN_WRITE 0
const nand_flash_info flash_info_table[] = {
        {"K9F4G08U0E", 0xec, 0xdc109554, SZ_2K, 64},
        {"K9F4G08U0E", 0xec, 0xdc109555, SZ_2K, 64},
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
u8 nand_read_status(nand_flash *nfc)
{
    u8 reg8_val;

    nand_writel(nfc, NAND_FACOMM_1, NAND_STATUS); //read status command 0x70

    nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_L); //set CE to low-level;

    reg8_val = 0x02;
    nand_writel(nfc, NAND_FSPEC, reg8_val); //trig to read flash status

    while (1)
    {
        if (!((u8)nand_readl(nfc, NAND_FSPEC) & FSPEC_RDSTAT_TRIG))
            break;
    }
    reg8_val = (u8)nand_readl(nfc, NAND_FSTAT);

    nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_H); //set CE to high-level

    return reg8_val;
}


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
            udelay(1);
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
            udelay(1);
            delay_number--;
        }
    }
    nand_debug("vsi nfc wait timer out! reg :%x mask:%x bitset: %d /n", reg, bit_mask, bitset);
    return -1;
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
            nand_debug("ident nand flash: %s page_size: %d + %doob \n", \
                    nfc->info.name, nfc->info.page_size, nfc->info.oob_size );

            return 0;
        }
    }
    nand_debug("NAND: Unsupported flash: M_ID 0x%x D_ID 0x%x\n", nfc->info.M_ID, nfc->info.D_ID);

    return 1;

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
    nand_wait(nfc, NAND_FCMDCTL, FCMDCTL_NORMALTRIG_ST, UNSET_BIT);
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
            nfc->ecc_code_loc = 3;
            nfc->ecc_code_sector_off = 16;
            break;
        case SZ_4K:
            nfc->sector_cnt = 8;
            nfc->sector_size = 512;

            val = FUNCCTL_PGDTBLANKCHK_EN |
                  FUNCCTL_ECC8_SEC512;
            nfc->ecc_code_size = 13;//13
            nfc->ecc_code_loc = 8;//8
            nfc->ecc_code_sector_off = 21;
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
                    INT_ECCDN_BIT |
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

    //nand_debug("  value (0x%x )=0x%x   \r\n",NAND_FCMDCTL,nand_readl(nfc, NAND_FCMDCTL) );

    nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_L);//set CE to low-level

    //trigger the command flow use the cmd_auto_mode
    nand_writel(nfc, NAND_INT_MSK, (INT_ECCERR_BIT|INT_FLSRDY_BIT));
    //nand_writel(nfc, NAND_FATCTL, 0x09);
    nand_writel(nfc, NAND_FATCTL,(FATCTL_AUTOMD_CM1ADRCM2CHRDYDT|FATCTL_CMDDT_PROC_TRIG));//COMM1+FA0~FA4+COMM2
    return;
}
void nand_en_spare_cmd(nand_flash *nfc, u16 col_addr,u16 cmd)
{
    nand_writel(nfc, NAND_SPRCMD_CTL, SPRCMD_CTL_NUM_1 | SPRCMD_CTL_EN);
    nand_writel(nfc, NAND_FBCOMM_1, (u8)(cmd & 0xff));
    nand_writel(nfc, NAND_FBCOMM_2, (u8)(cmd >> 8) & 0xff);
    nand_writel(nfc, NAND_FA0_B, (u8)(col_addr & 0xff));//FA0_B = ecc_code_start
    nand_writel(nfc, NAND_FA1_B, (u8)(col_addr >> 8) & 0xff);
}
void nand_read_sector_ecc(nand_flash *nfc, u32 p_adr)
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

    nand_writel(nfc, NAND_FDBACTL, FDBACTL_MD_NORMALDTMV|FDBACTL_RWIR_RD);//data+ECC+spare move
    nand_writel(nfc, NAND_FCMDCTL, FCMDCTL_FAS01234_ST);

    //nand_debug("  value (0x%x )=0x%x   \r\n",NAND_FCMDCTL,nand_readl(nfc, NAND_FCMDCTL) );

    /* enable Spare cmd to access ecc data in spare  */
    nand_en_spare_cmd(nfc, nfc->info.page_size + nfc->ecc_code_loc, NAND_RD_READ);

    nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_L);//set CE to low-level

    //trigger the command flow use the cmd_auto_mode
    nand_writel(nfc, NAND_INT_MSK, (INT_ECCERR_BIT|INT_FLSRDY_BIT));
    //nand_writel(nfc, NAND_FATCTL, 0x09);
    nand_writel(nfc, NAND_FATCTL,(FATCTL_AUTOMD_CM1ADRCM2CHRDYDT|FATCTL_CMDDT_PROC_TRIG));//COMM1+FA0~FA4+COMM2
    return;
}
void nand_read_sector_random(nand_flash *nfc, u32 col_addr)
{
    nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_RD); //DMA read direction
    //to fill the command
    nand_writel(nfc, NAND_FACOMM_1, (u8 )(NAND_RD_READ & 0xff));//flash read page command, cycle 1 -- 0x00
    nand_writel(nfc, NAND_FACOMM_2, (u8 )((NAND_RD_READ & 0xff00) >> 8));//flash read page command, cycle 2 -- 0x30
    //to fill the address
    nand_writel(nfc, NAND_FA0, (u8 )(col_addr & 0xff));
    nand_writel(nfc, NAND_FA1, (u8 )((col_addr & 0xff00) >> 8));


    nand_writel(nfc, NAND_FDBACTL, FDBACTL_MD_NORMALDTMV|FDBACTL_RWIR_RD);//data+ECC+spare move
    nand_writel(nfc, NAND_FCMDCTL, FCMDCTL_FAS01_ST);

    //nand_debug("  value (0x%x )=0x%x   \r\n",NAND_FCMDCTL,nand_readl(nfc, NAND_FCMDCTL) );

    /* enable Spare cmd to access ecc data in spare  */

    nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_L);//set CE to low-level

    //trigger the command flow use the cmd_auto_mode
    nand_writel(nfc, NAND_INT_MSK, (INT_ECCERR_BIT|INT_FLSRDY_BIT));
    //nand_writel(nfc, NAND_FATCTL, 0x09);
    nand_writel(nfc, NAND_FATCTL,(FATCTL_AUTOMD_CM1ADRCM2CHRDYDT|FATCTL_CMDDT_PROC_TRIG));//COMM1+FA0~FA4+COMM2
    return;
}

void nand_write_sector_ecc(nand_flash *nfc, u32 p_adr)
{
    nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_WR);//DMA write direction

    //to fill the command
    nand_writel(nfc, NAND_FACOMM_1, NAND_PROGRAM1);//flash write page command, cycle 1 -- 0x80
    //////nand_writel(nfc, NAND_FACOMM_2,NAND_PROGRAM2);//flash write page command, cycle 2 -- 0x10
    //to fill the address
    nand_writel(nfc, NAND_FA0, 0);
    nand_writel(nfc, NAND_FA1, 0);
    nand_writel(nfc, NAND_FA2, (u8 )(p_adr & 0xff));
    nand_writel(nfc, NAND_FA3, (u8 )((p_adr & 0xff00) >> 8));
    nand_writel(nfc, NAND_FA4, (u8 )((p_adr & 0xff0000) >> 16));
    nand_writel(nfc, NAND_FDBACTL, FDBACTL_MD_NORMALDTMV|FDBACTL_RWIR_WR);//data+ECC+spare move
    nand_writel(nfc, NAND_FCMDCTL, FCMDCTL_FAS01234_ST|FCMDCTL_NORMALTRIG_ST);//COMM1+FA0~FA4

    /* enable Spare cmd to access ecc data in spare  */
    //
    nand_en_spare_cmd(nfc, nfc->info.page_size + nfc->ecc_code_loc, NAND_RD_PROGRAM);
    ///////nand_writel(nfc, NAND_FDSEL,FLASH_CHCE_L);//set CE to low-level
    //trigger the command flow use the cmd_auto_mode
    //nand_writel(nfc, NAND_FATCTL, (FATCTL_AUTOMD_CM1ADRDT|FATCTL_CMDDT_PROC_TRIG));//COMM1+FA0~FA4+data
    nand_writel(nfc, NAND_FATCTL, (FATCTL_AUTOMD_CM1ADRDT));
    //enable external flash ready interrupt.
    nand_writel(nfc, NAND_INT_MSK, (INT_ECCERR_BIT|INT_FLSRDY_BIT));
}
void nand_write_sector_random(nand_flash *nfc, u32 col_addr)
{
    nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_WR);//DMA write direction

    //to fill the command
    nand_writel(nfc, NAND_FACOMM_1, NAND_RD_PROGRAM);//flash write page command, cycle 1 -- 0x80
    //////nand_writel(nfc, NAND_FACOMM_2,NAND_PROGRAM2);//flash write page command, cycle 2 -- 0x10
    //to fill the address

    nand_writel(nfc, NAND_FA0, (u8 )(col_addr & 0xff));
    nand_writel(nfc, NAND_FA1, (u8 )((col_addr & 0xff00) >> 8));

    nand_writel(nfc, NAND_FDBACTL, FDBACTL_MD_NORMALDTMV|FDBACTL_RWIR_WR);//data+ECC+spare move
    nand_writel(nfc, NAND_FCMDCTL, FCMDCTL_FAS01_ST|FCMDCTL_NORMALTRIG_ST);//COMM1+FA0~FA4

    /* enable Spare cmd to access ecc data in spare  */
    //

    ///////nand_writel(nfc, NAND_FDSEL,FLASH_CHCE_L);//set CE to low-level
    //trigger the command flow use the cmd_auto_mode
    //nand_writel(nfc, NAND_FATCTL, (FATCTL_AUTOMD_CM1ADRDT|FATCTL_CMDDT_PROC_TRIG));//COMM1+FA0~FA4+data
    nand_writel(nfc, NAND_FATCTL, (FATCTL_AUTOMD_CM1ADRDT));
    //enable external flash ready interrupt.
    nand_writel(nfc, NAND_INT_MSK, (INT_ECCERR_BIT|INT_FLSRDY_BIT));
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

    //nand_debug("  value (0x%x )=0x%x   \r\n",NAND_FCMDCTL,nand_readl(nfc, NAND_FCMDCTL) );

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
#define CONFIG_NAND_EN_WRITE 1
#if CONFIG_NAND_EN_WRITE
void nand_block_erase(nand_flash *nfc, u32 p_adr)
{
    u8 reg8_val;
    nand_writel(nfc, NAND_INT_CLR, 0x7f); //clear all interrupt

    nand_writel(nfc, NAND_INT_MSK, (INT_ECCERR_BIT|INT_FLSRDY_BIT));

    // to set the FDBACTL
    nand_writel(nfc, NAND_FDBACTL, FDBACTL_RWIR_WR);

    //note:set NFC_CFG[0] as 0.
    nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_WR);//DMA write direction

    //Note:set the command mode.
    nand_writel(nfc, NAND_FCMDCTL, FCMDCTL_FAS01234_ST|FCMDCTL_SKIPFA1FA0_ST);//mode select:FCOMM+FA0~FA4,skip FA1~FA0

    //to fill the command
    nand_writel(nfc, NAND_FACOMM_1, 0x60 /*NAND_ERASE1*/);//flash block erase command, cycle 1 -- 0x60

    //to fill the address
    nand_writel(nfc, NAND_FA2, (u8 )(p_adr & 0xff));
    nand_writel(nfc, NAND_FA3, (u8 )((p_adr & 0xff00) >> 8));
    nand_writel(nfc, NAND_FA4, (u8 )((p_adr & 0xff0000) >> 16));

    nand_writel(nfc, NAND_FACOMM_2, 0xD0 /*NAND_ERASE2*/); //flash block erase command, cycle 2 -- 0xD0

    nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_L); //set CE to low-level;

    //trigger the command flow use the cmd_auto_mode
    nand_writel(nfc, NAND_FATCTL, (FATCTL_AUTOMD_CM1ADRCM2|FATCTL_CMDDT_PROC_TRIG));//COMM1+FA0~FA4+COMM2

    //check the ready/busy pulse status!
    /*wait interrupt*/
    nand_wait(nfc,  NAND_INT, INT_FLSRDY_BIT, SET_BIT);

    nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_H); //set CE to high-level

    //at last to check the flash disk status!
    reg8_val = nand_read_status(nfc);
    if (reg8_val & 0x01) //IO0: pass -- '0', fail -- '1'
    {
        nand_debug(" to erase the page fails!\r\n");
        nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_WR); //DMA write direction
        return;
    }
    //note:set NFC_CFG[0] as 1.
    nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_WR); //DMA write direction

}

/*******************************************************************************************************
 *	init NFC register for one sector write
 *	parameter:
 *		p_adr - physical address for nand flash
 *******************************************************************************************************/
void nand_write_sector(nand_flash *nfc, u32 p_adr)
{
    nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_WR);//DMA write direction
    //to fill the command
    nand_writel(nfc, NAND_FACOMM_1, NAND_PROGRAM1);//flash write page command, cycle 1 -- 0x80
    //////nand_writel(nfc, NAND_FACOMM_2,NAND_PROGRAM2);//flash write page command, cycle 2 -- 0x10
    //to fill the address
    nand_writel(nfc, NAND_FA0, 0);
    nand_writel(nfc, NAND_FA1, 0);
    nand_writel(nfc, NAND_FA2, (u8 )(p_adr & 0xff));
    nand_writel(nfc, NAND_FA3, (u8 )((p_adr & 0xff00) >> 8));
    nand_writel(nfc, NAND_FA4, (u8 )((p_adr & 0xff0000) >> 16));
    nand_writel(nfc, NAND_FDBACTL, FDBACTL_MD_NORMALDTMV|FDBACTL_MD_ONLYDT|FDBACTL_RWIR_WR);//data+ECC+spare move
    nand_writel(nfc, NAND_FCMDCTL, FCMDCTL_FAS01234_ST);
    ///////nand_writel(nfc, NAND_FDSEL,FLASH_CHCE_L);//set CE to low-level
    //trigger the command flow use the cmd_auto_mode
    nand_writel(nfc, NAND_FATCTL, (FATCTL_AUTOMD_CM1ADRDT));//COMM1+FA0~FA4+data
    //enable external flash ready interrupt.
    nand_writel(nfc, NAND_INT_MSK, (INT_ECCERR_BIT|INT_FLSRDY_BIT));
    return;
}
void nand_write_spare(nand_flash *nfc, u32 p_adr, u32 col_addr)
{
    nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_WR);//DMA write direction
    //to fill the command
    nand_writel(nfc, NAND_FACOMM_1, NAND_PROGRAM1);//flash write page command, cycle 1 -- 0x80
    //////nand_writel(nfc, NAND_FACOMM_2,NAND_PROGRAM2);//flash write page command, cycle 2 -- 0x10
    //to fill the address
    nand_writel(nfc, NAND_FA0, (u8 )(col_addr & 0xff));
    nand_writel(nfc, NAND_FA1, (u8 )((col_addr & 0xff00) >> 8));
    nand_writel(nfc, NAND_FA2, (u8 )(p_adr & 0xff));
    nand_writel(nfc, NAND_FA3, (u8 )((p_adr & 0xff00) >> 8));
    nand_writel(nfc, NAND_FA4, (u8 )((p_adr & 0xff0000) >> 16));
    nand_writel(nfc, NAND_FDBACTL, FDBACTL_MD_NORMALSPARMV|FDBACTL_MD_ONLYSP|FDBACTL_RWIR_WR);//data+ECC+spare move
    nand_writel(nfc, NAND_FCMDCTL, FCMDCTL_FAS01234_ST);
    ///////nand_writel(nfc, NAND_FDSEL,FLASH_CHCE_L);//set CE to low-level
    //trigger the command flow use the cmd_auto_mode
    nand_writel(nfc, NAND_FATCTL, (FATCTL_AUTOMD_CM1ADRDT));//COMM1+FA0~FA4+data
    //enable external flash ready interrupt.
    nand_writel(nfc, NAND_INT_MSK, (INT_ECCERR_BIT|INT_FLSRDY_BIT));
    return;
}
static u32 nfc_page_num;

/*********************************************************************************
 *
 *   Note: to define the page write function
 *
 ********************************************************************************/
void nand_write_sector_dma(nand_flash *nfc, u32 p_adr)
{
    nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_WR);//DMA write direction
    //to fill the command
    nand_writel(nfc, NAND_FACOMM_1, NAND_PROGRAM1);//flash write page command, cycle 1 -- 0x80
    //////nand_writel(nfc, NAND_FACOMM_2,NAND_PROGRAM2);//flash write page command, cycle 2 -- 0x10
    //to fill the address
    nand_writel(nfc, NAND_FA0, 0);
    nand_writel(nfc, NAND_FA1, 0);
    nand_writel(nfc, NAND_FA2, (u8 )(p_adr & 0xff));
    nand_writel(nfc, NAND_FA3, (u8 )((p_adr & 0xff00) >> 8));
    nand_writel(nfc, NAND_FA4, (u8 )((p_adr & 0xff0000) >> 16));
    nand_writel(nfc, NAND_FDBACTL, FDBACTL_MD_NORMALSPARMV|FDBACTL_RWIR_WR);//data+ECC+spare move
    nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_WR | CFG_DMA_EN);//enable dma,DMA write direction
    nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_L);	//set CE to low-level
    nand_writel(nfc, NAND_FCMDCTL, FCMDCTL_FAS01234_ST|FCMDCTL_NORMALTRIG_ST);

    nand_wait(nfc, NAND_FCMDCTL, FCMDCTL_NORMALTRIG_ST, UNSET_BIT);
    //open the check flash ready flag interrupt.
    nand_writel(nfc, NAND_INT_MSK,
            (INT_ECCERR_BIT|INT_BUFSZDATARDY_BIT|INT_FIFOFULL_BIT));

#ifdef 	InterruptMode
    while(!(NFC_INT_Flag & INT_FLSRDY_BIT));
#else
    while (nfc_page_num)
    {
        nand_wait(nfc, NAND_INT, INT_FIFOFULL_BIT, SET_BIT);
        //while(!DMAC_CheckDone(1)){}
        nand_writel(nfc, NAND_FDBACTL, nand_readl(nfc, NAND_FDBACTL)|0x80);//enable data write
        nfc_page_num--;
        nand_writel(nfc, NAND_INT_CLR, INT_FIFOFULL_BIT);
        nand_wait(nfc, NAND_FDBACTL, FDBACTL_DTAUTO_TRIG, SET_BIT);
    }
#endif
    nand_wait(nfc, NAND_INT, INT_page_done, SET_BIT);
    return;
}

/*******************************************************************************************************
 *	write nand page
 *	parameter:
 *		pageaddr - start page address
 *		pageCount - total number of page
 *		src_mem_addr- source addresss
 *	return:
 *		data length
 *******************************************************************************************************/
u32 nand_write(nand_flash *nfc, u32 PageAddr, u8 PageCount, u32* scr_mem_addr)
{
    int i, data_addr_cnt = 0;
    int tmp32;
    u32* DestAddr;
    int ret = 0;
    int j = 0;
    u8 reg8_val;
    nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_WR);
    nand_writel(nfc, NAND_ECCTL, 0x0);//disable ecc
    /*use 8bit ecc*/
    //nand_writel(nfc, NAND_FUNC_CTL, 0x20);	//8bit ecc
    nand_writel(nfc, NAND_FSPR_CNT, 0x0f);
    nand_writel(nfc, NAND_INT_CLR, 0x7f);	//clear all interrupt
    //rd secs
    for (i = 0; i < PageCount; i++)
    {
        nand_write_sector(nfc, PageAddr);	//dmac master
        PageAddr += 1;

        while (!(nand_readl(nfc, NAND_INT) & INT_FIFOFULL_BIT))
        {
            for (j = 0; j < (nfc->sector_size >> 2); j++)
            {
                nand_writel(nfc, NAND_AHB_FiFo, *(scr_mem_addr + data_addr_cnt));
                data_addr_cnt++;
            }
        }
        nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_L);	//set CE to low-level
        /*start cmd*/
        nand_writel(nfc, NAND_FATCTL,
                (FATCTL_AUTOMD_CM1ADRDT|FATCTL_CMDDT_PROC_TRIG));//COMM1+FA0~FA4+data

        /*wait auto cmd finish*/
        while ((nand_readl(nfc, NAND_FATCTL) & FATCTL_CMDDT_PROC_TRIG))
            ;	//wait fatctl finish
        /*wait cmd findish*/
        while ((nand_readl(nfc, NAND_FCMDCTL) & FCMDCTL_NORMALTRIG_ST))
            ;	//wait fatctl finish

        nand_writel(nfc, NAND_INT_CLR,
                INT_BUFSZDATARDY_BIT|INT_FIFOFULL_BIT|CFG_FIFOEMPTY_BIT);
        /*write fifo and start data write*/
        for (j = 0; j < (nfc->sector_cnt - 1); j++)
        {
            while (!(nand_readl(nfc, NAND_INT) & INT_FIFOFULL_BIT))
            {
                nand_writel(nfc, NAND_AHB_FiFo, *(scr_mem_addr + data_addr_cnt));
                data_addr_cnt++;
            }
            for (int k = 0; k < 16; k ++)
            {
                nand_writel(nfc, NAND_FSPR_REG(k), k);
            }
            /*trig read/write process*/
            nand_writel(nfc, NAND_FDBACTL,
                    nand_readl(nfc, NAND_FDBACTL)|FDBACTL_DTAUTO_TRIG);
            /*wait data transfer end*/
            while ((nand_readl(nfc, NAND_FDBACTL) & FDBACTL_DTAUTO_TRIG))
            {
            }
            /*wait fifo empty*/
            while (!(nand_readl(nfc, NAND_CONFIG) & CFG_FIFOEMPTY_BIT))
            {
            }	//wait until sector transfer success
            nand_writel(nfc, NAND_INT_CLR,
                    INT_BUFSZDATARDY_BIT|INT_FIFOFULL_BIT|CFG_FIFOEMPTY_BIT);
        }
        /*after one page write*/
        nand_writel(nfc, NAND_FACOMM_1, NAND_PROGRAM2);
        nand_writel(nfc, NAND_FCMDCTL,
                FCMDCTL_SINGLETRIG_ST|FCMDCTL_NORMALTRIG_ST|FCMDCTL_FAS01234_ST); //

        /*wait for command end*/
        while ((nand_readl(nfc, NAND_FCMDCTL) & FCMDCTL_NORMALTRIG_ST))
        {
        }
        while (!(nand_readl(nfc, NAND_INT) & INT_FLSRDY_BIT))
            ;  //wait nand not busy
        /*read status*/
        reg8_val = nand_read_status(nfc);
        if (reg8_val & 0x01) //IO0: pass -- '0', fail -- '1'
        {
            nand_debug(" to write the page fails!\r\n");
            nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_WR); //DMA write direction
            return 1;
        }
        /*after one page write*/
        nand_writel(nfc, NAND_INT_CLR, 0x7f); //clear
    }
    nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_H);
    nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_WR);
    return ret;
}

/*******************************************************************************************************
 *  write nand page spare
 *  parameter:
 *      pageaddr - start page address
 *      pageCount - total number of page
 *      src_mem_addr- source addresss
 *  return:
 *      data length
 *******************************************************************************************************/
u32 nand_write_oob(nand_flash *nfc, u32 PageAddr, u8* scr_mem_addr)
{
    int i, data_addr_cnt = 0;
    int tmp32;
    u32* DestAddr;
    int ret = 0;
    int one_move_size = 0;
    u8 reg8_val;
    nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_WR);
    /*use 8bit ecc*/
    //nand_writel(nfc, NAND_FUNC_CTL, 0x20);    //8bit ecc
    //nand_writel(nfc, NAND_FSPR_CNT, 0x0f);
    nand_writel(nfc, NAND_INT_CLR, 0x7f);   //clear all interrupt
    //rd secs



    nand_write_spare(nfc, PageAddr, nfc->info.page_size);

    //for (j = 0; j < (nfc->info.oob_size ); j++)
    if (NAND_SPARE_MAX >= nfc->info.oob_size)
    {
        nand_writel(nfc,NAND_FSPR_CNT, FSPR_CNT_X(nfc->info.oob_size));
        for (i = 0; i < nfc->info.oob_size; i++)
        {
            nand_writel(nfc, NAND_FSPR_REG(i), *(scr_mem_addr + data_addr_cnt));
            data_addr_cnt++;
        }
    }
    else
    {
        nand_writel(nfc,NAND_FSPR_CNT, FSPR_CNT_X(NAND_SPARE_MAX));
        for (i = 0; i < NAND_SPARE_MAX; i++)
        {
            nand_writel(nfc, NAND_FSPR_REG(i), *(scr_mem_addr + data_addr_cnt));
            data_addr_cnt++;
        }
    }


    nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_L); //set CE to low-level
    /*start cmd*/
    nand_writel(nfc, NAND_FATCTL,
            (FATCTL_AUTOMD_CM1ADRDT|FATCTL_CMDDT_PROC_TRIG));//COMM1+FA0~FA4+data

    /*wait auto cmd finish*/
    while ((nand_readl(nfc, NAND_FATCTL) & FATCTL_CMDDT_PROC_TRIG))
        ;   //wait fatctl finish
    /*wait cmd findish*/
    while ((nand_readl(nfc, NAND_FCMDCTL) & FCMDCTL_NORMALTRIG_ST))
        ;   //wait fatctl finish

    nand_writel(nfc, NAND_INT_CLR,
            INT_BUFSZDATARDY_BIT|INT_FIFOFULL_BIT|CFG_FIFOEMPTY_BIT);
    /*write spare reg and start data write*/
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
        for (i = 0; i < one_move_size; i++)
        {
            nand_writel(nfc, NAND_FSPR_REG(i), *(scr_mem_addr + data_addr_cnt));
            data_addr_cnt++;
        }
        nand_writel(nfc, NAND_FDBACTL,
                nand_readl(nfc, NAND_FDBACTL)|FDBACTL_DTAUTO_TRIG);
        /*wait data transfer end*/
        while ((nand_readl(nfc, NAND_FDBACTL) & FDBACTL_DTAUTO_TRIG))
        {
        }

        nand_writel(nfc, NAND_INT_CLR,
                INT_BUFSZDATARDY_BIT|INT_FIFOFULL_BIT|CFG_FIFOEMPTY_BIT);
    }

    /*after one page write*/
    nand_writel(nfc, NAND_FACOMM_1, NAND_PROGRAM2);
    nand_writel(nfc, NAND_FCMDCTL,
            FCMDCTL_SINGLETRIG_ST|FCMDCTL_NORMALTRIG_ST|FCMDCTL_FAS01234_ST); //

    /*wait for command end*/
    while ((nand_readl(nfc, NAND_FCMDCTL) & FCMDCTL_NORMALTRIG_ST))
    {
    }
    while (!(nand_readl(nfc, NAND_INT) & INT_FLSRDY_BIT))
        ;  //wait nand not busy
    /*read status*/
    reg8_val = nand_read_status(nfc);
    if (reg8_val & 0x01) //IO0: pass -- '0', fail -- '1'
    {
        nand_debug(" to write the page oob fails!\r\n");
        ret = 1;
    }

    //clear sector_cnt_reg!!!
    nand_writel(nfc,NAND_SECTOR_NUM, 0);
    nand_writel(nfc,NAND_SECTOR_NUM, nfc->sector_cnt);
    /*after one page write*/
    nand_writel(nfc, NAND_INT_CLR, 0x7f); //clear

    nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_H);
    nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_WR);
    return ret;
}

#endif

u32 nand_write_ecc(nand_flash *nfc, u32 PageAddr, u8 PageCount, u32* scr_mem_addr)
{
    int i, data_addr_cnt = 0;

    int ret = 0;
    int j = 0;
    int k = 0;
    u8 reg8_val;

    nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_WR);
    /*use 8bit ecc*/

    nand_writel(nfc, NAND_FUNC_CTL, FUNCCTL_ECC8_SEC512);    //8bit ecc
    nand_writel(nfc, NAND_ECCTL, ECCCTL_ECC_EN);
    nand_writel(nfc, NAND_FSPR_CNT, FSPR_CNT_X(nfc->ecc_code_size));
    nand_writel(nfc, NAND_INT_CLR, 0x7f);   //clear all interrupt
    //rd secs
    for (i = 0; i < PageCount; i++)
    {

        nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_L); //set CE to low-level
        nand_write_sector_ecc(nfc, PageAddr);   //dmac master
        PageAddr += 1;
        nand_wait(nfc, NAND_FCMDCTL, FCMDCTL_NORMALTRIG_ST, UNSET_BIT);

        nand_writel(nfc, NAND_INT_CLR,
                INT_BUFSZDATARDY_BIT|INT_FIFOFULL_BIT|CFG_FIFOEMPTY_BIT);
        /*write fifo and start data write*/
        for (j = 0; j < (nfc->sector_cnt); j++)
        {
            if (j > 0)
            {
                nand_write_sector_random(nfc, nfc->sector_size * j);
                nand_en_spare_cmd(nfc, nfc->info.page_size + nfc->ecc_code_loc + nfc->ecc_code_sector_off * j, NAND_RD_PROGRAM);
            }
            for (k = 0; k < nfc->sector_size >> 2; k++)
            {
                nand_wait(nfc, NAND_INT, INT_FIFOFULL_BIT, UNSET_BIT);
                nand_writel(nfc, NAND_AHB_FiFo, *(scr_mem_addr + data_addr_cnt));
                data_addr_cnt++;
            }

            /*trig read/write process*/
            nand_writel(nfc, NAND_FDBACTL,
                    nand_readl(nfc, NAND_FDBACTL)|FDBACTL_DTAUTO_TRIG);
            /*wait data transfer end*/
            nand_wait(nfc, NAND_FDBACTL, FDBACTL_DTAUTO_TRIG, UNSET_BIT);
            /*wait fifo empty*/
            nand_wait(nfc, NAND_CONFIG, CFG_FIFOEMPTY_BIT, SET_BIT);
            nand_writel(nfc, NAND_INT_CLR,
                    INT_ECCERR_BIT|INT_ECCDN_BIT|INT_BUFSZDATARDY_BIT|INT_FIFOFULL_BIT|CFG_FIFOEMPTY_BIT);
        }
        /*after one page write*/
        nand_writel(nfc, NAND_FACOMM_1, NAND_PROGRAM2);
        nand_writel(nfc, NAND_FCMDCTL,
                FCMDCTL_SINGLETRIG_ST|FCMDCTL_NORMALTRIG_ST|FCMDCTL_FAS01234_ST); //

        /*wait for command end*/
        nand_wait(nfc, NAND_FCMDCTL, FCMDCTL_NORMALTRIG_ST, UNSET_BIT);
        nand_wait(nfc, NAND_INT, INT_FLSRDY_BIT, SET_BIT);

        /*read status*/
        reg8_val = nand_read_status(nfc);
        if (reg8_val & 0x01) //IO0: pass -- '0', fail -- '1'
        {
            nand_debug(" to write the page fails!\r\n");
            nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_WR); //DMA write direction
            return 1;
        }
        /*after one page write*/
        nand_writel(nfc, NAND_INT_CLR, 0x7f); //clear
    }
    //nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_H);
    //nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_WR);
    return ret;
}

u8 nand_get_ecc_error_number(nand_flash *nfc)
{
    u8 erro_number;
    erro_number = (nand_readl(nfc, NAND_ECCTL) & ECCCTL_ERROR_NB_MASK) >> ECCCTL_ERROR_NB_SHFT;
    return erro_number;
}
u32 nand_read_ecc(nand_flash *nfc, u32 PageAddr, u8 PageCount, u32* dst_mem_addr)
{
    int i, k, data_addr_cnt = 0;
    u32 tmp32;
    u32* DestAddr;
    int ret = 0;
    int j = 0;
    nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_RD);
    /*use 8bit ecc*/
    nand_writel(nfc, NAND_FUNC_CTL, FUNCCTL_ECC8_SEC512);    //8bit ecc
    nand_writel(nfc, NAND_ECCTL, ECCCTL_ECC_EN);
    nand_writel(nfc, NAND_FSPR_CNT, FSPR_CNT_X(nfc->ecc_code_size));
    nand_writel(nfc, NAND_INT_CLR, 0x7f); //clear all interrupt
    //rd secs
    for (i = 0; i < PageCount; i++)
    {
        j = 0;
        nand_read_sector_ecc(nfc, PageAddr);    //dmac master
        PageAddr += 1;
        /*wait auto cmd findish*/
        nand_wait(nfc, NAND_FATCTL, FATCTL_CMDDT_PROC_TRIG, UNSET_BIT);
        /*trig read/write process*/
        for(k = 0; k < nfc->sector_cnt; k++)
        {
            if (k > 0)
            {
                nand_read_sector_random(nfc, nfc->sector_size * k);
                nand_en_spare_cmd(nfc, nfc->info.page_size + nfc->ecc_code_loc + nfc->ecc_code_sector_off * k, NAND_RD_READ);
                nand_wait(nfc, NAND_FDBACTL, FDBACTL_DTAUTO_TRIG, UNSET_BIT);
            }
            nand_wait(nfc, NAND_INT, INT_FIFOFULL_BIT, SET_BIT);
            nand_wait(nfc, NAND_FCMDCTL, FCMDCTL_NORMALTRIG_ST, UNSET_BIT);
            /* wait ecc done */
            nand_wait(nfc, NAND_INT, INT_ECCDN_BIT, SET_BIT);

            if (nand_readl(nfc, NAND_ECCTL1) | (nand_readl(nfc, NAND_INT) & INT_ECCERR_BIT))
            {
                nand_debug("ecc erro greater than ecc capability : sector[%d] \n", k);
                nand_debug("error number : %d \n", nand_get_ecc_error_number(nfc));

                nand_debug("NAND_INT    : 0x%x \n", nand_readl(nfc, NAND_INT));
                nand_debug("NAND_ECCTL  : 0x%x \n", nand_readl(nfc, NAND_ECCTL));
                nand_debug("NAND_ECCTL1 : 0x%x \n", nand_readl(nfc, NAND_ECCTL1));
            }
            else
            {/*
                nand_debug("error number : %d \n", nand_get_ecc_error_number(nfc));
                nand_debug("NAND_INT    : 0x%x \n", nand_readl(nfc, NAND_INT));
                nand_debug("NAND_ECCTL  : 0x%x \n", nand_readl(nfc, NAND_ECCTL));
                nand_debug("NAND_ECCTL1 : 0x%x \n", nand_readl(nfc, NAND_ECCTL1));*/
            }

            for (j = 0; j < nfc->sector_size >> 2; j++)
            {
                //tmp32 = nand_readl(nfc, NAND_AHB_FiFo);
                //nand_writel(nfc, (u32)(dst_mem_addr + data_addr_cnt), tmp32);
                dst_mem_addr[data_addr_cnt] = nand_readl(nfc, NAND_AHB_FiFo);
                data_addr_cnt++;
            }
            nand_wait(nfc, NAND_INT, INT_FIFOEMPTY_BIT, SET_BIT);
            nand_writel(nfc, NAND_INT_CLR,
                    INT_FIFOEMPTY_BIT|INT_FIFOFULL_BIT|INT_BUFSZDATARDY_BIT);
        }
        /*after one page read*/
        nand_writel(nfc, NAND_INT_CLR, 0x7f);   //clear
    }
    nand_writel(nfc, NAND_FDSEL, FLASH_CHCE_H);
    nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_RD);
    return ret;
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
u32 nand_read(nand_flash *nfc, u32 PageAddr, u8 PageCount, u32* dst_mem_addr)
{
    int i, data_addr_cnt = 0;
    u32 tmp32;
    u32* DestAddr;
    int ret = 0;
    int j = 0;
    int k = 0;

    nand_writel(nfc, NAND_CONFIG, CFG_DMADIR_RD);
    /*use 8bit ecc*/
    //nand_writel(nfc, NAND_FUNC_CTL, 0x0);    //8bit ecc
    nand_writel(nfc, NAND_ECCTL, 0x0);//disable ecc
    //nand_writel(nfc, NAND_FSPR_CNT, 0x0f);
    nand_writel(nfc, NAND_INT_CLR, 0x7f); //clear all interrupt
    //rd secs
    for (i = 0; i < PageCount; i++)
    {
        j = 0;
        nand_read_sector(nfc, PageAddr);    //dmac master
        PageAddr += 1;
        /*wait auto cmd findish*/
        nand_wait(nfc, NAND_FATCTL, FATCTL_CMDDT_PROC_TRIG, UNSET_BIT);
        /*trig read/write process*/
        for (k = 0; k < nfc->sector_cnt; k ++)
        {
            if (j > 0)
            {
                nand_writel(nfc, NAND_FDBACTL,
                            nand_readl(nfc, NAND_FDBACTL)|FDBACTL_DTAUTO_TRIG);
                nand_wait(nfc, NAND_FDBACTL, FDBACTL_DTAUTO_TRIG, UNSET_BIT);
            }
            nand_wait(nfc, NAND_INT, INT_FIFOFULL_BIT, SET_BIT);
            nand_wait(nfc, NAND_FCMDCTL, FCMDCTL_NORMALTRIG_ST, UNSET_BIT);
            for (j = 0; j < nfc->sector_size >> 2; j++)
            {
                //tmp32 = nand_readl(nfc, NAND_AHB_FiFo);
                //nand_writel(nfc, (u32)(dst_mem_addr + data_addr_cnt), tmp32);
                dst_mem_addr[data_addr_cnt] = nand_readl(nfc, NAND_AHB_FiFo);
                data_addr_cnt++;
            }
            nand_wait(nfc, NAND_INT, INT_FIFOEMPTY_BIT, SET_BIT);
            nand_writel(nfc, NAND_INT_CLR,
                    INT_FIFOEMPTY_BIT|INT_FIFOFULL_BIT|INT_BUFSZDATARDY_BIT);
        }
        /*after one page read*/
        nand_writel(nfc, NAND_INT_CLR, 0x7f);   //clear
    }
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
    nand_writel(nfc, NAND_ECCTL, 0x0);//disable ecc
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

nand_flash nfc;

u32 data_write[8][128];
u32 data_read[8][128];
u32 data_read_ecc[8][128];

u8 oob_rdata[218];
u16 res_no_ecc_fail_nb = 0;
u16 res_ecc_fail_nb = 0;
u16 res_no_ecc_pass_nb = 0;
u16 res_ecc_pass_nb = 0;
void nand_test_page(u32 page_addr)
{
    u32 i, j;
    u8 res_no_ecc = 0;
    u8 res_ecc = 0;
#if 1

    for (i = 0; i < nfc.sector_cnt; i++)
    {
        for (j = 0; j < (nfc.sector_size >> 2) ; j++)
        {
            //data_write[i][j] = (i << 24) | ((j + i) << 16) | ((j + i) << 8) | (j + i);
            data_write[i][j] = CK_Timer_CurrentValue(0);
        }
    }
    nand_block_erase(&nfc, page_addr);

    nand_write_ecc(&nfc, page_addr, 1, data_write);
    nand_read_ecc(&nfc, page_addr, 1, data_read_ecc);

    nand_read(&nfc, page_addr, 1, data_read);
    nand_read_oob(&nfc, page_addr, &oob_rdata[0]);
    //nand_read_ecc(&nfc, page_addr, 1, data_read_ecc);

    for (i = 0; i < nfc.sector_cnt; i++)
    {
        for (j = 0; j < (nfc.sector_size >> 2); j++)
        {
            if (data_write[i][j] != data_read_ecc[i][j])
            {
                nand_debug("en ecc %d  %d   Wvalue  = 0x%x   Rvalue = 0x%x \r\n", i,j ,data_write[i][j], data_read_ecc[i][j]);
                res_ecc = 1;
            }
            if (data_write[i][j] != data_read[i][j])
            {
                nand_debug("no ecc %d  %d   Wvalue  = 0x%x   Rvalue = 0x%x \r\n", i,j ,data_write[i][j], data_read[i][j]);
                res_no_ecc = 1;
            }
        }
    }
    if (res_ecc == 1)
    {
        nand_debug("ecc_code\n");
        for (i = 0; i < nfc.sector_cnt; i++)
        {
            nand_debug("sector[%d] :", i);
            u16 ecc_loc = nfc.ecc_code_loc + nfc.ecc_code_sector_off * i;
            u16 ecc_end = ecc_loc +nfc.ecc_code_size ;
            for (j = ecc_loc; j < ecc_end; j++)
            {
                nand_debug("0x%x ", oob_rdata[j]);
            }
            nand_debug("\n");
        }
    }


    if (res_ecc == 1)
    {
        nand_debug("nand test with hw-ecc failed !!!\r\n");
        res_ecc_fail_nb ++;
    }
    else
    {
        res_ecc_pass_nb ++;
        nand_debug("nand test with hw-ecc succeed !!!\r\n");
    }

    if (res_no_ecc == 1)
    {
        res_no_ecc_fail_nb ++;
        nand_debug("nand test without hw-ecc failed !!!\r\n");
    }
    else
    {
        res_no_ecc_pass_nb ++;
        nand_debug("nand test without hw-ecc succeed !!!\r\n");
    }
    nand_block_erase(&nfc, page_addr);
#endif
}
void CK_nfc_test(void)
{

    u32 i;
    nfc.base = NFC_BASE_ADDR;
    nand_init(&nfc);

    for (i = 0; i < 100; i++)
    {
        nand_test_page(0x0);
        nand_test_page(0x2);
        nand_test_page(0x40);
        nand_test_page(0x42);
    }

    printf("no ecc test pass %d times fail %d times\r\n", res_no_ecc_pass_nb, res_no_ecc_fail_nb);
    printf("hw ecc test pass %d times fail %d times\r\n", res_ecc_pass_nb, res_ecc_fail_nb);
    printf("nfc.ecc_code_size %d nfc.ecc_code_loc %d\r\n", nfc.ecc_code_size, nfc.ecc_code_loc);
}



