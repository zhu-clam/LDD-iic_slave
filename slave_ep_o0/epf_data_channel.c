#include "epf_data_channel.h"
#include "epf_call_user_app.h"
#include <linux/kthread.h>   

void __attribute__((optimize("-O0")))pci_data_channels_cmd_handler(struct work_struct *work);
void __attribute__((optimize("-O0")))pci_data_irq_task(struct epf_data_transfer  *epf_channel);


volatile unsigned long *virt_sw_intr_reg_addr=NULL;

//DMA本地中断寄存器
volatile char *virt_DMA_intr_reg_F9500_addr=NULL;
volatile char *virt_DMA_intr_reg_F1380_addr=NULL;

struct epf_dma_done_channel  DMA_Read_chn[4];
struct epf_dma_done_channel  DMA_Write_chn[4];

struct task_struct *kick_wdt_tsk = NULL;

extern void * __attribute__((optimize("-O0"))) memcpy(void *,const void *,__kernel_size_t);

#define EPF_DMA_DONE_INTR  1


void __attribute__((optimize("-O0")))pci_epf_dma_init(void)
{
	int i;
	u32 reg_val;

    if (NULL==virt_DMA_intr_reg_F1380_addr)
    {//DMA中断
    	virt_DMA_intr_reg_F1380_addr = (char *)ioremap(0xf1380000,1024);
    }
    
    if (NULL==virt_DMA_intr_reg_F9500_addr) 
    {//DMA中断
    	virt_DMA_intr_reg_F9500_addr = (char *)ioremap(0xf9500000,1024);
    }

printk("__%s,__%d,virt_DMA_intr_reg_F1380_addr=%p\n",__FILE__,__LINE__,virt_DMA_intr_reg_F1380_addr);
printk("__%s,__%d,virt_DMA_intr_reg_F9500_addr=%p\n",__FILE__,__LINE__,virt_DMA_intr_reg_F9500_addr);

	for(i=0;i<4;i++)
	{
		DMA_Read_chn[i].status=0;
		DMA_Write_chn[i].status=0;
		
		init_completion(&DMA_Read_chn[i].dma_done);
		init_completion(&DMA_Write_chn[i].dma_done);
		
		#ifdef EPF_DMA_DONE_INTR
		//pci_epf_ReadDone_IntrEn(i);
		//pci_epf_WriteDone_IntrEn(i);
		#endif
	}
    
    if (NULL!=virt_DMA_intr_reg_F9500_addr)
    {//开启全局中断
        		
	    reg_val = ioread32(virt_DMA_intr_reg_F9500_addr+0x34);
	    reg_val |= 0x1;
	    iowrite32(reg_val, virt_DMA_intr_reg_F9500_addr+0x34);
printk("__%s,__%d,%x=%x\n",__FUNCTION__,__LINE__,0xf9500000+0x34,reg_val);

reg_val = ioread32(virt_DMA_intr_reg_F9500_addr+0x34);
printk("__%s,__%d,%x=%x\n",__FUNCTION__,__LINE__,0xf9500000+0x34,reg_val);
	}
}


void __attribute__((optimize("-O0")))pci_epf_dma_exit(void)
{
	if (NULL!=virt_DMA_intr_reg_F9500_addr)
		iounmap((void *)virt_DMA_intr_reg_F9500_addr);
		
	if (NULL!=virt_DMA_intr_reg_F1380_addr)
		iounmap((void *)virt_DMA_intr_reg_F1380_addr);
}

static void __attribute__((optimize("-O0")))reg_map(void)
{
    if (NULL==virt_sw_intr_reg_addr)
    {
    	virt_sw_intr_reg_addr = (unsigned long *)ioremap(0xf970105c,4);
    }
    
}


static void __attribute__((optimize("-O0")))reg_ummap(void)
{
	if (NULL!=virt_sw_intr_reg_addr)
		iounmap((void *)virt_sw_intr_reg_addr);
		
}

void __attribute__((optimize("-O0")))reg_sw_int_trigger(u32 reg_value)
{
    u32 reg_val;
    if (NULL!=virt_sw_intr_reg_addr)
    {
	    reg_val = ioread32(virt_sw_intr_reg_addr);
	    reg_val |= reg_value;
	    iowrite32(reg_val, virt_sw_intr_reg_addr);
	}
}

void __attribute__((optimize("-O0")))reg_sw_int_clean(u32 reg_value)
{
    u32 reg_val;
    if (NULL!=virt_sw_intr_reg_addr)
    {
	    reg_val = ioread32(virt_sw_intr_reg_addr);
	    reg_val &= ~reg_value;
	    iowrite32(reg_val, virt_sw_intr_reg_addr);
	}
}

// 外部申明 符号:pci_epf_enable_system_wdt();扩展pci_epf_enable_system_wdt的作用域.
extern u32 pci_epf_enable_system_wdt(int kick_wdt);


static irqreturn_t __attribute__((optimize("-O0")))pci_epf_host2gv_irqhandler(int irq, void *dev_id)
{

	int i;
	struct pci_epf_gv95x_bar_reg  *func_reg = NULL;
	struct epf_data_transfer  *data_transfer=dev_id;
	
	reg_sw_int_clean(1);

	//judge channel[EPF_NUM_DATA_CHANNELS-1] flag whether is CH_FLAG_CLR_WDT.
	func_reg=data_transfer[EPF_NUM_DATA_CHANNELS-1].func_reg;
	if(func_reg->Flag & CH_FLAG_CLR_WDT)
	{
		func_reg->Flag &=~CH_FLAG_CLR_WDT;
		pci_epf_enable_system_wdt(1);
		return IRQ_HANDLED;
	}
		
	for(i=EPF_NUM_READ_CHANNELS;i<EPF_NUM_DATA_CHANNELS;i++)
	{
		if (data_transfer[i].func_reg->command)
		{
			queue_delayed_work(data_transfer[i].kpcitest_workqueue, &data_transfer[i].cmd_handler,0);
		}
	}
	
	return IRQ_HANDLED;
}

/*
* Description: pci dma tranfer complete irq handler
*
*/
static irqreturn_t __attribute__((optimize("-O0")))pci_epf_dmadone_irqhandler(int irq, void *dev_id)
{

	int Global_DmaStatus;
	struct epf_data_transfer  *data_transfer=dev_id;

	struct pci_epf *epf = data_transfer->epf;
	//struct device *dev = &epf->dev;
	struct pci_epc *epc = epf->epc;
	

	disable_irq_nosync(irq);


//printk(">>irqhandler: irq=%d\n",irq);

	Global_DmaStatus = ioread32(virt_DMA_intr_reg_F9500_addr+0x30);
	if(Global_DmaStatus&0x1)
	{
		int chn;
		for(chn=0;chn<4;chn++)
		{
			if (pci_epc_dma_write_done(epc, epf->func_no, chn) == true)
			{
				DMA_Write_chn[chn].status=DMA_DONE_STATUS_SUCCESS;
				complete(&DMA_Write_chn[chn].dma_done);
			}
			
			if(pci_epc_dma_read_done(epc, epf->func_no,chn) == true)
			{
				DMA_Read_chn[chn].status=DMA_DONE_STATUS_SUCCESS;
				complete(&DMA_Read_chn[chn].dma_done);
			}
		}
	    Global_DmaStatus&=~0x01;
	    iowrite32(Global_DmaStatus,virt_DMA_intr_reg_F9500_addr+0x30);
	}
	enable_irq(irq);
	return IRQ_HANDLED;
}

