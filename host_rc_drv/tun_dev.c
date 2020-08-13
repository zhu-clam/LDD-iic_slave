#include <unistd.h>
#include <stdio.h>
//#include <curses.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>
#include <linux/if_tun.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>


#include "gv_host_channel_api.h"

int tun_creat(char *dev,int flags)
{

	 struct ifreq ifr;
	 int fd,err;

	 assert(dev != NULL);
	 if((fd = open ("/dev/net/tun",O_RDWR))<0) //you can replace it to tap to create tap device.
		return fd;
	  
	 memset(&ifr,0,sizeof (ifr));
	 ifr.ifr_flags|=flags;
	 if(*dev != '\0')
		strncpy(ifr.ifr_name,dev,IFNAMSIZ);
		
	 if((err = ioctl(fd,TUNSETIFF,(void *)&ifr))<0)
	 {
		  close (fd);
		  return err;
	 }
	 strcpy(dev,ifr.ifr_name);
	 return fd;
}

static inline void sleep_us(unsigned int nusecs) 
{ 
    struct timeval    tval; 
    
    tval.tv_sec = nusecs / 1000000; 
    tval.tv_usec = nusecs % 1000000; 
    select(0, NULL, NULL, NULL, &tval); 
} 


#define READ_WRITE_SIZE 81920
static char tx_Buf[READ_WRITE_SIZE];
static char ReadBuf[READ_WRITE_SIZE];

__GV_HOST_CHANNEL  tun_chn_rx;
__GV_HOST_CHANNEL  tun_chn_tx;
__GV_TUN gv_tun;
char tun_name[IFNAMSIZ]={0};

int gv_machine_id=-1;

//芯片ID
int gv_chip_id=-1;

static int EP_ALIVE_FLAG = 1;
pthread_t       tun_rx_thread;
pthread_t       tun_tx_thread;

pthread_t       wdt_clr_thread;
pthread_t       read_chipid_thread;

void read_chipid_task(void *data_ptr)
{
	int recan_fd = -1;
	char recan_file[64];
	int* pci_fd = (int* )data_ptr;

	while(1)
	{
		EP_ALIVE_FLAG = gv_read_chipid(tun_chn_tx.fd);//复位之后,改文件描述符毁坏掉了。
		if(EP_ALIVE_FLAG == 0)
		{
			printf("%s,%d EP_ALIVE:%d\n",__func__,__LINE__,EP_ALIVE_FLAG);
		}
		#if 0
		if(EP_ALIVE_FLAG == 0)
		{
			//线程睡眠12 后rescan.
			//1. wait for ep launch up
			//sleep(12);
			printf("%s,%d EP_ALIVE:%d\n",__func__,__LINE__,EP_ALIVE_FLAG);
			//close(tun);
			sleep(40);
			//2. exectue rescan file
			sprintf(recan_file,"./.rescan_%d.sh",gv_chip_id);
			system(recan_file);
			printf("%s,%d recan_file:%s\n",__func__,__LINE__,recan_file);
			//3.re-open
			{
			  if (gv_HostChannel_Open(&tun_chn_rx,gv_chip_id,(NUM_READ_CHANNELS-1))<0)//13
	    			return ;
	   		  if (gv_HostChannel_Open(&tun_chn_tx,gv_chip_id,(NUM_DATA_CHANNELS-1))<0)//27
	    			return ;
			}
			//4.config tun argument
			printf("tun.ip_v4  : %d.%d.%d.%d\n",gv_tun.ip_v4[0],gv_tun.ip_v4[1],gv_tun.ip_v4[2],gv_tun.ip_v4[3]);
			printf("tun.mask_v4: %d.%d.%d.%d\n",gv_tun.mask_v4[0],gv_tun.mask_v4[1],gv_tun.mask_v4[2],gv_tun.mask_v4[3]);
			printf("tun.gw_v4  : %d.%d.%d.%d\n",gv_tun.gw_v4[0],gv_tun.gw_v4[1],gv_tun.gw_v4[2],gv_tun.gw_v4[3]);
			printf("tun.dns_v4 : %d.%d.%d.%d\n",gv_tun.dns_v4[0],gv_tun.dns_v4[1],gv_tun.dns_v4[2],gv_tun.dns_v4[3]);
			gv_ChipTunDev_config(&tun_chn_tx,&gv_tun);
			//4.5 create tun device
			/*
			tun = tun_creat(tun_name,IFF_TUN|IFF_NO_PI);//如果需要配置tun设备，则把"IFF_TAP"改成“IFF_TUN”
			if(tun<0)
			{
				perror("tun_create");
				return 1;
			}*/
			//5.config host tun 
			{
				char shell_cmd[256];
				sprintf(shell_cmd,"ifconfig %s %d.%d.%d.%d netmask 255.255.255.252 mtu 32768 up",tun_name,gv_tun.gw_v4[0],gv_tun.gw_v4[1],gv_tun.gw_v4[2],gv_tun.gw_v4[3]);
				printf("shll-cmd=%s\n",shell_cmd);
				system(shell_cmd);
			}
		}
		#endif
		//printf("%s,%d EP_ALIVE:%d\n",__func__,__LINE__,EP_ALIVE_FLAG);
		sleep(1);
	}
	return ;

}

