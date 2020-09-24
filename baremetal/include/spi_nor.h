/*****************************************************************************
 *  File: spi.h
 *
 *  Descirption: contains the functions support VeriSilicon SPI Controller.
 *
 *  Copyright (C) : 2018 VeriSilicon.
 *
 *  Author: Chunming Li
 *  Mail:   chunming.li@verisilicon.com
 *  Date:   Feb 13 2018
 *
 *****************************************************************************/

#ifndef __SPI_H__
#define __SPI_H__

#include "ck810.h"
#include "datatype.h"
#include "intc.h"

#define SPI_REG_BASE  QSPI_BASE

// SPI
#define SPI_TXREG                   (volatile CK_UINT32 *)(0x00 + SPI_REG_BASE)
#define SPI_RXREG                   (volatile CK_UINT32 *)(0x04 + SPI_REG_BASE)
#define SPI_CSTAT                   (volatile CK_UINT32 *)(0x08 + SPI_REG_BASE)
#define SPI_INTSTAT                 (volatile CK_UINT32 *)(0x0C + SPI_REG_BASE)
#define SPI_INTEN                   (volatile CK_UINT32 *)(0x10 + SPI_REG_BASE)
#define SPI_INTCLR                  (volatile CK_UINT32 *)(0x14 + SPI_REG_BASE)
#define SPI_GCTL                    (volatile CK_UINT32 *)(0x18 + SPI_REG_BASE)
#define SPI_CCTL                    (volatile CK_UINT32 *)(0x1c + SPI_REG_BASE)
#define SPI_SPBRG                   (volatile CK_UINT32 *)(0x20 + SPI_REG_BASE)
#define SPI_RXDNR                   (volatile CK_UINT32 *)(0x24 + SPI_REG_BASE)
#define SPI_SCSR                    (volatile CK_UINT32 *)(0x28 + SPI_REG_BASE)
#define SPI_MIO_2_3_CTL             (volatile CK_UINT32 *)(0x2c + SPI_REG_BASE)

//SPI_CSTAT
#define SPI_Rxavl_4byte             (1 << 3)
#define SPI_Receive_Empty           (0 << 3)
#define SPI_TxFull                  (1 << 2)
#define SPI_TxNoFull                (0 << 2)
#define SPI_Rxavl                   (1 << 1)
#define SPI_RxEmpty                 (0 << 1)
#define SPI_TxEmpty                 (1 << 0)
#define SPI_TxNoEmpty               (0 << 0)

//SPI_INTSTAT
#define SPI_TxEmptyInt              (1 << 6)
#define SPI_TxNoEmptyInt            (0 << 6)
#define SPI_RxFullInt               (1 << 5)
#define SPI_RxNoFullInt             (0 << 5)
#define SPI_RxMatchInt              (1 << 4)
#define SPI_RxORError               (1 << 3)
#define SPI_TxURError               (1 << 2)
#define SPI_RxIntF                  (1 << 1)
#define SPI_TxIntF                  (1 << 0)

//SPI_INTEN
#define SPI_TxEmptyIntEn            (1 << 6)
#define SPI_RxFullIntEn             (1 << 5)
#define SPI_RxMatchEn               (1 << 4)
#define SPI_RxORIntEn               (1 << 3)
#define SPI_TxURIntEn               (1 << 2)
#define SPI_RxIntEn                 (1 << 1)
#define SPI_TxIntEn                 (1 << 0)

//SPI_INTCLR
#define SPI_TxEmptyIntClr           (1 << 6)
#define SPI_RxFullIntClr            (1 << 5)
#define SPI_RxMatchClr              (1 << 4)
#define SPI_RxORIntClr              (1 << 3)
#define SPI_TxURIntClr              (1 << 2)
#define SPI_RxIntClr                (1 << 1)
#define SPI_TxIntClr                (1 << 0)

//SPI_GCTL
#define SPI_ti_modeDis              (0 << 14)
#define SPI_ti_modeEn               (1 << 14)

#define SPI_quad_modeDis            (0 << 13)
#define SPI_quad_modeEn             (1 << 13)

#define SPI_dual_modeDis            (0 << 12)
#define SPI_dual_modeEn             (1 << 12)

#define SPI_CS_softc                (0 << 10)
#define SPI_CS_hardc                (1 << 10)
#define SPI_DMAmode                 (1 << 9)
#define SPI_Normalmode              (0 << 9)
#define SPI_Tx4Triglevel            (1 << 7)
#define SPI_Tx1Triglevel            (0 << 7)
#define SPI_Rx1Triglevel            (0 << 5)
#define SPI_Rx4Triglevel            (1 << 5)
#define SPI_RxEn                    (1 << 4)
#define SPI_RxDis                   (0 << 4)
#define SPI_TxEn                    (1 << 3)
#define SPI_TxDis                   (0 << 3)
#define SPI_MasterMode              (1 << 2)
#define SPI_SlaveMode               (0 << 2)
#define SPI_IntEn                   (1 << 1)
#define SPI_IntDis                  (0 << 1)
#define SPI_SPIEn                   (1 << 0)
#define SPI_SPIDis                  (0 << 0)

//SPI_CCTL
#define SPI_Length32                (31 << 8)
#define SPI_Length8                 (7 << 8)

#define SPI_TxEdgeHS                (1 << 5)
#define SPI_TxEdgeMS                (0 << 5)

