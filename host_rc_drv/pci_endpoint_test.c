#include "pci_endpoint_test.h"
#include "pci_data_channel.h"
#include "pci_channel_proc.h"
#include <linux/version.h>
#include <asm/cacheflush.h>

static bool no_msi;
module_param(no_msi, bool, 0444);
MODULE_PARM_DESC(no_msi, "Disable MSI interrupt in pci_endpoint_test");

static int irq_type = IRQ_TYPE_MSI;
module_param(irq_type, int, 0444);
MODULE_PARM_DESC(irq_type, "IRQ mode selection in pci_endpoint_test (0 - Legacy, 1 - MSI, 2 - MSI-X)");

//数据传输通道定义
//struct pci_epf_data_transfer data_channel[NUM_DATA_CHANNELS];

//host 端创建喂狗线程
//static struct task_struct *host_kick_wdt_tsk = NULL;
/*
unsigned int dmaMemSize[NUM_DATA_CHANNELS] = {64*1024, 3*1024*1024, 64*1024, 64*1024, 64*1024,
                                              64*1024, 64*1024, 64*1024, 64*1024, 64*1024, 64*1024, 64*1024, 64*1024, 64*1024,  //read end 
											  64*1024, 64*1024, 4*1024*1024, 64*1024, 64*1024, 64*1024,
											  64*1024, 64*1024, 64*1024, 64*1024, 64*1024,
											  64*1024, 64*1024, 64*1024};
*/
unsigned int dmaMemSize[NUM_DATA_CHANNELS] = { 1*1024*1024, 1*1024*1024,  1*1024*1024, 1*1024*1024,  1*1024*1024,
                                               1*1024*1024,  1*1024*1024,  1*1024*1024,1*1024*1024,  1*1024*1024,  1*1024*1024,  1*1024*1024,  1*1024*1024, 1*1024*1024,  //read end 
											   1*1024*1024,  1*1024*1024, 1*1024*1024,  1*1024*1024,  1*1024*1024, 1*1024*1024,
											   1*1024*1024, 1*1024*1024, 1*1024*1024, 1*1024*1024, 1*1024*1024,
											 1*1024*1024, 1*1024*1024, 1*1024*1024};


int pci_endpoint_alloc_irq_vectors(struct pci_dev *dev, unsigned int min_vecs,
        unsigned int max_vecs, unsigned int flags)
{
    int vecs = -ENOSPC;

    if (flags & PCI_IRQ_MSIX) {
        vecs = pci_enable_msix_range(dev, NULL, min_vecs, max_vecs);
        if (vecs > 0)
            return vecs;
    }

    if (flags & PCI_IRQ_MSI) {
        #if LINUX_VERSION_CODE <= KERNEL_VERSION(4,11,0)
		vecs = pci_enable_msi_range(dev, min_vecs, max_vecs);
		#else
		vecs = pci_alloc_irq_vectors(dev, min_vecs, max_vecs,PCI_IRQ_MSI);
		#endif
		if (vecs > 0)
            return vecs;
    }

    /* use legacy irq if allowed */
    if ((flags & PCI_IRQ_LEGACY) && min_vecs == 1) {
        pci_intx(dev, 1);
        return 1;
    }

    return vecs;
}

void pci_free_irq_vectors(struct pci_dev *dev)
{
    pci_disable_msix(dev);
    pci_disable_msi(dev);
}

int pci_irq_vector(struct pci_dev *dev, unsigned int nr)
{
	if (dev->msix_enabled) {
		struct msi_desc *entry;
		int i = 0;

		for_each_pci_msi_entry(entry, dev) {
			if (i == nr)
				return entry->irq;
			i++;
		}
		WARN_ON_ONCE(1);
		return -EINVAL;
	}

	if (dev->msi_enabled) {
		struct msi_desc *entry = first_pci_msi_entry(dev);

		if (WARN_ON_ONCE(nr >= entry->nvec_used))
			return -EINVAL;
	} else {
		if (WARN_ON_ONCE(nr > 0))
			return -EINVAL;
	}

	return dev->irq + nr;
}


static irqreturn_t pci_endpoint_test_irqhandler(int irq, void *dev_id)
{
	struct pci_endpoint_test *test = dev_id;
	struct pci_epf_data_transfer *data_channel=test->data_channel;
	u32 reg;

	int i;
	
//	printk("irqhandler: irq=%d\n",irq);
#ifdef ORIGIN_CODE	
	for (i = 0; i < NUM_DATA_CHANNELS; i++)
	{
		//if (data_channel[i].ep_reg->irq_number==irq)
		{
			reg=pci_endpoint_readl((void __iomem *)(&data_channel[i].ep_reg->status));
			if (reg & STATUS_IRQ_RAISED){
//printk("__%s,__%d,data_channel[%d].ep_reg->status=%x\n",__FILE__,__LINE__,i,reg);
				reg &= ~STATUS_IRQ_RAISED;
				pci_endpoint_writel((void __iomem *)(&data_channel[i].ep_reg->status),reg);
				
				data_channel[i].interrupts++;			
				complete(&data_channel[i].irq_raised);/* complete()同步机制;唤醒被wait_for_complete()的线程*/
			}
		}
	}
#endif
	for (i = 0; i < NUM_WRITE_CHANNELS; i++)
	{
		{
			reg=pci_endpoint_readl((void __iomem *)(&data_channel[i].ep_reg->status));
//printk("__%s,__%d,data_channel[%d].ep_reg->status=%x\n",__FILE__,__LINE__,i,reg);
			if (reg & STATUS_WRITE_SUCCESS){
//printk("__%s,__%d,data_channel[%d].ep_reg->status=%x\n",__FILE__,__LINE__,i,reg);
					reg &= ~STATUS_WRITE_SUCCESS;
					pci_endpoint_writel((void __iomem *)(&data_channel[i].ep_reg->status),reg);
					data_channel[i].poll_read_flag = 1;
					wake_up_interruptible(&data_channel[i].pci_epf_waitq);
			}
		}
	}

	return IRQ_HANDLED;
}