/*
* Description: register number 83 system irq for accept RC inform and number 5 for handler pci dma done.
*/
int __attribute__((optimize("-O0")))pci_epf_request_irq(struct device *dev,struct epf_data_transfer  *epf_channel)
{
	int err;
	int irq_number=83;

	printk("irq: %d -> GV-SW-INT-CH\n",irq_number);
	err = devm_request_irq(dev, irq_number,pci_epf_host2gv_irqhandler,IRQF_SHARED,"PCI-GV-SW-INT",epf_channel);
	if (err<0)
		printk("devm_request_irq fail: %d\n",err);

#ifdef EPF_DMA_DONE_INTR
	irq_number=5;
	printk("irq: %d -> GV-DMA-INT-CH\n",irq_number);
	err = devm_request_irq(dev, irq_number,pci_epf_dmadone_irqhandler,IRQF_SHARED,"PCI-EPF-DMA-INT",epf_channel);
	if (err<0)
		printk("devm_request_irq fail: %d\n",err);
#endif
	return err;
}


/*
* Description:set gpio14 directory to out,value to zero.
*/
void shoudown_pulldown_gpio14(void)
{
 	//echo out > /sys/class/gpio/gpio494/direction
	//echo 0 > /sys/class/gpio/gpio494/value
	volatile unsigned char *virt_reg_addr=NULL;
	u32 reg_val = 0 ;
	printk(KERN_DEBUG "Entry function shoudown_pulldown_gpio14\n");
	virt_reg_addr = (unsigned char *)ioremap(GPIO_BASE,1024);
	if(!virt_reg_addr)
		printk(KERN_DEBUG "ioremap GPIO_BASE:%p ok\n",virt_reg_addr);

	/*open leakage current.*/
	//1. gpio31 设置方向 - out.
	reg_val = ioread32(virt_reg_addr+GPIO_SWPORTA_DDR);
	reg_val | 0x1 << 31;
	iowrite32(reg_val,virt_reg_addr+GPIO_SWPORTA_DDR);
	//2.gpio31 value - 0
	reg_val = ioread32(virt_reg_addr+GPIO_SWPORTA_DR);
	reg_val &= ~(0x1 << 31);
	iowrite32(reg_val,virt_reg_addr+GPIO_SWPORTA_DR);
	
	//step1. gpio14设置方向-out
	reg_val = ioread32(virt_reg_addr+GPIO_SWPORTA_DDR);
	reg_val |= 0x1 << 14;
	iowrite32(reg_val,virt_reg_addr+GPIO_SWPORTA_DDR);
	//step2. 设置gpio14 value-0
	reg_val = ioread32(virt_reg_addr+GPIO_SWPORTA_DR);
	reg_val &= ~(0x1 << 14);
	iowrite32(reg_val,virt_reg_addr+GPIO_SWPORTA_DR);
	//printk(KERN_DEBUG"after clear bit\n");
  	iounmap((void *)virt_reg_addr);
}

/*
* Description:pci ep side kick wdt thread.
*/
int pci_epf_kick_wdt_thread(void* data)
{
	u32 reg_val  = 0;
	printk( "%s,%d,start kick wdt kthread!\n  ",__func__,__LINE__);
	
	/*1.set TOP 0xb[128M/24M ~ 5s]timeout about 5s*/
	iowrite32(0x8,virt_ck860_wdt_addr+WDT_TORR);

	/*2.update  current restart register*/
	iowrite32(0x76,virt_ck860_wdt_addr+WDT_CRR);

	/*3. Enable WDT_EN*/
	reg_val = ioread32(virt_ck860_wdt_addr+WDT_CR);
	reg_val |= 0x1;
	iowrite32(reg_val,virt_ck860_wdt_addr+WDT_CR);

	do 
	{
		/*延时等待*/
		iowrite32(0x76,virt_ck860_wdt_addr+WDT_CRR);	
		msleep(10);
	}while(!kthread_should_stop());

	printk( "%s,%d,finish kick wdt kthread!\n  ",__func__,__LINE__);
	return 0;
}

/*
//no use
int __attribute__((optimize("-O0")))pci_epc_map_addr_debug(struct pci_epc *epc, u8 func_no,
		     phys_addr_t phys_addr, u64 pci_addr, size_t size)
{
	int ret;
	unsigned long flags;

	if (IS_ERR_OR_NULL(epc) || func_no >= epc->max_functions)
	{
		printk("Error:__%s,__%d\n",__FILE__,__LINE__);
		return -EINVAL;
	}

	if (!epc->ops->map_addr)
	{
		printk("Error:__%s,__%d\n",__FILE__,__LINE__);
		return 0;
	}

	spin_lock_irqsave(&epc->lock, flags);
	ret = epc->ops->map_addr(epc, func_no, phys_addr, pci_addr, size);
	spin_unlock_irqrestore(&epc->lock, flags);

	return ret;
}
*/

/*
* Description: ioremap chip system software interrupt register
*/
int __attribute__((optimize("-O0")))pci_epf_PubReg_init(struct pci_epf_gv95x *epf_gv953x)
{
	struct pci_epf *epf = epf_gv953x->epf;
	//struct device *dev = &epf->dev;
	//struct pci_epc *epc = epf->epc;
	//enum pci_barno test_reg_bar = epf_gv953x->test_reg_bar;
	phys_addr_t pci_phys_addr;

	reg_map();

	epf_gv953x->pub_reg=epf_gv953x->reg[4];//io区域
	
	pci_phys_addr=epf->bar[4].phys_addr;
	
	//printk("__%s,__%d,virt_to_phys(%x)=%x\n",__FILE__,__LINE__,virt_sw_intr_reg_addr,virt_to_phys(virt_sw_intr_reg_addr));
	//printk("__%s,__%d,virt_to_phys(%x)=%x\n",__FILE__,__LINE__,epf_gv953x->pub_reg,virt_to_phys(epf_gv953x->pub_reg));
	printk("__%s,__%d,bar2.pci_phys_addr=%x\n",__FILE__,__LINE__,epf->bar[2].phys_addr);
	printk("__%s,__%d,bar4.pci_phys_addr=%x\n",__FILE__,__LINE__,pci_phys_addr);
	printk("__%s,__%d,epf_gv953x->pub_reg=%p\n",__FILE__,__LINE__,epf_gv953x->pub_reg);
	

	//initialization gv_sw_int_reg
	epf_gv953x->pub_reg->gv_sw_int_reg=0;
	return 0;
}

/*
* Description: unioremap chip system software interrupt register
*/
int __attribute__((optimize("-O0")))pci_epf_PubReg_exit(struct pci_epf_gv95x *epf_gv953x)
{
	//struct pci_epf *epf = epf_gv953x->epf;
	//struct device *dev = &epf->dev;
	//struct pci_epc *epc = epf->epc;
	//enum pci_barno test_reg_bar = epf_gv953x->test_reg_bar;
	
	reg_ummap();
	return 0;
}


