/*
 *  intr.h - Define the struct and interface for interrupt controller
 *
 *  Copyright (C):  2008 Hangzhou C-SKY Microsystems Co.,LTD.
 *  Author: Dongqi Hu  (dongqi_hu@c-sky.com)
 *          Jianyong Jiang  (jianyong_jiang@c-sky.com)
 *  Contributiors: Chunqiang Li
 *  Date: 2008-09-26
 *  Modify by liu jirang  on 2012-09-11
 */

#ifndef _INTR_H_
#define _INTR_H_

#include "datatype.h"

//Interrupt mode
#define AUTO_MODE           0
#define VECTOR_SHARE_MODE   1
#define VECTOR_UNIQUE_MODE  2
#define CKCORE_VECTOR_SYS  32
#define CKCORE_VECTOR_AUTOVEC 10
#define CKCORE_VECTOR_FASTAUTOVEC  11
#define CKCORE_VECTOR_TLBMISS 14
#define CKCORE_VECTOR_FASTVEC 96

/* define the data structure of interrupt description */
typedef struct CKS_IRQ_Handler{
    char        *devname;
    CK_UINT32   irqid;
    CK_UINT32    priority;
    void        (*handler)(CK_UINT32 irqid);
    BOOL        bfast;
    struct CKS_IRQ_Handler  *next;
}CKStruct_IRQHandler, *PCKStruct_IRQHandler;

typedef struct {
    CK_UINT32 id;               /* the number of INTC test */
    CK_UINT32 irq;              /* the interrupt number of timer */
    BOOL bfast;                 /* fast interrupt or not */
    CKStruct_IRQHandler irqhandler;
} CKStruct_INTCTestInfo, * PCKStruct_INTCTestInfo;

// VSR table
extern  volatile unsigned int ckcpu_vsr_table[256];
/* Statement of those functions which are used in intc.c*/
void CK_CPU_EnAllNormalIrq(void);
void CK_CPU_DisAllNormalIrq(void);
void CK_CPU_EnAllFastIrq(void);
void CK_CPU_DisAllFastIrq(void);
void CK_CPU_EnterCritical(CK_UINT32 *psr);
void CK_CPU_ExitCritical(CK_UINT32 psr);
void CK_INTC_EnNormalIrq(IN CK_UINT32 priority);
void CK_INTC_DisNormalIrq(IN CK_UINT32 priority);
void CK_INTC_EnFastIrq(IN CK_UINT32 priority);
void CK_INTC_DisFastIrq(IN CK_UINT32 priority);
void CK_INTC_MaskNormalIrq(IN CK_UINT32 primask);
void CK_INTC_UnMaskNormalIrq(IN CK_UINT32 primask);
void CK_INTC_MaskFastIrq(IN CK_UINT32 primask);
void CK_INTC_UnMaskFastIrq(IN CK_UINT32 primask);
CK_INT32 CK_INTC_RequestIrq(PCKStruct_IRQHandler priqhandler, IN CK_UINT32 mode);
CK_INT32 CK_INTC_FreeIrq(INOUT PCKStruct_IRQHandler priqhandler, IN CK_UINT32 mode);
void CK_INTC_Init(IN CK_UINT32 mode);
void CK_Exception_Init (void);


/* save context */
#define IRQ_HANDLER_START()  \
asm (  \
    "subi    sp, 124\n\t"   \
    "stw     r0, (sp, 0)\n\t"   \
    "stw     r1, (sp, 4)\n\t"   \
    "stw     r2, (sp, 8)\n\t"   \
    "stw     r3, (sp, 12)\n\t"   \
    "stw     r4, (sp, 16)\n\t"   \
    "stw     r5, (sp, 20)\n\t"   \
    "stw     r6, (sp, 24)\n\t"   \
    "stw     r7, (sp, 28)\n\t"   \
    "stw     r8, (sp, 32)\n\t"   \
    "stw     r9, (sp, 36)\n\t"   \
    "stw     r10, (sp, 40)\n\t"   \
    "stw     r11, (sp, 44)\n\t"   \
    "stw     r12, (sp, 48)\n\t"   \
    "stw     r13, (sp, 52)\n\t"   \
    "stw     r15, (sp, 56)\n\t"   \
    "stw     r16, (sp, 60)\n\t"   \
    "stw     r17, (sp, 64)\n\t"   \
    "stw     r18, (sp, 68)\n\t"   \
    "stw     r19, (sp, 72)\n\t"   \
    "stw     r20, (sp, 76)\n\t"   \
    "stw     r21, (sp, 80)\n\t"   \
    "stw     r22, (sp, 84)\n\t"   \
    "stw     r23, (sp, 88)\n\t"   \
    "stw     r24, (sp, 92)\n\t"   \
    "stw     r25, (sp, 96)\n\t"   \
    "stw     r26, (sp, 100)\n\t"   \
    "stw     r27, (sp, 104)\n\t"   \
    "stw     r28, (sp, 108)\n\t"   \
    "stw     r29, (sp, 112)\n\t"   \
    "stw     r30, (sp, 116)\n\t"   \
    "stw     r31, (sp, 120)\n\t"   \
\
    "subi    sp, 8\n\t"   \
    "mfcr    r2, epsr\n\t"   \
    "stw     r2, (sp,4)\n\t"   \
    "mfcr    r2, epc\n\t"   \
    "stw     r2, (sp,0)\n\t"   \
)

