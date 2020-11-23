#ifndef __PCI_EPF_GV953X_H
#define __PCI_EPF_GV953X_H

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
#include <linux/kfifo.h>
#include <linux/kthread.h>   



#pragma pack (4)
typedef	struct
{
	u8 ip_v4[4];
	u8 mask_v4[4];
	u8 gw_v4[4];
	u8 dns_v4[4];
}__GV_TUN;
#pragma pack ()


typedef struct
{//通道公用的信息
	u8  reserve[0x105c];
	u32 gv_sw_int_reg;
}__EPF_PUB_REG;

struct pci_epf_gv95x {
	void			*reg[6];
	struct pci_epf	*epf;
	enum pci_barno	test_reg_bar;
	bool			linkup_notifier;
	bool			msix_available;

	//dma通道队列
	struct kfifo dma_r_ch_fifo;
	struct kfifo dma_w_ch_fifo;
   __EPF_PUB_REG            *pub_reg;
};



#define PCI_DRV_VERSION (506)


struct pci_epf_gv95x_bar_reg {
	u32	magic;
	u32	command;
	u32	status;
	u64	src_addr;
	u64	dst_addr;
	u32	size;
	u32	checksum;
	u32	irq_type;
	u32	irq_number;
	u32 Flag;//48
	u32 phy_index;//52
	u32 phy_len;//56
	u32 hostbuf_size;
	u32 hostbuf_rdptr;
	u32 hostbuf_wrptr;
	u32 phy_mem_size;
	u32 pci_version;//76

	
   __GV_TUN gv_tun;
   
} __packed;



#define IRQ_TYPE_LEGACY			0
#define IRQ_TYPE_MSI			1
#define IRQ_TYPE_MSIX			2

#define COMMAND_RAISE_LEGACY_IRQ	BIT(0)
#define COMMAND_RAISE_MSI_IRQ		BIT(1)
#define COMMAND_RAISE_MSIX_IRQ		BIT(2)
#define COMMAND_READ			BIT(3)
#define COMMAND_WRITE			BIT(4)
#define COMMAND_COPY			BIT(5)
#define COMMAND_DMA_READ        BIT(6)
#define COMMAND_DMA_WRITE       BIT(7)


//启动网络
#define COMMAND_START_TUNDEV		BIT(8)
//复位GV9531
#define COMMAND_RESET_GV9531		BIT(9)

//shell
#define COMMAND_REMOTE_SHELL        BIT(10)

//start_wdt
#define COMMAND_START_WDT		BIT(11)
//stop_wdt
#define COMMAND_STOP_WDT		BIT(12)




#define STATUS_READ_SUCCESS		BIT(0)
#define STATUS_READ_FAIL		BIT(1)
#define STATUS_WRITE_SUCCESS	BIT(2)
#define STATUS_WRITE_FAIL		BIT(3)

#define STATUS_COPY_SUCCESS		    BIT(4)
#define STATUS_COPY_FAIL		    BIT(5)
#define STATUS_IRQ_RAISED		    BIT(6)
#define STATUS_SRC_ADDR_INVALID		BIT(7)

#define STATUS_DST_ADDR_INVALID		BIT(8)
#define STATUS_KFIFO_FULL			BIT(9)

#define TIMER_RESOLUTION		1

#define PCI_EPF_FUNC_MEM_SIZE (256 * 1024ULL)
#define PCI_EPF_FUNC_MEM_READ_SIZE (128 * 1024ULL)
#define IOC_MAGIC 			'B'


//WDT reg list
#define WDT_CR		0x0
#define WDT_TORR	0x4
#define WDT_CCVR	0x8
#define WDT_CRR		0xc


//触发通知
#define PCI_EPF_FUNC_IOC_NOTIFY			_IO(IOC_MAGIC, 0)

//读取大小
#define PCI_EPF_FUNC_IOC_READ_SIZE		_IO(IOC_MAGIC, 1)
//释放当前读取内存
#define PCI_EPF_FUNC_IOC_FREE_READ		_IO(IOC_MAGIC, 2)

//可写入状态
#define PCI_EPF_FUNC_IOC_WRITE_SIZE		_IO(IOC_MAGIC, 3)


//触发通知
#define PCI_EPF_FUNC_IOC_WRITE_NOTIFY	_IO(IOC_MAGIC, 4)

#define PCI_EPF_FUNC_IOC_SET_CHANNEL    _IO(IOC_MAGIC, 5)
#define PCI_EPF_FUNC_IOC_CHANNEL_START  _IO(IOC_MAGIC, 6)
#define PCI_EPF_FUNC_IOC_GET_TUN_IP     _IO(IOC_MAGIC, 7)


//远程shell命令
#define PCI_EPF_FUNC_IOC_CMD_SHELL      _IO(IOC_MAGIC, 0x8)


#define PCI_EPF_SET_PHY_ARRAY            _IO(IOC_MAGIC,0x9)
#define PCI_EPF_GET_PHY_ARRAY            _IO(IOC_MAGIC,0xa)
#define PCI_EPF_CLR_PHY_ARRAY            _IO(IOC_MAGIC,0xb)
#define PCI_EPF_PHY_MEM_WRITE_DMA        _IO(IOC_MAGIC,0xc)
#define PCI_EPF_GET_FLAG                 _IO(IOC_MAGIC,0xd)

#define PCI_EPF_EN_WDT         			 _IO(IOC_MAGIC,0xe)
#define PCI_EPF_GPIO14         			 _IO(IOC_MAGIC,0xf)
#define PCI_EPF_START_KICK_WDT         	 _IO(IOC_MAGIC,0x10)
#define PCI_EPF_STOP_KICK_WDT         	 _IO(IOC_MAGIC,0x11)


#endif

