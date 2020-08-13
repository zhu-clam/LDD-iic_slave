#include "gv_host_channel_api.h"


/*
* brief: when wdt ==1 will clear wdt; IOCTL_WDT_CLR will always read chip id.
* return: 1 means read chip id success. otherwise read chip id fail.
*/
int gv_clr_wdt(int ep_fd,int wdt)
{
    int ret;
    int chip_id;  
    //chip_id = ioctl(ep_fd,IOCTL_WDT_CLR,1);
	ret = ioctl(ep_fd,IOCTL_WDT_CLR,wdt);
	if (ret < 0)
    {
        perror("ioctl IOCTL_WDT_CLR");
        goto OPEN_ERR1;
    }
	return ret;
    
 OPEN_ERR1:
	return -1;

}

int gv_read_chipid(int ep_fd)
{
    int ret;
    int chip_id;  
	chip_id = ioctl(ep_fd,IOCTL_CHIP_ID,0);
	if (ret < 0)
    {
        perror("ioctl IOCTL_CHIP_ID");
        goto OPEN_ERR1;
    }

	//printf("%s,%d,ret = 0x%x\n",__func__,__LINE__,chip_id);
	if(GV_CHIP_ID == chip_id)
	{	
		ret = 1;		
	}
	else { 
		ret = 0;
	}
	return ret;
    
 OPEN_ERR1:
	return -1;

}


int  gv_HostChannel_Open(__GV_HOST_CHANNEL *pGvChannel,int chip_id,int channel_id)
{
    int ret;
    int pci_rc_fd=-255;
    char name[128];
    
    if(channel_id>=NUM_DATA_CHANNELS)
    	return -1;
    
    sprintf(name,"/dev/pci-endpoint-test.%d",chip_id);
    printf("gvdev:%s\n",name);
    pci_rc_fd = open(name,O_RDWR);
    if(pci_rc_fd < 0)
    {
        perror("open");
        pGvChannel->fd=-1;
        return -1;
    }

    ret = ioctl(pci_rc_fd,IOCTL_SET_CHANNEL,channel_id);
    if (ret < 0)
    {
        perror("ioctl IOCTL_SET_CHANNEL");
        goto OPEN_ERR1;
    }
    pGvChannel->fd=pci_rc_fd;
    pGvChannel->chip_id=chip_id;
    pGvChannel->channel_id=channel_id;
    
    printf("pci_rc_fd=%d\n",pci_rc_fd);
    printf("chip_id=%d\n",chip_id);
    printf("channel_id=%d\n",channel_id);
    
    return 0;
    
 OPEN_ERR1:
 	close(pGvChannel->fd);
 	pGvChannel->fd=0;
 	return -4;
 	
}


int gv_SetRemote_ShellMode(__GV_HOST_CHANNEL *pGvChannel)
{
    int ret;
      
    ret = ioctl(pGvChannel->fd,IOCTL_SET_SHELL,0);
    if (ret < 0)
    {
        perror("ioctl IOCTL_CMD_SHELL");
        goto OPEN_ERR1;
    }
    
    return 0;
    
 OPEN_ERR1:
	return -1;
}


int gv_ClrRemote_ShellMode(__GV_HOST_CHANNEL *pGvChannel)
{
    int ret;
      
    ret = ioctl(pGvChannel->fd,IOCTL_CLR_SHELL,0);
    if (ret < 0)
    {
        perror("ioctl IOCTL_CMD_SHELL");
        goto OPEN_ERR1;
    }
    
    return 0;
    
 OPEN_ERR1:
	return -1;
}

int gv_ChipTunDev_config(__GV_HOST_CHANNEL *pGvChannel,__GV_TUN *gv_tun)
{
    int ret;
      
    ret = ioctl(pGvChannel->fd,IOCTL_SET_TUN,gv_tun);
    if (ret < 0)
    {
        perror("ioctl IOCTL_SET_TUN");
        goto OPEN_ERR1;
    }
    
    return 0;
    
 OPEN_ERR1:
	return -1;
}


int gv_HostChannel_Close(__GV_HOST_CHANNEL *pGvChannel)
{

	close(pGvChannel->fd);
	pGvChannel->fd=0;
	pGvChannel->chip_id=-1;
	pGvChannel->channel_id=-1;

	return 0;
}

//#define CREATE_DATA_FILE
#ifdef CREATE_DATA_FILE
static int es_fd[NUM_DATA_CHANNELS] = {0};
#endif

int gv_HostChannel_Write(__GV_HOST_CHANNEL *pGvChannel,void *vAddr,unsigned int size)
{
    unsigned int ret  = 0;

	if (pGvChannel->fd<0)
		return -1;

#ifdef CREATE_DATA_FILE
	char name[128];
	int channel_id = pGvChannel->channel_id;

	/*put write data into host file*/
	sprintf(name,"./rc_data_file_ch_%d.txt",channel_id);
	if(es_fd[channel_id] == 0)
	{
			es_fd[channel_id]=open(name,O_CREAT|O_RDWR,0777);
	}
	if(es_fd < 0)
			printf("open rc_data_file error!\n");
	printf("open rc_data_file es_fd:%d!\n",es_fd[channel_id]);
	write(es_fd[channel_id],vAddr,size);
#endif

	ret = write(pGvChannel->fd,vAddr,size);
	if( ret !=size )
	{
		printf("gv_pcieHostDmaWrite write ERROR!\n");
		ret = -1;
	}
    return ret;
}


int gv_HostChannel_Read(__GV_HOST_CHANNEL *pGvChannel,void *vAddr,unsigned int size)
{
    unsigned int ret  = 0;

	if (pGvChannel->fd<0)
		return -1;


	ret = read(pGvChannel->fd,vAddr,size);
	if( ret !=size )
	{
		printf("gv_pcieHostDmaWrite write ERROR!\n");
		ret = -1;
	}
    return ret;
}
