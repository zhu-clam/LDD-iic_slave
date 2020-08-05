
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/slab.h>



#include "pci_data_channel.h"

//文件节点名称
#define MAX_CHIP_NUM 32

static int proc_channel_nums=0;


static ssize_t gv9531_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *f_pos)
{
    char *tmp = kzalloc((count+1), GFP_KERNEL);
	if (!tmp)
		return -ENOMEM;
 
	if (copy_from_user(tmp,buffer,count)) {
		kfree(tmp);
		return -EFAULT;
	}
    printk("%s Get user str :%s\n", __func__, tmp);
	kfree(tmp);
	return count;
}



static int gv9531_seq_show(struct seq_file *s, void *v) 
{ 
	//获取私有数据结构
	struct pci_epf_data_transfer *channel_base = s->private; 	
	int i;
#if 1
	seq_printf(s,"gv-intr-reg:\n");
	seq_printf(s,"[%p]: ",&channel_base[0].pub_reg->gv_sw_int_reg);
	seq_printf(s,"%08x\n",channel_base[0].pub_reg->gv_sw_int_reg);	
	seq_printf(s,"channel:\n");
	seq_printf(s,"address   ");
	seq_printf(s,"id  ");
	seq_printf(s,"magic	   ");
	seq_printf(s,"flag	   ");
	seq_printf(s,"command   ");
	seq_printf(s,"src_addr  ");
	seq_printf(s,"dst_addr  ");
	//seq_printf(s,"size	   ");
	//seq_printf(s,"checksum  ");
	seq_printf(s,"irq_type  ");
	seq_printf(s,"irq_number  ");
	
	seq_printf(s,"hostbuf_size  ");
	seq_printf(s,"valid_size  ");
	
	//seq_printf(s,"interrupts ");
	seq_printf(s,"direction \n");

	for(i=0;i<proc_channel_nums;i++)
	{
		seq_printf(s,"%p  ",channel_base[i].ep_reg);
		seq_printf(s,"%d  ",channel_base[i].channel_id);
		seq_printf(s,"%08x  ",channel_base[i].ep_reg->magic);
		seq_printf(s,"%08x  ",channel_base[i].ep_reg->Flag);
		seq_printf(s,"%08x  ",channel_base[i].ep_reg->command);
		seq_printf(s,"%08llx  ",channel_base[i].ep_reg->src_addr);
		seq_printf(s,"%08llx  ",channel_base[i].ep_reg->dst_addr);
		//seq_printf(s,"%08x  ",channel_base[i].ep_reg->size);
		//seq_printf(s,"%08x  ",channel_base[i].ep_reg->checksum);
		seq_printf(s,"%08x  ",channel_base[i].ep_reg->irq_type);
		seq_printf(s,"%08x  ",channel_base[i].ep_reg->irq_number);
		//seq_printf(s,"  %08lu	",channel_base[i].interrupts);
		seq_printf(s,"%08x  ",channel_base[i].ep_reg->hostbuf_size);
		seq_printf(s,"%08x  ",channel_base[i].ep_reg->size);	
		
		if(i<NUM_READ_CHANNELS)
			seq_printf(s,"read\n");
		else
			seq_printf(s,"write\n");

	}

	for(i=0;i<proc_channel_nums;i++)
	{
		if(channel_base[i].ep_reg->Flag&CH_FLAG_TUN)
		{
			seq_printf(s,"CH%d->TUN-DEV:\n",channel_base[i].channel_id);
			seq_printf(s,"ipV4  : %d.%d.%d.%d\n",channel_base[i].ep_reg->gv_tun.ip_v4[0],
			channel_base[i].ep_reg->gv_tun.ip_v4[1],channel_base[i].ep_reg->gv_tun.ip_v4[2],
			channel_base[i].ep_reg->gv_tun.ip_v4[3]);
			seq_printf(s,"maskV4: %d.%d.%d.%d\n",channel_base[i].ep_reg->gv_tun.mask_v4[0],
			channel_base[i].ep_reg->gv_tun.mask_v4[1],channel_base[i].ep_reg->gv_tun.mask_v4[2],
			channel_base[i].ep_reg->gv_tun.mask_v4[3]);
			seq_printf(s,"gw_v4 : %d.%d.%d.%d\n",channel_base[i].ep_reg->gv_tun.gw_v4[0],
			channel_base[i].ep_reg->gv_tun.gw_v4[1],channel_base[i].ep_reg->gv_tun.gw_v4[2],
			channel_base[i].ep_reg->gv_tun.gw_v4[3]);
			seq_printf(s,"dns_v4: %d.%d.%d.%d\n",channel_base[i].ep_reg->gv_tun.dns_v4[0],
			channel_base[i].ep_reg->gv_tun.dns_v4[1],channel_base[i].ep_reg->gv_tun.dns_v4[2],
			channel_base[i].ep_reg->gv_tun.dns_v4[3]);
		}
	}
#else  //打印到内核态中
	printk("gv-intr-reg:\n");
	printk("[%p]: ",&channel_base[0].pub_reg->gv_sw_int_reg);
	printk("%08x\n",channel_base[0].pub_reg->gv_sw_int_reg);
	printk("channel:\n");
	printk("		 address   ");
	printk("id  ");
	printk("magic    ");
	printk("flag	   ");
	printk("command	");
	printk("src_addr	");
	printk("dst_addr	");
	printk("size	   ");
	printk("checksum	");
	printk("irq_type	");
	printk("irq_number  ");
	printk("interrupts ");
	printk("direction \n");

	for(i=0;i<proc_channel_nums;i++)
	{
		printk("%p  ",channel_base[i].ep_reg);
		printk("%02x	",channel_base[i].channel_id);
		printk("%08x	",channel_base[i].ep_reg->magic);
		printk("%08x	",channel_base[i].ep_reg->Flag);
		printk("%08x	",channel_base[i].ep_reg->command);
		printk("%08llx  ",channel_base[i].ep_reg->src_addr);
		printk("%08llx  ",channel_base[i].ep_reg->dst_addr);
		printk("%08x	",channel_base[i].ep_reg->size);
		printk("%08x	",channel_base[i].ep_reg->checksum);
		printk("%08x	",channel_base[i].ep_reg->irq_type);
		printk("%08x	",channel_base[i].ep_reg->irq_number);
		printk("	%08lu	",channel_base[i].interrupts);
		
		if(i<NUM_READ_CHANNELS)
			printk("read\n");
		else
			printk("write\n");

	}

	for(i=0;i<proc_channel_nums;i++)
	{
		if(channel_base[i].ep_reg->Flag&CH_FLAG_TUN)
		{
			printk("CH%d->TUN-DEV:\n",channel_base[i].channel_id);
			printk("ipV4	: %d.%d.%d.%d\n",channel_base[i].ep_reg->gv_tun.ip_v4[0],
			channel_base[i].ep_reg->gv_tun.ip_v4[1],channel_base[i].ep_reg->gv_tun.ip_v4[2],
			channel_base[i].ep_reg->gv_tun.ip_v4[3]);
			printk("maskV4: %d.%d.%d.%d\n",channel_base[i].ep_reg->gv_tun.mask_v4[0],
			channel_base[i].ep_reg->gv_tun.mask_v4[1],channel_base[i].ep_reg->gv_tun.mask_v4[2],
			channel_base[i].ep_reg->gv_tun.mask_v4[3]);
			printk("gw_v4 : %d.%d.%d.%d\n",channel_base[i].ep_reg->gv_tun.gw_v4[0],
			channel_base[i].ep_reg->gv_tun.gw_v4[1],channel_base[i].ep_reg->gv_tun.gw_v4[2],
			channel_base[i].ep_reg->gv_tun.gw_v4[3]);
			printk("dns_v4: %d.%d.%d.%d\n",channel_base[i].ep_reg->gv_tun.dns_v4[0],
			channel_base[i].ep_reg->gv_tun.dns_v4[1],channel_base[i].ep_reg->gv_tun.dns_v4[2],
			channel_base[i].ep_reg->gv_tun.dns_v4[3]);
		}
	}
#endif


    return 0; 
} 



