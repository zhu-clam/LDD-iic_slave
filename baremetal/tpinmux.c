/*
 * [Description] Pin MUX test
 *
 * Maintainer: jwen <jianxian.wen@verisilicon.com>
 *
 * Copyright (C) 2019 Verisilicon Inc.
 *
 */

#include "ck810.h"
#include "misc.h"
#include "gpio.h"
#include "pinmux.h"

static void do_pinmux_test(u32 id) {
    unsigned int gpio0, gpio1;
    unsigned int gpio0_old, gpio1_old;
    int passed = 0;
    int pmux_offset1, pmux_offset2;
    int pmux_gpio1, pmux_gpio2;
    int pmux_port1, pmux_port2;

    printf ("\n");
    switch (id) {
    case 0:
        printf ("\nPWM pin MUX test. . . \n\n");
        printf("please connect:\n"
                "\t\tEVB board J14 PIN5 (PWM4, GPIOC31) <--> EVB Board J13 PIN1 (GPIOA16), \n"
                "\t\tEVB board J14 PIN7 (PWM6, GPIOD30) <--> EVB Board J13 PIN3 (GPIOA17)\n");
        pmux_offset1 = POLARIS_PMUX_PWM4;
        pmux_offset2 = POLARIS_PMUX_PWM6;
        pmux_gpio1 = 31;
        pmux_gpio2 = 30;
        pmux_port1 = GPIO_C;
        pmux_port2 = GPIO_D;
        break;
    case 1:
        printf ("\nAPTS pin MUX test. . . \n\n");
        printf("\tplease connect:\n"
                "\t\tEVB board J9 PIN2 (APTS CLK, GPIOC24) <--> EVB Board J13 PIN1 (GPIOA16), \n"
                "\t\tEVB board J9 PIN4 (APTS MOSI, GPIOC25) <--> EVB Board J13 PIN3 (GPIOA17)\n");
        pmux_offset1 = POLARIS_PMUX_APTS_SPI_CLK;
        pmux_offset2 = POLARIS_PMUX_APTS_SPI_MOSI;
        pmux_gpio1 = 24;
        pmux_gpio2 = 25;
        pmux_port1 = GPIO_C;
        pmux_port2 = GPIO_C;
        break;
    case 2:
        printf ("\nTS-PSI pin MUX test. . . \n\n");
        printf("\tplease connect:\n"
                "\t\tEVB board J15 PIN7 (TS_PSI_D0, GPIOC15) <--> EVB Board J13 PIN1 (GPIOA16), \n"
                "\t\tEVB board J15 PIN8 (TS_PSI_D1, GPIOC16) <--> EVB Board J13 PIN3 (GPIOA17)\n");
        pmux_offset1 = POLARIS_PMUX_TS_PSI_DATA0;
        pmux_offset2 = POLARIS_PMUX_TS_PSI_DATA1;
        pmux_gpio1 = 15;
        pmux_gpio2 = 16;
        pmux_port1 = GPIO_C;
        pmux_port2 = GPIO_C;
        break;
    case 3:
        printf ("\nSCI7816 pin MUX test. . . \n\n");
        printf("\tplease connect:\n"
                "\t\tEVB board J25 PIN2 (SCI7816_CLK, GPIOC9) <--> EVB Board J13 PIN1 (GPIOA16), \n"
                "\t\tEVB board J25 PIN3 (SCI7816_IO, GPIOC11) <--> EVB Board J13 PIN3 (GPIOA17)\n");
        pmux_offset1 = POLARIS_PMUX_SCI7816_CLK;
        pmux_offset2 = POLARIS_PMUX_SCI7816_IO;
        pmux_gpio1 = 9;
        pmux_gpio2 = 11;
        pmux_port1 = GPIO_C;
        pmux_port2 = GPIO_C;
        break;
    case 4:
        printf ("\nUART pin MUX test. . . \n\n");
        printf("\tplease connect:\n"
                "\t\tEVB board J16 PIN3 (DSP_UART0_RXD, GPIOC3) <--> EVB Board J13 PIN1 (GPIOA16), \n"
                "\t\tEVB board J16 PIN2 (DSP_UART0_TXD, GPIOC4) <--> EVB Board J13 PIN3 (GPIOA17)\n");
        pmux_offset1 = POLARIS_PMUX_DSP_UART0_RXD;
        pmux_offset2 = POLARIS_PMUX_DSP_UART0_TXD;
        pmux_gpio1 = 3;
        pmux_gpio2 = 4;
        pmux_port1 = GPIO_C;
        pmux_port2 = GPIO_C;
        break;
    case 5:
        printf ("\nNFC pin MUX test. . . \n\n");
        printf("\tplease connect:\n"
                "\t\tEVB board TP57 (NFC_CLE, GPIOD4) <--> EVB Board J13 PIN1 (GPIOA16), \n"
                "\t\tEVB board TP58 (NFC_ALE, GPIOD5) <--> EVB Board J13 PIN3 (GPIOA17)\n");
        pmux_offset1 = POLARIS_PMUX_NFC_CLE;
        pmux_offset2 = POLARIS_PMUX_NFC_ALE;
        pmux_gpio1 = 4;
        pmux_gpio2 = 5;
        pmux_port1 = GPIO_D;
        pmux_port2 = GPIO_D;
        break;
    case 6:
        printf ("\nQSPI pin MUX test. . . \n\n");
        printf("\tplease connect:\n"
                "\t\tEVB board TP49 (SFC_CLK, GPIOD23) <--> EVB Board J13 PIN1 (GPIOA16), \n"
                "\t\tEVB board TP51 (SFC_SIO_IO0, GPIOD25) <--> EVB Board J13 PIN3 (GPIOA17)\n");
        pmux_offset1 = POLARIS_PMUX_SFC_CLK;
        pmux_offset2 = POLARIS_PMUX_SFC_SIO_IO0;
        pmux_gpio1 = 23;
        pmux_gpio2 = 25;
        pmux_port1 = GPIO_D;
        pmux_port2 = GPIO_D;
        break;
    case 7:
        printf ("\nJTAG pin MUX test (boot from UART is recommended). . . \n\n");
        printf("\tplease connect:\n"
                "\t\tEVB board J37 PIN1 (JTAG_TDI_CK860, GPIOB1) <--> EVB Board J13 PIN1 (GPIOA16), \n"
                "\t\tEVB board J37 PIN3 (JTAG_TDO_CK860, GPIOB2) <--> EVB Board J13 PIN3 (GPIOA17)\n");
        pmux_offset1 = POLARIS_PMUX_JTAG_TDI_CK860;
        pmux_offset2 = POLARIS_PMUX_JTAG_TDO_CK860;
        pmux_gpio1 = 1;
        pmux_gpio2 = 2;
        pmux_port1 = GPIO_B;
        pmux_port2 = GPIO_B;
        break;
    default:
        return;
    }

    printf("\tconnect done? continue? - - - [y/n] ");
    while (CK_WaitForReply() != 1) {
        printf ("\n\tstart test? - - - [y/n] ");
    }

    printf ("\n\n\tset MUX pin as GPIO function\n");
    pinmux_set_func(pmux_offset1, 1);
    pinmux_set_func(pmux_offset2, 1);
    printf ("\t[%s:%d], *** CTRL + C to exit test ***\n", __FUNCTION__, __LINE__);
    dw_Gpio_Output(pmux_port1, pmux_gpio1, 1);
    dw_Gpio_Output(pmux_port2, pmux_gpio2, 1);

    gpio0 = dw_Gpio_Input(GPIO_A, 16);
    gpio1 = dw_Gpio_Input(GPIO_A, 17);
    printf ("\t[%s:%d], gpio0=%d\n", __FUNCTION__, __LINE__, gpio0);
    printf ("\t[%s:%d], gpio1=%d\n", __FUNCTION__, __LINE__, gpio1);
    gpio0_old = gpio0;
    gpio1_old = gpio1;

    dw_Gpio_Output(pmux_port1, pmux_gpio1, 0);
    dw_Gpio_Output(pmux_port2, pmux_gpio2, 0);
    while (1) {
        gpio0 = dw_Gpio_Input(GPIO_A, 16);
        gpio1 = dw_Gpio_Input(GPIO_A, 17);
        if (gpio0 != gpio0_old) {
            passed = 1;
            printf ("\t[%s:%d], gpio0=%d\n", __FUNCTION__, __LINE__, gpio0);
            gpio0_old = gpio0;
        }

        if (gpio1 != gpio1_old) {
            passed = 1;
            printf ("\t[%s:%d], gpio1=%d\n", __FUNCTION__, __LINE__, gpio1);
            gpio1_old = gpio1;
        }

        if (0x03 ==  getchar()) {
            if (passed == 0) {
                printf ("\t\t - - -FAILURE\n");
            } else {
                printf ("\t\t - - -PASS\n");
            }
            break;
        }
    }

    printf ("\tset MUX pin as specific function\n");
    pinmux_set_func(pmux_offset1, 0);
    pinmux_set_func(pmux_offset2, 0);
}

void pin_mux_test() {
    u8 test_id = 0;
    u8 ch;

    while (1) {
        printf("\nplease choose pin MUX test module:\n");
        printf("0 -- PWM/GPIO\n");
        printf("1 -- APTS/GPIO\n");
        printf("2 -- TS_PSI/GPIO\n");
        printf("3 -- SCI7816/GPIO\n");
        printf("4 -- UART/GPIO\n");
        printf("5 -- NFC/GPIO\n");
        printf("6 -- QSPI/GPIO\n");
        printf("7 -- JTAG/GPIO\n");
        printf("> ");
        ch = getchar();
        putchar(ch);
        test_id = asciitonum((CK_UINT8 *)&ch);
        if ((test_id >= 0) && (test_id <= 7)) {
            break;
        }
    }

    do_pinmux_test(test_id);

    printf("\npin MUX test done. . . \n");
}
