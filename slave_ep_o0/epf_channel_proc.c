
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/slab.h>



#include "epf_data_channel.h"


#define GV9531_PROC_DIR     "gv9531"


static int __attribute__((optimize("-O0"))) gv9531_proc_show(struct seq_file *m, void *v);
static int  __attribute__((optimize("-O0"))) gv9531_proc_open(struct inode *inode, struct file *file);
//static ssize_t __attribute__((optimize("-O0"))) gv9531_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *f_pos);
int __attribute__((optimize("-O0"))) gv9531_proc_init(void);
void __attribute__((optimize("-O0"))) gv9531_proc_exit(void);



static struct epf_data_transfer  *proc_data_channel=NULL;
static int proc_channel_nums=0;


static int __attribute__((optimize("-O0"))) gv9531_proc_show(struct seq_file *m, void *v)
{
    int i;
    
    seq_printf(m,"sw-intr-reg[%p]:",&proc_data_channel[0].epf_gv953x->pub_reg->gv_sw_int_reg);
    seq_printf(m,"%08x\n",proc_data_channel[0].epf_gv953x->pub_reg->gv_sw_int_reg);
    
    seq_printf(m,"channel:\n");
    seq_printf(m,"        address   ");
    seq_printf(m,"id  ");
    seq_printf(m,"magic     ");
    seq_printf(m,"Flag      ");
    seq_printf(m,"command   ");
    seq_printf(m,"src_addr  ");
    seq_printf(m,"dst_addr  ");
    seq_printf(m,"size      ");
    seq_printf(m,"checksum  ");
    seq_printf(m,"dma-ch    ");
    seq_printf(m,"dma-st    ");
    seq_printf(m,"irq_type  ");
    seq_printf(m,"irq_number  ");
    seq_printf(m,"write_magic_cnt  ");
    seq_printf(m,"read_magic_cnt ");
    seq_printf(m,"direction \n");

    for(i=0;i<proc_channel_nums;i++)
    {
    	seq_printf(m,"%p  ",proc_data_channel[i].func_reg);
    	seq_printf(m,"%02x  ",proc_data_channel[i].channel_id);
    	seq_printf(m,"%08x  ",proc_data_channel[i].func_reg->magic);
    	seq_printf(m,"%08x  ",proc_data_channel[i].func_reg->Flag);
    	seq_printf(m,"%08x  ",proc_data_channel[i].func_reg->command);
    	seq_printf(m,"%llu  ",proc_data_channel[i].func_reg->src_addr);
    	seq_printf(m,"%llu  ",proc_data_channel[i].func_reg->dst_addr);
    	seq_printf(m,"%08x  ",proc_data_channel[i].func_reg->size);
    	seq_printf(m,"%08x  ",proc_data_channel[i].func_reg->checksum);
    	seq_printf(m,"%08x  ",proc_data_channel[i].dma_ch);
    	seq_printf(m,"%08x  ",proc_data_channel[i].dma_status);
    	seq_printf(m,"%08x  ",proc_data_channel[i].func_reg->irq_type);
    	seq_printf(m,"%08x    ",proc_data_channel[i].func_reg->irq_number);
    	seq_printf(m,"%08d    ",proc_data_channel[i].write_magic_cnt);
    	seq_printf(m,"%08d    ",proc_data_channel[i].read_magic_cnt);
    	
    	if(i<EPF_NUM_WRITE_CHANNELS)
    		seq_printf(m,"write\n");
    	else
    		seq_printf(m,"read\n");

    }
    
	seq_printf(m,"fifo:\nid ");
    seq_printf(m,"size      ");
    seq_printf(m,"len       ");
	seq_printf(m,"avail       ");
	seq_printf(m,"Flag       ");	
	seq_printf(m,"func_reg->size       \n");

    for(i=0;i<proc_channel_nums;i++)
    {
		seq_printf(m,"%02d ",proc_data_channel[i].channel_id);
		seq_printf(m,"%08d ",kfifo_size(&proc_data_channel[i].data_fifo));
		seq_printf(m,"%08d ",kfifo_len(&proc_data_channel[i].data_fifo));
		seq_printf(m,"%08d ",kfifo_avail(&proc_data_channel[i].data_fifo));
		seq_printf(m,"%08x  ",proc_data_channel[i].func_reg->Flag);
		seq_printf(m,"%08x\n",proc_data_channel[i].func_reg->size);
    }
    
    for(i=0;i<proc_channel_nums;i++)
    {
    	if(proc_data_channel[i].func_reg->Flag&CH_FLAG_TUN)
    	{
    		seq_printf(m,"CH%d->TUN-DEV:\n",proc_data_channel[i].channel_id);
    		seq_printf(m,"\tipV4  : %d.%d.%d.%d\n",proc_data_channel[i].func_reg->gv_tun.ip_v4[0],
    		proc_data_channel[i].func_reg->gv_tun.ip_v4[1],proc_data_channel[i].func_reg->gv_tun.ip_v4[2],
    		proc_data_channel[i].func_reg->gv_tun.ip_v4[3]);
    		seq_printf(m,"\tmaskV4: %d.%d.%d.%d\n",proc_data_channel[i].func_reg->gv_tun.mask_v4[0],
    		proc_data_channel[i].func_reg->gv_tun.mask_v4[1],proc_data_channel[i].func_reg->gv_tun.mask_v4[2],
    		proc_data_channel[i].func_reg->gv_tun.mask_v4[3]);
    		seq_printf(m,"\tgw_v4 : %d.%d.%d.%d\n",proc_data_channel[i].func_reg->gv_tun.gw_v4[0],
    		proc_data_channel[i].func_reg->gv_tun.gw_v4[1],proc_data_channel[i].func_reg->gv_tun.gw_v4[2],
    		proc_data_channel[i].func_reg->gv_tun.gw_v4[3]);
    		seq_printf(m,"\tdns_v4: %d.%d.%d.%d\n",proc_data_channel[i].func_reg->gv_tun.dns_v4[0],
    		proc_data_channel[i].func_reg->gv_tun.dns_v4[1],proc_data_channel[i].func_reg->gv_tun.dns_v4[2],
    		proc_data_channel[i].func_reg->gv_tun.dns_v4[3]);
    	}
    }
	return 0;
}