//static int gv9531_proc_open(struct inode *inode, struct file *file);

static int gv9531_proc_open(struct inode *inode, struct file *file)
{
	//return single_open(file, gv9531_proc_show, NULL);
	/*open 时,使用PDE_DATA(inode) =channel_base  作为私有数据向下传,保存再seq_file 的private 中.*/
	return single_open(file, gv9531_seq_show, PDE_DATA(inode));

	//int ret = seq_open(file, &gv9531_seq_fops); 失败
	//if(!ret) {
	//	 struct seq_file *sf = file->private_data; 
	//	 sf->private = PDE(inode); 
	//}
	//return ret;
}


static struct file_operations gv9531_proc_fops = {
	.owner	= THIS_MODULE,
	.open	= gv9531_proc_open,
	.release = single_release,
	//.read	= gv9531_proc_read,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.write 	= gv9531_proc_write,
};


//store different chip directory name.
char GV9531_PROC_DIR[MAX_CHIP_NUM][32];
//store file name.
char gv9531_name[32];
struct proc_chip_id_entry {
	struct proc_dir_entry *pci_gv9531_proc_dir;
	int chip_id;
};
//store chip directory struct proc_dir_entrt* & chip id.
struct proc_chip_id_entry proc_chip_id[MAX_CHIP_NUM];

int pci_gv9531_proc_init(char *proc_file_name,struct pci_epf_data_transfer *channel_base,int chip_id,int channel_nums)
{
	struct proc_dir_entry* file;
	
    printk("%s,chip id:%d\n",__func__,chip_id);
    sprintf(GV9531_PROC_DIR[chip_id],"gv9531.%d",chip_id);

	proc_chip_id[chip_id].chip_id = chip_id;
	proc_chip_id[chip_id].pci_gv9531_proc_dir = proc_mkdir(GV9531_PROC_DIR[chip_id], NULL);
	
	if (proc_chip_id[chip_id].pci_gv9531_proc_dir == NULL) {
        printk("%s proc create %s failed\n", __func__, GV9531_PROC_DIR[chip_id]);
        return -EINVAL;
    }

	strncpy(gv9531_name,proc_file_name,sizeof(gv9531_name));

	file = proc_create_data(gv9531_name, 0777, proc_chip_id[chip_id].pci_gv9531_proc_dir, &gv9531_proc_fops,channel_base);
	if (!file) {
        printk("%s proc_create failed!\n", __func__);
	    return -ENOMEM;
    }	

	proc_channel_nums=channel_nums;
	return 0;
}

void pci_gv9531_proc_exit(int chip_id)
{
	int i = 0;

    printk("%s,chip id:%d\n",__func__,chip_id);

	for(i=0;i<MAX_CHIP_NUM;i++)
	{	
		if(chip_id == proc_chip_id[i].chip_id )
		{
			remove_proc_entry(gv9531_name, proc_chip_id[i].pci_gv9531_proc_dir);
			remove_proc_entry(GV9531_PROC_DIR[chip_id], NULL);
			break;
		}
	}
}


