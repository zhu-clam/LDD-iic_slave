/*
 * ep_aiTopsStateInfo.c
 *
 *  Created on: July 30, 2020
 *      Author: wangst
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include "ep_aiTopsStateInfo.h"

#define ALIGN_4K	4096
#define MEM_ALIGNMENT_UP(size, align_size) \
	((size + (align_size - 1))  & ~(align_size - 1))

#define MEM_ALIGNMENT_DOWN(size, align_size) \
	((size)  & ~(align_size - 1))

#define AI_CORE_NUM 5
#define READ_TIMES_NUM 20

static const char dev[]="/dev/mem";
void gv_epAiState();

static void help()
{
	printf("---------------H E L P---------------\n");

	printf("Please enter the correct command:\n");
	printf(".eg:\n");
	printf("	./readwritemem\n");

}

static int memmap(unsigned int phy_addr, unsigned int size, unsigned int *register_value)
{
	unsigned int phy_addr_in_page;
	unsigned int page_diff;
	unsigned int size_in_page;
	
	void *addr=NULL;

	if(size == 0)
	{
		printf("size can't be zero!\n");
		return -1;
	}
	
	int fd = -1;

	fd = open (dev, O_RDWR | O_SYNC);
	if (fd < 0)
	{
		printf("open %s error!\n", dev);
		return -1;
	}

	/* addr align in page_size(4K) */
	phy_addr_in_page = MEM_ALIGNMENT_DOWN(phy_addr, ALIGN_4K);
	page_diff = phy_addr - phy_addr_in_page;
	//printf("addr:0x%x, phy_addr_in_page:0x%x, page_diff:0x%x  \n", phy_addr,phy_addr_in_page,page_diff);

	/* size in page_size */
	size_in_page =MEM_ALIGNMENT_UP(size, ALIGN_4K);
	//printf("size_in_page: 0x%x \n", size_in_page);
	//void* mmap(void* start,size_t length,int prot,int flags,int fd,off_t offset);
	addr = mmap ((void *)0, size_in_page, PROT_READ|PROT_WRITE, MAP_SHARED , fd, phy_addr_in_page);
	if (addr == MAP_FAILED)
	{
		printf("mmap @ 0x%x error!\n", phy_addr_in_page);
		return -1;
	}


	void *virt_addr = addr+page_diff;

	*(register_value) = *(unsigned int*)(virt_addr);


	munmap(addr, size_in_page);
	if(fd)
	{
		close(fd);
		fd = -1;
	}

	
	return 0;
}


static int readmem(char * addr_char, char *value_char, unsigned int *register_value)
{
	//Extract the address
	unsigned int addr;
	char buff[10];
	if (addr_char[0] == '0' && (addr_char[1]=='x' || addr_char[1]=='X'))  
	{
		sscanf (addr_char, "%2s%8x", buff, &addr);
		//printf("addr: %s%x", buff, addr);
	}
	else
	{
		printf("please input addr like 0x12345678\n");
		return -1;
	}

	//Extract the value
	unsigned int value;
	if (value_char[0] == '0' && (value_char[1]=='x' || value_char[1]=='X'))  
	{
		sscanf (value_char, "%2s%8x", buff, &value);
		//printf("value: %s%x", buff, value);
	}
	else
	{
		sscanf (value_char, "%d", &value);
		//printf("value: %d", value);
	}

	//printf("====dump memory %#lX====\n", addr);

    int ret = memmap(addr, value, register_value);
    if(ret < 0)
    {
  		return -1;
  	}
	
	return 0;
}

int main(int argc , char* argv[])
{

    if ((argc != 1) )
    {
        help();
        return -1;
    }

	gv_epAiState();
    return 0;

}


void gv_epAiState()
{

	//char *register_addr[5] = {"0xF3400004"/*vip*/, "0xF009FFFC"/*c5A*/, "0xF00DFFFC"/*p6A*/, "0xF00FFFFC"/*p6B*/, "0xF00BFFFC"/*c5B*/};
	char *register_addr[5] = {"0xF3400004"/*vip*/, "0xF009FFFC"/*c5A*/,"0xF00BFFFC"/*c5B*/, "0xF00DFFFC"/*p6A*/, "0xF00FFFFC"/*p6B*/};
	unsigned int register_value[5] = {0};
	unsigned int TOPS_per[5] = {0};

	char *value_sz = "0x4";


	for(int i=0; i < 5 ;i++ )

	{
		int count = 0;
		if(i == 0)
		{
			for (int j=0; j < READ_TIMES_NUM; j++)
			{


				int ret = readmem(register_addr[i], value_sz, &(register_value[i]));
				if(ret < 0)
				{
					printf("readmem failed\n");
				}
				usleep(2000);

			    unsigned int idle = register_value[i];
				if ((idle & 0x00000001) == 0 && (idle & 0x00000008) == 0 && (idle & 0x00004000) == 0)
				{
					count = count+1;
				}

			}
				TOPS_per[i] = (((count)*(472)/READ_TIMES_NUM));


		}
		else if(i>=1 && i <= 2)
		{

			for (int j=0; j < READ_TIMES_NUM; j++)
			{


				int ret = readmem(register_addr[i], value_sz, &(register_value[i]));
				if(ret < 0)
				{
					printf("readmem failed\n");
				}
				usleep(2000);

			    unsigned int idle = register_value[i];
				if (idle == 0x03)
				{
					count = count+1;
				}

			}

			TOPS_per[i] = (((count)*(314)/READ_TIMES_NUM));

		}
		else if(i >=3 )
		{

			for (int j=0; j < READ_TIMES_NUM; j++)
			{


				int ret = readmem(register_addr[i], value_sz, &(register_value[i]));
				if(ret < 0)
				{
					printf("readmem failed\n");
				}
				usleep(2000);

			    unsigned int idle = register_value[i];
				if (idle == 0x03)
				{
					count = count+1;
				}

			}
			TOPS_per[i] = (((count)*(79)/READ_TIMES_NUM));
		}


		printf("%d : %d\n",i,TOPS_per[i]);	

	}

	
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */





