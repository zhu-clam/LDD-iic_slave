/*************************************************************************
	> File Name: ipcm_audio.c
	> Author: ma6174
	> Mail: ma6174@163.com 
	> Created Time: Thu 25 Apr 2019 06:40:13 PM PDT
	>
	> brief: ck810 解析 test.wav 文件,申请内存池一段地址
    > 将解析出的test.wav文件 数据memcpy() 到申请到的地址上，发送消息通知ck860 

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

#include "asoundlib.h"

#define IPCM_CMD_CODE_AUDIO 	(1)
#define IPCM_CMD_DECODE_AUDIO   (2)


#define IPCM_CMD_SET_PARAMETER_AUDIO (3)
#define IPCM_CMD_PLAY_AUDIO (4)
#define IPCM_CMD_RECORD_AUDIO (5)


#define MEM_SIZE 4096

#define mem_dev "/dev/mem"
#define audio_file "test.wav"

struct WAV_HEADER
{
    char rld[4];    //riff 标志符号
    int rLen;   
    char wld[4];    //格式类型（wave）
    char fld[4];    //"fmt"

    int fLen;   //sizeof(wave format matex)
    
    short wFormatTag;   //编码格式
    short wChannels;    //声道数
    int   nSamplesPersec ;  //采样频率
    int   nAvgBitsPerSample;//WAVE文件采样大小
    short  wBlockAlign; //块对齐
    short wBitsPerSample;   //WAVE文件采样大小
    
    char dld[4];        //”data“
    int wSampleLength;  //音频数据的大小

} wav_header;

static int fd_mem ;

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


void* ck810_play_audio_pthread(void *arg)
{
	int ret = 0;
	void* vir_addr;
	void* vir_data;

	GV_IPCM_HANDLE_ATTR_S* stConnectAttr = (GV_IPCM_HANDLE_ATTR_S* )arg;
	
	printf("CK810 ipcm audio pthread is run...\n");

	GV_IPCM_MSG_POOL_S* fd_pool = (GV_IPCM_MSG_POOL_S*)malloc(sizeof(GV_IPCM_MSG_POOL_S));
	if(!fd_pool)
	{
		printf("malloc mem for fd_pool fail!\n");
		exit(-1);
	}

	if(GV_SUCCESS != GV_IPCM_Connect(stConnectAttr,fd_pool))
	{
	     printf("Connect fail\n");
       	 return NULL;
	}

	
	printf("ck810 connect to target successful!\n");


/*for test ipcm connect */
#if 1
	char zxf[64];
	while (0 != strncmp(fgets(zxf, 64, stdin), "q", 1))
	{
		printf("Enter q to exit\n");
		usleep(200000);
	}
#endif


	
/*读取test.wav数据*/
	 int nread;
	 FILE *fp;
	 fp = fopen(audio_file,"rb");
	 if(fp==NULL)
	 {
		 perror("open file failed:\n");
		 exit(1);
	 }

	nread=fread(&wav_header,1,sizeof(wav_header),fp);
	printf("datablock: %d\n",wav_header.wBlockAlign);
	printf("nread=%d\n",nread);
	printf("wav_header.rLen:%d\n",wav_header.rLen);
	printf("wChannels:%d\n",wav_header.wChannels);
	printf("nSamplesPersec:%d\n",wav_header.nSamplesPersec);
	printf("wSampleLength=%d\n",wav_header.wSampleLength);
	printf("wBitsPerSample =%d \n",wav_header.wBitsPerSample);

	printf("read wav header ok!\n");
/* 将头信息发送给ck860，去设置pcm设备  parameter */
	GV_IPCM_MSG_S* pMsgWavHead =  GV_IPCM_CreateMessage(fd_pool,nread, IPCM_CMD_SET_PARAMETER_AUDIO, 1);
	GV_IPCM_MSG_S* pMsgWavHeadResp =  GV_IPCM_CreateMessage(fd_pool,0, 0, 0);

	vir_addr = phy_to_virtual(pMsgWavHead->msg_body->start,pMsgWavHead->msg_body->size);
	printf("PHY addr:0x%x,vir_addr:0x%x\n",pMsgWavHead->msg_body->start,vir_addr);
	//copy wav header to specific address vir_addr
	memcpy(vir_addr,&wav_header,nread);
 
//	ret = GV_IPCM_SendMessage(pMsgWavHead,fd_pool);
	ret = GV_IPCM_SendsyncMessage(fd_pool,pMsgWavHead, pMsgWavHeadResp);
	if (ret != GV_SUCCESS)
	{
		printf("send message fail!\n");
		return NULL;
	}	
	printf("recv sync message,msg_body->start:0x%x,msg_body->size:%d\n",pMsgWavHeadResp->msg_body->start,pMsgWavHeadResp->msg_body->size);


	munmap(vir_addr,pMsgWavHead->msg_body->size);//mmap 使用完后,munmap回去
	GV_IPCM_DestroyMessage(pMsgWavHead);
	GV_IPCM_DestroyMessage(pMsgWavHeadResp);
	
/* 将wav 数据文件发送给ck860 */
	GV_IPCM_MSG_S* pMsgdata =  GV_IPCM_CreateMessage(fd_pool,wav_header.wSampleLength, IPCM_CMD_PLAY_AUDIO, 1);
	GV_IPCM_MSG_S* pMsgdataResp =  GV_IPCM_CreateMessage(fd_pool,0, 0, 0);


	vir_data = phy_to_virtual(pMsgdata->msg_body->start,pMsgdata->msg_body->size);
	printf("the data virtual address is:0x%x \n",vir_data);

	printf("begin read test.wav data =====================!\n");
	
	fseek(fp,58,SEEK_SET); 
	while(1)
	{
		ret = fread(vir_data, 1,wav_header.wSampleLength, fp);//
		if(ret == 0)
		{
				printf("read test.wav finish\n");
				break;
		}
	} 

	printf("read end of test.wav file data_len: %d =========== \n",wav_header.wSampleLength);

//	ret = GV_IPCM_SendMessage(pMsgdata,fd_pool);
	ret = GV_IPCM_SendsyncMessage(fd_pool,pMsgdata, pMsgdataResp);
	if (ret)
	{
		printf("send message data fail!\n");
		return NULL;
	}	

	printf("recv sync message,msg_body->start:0x%x,msg_body->size:%d\n",pMsgdataResp->msg_body->start,pMsgdataResp->msg_body->size);
	
	munmap(vir_data,pMsgdata->msg_body->size);//mmap 使用完后,munmap回去
	GV_IPCM_DestroyMessage(pMsgdata);
	GV_IPCM_DestroyMessage(pMsgdataResp);
	printf("Enter q to exit\n");
	char cmd[64];


    while (0 != strncmp(fgets(cmd, 64, stdin), "q", 1))
    {
        printf("Enter q to exit\n");
    }

    GV_IPCM_Disconnect(fd_pool);
/*finish*/
	fclose(fp);
	close(fd_mem);
	printf("the thread ipcm audio is exit...\n");

	return NULL;
}


int main()
{

	GV_IPCM_HANDLE_ATTR_S stConnectAttr;
	stConnectAttr.target = TARGET_NODE;
	stConnectAttr.port = 1;
//	stConnectAttr.priority = HANDLE_MSG_NORMAL;//msg_priority
	stConnectAttr.priority = HANDLE_MSG_PRIORITY;//msg_priority

//	ipcm_audio(&stConnectAttr);

	pthread_t audiopid;
			
	if (0 != pthread_create(&audiopid, NULL, ck810_play_audio_pthread, &stConnectAttr))
	{
		printf("pthread_create ipcm_audio_pthread fail\n");
		exit(-1);
	}	

	pthread_join(audiopid, NULL);//阻塞等待子线程audiopid完成
	
	printf("THE audiopid pthread is finish!\n");

	char cmd[64];

    while (0 != strncmp(fgets(cmd, 64, stdin), "q", 1))
    {
        printf("Enter q to exit\n");
    }


	return 0;
}

