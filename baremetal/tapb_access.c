/*
 * tapb_access.c
 *
 *  Created on: Sep 27, 2018
 *      Author: cn0971
 */

#include "datatype.h"
#include "ck810.h"
#include "uart.h"
#include "misc.h"
#include "intc.h"

#define UART_DEBUG
#ifdef UART_DEBUG
#define debug(format, ...)  printf(format,##__VA_ARGS__)
#else
#define debug(format, ...)  do {} while (0)
#endif

int reg_access_test(char* name, void *base, int read_len, int write_add)
{
    u32 reg = 0;
    int i = 0;
    debug("%s ip base :0x%x \n",name, base);
    debug("read all regs val----------------- \n");
    for (i = 0; i <= read_len; i += 0x4)
    {
        reg = read_mreg32(base + i);
        debug("reg0x%x: 0x%x\n", i, reg);
    }
    debug("wirte reg 0x%x to 0xaa------------ \n", write_add);
    reg = 0xaa;
    write_mreg32(base + write_add,reg);
    reg = read_mreg32(base + write_add);
    debug("read reg 0x%x :0x%x \n", write_add, reg);
    return 0;
}

void apb_access_test(void)
{
    u32 i = 0;
    for (i = (u32)CK_UART_ADDRBASE1; i <= (u32)CK_UART_ADDRBASE4; i += 0x1000)
    {
        reg_access_test("uart", (void*)i, 0x22, 0x10);
    }
    reg_access_test("spi0", (void*)SPI0_BASE, 0x2c, 0x00);
    reg_access_test("spi1", (void*)SPI1_BASE, 0x2c, 0x00);
    reg_access_test("spi2", (void*)SPI2_BASE, 0x2c, 0x00);
    reg_access_test("spi3", (void*)SPI3_BASE, 0x2c, 0x00);
    for (i = (u32)I2S0_BASE; i <= (u32)I2S4_BASE; i += 0x1000)
    {
        reg_access_test("I2S", (void*)i, 0x1fc, 0x3c);
    }
    for (i = (u32)CK_I2C0_BASSADDR; i <= (u32)CK_I2C3_BASSADDR; i += 0x1000)
    {
        reg_access_test("I2C", (void*)i, 0x3c, 0x00);
    }
    for (i = (u32)PWM_BASE(0); i <= (u32)PWM_BASE(7); i += 0x200)
    {
        reg_access_test("pwm", (void*)i, 0x20, 0x1c);
    }
    reg_access_test("rtc", (void*)CK_RTC_ADDR, 0x20, 0x08);
    reg_access_test("sci7816", (void*)SCI7816_ADDR, 0x44, 0x30);
    reg_access_test("gpio", (void*)CK_GPIO_ADDR, 0x74, 0x00);
    reg_access_test("APTS", (void*)APTS_BASE, 0x2c, 0x00);
    reg_access_test("WDT_860", (void*)CK_WDT_CK860_ADDR, 0xfc, 0x04);
    reg_access_test("WDT_810", (void*)CK_WDT_CK810_ADDR, 0xfc, 0x04);
    reg_access_test("INTC_810", (void*)INTC_CK810_BASE, 0x1, 0x190);
    reg_access_test("INTC_UNI", (void*)INTC_UNI_BASE, 0x1, 0x190);
    reg_access_test("PCIe", (void*)PCIE_BASE_ADDR, 0x10, 0x4);

}

