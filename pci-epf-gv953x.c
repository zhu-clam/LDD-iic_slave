#include "pci-epf-gv953x.h"
#include "epf_data_channel.h"
#include "epf_channel_proc.h"



static struct pci_epf_gv95x *g_epf_953x;

static struct pci_epf_header gv953x_header = {
	.vendorid	= 0x16c3,//PCI_ANY_ID,
	.deviceid	= 0x001a,//PCI_ANY_ID,
	.baseclass_code = 0x0,
	.subsys_vendor_id = 0x16c3,	
	.subsys_id  = 0x1ae0,
	.baseclass_code = PCI_CLASS_OTHERS,
	.interrupt_pin	= PCI_INTERRUPT_INTA,
};

struct pci_epf_953x_data {
	enum pci_barno	test_reg_bar;
	bool		linkup_notifier;
};

static size_t bar_size[] = { 512, 512, 1024, 16384, 131072, 1048576 };
static bool bar_skip[] = {true, true, false, true, false, true };
static size_t bar_addr_type[] = {PCI_BASE_ADDRESS_MEM_TYPE_64,
                               PCI_BASE_ADDRESS_MEM_TYPE_64,
                               PCI_BASE_ADDRESS_MEM_TYPE_64,
                               PCI_BASE_ADDRESS_MEM_TYPE_64,
                               PCI_BASE_ADDRESS_MEM_TYPE_64,
                               PCI_BASE_ADDRESS_MEM_TYPE_64
                              };


//数据传输通道扩展
struct epf_data_transfer  data_transfer[EPF_NUM_DATA_CHANNELS];
static char dma_chn_r[4];
static char dma_chn_w[4];

//CK860 wdt 寄存器0xf700b000
#define CK860_WDT_REG 0xf700b000
volatile char *virt_ck860_wdt_addr=NULL;

static void __attribute__((optimize("-O0"))) pci_epf_gv95x_linkup(struct pci_epf *epf)
{
	//struct pci_epf_gv95x *epf_gv953x = epf_get_drvdata(epf);

}

static void __attribute__((optimize("-O0"))) pci_epf_gv95x_unbind(struct pci_epf *epf)
{
	struct pci_epf_gv95x *epf_gv953x = epf_get_drvdata(epf);
	struct pci_epc *epc = epf->epc;
	struct pci_epf_bar *epf_bar;
	int bar;

	printk("pci_epf_gv95x_unbind...");

	pci_epc_stop(epc);
	for (bar = BAR_0; bar <= BAR_5; bar++) {
		epf_bar = &epf->bar[bar];

		if (epf_gv953x->reg[bar]) {
			pci_epf_free_space(epf, epf_gv953x->reg[bar], bar);
			pci_epc_clear_bar(epc, epf->func_no, epf_bar);
		}
	}
}



static int __attribute__((optimize("-O0"))) pci_epf_gv95x_set_bar(struct pci_epf *epf)
{
	int bar;
	int ret;
	struct pci_epf_bar *epf_bar;
	struct pci_epc *epc = epf->epc;
	struct device *dev = &epf->dev;
	struct pci_epf_gv95x *epf_gv953x = epf_get_drvdata(epf);
	enum pci_barno test_reg_bar = epf_gv953x->test_reg_bar;

	for (bar = BAR_0; bar <= BAR_5; bar++) {
        if (bar_skip[bar] == true) {
            continue;
        }
		epf_bar = &epf->bar[bar];

		//epf_bar->flags |= upper_32_bits(epf_bar->size) ?
		//	PCI_BASE_ADDRESS_MEM_TYPE_64 : //bar[2] & bar[4] skip false,so can use
		//	PCI_BASE_ADDRESS_MEM_TYPE_32;
        epf_bar->flags |= bar_addr_type[bar];

		ret = pci_epc_set_bar(epc, epf->func_no, epf_bar);
		if (ret) {
			pci_epf_free_space(epf, epf_gv953x->reg[bar], bar);
			dev_err(dev, "Failed to set BAR%d\n", bar);
			if (bar == test_reg_bar)
				return ret;
		}

printk("bar%d: pci_epc_set_bar.ret=%d\n",bar,ret);

		/*
		 * pci_epc_set_bar() sets PCI_BASE_ADDRESS_MEM_TYPE_64
		 * if the specific implementation required a 64-bit BAR,
		 * even if we only requested a 32-bit BAR.
		 */
		//if (epf_bar->flags & PCI_BASE_ADDRESS_MEM_TYPE_64)
		//	bar++;
	}

	return 0;
}

