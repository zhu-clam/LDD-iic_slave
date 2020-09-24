/*****************************************************************************
 *  File: at24cm02.h
 *
 *  Descirption: contains the functions support Atmel AT24CM02.
 *
 *  Copyright (C) : 2018 VeriSilicon.
 *
 *  Author: Lu Gao
 *  Mail:   lu.gao@verisilicon.com
 *  Date:   Mar 16 2018
 *
 *****************************************************************************/
#include "datatype.h"
// bit 3 depends on A2, bit 2 and bit 1
// depends on the data address bit 17 - 16
#define AT24CM02_A2             0
#define AT24CM02_SLAVE_ADDR     ((0xa0 | (AT24CM02_A2 << 3)) >> 1)
#define AT24CM02_DEV_ADDR(addr) (AT24CM02_SLAVE_ADDR | ((addr & 0x30000) >> 16))

#define AT24C64_A210             0
#define AT24C64_SLAVE_ADDR     ((0xA0 >> 1) | AT24C64_A210)

#define WORD_ADDR_L(addr)       (addr & 0xffff)
#define WORD_ADDR_L_LEN         2

CK_UINT32 byte_write(CK_UINT32 i2c_id, CK_UINT32 addr,
                        CK_UINT8 data, CK_UINT8 int_en);
CK_UINT32 page_write(CK_UINT32 i2c_id, CK_UINT32 addr,
                        CK_UINT8 *buf, CK_UINT16 len, CK_UINT8 int_en);
CK_UINT32 current_addr_read(CK_UINT32 i2c_id, CK_UINT8 *buf,
                            CK_UINT32 len, CK_UINT8 int_en);
CK_UINT32 random_read(CK_UINT32 i2c_id, CK_UINT32 addr, CK_UINT8 *buf,
                        CK_UINT32 len, CK_UINT8 int_en);
