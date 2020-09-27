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


int set_pcm_play(FILE *fp);

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

static int fd_mem;


#define alignment_down(a, PAGE_SIZE) (a & (~(PAGE_SIZE-1)) )
#define alignment_up(a, PAGE_SIZE) ((a+PAGE_SIZE-1) & (~ (PAGE_SIZE-1))) 


#define PAGE_SIZE 4096
#define PGAE_MASK (~(PAGE_SIZE-1))
/* to align the pointer to the (next) page boundary */
#define PAGE_ALIGN(addr)        (((addr)+PAGE_SIZE-1) & PGAE_MASK) 

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
	/*物理地址4k bit对齐*/
	phys_addr_t phy_align;
    phy_align = PAGE_ALIGN(phy_start);
	printf("phy address ALign:phy_align:0x%x ,phy_size:%d\n",phy_align,phy_size);

	//share memory map
	//vir_addr = mmap(NULL, phy_size,PROT_READ | PROT_WRITE,MAP_SHARED, fd_mem, phy_start);
	vir_addr = mmap(NULL, phy_size,PROT_READ | PROT_WRITE,MAP_SHARED, fd_mem, phy_align);
	if (vir_addr == MAP_FAILED)
	{
		printf("Mmap Share Memory Fail \r\n");
		exit(-1);
	}
	printf("vir_addr = 0x%x\n",vir_addr);
	return vir_addr;
}

#if 1
int ck860_play_audio(void)
{
	 int nread;
	 FILE *fp;
	// fp=fopen(argv[1],"rb");
	 fp = fopen(audio_file,"rb");
	 if(fp==NULL)
	 {
		 perror("open file failed:\n");
		 exit(1);
	 }
	 
	 nread=fread(&wav_header,1,sizeof(wav_header),fp);
	 printf("nread=%d\n",nread);
	 
	 //printf("RIFF 标志%s\n",wav_header.rld);
	// printf("文件大小rLen:%d\n",wav_header.rLen);
	 
	 printf("wav_header.rLen:%d\n",wav_header.rLen);
	 //printf("wld=%s\n",wav_header.wld);
	 //printf("fld=%s\n",wav_header.fld);
	 
	// printf("fLen=%d\n",wav_header.fLen);
	 
	 //printf("wFormatTag=%d\n",wav_header.wFormatTag);
	 //printf("声道数:%d\n",wav_header.wChannels);
	// printf("采样频率:%d\n",wav_header.nSamplesPersec);
	 //printf("nAvgBitsPerSample=%d\n",wav_header.nAvgBitsPerSample);
	 //printf("wBlockAlign=%d\n",wav_header.wBlockAlign);
	// printf("采样的位数:%d\n",wav_header.wBitsPerSample);

	printf("wChannels:%d\n",wav_header.wChannels);
	printf("nSamplesPersec:%d\n",wav_header.nSamplesPersec);
	// printf("data=%s\n",wav_header.dld);
	printf("wSampleLength=%d\n",wav_header.wSampleLength);
	 
	 
	 
	 
	 
	 set_pcm_play(fp);
	 return 0;

}

int set_pcm_play(FILE *fp)
{
    int    rc;
    int    ret;
    int    size;
    snd_pcm_t*       handle;        //PCI设备句柄
    snd_pcm_hw_params_t*      params;//硬件信息和PCM流配置
    unsigned int       val;
    int                dir=0;
    snd_pcm_uframes_t  frames;
    char   *buffer;
    int channels=wav_header.wChannels;
    int frequency=wav_header.nSamplesPersec;
    int bit=wav_header.wBitsPerSample;
    int datablock=wav_header.wBlockAlign;
 //   unsigned char ch[100];  //用来存储wav文件的头信息

    rc=snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if(rc<0)
    {
            perror("\nopen PCM device failed:");
            exit(1);
    }


    snd_pcm_hw_params_alloca(&params); //分配params结构体
    if(rc<0)
    {
            perror("\nsnd_pcm_hw_params_alloca:");
            exit(1);
    }
     rc=snd_pcm_hw_params_any(handle, params);//初始化params
    if(rc<0)
    {
            perror("\nsnd_pcm_hw_params_any:");
            exit(1);
    }
    rc=snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED); //初始化访问权限
    if(rc<0)
    {
            perror("\nsed_pcm_hw_set_access:");
            exit(1);

    }

    //采样位数
    switch(bit/8)
    {
	    case 1:snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_U8);
	            break ;
	    case 2:snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
	            break ;
	    case 3:snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S24_LE);
	            break ;
		case 4:snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S32_LE);
          		break ;

    }
    rc=snd_pcm_hw_params_set_channels(handle, params, channels);  //设置声道,1表示单声>道，2表示立体声
    if(rc<0)
    {
            perror("\nsnd_pcm_hw_params_set_channels:");
            exit(1);
    }
    val = frequency;
    rc=snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);  //设置>频率
    if(rc<0)
    {
            perror("\nsnd_pcm_hw_params_set_rate_near:");
            exit(1);
    }

    rc = snd_pcm_hw_params(handle, params);
    if(rc<0)
    {
	    perror("\nsnd_pcm_hw_params: ");
	    exit(1);
    }

    rc=snd_pcm_hw_params_get_period_size(params, &frames, &dir);  /*获取周期长度*/
    if(rc<0)
    {
            perror("\nsnd_pcm_hw_params_get_period_size:");
            exit(1);
    }

    size = frames * datablock;   /*4 代表数据块长度*/

    buffer =(char*)malloc(size);
	fseek(fp,58,SEEK_SET);  //定位歌曲到数据区

	while (1)
	{
        memset(buffer,0,sizeof(buffer));
        ret = fread(buffer, 1, size, fp);
        if(ret == 0)
        {
            //    printf("\n歌曲写入结束\n");
			printf("playback test.wav finish! \n");
			break;
        }
         else if (ret != size)
        {
         }
            // 写音频数据到PCM设备,返回写入PCM设备的数据帧 
    	while(ret = snd_pcm_writei(handle, buffer, frames)<0)
       {
             usleep(2000);  
             if (ret == -EPIPE)
            {
              /* EPIPE means underrun */
              fprintf(stderr, "underrun occurred\n");
              //完成硬件参数设置，使设备准备好  
              snd_pcm_prepare(handle);
             }
             else if (ret < 0)
             {
                 fprintf(stderr,"error from writei: %s\n",snd_strerror(ret));
             }
        }

	}

    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    free(buffer);
    return 0;
}
#endif
static snd_pcm_uframes_t  frames_byavs;
static int datablock_byavs;
static short BitsPerSample;
static int CHANNELS;

