/*
 * Filename: tintc.c
 * Description: To test the C-SKY interrupt controller
 * Copyright: VeriSilicon
 * Author: Chunming Li
 * date: Jan 31 2018
 */

#include "ck810.h"
#include "intc.h"
#include "misc.h"
#include "ckintc.h"

static CKStruct_INTCTestInfo INTC_AUTO_MODE_Table[] ALIGN_4 =
{
    {0, 0,  FALSE},
    {1, 63, FALSE},
    {2, 0,  TRUE},
    {3, 63, TRUE}
};

// PCK_INTC: The base address of interrupt controller registers
volatile CKStruct_INTC *icrp_intc = PCK_INTC;
static CK_UINT8  intc_test;
volatile static CK_UINT32 irq_pri[2];

/*
 * Callback function for interrupt
 */
void CK_Common_Handler()
{
    CK_UINT32 IFRL;
    CK_UINT32 IFRH;
#if CK_128_INTC
    CK_UINT32 IFRL1;
    CK_UINT32 IFRH1;
#endif
    CK_UINT32 i, k;

    // Force interrupt clear
    IFRL = icrp_intc->IFRL;
    IFRH = icrp_intc->IFRH;
#if CK_128_INTC
    IFRL1 = icrp_intc->IFRL1;
    IFRH1 = icrp_intc->IFRH1;
#endif
#if CK_INTC_DEBUG
    printf("JJJ_DEBUG CK_Common_Handler IFRL=0x%x, IFRH=0x%x\n", IFRL, IFRH);
#if CK_128_INTC
    printf("JJJ_DEBUG CK_Common_Handler IFRL1=0x%x, IFRH1=0x%x\n", IFRL1, IFRH1);
#endif
#endif

    for (i = 0; i < 32; i++) {
        k = (1 << i);

        if (icrp_intc->IPRL & k) {
            icrp_intc->IFRL &= (~IFRL) & k;
            intc_test++;
        }

        if (icrp_intc->IPRH & k) {
            icrp_intc->IFRH &= (~IFRH) & k;
            intc_test++;
        }
#if CK_128_INTC
        if (icrp_intc->IPRL1 & k) {
            icrp_intc->IFRL1 &= (~IFRL1) & k;
            intc_test++;
        }

        if (icrp_intc->IPRH1 & k) {
            icrp_intc->IFRH1 &= (~IFRH1) & k;
            intc_test++;
        }
#endif
    }

#if CK_INTC_DEBUG
    printf("JJJ_DEBUG CK_Common_Handler intc_test=0x%x\n", intc_test);
#endif
}

void CK_IRQ10_Handler()
{
    // Force interrupt clear
    CK_UINT32 k;

    k = (1 << 10);
    if (icrp_intc->IPRL & k) {
        icrp_intc->IFRL &= ~k;
        irq_pri[0] = 10;
#if CK_INTC_DEBUG
        printf("JJJ_DEBUG CK_IRQ10_Handler irq_pri[%d]=%d\n", 0, irq_pri[0]);
#endif
    }
}

void CK_IRQ11_Handler()
{
    // Force interrupt clear
    CK_UINT32 k;

    k = (1 << 11);
    if (icrp_intc->IPRL & k) {
        icrp_intc->IFRL &= ~k;
        irq_pri[1] = 11;
#if CK_INTC_DEBUG
        printf("JJJ_DEBUG CK_IRQ11_Handler irq_pri[%d]=%d\n", 1, irq_pri[1]);
#endif
    }
}

