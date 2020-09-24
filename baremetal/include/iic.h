/*****************************************************************************
 *  File: iic.h
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
 
#ifndef __DW_I2C_H_
#define __DW_I2C_H_

#include "ck810.h"
#include "datatype.h"
#include "misc.h"

//#include "Platform.h"
//#include "typedef.h"

// IC_CLK in MHz
//#define IC_CLK			    175
// 12.5MHz on FPGA
#if CONFIG_IS_ASIC
#define IC_CLK  (I2C_CLK_FREQ / 1000000)
#else
#define IC_CLK              24
#endif
#define NANO_TO_MICRO		1000


/* High and low times in different speed modes (in ns) */
#define MIN_SS_SCL_HIGHTIME	        4000
#define MIN_SS_SCL_LOWTIME	        4700
#define MIN_FS_SCL_HIGHTIME	        600
#define MIN_FS_SCL_LOWTIME	        1300
#define MIN_FS_PLUS_SCL_HIGHTIME    260
#define MIN_FS_PLUS_SCL_LOWTIME     500
#define MIN_HS_SCL_HIGHTIME	        60
#define MIN_HS_SCL_LOWTIME	        160

/* Worst case timeout for 1 byte is kept as 2ms */
#define I2C_BYTE_TO		(I2C_CLK_FREQ / 500)
#define I2C_STOPDET_TO		(I2C_CLK_FREQ / 500)
#define I2C_BYTE_TO_BB		(I2C_BYTE_TO * 16)

/* i2c control register definitions */
#define IC_CON_SD	          	0x0040
#define IC_CON_RE		        0x0020
#define IC_CON_10BITADDRMASTER	0x0010
#define IC_CON_10BITADDR_SLAVE	0x0008
#define IC_CON_SPD_MSK		    0x0006
#define IC_CON_SPD_SS		    0x0002
#define IC_CON_SPD_FS		    0x0004
#define IC_CON_SPD_HS		    0x0006
#define IC_CON_MM		        0x0001

/* i2c target address register definitions */
#define TAR_ADDR		0x0050

/* i2c slave address register definitions */
#define IC_SLAVE_ADDR		0x0002

/* i2c data buffer and command register definitions */
#define IC_CMD			0x0100
#define IC_STOP			0x0200
#define IC_RESTART		0x0400


/* i2c interrupt status register definitions */
#define IC_MST_ON_HOLD  0x2000
#define IC_GEN_CALL		0x0800
#define IC_START_DET	0x0400
#define IC_STOP_DET		0x0200
#define IC_ACTIVITY		0x0100
#define IC_RX_DONE		0x0080
#define IC_TX_ABRT		0x0040
#define IC_RD_REQ		0x0020
#define IC_TX_EMPTY		0x0010
#define IC_TX_OVER		0x0008
#define IC_RX_FULL		0x0004
#define IC_RX_OVER 		0x0002
#define IC_RX_UNDER		0x0001

/* fifo threshold register definitions */
#define IC_TL0			0x00
#define IC_TL1			0x01
#define IC_TL2			0x02
#define IC_TL3			0x03
#define IC_TL4			0x04
#define IC_TL5			0x05
#define IC_TL6			0x06
#define IC_TL7			0x07
#define IC_RX_TL		IC_TL0
#define IC_TX_TL		IC_TL0

/* i2c enable register definitions */
#define IC_ENABLE_ABORT     0x0002
#define IC_ENABLE_0B		0x0001

/* i2c status register  definitions */
#define IC_STATUS_SA		0x0040
#define IC_STATUS_MA		0x0020
#define IC_STATUS_RFF		0x0010
#define IC_STATUS_RFNE		0x0008
#define IC_STATUS_TFE		0x0004
#define IC_STATUS_TFNF		0x0002
#define IC_STATUS_ACT		0x0001

// IC_DMA_CR Register
#define IC_DMA_CR_TDMAE     0x2
#define IC_DMA_CR_RDMAE     0x1

// IC_TX_ABRT_SOURCE
#define IC_ABRT_7B_ADDR_NOACK   0x1

/* Speed Selection */
#define IC_SPEED_MODE_STANDARD	1
#define IC_SPEED_MODE_FAST  	2
#define IC_SPEED_MODE_FAST_PLUS 3
#define IC_SPEED_MODE_MAX       4

#define I2C_MAX_SPEED		3400000
#define I2C_FAST_PLUS_SPEED 1000000
#define I2C_FAST_SPEED		400000
#define I2C_STANDARD_SPEED	100000

void dw_i2c_init(CK_UINT32 id, CK_UINT32 speed, CK_UINT8 int_en);
CK_UINT32 dw_i2c_set_bus_speed(CK_UINT32 id, CK_UINT32 speed);
CK_UINT32 dw_i2c_read(CK_UINT32 id, CK_UINT32 dev, CK_UINT32 addr, CK_UINT32 alen, CK_UINT8 *buffer, CK_UINT32 len);
CK_UINT32 dw_i2c_write(CK_UINT32 id, CK_UINT32 dev, CK_UINT32 addr, CK_UINT32 alen, CK_UINT8 *buffer, CK_UINT32 len);
CK_UINT32 dw_i2c_dma_write(CK_UINT32 dma_id, CK_UINT32 id,
            CK_UINT32 dev, CK_UINT32 addr,
            CK_UINT32 alen, CK_UINT8 *buffer, CK_UINT32 len,
            CK_UINT8 dma_ch, CK_UINT8 dma_intr, CK_UINT8 tdlr,
            CK_UINT32 dst_tr_width, CK_UINT32 dst_msize);
CK_UINT32 dw_i2c_dma_read(CK_UINT32 dma_id, CK_UINT32 id,
            CK_UINT32 dev, CK_UINT32 addr,
            CK_UINT32 alen, CK_UINT8 *buffer, CK_UINT32 len,
            CK_UINT8 dma_ch, CK_UINT8 dma_intr, CK_UINT8 rdlr);
void i2c0_handle(CK_UINT32 irq);
CK_UINT32 WM_WriteRegister(CK_UINT32 addr, CK_UINT32 data);



#endif /* __DW_I2C_H_ */

