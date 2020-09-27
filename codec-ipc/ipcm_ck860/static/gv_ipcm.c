/*****************************************************************
 * grand vision Inter-Processor conmmnuication source file
 * @file	gv_ipcm.c
 * @brief	Inter-Processor Commnunication
 * @author	xianfei.zhu@byavs.com 
 * @data	2019.03.27 	
 ****************************************************************/


#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <string.h>

#include "gv_ipcm.h"

#define IPCM_POOL_START 0x40000000 //high 1G
#define IPCM_POOL_SIZE  0x00100000 //SIZE 1M

GV_IPCM_MSG_S* GV_IPCM_CreateMessage(GV_IPCM_MSG_POOL_S *msg_pool,GV_U32 size,GV_U32 u32CMD,GV_BOOL bIsResp)
{
	GV_IPCM_MSG_S* pMsg = (GV_IPCM_MSG_S*)malloc(sizeof(GV_IPCM_MSG_S));
	if(!pMsg)
	{
		ipcm_trace(TRACE_ZXF_DEBUG,"malloc memory for ipcm message fail.");
		return NULL;
	}
	pMsg->msg_body = (struct ipcm_msg_body*)malloc(sizeof(struct ipcm_msg_body));
	if(!pMsg->msg_body)
	{
		ipcm_trace(TRACE_ZXF_DEBUG,"malloc memory for message body fail.");
		return NULL;
	}
//no need to memset to zero
	ipcm_trace(TRACE_ZXF_DEBUG,"alloc mem pool size is:%d",size);

	if( size > 0 )
	{
		long rc = xrp_allocate(&msg_pool->pool, size , 0x10,&pMsg->msg_alloc);
		if(rc < 0)
		{
			ipcm_err("allocate memory from pool fail!");
			goto createMsg_fail;
		}		
		pMsg->msg_body->start = pMsg->msg_alloc->start;
		//pMsg->msg_body->size = size; /* directly from alloc */
		pMsg->msg_body->size = pMsg->msg_alloc->size;
		ipcm_trace(TRACE_ZXF_DEBUG,"pMsg->msg_alloc->start:0x%x ,pMsg->msg_alloc->size:%d",pMsg->msg_alloc->start,pMsg->msg_alloc->size);
	} else {
	    //if no xrp allocate mem from memory pool,initlizatation to 0
		pMsg->msg_alloc = (struct xrp_allocation*)calloc(1,sizeof(struct xrp_allocation));
		pMsg->msg_body->start = 0;
		pMsg->msg_body->size = 0;
		ipcm_trace(TRACE_ZXF_DEBUG,"pMsg->msg_alloc->start:0x%x,pMsg->msg_alloc->size:%d",pMsg->msg_alloc->start,pMsg->msg_alloc->size);
	}
	
	pMsg->msg_body->u32CMD = u32CMD;
	pMsg->msg_body->bIsResp = bIsResp;


	ipcm_trace(TRACE_ZXF_DEBUG,"create Message successful!");
	return pMsg;

createMsg_fail:
	free(pMsg->msg_body);
	free(pMsg);
	return NULL;
}

GV_IPCM_RetValue_E GV_IPCM_DestroyMessage(GV_IPCM_MSG_S *pstMsg)
{
	if(NULL == pstMsg)
	{
		ipcm_err("Destroy message is NULL");
		return GV_SUCCESS;
	}

	if(pstMsg->msg_alloc != NULL)
	{		
		//if no
		if(pstMsg->msg_alloc->size > 0)
		{
			ipcm_trace(TRACE_ZXF_DEBUG,"pstMsg->msg_body->size: %d",pstMsg->msg_alloc->size);
			xrp_free(pstMsg->msg_alloc);
		}
		else
			{ free(pstMsg->msg_alloc);}
	}
	ipcm_trace(TRACE_ZXF_DEBUG,"pstMsg->msg_body.size: %d",pstMsg->msg_body->size);

	free(pstMsg->msg_body);
	free(pstMsg);

	pstMsg = NULL;
	ipcm_trace(TRACE_ZXF_DEBUG,"Destroy message successful!");

	return GV_SUCCESS;
}


