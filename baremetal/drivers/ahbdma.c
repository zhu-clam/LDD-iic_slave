/*****************************************************************************
 *  File: ahbdma.c
 *
 *  Descirption: contains the functions support Synopsys AHB DMA Controller.
 *
 *  Copyright (C) : 2018 VeriSilicon.
 *
 *  Author: Chunming Li
 *  Mail:   chunming.li@verisilicon.com
 *  Date:   Feb 13 2018
 *
 *****************************************************************************/
#include "ck810.h"
#include "intc.h"
#include "ahbdma.h"
#include "datatype.h"
#include "misc.h"
#include "uart.h"
#include "ckuart.h"
#include "at24c64.h"
#include "iic.h"
#include "spi_nand.h"
#include "stdlib.h"
#include <string.h>

extern BOOL CK_Uart_Get_FIFO_Access_Mode(CK_Uart_Device uartid);
extern void CK_Console_Init(void);
extern CK_Uart_Device consoleuart;
extern int spi_nand_int_mode;

#define UART_THR(id)    PHYSICAL_ADDRESS((CK_UINT32)CK_UART_ADDRBASE0 + CK_UART_THR + id * 0x1000)
#define UART_RBR(id)    PHYSICAL_ADDRESS((CK_UINT32)CK_UART_ADDRBASE0 + CK_UART_RBR + id * 0x1000)
#define peripheral_uart_tx_2_id(pid) ((pid - 4) / 2)
#define peripheral_uart_rx_2_id(pid) ((pid - 5) / 2)

#define I2C_IC_DATA_CMD(id)         PHYSICAL_ADDRESS((CK_UINT32)CK_I2C0_BASSADDR + 0x10 + id * 0x1000)
#define peripheral_i2c_tx_2_id(pid) ((pid - 4) / 2)
#define peripheral_i2c_rx_2_id(pid) ((pid - 5) / 2)

#define MEM2MEM_TEST_L	        0x1000
#define MEM2MEM_LINK_TEST_L     0x100
#define MEM2MEM_REALIGNMENT_L   0x9
#define MEM2UART0_TEST_L        0x24
#define MEM2I2C_TEST_L          0x20
#define MEM2SPI_TEST_L          0x24
#define MEM2SPIS_RX_TEST_L      0x14
#define MEM2SPIS_TX_TEST_L      0x14
#define MEM_TEST_SRC            0x20000000
#define MEM_TEST_SRC1           0x20000400
#define MEM_TEST_DEST           0x20004000
#define MEM_TEST_DEST1          0x20004400
#define SRAM_TEST_SRC           0xF0000000
#define SRAM_TEST_SRC1          0xF0000400
#define SRAM_TEST_DEST          0xF0009000
#define SRAM_TEST_DEST1         0xF0009400

// Channel 0 and 1
#define DMA_CH01_MAX_BLK_SIZE   1023
// Channel 2 - 7
#define DMA_CH27_MAX_BLK_SIZE   1023
#define DMAH_NUM_CHANNELS       8
// the memory starting address interval for Multi-Channel test
#define MULTI_CHANNEL_MEM_INTVL 0x200

#define DMAC_DMA_COMP_PARAM_1_VAL       (CK_UINT64)0x3820070c88888888
#define DMAC_DMA_COMP_PARAM_2_VAL       (CK_UINT64)0x0000000030041b00
#define DMAC_DMA_COMP_PARAM_3_VAL       (CK_UINT64)0x30041b0030041b00
#define DMAC_DMA_COMP_PARAM_4_VAL       (CK_UINT64)0x30041b0030041b00
#define DMAC_DMA_COMP_PARAM_5_VAL       (CK_UINT64)0x30041b0030041b00
#define DMAC_DMA_COMP_PARAM_6_VAL       (CK_UINT64)0x30041b0000000000
#define DMAC_DMAIDREG_VAL               (CK_UINT64)0x0
#define DMAC_DMA_COMP_ID_VAL            (CK_UINT64)0x3232312a44571110

#ifdef AHB_DMAC_DEBUG
    #define DBG(...) printf(__VA_ARGS__)
#else
    #define DBG(...)
#endif

CK_UINT32 id_regs[2][8] = {{DMAC_DMAIDREG(0), DMAC_DMA_COMP_ID(0),
                        DMAC_DMA_COMP_PARAM_1(0), DMAC_DMA_COMP_PARAM_2(0),
                        DMAC_DMA_COMP_PARAM_3(0), DMAC_DMA_COMP_PARAM_4(0),
                        DMAC_DMA_COMP_PARAM_5(0), DMAC_DMA_COMP_PARAM_6(0)},
                            {DMAC_DMAIDREG(1), DMAC_DMA_COMP_ID(1),
                        DMAC_DMA_COMP_PARAM_1(1), DMAC_DMA_COMP_PARAM_2(1),
                        DMAC_DMA_COMP_PARAM_3(1), DMAC_DMA_COMP_PARAM_4(1),
                        DMAC_DMA_COMP_PARAM_5(1), DMAC_DMA_COMP_PARAM_6(1)}};

CK_UINT64 id_regs_val[8] = {DMAC_DMAIDREG_VAL, DMAC_DMA_COMP_ID_VAL,
                            DMAC_DMA_COMP_PARAM_1_VAL,
                            DMAC_DMA_COMP_PARAM_2_VAL,
                            DMAC_DMA_COMP_PARAM_3_VAL,
                            DMAC_DMA_COMP_PARAM_4_VAL,
                            DMAC_DMA_COMP_PARAM_5_VAL,
                            DMAC_DMA_COMP_PARAM_6_VAL};

// Interrupt status flag
volatile CK_UINT32 DMAC_INT_Flag[2];

// test data
static CK_UINT32 test_mem_data[] = {
    0x00112233, 0x44556677, 0x8899aabb, 0xccddeeff
};

static CK_UINT8 test_char_data[] = {
    'V', 'e', 'r', 'i', 'S', 'i', 'l', 'i', 'c', 'o', 'n', '\n'
};

static CK_UINT8 test_char_data1[] = {
    'n', 'o', 'c', 'i', 'l', 'i', 'S', 'i', 'r', 'e', 'V', '\n'
};

static CK_UINT8 spis_test_patten[20] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
    0x11, 0x12, 0x13, 0x14
};

struct LLI{
    volatile u32 SAR;
    volatile u32 DAR;
    struct LLI  volatile *LLP;
    volatile u32 CTL_L;
    volatile u32 CTL_H;
    volatile u32 Rev[2];
};

#define  LLI_NULL       (( LLI_INST *)0x0)
#define  LLI0_ADDR      0x20010000
#define  LLI1_ADDR      0x20010100


CK_UINT32 ahbdma_testcase_no = 0;
CK_UINT32 passed_case = 0;
// interrupt handler
CKStruct_IRQHandler irqhandler0;
CKStruct_IRQHandler irqhandler1;
CK_UINT8 interrupt_registered[2] = {0, 0};
/****************************************************************
 * DMAC initial
 ****************************************************************/
void DMAC_Init(CK_UINT32 id) {
    //enable DMAC
    write_mreg32(DMAC_DMACFGREG(id), DMAC_EN);//D8+2C0
    //enable DMAC to normal mode
    write_mreg32(DMAC_DMATESTREG(id), DMAC_NORMAL_MODE);//F0

    //clear software request
    write_mreg32(DMAC_REQSRCREG(id), (DMAC_ALL_MASK << 8));//A8-368
    write_mreg32(DMAC_REQDSTREG(id), (DMAC_ALL_MASK << 8));//B0--370
    write_mreg32(DMAC_SGLRQSRCREG(id), (DMAC_ALL_MASK << 8));//B8--378
    write_mreg32(DMAC_SGLRQDSTREG(id), (DMAC_ALL_MASK << 8));//C0--380
    write_mreg32(DMAC_LSTSRCREG(id), (DMAC_ALL_MASK << 8));//C8--388
    write_mreg32(DMAC_LSTDSTREG(id), (DMAC_ALL_MASK << 8));//D8--398

    //disable DMAC channel
    write_mreg32(DMAC_CHENREG(id), (DMAC_ALL_MASK << 8));//E0--3A0

    DMAC_INT_Flag[id] = 0;

    if (interrupt_registered[id] == 0) {
        if (id == 0) {
            memset(&irqhandler0, 0, sizeof(irqhandler0));
            irqhandler0.devname = "AHBDMA";
            irqhandler0.irqid = CK_INTC_AHBDMA(id);
            irqhandler0.priority = CK_INTC_AHBDMA(id);
            irqhandler0.handler = dmac0_isr_handler;
            irqhandler0.bfast = FALSE;
            irqhandler0.next = NULL;
            /* register AHB DMA isr */
            CK_INTC_RequestIrq(&irqhandler0, AUTO_MODE);
        } else {
            memset(&irqhandler1, 0, sizeof(irqhandler1));
            irqhandler1.devname = "AHBDMA";
            irqhandler1.irqid = CK_INTC_AHBDMA(id);
            irqhandler1.priority = CK_INTC_AHBDMA(id);
            irqhandler1.handler = dmac1_isr_handler;
            irqhandler1.bfast = FALSE;
            irqhandler1.next = NULL;
            /* register AHB DMA isr */
            CK_INTC_RequestIrq(&irqhandler1, AUTO_MODE);
        }
        interrupt_registered[id] = 1;
    }
}

/****************************************************************
 * interrupt enable
 ****************************************************************/
void DMAC_Interrupt_en(CK_UINT32 id, CK_UINT32 type) {
    CK_UINT32 tmp32 = 0;

    //clear DMAC interrupt
    write_mreg32(DMAC_CLEARTFR(id), DMAC_ALL_MASK);
    write_mreg32(DMAC_CLEARBLOCK(id), DMAC_ALL_MASK);
    write_mreg32(DMAC_CLEARSRCTRAN(id), DMAC_ALL_MASK);
    write_mreg32(DMAC_CLEARDSTTRAN(id), DMAC_ALL_MASK);
    write_mreg32(DMAC_CLEARERR(id), DMAC_ALL_MASK);
    //check DMAC interrupt status
    tmp32 = read_mreg32(DMAC_STATUSINT(id));
    if (tmp32 != 0) {
        printf("\n\tDMAC Interrupt initial fail,"
                " StatusInt not Zero at beginning!\n");
        printf("                - - - FAIL.\n");
        return;
    }
    write_mreg32(DMAC_MASKTFR(id), (DMAC_ALL_MASK << 8));	// disable transfer int
    write_mreg32(DMAC_MASKBLOCK(id), (DMAC_ALL_MASK << 8));	// disable block int
    write_mreg32(DMAC_MASKSRCTRAN(id), (DMAC_ALL_MASK << 8)); // disable srctran int
    write_mreg32(DMAC_MASKDSTTRAN(id), (DMAC_ALL_MASK << 8)); // disable dsttran int
    write_mreg32(DMAC_MASKERR(id), (DMAC_ALL_MASK << 8));	// disable error int
    if ((type & DMAC_INTERRUPT_BLOCK) != 0) {
        write_mreg32(DMAC_MASKBLOCK(id), (DMAC_ALL_MASK | (DMAC_ALL_MASK << 8)));
    }
    if ((type & DMAC_INTERRUPT_TFR) != 0) {
        write_mreg32(DMAC_MASKTFR(id), (DMAC_ALL_MASK | (DMAC_ALL_MASK << 8)));
    }
    if ((type & DMAC_INTERRUPT_ERROR) != 0) {
        write_mreg32(DMAC_MASKERR(id), (DMAC_ALL_MASK | (DMAC_ALL_MASK << 8)));
    }
}

