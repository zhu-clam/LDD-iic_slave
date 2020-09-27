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
*@Filename: ipcm_func.h                                             
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


#ifndef __IPCM_VDD_FUNCTIONS_HEADER__
#define __IPCM_VDD_FUNCTIONS_HEADER__


enum message_type {
	MESSAGE_NORMAL,
	MESSAGE_URGENT,
	MESSAGE_CONNECT_REQUEST,
	MESSAGE_CONNECT_ACK,
	MESSAGE_DISCONNECT_REQUEST,
	MESSAGE_DISCONNECT_ACK
};

enum connect_block {
	CONNECT_NONBLOCK,
	CONNECT_BLOCK
};

enum __handle_state {
	__HANDLE_DISCONNECTED,
	__HANDLE_CONNECTING,
	__HANDLE_CONNECTED
};

enum __message_priority {
	__HANDLE_MSG_NORMAL,
	__HANDLE_MSG_PRIORITY
};

typedef int (*recv_notifier)(void *handle, void *buf, unsigned int len);

typedef struct ipcm_vdd_opt {
	recv_notifier recv;
	long long data;
} ipcm_vdd_opt_t;

unsigned int get_devices_number(void);

void *ipcm_vdd_open(int target, int port, int priority);
void ipcm_vdd_close(void *handle);
int ipcm_vdd_sendmsg(void *handle, const void *buf, unsigned int len);
int ipcm_vdd_connect(void *handle, int is_block);
void ipcm_vdd_disconnect(void *handle);
int ipcm_vdd_recvmsg(int *source, int *port, char *pbuf, int len);
int ipcm_vdd_check_handle(void *handle);
int ipcm_vdd_getopt(void *handle, struct ipcm_vdd_opt *opt);
int ipcm_vdd_setopt(void *handle, struct ipcm_vdd_opt *opt);
int ipcm_node_ready(int target);

struct ipcm_node *ipcm_get_node(int id);
int ipcm_vdd_localid(void);
int ipcm_vdd_remoteids(int *ids);
int ipcm_vdd_init(void);
void ipcm_vdd_cleanup(void);

int ipcm_node_ready(int target);
#endif
