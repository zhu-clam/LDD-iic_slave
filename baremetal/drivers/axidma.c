#include "ck810.h"
#include "datatype.h"
#include "intc.h"
#include "spi_nor.h"
#include "axidma.h"
#include "cache.h"
#include "misc.h"
#include <string.h>

#define M2M_TEST_LEN    44c//64

#define LLI_ADDR        0x10000000

CKStruct_IRQHandler irqhandler;
CK_UINT8 interrupt_flag = 0;
CK_UINT32 interrupt_open = 0;

volatile CK_UINT32 chx_InterVal[AXI_DMAC_MAX_CHANNELS];

/**
 * Interrupt type
*/
void CHOSE_TYPE(CK_UINT32 type){
    interrupt_open = type;
}

/**
 * Check AXI_DMA ID
*/
void CK_AXIDMA_ID_REG_Test()
{
    CK_UINT32 id;
    id = read_mreg32(AXI_DMAC_IDREG);
    if (id != AXI_DMAC_ID)
        printf("AXIDMA ID ERROR!, get:%d\n", id);
    else
        printf("AXIDMA ID:0x%d\n", id);
}

/**
 * Calculate block_ts from src_width and length
*/
CK_UINT32 cal_block_ts(CK_UINT32 src_width, CK_UINT32 length)
{
    CK_UINT32 tmp1 = src_width+3;
    CK_UINT32 tmp2 = 1;
    while (tmp1) {
        tmp2 = tmp2 * 2;
        tmp1--;
    }
    CK_UINT32 block_ts = ((length * 8) / tmp2) - 1;
    return block_ts;
}

/**
 * Cache flush function
*/
static inline void CK_Cache_Flush()
{
    #if CONFIG_CKCPU_MMU
        CK_Cache_FlushAll();
    #endif
}

/**
 * AXI DMA OPEN
*/
void AXI_DMA_OPEN()
{
    //DMAC_CFGREG(0x10) enable AXI_DMAC  & AXI_DMAC Interrupt
    write_mreg32(AXI_DMAC_CFGREG, AXI_DMAC_EN | AXI_DMAC_INT_EN);
}

/**
 * AXI DMA CLOSE
*/
void AXI_DMA_CLOSE()
{
    //DMAC_CFGREG(0x10)  disable AXI DMA and Interrupt
    write_mreg32(AXI_DMAC_CFGREG, AXI_DMAC_DE | AXI_DMAC_INT_DE);

    //Free IRQ
    CK_INTC_FreeIrq(&irqhandler, AUTO_MODE);
    interrupt_flag = 0;
}

/**
 * Channel enable to start DMA transfer
*/
void AXI_DMA_CHx_SATRT(CK_UINT32 channel)
{
    //DMAC_CHENREG(0x18) , set Channel enable to start DMA transfer
    write_mreg32(AXI_DMAC_CHENREG, CHx_EN(channel) | CHx_EN_WE(channel));
}

/**
 * Mask interrupt except DMA TFR DONE
*/
void AXI_DMA_CHx_ENABLEINT(CK_UINT32 channel)
{
    //Congigure CHx_INSIGNAL_ENABLEREG(0x190) mask interrupt except DMA TFR DONE
    write_mreg32(CHx_INTSIGNAL_ENABLEREG(channel), CHx_INSIGNAL_EN_DMA_TFR_DONE);
}

/**
 * Clear DMA TFR interrupt
*/
void AXI_DMA_CHx_CLEARINT(CK_UINT32 channel)
{
    //CHx_INTCLEARREG(0x198) clear DMA TFR interrupt
    write_mreg32(CHx_INTCLEARREG(channel), CLEAR_DMA_TFR_DONE);
}

/**
 * Disbale channel
*/
void AXI_DMA_CHx_CLOSE(CK_UINT32 channel)
{
    //DMAC_CHENREG(0x18) close Channel enable
    write_mreg32(AXI_DMAC_CHENREG, CHx_DE(channel) | CHx_DE_WE(channel));
}

/**
 * Check Transfer Done
*/
void AXI_DMA_CHECKDONE(CK_UINT32 channel){
    volatile CK_UINT32 axi_dma_status;
    if (interrupt_open == 1) {
        while((chx_InterVal[channel-1] & DMA_TFR_DONE) != DMA_TFR_DONE);
        chx_InterVal[channel-1] = 0;
    } else {
       axi_dma_status = read_mreg32(CHx_INTSTATUS(channel));
        while((axi_dma_status & DMA_TFR_DONE) != DMA_TFR_DONE){
            axi_dma_status = read_mreg32(CHx_INTSTATUS(channel));
        }
    }
}