/****************************************************************
 * DMAC run
 * Enable 8 Channels simultaneously
 ****************************************************************/
void DMAC_RUN_ALL(CK_UINT32 id) {
    write_mreg32(DMAC_CHENREG(id), 0xffff);
}

/****************************************************************
 * DMAC open
 * channel number should be from 0 to 7
 * BlockSize should be from 1 to DMA_CHx_MAX_BLK_SIZE
 ****************************************************************/
void DMAC_Open(CK_UINT32 id, DMAC_CH_INFO *channel, CK_UINT32 channel_number,
                CK_UINT16 BlockSize) {
    if ((channel->ctlLx & DMAC_CTL_INT_EN) != 0) {
        DMAC_INT_Flag[id] &= (~(1<<channel_number));
        DMAC_Interrupt_en(id, DMAC_INTERRUPT_BLOCK);
    }
    DMAC_Config(id, channel, channel_number, BlockSize);
}

void DMAC_Config(CK_UINT32 id, DMAC_CH_INFO *channel, CK_UINT32 channel_number,
                CK_UINT16 BlockSize) {
    CK_UINT32 tmp32 = 0;
    CK_UINT32 max_blk_size = 0;
    //check if the channel exist
    if (channel_number > CC_DMAC_NUM_CHANNELS) {
        printf("\n\tDMAC%d channel doesn't exist!\n", id);
        printf("                - - - FAIL.\n");
        return;
    }

    if (channel_number < 2) {
        max_blk_size = DMA_CH01_MAX_BLK_SIZE;
    } else {
        max_blk_size = DMA_CH27_MAX_BLK_SIZE;
    }
    if (BlockSize > max_blk_size) {
        printf("\n\tDMA%d Block Size too large!\n", id);
        printf("                - - - FAIL.\n");
        return;
    }

    while (1) {
        tmp32 = read_mreg32(DMAC_CHENREG(id));
        if ((tmp32 & (1 << channel_number)) != 0) {
            printf("\n\tDMAC%d is used!\n", id);
        } else {
            break;
        }
    }
    write_mreg32(DMAC_SAR(id, channel_number), channel->sarx);
    write_mreg32(DMAC_DAR(id, channel_number), channel->darx);
    write_mreg32(DMAC_CTLH(id, channel_number), (CK_UINT32)(BlockSize));
    write_mreg32(DMAC_CTL(id, channel_number), channel->ctlLx);
    write_mreg32(DMAC_CFGH(id, channel_number), channel->cfgHx);
    write_mreg32(DMAC_CFG(id, channel_number), channel->cfgLx);

    if ((channel->ctlLx & DMAC_CTL_SRC_GATHER_EN) != 0) {
        write_mreg32(DMAC_SGR(id, channel_number), channel->sgrx);
    }
    if ((channel->ctlLx & DMAC_CTL_DST_SCATTER_EN) != 0) {
        write_mreg32(DMAC_DSR(id, channel_number), channel->dsrx);
    }
    write_mreg32(DMAC_LLP(id, channel_number), channel->llpx);

    DBG("\n\tread_mreg32 (0x%x) = 0x%x\n",
        DMAC_SAR(id, channel_number), read_mreg32(DMAC_SAR(id, channel_number)));
    DBG("\tread_mreg32 (0x%x) = 0x%x\n",
        DMAC_DAR(id, channel_number), read_mreg32(DMAC_DAR(id, channel_number)));
    DBG("\tread_mreg32 (0x%x) = 0x%x\n",
        DMAC_CTLH(id, channel_number), read_mreg32(DMAC_CTLH(id, channel_number)));
    DBG("\tread_mreg32 (0x%x) = 0x%x\n",
        DMAC_CTL(id, channel_number), read_mreg32(DMAC_CTL(id, channel_number)));
    DBG("\tread_mreg32 (0x%x) = 0x%x\n",
        DMAC_CFGH(id, channel_number), read_mreg32(DMAC_CFGH(id, channel_number)));
    DBG("\tread_mreg32 (0x%x) = 0x%x\n",
        DMAC_CFG(id, channel_number), read_mreg32(DMAC_CFG(id, channel_number)));
    DBG("\tread_mreg32 (0x%x) = 0x%x\n",
        DMAC_SGR(id, channel_number), read_mreg32(DMAC_SGR(id, channel_number)));
    DBG("\tread_mreg32 (0x%x) = 0x%x\n",
        DMAC_DSR(id, channel_number), read_mreg32(DMAC_DSR(id, channel_number)));
    DBG("\tread_mreg32 (0x%x) = 0x%x\n",
        DMAC_LLP(id, channel_number), read_mreg32(DMAC_LLP(id, channel_number)));
}

void DMAC_Start(CK_UINT32 id, CK_UINT32 channel_number) {
    write_mreg32(DMAC_CHENREG(id), DMAC_CH_EN(channel_number));
}

void DMAC_Start_Ch_bits(CK_UINT32 id, CK_UINT8 channel_bits) {
    write_mreg32(DMAC_CHENREG(id), channel_bits | (channel_bits << 8));
}

void DMAC_Set_FIFO_MODE(CK_UINT32 id, CK_UINT8 channel_number) {
    CK_UINT8 reg = 0;
    reg = read_mreg32(DMAC_CFGH(id, channel_number));
    write_mreg32(DMAC_CFGH(id, channel_number), reg | DMAC_CFG_FIFO_MODE_1);
}

/****************************************************************
 * Read DMA done information
 ****************************************************************/
CK_UINT32 DMAC_CheckDone(CK_UINT32 id, CK_UINT32 channel_number, CK_UINT8 dma_intr) {
    CK_UINT32 tmp32 = 0;

    if (dma_intr == 1) {
        tmp32 = (DMAC_INT_Flag[id] & (1 << channel_number));
        if(tmp32 != 0) {
            DMAC_INT_Flag[id] &= ~(1 << channel_number);
            return 1;
        } else {
            return 0;
        }
    } else {
        tmp32 = read_mreg32(DMAC_RAWTFR(id));
        tmp32 &= (1 << channel_number);
        if (tmp32 != 0) {
            write_mreg32(DMAC_CLEARTFR(id), tmp32);
            tmp32 = read_mreg32(DMAC_STATUSBLOCK(id));
            if ((tmp32 & (1 << channel_number)) != 0) {
                printf("\n\tStatusBlock should not be set for channel %d,"
                        " Interrupt for channel %d is disabled\n",
                        channel_number);
                printf("                - - - FAIL.\n");
                exit(0);
            }
            return 1;
        } else {
            return 0;
        }
    }
}

/****************************************************************
 * Read DMA done information for any channel
 ****************************************************************/
CK_UINT32 DMAC_CheckDone_Any(CK_UINT32 id) {
    CK_UINT32 tmp32 = 0;
    tmp32 = read_mreg32(DMAC_RAWTFR(id));
    if (tmp32 != 0) {
        return 1;
    } else {
        return 0;
    }
}

/****************************************************************
 * DMAC close channel
 ****************************************************************/
void DMAC_Close(CK_UINT32 id, CK_UINT32 channel_number) {
    //clear software request
    write_mreg32(DMAC_REQSRCREG(id), DMAC_SW_REQ_DIS(channel_number));
    write_mreg32(DMAC_REQDSTREG(id), DMAC_SW_REQ_DIS(channel_number));
    write_mreg32(DMAC_SGLRQSRCREG(id), DMAC_SW_REQ_DIS(channel_number));
    write_mreg32(DMAC_SGLRQDSTREG(id), DMAC_SW_REQ_DIS(channel_number));
    write_mreg32(DMAC_LSTSRCREG(id), DMAC_SW_REQ_DIS(channel_number));
    write_mreg32(DMAC_LSTDSTREG(id), DMAC_SW_REQ_DIS(channel_number));
    //disable DMAC channel
    write_mreg32(DMAC_CHENREG(id), DMAC_CH_DIS(channel_number));
    //clear DMAC interrupt
    write_mreg32(DMAC_CLEARTFR(id), DMAC_INT_CLR(channel_number));
    write_mreg32(DMAC_CLEARBLOCK(id), DMAC_INT_CLR(channel_number));
    write_mreg32(DMAC_CLEARSRCTRAN(id), DMAC_INT_CLR(channel_number));
    write_mreg32(DMAC_CLEARDSTTRAN(id), DMAC_INT_CLR(channel_number));
    write_mreg32(DMAC_CLEARERR(id), DMAC_INT_CLR(channel_number));
    //disable DMAC interrupt
    write_mreg32(DMAC_MASKTFR(id), DMAC_INT_MASK(channel_number));
    write_mreg32(DMAC_MASKBLOCK(id), DMAC_INT_MASK(channel_number));
    write_mreg32(DMAC_MASKSRCTRAN(id), DMAC_INT_MASK(channel_number));
    write_mreg32(DMAC_MASKDSTTRAN(id), DMAC_INT_MASK(channel_number));
    write_mreg32(DMAC_MASKERR(id), DMAC_INT_MASK(channel_number));

    DMAC_INT_Flag[id] &= ~(1 << channel_number);
}

/****************************************************************
 * DMAC0 interrupt handler
****************************************************************/
void dmac0_isr_handler(CK_UINT32 irq) {
    CK_UINT32 tmp32 = 0;
    //read block interrupt status,then clear interrupt
    tmp32 = read_mreg32(DMAC_STATUSTFR(0));
    write_mreg32(DMAC_CLEARTFR(0), tmp32);
    tmp32 = read_mreg32(DMAC_STATUSBLOCK(0));
    DMAC_INT_Flag[0] |= (tmp32 & DMAC_ALL_MASK);

    DBG("\n\t<<<<<< DMA0_IRQHandler >>>>>> DMAC_INT_Flag = 0x%x\n",
        DMAC_INT_Flag[0]);
    write_mreg32(DMAC_CLEARBLOCK(0), tmp32);

    //read error interrupt status,then clear interrupt
    tmp32 = read_mreg32(DMAC_STATUSERR(0));

    //clrar error interrupt
    write_mreg32(DMAC_CLEARERR(0), tmp32);
    if (tmp32 != 0) {
        printf("\n\tDMAC0 error occur\n");
        printf("                - - - FAIL.\n");
    }
    DBG("\n\tDMA0 interrupt!\n");
}