static int __attribute__((optimize("-O0"))) pci_epf_gv95x_alloc_space(struct pci_epf *epf)
{
	struct pci_epf_gv95x *epf_gv953x = epf_get_drvdata(epf);
	struct device *dev = &epf->dev;
	void *base;
	int bar;
	enum pci_barno test_reg_bar = epf_gv953x->test_reg_bar;

	base = pci_epf_alloc_space(epf, sizeof(struct pci_epf_gv95x_bar_reg)*EPF_NUM_DATA_CHANNELS,test_reg_bar);
	if (!base) {
		dev_err(dev, "Failed to allocated register space\n");
		return -ENOMEM;
	}
	epf_gv953x->reg[test_reg_bar] = base;//test_reg_bar = BAR_2
printk("epf_gv953x->reg[%d]=%p,sizeof(struct pci_epf_gv95x_bar_reg)*EPF_NUM_DATA_CHANNELS=%d\n",test_reg_bar,base,sizeof(struct pci_epf_gv95x_bar_reg)*EPF_NUM_DATA_CHANNELS);

	printk("%s,%d, channel num:%d\n",__func__,__LINE__,EPF_NUM_DATA_CHANNELS);
	for (bar = BAR_0; bar <= BAR_5; bar++) {
		if ((bar == test_reg_bar) ||(bar_skip[bar] == true))
			continue;
		base = pci_epf_alloc_space(epf, bar_size[bar], bar);//no used.
		if (!base)
			dev_err(dev, "Failed to allocate space for BAR%d\n",bar);
		
		/*map SYST_CTRL[0xf9701000] base addr 0xf9700000*/		
		if (bar==BAR_4)
		{
			bar_size[bar]=8192;
			base = (unsigned long *)ioremap(0xf9700000,bar_size[bar]);
			epf->bar[bar].phys_addr=0xf9700000;
			epf->bar[bar].size=bar_size[bar];
		}
	    epf_gv953x->reg[bar] = base;

		printk("epf_gv953x->reg[%d]=%p,bar_size[%d]=%d\n",bar,base,bar,bar_size[bar]);

	}

	return 0;
}

static int __attribute__((optimize("-O0"))) pci_epf_gv95x_bind(struct pci_epf *epf)
{
	int ret;
	struct pci_epf_gv95x *epf_gv953x = epf_get_drvdata(epf);
	struct pci_epf_header *header = epf->header;
	struct pci_epc *epc = epf->epc;
	struct device *dev = &epf->dev;

	printk("pci_epf_gv95x_bind......\n");
	if (WARN_ON_ONCE(!epc))
		return -EINVAL;

	if (epc->features & EPC_FEATURE_NO_LINKUP_NOTIFIER)
		epf_gv953x->linkup_notifier = false;
	else
		epf_gv953x->linkup_notifier = true;

	epf_gv953x->msix_available = epc->features & EPC_FEATURE_MSIX_AVAILABLE;

	epf_gv953x->test_reg_bar = EPC_FEATURE_GET_BAR(epc->features);// 获取 PCI 硬件 BAR 空间.

	ret = pci_epc_write_header(epc, epf->func_no, header);
	if (ret) {
		dev_err(dev, "Configuration header write failed\n");
		return ret;
	}

	ret = pci_epf_gv95x_alloc_space(epf);
	if (ret)
		return ret;

	ret = pci_epf_gv95x_set_bar(epf);
	if (ret)
		return ret;

	ret = pci_epc_set_msi(epc, epf->func_no, epf->msi_interrupts);
	if (ret) {
		dev_err(dev, "MSI configuration failed\n");
		return ret;
	}
	printk("MSI configuration ok\n");

	if (epf_gv953x->msix_available) {
		ret = pci_epc_set_msix(epc, epf->func_no, epf->msix_interrupts);
		if (ret) {
			dev_err(dev, "MSI-X configuration failed\n");
			return ret;
		}
		printk("MSI-X configuration ok\n");
	}

	printk("pci_epf_gv95x_bind......ok\n");
	
    #if 1
    {
    	int i;
    	
    	//中断请求初始化
    	pci_epf_PubReg_init(g_epf_953x);
		pci_epf_request_irq(dev,data_transfer);

    	pci_epf_dma_init();
		
    	//管理DMA通道
    	ret=kfifo_init(&g_epf_953x->dma_r_ch_fifo,dma_chn_r,sizeof(dma_chn_r));
    	if (ret<0)
    	{
    		printk("err:kfifo_alloc-fail\n");
    		goto ERR0;
    	}
    	ret=kfifo_init(&g_epf_953x->dma_w_ch_fifo,dma_chn_w,sizeof(dma_chn_w));
    	if (ret<0)
    	{
    		printk("err:kfifo_alloc-fail\n");
    		goto ERR0;
    	}
    	
    	for(i=0;i<sizeof(dma_chn_r);i++)
    	{
    		dma_chn_r[i]=i;
    		dma_chn_w[i]=i;
    	}
    	kfifo_in(&g_epf_953x->dma_r_ch_fifo,dma_chn_r,sizeof(dma_chn_r));
    	kfifo_in(&g_epf_953x->dma_w_ch_fifo,dma_chn_w,sizeof(dma_chn_w));
    	
    	
	    printk("--------EPF_NUM_DATA_CHANNELS-------------------\n");
	    printk("\naddress:reg.base=%p\n",g_epf_953x->reg[epf_gv953x->test_reg_bar]);
	    printk("sizeof(struct pci_epf_gv95x_bar_reg)=%d\n",sizeof(struct pci_epf_gv95x_bar_reg));
	    for(i=0;i<EPF_NUM_DATA_CHANNELS;i++)
	    {
	    	pci_data_channels_init(i,g_epf_953x,&data_transfer[i],PCI_EPF_FUNC_MEM_READ_SIZE);
			printk("\tdata_transfer[%d].func_reg=%p\n",i,data_transfer[i].func_reg);
	    }
		
	    pci_gv9531_proc_init("channel",data_transfer,EPF_NUM_DATA_CHANNELS);
	    printk("----------------------OK------------------------\n");
	}
	
    #endif
	return 0;
ERR0:
	return -1;
}

