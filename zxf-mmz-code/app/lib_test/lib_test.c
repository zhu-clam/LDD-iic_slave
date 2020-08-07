/*************************************************************************
	> File Name: lib_test.c
	> Author: ma6174
	> Mail: ma6174@163.com 
	> Created Time: Tue 17 Sep 2019 10:45:27 PM PDT
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

#define mmb0_name "grandvision0"
#define mmb1_name "grandvision1"


void*  mmb_pthread(void* arg)
{

	printf("mmb_pthread is run...\n");
	GV_MMB_INFO_S* mmb_1;
	int ret;
	int mmz_fd;
	mmb_1  = GV_MMZ_GetMMB(0x200000, mmb1_name,&mmz_fd);
	if(!mmb_1)
	{
		printf("ERROR: get mmb1 failed!\n");
		exit(-1);
	}

	printf("mmb_1.mapped a:0x%x ,value1 = 0x%x\n",mmb_1->mapped,*((unsigned int *)mmb_1->mapped));
	*((unsigned int *)mmb_1->mapped) = 520;
	printf("mmb_1.mapped b:0x%x ,value1 = %d\n",mmb_1->mapped,*((unsigned int *)mmb_1->mapped));

	ret = GV_MMZ_FreeMMB(mmb_1,&mmz_fd);
	if(ret == GV_FALSE)
	{
		printf("ERROR: Free mmb1 failed!\n");
		exit(-1);
	}
	
	printf("mmb_pthread is exit.....\n");
	return NULL;

}

int main()
{
	GV_MMB_INFO_S* mmb_0;
	GV_BOOL ret;
	int mmz_fd;
	
	pthread_t mmb_pid;
	
	if (0 != pthread_create(&mmb_pid, NULL, mmb_pthread, NULL))
	{
		printf("pthread_create mmb_pid fail\n");
		exit(-1);
	}	
	
	mmb_0 = GV_MMZ_GetMMB(0x100000, mmb0_name,&mmz_fd);
	if(!mmb_0)
	{
		printf("ERROR: get mmb failed!\n");
		exit(-1);
	}
	printf("mmb_0.mapped a:0x%x ,value = 0x%x\n",mmb_0->mapped,*((unsigned int *)mmb_0->mapped));
	*((unsigned int *)mmb_0->mapped) = 1314;
	printf("mmb_0.mapped b:0x%x ,value = %d\n",mmb_0->mapped,*((unsigned int *)mmb_0->mapped));

	char cmd[64];
	printf("Enter q to continue\n");
	while (0 != strncmp(fgets(cmd, 64, stdin), "q", 1))
	{
		printf("Enter q to continue\n");
	}

	ret = GV_MMZ_FreeMMB(mmb_0,&mmz_fd);
	if(ret == GV_FALSE)
	{
		printf("ERROR: Free mmb failed!\n");
	}

	pthread_join(mmb_pid, NULL);
	printf("THE mmb_pid pthread is finish!\n");

}

