/*****************************************************************************
 *  File: iic.c
 *
 *  Descirption: contains the functions support Synopsys I2C Controller.
 *
 *  Copyright (C) : 2018 VeriSilicon.
 *
 *  Author: Chunming Li
 *  Mail:   chunming.li@verisilicon.com
 *  Date:   Feb 11 2018
 *
 *****************************************************************************/

#include "ck810.h"
#include "iic.h"
#include "ckiic.h"
#include "intc.h"
#include "datatype.h"
#include "ahbdma.h"

CKStruct_I2CInfo CK_I2C_Table[] = {
    {0, (PCKPStruct_I2C)CK_I2C0_BASSADDR, CK_INTC_I2C0, FALSE},
    {1, (PCKPStruct_I2C)CK_I2C1_BASSADDR, CK_INTC_I2C1, FALSE},
    {2, (PCKPStruct_I2C)CK_I2C2_BASSADDR, CK_INTC_I2C2, FALSE},
    {3, (PCKPStruct_I2C)CK_I2C3_BASSADDR, CK_INTC_I2C3, FALSE},
};

// used for transmitting and receiving in interrupt service
#define FIFO_DEPTH  64
CK_UINT8 *tx_buf = NULL;
CK_UINT8 *rx_buf = NULL;
CK_UINT32 tx_rx_len = 0;
CK_UINT32 tx_len = 0;
CK_UINT32 rx_len = 0;

static CK_UINT32 i2c_dw_read_clear_intrbits(CK_UINT32 id) {
    CK_UINT32 stat = 0;
    CK_UINT32 tmp32 = 0;
    PCKPStruct_I2C pi2c = NULL;

    pi2c = CK_I2C_Table[id].addr;

    stat = pi2c->ic_raw_intr_stat;

    if ((stat & IC_RX_UNDER) != 0) {
        tmp32 = pi2c->ic_clr_rx_under;
    }
    if ((stat & IC_RX_OVER) != 0) {
        tmp32 = pi2c->ic_clr_rx_over;
    }
    if ((stat & IC_TX_OVER) != 0) {
        tmp32 = pi2c->ic_clr_tx_over;
    }
    if ((stat & IC_RD_REQ) != 0) {
        tmp32 = pi2c->ic_clr_rd_req;
    }
    if ((stat & IC_TX_ABRT) != 0) {
        tmp32 = pi2c->ic_tx_abrt_source;
        tmp32 = pi2c->ic_clr_tx_abrt;
    }
    if ((stat & IC_RX_DONE) != 0) {
        tmp32 = pi2c->ic_clr_rx_done;
    }
    if ((stat & IC_ACTIVITY) != 0) {
        tmp32 = pi2c->ic_clr_activity;
    }
    if ((stat & IC_START_DET) != 0) {
        tmp32 = pi2c->ic_clr_start_det;
    }
    if ((stat & IC_GEN_CALL) != 0) {
        tmp32 = pi2c->ic_clr_gen_call;
    }

    return stat;
}

/*
 * Interrupt service routine. This gets called whenever an I2C interrupt
 * occurs.
 */
void i2c_handle_common(CK_UINT32 id) {
    CK_UINT32 stat = 0;
    CK_UINT32 enabled = 0;
    PCKPStruct_I2C pi2c = NULL;

    pi2c = CK_I2C_Table[id].addr;

    enabled = pi2c->ic_enable_status & IC_ENABLE_0B;
    stat = pi2c->ic_raw_intr_stat;
    //printf("I2C %d enabled = 0x%x   stat = 0x%x \n", id, enabled, stat);
    if ((!enabled) || (!(stat & (~IC_ACTIVITY)))) {
        printf("I2C %d is not enabled, or no inerrupt issued\n", id);
        return;
    }
    stat = i2c_dw_read_clear_intrbits(id);

    // do tx/rx in interrupt service if we enable tx_empty and rx_full interrupt
    stat =  pi2c->ic_intr_stat;
    // receive
    if ((stat & IC_RX_FULL) || (stat & IC_STOP_DET)) {
        // check STOP_DET for receiving, because there may be number of data
        // less than ic_rx_tl, which means it is not able to trigger RX_FULL
        // interrupt
        while ((rx_len < tx_rx_len) &&
                ((pi2c->ic_status & IC_STATUS_RFNE) != 0)) {
            rx_buf[rx_len] = pi2c->ic_cmd_data;
            rx_len++;
        }
        if (rx_len == tx_rx_len) {
            // close RX_FULL interrupt after receiving all data
            pi2c->ic_intr_mask &= ~(IC_RX_FULL | IC_STOP_DET);
        }
    }

    // transmit
    if (stat & IC_TX_EMPTY) {
        CK_UINT32 i = 0;
        CK_UINT32 tmp_len = FIFO_DEPTH - pi2c->ic_rxflr;
        while ((tx_len < tx_rx_len) &&
                ((pi2c->ic_status & IC_STATUS_TFNF) != 0)) {
            if (i >= tmp_len) {
                return;
            }
            if (tx_len < (tx_rx_len - 1)) {
                if (rx_len != tx_rx_len) {
                    pi2c->ic_cmd_data = IC_CMD;
                } else {
                    pi2c->ic_cmd_data = tx_buf[tx_len];
                }
            } else {
                // last byte
                if (rx_len != tx_rx_len) {
                    pi2c->ic_cmd_data = IC_STOP | IC_CMD;
                } else {
                    pi2c->ic_cmd_data = IC_STOP | tx_buf[tx_len];
                }
            }
            tx_len++;
            i++;
        }
        if (tx_len == tx_rx_len) {
            // close TX_EMPTY interrupt after transmitting all data
            pi2c->ic_intr_mask &= ~IC_TX_EMPTY;
        }
    }
}