/* 设置好参数后，返回pcm设备句柄 */
snd_pcm_t* ck860_set_parameter_audio(GV_IPCM_MSG_S* pMsg_SetParameter)
{

	printf("ck860 set pcm device parameter!\n");

/* 对保存数据的物理地址映射进线程 */
	struct WAV_HEADER* ck860_wav_header = (struct WAV_HEADER*)phy_to_virtual(pMsg_SetParameter->msg_body->start,pMsg_SetParameter->msg_body->size);

	int	  rc;
	int	  ret;
	int   resolution;
	snd_pcm_t*		handle; 	   //PCI设备句柄
	snd_pcm_hw_params_t* 	 params;//硬件信息和PCM流配置
	unsigned int 	  val;
	int				  dir=0;
	char   *buffer;
	int channels=ck860_wav_header->wChannels;
	CHANNELS = ck860_wav_header->wChannels;
	int frequency=ck860_wav_header->nSamplesPersec;
	int bit=ck860_wav_header->wBitsPerSample;
	//int datablock=ck860_wav_header->wBlockAlign;
	
	datablock_byavs=ck860_wav_header->wBlockAlign;
	BitsPerSample = ck860_wav_header->wBitsPerSample;
	//	 unsigned char ch[100];  //用来存储wav文件的头信息

	printf("datablock: %d\n",ck860_wav_header->wBlockAlign);
	printf("wav_header.rLen:%d\n",ck860_wav_header->rLen);
	printf("wChannels:%d\n",ck860_wav_header->wChannels);
	printf("nSamplesPersec:%d\n",ck860_wav_header->nSamplesPersec);
	printf("wSampleLength=%d\n",ck860_wav_header->wSampleLength);
	printf("wBitsPerSample=%d\n",BitsPerSample);

	rc=snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);//用于播放
	if(rc<0)
	{
		   perror("\nopen PCM device failed:");
		   exit(1);
	}


	snd_pcm_hw_params_alloca(&params); //分配params结构体
	if(rc<0)
	{
		   perror("\nsnd_pcm_hw_params_alloca:");
		   exit(1);
	}
	rc=snd_pcm_hw_params_any(handle, params);//初始化params
	if(rc<0)
	{
		   perror("\nsnd_pcm_hw_params_any:");
		   exit(1);
	}
	rc=snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED); //初始化访问权限
	if(rc<0)
	{
		   perror("\nsed_pcm_hw_set_access:");
		   exit(1);
	}

#if 0
	//采样精度
	switch(bit/8)
	{
	   case 1:snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_U8);
			   break ;
	   case 2:snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
			   break ;
	   case 3:snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S24_LE);
			   break ;
	   case 4:snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S32_LE);
			   break ;

	}
#else
	//采样精度
	switch(bit/8)
	{
	   case 1:
	   	resolution = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_U8);
			   break ;
	   case 2:
	   	resolution = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
			   break ;
	   case 3:
	   	resolution =snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S24_LE);
			   break ;
	   case 4:
	   	resolution =snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S32_LE);
			   break ;

	}

	if (resolution < 0)
	{
		printf("=Error! set format fail %d==",bit/8);
	}
	else {
		printf("set format success %d !",bit/8);	
	}