/*
* enum pci_barno {
*	BAR_0,
*	BAR_1,
*	BAR_2,
*	BAR_3,
*	BAR_4,
*	BAR_5,
*};
*   Description:initialization EP side channles, alloc dma memory,set kfifo,to ep read channel initialization work queue.
*               besides initialization wait queue for poll interface.
* 	There is total six bar space, BAR_4 is ioremap to polaris chip SYS_CTRL base addr;
*   but only two bar space is used.
*	BAR[2] --> EPF_NUM_DATA_CHANNELS*sizeof(struct pci_epf_gv95x_bar_reg);//bar_reg.
*	BAR[4] --> ioremap(0xf9700000,bar_size[bar]);//SYS_CTRL reg base addr.
*/
int __attribute__((optimize("-O0")))pci_data_channels_init(int channel_id,struct pci_epf_gv95x *epf_gv953x,struct epf_data_transfer  *epf_channel,size_t size)
{
	int ret;
	
	struct pci_epf *epf = epf_gv953x->epf;
	struct device *dev = &epf->dev;
	//struct pci_epc *epc = epf->epc;
	enum pci_barno test_reg_bar = epf_gv953x->test_reg_bar;
	struct pci_epf_gv95x_bar_reg *reg = epf_gv953x->reg[test_reg_bar];

	epf_gv953x->pub_reg=epf_gv953x->reg[4];//io space reg[4],corresponding to pci_epf_gv95x_alloc_space(),ioremap(0xf9700000,bar_size[bar]);
	
	epf_channel->epf=epf;
	epf_channel->func_reg=reg+channel_id;//represend reg[2].EPF_NUM_DATA_CHANNELS*sizeof(struct pci_epf_gv95x_bar_reg);
	epf_channel->channel_id=channel_id;
	memset(epf_channel->func_reg,0,sizeof(struct pci_epf_gv95x_bar_reg));
	epf_channel->dma_ch=-1;
	epf_channel->epf_gv953x=epf_gv953x;
	epf_channel->write_magic_cnt=0;
	epf_channel->read_magic_cnt=0;

	//printk("__%s,__%d,epf_gv953x->pub_reg=%x\n",__FILE__,__LINE__,epf_gv953x->pub_reg);
	//printk("__%s,__%d,%d.func_reg=%x\n",__FILE__,__LINE__,channel_id,epf_channel->func_reg);

	
	init_completion(&epf_channel->irq_raised);

	//EP side WRITE ,on the other hand,RC READ [ 0 ~ 13 ]
	if (channel_id<EPF_NUM_WRITE_CHANNELS)
	{
	
		if(NULL==epf_channel->dma.buf)
		{
		    epf_channel->dma.buf = dma_alloc_coherent(dev,EPF_DMZ_MEM_SIZE, &epf_channel->dma.buf_phys, GFP_KERNEL);
			if (!epf_channel->dma.buf) {
		        return -ENOMEM;
			}
		}
		
		epf_channel->func_reg->Flag|=CH_FLAG_GV_WRITE;
	}
	//EP side WRITE, RC READ.[ 14 ~ 27 ]
	else
	{
		 char name[128];
		
		epf_channel->func_reg->Flag|=CH_FLAG_GV_READ;
		
	    epf_channel->buffer = dma_alloc_coherent(dev,EPF_DMZ_MEM_SIZE, &epf_channel->buffer_phys, GFP_KERNEL);
		if (!epf_channel->buffer) {
	        return -ENOMEM;
		}
		
		ret=kfifo_init(&epf_channel->data_fifo,epf_channel->buffer,EPF_DMZ_MEM_SIZE);//for transfer data between rc & ep.
		printk("channel: %d.Ret=%d\n",channel_id,ret);
		if (ret<0)
			goto Err_free_kfifo1;


		//printk("__%s,__%d,data_fifo-kfifo_avail.ret=%d\n",__FILE__,__LINE__,ret);

		memset(name,0,sizeof(name));
		sprintf(name,"k-gv953x-ch%d",channel_id);
		
		epf_channel->phy_buffer_in=vmalloc(PHY_MEM_ARRAR_FIFO_SIZE);//for store phy memory data struct 
		epf_channel->phy_buffer_out=vmalloc(PHY_MEM_ARRAR_FIFO_SIZE);//for store phy memory data struct 

		//use for phy-mem-array
		ret=kfifo_init(&epf_channel->phy_array_fifo_in,epf_channel->phy_buffer_in,PHY_MEM_ARRAR_FIFO_SIZE);
		if (ret<0)
			goto Err_free_kfifo1;
		epf_channel->func_reg->phy_mem_size = kfifo_len(&epf_channel->phy_array_fifo_in);

		ret=kfifo_init(&epf_channel->phy_array_fifo_out,epf_channel->phy_buffer_out,PHY_MEM_ARRAR_FIFO_SIZE);
		if (ret<0)
			goto Err_free_kfifo1;

		ret=kfifo_avail(&epf_channel->phy_array_fifo_in);
		//printk("__%s,__%d,phy-kfifo_avail.ret=%d\n",__FILE__,__LINE__,ret);
		
		ret=kfifo_avail(&epf_channel->phy_array_fifo_out);
		//printk("__%s,__%d,phy-kfifo_avail.ret=%d\n",__FILE__,__LINE__,ret);
	    		
		/*initialization workqueue*/
		epf_channel->kpcitest_workqueue = alloc_workqueue(name,WQ_MEM_RECLAIM | WQ_HIGHPRI, 0);
		/*initialization work(work,func);first argument work_struct;second argument is work corresponding function*/
		INIT_DELAYED_WORK(&epf_channel->cmd_handler,pci_data_channels_cmd_handler);
		
		queue_work(epf_channel->kpcitest_workqueue, &epf_channel->cmd_handler.work);
	}
	
	epf_channel->func_reg->pci_version = PCI_DRV_VERSION;

	/*initialization waitqueue for poll file_operations*/
	epf_channel->poll_read_flag=0;
	printk("ch%d.init_waitqueue_head...\n",channel_id);
	init_waitqueue_head(&epf_channel->pci_epf_waitq);	

	return 0;

//Err_free_kfifo:
//	kfifo_free(&epf_channel->data_fifo);
	
Err_free_kfifo1:

	if(NULL!=epf_channel->buffer)
		dma_free_coherent(dev,EPF_DMZ_MEM_SIZE, epf_channel->buffer,epf_channel->buffer_phys);

//err_addr:
	if(NULL != epf_channel->dma.buf)
	 	dma_free_coherent(dev,EPF_DMZ_MEM_SIZE,epf_channel->dma.buf, epf_channel->dma.buf_phys);

//err:
	return ret;
}

/*
* no use
* Description: EP channels exit function,invoked when remove "/dev/epf_func".for release resource at channels_init function.
*/
int __attribute__((optimize("-O0")))pci_data_channels_exit(struct epf_data_transfer  *epf_channel)
{
	struct pci_epf *epf = epf_channel->epf;
	struct device *dev = &epf->dev;
	//struct pci_epc *epc = epf->epc;
	
	reg_ummap();
	
	if(NULL!=epf_channel->kpcitest_workqueue)
	{//销毁工作队列
		
		cancel_delayed_work(&epf_channel->cmd_handler);
		
		destroy_workqueue(epf_channel->kpcitest_workqueue);
		epf_channel->kpcitest_workqueue=NULL;
	}
		
	if(NULL != epf_channel->dma.buf)
	 	dma_free_coherent(dev,EPF_DMZ_MEM_SIZE,epf_channel->dma.buf, epf_channel->dma.buf_phys);
	if(NULL!=epf_channel->buffer)
		dma_free_coherent(dev,EPF_DMZ_MEM_SIZE, epf_channel->buffer,epf_channel->buffer_phys);
	return 0;
}

/*kfifo copy in debug*/
static inline unsigned int __attribute__((optimize("-O0")))kfifo_unused(struct __kfifo *fifo)
{
	return (fifo->mask + 1) - (fifo->in - fifo->out);
}


static void __attribute__((optimize("-O0")))kfifo_copy_in_debug(struct __kfifo *fifo, const void *src,
		unsigned int len, unsigned int off)
{
	unsigned int size = fifo->mask + 1;
	unsigned int esize = fifo->esize;
	unsigned int l;

	off &= fifo->mask;
	if (esize != 1) {
		off *= esize;
		size *= esize;
		len *= esize;
	}
	l = min(len, size - off);

#if 1
//printk("__%s,__%d\n,fifo->data + off=%x,src=%x,l=%x\n",__FILE__,__LINE__,fifo->data + off,src,l);
	memcpy(fifo->data + off, src, l);

//printk("__%s,__%d\n,fifo->data=%x,src + l=%x,len - l=%x\n",__FILE__,__LINE__,fifo->data,src + l,len - l);
	memcpy(fifo->data, src + l, len - l);
	
#else
{
	int i;
	for(i=0;i<l;i++)
	{
		*((char *)(fifo->data + off+i))=*((char *)(src+i));
	}

	for(i=0;i<(len - l);i++)
	{
		*((char *)(fifo->data +i))=*((char *)(src + l+i));
	}
}
#endif
	/*
	 * make sure that the data in the fifo is up to date before
	 * incrementing the fifo->in index counter
	 */
	smp_wmb();
}

