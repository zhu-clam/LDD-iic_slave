#ifndef __PCI_DATA_CHANNEL_H
#define __PCI_DATA_CHANNEL_H

#include <linux/kernel.h>
#include <linux/crc32.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>

#include <linux/pci_regs.h>
#include <linux/msi.h>
#include <linux/vmalloc.h>

#include <linux/uaccess.h>
#include <linux/kfifo.h>
#include <linux/poll.h>

#include "pci_endpoint_test.h"

#include <linux/init.h>
#include <linux/kthread.h>   
#include <linux/wait.h>


#define CH_FLAG_HOST_WRITE    BIT(0)
//配置芯片TUN网络
#define CH_FLAG_TUN           BIT(2)
#define CH_FLAG_HOST_READ     BIT(1)
#define CH_FLAG_GV_USED       BIT(6)//gv9531
#define CH_FLAG_HOST_USED     BIT(7)


//执行扩展的shell命令
#define CH_FLAG_REMOTE_SHELL  BIT(8)

//清看门狗
#define CH_FLAG_CLR_WDT  BIT(4)

#pragma pack (4)
typedef	struct
{
	u8 ip_v4[4];
	u8 mask_v4[4];
	u8 gw_v4[4];
	u8 dns_v4[4];
}__GV_TUN;
#pragma pack ()

/*
typedef struct
{//通道公用的信息
	u8  reserve[0x105c];
	u32 gv_sw_int_reg;
}__EPF_PUB_REG;
*/

typedef struct
{//通道公用的信息
	u8  reserve[0x1020];
	u32 gv_chip_id;
	u8  reserve1[0x38];
	u32 gv_sw_int_reg;
}__EPF_PUB_REG;



#pragma pack (4)
struct pci_epf_func_reg {
	u32	magic;//0
	u32	command;//4
	u32	status;//8
	u64	src_addr;//12
	u64	dst_addr;//20
	u32	size;//28
	u32	checksum;//32
	u32	irq_type;//36
	u32	irq_number;//40
	u32 Flag;//44
	u32 phy_index;//48
	u32 phy_len;//50
	u32 hostbuf_size;
	u32 hostbuf_rdptr;
	u32 hostbuf_wrptr;
	u32 phy_mem_size;
	u32 pci_version;
	
	__GV_TUN gv_tun;
};
#pragma pack ()

//物理地址读取
struct phy_mem_array
{
	unsigned int id;
	unsigned int buffer_size;//buf大小
	unsigned int data_len;//读写数据大小
	unsigned int data_index;//偏移位置
	void *virt_mem;
	void *phy_mem;
	int reserve[2];//凑到32字节对齐
};

typedef struct
{
	void *bufPtr;
	int bufIdx;
	unsigned int bufSize;
	unsigned int ttlSize;
}gv_bufWirte;



struct pci_epf_data_transfer{
	int channel_id;
	__EPF_PUB_REG            *pub_reg;
	struct pci_epf_func_reg  *ep_reg;
	
	//传输中断请求
	struct completion irq_raised;
	
	//读取数据线程
	struct task_struct *host_read_tsk;
	
	//中断数
	unsigned long interrupts;
	
	//缓存地址
	//void *buffer;
	//struct kfifo  data_fifo;
	
	//struct __kfifo buffer_fifo;
	//spinlock_t fifo_lock;
	
	struct __DMA{
		size_t alignment;
		size_t size;
				
		void *orig_addr; /*内核虚拟地址*/
		dma_addr_t orig_phys_addr; /* 内核物理地址 */
		dma_addr_t phys_addr;      /* 真正数据的物理地址*/ 
		void *addr;                /* 真正数据的虚拟地址*/
	}dma;
	
	wait_queue_head_t pci_epf_waitq;	/* 等待队列头 */
	volatile unsigned int poll_read_flag;
	
	struct phy_mem_array phy_mem;
	
};

//数据通道数量
#define NUM_READ_CHANNELS   14 
#define NUM_WRITE_CHANNELS  14
#define NUM_DATA_CHANNELS  (NUM_READ_CHANNELS+NUM_WRITE_CHANNELS)


extern  u32 pci_endpoint_readl(void __iomem *reg_base);
extern  void pci_endpoint_writel(void __iomem *reg_base,u32 value);

extern int pci_data_channels_read_thread(void *data);
extern int pci_data_channels_init(int channel_id,struct pci_endpoint_test *pci_host,struct pci_epf_data_transfer  *channel,size_t size,unsigned int irq_number);
extern int pci_data_channels_exit(struct device *dev,struct pci_epf_data_transfer  *channel);
extern void pci_epf_irq_trigger(struct pci_epf_data_transfer  *channel,u32 reg_value);
extern u32 pci_get_chip_id(struct pci_epf_data_transfer  *channel);


#endif


