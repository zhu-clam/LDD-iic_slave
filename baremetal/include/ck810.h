/*
 * Description: ck5a6.h - Define the system configuration, memory & IO base
 * address, flash size & address, interrupt resource for ck5a6 soc.
 *
 * Copyright (C) : 2008 Hangzhou C-SKY Microsystems Co.,LTD.
 * Author(s): Liu Bing (bing_liu@c-sky.com)
 * Contributors: Liu Bing
 * Date:  2010-06-26
 * Modify by liu jirang  on 2012-09-11
 */

#ifndef __INCLUDE_CK810_H
#define __INCLUDE_CK810_H

#include "crm.h"

#define CONFIG_IS_ASIC			1

#define CONFIG_DDR4_2400_SUPPORT	1
//#define CONFIG_DDR4_2133_SUPPORT	1
//#define CONFIG_DDR4_1600_SUPPORT	1
//#define CONFIG_DDR4_800_SUPPORT	1
//#define CONFIG_DDR3_1600_SUPPORT	1
//#define CONFIG_DDR3_800_SUPPORT	1

#define CONFIG_NR_DDR_CHANNELS		4
#define CONFIG_CHANNEL_IN_LOW_POWER

#define CONFIG_DUMP_VSI_PLL_REGS	1
#define CONFIG_DUMP_VSI_CLK_FREQ	1

/* TSM PLL reference clock select,
 * 0: 24MHz OSC, for CAMB
 * 1: external 27MHz clock, for TSM
 */
#define TSMPLL_24M_CLOCKIN		0

/* Whether power off the domain, don't release and clock gating if power off */
#define DOMAIN_CK810_POWER_OFF	0
#define DOMAIN_ISP_POWER_OFF		0
#define DOMAIN_GC620_POWER_OFF	0
#define DOMAIN_AVS_POWER_OFF		0
#define DOMAIN_VC8000E_POWER_OFF	0
#define DOMAIN_AI_POWER_OFF		0

/* Whether do IP release after bring-up */
#define IP_RELEASE_RSCODEC		0
#define IP_RELEASE_TSPSI		0
#define IP_RELEASE_AVS610		0
#define IP_RELEASE_AVSP			0
#define IP_RELEASE_AVS2			0

#define IP_RELEASE_C50_CORE		0
#define IP_RELEASE_C51_CORE		0
#define IP_RELEASE_P60_CORE		0
#define IP_RELEASE_P61_CORE		0
#define IP_RELEASE_CAMB		0
#define IP_RELEASE_CDVS			0
#define IP_RELEASE_C50_CSR		0
#define IP_RELEASE_C51_CSR		0
#define IP_RELEASE_P60_CSR		0
#define IP_RELEASE_P61_CSR		0
#define IP_RELEASE_WDT_C50		0
#define IP_RELEASE_WDT_C51		0
#define IP_RELEASE_WDT_P60		0
#define IP_RELEASE_WDT_P61		0
#define IP_RELEASE_TIMER_DSP		0
#define IP_RELEASE_UART0_DSP		0
#define IP_RELEASE_UART1_DSP		0
#define IP_RELEASE_UART2_DSP		0
#define IP_RELEASE_UART3_DSP		0

#define pmu_writel(val, offs) write_mreg32(CK_PMU_ADDR + offs, val)
#define pmu_readl(offs) read_mreg32(CK_PMU_ADDR + offs)
#define crm_writel(val, offs) write_mreg32(CK_CRM_ADDR + offs, val)
#define crm_readl(offs) read_mreg32(CK_CRM_ADDR + offs)
#define set_pll_param(ref, fb, post1, post2, reg) crm_writel((ref|(fb<<8)|(post1<<20)|(post2<<24)), reg)
#define PLL_CLKO(ref, param) ((((ref)*((crm_readl(param)>>8)&0xfff))/((crm_readl(param)&0x3f)*((crm_readl(param)>>20)&0x7)*((crm_readl(param)>>24)&0x7)))*1000000)

#if CONFIG_IS_ASIC
#define SYS_PLL_CLKO		PLL_CLKO(24, SYS_PLL_PARAM)
#define CK_PLL_CLKO		PLL_CLKO(24, CK_PLL_PARAM)
#define UNI_PLL_CLKO		PLL_CLKO(24, UNI_PLL_PARAM)
#define DDR_PLL_CLKO		PLL_CLKO(24, DDR_PLL_PARAM)
#define DSP_PLL_CLKO		PLL_CLKO(24, DSP_PLL_PARAM)
#define VIDEO_PLL0_CLKO		PLL_CLKO(24, VIDEO0_PLL_PARAM)
#define VIDEO_PLL1_CLKO		PLL_CLKO(24, VIDEO1_PLL_PARAM)
#define TSM_PLL24_CLKO		PLL_CLKO(24, TSM_PLL_PARAM)
#define TSM_PLL27_CLKO		PLL_CLKO(27, TSM_PLL_PARAM)
#define GMAC_PLL_CLKO		PLL_CLKO(24, GMAC_PLL_PARAM)
#define PIXEL_PLL_CLKO		PLL_CLKO(24, PIXEL_PLL_PARAM)
#define AUDIO_PLL_CLKO		PLL_CLKO(24, AUDIO_PLL_PARAM)
#define SENSOR_PLL_CLKO	PLL_CLKO(24, SENSOR_PLL_PARAM)

#define SYS_ACLK_H		(SYS_PLL_CLKO / ((crm_readl(SYS_CLK_CFG) & 0x7) + 1))
#define SYS_ACLK_L		(SYS_ACLK_H / (((crm_readl(SYS_CLK_CFG) >> 4) & 0x7) + 1))
#define SYS_HCLK		(SYS_ACLK_L / (((crm_readl(SYS_CLK_CFG) >> 8) & 0x7) + 1))
#define SYS_PCLK		(SYS_ACLK_L / (((crm_readl(SYS_CLK_CFG) >> 12) & 0x7) + 1))

#define REF_CLK_FREQ		24000000
#define CPU_DEFAULT_FREQ	CK_PLL_CLKO
/* if we update PLL source select, need to modify SDIO*_DEFAULT_FREQ also */
#define SDIO0_DEFAULT_FREQ	(SYS_PLL_CLKO / ((crm_readl(SDIO_CLK_CFG) & 0x3f) + 1))
#define SDIO1_DEFAULT_FREQ	(SYS_PLL_CLKO / (((crm_readl(SDIO_CLK_CFG) >> 8) & 0x3f) + 1))
#define TIMER_DEFAULT_FREQ	(SYS_PCLK / ((crm_readl(TIMER_CLK_CFG) & 0xffff) + 1))
#define UART_DEFAULT_FREQ	REF_CLK_FREQ
#define PWM_PCLK_FREQ		SYS_PCLK
#define PWM_SCLK_FREQ		24000000
#define I2C_CLK_FREQ		SYS_PCLK
#define APTS_CLK27_FREQ	((TSM_PLL27_CLKO / ((crm_readl(TS_CLK_CFG) & 0xf) + 1)) / (((crm_readl(TS_CLK_CFG) >> 4) & 0xf) + 1))
#define SPI_DEFAULT_FREQ	SYS_PCLK
#define QSPI_DEFAULT_FREQ	SYS_HCLK
#else
#define TIMER_DEFAULT_FREQ	30000000
#define UART_DEFAULT_FREQ	30000000
#define PWM_PCLK_FREQ		30000000
#define PWM_SCLK_FREQ		(PWM_PCLK_FREQ >> 1)
#define I2C_CLK_FREQ		30000000
#define APTS_CLK27_FREQ	30000000
#define SPI_DEFAULT_FREQ	30000000
#define QSPI_DEFAULT_FREQ	30000000
#endif  /* CONFIG_IS_ASIC */

/* Synopsys UART */
#define CK_UART_FIFO_EN	1
#define CK_UART_FC_EN		0

//#define CK_CK860            0
//#define CK_CK860            1
/* C-SKY Interrupt Controller */
#define CK_INTC_DEBUG		0
#define CK_128_INTC		1

/* Synopsys Watchdog Timer */
#define CK_WDT_DEBUG		0

/* Synopsys SDIO */
#define CK_SDIO_DEBUG		0

/* Synopsys AHB DMA Controller */
//#define AHB_DMAC_DEBUG      1

/* VSI SPI controller with GigaDevice GD25Q128 NOR flash */
#define CK_SPI_M_NOR_DEBUG	0

/*******************************
 * Configure MMU and cache
 ******************************/
#define CONFIG_CKCPU_MMU	1
#define CONFIG_CKCPU_ICACHE	1
#define CONFIG_CKCPU_DCACHE	1

#if CONFIG_CKCPU_MMU
#define PERI_BASE		(-0x40000000)
#else
#define PERI_BASE		(0x00000000)
#endif /* CONFIG_CKCPU_MMU */

