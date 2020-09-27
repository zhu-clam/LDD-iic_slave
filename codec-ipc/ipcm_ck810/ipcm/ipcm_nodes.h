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
*@Filename: ipcm_nodes.h                                             
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


#ifndef __IPCM_NODES_HEADER__
#define __IPCM_NODES_HEADER__

#define MAX_NODES	2
#define MAX_PORTS	16

enum ipcm_node_state {
	NODE_ALIVE,
	NODE_READY,
	NODE_HALT
};

enum ipcm_handle_state {
	STATE_INIT = 0,     //first state when module init
	CONNECT_REQUESTING,  //send requesting when connecting
	CONNECT_REQUESTED,   //received requesting when connecting
	CONNECT_ACKING,   //send acking when connecting
	CONNECT_ACKED,   //received acking when connecting
	DISCONNECT_REQUESTING,   //send requesting when disconnecting
	DISCONNECT_REQUESTED,   //received requesting when disconnecting
	DISCONNECT_ACKING,   //send acking when disconnecting
	DISCONNECT_ACKED    //received acking when disconnecting
};


/*
 * Be noted that the size of the following struct
 * is within 32 bytes. Please do not edit this
 * struct if unsure.
 */
struct ipcm_node_desc {
	volatile unsigned int state; 
	volatile unsigned int recvbuf_pfn[MAX_NODES - 1];
};

struct ipcm_node {
	unsigned int id;
	struct ipcm_node_desc *desc;
	unsigned int irq;
	struct ipcm_shared_zone *sendbuf;
	struct ipcm_shared_zone *recvbuf;
	int state;
	struct ipcm_transfer_handle **handlers;
	int *handlers_state;
};

#endif