GV_IPCM_RetValue_E  GV_IPCM_Connect(GV_IPCM_HANDLE_ATTR_S* stHandleAttr,GV_IPCM_MSG_POOL_S* ipcm_pool)
{
	int ret = 0;
	//ipcm_pool = (GV_IPCM_MSG_POOL_S*)malloc(sizeof(GV_IPCM_MSG_POOL_S));
	if (!ipcm_pool)
	{
		ipcm_trace(TRACE_ZXF_DEBUG,"malloc memory for ipcm message pool fail!");
		return -GV_ERR_MALLOC;
	}

	ipcm_pool->IpcmFd = open(DEV_NAME, O_RDWR);
	if (ipcm_pool->IpcmFd < 0) {
		ipcm_err("open %s fail.return:%d!",DEV_NAME,ipcm_pool->IpcmFd);
		return -GV_FAILED;
	}
	/*add pthread mutex lock*/
	pthread_mutex_init(&ipcm_pool->mutex,NULL);

	ipcm_trace(TRACE_ZXF_DEBUG,"%s connecting to target [%d:%d],ipcm_fd:%d ",DEV_NAME,stHandleAttr->target,stHandleAttr->port,ipcm_pool->IpcmFd);
	/*
	 * IOC_CONNECT wait and suspend until node connect success!
	 * ret = 0 :connect success ;ret !=0 connect fail
	 */
	ret = ioctl(ipcm_pool->IpcmFd, GV_IPCM_IOC_CONNECT, stHandleAttr);
	if(ret)
	{
		ipcm_trace(TRACE_ZXF_DEBUG,"connect fail!");
		goto connect_fail;
	}
	/*
	* Initilization ipcm memory pool
	*/
	xrp_init_pool(&ipcm_pool->pool,IPCM_POOL_START,IPCM_POOL_SIZE, &ipcm_pool->mutex);

	ipcm_trace(TRACE_ZXF_DEBUG,"connected target [%d,%d] ok~",stHandleAttr->target,stHandleAttr->port);
	return GV_SUCCESS;
	
connect_fail:
		close(ipcm_pool->IpcmFd);
		return -GV_FAILED;
}


GV_IPCM_RetValue_E GV_IPCM_TryConnect(GV_IPCM_HANDLE_ATTR_S* stHandleAttr,GV_IPCM_MSG_POOL_S* ipcm_pool)
{

	int ret = 0;
//	GV_IPCM_MSG_POOL_S* ipcm_pool = (GV_IPCM_MSG_POOL_S*)malloc(sizeof(*ipcm_pool)); /* ipcm_pool will malloc at outside */
	if (!ipcm_pool)
	{
		ipcm_trace(TRACE_ZXF_DEBUG,"malloc memory for ipcm message pool fail!");
		return -GV_ERR_MALLOC;
	}

	pthread_mutex_init(&ipcm_pool->mutex,NULL);

	/*
	* Initilization ipcm memory pool
	*/
	xrp_init_pool(&ipcm_pool->pool,IPCM_POOL_START,IPCM_POOL_SIZE,&ipcm_pool->mutex);

	/*
	* try to connect
	*/
	ipcm_pool->IpcmFd = open(DEV_NAME, O_RDWR);
	if (ipcm_pool->IpcmFd < 0) {
		ipcm_err("open %s fail.return:%d!",DEV_NAME,ipcm_pool->IpcmFd);
		return -GV_ERR_FILE;
	}
	
	 
	ipcm_trace(TRACE_ZXF_DEBUG,"%s trying to connect target [%d:%d],ipcm_fd:%d ",DEV_NAME,stHandleAttr->target,stHandleAttr->port,ipcm_pool->IpcmFd);
	/*
	 * IOC_try_CONNECT
	 * ret = 0 :connect success ;ret !=0 connect fail
	 */
	ret = ioctl(ipcm_pool->IpcmFd, GV_IPCM_IOC_TRY_CONNECT, stHandleAttr);
	if(ret)
	{
		ipcm_trace(TRACE_ZXF_DEBUG,"connect fail!");
		return -GV_FAILED; 
	} 
	return GV_SUCCESS;

}


/*
 * Destroy message before Disconnect
 * */
GV_IPCM_RetValue_E GV_IPCM_Disconnect(GV_IPCM_MSG_POOL_S *msg_pool)
{
	int ret = 0;
	ipcm_trace(TRACE_ZXF_DEBUG," Entry disconnect function");

	
	ret = ioctl(msg_pool->IpcmFd, GV_IPCM_IOC_DISCONNECT, NULL);
	if(ret < 0) {
		ipcm_err("Disconnect error!");
		return -GV_ERR_DISCONNECT;
	}
	
	
	close(msg_pool->IpcmFd);
	pthread_mutex_destroy(&msg_pool->mutex);
	//free(msg_pool);
	ipcm_trace(TRACE_ZXF_DEBUG,"disconnected OK");
	return GV_SUCCESS;
}


