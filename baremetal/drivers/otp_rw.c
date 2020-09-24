/*****************************************************************************
 *  File: otp_rw.c
 *
 *  Descirption: contains the functions support OTP Read/Write.
 *
 *  Copyright (C) : 2018 VeriSilicon.
 *
 *  Author: Lu Gao
 *  Mail:   lu.gao@verisilicon.com
 *  Date:   Dec 24 2018
 *
 *****************************************************************************/

#include "otp.h"
#include "misc.h"
#include "crm.h"

#define OPT_VDD18_CTLNB     13
#define TRANSFER_TIMEOUT	1000
#define TEST_LEN            4

static CK_UINT32 TEST_ADDR[TEST_LEN] = {0x0, 0x1e, 0x20, 0x34};
static CK_UINT32 TEST_DATA[TEST_LEN] = {0xa569, 0xc35a, 0x693c, 0x0ff0};

void program_otp(unsigned int addr, unsigned int data) {
    int i = 0;

    //---------Program flow
    // 5. Set row address
    write_mreg32(ADDR_ROW, addr);
    // 6. Set PROG bit
    write_mreg32(GLB_CTRL, 0x1);
    // 7. Wait 5us
    udelay(5);

    for(i = 0; i < 16; i++) {
        // 8. Set column address
        write_mreg32(ADDR_COL, i);
        // 9. Set PAS=0
        write_mreg32(PAS, 0x0);
        // 10. Set PDIN
        write_mreg32(PDIN, data);
        // 11. Set PWE bit
        write_mreg32(GLB_CTRL, 0x3);
        // 12. Wait 12us
        udelay(12);
        // 13. Clear PWE bit
        write_mreg32(GLB_CTRL, 0x1);
        // 14. Wait 1us
        udelay(1);
        // 15. Set PAS=1
        write_mreg32(PAS, 0x1);
        // 16. Set PWE bit
        write_mreg32(GLB_CTRL, 0x3);
        // 17. Wait 12us
        udelay(12);
        // 18. Clear PWE bit
        write_mreg32(GLB_CTRL, 0x1);
        // 19. Wait 1us ;  one loop = 0.15us
        udelay(1);

        data = data >> 1;
    }

    //Clear PDIN
    write_mreg32(PDIN, 0x0);
    // 21. Wait 5us
    udelay(5);

    // 22. Clear PROG bit
    write_mreg32(GLB_CTRL, 0x0);
    // 23. Wait 5us
    udelay(5);
}