/***************************** NOC ****************************/
#define PCIE_BASE_ADDR		(0xf1000000 + PERI_BASE)
#define CK_CAMB_BASE		(0xf3000000 + PERI_BASE)
#define CK_VIP_BASE		(0xf3400000 + PERI_BASE)
#define CK_CDVS_BASE		(0xf3500000 + PERI_BASE)
#define OTP_BASE_ADDR		(0xf4000000 + PERI_BASE)
#define NFC_BASE_ADDR		(0xf4100000 + PERI_BASE)
#define SPACC_BASE_ADDR	(0xf4500000 + PERI_BASE)
#define RSA_BASE_ADDR		(0xf4600000 + PERI_BASE)
#define QSPI_BASE		(0xf4700000 + PERI_BASE)
#define CK_DSP_C5_0_BASE	(0xf5000000 + PERI_BASE)
#define CK_DSP_P6_0_BASE	(0xf5200000 + PERI_BASE)
/************************* END OF NOC ************************/

/***************************** AHB ****************************/
#define CK_SDIO0_BASEADDRESS	(0xf6100000 + PERI_BASE)
#define CK_SDIO1_BASEADDRESS	(0xf6200000 + PERI_BASE)
#define CK_AHB_DMA_CONTROL(x)	(0xf6300000 + (x * 0x100000) + PERI_BASE)
#define CK_AXI_DMA_ADDRESS	(0xf6500000 + PERI_BASE)
#define CK_ISP0_Slave		(0xf6600000 + PERI_BASE)
#define CK_ISP1_Slave		(0xf6700000 + PERI_BASE)
#define CK_AVS2_BASE		(0xf6800000 + PERI_BASE)
#define CK_GC620_BASE		(0xf6e00000 + PERI_BASE)
/************************* END OF AHB ************************/

/***************************** APB0 ****************************/
#define SPI0_BASE		(volatile CK_UINT32 *)(0xf7000000 + PERI_BASE)
#define SPI1_BASE		(volatile CK_UINT32 *)(0xf7001000 + PERI_BASE)
#define I2S0_BASE		(volatile CK_UINT32 *)(0xf7002000 + PERI_BASE)
#define I2S1_BASE		(volatile CK_UINT32 *)(0xf7003000 + PERI_BASE)
#define I2S2_BASE		(volatile CK_UINT32 *)(0xf7004000 + PERI_BASE)
#define I2S3_BASE		(volatile CK_UINT32 *)(0xf7005000 + PERI_BASE)
#define I2S4_BASE		(volatile CK_UINT32 *)(0xf7006000 + PERI_BASE) // I2S_M
#define CK_I2C0_BASSADDR	(volatile CK_UINT32 *)(0xf7007000 + PERI_BASE)
#define CK_I2C1_BASSADDR	(volatile CK_UINT32 *)(0xf7008000 + PERI_BASE)
#define CK_I2C2_BASSADDR	(volatile CK_UINT32 *)(0xf7009000 + PERI_BASE)
#define CK_I2C3_BASSADDR	(volatile CK_UINT32 *)(0xf700a000 + PERI_BASE)
#define CK_WDT_CK860_ADDR	(volatile CK_UINT32 *)(0xf700b000 + PERI_BASE)
#define CK_WDT_CK810_ADDR	(volatile CK_UINT32 *)(0xf700c000 + PERI_BASE)
#define CK_WDT_UNI_ADDR	(volatile CK_UINT32 *)(0xf700d000 + PERI_BASE)
#define APTS_BASE		(0xf700e000 + PERI_BASE)
#define I2S_PTS_BASE		(0xf700f000 + PERI_BASE)
/************************* END OF APB0 ************************/