void CK_INTC_Mode_Test(IN CK_UINT32 mode)
{
    CK_UINT32 i, j, k;
    PCKStruct_INTCTestInfo info;

    CK_CPU_DisAllNormalIrq();
    CK_CPU_DisAllFastIrq();

    CK_INTC_Init(mode);
    CK_Exception_Init();

    for (i = 0; i < NR_IRQS; i++) {
        printf("            Trigger Normal interrupt%d %d times\n", i, IRQ_MAX_TEST_NUM);
        info = &(INTC_AUTO_MODE_Table[0]);
        info->irqhandler.devname = "Normal";
        info->irqhandler.irqid = i;
        info->irqhandler.priority = i % 64;
        info->irqhandler.handler = CK_Common_Handler;
        info->irqhandler.bfast = FALSE;
        info->irqhandler.next = NULL;

        /* register isr */
        CK_INTC_RequestIrq(&(info->irqhandler), mode);

        intc_test = 0;
        k = (i % 64) < 32 ? (1 << (i % 64)) : (1 << ((i % 64) - 32));
        
        for (j = 0; j < IRQ_MAX_TEST_NUM; j++) {
            if ( i > 63) {
                if ((i - 64) < 32) {
                    icrp_intc->IFRL1 |= k;
                    // Poll for completion
                    while (icrp_intc->IPRL1 & k)
                        delay(1);
                } else {
                    icrp_intc->IFRH1 |= k;
                    while (icrp_intc->IPRH1 & k)
                        delay(1);
                }
            } else {
                if (i < 32) {
                    icrp_intc->IFRL |= k;
                    // Poll for completion
                    while (icrp_intc->IPRL & k)
                        delay(1);
                } else {
                    icrp_intc->IFRH |= k;
                    while (icrp_intc->IPRH & k)
                        delay(1);
                }
            }
            delay(1);
        }

        if(intc_test == IRQ_MAX_TEST_NUM)
            printf("                - - - PASS.\n");
        else
            printf("                - - - FAILURE.\n");
        delay(1);
    }

    // Then release IRQ resources
    for (i = 0; i < NR_IRQS; i++) {
        info = &(INTC_AUTO_MODE_Table[0]);
        info->irqhandler.devname = "Normal";
        info->irqhandler.irqid = i;
        info->irqhandler.priority = i % 64;
        info->irqhandler.handler = CK_Common_Handler;
        info->irqhandler.bfast = FALSE;
        info->irqhandler.next = NULL;
        CK_INTC_FreeIrq(&(info->irqhandler), mode);

    }

    delay(1);
    for (i = 0; i < NR_IRQS; i++) {
        printf("            Trigger Fast interrupt%d %d times\n", i, IRQ_MAX_TEST_NUM);
        info->irqhandler.devname = "Fast";
        info->irqhandler.irqid = i;
        info->irqhandler.priority = i % 64;
        info->irqhandler.handler = CK_Common_Handler;
        info->irqhandler.bfast = TRUE;
        info->irqhandler.next = NULL;

        /* register isr */
        CK_INTC_RequestIrq(&(info->irqhandler), mode);

        intc_test = 0;
        k = (i % 64) < 32 ? (1 << (i % 64)) : (1 << ((i % 64) - 32));
        for (j = 0; j < IRQ_MAX_TEST_NUM; j++) {
            if (i > 63) {
                if ((i - 64) < 32) {
                    icrp_intc->IFRL1 |= k;
                    // Poll for completion
                    while (icrp_intc->IPRL1 & k)
                        delay(1);
                } else {
                    icrp_intc->IFRH1 |= k;
                    while (icrp_intc->IPRH1 & k)
                        delay(1);
                }
            } else {
                if (i < 32) {
                    icrp_intc->IFRL |= k;
                    // Poll for completion
                    while (icrp_intc->IPRL & k)
                        delay(1);
                } else {
                    icrp_intc->IFRH |= k;
                    while (icrp_intc->IPRH & k)
                        delay(1);
                }
            }
        }

        if(intc_test == IRQ_MAX_TEST_NUM)
            printf("                - - - PASS.\n");
        else
            printf("                - - - FAILURE.\n");
        delay(1);
    }

    // Then release IRQ resources
    for (i = 0; i < NR_IRQS; i++) {
        info->irqhandler.devname = "Fast";
        info->irqhandler.irqid = i;
        info->irqhandler.priority = i % 64;
        info->irqhandler.handler = CK_Common_Handler;
        info->irqhandler.bfast = TRUE;
        info->irqhandler.next = NULL;
        CK_INTC_FreeIrq(&(info->irqhandler), mode);
    }
}

