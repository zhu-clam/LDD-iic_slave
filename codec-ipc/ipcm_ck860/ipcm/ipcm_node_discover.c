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
*@Filename: ipcm_node_discover.c                                             
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


#include "ipcm_desc.h"
#include "ipcm_osadapt.h"
#include "device_config.h"
#include "ipcm_nodes.h"
#include "ipcm_buffer.h"
#include "ipcm_config_common.h"

extern struct ipcm_node_desc *g_nodes_desc;
extern struct ipcm_task *node_detect_task;

void set_local_alive(void)
{
	volatile unsigned int *state = &g_nodes_desc[LOCAL_ID].state;
	*state = *state | (1<<31);//g_nodes_desc[1].state (1<<31);
}

void clear_local_alive(void)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	desc[LOCAL_ID].state &= ~(1<<31);
}

/*set local node is ready for nid node */
void set_local_ready(unsigned int nid)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	if (MAX_NODES <= nid) {
		ipcm_err("invalid node id %d", nid);
	}
	desc[LOCAL_ID].state |= (1<<(nid + LOCAL_STATE_SHIFT));
}

void clear_local_ready(unsigned int nid)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	if (MAX_NODES <= nid) {
		ipcm_err("invalid node id %d", nid);
	}
	desc[LOCAL_ID].state &= ~(1<<(nid + LOCAL_STATE_SHIFT));
}
void clear_node_ready(unsigned int nid)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	if (MAX_NODES <= nid) {
		ipcm_err("invalid node id %d", nid);
	}
	desc[nid].state &= ~(1<<(LOCAL_ID + LOCAL_STATE_SHIFT));//
}

int is_local_ready(unsigned int nid)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	if (MAX_NODES <= nid) {
		ipcm_err("invalid node id %d", nid);
		return 0;
	}
	if (desc[LOCAL_ID].state & (1<<(nid + LOCAL_STATE_SHIFT))) {
		ipcm_trace(TRACE_DESC, "local for node %d is ready!", nid);
		return 1;
	}

	return 0;
}

int is_node_alive(unsigned int nid)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	if (MAX_NODES <= nid) {
		ipcm_err("invalid node id %d", nid);
		return 0;
	}
	if (desc[nid].state & (1<<31)) {//desc[1].state & (1<<31)
//		ipcm_trace(TRACE_DESC, "node %d is alive!", nid);
		return 1;
	}
	return 0;
}

int is_node_ready(unsigned int nid)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	if (MAX_NODES <= nid) {
		ipcm_err("invalid node id %d", nid);
		return 0;
	}
	if (desc[nid].state & (1<<(LOCAL_ID + LOCAL_STATE_SHIFT))) {
		ipcm_trace(TRACE_DESC, "node %d is ready!", nid);
		return 1;
	}
	return 0;
}

int is_node_connected(unsigned int nid)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	struct ipcm_node *node;

	if (MAX_NODES <= nid) {
		ipcm_err("invalid node id %d", nid);
		return 0;
	}
	
	if (desc[LOCAL_ID].state & (1 << nid))
		return 1;

	
	node = ipcm_get_node(nid);
	if (NULL == node) {
		ipcm_err("invalid node %d", nid);
		return 0;
	}
	if (NODE_READY == node->state) {
		__ipcm_mem_free__(node->handlers);
		node->handlers = NULL;
		__ipcm_mem_free__(node->handlers_state);
		node->handlers_state = NULL;
		node->state = NODE_HALT;
	}

	return 0;
}

