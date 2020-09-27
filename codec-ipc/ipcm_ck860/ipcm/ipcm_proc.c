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
*@Filename: ipcm_proc.c                                            
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

#include "ipcm_osadapt.h"
#include "ipcm_desc.h"
#include "ipcm_funcs.h"
#include "ipcm_buffer.h"
#include "ipcm_nodes.h"
#include "device_config.h"
#include "ipcm_config_common.h"

extern struct ipcm_node_desc *g_nodes_desc;


static unsigned long get_phys_from_desc(int nid, int hnid)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	unsigned int pfn;

	if (nid < hnid) {
		pfn = desc[hnid].recvbuf_pfn[nid];
	} else if (nid > hnid) {
		pfn = desc[hnid].recvbuf_pfn[nid - 1];
	} else {
		return 0;
	}
	return pfn<<PAGE_SHIFT;
}


int __ipcm_read_proc__(void * data)
{
	int nid, j;
	int flag = 0;
	struct ipcm_node *node;
	unsigned int size;
	unsigned long phys;
	struct ipcm_transfer_handle *handle;
	const char *state;

	for (nid=0; nid < MAX_NODES; nid++) {
		if (NULL == (node = ipcm_get_node(nid)))
			continue;

		switch (node->state) {
			case NODE_HALT:
				state = "HALT";
				break;
			case NODE_ALIVE:
				state = "ALIVE";
				break;
			case NODE_READY:
				state = "READY";
				break;
			default:
				state = "FAULT";
				break;
		}
		/* node information */
		if (nid == LOCAL_ID) {
			__ipcm_proc_printf__(data, "\n*---LOCAL  NODE: ID=%d, STATE: %s\n",
					node->id, state);
			continue;
		}
		else {
			__ipcm_proc_printf__(data, "\n*---REMOTE NODE: ID=%d, STATE: %s\n",
					node->id, state);
		}

		/* receive buffer information */
		__ipcm_proc_printf__(data, " |-RECV BUFFER,");
		if (0 == LOCAL_ID) {
			if (ipcm_node_ready(nid)) {
				phys = get_phys_from_desc(nid, 0);
				size = node->recvbuf->region.size;
			} else {
				phys = 0;
				size = 0;
			}
		} else {
			cfg_2local_shm_phys(nid, phys);
			cfg_2local_shm_size(nid, size);
		}
		if (phys)
			__ipcm_proc_printf__(data, " PHYS<0x%016X, 0x%08X>\n",
					phys, size);
		else
			__ipcm_proc_printf__(data, " PHYS<NULL, 0x%08X>\n",
					size);

		/* send buffer information */
		__ipcm_proc_printf__(data, " |-SEND BUFFER,");
		if (0 == nid) {
			cfg_2remote_shm_phys(nid, phys);
			cfg_2remote_shm_size(nid, size);
		} else {
			if (ipcm_node_ready(nid)) {
				phys = get_phys_from_desc(LOCAL_ID, nid);
				size = node->recvbuf->region.size;
			} else {
				phys = 0;
				size = 0;
			}
		}
		if (phys)
			__ipcm_proc_printf__(data, " PHYS<0x%016X, 0x%08X>\n",
					phys, size);
		else
			__ipcm_proc_printf__(data, " PHYS<NULL, 0x%08X>\n",
					size);

		if ((nid == LOCAL_ID) || (!ipcm_node_ready(nid))) {
			continue;
		}
		/* every handler information which is opened */
		flag = 0;
		for (j=0; j<MAX_PORTS; j++) {
			handle = node->handlers[j];
			if (!handle)
				continue;
			if (!flag) {
				flag = 1;
				__ipcm_proc_printf__(data, "  |-Port | State       ");
				__ipcm_proc_printf__(data, " | Send Count | Recv Count");
				__ipcm_proc_printf__(data, " | Max Send Len | Max Recv Len\n");
			}
			switch(handle->state) {
				case __HANDLE_CONNECTED:
					state = "Connected";
					break;
				case __HANDLE_CONNECTING:
					state = "Connecting";
					break;
				case __HANDLE_DISCONNECTED:
					state = "Disconnected";
					break;
				default:
					state = "Fault";
					break;
			}
			__ipcm_proc_printf__(data, "    %-4d   %-12s",
					handle->port, state);
			__ipcm_proc_printf__(data, "   %-10d   %-10d",
					__ipcm_atomic_read__(&handle->send_count),
					__ipcm_atomic_read__(&handle->recv_count));
			__ipcm_proc_printf__(data, "   %-12d   %-12d\n",
					__ipcm_atomic_read__(&handle->max_send_len),
					__ipcm_atomic_read__(&handle->max_recv_len));
		}
	}
	return 0;
}