void CK_INTC_PRI_Test(IN CK_UINT32 mode)
{
    PCKStruct_INTCTestInfo info;
    PCKStruct_INTCTestInfo info1;
    CK_UINT32 k;

    CK_INTC_Init(mode);
    CK_Exception_Init();

    printf("            Trigger Normal IRQ10 with PRI10 and IRQ11 with PRI11 at same time\n");
    info = &(INTC_AUTO_MODE_Table[0]);
    info->irqhandler.devname = "Normal10";
    info->irqhandler.irqid = 10;
    info->irqhandler.priority = 10;
    info->irqhandler.handler = CK_IRQ10_Handler;
    info->irqhandler.bfast = FALSE;
    info->irqhandler.next = NULL;
    /* register isr */
    CK_INTC_RequestIrq(&(info->irqhandler), mode);

    info1 = &(INTC_AUTO_MODE_Table[1]);
    info1->irqhandler.devname = "Normal11";
    info1->irqhandler.irqid = 11;
    info1->irqhandler.priority = 11;
    info1->irqhandler.handler = CK_IRQ11_Handler;
    info1->irqhandler.bfast = FALSE;
    info1->irqhandler.next = NULL;
    CK_INTC_RequestIrq(&(info1->irqhandler), mode);
    irq_pri[0] = 0;
    irq_pri[1] = 0;

    k = (3 << 10);
    icrp_intc->IFRL |= k;
    // Poll for completion
    while (icrp_intc->IPRL & k)
        delay(1);

    if(irq_pri[0] == 10 && irq_pri[1] == 11)
        printf("                - - - PASS.\n");
    else
        printf("                - - - FAILURE.\n");
    CK_INTC_FreeIrq(&(info->irqhandler), mode);
    CK_INTC_FreeIrq(&(info1->irqhandler), mode);

    printf("            Trigger Fast IRQ10 with PRI10 and Normal IRQ11 with PRI11 at same time\n");
    info = &(INTC_AUTO_MODE_Table[0]);
    info->irqhandler.devname = "Normal10";
    info->irqhandler.irqid = 10;
    info->irqhandler.priority = 10;
    info->irqhandler.handler = CK_IRQ10_Handler;
    info->irqhandler.bfast = TRUE;
    info->irqhandler.next = NULL;
    /* register isr */
    CK_INTC_RequestIrq(&(info->irqhandler), mode);

    info1 = &(INTC_AUTO_MODE_Table[1]);
    info1->irqhandler.devname = "Normal11";
    info1->irqhandler.irqid = 11;
    info1->irqhandler.priority = 11;
    info1->irqhandler.handler = CK_IRQ11_Handler;
    info1->irqhandler.bfast = FALSE;
    info1->irqhandler.next = NULL;
    CK_INTC_RequestIrq(&(info1->irqhandler), mode);
    irq_pri[0] = 0;
    irq_pri[1] = 0;

    icrp_intc->IFRL |= k;
    // Poll for completion
    while (icrp_intc->IPRL & k)
        delay(1);

    if(irq_pri[0] == 10 && irq_pri[1] == 11)
        printf("                - - - PASS.\n");
    else
        printf("                - - - FAILURE.\n");
    CK_INTC_FreeIrq(&(info->irqhandler), mode);
    CK_INTC_FreeIrq(&(info1->irqhandler), mode);

    printf("            Trigger Fast IRQ10 with PRI10 and Fast IRQ11 with PRI11 at same time\n");
    info = &(INTC_AUTO_MODE_Table[0]);
    info->irqhandler.devname = "Normal10";
    info->irqhandler.irqid = 10;
    info->irqhandler.priority = 10;
    info->irqhandler.handler = CK_IRQ10_Handler;
    info->irqhandler.bfast = TRUE;
    info->irqhandler.next = NULL;
    /* register isr */
    CK_INTC_RequestIrq(&(info->irqhandler), mode);

    info1 = &(INTC_AUTO_MODE_Table[1]);
    info1->irqhandler.devname = "Normal11";
    info1->irqhandler.irqid = 11;
    info1->irqhandler.priority = 11;
    info1->irqhandler.handler = CK_IRQ11_Handler;
    info1->irqhandler.bfast = TRUE;
    info1->irqhandler.next = NULL;
    CK_INTC_RequestIrq(&(info1->irqhandler), mode);
    irq_pri[0] = 0;
    irq_pri[1] = 0;

    icrp_intc->IFRL |= k;
    // Poll for completion
    while (icrp_intc->IPRL & k)
        delay(1);

    if(irq_pri[0] == 10 && irq_pri[1] == 11)
        printf("                - - - PASS.\n");
    else
        printf("                - - - FAILURE.\n");
    CK_INTC_FreeIrq(&(info->irqhandler), mode);
    CK_INTC_FreeIrq(&(info1->irqhandler), mode);
}

/*
 * main function of timer test program.
 */
void CK_INTC_Test()
{
    printf("\nInterrupt Controller Test. . . \n");

    printf("  1.    Auto Vectored Interrupt Mode. . . \n");
    CK_INTC_Mode_Test(AUTO_MODE);
    printf("        Auto Vectored Interrupt Mode Done \n");

    delay(5);
    printf("\n  2.  Share Vectored Interrupt Mode. . . \n");
    CK_INTC_Mode_Test(VECTOR_SHARE_MODE);
    printf("        Share Vectored Interrupt Mode Done \n");

    delay(5);

#ifndef CK_128_INTC
    printf("\n  3.  Unique Vectored Interrupt Mode. . . \n");
    CK_INTC_Mode_Test(VECTOR_UNIQUE_MODE);
    printf("        Unique Vectored Interrupt Mode Done\n");
#endif

    delay(5);
    printf("\n  4.  Interrupt Priority. . . \n");
    CK_INTC_PRI_Test(AUTO_MODE);
    printf("        Interrupt Priority Done \n");

    printf("\nEnd Interrupt Controller Test. . . \n");
}