unsigned int __kfifo_in_debug(struct __kfifo *fifo,
		const void *buf, unsigned int len)
{
	unsigned int l;

	l = kfifo_unused(fifo);
	if (len > l)
		len = l;

	kfifo_copy_in_debug(fifo, buf, len, fifo->in);
	fifo->in += len;
	return len;
}

#define	kfifo_in_debug(fifo, buf, n) \
({ \
	typeof((fifo) + 1) __tmp = (fifo); \
	typeof(__tmp->ptr_const) __buf = (buf); \
	unsigned long __n = (n); \
	const size_t __recsize = sizeof(*__tmp->rectype); \
	struct __kfifo *__kfifo = &__tmp->kfifo; \
	(__recsize) ?\
	__kfifo_in_r(__kfifo, __buf, __n, __recsize) : \
	__kfifo_in_debug(__kfifo, __buf, __n); \
})

#if 0
/*for axi-dma */

static void kfifo_copy_in_axi_dma(struct __kfifo *fifo, const void *src,
		unsigned int len, unsigned int off,phys_addr_t dest_phys,phys_addr_t src_phys)
{
	unsigned int size = fifo->mask + 1;
	unsigned int esize = fifo->esize;
	unsigned int l;

	off &= fifo->mask;
	if (esize != 1) {
		off *= esize;
		size *= esize;
		len *= esize;
	}
	l = min(len, size - off);

#if 1
/*之前已做过判断
fifo_phys				fifo_phys+off	
			|------******------------------	 |
			|	len-l			剩余长度l		 |
			|-------******------------------ |
所以需要拷贝两次.
*/
//printk("__%s,__%d\n,fifo->data + off=%x,src=%x,l=%x\n",__FILE__,__LINE__,fifo->data + off,src,l);
	//memcpy(fifo->data + off, src, l);
	//AXI DMA 函数.
	
	if( l != 0)
	{
		AXIDMA_M2M_Test(src_phys,dest_phys+off,axi,l);
	//	printk("%s,%d,src:0x%x,dst:0x%x,l=%x\n",__FILE__,__LINE__,src_phys+off,dest_phys,l);
	}
	

//printk("__%s,__%d\n,fifo->data=%x,src + l=%x,len - l=%x\n",__FILE__,__LINE__,fifo->data,src + l,len - l);
	//memcpy(fifo->data, src + l, len - l);
	//AXI DMA 函数.
	if( (len-l) !=0 )
	{
		AXIDMA_M2M_Test(src_phys+l,dest_phys,axi,len-l);
	//	printk("%s,%d,src:0x%x,dst:0x%x,len-l=%x\n",__FILE__,__LINE__,src_phys,dest_phys+l,len-l);
	}
	
#else
{
	int i;
	for(i=0;i<l;i++)
	{
		*((char *)(fifo->data + off+i))=*((char *)(src+i));
	}

	for(i=0;i<(len - l);i++)
	{
		*((char *)(fifo->data +i))=*((char *)(src + l+i));
	}
}
#endif
	/*
	 * make sure that the data in the fifo is up to date before
	 * incrementing the fifo->in index counter
	 */
	smp_wmb();
}



unsigned int __kfifo_in_axi_dma(struct __kfifo *fifo,
		const void *buf, unsigned int len,phys_addr_t dest_phys,phys_addr_t src_phys)
{
	unsigned int l;

	l = kfifo_unused(fifo);
	if (len > l)
		len = l;

	//kfifo_copy_in_debug(fifo, buf, len, fifo->in);
	kfifo_copy_in_axi_dma(fifo, buf, len, fifo->in,dest_phys,src_phys);
	fifo->in += len;
	return len;
}


#define	kfifo_in_axi_dma(fifo, buf, n, dest_phys, src_phys) \
({ \
	typeof((fifo) + 1) __tmp = (fifo); \
	typeof(__tmp->ptr_const) __buf = (buf); \
	unsigned long __n = (n); \
	const size_t __recsize = sizeof(*__tmp->rectype); \
	struct __kfifo *__kfifo = &__tmp->kfifo; \
	(__recsize) ?\
	__kfifo_in_r(__kfifo, __buf, __n, __recsize) : \
	__kfifo_in_axi_dma(__kfifo, __buf, __n,dest_phys,src_phys); \
})

#endif
//#define ONCE_CODE_MEMCPY 

/*copy use pci dma*/

static void kfifo_copy_in_pci_dma(struct __kfifo *fifo, const void *src,
		unsigned int len, unsigned int off,phys_addr_t dest_phys,phys_addr_t src_phys,struct pci_epc *epc,u8 func_no,int dma_ch,u8 irq_number)
{
	unsigned int size = fifo->mask + 1;
	unsigned int esize = fifo->esize;
	unsigned int l;

#ifndef EPF_DMA_DONE_INTR	
	unsigned int timeout;
#endif	
	int ret  =0 ;

	off &= fifo->mask;
	if (esize != 1) {
		off *= esize;
		size *= esize;
		len *= esize;
	}
	l = min(len, size - off);

/*之前已做过判断
fifo_phys				fifo_phys+off	
			|------******------------------	 |
			|	len-l			剩余长度l		 |
			|-------******------------------ |
*/
	if( l != 0)
	{
		pci_epc_dma_read(epc,func_no, dma_ch, src_phys,dest_phys + off, 1,l,irq_number);
		//	printk("%s,%d,src:0x%x,dst:0x%x,l=%x\n",__FILE__,__LINE__,src_phys+off,dest_phys,l);
#ifdef EPF_DMA_DONE_INTR
		DMA_Read_chn[dma_ch].status=DMA_DONE_STATUS_USED;
		if (wait_for_completion_timeout(&DMA_Read_chn[dma_ch].dma_done,msecs_to_jiffies(3000))==0)
		{
				printk("__%s,__%d,.func_reg->size=%d\n",__FILE__,__LINE__,l);
				ret = -EIO;
		}
		else if (DMA_Read_chn[dma_ch].status&DMA_DONE_STATUS_ABORT)
		{
			 ret = -EIO;
		}
		DMA_Read_chn[dma_ch].status=0;
#else
		while (pci_epc_dma_read_done(epc, func_no,dma_ch) != true) {
			if ((timeout++ > 1000)||(l<=0)) {
				printk("__%s,__%d,.func_reg->size=%d\n",__FILE__,__LINE__,l);
				ret = -EIO;
				break;
			}
			usleep_range(1,100);
		}
#endif
	}
	
	if( (len-l) !=0 )
	{

		pci_epc_dma_read(epc,func_no, dma_ch, src_phys+l ,dest_phys, 1,(len-l),irq_number);		
	//	printk("%s,%d,src:0x%x,dst:0x%x,len-l=%x\n",__FILE__,__LINE__,src_phys,dest_phys+l,len-l);

#ifdef EPF_DMA_DONE_INTR
			DMA_Read_chn[dma_ch].status=DMA_DONE_STATUS_USED;
			if (wait_for_completion_timeout(&DMA_Read_chn[dma_ch].dma_done,msecs_to_jiffies(3000))==0)
			{
					printk("__%s,__%d,.func_reg->size=%d\n",__FILE__,__LINE__,l);
					ret = -EIO;
			}
			else if (DMA_Read_chn[dma_ch].status&DMA_DONE_STATUS_ABORT)
			{
				 ret = -EIO;
			}
			DMA_Read_chn[dma_ch].status=0;
#else
			while (pci_epc_dma_read_done(epc, func_no,dma_ch) != true) {
				if ((timeout++ > 1000)||(l<=0)) {
					printk("__%s,__%d,.func_reg->size=%d\n",__FILE__,__LINE__,l);
					ret = -EIO;
					break;
				}
				usleep_range(1,100);
			}
#endif

	}
	
	/*
	 * make sure that the data in the fifo is up to date before
	 * incrementing the fifo->in index counter
	 */
	smp_wmb();
}


