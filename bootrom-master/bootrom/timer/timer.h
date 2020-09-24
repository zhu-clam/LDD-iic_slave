/*
 * (Copyright 2017) Verisilicon Inc. All Rights Reserved
 * Company confidential and Proprietary information.
 * This information may not be disclosed to unauthorized individual.
 * 
 * System Application Team <fengyin.wu@verisilicon.com>
 *
 */

#ifndef __TIMER_H__
#define __TIMER_H__

#include "datatype.h"
#include "platform.h"


#define CK_TIMER_TXCONTROL_ENABLE      (1UL << 0)
#define CK_TIMER_TXCONTROL_MODE        (1UL << 1)
#define CK_TIMER_TXCONTROL_INTMASK     (1UL << 2)

#define    TxLoadCount      0x00/* Timer Value to be loaded into Timer */
#define    TxCurrentValue     0x04/* Timer Current Value of Timer */
#define    TxControl         0x08/* Timer Control Register for Timer */
#define    TxEOI             0x0c/* Timer Clears the interrupt from Timer */
#define    TxIntStatus      0x10/* Timer0 Contains the interrupt status for Timer*/


void timer_init(void);
u32 get_timer_count(void);

#endif


