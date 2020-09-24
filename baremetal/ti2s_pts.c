/*****************************************************************************
 *  File: ti2s_pts.c
 *
 *  Descirption: this file contains the I2S PTS test cases.
 *
 *  Copyright (C) : 2018 VeriSilicon.
 *
 *  Author: Chunming Li
 *  Date:   Dec 26 2018
 *
 *****************************************************************************/

#include <string.h>
#include "i2s_pts.h"
#include "stc.h"
#include "crm.h"
#include "misc.h"
#include "intc.h"

#define CH_NUM              4
#define SAMPLE_LEN          127
#define FIFO_NUM            16
#define I2S_TIMEOUT         1000000

static CK_UINT8  intc_test;
static CK_UINT8  passed;

CKStruct_IRQHandler i2s_pts_irqhandler;

static void CK_I2S_PTS_ISR_Handler(u32 irq) {
    CK_INT32 stat;

    intc_test += 1;
    stat = read_mreg32(I2S_PTS_IRQ_STATUS);
    write_mreg32(I2S_PTS_IRQ_CLR, stat);
    //printf("\n\t\tInterrupt irq status=0x%x, int num=%d\n", stat, intc_test);
}

void INIT_STC() {
    // Set as BYPASS mode
    write_mreg32(CRM_TSM_PLL_BYP, 0x00000001);
    // Set as external 27MHz clock
    write_mreg32(CRM_TSMPLL_REFCLK_CFG, 0x00000001);
    // Set as tsm_clk27
    write_mreg32(CRM_TS_CLK_CFG, 0x00000000);
    // Disable STC clock gating
    write_mreg32(CRM_BLK_CLK_ICG3, 0x00000000);
    // Release STC
    write_mreg32(CRM_BLK_SW_RST3, STC_SW_RST);

    // Set STC CLK_DIVIDER
    write_mreg32(VSI_STC_CLK_DIVIDER, 0x12B);
    // Set initial value of timer
    write_mreg32(VSI_STC_TIMER_INIT_VALUE_L, 0);
    write_mreg32(VSI_STC_TIMER_INIT_VALUE_H, 0);
    // Enable STC
    write_mreg32(VSI_STC_ENABLE, VSI_STC_EN);
}

void CH_Test(CK_UINT8 ch_num, CK_UINT8 edge_type) {
    CK_INT32 stat;
    CK_INT32 i;
    CK_INT32 val;
    CK_INT32 start;

    // Set Audio sample length
    write_mreg32(I2S_AUDIO_LEN, SAMPLE_LEN);
    // Set SAMPLE EDGE
    write_mreg32(I2S_SAMPLE_EDGE, edge_type);
    // Enable interrupt
    write_mreg32(I2S_PTS_IRQ_EN, 1);
    // Enable channel
    write_mreg32(I2S_PTS_ENABLE, 1 << ch_num);

    start = 0;
    do {
        start += 1;
        udelay(1);
    } while ((intc_test < FIFO_NUM) && (start < I2S_TIMEOUT));

    if (start >= I2S_TIMEOUT) {
        printf("\n\t\tNot trigger enough interrupt number: %d\n", intc_test);
        passed = 0;
        return;
    }

    // Disable interrupt
    write_mreg32(I2S_PTS_IRQ_EN, 0);

    // Check FIFO full status
    stat = read_mreg32(I2S_FIFO_STATUS);
    if (!(stat & (1 << ch_num))) {
        printf("\n\t\t Channel %d FIFO is not full.\n", ch_num);
        passed = 0;
        return;
    }

    // Read PTS from FIFO
    for (i = 0; i < FIFO_NUM; i++) {
        udelay(100);
        val = read_mreg32(I2S_CH0_FIFO_DATA + 4 * ch_num);
        printf("\n\t\t PTS %d = 0x%x", i, val);
        if (val == 0)
            passed = 0;
    }

    // Disable channel
    write_mreg32(I2S_PTS_ENABLE, 0);
}

void CK_I2S_PTS_Test() {
    CK_UINT8 i;
    CK_INT32 get;
    CK_INT32 val;
    CK_UINT8 edg_type;

    printf("\nVSI I2S PTS Test. . . \n");

    INIT_STC();

    memset(&i2s_pts_irqhandler,0,sizeof(PCKStruct_IRQHandler));
    // Request Interrupt ISR
    i2s_pts_irqhandler.devname = "I2S_PTS";
    i2s_pts_irqhandler.irqid = CK_INTC_I2S_PTS;
    i2s_pts_irqhandler.priority = CK_INTC_I2S_PTS - 64;
    i2s_pts_irqhandler.handler = CK_I2S_PTS_ISR_Handler;
    i2s_pts_irqhandler.bfast = FALSE;
    i2s_pts_irqhandler.next = NULL;
    /* Register I2S PTS ISR */
    CK_INTC_RequestIrq(&i2s_pts_irqhandler, AUTO_MODE);

    // Release I2S_PTS
    val = read_mreg32(CRM_BLK_SW_RST3) | I2S_PTS_RST;
    write_mreg32(CRM_BLK_SW_RST3, val);

    for (i = 0; i < CH_NUM; i++) {
        printf("\n\tStart test I2S PTS Channel %d \n", i);
        printf("\n\t\t- - - Ready to start test?...\n");
        printf("\t\t- - - [y/n] ");

        while(1){
            get = CK_WaitForReply();

            if((get == 1)) {
                for (edg_type = 0; edg_type < 3; edg_type++) {
                    intc_test = 0;
                    passed = 1;
                    CH_Test(i, edg_type);

                    if (passed)
                        printf("\n\t\t\t --- Edge%d PASS\n", edg_type);
                    else
                        printf("\n\t\t\t --- Edge%d FAIL\n", edg_type);
                }

                break;
            } else if (get == 0)
                break;
            else
                printf("\n\tPlease enter 'y' or 'n'   ");
        }
    }

    printf("\n\tEnd VSI I2S PTS Test\n");
}