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

#include <linux/uaccess.h>

#include <linux/kfifo.h>
#include <linux/err.h>
#include <linux/vmalloc.h>

#include <linux/sched.h>
#include <linux/kthread.h>

#include "pci_endpoint_test.h"
#include "pci_data_channel.h"

#include <asm/cacheflush.h>

u32 pci_endpoint_readl(void __iomem *reg_base)
{
	return readl(reg_base);
}

void pci_endpoint_writel(void __iomem *reg_base,u32 value)
{
	writel(value, reg_base);
}



//数据传输通道初始化
int pci_data_channels_init(int channel_id,struct  pci_endpoint_test *pci_host,struct pci_epf_data_transfer  *channel,size_t size,unsigned int irq_number)
{
	struct pci_dev *pdev = pci_host->pdev;
	struct device *dev = &pdev->dev;
	
	//dma_addr_t orig_phys_addr;
	size_t offset;
	
	//设置通道寄存器地址
	//channel->pub_reg=pci_host->base;
	channel->pub_reg=pci_host->bar[4];	//ioremap(0xf9700000,bar_size[bar]);
	channel->ep_reg=sizeof(struct pci_epf_func_reg)*channel_id+pci_host->base;//+sizeof(__EPF_PUB_REG);
	//EPF_NUM_DATA_CHANNELS*sizeof(struct pci_epf_gv95x_bar_reg);
	channel->channel_id=channel_id;
	printk("channel: %d.ep_reg=%p\n",channel_id,channel->ep_reg);
	printk("channel: %d.alignment=%lx\n",channel_id,pci_host->alignment);


#if 1	 //移动到打开通道时,申请内存.PCI DMA 空间.
	/* 申请内核一致性内存 */
	channel->dma.orig_addr = dma_alloc_coherent(dev, size + pci_host->alignment, &channel->dma.orig_phys_addr,GFP_KERNEL);
	if (!channel->dma.orig_addr) {
		dev_err(dev, "Failed to allocate address\n");	
		return -1;
	}

	if (pci_host->alignment && !IS_ALIGNED(channel->dma.orig_phys_addr, pci_host->alignment)) {
		channel->dma.phys_addr =  PTR_ALIGN(channel->dma.orig_phys_addr, pci_host->alignment);
		offset =channel->dma.phys_addr - channel->dma.orig_phys_addr;
		channel->dma.addr = channel->dma.orig_addr + offset;
	} else {
		channel->dma.phys_addr = channel->dma.orig_phys_addr;
		channel->dma.addr = channel->dma.orig_addr;
	}

	channel->dma.alignment=pci_host->alignment;
	channel->dma.size=size;
#endif

	//设置中断
	channel->ep_reg->irq_type=IRQ_TYPE_MSI;
	channel->ep_reg->irq_number=irq_number;
	init_completion(&channel->irq_raised);
		
	//申请一个kefifo
	if(channel_id<NUM_READ_CHANNELS)
	{//读取通道

	#if 0 //读通道的kfifo 移动的打开读通道时创建.
		channel->buffer=vmalloc(0x100000*4);
		printk("channel: %d.buffer=%x\n",channel_id,channel->buffer);
		
		Ret=kfifo_init(&channel->data_fifo,channel->buffer,0x100000*4);
		printk("channel: %d.Ret=%d\n",channel_id,Ret);
		if (Ret<0)
			goto Err_free_kfifo1;
	#endif

		#if 0
		//创建读取的内核线程
		channel->host_read_tsk=kthread_run(pci_data_channels_read_thread,channel,"pci_data_channels_read_thread");
		if (IS_ERR(channel->host_read_tsk))
		{
	        printk(KERN_INFO "first create kthread failed!\n"); 
	        goto Err_free_kfifo;
	    }  
	
	    else {  
	        printk(KERN_INFO "first create ktrhead ok!\n");  
	    }
		#else
		//EP可吸入的目标地址
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->dst_addr),lower_32_bits(channel->dma.phys_addr));
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->dst_addr)+4,upper_32_bits(channel->dma.phys_addr));//搬运数据的实际物理地址
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->hostbuf_size), size);
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->hostbuf_rdptr), 0);
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->hostbuf_wrptr), 0);

		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->irq_type), channel->ep_reg->irq_type);
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->irq_number),channel->ep_reg->irq_number);	

		#endif
		

	}
	else
	{//写入通道
		
	}
	
	channel->poll_read_flag=0;
	printk("ch%d.init_waitqueue_head...\n",channel_id);
	init_waitqueue_head(&channel->pci_epf_waitq);	
	
	return 0;

}


