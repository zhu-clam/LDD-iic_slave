/*
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "ck810.h"
#include "crm.h"
#include "datatype.h"
#include "misc.h"
#include "pmu.h"

#ifdef CONFIG_IS_ASIC
void pll_init(void)
{
	u32 val;

#if DOMAIN_CK810_POWER_OFF
	printf("domain CK810 power down\n");
	domain_power_off(CK810_POWER_GATING_CTL, CK810_HARD_PWR_CTL);
#endif
#if DOMAIN_ISP_POWER_OFF
	printf("domain ISP power down\n");
	domain_power_off(ISP_POWER_GATING_CTL, ISP_HARD_PWR_CTL);
#endif
#if DOMAIN_GC620_POWER_OFF
	printf("domain GC620 power down\n");
	domain_power_off(GC620_POWER_GATING_CTL, GC620_HARD_PWR_CTL);
#endif
#if DOMAIN_AVS_POWER_OFF
	printf("domain AVS power down\n");
	domain_power_off(AVS_POWER_GATING_CTL, AVS2_HARD_PWR_CTL);
#endif
#if DOMAIN_VC8000E_POWER_OFF
	printf("domain VC8000E power down\n");
	domain_power_off(VC8000E_POWER_GATING_CTL, VC8000E_HARD_PWR_CTL);
#endif
#if DOMAIN_AI_POWER_OFF
	printf("domain AI power down\n");
	domain_power_off(AI_POWER_GATING_CTL, VIP_HARD_PWR_CTL);
#endif

	/* 1. Wait for all PLL stable */
	while (!(crm_readl(SYS_PLL_LOCK) & 0x01));
	while (!(crm_readl(CK_PLL_LOCK) & 0x01));
	while (!(crm_readl(UNI_PLL_LOCK) & 0x01));
	//while (!(crm_readl(VIDEO0_PLL_LOCK) & 0x01));
	//while (!(crm_readl(VIDEO1_PLL_LOCK) & 0x01));
	//while (!(crm_readl(DSP_PLL_LOCK) & 0x01));
	//while (!(crm_readl(TSM_PLL_LOCK) & 0x01));
	//while (!(crm_readl(GMAC_PLL_LOCK) & 0x01));
	//while (!(crm_readl(PIXEL_PLL_LOCK) & 0x01));
	//while (!(crm_readl(AUDIO_PLL_LOCK) & 0x01));
	//while (!(crm_readl(SENSOR_PLL_LOCK) & 0x01));

	/* 2. Enable BYPASS mode and PLL power down */
	crm_writel(0x01, SYS_PLL_BYP);
	crm_writel(0x0b, SYS_PLL_PD);
	crm_writel(0x01, CK_PLL_BYP);
	crm_writel(0x0b, CK_PLL_PD);
	crm_writel(0x01, UNI_PLL_BYP);
	crm_writel(0x0b, UNI_PLL_PD);
	crm_writel(0x01, VIDEO0_PLL_BYP);
	crm_writel(0x0f, VIDEO0_PLL_PD);
	crm_writel(0x01, VIDEO1_PLL_BYP);
	crm_writel(0x0f, VIDEO1_PLL_PD);
	crm_writel(0x01, DSP_PLL_BYP);
	crm_writel(0x0f, DSP_PLL_PD);
	crm_writel(0x01, TSM_PLL_BYP);
	crm_writel(0x0f, TSM_PLL_PD);
	crm_writel(0x01, GMAC_PLL_BYP);
	crm_writel(0x0f, GMAC_PLL_PD);
	crm_writel(0x01, PIXEL_PLL_BYP);
	crm_writel(0x0f, PIXEL_PLL_PD);
	crm_writel(0x01, AUDIO_PLL_BYP);
	crm_writel(0x0f, AUDIO_PLL_PD);
	crm_writel(0x01, SENSOR_PLL_BYP);
	crm_writel(0x0f, SENSOR_PLL_PD);

	/* 3. Update **_PLL_PARAM registers
	 * FOUT = ((24M or 27M) * FBDIV) / (REFDIV * POSTDIV1 * POSTDIV2)
	 */
	set_pll_param(3, 100, 2, 1, SYS_PLL_PARAM); // 400M
	set_pll_param(3, 177, 2, 1, CK_PLL_PARAM); // 710M, CK860
	// set_pll_param(3, 200, 2, 1, UNI_PLL_PARAM); // 800M
	set_pll_param(3, 233, 2, 1, UNI_PLL_PARAM); // 933M, CK810
	set_pll_param(3, 200, 1, 1, VIDEO0_PLL_PARAM); // 1600M
	set_pll_param(2, 100, 1, 1, VIDEO1_PLL_PARAM); // 1200M
	set_pll_param(2, 100, 1, 1, DSP_PLL_PARAM); // 1200M
