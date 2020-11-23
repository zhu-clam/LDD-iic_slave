#ifndef	__EPF_DATA_CHANNEL_H
#define __EPF_DATA_CHANNEL_H

#include <linux/crc32.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/pci_ids.h>
#include <linux/random.h>

#include <linux/pci-epc.h>
#include <linux/pci-epf.h>
#include <linux/pci_regs.h>

#include <asm/io.h>
#include <asm/pgtable.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/gfp.h>
#include <linux/poll.h>

#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <linux/kfifo.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

#include "pci-epf-gv953x.h"

//数据通道数量
#define EPF_NUM_WRITE_CHANNELS  34 
#define EPF_NUM_READ_CHANNELS   34
#define EPF_NUM_DATA_CHANNELS  (EPF_NUM_WRITE_CHANNELS+EPF_NUM_READ_CHANNELS)
#define EPF_NUM_SHELL_CHANNEL (EPF_NUM_DATA_CHANNELS-2) //64 for gv_cp & gv_shell

//GPIO corresponding
#define GPIO_BASE (0xf800c000)
#define GPIO_SWPORTA_DR		0x00
#define GPIO_SWPORTA_DDR	0x04



#define EPF_DMZ_MEM_SIZE (1*1024*1024)   //1M
#define PHY_MEM_ARRAR_FIFO_SIZE  (1024*2)//2K


#define CH_FLAG_GV_READ     BIT(0)
#define CH_FLAG_GV_WRITE    BIT(1)
#define CH_FLAG_TUN         BIT(2)//配置芯片TUN网络
#define CH_FLAG_GV_PHY_MEM  BIT(3)//外部物理内存模式

#define CH_FLAG_GV_USED     BIT(6)
#define CH_FLAG_HOST_USED   BIT(7)//host



//执行扩展的shell命令
#define CH_FLAG_REMOTE_SHELL  BIT(8)
//清看门狗标志
#define CH_FLAG_CLR_WDT  BIT(4)


//#define epf_func_reg pci_epf_gv95x_bar_reg

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


struct epf_data_transfer{
	int    channel_id;
	int    write_magic_cnt;
	int    read_magic_cnt;
	struct pci_epf 		     *epf;
	struct pci_epf_gv95x_bar_reg  *func_reg;
	struct pci_epf_gv95x      *epf_gv953x;

	//使用外部物理内存缓存
	void  *phy_buffer_in;//用于内部map映射的地址
	void  *phy_buffer_out;
	struct kfifo phy_array_fifo_in;//物理地址fifo有用户态传入
	struct kfifo phy_array_fifo_out;
	
	//缓存地址
	phys_addr_t buffer_phys;
	void  *buffer;
	struct kfifo  data_fifo;
	
	//DMA通道号码
	int dma_ch;
	//DMA状态
	int dma_status;

	//工作队列
	struct workqueue_struct *kpcitest_workqueue;/* 工作队列 */
	struct delayed_work	     cmd_handler;


	wait_queue_head_t pci_epf_waitq;	/* 等待队列头 */
	volatile unsigned int poll_read_flag;
	
	
	struct __DMA{
		phys_addr_t buf_phys;
		void *buf;
	}dma;

	//传输中断请求
	struct completion irq_raised;
	
	struct phy_mem_array phy_mem;

};


struct epf_data_file_private_data{
	struct epf_data_transfer *epf_channel;
	
	//数据传输通道号
	int epf_channel_id;
};


extern int __attribute__((optimize("-O0"))) pci_data_channels_init(int channel_id,struct pci_epf_gv95x *epf_test,struct epf_data_transfer  *epf_channel,size_t size);
extern int __attribute__((optimize("-O0"))) pci_data_channels_exit(struct epf_data_transfer  *epf_channel);
extern int	__attribute__((optimize("-O0"))) pci_data_channels_dma_read(struct epf_data_transfer  *epf_channel,void *vbuf);
extern int __attribute__((optimize("-O0")))pci_data_channels_dma_write(struct epf_data_transfer  *epf_channel,void *vbuf,int vbuf_len);
extern int __attribute__((optimize("-O0")))pci_epf_request_irq(struct device *dev,struct epf_data_transfer *epf_channel);

extern int __attribute__((optimize("-O0")))pci_epf_PubReg_init(struct pci_epf_gv95x *epf_test);
extern int __attribute__((optimize("-O0")))pci_epf_PubReg_exit(struct pci_epf_gv95x *epf_test);


#define DMA_DONE_STATUS_USED        1
#define DMA_DONE_STATUS_SUCCESS     2
#define DMA_DONE_STATUS_ABORT       4
struct epf_dma_done_channel
{
       int chn;
       int status;
       //传输中断请求
       struct completion dma_done;
};

extern void __attribute__((optimize("-O0")))pci_epf_dma_init(void);
extern void __attribute__((optimize("-O0")))pci_epf_dma_exit(void);
extern int __attribute__((optimize("-O0")))pci_data_channels_dma_write_PhyArray(struct epf_data_transfer  *epf_channel,struct phy_mem_array  *ptr_array);

extern struct task_struct *kick_wdt_tsk;
extern volatile char *virt_ck860_wdt_addr;

extern void __attribute__((optimize("-O0"))) shoudown_pulldown_gpio14(void);
extern int __attribute__((optimize("-O0"))) pci_epf_kick_wdt_thread(void* data);


#endif