int set_node_connected(unsigned int nid)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	struct ipcm_node *node;

	if (MAX_NODES <= nid) {
		ipcm_err("invalid node id %d", nid);
		return 0;
	}


	node = ipcm_get_node(nid);
	if (NULL == node) {
		ipcm_err("invalid node %d", nid);
		return -1;
	}
	node->handlers = __ipcm_mem_alloc__(MAX_PORTS * sizeof(void *));
	__memset__(node->handlers, 0, MAX_PORTS * sizeof(void *));
	node->handlers_state = __ipcm_mem_alloc__(MAX_PORTS * sizeof(void *));
	__memset__(node->handlers_state, 0, MAX_PORTS * sizeof(void *));

	desc[LOCAL_ID].state |= (1<<nid);

	while (!(desc[nid].state & (1<<LOCAL_ID))) {
		__ipcm_msleep__(10);
	}

	node->state = NODE_READY;
	return 1;
}


void clear_node_connected(unsigned int nid)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	struct ipcm_node *node;

	if (MAX_NODES <= nid) {
		ipcm_err("invalid node id %d", nid);
		return;
	}
	desc[LOCAL_ID].state &= ~(1<<nid);//desc[0].state 1 << nid set 0
	desc[nid].state &= ~(1<<LOCAL_ID);//desc[1].state 1 << Local set 0

	node = ipcm_get_node(nid);
	if (NULL == node) {
		ipcm_err("invalid node %d", nid);
		return;
	}
	if (NODE_READY != node->state) {
		return;
	}
	__ipcm_mem_free__(node->handlers);
	node->handlers = NULL;

	__ipcm_mem_free__(node->handlers_state);
	node->handlers_state = NULL;

	node->state = NODE_HALT;
}

int is_allnodes_connected(void)
{
	struct ipcm_node_desc *desc = g_nodes_desc;

#if 0
#ifdef NO_MULTITASKS
	if ((0x1) == (0xff & desc[LOCAL_ID].state))
#else
	if (0xff == (0xff & desc[LOCAL_ID].state))
//	if (0x33 == (0xff & desc[LOCAL_ID].state))//01-3 89-3
#endif
#endif
	if (0x33 == (0xff & desc[LOCAL_ID].state)){
		ipcm_info("%s,__ All counter nodes ready!",__func__);
		return 1;
	}
	return 0;
}

#if 0
static unsigned int get_recvbuf_pfn(unsigned int holdnid, unsigned int nid)
{
	struct ipcm_node_desc *desc = g_nodes_desc;
	if (nid < holdnid)
		return desc[holdnid].recvbuf_pfn[nid];
	else if (nid > holdnid)
		return desc[holdnid].recvbuf_pfn[nid - 1];
	else {
		ipcm_err("nid == holdnid");
		return 0;
	}
}
#endif
//nid = 0 ；hnid = 1
void set_desc_info(int nid, int hnid, unsigned int pfn, unsigned int size)
{
	struct ipcm_node_desc *desc = g_nodes_desc;

	ipcm_trace(TRACE_DESC, "nid %d hnid %d, pfn 0x%x, size 0x%x",
			nid, hnid, pfn, size);
	ipcm_trace(TRACE_DESC, "addr of desc[%d].recvbuf_pfn[%d] %p",
			hnid, nid, &desc[hnid].recvbuf_pfn[nid]);
			
	if (hnid > nid)
		desc[hnid].recvbuf_pfn[nid] = pfn;
	else if (hnid < nid)
		desc[hnid].recvbuf_pfn[nid - 1] = pfn;
	else
		ipcm_err("hnid = nid = %d", nid);

}
//nid = 1；hind = 0
void get_desc_info(int nid, int hnid, unsigned int *pfn, unsigned int *size)
{
	unsigned long zone_base;
	struct ipcm_node_desc *desc = g_nodes_desc;
	char *addr;
	int val;


	if (nid < hnid) {
		*pfn = desc[hnid].recvbuf_pfn[nid];
		ipcm_trace(TRACE_DESC, "pfn 0x%x, desc %p [%d%d] &pfn %p",
			*pfn, desc, hnid, nid, &desc[hnid].recvbuf_pfn[nid]);
	} else if (nid > hnid) {
		*pfn = desc[hnid].recvbuf_pfn[nid - 1];
		ipcm_trace(TRACE_DESC, "pfn 0x%x, desc %p [%d%d] &pfn %p",
			*pfn, desc, hnid, nid, &desc[hnid].recvbuf_pfn[nid - 1]);
	} else {
		*pfn = 0;
		*size = 0;
		ipcm_err("nid == hnid");
		return;
	}

	zone_base = (*pfn) << PAGE_SHIFT;
	ipcm_trace(TRACE_DESC, "zonebase 0x%lx", zone_base);
	addr = __ipcm_io_mapping__(zone_base, 0x1000);

	val = *(int *)(addr);
	if (ZONE_MAGIC != val) {
		ipcm_err("zone magic err[0x%x/0x%x]!", val, ZONE_MAGIC);
//		IPCM_BUG();
	}

	*size = *(int *)(addr + 4);
	//*(int *)(zone->region.base + 4) = zone->region.size;

	__ipcm_io_unmapping__(addr);

}