/***************************** APB1 ****************************/
#define SPI2_BASE		(0xf8000000 + PERI_BASE)
#define SPI3_BASE		(0xf8001000 + PERI_BASE)
#define INTC_CK860_BASE		(volatile CK_UINT32 *)(0xf8002000 + PERI_BASE)
#define INTC_CK810_BASE		(volatile CK_UINT32 *)(0xf8003000 + PERI_BASE)
#define INTC_UNI_BASE		(volatile CK_UINT32 *)(0xf8004000 + PERI_BASE)
#if CK_CK860
#define CK_INTC_BASEADDRESS	(0xf8002000 + PERI_BASE)
#else
#define CK_INTC_BASEADDRESS	(0xf8003000 + PERI_BASE)
#endif
#define CK_TIMER0_BASSADDR	(volatile CK_UINT32 *)(0xf8005000 + PERI_BASE)
#define CK_TIMER1_BASSADDR	(volatile CK_UINT32 *)(0xf8005014 + PERI_BASE)
#define CK_TIMER2_BASSADDR	(volatile CK_UINT32 *)(0xf8005028 + PERI_BASE)
#define CK_TIMER3_BASSADDR	(volatile CK_UINT32 *)(0xf800503c + PERI_BASE)
#define CK_TIMER4_BASSADDR	(volatile CK_UINT32 *)(0xf8005050 + PERI_BASE)
#define CK_TIMER5_BASSADDR	(volatile CK_UINT32 *)(0xf8005064 + PERI_BASE)
#define CK_TIMER6_BASSADDR	(volatile CK_UINT32 *)(0xf8005078 + PERI_BASE)
#define CK_TIMER7_BASSADDR	(volatile CK_UINT32 *)(0xf800508c + PERI_BASE)
#define CK_UART_ADDRBASE0	(volatile CK_UINT32 *)(0xf8006000 + PERI_BASE)
#define CK_UART_ADDRBASE1	(volatile CK_UINT32 *)(0xf8007000 + PERI_BASE)
#define CK_UART_ADDRBASE2	(volatile CK_UINT32 *)(0xf8008000 + PERI_BASE)
#define CK_UART_ADDRBASE3	(volatile CK_UINT32 *)(0xf8009000 + PERI_BASE)
#define CK_UART_ADDRBASE4	(volatile CK_UINT32 *)(0xf800a000 + PERI_BASE)
#define SCI7816_ADDR		(0xf800b000 + PERI_BASE)
#define CK_GPIO_ADDR		(0xf800c000 + PERI_BASE)
#define PWM_BASE(x)		(volatile CK_UINT32 *)(0xf800d000 + (x * 0x200) + PERI_BASE)
#define CK_RTC_ADDR		(0xf800e000 + PERI_BASE)
#define CK_STC_ADDR		(0xf800f000 + PERI_BASE)
/************************* END OF APB1 ************************/

/***************************** APB2 ****************************/
#define CK_CRM_ADDR		(0xf9700000 + PERI_BASE)
#define CK_SYS_CTRL_ADDR	(0xf9701000 + PERI_BASE)
#define CK_PMU_ADDR		(0xf9702000 + PERI_BASE)
#define CK_PINMUX_Control	(0xf9703000 + PERI_BASE)
/************************* END OF APB2 ************************/

/***************************** APB3 ****************************/
#define CK_MIPI_Slave		(0xfa004000 + PERI_BASE) //(0xFA004000 + PERI_BASE)////(0xFA003400 + PERI_BASE)
#define CK_VC8000E_BASE	(0xfa000000 + PERI_BASE)
#define CK_VC8000D_BASE	(0xfa002000 + PERI_BASE)
/************************* END OF APB3 ************************/

/*
 * define IRQ number of peripheral modules
 */
#define CK_INTC_QSPI		2
#define CK_INTC_SPACC		6
#define CK_INTC_RSA		7
#define CK_INTC_AXIDMA		8
#define CK_INTC_AHBDMA(x)	(9 + x)
#define CK_INTC_SDIO0		11
#define CK_INTC_SDIO0_WAKEUP	12
#define CK_INTC_SDIO1		13
#define CK_INTC_SDIO1_WAKEUP	14
#define CK_INTC_MIPI		30
#define CK_INTC_SPI0		31
#define CK_INTC_SPI1		32
#define CK_INTC_SPI2		33
#define CK_INTC_SPI3		34
#define CK_INTC_I2C0		40
#define CK_INTC_I2C1		41
#define CK_INTC_I2C2		42
#define CK_INTC_I2C3		43
#define CK_INTC_WDT_CK860	44
#define CK_INTC_WDT_CK810	45
#define CK_INTC_WDT_UNI	46
#define CK_INTC_UART0		47
#define CK_INTC_UART1		48
#define CK_INTC_UART2		49
#define CK_INTC_UART3		50
#define CK_INTC_UART4		51
#define CK_INTC_SCI7816		52
#define CK_INTC_GPIO		53
#define CK_INTC_TIM0		54
#define CK_INTC_TIM1		55
#define CK_INTC_TIM2		56
#define CK_INTC_TIM3		57
#define CK_INTC_TIM4		58
#define CK_INTC_TIM5		59
#define CK_INTC_TIM6		60
#define CK_INTC_TIM7		61
#define CK_INTC_PWM(x)		(62 + x)
#define CK_INTC_RTC		70
#define CK_INTC_APTS		71
#define CK_INTC_I2S_PTS		72

#define PHYSICAL_ADDRESS(x)	(volatile CK_UINT32 *)((CK_UINT32)x - PERI_BASE)

#endif /* __INCLUDE_CK810_H */
