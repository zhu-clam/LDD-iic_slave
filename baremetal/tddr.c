/*****************************************************************************
 *  File: tddr.c
 *
 *  Descirption: this file contains the DDR interleav test case.
 *
 *  Copyright (C) : 2018 VeriSilicon.
 *
 *  Author: Chunming Li
 *  Date:   Jan 20 2019
 *
 *****************************************************************************/

#include "crm.h"
#include "misc.h"
#include "sys_ctrl.h"

static int test_mode[6] = {4096, 1024, 256, 0, 512, 1024};
// N = DDR size / interleave size = 1GB / 4K or 1K or 256B or 512B or 1K
static int n_size[6] = {0x4000, 0x10000, 0x40000, 0, 0x20000, 0x10000};
static int test_data[4][8] = {
                        {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8},
                        {0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10},
                        {0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18},
                        {0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20}};

struct mem_test{
    void *sta_addr;
    u32 szie;
    u32 seed;
};
struct mem_test ddr_table[] = {
        /*   test addressing, bytes , write value*/
        {(void *)0x00000000, 0x1000, 0xaa112258},
        {(void *)0x00001000, 0x1000, 0x53cda258},
        {(void *)0x00002000, 0x1000, 0xbb784239},
        {(void *)0x00003000, 0x1000, 0x952ad239},
        {(void *)0x1fff4000, 0x4000, 0xcc852459},
        {(void *)0x2fff4000, 0x4000, 0x9ad2c839},
        {(void *)0x7fff4000, 0x4000, 0xdd415978},
        {(void *)0xffff4000, 0x4000, 0xdd415978},

        /*{(void *)0x80000000, 0x40, 0x1a351268},
        {(void *)0x80001000, 0x40, 0x9e6f2136},
        {(void *)0x80002000, 0x40, 0x95a1ef5a},
        {(void *)0x80003000, 0x40, 0x1b784236},
        {(void *)0x80004000, 0x40, 0x1c123548},
        {(void *)0x80005000, 0x40, 0x8a55fe02},
        {(void *)0x80006000, 0x40, 0x1d953587},

        {(void *)0xefffc000, 0x40, 0x5a123594},
        {(void *)0xefffd000, 0x40, 0x5b156739},
        {(void *)0xefffe000, 0x40, 0x5c957843},
        {(void *)0xeffff000, 0x40, 0x5d321894},*/

        {(void *)0, 0, 0}
};

int run_mem_test(struct mem_test *test)
{
    u32 i = 0;
    u32 *addr = test->sta_addr;

    for (i = 0; i < test->szie; i += 4) {

        write_mreg32(addr, 0);
        addr ++;
    }

    addr = test->sta_addr;
    for (i = 0; i < test->szie; i += 4) {

        if (read_mreg32(addr))
            return 1;
        addr ++;
    }
    addr = test->sta_addr;
    for (i = 0; i < test->szie; i += 4) {

        write_mreg32(addr, test->seed);
        addr ++;
    }

    addr = test->sta_addr;
    for (i = 0; i < test->szie; i += 4) {

        if (test->seed != read_mreg32(addr))
            return 1;
        addr ++;
    }
    return 0;
}

void DDR_test(void)
{
    int indx = 0;
    int test_ret = 0;
    printf("\t\t DDR read write test\n");
    while (ddr_table[indx].szie)
    {
        printf("test %d sta_addr 0x%x size 0x%x \n",
                indx, ddr_table[indx].sta_addr, ddr_table[indx].szie);
        if (run_mem_test(&ddr_table[indx])) {
            test_ret++;
            printf("\t\t FAIL\n");
        }
        else {
            printf("\t\t PASS\n");
        }
        indx++;
    }
    if (test_ret) {
        printf("All test %d, FAIL %d\n", indx, test_ret);
    }
    else {
        printf("All PASS\n");
    }

}

void CK_DDR_Interleave_Test() {
    CK_UINT8 i;
    CK_UINT8 fail = 0;
    CK_UINT32 addr;
    CK_UINT32 val;
    CK_UINT8 data;
    CK_UINT8 N;
    CK_UINT8 mode;

    printf("\nVSI DDR Interleave Test. . . \n");

    // Check default DDR mode
    val = read_mreg32(DDR_REMAP_CTL);
    if (val != 3) {
        fail = 1;
        printf("\n\tDefault DDR address mode is %d\n", val);
        goto test_end;
    }

    // Test different DDR address mode
    for (mode = 0; mode < 6; mode++) {
        fail = 0;

        if (mode == 3)
            continue;

        printf("\n\t Start test DDR address mode %d\n", mode);

        // Disable DDR interleave mode
        write_mreg32(DDR_REMAP_CTL, 3);

        // Write data when disable interleave and read data when enable interleave
        for (i = 0; i < 8; i++) {
            write_mreg8(0x0 + i, test_data[0][i]);
            write_mreg8(0x40000000 + i, test_data[1][i]);
            write_mreg8(0x80000000 + i, test_data[2][i]);
            write_mreg8(0xc0000000 + i, test_data[3][i]);
        }

        // Set DDR interleave mode
        write_mreg32(DDR_REMAP_CTL, mode);

        // Check data
        for (i = 0; i < 8; i++) {
            N = i > 3 ? 1 : 0;
            addr = 0x0 + i;
            if (read_mreg8(addr) != test_data[0][i]) {
                fail = 1;
                printf("\n\tAddress 0x%x data 0x%x != expect data 0x%x\n",
                        addr, read_mreg8(addr), test_data[0][i]);
            }

            addr = 0x0 + test_mode[mode] + i;
            if (read_mreg8(addr) != test_data[1][i]) {
                fail = 1;
                printf("\n\tAddress 0x%x data 0x%x != expect data 0x%x\n",
                        addr, read_mreg8(addr), test_data[1][i]);
            }

        /* DDR2&3 are Non-interleave in mode 5*/
        if (mode == 5) {
            addr = 0x80000000 + i;
        if (read_mreg8(addr) != test_data[2][i]) {
            fail = 1;
            printf("\n\tAddress 0x%x data 0x%x != expect data 0x%x\n",
                addr, read_mreg8(addr), test_data[2][i]);
        }

        addr = 0xc0000000 + i;
        if (read_mreg8(addr) != test_data[3][i]) {
            fail = 1;
            printf("\n\tAddress 0x%x data 0x%x != expect data 0x%x\n",
                addr, read_mreg8(addr), test_data[3][i]);
        }
        } else {
        addr = 0x0 + 2 * test_mode[mode] + i;
        if (read_mreg8(addr) != test_data[2][i]) {
            fail = 1;
            printf("\n\tAddress 0x%x data 0x%x != expect data 0x%x\n",
                addr, read_mreg8(addr), test_data[2][i]);
        }

        addr = 0x0 + 3 * test_mode[mode] + i;
        if (read_mreg8(addr) != test_data[3][i]) {
            fail = 1;
            printf("\n\tAddress 0x%x data 0x%x != expect data 0x%x\n",
                addr, read_mreg8(addr), test_data[3][i]);
        }
        }
        }
    }

test_end:
    if (fail)
        printf("\n\t\t ---FAIL\n");
    else
        printf("\n\t\t ---PASS\n");

    printf("\n\tEnd DDR Interleave Test\n");
}
