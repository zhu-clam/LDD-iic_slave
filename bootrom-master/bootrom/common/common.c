/*
 * (Copyright 2017) Verisilicon Inc. All Rights Reserved
 * Company confidential and Proprietary information.
 * This information may not be disclosed to unauthorized individual.
 * 
 * System Application Team <fengyin.wu@verisilicon.com>
 *
 */

#include "datatype.h"
#include "platform.h"
#include "common.h"
#include "timer.h"

#if 0

void usleep(unsigned long useconds)
{
    unsigned long i ;
    for(i = 0 ; i < useconds ;i++);

    return;
}
#else
void usleep(unsigned long useconds)
{
    u32 start,end,count;
    u32 clk = TIMER_DEFAULT_FREQ / 1000000;  //timer clock, unit:MHz

    start = get_timer_count();
    while(1)
    {
        end = get_timer_count();
        if(start>=end)
            count = start-end;
        else
            count = 0xffffffff - end + start;

        if((count/clk) >= useconds)
            break;
    }
}
#endif
void delay_ms(unsigned long mseconds)
{
    while(mseconds --)
    {
        usleep(1000);
    }

}
void delay_s(unsigned long seconds)
{
    while(seconds --)
    {
        usleep(1000000);
    }
}