void i2c0_handle(CK_UINT32 irq) {
    i2c_handle_common(0);
}

void i2c1_handle(CK_UINT32 irq) {
    i2c_handle_common(1);
}

void i2c2_handle(CK_UINT32 irq) {
    i2c_handle_common(2);
}

void i2c3_handle(CK_UINT32 irq) {
    i2c_handle_common(3);
}

/*
 * set_speed - Set the i2c speed mode (standard, high, fast, fast plus)
 * @id:         the I2C controller No.
 * @i2c_spd:    required i2c speed mode
 *
 * Set the i2c speed mode (standard, high, fast)
 */
static void set_speed(CK_UINT32 id, CK_UINT32 i2c_spd) {
    CK_UINT32 cntl = 0;
    CK_UINT32 hcnt = 0;
    CK_UINT32 lcnt = 0;
    CK_UINT32 enbl = 0;
    PCKPStruct_I2C pi2c = NULL;

    pi2c = CK_I2C_Table[id].addr;

    /* to set speed cltr must be disabled */
    enbl = pi2c->ic_enable;
    enbl &= ~IC_ENABLE_0B;
    pi2c->ic_enable = enbl;

    // wait until IC_EN goes to 0 in IC_ENABLE_STATUS
    // which means it has completely been disabled
    while ((pi2c->ic_enable_status & IC_ENABLE_0B) != 0);

    cntl = (pi2c->ic_con) & (~IC_CON_SPD_MSK);

    switch (i2c_spd) {
    case IC_SPEED_MODE_MAX:
        cntl |= IC_CON_SPD_HS;
        hcnt = (IC_CLK * MIN_HS_SCL_HIGHTIME) / NANO_TO_MICRO;
        pi2c->ic_hs_scl_hcnt = hcnt;
        lcnt = (IC_CLK * MIN_HS_SCL_LOWTIME) / NANO_TO_MICRO;
        pi2c->ic_hs_scl_lcnt = lcnt;
        break;
    case IC_SPEED_MODE_STANDARD:
        cntl |= IC_CON_SPD_SS;
        hcnt = (IC_CLK * MIN_SS_SCL_HIGHTIME) / NANO_TO_MICRO;
        pi2c->ic_ss_scl_hcnt = hcnt;
        lcnt = (IC_CLK * MIN_SS_SCL_LOWTIME) / NANO_TO_MICRO;
        pi2c->ic_ss_scl_lcnt = lcnt;
        break;
    case IC_SPEED_MODE_FAST_PLUS:
        cntl |= IC_CON_SPD_FS;
        hcnt = (IC_CLK * MIN_FS_PLUS_SCL_HIGHTIME) / NANO_TO_MICRO;
        pi2c->ic_fs_scl_hcnt = hcnt;
        lcnt = (IC_CLK * MIN_FS_PLUS_SCL_LOWTIME) / NANO_TO_MICRO;
        pi2c->ic_fs_scl_lcnt = lcnt;
        break;
    case IC_SPEED_MODE_FAST:
    default:
        cntl |= IC_CON_SPD_FS;
        hcnt = (IC_CLK * MIN_FS_SCL_HIGHTIME) / NANO_TO_MICRO;
        pi2c->ic_fs_scl_hcnt = hcnt;
        lcnt = (IC_CLK * MIN_FS_SCL_LOWTIME) / NANO_TO_MICRO;
        pi2c->ic_fs_scl_lcnt = lcnt;
        break;
    }

    pi2c->ic_con = cntl;

    /* Enable i2c with new speed set */
    enbl |= IC_ENABLE_0B;
    pi2c->ic_enable = enbl;
}

