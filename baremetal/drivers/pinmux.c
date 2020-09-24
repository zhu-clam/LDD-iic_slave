/*
 * [Description] PIN MUX driver
 *
 * Maintainer: jianheng <zhang.jianheng@verisilicon.com>
 *
 * Copyright (C) 2019 Verisilicon Inc.
 *
 */

#include "pinmux.h"

void pinmux_set_func(int pin_offs, int func)
{
	u32 reg = read_mreg32(CK_PINMUX_Control + pin_offs);
	reg = (reg & PMUX_FUNC_MASK) | (func & 0x3);
	write_mreg32(CK_PINMUX_Control + pin_offs, reg);
}
