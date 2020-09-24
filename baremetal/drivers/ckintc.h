/*
 * Description: ckintc.h - Define the structure and state the interface for
 * interrupt controller.
 *
 * Copyright (C) : 2008 Hangzhou C-SKY Microsystems Co.,LTD.
 * Author(s): Jianyong Jiang (jianyong_jiang@c-sky.com)
              Dongqi Hu  (dongqi_hu@c-sky.com)
 * Contributors: Chunqiang Li
 * Date:  2008-09-26
 */

#ifndef _CKINTC_H_
#define _CKINTC_H_

#include "ck810.h"

/* define the registers structure of the interrupt controller */
typedef struct CKS_INTC
{
    volatile CK_REG    ICR_ISR; //0x00
    volatile CK_REG    Rev0;    //0x04
    volatile CK_REG    IFRL;    //0x08
    volatile CK_REG    IPRL;    //0x0c
    volatile CK_REG    NIERL;   //0x10
    volatile CK_REG    NIPRL;   //0x14
    volatile CK_REG    FIERL;   //0x18
    volatile CK_REG    FIPRL;   //0x1c
    volatile CK_REG    IFRH;    //0x20
    volatile CK_REG    IPRH;    //0x24
    volatile CK_REG    NIERH;   //0x28
    volatile CK_REG    NIPRH;   //0x2c
    volatile CK_REG    FIERH;   //0x30
    volatile CK_REG    FIPRH;   //0x34
    volatile CK_REG    Rev1[2];  //0x38 - 0x3c
    volatile CK_REG    PR[16];  //0x40 - 0x7f
    volatile CK_REG    Rev2[4];  //0x80 - 0x8f
    volatile CK_REG    IMASKRL;  //0x90
    volatile CK_REG    IMASKRH;  //0x94
    volatile CK_REG    Rev3[28];  //0x98 - 0x107
    volatile CK_REG    IFRL1;    //0x108
    volatile CK_REG    IPRL1;    //0x10c
    volatile CK_REG    NIERL1;   //0x110
    volatile CK_REG    NIPRL1;   //0x114
    volatile CK_REG    FIERL1;   //0x118
    volatile CK_REG    FIPRL1;   //0x11c
    volatile CK_REG    IFRH1;    //0x120
    volatile CK_REG    IPRH1;    //0x124
    volatile CK_REG    NIERH1;   //0x128
    volatile CK_REG    NIPRH1;   //0x12c
    volatile CK_REG    FIERH1;   //0x130
    volatile CK_REG    FIPRH1;   //0x134
    volatile CK_REG    Rev4[2];  //0x138 - 0x13c
    volatile CK_REG    PR1[16];  //0x140 - 0x17f
    volatile CK_REG    Rev5[4];  //0x180 - 0x18f
    volatile CK_REG    IMASKRL1; //0x190
    volatile CK_REG    IMASKRH1; //0x194
}CKStruct_INTC, *PCKStruct_INTC;


#define PCK_INTC    ((PCKStruct_INTC)CK_INTC_BASEADDRESS)


/*
 *  Bit Definition for the PIC Interrupt control register
 */
#define ICR_AVE   0x80000000  /* Select vectored interrupt */
#define ICR_FVE   0x40000000  /* Unique vector number for fast vectored*/
#define ICR_ME    0x20000000  /* Interrupt masking enabled */
#define	ICR_MFI	  0x10000000  /* Fast interrupt requests masked by MASK value */

/*
 * NR_IRQS: The number of member in normal_irq_list and fast_irq_list
 *          respective.It is equal to the number of priority levels.
 */
#if CK_128_INTC
#define NR_IRQS             128
#else
#define NR_IRQS             64
#endif
#define IRQ_MAX_TEST_NUM    10

#endif