static void pci_endpoint_test_free_irq_vectors(struct pci_endpoint_test *test)
{
	struct pci_dev *pdev = test->pdev;

	pci_free_irq_vectors(pdev);
}

static bool pci_endpoint_test_alloc_irq_vectors(struct pci_endpoint_test *test,
						int type)
{
	int irq = -1;
	struct pci_dev *pdev = test->pdev;
	struct device *dev = &pdev->dev;
	bool res = true;

	switch (type) {
	case IRQ_TYPE_LEGACY:
		irq = pci_endpoint_alloc_irq_vectors(pdev, 1, 1, PCI_IRQ_LEGACY);
		if (irq < 0)
			dev_err(dev, "Failed to get Legacy interrupt\n");
		break;
	case IRQ_TYPE_MSI:
		irq = pci_endpoint_alloc_irq_vectors(pdev, 1, 32, PCI_IRQ_MSI);
		if (irq < 0)
			dev_err(dev, "Failed to get MSI interrupts\n");
		break;
	case IRQ_TYPE_MSIX:
		irq = pci_endpoint_alloc_irq_vectors(pdev, 1, 2048, PCI_IRQ_MSIX);
		if (irq < 0)
			dev_err(dev, "Failed to get MSI-X interrupts\n");
		break;
	default:
		dev_err(dev, "Invalid IRQ type selected\n");
	}

	if (irq < 0) {
		irq = 0;
		res = false;
	}
	test->num_irqs = irq;
printk("__%s,%d: num_irqs=%d\n",__FUNCTION__,__LINE__,test->num_irqs);
	return res;
}

static void pci_endpoint_test_release_irq(struct pci_endpoint_test *test)
{
	int i;
	struct pci_dev *pdev = test->pdev;
	struct device *dev = &pdev->dev;

	for (i = 0; i < test->num_irqs; i++)
		devm_free_irq(dev, pci_irq_vector(pdev, i), test);

	test->num_irqs = 0;
}

static bool pci_endpoint_test_request_irq(struct pci_endpoint_test *test)
{
	int i;
	int err;
	struct pci_dev *pdev = test->pdev;
	struct device *dev = &pdev->dev;

	for (i = 0; i < test->num_irqs; i++) {
		int irq_number;
		irq_number=pci_irq_vector(pdev, i);
		
		printk("%d.irq_number=%d\n",i,irq_number);
		
		err = devm_request_irq(dev, irq_number,
				       pci_endpoint_test_irqhandler,
				       IRQF_SHARED, DRV_MODULE_NAME, test);
		if (err)
			goto fail;
	}

	return true;

fail:
	switch (irq_type) {
	case IRQ_TYPE_LEGACY:
		dev_err(dev, "Failed to request IRQ %d for Legacy\n",
			pci_irq_vector(pdev, i));
		break;
	case IRQ_TYPE_MSI:
		dev_err(dev, "Failed to request IRQ %d for MSI %d\n",
			pci_irq_vector(pdev, i),
			i + 1);
		break;
	case IRQ_TYPE_MSIX:
		dev_err(dev, "Failed to request IRQ %d for MSI-X %d\n",
			pci_irq_vector(pdev, i),
			i + 1);
		break;
	}

	return false;
}



