
/******************************************************************************
 *  File: cktimer.h
 *
 *  Description: this file contains the functions support timer operations.
 *
 *  Copyright (C) : 2008 Hangzhou C-Sky Microsystems Co.,Ltd.
 *
 *  Author: ShuLi Wu
 *  Mail:   shuli_wu@c-sky.com
 *  Date:   Oct 9 2008
 *
 *****************************************************************************/

#ifndef __CKTIMER_H__
#define __CKTIMER_H__

#include "ck810.h"
#include "datatype.h"
#include "intc.h"

/*
 * define number of the timer interrupt
 */
#define  CK_TIMER_IRQ0   CK_INTC_TIM0
#define  CK_TIMER_IRQ1   CK_INTC_TIM1
#define  CK_TIMER_IRQ2   CK_INTC_TIM2
#define  CK_TIMER_IRQ3   CK_INTC_TIM3
#define  CK_TIMER_IRQ4   CK_INTC_TIM4
#define  CK_TIMER_IRQ5   CK_INTC_TIM5
#define  CK_TIMER_IRQ6   CK_INTC_TIM6
#define  CK_TIMER_IRQ7   CK_INTC_TIM7

typedef struct CKS_TIMER
{
    volatile CK_REG    TxLoadCount;     /* Timer Value to be loaded into Timer */
    volatile CK_REG    TxCurrentValue;  /* Timer Current Value of Timer */
    volatile CK_REG    TxControl;       /* Timer Control Register for Timer */
    volatile CK_REG    TxEOI;           /* Timer Clears the interrupt from Timer */
    volatile CK_REG    TxIntStatus;     /* Timer0 Contains the interrupt status for Timer*/
    volatile CK_REG    reserve[41];
    volatile CK_REG    TxLoadCount2;    /* Value to be loaded into Timer when timer_N_toggle output changes from 0 to 1. */
} CKStruct_TIMER, *PCKPStruct_TIMER;

typedef struct {
    CK_UINT32 id;           /* the number of timer */
    PCKPStruct_TIMER  addr; /* the base-address of timer */
    CK_UINT32 irq;          /* the interrupt number of timer */
    BOOL      opened;       /* indicate whether have been opened or not */
    CK_UINT32 timeout;      /* the set time (us) */
    CKStruct_IRQHandler irqhandler;
} CKStruct_TimerInfo, * PCKStruct_TimerInfo;

/*
 *  define the bits for TxControl
 */
#define CK_TIMER_TXCONTROL_ENABLE      (1UL << 0)
#define CK_TIMER_TXCONTROL_MODE        (1UL << 1)
#define CK_TIMER_TXCONTROL_INTMASK     (1UL << 2)

// #define  PCK_TIMER_CONTROL  ((PCKStruct_TIMER_CON)CK_TIMER_CONTROL_BASSADDR)

typedef struct CKS_TIMER_CON
{
    volatile CK_REG    IntStatus;       /*  Contains the interrupt status of all
                                        *  timers in the component.
                                        */
    volatile CK_REG    EOI;             /*  Returns all zeroes (0) and clears all
                                        *  active interrupts.
                                        */
    volatile CK_REG    RawIntStatus;   /*  Contains the unmasked interrupt status
                                        *  of all timers in the component.
                                        */
} CKStruct_TIMER_CON,* PCKStruct_TIMER_CON;

#endif
