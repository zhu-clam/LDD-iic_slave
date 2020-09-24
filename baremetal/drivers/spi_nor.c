/*****************************************************************************
 *  File: spi.c
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

#include "ck810.h"
#include "datatype.h"
#include "misc.h"
#include "spi_nor.h"
#include "ahbdma.h"
#include "axidma.h"
#include "intc.h"

static CK_UINT32 SPI_READ_NUM;
static CK_UINT8 SPI_READ_BUF[NOR_TEST_LEN];
static CK_UINT32 SPI_READ_INDEX;
static CKStruct_SpiMInfo CK_Spi_Master = {CK_INTC_QSPI, FALSE, 0};
int spi_nor_int_mode = 0;

static void spi_write_word(void *buffer, CK_UINT32 number, CK_UINT8 PortN);

void spi_irq_handler(CK_UINT32 irq)
{
    CK_UINT32 val;

    val = read_mreg32(SPI_INTSTAT);
    write_mreg32(SPI_INTCLR, val);
#if CK_SPI_M_NOR_DEBUG
    printf("spi_irq_handler SPI_INTSTAT=0x%x \r\n", val);
#endif

    if (val & 0x12) {
        do {
            val = read_mreg32(SPI_CSTAT);
#if CK_SPI_M_NOR_DEBUG
            printf("spi_irq_handler SPI_CSTAT=0x%x \r\n",val);
#endif

            if(val & 0x02) {
                SPI_READ_BUF[SPI_READ_INDEX] = read_mreg32(SPI_RXREG);
#if CK_SPI_M_NOR_DEBUG
                printf("spi_irq_handler SPI_READ_BUF[%d]=0x%x\r\n",
                    SPI_READ_INDEX, SPI_READ_BUF[SPI_READ_INDEX]);
#endif
                SPI_READ_INDEX++;
                SPI_READ_NUM--;
            }
       } while (val & 0x02);
    }
}

CK_INT32 spi_register_isr(void(*handler)(CK_UINT32),
                          CK_UINT16 priority, BOOL fast)
{
    PCKStruct_SpiMInfo info;

    info = &CK_Spi_Master;
    if(info->opened)
        return FAILURE;

    /* Initialize IRQ handler */
    if (NULL != handler) {
        info->irqhandler.devname = "SPI_M";
        info->irqhandler.irqid = info->irq;
        info->irqhandler.priority = priority;
        info->irqhandler.handler = handler;
        info->irqhandler.bfast = fast;
        info->irqhandler.next = NULL;
        /* Register timer ISR */
        CK_INTC_RequestIrq(&(info->irqhandler), AUTO_MODE);
    }

    info->opened = TRUE;
    return SUCCESS;
}

CK_INT32 spi_unregister_isr()
{
    PCKStruct_SpiMInfo info;

    info = &CK_Spi_Master;
    if(!info->opened)
        return FAILURE;

    // disable RX Interrupt
    write_mreg32(SPI_INTEN, read_mreg32(SPI_INTEN) & (~0x12));
    write_mreg32(SPI_GCTL, read_mreg32(SPI_GCTL) & (~SPI_IntEn));
    /* Clear the callback function*/
    CK_INTC_FreeIrq(&(info->irqhandler), AUTO_MODE);
    info->opened = FALSE;

    return SUCCESS;
}

void cs_low()
{
    write_mreg32(SPI_SCSR, 0xfe); // Select CS#
}

void cs_high()
{
    write_mreg32(SPI_SCSR, 0xff); // Deselect CS#
}

void spi_reg_init(void)
{
    // 8bit, softcs, no dma,
    // txfifo not full, rxfile available
    // receive disable, trans disable
    // master mode, disable intr, spi disable
    write_mreg32(SPI_GCTL, 0x00000004);
    write_mreg32(SPI_SCSR, 0x000000FF);
    write_mreg32(SPI_INTEN, 0L);
    write_mreg32(SPI_INTCLR, 0x7F);

    // 8bit, MSB, active high clock, clock delay halt
    write_mreg32(SPI_CCTL,  0x00000702); // mode3
    write_mreg32(SPI_SPBRG, 0x00000030);

    // 8bit, softcs, no dma,
    // txfifo not full, rxfile available
    // receive disable, trans enable
    // master mode, disable intr, spi enable
    write_mreg32(SPI_GCTL, 0x0000000D);
}