static int __attribute__((optimize("-O0"))) gv9531_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, gv9531_proc_show, NULL);
}

#if 0
static ssize_t __attribute__((optimize("-O0"))) gv9531_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *f_pos)
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
#endif

ssize_t __attribute__((optimize("-O0"))) gv9531_proc_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    int len_copy, ret;
    static char data[4096];
    int data_len=0;
    
    int i;
    data_len+=sprintf(data+data_len,"channel:\n");
    data_len+=sprintf(data+data_len,"        address   ");
    data_len+=sprintf(data+data_len,"id  ");
    data_len+=sprintf(data+data_len,"magic     ");
    data_len+=sprintf(data+data_len,"command   ");
    data_len+=sprintf(data+data_len,"src_addr  ");
    data_len+=sprintf(data+data_len,"dst_addr  ");
    data_len+=sprintf(data+data_len,"size      ");
    data_len+=sprintf(data+data_len,"checksum  ");
    data_len+=sprintf(data+data_len,"irq_type  ");
    data_len+=sprintf(data+data_len,"irq_number  ");
    //data_len+=sprintf(data+data_len,"interrupts ");
    data_len+=sprintf(data+data_len,"write_magic_cnt ");
    data_len+=sprintf(data+data_len,"read_magic_cnt  ");
    data_len+=sprintf(data+data_len,"direction \n");

    for(i=0;i<proc_channel_nums;i++)
    {
    	data_len+=sprintf(data+data_len,"%p  ",proc_data_channel[i].func_reg);
    	data_len+=sprintf(data+data_len,"%02x  ",proc_data_channel[i].channel_id);
    	data_len+=sprintf(data+data_len,"%08x  ",proc_data_channel[i].func_reg->magic);
    	data_len+=sprintf(data+data_len,"%08x  ",proc_data_channel[i].func_reg->command);
    	data_len+=sprintf(data+data_len,"%llu  ",proc_data_channel[i].func_reg->src_addr);
    	data_len+=sprintf(data+data_len,"%llu  ",proc_data_channel[i].func_reg->dst_addr);
    	data_len+=sprintf(data+data_len,"%08x  ",proc_data_channel[i].func_reg->size);
    	data_len+=sprintf(data+data_len,"%08x  ",proc_data_channel[i].func_reg->checksum);
    	data_len+=sprintf(data+data_len,"%08x  ",proc_data_channel[i].func_reg->irq_type);
    	data_len+=sprintf(data+data_len,"%08x  ",proc_data_channel[i].func_reg->irq_number);
    	//data_len+=sprintf(data+data_len,"  %0d          ",proc_data_channel[i].interrupts);
    	data_len+=sprintf(data+data_len,"  %0d          ",proc_data_channel[i].write_magic_cnt);
    	data_len+=sprintf(data+data_len,"  %0d          ",proc_data_channel[i].read_magic_cnt);
    	
    	if(i<EPF_NUM_WRITE_CHANNELS)
    		data_len+=sprintf(data+data_len,"write\n");
    	else
    		data_len+=sprintf(data+data_len,"read\n");

    }
    