/*
 * Initialization for zone that contains messages sent from
 * node nid to local.
 * Only if local is slave, would this function be called.
 * If local is master, the initialization needs to be done
 * by node nid, and the master will call get_zone_from_node
 * instead.
 */
static int init_zone_from_node(int nid)
{
	int ret;
	unsigned int pfn;
	unsigned int size;
	unsigned long phys;

	if (0 == LOCAL_ID) {
		/*
		 * master does not actually init any shared zone
		 * by itself, slaves do it instead.
		 */
		return 0;
	}
#if 0 
//by zxf
	/*
	 * get zone information from configuration files.
	 * and supposed that 0 phys is illegal here, roughly.
	 * 从配置文件中获取 共享区信息
	 */
	cfg_2local_shm_phys(nid, phys);//宏定义，从配置文件中获取phys 地址
	if (phys == 0) {
		ipcm_trace(TRACE_MEM, "cfg err: phys = 0");
		return -1;
	}

	cfg_2local_shm_size(nid, size);//宏定义，从配置文件中获取size大小
	if (size == 0) {
		ipcm_trace(TRACE_MEM, "cfg err: size = 0");
		return -1;
	}
#endif
	phys = __SHARE_MEM_RECV_BASE__;
	size = __SHARE_MEM_SIZE__;

	ipcm_trace(TRACE_ZXF_DEBUG,"phys 0x%x, size 0x%x", phys, size);
#if 0
	/* this part of work would be done in ipcm_shared_zone_init */
	zone_base = __ipcm_io_mapping__(phys, size);
	if (zone_base == NULL) {
		ipcm_err("io mapping failed!");
		return -1;
	}
#endif
	ret = ipcm_shared_zone_init(nid, phys, size, 0);
	if (ret) {
		ipcm_err("[%d->%d] shared zone init failed!",
				nid, LOCAL_ID);
		return -1;
	}

	pfn = phys >> PAGE_SHIFT;
	set_desc_info(nid, LOCAL_ID, pfn, size);

	return 0;
}

/*
 * Initialization for zone that contains messages sent from
 * local to node nid.
 * Only if remote is master, would this function be called,
 * because slaves will always initialize its message-receive
 * zone by itself.
 */
static int init_zone_to_node(int nid)
{
	int ret;
	unsigned int pfn;
	unsigned long phys;
	unsigned int size;

	if (0 == nid) {
#if 0		
		cfg_2remote_shm_phys(nid, phys);
		if (phys == 0) {
			ipcm_trace(TRACE_MEM, "cfg err: phys = 0");
			return -1;
		}

		cfg_2remote_shm_size(nid, size);
		if (size == 0) {
			ipcm_trace(TRACE_MEM, "cfg err: size = 0");
			return -1;
		}
#endif
		phys = __SHARE_MEM_SEND_BASE__;
		size = __SHARE_MEM_SIZE__;
		
		ipcm_trace(TRACE_ZXF_DEBUG,"phys 0x%x, size 0x%x", phys, size);
		
		ret = ipcm_shared_zone_init(nid, phys, size, 1);
		if (ret) {
			ipcm_err("[%d->%d] shared zone init failed!",
				nid, LOCAL_ID);
			return -1;
		}

		pfn = phys >> PAGE_SHIFT;
		ipcm_trace(TRACE_ZXF_DEBUG,"pfn 0x%x, size 0x%x", pfn, size);
		set_desc_info(LOCAL_ID, nid, pfn, size);
	}

	return 0;
}

