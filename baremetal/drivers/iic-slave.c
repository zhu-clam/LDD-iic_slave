/*****************************************************************************
 *  File: iic-slave.c
 *
 *  Descirption: contains the functions support I2C slave.
 *
 *  Copyright (C) : 2019 VeriSilicon.
 *
 *  Author: Lu Gao
 *  Mail:   lu.gao@verisilicon.com
 *  Date:   Jan. 28 2019
 *
 *****************************************************************************/

#include "datatype.h"
#include "iic.h"
#include "ckiic.h"

#define I2C_TEST_SLAVE		0
#define I2C_TEST_MASTER		1

volatile int iic_slave_intr_flag = 1;

extern CKStruct_I2CInfo CK_I2C_Table[];

CK_UINT8 rd_data[8] = {15, 223, 236, 94, 116, 83, 139, 34};
CK_UINT8 wr_data[8] = {192, 12, 237, 227, 116, 186, 65, 104};

//RD_REG bit 只读寄存器,当i2c 作为从设备且当i2c master尝试从该i2c读取数据时被置1.

void i2c_slave_handle(CK_UINT32 irq) {
    int tmp;
    CK_UINT8 i = 0;
    PCKPStruct_I2C pi2c = NULL;
    pi2c = CK_I2C_Table[irq - CK_INTC_I2C0].addr;

    tmp = pi2c->ic_intr_stat;
	/*This bit is set to 1 when DW_apb_i2c is acting as a slave and another I2C
master is attempting to read data from DW_apb_i2c*/
    if ((tmp & IC_RD_REQ) != 0) {
        printf("send out data per read request from master\n");
        for (i = 0; i < 8; i++) {
            pi2c->ic_cmd_data = rd_data[i];
        }
        tmp = pi2c->ic_clr_rd_req;//清除IC_RD_REQ 中断
    } else if ((tmp & IC_RX_FULL) != 0) {//当接受数据RX_FULL 中断被处罚时
        printf("receive data for write from master\n");
        for (i = 0; i < 8; i++) {
            tmp = (pi2c->ic_cmd_data & 0xff);
			printf(" %d\n",tmp);
            if (tmp != wr_data[i]) {
                printf("\tslave received %d: %d is not equal to wr_data %d\n",
                        i, tmp, wr_data[i]);
                printf("                - - - FAIL.\n");
                exit(1);
            }
        }
        tmp = pi2c->ic_clr_intr;
        iic_slave_intr_flag=0;
    }
}

//I2C 从设备初始化.
int dw_i2c_slave_init(CK_UINT32 id) {
    PCKStruct_I2CInfo info = NULL;
    PCKPStruct_I2C pi2c = NULL;
    info = &(CK_I2C_Table[id]);
    pi2c = info->addr;

    pi2c->ic_enable &= ~IC_ENABLE_0B;//step1. DISABLE i2c
    while ((pi2c->ic_enable_status & IC_ENABLE_0B) != 0);//step2.确认deem i2c is inactive.

    pi2c->ic_rx_tl = 7; // 8 bytes to trigger 控制I2C 触发 RX_FULL 中断的阈值水平.
    pi2c->ic_tx_tl = 0x0; // 0 bytes to trigger I2C TX_EMPTY

    pi2c->ic_con = IC_CON_SPD_HS;//设置high speed mode
    pi2c->ic_intr_mask = 0; //中断屏蔽寄存器
    pi2c->ic_sar = 0xb; //i2c slave init define ic_sar = 0xb.

    info->irqhandler.devname = "I2C-SLAVE";
    info->irqhandler.irqid = info->irq;
    info->irqhandler.priority = info->irq;
    info->irqhandler.handler = i2c_slave_handle;
    info->irqhandler.bfast = FALSE;
    info->irqhandler.next = NULL;
    /* register timer isr */
    CK_INTC_RequestIrq(&(info->irqhandler), AUTO_MODE);

    pi2c->ic_enable = IC_ENABLE_0B;
}

void CK_I2C_Slave_Test(void) {
    CK_UINT32 ret = 0;
    CK_UINT8 i = 0;
    CK_UINT32 get = 0;
    CK_UINT8 recv_data[8] = {0};
    // we use i2c0 as slave
    PCKPStruct_I2C pi2c_s = NULL;
    pi2c_s = CK_I2C_Table[I2C_TEST_SLAVE].addr;

    dw_i2c_slave_init(I2C_TEST_SLAVE);//i2c slave 初始化时,进行了中断注册.
    dw_i2c_set_bus_speed(I2C_TEST_SLAVE, I2C_MAX_SPEED);

    // we use i2c3 as master
    dw_i2c_init(I2C_TEST_MASTER, I2C_MAX_SPEED, 1);

    printf("For this test to run correctly, connect\n"
            " EVB board TP109 - EVB Board TP112,"
            " EVB board TP110 - EVB Board TP111!\n");
    printf("connection done, continue? - - - [y/n] ");
    while(1) {
        get = CK_WaitForReply();
        if (get == 1) {
            break;
        } else if (get == 0) {
            return;
        } else {
            printf("\n\tPlease enter 'y' or 'n'");
        }
    }

    printf("\ntest start!\n");
    pi2c_s->ic_intr_mask |= (IC_RD_REQ | IC_RX_FULL); //unmask RX FIFO FULL interrupt

    printf("i2c slave %d waiting for data write from master . . .\n", I2C_TEST_SLAVE);
    // i2c1 write to slave
    ret = dw_i2c_write(I2C_TEST_MASTER, 0xb, 0, 0, &wr_data, 8);
    if (ret != 0) {
        printf("failed writing to slave\n");
        printf("                - - - FAIL.\n");
        return;
    }

    while (iic_slave_intr_flag != 0);
    printf("i2c slave %d got all data from master!\n", I2C_TEST_SLAVE);

    printf("i2c slave %d waiting for data read from master . . .\n", I2C_TEST_SLAVE);
    // i2c0 read from slave
    ret = dw_i2c_read(I2C_TEST_MASTER, 0xb, 0, 0, &recv_data, 8);
    if (ret != 0) {
        printf("failed reading from slave\n");
        printf("                - - - FAIL.\n");
        return;
    }

    for (i = 0; i < 8; i ++) {
        if (recv_data[i] != rd_data[i]) {
            printf("\tmaster received %d: %d is not equal to rd_data %d\n",
                    i, recv_data[i], rd_data[i]);
            printf("                - - - FAIL.\n");
            return;
        }
    }

    printf("i2c slave %d sent all data to master!\n", I2C_TEST_SLAVE);
    printf("                - - - PASS.\n");
}
