/*
 * [Description] PMU test
 *
 * Maintainer: jwen <jianxian.wen@verisilicon.com>
 *
 * Copyright (C) 2019 Verisilicon Inc.
 *
 */

#include "ck810.h"
#include "misc.h"
#include "pmu.h"

void domain_power_test() {
    u8 test_id = 0;
    u8 ch;
    u32 gating_offset;
    u32 pwr_offset;

    printf("\ndomain power up/down test. . . \n");
    while (1) {
        printf("\n please choose test domain:\n");
        printf("0 -- ISP\n");
        printf("1 -- GC620\n");
        printf("2 -- AVS\n");
        printf("3 -- VC8000E\n");
        printf("4 -- AI(VIP, Cambricon, CDVS and DSP)\n");
#if CK_CK860
        printf("5 -- CK810\n");
#else
        printf("5 -- CK860\n");
#endif
        printf("> ");
        ch = getchar();
        putchar(ch);
        test_id = asciitonum((CK_UINT8 *)&ch);
        if ((test_id >= 0) && (test_id <= 5)) {
            break;
        }
    }

    switch (test_id) {
    case 0:
        gating_offset = ISP_POWER_GATING_CTL;
        pwr_offset = ISP_HARD_PWR_CTL;
        break;
    case 1:
        gating_offset = GC620_POWER_GATING_CTL;
        pwr_offset = GC620_HARD_PWR_CTL;
        break;
    case 2:
        gating_offset = AVS_POWER_GATING_CTL;
        pwr_offset = AVS2_HARD_PWR_CTL;
    case 3:
        gating_offset = VC8000E_POWER_GATING_CTL;
        pwr_offset = VC8000E_HARD_PWR_CTL;
        break;
    case 4:
        gating_offset = AI_POWER_GATING_CTL;
        pwr_offset = VIP_HARD_PWR_CTL;
        break;
    case 5:
#if CK_CK860
        gating_offset = CK810_POWER_GATING_CTL;
        pwr_offset = CK810_HARD_PWR_CTL;
#else
        gating_offset = CK860_POWER_GATING_CTL;
        pwr_offset = CK860_HARD_PWR_CTL;
#endif
        break;
    default:
        return;
    }

    domain_power_off(gating_offset, pwr_offset);
    printf("\n whether to power up domain:\n");
    printf("\r\t y: power up\n");
    printf("\r\t n: keep power down\n\t\t");
    printf("- - - [y/n] ");
    while(1){
        ch = CK_WaitForReply();
        if((ch == 1) || (ch == 0))
            break;
        else
            printf("\n\t please enter 'y' or 'n'   ");
    }
    printf("\n");

    if(ch == 1) {
        domain_power_on(gating_offset, pwr_offset);
    }
    printf("\ndomain power test done. . . \n");
}