/*
* description:use pci dma to transfer data,from phy_mem_array to RC dst_addr.
* transfer data len: phy_mem_array->data_len - phy_mem_array->data_index; outstanding (vbuf_len)
* dma source address:phy_mem_array->phy_mem+phy_mem_array->data_index;
* dma destnation address:func_reg->dst_addr+func_reg->hostbuf_wrptr or func_reg->dst_addr
* according to RC DMA avariable length,let vbuf_len divide into two part,len0_size is represent RC-DMA-AREA reserver part,len1_size is represend RC-DMA-AREA begin part.
* 
* return: return transfer success data size.
*		  else when RC-DMA-AREA donot have enough res_size, return -1; 
*		  DMA transfer fail return -EIO.  
* argument:struct epf_data_transfer* represent DMA channel,struct phy_mem_array * is represent physical memory array which alloc by gv_polaris memory.
*/
static ssize_t pci_endpoint_read(struct file* file, char __user *buf,size_t count, loff_t *f_pos)
{
	
	int channel_id=0;
	int ret;
	int userSpace = 0;
	unsigned int size = 0;
	u32 chip_id;
	unsigned int valid_size,rdptr,wrptr,buf_size,len0_size,len1_size;
	struct pci_epf_data_transfer  *channel=NULL;
	
	struct pci_endpoint_file_private_data *use_private_data;
	use_private_data=(struct pci_endpoint_file_private_data *)file->private_data;
	
	if (use_private_data->data_channel_id<0)
	{
		printk("Error: channel_id not set\n");
		return -1;
	}
	// if (use_private_data->chip_gv9531_status == -1)
	// {
		// printk("Error: chip side is not alive...\n");
		// return -1;
	// }

	channel_id=use_private_data->data_channel_id;
	channel=&use_private_data->data_channel[use_private_data->data_channel_id];
	
	if ((channel->ep_reg->Flag&CH_FLAG_HOST_READ)==0)
	{
		printk("Error: CH%d not supported read\n",use_private_data->data_channel_id);
		return -11;
	}	

	chip_id = pci_get_chip_id(channel);
	if(chip_id  !=GV_CHIP_ID )
	{
		printk("%s,%d, EP is turn down \n",__func__,__LINE__);
		return -1;
	}

	#if LINUX_VERSION_CODE <= KERNEL_VERSION(5,0,0)
		userSpace = access_ok(VERIFY_WRITE, (void __user *)buf, _IOC_SIZE(count));
	#else
		userSpace = access_ok((void __user *)buf, _IOC_SIZE(count));
	#endif
     
	 {
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->dst_addr),lower_32_bits(channel->dma.phys_addr));
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->dst_addr)+4,upper_32_bits(channel->dma.phys_addr));//搬运数据的实际物理地址
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->command),COMMAND_DMA_WRITE);//会影响调试程序不停的写入数据
		
		rdptr = pci_endpoint_readl((void __iomem *)(&channel->ep_reg->hostbuf_rdptr));// fifo->out.
		wrptr = pci_endpoint_readl((void __iomem *)(&channel->ep_reg->hostbuf_wrptr));// fifo->in.
		buf_size = pci_endpoint_readl((void __iomem *)(&channel->ep_reg->hostbuf_size));//fifo->size.

		
		if (wrptr>=rdptr)
		{
			valid_size = wrptr - rdptr;
		}
		else
		{
			valid_size = wrptr + buf_size - rdptr;
		}
		
		
		if (count<=valid_size)
			size = count;
		else
			size = valid_size;
		 	
	//	printk("%s.%d.size = %d,buf_size=%d,rdptr=%d,wrptr%d,valid_size=%d\n",__func__,__LINE__,size,buf_size,rdptr,wrptr,valid_size);	
		/*for compatible further more server system*/
 		//smp_rmb();   
		len0_size = min(size,buf_size - (rdptr  & (buf_size -1)));
		len1_size = size - len0_size;

		
		if(len0_size !=0 )
		{
			 #if defined(__x86_64__) || defined(__i386__)
			clflush_cache_range(channel->dma.addr+(rdptr  & (buf_size -1)),len0_size);
			#endif
		if(userSpace)
        {
			ret = copy_to_user(buf, channel->dma.addr+(rdptr  & (buf_size -1)),len0_size);
		}
		else
		{
			memcpy(buf, channel->dma.addr+(rdptr  & (buf_size -1)),len0_size);
			ret = 0;
		}

			if(ret == 0){
				pci_endpoint_writel((void __iomem *)(&channel->ep_reg->status),STATUS_IRQ_RAISED|STATUS_READ_SUCCESS);		
			}
			else if (ret > 0)
			{
				size = size -ret;
				pci_endpoint_writel((void __iomem *)(&channel->ep_reg->status),STATUS_IRQ_RAISED|STATUS_READ_FAIL);
				printk("0 %d copy to user fail copy len0_size:%d\n",userSpace, ret);				
			}
		}
		if (len1_size !=0)
		{
 			#if defined(__x86_64__) || defined(__i386__)
			clflush_cache_range(channel->dma.addr,len1_size);
			#endif
			if(userSpace)
			{
				ret = copy_to_user(buf+len0_size, channel->dma.addr,len1_size);
			}
			else
			{
				memcpy(buf+len0_size, channel->dma.addr,len1_size);
				ret = 0;
			}
			
			if(ret == 0){
				pci_endpoint_writel((void __iomem *)(&channel->ep_reg->status),STATUS_IRQ_RAISED|STATUS_READ_SUCCESS);		
			}
			else if (ret > 0)
			{
				size = size -ret;
				pci_endpoint_writel((void __iomem *)(&channel->ep_reg->status),STATUS_IRQ_RAISED|STATUS_READ_FAIL);
				printk("1 %d copy to user fail copy len1_size:%d\n", userSpace, ret);			
			}
		}

		//smp_mb();   
		rdptr +=size;
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->hostbuf_rdptr),rdptr);

	}

	
	//printk("%s,%d,size %d,count %d\n",__func__,__LINE__,size,count);	
	return size;
}

/*
* Description: transfer data from RC-DMA-AREA to ep (PHY_MEM or EP-DMA-AREA decide by ep read function)
* 
* 1. first copy data from user space const char __user *buf to kernel RC-DMA-AREA. 
* 2. second trigger ep number 83 interrupt to inform ep move data from RC-DMA-AREA to ep's zone
*    which could be PHY_MEM or EP-DMA-AREA decide by ep read function.
* hint: use userSpace flag to judgement data copy from user space or kernel space.
* Return:write_len, write to ep side successful data size.
*/
static ssize_t pci_endpoint_write(struct file *file, const char __user *buf,
		size_t count, loff_t *f_pos)

{

	int channel_id=0;
	u32 chip_id;
	int userSpace = 0;
	int write_len=0;
	int timeout = 0;
	struct pci_epf_data_transfer  *channel=NULL;
	
	struct pci_endpoint_file_private_data *use_private_data;
	use_private_data=(struct pci_endpoint_file_private_data *)file->private_data;
      	
	
    channel_id =use_private_data->data_channel_id;
	if (use_private_data->data_channel_id<0)
	{
		printk("Error: channel_id not set\n");
		return -1;
	}
	
	// if (use_private_data->chip_gv9531_status == -1)
	// {
		// printk("Error: chip side is not alive...\n");
		// return -1;
	// }
	
	channel=&use_private_data->data_channel[use_private_data->data_channel_id];

	if ((channel->ep_reg->Flag&CH_FLAG_HOST_WRITE)==0)
	{
		printk("Error: CH%d not supported write\n",use_private_data->data_channel_id);
		return -11;
	}
	
	
	
	if ((channel->ep_reg->Flag&CH_FLAG_GV_USED)==0)
	{
		if (use_private_data->data_channel_id!=(NUM_DATA_CHANNELS-2)){
			//printk("Error-CH%d: Please confirm read is on\n",use_private_data->data_channel_id);
			return -12;
		}
	}
	chip_id = pci_get_chip_id(channel);
	if(chip_id  !=GV_CHIP_ID )
	{
		printk("%s,%d, EP is turn down \n",__func__,__LINE__);
		return -1;
	}
	
	#if LINUX_VERSION_CODE <= KERNEL_VERSION(5,0,0)
		userSpace = access_ok(VERIFY_READ, (void __user *)buf, _IOC_SIZE(count));
	#else
		userSpace = access_ok((void __user *)buf, _IOC_SIZE(count));
	#endif	
	//userSpace = access_ok(VERIFY_READ, (void __user *)buf, _IOC_SIZE(count));
	
	while(write_len<count)
	{
		int ret;
		int reg;
		int dma_len=0;
		static unsigned int magic_cnt=0;
		
		channel->phy_mem.data_index = write_len;
		channel->phy_mem.data_len = count;
		
		if ((count-write_len)>dmaMemSize[channel_id])
		{
			dma_len=dmaMemSize[channel_id];
		}
		else
		{
			dma_len=count-write_len;
		}
		
        if(userSpace)
        {
			ret = copy_from_user(channel->dma.addr,buf+write_len,dma_len);
			if (ret) 
			{
				printk("pci_endpoint_write copy from user err:%d \n",ret);
				break;
			}
        }
		else
		{
			memcpy(channel->dma.addr,buf+write_len,dma_len);
		}
		
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->magic),magic_cnt++);
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->status),0);//清楚状态
	    pci_endpoint_writel((void __iomem *)(&channel->ep_reg->src_addr),lower_32_bits(channel->dma.phys_addr));
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->src_addr)+4,upper_32_bits(channel->dma.phys_addr));//搬运数据的实际物理地址
	
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->irq_type), channel->ep_reg->irq_type);
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->irq_number),channel->ep_reg->irq_number);

		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->size),dma_len);//设置数据传输大小
		
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->phy_index),write_len);//设置已写数据大小
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->phy_len),count);//总共数据大小

		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->command),COMMAND_DMA_READ);
		
		//产生软中断
		pci_epf_irq_trigger(channel,1);//83
	