unsigned int __kfifo_in_pci_dma(struct __kfifo *fifo,
		const void *buf, unsigned int len,phys_addr_t dest_phys,phys_addr_t src_phys,struct pci_epc *epc,u8 func_no,int dma_ch,u8 irq_number)
{
	unsigned int l;

	l = kfifo_unused(fifo);
	if (len > l)
		len = l;

	kfifo_copy_in_pci_dma(fifo, buf, len, fifo->in,dest_phys,src_phys,epc,func_no,dma_ch,irq_number);
	fifo->in += len;
	return len;
}

//kfifo_in_pci_dma(&epf_channel->data_fifo,epf_channel->buffer,func_reg->size,epf_channel->buffer_phys,func_reg->src_addr,epc,epf->func_no,dma_ch,func_reg->irq_number);	

#define	kfifo_in_pci_dma(fifo, buf, n, dest_phys, src_phys,epc,func_no,dma_ch,irq_number) \
({ \
	typeof((fifo) + 1) __tmp = (fifo); \
	typeof(__tmp->ptr_const) __buf = (buf); \
	unsigned long __n = (n); \
	const size_t __recsize = sizeof(*__tmp->rectype); \
	struct __kfifo *__kfifo = &__tmp->kfifo; \
	(__recsize) ?\
	__kfifo_in_r(__kfifo, __buf, __n, __recsize) : \
	__kfifo_in_pci_dma(__kfifo, __buf, __n,dest_phys,src_phys,epc,func_no,dma_ch,irq_number); \
})


/*
* Description:use pci dma transfer data from RC-DMA-AREA into ep kfifo,which also one ep dma area,meanwhile wake up poll wait queue. 
* 		 note: when argu void *vbuf isnot NULL,is represent read gv-cp cmd argument.otherwise is represent normal transfer data.
*           
* Return: invoke success return 0,negative means invoke fail.      
*/
int __attribute__((optimize("-O0"))) pci_data_channels_dma_read(struct epf_data_transfer  *epf_channel,void *vbuf)
{
	int ret = 0;
	//u32 crc32 = 0;
    //u64 timeout = 0;
    struct pci_epf_gv95x_bar_reg  *func_reg=epf_channel->func_reg;
	struct pci_epf *epf = epf_channel->epf;
	//struct device *dev = &epf->dev;
	struct pci_epc *epc = epf->epc;

	int dat_len;
	
	char dma_ch=epf_channel->channel_id%4;
	
	
	if (func_reg->size<=0)
	{
		printk("__%s,__%d,func_reg->size=%d\n",__FILE__,__LINE__,func_reg->size);
		ret = -44;
		goto READ_ERR1;
	}
	

	epf_channel->dma_status=1;
	while(kfifo_out(&epf_channel->epf_gv953x->dma_r_ch_fifo,&dma_ch,sizeof(char))!=1)
	{
	//	printk("__%s,__%d,dma_r_ch_fifo-len=%x\n",__FILE__,__LINE__,kfifo_len(&epf_channel->epf_gv953x->dma_r_ch_fifo));		
		usleep_range(10,1000);
	}
	epf_channel->dma_ch=dma_ch;

    dat_len=func_reg->size;

   if (NULL!=vbuf)
   {
   		/* first at all reset kfifo */
		kfifo_reset(&epf_channel->data_fifo);
   		
	   if (kfifo_avail(&epf_channel->data_fifo)>func_reg->size)
	   {
			kfifo_in_pci_dma(&epf_channel->data_fifo,epf_channel->buffer,func_reg->size,epf_channel->buffer_phys,func_reg->src_addr,epc,epf->func_no,dma_ch,func_reg->irq_number);	
	   }
	   else
	   {
	   		printk("%d,Err-kfifo-overflow: epf_channel->channel_id=%x\n",__LINE__,epf_channel->channel_id);
	   		ret=-4;
	   }
	   
	    //printk("__%s,__%d,func_reg->size=%d,kfifo_len:%d,epf_channel->channel_id=%d\n",__func__,__LINE__,func_reg->size,kfifo_len(&epf_channel->data_fifo),epf_channel->channel_id);
		memcpy(vbuf,epf_channel->buffer,dat_len);
		/*at the end reset kfifo again*/
		kfifo_reset(&epf_channel->data_fifo);
   }
   else
   {
	   if (kfifo_avail(&epf_channel->data_fifo)>func_reg->size)
	   {
		kfifo_in_pci_dma(&epf_channel->data_fifo,epf_channel->buffer,func_reg->size,epf_channel->buffer_phys,func_reg->src_addr,epc,epf->func_no,dma_ch,func_reg->irq_number);	
	   }
	   else
	   {
	   		//printk("Err-kfifo-overflow: epf_channel->channel_id=%x\n",epf_channel->channel_id);
	   		ret=-4;
	   }
	   
   }

    kfifo_in(&epf_channel->epf_gv953x->dma_r_ch_fifo,&dma_ch,sizeof(char));
    epf_channel->dma_ch=-1;
    epf_channel->dma_status=0;
	

READ_ERR1:
    epf_channel->poll_read_flag = 1;
    wake_up_interruptible(&epf_channel->pci_epf_waitq);
	//printk("__%s,__%d,ret=%d,func_reg->size=%d,kfifo_len:%d,epf_channel->channel_id=%d\n",__func__,__LINE__,ret,func_reg->size,kfifo_len(&epf_channel->data_fifo),epf_channel->channel_id);
	return ret;

}

