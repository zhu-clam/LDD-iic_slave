/*************************************************************************
	> File Name: gv_mmz_lib.c
	> Author: ma6174
	> Mail: ma6174@163.com 
	> Created Time: Tue 17 Sep 2019 08:03:59 PM PDT
 ************************************************************************/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <string.h>


#include <sys/mman.h>

#include "gv_mmz_lib.h"


#define MMZ_DEV "/dev/mmz_userdev"

//#define error(s...) do{ printf("mmz_userdev:%s: ", __FUNCTION__); printf(s); }while(0)
//#define warning(s...) do{ printf("mmz_userdev:%s: ", __FUNCTION__); printf(s); }while(0)

#define hal_err(s...) do { \
	pr_ipc("<hal_err>[%s:%d] ", __func__, __LINE__); \
	pr_ipc(s); \
	pr_ipc("\r\n"); \
} while (0)

static inline void pr_ipc(const char *fmt, ...)
{
	printf(fmt);
}

//static int mmz_fd;


/*malloc mmb*/
/*
* OUTPUT: 1. mmb_info 2. mmz_fd
*/
GV_MMB_INFO_S* GV_MMZ_GetMMB(unsigned int size,char *mmb_name,int* mmz_fd)
{
	
	if(strlen(mmb_name) >= GV_MMB_NAME_LEN)
	{
		hal_err("mmb_name is beyond max mmb name length!\n");
		return NULL;
	}

	int ret;
	GV_MMB_INFO_S* mmb_info_lib = (GV_MMB_INFO_S* )malloc(sizeof(GV_MMB_INFO_S));
	if(!mmb_info_lib)
	{
		hal_err("malloc memory for mmb_info_lib fail.");
		return NULL;
	}

	memset(mmb_info_lib,0,sizeof(GV_MMB_INFO_S));

	mmb_info_lib->size = size;
	strcpy(mmb_info_lib->mmb_name,mmb_name);
	strcpy(mmb_info_lib->mmz_name,"anonymous");

	*mmz_fd = open(MMZ_DEV,O_RDWR);
	if(!(*mmz_fd)) {
		hal_err("open mmz-dev error!\n");	
		free(mmb_info_lib);
		return NULL;
	}


	/*step2:send ioctl for allcator mmb from mmz-dev*/
	ret = ioctl(*mmz_fd,IOC_MMB_ALLOC,mmb_info_lib);
	if(ret)
	{
		hal_err("ioctl IOC_MMB_ALLOC error!\n");
		goto exit;
	}

	ret = ioctl(*mmz_fd,IOC_MMB_USER_REMAP,mmb_info_lib);
	if(ret)
	{
		hal_err("ioctl IOC_MMB_USER_REMAP error!\n");
		goto exit;
	}
 
	*((unsigned int *)mmb_info_lib->mapped)  = 0;
	
	return mmb_info_lib;
	
exit:
	free(mmb_info_lib);
	close(*mmz_fd);
	return NULL;
}
/*free mmb*/
GV_BOOL GV_MMZ_FreeMMB(GV_MMB_INFO_S *mmb,int* mmz_fd)
{
	int ret;
	ret = ioctl(*mmz_fd,IOC_MMB_USER_UNMAP,mmb);
	if(ret)
	{
		hal_err("ioctl IOC_MMB_USER_REMAP error!\n");
		return GV_FALSE;
	}

/*free malloc resource*/
	free(mmb);
	mmb = NULL;
	close(*mmz_fd);

	return GV_TRUE;
}