void spi_reg_write(CK_UINT32 id, CK_UINT32 addr, CK_UINT32 val)
{
    cs_low();
    write_mreg32(SPI_TXREG, 0x08);
    write_mreg32(SPI_TXREG, addr & BYTE_MASK);
    write_mreg32(SPI_TXREG, val & BYTE_MASK);
    while (1) {
        val = read_mreg32(SPI_CSTAT);
        if ( val&0x01 )  break;
    }
    cs_high();
}

CK_UINT32 spi_reg_read(CK_UINT32 id, CK_UINT32 addr)
{
    CK_UINT32 val;
    cs_low();
    write_mreg32(SPI_TXREG, 0x09);

    write_mreg32(SPI_TXREG, addr & 0x1F);
    while (1) {
        val = read_mreg32(SPI_CSTAT);
        if (val&0x01) break;
    }
    write_mreg32(SPI_GCTL, 0x0015);
    write_mreg32(SPI_RXDNR, 0x0001);
    while (1) {
        val = read_mreg32(SPI_CSTAT);
        if ( val&0x02 ) break;
    }
    cs_high();
    val = read_mreg8(SPI_RXREG);
    write_mreg32(SPI_GCTL, 0x000D);
    return val;
}

void SPI_WRCMD(CK_UINT8 *buf, CK_UINT8 len, CK_UINT8 PortN)
{
    CK_UINT32 val;
    CK_UINT8 *pbuf;

    pbuf = (CK_UINT8*)buf;
    // Enable transmit
    write_mreg32(SPI_GCTL, read_mreg32(SPI_GCTL) | SPI_TxEn);

    //Write data
    while (1) {
        val = read_mreg32(SPI_CSTAT);
        if ((val & 0x04) == 0x0) { // TX FIFO is not full
            write_mreg32(SPI_TXREG, *pbuf);
            pbuf++;
            len--;
            if (len == 0) break;
        }
    }

    // Wait transfer finish
    while (1) {
        val = read_mreg32(SPI_CSTAT);
        if (val & 0x01) break;
    }

    // Disable transmit
    write_mreg32(SPI_GCTL, read_mreg32(SPI_GCTL) & (~SPI_TxEn));
}

CK_UINT32 SPI_READID(CK_UINT8 PortN)
{
    CK_UINT8 idcode[5];
    CK_UINT8 cmd;

    cmd = NOR_CMD_RDID;
    cs_low();
    SPI_WRCMD(&cmd, 0x01, PortN);
    spi_read_word(&idcode[0], 0x03, PortN);
    cs_high();

    return (idcode[0] << 16) | (idcode[1] << 8) | (idcode[2]);
}

CK_UINT32 SPI_READLSR(CK_UINT8 PortN)
{
    CK_UINT32 id = 0;
    CK_UINT8 cmd;

    cmd = NOR_CMD_RSRL;
    cs_low();
    SPI_WRCMD(&cmd, 0x01, PortN);
    spi_read_word(&id, 0x01, PortN);
    cs_high();

    return (id & 0xff);
}

CK_UINT32 SPI_READMSR(CK_UINT8 PortN)
{
    CK_UINT32 id = 0;
    CK_UINT8 cmd;

    cmd = NOR_CMD_RSRM;
    cs_low();
    SPI_WRCMD(&cmd, 0x01, PortN);
    spi_read_word(&id, 0x01, PortN);
    cs_high();

    return (id & 0xff);
}

void SPI_WRITEMSR(CK_UINT8 PortN, CK_UINT8 status)
{
    CK_UINT8 cmd;
    CK_UINT8 tmp[4];

    cs_low();
    cmd = NOR_CMD_WREN;
    SPI_WRCMD(&cmd, 0x01, PortN);
    cs_high();

    cs_low();
    tmp[0] = NOR_CMD_WSRM;
    tmp[1] = status;
    spi_write_word(tmp, 2, PortN);
    cs_high();
}