#if TSMPLL_24M_CLOCKIN
	set_pll_param(3, 175, 2, 1, TSM_PLL_PARAM); // 700M, CAMB
#else
	set_pll_param(3, 192, 2, 1, TSM_PLL_PARAM); // 864M, TSM
#endif
	set_pll_param(2, 125, 1, 1, GMAC_PLL_PARAM); // 1500M
	set_pll_param(2, 99, 1, 1, PIXEL_PLL_PARAM); // 1188M
	set_pll_param(2, 128, 1, 1, AUDIO_PLL_PARAM); // 1536M
	set_pll_param(2, 99, 1, 1, SENSOR_PLL_PARAM); // 1188M

	/* 4. PLL power on */
	crm_writel(0x0a, SYS_PLL_PD);
	crm_writel(0x0a, CK_PLL_PD);
	crm_writel(0x0a, UNI_PLL_PD);
	crm_writel(0x0a, VIDEO0_PLL_PD);
	crm_writel(0x0a, VIDEO1_PLL_PD);
	crm_writel(0x0a, DSP_PLL_PD);
	crm_writel(0x0a, TSM_PLL_PD);
	crm_writel(0x0a, GMAC_PLL_PD);
	crm_writel(0x0a, PIXEL_PLL_PD);
	crm_writel(0x0a, AUDIO_PLL_PD);
	crm_writel(0x0a, SENSOR_PLL_PD);

	/* 5. Wait for all PLL lock */
	while (!(crm_readl(SYS_PLL_LOCK) & 0x01));
	while (!(crm_readl(CK_PLL_LOCK) & 0x01));
	while (!(crm_readl(UNI_PLL_LOCK) & 0x01));
	while (!(crm_readl(VIDEO0_PLL_LOCK) & 0x01));
	while (!(crm_readl(VIDEO1_PLL_LOCK) & 0x01));
	while (!(crm_readl(DSP_PLL_LOCK) & 0x01));
	while (!(crm_readl(TSM_PLL_LOCK) & 0x01));
	while (!(crm_readl(GMAC_PLL_LOCK) & 0x01));
	while (!(crm_readl(PIXEL_PLL_LOCK) & 0x01));
	while (!(crm_readl(AUDIO_PLL_LOCK) & 0x01));
	while (!(crm_readl(SENSOR_PLL_LOCK) & 0x01));

	/* 6. Set DIV and change PLL source clock */
	/* 6.1 IP clock gating */
	crm_writel(0x001fff00, BLK_CLK_ICG0);
	crm_writel(0x00001fff, BLK_CLK_ICG1);
	crm_writel(0x0000007f, BLK_CLK_ICG2);
	crm_writel(0x82000003, BLK_CLK_ICG3);
	//crm_writel(0x0000001f, BLK_CLK_ICG4);

	/* 6.2 IP release */
	crm_writel(0x001fff00, BLK_SW_RST0);

	val = 0x00000461;
#if IP_RELEASE_RSCODEC
	val |= BIT(12);
#endif
#if IP_RELEASE_TSPSI
	val |= BIT(11);
#endif

#if !DOMAIN_AVS_POWER_OFF
#if IP_RELEASE_AVS610
	val |= BIT(9);
#endif
#if IP_RELEASE_AVSP
	val |= BIT(8);
#endif
#if IP_RELEASE_AVS2
	val |= BIT(7);
#endif
#endif /* DOMAIN_AVS_POWER_OFF */

#if !DOMAIN_VC8000E_POWER_OFF
	val |= BIT(4);
#endif

#if !DOMAIN_GC620_POWER_OFF
	val |= BIT(3);