void dmac1_isr_handler(CK_UINT32 irq) {
    CK_UINT32 tmp32 = 0;
    //read block interrupt status,then clear interrupt
    tmp32 = read_mreg32(DMAC_STATUSTFR(1));
    write_mreg32(DMAC_CLEARTFR(1), tmp32);
    tmp32 = read_mreg32(DMAC_STATUSBLOCK(1));
    DMAC_INT_Flag[1] |= (tmp32 & DMAC_ALL_MASK);

    DBG("\n\t<<<<<< DMA1_IRQHandler >>>>>> DMAC_INT_Flag = 0x%x\n",
        DMAC_INT_Flag[1]);
    write_mreg32(DMAC_CLEARBLOCK(1), tmp32);

    //read error interrupt status,then clear interrupt
    tmp32 = read_mreg32(DMAC_STATUSERR(1));

    //clrar error interrupt
    write_mreg32(DMAC_CLEARERR(1), tmp32);
    if (tmp32 != 0) {
        printf("\n\tDMAC1 error occur\n");
        printf("                - - - FAIL.\n");
    }
    DBG("\n\tDMA1 interrupt!\n");
}

void DMAMem2MemOpen(CK_UINT32 id, CK_UINT8 channel, CK_UINT32 src_addr, CK_UINT32 dst_addr,
                    CK_UINT32 count, CK_UINT8 dma_intr, CK_UINT8 UNIT,
                    CK_UINT8 prio, CK_UINT32 llpx, CK_UINT8 llp_src_upd,
                    CK_UINT8 llp_dst_upd) {
    DMAC_CH_INFO channel_info;
    memset(&channel_info, 0, sizeof(channel_info));

    channel_info.sarx = src_addr;
    channel_info.darx = dst_addr;
    channel_info.ctlHx = count;
    channel_info.llpx = llpx;

    switch (UNIT) {
        case Align8:
            channel_info.ctlLx = (DMAC_CTL_M2M_DW | DMAC_CTL_SRC_MSIZE4 |
                                    DMAC_CTL_DEST_MSIZE4 | DMAC_CTL_SINC_INC |
                                    DMAC_CTL_DINC_INC | DMAC_CTL_SRC_TR_WIDTH8 |
                                    DMAC_CTL_DST_TR_WIDTH8 | dma_intr |
                                    llp_src_upd << 28 | llp_dst_upd << 27);
            break;
        case Align16:
            channel_info.ctlLx = (DMAC_CTL_M2M_DW | DMAC_CTL_SRC_MSIZE4 |
                                    DMAC_CTL_DEST_MSIZE4 | DMAC_CTL_SINC_INC |
                                    DMAC_CTL_DINC_INC |
                                    DMAC_CTL_SRC_TR_WIDTH16 |
                                    DMAC_CTL_DST_TR_WIDTH16 | dma_intr |
                                    llp_src_upd << 28 | llp_dst_upd << 27);
            break;
        case Align32:
            channel_info.ctlLx = (DMAC_CTL_M2M_DW | DMAC_CTL_SRC_MSIZE4 |
                                    DMAC_CTL_DEST_MSIZE4 | DMAC_CTL_SINC_INC |
                                    DMAC_CTL_DINC_INC |
                                    DMAC_CTL_SRC_TR_WIDTH32 |
                                    DMAC_CTL_DST_TR_WIDTH32 | dma_intr |
                                    llp_src_upd << 28 | llp_dst_upd << 27);
            break;
        case Align64:
            channel_info.ctlLx = (DMAC_CTL_M2M_DW | DMAC_CTL_SRC_MSIZE4 |
                                    DMAC_CTL_DEST_MSIZE4 | DMAC_CTL_SINC_INC |
                                    DMAC_CTL_DINC_INC |
                                    DMAC_CTL_SRC_TR_WIDTH64 |
                                    DMAC_CTL_DST_TR_WIDTH64 | dma_intr |
                                    llp_src_upd << 28 | llp_dst_upd << 27);
            break;
    }
    channel_info.cfgLx = (DMAC_CFG_HS_SRC_SOFTWARE | DMAC_CFG_HS_DST_SOFTWARE |
                            DMAC_CFG_SRC_HS_POL_H | DMAC_CFG_DST_HS_POL_H |
                            DMAC_CFG_CH_PRIOR(prio));
    //Source handshake mode
    //Destination handshake mode
    //Source handshake polarity
    //Destination handshake polarity
    DMAC_Open(id, &channel_info, channel, count);
}

/************************************************************************
 * channel: DMA channel number
 * src_addr: source address
 * count:   byte length for transfering
 * dma_intr: 0 disable dma interrupt, 1 enable dma interrupt
 ***********************************************************************/
void DMAMem2PeripheralOpen(CK_UINT32 id, CK_UINT8 channel, CK_UINT32 src_addr,
                            CK_UINT32 count, CK_UINT8 peripheral_ID,
                            CK_UINT8 dma_intr,CK_UINT32 PortNum,
                            CK_UINT16 src_gth_cnt, CK_UINT32 src_gth_intvl,
                            CK_UINT32 dst_tr_width, CK_UINT32 dst_msize) {
    DMAC_CH_INFO  channel_info;
    CK_UINT8 p_id = 0;
    memset(&channel_info, 0, sizeof(channel_info));

    if (id == 1) {
        switch(peripheral_ID) {
            case peripheral_uart_tx(0):
            case peripheral_uart_tx(1):
            case peripheral_uart_tx(2):
            case peripheral_uart_tx(3):
            case peripheral_uart_tx(4):
                p_id = peripheral_uart_tx_2_id(peripheral_ID);
                channel_info.sarx = (CK_UINT32)src_addr;
                channel_info.darx = (CK_UINT32)UART_THR(p_id);
                channel_info.ctlHx = count;
                channel_info.ctlLx = DMAC_CTL_M2P_DW | DMAC_CTL_SRC_MSIZE1 |
                                        dst_msize | DMAC_CTL_SINC_INC |
                                        DMAC_CTL_DINC_NO | DMAC_CTL_SRC_TR_WIDTH32 |
                                        dst_tr_width | dma_intr;
                // UART handshaking interface is active High
                channel_info.cfgLx = DMAC_CFG_CH_PRIOR(0) |
                                        DMAC_CFG_HS_SRC_SOFTWARE |
                                        DMAC_CFG_HS_DST_HARDWARE |
                                        DMAC_CFG_DST_HS_POL_H;
                channel_info.cfgHx = DMAC_CFG_DEST_PER(peripheral_ID);
                break;
            case peripheral_spi_tx(2):
            case peripheral_spi_tx(3):
                channel_info.sarx = (CK_UINT32)src_addr;
                channel_info.darx = (CK_UINT32)PHYSICAL_ADDRESS(SPI_TXREG);
                channel_info.ctlHx = count;
                channel_info.ctlLx = DMAC_CTL_M2P_DW | DMAC_CTL_SRC_MSIZE1 |
                                        dst_msize | DMAC_CTL_SINC_INC |
                                        DMAC_CTL_DINC_NO | DMAC_CTL_SRC_TR_WIDTH32 |
                                        dst_tr_width | dma_intr;
                channel_info.cfgLx = DMAC_CFG_CH_PRIOR(0) |
                                        DMAC_CFG_HS_SRC_SOFTWARE |
                                        DMAC_CFG_HS_DST_HARDWARE |
                                        DMAC_CFG_DST_HS_POL_H;
                channel_info.cfgHx = DMAC_CFG_DEST_PER(peripheral_ID);
                break;
        }
    }
    if (id == 0) {
        switch(peripheral_ID) {
#if 0
           case peripheral_spi_slave_tx:
                channel_info.sarx = (CK_UINT32)src_addr;
                channel_info.darx = (CK_UINT32)PHYSICAL_ADDRESS(SPI_SLAVE_TXREG);
                channel_info.ctlHx = count;
                channel_info.ctlLx = DMAC_CTL_M2P_DW | DMAC_CTL_SRC_MSIZE1 |
                                        dst_msize | DMAC_CTL_SINC_INC |
                                        DMAC_CTL_DINC_NO | DMAC_CTL_SRC_TR_WIDTH32 |
                                        dst_tr_width | dma_intr;
                channel_info.cfgLx = DMAC_CFG_CH_PRIOR(0) |
                                        DMAC_CFG_HS_SRC_SOFTWARE |
                                        DMAC_CFG_HS_DST_HARDWARE |
                                        DMAC_CFG_DST_HS_POL_H;
                channel_info.cfgHx = DMAC_CFG_DEST_PER(peripheral_spi_slave_tx);
                break;
#endif
            case peripheral_i2c_tx(1):
            case peripheral_i2c_tx(2):
            case peripheral_i2c_tx(3):
            case peripheral_i2c_tx(4):
                p_id = peripheral_i2c_tx_2_id(peripheral_ID);
                channel_info.sarx = (CK_UINT32)src_addr;
                channel_info.darx = (CK_UINT32)I2C_IC_DATA_CMD(p_id);
                channel_info.ctlHx = count;
                channel_info.ctlLx = DMAC_CTL_M2P_DW | DMAC_CTL_SRC_MSIZE1 |
                                        dst_msize | DMAC_CTL_SINC_INC |
                                        DMAC_CTL_DINC_NO | DMAC_CTL_SRC_TR_WIDTH32 |
                                        dst_tr_width | dma_intr;
                // I2C handshaking interface is active High
                channel_info.cfgLx = DMAC_CFG_CH_PRIOR(0) |
                                        DMAC_CFG_HS_SRC_SOFTWARE |
                                        DMAC_CFG_HS_DST_HARDWARE |
                                        DMAC_CFG_DST_HS_POL_H;
                channel_info.cfgHx = DMAC_CFG_DEST_PER(peripheral_ID);
                break;
        }
    }
    if ((src_gth_cnt != 0) && (src_gth_intvl != 0)) {
        // enable source gather
        channel_info.sgrx = DMAC_SG_SET(src_gth_cnt, src_gth_intvl);
        channel_info.ctlLx |= DMAC_CTL_SRC_GATHER_EN;
    } else {
        channel_info.sgrx = 0x0;
    }
    channel_info.dsrx = 0x0;
    channel_info.llpx = 0x0;
    DMAC_Open(id, &channel_info, channel, count);
}

/**********************************************************************
 * channel: DMA channel number
 * dst_addr: destination address
 * count:   32bit word length for transfering
 * dma_intr: 0 disable dma interrupt, 1 enable dma interrupt
 *********************************************************************/
