/*****************************************************************************
 *  File: twdt.c
 *
 *  Descirption: this file contains the WDT test cases.
 *
 *  Copyright (C) : 2018 VeriSilicon.
 *
 *  Author: Chunming Li
 *  Mail:   shuli_wu@c-sky.com
 *  Date:   Feb 9 2018
 *
 *****************************************************************************/

#include "ck810.h"
#include "intc.h"
#include "wdt.h"
#include "ckwdt.h"
#include "misc.h"

#define CK_WDT_TEST_TIME        1000

static volatile CK_UINT32 wdt_intc_count[WDTRID_MAX] = {0};
static volatile CK_UINT8 wdt_feed_dog[WDTRID_MAX] = {0};

/*
 * Callback function for timer interrupt, set timer_flag.
 */
void CK_WDT_Handler(CKEnum_WDT_Device wdtid)
{
    #if CK_WDT_DEBUG
        printf("JJJ_DEBUG CK_WDT_Handler wdt_intc_count[%d]=%d, wdt_feed_dog[%d]=%d\n",
            wdtid, wdt_intc_count[wdtid], wdtid, wdt_feed_dog[wdtid]);
    #endif

    wdt_intc_count[wdtid]++;

    if (wdt_feed_dog[wdtid] != 0) {
        CK_WDT_ClearIrqFlag(wdtid);
        CK_WDT_Counter_RST(wdtid);
    }
}

void CK_WDT_Handler0() {
    CK_WDT_Handler(WDT_CK860);
}

void CK_WDT_Handler1() {
    CK_WDT_Handler(WDT_CK810);
}

void CK_WDT_Handler2() {
    CK_WDT_Handler(WDT_UNI);
}

void CK_WDT_Counter_RST_Test(CKEnum_WDT_Device wdtid) {
    CK_UINT32 lastvalue;

    printf("  1.	Watchdog Timer Counter Restart Test. . . \n");

    #if CK_WDT_DEBUG
        printf("JJJ_DEBUG  CK_WDT_Counter_RST_Test WDT%d\n", wdtid);
    #endif
    CK_WDT_Close(wdtid);
    CK_WDT_Open(CK_WDT_Table[wdtid].id,
                CK_WDT_Table[wdtid].irqhandler.handler,
                CK_WDT_Table[wdtid].irq,
                FALSE);

    wdt_feed_dog[wdtid] = 1;
    CK_WDT_Start(wdtid, CK_WDT_INTC_RST, CK_WDT_TEST_TIME);
    CK_WDT_Counter_RST(wdtid);
    udelay(1000 * 10);
    lastvalue = CK_WDT_CurrentValue(wdtid);

    CK_WDT_Counter_RST(wdtid);
    if (CK_WDT_CurrentValue(wdtid) > lastvalue)
        printf("                - - - Watchdog Timer %d PASS.\n", wdtid);
    else
        printf("                - - - Watchdog Timer %d FAILURE.\n", wdtid);
}

void CK_WDT_INTC_RST_Test(CKEnum_WDT_Device wdtid) {
    int reset = 0;
    CK_UINT32 get;

    printf("  2.	Watchdog Timer Interrupt Mode Test. . . \n");

    printf("\n\n\t- - - Reset System?...\n");
    printf("\r\tY: Trigger interrupt and reset system later.\n\t\t");
    printf("\r\tN: Trigger interrupt and restart counter\n\t\t");
    printf("- - - [y/n] ");

    while(1){
        get = CK_WaitForReply();
        if((get == 1) ||(get == 0))
        break;
        else
        printf("\n\tPlease enter 'y' or 'n'   ");
    }

    if(get == 1) {
        wdt_feed_dog[wdtid] = 0;
        reset = 1;
     } else {
        reset = 0;
        wdt_feed_dog[wdtid] = 1;
    }

    wdt_intc_count[wdtid] = 0;

    printf("\n");
    CK_WDT_Close(wdtid);
    CK_WDT_Open(CK_WDT_Table[wdtid].id,
                CK_WDT_Table[wdtid].irqhandler.handler,
                CK_WDT_Table[wdtid].irq,
                FALSE);

    CK_WDT_Start(wdtid, CK_WDT_INTC_RST, CK_WDT_TEST_TIME);

    printf("                - - - Wait Interrupt Trigger.\n");

    // a second timeout occurs then generate a system reset
    while(wdt_intc_count[wdtid] == 0) {
        #if CK_WDT_DEBUG
            printf("JJJ_DEBUG  wait wdt_intc_count[%d]=%d\n",
                wdtid, wdt_intc_count[wdtid]);
        #endif
        delay(1);
    }

    if (reset) {
        printf("                - - - Wait System Reset.\n");
        while(1) {
            delay(1000);
            printf("%d", wdtid);
        }
    } else {
        delay(1000);
        printf("                - - - Watchdog Timer %d Interrupt Mode PASS.\n", wdtid);
    }
}

void CK_WDT_System_RST_Test(CKEnum_WDT_Device wdtid) {

    printf("  2.	Watchdog Timer System Reset Mode Test. . . \n");

    CK_WDT_Close(wdtid);
    CK_WDT_Open(CK_WDT_Table[wdtid].id,
                CK_WDT_Table[wdtid].irqhandler.handler,
                CK_WDT_Table[wdtid].irq,
                FALSE);

    CK_WDT_Start(wdtid, CK_WDT_SYSTEM_RST, CK_WDT_TEST_TIME);

    printf("                - - - Wait System Reset.\n");
    while(1) {
        delay(1000);
        printf("%d", wdtid);
    }
}

/*
 * main function of watchdog timer test program.
 */
void CK_Watchdog_Test()
{
    CK_UINT32 get;
    unsigned char wdt_id;

    printf("\nWatchdog Timer Test. . . \n");
    
    CK_WDT_Init();

    CK_WDT_Table[0].irqhandler.handler = CK_WDT_Handler0;
    CK_WDT_Table[1].irqhandler.handler = CK_WDT_Handler1;
    CK_WDT_Table[2].irqhandler.handler = CK_WDT_Handler2;

    wdt_id = 10;
    while (1) {
        printf("\nplease input watchdog number (0, 1, 2):");
        get = getchar();
        putchar(get);
        wdt_id = asciitonum((CK_UINT8 *)&get);
        if ((wdt_id == 0) || (wdt_id == 1) || (wdt_id == 2)) {
            break;
        }
    }
    printf("\ntest watchdog %d\n", wdt_id);
    CK_WDT_Counter_RST_Test(wdt_id);

    printf("\n\n\t- - - Testing mode...\n");
    printf("\r\tY: Interrupt and Reset Mode; N: System Reset mode\n\t\t");
    printf("- - - [y/n] ");
    while(1){
        get = CK_WaitForReply();
        if((get == 1) || (get == 0))
            break;
        else
            printf("\n\tPlease enter 'y' or 'n'   ");
    }

    printf("\n");

    if(get == 1)
        CK_WDT_INTC_RST_Test(wdt_id);
    else
        CK_WDT_System_RST_Test(wdt_id);
    printf("\n");
}