#endif

#if !DOMAIN_ISP_POWER_OFF
	val |= (BIT(1) | BIT(2));
#endif
	crm_writel(val, BLK_SW_RST1);

#if DOMAIN_AI_POWER_OFF
	val = 0x00;
#else
	val = BIT(4); /* Release VIPNano */
#if IP_RELEASE_C50_CORE
	val |= BIT(0);
#endif
#if IP_RELEASE_C51_CORE
	val |= BIT(1);
#endif
#if IP_RELEASE_P60_CORE
	val |= BIT(2);
#endif
#if IP_RELEASE_P61_CORE
	val |= BIT(3);
#endif
#if IP_RELEASE_CAMB
	val |= BIT(5);
#endif
#if IP_RELEASE_CDVS
	val |= BIT(6);
#endif
#if IP_RELEASE_C50_CSR
	val |= BIT(7);
#endif
#if IP_RELEASE_C51_CSR
	val |= BIT(8);
#endif
#if IP_RELEASE_P60_CSR
	val |= BIT(9);
#endif
#if IP_RELEASE_P61_CSR
	val |= BIT(10);
#endif
#if IP_RELEASE_WDT_C50
	val |= BIT(16);
#endif
#if IP_RELEASE_WDT_C51
	val |= BIT(17);
#endif
#if IP_RELEASE_WDT_P60
	val |= BIT(18);
#endif
#if IP_RELEASE_WDT_P61
	val |= BIT(19);
#endif
#if IP_RELEASE_TIMER_DSP
	val |= BIT(20);
#endif
#if IP_RELEASE_UART0_DSP
	val |= BIT(21);
#endif
#if IP_RELEASE_UART1_DSP
	val |= BIT(22);
#endif
#if IP_RELEASE_UART2_DSP
	val |= BIT(23);
#endif
#if IP_RELEASE_UART3_DSP
	val |= BIT(24);
#endif
#endif /* DOMAIN_AI_POWER_OFF */
	crm_writel(val, BLK_SW_RST2);

	crm_writel(0x82000003, BLK_SW_RST3);

	/* 6.3 Set DIV and clock source */
	crm_writel(0x3110, SYS_CLK_CFG);
	crm_writel(0x01, CK860_CLK_CFG);
	crm_writel(0x01, CK810_CLK_CFG); /* set ck810 clock source from UNI */
	// crm_writel(0x21, UNI_CLK_CFG);
	crm_writel(0x01, XDMA_CLK_CFG);
	crm_writel(0x05, GMAC_CLK_CFG);
	crm_writel(0x101, SDIO_CLK_CFG); /* SD clock source from SYS_PLL */
	crm_writel(0x0e, PCIE_CLK_CFG);
	crm_writel(0x307c, I2S_CLK_CFG); /* clock source from AUDIO_PLL */
	crm_writel(0x03, VIN_CLK_CFG);
	crm_writel(0x03, VDEC_CLK_CFG);
	crm_writel(0x03, VENC_CLK_CFG);
	crm_writel(0x03, JPEG_CLK_CFG);
	crm_writel(0x03, ISP_CLK_CFG);
	crm_writel(0x01, GPU_CLK_CFG);
	crm_writel(0x07, VOUT_CLK_CFG);
	crm_writel(0x121, AVS2_CLK_CFG);
#if TSMPLL_24M_CLOCKIN
	/* 24MHz */
	crm_writel(0x00, TSMPLL_REFCLK_CFG);
	crm_writel(0x00, CAMB_CLK_CFG);
#else
	/* external 27MHz */
	crm_writel(0x01, TSMPLL_REFCLK_CFG);
	crm_writel(0x2bf1, TS_CLK_CFG);
#endif
	crm_writel(0x01, C5_CLK_CFG);
	crm_writel(0x01, P6_CLK_CFG);
	crm_writel(0x303, CDVS_CLK_CFG);
	crm_writel(0x01, VIP_CLK_CFG);
	crm_writel(0x0e, SCI_CLK_CFG);
	crm_writel(0x00, TIMER_CLK_CFG);
	crm_writel(0x62, SENSOR0_CLK_CFG);
	crm_writel(0x62, SENSOR1_CLK_CFG);
	crm_writel(0x07, DISPLAY_CLK_CFG);
	// MIPI_EXT_CLK_CFG

	/* 6.4 IP clock enable */
	crm_writel(0x00, BLK_CLK_ICG0);

	val = 0x00;