void DMAPeripheral2MemOpen(CK_UINT32 id, CK_UINT8 channel,CK_UINT32 dst_addr,CK_UINT32 count,
                            CK_UINT8 peripheral_ID,CK_UINT8 dma_intr,
                            CK_UINT32 PortNum, CK_UINT16 dst_sct_cnt,
                            CK_UINT32 dst_sct_intvl) {
    DMAC_CH_INFO  channel_info;
    CK_UINT8 p_id = 0;
    memset(&channel_info, 0, sizeof(channel_info));

    if (id == 1) {
        switch(peripheral_ID) {
            case peripheral_uart_rx(0):
            case peripheral_uart_rx(1):
            case peripheral_uart_rx(2):
            case peripheral_uart_rx(3):
            case peripheral_uart_rx(4):
                p_id = peripheral_uart_rx_2_id(peripheral_ID);
                channel_info.sarx = (CK_UINT32)UART_RBR(p_id);
                channel_info.darx = (CK_UINT32)dst_addr;
                channel_info.ctlHx = count;
                channel_info.ctlLx = DMAC_CTL_P2M_DW | DMAC_CTL_SRC_MSIZE16 |
                                        DMAC_CTL_DEST_MSIZE4 | DMAC_CTL_SINC_NO |
                                        DMAC_CTL_DINC_INC | DMAC_CTL_SRC_TR_WIDTH8 |
                                        DMAC_CTL_DST_TR_WIDTH8 | dma_intr;

                // UART handshaking interface is active High
                channel_info.cfgLx = DMAC_CFG_CH_PRIOR(0) |
                                        DMAC_CFG_HS_SRC_HARDWARE |
                                        DMAC_CFG_HS_DST_SOFTWARE |
                                        DMAC_CFG_SRC_HS_POL_H;
                channel_info.cfgHx = DMAC_CFG_SRC_PER(peripheral_ID);
                break;
            case peripheral_spi_rx(2):
            case peripheral_spi_rx(3):
                channel_info.sarx = (CK_UINT32)PHYSICAL_ADDRESS(SPI_RXREG);
                channel_info.darx = (CK_UINT32)dst_addr;
                channel_info.ctlHx = count;
                channel_info.ctlLx = DMAC_CTL_P2M_DW | DMAC_CTL_SRC_MSIZE16 |
                                        DMAC_CTL_DEST_MSIZE4 | DMAC_CTL_SINC_NO |
                                        DMAC_CTL_DINC_INC | DMAC_CTL_SRC_TR_WIDTH8 |
                                        DMAC_CTL_DST_TR_WIDTH8 | dma_intr;
                channel_info.cfgLx = DMAC_CFG_CH_PRIOR(0) |
                                        DMAC_CFG_HS_SRC_HARDWARE |
                                        DMAC_CFG_HS_DST_SOFTWARE |
                                        DMAC_CFG_SRC_HS_POL_H;
                channel_info.cfgHx = DMAC_CFG_SRC_PER(peripheral_ID);
                break;
        }
    }
#if 0
        case peripheral_spi_slave_rx :
            channel_info.sarx = (CK_UINT32)PHYSICAL_ADDRESS(SPI_SLAVE_RXREG);
            channel_info.darx = (CK_UINT32)dst_addr;
            channel_info.ctlHx = count;
            channel_info.ctlLx = DMAC_CTL_P2M_DW | DMAC_CTL_SRC_MSIZE16 |
                                    DMAC_CTL_DEST_MSIZE4 | DMAC_CTL_SINC_NO |
                                    DMAC_CTL_DINC_INC | DMAC_CTL_SRC_TR_WIDTH8 |
                                    DMAC_CTL_DST_TR_WIDTH8 | dma_intr;
            channel_info.cfgLx = DMAC_CFG_CH_PRIOR(0) |
                                    DMAC_CFG_HS_SRC_HARDWARE |
                                    DMAC_CFG_HS_DST_SOFTWARE |
                                    DMAC_CFG_SRC_HS_POL_H;
            channel_info.cfgHx = DMAC_CFG_SRC_PER(peripheral_spi_slave_rx);
            break;
#endif
    if (id == 0) {
        switch(peripheral_ID) {
            case peripheral_i2c_rx(1):
            case peripheral_i2c_rx(2):
            case peripheral_i2c_rx(3):
            case peripheral_i2c_rx(4):
                p_id = peripheral_i2c_tx_2_id(peripheral_ID);
                channel_info.sarx = (CK_UINT32)I2C_IC_DATA_CMD(p_id);
                channel_info.darx = (CK_UINT32)dst_addr;
                channel_info.ctlHx = count;
                channel_info.ctlLx = DMAC_CTL_P2M_DW | DMAC_CTL_SRC_MSIZE4 |
                                        DMAC_CTL_DEST_MSIZE4 | DMAC_CTL_SINC_NO |
                                        DMAC_CTL_DINC_INC | DMAC_CTL_SRC_TR_WIDTH8 |
                                        DMAC_CTL_DST_TR_WIDTH8 | dma_intr;

                // I2C handshaking interface is active High
                channel_info.cfgLx = DMAC_CFG_CH_PRIOR(0) |
                                        DMAC_CFG_HS_SRC_HARDWARE |
                                        DMAC_CFG_HS_DST_SOFTWARE |
                                        DMAC_CFG_SRC_HS_POL_H;
                channel_info.cfgHx = DMAC_CFG_SRC_PER(peripheral_ID);
                break;
        }
    }

    if ((dst_sct_cnt != 0) && (dst_sct_intvl != 0)) {
        channel_info.dsrx = DMAC_SG_SET(dst_sct_cnt, dst_sct_intvl);
        channel_info.ctlLx |= DMAC_CTL_DST_SCATTER_EN;
    } else {
        channel_info.dsrx = 0x0;
    }
    channel_info.sgrx = 0x0;
    channel_info.llpx = 0x0;
    DMAC_Open(id, &channel_info, channel, count);
}

void CK_AHBDMA_UART_Loopback_Test(CK_UINT32 dma_id, CK_UINT32 id,
                            BOOL gather, BOOL scatter, CK_UINT8 dma_intr,
                            CK_UINT32 dst_tr_width, CK_UINT32 dst_msize) {
    CK_UINT32 i = 0;
    CK_UINT8 data = 0;
    CK_UINT32 timeout = 0;
    CK_UINT32 open_len = 0;

    CK_UINT16 src_gth_cnt = 0;
    CK_UINT32 src_gth_intvl = 0;
    CK_UINT32 src_intvl_cnt = 0;
    CK_UINT32 src_mem_len = 0;
    CK_UINT32 src_tr_width = 4; // in bytes

    CK_UINT16 dst_sct_cnt = 0;
    CK_UINT32 dst_sct_intvl = 0;
    CK_UINT32 dst_mem_len = 0;
    CK_UINT32 dst_intvl_cnt = 0;

    if (scatter == TRUE) {
        dst_sct_cnt = 12; // which is the number of CTLx.DST_TR_WIDTH in 1 loop
        dst_sct_intvl = 10; // source gather interval: 10 of CTLx.DST_TR_WIDTH
        dst_intvl_cnt = (MEM2UART0_TEST_L - 1) / dst_sct_cnt;
    }

    dst_mem_len = MEM2UART0_TEST_L + dst_sct_intvl * dst_intvl_cnt;

    if (gather == TRUE) {
        src_gth_cnt = 12; // which is the number of CTLx.SRC_TR_WIDTH in 1 loop: 12 bytes
        src_gth_intvl = 2; // source gather interval: 10 of CTLx.SRC_TR_WIDTH: 8 bytes
        src_intvl_cnt = (MEM2UART0_TEST_L - 1) / (src_gth_cnt * src_tr_width);
    }

    printf("\n%d. Memory to UART%d Transfer%s with"
            " DST_TR_WIDTH %d, DST_MSIZE %d . . .\n"
            "   & UART%d to Memory Transfer%s . . .\n",
                            ahbdma_testcase_no++, id,
                            gather ? " in Gather" : "",
                            DST_TR_WIDTH(dst_tr_width),
                            DST_MSIZE(dst_msize), id,
                            scatter ? " in Scatter" : "");


    // Initialize a memory area for DMA to get the data from
    if (dst_tr_width == DMAC_CTL_DST_TR_WIDTH32) {
        src_mem_len = MEM2UART0_TEST_L * 4 +
                        src_gth_intvl * src_intvl_cnt * src_tr_width;
        memset((void *)MEM_TEST_SRC, 0, src_mem_len);
        for (i = 0; i < MEM2UART0_TEST_L; i++) {
            CK_UINT32 tmp_intvl = 0;
            if (src_gth_cnt != 0) {
                tmp_intvl = ((i * 4) / (src_gth_cnt * src_tr_width)
                            * (src_gth_intvl * src_tr_width));
            }
            write_mreg8(MEM_TEST_SRC + i * 4 + tmp_intvl,
                        test_char_data[i % 12]);
        }
        open_len = MEM2UART0_TEST_L;
    } else if (dst_tr_width == DMAC_CTL_DST_TR_WIDTH8) {
        src_mem_len = MEM2UART0_TEST_L +
                        src_gth_intvl * src_intvl_cnt * src_tr_width;
        memset((void *)MEM_TEST_SRC, 0, src_mem_len);
        for (i = 0; i < MEM2UART0_TEST_L; i++) {
            CK_UINT32 tmp_intvl = 0;
            if (src_gth_cnt != 0) {
                tmp_intvl = (i / (src_gth_cnt * src_tr_width))
                                * (src_gth_intvl * src_tr_width);
            }
            write_mreg8(MEM_TEST_SRC + i + tmp_intvl, test_char_data[i % 12]);
        }
        open_len = MEM2UART0_TEST_L / 4;
    } else {
        printf("\n\tDMAC_CTL_DST_TR_WIDTH %d not supported in test for now\n",
                DST_TR_WIDTH(dst_tr_width));
        printf("                - - - FAIL.\n");
        return;
    }

    // clear a memory area starting from MEM_TEST_DEST used for reading TFR into
    memset((void *)MEM_TEST_DEST, 0, dst_mem_len);

    // Configure channel 1 to transfer data
    DMAC_Init(dma_id);
    DMAMem2PeripheralOpen(dma_id, DMA_CHANNEL_1, MEM_TEST_SRC,
                        open_len, peripheral_uart_tx(id),
                        dma_intr, 0, src_gth_cnt, src_gth_intvl,
                        dst_tr_width, dst_msize);

    printf("\n\tstart transfer\n\n");
    // wait for all transmit done
    delay(50);
    CK_Uart_Open(id, NULL);
    // we use loopback to do UART DMA test
    CK_Uart_Set_Loopback_Mode(id);
    CK_Uart_Set_FIFO_Access_Mode(id);

    // Configure channel 2 to receive data
    DMAPeripheral2MemOpen(dma_id, DMA_CHANNEL_2, MEM_TEST_DEST, MEM2UART0_TEST_L,
                            peripheral_uart_rx(id), dma_intr, 0,
                            dst_sct_cnt, dst_sct_intvl);

    DMAC_Start_Ch_bits(dma_id, (1 << DMA_CHANNEL_1) | (1 << DMA_CHANNEL_2));

    while (1) {
        if (DMAC_CheckDone(dma_id, DMA_CHANNEL_1, dma_intr) == 1) {
            break;
        } else {
            if (timeout++ > 0x5000) {
                DMAC_Close(dma_id, DMA_CHANNEL_1);
                CK_Uart_Unset_Loopback_Mode(id);
                printf("\n\twaiting for CH%d Done timeout!\n", DMA_CHANNEL_1);
                printf("                - - - FAIL.\n");
                return;
            }
        }
    }
    timeout = 0;
    while (1) {
        if (DMAC_CheckDone(dma_id, DMA_CHANNEL_2, dma_intr) == 1) {
            break;
        } else {
            if (timeout++ > 0x5000) {
                DMAC_Close(dma_id, DMA_CHANNEL_2);
                CK_Uart_Unset_Loopback_Mode(id);
                printf("\n\twaiting for CH%d Done timeout!\n", DMA_CHANNEL_2);
                printf("                - - - FAIL.\n");
                return;
            }
        }
    }
    DMAC_Close(dma_id, DMA_CHANNEL_1);
    DMAC_Close(dma_id, DMA_CHANNEL_2);

    CK_Uart_Unset_Loopback_Mode(id);

    for (i = 0; i < MEM2UART0_TEST_L; i++) {
        CK_UINT8 tmp_intvl = 0;
        if (dst_sct_cnt != 0) {
            tmp_intvl = dst_sct_intvl * (i / dst_sct_cnt);
        }
        data = read_mreg8(MEM_TEST_DEST + i + tmp_intvl);
        if (data != test_char_data[i % 12]) {
            printf("\n\taddr 0x%x: 0x%x not equal to 0x%x\n",
                    MEM_TEST_DEST + i,
                    data, test_char_data[i % 12]);
            printf("                - - - FAIL.\n");
            return;
        }
    }
    passed_case++;
    printf("                - - - PASS.\n");
}

