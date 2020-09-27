/*
*                                                                          
*Copyright (c)  :2019-01-4  Grand Vision Design Systems Inc.   /
*Permission is hereby granted, free of charge, to any person obtaining  /
*a copy of this software and associated documentation files (the   /
*Software), to deal in the Software without restriction, including   /
*without limitation the rights to use, copy, modify, merge, publish,
*distribute, sublicense, and/or sell copies of the Software, and to   /
*permit persons to whom the Software is furnished to do so, subject to   /
*the following conditions:  /
*The above copyright notice and this permission notice shall be included   /
*in all copies or substantial portions of the Software.  /
*                                                                          
*THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND,  /
*EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF  /
*MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  /
*IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   /
*CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  /
*TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE   /
*SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  /
*                                                                          
*                                                                          
*@Filename: ipcm_buffer.h                                             
*                                                                          
*@Author: zhuxianfei                            
*@Created on     : 2019-4 -25               
*------------------------------------------------------------------------------
*@Description:                                                          
*                                                                          
*@Modification History                                                                          
*                                                                          
*                                                                          
*/

#ifndef __IPCM_SHARED_BUFFER_HEADER__
#define __IPCM_SHARED_BUFFER_HEADER__

#include "ipcm_funcs.h"

/* for linux kernel */
#ifdef __KERNEL__
#include "asm/page.h"
#endif

#define __DATA_ALIGN(end, align) (((unsigned int)end+align-1)&(~(align-1)))
#define __MSG_ALIGNED(end)       __DATA_ALIGN(end, 0x10)
#define __MSG_HEAD_SIZE          sizeof(struct ipcm_transfer_head)
#define __MSG_JUMP_MARK__        0x166

#define MAX_TRANSFER_LEN 2016
#ifndef PAGE_SHIFT
#define PAGE_SHIFT	12
#endif
#ifndef PAGE_SIZE
#define PAGE_SIZE	0x1000
#endif

#define ZONE_MAGIC	0x89765000


struct ipcm_transfer_head {
	unsigned int target:6;
	unsigned int source:6;
	unsigned int port:10;
	unsigned int type:10;
	unsigned int len:20;
	unsigned int reserve:12;
};

struct ipcm_transfer_handle {
	unsigned int target;
	unsigned int port;
	recv_notifier recv;
	unsigned long long data;
	int state;
	unsigned int priority;

	ipcm_atomic_t send_count;
	ipcm_atomic_t recv_count;
	ipcm_atomic_t max_send_len;
	ipcm_atomic_t max_recv_len;
	struct ipcm_lock lock;
};

struct mem_region {
	unsigned long base;
	unsigned int size;
};

struct ipcm_shared_area {
	struct mem_region region;
	volatile char *data;
	unsigned int len;
	volatile unsigned int *rp;
	volatile unsigned int *wp;
	struct ipcm_lock lock;
};

struct ipcm_shared_zone {
	struct mem_region region;
	struct ipcm_shared_area prio_area;
	struct ipcm_shared_area normal_area;
};

int ipcm_send_message(struct ipcm_transfer_handle *handle,
		const void *buf, unsigned int len, unsigned int flag);
int ipcm_shared_zone_init(int nid, unsigned long base, unsigned long size, int sendbuf);
int get_one_message(struct ipcm_node *node, int *port, char *buf, int len);

#endif