#if !IP_RELEASE_RSCODEC
	val |= BIT(12);
#endif
#if !IP_RELEASE_TSPSI
	val |= BIT(11);
#endif

#if DOMAIN_AVS_POWER_OFF
	val |= (BIT(7) | BIT(8) | BIT(9));
#else /* power on */
#if !IP_RELEASE_AVS610
	val |= BIT(9);
#endif
#if !IP_RELEASE_AVSP
	val |= BIT(8);
#endif
#if !IP_RELEASE_AVS2
	val |= BIT(7);
#endif
#endif /* DOMAIN_AVS_POWER_OFF */

#if DOMAIN_VC8000E_POWER_OFF
	val |= BIT(4);
#endif

#if DOMAIN_GC620_POWER_OFF
	val |= BIT(3);
#endif

#if DOMAIN_ISP_POWER_OFF
	val |= (BIT(1) | BIT(2));
#endif
	crm_writel(val, BLK_CLK_ICG1);

#if DOMAIN_AI_POWER_OFF
	val = 0x7f;
#else
	val = 0x00;
#if !IP_RELEASE_C50_CORE
	val |= BIT(0);
#endif
#if !IP_RELEASE_C51_CORE
	val |= BIT(1);
#endif
#if !IP_RELEASE_P60_CORE
	val |= BIT(2);
#endif
#if !IP_RELEASE_P61_CORE
	val |= BIT(3);
#endif
#if !IP_RELEASE_CAMB
	val |= BIT(5);
#endif
#if !IP_RELEASE_CDVS
	val |= BIT(6);
#endif
#endif /* DOMAIN_AI_POWER_OFF */
	crm_writel(val, BLK_CLK_ICG2);

	crm_writel(0x00, BLK_CLK_ICG3);
	crm_writel(0x00, BLK_CLK_ICG4);

	/* 7. Disable PLL BYPASS, set to normal mode */
	crm_writel(0x00, SYS_PLL_BYP);
	crm_writel(0x00, CK_PLL_BYP);
	// crm_writel(0x00, UNI_PLL_BYP);
	crm_writel(0x00, VIDEO0_PLL_BYP);
	crm_writel(0x00, VIDEO1_PLL_BYP);
	crm_writel(0x00, DSP_PLL_BYP);
	crm_writel(0x00, TSM_PLL_BYP);
	crm_writel(0x00, GMAC_PLL_BYP);
	crm_writel(0x00, PIXEL_PLL_BYP);
	crm_writel(0x00, AUDIO_PLL_BYP);
	crm_writel(0x00, SENSOR_PLL_BYP);
}