/*
 * dw_i2c_set_bus_speed - Set the i2c speed
 * @id:     the I2C controller No.
 * @speed:  required i2c speed
 *
 * Set the i2c speed.
 */
CK_UINT32 dw_i2c_set_bus_speed(CK_UINT32 id, CK_UINT32 speed) {
    CK_UINT32 i2c_spd = 0;

    if (speed >= I2C_MAX_SPEED) {
        i2c_spd = IC_SPEED_MODE_MAX;
    } else if (speed >= I2C_FAST_PLUS_SPEED) {
        i2c_spd = IC_SPEED_MODE_FAST_PLUS;
    } else if (speed >= I2C_FAST_SPEED) {
        i2c_spd = IC_SPEED_MODE_FAST;
    } else {
        i2c_spd = IC_SPEED_MODE_STANDARD;
    }
    set_speed(id, i2c_spd);

    return 0;
}

/*
 * dw_i2c_init - Init function
 * @id:     the I2C controller No.
 * @speed:  required i2c speed
 * @int_en: 1 to use interrupt service; 0 not to
 *
 * Initialization function.
 */
void dw_i2c_init(CK_UINT32 id, CK_UINT32 speed, CK_UINT8 int_en) {
    PCKStruct_I2CInfo info = NULL;
    PCKPStruct_I2C pi2c = NULL;

    info = &(CK_I2C_Table[id]);
    pi2c = info->addr;

    /* Disable i2c */
    pi2c->ic_enable &= ~IC_ENABLE_0B;

    // wait until IC_EN goes to 0 in IC_ENABLE_STATUS
    // which means it has completely been disabled
    while ((pi2c->ic_enable_status & IC_ENABLE_0B) != 0);

    pi2c->ic_con = IC_CON_SD | IC_CON_RE | IC_CON_SPD_FS | IC_CON_MM;
    pi2c->ic_rx_tl = IC_RX_TL;
    pi2c->ic_tx_tl = IC_TX_TL;
    pi2c->ic_intr_mask = 0;
    dw_i2c_set_bus_speed(id, speed);
    if (int_en == 1) {
        info->irqhandler.devname = "I2C";
        info->irqhandler.irqid = info->irq;
        info->irqhandler.priority = info->irq;
        if (id == 0) {
            info->irqhandler.handler = i2c0_handle;
        } else if (id == 1) {
            info->irqhandler.handler = i2c1_handle;
        } else if (id == 2) {
            info->irqhandler.handler = i2c2_handle;
        } else {
            info->irqhandler.handler = i2c3_handle;
        }
        info->irqhandler.bfast = FALSE;
        info->irqhandler.next = NULL;
        /* register timer isr */
        CK_INTC_RequestIrq(&(info->irqhandler), AUTO_MODE);
    }
    /* Enable i2c */
    pi2c->ic_enable = IC_ENABLE_0B;
}

/*
 * i2c_setaddress - Sets the target slave address
 * @id:        the I2C controller No.
 * @i2c_addr:  target i2c slave address
 *
 * Sets the target slave address.
 */
static void i2c_setaddress(CK_UINT32 id, unsigned int i2c_addr) {
    PCKPStruct_I2C pi2c = NULL;

    pi2c = CK_I2C_Table[id].addr;

    /* Disable i2c */
    pi2c->ic_enable &= ~IC_ENABLE_0B;//程序软中止abort,

    // wait until IC_EN goes to 0 in IC_ENABLE_STATUS
    // which means it has completely been disabled
    while ((pi2c->ic_enable_status & IC_ENABLE_0B) != 0);

    pi2c->ic_tar = i2c_addr;//step1.先disable I2C ,step2. 设置I2C 目标地址寄存器=0xb;step3.使能I2C.

    /* Enable i2c */
    pi2c->ic_enable |= IC_ENABLE_0B;
}

/*
 * i2c_flush_rxfifo - Flushes the i2c RX FIFO
 * @id:        the I2C controller No.
 *
 * Flushes the i2c RX FIFO
 */
