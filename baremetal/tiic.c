/*****************************************************************************
 *  File: tiic.c
 *
 *  Descirption: contains the functions support Atmel AT24C64.
 *
 *  Copyright (C) : 2018 VeriSilicon.
 *
 *  Author: Lu Gao
 *  Mail:   lu.gao@verisilicon.com
 *  Date:   Mar 16 2018
 *
 *****************************************************************************/
#include "datatype.h"
#include "iic.h"
#include "at24c64.h"

void CK_I2C_Test(CK_UINT8 i2c_id) {
    CK_UINT8 ret = 0;
    //CK_UINT8 i2c_id = 2;
    CK_UINT32 byte_write_addr = 0x50;
    CK_UINT32 page_write_addr = 0x60;
    CK_UINT8 byte_write_data[2] = {0x16, 0x18};
    CK_UINT8 byte_receive_data = 0;
    CK_UINT8 page_write_data[2][20] = {{42, 90, 70, 183, 242, 51, 232, 117,
                                        93, 248, 162, 77, 28, 122, 81, 136,
                                        86, 163, 217, 216},
                                       {203, 154, 253, 57, 12, 159, 199,
                                        13, 225, 181, 35, 52, 77, 127,
                                        251, 196, 184, 230, 6, 11}};
    CK_UINT8 page_receive_data[20] = {0};
    CK_UINT8 i = 0;
    CK_UINT8 j = 0;
    CK_UINT32 speed_supported[3] = {I2C_STANDARD_SPEED,
                                    I2C_FAST_SPEED,
                                    I2C_FAST_PLUS_SPEED};
    CK_UINT32 speed = 0;
    CK_UINT8 int_en[2] = {0, 1};
    CK_UINT8 int_id = 0;
    CK_UINT8 status = 0;

    dw_i2c_init(i2c_id, I2C_STANDARD_SPEED, 1);
    for (int_id = 0; int_id < 2; int_id++) {
        for (speed = 0; speed < 3; speed++) {
            printf("I2C %d test in %s with speed %s. . .\n",
                i2c_id,
                int_id == 0 ? "polling mode" : "interrupt mode",
                speed == 0 ? "standard" : (speed == 1 ? "fast" : "fast plus"));
            dw_i2c_set_bus_speed(i2c_id, speed_supported[speed]);
            for (i = 0; i < 2; i++) {
                // do byte write and current address read
                ret = byte_write(i2c_id, byte_write_addr,
                                    byte_write_data[i], int_en[int_id]);
                if (ret != 0) {
                    printf("failed byte writing of AT24C64 at 0x%x \n",
                            byte_write_addr);
                    printf("                - - - FAIL.\n");
                    status = 1;
                    continue;
                }
                // write 0 to the byte_write_addr - 1
                ret = byte_write(i2c_id, byte_write_addr - 1, 0, int_en[int_id]);
                if (ret != 0) {
                    printf("failed byte writing of AT24C64 at 0x%x \n",
                            byte_write_addr);
                    printf("                - - - FAIL.\n");
                    status = 1;
                    continue;
                }

                // current address read will be on byte_write_addr
                ret = current_addr_read(i2c_id, &byte_receive_data,
                                        1, int_en[int_id]);
                if (ret != 0) {
                    printf("failed current address reading"
                            " of AT24C64 at 0x%x \n", byte_write_addr);
                    printf("                - - - FAIL.\n");
                    status = 1;
                    continue;
                }
                if (byte_receive_data != byte_write_data[i]) {
                    printf("read data byte 0x%x not equal to write data"
                            "byte 0x%x at address 0x%x \n", byte_receive_data,
                            byte_write_data[i], byte_write_addr);
                    printf("                - - - FAIL.\n");
                    status = 1;
                    continue;
                }

                // do page write and sequential read with random read
                ret = page_write(i2c_id, page_write_addr,
                                    page_write_data[i],
                                    sizeof(page_write_data[i]), int_en[int_id]);
                if (ret != 0) {
                    printf("failed page writing of AT24C64 from 0x%x \n",
                            page_write_addr);
                    printf("                - - - FAIL.\n");
                    status = 1;
                    continue;
                }
                ret = random_read(i2c_id, page_write_addr,
                                    page_receive_data,
                                    sizeof(page_write_data[i]), int_en[int_id]);
                if (ret != 0) {
                    printf("failed random reading of AT24C64 from 0x%x \n",
                            page_write_addr);
                    printf("                - - - FAIL.\n");
                    status = 1;
                    continue;
                }

                for (j = 0; j < sizeof(page_write_data[i]); j++) {
                    if (page_receive_data[j] != page_write_data[i][j]) {
                        printf("random read data 0x%x not equal to write data"
                                " 0x%x at address 0x%x \n",
                                page_receive_data[j],
                                page_write_data[i][j],
                                page_write_addr + i);
                        printf("                - - - FAIL.\n");
                        status = 1;
                        continue;
                    }
                }
            }
        }
    }
    if (status == 0) {
        printf("                - - - PASS.\n");
    }
}
