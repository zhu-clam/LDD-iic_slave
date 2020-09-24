/*
 * [Description] Clock Gating test
 *
 * Maintainer: jwen <jianxian.wen@verisilicon.com>
 *
 * Copyright (C) 2019 Verisilicon Inc.
 *
 */

#include "ck810.h"
#include "misc.h"
#include "crm.h"


// check macro IP_RELEASE_* in ck810.h
void ip_clk_gating_test(void) {
    u32 value;
    u32 default_value;
    u32 pll_reg;

    printf("\nIP clock gating test. . . \n");

    printf("\n\n\t- - - ISP0 Clock Gating...\n");
    /* Contains the release specific revision ID */
    default_value = read_mreg32(CK_ISP0_Slave + 0x008);
    pll_reg = crm_readl(BLK_CLK_ICG1);
    pll_reg |= BIT(1);
    crm_writel(pll_reg, BLK_CLK_ICG1);
    value = read_mreg32(CK_ISP0_Slave + 0x008);
    pll_reg &= ~BIT(1);
    crm_writel(pll_reg, BLK_CLK_ICG1);
    if (default_value != value)
        printf("\n\t\t- - - PASS...\n");
    else
        printf("\n\t\t- - - FAIL...\n");

    printf("\n\n\t- - - ISP1 Clock Gating...\n");
    /* Contains the release specific revision ID */
    default_value = read_mreg32(CK_ISP1_Slave + 0x008);
    pll_reg = crm_readl(BLK_CLK_ICG1);
    pll_reg |= BIT(2);
    crm_writel(pll_reg, BLK_CLK_ICG1);
    value = read_mreg32(CK_ISP1_Slave + 0x008);
    pll_reg &= ~BIT(2);
    crm_writel(pll_reg, BLK_CLK_ICG1);
    if (default_value != value)
        printf("\n\t\t- - - PASS...\n");
    else
        printf("\n\t\t- - - FAIL...\n");

    printf("\n\n\t- - - GC620 Clock Gating...\n");
    /* GCChipRev */
    default_value = read_mreg32(CK_GC620_BASE + 0x024);
    pll_reg = crm_readl(BLK_CLK_ICG1);
    pll_reg |= BIT(3);
    crm_writel(pll_reg, BLK_CLK_ICG1);
    value = read_mreg32(CK_GC620_BASE + 0x024);
    pll_reg &= ~BIT(3);
    crm_writel(pll_reg, BLK_CLK_ICG1);
    if (default_value != value)
        printf("\n\t\t- - - PASS...\n");
    else
        printf("\n\t\t- - - FAIL...\n");

    printf("\n\n\t- - - AVS2 Clock Gating...\n");
    default_value = read_mreg32(CK_AVS2_BASE + 0x3004);
    pll_reg = crm_readl(BLK_CLK_ICG1);
    pll_reg |= BIT(7);
    crm_writel(pll_reg, BLK_CLK_ICG1);
    value = read_mreg32(CK_AVS2_BASE + 0x3004);
    pll_reg &= ~BIT(7);
    crm_writel(pll_reg, BLK_CLK_ICG1);
    if (default_value != value)
        printf("\n\t\t- - - PASS...\n");
    else
        printf("\n\t\t- - - FAIL...\n");

    printf("\n\n\t- - - VC8000E Clock Gating...\n");
    /* ID register */
    default_value = read_mreg32(CK_VC8000E_BASE + 0x00);
    pll_reg = crm_readl(BLK_CLK_ICG1);
    pll_reg |= BIT(4);
    crm_writel(pll_reg, BLK_CLK_ICG1);
    value = read_mreg32(CK_VC8000E_BASE + 0x00);
    pll_reg &= ~BIT(4);
    crm_writel(pll_reg, BLK_CLK_ICG1);
    if (default_value != value)
        printf("\n\t\t- - - PASS...\n");
    else
        printf("\n\t\t- - - FAIL...\n");

    printf("\n\n\t- - - VC8000D Clock Gating...\n");
    /* ID register */
    default_value = read_mreg32(CK_VC8000D_BASE + 0x00);
    pll_reg = crm_readl(BLK_CLK_ICG1);
    pll_reg |= BIT(6);
    crm_writel(pll_reg, BLK_CLK_ICG1);
    value = read_mreg32(CK_VC8000D_BASE + 0x00);
    pll_reg &= ~BIT(6);
    crm_writel(pll_reg, BLK_CLK_ICG1);
    if (default_value != value)
        printf("\n\t\t- - - PASS...\n");
    else
        printf("\n\t\t- - - FAIL...\n");

    printf("\n\n\t- - - DSP C5 Clock Gating...\n");
    /* ID register */
    default_value = read_mreg32(CK_DSP_C5_0_BASE + 0x04);
    pll_reg = crm_readl(BLK_CLK_ICG2);
    pll_reg |= BIT(0);
    crm_writel(pll_reg, BLK_CLK_ICG2);
    value = read_mreg32(CK_DSP_C5_0_BASE + 0x04);
    pll_reg &= ~BIT(0);
    crm_writel(pll_reg, BLK_CLK_ICG2);
    if (default_value != value)
        printf("\n\t\t- - - PASS...\n");
    else
        printf("\n\t\t- - - FAIL...\n");

    printf("\n\n\t- - - DSP P6 Clock Gating...\n");
    /* ID register */
    default_value = read_mreg32(CK_DSP_P6_0_BASE + 0x404);
    pll_reg = crm_readl(BLK_CLK_ICG2);
    pll_reg |= BIT(2);
    crm_writel(pll_reg, BLK_CLK_ICG2);
    value = read_mreg32(CK_DSP_P6_0_BASE + 0x404);
    pll_reg &= ~BIT(2);
    crm_writel(pll_reg, BLK_CLK_ICG2);
    if (default_value != value)
        printf("\n\t\t- - - PASS...\n");
    else
        printf("\n\t\t- - - FAIL...\n");

    printf("\n\n\t- - - VIP Clock Gating...\n");
    /* GCChipRev */
    default_value = read_mreg32(CK_VIP_BASE + 0x024);
    pll_reg = crm_readl(BLK_CLK_ICG2);
    pll_reg |= BIT(4);
    crm_writel(pll_reg, BLK_CLK_ICG2);
    value = read_mreg32(CK_VIP_BASE + 0x024);
    pll_reg &= ~BIT(4);
    crm_writel(pll_reg, BLK_CLK_ICG2);
    if (default_value != value)
        printf("\n\t\t- - - PASS...\n");
    else
        printf("\n\t\t- - - FAIL...\n");

    printf("\n\n\t- - - CAMB Clock Gating...\n");
    default_value = read_mreg32(CK_CAMB_BASE + 0x210);
    pll_reg = crm_readl(BLK_CLK_ICG2);
    pll_reg |= BIT(5);
    crm_writel(pll_reg, BLK_CLK_ICG2);
    value = read_mreg32(CK_CAMB_BASE + 0x210);
    pll_reg &= ~BIT(5);
    crm_writel(pll_reg, BLK_CLK_ICG2);
    if (default_value != value)
        printf("\n\t\t- - - PASS...\n");
    else
        printf("\n\t\t- - - FAIL...\n");

    printf("\n\n\t- - - CDVS Clock Gating...\n");
    default_value = read_mreg32(CK_CDVS_BASE + 0x040);
    pll_reg = crm_readl(BLK_CLK_ICG2);
    pll_reg |= BIT(6);
    crm_writel(pll_reg, BLK_CLK_ICG2);
    value = read_mreg32(CK_CDVS_BASE + 0x040);
    pll_reg &= ~BIT(6);
    crm_writel(pll_reg, BLK_CLK_ICG2);
    if (default_value != value)
        printf("\n\t\t- - - PASS...\n");
    else
        printf("\n\t\t- - - FAIL...\n");

    printf("\nIP clock gating test done. . . \n");
}