CK_UINT32 SPI_READMID(CK_UINT8 PortN)
{
    CK_UINT8 idcode[5];
    CK_UINT8 cmd;

    cmd = NOR_CMD_REMS;
    cs_low();
    SPI_WRCMD(&cmd, 0x01, PortN);

    cmd = (NOR_MID_ADDR >> 16) & BYTE_MASK;
    SPI_WRCMD(&cmd, 0x01, PortN);

    cmd = (NOR_MID_ADDR >> 8) & BYTE_MASK;
    SPI_WRCMD(&cmd, 0x01, PortN);

    cmd = NOR_MID_ADDR & BYTE_MASK;
    SPI_WRCMD(&cmd, 0x01, PortN);

    spi_read_word(&idcode[0], 0x02, PortN);
    cs_high();

    return (idcode[0] << 8) | (idcode[1]);
}

/*======================================================
    buffer:  data buffer address
    number:  total transfer data byte count
=======================================================*/
static void spi_write_word(void *buffer, CK_UINT32 number, CK_UINT8 PortN)
{
    CK_UINT32 val;
    CK_UINT8 *pbuf;

    pbuf = (CK_UINT8*)buffer;
    //Enable transmit
    write_mreg32(SPI_GCTL, read_mreg32(SPI_GCTL) | SPI_TxEn);

    //Write data
    while (1) {
        val = read_mreg32(SPI_CSTAT);
        if ((val & 0x04) == 0x0) { //fifo is not full
            write_mreg32(SPI_TXREG, *pbuf);
            pbuf++;
            number--;
            if (number == 0) break;
        }
    }

    //wait transfer finish
    while (1) {
        val = read_mreg32(SPI_CSTAT);
        if(val & 0x01) break;
    }

    //disable transmit
    write_mreg32(SPI_GCTL, read_mreg32(SPI_GCTL) & (~SPI_TxEn));
}

/*======================================================
    buffer:  data buffer address
    number:  total transfer data half word count
=======================================================*/
void spi_read_word(void *buffer, CK_UINT32 number, CK_UINT8 PortN)
{
    CK_UINT32 val;
    CK_UINT8 *pbuf;

    pbuf = (CK_UINT8 *)buffer;
    write_mreg32(SPI_RXDNR, number);
    //Enable SPI receive
    write_mreg32(SPI_GCTL, read_mreg32(SPI_GCTL) | SPI_RxEn);

    if (spi_nor_int_mode) {
        SPI_READ_NUM = number;
        SPI_READ_INDEX = 0;
        while (SPI_READ_NUM); // Wait ISR read data then decrease it.
        for (val = 0; val < number; val++) {
            pbuf[val] = SPI_READ_BUF[val];
        }
    } else {
        while (1) {
            val = read_mreg32(SPI_CSTAT);
            if (val & 0x02) {
                *pbuf = read_mreg32(SPI_RXREG);
                pbuf++;
                number--;
                if (number == 0) break;
            }
        }
    }

    //disable receive
    write_mreg32(SPI_GCTL, read_mreg32(SPI_GCTL) & (~SPI_RxEn));
}

//****************************************************************
//Check whether the Flash is busy now.
//****************************************************************
void flash_checkbusy(CK_UINT8 PortN)
{
    CK_UINT32 val;

    while (1) {
        val = SPI_READLSR(PortN);
        //Check the busy bit
        if ((val & STATUS_WIP) == 0x0)
            break;
    }
}

