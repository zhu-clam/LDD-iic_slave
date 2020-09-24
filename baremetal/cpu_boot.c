/*****************************************************************************
 *  File: cpu_boot.c
 *
 *  Description: this file contains the multiple CPU boot test cases.
 *
 *  Copyright (C) : 2019 VeriSilicon.
 *
 *  Author: Renwei Liu
 *  Date:   Feb. 2019
 *
*****************************************************************************/

#include <string.h>
#include "ck810.h"
#include "datatype.h"
#include "timer.h"
#include "cache.h"
#include "misc.h"

#include "pmu.h"

#define CK860_BOOT_CTL          (CK_SYS_CTRL_ADDR + 0x000)
#define CK810_BOOT_CTL          (CK_SYS_CTRL_ADDR + 0x004)
#define BOOT_SEL                (CK_SYS_CTRL_ADDR + 0x010)

#define SLAVE_CPU_BOOT_ADDR     0xf0040000

#define TEST_MEM_ADDR           0x10000000
#define TEST_SRC_ADDR           0x20000000

/*
int main() {
    while (1) {
        *((volatile int *)0x10000000) = *(volatile unsigned int *)(0x20000000);
    }
    return 0;
}

    84c8:	ea231000 	movih      	r3, 4096
    84cc:	ea222000 	movih      	r2, 8192
    84d0:	9240      	ld.w      	r2, (r2, 0)
    84d2:	b340      	st.w      	r2, (r3, 0)
    84d4:	07fa      	br      	0x84c8	// 84c8 <main+0x8>
*/

void set_slave_program()
{
    // set code .text
    write_iram32(SLAVE_CPU_BOOT_ADDR + 0x100, 0x1000ea23);
    write_iram32(SLAVE_CPU_BOOT_ADDR + 0x104, 0x2000ea22);
    write_iram16(SLAVE_CPU_BOOT_ADDR + 0x108, 0x9240);
    write_iram16(SLAVE_CPU_BOOT_ADDR + 0x10a, 0xb340);
    write_iram16(SLAVE_CPU_BOOT_ADDR + 0x10c, 0x07fa);

    // set CPU start address
    write_iram32(SLAVE_CPU_BOOT_ADDR, SLAVE_CPU_BOOT_ADDR + 0x100);
}

void CK_CPU_Boot_Test()
{
    CK_UINT32 boot_sel;
    CK_UINT32 val;

    printf("\n--- CPU Boot Test start---\n");

    set_slave_program();
    write_iram32(TEST_MEM_ADDR, 0x0);
    write_iram32(TEST_SRC_ADDR, 0x5aa5); // set initial value
    __asm__ __volatile__ ("sync\n");

    boot_sel = read_mreg32(BOOT_SEL);
    if (boot_sel == 0) {
        printf("boot from CK860\n");
        val = read_iram32(TEST_MEM_ADDR);
        if (val != 0) {
            printf("\tset value of addr 0x%x to 0 fail", TEST_MEM_ADDR);
        }
        printf("release slave CPU -> CK810\n");
        write_mreg32(CK810_BOOT_CTL, 0xa501);
        delay(10);

        // check value of TEST_MEM_ADDR
        val = read_iram32(TEST_MEM_ADDR);
        if (val != 0x5aa5) {
            printf("\tcheck value of addr 0x%x fail, expect 0x5aa5, got 0x%x\n",
                TEST_MEM_ADDR, val);
            printf("\tslave CPU run fail\n");
            goto done;
        } else {
            printf("\tslave CPU run pass\n");
        }
    } else if (boot_sel == 1) {
        printf("boot from CK810\n");
        printf("release slave CPU -> CK860\n");
        write_mreg32(CK860_BOOT_CTL, 0xa501);
        delay(10);

        // check value of TEST_MEM_ADDR
        val = read_iram32(TEST_MEM_ADDR);
        if (val != 0x5aa5) {
            printf("\tcheck value of addr 0x%x fail, expect 0x5aa5, got 0x%x\n",
                TEST_MEM_ADDR, val);
            printf("\tslave CPU run fail\n");
            goto done;
        } else {
            printf("\tslave CPU run pass\n");
        }
    } else {
        printf("unsupported boot selection %d\n", boot_sel);
        goto done;
    }

    write_iram32(TEST_SRC_ADDR, 0x1234);
    __asm__ __volatile__ ("sync\n");
    printf("\n\tinject value 0x1234 to source address\n");
    udelay(1000);
    val = read_iram32(TEST_MEM_ADDR);
    if (val != 0x1234) {
        printf("\tslave CPU is not running(fail), expect 0x1234, got 0x%x\n", val);
        goto done;
    } else {
        printf("\tslave CPU is still running(pass)\n");
    }

    write_iram32(TEST_SRC_ADDR, 0xaabb);
    __asm__ __volatile__ ("sync\n");
    printf("\n\tinject value 0xaabb to source address\n");
    udelay(1000);
    printf("\tpower down slave CPU\n");
    domain_power_off(CK810_POWER_GATING_CTL, CK810_HARD_PWR_CTL);
    val = read_iram32(TEST_MEM_ADDR);
    if (val != 0xaabb) {
        printf("\tslave CPU is power down now -- pass\n");
    } else {
        printf("\tslave CPU is still running -- fail\n");
    }

done:
    printf("\n--- CPU Boot Test end---\n");
}

