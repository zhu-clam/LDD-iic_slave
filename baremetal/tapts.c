/*****************************************************************************
 *  File: tstc.c
 *
 *  Descirption: this file contains the APTS test cases.
 *
 *  Copyright (C) : 2018 VeriSilicon.
 *
 *  Author: Chunming Li
 *  Date:   Dec 20 2018
 *
 *****************************************************************************/

#include "stc.h"
#include "apts.h"
#include "crm.h"
#include "misc.h"
#include "intc.h"

extern void INIT_STC();

#define SPI_CLK     600000

CKStruct_IRQHandler apts_irqhandler;


static void CK_APTS_ISR_Handler(u32 irq) {
    CK_INT32 stat;

    stat = read_mreg32(APTS_INTSTS);
    write_mreg32(APTS_INTCLR, stat);
    //printf("\n\t\tInterrupt irq status=0x%x, int num=%d\n", stat, intc_test);
}

void Simple_SPI_Test() {
    CK_UINT8 get;
    CK_UINT32 div;

    printf("\n\tVSI APTS Simple SPI transfer Test. . . \n");

    write_mreg32(APTS_CTRL1, APTS_EN(0));
    write_mreg32(APTS_CTRL1, SPI_EN(0));
    // Init APTS
    write_mreg32(APTS_Tx0, 0x00010203);
    write_mreg32(APTS_Tx1, 0x04050607);
    div = (APTS_CLK27_FREQ / SPI_CLK) / 2 - 1;
    write_mreg32(APTS_DIVIDER, div);

    // Transfer 8 bytes once
    write_mreg32(APTS_CTRL2, WD_LEN(0) | CHAR_LEN(0x3f) | ASS(1) | LSB(1) | NEG(0));
    //write_mreg32(APTS_SS, 0);
    write_mreg32(APTS_CTRL1, SPI_EN(1));
    udelay(1000);

    printf("\t\t- - - Check data on logic analyzer? [y/n] ");

    while(1){
        get = CK_WaitForReply();

        if((get == 1)) {
            printf("\n\t\t\t ---PASS\n");
            break;
        } else if (get == 0) {
            printf("\n\t\t\t ---FAIL\n");
            break;
        } else
            printf("\n\tPlease enter 'y' or 'n'   ");
    }
}

void Mode0_Test() {
    CK_UINT8 get;
    CK_UINT32 div;
    CK_UINT32 val_h;
    CK_UINT32 val_l;

    printf("\n\tVSI APTS Mode0 transfer Test. . . \n");

    // Init APTS
    write_mreg32(APTS_CTRL1, APTS_EN(0));
    div = (APTS_CLK27_FREQ / SPI_CLK) / 2 - 1;
    write_mreg32(APTS_DIVIDER, div);
    // Transfer 16 bits once, Mode0 and Mode1 have to use 16 bits mode
    // STC timer value is 33 bits, so the SPI will trigger 3 times for it.
    write_mreg32(APTS_CTRL2, WD_LEN(0) | APTS_OPMOD(0) | CHAR_LEN(0x0f) | ASS(1) | LSB(0) | NEG(0));
    write_mreg32(APTS_CTRL1, APTS_EN(1));
    udelay(1000);

    printf("\n\t\t- - - Press SW1 PB to trigger APTS_REQ ...\n");
    printf("\t\t- - - Check data and interrupt on logic analyzer? [y/n] ");

    while(1){
        get = CK_WaitForReply();

        if((get == 1)) {

            val_l = read_mreg32(VSI_STC_TIMER_CUR_VALUE_L);
            val_h = read_mreg32(VSI_STC_TIMER_CUR_VALUE_H);
            printf("\n\t\t\t STC current value_h=0x%x, value_l=0x%x\n",
                    val_h, val_l);
            printf("\n\t\t\t Mode0 PASS\n");
            break;
        } else if (get == 0) {
            printf("\n\t\t\t Mode0 FAIL\n");
            break;
        } else
            printf("\n\tPlease enter 'y' or 'n'   ");
    }
}

void Mode1_Test() {
    CK_UINT8 get;
    CK_UINT32 div;
    CK_UINT32 val_h;
    CK_UINT32 val_l;

    printf("\n\tVSI APTS Mode1 transfer Test. . . \n");

    // Init APTS
    write_mreg32(APTS_CTRL1, APTS_EN(0));
    div = (APTS_CLK27_FREQ / SPI_CLK) / 2 - 1;
    write_mreg32(APTS_DIVIDER, div);
    // Transfer 16 bits once, Mode0 and Mode1 have to use 16 bits mode
    // STC timer value is 33 bits, so the SPI will trigger 3 times for it.
    write_mreg32(APTS_CTRL2, WD_LEN(0) | APTS_OPMOD(1) | CHAR_LEN(0x0f) | ASS(1) | LSB(0) | NEG(0));
    // Set AOFFST = 0x10, AUDLEN = 0x20
    write_mreg32(APTS_AUDCTL, AOFFSET(0x10) | AUDLEN(0x480));
    write_mreg32(APTS_INTEN, 0x7);
    udelay(100);
    write_mreg32(APTS_CTRL1, APTS_EN(1));

    printf("\t\t- - - Check data and interrupt on logic analyzer? [y/n] ");

    while(1){
        get = CK_WaitForReply();

        if((get == 1)) {

            val_l = read_mreg32(VSI_STC_TIMER_CUR_VALUE_L);
            val_h = read_mreg32(VSI_STC_TIMER_CUR_VALUE_H);
            printf("\n\t\t\t STC current value_h=0x%x, value_l=0x%x\n",
                    val_h, val_l);
            printf("\n\t\t\t Mode1 PASS\n");
            break;
        } else if (get == 0) {
            printf("\n\t\t\t Mode1 FAIL\n");
            break;
        } else
            printf("\n\tPlease enter 'y' or 'n'   ");
    }
}

void CK_APTS_Test() {
    CK_UINT32 val;

    printf("\nVSI APTS Test. . . \n");

    INIT_STC();

    memset(&apts_irqhandler,0,sizeof(PCKStruct_IRQHandler));
    // Request Interrupt ISR
    apts_irqhandler.devname = "APTS";
    apts_irqhandler.irqid = CK_INTC_APTS;
    apts_irqhandler.priority = CK_INTC_APTS - 64;
    apts_irqhandler.handler = CK_APTS_ISR_Handler;
    apts_irqhandler.bfast = FALSE;
    apts_irqhandler.next = NULL;
    /* Register APTS ISR */
    CK_INTC_RequestIrq(&apts_irqhandler, AUTO_MODE);

    // Release I2S_PTS
    val = read_mreg32(CRM_BLK_SW_RST3) | APTS_SW_RST;
    write_mreg32(CRM_BLK_SW_RST3, val);

    Simple_SPI_Test();

    Mode0_Test();

    Mode1_Test();

    printf("\n\tEnd VSI APTS Test\n");
}
