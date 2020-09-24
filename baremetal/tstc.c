/*****************************************************************************
 *  File: tstc.c
 *
 *  Descirption: this file contains the STC test cases.
 *
 *  Copyright (C) : 2018 VeriSilicon.
 *
 *  Author: Chunming Li
 *  Date:   Dec 20 2018
 *
 *****************************************************************************/

#include "stc.h"
#include "crm.h"
#include "misc.h"

#define TEST_LEN                    1

static CK_UINT32 init_val_h[TEST_LEN] = {0x0, 0x1, 0x0, 0x1};
static CK_UINT32 init_val_l[TEST_LEN] = {0x0, 0x1000, 0xFFFFFFF0, 0xFFFFFFF0};

void STC_INIT_VAL(CK_UINT32 init_h, CK_UINT32 init_l, bool overturn) {
    CK_UINT32 val_h;
    CK_UINT32 val_l;
    CK_UINT64 init_val;
    CK_UINT64 current_val;

    init_val = ((init_h & 0x1) << 32) | init_l;

    printf("\n\t\tTest initial init_h=0x%x, init_l=0x%x\n", init_h, init_l);

    // Set initial value of timer
    write_mreg32(VSI_STC_TIMER_INIT_VALUE_L, init_l);
    write_mreg32(VSI_STC_TIMER_INIT_VALUE_H, init_h);
    // Enable STC
    write_mreg32(VSI_STC_ENABLE, VSI_STC_EN);

    // STC counter should add 90
    udelay(1000 * 1000 * 20);

    val_l = read_mreg32(VSI_STC_TIMER_CUR_VALUE_L);
    val_h = read_mreg32(VSI_STC_TIMER_CUR_VALUE_H);

    current_val = (((CK_UINT64)val_h & 0x1) << 32) | val_l;

    if (((current_val > init_val) && !overturn) || ((current_val <init_val) && overturn))
        printf("\n\t\t\t ---PASS val_h=0x%x, val_l=0x%x\n", val_h, val_l);
    else
        printf("\n\t\t\t ---FAIL val_h=0x%x, val_l=0x%x\n", val_h, val_l);

    // Disable STC
    write_mreg32(VSI_STC_ENABLE, 0);
}

void CK_STC_Test() {
    CK_UINT8 i;

    printf("\nVSI STC Test. . . \n");

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
    // STC Clock should be 90K
    write_mreg32(VSI_STC_CLK_DIVIDER, 0x12B);

    printf("\n\tTest different initial value of timer \n");

    for (i = 0; i < TEST_LEN; i++)
        STC_INIT_VAL(init_val_h[i], init_val_l[i], i == 3);

    printf("\n\tEnd VSI STC Test\n");
}