/*   Restore the psr and pc     */
#define IRQ_HANDLER_END()   \
asm (   \
    "ldw     r2, (sp,0)\n\t"  \
    "mtcr    r2, epc\n\t"  \
    "ldw     r2, (sp,4)\n\t"  \
    "mtcr    r2, epsr\n\t"  \
    "addi    sp, 8\n\t"  \
\
    "ldm     r0-r13,(sp)\n\t"  \
    "addi    sp,56\n\t"  \
    "ldm     r15-r31,(sp)\n\t"  \
    "addi    sp,68\n\t"  \
\
    "rte"  \
)

/*
 * Fast interrupt vector handler.
 */

/* do nothing */
#define FIQ_HANDLER_START()\
asm (  \
"subi    sp, 124\n\t"   \
    "stw     r0, (sp, 0)\n\t"   \
    "stw     r1, (sp, 4)\n\t"   \
    "stw     r2, (sp, 8)\n\t"   \
    "stw     r3, (sp, 12)\n\t"   \
    "stw     r4, (sp, 16)\n\t"   \
    "stw     r5, (sp, 20)\n\t"   \
    "stw     r6, (sp, 24)\n\t"   \
    "stw     r7, (sp, 28)\n\t"   \
    "stw     r8, (sp, 32)\n\t"   \
    "stw     r9, (sp, 36)\n\t"   \
    "stw     r10, (sp, 40)\n\t"   \
    "stw     r11, (sp, 44)\n\t"   \
    "stw     r12, (sp, 48)\n\t"   \
    "stw     r13, (sp, 52)\n\t"   \
    "stw     r15, (sp, 56)\n\t"   \
    "stw     r16, (sp, 60)\n\t"   \
    "stw     r17, (sp, 64)\n\t"   \
    "stw     r18, (sp, 68)\n\t"   \
    "stw     r19, (sp, 72)\n\t"   \
    "stw     r20, (sp, 76)\n\t"   \
    "stw     r21, (sp, 80)\n\t"   \
    "stw     r22, (sp, 84)\n\t"   \
    "stw     r23, (sp, 88)\n\t"   \
    "stw     r24, (sp, 92)\n\t"   \
    "stw     r25, (sp, 96)\n\t"   \
    "stw     r26, (sp, 100)\n\t"   \
    "stw     r27, (sp, 104)\n\t"   \
    "stw     r28, (sp, 108)\n\t"   \
    "stw     r29, (sp, 112)\n\t"   \
    "stw     r30, (sp, 116)\n\t"   \
    "stw     r31, (sp, 120)\n\t"   \
)

/*   Restore the psr and pc     */
#define FIQ_HANDLER_END()  \
asm (   \
    "ldm     r0-r13,(sp)\n\t"  \
    "addi    sp,56\n\t"  \
    "ldm     r15-r31,(sp)\n\t"  \
    "addi    sp,68\n\t"  \
\
	"rfi"   \
)

/*
 * return the irq ID
 */
#define IRQ_ID()  \
({  \
	int  irqid;   \
	asm ( "mfcr		%0, psr\n\t"   \
		  "lsri		%0, 16\n\t"   \
		  "sextb	%0\n\t"   \
		  : "=&r" (irqid)  \
		  :   \
	); \
	irqid; \
})

#endif