void release_zone_from_node(int nid)
{

}

void release_zone_to_node(int nid)
{
}

/*
 * Get information of zone which contains messages sent from
 * node nid to local.
 * The information was prepare by node nid, and the initialization
 * of the zone was also done by node nid.
 * Only if local is master, should this function be called, otherwise
 * init_zone_from_node would be called instead.
 */

static int get_zone_from_node(int nid)
{
	int ret;
	unsigned int pfn;
	unsigned int size;
	unsigned long phys;

	if (nid == 0) {
		ipcm_err("nid should never be 0");
		return -1;
	}

	if (LOCAL_ID != 0) {
		ipcm_err("local id should be 0");
		return -1;
	}

	get_desc_info(nid, 0, &pfn, &size);

	phys = ((unsigned long)pfn) << PAGE_SHIFT;
	ret = ipcm_shared_zone_init(nid, phys, size, 0);//LOCAL = 0 ,nid = 1
	if (ret) {
		ipcm_err("[%d->%d] shared zone init failed!",
				nid, LOCAL_ID);
		return -1;
	}
	ipcm_trace(TRACE_ZXF_DEBUG,"from %d phys 0x%x, size 0x%x", nid, phys, size);

	return 0;
}

/*
 * Get information of zone contains messages sent from local
 * to node nid.
 * The information is prepare by node nid.
 * Only if remote is slave, would this function be called.
 * If node nid is master, then init_zone_to_node should be
 * called instead.
 */
static int get_zone_to_node(int nid)
{
	int ret;
	unsigned int pfn;
	unsigned int size;
	unsigned long phys;

	if (nid == 0) {
		ipcm_err("nid should never be 0");
		return -1;
	}

	get_desc_info(LOCAL_ID, nid, &pfn, &size);
	phys = pfn << PAGE_SHIFT;
	ret = ipcm_shared_zone_init(nid, phys, size, 1);
	if (ret) {
		ipcm_err("[%d->%d] shared zone init failed!",
				nid, LOCAL_ID);
		return -1;
	}
	ipcm_trace(TRACE_ZXF_DEBUG,"to %d phys 0x%x, size 0x%x", nid, phys, size);

	return 0;
}


