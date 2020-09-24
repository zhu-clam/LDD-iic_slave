/*****************************************************************************
 *  File: wdt.h
 *
 *  Descirption: contains the functions support Synopsys Watchdog Timer.
 *
 *  Copyright (C) : 2018 VeriSilicon.
 *
 *  Author: Chunming Li
 *  Mail:   chunming.li@verisilicon.com
 *  Date:   Feb 9 2018
 *
 *****************************************************************************/

#ifndef __WDT_H__
#define __WDT_H__

#include "datatype.h"

/*
 * enum the watchdog timer
 */
typedef enum
{
    WDT_CK860,
    WDT_CK810,
    WDT_UNI  
} CKEnum_WDT_Device;


/*
 * initialize the Watchdog Timer.
 */
void CK_WDT_Init(void);

/*
 * open the timer, register the interrupt.
 *
 * wdtid: the number of the corresponding watchdog timer;
 * handler: the interrupt service function of the corresponding timer;
 * bfast: indicate whether the fast interrupt or not;
 */
CK_INT32 CK_WDT_Open (  CKEnum_WDT_Device wdtid,
                        IN void(*handler)(),
                        IN CK_UINT16 priority,
                        IN BOOL bfast);

/*
 * close the watchdog timer, free interrupt.
 *
 * wdtid: the number of the corresponding watchdog timer;
 */
CK_INT32 CK_WDT_Close ( CKEnum_WDT_Device wdtid );

/*
 * start the corresponding watchdog timer
 *
 * wdtid: the number of the corresponding timer;
 * timeout: the set time
 */
CK_INT32 CK_WDT_Start ( CKEnum_WDT_Device wdtid,
                        IN CK_UINT32 mode,
                        IN CK_UINT32 timeout);

/*
 * clear a timer interrupt
 * by reading its End of Interrupt register(EOI)
 */
void CK_WDT_ClearIrqFlag ( CKEnum_WDT_Device wdtid );

/*
 * read the current value of the watchdog timer
 *
 * wdtid: the number of the corresponding  watchdog timer;
 * current_val: the current count-value
 */
CK_UINT32 CK_WDT_CurrentValue ( CKEnum_WDT_Device wdtid );

/*
 * restart the counter of the timer
 *
 * wdtid: the number of the corresponding  watchdog timer;
 */
void CK_WDT_Counter_RST ( CKEnum_WDT_Device wdtid );

#endif