#endif

	
	rc=snd_pcm_hw_params_set_channels(handle, params, channels);  //设置声道,1表示单声>道，2表示立体声
	if(rc<0)
	{
		   perror("\nsnd_pcm_hw_params_set_channels:");
		   exit(1);
	}
	val = frequency;
	rc=snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);	//设置>频率
	if(rc<0)
	{
		   perror("\nsnd_pcm_hw_params_set_rate_near:");
		   exit(1);
	}

/*设置pcm hw params 完成*/
	rc = snd_pcm_hw_params(handle, params);
	if(rc<0)
	{
	   perror("\nsnd_pcm_hw_params: ");
	   exit(1);
	}
	
    rc=snd_pcm_hw_params_get_period_size(params, &frames_byavs, &dir);  /*获取周期长度*/
    if(rc<0)
    {
            perror("\nsnd_pcm_hw_params_get_period_size:");
            exit(1);
    }

	printf("frames_byavs:%d\n",frames_byavs);	
	printf("set pcm parameter finish!\n");
	return handle;
	
}

int ck860_playback(GV_IPCM_MSG_S* pMsg_playback,snd_pcm_t*		handle)
{
	printf("ck860 begin playback!\n");
	int ret ;

	int r;
	int wcount;//代表音频数据帧数
/* 从pcm 数据读取到paly_vir当中 */
	void* play_vir = phy_to_virtual(pMsg_playback->msg_body->start,pMsg_playback->msg_body->size); 
	int size = pMsg_playback->msg_body->size;//pcm 字节大小
	
/* Transfer to size frame */
	wcount = size*8/BitsPerSample * CHANNELS;
	printf("frames_byavs :%d,wcount: %d\n",frames_byavs,wcount); 

// snd_pcm_writei() snd_pcm_readi();size 是以frame为单位，frame与bytes的转换
// 1 frame = channels*sample_size ;1frame = 2bytes
	
	while(wcount > 0){
		r = snd_pcm_writei(handle,play_vir,wcount);
		if (r == -EAGAIN || (r>=0 && (size_t)r << wcount)){
			snd_pcm_wait(handle,1000);
			}	else if (ret == -EPIPE)
			{
				/* EPIPE means underrun */
		      fprintf(stderr, "underrun occurred\n");
		      //完成硬件参数设置，使设备准备好  
		      snd_pcm_prepare(handle);
			}
			else if (r == -ESTRPIPE)
			{
				 fprintf(stderr, "need suspend!\n");
			}
		    else if (ret < 0)
		   {
		         fprintf(stderr,"error from writei: %s\n",snd_strerror(ret));
				 exit(-1);
		   }

		if (r > 0)
		{
			wcount -= r;
			play_vir+= r*BitsPerSample/8;//每次偏移r帧
		}
	
	}

	printf("end of test.wav play!===========\n");
#if 0
	for(;count != 0 ;count--)
	{
		//snd_pcm_writei(snd_pcm_t * ,const void* snd_pcm_uframes_t);-帧size
//		while(ret = snd_pcm_writei(handle, play_vir, frames_byavs)<0)
		ret = snd_pcm_writei(handle, play_vir, frames_byavs);
		
	    usleep(2000);  
	    if (ret == -EPIPE)
	    {
		      /* EPIPE means underrun */
		      fprintf(stderr, "underrun occurred\n");
		      //完成硬件参数设置，使设备准备好  
		      snd_pcm_prepare(handle);
	    }
	    else if (ret < 0)
	   {
	         fprintf(stderr,"error from writei: %s\n",snd_strerror(ret));
	    }
		
		play_vir += ret;//循环写入，偏移一段
		
	}
#endif


    snd_pcm_drain(handle);
    snd_pcm_close(handle);


	munmap(play_vir,pMsg_playback->msg_body->size);
	close(fd_mem);
	printf("ck860 playback test.wav finish!\n");
	return 0;

}
//handle->bits_per_frame;

/*
* 接收810 消息后。播放音频
*/
void* ck860_play_audio_pthread(void *arg)
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

	/*
	if(GV_SUCCESS != GV_IPCM_Connect(stConnectAttr,fd_pool))
	{
		 printf("Connect fail\n");
		 exit(-1);
	}
	*/

	GV_IPCM_TryConnect(stConnectAttr,fd_pool);

	while (GV_FALSE == GV_IPCM_CheckConnected(fd_pool))
    {
        printf("Wait connection ok\n");
        usleep(200000);
    }

	
	printf("ck860 connect to target successful!\n");

/*for test ipcm connect */
#if 1
	
	char zxf[64];
	while (0 != strncmp(fgets(zxf, 64, stdin), "q", 1))
	{
		printf("Enter q to exit\n");
		usleep(200000);
	}
#endif

	
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
	pthread_t audiopid;
			
	if (0 != pthread_create(&audiopid, NULL, ck860_play_audio_pthread, &stConnectAttr))
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