/****************************************************************************************************
*  init spi flash controller
****************************************************************************************************/
void spi_init(CK_UINT8 PortN)
{
    //Master//Disable SPI//Tx Disable//Rx Disable//Rx TLF//Tx TLF//DMA Disable//CS By SoftWare//8 Bit
    write_mreg32(SPI_GCTL, (SPI_dual_modeDis | SPI_quad_modeDis | \
                            SPI_Normalmode | SPI_Tx1Triglevel | \
                            SPI_Rx1Triglevel | SPI_MasterMode | \
                            SPI_SPIEn | SPI_IntDis));
    //Normal Clock//Character Length 8//MSB//Active Low
    //write_mreg32(SPI_CCTL, (SPI_Length8 | SPI_MSBFirst | SPI_CKPLH | SPI_CKPHL | SPI_RxEdgeHS)); // mode 3
    write_mreg32(SPI_CCTL, (SPI_Length8 | SPI_MSBFirst | SPI_CKPLL | SPI_CKPHH | SPI_RxEdgeHS)); // mode 0
    if (spi_nor_int_mode) {
        write_mreg32(SPI_INTEN, 0x12); //Enable Recv Interrupt
        spi_register_isr(spi_irq_handler, CK_INTC_QSPI, FALSE);
        SPI_READ_NUM = 0;
        SPI_READ_INDEX = 0;
        write_mreg32(SPI_GCTL, read_mreg32(SPI_GCTL) | SPI_IntEn);
    } else {
        write_mreg32(SPI_INTEN, 0); //Disable All Interrupt
    }
    write_mreg32(SPI_INTCLR, 0x7F); //Clear All Interrupt
    write_mreg32(SPI_RXDNR, 0x01); //Hold Count
    write_mreg32(SPI_SCSR, 0xFF); //De-select All

#if CONFIG_IS_ASIC
        write_mreg32(SPI_SPBRG, 3); // 33MHz
#else
    write_mreg32(SPI_SPBRG, QSPI_DEFAULT_FREQ / 500000); // 500KHz
#endif
}

void spi_read_byte(CK_UINT8 *dst_data, CK_UINT32 length,
                   CK_UINT32 offset, CK_UINT8 PortN)
{
    CK_UINT8 temp[4];
    flash_checkbusy(PortN);
    //select the device
    cs_low();
    temp[0] = NOR_CMD_READ;
    temp[1] = ((offset >> 16) & BYTE_MASK); //address
    temp[2] = ((offset >> 8) & BYTE_MASK);
    temp[3] = (offset & BYTE_MASK);
    spi_write_word(temp, 4, PortN);

    spi_read_word(dst_data, length, PortN);
    cs_high();
}

void spi_fast_read_byte(CK_UINT8 *dst_data, CK_UINT32 length,
                        CK_UINT32 offset, CK_UINT8 PortN)
{
    CK_UINT8 temp[4];
    CK_UINT32 value;

    flash_checkbusy(PortN);

    //select the device
    cs_low();
    temp[0] = NOR_DUAL_FAST_READ;
    temp[1] = ((offset >> 16) & BYTE_MASK); //address
    temp[2] = ((offset >> 8) & BYTE_MASK);
    temp[3] = (offset & BYTE_MASK);
    spi_write_word(temp, 4, PortN);

    // one dummy byte
    temp[0] = 0;
    spi_write_word(temp, 1, PortN);

    value = read_mreg32(SPI_GCTL);
    value = value | SPI_dual_modeEn;
    write_mreg32(SPI_GCTL, value);
    spi_read_word(dst_data, length, PortN);
    cs_high();

    //restore to normal mode
    value &= (~SPI_dual_modeEn);
    write_mreg32(SPI_GCTL, value);
}

void spi_dual_read_byte(CK_UINT8 *dst_data, CK_UINT32 length,
                        CK_UINT32 offset, CK_UINT8 PortN)
{
    CK_UINT8 temp[4];
    CK_UINT32 value;

    flash_checkbusy(PortN);

    //select the device
    cs_low();

    temp[0] = NOR_DUAL_READ;
    spi_write_word(temp, 1, PortN);

    value = read_mreg32(SPI_GCTL);
    value = value | SPI_dual_modeEn;
    write_mreg32(SPI_GCTL, value);

    temp[0] = ((offset >> 16) & BYTE_MASK); //address
    temp[1] = ((offset >> 8) & BYTE_MASK);
    temp[2] = (offset & BYTE_MASK);
    temp[3] = 0x00; // M7:M0
    spi_write_word(temp, 4, PortN);

    spi_read_word(dst_data, length, PortN);
    cs_high();

    //restore to normal mode
    value &= (~SPI_dual_modeEn);
    write_mreg32(SPI_GCTL, value);
}