void CK_OTP_Test(void) {
    int result = 0;
    int i = 0;
    int start = 0;

    CK_Gpio_Output(OPT_VDD18_CTLNB, 0);//enable otp VDD1.8
    // release OTP reset
    result = read_mreg32(CK_CRM_ADDR + BLK_SW_RST0);
    write_mreg32(CK_CRM_ADDR + BLK_SW_RST0, result | OTP_SW_RST);

    printf("\nOTP Controller Test. . . \n");

    //-------Check default value
    // 8.3 OTP Initialization, OTP stays at ready state for read
    // after power on / HW reset released and auto loading
    //Wait for OTP ready
    start = 0;
    do {
        if (read_mreg32(OTP_STATUS)) {
            break;
        }
        start += 1;
        udelay(10);
    } while (start < TRANSFER_TIMEOUT);

    if (start >= TRANSFER_TIMEOUT) {
        printf("\t\tOTP not ready, Test Fail\n");
        return;
    }

    printf("\t# Check default data #\n");

    // 8.2 OTP Read, directly read AHB address at 0x000 ~ 0x1fff
    for (i = 0; i < TEST_LEN; i++) {
        result = read_mreg16(OTP_BASE_ADDR + TEST_ADDR[i]);
        if (result != 0xFFFF) {
            printf("\t\tAddress 0x%x default value 0x%x != 0xFFFF, Test Fail\n",
                    TEST_ADDR[i], result);
            return;
        }
    }

    // 8.1 OTP Program
    // 1 ---------Enter standby
    write_mreg32(LOW_POWER, 0x2);

    //Wait 100us
    udelay(100);

    printf("\t# Program eMemory #\n");
    // ---------Program flow
    // 2. Set PTM
    write_mreg32(TEST_CTRL, 0x2);
    // 3. Exit standby
    write_mreg32(LOW_POWER, 0x0);
    // 4. Wait for OTP ready
    start = 0;
    do {
        if (read_mreg32(OTP_STATUS)) {
            break;
        }
        start += 1;
        udelay(10);
    } while (start < TRANSFER_TIMEOUT);

    if (start >= TRANSFER_TIMEOUT) {
        printf("\t\tOTP not ready, Test Fail\n");
        return;
    }

    if (!reg_readbk32(TEST_CTRL, 0x2)) {
        printf("\t\tTEST_CTRL value != 0x2, Test Fail\n");
        return;
    }

    //Program OTP
    program_otp(TEST_ADDR[0] / 2, TEST_DATA[0]);
    //Program OTP
    program_otp(TEST_ADDR[1] / 2, TEST_DATA[1]);

    // 25. ---------Enter deep standby
    write_mreg32(LOW_POWER, 0x3);

    //Wait 100us
    udelay(100);

    //Set PTM for read cycle
    write_mreg32(TEST_CTRL, 0x0);
    //-------Exit deep standby
    write_mreg32(LOW_POWER, 0x0);
    //Wait for OTP ready
    start = 0;
    do {
        if (read_mreg32(OTP_STATUS)) {
            break;
        }
        start += 1;
        udelay(10);
    } while (start < TRANSFER_TIMEOUT);

    if (start >= TRANSFER_TIMEOUT) {
        printf("\t\tOTP not ready, Test Fail\n");
        return;
    }

    if (!reg_readbk32(TEST_CTRL, 0x0)) {
        printf("\t\tTEST_CTRL value != 0x0, Test Fail\n");
        return;
    }

    //---------Check data
    printf("\t\t# Read program data #\n");


    for (i = 0; i < 2; i++) {
        result = read_mreg16(OTP_BASE_ADDR + TEST_ADDR[i]);
        if (result != TEST_DATA[i]) {
            printf("\t\t Addr 0x%x data 0x%x != 0x%x, Test Fail\n",
                    TEST_ADDR[i], result, TEST_DATA[i]);
            return;
        }
    }

    // 1. ---------Enter deep standby
    write_mreg32(LOW_POWER, 0x2);

    // Wait 50us
    udelay(50);

    write_mreg32(LOW_POWER, 0x3);

    //Wait 50us
    udelay(50);


    //-------Exit deep standby
    write_mreg32(LOW_POWER, 0x2);

    //Wait 50us
    udelay(50);

    //---------Program flow
    // 2. Set PTM
    write_mreg32(TEST_CTRL, 0x2);
    // 3. Exit standby
    write_mreg32(LOW_POWER, 0x0);
    // 4. Wait for OTP ready
    start = 0;
    do {
        if (read_mreg32(OTP_STATUS)) {
            break;
        }
        start += 1;
        udelay(10);
    } while (start < TRANSFER_TIMEOUT);

    if (start >= TRANSFER_TIMEOUT) {
        printf("\t\tOTP not ready, Test Fail\n");
        return;
    }

    if (!reg_readbk32(TEST_CTRL, 0x2)) {
        printf("\t\tTEST_CTRL value != 0x2, Test Fail\n");
        return;
    }

    //Program OTP
    program_otp(TEST_ADDR[2] / 2, TEST_DATA[2]);
    //Program OTP
    program_otp(TEST_ADDR[3] / 2, TEST_DATA[3]);

    // 25. ---------Enter standby
    write_mreg32(LOW_POWER, 0x2);

    //Wait 50us
    udelay(50);

    //Set PTM for read cycle
    write_mreg32(TEST_CTRL, 0x0);
    //-------Exit standby
    write_mreg32(LOW_POWER, 0x0);

    //Wait for OTP ready
    start = 0;
    do {
        if (read_mreg32(OTP_STATUS)) {
            break;
        }
        start += 1;
        udelay(10);
    } while (start < TRANSFER_TIMEOUT);

    if (start >= TRANSFER_TIMEOUT) {
        printf("\t\tOTP not ready, Test Fail\n");
        return;
    }

    //---------Check data
    printf("\t\t# Read program data #\n");

    for (i = 2; i < 4; i++) {
        result = read_mreg16(OTP_BASE_ADDR + TEST_ADDR[i]);
        if (result != TEST_DATA[i]) {
            printf("\t\t Addr 0x%x data 0x%x != 0x%x, Test Fail\n",
                    TEST_ADDR[i], result, TEST_DATA[i]);
            return;
        }
    }

    printf("\n\tOTP Test PASS. . . \n");
}