void dump_pll_regs(void)
{
#ifdef CONFIG_DUMP_VSI_PLL_REGS
	printf("\nDump PLL registers:\n\n");
	printf("SYS_PLL_PARAM = 0x%x\n", crm_readl(SYS_PLL_PARAM));
	printf("CK_PLL_PARAM = 0x%x\n", crm_readl(CK_PLL_PARAM));
	printf("UNI_PLL_PARAM = 0x%x\n", crm_readl(UNI_PLL_PARAM));
	printf("VIDEO0_PLL_PARAM = 0x%x\n", crm_readl(VIDEO0_PLL_PARAM));
	printf("VIDEO1_PLL_PARAM = 0x%x\n", crm_readl(VIDEO1_PLL_PARAM));
	printf("DSP_PLL_PARAM = 0x%x\n", crm_readl(DSP_PLL_PARAM));
	printf("TSM_PLL_PARAM = 0x%x\n", crm_readl(TSM_PLL_PARAM));
	printf("GMAC_PLL_PARAM = 0x%x\n", crm_readl(GMAC_PLL_PARAM));
	printf("PIXEL_PLL_PARAM = 0x%x\n", crm_readl(PIXEL_PLL_PARAM));
	printf("AUDIO_PLL_PARAM = 0x%x\n", crm_readl(AUDIO_PLL_PARAM));
	printf("SENSOR_PLL_PARAM = 0x%x\n", crm_readl(SENSOR_PLL_PARAM));
	printf("SYS_CLK_CFG = 0x%x\n", crm_readl(SYS_CLK_CFG));
	printf("CK860_CLK_CFG = 0x%x\n", crm_readl(CK860_CLK_CFG));
	printf("CK810_CLK_CFG = 0x%x\n", crm_readl(CK810_CLK_CFG));
	printf("UNI_CLK_CFG = 0x%x\n", crm_readl(UNI_CLK_CFG));
	printf("XDMA_CLK_CFG = 0x%x\n", crm_readl(XDMA_CLK_CFG));
	printf("GMAC_CLK_CFG = 0x%x\n", crm_readl(GMAC_CLK_CFG));
	printf("SDIO_CLK_CFG = 0x%x\n", crm_readl(SDIO_CLK_CFG));
	printf("PCIE_CLK_CFG = 0x%x\n", crm_readl(PCIE_CLK_CFG));
	printf("I2S_CLK_CFG = 0x%x\n", crm_readl(I2S_CLK_CFG));
	printf("VIN_CLK_CFG = 0x%x\n", crm_readl(VIN_CLK_CFG));
	printf("VDEC_CLK_CFG = 0x%x\n", crm_readl(VDEC_CLK_CFG));
	printf("VENC_CLK_CFG = 0x%x\n", crm_readl(VENC_CLK_CFG));
	printf("JPEG_CLK_CFG = 0x%x\n", crm_readl(JPEG_CLK_CFG));
	printf("ISP_CLK_CFG = 0x%x\n", crm_readl(ISP_CLK_CFG));
	printf("GPU_CLK_CFG = 0x%x\n", crm_readl(GPU_CLK_CFG));
	printf("VOUT_CLK_CFG = 0x%x\n", crm_readl(VOUT_CLK_CFG));
	printf("AVS2_CLK_CFG = 0x%x\n", crm_readl(AVS2_CLK_CFG));
	printf("TSMPLL_REFCLK_CFG = 0x%x\n", crm_readl(TSMPLL_REFCLK_CFG));
	printf("TS_CLK_CFG = 0x%x\n", crm_readl(TS_CLK_CFG));
	printf("C5_CLK_CFG = 0x%x\n", crm_readl(C5_CLK_CFG));
	printf("P6_CLK_CFG = 0x%x\n", crm_readl(P6_CLK_CFG));
	printf("CDVS_CLK_CFG = 0x%x\n", crm_readl(CDVS_CLK_CFG));
	printf("VIP_CLK_CFG = 0x%x\n", crm_readl(VIP_CLK_CFG));
	printf("CAMB_CLK_CFG = 0x%x\n", crm_readl(CAMB_CLK_CFG));
	printf("SCI_CLK_CFG = 0x%x\n", crm_readl(SCI_CLK_CFG));
	printf("GPIO_DBCLK_CFG = 0x%x\n", crm_readl(GPIO_DBCLK_CFG));
	printf("TIMER_CLK_CFG = 0x%x\n", crm_readl(TIMER_CLK_CFG));
	printf("SENSOR0_CLK_CFG = 0x%x\n", crm_readl(SENSOR0_CLK_CFG));
	printf("SENSOR1_CLK_CFG = 0x%x\n", crm_readl(SENSOR1_CLK_CFG));
	printf("DISPLAY_CLK_CFG = 0x%x\n", crm_readl(DISPLAY_CLK_CFG));
	printf("MIPI_EXT_CLK_CFG = 0x%x\n", crm_readl(MIPI_EXT_CLK_CFG));
#endif
}

