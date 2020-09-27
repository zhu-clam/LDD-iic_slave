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


#include <math.h>
#include "gv_ipcm.h"
//#include "testdata.h"
#include "defineall.h"


/*Define ipcm message cmd*/
#define IPCM_CMD_CODE_AUDIO (1)
#define IPCM_CMD_DECODE_AUDIO (2)

#define IPCM_CMD_PLAY_AUDIO (4)
#define IPCM_CMD_RECORD_AUDIO (5)

#define MEM_SIZE 4096

#define mem_dev "/dev/mem"
static int fd_mem;

#if 0
void read_file_data(void *pcm_buffer,long size)
{
	FILE * pcm_data_fd;
	unsigned int count = 0;
	if((pcm_data_fd = fopen("pcm_data.txt","rb")) == NULL)
	{
		printf("open file pcm_data.txt error!\n");
		exit(-1);
	}

	
    count =	fread(pcm_buffer,sizeof(float),size,pcm_data_fd);
	printf("read from pcm_data.txt count %d\n ",count);
	fclose(pcm_data_fd);
	
}
#endif

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







/* 执行编码函数 */
int  ck810_code(GV_IPCM_MSG_S* pMsg_code)
{

	printf("ck810 entry code program!\n");

	struct handleAPI handle_code;
	handle_code.type = 1;
	handle_code.sampleRate = 8000;
	handle_code.bitRate = 32000; 
	handle_code.bitsPerSample = 1;
	handle_code.numOfChannels = 1;
	handle_code.amu = 2;

	handle_code.input = phy_to_virtual(pMsg_code->msg_body->start,pMsg_code->msg_body->size);
//	handle_code.numofinput = pMsg_code->msg_body->size;
	handle_code.numofinput = pMsg_code->msg_body->size / sizeof(float);
	handle_code.numofoutput = 0;  
	handle_code.output = NULL;

	//将ipcm传递过来的值输出到文件当中,与testdata.h 对比
	
	printf("numofinput:%ld,numofoutput:%ld,sampleRate:%ld,bitRate:%ld \n",handle_code.numofinput,handle_code.numofoutput,handle_code.sampleRate,handle_code.bitRate);

	float *input_code = (float *)handle_code.input;
	FILE *fp_code = fopen("ck810_code.txt", "w");
	for (int i = 0; i < handle_code.numofinput; i++)
	{
		fprintf(fp_code, "%f", input_code[i]);
	}
	fclose(fp_code);

	
	if (handle_code.type < 5)
	{
		handle_code.numofoutput = (long)((((float)handle_code.numofinput / (float)handle_code.sampleRate) * (float)handle_code.bitRate * 1.5) / 8.0);
printf("numofinput:%ld,numofoutput:%ld,sampleRate:%ld,bitRate:%ld \n",handle_code.numofinput,handle_code.numofoutput,handle_code.sampleRate,handle_code.bitRate);
	}
	else
	{
		handle_code.numofoutput = (long)(((float)handle_code.numofinput * 8.0 / (float)handle_code.bitRate) * (float)handle_code.sampleRate * 1.5);
	}

	if (handle_code.type < 5)
	{
		//handle_code.output = (void*)malloc(sizeof(unsigned char) * handle_code.numofoutput*4);
		handle_code.output = (void*)malloc(sizeof(unsigned char) * handle_code.numofoutput);
		if(!handle_code.output)
		{
			printf("numofoutput:%d,output:0x%x\n ",(sizeof(unsigned char) * handle_code.numofoutput),handle_code.output);
		}
	}
	else
	{
	//	handle_code.output = (void*)malloc(sizeof(float) * handle_code.numofoutput*4);
		handle_code.output = (void*)malloc(sizeof(float) * handle_code.numofoutput);
	}
	
	
	handle_code = Open(handle_code);
	
	printf("after invocate codec API!========**********=============================\n");
	if (handle_code.type < 5)
	{
		unsigned char *out = (unsigned char *)handle_code.output;
		//生成编码文件
		FILE *fp = fopen("avs2_out_test01.txt", "w");
		for (int i = 0; i < handle_code.numofoutput; i++)
		{
			fprintf(fp, "0x");
			fprintf(fp, "%x", out[i]);
			fprintf(fp, "%s", ",");
		}
		fclose(fp);
	}
	else
	{
		float *out = (float *)handle_code.output;
		//生成解码文件
		FILE *fp = fopen("avs2_out_test03.txt", "w");
		for (int i = 0; i < handle_code.numofoutput; i++)
		{
			
			fprintf(fp, "%f,", out[i]);
			
		}
		fclose(fp);
	}

	printf("Begin invocate codec Close function ==============***************************\n");
	Close(handle_code);
	printf("After invocate codec Close function ==============***************************\n");
	printf("ck810 exit code program!\n");

	return 0;
}


