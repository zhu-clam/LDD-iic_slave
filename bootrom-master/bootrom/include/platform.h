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

#ifndef __INCLUDE_PLATFORM_H
#define __INCLUDE_PLATFORM_H

// Different configuration for FPGA and ASIC
#define FPGA_TEST           1

/* Synopsys UART */
#define CK_UART_FIFO_EN     1
#define CK_UART_FC_EN       1

/* C-SKY Interrupt Controller */
#define CK_CK860            0
#define CK_INTC_DEBUG       0
#define CK_128_INTC         0

/* Synopsys Watchdog Timer */
#define CK_WDT_DEBUG        0

/*******************************
 * Configure MMU and cache
 ******************************/
#define CONFIG_CKCPU_MMU            0
#define CONFIG_CKCPU_ICACHE         0
#define CONFIG_CKCPU_DCACHE         0
#define CONFIG_CKCPU_L2CACHE        0
#if CONFIG_CKCPU_MMU
#define PERI_BASE   (-0x40000000)
#else
#define PERI_BASE   (0x00000000)
#endif /* CONFIG_CKCPU_MMU */

/**************************************
 * MCU & Boards.
 *************************************/

/* PLL input clock(crystal frequency) */
#define CONFIG_PLL_INPUT_CLK   12000000   /* HZ */
/* CPU frequency definition */
#define CPU_DEFAULT_FREQ       24000000  /* Hz */
/* AHB frequency definition */
#define AHB_DEFAULT_FREQ       24000000   /* Hz */

/* APB frequency definition */
#define APB_DEFAULT_FREQ       24000000
#define TIMER_DEFAULT_FREQ     24000000
/***************************** AHB ****************************/
/**** SDIO ****/
#define CK_SDIO0_BASEADDRESS        (0xF6100000 + PERI_BASE)
#define CK_SDIO1_BASEADDRESS        (0xF6200000 + PERI_BASE)
/***************************** NOC ****************************/
#define QSPI_BASE                   (0xF4700000 + PERI_BASE)
/***************************** APB0 ****************************/
/***** SPI ******/
#define SPI0_BASE   (volatile CK_UINT32 *)(0xF7000000 + PERI_BASE)
#define SPI1_BASE   (volatile CK_UINT32 *)(0xF7001000 + PERI_BASE)
/***** I2S ******/
#define I2S0_BASE   (volatile CK_UINT32 *)(0xF7002000 + PERI_BASE)
#define I2S1_BASE   (volatile CK_UINT32 *)(0xF7003000 + PERI_BASE)
#define I2S2_BASE   (volatile CK_UINT32 *)(0xF7004000 + PERI_BASE)
#define I2S3_BASE   (volatile CK_UINT32 *)(0xF7005000 + PERI_BASE)
#define I2S4_BASE   (volatile CK_UINT32 *)(0xF7006000 + PERI_BASE)
/***** I2C ******/
#define I2C0_BASE   (volatile CK_UINT32 *)(0xF7007000 + PERI_BASE)
#define I2C1_BASE   (volatile CK_UINT32 *)(0xF7008000 + PERI_BASE)
#define I2C2_BASE   (volatile CK_UINT32 *)(0xF7009000 + PERI_BASE)
#define I2C3_BASE   (volatile CK_UINT32 *)(0xF700A000 + PERI_BASE)
/****** WDT ******/
#define CK_WDT_CK860_ADDR           (volatile CK_UINT32 *)(0xF700B000 + PERI_BASE)
#define CK_WDT_CK810_ADDR           (volatile CK_UINT32 *)(0xF700C000 + PERI_BASE)
#define CK_WDT_UNI_ADDR             (volatile CK_UINT32 *)(0xF700D000 + PERI_BASE)

#define APTS_BASE        (volatile CK_UINT32 *)(0xF700E000 + PERI_BASE)
#define I2S_PYS_BASE     (volatile CK_UINT32 *)(0xF700F000 + PERI_BASE)
/************************* END OF APB0 ************************/

