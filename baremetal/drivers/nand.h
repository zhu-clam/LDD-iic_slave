#ifndef __NAND_H__
#define __NAND_H__

#include "ck810.h"
#include "datatype.h"

#define NAND_FCMDCTL                    0x00
#define NAND_FDBACTL                    0x04
#define NAND_FATCTL                     0x08
#define NAND_SPRCMD_CTL                 0x0C
#define NAND_ECCTL                      0x10
#define NAND_FDSEL                      0x14
#define NAND_FACOMM_1                   0x18
#define NAND_FACOMM_2                   0x1C
#define NAND_FA0                        0x20
#define NAND_FA1                        0x24
#define NAND_FA2                        0x28
#define NAND_FA3                        0x2C
#define NAND_FA4                        0x30
#define NAND_FA5                        0x34
#define NAND_FA6                        0x38
#define NAND_FA7                        0x3C
#define NAND_FBCOMM_1                   0x40
#define NAND_FBCOMM_2                   0x44
#define NAND_FA0_B                      0x48
#define NAND_FA1_B                      0x4C
#define NAND_FA2_B                      0x50
#define NAND_FA3_B                      0x54
#define NAND_FA4_B                      0x58
#define NAND_FCTL                       0x5C
#define NAND_FSEQ                       0x60
#define NAND_FEBI                       0x64
#define NAND_FDATA_LB                   0x68
#define NAND_FDATA_HB                   0x6C
#define NAND_FTYPE                      0x70
#define NAND_BUSY_CNT                   0x74
#define NAND_FMAP                       0x78
#define NAND_FUNC_CTL                   0x7C
#define NAND_STB_LWIDTH                 0x80
#define NAND_STB_HWIDTH                 0x84
#define NAND_FSPEC                      0x88
#define NAND_FSTAT                      0x8C
#define NAND_FMAK                       0x90
#define NAND_FDEV                       0x94
#define NAND_Flash_ID3                  0x98
#define NAND_Flash_ID4                  0x9C
#define NAND_Flash_ID5                  0xA0
#define NAND_FSPR_CNT                   0xA4
#define NAND_FCMPCTL                    0xA8
#define NAND_FCMPST                     0xAC
#define NAND_ECCTL1                     0xB0
#define NAND_SECTOR_NUM                 0xB4
#define NAND_FLASH_ID6                  0xB8
#define NAND_FLASH_ID7                  0xBC
#define NAND_FLASH_ID8                  0xC0
#define NAND_FSPR_REG(X)                (0xC4+X*0x04)
#define NAND_INT_EN                     0x2C0
#define NAND_INT_MSK                    0x2C4
#define NAND_INT                        0x2C8
#define NAND_MSKED_INT                  0x2CC
#define NAND_INT_CLR                    0x2D0
#define NAND_CONFIG                     0x2D4
#define NAND_AHB_FiFo                   0x300

#define NAND_BUF_AHB_ADR                0x300

#define NAND_SPARE_MAX                  0x68

#define ECCCTL_ECC_EN                   (0x1<<0x7)      //ECCCTL[7] --- en ecc
#define ECCCTL_ERROR_NB_MASK            (0x7e)          //ECCCTL[6:1] --- the error number
#define ECCCTL_ERROR_NB_SHFT            (0x01)

#define FCTL_MODE_AUTO                  (0x1<<0x0)      // FCTL[0]   --- '1' flash read/write auto mode
#define FCTL_MODE_MCU                   (0)             // FCTL[0]   --- '0' flash read/write MCU mode
#define FCTL_ECCHWAUTOCLR_ST            (0x1<<0x2)      // FCTL[2] --- '1' enable auto clear ECC circuit as starting a FDBA transfer
#define FCTL_DEVSELMOD_DECODE           (0x0<<0x3)      // FCTL[3] --- flash devices selection mode: '0' decoding mode; '1' bit-mapped mode
#define FCTL_DEVSELMOD_BITMAP           (0x1<<0x3)      //
#define FCTL_DEWRITEPROTECT_ST          (0x1<<0x5)      // FCTL[5] --- to cancel write protect for NAND
#define FCTL_PGBLANK_CHK                (0x1<<0x7)      // FCTL[7]  --- Flash Page Blank Check