static void i2c_flush_rxfifo(CK_UINT32 id) {
    PCKPStruct_I2C pi2c = NULL;
    CK_UINT32 tmp32 = 0;

    pi2c = CK_I2C_Table[id].addr;

    while ((pi2c->ic_status & IC_STATUS_RFNE) != 0) {
        tmp32 = pi2c->ic_cmd_data;
    }
}

/*
 * i2c_wait_for_bb - Waits for bus busy
 * @id:        the I2C controller No.
 * Waits for bus busy
 */
static CK_UINT32 i2c_wait_for_bb(CK_UINT32 id) {
    PCKPStruct_I2C pi2c = NULL;
    CK_UINT32 timeout = 0 ;

    pi2c = CK_I2C_Table[id].addr;

/*
* 判断master FSM 状态是处于idle 还是 activity.|| Transmit FIFO 为空.
*/
    while (((pi2c->ic_status & IC_STATUS_MA) != 0) ||
           ((pi2c->ic_status & IC_STATUS_TFE) == 0)) {
        /* Evaluate timeout */
        if (timeout++ > 0x5000) {
            return 1;
        }
    }

    return 0;
}

static CK_UINT8 i2c_get_ack(CK_UINT32 id) {
    PCKPStruct_I2C pi2c = NULL;
    CK_UINT32 tmp32 = 0;
    pi2c = CK_I2C_Table[id].addr;

    // waiting for transmitting done
    while((pi2c->ic_raw_intr_stat & IC_MST_ON_HOLD) == 0) {
        if ((pi2c->ic_raw_intr_stat & IC_STOP_DET) != 0) {
            // wait for STOP_DET if not MST_ON_HOLD
            if ((pi2c->ic_tx_abrt_source & IC_ABRT_7B_ADDR_NOACK) != 0) {
                // means we got NOACK, need to restart the transfer
                tmp32 = pi2c->ic_clr_tx_abrt;
                tmp32 = pi2c->ic_clr_stop_det;
                return 1;
            } else {
                // if it is a read operation without data address,
                // then there could be no NOACK, but STOP_DET normally
                // after receiving data, so go on normally
                return 0;
            }
        }
    }
    return 0;
}

/*
 * i2c_xfer_init:   Initialization process to setup the read/write data address
 * @id:             the I2C controller No. == 1;
 * @chip:           slave address of the target device == 0xb
 * @addr:           data address in the slave device to read from / write to
 * @alen:           length of the i2c data address (1..2 bytes)
 *
 */
static CK_UINT32 i2c_xfer_init(CK_UINT32 id, CK_UINT32 chip, CK_UINT32 addr,
             CK_UINT32 alen) {
    CK_UINT32 value = 0;
    PCKPStruct_I2C pi2c = NULL;
    CK_UINT32 timeout = 0;
    CK_UINT32 len = alen;
    CK_UINT32 tmp32 = 0;
	//chip = 0xb,addr = 0;alen = 0;
    pi2c = CK_I2C_Table[id].addr;

    if (i2c_wait_for_bb(id)) {
        return 1;
    }

    i2c_setaddress(id, chip);//设置目标slave 设备的地址.

    while (alen) {
        if ((pi2c->ic_status & IC_STATUS_TFNF) != 0) {
            alen--;
            /* high byte address going out first */
            value = ((addr >> (alen * 8)) & 0xff);
            // it must be write operation for setting up the data address
            pi2c->ic_cmd_data = value;
            if (alen == len - 1) {
                tmp32 = i2c_get_ack(id);
                if (tmp32 == 1) {
                    timeout++;
                    if (timeout > 0x5000) {
                        printf("Timeout, No ACK for address 0x%x \n", chip);
                        return 1;
                    }
                    alen = len;
                    continue;
                }
            }
        }
    }

    return 0;
}

// wait for all the transfer is finished after a STOP command issued
static CK_UINT32 i2c_xfer_finish(CK_UINT32 id) {
    CK_UINT32 timeout = 0;
    CK_UINT32 tmp32 = 0;
    PCKPStruct_I2C pi2c = NULL;

    pi2c = CK_I2C_Table[id].addr;

    while (1) {
        if ((pi2c->ic_raw_intr_stat & IC_STOP_DET) != 0) {
            tmp32 = pi2c->ic_clr_stop_det;
            break;
        } else if (timeout++> 0x5000) {
            printf("Timed out waiting for STOP_DET\n");
            break;
        }
    }

    if (i2c_wait_for_bb(id)) {
        printf("Timed out waiting for bus\n");
        return 1;
    }
    // Since we have received all we want in read
    // operation, flush the rest data in FIFO
    i2c_flush_rxfifo(id);

    // disable i2c at last
    pi2c->ic_enable &= ~IC_ENABLE_0B;
    // wait until IC_EN goes to 0 in IC_ENABLE_STATUS
    // which means it has completely been disabled
    while ((pi2c->ic_enable_status & IC_ENABLE_0B) != 0);

    return 0;
}