static const struct pci_epf_device_id pci_epf_gv95x_ids[] = {
	{
		.name = "pci_epf_test",
	},
	{},
};


void wdt_init(void)
{

	virt_ck860_wdt_addr = (char *)ioremap(CK860_WDT_REG,0x1000);

}

void wdt_exit(void)
{
	iounmap((void*)virt_ck860_wdt_addr);
}

#if 0
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
#endif


/*
* host side query ck860_wdt WDT_EN bit to get ck860`s status
* 
*  step 1. enable ck860 wdt; set defalut timeout is about 5s.
*  step 2. get ck860_wdt WDT_EN bit`s value.
* 
*  return: WDT_EN bit`s value.
*/
u32 pci_epf_enable_system_wdt(int kick_wdt)
{
	u32 reg_val  = 0;
	if( kick_wdt == 1)
	{
		//virt_DMA_intr_reg_F9500_addr = (char *)ioremap(0xf9500000,1024);
		//virt_ck860_wdt_addr = (char *)ioremap(CK860_WDT_REG,0x1000);
		
	/*1.set TOP 0xb[128M/24M ~ 5s]timeout about 5s*/

		iowrite32(0x8,virt_ck860_wdt_addr+WDT_TORR);

	/*2.update  current restart register*/
		iowrite32(0x76,virt_ck860_wdt_addr+WDT_CRR);

	/*3. Enable WDT_EN*/
		reg_val = ioread32(virt_ck860_wdt_addr+WDT_CR);
		reg_val |= 0x1;
		iowrite32(reg_val,virt_ck860_wdt_addr+WDT_CR);
	/*4.create kick wdt kthread*/
		#if 0	
		if (kick_wdt_tsk == NULL)
		{
			kick_wdt_tsk= kthread_run(pci_epf_kick_wdt_thread,NULL,"pci_epf_kick_wdt_thread");
			if(IS_ERR(kick_wdt_tsk))
			{
				  printk(KERN_INFO "create pci_epf_kick_wdt_thread failed!\n"); 
			}
		}
		#endif	
	}
	else if(kick_wdt == 2) /*query wdt status*/
	{
		reg_val = ioread32(virt_ck860_wdt_addr+WDT_CR);
	}
	else if(kick_wdt == 3) /*kick wdt */
	{
		/*2.update  current restart register*/
		iowrite32(0x76,virt_ck860_wdt_addr+WDT_CRR);
	}
	return reg_val;
}


