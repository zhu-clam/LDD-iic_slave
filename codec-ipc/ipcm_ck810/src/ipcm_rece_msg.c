/*************************************************************************
	> File Name: codec_receiver.c
	> Author: ma6174
	> Mail: ma6174@163.com 
	> Created Time: Thu 11 Apr 2019 11:12:47 PM PDT
 ************************************************************************/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>



#include "gv_ipcm.h"
#include "testdata.h"
#include "defineall.h"


/*Define ipcm message module*/
//#define IPCM_MODULE_AUDIO_CODEC  'a'

/*Define ipcm message cmd*/
#define IPCM_CMD_OPEN_CODEC (1)
#define IPCM_CMD_ENABLE_CODEC (2)
#define IPCM_CMD_CLOSE_CODEC (3)
#define MEM_SIZE 4096


void* recv_msg_fn(void* arg)
{
	printf("entry pthread: %s\n",__func__);
	GV_IPCM_HANDLE_ATTR_S* stHandleAttr = (GV_IPCM_HANDLE_ATTR_S* )arg;
	int msg_len = 0;
	int ret = 0;
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

	
	printf("connect to target success! port:%d\n",stHandleAttr->port);
	GV_IPCM_MSG_S* pMsg = GV_IPCM_CreateMessage(fd_pool, 0, 0, 0);

	while(1)
	{
		ret =  GV_IPCM_ReceiveMessage(fd_pool, pMsg, &msg_len);
		if(ret != GV_SUCCESS)
		{
			printf("remote node is disconneted returnNum[%d] \n",ret);
			break;			
		} else if (msg_len == 0)
		{
			printf("GV_IPCM_ReceiveMessage len: %d \n",msg_len);
			break;
		}
		printf("Message phy addr:0x%#x ,size:%d,CMD:%d,IsResp:%d \n",pMsg->msg_body->start,pMsg->msg_body->size,pMsg->msg_body->u32CMD,pMsg->msg_body->bIsResp);

		if(pMsg->msg_body->bIsResp)
		{
			GV_IPCM_MSG_S* pMsgResp = GV_IPCM_CreateMessage(fd_pool, pMsg->msg_body->size, pMsg->msg_body->u32CMD, 0);
			
			ret = GV_IPCM_SendMessage(pMsgResp,fd_pool);

			GV_IPCM_DestroyMessage(pMsgResp);
		}


	}

	GV_IPCM_DestroyMessage(pMsg); 	
	printf("Enter q to exit\n");
	char cmd[64];

    while (0 != strncmp(fgets(cmd, 64, stdin), "q", 1))
    {
        printf("Enter q to exit\n");
    }

    GV_IPCM_Disconnect(fd_pool);
	free(fd_pool);
	printf("exit pthread: %s\n",__func__);
	return NULL;
}

int main()
{

	pthread_t recvpid;
	int ret = 0;
	GV_IPCM_HANDLE_ATTR_S stConnectAttr;
	stConnectAttr.target = TARGET_NODE;
	stConnectAttr.port = 2;
	stConnectAttr.priority = HANDLE_MSG_NORMAL;//msg_priority
//	stConnectAttr.priority = HANDLE_MSG_PRIORITY;//msg_priority

//	ipcm_recv_message(&stConnectAttr);

	ret = pthread_create(&recvpid,NULL, recv_msg_fn,&stConnectAttr);
	if(ret) {
		printf("err, create thread fail.ret:%d\n",ret);
		return -1;
	}


	ret = pthread_join(recvpid,NULL);
	printf("The recvpid pthread is finish!\n");

	char cmd[64];

    while (0 != strncmp(fgets(cmd, 64, stdin), "q", 1))
    {
        printf("Enter q to exit\n");
    }

    printf("exit main\n");

	return 0;
}