#ifdef  ORIGIN_CODE	
		ret = wait_for_completion_timeout(&channel->irq_raised,msecs_to_jiffies(3000));
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->command),0);
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->size),0);//设置数据传输大小
		if (!ret)
		{
			printk("RC write-err:wait irq timeout\n");	
			break;
		}
		
#else 

		
	//polling STATUS_IRQ_RAISED
	
	while(!(pci_endpoint_readl((void __iomem *)(&channel->ep_reg->status)) & STATUS_IRQ_RAISED))
	{
		if(timeout++ >= 5000){
			printk("RC write-err:wait irq timeout\n");	
			//use_private_data->chip_gv9531_status = -1;
			return write_len;
		}
		//msleep(1);
		usleep_range(100,101);
	}	

	pci_endpoint_writel((void __iomem *)(&channel->ep_reg->command),0);
	pci_endpoint_writel((void __iomem *)(&channel->ep_reg->size),0);//ÉÖÊ¾ݴ«Ê´ó



	reg=pci_endpoint_readl((void __iomem *)(&channel->ep_reg->status));
	reg &= ~STATUS_IRQ_RAISED;
	pci_endpoint_writel((void __iomem *)(&channel->ep_reg->status),reg);

#endif
		reg=pci_endpoint_readl((void __iomem *)(&channel->ep_reg->status));
		if(reg&STATUS_READ_FAIL)
		{//读取失败
			//printk("epc-read-fail:write_len=%d\n",write_len);
			break;
		}
		write_len+=dma_len;
		//printk("pci_endpoint_write:write_len=%d\n",write_len);
	}
	return write_len;
}


/*
* Description: invoked by IOCTL_PHY_MEM_WRITE command,
* 			   user space commit in	gv_bufWirte,by this data structure get data source address and data size.
*
* hint: use userSpace flag to judgement data copy from user space or kernel space.
* Return:write_len, write to ep side successful data size.
*/
static ssize_t pci_endpoint_write_PhyMem(struct file *file, const char __user *buf,
		size_t count, u32 bufIdx ,u32 ttlSize)

{

	int channel_id=0;
	int userSpace = 0;
	
	int write_len=0;
	u32 chip_id;
	int timeout = 0;
	struct pci_epf_data_transfer  *channel=NULL;
	
	struct pci_endpoint_file_private_data *use_private_data;
	use_private_data=(struct pci_endpoint_file_private_data *)file->private_data;

    channel_id =use_private_data->data_channel_id;
	if (use_private_data->data_channel_id<0)
	{
		printk("Error: channel_id not set\n");
		return -1;
	}
	// if (use_private_data->chip_gv9531_status == -1)
	// {
		// printk("Error: chip side is not alive...\n");
		// return -1;
	// }
	
	channel=&use_private_data->data_channel[use_private_data->data_channel_id];

	if ((channel->ep_reg->Flag&CH_FLAG_HOST_WRITE)==0)
	{
		printk("Error: CH%d not supported write\n",use_private_data->data_channel_id);
		return -11;
	}
	
	if ((channel->ep_reg->phy_mem_size ) <= 0)
	{
		printk("Error: CH%d have no more phy mem\n",use_private_data->data_channel_id);
		return 0;
	}
	
	
	if ((channel->ep_reg->Flag&CH_FLAG_GV_USED)==0)
	{
		if (use_private_data->data_channel_id!=(NUM_DATA_CHANNELS-2)){
			//printk("Error-CH%d: Please confirm read is on\n",use_private_data->data_channel_id);
			return -12;
		}
	}
	chip_id = pci_get_chip_id(channel);
	if(chip_id  !=GV_CHIP_ID )
	{
		printk("%s,%d, EP is turn down \n",__func__,__LINE__);
		return -1;
	}

	#if LINUX_VERSION_CODE <= KERNEL_VERSION(5,0,0)
		userSpace = access_ok(VERIFY_READ, (void __user *)buf, _IOC_SIZE(count));
	#else
		userSpace = access_ok((void __user *)buf, _IOC_SIZE(count));
	#endif	
	//userSpace = access_ok(VERIFY_READ, (void __user *)buf, _IOC_SIZE(count));
	
	while(write_len<count)
	{
		int ret;
		int reg;
		int dma_len=0;
		static unsigned int magic_cnt=0;
		
		channel->phy_mem.data_index = write_len;
		channel->phy_mem.data_len = count;
		
		if ((count-write_len)>dmaMemSize[channel_id])
		{
			dma_len=dmaMemSize[channel_id];
		}
		else
		{
			dma_len=count-write_len;
		}
		
        if(userSpace)
        {
			ret = copy_from_user(channel->dma.addr,buf+write_len,dma_len);
			if (ret) 
			{
				printk("pci_endpoint_write copy from user err:%d \n",ret);
				break;
			}
        }
		else
		{
			memcpy(channel->dma.addr,buf+write_len,dma_len);
		}
		
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->magic),magic_cnt++);
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->status),0);//清楚状态
	    pci_endpoint_writel((void __iomem *)(&channel->ep_reg->src_addr),lower_32_bits(channel->dma.phys_addr));
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->src_addr)+4,upper_32_bits(channel->dma.phys_addr));//搬运数据的实际物理地址
	
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->irq_type), channel->ep_reg->irq_type);
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->irq_number),channel->ep_reg->irq_number);

		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->size),dma_len);//设置数据传输大小
		
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->phy_index),write_len + bufIdx);//设置数据传输大小		
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->phy_len),ttlSize);//设置数据传输大小

		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->command),COMMAND_DMA_READ);
		
		//产生软中断
		pci_epf_irq_trigger(channel,1);//83
	

