/*************************************************************************
	> File Name: app_test.c
	> Author: ma6174
	> Mail: ma6174@163.com 
	> Created Time: Sun 15 Sep 2019 10:36:55 PM PDT
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


#include "mmz_app.h"

#define MMZ_DEV "/dev/mmz_userdev"

void*  mmb_pthread(void* arg)
{
	int ret;
	struct mmb_info mmb_1;	/*未初始化清零*/
	
	memset(&mmb_1,0,sizeof(struct mmb_info));
	
	mmb_1.size = 0x100000;//1M
	mmb_1.align = 0x1000;//默认4k 字节[页对齐]
	sprintf(mmb_1.mmb_name,"grandvision%d",1);
	strcpy(mmb_1.mmz_name,"anonymous");

	printf("mmb_pthread is run...\n");

	int mmz_fd = *(int *)arg; //必须先转换类型,再取值.不能先取值在转类型.
#if 0
	mmz_fd = open(MMZ_DEV,O_RDWR);
	if(!mmz_fd) {
		printf("open dev error!\n");	
		exit(-1);
	}
#endif
	/*step2:send ioctl for allcator mmb from mmz-dev*/
	ret = ioctl(mmz_fd,IOC_MMB_ALLOC,&mmb_1);
	if(ret)
	{
		printf("ioctl IOC_MMB_ALLOC error!\n");
		exit(-1);
	}

	printf("mmb_1 alloc success: phys_addr:0x%x,size:0x%x,mmb is %s,in mmz %s\n",mmb_1.phys_addr,mmb_1.size,mmb_1.mmb_name,mmb_1.mmz_name);

	/*step3:send ioctl for user remap mmb into user space*/
	ret = ioctl(mmz_fd,IOC_MMB_USER_REMAP,&mmb_1);
	if(ret)
	{
		printf("ioctl IOC_MMB_USER_REMAP error!\n");
		exit(-1);	
	}
	//mmb_0.mapped = mmap(null, mmb_0.size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	printf("mmb_1.mapped a:0x%x ,value = 0x%x\n",mmb_1.mapped,*((unsigned int *)mmb_1.mapped));
	*((unsigned int *)mmb_1.mapped) = 520; 
	printf("mmb_1.mapped b:0x%x ,value = %d\n",mmb_1.mapped,*((unsigned int *)mmb_1.mapped));

	ret = ioctl(mmz_fd,IOC_MMB_USER_UNMAP,&mmb_1);
	if(ret)
	{
		printf("ioctl IOC MMB USER UNMAP error!\n");
		exit(-1);
	}
#if 0
	close(mmz_fd);
#endif
	printf("mmb_pthread is exit.....\n");
	return NULL;
}
int main()
{
	int ret = 0;
	int i = 0;
	struct mmb_info mmb_0;	

	memset(&mmb_0,0,sizeof(struct mmb_info));
	
	mmb_0.size = 0x100000;//1M
	mmb_0.align = 0x1000;//默认4k 字节[页对齐]
	mmb_0.gfp = 0;
	sprintf(mmb_0.mmb_name,"grandvision%d",0);
	strcpy(mmb_0.mmz_name,"anonymous");
	
	/*step1: open mmz-device*/	
	int mmz_fd;
	mmz_fd = open(MMZ_DEV,O_RDWR);
	if(!mmz_fd) {
		printf("open dev error!\n");	
	}

	/*创建线程函数，在里面申请mmb块*/
	pthread_t mmb_pid;
	if (0 != pthread_create(&mmb_pid, NULL, mmb_pthread, (void *)&mmz_fd))
	{
		printf("pthread_create mmb_pid fail\n");
		exit(-1);
	}	

	/*step2:send ioctl for allcator mmb from mmz-dev*/
	ret = ioctl(mmz_fd,IOC_MMB_ALLOC,&mmb_0);
	if(ret)
	{
		printf("ioctl IOC_MMB_ALLOC error!\n");
		goto exit;
	}

	printf("mmb_0 alloc success: phys_addr:0x%x,size:0x%x,mmb is %s,in mmz %s\n",mmb_0.phys_addr,mmb_0.size,mmb_0.mmb_name,mmb_0.mmz_name);


#if 0
	/*直接使用mmap(mmz_fd)出来的虚拟地址可以在应用空间使用,但无法释放，按正规操作时调用ioctl命令*/
	//mmb_0.mapped = mmap(0,mmb_0.size,PROT_READ|PROT_WRITE, MAP_SHARED,mmz_fd,0);
	/*直接mmap后,在linux 内核中mmb_info链表里mapped是没有更新的,而是直接map到应用下的mmb_0*/
	mmb_0.mapped = mmap(0,mmb_0.size,PROT_READ|PROT_WRITE, MAP_SHARED,mmz_fd,mmb_0.phys_addr);
	if(mmb_0.mapped == MAP_FAILED)
	{
		mmb_0.mapped = 0;
		goto exit;
	}
	
	*((int *)mmb_0.mapped) = 0x12345678; 
	printf("mmb_0.mapped:0x%x ,value = 0x%x\n",mmb_0.mapped,*((int *)mmb_0.mapped));

	printf("mmap is success!\n");
	char cmd[64];
	
	while (0 != strncmp(fgets(cmd, 64, stdin), "q", 1))
	{
		printf("Enter q to continue\n");
	}

#endif
#if 0
	for(i =0 ; i <10 ;i++)
	{
		((unsigned int *)mmb_0.mapped)[i] = i;
	}
	printf("mapped:0x%8x \n",mmb_0.mapped);

	for(i =0 ; i <10 ;i++)
	{
		printf("%d\n",((unsigned int *)mmb_0.mapped)[i]);
	}
#endif

#if 1
	/*step3:send ioctl for user remap mmb into user space*/
	ret = ioctl(mmz_fd,IOC_MMB_USER_REMAP,&mmb_0);
	if(ret)
	{
		printf("ioctl IOC_MMB_USER_REMAP error!\n");
		goto exit;
	}
	//mmb_0.mapped = mmap(null, mmb_0.size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	printf("mmb_0.mapped a:0x%x ,value = 0x%x\n",mmb_0.mapped,*((unsigned int *)mmb_0.mapped));
	*((unsigned int *)mmb_0.mapped) = 1314; 
	printf("mmb_0.mapped b:0x%x ,value = %d\n",mmb_0.mapped,*((unsigned int *)mmb_0.mapped));

#endif


	char cmd[64];
	
	while (0 != strncmp(fgets(cmd, 64, stdin), "q", 1))
	{
		printf("Enter q to continue\n");
	}

	ret = ioctl(mmz_fd,IOC_MMB_USER_UNMAP,&mmb_0);
	if(ret)
	{
		printf("ioctl IOC MMB USER UNMAP error!\n");
		goto exit;
	}

	pthread_join(mmb_pid, NULL);//阻塞等待子线程audiopid完成
	printf("THE mmb_pid pthread is finish!\n");


exit:

	close(mmz_fd);

	return 0;
}
