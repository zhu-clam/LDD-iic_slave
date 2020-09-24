/*
 * ckpwm.c
 *
 *  Created on: Sep 28, 2018
 *      Author: cn0971
 */


#include "ckpwm.h"
#include "ck810.h"
/* bit field of PWM_CTL@0x00 */
#define PWM_EN              (0x01 << 0x0)
#define PWM_CENTER          (0x01 << 0x01)
#define PWM_POLAR           (0x01 << 0x02)
#define PWM_RESOLUT8        (0x01 << 0x03)
#define PWM_REPET_NUMBER(x) ((x & 0xFF) << 0x04)
#define PWM_REPET_NUMBER_MASK   (0xFF << 0x04)

/* bit field of PWM_CSR@0x04 */
#define PWM_IRQEN           (0x01 << 0x0)
#define PWM_CLR             (0x01 << 0x01)
#define PWM_INT             (0x01 << 0x02)
#define PWM_STATUS          (0x01 << 0x03)
#define FIFO_STATUS_MASK    (0x07 << 0x04)

/* bit field of PWM_CLK@0x08 */
#define PWM_CLKSEL_SCLK       (0x01 << 0x0)
#define PWM_CLKSEL_ACLK       (0x00)
#define PWM_CLKDIV_1          (0x00 << 0x01)
#define PWM_CLKDIV_2          (0x01 << 0x01)
#define PWM_CLKDIV_4          (0x02 << 0x01)
#define PWM_CLKDIV_8          (0x03 << 0x01)
#define PWM_CLKDIV_16         (0x04 << 0x01)
#define PWM_CLKDIV_32         (0x05 << 0x01)
#define PWM_CLKDIV_64         (0x06 << 0x01)
#define PWM_RESCALE(x)        (((x - 1) & 0xff) << 4)

/*
 * set pwm clk
 * @pwm: pwm
 * @clk_sel: PWM_CLK_SEL_APB, PWM_CLK_SEL_50M, PWM_CLK_SEL_3M, PWM_CLK_SEL_24M
 * @div_reg: CLKDIV reg val, div_val = 1 << div_reg, = PWM_CLKDIV_1 - PWM_CLKDIV_128
 * @prescale: prescale val for pwm_clk :1 - 256
 * @return: frequency(hz) of pwm_clk
 * */
CK_INT32 CK_PWM_set_clk(PCKStruct_PWMInfo pwm, u32 clk_sel, u8 div_reg, u8 prescale)
{
    u32 fcy = PWM_PCLK_FREQ;

    switch (clk_sel)
    {
        case PWM_CLKSEL_SCLK: fcy = PWM_SCLK_FREQ; break;
        case PWM_CLKSEL_ACLK: fcy = PWM_PCLK_FREQ; break;
        default: return 1;
    }
    if (0 == prescale)
    {
        printf("prescale can not set to 0!");
        return 1;
    }
    if (1 == prescale)
    {
        prescale = 2;
    }

    div_reg = div_reg & 0x7;
    pwm->addr->clk = clk_sel | (div_reg << 1) | PWM_RESCALE(prescale);

    fcy = fcy >> (div_reg + 1);//div_val = 1 << (div_reg + 1)
    fcy /= prescale;
    pwm->clk_fcy = fcy;
    return 0;
}

CK_INT32 CK_PWM_irq_enable(PCKStruct_PWMInfo pwm)
{
    pwm->addr->csr |= PWM_IRQEN;
    return SUCCESS;
}
CK_INT32 CK_PWM_check_isr(PCKStruct_PWMInfo pwm)
{
    return (pwm->addr->csr & PWM_INT) ? 1 : 0;
}

void CK_PWM_clr_isr(PCKStruct_PWMInfo pwm)
{
    pwm->addr->csr |= PWM_CLR;
    pwm->addr->csr &= ~PWM_CLR;
}
/*
 * set pwm output frequency
 * @pwm: pwm
 * @fcy_hz: out frequency
 * @return: actual frequency(hz) of output
 * */
