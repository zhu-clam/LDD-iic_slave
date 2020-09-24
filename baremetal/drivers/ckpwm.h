/*
 * ckpwm.h
 *
 *  Created on: Sep 28, 2018
 *      Author: cn0971
 */

#ifndef DRIVERS_CKPWM_H_
#define DRIVERS_CKPWM_H_

#include "ck810.h"
#include "datatype.h"
#include "intc.h"

typedef struct CKS_PWM
{
    volatile CK_REG  ctl; /* 0x00 */
    volatile CK_REG  csr;
    volatile CK_REG  clk;
    volatile CK_REG  prd; /* 0x0c */
    volatile CK_REG  cnt;
    volatile CK_REG  pwr;
} CKStruct_PWM,* PCKPStruct_PWM;


typedef struct {
    CK_UINT32 id;               /* the number of pwm */
    PCKPStruct_PWM  addr;       /* the base-address of timer */
    CK_UINT32 irq;              /* the interrupt number of timer */
    BOOL      bopened;          /* indicate whether have been opened or not */
    CKStruct_IRQHandler irqhandler; /* ISR */
    CK_UINT32 clk_fcy;          /* frequency of internal counter */
} CKStruct_PWMInfo, *PCKStruct_PWMInfo;

extern void CK_PWM_test();
#endif /* DRIVERS_CKPWM_H_ */