int pci_data_channels_exit(struct device *dev,struct pci_epf_data_transfer  *channel)
{
//	struct task_struct *task = (struct task_struct *)channel->host_read_tsk;
//	struct pci_dev *pdev = pci_host->pdev;

#if 0
		if(task != NULL)
		{
			kthread_stop(task);
			task = NULL;
		}
#endif

#if 1
	if(NULL !=channel->dma.orig_addr )
	dma_free_coherent(dev, channel->dma.size + channel->dma.alignment, channel->dma.orig_addr, channel->dma.orig_phys_addr);

//	if(NULL != &channel->data_fifo)
//		kfifo_free(&channel->data_fifo);

//	if (NULL!=channel->buffer)
//		vfree(channel->buffer);

#endif
	
	return 0;
	
}

void pci_epf_irq_trigger(struct pci_epf_data_transfer  *channel,u32 reg_value)
{
    u32 reg_val;

	//while(1)
	//{
		reg_val=pci_endpoint_readl((void __iomem *)(&channel->pub_reg->gv_sw_int_reg));
	//	if(reg_val == 0)
	//		break;
	//}
	reg_val |= reg_value;
    pci_endpoint_writel((void __iomem *)(&channel->pub_reg->gv_sw_int_reg),reg_val);

}


u32 pci_get_chip_id(struct pci_epf_data_transfer  *channel)
{
    u32 reg_val;

	reg_val=pci_endpoint_readl((void __iomem *)(&channel->pub_reg->gv_chip_id));
	return reg_val;
}

#define GV_CHIP_ID (0x20190101) 



#if 0
//有中断唤醒读取数据，并将输入存入kfifo
int pci_data_channels_read_thread(void *data)
{
	int ret;
	struct pci_epf_data_transfer  *channel=(struct pci_epf_data_transfer *)data;
	
	
	printk("read_thread: channel_id=%d\n",channel->channel_id);
	pci_endpoint_writel((void __iomem *)(&channel->ep_reg->status),0);

	while(1)
	{
		set_current_state(TASK_UNINTERRUPTIBLE);//将当前的状态表示设置为休眠
		if(kthread_should_stop()) break;  //

		u32 crc32;
		u32 size;
		
		//EP可写入的目标地址
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->dst_addr),lower_32_bits(channel->dma.phys_addr));
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->dst_addr)+4,upper_32_bits(channel->dma.phys_addr));//搬运数据的实际物理地址
	

		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->irq_type), channel->ep_reg->irq_type);
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->irq_number),channel->ep_reg->irq_number);
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->command),COMMAND_DMA_WRITE);//会影响调试程序不停的写入数据

#ifdef ORIGIN_CODE
		//等待中断
		wait_for_completion(&channel->irq_raised);
		
#else
	
	//polling STATUS_IRQ_RAISED
	int timeout = 0;
	while((pci_endpoint_readl((void __iomem *)(&channel->ep_reg->status)) & STATUS_WRITE_SUCCESS) == 0)
	{
		if (timeout<5000)
		{
			usleep_range(100,101);
			timeout++;
		}
		else
			msleep(1);
	}

	u32 reg=pci_endpoint_readl((void __iomem *)(&channel->ep_reg->status));
	reg &= ~STATUS_WRITE_SUCCESS;
	pci_endpoint_writel((void __iomem *)(&channel->ep_reg->status),reg);

#endif
		//读取大小
		size=pci_endpoint_readl((void __iomem *)(&channel->ep_reg->size));

		#if defined(__x86_64__) || defined(__i386__)
		//刷新cache
		clflush_cache_range(channel->dma.addr,size);
		#endif

		ret=pci_endpoint_readl((void __iomem *)(&channel->ep_reg->status));
		if(ret&STATUS_WRITE_FAIL)
		{//读取失败
			printk("gv9531-write-fail:write_len=%d\n",channel->ep_reg->size);
			goto ERR_1;
		}

	   //读取数据,正确
	   if (kfifo_avail(&channel->data_fifo)>size)
	   {
	   		kfifo_in(&channel->data_fifo,channel->dma.addr,size);
	   		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->status),STATUS_IRQ_RAISED|STATUS_READ_SUCCESS);
	   }
	   else
	   {
	   		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->status),STATUS_IRQ_RAISED|STATUS_KFIFO_FULL);
	   		printk("Err-kfifo-overflow: channel->channel_id=%x\n",channel->channel_id);
	   }
		
		//清除通道，禁止写入
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->size),0);
		pci_epf_irq_trigger(channel,1);//83
    	channel->poll_read_flag = 1;
    	wake_up_interruptible(&channel->pci_epf_waitq);		

	}
	//}while(!kthread_should_stop());

	printk("##pci_data_channels_read_thread  thread exit\n");
	return 0;
ERR_1:
	pci_endpoint_writel((void __iomem *)(&channel->ep_reg->size),0);
	printk("##pci_data_channels_read_thread  thread exit\n");
	return -1;
}
#endif