CK_INT32 CK_PWM_set_out_fcy(PCKStruct_PWMInfo pwm, u32 fcy_hz)
{
    u16 reg = pwm->clk_fcy / fcy_hz;
    u32 fcy_out = 0;
    if (0 == reg)
    {
        reg = 1;
    }
    pwm->addr->pwr = reg;
    fcy_out = pwm->clk_fcy / reg;

    return fcy_out;
}

CK_INT32 CK_PWM_set_out_width(PCKStruct_PWMInfo pwm, u32 width)
{
    u16 reg = 0;
    if (pwm->addr->ctl & PWM_RESOLUT8)
    {/* 8bit mode */
        reg = width & 0xff;
        pwm->addr->pwr = ((u8)reg << 24)
                       | ((u8)reg << 16)
                       | ((u8)reg << 8)
                       | (u8)reg;
    }
    else
    {/* 16bit mode */
        reg = width & 0xffff;
        pwm->addr->pwr = (reg << 16) | reg;
    }

    return 0;
}

CK_INT32 CK_PWM_set_period(PCKStruct_PWMInfo pwm, u32 period )
{
    period += 1;
    if (pwm->addr->ctl & PWM_RESOLUT8)
    {/* 8bit mode */
        pwm->addr->prd = period & 0xff;
    }
    else
    {/* 16bit mode */
        pwm->addr->prd = period & 0xffff;
    }
    return 0;
}

CK_INT32 CK_PWM_get_frequency(PCKStruct_PWMInfo pwm)
{
    CK_INT32 out_frcy = 0;
    if (pwm->addr->ctl & PWM_RESOLUT8)
    {/* 8bit mode */
        out_frcy = pwm->clk_fcy / ((pwm->addr->prd & 0xff) - 1);
    }
    else
    {/* 16bit mode */
        out_frcy = pwm->clk_fcy / ((pwm->addr->prd & 0xffff) - 1);
    }
    return out_frcy;
}

CK_INT32 CK_PWM_get_duty(PCKStruct_PWMInfo pwm)
{
    CK_INT32 out_duty = 0;/* out_duty = duty * 100 */
    if (pwm->addr->ctl & PWM_RESOLUT8)
    {/* 8bit mode */
        out_duty = (pwm->addr->pwr & 0xff) * 100 / (pwm->addr->prd & 0xff);
    }
    else
    {/* 16bit mode */
        out_duty = (pwm->addr->pwr & 0xffff) * 100 / (pwm->addr->prd & 0xffff);
    }
    return out_duty;
}
/**
 * Notice: the flowing operation need re enable pwm by ctl reg[0]
 * Program PWM_CSR
 * Program PWM_CLK, PWM_PRD
 * Program PWM_PWR
 *
 */
CK_INT32 CK_PWM_set_disable_mode(PCKStruct_PWMInfo pwm)
{
    pwm->addr->ctl = 0x0;
    return 0;
}
CK_INT32 CK_PWM_set_keepout_mode(PCKStruct_PWMInfo pwm)
{
    pwm->addr->ctl &= ~PWM_REPET_NUMBER_MASK;
    pwm->addr->ctl |= PWM_EN;
    return 0;
}

CK_INT32 CK_PWM_set_repeat_mode(PCKStruct_PWMInfo pwm, u32 pulse_nb)
{

    pwm->addr->ctl &= ~PWM_REPET_NUMBER(0xff);
    pwm->addr->ctl |= PWM_REPET_NUMBER(pulse_nb);
    pwm->addr->ctl |= PWM_EN;
    return 0;
}

CK_INT32 CK_PWM_set_polarity_reversed(PCKStruct_PWMInfo pwm, bool reversed)
{

    if (reversed)
        pwm->addr->ctl |= PWM_POLAR;
    else
        pwm->addr->ctl &= ~PWM_POLAR;
    return 0;
}


CK_INT32 CK_PWM_set_resolution(PCKStruct_PWMInfo pwm, bool use_8it_mode)
{

    if (use_8it_mode)
        pwm->addr->ctl |= PWM_RESOLUT8;
    else
        pwm->addr->ctl &= ~PWM_RESOLUT8;
    return 0;
}
/*
 * features to be test
 * 1. clock sel - include
 * 2. Programmable frequency prescale and division - include
 * 4. Programmable period (16/8-bit) and pulse (16/8-bit) width - include
 * 5. Programmable pulse polarity - include
 * 6. Programmable repeat number or continuous output. - include
 * 7. Interrupt. - include
 * */
