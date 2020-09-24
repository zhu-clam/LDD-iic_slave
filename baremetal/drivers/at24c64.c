/*****************************************************************************
 *  File: at24c64.c
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
#include "at24c64.h"
#include "datatype.h"
#include "misc.h"

extern CK_UINT32 dw_i2c_write(CK_UINT32 id, CK_UINT32 dev, CK_UINT32 addr,
            CK_UINT32 alen, CK_UINT8 *buffer, CK_UINT32 len);
extern CK_UINT32 dw_i2c_write_int(CK_UINT32 id, CK_UINT32 dev, CK_UINT32 addr,
            CK_UINT32 alen, CK_UINT8 *buffer, CK_UINT32 len);
extern CK_UINT32 dw_i2c_read(CK_UINT32 id, CK_UINT32 dev, CK_UINT32 addr,
               CK_UINT32 alen, CK_UINT8 *buffer, CK_UINT32 len);
extern CK_UINT32 dw_i2c_read_int(CK_UINT32 id, CK_UINT32 dev, CK_UINT32 addr,
               CK_UINT32 alen, CK_UINT8 *buffer, CK_UINT32 len);

CK_UINT32 byte_write(CK_UINT32 i2c_id, CK_UINT32 addr,
                        CK_UINT8 data, CK_UINT8 int_en) {
    CK_UINT32 ret = 0;
    if (int_en == 0) {
        ret = dw_i2c_write(i2c_id, AT24C64_SLAVE_ADDR, WORD_ADDR_L(addr),
                            WORD_ADDR_L_LEN, &data, 1);
    } else {
        ret = dw_i2c_write_int(i2c_id, AT24C64_SLAVE_ADDR,
                                WORD_ADDR_L(addr), WORD_ADDR_L_LEN, &data, 1);
    }
    return ret;
}

CK_UINT32 page_write(CK_UINT32 i2c_id, CK_UINT32 addr, CK_UINT8 *buf,
                        CK_UINT16 len, CK_UINT8 int_en) {
    CK_UINT32 ret = 0;

    if (((addr & 0xff) + len) > 256) {
        printf("can't do page write from 0x%x with %d bytes,"
                "out of page boundary", addr, len);
        return 1;
    }
    if (int_en == 0) {
        ret = dw_i2c_write(i2c_id, AT24C64_SLAVE_ADDR, WORD_ADDR_L(addr),
                            WORD_ADDR_L_LEN, buf, len);
    } else {
        ret = dw_i2c_write_int(i2c_id, AT24C64_SLAVE_ADDR,
                                WORD_ADDR_L(addr), WORD_ADDR_L_LEN, buf, len);
    }
    return ret;
}

CK_UINT32 current_addr_read(CK_UINT32 i2c_id, CK_UINT8 *buf,
                            CK_UINT32 len, CK_UINT8 int_en) {
    CK_UINT32 ret = 0;

    if (int_en == 0) {
        // we don't need data address in current address read mode
        ret = dw_i2c_read(i2c_id, AT24C64_SLAVE_ADDR, 0, 0, buf, len);
    } else {
        ret = dw_i2c_read_int(i2c_id, AT24C64_SLAVE_ADDR, 0, 0, buf, len);
    }
    return ret;
}

CK_UINT32 random_read(CK_UINT32 i2c_id, CK_UINT32 addr, CK_UINT8 *buf,
                        CK_UINT32 len, CK_UINT8 int_en) {
    CK_UINT32 ret = 0;

    if (int_en == 0) {
        ret = dw_i2c_read(i2c_id, AT24C64_SLAVE_ADDR, WORD_ADDR_L(addr),
                            WORD_ADDR_L_LEN, buf, len);
    } else {
        ret = dw_i2c_read_int(i2c_id, AT24C64_SLAVE_ADDR,
                                WORD_ADDR_L(addr), WORD_ADDR_L_LEN, buf, len);
    }
    return ret;
}