static long __attribute__((optimize("-O0"))) pci_epf_func_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret;
	struct epf_data_file_private_data *use_private_data=(struct epf_data_file_private_data *)file->private_data;
   // enum pci_barno test_reg_bar = g_epf_953x->test_reg_bar;
   // struct pci_epf_gv95x_bar_reg*reg = g_epf_953x->reg[test_reg_bar];

    
	switch(cmd)
	{
		/*
		* description: after open /dev/pci_epf_func,for set channel id.meanwhile clean data_fifo.  
		*              importantly, set struct file private member corresponding to struct epf_data_transfer[data_transfer],than
		* other file_operations function can access struct epf_data_transfer.
		* [INPUT] int channel_id: the channel id which want to set
		* [OUTPUT] int ret: set channel id success return 0,otherwise return -1.
		* Useage: invoke combo with IOC PCI_EPF_FUNC_IOC_CHANNEL_START together. 
		*/
		case PCI_EPF_FUNC_IOC_SET_CHANNEL:
			if(arg>=EPF_NUM_DATA_CHANNELS)
				return -1;
			//设置数据传输通道ID
			use_private_data->epf_channel_id=arg;
			use_private_data->epf_channel=data_transfer+use_private_data->epf_channel_id;

			use_private_data->epf_channel->func_reg->hostbuf_rdptr = 0;
			use_private_data->epf_channel->func_reg->hostbuf_wrptr = 0;
			

			kfifo_reset(&use_private_data->epf_channel->data_fifo);
			use_private_data->epf_channel->func_reg->size=0;
			/*
			if(use_private_data->epf_channel_id != EPF_NUM_SHELL_CHANNEL)
			{
				kfifo_reset(&use_private_data->epf_channel->data_fifo);
			}
			else
			{
				use_private_data->epf_channel->func_reg->size=0;//debug-tun
			}*/
			break;
		/*
		* Description: set channel flag CH_FLAG_GV_USED, in order to sync with RC side to ensure data accuracy before transfer data. 
 		* 
		* Useage: invoke combo with IOC PCI_EPF_FUNC_IOC_CHANNEL_START together. 
		*/
		case PCI_EPF_FUNC_IOC_CHANNEL_START:	
			use_private_data->epf_channel->func_reg->Flag|=CH_FLAG_GV_USED;
			
			break;
		/*
		* Description: for user get tun ip.
		*
		* [OUPUT]:__GV_TUN
		*/
		case PCI_EPF_FUNC_IOC_GET_TUN_IP:
			if(use_private_data->epf_channel_id!=-1)
			{
				__GV_TUN *gv_tun=&use_private_data->epf_channel->func_reg->gv_tun;
				
				if(use_private_data->epf_channel->func_reg->Flag&CH_FLAG_TUN)
				{
					printk("sizeof(__GV_TUN)=%d\n",sizeof(__GV_TUN));
					copy_to_user((void *)arg,(void *)gv_tun,sizeof(__GV_TUN));
					printk("gv.ip_v4  : %d.%d.%d.%d\n",gv_tun->ip_v4[0],gv_tun->ip_v4[1],gv_tun->ip_v4[2],gv_tun->ip_v4[3]);
					printk("gv.mask_v4: %d.%d.%d.%d\n",gv_tun->mask_v4[0],gv_tun->mask_v4[1],gv_tun->mask_v4[2],gv_tun->mask_v4[3]);
					printk("gv.gw_v4  : %d.%d.%d.%d\n",gv_tun->gw_v4[0],gv_tun->gw_v4[1],gv_tun->gw_v4[2],gv_tun->gw_v4[3]);
					printk("gv.dns_v4 : %d.%d.%d.%d\n",gv_tun->dns_v4[0],gv_tun->dns_v4[1],gv_tun->dns_v4[2],gv_tun->dns_v4[3]);
				}
				else
					return -2;

			}
			break;
		/*
		* Description: clear special channle id's PHY_MEM flag,meanwhile clear phy_array_fifo_in and phy_array_fifo_out.
		* phy_array_fifo_in: PHY_MEM set by user invoke PCI_EPF_SET_PHY_ARRAY
		* phy_array_fifo_out:In pci_data_channels_dma_read_PhyArray function,kfifo in when EP receive data size equal to phy_mem.data len
		* (epf_channel->phy_mem.data_index + func_reg->size) == epf_channel->phy_mem.data_len 
		* 
		*  Useage: invoke when use phy_mem transfer data completely.
		*/	
		case PCI_EPF_CLR_PHY_ARRAY:
			
			 //printk("__%s,__%d,PCI_EPF_CLR_PHY_ARRAY-epf_channel_id=%d\n",__FILE__,__LINE__,use_private_data->epf_channel_id);
	    	 if(use_private_data->epf_channel_id!=-1)
	    	 {
	    	 	
	    	 	 use_private_data->epf_channel->func_reg->Flag&=~CH_FLAG_GV_PHY_MEM;
	    	 	
				 kfifo_reset(&use_private_data->epf_channel->phy_array_fifo_in);
				 use_private_data->epf_channel->func_reg->phy_mem_size = kfifo_len(&use_private_data->epf_channel->phy_array_fifo_in);
				 kfifo_reset(&use_private_data->epf_channel->phy_array_fifo_out);
				 
	    	 }
			 break;
		/*
		* Description: in the case,EP use PHY_MEM transfer data,the space is apply for hold read and write data from/to RC.
		* finally PHY_MEM will kfifo into phy_array_fifo_in.
		* [INPUT] struct phy_mem_array* ; the data structure use for contain memory alloc by polarisMem.ko,detail introduce about
		* this data structure see  phy_mem_array's member .
		* Return: 0 is kfifo in success, otherwise error.
		*/			 
	    case PCI_EPF_SET_PHY_ARRAY:
	    	if(use_private_data->epf_channel_id!=-1)
	    	{
	    		struct phy_mem_array  phy_array;
	    		copy_from_user((void *)&phy_array,(void *)arg,sizeof(struct phy_mem_array));
	    		//printk("__%s,__%d,phy_array.id=%x\n",__FILE__,__LINE__,phy_array.id);
	    		//printk("__%s,__%d,phy_array.phy_mem=%x\n",__FILE__,__LINE__,phy_array.phy_mem);
	    		if(phy_array.phy_mem==NULL)
	    		  return -3;

				if(kfifo_avail(&use_private_data->epf_channel->phy_array_fifo_in) > sizeof(struct phy_mem_array))
				{		    	
					ret=kfifo_in(&use_private_data->epf_channel->phy_array_fifo_in,&phy_array,sizeof(struct phy_mem_array));
					use_private_data->epf_channel->func_reg->phy_mem_size = kfifo_len(&use_private_data->epf_channel->phy_array_fifo_in);
		    		use_private_data->epf_channel->func_reg->Flag|=CH_FLAG_GV_PHY_MEM;
		    		//printk("__%s,__%d,ret=%d\n",__FILE__,__LINE__,ret);
				}
				else {
					printk("__%s,__%d, phy_array_fifo_in kfifo is full\n",__FILE__,__LINE__);
					return -4;
				}
	    		
	    	}
	    	break;

		/*
		* Description:return struct phy_mem_array for user ,in the case that EP read data will be keep in PHY_MEM, from RC side. 
		* [OUTPUT]1. the size of data struct phy_mem_array.when size is 32,return is correct.0 is means there is not have phy_mem_array
		*         	 in phy_array_fifo_out at all.
		*         2. data struct phy_mem_array.
		*/	
	    case PCI_EPF_GET_PHY_ARRAY:
	    	if(use_private_data->epf_channel_id!=-1)
	    	{
	    		
	    		int copied=0;

	    		ret=kfifo_to_user(&use_private_data->epf_channel->phy_array_fifo_out,(void *)arg,sizeof(struct phy_mem_array),&copied);
	    		if (copied==sizeof(struct phy_mem_array))
	    			return copied;
	    	}
	    	break;
		/*
		* Description:use pci_data_channels_dma_write_PhyArray function to transfer data from [PHY_MEM]phy-mem-array to RC-DMA-AREA,which alloc by polarisMem.ko and 
		* set by ioctl PCI_EPF_SET_PHY_ARRAY.
		* IN : struct phy_mem_array*, date store in phy-mem-array and want to be transfer.
		* OUT: struct phy_mem_array*, return phy_array.data_index which means data have transfered size.
		* 
		* return 0 means phy mem if full, positive means transfer data size,negative means transfer fail.
		*/
	    case PCI_EPF_PHY_MEM_WRITE_DMA:
	    	if(use_private_data->epf_channel_id!=-1)
	    	{
	    		struct epf_data_transfer  *epf_channel=use_private_data->epf_channel;
	    		struct phy_mem_array  phy_array;
	    		//int write_index=0;
	    		
				if ((epf_channel->func_reg->Flag&(CH_FLAG_GV_WRITE|CH_FLAG_HOST_USED))==0)
				{
					if ((epf_channel->func_reg->Flag&CH_FLAG_HOST_USED)==0)
					{
					//	printk("%d.Error: Please confirm read is on\n",epf_channel->channel_id);
						return -12;
					}
					printk("%d.Error: not supported write\n",epf_channel->channel_id);
					return -11;
				}

				if (epf_channel->func_reg->dst_addr==0)
				{
					printk("%d.Error: dst_addr not set\n",epf_channel->channel_id);
					return -2;
				}


				copy_from_user((void *)&phy_array,(void *)arg,sizeof(struct phy_mem_array));

				{
				//	printk("phy_array.data_len:%d,phy_array.data_index:%d\n",phy_array.data_len,phy_array.data_index);
				//	printk("hostbuf_size:%d,rdptr:%d,wrptr:%d\n",epf_channel->func_reg->hostbuf_size,epf_channel->func_reg->hostbuf_rdptr,epf_channel->func_reg->hostbuf_wrptr);
				}
				
	    		if ((phy_array.data_len-phy_array.data_index)<=epf_channel->func_reg->hostbuf_size)
	    		{
	    			 ret = pci_data_channels_dma_write_PhyArray(epf_channel,&phy_array);
	    			
	    			 if ((phy_array.data_len - phy_array.data_index)==ret)
	    			 {
						 phy_array.data_index+=ret;	
						 copy_to_user((void *)arg,(void *)&phy_array,sizeof(struct phy_mem_array));
	    			 }

					 return ret;
	    		}
	    		else
	    		{
	    			//int data_index=phy_array.data_index;
	    			do{
		    			struct phy_mem_array  dma_array;
		    			int dma_len;
		    			if ((phy_array.data_len-phy_array.data_index)>epf_channel->func_reg->hostbuf_size)
		    			{
		    				dma_len=epf_channel->func_reg->hostbuf_size;
		    			}
		    			else
		    				dma_len=(phy_array.data_len-phy_array.data_index);
		    				
	    			    dma_array.id =phy_array.id;
	    			    dma_array.buffer_size=phy_array.buffer_size;
	    			    dma_array.data_len=dma_len;
	    			    dma_array.data_index=0;
	    				dma_array.virt_mem=phy_array.virt_mem+phy_array.data_index;
	    				dma_array.phy_mem=phy_array.phy_mem+phy_array.data_index;

						ret = pci_data_channels_dma_write_PhyArray(epf_channel,&dma_array);
						if( dma_len == ret)
						{
							phy_array.data_index+=ret;
						}else {
							copy_to_user((void *)arg,(void *)&phy_array,sizeof(struct phy_mem_array));
    			  			return ret;
						}

    			   }while(phy_array.data_index<phy_array.data_len);
    		
    			   copy_to_user((void *)arg,(void *)&phy_array,sizeof(struct phy_mem_array));
    			   return ret;
	    		}
	    		
	    	
	        }
	        break;
		/*
		* Description: get func_reg->Flag for user,temporary no use.
		* 
		*/	
	    case PCI_EPF_GET_FLAG:
	    	if(use_private_data->epf_channel_id!=-1)
	    	{
                if ((void *)arg!=NULL)
                {
                    copy_to_user((void *)arg,(void *)&use_private_data->epf_channel->func_reg->Flag,sizeof(int));
                }
                else
                	return -2;
	    	}
	    	else
	    		return -1;
	    	break;
		/*
		* Description: when arg is 1,this interface will enable wdt and kick the wdt at the same time.
		*  			   used in reboot EP chip.
		*/
		case PCI_EPF_EN_WDT:
			{
				/*enable wdt  */
				int kick_wdt = arg;
				u32 wdt_en =  pci_epf_enable_system_wdt(kick_wdt);
				return wdt_en;
			}
			break;
		/*
		* Description: shoudown chip by GPIO14.
		*  			   
		*/		
		case PCI_EPF_GPIO14:
			{
				/*set gpio14 output;value 0*/
				shoudown_pulldown_gpio14();
			}
			break;
		/*
		* Description: start kick wdt kernel thread.
		*  			   
		*/		
		case PCI_EPF_START_KICK_WDT:
			{
				/*start kick wdt*/
				if (kick_wdt_tsk == NULL)
				{
					kick_wdt_tsk= kthread_run(pci_epf_kick_wdt_thread,NULL,"pci_epf_kick_wdt_thread");
					if(IS_ERR(kick_wdt_tsk))
					{
						  printk(KERN_INFO "create pci_epf_kick_wdt_thread failed!\n"); 
					}
				}				
			}
			break;		 
		/*
		* Description: stop kick wdt kernel thrad.
		*  			   
		*/		
		case PCI_EPF_STOP_KICK_WDT:
			{
				if(kick_wdt_tsk) {
					kthread_stop(kick_wdt_tsk);
					kick_wdt_tsk = NULL;
				}
			}
			break;		
	    default:
	        	printk("Err-cmd: cmd=%x\n",cmd);
	            return -1;

    }
    return 0;
}



