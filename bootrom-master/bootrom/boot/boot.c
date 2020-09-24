/*
 * boot.c
 *
 *  Created on: Oct 9, 2018
 *      Author: cn0971
 */
#include "common.h"
#include "boot.h"
#include "platform.h"
unsigned short check_sum(unsigned char *data, unsigned int len)
{
    int i;
    unsigned short crc_value = 0;

    for (i = 0; i < len; i++)
    {
        crc_value += *data++;
    }
    return crc_value;
}

u32 sys_get_boot_mode()
{
    u32 mode = 0;
    mode = read_mreg32(BOOT_MODE_ADDR) & 0x0f;
    return mode;
}

#ifdef CONFIG_UART_BOOT_SEL
extern u32 uart_get_boot_mode(void);
#endif

u32 get_bootmode(void)
{
#if CONFIG_UART_BOOT_SEL

    return uart_get_boot_mode();
#else
    return sys_get_boot_mode();
#endif
}

int pcie_boot(void)
{
    LOAD_ENTRY enter_jump_func = (LOAD_ENTRY)read_mreg32(PCIE_BOOT_PC);
    printf("jump to PCIE_BOOT_PC : 0x%x \n", enter_jump_func);
    enter_jump_func();
    return -BOOT_FAILED;
}