int do_connecting(int nid)
{
	int ret;

	//ipcm_info("do connecting ...");

	if (!is_local_ready(nid) && !is_node_ready(nid)) {
		if (0 == LOCAL_ID)
			return 0;
//		ipcm_info("local %d not ready and %d not ready",
//					LOCAL_ID, nid);
		ret = init_zone_from_node(nid);
		if (ret) {
			ipcm_trace(TRACE_INIT, "init msg zone[from %d to %d] failed!",
					nid, LOCAL_ID);
			release_zone_from_node(nid);
			return -1;
		}

		if (0 == nid) {
			ret =init_zone_to_node(nid);
			if (ret) {
				ipcm_trace(TRACE_INIT, "init msg zone[from %d to %d] failed!",
					LOCAL_ID, nid);
				release_zone_to_node(nid);
				return -1;
			}
		}
		set_local_ready(nid);
		return 0;
	}

	if (!is_local_ready(nid) && is_node_ready(nid)) {
//		ipcm_info("local %d not ready and %d  ready",
//				LOCAL_ID, nid);
		if (0 != LOCAL_ID) {
			ret = init_zone_from_node(nid);
			if (ret) {
				ipcm_trace(TRACE_INIT, "init msg zone[from %d to %d] failed!",
						nid, LOCAL_ID);
				release_zone_from_node(nid);
				return -1;
			}
		} else {
			ret = get_zone_from_node(nid);
			if (ret) {
				ipcm_trace(TRACE_INIT, "get msg zone[from %d to %d] failed!",
						nid, LOCAL_ID);
				release_zone_from_node(nid);
				return -1;
			}
		}

		ret = get_zone_to_node(nid);
		if (ret) {
			ipcm_err("get msg zone[from %d to %d] failed!",
					LOCAL_ID, nid);
			release_zone_from_node(nid);
			return -1;
		}


		set_local_ready(nid);
		set_node_connected(nid);
		return 0;
	}

	if (is_local_ready(nid) && is_node_ready(nid)) {
//		ipcm_info("local %d  ready and %d  ready",
//			LOCAL_ID, nid);
		if (LOCAL_ID == 0) {
			/* normally this is not going to happen. */
			set_node_connected(nid);/*Because of node connected will not entry to this*/
			return 0;
		}

		if (nid == 0) {
			set_node_connected(nid);
			return 0;
		}

		ret = get_zone_to_node(nid);
		if (ret) {
			ipcm_err("get msg zone[from %d to %d] failed!",
					LOCAL_ID, nid);
			release_zone_from_node(nid);
			return -1;
		}
		set_node_connected(nid);
	}

	/*
	 * last case : local ready && node not ready
	 * in this case, we can do nothing, just return 0;
	 */
	//set_node_connected(nid);
	//ipcm_trace(TRACE_ZXF_DEBUG,"local %d  ready and %d node not ready",
	//	LOCAL_ID, nid);

	return 0;
}



int try_connect_node(int nid)
{
	if (nid == LOCAL_ID)
		return 0;

	if (is_node_connected(nid))//nid = 1
		return 0;

	
	if (!is_node_alive(nid))
		return 0;


//	ipcm_info("nid = %d is alive,LOCAL_ID = %d",nid,LOCAL_ID);
	if (do_connecting(nid)) {
		ipcm_trace(TRACE_INIT, "connected to node %d failed!", nid);
		return -1;
	}

	return 0;
}

static int node_release = 0;

extern void ipcm_shared_zone_release(int nid, int sendbuf);


int ipcm_nodes_detecting(void *p)
{
	unsigned int nid;
	int ret;

	/* clear all status include connect and ready flags */
	for (nid = 0; nid < MAX_NODES; nid++) {
		clear_local_ready(nid);
		clear_node_connected(nid);
		clear_node_ready(nid); 
	}
	set_local_alive();

	ipcm_trace(TRACE_ZXF_DEBUG,"detecting thread running!\n");
	do {
		for (nid = 0; nid < MAX_NODES; nid++) {
			ret = try_connect_node(nid);
			if (ret < 0) {
				ipcm_trace(TRACE_INIT, "try connect node %d failed!", nid);
			}
		}

#if 0
		if (is_allnodes_connected()) {
			ipcm_info("All counter nodes ready!");
			goto discovery_out;
		}
#endif

		__ipcm_msleep__(10);

	} while (!__ipcm_thread_check_stop__(node_detect_task));

//discovery_out:
	/* clear all status include connect and ready flags */
	if (node_release) {
		for (nid = 0; nid < MAX_NODES; nid++) {
			clear_local_ready(nid);
			clear_node_connected(nid);
			clear_node_ready(nid);
			ipcm_shared_zone_release(nid,0);//nid = 1;LOCAL ID = 0;recvbuf
			
			ipcm_shared_zone_release(nid,1);//nid = 1;LOCAL ID = 0;sendbuf
		}
		clear_local_alive();
	}

	ipcm_info("Nodes discovery thread quit!");
	return 0;
}

void ipcm_node_release(int release)
{
	node_release = release;
#if 0 	
	int nid;
	for (nid = 0; nid < MAX_NODES; nid++) {
			clear_local_ready(nid);
			clear_node_connected(nid);
			clear_node_ready(nid);
		}
		clear_local_alive();
#endif	
}
