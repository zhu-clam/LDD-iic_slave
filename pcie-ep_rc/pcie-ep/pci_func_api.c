#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <sys/ioctl.h>
#include "pci_func_api.h"


#define PAGE_SIZE (1024)
#define BUF_SIZE (256*PAGE_SIZE)
#define BUF_READ_SIZE (128 * PAGE_SIZE)

#define PCI_EPF_FUNC_IOC_MAGIC          'B'
#define PCI_EPF_FUNC_IOC_NOTIFY         _IO(PCI_EPF_FUNC_IOC_MAGIC, 0)
#define PCI_EPF_FUNC_IOC_READ_SIZE			_IO(PCI_EPF_FUNC_IOC_MAGIC, 1)

static int pci_epf_fd = -1;
static char *ptrRead = NULL;
static char * ptrWrite = NULL;


/*
*  调用EP 端 mmap() 函数.
*/
static int gv_pci_ep_mmap()
{
    ptrWrite = mmap(NULL, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, pci_epf_fd, 0);
    if(NULL == ptrWrite)
    {
        perror("gv_pci_ep_mmap:");
        return -1;
    }
    ptrRead = ptrWrite + BUF_READ_SIZE;

    return 0;
}

GV_S32 gv_pci_ep_init()
{
    if(pci_epf_fd > 0)
        return -1;
    pci_epf_fd = open("/dev/pci_epf_func", O_RDWR);
    if(pci_epf_fd < 0)
    {
        perror("/dev/pci_epf_func");
        return -1;
    }


    if( gv_pci_ep_mmap() < 0 )
    {
         close(pci_epf_fd);
         pci_epf_fd = -1;
    }

    return pci_epf_fd;
}

GV_VOID gv_pci_ep_deinit()
{
    munmap(ptrWrite, BUF_SIZE);
    close(pci_epf_fd);
}

GV_S32 gv_pci_ep_read(GV_S8 * ptrReadBuf, GV_U32 readBuflen, GV_S32 ms)
{
    int size,ret;
    struct pollfd fds[1];

    printf("poll wait\n");
    fds[0].fd     = pci_epf_fd;
    fds[0].events = POLLIN;
    ret = poll(fds, 1, ms>0 ? ms : -1);
    if (ret <= 0)
    {
        printf("poll err\n");
        return -1;
    }
    printf("poll finish\n");
    
    if(NULL ==  ptrReadBuf || readBuflen >  BUF_READ_SIZE)
        return -1;

    size = ioctl(pci_epf_fd, PCI_EPF_FUNC_IOC_READ_SIZE, 0);
    if( size < 0 ){
        printf("gv_pci_ep_read failed\n");
        return -1;
    }
    size = size>BUF_READ_SIZE ? BUF_READ_SIZE : size;
    memcpy(ptrReadBuf, ptrRead, size);
    return size;
}


GV_S32 gv_pci_ep_write(GV_S8 * ptrWriteBuf, GV_U32 writeSize)
{
    int ret;
        
    if(NULL == ptrWriteBuf)
        return -1;
    
    if(writeSize > BUF_READ_SIZE || writeSize == 0)
        return -1;
    memcpy(ptrWrite, ptrWriteBuf, writeSize);
    ret = ioctl(pci_epf_fd, PCI_EPF_FUNC_IOC_NOTIFY, writeSize);
    if(ret){
        printf("gv_pci_ep_write failed\n");
        return -1;
    }
    return 0;
}

int main(int argc, const char *argv[])
{
    int ret,i;
    char buf[100];
    
    int fd;
    fd = gv_pci_ep_init();
    if(fd < 0)
        return -1;


	//EP 端轮询等待数据可读
    gv_pci_ep_read(buf, sizeof(buf), 0);
    for(i=0; i<256; i++)
    {
        printf("0x%02x ", buf[i]);
        if((i+1)%10 == 0)
            printf("\n");
    }
    
    memset(buf,0x33, sizeof(buf));
    gv_pci_ep_write(buf, 100);
    while(1)
        sleep(1);
    return 0;
}