// fl->f_pos表示当前文件描述符对文件的偏移, len表示用户进程想要读的大小
    if ((file->f_pos + len) > data_len) //如果剩下没读的数据长度少于len,则只复制出剩下没读部分
            len_copy = data_len - file->f_pos;
    else
            len_copy = len; //如果剩下的数据长度超出len,则本次复制len字节

    ret = copy_to_user(buf, data+file->f_pos, len_copy);
    //内容复制后，需要改变文件描述符的位置偏移
    *off += len_copy - ret;  //在read/write函数里必须通过off来改变
    return len_copy - ret;
}

#if 0
static struct file_operations gv9531_proc_fops = {
	.owner	= THIS_MODULE,
	.open	= gv9531_proc_open,
	.release = single_release,
	.read	= gv9531_proc_read,
	.llseek	= seq_lseek,
	.write 	= gv9531_proc_write,
};
#else
static struct file_operations gv9531_proc_fops = {
        .open           = gv9531_proc_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

//文件节点名称
char gv9531_name[20];
struct proc_dir_entry *pci_gv9531_proc_dir = NULL;

int __attribute__((optimize("-O0"))) pci_gv9531_proc_init(char *proc_file_name,struct epf_data_transfer  *channel_base,int channel_nums)
{
	struct proc_dir_entry* file;
	
	strncpy(gv9531_name,proc_file_name,sizeof(gv9531_name));
	proc_data_channel=channel_base;
	proc_channel_nums=channel_nums;
	    
    pci_gv9531_proc_dir = proc_mkdir(GV9531_PROC_DIR, NULL);
    if (pci_gv9531_proc_dir == NULL) {
        printk("%s proc create %s failed\n", __func__, GV9531_PROC_DIR);
        return -EINVAL;
    }

	file = proc_create(gv9531_name, 0777, pci_gv9531_proc_dir, &gv9531_proc_fops);
	if (!file) {
        printk("%s proc_create failed!\n", __func__);
	    return -ENOMEM;
    }
    
	return 0;
}

void __attribute__((optimize("-O0"))) pci_gv9531_proc_exit(void)
{
	remove_proc_entry(gv9531_name, pci_gv9531_proc_dir);
	remove_proc_entry(GV9531_PROC_DIR, NULL);
}



//
//static int __init my_init(void)
//{
//    pci_gv9531_proc_init("test_proc",NULL,0);
//	return 0;
//}
//
//static void __exit my_exit(void)
//{
//    pci_gv9531_proc_exit();
//}
//
//module_init(my_init);
//module_exit(my_exit);
//
//MODULE_AUTHOR("WXY");
//MODULE_LICENSE("GPL");
