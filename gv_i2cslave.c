#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/workqueue.h>
#include <linux/semaphore.h>



#define DW_IC_CON		0x0
#define DW_IC_TAR		0x4
#define DW_IC_SAR		0x8
#define DW_IC_DATA_CMD		0x10
#define DW_IC_SS_SCL_HCNT	0x14
#define DW_IC_SS_SCL_LCNT	0x18
#define DW_IC_FS_SCL_HCNT	0x1c
#define DW_IC_FS_SCL_LCNT	0x20
#define DW_IC_HS_SCL_HCNT	0x24
#define DW_IC_HS_SCL_LCNT	0x28
#define DW_IC_INTR_STAT		0x2c
#define DW_IC_INTR_MASK		0x30
#define DW_IC_RAW_INTR_STAT	0x34
#define DW_IC_RX_TL		0x38
#define DW_IC_TX_TL		0x3c
#define DW_IC_CLR_INTR		0x40
#define DW_IC_CLR_RX_UNDER	0x44
#define DW_IC_CLR_RX_OVER	0x48
#define DW_IC_CLR_TX_OVER	0x4c
#define DW_IC_CLR_RD_REQ	0x50
#define DW_IC_CLR_TX_ABRT	0x54
#define DW_IC_CLR_RX_DONE	0x58
#define DW_IC_CLR_ACTIVITY	0x5c
#define DW_IC_CLR_STOP_DET	0x60
#define DW_IC_CLR_START_DET	0x64
#define DW_IC_CLR_GEN_CALL	0x68
#define DW_IC_ENABLE		0x6c
#define DW_IC_STATUS		0x70
#define DW_IC_TXFLR		0x74
#define DW_IC_RXFLR		0x78
#define DW_IC_SDA_HOLD		0x7c
#define DW_IC_TX_ABRT_SOURCE	0x80
#define DW_IC_ENABLE_STATUS	0x9c
#define DW_IC_COMP_PARAM_1	0xf4
#define DW_IC_COMP_VERSION	0xf8
#define DW_IC_SDA_HOLD_MIN_VERS	0x3131312A
#define DW_IC_COMP_TYPE		0xfc
#define DW_IC_COMP_TYPE_VALUE	0x44570140


/* i2c interrupt status register definitions */
#define IC_MST_ON_HOLD  0x2000
#define IC_GEN_CALL		0x0800
#define IC_START_DET	0x0400
#define IC_STOP_DET		0x0200
#define IC_ACTIVITY		0x0100
#define IC_RX_DONE		0x0080
#define IC_TX_ABRT		0x0040
#define IC_RD_REQ		0x0020
#define IC_TX_EMPTY		0x0010
#define IC_TX_OVER		0x0008
#define IC_RX_FULL		0x0004
#define IC_RX_OVER 		0x0002
#define IC_RX_UNDER		0x0001

/* i2c enable register definitions */
#define IC_ENABLE_ABORT     0x0002
#define IC_ENABLE_0B		0x0001


#define  DEBUG
#ifdef   DEBUG
#define print_debug printk
#else
static inline void print_debug(const char *fmt, ...)
{
	(void)fmt;
}
#endif


struct i2c_slave_dev{
    void  *base;
    int	   irq;
	struct class *pClass;
	struct device *pdev;
	struct cdev  *pcdev;
	dev_t   devID;
};

struct i2c_slave_dev *dev;



#define read_mreg32( addr )				*(volatile unsigned int *)(addr)
#define write_mreg32( addr, val)		*(volatile unsigned int *)(addr)= (volatile unsigned int)(val)

static long gvsp_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return 0;
}
static int gvsp_open(struct inode *inode, struct file *filp)
{
	print_debug("gvsp OPEN!  %ld  inode %p  filp %p\n",atomic_long_read(&filp->f_count),inode,filp);
	return 0;
}
static int gvsp_release(struct inode *inode, struct file *file)
{
	print_debug("gvsp release\n");
	return 0;
}
static ssize_t gvsp_read (struct file *file, char *buff, size_t count, loff_t *offp)
{
	
	return 0;
}
ssize_t gvsp_write (struct file *file, const char *buf, size_t count, loff_t *f_pos)
{
	
	return 0;
}

 struct file_operations dev_fops = {
	.owner 				= THIS_MODULE,
	.open               = gvsp_open,
	.read               = gvsp_read,
	.write              = gvsp_write,
	.release            = gvsp_release,
	.unlocked_ioctl     = gvsp_unlocked_ioctl,
	
};
static irqreturn_t interrupt_function(int irq, void *arg)
{
	int tmp;
	struct i2c_slave_dev *dev = (struct i2c_slave_dev *)arg;
	tmp = read_mreg32(dev->base+DW_IC_INTR_STAT);
    if ((tmp & DW_IC_CLR_RD_REQ) != 0) {
        print_debug("send out data per read request from master\n");
      
    } else if ((tmp & IC_RX_FULL) != 0) {
        print_debug("receive data for write from master\n");
       
    }
	return IRQ_HANDLED;
}



