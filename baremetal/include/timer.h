/*****************************************************************************
 *  File: timer.h
 *
 *  Description: this file contains the functions support uart operations.
 *
 *  Copyright (C) : 2008 Hangzhou C-Sky Microsystems Co.,Ltd.
 *
 *  Author: ShuLi Wu
 *  Mail:   shuli_wu@c-sky.com
 *  Date:   Oct 9 2008
 *
 *****************************************************************************/
#ifndef __TIMER_H__
#define __TIMER_H__

#include "datatype.h"

/*
 * enum the timer
 */
typedef enum
{
    TIMER0,
    TIMER1,
    TIMER2,
    TIMER3,
    TIMER4,
    TIMER5,
    TIMER6,
    TIMER7
} CKEnum_Timer_Device;

#define TIMERID_MAX     8

/*
 * initialize the timer driver.
 */
void CK_Timer_Init(void);

/*
 * open the timer, register the interrupt.
 *
 * timerid: the number of the corresponding timer port;
 * handler: the interrupt service function of the corresponding timer;
 * bfast: indicate whether the fast interrupt or not;
 */
CK_INT32 CK_Timer_Open(CKEnum_Timer_Device timerid,
                       void(*handler)(), CK_UINT16 priority, BOOL bfast);

/*
 * close the timer, free interrupt.
 *
 * timerid: the number of the corresponding timer port;
 */
CK_INT32 CK_Timer_Close(CKEnum_Timer_Device timerid);

/*
 * start the corresponding timer
 *
 * timerid: the number of the corresponding timer port;
 * timeout: the set time
 */
CK_INT32 CK_Timer_Start(CKEnum_Timer_Device timerid, CK_UINT32 timeout);
CK_INT32 CK_Timer_Start_Free_Running(CKEnum_Timer_Device timerid);
/*
 * stop a designated timer
 *
 * timerid: the number of the corresponding timer port;
 * stop_val: the count value when the timer stops
 */
CK_UINT32 CK_Timer_Stop(CKEnum_Timer_Device timerid);

/*
 * clear a timer interrupt
 * by reading its End of Interrupt register(EOI)
 */
BOOL CK_Timer_ClearIrqFlag(CKEnum_Timer_Device timerid);

/*
 * read the current value of the timer
 *
 * timerid: the number of the corresponding timer;
 * current_val: the current count-value
 */
CK_UINT32 CK_Timer_CurrentValue(CKEnum_Timer_Device timerid);

void init_time0(void);
void timer_udelay(CK_UINT64 usec);
#endif
