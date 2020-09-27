/*
 * 
 * author:zhuxianfei@byavs.com
 * 
 * @brief: create two pthread one for receive message 
 * and the other use for send message
 * detail:at send message pthread,1.create message with msg attr
 * 2. send message 3. destroy message
 * At receive message pthread.select read message
 */

#include <pthread.h>

#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <locale.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>

#include "gv_ipcm.h"

#define IPCM_CMD_OPEN_CODEC (1)
#define IPCM_CMD_ENABLE_CODEC (2)
#define IPCM_CMD_CLOSE_CODEC (3)

#define IPCM_CMD_PLAY_AUDIO (4)
#define IPCM_CMD_RECORD_AUDIO (5)


void* send_message_fn(void *arg)
{

	printf("entry pthread: %s\n",__func__);
	GV_IPCM_HANDLE_ATTR_S* stHandleAttr = (GV_IPCM_HANDLE_ATTR_S* )arg;
	int msg_len = 0;
	int ret = 0;
	int i = 3;

	GV_IPCM_MSG_POOL_S* fd_pool = NULL;
	fd_pool = (GV_IPCM_MSG_POOL_S* )malloc(sizeof(GV_IPCM_MSG_POOL_S));
	if(!fd_pool)
	{
		printf("malloc for fd_pool fail!\n");
		exit(-1);
	}

	if(GV_SUCCESS != GV_IPCM_Connect(stHandleAttr,fd_pool))
	{
	     printf("Connect fail\n");
       	 exit(-1);
	}
	printf("connect to target success!\n");

	do {
		//1.创建消息
		GV_IPCM_MSG_S* pMessage =  GV_IPCM_CreateMessage(fd_pool,0,i,1);
		GV_IPCM_MSG_S* pstMsgResp = GV_IPCM_CreateMessage(fd_pool,0,0,0);
		//2.发送消息
//		GV_IPCM_SendMessage(pMessage, fd_pool);
		ret = GV_IPCM_SendsyncMessage(fd_pool, pMessage , pstMsgResp);
		if(GV_SUCCESS != ret)
		{
			printf("remote node is disconneted returnNum[%d] \n",ret);
			break;
		} 
		printf("recv sync message,msg_body->start:0x%x,msg_body->size:%d,msg_body->bIsResp:%d\n",pstMsgResp->msg_body->start,pstMsgResp->msg_body->size,pstMsgResp->msg_body->bIsResp);
		//3.销毁消息
		GV_IPCM_DestroyMessage(pMessage);
		GV_IPCM_DestroyMessage(pstMsgResp);
	} while(i--);

	printf("Enter q to exit\n");
	char cmd[64];

    while (0 != strncmp(fgets(cmd, 64, stdin), "q", 1))
    {
        printf("Enter q to exit\n");
    }

    GV_IPCM_Disconnect(fd_pool);
	printf("exit pthread: %s\n",__func__);
	return NULL;

}

void* sendsync_message_fn(void *arg)
{

}

int main()
{

	pthread_t sendpid;
	int ret = 0;
	GV_IPCM_HANDLE_ATTR_S stConnectAttr;
	stConnectAttr.target = TARGET_NODE;
	stConnectAttr.port = 1;
	stConnectAttr.priority = HANDLE_MSG_NORMAL;//msg_priority
//	stConnectAttr.priority = HANDLE_MSG_PRIORITY;

//	ipcm_send_message(&stConnectAttr);

	ret = pthread_create(&sendpid,NULL, send_message_fn,&stConnectAttr);
	if(ret) {
		printf("err, create send_message_fn fail.ret:%d\n",ret);
		return -1;
	}	
	
	pthread_join(sendpid,NULL);
	printf("The sendpid pthread is finish!\n");

	char cmd[64];

    while (0 != strncmp(fgets(cmd, 64, stdin), "q", 1))
    {
        printf("Enter q to exit\n");
    }

    printf("exit main\n");

	return 0;
}