// Quad Output Fast Read
void spi_quad_fast_read_byte(CK_UINT8 *dst_data, CK_UINT32 length,
                             CK_UINT32 offset, CK_UINT8 PortN)
{
    CK_UINT8 temp[4];
    CK_UINT32 value;

    flash_checkbusy(PortN);

    //select the device
    cs_low();
    temp[0] = NOR_QUAD_FAST_READ;
    temp[1] = ((offset >> 16) & BYTE_MASK); //address
    temp[2] = ((offset >> 8) & BYTE_MASK);
    temp[3] = (offset & BYTE_MASK);
    spi_write_word(temp, 4, PortN);

    // one dummy byte
    temp[0] = 0;
    spi_write_word(temp, 1, PortN);

    value = read_mreg32(SPI_GCTL);
    value = value | SPI_quad_modeEn;
    write_mreg32(SPI_GCTL, value);
    spi_read_word(dst_data, length, PortN);
    cs_high();

    //restore to normal mode
    value &= (~SPI_quad_modeEn);
    write_mreg32(SPI_GCTL, value);
}

void spi_erase_sector(CK_UINT8 PortN, CK_UINT32 offset)
{
    CK_UINT8 cmd;
    CK_UINT32 val;
    CK_UINT8 temp[4];

    val = read_mreg32(SPI_GCTL);
    write_mreg32(SPI_GCTL, (val & 0xFFF7) | SPI_TxEn); //Tx Enable

    cs_low();
    cmd = NOR_CMD_WREN;
    spi_write_word(&cmd, 0x01, PortN);
    cs_high();

    cs_low();
    temp[0] = NOR_CMD_SE;
    temp[1] = ((offset >> 16) & BYTE_MASK); //address
    temp[2] = ((offset >> 8) & BYTE_MASK);
    temp[3] = (offset & BYTE_MASK);
    spi_write_word(temp, 4, PortN);
    cs_high();

    val = read_mreg32(SPI_GCTL);
    write_mreg32(SPI_GCTL, (val & 0xFFF7) | 0x0000); //Tx Disable
    //SPI_CHECKBUSY( );
}

void spi_write_byte(CK_UINT8 *dst_mem, CK_UINT32 data_length,
                    CK_UINT32 addr, CK_UINT8 PortN)
{
    CK_UINT8 cmd[4];

    flash_checkbusy(PortN);
    cs_low();
    cmd[0] = NOR_CMD_WREN;
    spi_write_word(cmd, 0x01, PortN);
    cs_high();

    flash_checkbusy(PortN);
    cs_low();
    cmd[0] = NOR_CMD_PP;
    spi_write_word(cmd, 0x01, PortN);

    cmd[0] = (addr >> 16) & BYTE_MASK;
    cmd[1] = (addr >> 8) & BYTE_MASK;;
    cmd[2] = addr & BYTE_MASK;;
    spi_write_word(cmd, 0x03, PortN);

    spi_write_word(dst_mem, data_length, PortN);
    cs_high();
}

void spi_quad_write_byte(CK_UINT8 *dst_mem, CK_UINT32 data_length,
                    CK_UINT32 addr, CK_UINT8 PortN)
{
    CK_UINT8 cmd[4];
    CK_UINT32 value;

    flash_checkbusy(PortN);
    cs_low();
    cmd[0] = NOR_CMD_WREN;
    spi_write_word(cmd, 0x01, PortN);
    cs_high();

    flash_checkbusy(PortN);
    cs_low();
    cmd[0] = NOR_CMD_QUAD_PP;
    spi_write_word(cmd, 0x01, PortN);

    cmd[0] = (addr >> 16) & BYTE_MASK;
    cmd[1] = (addr >> 8) & BYTE_MASK;;
    cmd[2] = addr & BYTE_MASK;;
    spi_write_word(cmd, 0x03, PortN);

    value = read_mreg32(SPI_GCTL);
    value = value | SPI_quad_modeEn;
    write_mreg32(SPI_GCTL, value);
    spi_write_word(dst_mem, data_length, PortN);
    cs_high();

    //restore to normal mode
    value &= (~SPI_quad_modeEn);
    write_mreg32(SPI_GCTL, value);
}