static int gvsp_probe(struct platform_device *pdev)
{
	int ret;
	int reg_val;
	print_debug("%s %d\n",__FUNCTION__,__LINE__);
	struct resource *mem;
	dev = devm_kzalloc(&pdev->dev, sizeof(struct i2c_slave_dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;
	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	dev->base = devm_ioremap_resource(&pdev->dev, mem);
	if (IS_ERR(dev->base))
		return PTR_ERR(dev->base);
	reg_val = read_mreg32(dev->base +DW_IC_ENABLE);
	reg_val &= ~IC_ENABLE_0B;
    while ((read_mreg32(dev->base +DW_IC_ENABLE +DW_IC_STATUS)&IC_ENABLE_0B) != 0);
	write_mreg32(dev->base+DW_IC_RX_TL,0x7);
	write_mreg32(dev->base+DW_IC_RX_TL,0x0);
	write_mreg32(dev->base+DW_IC_CON,0x0006);
	write_mreg32(dev->base+DW_IC_INTR_MASK,0x0);
	write_mreg32(dev->base+DW_IC_SAR,0xb);   // slave addr  
	
	write_mreg32(dev->base+DW_IC_ENABLE,0x1);   // enable i2c  
	
	dev->irq = platform_get_irq(pdev, 0);
	if (dev->irq < 0)
		return dev->irq;
	
	devm_request_irq(&pdev->dev,dev->irq,interrupt_function,
			     IRQF_SHARED | IRQF_COND_SUSPEND,
			     "i2c_slave", dev);
	dev->pClass = class_create(THIS_MODULE, "i2c_slave");
	ret = alloc_chrdev_region(&dev->devID, 0, 1, "i2c_slave");
	
	if(ret)
	{
		print_debug(KERN_ALERT"fail to allocate chrdev.\n");
		return ret;
	}
	/*--2--*/
	dev->pcdev = cdev_alloc();
	if(dev->pcdev == NULL)
	{
		print_debug(KERN_ALERT"gvsp_cdev alloc fail.\n");  
		goto error;
	}
		
	/*--3--*/
	cdev_init(dev->pcdev, &dev_fops);
	dev->pcdev->owner = THIS_MODULE;
		
	/*--4--*/
	ret = cdev_add(dev->pcdev,dev->devID, 1);
	if(ret)
	{
		print_debug("fail to cdev_add core_id \n");
		goto error;
	} 
	dev->pdev = device_create( dev->pClass, NULL,dev->devID, NULL,"i2c_slave");
	if (IS_ERR(dev->pdev))
	{
		print_debug("device create fail\n");
		goto error;
	}
	return 0;
error:
	
	if (dev->pdev)
	{
		device_destroy(dev->pClass,dev->devID);
	}
	
	if (dev->pClass)
    {
		class_destroy(dev->pClass);
	}
	
	if(dev->pcdev)
	{
	    cdev_del(dev->pcdev);
	}
	if(dev->devID)
	{
			unregister_chrdev_region(dev->devID, 1);
	}
	return -1; 
}

static int gvsp_remove(struct platform_device *pdev)
{
	if (dev->pdev)
	{
		device_destroy(dev->pClass,dev->devID);
	}
	
	if (dev->pClass)
    {
		class_destroy(dev->pClass);
	}
	
	if(dev->pcdev)
	{
	    cdev_del(dev->pcdev);	
	}
	if(dev->devID)
	{
			unregister_chrdev_region(dev->devID, 1);
	}
	return 0;
}

static const struct of_device_id gvsp_match[] = {
	{ .compatible = "byavs,i2c_slave",},
	{},
};
MODULE_DEVICE_TABLE(of, gvsp_match);

static struct platform_driver gvsp_drv = {
	.probe = gvsp_probe,
	.remove = gvsp_remove,
	.driver = {
		.name = "byavs,i2c",
		.of_match_table = gvsp_match,
	},
	
};

static int __init gvsp_init(void)
{
	print_debug("%s %d\n",__FUNCTION__,__LINE__);
	return platform_driver_register(&gvsp_drv);
}

static void __exit gvsp_exit(void)
{
	print_debug("%s %d\n",__FUNCTION__,__LINE__);
	platform_driver_unregister(&gvsp_drv);
} 
module_init(gvsp_init);  
module_exit(gvsp_exit);  
  
MODULE_LICENSE("Dual BSD/GPL");  
MODULE_AUTHOR("chengyouliang");  