void CK_AHBDMA_UART_Test(CK_UINT32 id, CK_UINT8 dma_intr) {
    CK_UINT8 i = 0;
    CK_UINT8 gather = 0;
    CK_UINT8 scatter = 0;
    for (i = 0; i < 5; i++) {
        for (gather = 0; gather < 2; gather++) {
            for (scatter = 0; scatter < 2; scatter++) {
                CK_AHBDMA_UART_Loopback_Test(id, i, gather, scatter, dma_intr,
                    DMAC_CTL_DST_TR_WIDTH8, DMAC_CTL_DEST_MSIZE4);
                CK_AHBDMA_UART_Loopback_Test(id, i, gather, scatter, dma_intr,
                    DMAC_CTL_DST_TR_WIDTH32, DMAC_CTL_DEST_MSIZE1);
            }
        }
    }
}

void CK_AHBDMA_SPI_M_Test(CK_UINT32 dma_id, CK_UINT32 id, CK_UINT8 dma_intr,
                            CK_UINT32 dst_tr_width, CK_UINT32 dst_msize) {
    CK_UINT32 i = 0;
    CK_UINT8 data = 0;
    CK_UINT32 flash_addr = 0x6400000;
    CK_UINT32 result = 0;
    CK_UINT8 status = 0;
    CK_UINT32 spi_int = spi_nand_int_mode;
    CK_UINT32 open_len = 0;
    printf("\n%d. Memory <-> SPI Mater Transfer with\n"
            "   DST_TR_WIDTH %d, DST_MSIZE %d to SPI Master. . .\n",
            ahbdma_testcase_no++,
            DST_TR_WIDTH(dst_tr_width),
            DST_MSIZE(dst_msize));

    // Initialize a memory area for DMA to get the data from
    if (dst_tr_width == DMAC_CTL_DST_TR_WIDTH32) {
        memset((void *)MEM_TEST_SRC, 0, MEM2SPI_TEST_L * 4);
        for (i = 0; i < MEM2SPI_TEST_L; i++) {
            write_mreg8(MEM_TEST_SRC + i * 4, test_char_data[i % 12]);
        }
        open_len = MEM2SPI_TEST_L;
    } else if (dst_tr_width == DMAC_CTL_DST_TR_WIDTH8) {
        for (i = 0; i < MEM2SPI_TEST_L; i++) {
            write_mreg8(MEM_TEST_SRC + i, test_char_data[i % 12]);
        }
        open_len = MEM2SPI_TEST_L / 4;
    } else {
        printf("\n\tDMAC_CTL_DST_TR_WIDTH %d not supported in test for now\n",
                DST_TR_WIDTH(dst_tr_width));
        printf("                - - - FAIL.\n");
        return;
    }

    // clear destination memory for reading from eeprom
    memset((void *)MEM_TEST_DEST, 0, MEM2SPI_TEST_L);
    spi_nand_int_mode = 0;
    Initial_Spi();
    CMD_RESET_OP();
    /* Clear the block protection bit*/
    CMD_GET_FEATURE(0xa0, &status);
    if (status & 0x38) {
        CMD_SET_FEATURE(0xa0, (status & 0xc7));
    }

    result = CMD_BE(flash_addr);
    if (result != Flash_Success) {
        printf("\n\terase failed\n");
        printf("                - - - FAIL.\n");
        spi_nand_int_mode = spi_int;
        return;
    }

    DMA_Write_NAND(dma_id, id, flash_addr, (CK_UINT8 *)MEM_TEST_SRC,
                    open_len, DMA_CHANNEL_4, dma_intr,
                    dst_tr_width, dst_msize);

    DMA_Read_NAND(dma_id, id, flash_addr, (CK_UINT8 *)MEM_TEST_DEST,
                    MEM2SPI_TEST_L, DMA_CHANNEL_4, dma_intr);

    for (i = 0; i < MEM2SPI_TEST_L; i++) {
        data = read_mreg8(MEM_TEST_DEST + i);
        if (data != test_char_data[i % 12]) {
            printf("\n\taddr 0x%x: 0x%x not equal to 0x%x\n",
                    MEM_TEST_DEST + i, data, test_char_data[i % 12]);
            printf("                - - - FAIL.\n");
            spi_nand_int_mode = spi_int;
            return;
        }
    }
    passed_case++;
    printf("                - - - PASS.\n");
    spi_nand_int_mode = spi_int;
}
#if 0
void CK_AHBDMA_SPI_M_W_Subsys_Pre(CK_UINT8 channel) {
    CK_UINT32 i = 0;
    CK_UINT32 flash_addr = 0x6400000;
    CK_UINT32 result = 0;
    CK_UINT32 status = 0;
    CK_UINT32 spi_int = spi_nand_int_mode;

    // Initialize a memory area for DMA to get the data from
    for (i = 0; i < AHB_DMA_SUB_TEST_L; i++) {
        write_mreg8(AHB_DMA_SUB_SRC_SPI + i, test_char_data[i % 12]);
    }

    spi_nand_int_mode = 0;
    Initial_Spi();
    CMD_RESET_OP();
    /* Clear the block protection bit*/
    CMD_GET_FEATURE(0xa0, &status);
    if (status & 0x38) {
        CMD_SET_FEATURE(0xa0, (status & 0xc7));
    }

    result = CMD_BE(flash_addr);
    if (result != Flash_Success) {
        printf("\n\terase failed\n");
        printf("                - - - FAIL.\n");
        spi_nand_int_mode = spi_int;
        return;
    }

    result = DMA_Write_NAND_pre(flash_addr, AHB_DMA_SUB_SRC_SPI,
                    AHB_DMA_SUB_TEST_L / 4, channel, 0,
                    DMAC_CTL_DST_TR_WIDTH8, DMAC_CTL_DEST_MSIZE4);

    if (result != Flash_Success) {
        printf("\n\tDMA write to SPI failed. . .\n");
        printf("                - - - FAIL.\n");
        spi_nand_int_mode = spi_int;
        return;
    }
    spi_nand_int_mode = spi_int;
}

void CK_AHBDMA_SPI_M_W_Subsys_Post(CK_UINT8 channel) {
    CK_UINT32 ret = 0;
    CK_UINT32 flash_addr = 0x6400000;
    CK_UINT32 spi_int = spi_nand_int_mode;
    spi_nand_int_mode = 0;
    ret = DMA_Write_NAND_post(flash_addr, channel, 0);
    if (ret != Flash_Success) {
        printf("\n\tDMA write to SPI failed. . .\n");
        printf("                - - - FAIL.\n");
    }
    spi_nand_int_mode = spi_int;
}

void CK_AHBDMA_SPI_M_R_Subsys_Pre(CK_UINT8 channel) {
    CK_UINT32 i = 0;
    CK_UINT32 flash_addr = 0x6400000;
    CK_UINT32 spi_int = spi_nand_int_mode;

    // clear destination memory for reading from eeprom
    memset(AHB_DMA_SUB_DST_SPI, 0, AHB_DMA_SUB_TEST_L);
    spi_nand_int_mode = 0;
    Initial_Spi();
    CMD_RESET_OP();

    DMA_Read_NAND_pre(flash_addr, AHB_DMA_SUB_DST_SPI,
                    AHB_DMA_SUB_TEST_L, channel, 0);

    spi_nand_int_mode = spi_int;
}

void CK_AHBDMA_SPI_M_R_Subsys_Post(CK_UINT8 channel) {
    CK_UINT32 i = 0;
    CK_UINT8 data = 0;
    CK_UINT32 spi_int = spi_nand_int_mode;
    spi_nand_int_mode = 0;

    DMA_Read_NAND_post(channel, 0);

    for (i = 0; i < AHB_DMA_SUB_TEST_L; i++) {
        data = read_mreg8(AHB_DMA_SUB_DST_SPI + i);
        if (data != test_char_data[i % 12]) {
            printf("\n\taddr 0x%x: 0x%x not equal to 0x%x\n",
                    AHB_DMA_SUB_DST_SPI + i, data, test_char_data[i % 12]);
            printf("                - - - FAIL.\n");
            spi_nand_int_mode = spi_int;
            return;
        }
    }
    printf("                - - - PASS.\n");
    spi_nand_int_mode = spi_int;
}