#define SPI_RxEdgeHS                (1 << 4)
#define SPI_RxEdgeMS                (0 << 4)

#define SPI_LSBFirst                (1 << 2)
#define SPI_MSBFirst                (0 << 2)
#define SPI_CKPLH                   (1 << 1)
#define SPI_CKPLL                   (0 << 1)
#define SPI_CKPHH                   (1 << 0)
#define SPI_CKPHL                   (0 << 0)

#define SPI_Slabe0                  (0xfe)

// SPI NOR flash commands
#define NOR_CMD_QUAD_RDID           (0x94)
#define NOR_CMD_DUAL_RDID           (0x92)
#define NOR_CMD_RDID                (0x9F)
#define NOR_CMD_REMS                (0x90)
#define NOR_MID_ADDR                (0x000000)
#define NOR_DID_ADDR                (0x000001)
#define NOR_CMD_RSRL                (0x05)
#define NOR_CMD_RSRM                (0x35)
#define NOR_CMD_RSRH                (0x15)
#define NOR_CMD_WSRL                (0x01)
#define NOR_CMD_WSRM                (0x31)
#define NOR_CMD_WSRH                (0x11)
#define NOR_CMD_READ                (0x03)
#define NOR_DUAL_FAST_READ          (0x3B)
#define NOR_QUAD_FAST_READ          (0x6B)
#define NOR_DUAL_READ               (0xBB)
#define NOR_CMD_FAST_READ           (0x0b)
#define NOR_CMD_WREN                (0x06)
#define NOR_CMD_WRDIS               (0x04)
#define NOR_CMD_SE                  (0x20)
#define NOR_CMD_BE                  (0x52)
#define NOR_CMD_CE                  (0xC7)
#define NOR_CMD_PP                  (0x02)
#define NOR_CMD_QUAD_PP             (0x32)
#define NOR_CMD_FSRD                (0xeb)

#define BIT(nr)                     (1UL << (nr))
/* GD25Q128 NOR flash status register */
#define STATUS_WIP                  BIT(0)
#define STATUS_WEL                  BIT(1)
#define STATUS_BP0                  BIT(2)
#define STATUS_BP1                  BIT(3)
#define STATUS_BP2                  BIT(4)
#define STATUS_BP3                  BIT(5)
#define STATUS_BP4                  BIT(6)
#define STATUS_SRP0                 BIT(7)

#define STATUS_SRP1                 BIT(0)
#define STATUS_QE                   BIT(1)
#define STATUS_SUS2                 BIT(2)
#define STATUS_LB1                  BIT(3)
#define STATUS_LB2                  BIT(4)

#define NOR_TEST_LEN                64
#define BYTE_MASK                   0xFF
#define NOR_FLASH_ID                0xC84018

typedef struct {
    CK_UINT32 irq;      /* the interrupt number of SPI master */
    BOOL      opened;   /* indicate whether have been opened or not */
    CK_UINT32 timeout;  /* the set time (us) */
    CKStruct_IRQHandler irqhandler;
} CKStruct_SpiMInfo, *PCKStruct_SpiMInfo;

// Parameter PortN is not used in all methods.
void spi_init(CK_UINT8 PortN);
void spi_read_word(void *buffer, CK_UINT32 number, CK_UINT8 PortN);
void SPI_WRCMD(CK_UINT8 *buffer, CK_UINT8 length, CK_UINT8 PortN);
CK_UINT32 SPI_READID(CK_UINT8 PortN);
CK_UINT32 SPI_READMSR(CK_UINT8 PortN);
CK_UINT32 SPI_READLSR(CK_UINT8 PortN);
void SPI_WRITEMSR(CK_UINT8 PortN, CK_UINT8 status);
CK_UINT32 SPI_READMID(CK_UINT8 PortN);
void spi_fast_read_byte(CK_UINT8 *dst_data,CK_UINT32 length,CK_UINT32 offset, CK_UINT8 PortN);
void spi_read_byte(CK_UINT8 *dst_data, CK_UINT32 length, CK_UINT32 offset, CK_UINT8 PortN);
void spi_write_byte(CK_UINT8 *dst_mem, CK_UINT32 length, CK_UINT32 addr, CK_UINT8 PortN);
void spi_quad_write_byte(CK_UINT8 *dst_mem, CK_UINT32 length, CK_UINT32 addr, CK_UINT8 PortN);
void spi_dual_read_byte(CK_UINT8 *dst_data, CK_UINT32 length, CK_UINT32 offset, CK_UINT8 PortN);
void spi_quad_fast_read_byte(CK_UINT8 *dst_data, CK_UINT32 length, CK_UINT32 offset, CK_UINT8 PortN);
void flash_checkbusy(CK_UINT8 PortN);
void spi_erase_sector(CK_UINT8 PortN, CK_UINT32 offset);
void spi_irq_handler(CK_UINT32 irq);
CK_INT32 spi_register_isr(void(*handler)(CK_UINT32), CK_UINT16 priority, BOOL fast);
CK_INT32 spi_unregister_isr();
void spi_reg_init(void);
void spi_reg_write(CK_UINT32 id, CK_UINT32 addr, CK_UINT32 val);
CK_UINT32 spi_reg_read(CK_UINT32 id, CK_UINT32 addr);
BOOL spi_quad_enable(CK_UINT8 PortN);
BOOL spi_quad_disable(CK_UINT8 PortN);

#endif