/*
 * i2c_read - Read from i2c memory
 * @id:     the I2C controller No.
 * @dev:    target i2c address
 * @addr:   address to read from
 * @alen:   length of the i2c data address (1..2 bytes)
 * @buffer: buffer for read data
 * @len:    number of bytes to be read
 *
 * Read from i2c memory.
 */
CK_UINT32 dw_i2c_read(CK_UINT32 id, CK_UINT32 dev, CK_UINT32 addr,
               CK_UINT32 alen, CK_UINT8 *buffer, CK_UINT32 len) {
    CK_UINT32 timeout = 0;
    CK_UINT32 timeout_ack = 0;
    CK_UINT32 tmp32 = 0;
    CK_UINT32 tmp_len = len;
    PCKPStruct_I2C pi2c = NULL;

    pi2c = CK_I2C_Table[id].addr;

    if (i2c_xfer_init(id, dev, addr, alen)) {
        return 1;
    }
    while (len) {//len = 8.
		/* 判断Transmit FIFO 是否为空.当Transmit FIFO 为空时,该位被置1.*/
        if ((pi2c->ic_status & IC_STATUS_TFNF) != 0) {
            if (len == 1) {
                pi2c->ic_cmd_data = IC_CMD | IC_STOP; // CMD = Read
            } else {
                pi2c->ic_cmd_data = IC_CMD; // CMD = Read
            }
            // some read operation may not have word address
            if ((alen == 0) && (len == tmp_len)) {
                tmp32 = i2c_get_ack(id);
                if (tmp32 == 1) {
                    timeout_ack++;
                    if (timeout_ack > 0x5000) {
                        printf("Timeout, No ACK for address %x \n", dev);
                        return 1;
                    }
                    continue;
                }
            }
        }

        while (1) {
			//RFNE receive FIFO Not empty.当接收fifo 包含一个或者更多数据时被置1.当receive-fifo
			// empty 是被清除.
            if ((pi2c->ic_status & IC_STATUS_RFNE) != 0) {
                *buffer = (pi2c->ic_cmd_data & 0xff);//从i2c 的寄存器当中读取数据.
                 printf(" %d \n",*buffer);
                buffer++;
                len--;
                timeout = 0;
                break;
            } else if (timeout++ > 0x5000) {
                printf("Timed out. i2c read Failed\n");
                return 1;
            }
        }
    }

    return i2c_xfer_finish(id);
}

/*
 * i2c_write - Write to i2c memory
 * @id:     the I2C controller No.
 * @dev:    target i2c address
 * @addr:   address to write
 * @alen:   length of the i2c data address (1..2 bytes)
 * @buffer: buffer of data to be written
 * @len:    number of bytes to be written
 *
 * Write to i2c memory.
 */
CK_UINT32 dw_i2c_write(CK_UINT32 id, CK_UINT32 dev, CK_UINT32 addr,
            CK_UINT32 alen, CK_UINT8 *buffer, CK_UINT32 len) {
    CK_UINT32 timeout = 0;
    PCKPStruct_I2C pi2c = NULL;

    pi2c = CK_I2C_Table[id].addr;//i2c1 上面的ic_cmd_data 寄存器.
//step1: 初始化I2C 传输;dev = 0xb,addr = 0;alen = 0;
    if (i2c_xfer_init(id, dev, addr, alen)) {
        return 1;
    }

    while (len) {
		/* 判断Transmit FIFO 是否为空.当Transmit FIFO 为空时,该位被置1.*/
        if ((pi2c->ic_status & IC_STATUS_TFNF) != 0) {
            if (--len == 0) {
                pi2c->ic_cmd_data = *buffer | IC_STOP;
            } else {
                pi2c->ic_cmd_data = *buffer;
            }
            buffer++;
            timeout = 0;
        } else if (timeout++ > 0x5000) {
            printf("Timed out. i2c write Failed\n");
            return 1;
        }
    }
//step3: 结束I2C传输
    return i2c_xfer_finish(id);
}

