
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/slab.h>



#include "pci_data_channel.h"



static int gv9531_proc_show(struct seq_file *m, void *v);
static int gv9531_proc_open(struct inode *inode, struct file *file);
static ssize_t gv9531_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *f_pos);
//static int gv9531_proc_init(void);
//static void gv9531_proc_exit(void);



static struct pci_epf_data_transfer  *proc_data_channel=NULL;
static int proc_channel_nums=0;


static int gv9531_proc_show(struct seq_file *m, void *v)
{
	
	return 0;
}

static int gv9531_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, gv9531_proc_show, NULL);
}

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


ssize_t gv9531_proc_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    int len_copy, ret;
//  static char data[4096];
	static char data[8192];
    int data_len=0;
    
    int i;
    
    data_len+=sprintf(data+data_len,"gv-intr-reg:\n");
    data_len+=sprintf(data+data_len,"[%p]: ",&proc_data_channel[0].pub_reg->gv_sw_int_reg);
    data_len+=sprintf(data+data_len,"%08x\n",proc_data_channel[0].pub_reg->gv_sw_int_reg);
    
    
    data_len+=sprintf(data+data_len,"channel:\n");
    data_len+=sprintf(data+data_len,"        address   ");
    data_len+=sprintf(data+data_len,"id  ");
    data_len+=sprintf(data+data_len,"magic     ");
    data_len+=sprintf(data+data_len,"flag      ");
    data_len+=sprintf(data+data_len,"command   ");
    data_len+=sprintf(data+data_len,"src_addr  ");
    data_len+=sprintf(data+data_len,"dst_addr  ");
    data_len+=sprintf(data+data_len,"size      ");
    data_len+=sprintf(data+data_len,"checksum  ");
    data_len+=sprintf(data+data_len,"irq_type  ");
    data_len+=sprintf(data+data_len,"irq_number  ");
    data_len+=sprintf(data+data_len,"interrupts ");
    data_len+=sprintf(data+data_len,"direction \n");

    for(i=0;i<proc_channel_nums;i++)
    {
    	data_len+=sprintf(data+data_len,"%p  ",proc_data_channel[i].ep_reg);
    	data_len+=sprintf(data+data_len,"%02x  ",proc_data_channel[i].channel_id);
    	data_len+=sprintf(data+data_len,"%08x  ",proc_data_channel[i].ep_reg->magic);
    	data_len+=sprintf(data+data_len,"%08x  ",proc_data_channel[i].ep_reg->Flag);
    	data_len+=sprintf(data+data_len,"%08x  ",proc_data_channel[i].ep_reg->command);
    	data_len+=sprintf(data+data_len,"%08llx  ",proc_data_channel[i].ep_reg->src_addr);
    	data_len+=sprintf(data+data_len,"%08llx  ",proc_data_channel[i].ep_reg->dst_addr);
    	data_len+=sprintf(data+data_len,"%08x  ",proc_data_channel[i].ep_reg->size);
    	data_len+=sprintf(data+data_len,"%08x  ",proc_data_channel[i].ep_reg->checksum);
    	data_len+=sprintf(data+data_len,"%08x  ",proc_data_channel[i].ep_reg->irq_type);
    	data_len+=sprintf(data+data_len,"%08x  ",proc_data_channel[i].ep_reg->irq_number);
    	data_len+=sprintf(data+data_len,"  %08lu    ",proc_data_channel[i].interrupts);
    	
    	if(i<NUM_READ_CHANNELS)
    		//data_len+=sprintf(data+data_len,"read\n",proc_data_channel[i].interrupts);
			data_len+=sprintf(data+data_len,"read\n");
    	else
    		data_len+=sprintf(data+data_len,"write\n");

    }

	/*
	data_len+=sprintf(data+data_len,"fifo:\nid ");
    data_len+=sprintf(data+data_len,"size      ");
    data_len+=sprintf(data+data_len,"len       \n");
    for(i=0;i<proc_channel_nums;i++)
    {
    	data_len+=sprintf(data+data_len,"%02d ",proc_data_channel[i].channel_id);
    	//data_len+=sprintf(data+data_len,"%08d ",kfifo_size(&proc_data_channel[i].data_fifo));
    	//data_len+=sprintf(data+data_len,"%08d\n",kfifo_len(&proc_data_channel[i].data_fifo));
    }
	*/
    for(i=0;i<proc_channel_nums;i++)
    {
    	if(proc_data_channel[i].ep_reg->Flag&CH_FLAG_TUN)
    	{
    		data_len+=sprintf(data+data_len,"CH%d->TUN-DEV:\n",proc_data_channel[i].channel_id);
    		data_len+=sprintf(data+data_len,"ipV4  : %d.%d.%d.%d\n",proc_data_channel[i].ep_reg->gv_tun.ip_v4[0],
    		proc_data_channel[i].ep_reg->gv_tun.ip_v4[1],proc_data_channel[i].ep_reg->gv_tun.ip_v4[2],
    		proc_data_channel[i].ep_reg->gv_tun.ip_v4[3]);
    		data_len+=sprintf(data+data_len,"maskV4: %d.%d.%d.%d\n",proc_data_channel[i].ep_reg->gv_tun.mask_v4[0],
    		proc_data_channel[i].ep_reg->gv_tun.mask_v4[1],proc_data_channel[i].ep_reg->gv_tun.mask_v4[2],
    		proc_data_channel[i].ep_reg->gv_tun.mask_v4[3]);
    		data_len+=sprintf(data+data_len,"gw_v4 : %d.%d.%d.%d\n",proc_data_channel[i].ep_reg->gv_tun.gw_v4[0],
    		proc_data_channel[i].ep_reg->gv_tun.gw_v4[1],proc_data_channel[i].ep_reg->gv_tun.gw_v4[2],
    		proc_data_channel[i].ep_reg->gv_tun.gw_v4[3]);
    		data_len+=sprintf(data+data_len,"dns_v4: %d.%d.%d.%d\n",proc_data_channel[i].ep_reg->gv_tun.dns_v4[0],
    		proc_data_channel[i].ep_reg->gv_tun.dns_v4[1],proc_data_channel[i].ep_reg->gv_tun.dns_v4[2],
    		proc_data_channel[i].ep_reg->gv_tun.dns_v4[3]);
    	}
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

static struct file_operations gv9531_proc_fops = {
	.owner	= THIS_MODULE,
	.open	= gv9531_proc_open,
	.release = single_release,
	.read	= gv9531_proc_read,
	.llseek	= seq_lseek,
	.write 	= gv9531_proc_write,
};




//文件节点名称
#define MAX_CHIP_NUM 32

char GV9531_PROC_DIR[MAX_CHIP_NUM][32];
char gv9531_name[32];

struct proc_chip_id_entry {
	struct proc_dir_entry *pci_gv9531_proc_dir;
	int chip_id;
};
struct proc_chip_id_entry proc_chip_id[MAX_CHIP_NUM];

int pci_gv9531_proc_init(char *proc_file_name,struct pci_epf_data_transfer  *channel_base,int chip_id,int channel_nums)
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
	file = proc_create(gv9531_name, 0777, proc_chip_id[chip_id].pci_gv9531_proc_dir, &gv9531_proc_fops);
	if (!file) {
        printk("%s proc_create failed!\n", __func__);
	    return -ENOMEM;
    }
    
	proc_data_channel=channel_base;
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