/*
* Description:use pci dma transfer data between PHY_MEM with RC-DMA-AREA.
*             when phy_mem.data_index add func_reg->size is equal to epf_channel->phy_mem.data_len will kfifo in phy_array_fifo_out.
*             will let application use PCI_EPF_GET_PHY_ARRAY to get the virt address .meanwhile wakeup waitqueue for poll interface.
* Return: 0 is invoke success,negative is means invoke fail. EP read data function always be invoked when receive 83 irq.
*/
int __attribute__((optimize("-O0"))) pci_data_channels_dma_read_PhyArray(struct epf_data_transfer  *epf_channel)
{
	int ret = 0;
#ifndef EPF_DMA_DONE_INTR	
    u64 timeout = 0;
#endif
    struct pci_epf_gv95x_bar_reg  *func_reg=epf_channel->func_reg;
	struct pci_epf *epf = epf_channel->epf;
	struct device *dev = &epf->dev;
	struct pci_epc *epc = epf->epc;

	//int dat_len;
	
	//char dma_ch=epf_channel->channel_id%4;
	
	int dma_ch=epf_channel->channel_id%4;
	
	struct phy_mem_array  phy_array;
	
	//phy_array.id=0xaa55aa55;
	epf_channel->phy_mem.data_index = func_reg->phy_index;
	epf_channel->phy_mem.data_len = func_reg->phy_len;
	if( func_reg->phy_index == 0)
	{
		ret=kfifo_len(&epf_channel->phy_array_fifo_in);
		if (ret< sizeof(struct phy_mem_array))
		{
		//	printk("__%s,__%d,kfifo_len <=0  channel_id %d\n",__FILE__,__LINE__,epf_channel->channel_id); 			
			return -10;
		}
		ret=kfifo_out(&epf_channel->phy_array_fifo_in,&phy_array,sizeof(struct phy_mem_array));
		//epf_channel->func_reg->phy_mem_size = kfifo_len(&epf_channel->phy_array_fifo_in);
		epf_channel->phy_mem.id = 	phy_array.id;
		
		epf_channel->phy_mem.phy_mem = 	phy_array.phy_mem;
		epf_channel->phy_mem.virt_mem = 	phy_array.virt_mem;
		epf_channel->phy_mem.buffer_size = 	phy_array.buffer_size;
		if (ret!=sizeof(struct phy_mem_array))
			return -11;
	}


    //printk("__%s,__%d,ret=%d,phy_array.phy_mem=%x,id=%d\n",__FILE__,__LINE__,ret,phy_array.phy_mem,phy_array.id);
	
	if (func_reg->size<=0)
	{
		printk("__%s,__%d,func_reg->size=%d\n",__FILE__,__LINE__,func_reg->size);
		ret = -44;
		return ret;
	}
	

	epf_channel->dma_status=1;
	while(kfifo_out(&epf_channel->epf_gv953x->dma_r_ch_fifo,&dma_ch,sizeof(char))!=1)
	{
	//	printk("__%s,__%d,dma_r_ch_fifo-len=%x\n",__FILE__,__LINE__,kfifo_len(&epf_channel->epf_gv953x->dma_r_ch_fifo));		
		usleep_range(10,1000);
	}
	epf_channel->dma_ch=dma_ch;
	
    epf_channel->dma_status=2;
    pci_epc_dma_read(epc, epf->func_no, dma_ch, func_reg->src_addr,(unsigned int)epf_channel->phy_mem.phy_mem+epf_channel->phy_mem.data_index,1,func_reg->size,func_reg->irq_number);
	epf_channel->dma_status=3;
	
#ifdef EPF_DMA_DONE_INTR

	DMA_Read_chn[dma_ch].status=DMA_DONE_STATUS_USED;
	if (wait_for_completion_timeout(&DMA_Read_chn[dma_ch].dma_done,msecs_to_jiffies(3000))==0)
	{
            dev_err(dev, "ch%d timeout waiting for READ-DMA done\n",epf_channel->channel_id);
            printk("__%s,__%d,ch%d.func_reg->size=%d\n",__FILE__,__LINE__,epf_channel->channel_id,func_reg->size);
            ret = -EIO;
	}
	else if (DMA_Read_chn[dma_ch].status&DMA_DONE_STATUS_ABORT)
	{
		 dev_err(dev, "ch%d DMA-IRQ-ABORT\n",epf_channel->channel_id);
		 ret = -EIO;
	}
	DMA_Read_chn[dma_ch].status=0;
#else
    while (pci_epc_dma_read_done(epc, epf->func_no, dma_ch) != true) {
        if ((timeout++ > 1000)||(func_reg->size<=0)) {
            dev_err(dev, "ch%d timeout waiting for READ DMA done\n",epf_channel->channel_id);
            printk("__%s,__%d,ch%d.func_reg->size=%d\n",__FILE__,__LINE__,epf_channel->channel_id,func_reg->size);
            ret = -EIO;
            break;
        }
        usleep_range(1,100);
    }
#endif

    kfifo_in(&epf_channel->epf_gv953x->dma_r_ch_fifo,&dma_ch,sizeof(char));
    epf_channel->dma_ch=-1;
    epf_channel->dma_status=0;
    
    if (ret<0)
		return ret;

	if( (epf_channel->phy_mem.data_index + func_reg->size) == epf_channel->phy_mem.data_len)
	{
    	epf_channel->phy_mem.data_index += func_reg->size;
    	ret=kfifo_in(&epf_channel->phy_array_fifo_out,&epf_channel->phy_mem,sizeof(struct phy_mem_array));
		epf_channel->func_reg->phy_mem_size = kfifo_len(&epf_channel->phy_array_fifo_in);
    	//printk("__%s,__%d,ret=%d,phy_array.data_len=%d,id=%d\n",__FILE__,__LINE__,ret,phy_array.data_len,phy_array.id);
	    epf_channel->poll_read_flag = 1;
	    wake_up_interruptible(&epf_channel->pci_epf_waitq);
	}
    
	return ret;

}

/*
* Description:transfer data from EP-DMA-AREA to RC-DMA-AREA. 
*	      when transfer data to rc side complete will set STATUS_WRITE_SUCCESS flag at bar reg,and trigger msi irq to inform RC receive data.  
*
* return:when data transfer success,return transfer data size.
*        else when RC-DMA-AREA donot have enough res_size, return 0; 
*        DMA transfer fail return -EIO.  
*/
int __attribute__((optimize("-O0"))) pci_data_channels_dma_write(struct epf_data_transfer  *epf_channel,void *vbuf,int vbuf_len)
{
	int ret = 0;
#ifndef EPF_DMA_DONE_INTR	
    u64 timeout = 0;
#endif
    struct pci_epf_gv95x_bar_reg  *func_reg=epf_channel->func_reg;
	struct pci_epf *epf = epf_channel->epf;
	struct device *dev = &epf->dev;
	struct pci_epc *epc = epf->epc;
	int dma_ch=3;
	unsigned int res_size,len0_size,len1_size,hostbuf_reduce;

	res_size = func_reg->hostbuf_size - (func_reg->hostbuf_wrptr-func_reg->hostbuf_rdptr);

	

	if (vbuf_len > res_size) 
	{
		return 0;
	}

	hostbuf_reduce = ( func_reg->hostbuf_size - 1);

	smp_mb(); 
	len0_size = min((unsigned int)vbuf_len, func_reg->hostbuf_size - (func_reg->hostbuf_wrptr & hostbuf_reduce ));
	len1_size = vbuf_len -len0_size;


	epf_channel->dma_status=1;
	while(kfifo_out(&epf_channel->epf_gv953x->dma_w_ch_fifo,&dma_ch,sizeof(char))!=1)
	{
		usleep_range(10,1000);
	}
	epf_channel->dma_ch=dma_ch;
	
	if (NULL!=vbuf)
		memcpy(epf_channel->dma.buf,vbuf,vbuf_len);
		


    epf_channel->dma_status=2;
	pci_epc_dma_write(epc, epf->func_no,dma_ch,epf_channel->dma.buf_phys,(func_reg->dst_addr + (func_reg->hostbuf_wrptr & hostbuf_reduce ) ),1,
                len0_size, func_reg->irq_number);
	epf_channel->dma_status=3;

#ifdef EPF_DMA_DONE_INTR

	DMA_Write_chn[dma_ch].status=DMA_DONE_STATUS_USED;
	if (wait_for_completion_timeout(&DMA_Write_chn[dma_ch].dma_done,msecs_to_jiffies(3000))==0)
	{
            dev_err(dev, "ch%d timeout waiting for WRITE-DMA done\n",epf_channel->channel_id);
            printk("__%s,__%d,ch%d.func_reg->size=%d\n",__FILE__,__LINE__,epf_channel->channel_id,func_reg->size);
            ret = -EIO;
	}
	else if (DMA_Write_chn[dma_ch].status&DMA_DONE_STATUS_ABORT)
	{
		 dev_err(dev, "ch%d DMA-IRQ-ABORT\n",epf_channel->channel_id);
		 ret = -EIO;
	}
	DMA_Write_chn[dma_ch].status=0;
#else

    while (pci_epc_dma_write_done(epc, epf->func_no, dma_ch) != true) {
        if (timeout++ > 0x500) {
            dev_err(dev, "timeout waiting for WRITE DMA done\n");
            ret = -EIO;
            break;
        }
        usleep_range(1,100);
    }
    
#endif
    


	if (len1_size != 0)
	{
	    epf_channel->dma_status=2;
    	pci_epc_dma_write(epc, epf->func_no,dma_ch,epf_channel->dma.buf_phys+len0_size,func_reg->dst_addr,1,
                len1_size, func_reg->irq_number);
		epf_channel->dma_status=3;

#ifdef EPF_DMA_DONE_INTR

		DMA_Write_chn[dma_ch].status=DMA_DONE_STATUS_USED;
		if (wait_for_completion_timeout(&DMA_Write_chn[dma_ch].dma_done,msecs_to_jiffies(3000))==0)
		{
	            dev_err(dev, "ch%d timeout waiting for WRITE-DMA done\n",epf_channel->channel_id);
	            printk("__%s,__%d,ch%d.func_reg->size=%d\n",__FILE__,__LINE__,epf_channel->channel_id,func_reg->size);
	            ret = -EIO;
		}
		else if (DMA_Write_chn[dma_ch].status&DMA_DONE_STATUS_ABORT)
		{
			 dev_err(dev, "ch%d DMA-IRQ-ABORT\n",epf_channel->channel_id);
			 ret = -EIO;
		}
		DMA_Write_chn[dma_ch].status=0;
#else

	    while (pci_epc_dma_write_done(epc, epf->func_no, dma_ch) != true) {
	        if (timeout++ > 0x500) {
	            dev_err(dev, "timeout waiting for WRITE DMA done\n");
	            ret = -EIO;
	            break;
	        }
	        usleep_range(1,100);
	    }
    
#endif
	
	}
    
    kfifo_in(&epf_channel->epf_gv953x->dma_w_ch_fifo,&dma_ch,sizeof(char));
    epf_channel->dma_ch=-1;
    epf_channel->dma_status=0;

	if(ret>=0)
	{
		smp_mb();
		
		func_reg->hostbuf_wrptr += vbuf_len;

		func_reg->status |= STATUS_WRITE_SUCCESS;
		func_reg->status |= STATUS_IRQ_RAISED;
	    pci_epc_raise_irq(epc, epf->func_no, PCI_EPC_IRQ_MSI,func_reg->irq_number);
	
	}
	
	func_reg->size=0;
	//printk("ret=%d ,vbuf_len=%d,wrptr:%d,rdptr:%d\n",ret,vbuf_len,func_reg->hostbuf_wrptr,func_reg->hostbuf_rdptr);
//printk("__%s,__%d,ret=%d,magic=%x\n",__FILE__,__LINE__,ret,epf_channel->func_reg->magic);
	return vbuf_len;
	
}