GV_IPCM_RetValue_E GV_IPCM_SendMessage(const GV_IPCM_MSG_S *pstMsg,GV_IPCM_MSG_POOL_S *msg_pool)
{
	int ret = 0;

	int msg_len = sizeof(struct ipcm_msg_body);

	ipcm_trace(TRACE_ZXF_DEBUG,"entry send message begin fd: %d,msg_len = %d",msg_pool->IpcmFd,msg_len);
	if(msg_len > MAX_SEND_LEN)
	{
		ipcm_err("send Message length is beyond of max_send_len");
		return -GV_EVALID;
	}
	ipcm_trace(TRACE_ZXF_DEBUG,"send message begin fd: %d",msg_pool->IpcmFd);
	ret = write(msg_pool->IpcmFd,(void *)pstMsg->msg_body,msg_len);
	if(ret != msg_len) {
		ipcm_err("write error which return %d",ret);
		ret = -GV_ERR_WRITE;
	}

	ipcm_trace(TRACE_ZXF_DEBUG,"send message after fd: %d",msg_pool->IpcmFd);
	return GV_SUCCESS;	
}

GV_IPCM_RetValue_E GV_IPCM_ReceiveMessage(GV_IPCM_MSG_POOL_S *msg_pool,GV_IPCM_MSG_S *pstMsgResp,GV_S32* msg_len)
{
	int ret = 0;	
	int len = 0;
	
	fd_set rfds;
	struct timeval timeout;

	if(!msg_pool->IpcmFd)
	{
		ipcm_err("file desc is error %d",msg_pool->IpcmFd);
		return -GV_EVALID;
	}

	ipcm_trace(TRACE_ZXF_DEBUG,"recv message before fd: %d",msg_pool->IpcmFd);	
	while(1) {
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_SET(msg_pool->IpcmFd, &rfds);
		ret = select(msg_pool->IpcmFd + 1, &rfds, NULL, NULL, &timeout);
		if (-1 == ret) {
			ipcm_err("SELECT error");
			ret = -GV_FAILED;
			break;
		} else if (!ret) {
			if(HANDLE_CONNECTED != ioctl(msg_pool->IpcmFd, GV_IPCM_IOC_CHECK, NULL)) {
				ipcm_err("Disconnected by remote,exit");				
				ret = -GV_FAILED;
				break;
			}
			/*if timeout and handle is connect,routine will infinite run*/
		}

		if (FD_ISSET(msg_pool->IpcmFd, &rfds)) {
			len = read(msg_pool->IpcmFd,(void *)pstMsgResp->msg_body,MAX_SEND_LEN);
			if(len < 0) {
				ipcm_err("read error %d.",ret);
				ret = -1;
				break;
			}
			*msg_len = len;
			ret = GV_SUCCESS;
			ipcm_trace(TRACE_ZXF_DEBUG,"Read message length %d.",*msg_len);
			break;
		}
	}
	return ret;
}


GV_IPCM_RetValue_E GV_IPCM_SendsyncMessage(GV_IPCM_MSG_POOL_S *msg_pool,const GV_IPCM_MSG_S *pstMsg,GV_IPCM_MSG_S *pstMsgResp)
{
	int ret = 0;	
	int len;
	
	fd_set rfds;
	struct timeval timeout;
	int msg_len = sizeof(struct ipcm_msg_body);

	ret = write(msg_pool->IpcmFd,pstMsg->msg_body,msg_len);
	if(ret != msg_len) {
		ipcm_err("write error which return %d.",ret);
		ret = -GV_ERR_WRITE;
	}

	while(1) {
		
		timeout.tv_sec  = 2;
		timeout.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_SET(msg_pool->IpcmFd, &rfds);
		ret = select(msg_pool->IpcmFd + 1, &rfds, NULL, NULL, &timeout);

		if (-1 == ret) {
			ipcm_err("SELECT error");
			break;
		} else if (!ret) {
			if(HANDLE_CONNECTED != ioctl(msg_pool->IpcmFd, GV_IPCM_IOC_CHECK, NULL)) {
				ipcm_err("Disconnected by remote,exit.");				
				ret = -GV_FAILED;
				break;
			}
		}
		
		if (FD_ISSET(msg_pool->IpcmFd, &rfds)) {
			len = read(msg_pool->IpcmFd,(void *)pstMsgResp->msg_body,MAX_SEND_LEN);
			if(len < 0) {
				ipcm_err("read error %d.",ret);
				ret = -GV_FAILED;
				break;
			}
			/*read complete!*/
			ipcm_trace(TRACE_ZXF_DEBUG,"read sync message length %d.",len);
			ret = GV_SUCCESS;
			break;
		}
	}
	
	return ret;
}

/*
 * Check handle is connected or not
 */
GV_BOOL GV_IPCM_CheckConnected(GV_IPCM_MSG_POOL_S *msg_pool)
{	

	if(!msg_pool->IpcmFd)
	{
		ipcm_err("file desc error %d.",msg_pool->IpcmFd);
		return GV_FALSE;
	}

	int state = HANDLE_DISCONNECTED;
	state =	ioctl(msg_pool->IpcmFd,GV_IPCM_IOC_CHECK,NULL);
	
	if(HANDLE_CONNECTED == state)
		return GV_TRUE;
	else 
		return GV_FALSE;
	
}