/*
* 创建文件,初始为0 每次读取一个字符.字符1, IOCTL_WDT_CLR.
*/
void wdt_clr_task(void *data_ptr)
{
	int ret;
	int* pci_fd = (int* )data_ptr;
	int es_fd = -1;
	char file_buf;	
	char file_init = '0';

	int wdt_flag = 0;
	int recan_fd = -1;
	char recan_file[64];
	char name[128];
    
	sprintf(name,".WDT_EN_%d",gv_chip_id);
	
	es_fd=open(name,O_CREAT|O_RDWR,0777);
	if (es_fd<0)
	{
		perror("open");
		return ;
	}

	ret = write(es_fd,&file_init,sizeof(file_init));
	while(1)
	{
		lseek(es_fd, 0, SEEK_SET);//每次只读首个字符.
		ret = read(es_fd,&file_buf,sizeof(file_buf));
		//printf("read file_buf:%c\n", file_buf);
		
		if(file_buf == '1' )
			ret = gv_clr_wdt(tun_chn_tx.fd,1);
		sleep_us(500000);
	}

	close(es_fd);
	return ;
}


void tun_tx_task(void *data_ptr)
{
	int ret;
	int tun=*((int *)data_ptr);
	while(1)
	{
		if (EP_ALIVE_FLAG == 1)
		{	
			ret=read(tun,tx_Buf,READ_WRITE_SIZE);
			if (ret>0)
			{
				ret = write(tun_chn_tx.fd,tx_Buf,ret);
				//if (ret == 0)
					//EP_ALIVE_FLAG = 0;
					//printf("channel->tun %d bytes\n", ret);
			}
		} else {
			printf("%s,%d EP_ALIVE:%d\n",__func__,__LINE__,EP_ALIVE_FLAG);
			sleep(1);
		}
	}
}


