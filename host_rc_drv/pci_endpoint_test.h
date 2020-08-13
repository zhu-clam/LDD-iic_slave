#ifndef 	__PCI_ENDPOINT_TEST_H
#define 	__PCI_ENDPOINT_TEST_H

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
#include <linux/version.h>

#include <linux/init.h>
#include <linux/uaccess.h>


#define PCI_IRQ_LEGACY		(1 << 0) /* Allow legacy interrupts */
#define PCI_IRQ_MSI			(1 << 1) /* Allow MSI interrupts */
#define PCI_IRQ_MSIX		(1 << 2) /* Allow MSI-X interrupts */


#define PCITEST_BAR             _IO('P', 0x1)
#define PCITEST_LEGACY_IRQ      _IO('P', 0x2)
#define PCITEST_MSI             _IOW('P', 0x3, int)
#define PCITEST_WRITE           _IOW('P', 0x4, unsigned long)
#define PCITEST_READ            _IOW('P', 0x5, unsigned long)
#define PCITEST_COPY            _IOW('P', 0x6, unsigned long)
#define PCITEST_MSIX            _IOW('P', 0x7, int)
#define PCITEST_SET_IRQTYPE     _IOW('P', 0x8, int)
#define PCITEST_GET_IRQTYPE     _IO('P', 0x9)
#define PCITEST_DMA_WRITE       _IOW('P', 0xa, unsigned long)
#define PCITEST_DMA_READ        _IOW('P', 0xb, unsigned long)


#define DRV_MODULE_NAME				"pci-endpoint-test"

#define IRQ_TYPE_UNDEFINED			-1
#define IRQ_TYPE_LEGACY				0
#define IRQ_TYPE_MSI				1
#define IRQ_TYPE_MSIX				2

#define PCI_ENDPOINT_TEST_MAGIC			0x0

#define PCI_ENDPOINT_TEST_COMMAND		0x4
#define COMMAND_RAISE_LEGACY_IRQ		BIT(0)
#define COMMAND_RAISE_MSI_IRQ			BIT(1)
#define COMMAND_RAISE_MSIX_IRQ			BIT(2)
#define COMMAND_READ				BIT(3)
#define COMMAND_WRITE				BIT(4)
#define COMMAND_COPY				BIT(5)
#define COMMAND_DMA_READ			BIT(6)
#define COMMAND_DMA_WRITE			BIT(7)

//启动网络
#define COMMAND_START_TUNDEV		BIT(8)
//复位GV9531
#define COMMAND_RESET_GV9531		BIT(9)


#define PCI_ENDPOINT_TEST_STATUS		0x8
#define STATUS_READ_SUCCESS			BIT(0)
#define STATUS_READ_FAIL			BIT(1)
#define STATUS_WRITE_SUCCESS		BIT(2)
#define STATUS_WRITE_FAIL			BIT(3)
#define STATUS_COPY_SUCCESS			BIT(4)
#define STATUS_COPY_FAIL			BIT(5)
#define STATUS_IRQ_RAISED			BIT(6)
#define STATUS_SRC_ADDR_INVALID		BIT(7)
#define STATUS_DST_ADDR_INVALID		BIT(8)
#define STATUS_KFIFO_FULL			BIT(9)


//static DEFINE_IDA(pci_endpoint_test_ida);

#define to_endpoint_test(priv) container_of((priv), struct pci_endpoint_test, \
					    miscdev)



enum pci_barno {
	BAR_0,
	BAR_1,
	BAR_2,
	BAR_3,
	BAR_4,
	BAR_5,
};

struct pci_endpoint_test {
	struct pci_dev	*pdev;
	void __iomem	*base;
	void __iomem	*bar[6];
	struct completion irq_raised;
	int		last_irq;
	int		num_irqs;
	/* mutex to protect the ioctls */
	struct mutex	mutex;
	struct miscdevice miscdev;
	enum pci_barno test_reg_bar;
	size_t alignment;
	
	void *data_channel;
};

struct pci_endpoint_test_data {
	enum pci_barno test_reg_bar;
	size_t alignment;
	int irq_type;
};




//DMA分配内存大小
#define DMA_MEM_SIZE                   (2*1024*1024)    //2M


#define IOCTL_SET_CHANNEL              _IO('I', 0x1)
#define IOCTL_GET_CHANNEL              _IO('I', 0x2)

#define IOCTL_SET_TUN                  _IO('I', 0x3)

//远程shell命令
#define IOCTL_SET_SHELL                _IO('I', 0x4)
#define IOCTL_CLR_SHELL                _IO('I', 0x5)

#define IOCTL_GET_FLAG                 _IO('I', 0x6)

#define IOCTL_WDT_CLR                   _IO('I', 0x7)
#define IOCTL_CHIP_ID                   _IO('I', 0x8)
#define IOCTL_PHY_MEM_WRITE             _IO('I', 0x9)
#define GV_CHIP_ID (0x20190101)

struct pci_endpoint_file_private_data{
	struct pci_epf_data_transfer *data_channel;
	
	//数据传输通道号
	int data_channel_id;
};


#endif
