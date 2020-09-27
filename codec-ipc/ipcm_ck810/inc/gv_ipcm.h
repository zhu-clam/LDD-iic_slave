/* **************************************************************
 * grand vision Inter-Processor conmmnuication header file
 * @file	gv_ipcm.h
 * @brief	Inter-Processor Commnunication
 * @author	xianfei.zhu@byavs.com 
 * @data	2019.03.27 	
 ****************************************************************/


#include <linux/ioctl.h>

#ifndef __GV_IPCM_H__
#define __GV_IPCM_H__

#include "gv_ipc_type.h"
#include "xrp_alloc.h"
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/* macro define for Debug program */
#define IPCM_TRACE_MASK		(0x1)
//#define IPCM_TRACE_MASK	(0)
#define TRACE_ZXF_DEBUG 	(1<<0) 

#define ipcm_trace(mask, ...) do { \
	if (mask & IPCM_TRACE_MASK) { \
		pr_ipc("<hal>[%s:%d] ", __func__, __LINE__); \
		printf(__VA_ARGS__); \
		pr_ipc("\r\n"); \
	} \
} while (0)


#define ipcm_err(s...) do { \
	pr_ipc("<hal_err>[%s:%d] ", __func__, __LINE__); \
	pr_ipc(s); \
	pr_ipc("\r\n"); \
} while (0)

static inline void pr_ipc(const char *fmt, ...)
{
	printf(fmt);
}


#if 0
#define ipcm_trace1(mask,format, ...) do {  \
	if (mask & IPCM_TRACE_MASK) {		 \
        printf(format, __VA_ARGS__);    \
    }				                      \
} while(0)
#endif


struct ipcm_msg_body {
	phys_addr_t start;
	GV_U32 size;
	GV_U32 u32CMD;
	GV_BOOL bIsResp;
} ;

typedef struct ipcm_msg_pool 
{
	FD IpcmFd;
	struct xrp_allocation_pool pool;
} GV_IPCM_MSG_POOL_S ;

typedef struct gv_ipcm_msg_struct {
	
	struct xrp_allocation* msg_alloc; /*用于申请内存，释放内存*/
	struct ipcm_msg_body* msg_body;  /*写入共享内存的消息体,segment default*/

} GV_IPCM_MSG_S;



/**
 * @brief Create message, used by GV_IPCM_SendMessage 
 * @param[in] pBuf Message contents what is want to be transfer
 * @param[in] u32MessageLen Length of pBuf
 * @return GV_IPCM_MSG_S* created message.
 * @return NULL create message fail
 * */
//GV_IPCM_MSG_S* GV_IPCM_CreateMessage(GV_VOID *pBuf,GV_U32 u32MessageLen); before 

GV_IPCM_MSG_S* GV_IPCM_CreateMessage(GV_IPCM_MSG_POOL_S *msg_pool,GV_U32 size,GV_U32 u32CMD,GV_BOOL bIsResp);


//GV_IPCM_MSG_S* GV_IPCM_CreateMessageResp(GV_VOID *pBuf,GV_U32 u32MessageLen);

/**
 * @brief Destroy the message. Message must be destroyed when send and receive finish
 * @param[in] pMsg Message to destroy.
 * @return GV_SUCCESS destroy message success
 * @return GV_EVALID Destroy message is NULL
 */
GV_IPCM_RetValue_E GV_IPCM_DestroyMessage(GV_IPCM_MSG_S *pstMsg);

/*
 * @brief wait and suspend until connect to remote node.
 * @param[out] pIpcmFd file descriptor of device node. 
 * @param[in] stHandleAttr ipcm handle attribute.
 */
GV_IPCM_RetValue_E	GV_IPCM_Connect(GV_IPCM_HANDLE_ATTR_S* stHandleAttr,GV_IPCM_MSG_POOL_S* ipcm_pool);

/*
 * @brief try to connect to remote node.
 * @param[out] pIpcmFd file descriptor of device node. 
 * @param[in] stHandleAttr ipcm handle attribute.
 * @return GV_ERR_FILE open ipcm device file fail.
 * @return -GV_FAILED connected to remote node failed.
 * @return GV_SUCCESS connected to remote node successful.
 */ 
//GV_IPCM_MSG_POOL_S* GV_IPCM_TryConnect(const GV_IPCM_HANDLE_ATTR_S stHandleAttr);
GV_IPCM_RetValue_E GV_IPCM_TryConnect(GV_IPCM_HANDLE_ATTR_S* stHandleAttr,GV_IPCM_MSG_POOL_S* ipcm_pool);


/* @brief Disconnect when don't want to send or receive message.
 * @param[in] pIpcmFd file descriptor of device node. 
 * @return GV_ERR_DISCONNECT disconnect error.
 * @return GV_SUCCESS disconnect success.
 */
GV_IPCM_RetValue_E GV_IPCM_Disconnect(GV_IPCM_MSG_POOL_S *msg_pool);

/*
 * @brief Send message asynchronously. the function will return immediately.
 * @param[in] pstMsg Message to send.
 * @param[in] IpcmFd file descriptor of device node. 
 * @return HI_SUCCESS Send success.
 * @return GV_ERR_WRITE Send fail. 
 */
GV_IPCM_RetValue_E GV_IPCM_SendMessage(const GV_IPCM_MSG_S *pstMsg,GV_IPCM_MSG_POOL_S *msg_pool);

/*
* @brief Send message synchronously.the function will block until response message received.
* 
* @param[in] IpcmFd:file descriptor of device /dev/ipcm node.
* @param[in] pstMsg:Message to send.
* @param[out] pBuf pointer to receive message.
* @return GV_TIMEOUT 
* @return GV_SUCCESS 
*/
GV_IPCM_RetValue_E GV_IPCM_SendsyncMessage(GV_IPCM_MSG_POOL_S *msg_pool,const GV_IPCM_MSG_S *pstMsg,GV_IPCM_MSG_S *pstMsgResp);
//GV_IPCM_RetValue_E GV_IPCM_SendsyncMessage(const FD IpcmFd,const GV_IPCM_MSG_S *pstMsg,GV_VOID *pBuf,GV_S32 *ps32MsgLength);

/**
* @brief receive message, will block caller until read message success.
* @param[in] IpcmFd file descriptor of device node. 
* @param[out] pBuf pointer to receive message 
* @param[out] ps32MsgLength receive message's length 
* @return GV_EVALID read buf is invalid
* @return GV_FAILED remote node is disconnected before received message.
* @return GV_SUCCESS read message success. 
 */
//GV_IPCM_RetValue_E GV_IPCM_ReceiveMessage(const FD IpcmFd,GV_VOID *pBuf,GV_S32 *ps32MsgLength);
GV_IPCM_RetValue_E GV_IPCM_ReceiveMessage(GV_IPCM_MSG_POOL_S *msg_pool ,GV_IPCM_MSG_S *pstMsgResp,GV_S32 *msg_len);

/*
* @brief check ipcm node whether is connected or not
* @param[in] IpcmFd file descriptor of device node. 
* @return GV_FALSE remote node isnot connected.
* @return GV_TRUE remote node is connected.
*/
GV_BOOL GV_IPCM_CheckConnected(GV_IPCM_MSG_POOL_S *msg_pool);

//phys_addr_t GV_IPCM_AllocMem(GV_IPCM_MSG_POOL_S *msg_pool,GV_U32 size, GV_U32 align);

//GV_VOID GV_IPCM_FreeMem(GV_IPCM_MSG_POOL_S *msg_pool);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus*/

#endif /* __GV_IPCM_H__ end*/