static int __attribute__((optimize("-O0"))) pci_epf_func_open(struct inode *inode, struct file *file)
{
	
	struct epf_data_file_private_data *use_private_data;
	use_private_data = kmalloc(sizeof(struct epf_data_file_private_data),  GFP_ATOMIC);
	if(!use_private_data) {
		file->private_data = NULL;
		return -EFAULT;
	}

	memset(use_private_data,0,sizeof(struct epf_data_file_private_data));

	file->private_data = (void *)use_private_data;
	use_private_data->epf_channel_id=-1;
	use_private_data->epf_channel=NULL;

	return 0;
}

static int __attribute__((optimize("-O0"))) pci_epf_func_release(struct inode *inode, struct file *file)
{
	struct epf_data_file_private_data *use_private_data=(struct epf_data_file_private_data *)file->private_data;
	struct epf_data_transfer  *epf_channel=use_private_data->epf_channel;
	
	
	if (!use_private_data) {
		printk("try to close a invalid usr_data");
		return -EFAULT;
	}
	
	if (use_private_data->epf_channel_id!=-1)
		epf_channel->func_reg->Flag&=~CH_FLAG_GV_USED;


 	use_private_data->epf_channel->func_reg->Flag&=~CH_FLAG_GV_PHY_MEM;
	kfifo_reset(&use_private_data->epf_channel->phy_array_fifo_in);
	use_private_data->epf_channel->func_reg->phy_mem_size = kfifo_len(&use_private_data->epf_channel->phy_array_fifo_in);
	
	kfifo_reset(&use_private_data->epf_channel->phy_array_fifo_out);

	#if 0
	if (NULL!=epf_channel->pci_epf_func_mem)
		free_pages((unsigned long)epf_channel->pci_epf_func_mem, get_order(PCI_EPF_FUNC_MEM_READ_SIZE));
	#endif
	file->private_data = NULL;
	kfree(use_private_data);
	return 0;
}