BOOL spi_quad_enable(CK_UINT8 PortN) {
    CK_UINT32 status;

    status = SPI_READMSR(PortN);
    if (status & STATUS_QE)
        return TRUE;

    flash_checkbusy(PortN);
    cs_low();
    status = NOR_CMD_WREN;
    spi_write_word(&status, 0x01, PortN);
    cs_high();

    SPI_WRITEMSR(PortN, status | STATUS_QE);
    flash_checkbusy(PortN);

    /* read SR and check it */
    status = SPI_READMSR(PortN);
    if (!(status & STATUS_QE)) {
        printf("failed enable flash quad bit\n");
        return FALSE;
    }

    return TRUE;
}

BOOL spi_quad_disable(CK_UINT8 PortN) {
    CK_UINT32 status;

    status = SPI_READMSR(PortN);
    if (!(status & STATUS_QE))
        return TRUE;

    SPI_WRITEMSR(PortN, status & (~STATUS_QE));
    flash_checkbusy(PortN);
    /* read SR and check it */
    status = SPI_READMSR(PortN);
    if (status & STATUS_QE) {
        printf("failed disable flash quad bit\n");
        return FALSE;
    }

    return TRUE;
}

void spi_write_byte_dma(CK_UINT32 dst_mem, CK_UINT32 data_length,
                        CK_UINT32 addr, CK_UINT8 PortN)
{
    CK_UINT8 cmd[4];
    CK_UINT32 val = 0;

    flash_checkbusy(PortN);
    cs_low();
    cmd[0] = NOR_CMD_WREN;
    spi_write_word(cmd, 0x01, PortN);
    cs_high();

    flash_checkbusy(PortN);
    cs_low();
    cmd[0] = NOR_CMD_PP;
    spi_write_word(cmd, 0x01, PortN);

    cmd[0] = (addr >> 16) & BYTE_MASK;
    cmd[1] = (addr >> 8) & BYTE_MASK;;
    cmd[2] = addr & BYTE_MASK;;
    spi_write_word(cmd, 0x03, PortN);

    struct axi_dma_info axi;
    axi.channel = 1;
    axi.direction = CHx_M2P_DMAC;
    axi.src_msize = SRC_MSIZE8;
    axi.dst_msize = DST_MSIZE16;
    axi.src_width = SRC_WIDTH128;
    axi.dst_width = DST_WIDTH8;
    AXI_DMA_TRAN_INIT(axi, dst_mem, (CK_UINT32)PHYSICAL_ADDRESS(SPI_TXREG), data_length);
    val = read_mreg32(SPI_GCTL);
    write_mreg32(SPI_GCTL, (val | SPI_DMAmode | SPI_TxEn | SPI_SPIEn | SPI_Tx4Triglevel));

    AXI_DMA_TRANS(axi);
    val = read_mreg32(SPI_CSTAT);

    while (!(val & SPI_TxEmpty)) {
        val = read_mreg32(SPI_CSTAT);
    }

    cs_high();
}

void spi_read_byte_dma(CK_UINT32 dst_data, CK_UINT32 length,
                       CK_UINT32 offset, CK_UINT8 PortN)
{
    CK_UINT8 temp[4];
    CK_UINT32 val;

    flash_checkbusy(PortN);
    //select the device
    cs_low();
    temp[0] = NOR_CMD_READ;
    temp[1] = ((offset >> 16) & BYTE_MASK); //address
    temp[2] = ((offset >> 8) & BYTE_MASK);
    temp[3] = (offset & BYTE_MASK);
    spi_write_word(temp, 4, PortN);
    //Set SPI_RXDNR
    write_mreg32(SPI_RXDNR, length);

    struct axi_dma_info axi;
    axi.channel = 2;
    axi.direction = CHx_P2M_DMAC;
    axi.src_msize = SRC_MSIZE16;
    axi.dst_msize = DST_MSIZE16;
    axi.src_width = SRC_WIDTH8;
    axi.dst_width = DST_WIDTH128;
    AXI_DMA_TRAN_INIT(axi, (CK_UINT32)PHYSICAL_ADDRESS(SPI_RXREG), (CK_UINT32)dst_data, length);

    val = read_mreg32(SPI_GCTL);
    write_mreg32(SPI_GCTL, (val | SPI_DMAmode | SPI_RxEn | SPI_SPIEn | SPI_Rx4Triglevel));

    AXI_DMA_TRANS(axi);
    val = read_mreg32(SPI_CSTAT);

    while ((val & SPI_RxEmpty)) {
        val = read_mreg32(SPI_CSTAT);
    }

    val = read_mreg32(SPI_GCTL);
    write_mreg32(SPI_GCTL, (val & 0xFFEF) | 0x0000); //Rx Disable
    cs_high();
}