#define PWM_NUM     8
CKStruct_PWMInfo CK_PWM_Table[PWM_NUM];
int irq_triggered = 0;
/*
 * Callback function for pwm interrupt.
 */
void CK_Pwm_Handler(CK_UINT32 irq)
{
    CK_UINT32 i;
#ifdef PWM_IRQ_DEBUG
    printf("[%s:%d]\n", __FUNCTION__, __LINE__);
#endif
    for(i = 0; i < PWM_NUM; i++)
    {
        if ((CK_PWM_check_isr(&CK_PWM_Table[i]) ))
        {
            CK_PWM_clr_isr(&CK_PWM_Table[i]);
#ifdef PWM_IRQ_DEBUG
            printf("pwm[%d] irq triggered!\n", i);
#endif
            irq_triggered = 1;
        }
    }
}

void CK_PWM_test()
{
    CK_UINT32 get;
    CK_UINT32 i, j;
    int failed = 0;
    unsigned char test_id;
    u8 clk_sel, div_reg, prescale;
    printf("\nPWM Test. . . \n");
    for (i = 0; i < 8; i ++)
    {
        CK_PWM_Table[i].addr = (PCKPStruct_PWM)PWM_BASE(i);
        CK_PWM_Table[i].irqhandler.devname = "PWM";
        CK_PWM_Table[i].irqhandler.irqid = CK_INTC_PWM(i);
        CK_PWM_Table[i].irqhandler.priority = i;
        CK_PWM_Table[i].irqhandler.handler = CK_Pwm_Handler;
        CK_PWM_Table[i].irqhandler.bfast = FALSE;
        CK_PWM_Table[i].irqhandler.next = NULL;
        /* Register pwm ISR */
        CK_INTC_RequestIrq(&(CK_PWM_Table[i].irqhandler), AUTO_MODE);
    }

    while (1) {
        printf("\nplease input PWM number (0 .. 7):");
        get = getchar();
        putchar(get);
        test_id = asciitonum((CK_UINT8 *)&get);
        if ((test_id >= 0) && (test_id <= 7)) {
            break;
        }
    }

    CK_PWM_set_disable_mode(&CK_PWM_Table[test_id]);
    irq_triggered = 0;
    while (1) {
        printf("\nplease select PWM clk :");
        printf("\n0 -- APB_clk");
        printf("\n1 -- pwm slower clk");
        get = getchar();
        putchar(get);
        clk_sel = asciitonum((CK_UINT8 *)&get);
        if ((clk_sel >= 0) && (clk_sel <= 1)) {
            break;
        }
    }
    while (1) {
        printf("\nplease input PWM div_reg val(0 - 7) :");
        get = getchar();
        putchar(get);
        div_reg = asciitonum((CK_UINT8 *)&get);
        if ((div_reg >= 0) && (div_reg <= 7)) {
            break;
        }
    }
    while (1) {
        printf("\nplease input PWM prescale val(001 - 256) :");
        prescale = 0;
        for (i = 100; i > 0; i /= 10)
        {
            get = getchar();
            putchar(get);
            prescale += asciitonum((CK_UINT8 *)&get) * i;
        }
        if ((prescale >= 1) && (prescale <= 256)) {
            break;
        }
    }
    if (clk_sel)
        clk_sel = PWM_CLKSEL_SCLK;
    else
        clk_sel = PWM_CLKSEL_ACLK;
    CK_PWM_set_clk(&CK_PWM_Table[test_id], clk_sel, div_reg, prescale);
    printf("\ncounter frequency is :%d", CK_PWM_Table[test_id].clk_fcy);

    printf("\ncontinuous output 16bit mode");

    for (i = 1; i <= 3; i ++)
    {



        for (j = 1; j <= 3; j++)
        {
            CK_PWM_set_resolution(&CK_PWM_Table[test_id], 0);
            CK_PWM_set_clk(&CK_PWM_Table[test_id], clk_sel, div_reg, prescale);
            CK_PWM_set_period(&CK_PWM_Table[test_id], 1000 * i);/* set period */
            CK_PWM_set_out_width(&CK_PWM_Table[test_id], (1000 * i) >> j);/* set high level width */
            CK_PWM_set_keepout_mode(&CK_PWM_Table[test_id]);
            CK_PWM_set_polarity_reversed(&CK_PWM_Table[test_id], 0);
            printf("\n output pulse frequency: %d", CK_PWM_get_frequency(&CK_PWM_Table[test_id]));
            printf("\n output pulse duty: %d%%", CK_PWM_get_duty(&CK_PWM_Table[test_id]));
            printf("\n output pulse polarity : high level");
            printf("\n Is frequency&duty&polarity correct?");
            if (CK_WaitForReply() != 1) {
                failed = 1;
            }
            CK_PWM_set_polarity_reversed(&CK_PWM_Table[test_id], 1);
            printf("\n output pulse polarity : low level");
            printf("\n Is frequency&duty&polarity correct?");
            if (CK_WaitForReply() != 1) {
                failed = 1;
            }
            CK_PWM_set_disable_mode(&CK_PWM_Table[test_id]);
        }


    }
#if PWM_TEST_8BIT_CONTINUOUS_MODE
    printf("\ncontinuous output 8bit mode");

    for (i = 1; i <= 3; i ++)
    {
        for (j = 1; j <= 3; j++)
        {
            CK_PWM_set_resolution(&CK_PWM_Table[test_id], 1);
            CK_PWM_set_clk(&CK_PWM_Table[test_id], clk_sel, div_reg, prescale);
            CK_PWM_set_period(&CK_PWM_Table[test_id], 80 * i);/* set period */
            CK_PWM_set_out_width(&CK_PWM_Table[test_id], (80 * i) >> j);/* set high level width */
            CK_PWM_set_keepout_mode(&CK_PWM_Table[test_id]);
            CK_PWM_set_polarity_reversed(&CK_PWM_Table[test_id], 0);
            printf("\n output pulse frequency: %d HZ", CK_PWM_get_frequency(&CK_PWM_Table[test_id]));
            printf("\n output pulse duty: %d%%", CK_PWM_get_duty(&CK_PWM_Table[test_id]));
            printf("\n output pulse polarity : high level");
            printf("\n Is frequency&duty&polarity correct?");
            if (CK_WaitForReply() != 1) {
                failed = 1;
            }
            CK_PWM_set_polarity_reversed(&CK_PWM_Table[test_id], 1);
            printf("\n output pulse polarity : low level");
            printf("\n Is frequency&duty&polarity correct?");
            if (CK_WaitForReply() != 1) {
                failed = 1;
            }
            CK_PWM_set_disable_mode(&CK_PWM_Table[test_id]);
        }
    }
#endif
    printf("\nrepeat output 8bit mode with irq en");
    CK_PWM_irq_enable(&CK_PWM_Table[test_id]);
    for (i = 1; i <= 3; i ++)
    {
        for (j = 1; j <= 3; j++)
        {
            CK_PWM_set_resolution(&CK_PWM_Table[test_id], 1);
            CK_PWM_set_clk(&CK_PWM_Table[test_id], clk_sel, div_reg, prescale);
            CK_PWM_set_period(&CK_PWM_Table[test_id], 80 * i);/* set period */
            CK_PWM_set_out_width(&CK_PWM_Table[test_id], (80 * i) >> 1);/* set high level width */
            irq_triggered = 0;
            CK_PWM_set_repeat_mode(&CK_PWM_Table[test_id], j << 2);

            printf("\n output pulse frequency: %d HZ", CK_PWM_get_frequency(&CK_PWM_Table[test_id]));
            printf("\n output pulse number: %d", (j << 2) * 4);
            printf("\n Is frequency&pulse number correct?");
            if (CK_WaitForReply() != 1) {
                failed = 1;
            }
            if (0 == irq_triggered)
            {
                failed = 1;
                printf("\n pwm irq test failed !");
            }
            else
            {
                printf("\n pwm irq test pass !");
            }
            CK_PWM_set_disable_mode(&CK_PWM_Table[test_id]);
        }
    }

    if (failed)
        printf("\n pwm test failed !");
    else
        printf("\n pwm test pass !");
}