/*
* description:transfer data from ep PHY_MEM_ARRAY to RC-DMA-AREA .
* transfer data len: phy_mem_array->data_len - phy_mem_array->data_index; outstanding (vbu
* dma source address:phy_mem_array->phy_mem+phy_mem_array->data_index;
* dma destnation address:func_reg->dst_addr+func_reg->hostbuf_wrptr or func_reg->dst_addr
* according to RC DMA avariable length,let vbuf_len divide into two part,len0_size is repr
* 
* return:when data transfer success,return transfer data size.
*        else when RC-DMA-AREA donot have enough res_size, return 0; 
*        DMA transfer fail return -EIO.  
* argument:struct epf_data_transfer* represent DMA channel,struct phy_mem_array * is repre
*/
int __attribute__((optimize("-O0")))pci_data_channels_dma_write_PhyArray(struct epf_data_transfer  *epf_channel,struct phy_mem_array  *ptr_array)
{
	int ret = 0;
#ifndef EPF_DMA_DONE_INTR	
    u64 timeout = 0;
#endif
    struct pci_epf_gv95x_bar_reg  *func_reg=epf_channel->func_reg;
	struct pci_epf *epf = epf_channel->epf;
	struct device *dev = &epf->dev;
	struct pci_epc *epc = epf->epc;
	int dma_ch=3;
	unsigned int vbuf_len,hostbuf_reduce ;

	unsigned int res_size,len0_size,len1_size;
	vbuf_len = ptr_array->data_len-ptr_array->data_index;

	
	res_size = func_reg->hostbuf_size - (func_reg->hostbuf_wrptr-func_reg->hostbuf_rdptr);


	if (vbuf_len > res_size) 
	{
		//printk("%s,%d,vbuf_len:%d.res_size:%d \n",__func__,__LINE__,vbuf_len,res_size);
		//printk("%s,%d,size:%d,rdptr:%d,wrptr:%d \n",__func__,__LINE__,func_reg->hostbuf_size,func_reg->hostbuf_rdptr, func_reg->hostbuf_wrptr);
		return 0;
	}

	hostbuf_reduce = ( func_reg->hostbuf_size - 1);

	smp_mb(); 

	len0_size = min((unsigned int)vbuf_len, func_reg->hostbuf_size - (func_reg->hostbuf_wrptr & hostbuf_reduce ));
	len1_size = vbuf_len -len0_size;
	
	if ((NULL==ptr_array)||(ptr_array->data_len==0))
		return -11;

	epf_channel->dma_status=1;
	while(kfifo_out(&epf_channel->epf_gv953x->dma_w_ch_fifo,&dma_ch,sizeof(char))!=1)
	{
		usleep_range(10,1000);
	}
	epf_channel->dma_ch=dma_ch;
	

    epf_channel->dma_status=2;
	pci_epc_dma_write(epc, epf->func_no,dma_ch,(unsigned int)ptr_array->phy_mem+ptr_array->data_index,(func_reg->dst_addr + (func_reg->hostbuf_wrptr & hostbuf_reduce ) ) ,1,len0_size, func_reg->irq_number);
	epf_channel->dma_status=3;

#ifdef EPF_DMA_DONE_INTR
	DMA_Write_chn[dma_ch].status=DMA_DONE_STATUS_USED;
	if (wait_for_completion_timeout(&DMA_Write_chn[dma_ch].dma_done,msecs_to_jiffies(3000))==0)
	{
            dev_err(dev, "ch%d timeout waiting for WRITE-DMA done\n",epf_channel->channel_id);
            printk("__%s,__%d,ch%d.func_reg->size=%d\n",__FILE__,__LINE__,epf_channel->channel_id,func_reg->size);
            ret = -EIO;
	}
	else if (DMA_Write_chn[dma_ch].status&DMA_DONE_STATUS_ABORT)
	{
		 dev_err(dev, "ch%d DMA-IRQ-ABORT\n",epf_channel->channel_id);
		 ret = -EIO;
	}
	DMA_Write_chn[dma_ch].status=0;
#else
    while (pci_epc_dma_write_done(epc, epf->func_no, dma_ch) != true) {
        if (timeout++ > 0x500) {
            dev_err(dev, "timeout waiting for WRITE DMA done\n");
            ret = -EIO;
            break;
        }
        usleep_range(1,100);
    }
#endif  

	if (len1_size != 0)
	{
		epf_channel->dma_status=2;
	    pci_epc_dma_write(epc, epf->func_no,dma_ch,(unsigned int)ptr_array->phy_mem+ptr_array->data_index+len0_size,func_reg->dst_addr,1,len1_size, func_reg->irq_number);
		epf_channel->dma_status=3;

#ifdef EPF_DMA_DONE_INTR
		DMA_Write_chn[dma_ch].status=DMA_DONE_STATUS_USED;
		if (wait_for_completion_timeout(&DMA_Write_chn[dma_ch].dma_done,msecs_to_jiffies(3000))==0)
		{
	            dev_err(dev, "ch%d timeout waiting for WRITE-DMA done\n",epf_channel->channel_id);
	            printk("__%s,__%d,ch%d.func_reg->size=%d\n",__FILE__,__LINE__,epf_channel->channel_id,func_reg->size);
	            ret = -EIO;
		}
		else if (DMA_Write_chn[dma_ch].status&DMA_DONE_STATUS_ABORT)
		{
			 dev_err(dev, "ch%d DMA-IRQ-ABORT\n",epf_channel->channel_id);
			 ret = -EIO;
		}
		DMA_Write_chn[dma_ch].status=0;
#else
	    while (pci_epc_dma_write_done(epc, epf->func_no, dma_ch) != true) {
	        if (timeout++ > 0x500) {
	            dev_err(dev, "timeout waiting for WRITE DMA done\n");
	            ret = -EIO;
	            break;
	        }
	        usleep_range(1,100);
	    }
#endif
	}
    kfifo_in(&epf_channel->epf_gv953x->dma_w_ch_fifo,&dma_ch,sizeof(char));
    epf_channel->dma_ch=-1;
    epf_channel->dma_status=0;


    if (ret>=0)
    {
    	func_reg->size=ptr_array->data_len;	

		smp_mb();
		
		func_reg->hostbuf_wrptr += vbuf_len;

		func_reg->status |= STATUS_WRITE_SUCCESS;
		func_reg->status |= STATUS_IRQ_RAISED;
	    pci_epc_raise_irq(epc, epf->func_no, PCI_EPC_IRQ_MSI,func_reg->irq_number);
	
	}
	func_reg->size=0;
	//printk("ret=%d ,vbuf_len=%d,wrptr:%d,rdptr:%d\n",ret,vbuf_len,func_reg->hostbuf_wrptr,func_reg->hostbuf_rdptr);
	
	return vbuf_len;//return vbuf size.
}


