/*
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "ck810.h"
#include "misc.h"
#include "pmu.h"

void domain_power_off(u32 gating, u32 pwr) {
    u32 reg;

    printf("\n do domain power down. . . \n");

    /* request disconnection of the power domain */
    pmu_writel(0x01, pwr);

    /* wait for bus enter into idle state */
    while ((pmu_readl(pwr) & 0x06) != 0x06); // Add timeout
#if 0
    /* AI power domain */
    if (pwr == VIP_HARD_PWR_CTL) {
        pmu_writel(0x01, CAMB_HARD_PWR_CTL);
        while ((pmu_readl(CAMB_HARD_PWR_CTL) & 0x06) != 0x06);

        pmu_writel(0x01, CDVS_HARD_PWR_CTL);
        while ((pmu_readl(CDVS_HARD_PWR_CTL) & 0x06) != 0x06);
    }
#endif

    /* enable isolation */
    reg = pmu_readl(gating);
    reg |= BIT(2);
    pmu_writel(reg, gating);

    /* reset block which will be power down */
    reg &= ~BIT(1);
    pmu_writel(reg, gating);

    /* power down block */
    reg |= BIT(0);
    pmu_writel(reg, gating);
    printf("domain power down now. . . \n\n");
}

void domain_power_on(u32 gating, u32 pwr) {
    u32 reg;

    printf("\n do domain power up. . . \n");

    /* power on block */
    reg = pmu_readl(gating);
    reg &= ~BIT(0);
    pmu_writel(reg, gating);

    /* wait for the supply stable */
    udelay(1000);

    /* release reset */
    reg = pmu_readl(gating);
    reg |= BIT(1);
    pmu_writel(reg, gating);

    /* release isolation */
    reg = pmu_readl(gating);
    reg &= ~BIT(2);
    pmu_writel(reg, gating);

    pmu_writel(0x00, pwr);
    while (pmu_readl(pwr) & 0x06);
#if 0
    if (pwr == VIP_HARD_PWR_CTL) {
        /* AI power domain */
        pmu_writel(0x00, CAMB_HARD_PWR_CTL);
        while (pmu_readl(CAMB_HARD_PWR_CTL) & 0x06);

        pmu_writel(0x00, CDVS_HARD_PWR_CTL);
        while (pmu_readl(CAMB_HARD_PWR_CTL) & 0x06);
    }
#endif
    printf("domain power up now. . . \n");
}