/**
 * AXI DMA IRQ handler
*/
void axi_dma_irqHandler(CK_UINT32 irq)
{
    CK_UINT32 tmp_DMAC, tmp_Common, tmp_CHx;
    CK_UINT32 CHx, cnt;
    //read DMAC_INTSTATUSREG and clear
    tmp_DMAC = read_mreg32(AXI_DMAC_INTSTATUSREG);
    if ((tmp_DMAC & CommonReg_IntStat) == CommonReg_IntStat) {
        //read DMAC_COMMONREG_INTSTATUSREG and clean
        tmp_Common = read_mreg32(AXI_DMAC_COMMONREG_INTSTATUSREG);
        ;
        write_iram32(AXI_DMAC_COMMONREG_INTCLEARREG, tmp_Common);
    }
    if ((tmp_DMAC && 0xff) > 0) {
        CHx = (tmp_DMAC & 0xff);
        for (cnt=1; cnt <= AXI_DMAC_MAX_CHANNELS; cnt++) {
            if (CHx_IntStat(cnt) == CHx) {
                //read CHx_INTSTATUS and clear
                tmp_CHx = read_mreg32(CHx_INTSTATUS(cnt));
                chx_InterVal[cnt-1] = tmp_CHx;
                write_mreg32(CHx_INTCLEARREG(cnt), tmp_CHx);
                break;
            }
        }
    }
}

/**
 * AXI DMA Init
*/
void AXI_DMA_INIT()
{
    AXI_DMA_CLOSE();

    if (interrupt_open == 1) {
        if (interrupt_flag == 0) {
            memset(&irqhandler, 0, sizeof(irqhandler));
            irqhandler.devname = "AXIDMA";
            irqhandler.irqid = CK_INTC_AXIDMA;
            irqhandler.priority = CK_INTC_AXIDMA;
            irqhandler.handler = axi_dma_irqHandler;
            irqhandler.bfast = FALSE;
            irqhandler.next = NULL;
            /*register AXI DMA ISR*/
            CK_INTC_RequestIrq(&irqhandler, AUTO_MODE);
            interrupt_flag = 1;
        }
    }
    AXI_DMA_OPEN();
}

/**
 * Channel register CHx_CFG & CHx_CTL config
*/
CK_UINT32 CHx_CONFIG(struct axi_dma_info axi, CK_UINT32 type){
    //configure DMAC_CHx_CFG(0x120)
    CK_UINT32 axi_data = 0x00;
    if (type == TYPE_SINGLE)
        write_mreg32(CHx_CFG_LOW(axi.channel), 0x00);
    else
        write_mreg32(CHx_CFG_LOW(axi.channel), 0xf);

    switch (axi.direction)
    {
    case CHx_M2M_DMAC:
        write_mreg32(CHx_CFG_HIGH(axi.channel), axi.direction | CHx_HS_SEL_SRC_S
            | CHx_HS_SEL_DST_S | CHx_PRIOR(axi.channel));
        break;
    case CHx_M2P_DMAC:
        write_mreg32(CHx_CFG_HIGH(axi.channel), axi.direction | CHx_HS_SEL_SRC_S
            | CHx_HS_SEL_DST_H | CHx_PRIOR(axi.channel));
        break;
    case CHx_P2M_DMAC:
        write_mreg32(CHx_CFG_HIGH(axi.channel), axi.direction | CHx_HS_SEL_SRC_H
            | CHx_HS_SEL_DST_S | CHx_SRC_PER | CHx_PRIOR(axi.channel));
        break;
    case CHx_P2P_DMAC:
        write_mreg32(CHx_CFG_HIGH(axi.channel), axi.direction | CHx_HS_SEL_SRC_H
            | CHx_HS_SEL_DST_H | CHx_PRIOR(axi.channel));
        break;
    }

    //Configure DMAC_CHx_CTL(0x118) registers
    axi_data =  CHx_DST_MSIZE(axi.dst_msize) | CHx_SRC_MSIZE(axi.src_msize)
            | CHx_DST_WIDTH(axi.dst_width) | CHx_SRC_WIDTH(axi.src_width);

    switch (axi.direction)
    {
    case CHx_M2M_DMAC:
        if(axi.channel == 1 || axi.channel == 2)
            ;
        else
            axi_data |= CHx_SMS | CHx_DMS;
        break;
    case CHx_M2P_DMAC:
        if(axi.channel == 1 || axi.channel == 2)
            axi_data |= CHx_DINC;
        else
            axi_data |= CHx_DINC | CHx_SMS | CHx_DMS;
        break;
    case CHx_P2M_DMAC:
        if(axi.channel==1 || axi.channel==2)
           axi_data |= CHx_SINC;
        else
            axi_data |= CHx_SINC | CHx_SMS | CHx_DMS;
        break;
    case CHx_P2P_DMAC:
        if(axi.channel==1 || axi.channel==2){
            axi_data |= CHx_SINC | CHx_DINC;
        }else{
            axi_data |= CHx_SINC | CHx_DINC | CHx_SMS | CHx_DMS;
        }break;
    }

    if (type == TYPE_SINGLE) {
        write_mreg32(CHx_CTL_LOW(axi.channel), axi_data);
        axi_data = 0x00;
        write_mreg32(CHx_CTL_HIGH(axi.channel), axi_data);
    }

    AXI_DMA_CHx_ENABLEINT(axi.channel);

    return axi_data;
}