void dump_clk_freq(void)
{
#ifdef CONFIG_DUMP_VSI_CLK_FREQ
	u32 clk;

	printf("\nDump clock frequency:\n\n");

	/* Spec. Table 5.1-1 */
	printf("SYS_PLL_CLKO = %dHz\n", SYS_PLL_CLKO);
	printf("CK_PLL_CLKO = %dHz\n", CK_PLL_CLKO);
	printf("UNI_PLL_CLKO = %dHz\n", UNI_PLL_CLKO);
	printf("DSP_PLL_CLKO = %dHz\n", DSP_PLL_CLKO);
	printf("VIDEO_PLL0_CLKO = %dHz\n", VIDEO_PLL0_CLKO);
	printf("VIDEO_PLL1_CLKO = %dHz\n", VIDEO_PLL1_CLKO);
	printf("GMAC_PLL_CLKO = %dHz\n", GMAC_PLL_CLKO);
	printf("PIXEL_PLL_CLKO = %dHz\n", PIXEL_PLL_CLKO);
	printf("AUDIO_PLL_CLKO = %dHz\n", AUDIO_PLL_CLKO);
	printf("SENSOR_PLL_CLKO = %dHz\n", SENSOR_PLL_CLKO);
#if TSMPLL_24M_CLOCKIN
	printf("TSM_PLL24_CLKO = %dHz\n", TSM_PLL24_CLKO);
#else
	printf("TSM_PLL27_CLKO = %dHz\n", TSM_PLL27_CLKO);
#endif

	/* Spec. Table 5.1-2 */
	printf("ck860_cclk = %dHz\n", CPU_DEFAULT_FREQ);
	printf("ck860_aclk = %dHz\n", CK_PLL_CLKO / ((crm_readl(CK860_CLK_CFG) & 0x7) + 1));
	if (crm_readl(CK810_CLK_CFG) & 0x10) {
		printf("ck810_cclk = %dHz\n", CK_PLL_CLKO);
		printf("ck810_aclk = %dHz\n", CK_PLL_CLKO / ((crm_readl(CK810_CLK_CFG) & 0x7) + 1));
	} else {
		printf("ck810_cclk = %dHz\n", UNI_PLL_CLKO);
		printf("ck810_aclk = %dHz\n", UNI_PLL_CLKO / ((crm_readl(CK810_CLK_CFG) & 0x7) + 1));
	}
	printf("sys_aclk_h = %dHz\n", SYS_ACLK_H);
	printf("sys_aclk_l = %dHz\n", SYS_ACLK_L);
	printf("sys_hclk = %dHz\n", SYS_HCLK);
	printf("sys_pclk = %dHz\n", SYS_PCLK);
	printf("sd0_cclk = %dHz\n", SDIO0_DEFAULT_FREQ);
	printf("sd1_cclk = %dHz\n", SDIO1_DEFAULT_FREQ);
	printf("hdma_hclk = %dHz\n", SYS_HCLK);
	printf("xdma_aclk = %dHz\n", SYS_PLL_CLKO / ((crm_readl(XDMA_CLK_CFG) & 0xf) + 1));
	if (crm_readl(VIN_CLK_CFG) & 0x10)
		printf("vin_aclk = %dHz\n", PIXEL_PLL_CLKO / ((crm_readl(VIN_CLK_CFG) & 0xf) + 1));
	else
		printf("vin_aclk = %dHz\n", VIDEO_PLL0_CLKO / ((crm_readl(VIN_CLK_CFG) & 0xf) + 1));
	if (crm_readl(VOUT_CLK_CFG) & 0x10)
		printf("dc8000_clk = %dHz\n", PIXEL_PLL_CLKO / ((crm_readl(VOUT_CLK_CFG) & 0xf) + 1));
	else
		printf("dc8000_clk = %dHz\n", VIDEO_PLL0_CLKO / ((crm_readl(VOUT_CLK_CFG) & 0xf) + 1));
	printf("dc_pix_clk = %dHz\n", PIXEL_PLL_CLKO / ((crm_readl(DISPLAY_CLK_CFG) & 0xff) + 1));
	printf("vc8000e_aclk = %dHz\n", VIDEO_PLL0_CLKO / ((crm_readl(VENC_CLK_CFG) & 0xf) + 1));
	printf("jpeg_aclk = %dHz\n", VIDEO_PLL0_CLKO / ((crm_readl(JPEG_CLK_CFG) & 0xf) + 1));
	printf("avs2_clk = %dHz\n", VIDEO_PLL1_CLKO / ((crm_readl(AVS2_CLK_CFG) & 0xf) + 1));
	printf("avsp_clk = %dHz\n", VIDEO_PLL1_CLKO / (((crm_readl(AVS2_CLK_CFG) >> 4) & 0xf) + 1));
	if (crm_readl(VDEC_CLK_CFG) & 0x10)
		printf("vc8000d_aclk = %dHz\n", PIXEL_PLL_CLKO / ((crm_readl(VDEC_CLK_CFG) & 0xf) + 1));
	else
		printf("vc8000d_aclk = %dHz\n", VIDEO_PLL0_CLKO / ((crm_readl(VDEC_CLK_CFG) & 0xf) + 1));
	if (crm_readl(ISP_CLK_CFG) & 0x10)
		printf("isp_clk = %dHz\n", GMAC_PLL_CLKO / ((crm_readl(ISP_CLK_CFG) & 0xf) + 1));
	else
		printf("isp_clk = %dHz\n", VIDEO_PLL0_CLKO / ((crm_readl(ISP_CLK_CFG) & 0xf) + 1));
	if (crm_readl(GPU_CLK_CFG) & 0x10)
		printf("gc620_clk1x = %dHz\n", PIXEL_PLL_CLKO / ((crm_readl(GPU_CLK_CFG) & 0xf) + 1));
	else
		printf("gc620_clk1x = %dHz\n", VIDEO_PLL1_CLKO / ((crm_readl(GPU_CLK_CFG) & 0xf) + 1));

#if TSMPLL_24M_CLOCKIN
	printf("camb_aclk = %dHz\n", TSM_PLL24_CLKO / ((crm_readl(CAMB_CLK_CFG) & 0xf) + 1));
#else
	clk = TSM_PLL27_CLKO / ((crm_readl(TS_CLK_CFG) & 0xf) + 1);
	printf("tsm_clk = %dHz\n", clk);
	clk = clk / (((crm_readl(TS_CLK_CFG) >> 4) & 0xf) + 1);
	printf("tsm_clk27 = %dHz\n", clk);
	if (crm_readl(TS_CLK_CFG) & 0x10000)
		printf("stc_clk27 = %dHz\n", PIXEL_PLL_CLKO / (((crm_readl(TS_CLK_CFG) >> 8) & 0xff) + 1));
	else
		printf("stc_clk27 = %dHz\n", clk);
#endif
	switch ((crm_readl(VIP_CLK_CFG) >> 4) & 0x03) {
	case 0:
		printf("vip_clk = %dHz\n", VIDEO_PLL0_CLKO / ((crm_readl(VIP_CLK_CFG) & 0xf) + 1));
		break;
	case 1:
		// TSM_PLL24_CLKO
		printf("vip_clk = %dHz\n", TSM_PLL27_CLKO / ((crm_readl(VIP_CLK_CFG) & 0xf) + 1));
		break;
	case 2:
		printf("vip_clk = %dHz\n", DSP_PLL_CLKO / ((crm_readl(VIP_CLK_CFG) & 0xf) + 1));
		break;
	default:
		break;
	}

	clk = VIDEO_PLL0_CLKO / ((crm_readl(CDVS_CLK_CFG) & 0xf) + 1);
	printf("cdvs_aclk = %dHz\n", clk);
	printf("cdvs_pclk = %dHz\n", clk / (((crm_readl(CDVS_CLK_CFG) >> 8) & 0xf) + 1));
	printf("sensor0_refclk = %dHz\n", SENSOR_PLL_CLKO / ((crm_readl(SENSOR0_CLK_CFG) & 0xfff) + 1));
	printf("sensor1_refclk = %dHz\n", PIXEL_PLL_CLKO / ((crm_readl(SENSOR1_CLK_CFG) & 0xfff) + 1));

	clk = AUDIO_PLL_CLKO / ((crm_readl(I2S_CLK_CFG) & 0xfff) + 1);
	printf("i2s_mclk = %dHz\n", clk);
	printf("i2s_sclk = %dHz\n", clk / (((crm_readl(I2S_CLK_CFG) >> 12) & 0xfff) + 1));
	//printf("sci_tx_clk = %dHz\n", SYS_PCLK / ((crm_readl(SCI_CLK_CFG) & 0xff) + 1));
	printf("timer_clk = %dHz\n", SYS_PCLK / ((crm_readl(TIMER_CLK_CFG) & 0xffff) + 1));
#endif
}
#endif