#define SPRCMD_CTL_NUM_0                (0x00<<0x01)    // FA_B0
#define SPRCMD_CTL_NUM_1                (0x01<<0x01)    // FA_B0 + FA_B1
#define SPRCMD_CTL_NUM_2                (0x02<<0x01)    // FA_B0 + FA_B1 + FA_B2
#define SPRCMD_CTL_NUM_3                (0x03<<0x01)    // FA_B0 + FA_B1 + FA_B2 + FA_B3
#define SPRCMD_CTL_NUM_4                (0x04<<0x01)    // FA_B0 + FA_B1 + FA_B2 + FA_B3 + FA_B4
#define SPRCMD_CTL_EN                   (0x01)      // Flash Ext Spare Address /Command enable.
#define SPRCMD_CTL_DIS                  (0x00)

#define FSEQ_APORTSTS_READY             (0x1<<0x00)     // FSEQ[0]  --- '1' A-port ready; '0' A-port busy
#define FSEQ_APORTSTS_BUSY              (0x0<<0x00)     //
#define FSEQ_FOE                        (0x1<<0x02)     // FSEQ[2]  --- FOE (flash output enable) -- MCU mode
#define FSEQ_FWE                        (0x1<<0x03)     // FSEQ[3]  --- FWE (flash write enable) -- MCU mode
#define FSEQ_FDOE                       (0x1<<0x04)     // FSEQ[4]  --- Set FD output enable -- MCU mode
#define FSEQ_CE                         (0x1<<0x05)     // FSEQ[5]  --- CE (flash chip enable)
#define FSEQ_FCLE                       (0x1<<0x06)     // FSEQ[6]  --- FCLE (flash command latch enable) -- MCU mode
#define FSEQ_FALE                       (0x1<<0x07)     // FSEQ[7]  --- FALE (flas  h address latch enable) -- MCU mode

#define FCMDCTL_FACOMM_IS_CMD           (0x0<<0x00)     // FACOMM_1 is command
#define FCMDCTL_FACOMM_IS_ADDR          (0x1<<0x00)     // FACOMM_1 is address
#define FCMDCTL_NORMALTRIG_ST           (0x1<<0x01)     // FCMDCTL[1] --- Trigger to flash automatic command sequence
#define FCMDCTL_SINGLETRIG_ST           (0x1<<0x02)     // FCMDCTL[2] --- Trigger a signle flash command write action
#define FCMDCTL_SKIPCMDPHS_ST           (0x1<<0x03)     // FCMDCTL[3] --- Skip Command phase for address only in the FACMD sequence
#define FCMDCTL_SKIPFA1FA0_ST           (0x1<<0x04)     // FCMDCTL[4] --- FACMD skip FA1,FA0
#define FCMDCTL_FAS0_ST                 (0x0<<0x05)     // Mode_0    0,0,0         FCOMM+FA0
#define FCMDCTL_FAS01_ST                (0x1<<0x05)     // Mode_1    0,0,1         FCOMM+FA0+FA1
#define FCMDCTL_FAS012_ST               (0x2<<0x05)     // Mode_2    0,1,0         FCOMM+FA0+FA1+FA2
#define FCMDCTL_FAS0123_ST              (0x3<<0x05)     // Mode_3    0,1,1         FCOMM+FA0+FA1+FA2+FA3
#define FCMDCTL_FAS01234_ST             (0x4<<0x05)     // Mode_4    1,0,0         FCOMM+FA0+FA1+FA2+FA3+FA4
#define FCMDCTL_FAS012345_ST            (0x5<<0x05)     // Mode_5    1,0,1         FCOMM+FA0+FA1+FA2+FA3+FA4+FA5
#define FCMDCTL_FAS012346_ST            (0x6<<0x05)     // Mode_6    1,1,0         FCOMM+FA0+FA1+FA2+FA3+FA4+FA5+FA6
#define FCMDCTL_FAS01234567_ST          (0x7<<0x05)     // Mode_7    1,1,1         FCOMM+FA0+FA1+FA2+FA3+FA4+FA5+FA6+FA7

                                                        //                        ECC      0~511B    512~519B    520~527B