#ifdef  ORIGIN_CODE	
		ret = wait_for_completion_timeout(&channel->irq_raised,msecs_to_jiffies(3000));
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->command),0);
		pci_endpoint_writel((void __iomem *)(&channel->ep_reg->size),0);//设置数据传输大小
		if (!ret)
		{
			printk("RC write-err:wait irq timeout\n");	
			break;
		}
		
#else 

		
	//polling STATUS_IRQ_RAISED

	while(!(pci_endpoint_readl((void __iomem *)(&channel->ep_reg->status)) & STATUS_IRQ_RAISED))
	{
		if(timeout++ >= 5000){
			printk("RC write-err:wait irq timeout\n");	
			//use_private_data->chip_gv9531_status = -1;
			return write_len;
		}
		//msleep(1);
		usleep_range(100,101);
	}	

	pci_endpoint_writel((void __iomem *)(&channel->ep_reg->command),0);
	pci_endpoint_writel((void __iomem *)(&channel->ep_reg->size),0);//ÉÖÊ¾ݴ«Ê´ó



	reg=pci_endpoint_readl((void __iomem *)(&channel->ep_reg->status));
	reg &= ~STATUS_IRQ_RAISED;
	pci_endpoint_writel((void __iomem *)(&channel->ep_reg->status),reg);

#endif
		reg=pci_endpoint_readl((void __iomem *)(&channel->ep_reg->status));
		if(reg&STATUS_READ_FAIL)
		{//读取失败
			//printk("epc-read-fail:write_len=%d\n",write_len);
			break;
		}
		write_len+=dma_len;
		//printk("pci_endpoint_write:write_len=%d\n",write_len);
	}
	return write_len;
}

static int pci_endpoint_host_open(struct inode *inode, struct file *file)
{
	struct pci_endpoint_test *test = to_endpoint_test(file->private_data);
	struct pci_endpoint_file_private_data *use_private_data;

	use_private_data = kmalloc(sizeof(struct pci_endpoint_file_private_data),  GFP_ATOMIC);
	if(!use_private_data) {
		file->private_data = NULL;
		return -EFAULT;
	}

	memset(use_private_data,0,sizeof(struct pci_endpoint_file_private_data));
	use_private_data->data_channel_id=-1;
	use_private_data->data_channel=test->data_channel;
	printk("pci_endpoint_host_open...\n");
	file->private_data = (void *)use_private_data;
	
	return 0;
}


static int pci_endpoint_host_release(struct inode *inode, struct file *file)
{
	//struct pci_endpoint_test *test = to_endpoint_test(file->private_data);
	
	struct pci_epf_data_transfer  *channel=NULL;
	struct pci_endpoint_file_private_data *use_private_data;
	use_private_data = (struct pci_endpoint_file_private_data *)file->private_data;
	if (!use_private_data) {
		printk("try to close a invalid usr_data");
		return -EFAULT;
	}
	
	if(use_private_data->data_channel_id!=-1)
	{
		channel=&use_private_data->data_channel[use_private_data->data_channel_id];
		channel->ep_reg->Flag&=~(CH_FLAG_HOST_USED|CH_FLAG_TUN|0xffff00);
	}
	
	file->private_data = NULL;
	kfree(use_private_data);
	return 0;
}




/*
*  Host 端喂狗线程;定时触发83号软中断; 
*	读取文件的值,来决定是否触发83软中断.
*
*/

//__EPF_PUB_REG			 *sw_int_reg;

/*
void pci_epf_wdt_irq_trigger(__EPF_PUB_REG			 * sw_wdt_reg,u32 reg_value)
{
    u32 reg_val;

	while(1)
	{
    	reg_val=pci_endpoint_readl((void __iomem *)(&sw_wdt_reg->gv_sw_int_reg));
		if(reg_val == 0)
			break;
	}
    reg_val |= reg_value;
	reg_val |= 0x8; //清wdt bit 
	
    pci_endpoint_writel((void __iomem *)(&sw_wdt_reg->gv_sw_int_reg),reg_val);
	//printk("%s,%d,reg_val:0x%x \n  ",__func__,__LINE__,pci_endpoint_readl((void __iomem *)(&sw_wdt_reg->gv_sw_int_reg)););
}
*/
#if 0
int pci_rc_kick_wdt_thread(void* data)
{
	printk("%s,%d,start kick wdt kthread!\n  ",__func__,__LINE__);

    struct file *fp;
    mm_segment_t fs;
    loff_t pos;
	char file_buf;//每次仅读一个
	int cmd;
	
	printk("%s,%d,entry do while cyle!\n  ",__func__,__LINE__);
	do 
	{
		fp = filp_open("./.WDT",O_RDWR,0644);
	    if (IS_ERR(fp)) {
	        printk("read /home/wdt_file error/n");
	        continue;
	    }
		printk("%s,%d,read wdt file ok !\n  ",__func__,__LINE__);

	    fs = get_fs();
	    set_fs(KERNEL_DS);
	    pos =0;
		printk("%s,%d,set KERNEL_DS ok !\n  ",__func__,__LINE__);

	    vfs_read(fp, &file_buf, sizeof(file_buf), &pos);
		//cmd = aoti(file_buf);
		printk("%s,%d,read file_buf :%c !\n  ",__func__,__LINE__,file_buf);
		if( file_buf == '1' )
			pci_epf_wdt_irq_trigger(sw_int_reg,1);//83

		msleep(500);
	}while(!kthread_should_stop());

    filp_close(fp,NULL);
    set_fs(fs);

	printk("%s,%d,finish kick wdt kthread!\n  ",__func__,__LINE__);
	return 0;
}
#endif
/*
int pci_rc_kick_wdt(int kick_wdt)
{
	printk("%s,%d,start  pci_rc_kick_wdt!\n  ",__func__,__LINE__);
	
	if( kick_wdt == 1 )
		pci_epf_wdt_irq_trigger(sw_int_reg,1);//83

	return 0;
}
*/

