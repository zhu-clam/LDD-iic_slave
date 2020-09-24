/*
 * [Description] PMU header
 *
 * Maintainer: jwen <jianxian.wen@verisilicon.com>
 *
 * Copyright (C) 2019 Verisilicon Inc.
 *
 */
 
#ifndef __VSI_PMU_H_
#define __VSI_PMU_H_

#include "ck810.h"
#include "datatype.h"
#include "misc.h"

#define AI_POWER_GATING_CTL         0x000
#define CDVS_HARD_PWR_CTL           0x130
#define CAMB_HARD_PWR_CTL           0x138
#define VIP_HARD_PWR_CTL            0x12c

#define AVS_POWER_GATING_CTL        0x004
#define AVS2_HARD_PWR_CTL           0x134

#define VC8000E_POWER_GATING_CTL    0x008
#define VC8000E_HARD_PWR_CTL        0x118

#define GC620_POWER_GATING_CTL      0x00c
#define GC620_HARD_PWR_CTL          0x120

#define ISP_POWER_GATING_CTL        0x010
#define ISP_HARD_PWR_CTL            0x128

#define UNICORE_POWER_GATING_CTL    0x020
#define UNICORE_HARD_PWR_CTL        0x108

#define CK860_POWER_GATING_CTL      0x018
#define CK860_HARD_PWR_CTL          0x100

#define CK810_POWER_GATING_CTL      0x01c
#define CK810_HARD_PWR_CTL          0x104

#define PCIE_HARD_PWR_CTL           0x10c
#define JPEG_HARD_PWR_CTL           0x110
#define VIDEOIN_HARD_PWR_CTL        0x114
#define VC8000D_HARD_PWR_CTL        0x11c
#define DC8000_HARD_PWR_CTL         0x124

void domain_power_off(u32 gating, u32 pwr);
void domain_power_on(u32 gating, u32 pwr);

#endif /* __VSI_PMU_H_ */