void CK_AHBDMA_SPI_S_Test(CK_UINT8 dma_intr, CK_UINT32 dst_tr_width,
                            CK_UINT32 dst_msize) {
    CK_UINT32 i = 0;
    CK_UINT8 data = 0;
    CK_UINT32 result = 1;
    CK_UINT32 open_len;

    printf("\n%d. Memory <-> SPI Slave Transfer with\n"
            "   DST_TR_WIDTH %d, DST_MSIZE %d to SPI Slave. . .\n",
            ahbdma_testcase_no++,
            DST_TR_WIDTH(dst_tr_width),
            DST_MSIZE(dst_msize));

    // Initialize a memory area for DMA to get the data from
    if (dst_tr_width == DMAC_CTL_DST_TR_WIDTH32) {
        memset((void *)MEM_TEST_SRC, 0, MEM2SPIS_TX_TEST_L);
        for (i = 0; i < MEM2SPIS_TX_TEST_L; i++) {
            write_mreg8(MEM_TEST_SRC + i * 4, 0x10 + i);
        }
        open_len = MEM2SPIS_TX_TEST_L;
    } else if (dst_tr_width == DMAC_CTL_DST_TR_WIDTH8) {
        for (i = 0; i < MEM2SPIS_TX_TEST_L; i++) {
            write_mreg8(MEM_TEST_SRC + i, 0x10 + i);
        }
        open_len = MEM2SPIS_TX_TEST_L / 4;
    } else {
        printf("\n\tDMAC_CTL_DST_TR_WIDTH %d not supported in test for now\n",
                DST_TR_WIDTH(dst_tr_width));
        printf("                - - - FAIL.\n");
        return;
    }

    spi_slave_init(MODE0, 2);
    printf("\n\tTest Receive data from SPI Controller side: \n");
    printf("\n\t\t Run SPI Slave TX test application: \n");
    spis_dma_rx((CK_UINT8 *)MEM_TEST_DEST, MEM2SPIS_RX_TEST_L, DMA_CHANNEL_3, dma_intr);

    for (i = 0; i < MEM2SPIS_RX_TEST_L; i++) {
        data = read_mreg8(MEM_TEST_DEST + i);
        if (data != spis_test_patten[i]) {
            printf("\n\taddr 0x%x: 0x%x not equal to 0x%x\n",
                    MEM_TEST_DEST + i, data, spis_test_patten[i]);
            result = 0;
        }
    }

    if(result)
        printf("\t\t\t- - -PASS\n");
    else
        printf("\t\t\t- - -FAILURE\n");

    printf("\n\tTest Transmit 32 data to SPI Controller side: \n");
    printf("\n\t\t Run SPI Slave RX test application: \n");
    result = spis_dma_tx((CK_UINT8 *)MEM_TEST_SRC, open_len, DMA_CHANNEL_3, dma_intr,
                        dst_tr_width, dst_msize);

    if(result) {
        passed_case++;
        printf("\t\t\t- - -PASS\n");
    } else
        printf("\t\t\t- - -FAILURE\n");
}
#endif
void CK_AHBDMA_I2C_Test(CK_UINT32 dma_id, CK_UINT8 i2c_id, CK_UINT8 dma_intr,
                        CK_UINT32 dst_tr_width, CK_UINT32 dst_msize) {
    CK_UINT32 i = 0, j = 0;
    CK_UINT8 data = 0;
    CK_UINT32 eeprom_data_addr = 0x160;
    CK_UINT8 ret = 0;
    CK_UINT8 open_len = 0;
    CK_UINT8 *src_data[] = {test_char_data, test_char_data1};
    printf("\n%d. I2C %d Transfer with DST_TR_WIDTH %d,"
            " DST_MSIZE %d to I2C . . .\n",
            ahbdma_testcase_no++, i2c_id,
            DST_TR_WIDTH(dst_tr_width), DST_MSIZE(dst_msize));

    for (j = 0; j < 2; j++) {
        // Initialize a memory area for DMA to get the data from
        if (dst_tr_width == DMAC_CTL_DST_TR_WIDTH32) {
            memset(MEM_TEST_SRC, 0, MEM2I2C_TEST_L * 4);
            for (i = 0; i < MEM2I2C_TEST_L; i++) {
                write_mreg8(MEM_TEST_SRC + i * 4, src_data[j][i % 12]);
            }
            open_len = MEM2I2C_TEST_L;
        } else if (dst_tr_width == DMAC_CTL_DST_TR_WIDTH8) {
            for (i = 0; i < MEM2I2C_TEST_L; i++) {
                write_mreg8(MEM_TEST_SRC + i, src_data[j][i % 12]);
            }
            open_len = MEM2I2C_TEST_L / 4;
        } else {
            printf("\n\tDMAC_CTL_DST_TR_WIDTH %d not supported in test for now\n",
                    DST_TR_WIDTH(dst_tr_width));
            printf("                - - - FAIL.\n");
            return;
        }
        // clear destination memory for reading from eeprom
        memset(MEM_TEST_DEST, 0, MEM2I2C_TEST_L);

        dw_i2c_init(i2c_id, I2C_STANDARD_SPEED, 0);

        ret = dw_i2c_dma_write(dma_id, i2c_id, AT24C64_SLAVE_ADDR,
                                WORD_ADDR_L(eeprom_data_addr), WORD_ADDR_L_LEN,
                                MEM_TEST_SRC, open_len, DMA_CHANNEL_3,
                                dma_intr, 4, dst_tr_width, dst_msize);
        if (ret != 0) {
            printf("\n\tDMA write to EEPROM failed. . .\n");
            printf("                - - - FAIL.\n");
            return;
        }

        ret = dw_i2c_dma_read(dma_id, i2c_id, AT24C64_SLAVE_ADDR,
                                WORD_ADDR_L(eeprom_data_addr), WORD_ADDR_L_LEN,
                                MEM_TEST_DEST, MEM2I2C_TEST_L, DMA_CHANNEL_3,
                                dma_intr, 3);

        if (ret != 0) {
            printf("\n\tDMA read from EEPROM failed. . .\n");
            printf("                - - - FAIL.\n");
            return;
        }

        for (i = 0; i < MEM2I2C_TEST_L; i++) {
            data = read_mreg8(MEM_TEST_DEST + i);
            if (data != src_data[j][i % 12]) {
                printf("\n\taddr 0x%x: 0x%x not equal to 0x%x\n", MEM_TEST_DEST + i,
                         data, src_data[j][i % 12]);
                printf("                - - - FAIL.\n");
                return;
            }
        }
    }
    passed_case++;
    printf("                - - - PASS.\n");
}
#if 0
void CK_AHBDMA_I2C1_W_Subsys_Pre(CK_UINT8 channel) {
    CK_UINT32 i = 0;
    CK_UINT32 eeprom_data_addr = 0x200;
    CK_UINT8 i2c_id = 1;
    CK_UINT8 ret = 0;

    // Initialize a memory area for DMA to get the data from
    for (i = 0; i < AHB_DMA_SUB_TEST_L; i++) {
        write_mreg8(AHB_DMA_SUB_SRC_I2C + i, test_char_data[i % 12]);
    }

    dw_i2c_init(i2c_id, I2C_STANDARD_SPEED, 0);
    ret = dw_i2c_dma_write_pre(i2c_id, AT24CM02_DEV_ADDR(eeprom_data_addr),
                            WORD_ADDR_L(eeprom_data_addr), WORD_ADDR_L_LEN,
                            AHB_DMA_SUB_SRC_I2C, AHB_DMA_SUB_TEST_L / 4,
                            channel, 0, 4,
                            DMAC_CTL_DST_TR_WIDTH8, DMAC_CTL_DEST_MSIZE4);
    if (ret != 0) {
        printf("\n\tDMA write to EEPROM failed. . .\n");
        printf("                - - - FAIL.\n");
        return;
    }
}

void CK_AHBDMA_I2C1_W_Subsys_Post(CK_UINT8 channel) {
    CK_UINT8 i2c_id = 1;
    CK_UINT8 ret = 0;

    ret = dw_i2c_dma_write_post(i2c_id, AHB_DMA_SUB_SRC_I2C,
                            AHB_DMA_SUB_TEST_L / 4, channel,
                            0, DMAC_CTL_DST_TR_WIDTH8);
    if (ret != 0) {
        printf("\n\tDMA write to EEPROM failed. . .\n");
        printf("                - - - FAIL.\n");
        return;
    }
}

void CK_AHBDMA_I2C1_R_Subsys_Pre(CK_UINT8 channel) {
    CK_UINT32 i = 0;
    CK_UINT32 eeprom_data_addr = 0x200;
    CK_UINT8 i2c_id = 1;
    CK_UINT8 ret = 0;

    // clear destination memory for reading from eeprom
    memset(AHB_DMA_SUB_DST_I2C, 0, AHB_DMA_SUB_TEST_L);

    dw_i2c_init(i2c_id, I2C_STANDARD_SPEED, 0);

    ret = dw_i2c_dma_read_pre(i2c_id, AT24CM02_DEV_ADDR(eeprom_data_addr),
                            WORD_ADDR_L(eeprom_data_addr), WORD_ADDR_L_LEN,
                            AHB_DMA_SUB_DST_I2C, AHB_DMA_SUB_TEST_L,
                            channel, 0, 3);

    if (ret != 0) {
        printf("\n\tDMA read from EEPROM failed. . .\n");
        printf("                - - - FAIL.\n");
        return;
    }
}