/*
 * i2c_dma_read_pre - Pre work before reading from i2c memory with DMA
 * @dma_id:     the DMA controller No.
 * @id:         the I2C controller No.
 * @dev:        target i2c address
 * @addr:       address to read from
 * @alen:       length of the i2c data address (1..2 bytes)
 * @buffer:     buffer for read data
 * @len:        number of bytes to be read
 * @dma_ch:     DMA channel No.
 * @dma_intr:   whether to use interrupt mode of DMA
 * @rdlr:       Receive Data Level
 *
 * Pre work before reading from i2c memory with DMA
 */
CK_UINT32 dw_i2c_dma_read_pre(CK_UINT32 dma_id,
            CK_UINT32 id, CK_UINT32 dev, CK_UINT32 addr,
            CK_UINT32 alen, CK_UINT8 *buffer, CK_UINT32 len,
            CK_UINT8 dma_ch, CK_UINT8 dma_intr, CK_UINT8 rdlr) {
    PCKPStruct_I2C pi2c = NULL;

    pi2c = CK_I2C_Table[id].addr;

    if (i2c_xfer_init(id, dev, addr, alen)) {
        return 1;
    }

    pi2c->ic_dma_cr = IC_DMA_CR_RDMAE;
    pi2c->ic_dma_rdlr = rdlr;

    DMAPeripheral2MemOpen(dma_id, dma_ch, (CK_UINT32)buffer,
                            len, peripheral_i2c_rx(id),
                            dma_intr, 0, 0, 0);
    return 0;
}

/*
 * i2c_dma_read_post - Post work after reading from i2c memory with DMA
 * @dma_id:     the DMA controller No.
 * @id:         the I2C controller No.
 * @len:        number of bytes to be read
 * @dma_ch:     DMA channel No.
 * @dma_intr:   whether to use interrupt mode of DMA
 *
 * Post work after reading from i2c memory with DMA
 */
CK_UINT32 dw_i2c_dma_read_post(CK_UINT32 dma_id,
            CK_UINT32 id, CK_UINT32 len,
            CK_UINT8 dma_ch, CK_UINT8 dma_intr) {
    PCKPStruct_I2C pi2c = NULL;

    pi2c = CK_I2C_Table[id].addr;

    while (len) {
        if ((pi2c->ic_status & IC_STATUS_TFNF) != 0) {
            if (len == 1) {
                pi2c->ic_cmd_data = IC_CMD | IC_STOP; // CMD = Read
            } else {
                pi2c->ic_cmd_data = IC_CMD; // CMD = Read
            }
            len--;
        }
    }

    while(!(DMAC_CheckDone(dma_id, dma_ch, dma_intr)));
    DMAC_Close(dma_id, dma_ch);

    return i2c_xfer_finish(id);
}

/*
 * i2c_dma_read - Read from i2c memory with DMA
 * @dma_id:     the DMA controller No.
 * @id:         the I2C controller No.
 * @dev:        target i2c address
 * @addr:       address to read from
 * @alen:       length of the i2c data address (1..2 bytes)
 * @buffer:     buffer for read data
 * @len:        number of bytes to be read
 * @dma_ch:     DMA channel No.
 * @dma_intr:   whether to use interrupt mode of DMA
 * @rdlr:       Receive Data Level
 *
 * Read from i2c memory with DMA
 */
CK_UINT32 dw_i2c_dma_read(CK_UINT32 dma_id, CK_UINT32 id,
            CK_UINT32 dev, CK_UINT32 addr,
            CK_UINT32 alen, CK_UINT8 *buffer, CK_UINT32 len,
            CK_UINT8 dma_ch, CK_UINT8 dma_intr, CK_UINT8 rdlr) {
    CK_UINT8 ret = 0;
    DMAC_Init(dma_id);

    ret = dw_i2c_dma_read_pre(dma_id, id, dev, addr, alen, buffer, len, dma_ch,
                                dma_intr, rdlr);
    if (ret != 0) {
        return 1;
    }
    DMAC_Start(dma_id, dma_ch);

    ret = dw_i2c_dma_read_post(dma_id, id, len, dma_ch, dma_intr);
    return ret;
}

/*
 * i2c_dma_write_pre - Pre work before writing to i2c memory with DMA
 * @dma_id:         the DMA controller No.
 * @id:             the I2C controller No.
 * @dev:            target i2c address
 * @addr:           address to read from
 * @alen:           length of the i2c data address (1..2 bytes)
 * @buffer:         buffer of data to be written
 * @len:            number of bytes to be written
 * @dma_ch:         DMA channel No.
 * @dma_intr:       whether to use interrupt mode of DMA
 * @tdlr:           Transmit Data Level
 * @dst_tr_width:   DST_TR_WIDTH of DMA
 * @dts_msize:      DST_MSIZE of DMA
 *
 * Pre work before writing to i2c memory with DMA
 */
