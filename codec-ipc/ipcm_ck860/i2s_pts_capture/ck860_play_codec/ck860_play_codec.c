/*
 * 
 * author:zhuxianfei@byavs.com
 * 
 * @brief: parse audio file and tell ck810 PCM data addr,then ck810 execute codec , 
 *         at the end,tell ck860 PCM data which is after ck810 codec,and playback PCM data 
 *
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
#include <sys/mman.h>



//#include <alsa/asoundlib.h>
#include "asoundlib.h"

#include "gv_ipcm.h"
#include "wav_parser.h"
#include "sndwav_common.h"

#define mem_dev "/dev/mem"
//#define audio_file "test.wav"


#define IPCM_CMD_CODE_AUDIO 	(1)
#define IPCM_CMD_DECODE_AUDIO   (2)


#define IPCM_CMD_SET_PARAMETER_AUDIO (3)
#define IPCM_CMD_PLAY_AUDIO (4)
#define IPCM_CMD_RECORD_AUDIO (5)





void* ck860_play_codec_pthread(void *arg)
{

	int ret = 1;
	int len = 0;//读写消息的长度
	snd_pcm_t*		handle; 	
	GV_IPCM_HANDLE_ATTR_S* stConnectAttr = (GV_IPCM_HANDLE_ATTR_S*)arg;

	printf("ck860 ipcm audio pthread is run...\n");

	GV_IPCM_MSG_POOL_S* fd_pool =(GV_IPCM_MSG_POOL_S* )malloc(sizeof(GV_IPCM_MSG_POOL_S));
	if(!fd_pool)
	{
		printf("malloc mem for fd_pool fail!\n");
		exit(-1);
	}
	if(GV_SUCCESS != GV_IPCM_Connect(stConnectAttr,fd_pool))
	{
		 printf("Connect fail\n");
		 exit(-1);
	}
	printf("ck860 connect to target successful!\n");
	GV_IPCM_MSG_S* pMsg =  GV_IPCM_CreateMessage(fd_pool,0 , 0, 0);
	
	while(1)
	{
		ret = GV_IPCM_ReceiveMessage(fd_pool, pMsg,&len);
		if(GV_SUCCESS != ret)
		{
			printf("remote node is disconneted returnNum[%d] \n",ret);
			break;
		} else if (0 == len)
		{
			printf("GV_IPCM_ReceiveMessage len: %d \n",len);
			break;
		}	
		//3.打印消息,或消息处理
printf("Message phy addr:0x%#x ,size:%d,CMD:%d,IsResp:%d \n",pMsg->msg_body->start,pMsg->msg_body->size,pMsg->msg_body->u32CMD,pMsg->msg_body->bIsResp);

		switch (pMsg->msg_body->u32CMD)
		{
			case IPCM_CMD_SET_PARAMETER_AUDIO:
					//消息处理
				handle =ck860_set_parameter_audio(pMsg);
				break;
			case IPCM_CMD_PLAY_AUDIO:
				//播发音频
				//ret = ck860_play_audio();
				  ret = ck860_playback(pMsg,handle);
				break;
			case IPCM_CMD_RECORD_AUDIO:
				//录音record
			//	ret = record_audio();
				break;
			case IPCM_CMD_OPEN_CODEC:
				
				break;
			default:
				printf("receive cmd is not find!\n");
				break;
		}
		if(pMsg->msg_body->bIsResp) //返回同步消息
		{
			GV_IPCM_MSG_S* pMsgResp =  GV_IPCM_CreateMessage(fd_pool,0 , 0, 0);
			ret = GV_IPCM_SendMessage(pMsgResp, fd_pool);
			if(ret != GV_SUCCESS)
			{
				printf("send message fail!\n");
				exit(-1);
			}
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

	printf("CK860 ipcm audio thread is exit.....\n");
	return NULL;

}


int main()
{

	GV_IPCM_HANDLE_ATTR_S stConnectAttr;
	stConnectAttr.target = TARGET_NODE;
	stConnectAttr.port = 1;
//	stConnectAttr.priority = HANDLE_MSG_NORMAL;//msg_priority
	stConnectAttr.priority = HANDLE_MSG_PRIORITY;

//	ck860_play(&stConnectAttr);
	pthread_t audiorecvpid;
	pthread_t audioplaypid;

			
	if (0 != pthread_create(&audiorecvpid, NULL, ck860_play_codec_pthread, &stConnectAttr))
	{
		printf("pthread_create ipcm_audio_pthread fail\n");
		exit(-1);
	}	

	if (0 != pthread_create(&audioplaypid, NULL, ck860_play_codec_pthread, &stConnectAttr))
	{
		printf("pthread_create ipcm_audio_pthread fail\n");
		exit(-1);
	}	


	pthread_join(audiorecvpid, NULL);//阻塞等待子线程audiopid完成
	
	printf("THE audiopid pthread is finish!\n");
	char cmd[64];

	while (0 != strncmp(fgets(cmd, 64, stdin), "q", 1))
	{
		printf("Enter q to exit\n");
	}

	return 0;
}

