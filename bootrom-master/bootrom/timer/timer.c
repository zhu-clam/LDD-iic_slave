/*
 * (Copyright 2017) Verisilicon Inc. All Rights Reserved
 * Company confidential and Proprietary information.
 * This information may not be disclosed to unauthorized individual.
 * 
 * System Application Team <fengyin.wu@verisilicon.com>
 *
 */

#include "datatype.h"
#include "uart.h"
#include "platform.h"
#include "timer.h"


void timer_init(void)
{
    u32 val,load;
    val = read_mreg32(TIMER_BASE_0+TxControl);
    val  &= ~(CK_TIMER_TXCONTROL_ENABLE);
    val |= CK_TIMER_TXCONTROL_INTMASK;
    write_mreg32(TIMER_BASE_0+TxControl, val);
    
    load = (u32)(0xffffffff);
    write_mreg32(TIMER_BASE_0+TxLoadCount, load);

    val = read_mreg32(TIMER_BASE_0+TxControl);
    /* In user-defined running mode*/
    val |= CK_TIMER_TXCONTROL_MODE;
    /* Enable the corresponding timer */
    val |= CK_TIMER_TXCONTROL_ENABLE;
    write_mreg32(TIMER_BASE_0+TxControl, val);
}

u32 get_timer_count(void)
{
    return read_mreg32(TIMER_BASE_0+TxCurrentValue);
}
