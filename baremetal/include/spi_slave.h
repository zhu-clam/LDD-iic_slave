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

#ifndef __SPI_SLAVE_H__
#define __SPI_SLAVE_H__

#include "ck810.h"
#include "datatype.h"
#include "intc.h"

#define SPI_TEST_LEN        128
#define SPI_SLAVE_REG_BASE  SPI2_BASE

// SPI Slave
#define SPI_SLAVE_TXREG             (0x00 + SPI_SLAVE_REG_BASE)
#define SPI_SLAVE_RXREG             (0x04 + SPI_SLAVE_REG_BASE)
#define SPI_SLAVE_CSTAT             (0x08 + SPI_SLAVE_REG_BASE)
#define SPI_SLAVE_INTSTAT           (0x0C + SPI_SLAVE_REG_BASE)
#define SPI_SLAVE_INTEN             (0x10 + SPI_SLAVE_REG_BASE)
#define SPI_SLAVE_INTCLR            (0x14 + SPI_SLAVE_REG_BASE)
#define SPI_SLAVE_GCTL              (0x18 + SPI_SLAVE_REG_BASE)
#define SPI_SLAVE_CCTL              (0x1c + SPI_SLAVE_REG_BASE)
#define SPI_SLAVE_SPBRG             (0x20 + SPI_SLAVE_REG_BASE)
#define SPI_SLAVE_RXDNR             (0x24 + SPI_SLAVE_REG_BASE)
#define SPI_SLAVE_SCSR              (0x28 + SPI_SLAVE_REG_BASE)
#define SPI_SLAVE_MIO_2_3_CTL       (0x2c + SPI_SLAVE_REG_BASE)

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

//                                 Mode ckpl ckph
#define		MODE0		(0x01) //  0    0     1
#define		MODE1		(0x00) //  1    0     0
#define		MODE2		(0x03) //  2    1     1
#define		MODE3		(0x02) //  3    1     0

typedef struct {
    CK_UINT32 irq;      /* the interrupt number of SPI slave */
    BOOL      opened;   /* indicate whether have been opened or not */
    CK_UINT32 timeout;  /* the set time (us) */
    CKStruct_IRQHandler irqhandler;
} CKStruct_SpiSInfo, *PCKStruct_SpiSInfo;

void spi_slave_init(CK_UINT8 mode, CK_UINT16 rate);

#endif
