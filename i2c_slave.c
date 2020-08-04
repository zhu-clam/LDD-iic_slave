#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>

#define I2C_SLAVE_ID  0

#define GV_I2C_BASE_IRQ 40

#define GV_I2C_BASE_ADDR 0xf7007000
#define GV_I2C_BASE_SIZE 0x1000

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


#define DW_IC_INTR_RX_UNDER	0x001
#define DW_IC_INTR_RX_OVER	0x002
#define DW_IC_INTR_RX_FULL	0x004
#define DW_IC_INTR_TX_OVER	0x008
#define DW_IC_INTR_TX_EMPTY	0x010
#define DW_IC_INTR_RD_REQ	0x020
#define DW_IC_INTR_TX_ABRT	0x040
#define DW_IC_INTR_RX_DONE	0x080
#define DW_IC_INTR_ACTIVITY	0x100
#define DW_IC_INTR_STOP_DET	0x200
#define DW_IC_INTR_START_DET	0x400
#define DW_IC_INTR_GEN_CALL	0x800

#define DW_IC_INTR_DEFAULT_MASK		(DW_IC_INTR_RX_FULL | \
					 DW_IC_INTR_TX_EMPTY | \
					 DW_IC_INTR_TX_ABRT | \
					 DW_IC_INTR_STOP_DET)

struct gv_i2c_slave_dev{
	void __iomem		*base;
	unsigned long base_addr;
	int irq;
	int slave_addr;
	char dev_name[64];
};

#define DRIVER_NAME "gvi2c"

struct gv_i2c_slave_dev gv_i2c_slave;

static inline void i2c_slave_write(struct gv_i2c_slave_dev * pdev, u32 offset, u32 val)
{
	iowrite32(val, pdev->base + offset);
}

static inline u32 i2c_slave_read(struct gv_i2c_slave_dev * pdev, u32 offset)
{
	return ioread32(pdev->base + offset);
}

irqreturn_t gv_i2c_slave_isr(int irq, void *dev_id)
{
	int stat;
	unsigned int value = 0;
	
	struct gv_i2c_slave_dev *pdev = (struct gv_i2c_slave_dev *)dev_id;

	stat  = i2c_slave_read(pdev, DW_IC_RAW_INTR_STAT);
	printk("orig stat:0x%x\n", stat);
	stat = i2c_slave_read(pdev, DW_IC_INTR_MASK);
	printk("stat:0x%x\n", stat);
	if(stat & DW_IC_INTR_RX_FULL)
	{
		value = i2c_slave_read(pdev, DW_IC_DATA_CMD);
	}
	i2c_slave_read(pdev, DW_IC_CLR_INTR);
		
	printk("value:0x%x\n", value);

	return IRQ_HANDLED;
}


int gv_i2c_slave_hw_init(void)
{
	int ret;
	
	struct gv_i2c_slave_dev *pdev = &gv_i2c_slave;

	/*
	* get i2c base addr，这个与芯片的手册的地址偏移
	* get i2c irq
	* get i2c slave addr这个与gpio相关
	*/
	strcpy(pdev->dev_name, "gv_i2c");
	pdev->slave_addr = 0x20;
	pdev->irq = GV_I2C_BASE_IRQ + I2C_SLAVE_ID;
	pdev->base_addr = GV_I2C_BASE_ADDR + GV_I2C_BASE_SIZE * I2C_SLAVE_ID;
	pdev->base = ioremap(pdev->base_addr, GV_I2C_BASE_SIZE);
	
	ret = request_threaded_irq(pdev->irq, gv_i2c_slave_isr, NULL, IRQF_SHARED, pdev->dev_name,
				  (void *)pdev);


	//disable ic
	i2c_slave_write(pdev, DW_IC_ENABLE, 0);
	msleep(10);
	i2c_slave_write(pdev, DW_IC_RX_TL, 1);
	i2c_slave_write(pdev, DW_IC_TX_TL, 1);
	i2c_slave_write(pdev, DW_IC_CON, 0);
	i2c_slave_write(pdev, DW_IC_INTR_MASK, 0);
	i2c_slave_write(pdev, DW_IC_INTR_MASK, DW_IC_INTR_RX_FULL | DW_IC_INTR_RD_REQ);
	i2c_slave_write(pdev, DW_IC_SAR, pdev->slave_addr);

	i2c_slave_write(pdev, DW_IC_ENABLE, 1);

	i2c_slave_read(pdev, DW_IC_CLR_INTR);

	return 0;
}

void gv_i2c_slave_hw_exit(void)
{
	struct gv_i2c_slave_dev *pdev = &gv_i2c_slave;

	free_irq(pdev->irq, pdev);
	iounmap(pdev->base);
}


static struct file_operations gv_i2c_slave_fops = {
    .owner        = THIS_MODULE,
    .llseek        = no_llseek,
};

static struct miscdevice gv_i2c_slave_miscdev = {
    .minor        = MISC_DYNAMIC_MINOR,
    .name        = DRIVER_NAME,
    .fops        = &gv_i2c_slave_fops,
};

static int __init i2c_slave_init(void)
{
    int ret = 0;

	ret = gv_i2c_slave_hw_init();
	 if(ret) {
        printk (KERN_ERR "Failed to init i2c slave hw\n");
        return ret;
    }
	 
    ret = misc_register(&gv_i2c_slave_miscdev);
    if(ret) {
        printk (KERN_ERR "cannot register miscdev (err=%d)\n", ret);
        return ret;
    }
    return 0;
}

static void __exit i2c_slave_exit(void)
{   
	gv_i2c_slave_hw_exit();
    misc_deregister(&gv_i2c_slave_miscdev);
}

module_init(i2c_slave_init);
module_exit(i2c_slave_exit);

MODULE_AUTHOR("tao.wang@byavs.com");
MODULE_DESCRIPTION("Byavs i2c slave driver");

