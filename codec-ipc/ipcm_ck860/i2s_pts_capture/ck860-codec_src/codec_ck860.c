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
#include <sys/mman.h>


#include "gv_ipcm.h"
//#include "defineall.h" codec lib header file
#include "testdata.h" 


#define mem_dev "/dev/mem"

#define IPCM_CMD_CODE_AUDIO 	(1)
#define IPCM_CMD_DECODE_AUDIO   (2)


#define IPCM_CMD_SET_PARAMETER_AUDIO (3)
#define IPCM_CMD_PLAY_AUDIO (4)
#define IPCM_CMD_RECORD_AUDIO (5)



static int fd_mem;

void* phy_to_virtual(phys_addr_t phy_start,unsigned int phy_size)
{

	void *vir_addr = NULL;
	// Open the /dev/mem Device
	fd_mem = open(mem_dev,O_RDWR | O_SYNC);
	if (fd_mem < 0) 
	{
		printf("Open /dev/mem Device Fail \r\n");
		exit(-1);
	}
	
	printf("phy_to_virtual:phy_start:0x%x ,phy_size:%d\n",phy_start,phy_size);
	//share memory map
	vir_addr = mmap(NULL, phy_size,PROT_READ | PROT_WRITE,MAP_SHARED, fd_mem, phy_start);
	if (vir_addr == MAP_FAILED)
	{
		printf("Mmap Share Memory Fail \r\n");
		exit(-1);
	}
	printf("vir_addr = 0x%x\n",vir_addr);
	return vir_addr;
}


/*
* 解析testdata.h 文件中的inputwavg711，发送给ck810;
*/
void* ck860_codec_pthread(void *arg)
{
	int ret = 1;
	int len = 0;//读写消息的长度
	GV_IPCM_HANDLE_ATTR_S* stConnectAttr = (GV_IPCM_HANDLE_ATTR_S*)arg;

	printf("ck860_codec_pthread pthread is run...\n");

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


	void* vir_data = NULL;
	void* input = inputwavg711;
	long numofinput = sizeof(inputwavg711) / sizeof(float);
	int sizeofinput = sizeof(inputwavg711);


	printf("numofinput is :%d,sizeofinput: %d\n",numofinput,sizeofinput);
	//创建同步消息 code
	GV_IPCM_MSG_S* pMsg =  GV_IPCM_CreateMessage(fd_pool,sizeofinput, IPCM_CMD_CODE_AUDIO, 1);
	GV_IPCM_MSG_S* pMsgResp =  GV_IPCM_CreateMessage(fd_pool,0, 0, 0);

	
	//将pcm数据拷贝到申请的内存池当中
	vir_data = phy_to_virtual(pMsg->msg_body->start,pMsg->msg_body->size);
	printf("PHY addr:0x%x,vir_addr:0x%x\n",pMsg->msg_body->start,vir_data);
	memcpy(vir_data,input,sizeofinput);


	//查看memcpy后vir_data数据和inputwavg711 是否一致
	float *input_code = (float *)vir_data;
	FILE *fp_code = fopen("ck860_code.txt", "w");
	for (int i = 0; i < numofinput; i++)
	{
		fprintf(fp_code, "%f,", input_code[i]);
	}
	fclose(fp_code);
//
	
	ret = GV_IPCM_SendsyncMessage(fd_pool,pMsg, pMsgResp);
	if (ret != GV_SUCCESS)
	{
		printf("send code cmd fail!\n");
		return NULL;
	}	

	printf("recv sync message,msg_body->start:0x%x,msg_body->size:%d\n",pMsgResp->msg_body->start,pMsgResp->msg_body->size);

	munmap(vir_data,numofinput);//mmap 使用完后,munmap回去
	GV_IPCM_DestroyMessage(pMsgResp);
	GV_IPCM_DestroyMessage(pMsg);

	//for decode
	//void* decode = inputcodeavs2;
	long numofdecode = sizeof(inputcodeavs2) / sizeof(unsigned char);
	int sizeofoutput = sizeof(inputcodeavs2);
	void* vir_dec = NULL;

	printf("num of decode is :%d,sizeofoutput:%d \n",numofdecode,sizeofoutput);
	//send cmd to decode
	
	GV_IPCM_MSG_S* pMsgDecode =  GV_IPCM_CreateMessage(fd_pool,sizeofoutput, IPCM_CMD_DECODE_AUDIO, 1);
	GV_IPCM_MSG_S* pMsgDecodeResp =  GV_IPCM_CreateMessage(fd_pool,0, 0, 0);


	//将pcm数据拷贝到申请的内存池当中
	vir_dec = phy_to_virtual(pMsgDecode->msg_body->start,pMsgDecode->msg_body->size);
	printf("PHY addr:0x%x,vir_addr:0x%x\n",pMsgDecode->msg_body->start,vir_data);

	memcpy(vir_dec,inputcodeavs2,sizeofoutput);

	unsigned char* decode = (unsigned char*)vir_dec;
	
	FILE *fp_decode = fopen("ck860_decode.txt", "w");
	for (int i = 0; i < numofdecode; i++)
	{
	   fprintf(fp_decode, "0x");
	   fprintf(fp_decode, "%x", decode[i]);
	   fprintf(fp_decode, "%s", ",");
	}
	
	ret = GV_IPCM_SendsyncMessage(fd_pool,pMsgDecode, pMsgDecodeResp);
	if (ret != GV_SUCCESS)
	{
		printf("send decode cmd fail!\n");
		return NULL;
	}

	printf("recv sync message,msg_body->start:0x%x,msg_body->size:%d\n",pMsgDecodeResp->msg_body->start,pMsgDecodeResp->msg_body->size);

	munmap(vir_dec,numofdecode);//mmap 使用完后,munmap回去
	GV_IPCM_DestroyMessage(pMsgDecode);
	GV_IPCM_DestroyMessage(pMsgDecodeResp);

	printf("Enter q to exit\n");
	char cmd[64];

	while (0 != strncmp(fgets(cmd, 64, stdin), "q", 1))
	{
		printf("Enter q to exit\n");
	}
	GV_IPCM_Disconnect(fd_pool);
	
	close(fd_mem);
	printf("ck860_codec_pthread is exit.....\n");
	return NULL;

}
int main()
{

	GV_IPCM_HANDLE_ATTR_S stConnectAttr;
	stConnectAttr.target = TARGET_NODE;
	stConnectAttr.port = 1;
	stConnectAttr.priority = HANDLE_MSG_NORMAL;//msg_priority
//	stConnectAttr.priority = HANDLE_MSG_PRIORITY;

//	ck860_play(&stConnectAttr);
	pthread_t codecpid;
			
	if (0 != pthread_create(&codecpid, NULL, ck860_codec_pthread, &stConnectAttr))
	{
		printf("pthread_create ipcm_codecpid_pthread fail\n");
		exit(-1);
	}	

	pthread_join(codecpid, NULL);//阻塞等待子线程audiopid完成
	
	printf("THE codecpid pthread is finish!\n");
	char cmd[64];

    while (0 != strncmp(fgets(cmd, 64, stdin), "q", 1))
    {
        printf("Enter q to exit\n");
    }

	return 0;
}