static ssize_t __attribute__((optimize("-O0"))) pci_epf_func_read(struct file* file, char __user *buf,size_t count, loff_t *f_pos)
{
	
	//int channel_id=0;
	int read_len=0;
	int ret;

	//printk("%s,%d ,count =%d\n",__func__,__LINE__,count);	
	struct epf_data_file_private_data *use_private_data=(struct epf_data_file_private_data *)file->private_data;
	struct epf_data_transfer  *epf_channel=use_private_data->epf_channel;
	struct pci_epf_gv95x_bar_reg  *func_reg=epf_channel->func_reg;
	
	if (epf_channel->channel_id<0)
	{
		printk("Error: channel_id not set\n");
		return -1;
	}
	
	if ((func_reg->Flag&CH_FLAG_GV_READ)==0)
	{
		printk("Error: not supported read ,channel_id %d\n",epf_channel->channel_id);
		return -11;
	}
	
	
	if (func_reg->src_addr==0)
	{
		//printk("Error: src_addr not set\n");
		return 0;
	}
	
	ret=kfifo_len(&epf_channel->data_fifo);
	//printk("%s,%d ,count =%d,read_len:%d,epf_channel->channel_id:%d,kfifo_len:%d\n",__func__,__LINE__,count,read_len,epf_channel->channel_id,kfifo_len(&epf_channel->data_fifo));
	if(ret>0)
	{
		do{
			int copied=0;
			ret=kfifo_to_user(&epf_channel->data_fifo,buf+read_len,count-read_len,&copied);
			if ((ret<0)||(copied==0))
			{
				msleep(1);
				break;
			}
			read_len+=copied;
		}while(read_len<count);
	}

	//printk("%s,%d ,count =%d,read_len:%d,epf_channel->channel_id:%d,kfifo_len:%d\n",__func__,__LINE__,count,read_len,epf_channel->channel_id,kfifo_len(&epf_channel->data_fifo));
	return read_len;
}


static ssize_t __attribute__((optimize("-O0"))) pci_epf_func_write(struct file *file, const char __user *buf,
		size_t count, loff_t *f_pos)