/***************************** APB1 ****************************/
/***** SPI ******/
#define SPI2_BASE   (volatile CK_UINT32 *)(0xF8000000 + PERI_BASE)
#define SPI3_BASE   (volatile CK_UINT32 *)(0xF8001000 + PERI_BASE)
/***** INTC ******/
#if CK_CK860
#define CK_INTC_BASEADDRESS         (0xF8002000 + PERI_BASE)
#else
#define CK_INTC_BASEADDRESS         (0xF8003000 + PERI_BASE)  
#endif
/**** Timer ****/
#define CK_TIMER0_BASSADDR          (0xF8005000 + PERI_BASE)
#define CK_TIMER1_BASSADDR          (volatile CK_UINT32 *)(0xF8005014 + PERI_BASE)
#define CK_TIMER2_BASSADDR          (volatile CK_UINT32 *)(0xF8005028 + PERI_BASE)
#define CK_TIMER3_BASSADDR          (volatile CK_UINT32 *)(0xF800503C + PERI_BASE)
#define CK_TIMER4_BASSADDR          (volatile CK_UINT32 *)(0xF8005050 + PERI_BASE)
#define CK_TIMER5_BASSADDR          (volatile CK_UINT32 *)(0xF8005064 + PERI_BASE)
#define CK_TIMER6_BASSADDR          (volatile CK_UINT32 *)(0xF8005078 + PERI_BASE)
#define CK_TIMER7_BASSADDR          (volatile CK_UINT32 *)(0xF800508C + PERI_BASE)
#define TIMER_BASE_0                CK_TIMER0_BASSADDR
#define CONFIG_SYS_HZ  CPU_DEFAULT_FREQ
/***** Uart *******/
#define CK_UART_ADDRBASE0           (volatile CK_UINT32 *)(0xF8006000 + PERI_BASE)
#define CK_UART_ADDRBASE1           (volatile CK_UINT32 *)(0xF8007000 + PERI_BASE)
#define CK_UART_ADDRBASE2           (volatile CK_UINT32 *)(0xF8008000 + PERI_BASE)
#define CK_UART_ADDRBASE3           (volatile CK_UINT32 *)(0xF8009000 + PERI_BASE)
#define CK_UART_ADDRBASE4           (volatile CK_UINT32 *)(0xF800A000 + PERI_BASE)
#define CK_UART_ADDR0               (0xF8006000 + PERI_BASE)

#define SCI7816_BASE                (volatile CK_UINT32 *)(0xF800B000 + PERI_BASE)
#define GPIO_BASE                   (volatile CK_UINT32 *)(0xF800C000 + PERI_BASE)
#define PWM_BASE                    (volatile CK_UINT32 *)(0xF800D000 + PERI_BASE)
#define RTC_BASE                    (volatile CK_UINT32 *)(0xF800E000 + PERI_BASE)
#define STC_BASE                    (volatile CK_UINT32 *)(0xF800F000 + PERI_BASE)
/************************* END OF APB1 ************************/

#define SYS_CTL_BASE                0xF9701000

#define NFC_BASE                    0xF4100000


/** APB2 **/

/***** MISC *****/
#define CK_PINMUX_Control            (0xF9703000 + PERI_BASE)

/*
 * define irq number of peripheral modules
 */
#define  CK_INTC_WDT_CK860      44
#define  CK_INTC_WDT_CK810      45
#define  CK_INTC_WDT_UNI        46
#define  CK_INTC_UART0          0
#define  CK_INTC_UART1          48
#define  CK_INTC_UART2          49
#define  CK_INTC_UART3          50
#define  CK_INTC_UART4          51
#define  CK_INTC_TIM0           1
#define  CK_INTC_TIM1           2
#define  CK_INTC_TIM2           56
#define  CK_INTC_TIM3           57
#define  CK_INTC_TIM4           58
#define  CK_INTC_TIM5           59
#define  CK_INTC_TIM6           60
#define  CK_INTC_TIM7           61

#define PHYSICAL_ADDRESS(x)         (volatile CK_UINT32 *)((CK_UINT32)x - PERI_BASE)

#define UART_DEBUG
#ifdef UART_DEBUG
#define debug(format, ...)  printf(format,##__VA_ARGS__)
#else
#define debug(format, ...)  do {} while (0)
#endif

#define INFO_DEG
#ifdef INFO_DEG
#define info_debug  debug
#else
#define info_debug(format, ...)  do {} while (0)
#endif

/* #define CONFIG_DEBUG */

#endif /* __INCLUDE_PLATFORM_H */
