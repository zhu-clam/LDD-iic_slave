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

#include "ck810.h"

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

    /*
     * Step 1: Setup the initial page tables.
     */
__create_page_table:
    /*
     * invalid the jTLB
     */
    bgeni   a0, 26
    mtcr    a0, cr<8,15>

    /* Virtual address */
    lrw     a3, __origin
    mtcr    a3, cr<4,15>    // MEH

    /* Page size 16M */
    bmaski  a2, 12
    lsli    a2, 13
    mtcr    a2, cr<6,15>    // MPR

    /* MEL0 */
    movi    a1, 0x1e        // C = 011, D = 1, V = 1, G = 0, PFN = 0x0
    or      a1, a3          // Set PFN
    mtcr    a1, cr<2,15>    // MEL0

    /* MEL1 */
    /* movi    a2, 0x101e */
    movi    a2, 0x1e        // C = 011, D = 1, V = 1, G = 0, PFN = 0x0
    or      a2, a3          // Set PFN
    bseti   a2, 24          // Add size of page
    mtcr    a2, cr<3,15>    // MEL1

    /* rand write */
    bgeni   a0, 28
    mtcr    a0, cr<8,15>    // MCIR

    /* update MMU SSEEG1 space
     * Virtual Address:  0xa0000000 - 0xc0000000
     * Physical Address: 0xe0000000 - 0xffffffff
     */
    movi    a0, 0x0
    bseti   a0, 29
    bseti   a0, 30
    bseti   a0, 31

    movi    a1, 0x26      // SO = 1, V = 1, D = 1
    or      a0, a1
    mtcr    a0, cr<31,15> // Set MSA1

    mfcr    a3, cr23
    bclri   a3, 3       // disable L2 Cache
    mtcr    a3, cr23

#if CONFIG_CKCPU_L2CACHE
    // ADD by C-SKY
    // 1. L2 invalid and enable
    l2cache.iall
    mfcr    r2, cr23
    bseti   r2, 3
    bseti   r2, 0
    mtcr    r2, cr23

    // 2. BHT invalid and enable
    mfcr    r2, cr17
    bseti   r2, 16
    mtcr    r2, cr17
    mfcr    r2, cr18
    bseti   r2, 6
    mtcr    r2, cr18

    // 3. BTB invalid and enable
    mfcr    r2, cr17
    bseti   r2, 17
    mtcr    r2, cr17
    mfcr    r2, cr18
    bseti   r2, 11
    mtcr    r2, cr18

    // 4. IND_BTB invalid and enable
    mfcr    r2, cr17
    bseti   r2, 18
    mtcr    r2, cr17
    mfcr    r2, cr18
    bseti   r2, 16
    mtcr    r2, cr18

    // 5. RAS enable
    mfcr    r2, cr18
    bseti   r2, 5
    mtcr    r2, cr18 

    // 6. DCACHE prefetch    
    mfcr    r2, cr31
    bseti   r2, 13
    bseti   r2, 14
    bseti   r2, 2
    mtcr    r2, cr31

    // 7. enable Write Back and Write Burst
    mfcr    r2, cr<18, 0>
    bseti   r2, 4
    bseti   r2, 14
    mtcr    r2, cr<18, 0>
#endif /* CONFIG_CKCPU_L2CACHE */

    /*
     * Step 2: Enable MMU
     */
#if CONFIG_CKCPU_MMU
    mfcr    a3, cr18
    bseti   a3, 0       // bit 0~1, MMU enable
    bclri   a3, 1
    bseti   a3, 4       // write back
    bseti   a3, 5       // return stack
    bseti   a3, 6       // jumping predict
    mtcr    a3, cr18
#endif /* CONFIG_CKCPU_MMU */

    /*
     * Step 3: enable instruction & data cache
     */
#if CONFIG_CKCPU_ICACHE
    /* Invalid Instruction cache content */
    movi    a3, 0x11
    mtcr    a3, cr17

    /* Enable instruction cache */
    mfcr    a2, cr18
    bseti   a2, 2
    mtcr    a2, cr18

    /* Enable instruction cache prefetch */
    mfcr    a0, cr31
    bseti   a0, 8
    mtcr    a0, cr31
#endif /* CONFIG_CKCPU_ICACHE */

#if CONFIG_CKCPU_DCACHE
    /* Invalid Data cache content */
    movi    a3, 0x12
    mtcr    a3, cr17

    /* Enable data cache */
    mfcr    a3, cr18
    bseti   a3, 3
    mtcr    a3, cr18

    /* Enable data cache prefetch */
    mfcr    a0, cr31
    bseti   a0, 2
    mtcr    a0, cr31
#endif /* CONFIG_CKCPU_DCACHE */

    /*
     * Zero out the bss region.
     * NOTE: __sbss must align 4
     */
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