#define FDBACTL_MD_NORMALDTMV           (0x01)          // Normal Data Move      ON        Data      ECC code    0xFF
#define FDBACTL_MD_NORMALSPARMV         (0x05)          // Normal Spare Move     ON        Data      ECC code    Spare
#define FDBACTL_MD_SPARMV               (0x04)          // Spare Move            OFF       0xFF      0xFF        Spare
#define FDBACTL_MD_ONLYSP               (0x1<<0x03)
#define FDBACTL_MD_ONLYDT               (0x1<<0x05)
#define FDBACTL_RWIR_RD                 (0x1<<0x06)     // FDBACTL[6]    FW_RDIR, Flash Data Read(1) / Write(0) direction
#define FDBACTL_RWIR_WR                 (0x0<<0x06)     // FDBACTL[6]
#define FDBACTL_DTAUTO_TRIG             (0x1<<0x07)     // FDBACTL[7]    SET_FDBA, Trigger to flash Automatic Data transfer

#define FSPR_CNT_X(x)                   (x-1)

#define FTYPE_16BS_DTWIDTH              (0x1<<0x02)     // FTYPE[2]  ----  '1' Enable the Flash 16bits data bus, '0' Enable the Flash 8bits data bus
#define FTYPE_8BS_DTWIDTH               (0x0<<0x02)
#define FTYPE_2XFACMD                   (0x1<<0x03)     // FTYPE[3]  ----  '1' Enable 2x FACMD cycle function

#define FSPEC_RDID_TRIG                 (0x1<<0x00)     // FSPEC[0]  ----  '1' Trigger the Read Flash ID process
#define FSPEC_RDSTAT_TRIG               (0x1<<0x01)     // FSPEC[1]  ----  '1' Trigger the Read Flash Status Process
#define FSPEC_CM1RDSTSCM2_TRIG          (0x1<<0x02)     // FSPEC[2]  ----  '1' Trigger the Special Flash Read Status: FCOMM1+Read Status+FCoMM2

#define FMAP_FFVALMAP_ST                (0x1<<0x05)     // FMAP[5]   ----  '1' enable the function that puts "0xFF" ob the flash data bus when user triggers write flash process.

#define FATCTL_CMDDT_PROC_TRIG          (0x1<<0x00)     // FATCTL[0] ----  Trigger Flash Auto command& data transfer process
#define FATCTL_AUTOMD_CM1ADRDTCM2       (0x0<<0x01)     //        0,0,0           FCOM1+Address+Data_wr+FCOM2
#define FATCTL_AUTOMD_CM1ADRCHKRDYDT    (0x1<<0x01)     //        0,0,1           FCOM1+Address+Check Ready+Data_rd
#define FATCTL_AUTOMD_CM1ADRDT          (0x2<<0x01)     //        0,1,0           FCOM1+Address+Data
#define FATCTL_AUTOMD_CM1ADRCM2DT       (0x3<<0x01)     //        0,1,1           FCOM1+Address+FCOM2+Data
#define FATCTL_AUTOMD_CM1ADRCM2CHRDYDT  (0x4<<0x01)     //        1,0,0           FCOM1+Address+FCOM2+Check Ready + Data
#define FATCTL_AUTOMD_CM1ADRCM2         (0x5<<0x01)     //        1,0,1           FCOM1+Address+FCOM2

/* FUNC_CTL[2:0] Reserved in spec */
#define FUNCCTL_PGDTBLANKCHK_EN          (0x1<<0x03)    // FUNC_CTL[3] ---- Flash page all data Blank (0xFF) Check function enable
#define FUNCCTL_FAMCDSKIPFA0             (0x1<<0x04)    // FUNC_CTL[4] ---- FACMD only Skip FA0
#define FUNCCTL_ECC15_SEC512             (0x0<<0x05)    // FUNC_CTL[7:5] ---- one sector 512B ecc 15bit
#define FUNCCTL_ECC8_SEC512              (0x1<<0x05)    // FUNC_CTL[7:5] ---- one sector 512B ecc 8bit
#define FUNCCTL_ECC24_SEC1K              (0x2<<0x05)    // FUNC_CTL[7:5] ---- one sector 1024B ecc 24bit
#define FUNCCTL_ECC40_SEC1K              (0x3<<0x05)    // FUNC_CTL[7:5] ---- one sector 1024B ecc 40bit
#define FUNCCTL_ECC60_SEC1K              (0x4<<0x05)    // FUNC_CTL[7:5] ---- one sector 1024B ecc 60bit


#define STBLWIDTH_xCYCLS(x)             (x-1)     //x=0 --> x=5
#define STBHWIDTH_xCYCLS(x)             (x-1)     //x=0 --> x=5