//设置数据传输ID设置
long pci_endpoint_host_ioctl(struct file *file, unsigned int cmd,unsigned long arg)
{
	int ret = 0;
	struct pci_epf_data_transfer  *channel=NULL;
	struct pci_endpoint_file_private_data *use_private_data;
	use_private_data=(struct pci_endpoint_file_private_data *)file->private_data;
	
	if(use_private_data->data_channel_id!=-1)
		channel=&use_private_data->data_channel[use_private_data->data_channel_id];

	// if (use_private_data->chip_gv9531_status == -1)
	// {
		// printk("Error: chip side is not alive...\n");
		// return -1;
	// }	
	//printk("pci_endpoint_host_ioctl...\n");
	switch (cmd) {
		case IOCTL_SET_CHANNEL:
			//设置数据传输通道ID
			use_private_data->data_channel_id=arg;

			channel=&use_private_data->data_channel[use_private_data->data_channel_id];

			
			printk("use_private_data->data_channel_id=%d\n",use_private_data->data_channel_id);
			
			//清除数据
			//kfifo_reset(&channel->data_fifo);
			pci_endpoint_writel((void __iomem *)(&channel->ep_reg->hostbuf_rdptr),0);
			pci_endpoint_writel((void __iomem *)(&channel->ep_reg->hostbuf_wrptr),0);
			channel->ep_reg->Flag|=CH_FLAG_HOST_USED;
			
			break;
		case IOCTL_GET_CHANNEL:
			//获取通道数量(中断)
			//ret=pci_host->num_irqs;
			break;
		case IOCTL_SET_TUN:
			if(NULL!=channel)
			{
				__GV_TUN *gv_tun=&channel->ep_reg->gv_tun;
				
				printk("sizeof(__GV_TUN)=%lu\n",sizeof(__GV_TUN));
				
				copy_from_user(&channel->ep_reg->gv_tun,(void*)arg,sizeof(__GV_TUN));
				printk("host2gv.ip_v4  : %d.%d.%d.%d\n",gv_tun->ip_v4[0],gv_tun->ip_v4[1],gv_tun->ip_v4[2],gv_tun->ip_v4[3]);
				printk("host2gv.mask_v4: %d.%d.%d.%d\n",gv_tun->mask_v4[0],gv_tun->mask_v4[1],gv_tun->mask_v4[2],gv_tun->mask_v4[3]);
				printk("host2gv.gw_v4  : %d.%d.%d.%d\n",gv_tun->gw_v4[0],gv_tun->gw_v4[1],gv_tun->gw_v4[2],gv_tun->gw_v4[3]);
				printk("host2gv.dns_v4 : %d.%d.%d.%d\n",gv_tun->dns_v4[0],gv_tun->dns_v4[1],gv_tun->dns_v4[2],gv_tun->dns_v4[3]);

				channel->ep_reg->Flag|=CH_FLAG_TUN;
				pci_endpoint_writel((void __iomem *)(&channel->ep_reg->command),COMMAND_START_TUNDEV);
				
				//产生软中断
				pci_epf_irq_trigger(channel,1);//83
			}
			break;
		case IOCTL_SET_SHELL:
			{
				//设置通道带有shell属性数据
				channel->ep_reg->Flag|=CH_FLAG_REMOTE_SHELL;
			}
			break;
		case IOCTL_CLR_SHELL:
			{
				//设置通道带有shell属性数据
				channel->ep_reg->Flag&=~CH_FLAG_REMOTE_SHELL;
			}
			break;
		case IOCTL_GET_FLAG:
			{//返回通道flag
				if ((void *)arg!=NULL)
				{
					copy_to_user((void *)arg,&channel->ep_reg->Flag,sizeof(int));
				}
			}
			break;
		case IOCTL_WDT_CLR:
			{
				/* enable wdt:1. set bar reg flag 2. trigger interrupt  */
				int kick_wdt = arg;
				printk("%s,%d, kick_wdt = %d\n",__func__,__LINE__,kick_wdt);
				if (kick_wdt == 1)
				{
					//1.设置通道带清看门狗标记
					channel->ep_reg->Flag|=CH_FLAG_CLR_WDT;
					//2. 触发83号软中断
					pci_epf_irq_trigger(channel,1);//83
					
				}
			}
			break;
		case IOCTL_CHIP_ID:
			{
				u32 chip_id = pci_get_chip_id(channel);
				return chip_id;
			}
			break;
		case IOCTL_PHY_MEM_WRITE:
			{
				gv_bufWirte iobufWrite;
				ssize_t write_len;
				copy_from_user(&iobufWrite,(void *)arg,sizeof(gv_bufWirte));
				
				write_len = pci_endpoint_write_PhyMem(file,iobufWrite.bufPtr,iobufWrite.bufSize,iobufWrite.bufIdx,iobufWrite.ttlSize);
				return 	write_len;
			}
			break;
	}
	
	return ret;
}



