/*
 * crt0.S -- startup codes for CK-CPU.
 *
 * Define a exception vector table
 *
 * Initial PSR register.
 * Initial VBR register, relocate exception vector table.
 * Initial sp(stack pointer) register.
 * Initial MMU (for ckcpu having MMU)
 * Initial Cache.
 * Initial BSS section to "0"
 * Then goto "main".
 *
 * Copyright (C) 2012  Hangzhou C-SKY Microsystems Co., Ltd
 */

#include "platform.h"

.file "crt0.S"


/*
 * For importing variable or functions from script file "ckcpu.ld".
 */
.import __stack     /* The stack pointer from linking script file */
.import __sbss      /* The end of bss from linking script file */
.import __ebss      /* The end of bss from linking script file */

/*
 * For importing variable or functions from other c or assemble files.
 */
.import main        /* The c codes entry */

/*
 * For importing variable or functions from entry.S.
 */
.import default_exception_handler /* default exception service routine */

/* For exporting varialbe or functions */
.export ckcpu_vsr_table     /* Vector table base address. */
.export __start             /* The system entry. */


.section .exp_table
/* Vector table space. */

.align 10
ckcpu_vsr_table:
    .long   __start
.align 2
.rept 255
    .long   default_exception_handler
.endr


/* The ckcpu startup codes. */
.text
    /*
     * This is the codes first entry point. This is where it all
     * begins...
     */

__start:
    /*
     * Init psr value, enable exception, disable interrupt and fast interrupt.
     * psr = 0x80000100
     */
    bgeni   a3, 31
    bseti   a3, 8
    mtcr    a3, psr

    /*
     * Setup initial vector base table for interrupts and exceptions
     */
    lrw     a2, ckcpu_vsr_table
    mtcr    a2, vbr

    /* Initialize the normal stack pointer from the linker definition. */
    lrw     a3, __stack
    mov     sp, a3

    /* Disable CKCPU MMU */
    movi    a3, 0x0
    mtcr    a3, cr18

__load_data:
    /* load data section to sram */
    lrw     a0, __erodata
    lrw     a1, __sdata
    lrw     a2, __edata
1:
    cmpne   a1, a2
    bf      __clear_bss

    ldw     a3, (a0, 0)
    stw     a3, (a1, 0)
    addi    a0, 4
    addi    a1, 4
    br      1b
    /*
     * Zero out the bss region.
     * NOTE: __sbss must align 4
     */
__clear_bss:
    lrw     a3, __sbss  /* Get start of bss from linking script file */
    lrw     a2, __ebss  /* Get end of bss from linking script file */
    subu    a2, a3      /* Calculate size of bss */
    lsri    a2, 2       /* Size of whole words */
    cmpnei  a2, 0
    bf      __goto_c
    movi    a1, 0       /* Set zero value to write */
2:
    stw     a1, (a3)    /* Zero next word */
    addi    a3, 4       /* Increase bss pointer */
    decne   a2          /* Decrease counter */
    bt      2b          /* Repeat for all bss */

    /*
     * Assember start up done, C codes start here.
     */
__goto_c:
    bsr     main

    /* Should never get here. */
__exit:
    bkpt