/*
* Description: ep side workqueue's function,workqueue begin work when trigger 83 irq handle.
*/
void __attribute__((optimize("-O0")))pci_data_channels_cmd_handler(struct work_struct *work)
{
	
	struct epf_data_transfer  *epf_channel = container_of(work, struct epf_data_transfer,
						     cmd_handler.work);
	
	pci_data_irq_task(epf_channel);
   
   //启动不停的轮询
	//queue_delayed_work(epf_channel->kpcitest_workqueue, &epf_channel->cmd_handler,msecs_to_jiffies(1));
}

/*
* Description: detail work handler function,according to command category to excute different work.
*/
void __attribute__((optimize("-O0")))pci_data_irq_task(struct epf_data_transfer  *epf_channel)
{
	struct pci_epf_gv95x_bar_reg  *func_reg=epf_channel->func_reg;
	//struct pci_epf *epf = epf_channel->epf;
	//struct device *dev = &epf->dev;
	//struct pci_epc *epc = epf->epc;
	
	int ret;
	//int count;
	u32 command;
	
	command = func_reg->command;

	if (!command)
		return;

	func_reg->command = 0;
	func_reg->status = 0;

	//COMMAND_DMA_READ:Receive data from rc side.
   if ((command&COMMAND_DMA_READ)) {
        //printk("__%s,__%d,%d.magic=%x\n",__FILE__,__LINE__,epf_channel->channel_id,func_reg->magic);
        //printk("__%s,__%d,func_reg->size=%d\n",__FILE__,__LINE__,func_reg->size);
        //printk("__%s,__%d,func_reg->Flag=%x\n",__FILE__,__LINE__,func_reg->Flag);
        
        if (epf_channel->func_reg->Flag&CH_FLAG_GV_PHY_MEM)
        {
        	ret=pci_data_channels_dma_read_PhyArray(epf_channel);
        	//printk("__%s,__%d,ret=%d\n",__func__,__LINE__,ret);
        }else if((EPF_NUM_SHELL_CHANNEL==epf_channel->channel_id)&&(epf_channel->func_reg->Flag&CH_FLAG_REMOTE_SHELL))
        {
        	struct GV_ShellHead cmd_head;
        	int i;
        	epf_channel->func_reg->Flag&=~CH_FLAG_REMOTE_SHELL;
        	
        	ret=pci_data_channels_dma_read(epf_channel,&cmd_head);
        	if(ret>=0)
        	{
				printk("\targc:%d\n",cmd_head.argc);
				for(i=0;i<cmd_head.argc;i++)
				{
					printk("\targv[%d]=%s\n",i,cmd_head.argv[i]);
				}
				call_user_shell(cmd_head.argc,cmd_head.argv);
			}
        }
        else
        {
        	ret=pci_data_channels_dma_read(epf_channel,NULL);
        }
        
	   	if(ret>=0)
	   		func_reg->status=STATUS_IRQ_RAISED|STATUS_READ_SUCCESS;
	   	else
	   		func_reg->status=STATUS_IRQ_RAISED|STATUS_READ_FAIL;

	  //printk("__%s,__%d,%d.status=%x\n",__FILE__,__LINE__,epf_channel->channel_id,func_reg->status);
	  // pci_epc_raise_irq(epc, epf->func_no, PCI_EPC_IRQ_MSI,func_reg->irq_number);

	   return;
   }
   //COMMAND_START_TUNDEV:Start pci virtual net
   if (command&COMMAND_START_TUNDEV)
   {
   	    char ip_str[16];
   	    char mask_str[16];
   	    char gw_str[16];
   	    char dns_str[16];
   	    
   	    __GV_TUN *gv_tun=&func_reg->gv_tun;
   	    
		sprintf(ip_str,"%d.%d.%d.%d",gv_tun->ip_v4[0],gv_tun->ip_v4[1],gv_tun->ip_v4[2],gv_tun->ip_v4[3]);
		sprintf(mask_str,"%d.%d.%d.%d",gv_tun->mask_v4[0],gv_tun->mask_v4[1],gv_tun->mask_v4[2],gv_tun->mask_v4[3]);
		sprintf(gw_str,"%d.%d.%d.%d",gv_tun->gw_v4[0],gv_tun->gw_v4[1],gv_tun->gw_v4[2],gv_tun->gw_v4[3]);
		sprintf(dns_str,"%d.%d.%d.%d",gv_tun->dns_v4[0],gv_tun->dns_v4[1],gv_tun->dns_v4[2],gv_tun->dns_v4[3]);
		
		printk("gv9531.ip_v4  : %s\n",ip_str);
		printk("gv9531.mask_v4: %s\n",mask_str);
		printk("gv9531.gw_v4  : %s\n",gw_str);
		printk("gv9531.dns_v4 : %s\n",dns_str);
		
		call_user_app("/usr/local/sbin/gv_tun_start.sh",ip_str,mask_str,gw_str,dns_str);
   }
   //COMMAND_RESET_GV9531:Reset chip ,no use.
    if (command&COMMAND_RESET_GV9531)
	{
		shoudown_pulldown_gpio14();
	}
    if (command&COMMAND_START_WDT)
	{
		/*start kick wdt kernel thread.*/
		if (kick_wdt_tsk == NULL)
		{
			kick_wdt_tsk= kthread_run(pci_epf_kick_wdt_thread,NULL,"pci_epf_kick_wdt_thread");
			if(IS_ERR(kick_wdt_tsk))
			{
				  printk(KERN_INFO "create pci_epf_kick_wdt_thread failed!\n"); 
			}
		}				
	}
    if(command&COMMAND_STOP_WDT)
	{
		/*stop kick wdt kernel thread.*/
		if(kick_wdt_tsk) {
			kthread_stop(kick_wdt_tsk);
			kick_wdt_tsk = NULL;
		}
	}   	
   
}