int main(int argc, char** argv)
{
	int i;
	int ret;
	int tun;
	pid_t pid;
	int pid_fd = -1;
	char pid_file[64];
	char pid_ch[64];
	//char tun_name[IFNAMSIZ]={0};
	
	//__GV_TUN gv_tun;
	
	printf("Build: %s %s\n",__DATE__,__TIME__);
	
	if(argc<2)
	{
		ERR1:
		printf("%s [machine] [gv_chip_id]\n",argv[0]);
		return 1;
	}

	gv_machine_id=atol(argv[1]);
	gv_chip_id=atol(argv[2]);
	
	printf("host.gv_machine_id=%d\n",gv_machine_id);
	printf("host.gv_chip_id=%d\n",gv_chip_id);
	if ((gv_machine_id<0)||(gv_machine_id>240)||(gv_chip_id<0)||(gv_chip_id>40))
	{
		goto ERR1;
	}
	

    if (gv_HostChannel_Open(&tun_chn_rx,gv_chip_id,(NUM_READ_CHANNELS-1))<0)//13
    	goto ERR1;
    if (gv_HostChannel_Open(&tun_chn_tx,gv_chip_id,(NUM_DATA_CHANNELS-1))<0)//27
    	goto ERR1;
	/*生成tun 服务的pid 文件*/
	pid = getpid();
	printf("当前进程ID：%d\n", pid);
	//sprintf(pid_file,"./.pid_%d.sh",gv_chip_id);
	sprintf(pid_file,"/tmp/.pid_%d.sh",gv_chip_id);
	pid_fd = open(pid_file,O_CREAT|O_RDWR,0777);
						
	sprintf(pid_ch,"%d\t", pid);
	ret = write(pid_fd,&pid_ch,strlen(pid_ch));
	if(ret < sizeof(pid))
	{
		printf("err write ./.pid_sh\n");
		goto ERR1;
	}
	#if 1
	gv_tun.ip_v4[0]=10;
	gv_tun.ip_v4[1]=254;
	gv_tun.ip_v4[2]=10+gv_machine_id;//服务器ID
	gv_tun.ip_v4[3]=100+gv_chip_id;//芯片

	gv_tun.mask_v4[0]=255;
	gv_tun.mask_v4[1]=255;
	gv_tun.mask_v4[2]=252;
	gv_tun.mask_v4[3]=0;
	
	gv_tun.gw_v4[0]=10;
	gv_tun.gw_v4[1]=254;
	gv_tun.gw_v4[2]=10+gv_machine_id;
	gv_tun.gw_v4[3]=gv_chip_id;

	gv_tun.dns_v4[0]=114;
	gv_tun.dns_v4[1]=114;
	gv_tun.dns_v4[2]=114;
	gv_tun.dns_v4[3]=114;
	#endif
	printf("tun.ip_v4  : %d.%d.%d.%d\n",gv_tun.ip_v4[0],gv_tun.ip_v4[1],gv_tun.ip_v4[2],gv_tun.ip_v4[3]);
	printf("tun.mask_v4: %d.%d.%d.%d\n",gv_tun.mask_v4[0],gv_tun.mask_v4[1],gv_tun.mask_v4[2],gv_tun.mask_v4[3]);
	printf("tun.gw_v4  : %d.%d.%d.%d\n",gv_tun.gw_v4[0],gv_tun.gw_v4[1],gv_tun.gw_v4[2],gv_tun.gw_v4[3]);
	printf("tun.dns_v4 : %d.%d.%d.%d\n",gv_tun.dns_v4[0],gv_tun.dns_v4[1],gv_tun.dns_v4[2],gv_tun.dns_v4[3]);
	gv_ChipTunDev_config(&tun_chn_tx,&gv_tun);

	EP_ALIVE_FLAG = 1;
	sprintf(tun_name,"tun4gv9531_%d_%d",gv_machine_id,gv_chip_id);
	tun = tun_creat(tun_name,IFF_TUN|IFF_NO_PI);//如果需要配置tun设备，则把"IFF_TAP"改成“IFF_TUN”
	if(tun<0)
	{
		perror("tun_create");
		printf("tun_create %s failed\n",tun_name);
		return 1;
	}
	printf("TUN name is %s\n",tun_name);
	
	pthread_create(&tun_tx_thread,NULL,(void *)&tun_tx_task,&tun);
	//enable wdt	
	pthread_create(&wdt_clr_thread,NULL,(void *)&wdt_clr_task,&tun_chn_tx.fd);
	//read chip id
	//pthread_create(&read_chipid_thread,NULL,(void *)&read_chipid_task,&tun_chn_tx.fd);
	
	//system("ifconfig tun4gv9531_0 10.254.10.20 up");
	//system("route add -net 192.168.10.0/24 dev tun4gv9531_0");
	{
		char shell_cmd[256];
		sprintf(shell_cmd,"ifconfig %s %d.%d.%d.%d netmask 255.255.252.0 mtu 32768 up",tun_name,gv_tun.gw_v4[0],gv_tun.gw_v4[1],gv_tun.gw_v4[2],gv_tun.gw_v4[3]);
		printf("shll-cmd=%s\n",shell_cmd);
		system(shell_cmd);
		sprintf(shell_cmd,"route add -host %d.%d.%d.%d dev %s",gv_tun.ip_v4[0],gv_tun.ip_v4[1],gv_tun.ip_v4[2],gv_tun.ip_v4[3],tun_name);
		printf("shll-cmd=%s\n",shell_cmd);
		system(shell_cmd);
	}
	
	while(1)
	{
          if (EP_ALIVE_FLAG == 1)	  
          {
        	//printf("%s,%d EP_ALIVE:%d\n",__func__,__LINE__,EP_ALIVE_FLAG);
	
		struct pollfd fds[1];
		fds[0].fd     = tun_chn_rx.fd;
		 fds[0].events = POLLIN;
		 //ret = poll(fds,1,-1);
		 ret = poll(fds,1,1000);
		 if (ret<0)
		 {
		       printf("poll err\n");
		       return -1;
		 }
		    
		ret = read(tun_chn_rx.fd,ReadBuf,READ_WRITE_SIZE);
		if (ret>0)
		{
			ret = write(tun,ReadBuf,ret);
			if (ret<=0)
			printf("tun->channel %d bytes\n", ret);
		}
          }else {
	        printf("%s,%d EP_ALIVE:%d\n",__func__,__LINE__,EP_ALIVE_FLAG);
	        exit(1);
	        sleep(1);
	  }	
	}

	
	gv_HostChannel_Close(&tun_chn_rx);
	gv_HostChannel_Close(&tun_chn_tx);	

	//gv_HostChannel_Close(&wdt_clr_ch);	

	return 0;
}
