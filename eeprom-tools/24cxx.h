/***************************************************************************
    copyright            : (C) by 2002-2003 Stefano Barbato
    email                : stefano@codesink.org

    $Id: 24cXX.h 6048 2012-04-26 10:10:22Z khali $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef _24CXX_H_
#define _24CXX_H_
#include <linux/types.h>

#define EEPROM_TYPE_UNKNOWN	0
#define EEPROM_TYPE_8BIT_ADDR	1
#define EEPROM_TYPE_16BIT_ADDR 	2

#define DEFAULT_EEPROM_ADDR	0x50
#define DEFAULT_NUM_PAGES	8
#define BYTES_PER_PAGE		32768
/*
 * FIXME：
 * 如果每次不加延迟的话，会失败。
 */
#define MAX_BYTES		32
#define DELAY_TIME_PER_PAGE	1000

struct eeprom
{
#define EE_BANK0 0
#define EE_BANK1 1
	unsigned char bank; /*0: 0x0000~0x7fffh, 1: 0x8000~0xffff ; the addr include bit7:1 0 1 0 A2 A1 A15 bit0:R/W*/
	char *dev; 	// device file i.e. /dev/i2c-N
	int addr;	// i2c address
	int fd;		// file descriptor
	int type; 	// eeprom type
	int pagesize;
};

/*
 * opens the eeprom device at [dev_fqn] (i.e. /dev/i2c-N) whose address is
 * [addr] and set the eeprom_24c32 [e]
 */
int eeprom_open(char *dev_fqn, int addr, int type, struct eeprom*, int pagesize, int bank);
/*
 * closees the eeprom device [e] 
 */
int eeprom_close(struct eeprom *e);
/*
 * read and returns the eeprom byte at memory address [mem_addr] 
 * Note: eeprom must have been selected by ioctl(fd,I2C_SLAVE,address) 
 */
int eeprom_read_byte(struct eeprom* e, __u16 mem_addr);
/*
 * read the current byte
 * Note: eeprom must have been selected by ioctl(fd,I2C_SLAVE,address) 
 */
int eeprom_read_current_byte(struct eeprom *e);
/*
 * writes [data] at memory address [mem_addr] 
 * Note: eeprom must have been selected by ioctl(fd,I2C_SLAVE,address) 
 */
int eeprom_write_byte(struct eeprom *e, __u16 mem_addr, __u8 data);

//int i2c_read_from_eeprom(struct eeprom *e, int addr, unsigned char *buf, int len);
//int i2c_write_to_eeprom(struct eeprom *e, int addr, unsigned char *buf, int len);

/*
 * Note:
 *  1. 最大长度: 32 Byte.
 *  2. 两次操作之间，需要延时一下，确保数据正确操作
 */
int i2c_write_eeprom_block(struct eeprom *e, int offset, unsigned char *buf, int len);
int i2c_read_eeprom_block(struct eeprom *e, int offset, unsigned char *buf, int len);


#endif