void CK_AHBDMA_I2C1_R_Subsys_Post(CK_UINT8 channel) {
    CK_UINT32 i = 0;
    CK_UINT8 data = 0;
    CK_UINT8 i2c_id = 1;
    CK_UINT8 ret = 0;

    ret = dw_i2c_dma_read_post(i2c_id, AHB_DMA_SUB_TEST_L, channel, 0);

    if (ret != 0) {
        printf("\n\tDMA read from EEPROM failed. . .\n");
        printf("                - - - FAIL.\n");
        return;
    }

    for (i = 0; i < AHB_DMA_SUB_TEST_L; i++) {
        data = read_mreg8(AHB_DMA_SUB_DST_I2C + i);
        if (data != test_char_data[i % 12]) {
            printf("\n\taddr 0x%x: 0x%x not equal to 0x%x\n",
                     AHB_DMA_SUB_DST_I2C + i,
                     data, test_char_data[i % 12]);
            printf("                - - - FAIL.\n");
            return;
        }
    }
    printf("                - - - PASS.\n");
}
#endif
void CK_AHBDMA_MEM2MEM_Test(CK_UINT32 id, CK_UINT8 dma_intr, CK_UINT8 unit) {
    CK_UINT32 data_flag = 0;
    CK_UINT32 rest_val = 0;
    CK_UINT32 loop = 0;
    CK_UINT32 src = 0;
    CK_UINT32 dest = 0;
    CK_UINT32 tmp32 = 0;
    int i = 0;

    printf("\n%d. Memory to Memory Transfer, width %d . . .\n",
            ahbdma_testcase_no++, unit * 8);

    // Init test memory area
    for(loop = 0;loop < MEM2MEM_TEST_L; loop += 16) {
        for (i = 0; i < 4; i++)
            write_mreg32((MEM_TEST_SRC + loop + i * 4), test_mem_data[i]);
    }

    memset((void *)MEM_TEST_DEST, 0, MEM2MEM_TEST_L);

    // Configure channel 0 to transfer data
    rest_val = MEM2MEM_TEST_L;
    src = MEM_TEST_SRC;
    dest = MEM_TEST_DEST;
    DMAC_Init(id);

    DBG("\n\ttransfer %d bytes from address 0x%x to 0x%x\n",
        MEM2MEM_TEST_L, MEM_TEST_SRC, MEM_TEST_DEST);

    while(1) {
        if(rest_val > (DMA_CH01_MAX_BLK_SIZE * unit)) {
            DBG("\n\trest 0x%x bytes > (DMA_CH01_MAX_BLK_SIZE * 4) 0x%x bytes\n",
                rest_val, DMA_CH01_MAX_BLK_SIZE * unit);
            DMAMem2MemOpen(id, DMA_CHANNEL_0, src, dest,
                            DMA_CH01_MAX_BLK_SIZE, dma_intr, unit, 0, 0, 0, 0);
            DMAC_Start(id, DMA_CHANNEL_0);

            while(!(DMAC_CheckDone(id, DMA_CHANNEL_0, dma_intr)));
            rest_val -= (DMA_CH01_MAX_BLK_SIZE * unit);
            src += (DMA_CH01_MAX_BLK_SIZE * unit);
            dest += (DMA_CH01_MAX_BLK_SIZE * unit);
        } else {
            DBG("\n\trest 0x%x bytes < (DMA_CH01_MAX_BLK_SIZE * %d) 0x%x bytes\n",
                rest_val, unit, DMA_CH01_MAX_BLK_SIZE * unit);
            DMAMem2MemOpen(id, DMA_CHANNEL_0, src, dest,
                            (rest_val / unit), dma_intr, unit, 0, 0, 0, 0);
            DMAC_Start(id, DMA_CHANNEL_0);
            rest_val = 0;
            break;
        }
    }

    DBG("\n\ttransfer Done\n");
    while(!(DMAC_CheckDone(id, DMA_CHANNEL_0, dma_intr)));
    DMAC_Close(id, DMA_CHANNEL_0);

    DBG("\n\tcompare transfer data value\n");

    data_flag = 0;
    for(loop = 0; loop < MEM2MEM_TEST_L; loop += 16) {
        for (i = 0; i < 4; i++) {
            tmp32 = read_mreg32(MEM_TEST_DEST + loop + i * 4);
            if (tmp32 != test_mem_data[i]) {
                DBG("\taddr 0x%x:  0x%x not equal to 0x%x\n",
                        MEM_TEST_DEST + loop + i * 4, tmp32, test_mem_data[i]);
                data_flag = 1;
            }
        }
    }

    if (data_flag == 0) {
        passed_case++;
        printf("                - - - PASS.\n");
    } else {
        printf("                - - - FAIL.\n");
    }
}

CK_UINT8 CK_AHBDMA_Linked_List_Test(CK_UINT32 id, CK_UINT32 src1, CK_UINT32 src2, CK_UINT32 dest1,
                            CK_UINT32 dest2, CK_UINT32 len, CK_UINT8 dma_intr) {
    typedef struct LLI LLI_INST;
    CK_UINT32 data_flag = 0;
    CK_UINT32 loop = 0;
    CK_UINT32 tmp32 = 0;
    int i = 0;


    // Init test memory area
    for(loop = 0;loop < len; loop += 16) {
        for (i = 0; i < 4; i++)
            write_mreg32((src1 + loop + i * 4), test_mem_data[i]);
    }

    for(loop = 0;loop < len; loop += 16) {
        for (i = 0; i < 4; i++)
            write_mreg32((src2 + loop + i * 4), test_mem_data[i]);
    }

    memset((void *)dest1, 0, len);
    memset((void *)dest2, 0, len);

    DMAC_Init(id);

    LLI_INST * volatile LLI0=( LLI_INST *)(LLI0_ADDR);
    LLI_INST * volatile LLI1=( LLI_INST *)(LLI1_ADDR);

    LLI0->SAR = src1;
    LLI0->DAR = dest1;
    LLI0->LLP = LLI1;
    LLI0->CTL_L = (DMAC_CTL_M2M_DW | DMAC_CTL_SRC_MSIZE4 |
                    DMAC_CTL_DEST_MSIZE4 | DMAC_CTL_SINC_INC |
                    DMAC_CTL_DINC_INC |
                    DMAC_CTL_SRC_TR_WIDTH32 |
                    DMAC_CTL_DST_TR_WIDTH32 | dma_intr |
                    DMAC_CTL_LLP_SRC_EN | DMAC_CTL_LLP_DST_EN);
    LLI0->CTL_H = len / 4;

    //NOTE:Last LLI,SAR,DAR update method bit must set to 0
    LLI1->SAR = src2;
    LLI1->DAR = dest2;
    LLI1->LLP = LLI_NULL;   	//Last LLI, point to LLI_NULL
    LLI1->CTL_L = (DMAC_CTL_M2M_DW | DMAC_CTL_SRC_MSIZE4 |
                   DMAC_CTL_DEST_MSIZE4 | DMAC_CTL_SINC_INC |
                   DMAC_CTL_DINC_INC |
                   DMAC_CTL_SRC_TR_WIDTH32 |
                   DMAC_CTL_DST_TR_WIDTH32 | dma_intr);
    LLI1->CTL_H = len / 4;

    printf("\n\t start linked mode transfer\n");

    // The LLI.SARx, LLI.DARx, LLI.LLPx, and LLI.CTLx registers are fetched.
    // The DW_ahb_dmac automatically reprograms the SARx, DARx, LLPx,
    // and CTLx channel registers from the LLPx(0).
    DMAMem2MemOpen(id, DMA_CHANNEL_0, 0, 0,
                            len / 4, dma_intr, 4, 0,
                            LLI0_ADDR, 1, 1);

    DMAC_Start(id, DMA_CHANNEL_0);

    DBG("\n\ttransfer Done\n");

    CK_UINT32 timeout = 0;
    while (1) {
        if (DMAC_CheckDone(id, DMA_CHANNEL_0, dma_intr) == 1) {
            break;
        } else {
            if (timeout++ > 0x500) {
                DMAC_Close(id, DMA_CHANNEL_0);
                printf("\n\twaiting for DMA Done timeout!\n");
                return 1;
            }
        }
    }

    DMAC_Close(id, DMA_CHANNEL_0);

    DBG("\n\tcompare transfer data value\n");

    data_flag = 0;
    for(loop = 0; loop < len; loop += 16) {
        for (i = 0; i < 4; i++) {
            tmp32 = read_mreg32(dest1 + loop + i * 4);
            if (tmp32 != test_mem_data[i]) {
                DBG("\taddr 0x%x:  0x%x not equal to 0x%x\n",
                        dest1 + loop + i * 4, tmp32, test_mem_data[i]);
                data_flag = 1;
            }
        }
    }

    for(loop = 0; loop < len; loop += 16) {
        for (i = 0; i < 4; i++) {
            tmp32 = read_mreg32(dest2 + loop + i * 4);
            if (tmp32 != test_mem_data[i]) {
                DBG("\taddr 0x%x:  0x%x not equal to 0x%x\n",
                        dest2 + loop + i * 4, tmp32, test_mem_data[i]);
                data_flag = 1;
            }
        }
    }

    if (data_flag == 0)
        return 0;
    else
        return 1;
}

void CK_AHBDMA_MEM2MEM_Linked_List_Test(CK_UINT32 id, CK_UINT8 dma_intr) {
    CK_UINT32 data_flag = 0;

    printf("\n%d. Linked List Memory to Linked List Memory Transfer. . .\n", ahbdma_testcase_no++);

   data_flag = CK_AHBDMA_Linked_List_Test(id, MEM_TEST_SRC, MEM_TEST_SRC1,
        MEM_TEST_DEST, MEM_TEST_DEST1, MEM2MEM_LINK_TEST_L, dma_intr);

    if (data_flag == 0) {
        passed_case++;
        printf("                - - - PASS.\n");
    } else {
        printf("                - - - FAIL.\n");
    }
}

void CK_AHBDMA_SRAM2MEM_Linked_List_Test(CK_UINT32 id, CK_UINT8 dma_intr) {
    CK_UINT32 data_flag = 0;

    printf("\n%d. Linked List SRAM to Linked List Memory Transfer. . .\n", ahbdma_testcase_no++);

   data_flag = CK_AHBDMA_Linked_List_Test(id, SRAM_TEST_SRC, SRAM_TEST_SRC1,
        MEM_TEST_DEST, MEM_TEST_DEST1, MEM2MEM_LINK_TEST_L, dma_intr);

    if (data_flag == 0) {
        passed_case++;
        printf("                - - - PASS.\n");
    } else {
        printf("                - - - FAIL.\n");
    }
}

void CK_AHBDMA_MEM2SRAM_Linked_List_Test(CK_UINT32 id, CK_UINT8 dma_intr) {
    CK_UINT32 data_flag = 0;

    printf("\n%d. Linked List Memory to Linked List SRAM Transfer. . .\n", ahbdma_testcase_no++);

   data_flag = CK_AHBDMA_Linked_List_Test(id, MEM_TEST_SRC, MEM_TEST_SRC1,
        SRAM_TEST_DEST, SRAM_TEST_DEST1, MEM2MEM_LINK_TEST_L, dma_intr);

    if (data_flag == 0) {
        passed_case++;
        printf("                - - - PASS.\n");
    } else {
        printf("                - - - FAIL.\n");
    }
}