static unsigned pci_endpoint_poll(struct file *file, poll_table *wait)
{
    unsigned int mask = 0;
	int rdptr,wrptr;
	u32 chip_id;
	struct pci_epf_data_transfer  *channel=NULL;
	struct pci_endpoint_file_private_data *use_private_data;
	use_private_data=(struct pci_endpoint_file_private_data *)file->private_data;
	
	if (use_private_data->data_channel_id<0)
	{
		printk("Error: channel_id not set\n");
		return -1;
	}
	// if (use_private_data->chip_gv9531_status == -1)
	// {
		// printk("Error: chip side is not alive...\n");
		// return -1;
	// }

	channel=&use_private_data->data_channel[use_private_data->data_channel_id];

	chip_id = pci_get_chip_id(channel);
	if(chip_id  !=GV_CHIP_ID )
	{
		printk("%s,%d, EP is turn down \n",__func__,__LINE__);
		return -1;
	}
	
	rdptr = pci_endpoint_readl((void __iomem *)(&channel->ep_reg->hostbuf_rdptr));
	wrptr = pci_endpoint_readl((void __iomem *)(&channel->ep_reg->hostbuf_wrptr));
	
	if(rdptr != wrptr)
	{
	   mask |= POLLIN | POLLRDNORM;
       channel->poll_read_flag = 0;	 	
	   return mask;
	}
	
    poll_wait(file, &channel->pci_epf_waitq,wait);
    /* 当轮询可读标志flag等于真 1时,返回POLLIN:表示有数据可读 */
	
	rdptr = pci_endpoint_readl((void __iomem *)(&channel->ep_reg->hostbuf_rdptr));
	wrptr = pci_endpoint_readl((void __iomem *)(&channel->ep_reg->hostbuf_wrptr));


    if(channel->poll_read_flag && (rdptr != wrptr)) 
    {
       mask |= POLLIN | POLLRDNORM;
       channel->poll_read_flag = 0;	   
    }
	
    return mask; /*返回POLLIN  时，应用poll()函数阻塞等待结束;并读取数据 */
}


static const struct file_operations pci_endpoint_test_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = pci_endpoint_host_ioctl,
	.open           = pci_endpoint_host_open,
	.release        = pci_endpoint_host_release,
	.write          = pci_endpoint_write,
	.read           = pci_endpoint_read,
	.poll           = pci_endpoint_poll,
};


static DEFINE_IDA(pci_endpoint_test_ida);


static int pci_endpoint_test_probe(struct pci_dev *pdev,
				   const struct pci_device_id *ent)
{
	int i;
	int err;
	int id;
	u32 version_info;
	char name[30];
	enum pci_barno bar;
	void __iomem *base;
	struct device *dev = &pdev->dev;
	struct pci_endpoint_test *test;
	struct pci_endpoint_test_data *data;
	enum pci_barno test_reg_bar = BAR_2;
	struct miscdevice *misc_device;
	struct pci_epf_data_transfer *data_channel=NULL;
	struct pci_epf_data_transfer  *channel = NULL;
	
	
 	printk("INFO:__%s,%d\n",__FILE__,__LINE__);
	if (pci_is_bridge(pdev))
	{
 		printk("ERR:__%s,%d\n",__FILE__,__LINE__);
		return -ENODEV;
	}

	test = devm_kzalloc(dev, sizeof(*test), GFP_KERNEL);
	if (!test)
	{
 		printk("ERR:__%s,%d\n",__FILE__,__LINE__);
		return -ENOMEM;
	}
	
	//分配空间给通道数组.[28]
	data_channel=kmalloc(sizeof(struct pci_epf_data_transfer)*NUM_DATA_CHANNELS,GFP_KERNEL);
	if (!data_channel)
	{
 		printk("ERR:__%s,%d\n",__FILE__,__LINE__);
		return -ENOMEM;
	}
    test->data_channel=data_channel;

	test->test_reg_bar = 2;
	test->alignment = 0;
	test->pdev = pdev;

	if (no_msi)
		irq_type = IRQ_TYPE_LEGACY;

	data = (struct pci_endpoint_test_data *)ent->driver_data;
	if (data) {
		test_reg_bar = data->test_reg_bar;
		test->alignment = data->alignment;
		irq_type = data->irq_type;
	}

	init_completion(&test->irq_raised);
	mutex_init(&test->mutex);

	err = pci_enable_device(pdev);
	if (err) {
		dev_err(dev, "Cannot enable PCI device\n");
		return err;
	}

	err = pci_request_regions(pdev, DRV_MODULE_NAME);
	if (err) {
		dev_err(dev, "Cannot obtain PCI resources\n");
		goto err_disable_pdev;
	}

	pci_set_master(pdev);
	

	if (!pci_endpoint_test_alloc_irq_vectors(test, irq_type))
		goto err_disable_irq;

	if (!pci_endpoint_test_request_irq(test))
		goto err_disable_irq;

	for (bar = BAR_0; bar <= BAR_5; bar++) {
		if (pci_resource_flags(pdev, bar) & IORESOURCE_MEM) {
			base = pci_ioremap_bar(pdev, bar);
			if (!base) {
				dev_err(dev, "Failed to read BAR%d\n", bar);
				WARN_ON(bar == test_reg_bar);
			}
			test->bar[bar] = base;
			printk("INFO:__%s,__%d,test->bar[%d]=%p\n",__FILE__,__LINE__,bar,test->bar[bar]);
		}
	}

	test->base = test->bar[test_reg_bar];//base = BAR_2
	if (!test->base) {
		err = -ENOMEM;
		dev_err(dev, "Cannot perform PCI test without BAR%d\n",
			test_reg_bar);
		goto err_iounmap;
	}

	//printk("\naddress:pci_host.test=%x\n",test);
	//pci_set_drvdata(pdev, test);

	id = ida_simple_get(&pci_endpoint_test_ida, 0, 0, GFP_KERNEL);
	if (id < 0) {
		err = id;
		dev_err(dev, "Unable to get id\n");
		goto err_iounmap;
	}

	snprintf(name, sizeof(name), DRV_MODULE_NAME ".%d", id);
	misc_device = &test->miscdev;
	misc_device->minor = MISC_DYNAMIC_MINOR;
	misc_device->name = kstrdup(name, GFP_KERNEL);
	if (!misc_device->name) {
		err = -ENOMEM;
		goto err_ida_remove;
	}
	misc_device->fops = &pci_endpoint_test_fops,

	err = misc_register(misc_device);
	if (err) {
		dev_err(dev, "Failed to register device\n");
		goto err_kfree_name;
	}
	
	//初始化通道变量
	printk("\naddress:pci_host.base=%p\n",test->base);
	memset(data_channel,0,NUM_DATA_CHANNELS*sizeof(struct pci_epf_data_transfer));
	for(i=0;i<NUM_DATA_CHANNELS;i++)
	{
		int irq_number=0;

		//读取通道需要中断
		//irq_number=pci_irq_vector(pdev, i%test->num_irqs);
		irq_number=(i%test->num_irqs)+1;
		printk("read%d.irq_number=%d[%d]\n",i,irq_number,pci_irq_vector(pdev, i%test->num_irqs));
        printk("ydq chnl %d size %d\n", i, dmaMemSize[i]);
		if (pci_data_channels_init(i,test,&data_channel[i],dmaMemSize[i],irq_number)<0)
		{
			printk("Err:__%s,__%d,err_kfree_name\n",__FILE__,__LINE__);
			goto err_kfree_name;
		}
		//printk("\tdata_channel[%d].ep_reg=%p\n",i,data_channel[i].ep_reg);
		//printk("\tdata_channel[%d].dma.size=%lx\n",i,data_channel[i].dma.size);
		//printk("\tdata_channel[%d].dma.alignment=%lx\n",i,data_channel[i].dma.alignment);
		//printk("\tdata_channel[%d].dma.orig_addr=%p\n",i,data_channel[i].dma.orig_addr);
		//printk("\tdata_channel[%d].dma.orig_phys_addr=%llx\n",i,data_channel[i].dma.orig_phys_addr);
		//printk("\tdata_channel[%d].dma.phys_addr=%llx\n",i,data_channel[i].dma.phys_addr);
		//printk("\tdata_channel[%d].dma.addr=%p\n",i,data_channel[i].dma.addr);

	}

//print pci driver version information.
	channel = &data_channel[0];
	version_info = pci_endpoint_readl((void __iomem *)(&channel->ep_reg->pci_version));
	printk("gv9531pci's version=%d\n",version_info);

	pci_gv9531_proc_init("channel",data_channel,id,NUM_DATA_CHANNELS);

	pci_set_drvdata(pdev, test);
	printk("==============OK==============\n");
	return 0;

err_kfree_name:
	printk("ERR:__%d\n",__LINE__);
	kfree(misc_device->name);

err_ida_remove:
	printk("ERR:__%d\n",__LINE__);
	ida_simple_remove(&pci_endpoint_test_ida, id);

err_iounmap:
	printk("ERR:__%d\n",__LINE__);
	for (bar = BAR_0; bar <= BAR_5; bar++) {
		if (test->bar[bar])
			pci_iounmap(pdev, test->bar[bar]);
	}
	pci_endpoint_test_release_irq(test);

err_disable_irq:
	printk("ERR:__%d\n",__LINE__);
	pci_endpoint_test_free_irq_vectors(test);
	pci_release_regions(pdev);

err_disable_pdev:
	printk("ERR:__%d\n",__LINE__);
	pci_disable_device(pdev);

	return err;
}

