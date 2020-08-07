/***************************************************************************
    copyright            : (C) by 2002-2003 Stefano Barbato
    email                : stefano@codesink.org

    $Id: 24cXX.c 6228 2014-02-20 08:37:15Z khali $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/fs.h>
#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <linux/i2c.h>
#include "24cxx.h"




#define CHECK_I2C_FUNC( var, label ) \
	do { 	if(0 == (var & label)) { \
		fprintf(stderr, "\nError: " \
			#label " function is required. Program halted.\n\n"); \
		exit(1); } \
	} while(0);

int eeprom_open(char *dev_fqn, int addr, int type, struct eeprom* e, int pagesize, int bank)
{
	int fd, r;
	unsigned long funcs;
	e->fd = e->addr = 0;
	e->dev = 0;
	
	fd = open(dev_fqn, O_RDWR);
	if(fd <= 0)
		return -1;

	// get funcs list
	if((r = ioctl(fd, I2C_FUNCS, &funcs) < 0))
		return r;

	
	// check for req funcs
	CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_READ_BYTE );
	CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_WRITE_BYTE );
	CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_READ_BYTE_DATA );
	CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_WRITE_BYTE_DATA );
	CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_READ_WORD_DATA );
	CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_WRITE_WORD_DATA );

	// set working device
	if( ( r = ioctl(fd, I2C_SLAVE_FORCE, addr+bank)) < 0)
		return r;
	e->fd = fd;
	e->addr = addr+bank;
	e->dev = dev_fqn;
	e->type = type;
	e->pagesize = pagesize;
	
	return 0;
}

int eeprom_close(struct eeprom *e)
{
	close(e->fd);
	e->fd = -1;
	e->dev = 0;
	e->type = EEPROM_TYPE_UNKNOWN;
	return 0;
}


/*field len can be zero, when len is zero , it ought to trigger read*/
int __i2c_write_eeprom(struct eeprom *e, int offset, unsigned char *buf, int len)
{
	struct i2c_rdwr_ioctl_data msg_rdwr;
	struct i2c_msg             i2cmsg;
	int i;
	char _buf[MAX_BYTES + 2];/*2 == EEPROM_TYPE_16BIT_ADDR*/

	if (!e ) return 0;

	if( len > MAX_BYTES){
	    fprintf(stderr,"I can only write MAX_BYTES bytes at a time!\n");
	    return -1;
	}
#if 0
	if(len+offset >256){
	    fprintf(stderr,"Sorry, len(%d)+offset(%d) > 256 (page boundary)\n",
			len,offset);
	    return -1;
	}
#endif
	if (e->type == EEPROM_TYPE_16BIT_ADDR){
		_buf[0]=offset>>8;
		_buf[1]=offset &0xff;
	}else
		_buf[0]=offset &0xff;



	for( i = 0; i < len; i++){ /* copy buf[0..n] -> _buf[1..n+1] */
	    _buf[e->type + i] = buf[i];
	}

	msg_rdwr.msgs = &i2cmsg;
	msg_rdwr.nmsgs = 1;

	i2cmsg.addr = e->addr;
	i2cmsg.flags = 0;
	i2cmsg.len  = len+e->type;
	i2cmsg.buf  = (unsigned char *)_buf;
	if((i = ioctl(e->fd, I2C_RDWR, &msg_rdwr)) < 0){
	    perror("ioctl()");
	    fprintf(stderr,"ioctl returned %d write_len:%d offset_x:%x e->addr:%x\n",i, e->type, offset, e->addr);
	    return -1;
	}
#if 0
	if(len>0)
	    fprintf(stderr,"Wrote %d bytes to eeprom at 0x%02x, offset %08x\n",
		    len,addr,offset);
	else
	    fprintf(stderr,"Positioned pointer in eeprom at 0x%02x to offset %08x\n",
		    addr,offset);
#endif
	/*marshy_here*/
	usleep(3000);
	return 0;

}

/*the offset of EEPROM can't bigger than 0xFFFF when WRITE, HOWEVER, READ IS ROLLER-OVER*/
int i2c_write_eeprom_block(struct eeprom *e, int offset, unsigned char *buf, int len)
{
	int ret, tmp_len;

	unsigned char *tmp_ptr = buf;

	if (!e ) return -1;
	/*TODO, AVOID MORE THAN 0xFFFF(THE VALUE: LEN+OFFSET), GOD BLESS YOU*/
#if 0	
	if (e->bank == EE_BANK0){
		if (offset & 0x8000){
			fprintf(stderr, "error address in BANK0 A15:%d", offset);
			return -1;
		}	
	}else{
		if (!(offset & 0x8000)){
			fprintf(stderr, "error address in BANK1 A15:%d", offset);
			return -1;
		}
	}
#endif		
	/*fragment*/
	do{
		tmp_len = len > e->pagesize? e->pagesize : len; 
		if ((ret = __i2c_write_eeprom(e, offset, tmp_ptr, tmp_len)))
			break;

		len -= tmp_len;
		offset += tmp_len;
		tmp_ptr += tmp_len;
		usleep(DELAY_TIME_PER_PAGE);
	}while(len);

	return (tmp_ptr-buf);
}

int __i2c_read_eeprom(struct eeprom *e, int offset, unsigned char *buf, int len)
{
	struct i2c_rdwr_ioctl_data msg_rdwr;
	struct i2c_msg             i2cmsg;
	int i;

	if ( len > MAX_BYTES ){
	    fprintf(stderr,"I can only write MAX_BYTES bytes at a time!\n");
	    return -1;
	}

	if(__i2c_write_eeprom(e, offset, NULL, 0) < 0)
	    return -1;

	msg_rdwr.msgs = &i2cmsg;
	msg_rdwr.nmsgs = 1;

	i2cmsg.addr  = e->addr;
	i2cmsg.flags = I2C_M_RD;
	i2cmsg.len   = len;
	i2cmsg.buf   = buf;

	if((i = ioctl(e->fd, I2C_RDWR, &msg_rdwr))<0){
	    perror("ioctl()");
	    fprintf(stderr,"ioctl returned %d len=%d\n",i, len);
	    return -1;
	}

#if 0
	fprintf(stderr,"Read %d bytes from eeprom at 0x%02x, offset %08x\n",
		len,addr,offset);
#endif
	return 0;
}

int i2c_read_eeprom_block(struct eeprom *e, int offset, unsigned char *buf, int len)
{
	int ret, tmp_len;
	unsigned char* tmp_ptr = buf;
	
	if (!e || !buf || !len) return -1;
	/*fragment*/
	do{
		tmp_len = len > e->pagesize? e->pagesize : len; 
		if ((ret = __i2c_read_eeprom(e, offset, tmp_ptr, tmp_len)))
			break;
		len -= tmp_len;
		offset += tmp_len;
		tmp_ptr += tmp_len;
		usleep(DELAY_TIME_PER_PAGE);
	}while(len);

	return (tmp_ptr-buf);
}