#define INT_ECCERR_BIT               (0x01)       // INT_x_REG[0] ---- ecc uncorrect error interrupt
#define INT_FLSRDY_BIT               (0x02)       // INT_x_REG[1] ---- external flash ready interrupt
#define INT_BUFSZDATARDY_BIT         (0x04)       // INT_x_REG[2] ---- 512B data transfer complete interrupt
#define INT_FIFOEMPTY_BIT            (0x08)       // INT_x_REG[3] ---- fifo empty interrupt
#define INT_FIFOFULL_BIT             (0x10)       // INT_x_REG[4] ---- fifo full interrupt
#define INT_ECCDN_BIT                (0x20)       // INT_x_REG[5] ---- ECC done interrupt
#define INT_page_done                (0x40)       // INT_x_REG[6] ---- one page data transfer complete interrupt
// INT_x_REG[7] ---- Spare compare failed interrupt
#define INT_page_done                (0x40)


#define CFG_DMA_EN                   (0x1<<0x01)  // NFC_CFG[1] ---- dma enable, high active
#define CFG_DMA_DIS                  (0x0<<0x01)
#define CFG_DMADIR_RD                (0x0<<0x02)  // NFC_CFG[2] ----  dma write/read direction, 1'b1: write, 1'b0: read
#define CFG_DMADIR_WR                (0x1<<0x02)
#define CFG_FIFO_CLEAR               (0x1<<0x03)
#define CFG_USBDMA_EN                (0x1<<0x04)  // NFC_CFG[4] ---- usb dma mode enable, high active
#define CFG_USBDMA_DIS               (0x0<<0x04)
#define CFG_FIFOEMPTY_BIT            (0x1<<0x06)  // NFC_CFG[6] ---- fifo empty status flag
#define CFG_FIFOFULL_BIT             (0x1<<0x07)  // NFC_CFG[7] ---- fifo full status flag


#define FLASH_CE_SEL0                0x1
#define FLASH_CE_SEL1                0x2
#define FLASH_CE_SEL2                0x4
#define FLASH_CE_SEL3                0x8
#define FLASH_CE_NONE                0x0
#define FLASH_CHCE_H                 FLASH_CE_NONE
#define FLASH_CHCE_L                 FLASH_CE_SEL0

/**************************************************************************************************
 *nand command define
 ***************************************************************************************************/
#define NAND_RESET 0xFF
#define NAND_READ1 0x00
#define NAND_READ2 0x30
#define NAND_PROGRAM1 0x80
#define NAND_PROGRAM2 0x10
#define NAND_ERASE1 0x60
#define NAND_ERASE2 0xD0
#define NAND_STATUS 0x70
#define NAND_READ_ID 0x90

#define NAND_RD_READ 0xE005
#define NAND_RD_PROGRAM 0x0085

/*parameter for nand id read*/
#define NAND_BUS_INDEX 3 //bus width bit is in id data[3]
#define NAND_BUS16_BIT 0x40 //io6=1: nand is has 16bit width data
#define NAND_PAGE_INDEX 3
#define NAND_PAGE_BIT 0x03

#define NAND_SECTOR_CNT 128 //use 8 bit ecc, so one sector is 128x4 bytes
#define MAX_LEN 0x20000 //128k bytes 

#define CONFIG_NAND_EN_WRITE    0
enum BIT_SET {
    UNSET_BIT,
    SET_BIT
};



typedef struct {
    const char *name;
    u8 M_ID;
    u32 D_ID;
    u32 page_size; //byte counter of one read/write page:512/2048/4096
    u32 oob_size; //spare data size of one page
} nand_flash_info;

typedef struct {
    void *base;
    nand_flash_info info;
    u32 page_shift;
    u32 ecc_code_size; /* ecc code size per sector */
    u32 ecc_code_loc; /* first ecc code loc in oob */
    u32 ecc_code_sector_off; /*  */
    u32 sector_cnt;
    u32 sector_size;
} nand_flash;



/**************************************************************************************************
 *nand function define
 ***************************************************************************************************/
bool load_nand_data(u32* dst_mem);
void nand_read_sector(nand_flash *nfc, u32 p_adr);
int nand_init(nand_flash *nfc);
void nand_reset(nand_flash *nfc);
void nand_read_id(nand_flash *nfc, u8 *m_id, u32 *d_id);
int nand_wait(nand_flash *nfc, u32 reg, u32 bit_mask, enum BIT_SET bitset);

#endif