CK_UINT32 dw_i2c_dma_write_pre(CK_UINT32 dma_id, CK_UINT32 id,
            CK_UINT32 dev, CK_UINT32 addr,
            CK_UINT32 alen, CK_UINT8 *buffer, CK_UINT32 len,
            CK_UINT8 dma_ch, CK_UINT8 dma_intr, CK_UINT8 tdlr,
            CK_UINT32 dst_tr_width, CK_UINT32 dst_msize) {

    PCKPStruct_I2C pi2c = NULL;

    pi2c = CK_I2C_Table[id].addr;

    if (i2c_xfer_init(id, dev, addr, alen)) {
        return 1;
    }

    pi2c->ic_dma_cr = IC_DMA_CR_TDMAE;
    pi2c->ic_dma_tdlr = tdlr;
    // do len - 1 transfer, and then do last transfer with STOP bit set
    DMAMem2PeripheralOpen(dma_id, dma_ch, (CK_UINT32)buffer, len - 1,
                            peripheral_i2c_tx(id),
                            dma_intr, 0, 0, 0, dst_tr_width, dst_msize);
    return 0;
}

/*
 * i2c_dma_write_post - Post work after writing to i2c memory with DMA
 * @dma_id:         the DMA controller No.
 * @id:             the I2C controller No.
 * @buffer:         buffer of data to be written
 * @len:            number of bytes to be written
 * @dma_ch:         DMA channel No.
 * @dma_intr:       whether to use interrupt mode of DMA
 * @dst_tr_width:   DST_TR_WIDTH of DMA
 *
 * Post work after writing to i2c memory with DMA
 */
CK_UINT32 dw_i2c_dma_write_post(CK_UINT32 dma_id, CK_UINT32 id,
            CK_UINT8 *buffer, CK_UINT32 len,
            CK_UINT8 dma_ch, CK_UINT8 dma_intr,
            CK_UINT32 dst_tr_width) {
    CK_UINT32 timeout = 0;
    CK_UINT32 rest_len = 4; //4 bytes

    PCKPStruct_I2C pi2c = NULL;

    pi2c = CK_I2C_Table[id].addr;

    while (1) {
        if (DMAC_CheckDone(dma_id, dma_ch, dma_intr) == 1) {
            break;
        } else {
            if (timeout++ > 0x5000) {
                printf("\n\t%s: waiting for DMA Done timeout!\n", __func__);
                printf("                - - - FAIL.\n");
                pi2c->ic_dma_cr = 0; // clear DMA bit
                pi2c->ic_enable |= IC_ENABLE_ABORT; // abort
                return 1;
            }
        }
    }

    DMAC_Close(dma_id, dma_ch);

    while (rest_len != 0) {
        if (pi2c->ic_status & IC_STATUS_TFNF) {
            if (dst_tr_width == DMAC_CTL_DST_TR_WIDTH8) {
                if (rest_len == 1) {
                    pi2c->ic_cmd_data = buffer[len * 4 - rest_len] | IC_STOP;
                } else {
                    pi2c->ic_cmd_data = buffer[len * 4 - rest_len];
                }
                rest_len--;
            } else if (dst_tr_width == DMAC_CTL_DST_TR_WIDTH32) {
                pi2c->ic_cmd_data = buffer[(len - 1) * 4] | IC_STOP;
                rest_len = 0;
            } else {
                printf("\n\tDMAC_CTL_DST_TR_WIDTH %d not supported"
                        " in test for now\n",
                        DST_TR_WIDTH(dst_tr_width));
                return 1;
            }
        } else if (timeout++ > 0x5000) {
            printf("\n\tTimed out. i2c write Failed\n");
            return 1;
        }
    }

    return i2c_xfer_finish(id);
}

/*
 * i2c_dma_write - Write to i2c memory with DMA
 * @dma_id:         the DMA controller No.
 * @id:             the I2C controller No.
 * @dev:            target i2c address
 * @addr:           address to write
 * @alen:           length of the i2c data address (1..2 bytes)
 * @buffer:         buffer of data to be written
 * @len:            number of bytes to be written
 * @dma_ch:         DMA channel No.
 * @dma_intr:       whether to use interrupt mode of DMA
 * @tdlr:           Transmit Data Level
 * @dst_tr_width:   DST_TR_WIDTH of DMA
 * @dts_msize:      DST_MSIZE of DMA
 *
 * Write to i2c memory with DMA
 */