void CK_AHBDMA_MEM2MEM_Realignment_Test(CK_UINT32 id, CK_UINT8 dma_intr) {
    DMAC_CH_INFO channel_info;
    memset(&channel_info, 0, sizeof(channel_info));
    CK_UINT32 data_flag = 0;
    CK_UINT32 loop = 0;
    CK_UINT32 tmp32 = 0;
    int i = 0;

    printf("\n%d. Memory to Memory Realignment Transfer. . .\n", ahbdma_testcase_no++);

    // Init test memory area
    for(loop = 0;loop < MEM2MEM_LINK_TEST_L; loop += 16) {
        for (i = 0; i < 4; i++)
            write_mreg32((MEM_TEST_SRC + loop + i * 4), test_mem_data[i]);
    }

    memset((void *)MEM_TEST_DEST, 0, MEM2MEM_LINK_TEST_L);

    DMAC_Init(id);

    printf("\n\t start ransfer\n");

    channel_info.sarx = MEM_TEST_SRC;
    channel_info.darx = MEM_TEST_DEST;
    channel_info.ctlHx = MEM2MEM_REALIGNMENT_L;
    channel_info.llpx = 0;
    channel_info.ctlLx = (DMAC_CTL_M2M_DW | DMAC_CTL_SRC_MSIZE4 |
                            DMAC_CTL_DEST_MSIZE4 | DMAC_CTL_SINC_INC |
                            DMAC_CTL_DINC_INC |
                            DMAC_CTL_SRC_TR_WIDTH16 |
                            DMAC_CTL_DST_TR_WIDTH32 | dma_intr);

    channel_info.cfgLx = (DMAC_CFG_HS_SRC_SOFTWARE | DMAC_CFG_HS_DST_SOFTWARE |
                            DMAC_CFG_SRC_HS_POL_H | DMAC_CFG_DST_HS_POL_H |
                            DMAC_CFG_CH_PRIOR(0) | DMAC_CFG_RELOAD_SRC);
    //Source handshake mode
    //Destination handshake mode
    //Source handshake polarity
    //Destination handshake polarity
    DMAC_Open(id, &channel_info, DMA_CHANNEL_0, MEM2MEM_REALIGNMENT_L);

    DMAC_Start(id, DMA_CHANNEL_0);

    delay(10);

    // Stop transfer
    tmp32 = read_mreg32(DMAC_CFG(id, 0));
    tmp32 &= ~DMAC_CFG_RELOAD_SRC;
    write_mreg32(DMAC_CFG(id, 0), tmp32);

    DBG("\n\ttransfer Done\n");

    CK_UINT32 timeout = 0;
    while (1) {
        if (DMAC_CheckDone(id, DMA_CHANNEL_0, dma_intr) == 1) {
            break;
        } else {
            if (timeout++ > 0x500) {
                DMAC_Close(id, DMA_CHANNEL_0);
                printf("\n\twaiting for DMA Done timeout!\n");
                printf("                - - - FAIL.\n");
                return;
            }
        }
    }

    DMAC_Close(id, DMA_CHANNEL_0);

    DBG("\n\tcompare transfer data value\n");

    // Every block will transfer 18 bytes
    // So data in offset 0x0 will be 0x00112233
    //            offset 0x4 will be 0x44556677
    //            offset 0x8 will be 0x8899aabb
    //            offset 0xc will be 0xccddeeff
    //            offset 0x10 will be 0x00002233 -> only 2 bytes
    //            offset 0x14 will be 0x00112233 -> HW realignment address for next block
    data_flag = 0;
    for(loop = 0; loop < 60; loop += 20) {
        for (i = 0; i < 5; i++) {
            tmp32 = read_mreg32(MEM_TEST_DEST + loop + i * 4);
            if (((i < 4 && (tmp32 != test_mem_data[i])))) {
                DBG("\taddr 0x%x:  0x%x not equal to 0x%x\n",
                        MEM_TEST_DEST + loop + i * 4, tmp32, test_mem_data[i]);
                data_flag = 1;
            }

            if (i == 4 && (tmp32 != (test_mem_data[0] & 0x0000FFFF))) {
                DBG("\taddr 0x%x:  0x%x not equal to 0x%x\n",
                        MEM_TEST_DEST + loop + i * 4, tmp32, test_mem_data[0] & 0x0000FFFF);
                data_flag = 1;
            }
        }
    }


    if (data_flag == 0) {
        passed_case++;
        printf("                - - - PASS.\n");
    } else {
        printf("                - - - FAIL.\n");
    }
}

void CK_AHBDMA_Multi_Channel_Test(CK_UINT32 id) {
    CK_UINT8 data_flag = 0;
    CK_UINT32 src = 0;
    CK_UINT32 dest = 0;
    CK_UINT32 max_len = 0;
    CK_UINT32 i = 0;
    CK_UINT8 channel_number = 0;
    CK_UINT8 prio[8] = {2, 0, 6, 7, 6, 1, 5, 3};
    CK_UINT8 complete_seq[8] = {3, 2, 4, 6, 7, 0, 5, 1};
    CK_UINT8 complete_no = 0;

    printf("\n%d. Multi-Channel Transfer with different priorities. . .\n",
            ahbdma_testcase_no++);

    DMAC_Init(id);

    for (channel_number = 0;
            channel_number < DMAH_NUM_CHANNELS; channel_number++) {
        if (channel_number < 2) {
            max_len = DMA_CH01_MAX_BLK_SIZE * 4;
        } else {
            max_len = DMA_CH27_MAX_BLK_SIZE * 4;
        }
        src = MEM_TEST_SRC + channel_number * MULTI_CHANNEL_MEM_INTVL;
        dest = MEM_TEST_DEST + channel_number * MULTI_CHANNEL_MEM_INTVL;
        // Initialize Source memory area for each channel
        for (i = 0; i < max_len; i++) {
            write_mreg8(src + i, test_char_data[i % 12]);
        }
        // clear Destination memory area for each
        memset((void *)dest, 0, max_len);

        DMAMem2MemOpen(id, channel_number, src, dest,
                        (max_len / 4), 0, 4, prio[channel_number], 0, 0, 0);
        DBG("\n\ttransfer %d bytes from address 0x%x to 0x%x\n",
            max_len, src, dest);
    }

    // run Multiple channels simultaneously
    DMAC_RUN_ALL(id);

    // Channels should complete in correct priority sequence
    while(complete_no < 8) {
        CK_UINT32 channel = complete_seq[complete_no];
        if (DMAC_CheckDone_Any(id)) {
            CK_UINT32 rawtfr = read_mreg32(DMAC_RAWTFR(id));
            if (rawtfr != (1 << channel)) {
                printf("\n\tchannel %d with priority %d should complete in"
                        " No. %d, but not... RAWTFR 0x%x\n",
                        channel, prio[channel], complete_no, rawtfr);
                printf("                - - - FAIL.\n");
                return;
            } else {
                // clear Interrupt status
                write_mreg32(DMAC_CLEARTFR(id), 1 << channel);
                complete_no++;
            }
        }
    }

    // Close all channels
    for (channel_number = 0;
            channel_number < DMAH_NUM_CHANNELS; channel_number++) {
        DMAC_Close(id, channel_number);
    }

    for (channel_number = 0;
            channel_number < DMAH_NUM_CHANNELS; channel_number++) {
        DBG("\n\tcompare transfer data value for channel %d\n", channel_number);

        if (channel_number < 2) {
            max_len = DMA_CH01_MAX_BLK_SIZE * 4;
        } else {
            max_len = DMA_CH27_MAX_BLK_SIZE * 4;
        }
        dest = MEM_TEST_DEST + channel_number * MULTI_CHANNEL_MEM_INTVL;

        data_flag = 0;
        for (i = 0; i < max_len; i++) {
            CK_UINT8 data = read_mreg8(dest + i);
            if (data != test_char_data[i % 12]) {
                DBG("\taddr 0x%x: 0x%x not equal to 0x%x, channel %d\n",
                    dest + i, data, test_char_data[i % 12], channel_number);
                data_flag = 1;
            }
        }
    }

    if (data_flag == 0) {
        passed_case++;
        printf("                - - - PASS.\n");
    } else {
        printf("                - - - FAIL.\n");
    }
}

void CK_AHBDMA_ID_REG_Test(CK_UINT32 id) {
    CK_UINT8 i = 0;
    CK_UINT8 status = 0;

    printf("\n%d. Read ID Registers. . .\n", ahbdma_testcase_no++);
    for (i = 0; i < 8; i++) {
        if (read_mreg32(id_regs[id][i]) != (id_regs_val[i] & 0xffffffff)) {
            printf("\n\tlower 32 bits of 0x%x: 0x%x not equal to 0x%x\n",
                    id_regs[id][i], read_mreg32(id_regs[id][i]),
                    (id_regs_val[i] & 0xffffffff));
            status = 1;
        }

        if (read_mreg32(id_regs[id][i] + 4) !=
            ((id_regs_val[i] >> 32) & 0xffffffff)) {
            printf("\n\thigher 32 bits of 0x%x: 0x%x not equal to 0x%x\n",
                    id_regs[id][i], read_mreg32(id_regs[id][i] + 4),
                    (id_regs_val[i] >> 32) & 0xffffffff);
            status = 1;
        }
    }
    if (status == 0) {
        passed_case++;
        printf("                - - - PASS.\n");
    } else {
        printf("                - - - FAIL.\n");
    }
}

void CK_AHBDMA_Test(CK_UINT32 id) {
    CK_UINT8 dma_intr = 0;
    CK_UINT8 supported_unit[] = {1, 2, 4, 8};
    CK_UINT8 unit = 0;
    printf("\nSynopsys AHB DMA Controller %d Test. . . \n", id);
    ahbdma_testcase_no = 0;
    passed_case = 0;
    CK_AHBDMA_ID_REG_Test(id);
    for (dma_intr = 0; dma_intr < 2; dma_intr++) {
        if (dma_intr == 0) {
            printf("\n\n**************** DMA test with interrupt disabled"
                    " ****************\n");
        } else {
            printf("\n\n**************** DMA test with interrupt enabled"
                    " ****************\n");
        }
        for (unit = 0; unit < 4; unit++) {
            CK_AHBDMA_MEM2MEM_Test(id, dma_intr, supported_unit[unit]);
        }
        CK_AHBDMA_MEM2MEM_Linked_List_Test(id, dma_intr);
        CK_AHBDMA_SRAM2MEM_Linked_List_Test(id, dma_intr);
        CK_AHBDMA_MEM2SRAM_Linked_List_Test(id, dma_intr);

        if (id == 0) {
            // i2c3
            CK_AHBDMA_I2C_Test(id, 3, dma_intr, DMAC_CTL_DST_TR_WIDTH8, DMAC_CTL_DEST_MSIZE4);
            CK_AHBDMA_I2C_Test(id, 3, dma_intr, DMAC_CTL_DST_TR_WIDTH32, DMAC_CTL_DEST_MSIZE1);
        }
        if (id == 1) {
            CK_AHBDMA_UART_Test(id, dma_intr);
            CK_AHBDMA_SPI_M_Test(id, 3, dma_intr, DMAC_CTL_DST_TR_WIDTH8,
                                    DMAC_CTL_DEST_MSIZE4);
            CK_AHBDMA_SPI_M_Test(id, 3, dma_intr, DMAC_CTL_DST_TR_WIDTH32,
                                    DMAC_CTL_DEST_MSIZE4);
        }
        //JJJ_DEBUGCK_AHBDMA_SPI_S_Test(dma_intr, DMAC_CTL_DST_TR_WIDTH8,
        //JJJ_DEBUG                        DMAC_CTL_DEST_MSIZE4);
        //JJJ_DEBUGCK_AHBDMA_SPI_S_Test(dma_intr, DMAC_CTL_DST_TR_WIDTH32,
        //JJJ_DEBUG                        DMAC_CTL_DEST_MSIZE4);
    }
    CK_AHBDMA_MEM2MEM_Realignment_Test(id, 0);
    CK_AHBDMA_Multi_Channel_Test(id);
    printf("\n\n************************************************\n");
    printf("                      Pass Rate: %d/%d\n",
            passed_case, ahbdma_testcase_no);
    printf("************************************************\n");
    ahbdma_testcase_no = 0;
    passed_case = 0;
    printf("\nEnd Synopsys AHB DMA Controller %d Test. . . \n", id);
}