/**
 * AXI_DMA Transfer Memory to Memory Init
 * **/
void AXI_DMA_TRAN_INIT(struct axi_dma_info axi, CK_UINT32 src_addr,
        CK_UINT32 dst_addr, CK_UINT32 length)
{
    CK_UINT32 block_ts;

    AXI_DMA_INIT();

    //DMAC_CHx_SAR(0x100) & DMAC_CHx_DAR(0x108) set source addr & dest addr
    write_mreg32(CHx_SAR(axi.channel), src_addr);
    write_mreg32(CHx_DAR(axi.channel), dst_addr);

    //DMAC_CHx_BLOCK_TS(0x110) set block_ts
    block_ts = cal_block_ts(axi.src_width, length);
    write_mreg32(CHx_BLOCK_TS(axi.channel), block_ts);

    CHx_CONFIG(axi, TYPE_SINGLE);
}

/**
 * AXI_DMA Transfer Init (LLI)
*/
void AXI_DMA_TRAN_INIT_LLI(struct axi_dma_info axi, CK_UINT32 src_addr,
        CK_UINT32 dst_addr, CK_UINT32 length)
{
    unsigned long axi_data;

    AXI_DMA_INIT();

    axi_data = CHx_CONFIG(axi, TYPE_LLI);

    struct axi_dma_lli* head = (void *)LLI_ADDR;
    struct axi_dma_lli* tmp = head;
    CK_UINT32 num = 1;
    while (tmp->llp != (long)NULL) {
        tmp = (struct axi_dma_lli*)(CK_INT32)tmp->llp;
        num++;
    }
    tmp = head;

    CK_UINT32 cnt;
    CK_UINT32 increment = length / num;
    CK_UINT32 block_ts = cal_block_ts(axi.src_width, increment);

    for (cnt = 0; cnt < num; cnt++) {
        tmp->sar = src_addr + increment * cnt;
        tmp->dar = dst_addr + increment * cnt;
        tmp->block_ts_lo = block_ts;
        tmp->ctl_lo = axi_data;
        if (tmp->llp == (long)NULL)
            tmp->ctl_hi = LAST_ITEM | VALID;   //last & valid
        else
            tmp->ctl_hi = NOT_LAST_ITEM | VALID;

        tmp = (struct axi_dma_lli*)(CK_INT32)tmp->llp;
    }

    //Configure CHx_LLP(x0128) Starting Address Memory of LLI block
    write_mreg32(CHx_LLP(axi.channel), LLI_ADDR);
}

/**
 * AXI_DMA Trans
 * **/
void AXI_DMA_TRANS(struct axi_dma_info axi)
{
    CK_Cache_Flush();

    AXI_DMA_CHx_SATRT(axi.channel);

    //CHx_SWHSSRCREG(0x138) & CHx_SWHSDSTREG(0x140), open source/destination software handshaking
    switch (axi.direction)
    {
    case CHx_M2P_DMAC:
        write_mreg32(CHx_SWHSSRCREG(axi.channel), OPEN_SRC_SWHS);
        break;
    case CHx_P2M_DMAC:
        write_mreg32(CHx_SWHSDSTREG(axi.channel), OPEN_DST_SWHS);
        break;
    case CHx_M2M_DMAC:
        write_mreg32(CHx_SWHSSRCREG(axi.channel), OPEN_SRC_SWHS);
        write_mreg32(CHx_SWHSDSTREG(axi.channel), OPEN_DST_SWHS);
        break;
    }

    AXI_DMA_CHECKDONE(axi.channel);

    AXI_DMA_CHx_CLEARINT(axi.channel);

    AXI_DMA_CHx_CLOSE(axi.channel);

    AXI_DMA_CLOSE();
}

