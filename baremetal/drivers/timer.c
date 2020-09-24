/************************************************************************
 *  File: timer.c
 *
 *  Descirption: This file contains the functions support timer operations.
 *
 *  Copyright (C) : 2008 Hangzhou C-Sky Microsystems Co.,Ltd.
 *  Author: ShuLi Wu
 *  Mail:   shuli_wu@c-sky.com
 *  Date:   Oct 9 2008
 *
 ************************************************************************/

#include "ck810.h"
#include "datatype.h"
#include "cktimer.h"
#include "timer.h"
#include "intc.h"
#include "misc.h"

#define TIMER_LOAD_VAL		    0xffffffff
#define USEC_TO_COUNT(x)	    ((x) * (TIMER_DEFAULT_FREQ / 1000000))

static CKStruct_TimerInfo CK_Timer_Table[] =
{
    {0, (PCKPStruct_TIMER)CK_TIMER0_BASSADDR, CK_TIMER_IRQ0, FALSE, 0},
    {1, (PCKPStruct_TIMER)CK_TIMER1_BASSADDR, CK_TIMER_IRQ1, FALSE, 0},
    {2, (PCKPStruct_TIMER)CK_TIMER2_BASSADDR, CK_TIMER_IRQ2, FALSE, 0},
    {3, (PCKPStruct_TIMER)CK_TIMER3_BASSADDR, CK_TIMER_IRQ3, FALSE, 0},
    {4, (PCKPStruct_TIMER)CK_TIMER4_BASSADDR, CK_TIMER_IRQ4, FALSE, 0},
    {5, (PCKPStruct_TIMER)CK_TIMER5_BASSADDR, CK_TIMER_IRQ5, FALSE, 0},
    {6, (PCKPStruct_TIMER)CK_TIMER6_BASSADDR, CK_TIMER_IRQ6, FALSE, 0},
    {7, (PCKPStruct_TIMER)CK_TIMER7_BASSADDR, CK_TIMER_IRQ7, FALSE, 0},
};


/*
 * Make all timers in the idle state;
 * this function should be called before
 * INTC module working;
 */
void CK_Deactive_TimerModule()
{
    int i;
    PCKPStruct_TIMER ptimer;

    for(i = 0; i < TIMERID_MAX; i++ ) {
        ptimer = CK_Timer_Table[i].addr;
        /* Stop the corresponding timer */
        ptimer->TxControl &= ~(CK_TIMER_TXCONTROL_ENABLE);
        /* Disable interrupt */
        ptimer->TxControl |= CK_TIMER_TXCONTROL_INTMASK;
    }
}

/*
 * Initialize the timer driver.
 */
void CK_Timer_Init(void)
{
    CK_Deactive_TimerModule();
}

/*
 * Open the timer, register the interrupt.
 *
 * timerid: the number of the corresponding timer port;
 * handler: the interrupt service function of the corresponding timer;
 * fast: indicate whether the fast interrupt or not;
 * opened: indicate the state whether be opened or not
 */
CK_INT32 CK_Timer_Open(CKEnum_Timer_Device timerid,
                       void(*handler)(CK_UINT32),
                       CK_UINT16 priority, BOOL fast)
{
    PCKStruct_TimerInfo info;
    PCKPStruct_TIMER ptimer;

    if ((timerid < 0) || (timerid > TIMERID_MAX))
        return FAILURE;

    info = &CK_Timer_Table[timerid];
    ptimer = info->addr;
    if(info->opened)
        return FAILURE;

    /* Initialize IRQ handler */
    if (NULL != handler) {
        info->irqhandler.devname = "TIMER";
        info->irqhandler.irqid = info->irq;
        info->irqhandler.priority = priority;
        info->irqhandler.handler = handler;
        info->irqhandler.bfast = fast;
        info->irqhandler.next = NULL;
        /* Register timer ISR */
        CK_INTC_RequestIrq(&(info->irqhandler), AUTO_MODE);
    }

    info->opened = TRUE;
    /* Enable Timer interrupt. */
    ptimer->TxControl &= ~(CK_TIMER_TXCONTROL_INTMASK);
    printf("[%s:%d], Timer #%d\n", __FUNCTION__, __LINE__, timerid);

    return SUCCESS;
}

/*
 * Close the timer, free interrupt.
 *
 * timerid: the number of the corresponding timer port;
 * opened: indicate the state whether be opened or not
 */
CK_INT32 CK_Timer_Close(CKEnum_Timer_Device timerid)
{
    PCKStruct_TimerInfo info;
    PCKPStruct_TIMER ptimer;

    if ((timerid < 0) || (timerid > TIMERID_MAX))
        return FAILURE;

    info = &CK_Timer_Table[timerid];
    ptimer = info->addr;
    if(!info->opened)
        return FAILURE;

    /* Stop the corresponding timer */
    ptimer->TxControl &= ~CK_TIMER_TXCONTROL_ENABLE;
    /* Disable interrupt. */
    ptimer->TxControl |= CK_TIMER_TXCONTROL_INTMASK;
    /* Clear the callback function*/
    CK_INTC_FreeIrq(&(info->irqhandler), AUTO_MODE);
    info->opened = FALSE;
    printf("[%s:%d], Timer #%d\n", __FUNCTION__, __LINE__, timerid);

    return SUCCESS;
}