{
	
	struct epf_data_file_private_data *use_private_data=(struct epf_data_file_private_data *)file->private_data;
	struct epf_data_transfer  *epf_channel=use_private_data->epf_channel;
	struct pci_epf_gv95x_bar_reg  *func_reg=epf_channel->func_reg;

	int write_len=0;
	unsigned int max_bufsize = min(func_reg->hostbuf_size,EPF_DMZ_MEM_SIZE);
	

	if (epf_channel->channel_id<0)
	{
		printk("Error: channel_id not set\n");
		return -1;
	}
	
	if ((func_reg->Flag&CH_FLAG_GV_WRITE)==0)
	{
		printk("%d.Error: not supported write\n",epf_channel->channel_id);
		return -11;
	}
	
	if ((func_reg->Flag&CH_FLAG_HOST_USED)==0)
	{
		//printk("%d.Error: Please confirm read is on\n",epf_channel->channel_id);
		return -12;
	}
	
	if (func_reg->dst_addr==0)
	{
		printk("%d.Error: dst_addr not set\n",epf_channel->channel_id);
		return 0;
	}
	
	while(write_len<count)
	{
		int ret;
		//int reg;
		int dma_len=0;
		static int magic_cnt=1;

		if (epf_channel->func_reg->status&STATUS_KFIFO_FULL)
		{
			printk("Error: STATUS_KFIFO_FULL\n");
			return 0;
		}

		if ((count-write_len)>max_bufsize)
		{
			dma_len=max_bufsize;
		}
		else
		{
			dma_len=count-write_len;
		}

		ret = copy_from_user(epf_channel->dma.buf,buf+write_len,dma_len);
		if (ret) 
		{
			printk("pci_endpoint_write copy from user err:%d \n",ret);
			break;
		}
		
		func_reg->magic=magic_cnt++;
		epf_channel->write_magic_cnt=magic_cnt++;
		if (pci_data_channels_dma_write(epf_channel,NULL,dma_len)== dma_len) //success transfer dam_len size data.
			write_len+=dma_len;
		else
			break;
	}
	return write_len;
}


/*
* pci_epf_func_mem 是get_free_page()函数申请的256K内核空间内存.
*
*  1.获取 pci_epf_func_mem 内存的页帧号;physical address >> PAGE_SHIFT.
*  2.根据pci_epf_func_mem物理地址的 页帧号,调用内存映射函数.输出vma 应用态虚拟内存地址.
*  https://www.cnblogs.com/pengdonglin137/p/8149859.html
*  No use.
*/
int __attribute__((optimize("-O0"))) pci_epf_func_mmap(struct file * file, struct vm_area_struct * vma)
{
	
	//struct epf_data_file_private_data *use_private_data=(struct epf_data_file_private_data *)file->private_data;
	//struct epf_data_transfer  *epf_channel=use_private_data->epf_channel;
	//unsigned long pfn;
	#if 0
	epf_channel->pci_epf_func_mem = (void *)__get_free_pages(GFP_KERNEL, get_order(PCI_EPF_FUNC_MEM_READ_SIZE));	
    pfn = (virt_to_phys(epf_channel->pci_epf_func_mem) >> PAGE_SHIFT);
	if (!pfn_valid(pfn))
    {   
        printk(KERN_INFO "pci_epf_func_mem err\n");
		return -EIO;
    }
    
	/*Linux 内核提供了remap_pfn_rang()函数,实现将内核空间的内存映射到用户空间.  */
    if (remap_pfn_range(vma, vma->vm_start, pfn,
                vma->vm_end - vma->vm_start,
                vma->vm_page_prot))
        return -EAGAIN;
        
   #endif
    return 0;
}


static unsigned __attribute__((optimize("-O0"))) pci_epf_func_poll(struct file *file, poll_table *wait)
{
     unsigned int mask = 0;
	 struct epf_data_file_private_data *use_private_data=(struct epf_data_file_private_data *)file->private_data;
	 struct epf_data_transfer  *epf_channel=use_private_data->epf_channel;


	/*有数据立马return.*/
	if(epf_channel->func_reg->Flag & CH_FLAG_GV_PHY_MEM)
	{
		if(kfifo_len(&epf_channel->phy_array_fifo_out) > 0)
		{
			mask |= POLLIN | POLLRDNORM;
        	epf_channel->poll_read_flag = 0;
			return mask;
		}
	}
	else 
	{
		if(kfifo_len(&epf_channel->data_fifo) > 0)
		{
			mask |= POLLIN | POLLRDNORM;
			epf_channel->poll_read_flag = 0;
			return mask;
		}
	}
    poll_wait(file, &epf_channel->pci_epf_waitq,wait);


	if(epf_channel->func_reg->Flag & CH_FLAG_GV_PHY_MEM)
	{
	    if ((epf_channel->poll_read_flag) && (kfifo_len(&epf_channel->phy_array_fifo_out) > 0)) /* 当轮询可读标志flag等于真 1时,返回POLLIN:表示有数据可读 */
	    {
	        mask |= POLLIN | POLLRDNORM;
	        epf_channel->poll_read_flag = 0;
	    }
	}
	else {
	    if ((epf_channel->poll_read_flag) && (kfifo_len(&epf_channel->data_fifo) > 0)) /* 当轮询可读标志flag等于真 1时,返回POLLIN:表示有数据可读 */
	    {
	        mask |= POLLIN | POLLRDNORM;
	        epf_channel->poll_read_flag = 0;
	    }
	}
    return mask; /*返回POLLIN  时，应用poll()函数阻塞等待结束;并读取数据 */
}

