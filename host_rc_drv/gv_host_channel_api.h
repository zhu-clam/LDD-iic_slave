#ifndef __GV_HOST_CHANNEL_H
#define __GV_HOST_CHANNEL_H

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

typedef struct
{
	int fd;
	int chip_id;
	int channel_id;
	int error_no;
}__GV_HOST_CHANNEL;


#pragma pack (4)
typedef	struct
{
	unsigned char ip_v4[4];
	unsigned char mask_v4[4];
	unsigned char gw_v4[4];
	unsigned char dns_v4[4];
}__GV_TUN;
#pragma pack ()


//#define CREATE_DATA_FILE

//DMA分配内存大小
#define DMA_MEM_SIZE                   (2*1024*1024) 
#define IOCTL_SET_CHANNEL              _IO('I', 0x1)
#define IOCTL_GET_CHANNEL              _IO('I', 0x2)

//配置网络
#define IOCTL_SET_TUN                  _IO('I', 0x3)

//远程shell命令
#define IOCTL_SET_SHELL                _IO('I', 0x4)
#define IOCTL_CLR_SHELL                _IO('I', 0x5)

#define IOCTL_GET_FLAG                 _IO('I', 0x6)


//使能WDT
#define IOCTL_WDT_CLR                 _IO('I', 0x7)
#define IOCTL_CHIP_ID                 _IO('I', 0x8)
#define IOCTL_PHY_MEM_WRITE           _IO('I', 0x9)



extern int gv_HostChannel_Open(__GV_HOST_CHANNEL *pGvChannel,int chip_id,int channel_id);
extern int gv_HostChannel_Close(__GV_HOST_CHANNEL *pGvChannel);
extern int gv_HostChannel_Read(__GV_HOST_CHANNEL *pGvChannel,void *vAddr,unsigned int size);
extern int gv_HostChannel_Write(__GV_HOST_CHANNEL *pGvChannel,void *vAddr,unsigned int size);

extern int gv_ChipTunDev_config(__GV_HOST_CHANNEL *pGvChannel,__GV_TUN *gv_tun);
extern int gv_SetRemote_ShellMode(__GV_HOST_CHANNEL *pGvChannel);
extern int gv_ClrRemote_ShellMode(__GV_HOST_CHANNEL *pGvChannel);

extern int gv_clr_wdt(int ep_fd,int wdt);
extern int gv_read_chipid(int ep_fd);


#define NUM_READ_CHANNELS   14
#define NUM_WRITE_CHANNELS  14
#define NUM_DATA_CHANNELS  (NUM_READ_CHANNELS+NUM_WRITE_CHANNELS)

#define GV_CHIP_ID (0x20190101) 


#endif



