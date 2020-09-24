/*
 * main.c - main function Modulation.
 *
 * Copyright (C):
 * Author:
 * Contributor:
 * Date:
 *
 */

#include "datatype.h"
#include "ck810.h"
#include "uart.h"
#include "misc.h"
#include "intc.h"
#include "timer.h"
#include "rtc.h"
#include "pmu.h"

CK_Uart_Device consoleuart = CONFIG_TERMINAL_UART;

extern void CK_Console_Init(void);
extern void CK_Exception_Init(void);
extern void CK_INTC_Init(IN CK_UINT32 mode);
extern void CK_UART_Test(void);
extern void CK_INTC_Test(void);
extern void CK_Timer_Test(void);
extern void CK_Watchdog_Test(void);
extern void apb_access_test(void);
extern void SPI_Master_GD25Q128_APP(CK_UINT8 PortN);
extern void SPI_Master_w25n01_App();
extern void CK_SDIO_Test(void);
extern void CK_AHBDMA_Test(CK_UINT32 id);
extern void init_time0(void);
extern void CK_Gpio_Test(void);
extern void CK_RSA_Test(void);
extern void CK_RSA_Reg_RW_Test(void);
extern void CK_SPACC_Reg_RW_Test(void);
extern void CK_SPACC_Test(void);
extern void CK_I2C_Test(CK_UINT8 i2c_id);
extern void CK_CPU_L2_Test(void);
extern void CK_STC_Test(void);
extern void CK_OTP_Test(void);
extern void CK_I2S_PTS_Test(void);
extern void CK_SCI7816_Test(void);
extern void CK_APTS_Test(void);
extern void CK_nfc_test(void);
extern void CK_PWM_test(void);
extern void CK_CPU_Boot_Test(void);
extern void CK_AXIDMA_Test(void);
extern void Audio_Test(void);
extern void ip_clk_gating_test(void);
extern void domain_power_test(void);
extern void sdram_init(void);
extern void pll_init(void);
extern void dump_pll_regs(void);
extern void dump_clk_freq(void);
extern void ISP_test(void);
extern void pin_mux_test(void);
extern void DDR_test(void);
extern void CK_DDR_Interleave_Test(void);
extern void CK_MIPI_Test(void);
extern void CK_I2C_Test(CK_UINT8 i2c_id);
extern void CK_I2C_Slave_Test(void);

/*
 * Initialize the device registered
 */
static void CK_Drivers_Init(void)
{
    CK_Uart_DriverInit();
#ifndef CONFIG_INTC_DIS
    CK_INTC_Init(AUTO_MODE);
#endif
}

/*
 * The main function of bare-metal tests
 */
int main ( void )
{
    unsigned char test_id;

#if CONFIG_IS_ASIC
//    pll_init();
#endif

    CK_Drivers_Init();
#ifndef CONFIG_INTC_DIS
    CK_Exception_Init();
#endif
    CK_Console_Init();
    init_time0();

#if CONFIG_IS_ASIC
    // dump_pll_regs();
    // dump_clk_freq();
#endif
    while (1) {
        printf ("\nBare-metal test begin ...\n");
        printf ("build time %s %s\n", __DATE__, __TIME__);
        while (1) {
            printf("\nplease input test module ID:\n");
            printf("0 -- Synopsys UART\n");
            printf("1 -- C-SKY Interrupt Controller\n");
            printf("2 -- Synopsys AHB DMA Controller\n");
            printf("3 -- Synopsys GPIO Controller\n");
            printf("4 -- DesignWare DW_apb_timers\n");
            printf("5 -- Synopsys Watchdog Timer\n");
            printf("6 -- Synopsys I2C\n");
            printf("7 -- VSI STC\n");
            printf("8 -- VSI RTC\n");
            printf("9 -- VSI SPI controller with GD25Q128 NOR flash\n");
            printf("a -- VSI SPI controller with w25n01 NAND flash\n");
            printf("b -- Synopsys SDIO Controller\n");
            printf("c -- VSI I2S PTS\n");
            printf("d -- DDR init\n");
            printf("e -- VSI SCI7816\n");
            printf("f -- MIPI\n");
            printf("g -- VSI APTS\n");
            printf("h -- DDR Interleave\n");
            printf("i -- VSI OTP controller\n");
            printf("j -- CK860 L2 Cache\n");
            printf("k -- DDR test\n");
            printf("l -- Sub-system Test 2\n");
            printf("m -- apb_access_test\n");
            printf("n -- PWM Test\n");
            printf("o -- RSA Test\n");
            printf("p -- SPACC Test\n");
            printf("q -- NFC Test\n");
            printf("r -- Slave CPU Boot Test\n");
            printf("s -- Synopsys AXI-DMA Controller\n");
            printf("t -- Audio Test\n");
            printf("u -- Domain Power Test\n");
            printf("v -- Clock Gating Test\n");
            printf("w -- ISP Test SDI Test\n");
            printf("x -- Pin MUX Test\n");
            printf(">");
            test_id = getchar();
            putchar(test_id);
            if (test_id >= '0' && test_id <= 'x') {
                break;
            }
        }
        printf ("\n");

        switch(test_id) {
        case '0':
            CK_UART_Test();
            break;
        case '1':
            CK_INTC_Test();
            break;
        case '2':
            CK_AHBDMA_Test(0);
            CK_AHBDMA_Test(1);
            break;
        case '3':
            CK_Gpio_Test();
            break;
        case '4':
            CK_Timer_Test();
            break;
        case '5':
            CK_Watchdog_Test();
            break;
        case '6':
            //CK_I2C_Test(3);
            //CK_I2C_Test(2);
            CK_I2C_Slave_Test();
            break;
        case '7':
            CK_STC_Test();
            break;
        case '8':
            CK_Rtc_Test();
            break;
        case '9':
            SPI_Master_GD25Q128_APP(0);
            break;
        case 'a':
            SPI_Master_w25n01_App();
            break;
        case 'b':
            CK_SDIO_Test();
            break;
        case 'c':
            CK_I2S_PTS_Test();
            break;
        case 'd':
            sdram_init();
            break;
        case 'e':
            CK_SCI7816_Test();
            break;
        case 'f':
            CK_MIPI_Test();
            break;
        case 'g':
            CK_APTS_Test();
            break;
        case 'h':
            printf("DDR Interleave case need in SRAM and Disable MMU, L1 & L2 cache\n");
            CK_DDR_Interleave_Test();
            break;
        case 'i':
            CK_OTP_Test();
            break;
        case 'j':
            CK_CPU_L2_Test();
            break;
        case 'k':
            DDR_test();
            break;
        case 'l':
            //CK_SUB2_Test();
            break;
        case 'm':
            apb_access_test();
            break;
        case 'n':
            CK_PWM_test();
            break;
        case 'o':
            //CK_RSA_Reg_RW_Test();
            CK_RSA_Test();
            break;
        case 'p':
            CK_SPACC_Reg_RW_Test();
            CK_SPACC_Test();
            break;
        case 'q':
            CK_nfc_test();
            break;
        case 'r':
            CK_CPU_Boot_Test();
            break;
        case 's':
            CK_AXIDMA_Test();
            break;
        case 't':
            Audio_Test();
            break;
        case 'u':
            domain_power_test();
            break;
        case 'v':
            ip_clk_gating_test();
            break;
        case 'x':
            pin_mux_test();
            break;
        default:
            printf("unsupported test module\n");
            break;
        }

        printf("\nBare-metal test end\n");
    }

    return 0;
}
