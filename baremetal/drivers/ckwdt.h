
/*****************************************************************************
 *  File: ckwdt.h
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

#ifndef __CKWDT_H__
#define __CKWDT_H__

#include "ck810.h"
#include "datatype.h"
#include "intc.h"

/*
 * define number of the timer interrupt
 */
#define  CK_WDT_CK860   CK_INTC_WDT_CK860
#define  CK_WDT_CK810   CK_INTC_WDT_CK810
#define  CK_WDT_UNI     CK_INTC_WDT_UNI  

typedef struct CKS_WDT
{
    volatile CK_REG  WDT_CR;            /* WDT  Control Register */
    volatile CK_REG  WDT_TORR;          /* WDT Timeout Range Register */
    volatile CK_REG  WDT_CCVR;          /* WDT Current Counter Value Register */
    volatile CK_REG  WDT_CRR;	        /* WDT Counter Restart Register */
    volatile CK_REG  WDT_STAT;          /* WDT Interrupt Status Register */
    volatile CK_REG  WDT_EOI;           /* WDT Interrupt Clear Register */
    volatile CK_REG  Rev[51];	        /* 0x18 - 0xe0 */
    volatile CK_REG  WDT_COMP_PARAMS_5;	/* Component Parameters Register 5 */
    volatile CK_REG  WDT_COMP_PARAMS_4;	/* Component Parameters Register 4 */
    volatile CK_REG  WDT_COMP_PARAMS_3;	/* Component Parameters Register 3 */
    volatile CK_REG  WDT_COMP_PARAMS_2;	/* Component Parameters Register 2 */
    volatile CK_REG  WDT_COMP_PARAMS_1;	/* Component Parameters Register 1 */
    volatile CK_REG  WDT_COMP_VERSION;	/* Component Version Register */
    volatile CK_REG  WDT_COMP_TYPE;	    /* Component Type Register */
} CKStruct_WDT,* PCKPStruct_WDT;

typedef struct {
        CK_UINT32 id;               /* the number of timer */
        PCKPStruct_WDT  addr;       /* the base-address of timer */
        CK_UINT32 irq;              /* the interrupt number of timer */
        BOOL      bopened;          /* indicate whether have been opened or not */
        CK_UINT32 timeout;          /* the set time (us) */
        CKStruct_IRQHandler irqhandler; /* ISR */
} CKStruct_WDTInfo, *PCKStruct_WDTInfo;

/*
 *  define the bits for WDT_CR
 */
#define CK_WDT_CR_ENABLE        (1UL << 0)
#define CK_WDT_CR_RMOD          (1UL << 1)
#define CK_WDT_CR_RPL           (1UL << 2)

#define COUNTER_RST_CMD         0x76

#define CK_WDT_SYSTEM_RST       0
#define CK_WDT_INTC_RST         1

static CKStruct_WDTInfo CK_WDT_Table[] =
{
    {0, (PCKPStruct_WDT)CK_WDT_CK860_ADDR, CK_WDT_CK860, FALSE, 0},
    {1, (PCKPStruct_WDT)CK_WDT_CK810_ADDR, CK_WDT_CK810, FALSE, 0},
    {2, (PCKPStruct_WDT)CK_WDT_UNI_ADDR  , CK_WDT_UNI  , FALSE, 0},
};

#define WDTRID_MAX  (sizeof(CK_WDT_Table) / sizeof(CKStruct_WDTInfo))

#endif