/*
 * start the corresponding timer
 *
 * timerid: the number of the corresponding timer port;
 * timeout: the set time (uS)
 */
CK_INT32 CK_Timer_Start(CKEnum_Timer_Device timerid, CK_UINT32 timeout)
{
    CK_UINT32 load;
    PCKStruct_TimerInfo info;
    PCKPStruct_TIMER ptimer;

    if ((timerid < 0) || (timerid > TIMERID_MAX))
        return FAILURE;

    info = &CK_Timer_Table[timerid];
    ptimer = info->addr;
    if(!info->opened)
        return FAILURE;

    load = (CK_UINT32)((TIMER_DEFAULT_FREQ / 1000000) * timeout);

    /* load time(us)  */
    ptimer->TxLoadCount = load;
    //ptimer->TxLoadCount = TIMER_LOAD_VAL;
    info->timeout = timeout;
    /* In user-defined running mode*/
    ptimer->TxControl |= CK_TIMER_TXCONTROL_MODE;

    printf("[%s:%d], Timer #%d, LoadCount=0x%x, Control=0x%x, IntStatus=0x%x\n",
        __FUNCTION__, __LINE__, timerid, ptimer->TxLoadCount,
        ptimer->TxControl, ptimer->TxIntStatus);
    /* Enable the corresponding timer */
    ptimer->TxControl |= CK_TIMER_TXCONTROL_ENABLE;

    return SUCCESS;
}

CK_INT32 CK_Timer_Start_Free_Running(CKEnum_Timer_Device timerid)
{
    PCKStruct_TimerInfo info;
    PCKPStruct_TIMER ptimer;

    if ((timerid < 0) || (timerid > TIMERID_MAX))
        return FAILURE;

    info = &CK_Timer_Table[timerid];
    ptimer = info->addr;
    if(!info->opened)
        return FAILURE;

    ptimer->TxControl &= ~CK_TIMER_TXCONTROL_MODE;
    /* Enable the corresponding timer */
    ptimer->TxControl |= CK_TIMER_TXCONTROL_ENABLE;
    
    printf("Timer #%d, LoadCount=0x%x, Control=0x%x, IntStatus=0x%x\n",
        timerid, ptimer->TxLoadCount, ptimer->TxControl, ptimer->TxIntStatus);

    return SUCCESS;
}

/*
 * Stop a designated timer
 *
 * timerid: the number of the corresponding timer port;
 * stop_val: the count value when the timer stops
 */
CK_UINT32 CK_Timer_Stop(CKEnum_Timer_Device timerid)
{
    PCKStruct_TimerInfo info;
    PCKPStruct_TIMER ptimer;

    /* If the timer does not open, return failure */
    info = &CK_Timer_Table[timerid];
    if(!info->opened)
        return FAILURE;

    ptimer = info->addr;
    /* Disable the timer */
    ptimer->TxControl &= ~(CK_TIMER_TXCONTROL_ENABLE);
    printf("[%s:%d], Timer #%d\n", __FUNCTION__, __LINE__, timerid);

    return ptimer->TxCurrentValue;
}

/*
 * Clear a timer interrupt
 * by reading its End of Interrupt register(EOI)
 */
BOOL CK_Timer_ClearIrqFlag(CKEnum_Timer_Device timerid)
{
    PCKStruct_TimerInfo info;
    PCKPStruct_TIMER ptimer;

    info = &(CK_Timer_Table[timerid]);
    ptimer = info->addr;
    if (ptimer->TxIntStatus) {
        ptimer->TxEOI;
        return TRUE;
    }

    return FALSE;
}

/*
 * Read the current value of the timer
 *
 * timerid: the number of the corresponding timer;
 * current_val: the current count-value
 */
CK_UINT32 CK_Timer_CurrentValue(CKEnum_Timer_Device timerid)
{
    PCKStruct_TimerInfo info;
    PCKPStruct_TIMER ptimer;

    info = &CK_Timer_Table[timerid];
    ptimer = info->addr;

    return ptimer->TxCurrentValue;
}

void init_time0()
{
    PCKStruct_TimerInfo info;
    PCKPStruct_TIMER ptimer;

    info = &CK_Timer_Table[0];
    ptimer = info->addr;
    
    /* load time (FULL) */
	ptimer->TxLoadCount = TIMER_LOAD_VAL;

	/*in user-defined running mode*/
	ptimer->TxControl |= CK_TIMER_TXCONTROL_MODE;

	/* enable the corresponding timer */
	ptimer->TxControl &= ~(CK_TIMER_TXCONTROL_ENABLE);
	ptimer->TxControl |= CK_TIMER_TXCONTROL_ENABLE;
}

/* delay x useconds */
void timer_udelay(CK_UINT64 usec)
{
	long tmo = USEC_TO_COUNT(usec);
	CK_INT64 now, last = CK_Timer_CurrentValue(0);

	while (tmo > 0) {
		now = CK_Timer_CurrentValue(0);
		if (now > last)	/* normal (non rollover) */ {
            tmo -= (TIMER_LOAD_VAL - now + last);
		} else {		/* rollover */
            tmo -= (last - now);
        }
		last = now;
	}
}