static const struct file_operations pci_epf_fops = {
	.open       = pci_epf_func_open,
	.release    = pci_epf_func_release,
	.write      = pci_epf_func_write,
	.read       = pci_epf_func_read,
    .mmap       = pci_epf_func_mmap,
    .poll    =  pci_epf_func_poll,
    .unlocked_ioctl = pci_epf_func_ioctl,
};
static struct miscdevice pci_epf_misc_device = {
	.minor  = MISC_DYNAMIC_MINOR,
	.name   = "pci_epf_func",
	.fops  = &pci_epf_fops
};


static int __attribute__((optimize("-O0"))) pci_epf_gv95x_probe(struct pci_epf *epf)
{

	struct device *dev = &epf->dev;
	const struct pci_epf_device_id *match;
	struct pci_epf_953x_data *data;
	enum pci_barno test_reg_bar = BAR_0;
	bool linkup_notifier = true;
    int ret;
    struct pci_epc *epc;
    
    printk("pci_epf_gv95x_probe...\n");
	match = pci_epf_match_device(pci_epf_gv95x_ids, epf);
	data = (struct pci_epf_953x_data *)match->driver_data;
	if (data) {
		test_reg_bar = data->test_reg_bar;
		linkup_notifier = data->linkup_notifier;
	}

	g_epf_953x = devm_kzalloc(dev, sizeof(*g_epf_953x), GFP_KERNEL);
	if (!g_epf_953x)
		return -ENOMEM;
	memset(g_epf_953x,0,sizeof(*g_epf_953x));
	
	epf->header = &gv953x_header;
    epf->msi_interrupts = 1;
	g_epf_953x->epf = epf;
	g_epf_953x->test_reg_bar = test_reg_bar;
	g_epf_953x->linkup_notifier = linkup_notifier;

	/*初始化work(work,func);第一个参数是work_struct;第二个参数是work要执行的函数体*/
	//INIT_DELAYED_WORK(&g_epf_953x->cmd_handler, pci_epf_test_cmd_handler);

	epf_set_drvdata(epf, g_epf_953x);

    epc = pci_epc_get("f1000000.pcie");
	if (IS_ERR(epc)) {
		ret = PTR_ERR(epc);
		return ret;
	}
	ret = pci_epc_add_epf(epc, epf);
	if (ret)
    {   
        pr_err("pci_epc_add_epf err\n");
		return ret;
    }
	ret = pci_epf_bind(epf);
    if (ret)
    {   
        pr_err("pci_epf_bind err\n");
        return ret;
    }
	ret = pci_epc_start(epc);
	if (ret) {
		dev_err(&epc->dev, "failed to start endpoint controller\n");
		return -EINVAL;
	}

    ret = misc_register(&pci_epf_misc_device);
    if( ret )
    {
		pr_err("cannot register miscdev (err=%d)\n", ret);
		return ret;
    }

	
    printk("pci_epf_gv95x_probe......ok\n");
    return 0;
}

static struct pci_epf_ops ops = {
	.unbind	= pci_epf_gv95x_unbind,
	.bind	= pci_epf_gv95x_bind,
	.linkup = pci_epf_gv95x_linkup,
};

static struct pci_epf_driver gv95x_driver = {
	.driver.name	= "pci_epf_test",
	.probe		= pci_epf_gv95x_probe,
	.id_table	= pci_epf_gv95x_ids,
	.ops		= &ops,
	.owner		= THIS_MODULE,
};

static int __init __attribute__((optimize("-O0"))) pci_epf_gv95x_init(void)
{
	int ret;


	ret = pci_epf_register_driver(&gv95x_driver);
	if (ret) {
		pr_err("Failed to register pci epf test driver --> %d\n", ret);
		return ret;
	}
    
    if (IS_ERR(pci_epf_create("pci_epf_test.0"))) {
        pr_err("failed to create endpoint function device\n");
        return -EINVAL;
    }
	wdt_init();
    printk("pci_epf_gv95x_init......OK\n");
	return 0;
}
module_init(pci_epf_gv95x_init);

static void __exit __attribute__((optimize("-O0"))) pci_epf_95x_exit(void)
{
	int i;

	pci_gv9531_proc_exit();
    for(i=0;i<EPF_NUM_DATA_CHANNELS;i++)
    {
    	pci_data_channels_exit(&data_transfer[i]);
    }
    pci_epf_dma_exit();
	wdt_exit();

	if(kick_wdt_tsk) {
		kthread_stop(kick_wdt_tsk);
		kick_wdt_tsk = NULL;
	}
	pci_epf_unregister_driver(&gv95x_driver);
}
module_exit(pci_epf_95x_exit);

MODULE_DESCRIPTION("PCI EPF TEST DRIVER");
MODULE_AUTHOR("Kishon Vijay Abraham I <kishon@ti.com>");
MODULE_LICENSE("GPL v2");