void spi_read_byte_dma_x4(CK_UINT32 dst_data, CK_UINT32 length,
                       CK_UINT32 offset, CK_UINT8 PortN)
{
    CK_UINT8 temp[4];
    CK_UINT32 val;

    flash_checkbusy(PortN);
    //select the device
    cs_low();
    temp[0] = NOR_QUAD_FAST_READ;
    temp[1] = ((offset >> 16) & BYTE_MASK); //address
    temp[2] = ((offset >> 8) & BYTE_MASK);
    temp[3] = (offset & BYTE_MASK);
    spi_write_word(temp, 4, PortN);
    // one dummy byte
    temp[0] = 0;
    spi_write_word(temp, 1, PortN);
    //Set SPI_RXDNR
    write_mreg32(SPI_RXDNR, length);

    struct axi_dma_info axi;
    axi.channel = 2;
    axi.direction = CHx_P2M_DMAC;
    axi.src_msize = SRC_MSIZE16;
    axi.dst_msize = DST_MSIZE16;
    axi.src_width = SRC_WIDTH8;
    axi.dst_width = DST_WIDTH128;
    AXI_DMA_TRAN_INIT(axi, (CK_UINT32)PHYSICAL_ADDRESS(SPI_RXREG), (CK_UINT32)dst_data, length);

    val = read_mreg32(SPI_GCTL);
    write_mreg32(SPI_GCTL, (val | SPI_DMAmode | SPI_RxEn | SPI_SPIEn | SPI_Rx4Triglevel | SPI_quad_modeEn));

    AXI_DMA_TRANS(axi);
    val = read_mreg32(SPI_CSTAT);

    while ((val & SPI_RxEmpty)) {
        val = read_mreg32(SPI_CSTAT);
    }

    val = read_mreg32(SPI_GCTL);
    val &= (~SPI_quad_modeEn);
    write_mreg32(SPI_GCTL, (val & 0xFFEF) | 0x0000); //Rx Disable
    cs_high();
}

void spi_gs2971_write_byte(CK_UINT8 *dst_mem, CK_UINT32 data_length,
                    CK_UINT16 addr, CK_UINT8 PortN)
{
    CK_UINT8 cmd[4];

    flash_checkbusy(PortN);
    cs_low();
    cmd[0] = NOR_CMD_WREN;
    spi_write_word(cmd, 0x01, PortN);
    cs_high();

    flash_checkbusy(PortN);
    cs_low();

    cmd[0] = (addr >> 8) & 0x0F;
    cmd[1] = (addr & BYTE_MASK);

    spi_write_word(cmd, 0x02, PortN);

    spi_write_word(dst_mem, data_length, PortN);
    cs_high();
}

void spi_gs2971_read_byte(CK_UINT8 *dst_data, CK_UINT32 length,
                   CK_UINT16 offset, CK_UINT8 PortN)
{
    CK_UINT8 temp[4];
    flash_checkbusy(PortN);
    //select the device
    cs_low();
    temp[0] = ((offset >> 8) & 0x0f)|0x80000000;
    temp[1] = (offset & 0xff);
    spi_write_word(temp, 2, PortN);

    spi_read_word(dst_data, length, PortN);
    cs_high();
}