CK_UINT32 dw_i2c_dma_write(CK_UINT32 dma_id, CK_UINT32 id,
            CK_UINT32 dev, CK_UINT32 addr,
            CK_UINT32 alen, CK_UINT8 *buffer, CK_UINT32 len,
            CK_UINT8 dma_ch, CK_UINT8 dma_intr, CK_UINT8 tdlr,
            CK_UINT32 dst_tr_width, CK_UINT32 dst_msize) {
    CK_UINT8 ret = 0;

    DMAC_Init(dma_id);

    ret = dw_i2c_dma_write_pre(dma_id, id, dev, addr, alen, buffer, len,
                                dma_ch, dma_intr, tdlr, dst_tr_width,
                                dst_msize);
    if (ret != 0) {
        return 1;
    }

    DMAC_Start(dma_id, dma_ch);

    ret = dw_i2c_dma_write_post(dma_id, id, buffer, len, dma_ch,
                                dma_intr, dst_tr_width);
    return ret;
}

/*
 * i2c_read_int - Read from i2c memory in interrupt service
 * @id:     the I2C controller No.
 * @dev:    target i2c address
 * @addr:   address to read from
 * @alen:   length of the i2c data address (1..2 bytes)
 * @buffer: buffer for read data
 * @len:    number of bytes to be read
 *
 * Read from i2c memory.
 */
CK_UINT32 dw_i2c_read_int(CK_UINT32 id, CK_UINT32 dev, CK_UINT32 addr,
               CK_UINT32 alen, CK_UINT8 *buffer, CK_UINT32 len) {
    PCKPStruct_I2C pi2c = NULL;
    CK_UINT8 ret = 0;
    CK_UINT8 timeout_ack = 0;
    CK_UINT32 tmp32 = 0;
    pi2c = CK_I2C_Table[id].addr;

    if (i2c_xfer_init(id, dev, addr, alen)) {
        return 1;
    }
    tx_len = 0;
    rx_len = 0;
    tx_rx_len = len;
    rx_buf = buffer;
    // choose the tx threshold to make the transmit contineously
    // as much as possible
    pi2c->ic_tx_tl = IC_TL4;
    // choose the rx threshold to minimize times of interrupt
    // and also try not to overflow
    pi2c->ic_rx_tl = IC_TL3;

    // write one byte to see if we can get ACK
    if (alen == 0) {
        while (1) {
            if ((pi2c->ic_status & IC_STATUS_TFNF) != 0) {
                if (len == 1) {
                    pi2c->ic_cmd_data = IC_CMD | IC_STOP; // CMD = Read
                } else {
                    pi2c->ic_cmd_data = IC_CMD; // CMD = Read
                }
                tmp32 = i2c_get_ack(id);
                if (tmp32 == 1) {
                    timeout_ack++;
                    if (timeout_ack > 0x5000) {
                        printf("Timeout, No ACK for address %x \n", dev);
                        return 1;
                    }
                    continue;
                }
                break;
            }
        }
        tx_len = 1;
    }
    // enable RX_FULL and TX_EMPTY start transmitting/receiving
    pi2c->ic_intr_mask = IC_RX_FULL | IC_TX_EMPTY | IC_STOP_DET;
    // we keep waiting for finish for test convenience
    ret = i2c_xfer_finish(id);
    return ret;
}

/*
 * i2c_write_int - Write to i2c memory in interrupt service
 * @id:     the I2C controller No.
 * @dev:    target i2c address
 * @addr:   address to write
 * @alen:   length of the i2c data address (1..2 bytes)
 * @buffer: buffer of data to be written
 * @len:    number of bytes to be written
 *
 * Write to i2c memory.
 */
CK_UINT32 dw_i2c_write_int(CK_UINT32 id, CK_UINT32 dev, CK_UINT32 addr,
            CK_UINT32 alen, CK_UINT8 *buffer, CK_UINT32 len) {
    PCKPStruct_I2C pi2c = NULL;

    pi2c = CK_I2C_Table[id].addr;

    if (i2c_xfer_init(id, dev, addr, alen)) {
        return 1;
    }

    tx_len = 0;
    // no receiving
    rx_len = len;
    tx_rx_len = len;
    tx_buf = buffer;
    // choose the tx threshold to make the transmit contineously
    // as much as possible
    pi2c->ic_tx_tl = IC_TL4;
    // enable TX_EMPTY, start transmitting
    pi2c->ic_intr_mask = IC_TX_EMPTY;

    // we keep waiting for finish for test convenience
    return i2c_xfer_finish(id);
}