/*decode*/
int  ck810_decode(GV_IPCM_MSG_S* pMsg_decode)
{
	printf("ck810 entry decode program!\n");

	struct handleAPI handle1;
	handle1.type = 6;
	handle1.sampleRate = 8000;
	handle1.bitRate = 32000;//32000
	handle1.bitsPerSample = 1;
	handle1.numOfChannels = 1;
	handle1.amu = 2;
//	handle1.input = inputcodeavs2;			 // 输入码流地址
//	handle1.numofinput = sizeof(inputcodeavs2) / sizeof(unsigned char); //输入码流数据量
	handle1.input = phy_to_virtual(pMsg_decode->msg_body->start,pMsg_decode->msg_body->size);
	handle1.numofinput = pMsg_decode->msg_body->size / sizeof(unsigned char) ;

	handle1.numofoutput = 0;  
	handle1.output = NULL;	 

	printf("numofinput:%ld,numofoutput:%ld,sampleRate:%ld,bitRate:%ld \n",handle1.numofinput,handle1.numofoutput,handle1.sampleRate,handle1.bitRate);
	unsigned char *out = (unsigned char *)handle1.input;
	//生成编码文件
	FILE *fp = fopen("ck810_decode.txt", "w");
	for (int i = 0; i < handle1.numofinput; i++)
	{
		fprintf(fp, "0x");
		fprintf(fp, "%x", out[i]);
		fprintf(fp, "%s", ",");
	}
	fclose(fp);


	//估计所需缓存区
	if (handle1.type < 5)
	{
		handle1.numofoutput = (long)((((float)handle1.numofinput / (float)handle1.sampleRate) * (float)handle1.bitRate * 1.5) / 8.0);
	}
	else
	{
		handle1.numofoutput = (long)(((float)handle1.numofinput * 8.0 / (float)handle1.bitRate) * (float)handle1.sampleRate * 1.5);
		printf("numofinput:%ld,numofoutput:%ld,sampleRate:%ld,bitRate:%ld \n",handle1.numofinput,handle1.numofoutput,handle1.sampleRate,handle1.bitRate);
	}

	printf("numofinput:%ld,numofoutput:%ld,input:0x%x\n",handle1.numofinput,handle1.numofoutput,handle1.input);

	if (handle1.type < 5)
	{
		//根据numofCode开辟编码缓冲区
		handle1.output = (void*)malloc(sizeof(unsigned char) * handle1.numofoutput);
	}
	else
	{
		//根据numofPCM开辟解码缓冲区
		handle1.output = (void*)malloc(sizeof(float) * handle1.numofoutput);
	}


	printf("before Decode function*******************\n");
	//执行open
	handle1 = Open(handle1);
	
	printf("after Decode function*******************\n");
	if (handle1.type < 5)
	{
		unsigned char *out = (unsigned char *)handle1.output;
		FILE *fp = fopen("avs2_out_test01.txt", "w");
		for (int i = 0; i < handle1.numofoutput; i++)
		{
			fprintf(fp, "0x");
			fprintf(fp, "%x", out[i]);
			fprintf(fp, "%s", ",");
		}
		fclose(fp);
	}
	else
	{
		float *out = (float *)handle1.output;
		FILE *fp = fopen("avs2_out_test03.txt", "w");
		for (int i = 0; i < handle1.numofoutput; i++)
		{
			
			fprintf(fp, "%f,", out[i]);
			
		}
		fclose(fp);
	}
	
	printf("before Decode close function*******************\n");
	Close(handle1);
	printf("after Decode close function*******************\n");

	printf("ck810 exit decode program!\n");

	return 0;
}

void* ck810_codec_pthread(void* arg)
{

	int ret = 1;
	int len = 0;//读写消息的长度
	GV_IPCM_HANDLE_ATTR_S* stConnectAttr = (GV_IPCM_HANDLE_ATTR_S*)arg;

	printf("ck810 ipcm audio pthread is run...\n");

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
	printf("ck810 connect to target successful!\n");

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
		printf("Message phy addr:0x%#x ,size:%d,CMD:%d,IsResp:%d \n",pMsg->msg_body->start,pMsg->msg_body->size,pMsg->msg_body->u32CMD,pMsg->msg_body->bIsResp);

		switch (pMsg->msg_body->u32CMD)
		{
			case IPCM_CMD_CODE_AUDIO:
				//code
				ck810_code(pMsg);
				break;
			case IPCM_CMD_DECODE_AUDIO:
				//decode
				ck810_decode(pMsg);
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

	printf("CK810 ipcm audio thread is exit.....\n");
	return NULL;

}

int main()
{

	GV_IPCM_HANDLE_ATTR_S stConnectAttr;
	stConnectAttr.target = TARGET_NODE;
	stConnectAttr.port = 1;
	stConnectAttr.priority = HANDLE_MSG_NORMAL;//msg_priority
//	stConnectAttr.priority = HANDLE_MSG_PRIORITY;

	pthread_t codecpid;
			
	if (0 != pthread_create(&codecpid, NULL, ck810_codec_pthread, &stConnectAttr))
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






