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
*@Filename: ipcm_userdev.h                                             
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


#include <linux/ioctl.h>

#ifndef __GV_IPCM_USERDEV_H__
#define __GV_IPCM_USERDEV_H__

#define IPCM_MAX_DEV_NR			2

enum handle_state {
	HANDLE_DISCONNECTED,
	HANDLE_CONNECTING,
	HANDLE_CONNECTED
};

enum handle_priority {
	HANDLE_MSG_NORMAL,
	HANDLE_MSG_PRIORITY
};

struct ipcm_handle_attr {
	int target;
	int port;
	int priority;
};

#define	GV_IOC_IPCM_BASE  'M'

/* Create a new ipcm handle. A file descriptor is only used*
 * once for one ipcm handle. */
#define GV_IPCM_IOC_CONNECT  \
	_IOW(GV_IOC_IPCM_BASE, 1, struct ipcm_handle_attr)
#define GV_IPCM_IOC_TRY_CONNECT  \
	_IOW(GV_IOC_IPCM_BASE, 2, struct ipcm_handle_attr)

#define GV_IPCM_IOC_CHECK  \
	_IOW(GV_IOC_IPCM_BASE, 3, unsigned long)
#define GV_IPCM_IOC_DISCONNECT  \
	_IOW(GV_IOC_IPCM_BASE, 4, unsigned long)
#define GV_IPCM_IOC_GET_LOCAL_ID \
	_IOW(GV_IOC_IPCM_BASE, 5, unsigned long)
#if 0
#define GVSP_IPCM_IOC_GET_REMOTE_ID \
	_IOW(GVSP_IOC_IPCM_BASE, 6, struct ipcm_handle_attr)
#define GVSP_IPCM_IOC_ATTR_INIT \
	_IOW(GVSP_IOC_IPCM_BASE, 7, struct ipcm_handle_attr)
#endif

#endif  /* __GV_IPCM_H__ */