static void pci_endpoint_test_remove(struct pci_dev *pdev)
{
	int id;
	int i =0;
	enum pci_barno bar;
	struct pci_endpoint_test *test = pci_get_drvdata(pdev);
	struct miscdevice *misc_device = &test->miscdev;
 	struct device *dev = &pdev->dev;
	struct pci_epf_data_transfer *data_channel=NULL;
//	u32 chip_id;
	printk("INFO:__%s,%d,pci_endpoint_test_remove...\n",__FILE__,__LINE__);
	if (sscanf(misc_device->name, DRV_MODULE_NAME ".%d", &id) != 1)
		return;
	if (id < 0)
		return;
	data_channel = test->data_channel;

	#if 0
	//forbit remove when ep side is not alive	
	chip_id = pci_get_chip_id(data_channel);
	if(chip_id != GV_CHIP_ID){
		printk("%s,%d ep chip id is 0x%x,which means ep side is not alive...\n",__func__,__LINE__,chip_id);
		return ;
	}
	#endif

	for(i=0;i<NUM_DATA_CHANNELS;i++)
	{
		printk("\tdata_channel[%d].dma.orig_addr=%p\n",i,data_channel[i].dma.orig_addr);
		printk("\tdata_channel[%d].dma.orig_phys_addr=%llx\n",i,data_channel[i].dma.orig_phys_addr);
 		dma_free_coherent(dev, data_channel[i].dma.size + data_channel[i].dma.alignment, data_channel[i].dma.orig_addr, data_channel[i].dma.orig_phys_addr);
	//	pci_data_channels_exit(dev,&test->data_channel[i]);
	}
	if(test->data_channel)
		kfree(test->data_channel);
	pci_gv9531_proc_exit(id);
	misc_deregister(&test->miscdev);
	kfree(misc_device->name);
	ida_simple_remove(&pci_endpoint_test_ida, id);

	for (bar = BAR_0; bar <= BAR_5; bar++) {
		if (test->bar[bar])
			pci_iounmap(pdev, test->bar[bar]);
	}

	pci_endpoint_test_release_irq(test);
	pci_endpoint_test_free_irq_vectors(test);

	pci_release_regions(pdev);
	pci_disable_device(pdev);

	printk("---------------remove-ok---------------\n");
}

static const struct pci_device_id pci_endpoint_test_tbl[] = {
	{ PCI_DEVICE(0x16c3, 0x001a) },
	{ }
};
MODULE_DEVICE_TABLE(pci, pci_endpoint_test_tbl);

static struct pci_driver pci_endpoint_test_driver = {
	.name		= DRV_MODULE_NAME,
	.id_table	= pci_endpoint_test_tbl,
	.probe		= pci_endpoint_test_probe,
	.remove		= pci_endpoint_test_remove,
};
module_pci_driver(pci_endpoint_test_driver);

MODULE_DESCRIPTION("PCI ENDPOINT TEST HOST DRIVER");
MODULE_AUTHOR("Kishon Vijay Abraham I <kishon@ti.com>");
MODULE_LICENSE("GPL v2");
