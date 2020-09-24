/*
 * entry.S:
 * CKCORE exception, normal interrupt and fast interrupt entries.
 *
 * Copyright (C) 2012  Hangzhou C-SKY Microsystems Co., Ltd
 * Modify by liu jirang  on 2012-09-11
 */

#include "platform.h"

.global default_exception_handler
/*
 * default exception service routine, before r0 initialized
 */
    .text
    .balign 4
default_exception_handler:
    br    default_exception_handler
    rte



