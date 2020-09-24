// CRM registers addr define

#ifndef ___CRM_H___
#define ___CRM_H___

#include "ck810.h"

#define CRM_TSM_PLL_BYP                 CK_CRM_ADDR + 0x74
#define CRM_TSMPLL_REFCLK_CFG           CK_CRM_ADDR + 0x144
#define CRM_TS_CLK_CFG                  CK_CRM_ADDR + 0x148
#define CRM_BLK_CLK_ICG3                CK_CRM_ADDR + 0x20c

#define OTP_SW_RST      1 << 18
#define CRM_BLK_SW_RST3                 CK_CRM_ADDR + 0x81c
#define I2S_PTS_RST                     BIT(31)
#define SCI_SW_RST                      BIT(25)
#define STC_SW_RST                      BIT(1)
#define APTS_SW_RST                     BIT(0)

/* Offset in CRM */
#define SYS_PLL_PARAM		0x000
#define SYS_PLL_BYP		0x004
#define SYS_PLL_PD		0x008
#define SYS_PLL_LOCK		0x00C
#define CK_PLL_PARAM		0x010
#define CK_PLL_BYP		0x014
#define CK_PLL_PD		0x018
#define CK_PLL_LOCK		0x01C
#define UNI_PLL_PARAM		0x020
#define UNI_PLL_BYP		0x024
#define UNI_PLL_PD		0x028
#define UNI_PLL_LOCK		0x02C
#define DDR_PLL_PARAM		0x030
#define DDR_PLL_BYP		0x034
#define DDR_PLL_PD		0x038
#define DDR_PLL_LOCK		0x03C
#define VIDEO0_PLL_PARAM	0x040
#define VIDEO0_PLL_BYP		0x044
#define VIDEO0_PLL_PD		0x048
#define VIDEO0_PLL_LOCK		0x04C
#define VIDEO1_PLL_PARAM	0x050
#define VIDEO1_PLL_BYP		0x054
#define VIDEO1_PLL_PD		0x058
#define VIDEO1_PLL_LOCK		0x05C
#define DSP_PLL_PARAM		0x060
#define DSP_PLL_BYP		0x064
#define DSP_PLL_PD		0x068
#define DSP_PLL_LOCK		0x06C
#define TSM_PLL_PARAM		0x070
#define TSM_PLL_BYP		0x074
#define TSM_PLL_PD		0x078
#define TSM_PLL_LOCK		0x07C
#define GMAC_PLL_PARAM		0x080
#define GMAC_PLL_BYP		0x084
#define GMAC_PLL_PD		0x088
#define GMAC_PLL_LOCK		0x08C
#define PIXEL_PLL_PARAM		0x090
#define PIXEL_PLL_BYP		0x094
#define PIXEL_PLL_PD		0x098
#define PIXEL_PLL_LOCK		0x09C
#define AUDIO_PLL_PARAM	0x0A0
#define AUDIO_PLL_BYP		0x0A4
#define AUDIO_PLL_PD		0x0A8
#define AUDIO_PLL_LOCK		0x0AC
#define SENSOR_PLL_PARAM	0x0B0
#define SENSOR_PLL_BYP		0x0B4
#define SENSOR_PLL_PD		0x0B8
#define SENSOR_PLL_LOCK	0x0BC
#define SYS_CLK_CFG		0x100
#define CK860_CLK_CFG		0x104
#define CK810_CLK_CFG		0x108
#define UNI_CLK_CFG		0x10C
#define XDMA_CLK_CFG		0x110
#define GMAC_CLK_CFG		0x114
#define SDIO_CLK_CFG		0x118
#define PCIE_CLK_CFG		0x11C
#define I2S_CLK_CFG		0x120
#define VIN_CLK_CFG		0x124
#define VDEC_CLK_CFG		0x128
#define VENC_CLK_CFG		0x12C
#define JPEG_CLK_CFG		0x130
#define ISP_CLK_CFG		0x134
#define GPU_CLK_CFG		0x138
#define VOUT_CLK_CFG		0x13C
#define AVS2_CLK_CFG		0x140
#define TSMPLL_REFCLK_CFG	0x144
#define TS_CLK_CFG		0x148
#define C5_CLK_CFG		0x14C
#define P6_CLK_CFG		0x150
#define CDVS_CLK_CFG		0x154
#define VIP_CLK_CFG		0x158
#define CAMB_CLK_CFG		0x15C
#define SCI_CLK_CFG		0x160
#define GPIO_DBCLK_CFG		0x168
#define TIMER_CLK_CFG		0x16C
#define SENSOR0_CLK_CFG	0x170
#define SENSOR1_CLK_CFG	0x174
#define DISPLAY_CLK_CFG		0x178
#define MIPI_EXT_CLK_CFG	0x180
#define BLK_CLK_ICG0		0x200
#define BLK_CLK_ICG1		0x204
#define BLK_CLK_ICG2		0x208
#define BLK_CLK_ICG3		0x20C
#define BLK_CLK_ICG4		0x210

#define GLB_SW_RST		0x800
#define GLB_SW_WIDTH		0x804
#define BLK_SW_RST0		0x810
#define BLK_SW_RST1		0x814
#define BLK_SW_RST2		0x818
#define BLK_SW_RST3		0x81C
#define RESET_STATUS		0x880

#endif // ___CRM_H___
