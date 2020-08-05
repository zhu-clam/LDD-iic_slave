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

#define  DEBUG
#ifdef   DEBUG
#define print_debug printk
#else
static inline void print_debug(const char *fmt, ...)
{
	(void)fmt;
}
#endif

//#define  S2C 

#define GVSP_CORE 4 
  
#define DSP_CLOCK_ADDR          0xf9700208  // clock reg addr   
#define DSP_RESET_ADDR 		    0xf9700818  //reset reg addr

#define DSP_C5A_INTL_ADDR		0xf5809000  //intc reg addr
#define DSP_P6A_INTL_ADDR		0xf5809400  //intc reg addr
#define DSP_P6B_INTL_ADDR		0xf5809800  //intc reg addr
#define DSP_C5B_INTL_ADDR		0xf5809c00  //intc reg addr

#define IOREMAP_SIZE 		1024
#define DSP_INTC_OFFSET     (0x50)
#define DSP_RELEASE_BIT  (0xffffff80)


#define INTC_MASK (0x1 << 0)
//reset bit
#define C5A_SW_RST_BIT   0
#define P6A_SW_RST_BIT   2
#define P6B_SW_RST_BIT   3
#define C5B_SW_RST_BIT   1

#define MAX_IOC		        (4)
#define P6A_IOC_MAGIC	    ('A')
#define P6B_IOC_MAGIC	    ('B')
#define C5A_IOC_MAGIC       ('C')
#define C5B_IOC_MAGIC       ('D')


/*
  *  !brief //@	0: reset cnn the cnn engine;
  *  !brief //@	1:block the cnn engine;
  *  !brief //@ 2:enable the cnn engine;
  *  !brief //@ 3:disable the cnn engine;
   */
 typedef enum _cmd_id_e
{
	ARCH_RESET = 0,
	ARCH_BLOCK,
	ARCH_ENABLE,
	ARCH_DISABLE,
} cmd_id;

 typedef enum _core_id
{
	C5A = 0, 
	P6A,
	P6B,
	C5B,
} core_id;

static DEFINE_SPINLOCK(gvsp_Lock);

typedef struct{
	dev_t   devID;
	int  ev_press;
	int  reset_index;
	void *reg_reset;
	void *reg_intl;
	wait_queue_head_t wait_queue;
	struct class *pClass;
	struct device *pdev;
	struct cdev  *pcdev;
} gvsp_data;

static  gvsp_data  gvsp_dev[GVSP_CORE] = {0};

static char *devname[GVSP_CORE] = {
 "c5a",
 "p6a",
 "p6b",
 "c5b",
};

static void *clock_addr = NULL;
static struct proc_dir_entry *proc_show_entry ,*proc_dir;
static phys_addr_t  int_addr[GVSP_CORE] = {
	DSP_C5A_INTL_ADDR	,	//0xf5809000 intc reg addr
	DSP_P6A_INTL_ADDR	,	//0xf5809400 intc reg addr
	DSP_P6B_INTL_ADDR	,	//0xf5809800 intc reg addr
	DSP_C5B_INTL_ADDR	,	//0xf5809c00 intc reg addr
};

static int gvsp_proc_show(struct seq_file *m, void *v)
{
	u32 val = 0,i;
    seq_printf(m, "gvsp v0.01\n");
	if (gvsp_dev[C5A].reg_reset)
	{
		val = readl(gvsp_dev[C5A].reg_reset);
	}
	seq_printf(m,"reset register value 0x%x\n",val);
	seq_printf(m,"load addr  C5A [0]  P6A  [1]   P6B  [2] C5B [3] \n");
	for ( i = 0; i < GVSP_CORE; i++)
	{
		seq_printf(m,"index [%d] 0x%x  0x%x\n",i,readl(gvsp_dev[i].reg_intl), readl(gvsp_dev[i].reg_intl + 4));
	}
    return 0;
}

static int gvsp_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, gvsp_proc_show, NULL);
}

static const struct file_operations gvsp_proc_fops = {
    .owner = THIS_MODULE,
    .open = gvsp_proc_open,
    .read = seq_read,
    .release = single_release,
};

static int proc_create_entry(void)
{
	print_debug(KERN_ALERT"proc initialized.\n");
	/*
	* struct proc_dir_entry *proc_mkdir(const char* name,struct proc_dir_entry *parent);
	* 该函数用于创建一个proc 目录,参数name 指定要创建的proc 目录名,参数 parent 为该proc 目录所在的目录,NULL 表示在/proc/下面创建.
	*/
	proc_dir = proc_mkdir("gvsp", NULL);//效果：在/proc 下面生成/proc/gvsp 的目录
	if (proc_dir == 0)
    {
		print_debug(KERN_ALERT"proc fail 1.\n");
		goto error;
	}
	
	/*
	*	struct proc_dir_entry *proc_create(const char *name,umode_t mode, 
		struct proc_dir_entry *parent, const struct file_operations *proc_fops);
	*   该函数用于创建proc 条目.其中 name 为文件名称, mode 为文件权限, parent 为文件的父目录指针
	*	proc_fops 为 文件操作结构体指针.
	*/
	proc_show_entry =  proc_create("gvsp_show", 0644,proc_dir, &gvsp_proc_fops);//效果：在/proc/gvsp 下面生成/proc/gvsp/gvsp_show 的条目
    if (proc_show_entry == 0)
    {
		print_debug(KERN_ALERT"proc fail 2.\n");
		goto error;
    }
    return 0;
error :
	if(proc_show_entry)
	{
		proc_remove(proc_show_entry);
	}
	if(proc_dir)
	{
		proc_remove(proc_dir);
	}
    return -1;
}
static int proc_remove_entry(void)
{
	if(proc_show_entry)
	{
		proc_remove(proc_show_entry);
	}
	if(proc_dir)
	{
		proc_remove(proc_dir);
	}
    return 0;
}



void gvsp_ioreset(void *addr,unsigned int index)
{
	/*0xf9700818 bit2 set to 0 */
    u32 reg,va1;
	reg = readl(addr);
	print_debug("before  IOCRESET:%#x index %d\n", reg,index);	
	va1 = (0x1 << index);
	if (reg & va1 ) //fff 
	{
		reg &= ~va1;	
		writel(reg,addr);//ffb
		print_debug("middle IOCRESET:%#x\n", readl(addr));
		reg |= va1; 
		writel(reg,addr);//fff
	}
	else 
	{ 
		reg |= va1;//FFB ->FFF		
		writel(reg,addr);
	}
	print_debug("after  IOCRESET:%#x\n", readl(addr));

}


void gvsp_ioenable(void *addr,unsigned int index)
{
	u32 reg,val;
	val = (0x1 << index);
	reg = readl(addr);
	print_debug("before  IOENABLE:%#x index %d\n", reg,index);	
	if((reg & val) != 0)//p6a is not start;———— fff->ffb->fff
	{	
		reg &= ~val;
		writel(reg,addr);//ffb
		print_debug("middle  IOCENABLE:%#x\n", readl(addr));
		reg |= val; 
		writel(reg,addr);//fff
		print_debug("after IOCENABLE:%#x\n", readl(addr));
	}

}

void gvsp_iodisable(void *addr, unsigned int index)
{
	u32 reg,val;
	val = (0x1 << index);
	reg = readl(addr);	
	print_debug("before  IODISABLE:%#x index %d\n", reg,index);	
	reg &= ~val; //bit2 set to 0
	writel(reg,addr);//ffb
	print_debug("after IOCDISABLE :%#x\n", readl(addr));
}

static int gvsp_open(struct inode *inode, struct file *filp)
{
	print_debug("gvsp OPEN!\n");
	return 0;
}
static int gvsp_release(struct inode *inode, struct file *file)
{
	print_debug("gvsp release\n");
	return 0;
}
static ssize_t gvsp_read (struct file *file, char *buff, size_t count, loff_t *offp)
{
	ssize_t result ,i = 0;
	u32 val;
	if(count != sizeof(u32))
	{
		print_debug(KERN_ERR"size error \n");	
		return 0;
	}
	for (i = 0; i < GVSP_CORE ; i++)
	{
		print_debug("%s ioctl dev devID  %d \n",__func__,gvsp_dev[i].devID);
		if (file->f_inode->i_cdev->dev == gvsp_dev[i].devID)
		{
			break;
		}
	}
	if(i >= GVSP_CORE )
	{
		print_debug(KERN_ERR"error \n");	
		return 0;
	}
	val  = readl(gvsp_dev[i].reg_intl + 4);
	if (copy_to_user (buff, &val, count))
	{
		result = -EFAULT;
	}
	else
	{
		print_debug (KERN_INFO "wrote %d bytes\n", (int)count);
		result = count;
	}
	return result;
}
ssize_t gvsp_write (struct file *file, const char *buf, size_t count, loff_t *f_pos)
{
	ssize_t ret = 0,i;
	u32 val = 0;
	if(count != sizeof(u32))
	{
		print_debug(KERN_ERR"error \n");	
		return 0;
	}
	for (i = 0; i < GVSP_CORE ; i++)
	{
		print_debug("%s ioctl dev devID  %d \n",__func__,gvsp_dev[i].devID);
		if (file->f_inode->i_cdev->dev == gvsp_dev[i].devID)
		{
			break;
		}
	}
	if(i >= GVSP_CORE )
	{
		print_debug("error \n");	
		return 0;
	}
	if (copy_from_user (&val, buf, count)) {
		ret = -EFAULT;
	}
	else {
		print_debug(KERN_INFO"Received: 0x%x\n", val);
		ret = count;
	}
	//#set DSP altenate reset vector
	writel(0x1,gvsp_dev[i].reg_intl);
	writel(val,gvsp_dev[i].reg_intl + 4);
	return ret;
}

static long gvsp_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int i ;
	print_debug("%s ioctl dev  %d  cmd index %d\n",__func__,file->f_inode->i_cdev->dev,_IOC_NR (cmd));
	
	for (i = 0; i < GVSP_CORE ; i++)
	{
		if (file->f_inode->i_cdev->dev == gvsp_dev[i].devID) //根据打开的设备文件之主设备号 遍历找到对应的 设备.
		{
			break;
		}
	}
	
	if(i >= GVSP_CORE )
	{
		print_debug("error \n");	
		return 0;
	}
	switch(_IOC_NR (cmd)) //通过宏_IOC_NR 获取cmd 的NR 号.
	{
		case ARCH_RESET:
			spin_lock(&gvsp_Lock);
			gvsp_ioreset(gvsp_dev[i].reg_reset,gvsp_dev[i].reset_index);
			spin_unlock(&gvsp_Lock);
		break;
			
		case ARCH_BLOCK:
			/*put process into wait queue*/
			print_debug("[waitq]%s Start  wait queue,ev_press = %d  core id %s \n",__func__,gvsp_dev[i].ev_press,devname[i]);
			wait_event_interruptible(gvsp_dev[i].wait_queue,gvsp_dev[i].ev_press);
			print_debug("Wake up %s from the sleep status",devname[i]);
			gvsp_dev[i].ev_press = 0;
		break;
			
		case ARCH_ENABLE:			
			spin_lock(&gvsp_Lock);
			gvsp_ioenable(gvsp_dev[i].reg_reset,gvsp_dev[i].reset_index);
			spin_unlock(&gvsp_Lock);
		break;
	
		case ARCH_DISABLE:			
			spin_lock(&gvsp_Lock);
			gvsp_iodisable(gvsp_dev[i].reg_reset,gvsp_dev[i].reset_index);
			print_debug(KERN_ALERT"%s %d core_id %d \n",__FUNCTION__,__LINE__,C5A);
			spin_unlock(&gvsp_Lock);
		break;
	
		default:
			print_debug("%s,Don't support cmd [%d]\n",__func__,cmd);
		break;
	}
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

static irqreturn_t c5a_interrupt(int irq, void *dev)
{
	u32 intc_val;
	intc_val = readl(gvsp_dev[C5A].reg_intl + DSP_INTC_OFFSET);
	//intc_val &= ~(0x1<<0);
	intc_val &= ~INTC_MASK;
           
	writel(intc_val, gvsp_dev[C5A].reg_intl + DSP_INTC_OFFSET);
	/*wake up process whose in wait queue*/
	gvsp_dev[C5A].ev_press = 1;
	wake_up_interruptible(&gvsp_dev[C5A].wait_queue);
	print_debug("receive c5a interrupt %s, ev_press = %d \n",__func__,gvsp_dev[C5A].ev_press);
	return IRQ_HANDLED;
}
static irqreturn_t p6a_interrupt(int irq, void *dev)
{
	u32 intc_val;
	intc_val = readl(gvsp_dev[P6A].reg_intl + DSP_INTC_OFFSET);
	intc_val &= ~(0x1<<0);
	writel(intc_val, gvsp_dev[P6A].reg_intl + DSP_INTC_OFFSET);
	/*wake up process whose in wait queue*/
	gvsp_dev[P6A].ev_press = 1;
	wake_up_interruptible(&gvsp_dev[P6A].wait_queue);
	print_debug("receive p6a interrupt %s, ev_press = %d \n",__func__,gvsp_dev[P6A].ev_press);
	return IRQ_HANDLED;
}
static irqreturn_t p6b_interrupt(int irq, void *dev)
{
	u32 intc_val;
	intc_val = readl(gvsp_dev[P6B].reg_intl + DSP_INTC_OFFSET);
	intc_val  &= ~(0x1<<0);
	writel(intc_val, gvsp_dev[P6B].reg_intl + DSP_INTC_OFFSET);
	gvsp_dev[P6B].ev_press = 1;
	wake_up_interruptible(&gvsp_dev[P6B].wait_queue);
	print_debug("receive p6b interrupt! %s, ev_press = %d \n",__func__,gvsp_dev[P6B].ev_press);

	return IRQ_HANDLED;
}
static irqreturn_t c5b_interrupt(int irq, void *dev)
{
	u32 intc_val; 
	intc_val = readl(gvsp_dev[C5B].reg_intl + DSP_INTC_OFFSET);
	intc_val &= ~(0x1<<0);
	writel(intc_val, gvsp_dev[C5B].reg_intl + DSP_INTC_OFFSET);
	gvsp_dev[C5B].ev_press = 1;
	wake_up_interruptible(&gvsp_dev[C5B].wait_queue);
	print_debug("receive c5b interrupt! %s, ev_press = %d \n",__func__,gvsp_dev[C5B].ev_press);
	return IRQ_HANDLED;
}

//函数指针数组.
// irqreturn_t xxx_interrupt (int irq, void *);
//typedef irqreturn_t (*irq_handler_t)(int, void *); //
//irqreturn_t (*irq_handler_t)(int, void *);  定义一个名为：irq_handler_t 的函数指针变量. 
//用 typedef 定义了一个函数指针类型irq_handler_t,指向的函数原型返回类型为 irqreturn_t
//它接收的参数类型就是int 和void* 两个参数
static irq_handler_t irq_function[GVSP_CORE] = 
{
	c5a_interrupt,
	p6a_interrupt,
	p6b_interrupt,
	c5b_interrupt,
};


static int gvsp_MAJOR = 0;
#define DRIVER_NAME "gvsp_dsp"
struct cdev dsp_cdev; 
static struct class *dsp_class;
/*字符设备注册、设备文件创建函数. */
static int gvsp_dev_init(void)
{
    int i,ret;
//============= 创建 /sys/class/gvsp 类 ===================	
	gvsp_dev[C5A].pClass = class_create(THIS_MODULE, "gvsp");
	for(i = 1; i < GVSP_CORE; i ++)
	{
		gvsp_dev[i].pClass = gvsp_dev[C5A].pClass;
 	}
	
//==========  动态分配设备号 =============================	
#if ZXF_MODIFY		
	dev_t dev_no;
	ret = alloc_chrdev_region(&dev_no, 0, GVSP_CORE, DRIVER_NAME);
	if(ret < 0 )
	{
		print_debug(KERN_ALERT"fail to allocate chrdev.\n");
		return ret;
	}
	
	gvsp_MAJOR =  MAJOR(devno); 	// 获取动态申请的主设备号.
	cdev_init(&cdev, &dev_fops);
	cdev.owner = THIS_MODULE;
	cdev_add(&cdev, devno, GVSP_CORE);//利用之前向内核动态申请设备号，再此正式申请。
	
//========= 创建字符设备文件.	==========================
	dsp_class = class_create(THIS_MODULE, DRIVER_NAME);
	if (IS_ERR(dsp_class))
	{
	    ret = PTR_ERR(dsp_class);
	    pr_err("[%s] create class failed\n", __func__);
        goto fail_class;
    }
	
	for (i = 0; i < GVSP_CORE; i++)
	{
		gvsp_dev[i].pdev = device_create( dsp_class, NULL, MKDEV(gvsp_MAJOR, i), NULL, devname[i]);
		if (IS_ERR( gvsp_dev[i].pdev))
		{
			print_debug(KERN_ALERT"device create fail, error code(%ld).\n", PTR_ERR( gvsp_dev[i].pdev));
			goto fail_dev;
		}				

		init_waitqueue_head(& gvsp_dev[i].wait_queue);
		switch(i)
		{
			case P6A:
				gvsp_dev[i].reset_index = P6A_SW_RST_BIT;
				break;
			case P6B:
				gvsp_dev[i].reset_index = P6B_SW_RST_BIT;
				break;
			case C5A:
				gvsp_dev[i].reset_index = C5A_SW_RST_BIT;
				break;
			case C5B:
				gvsp_dev[i].reset_index = C5B_SW_RST_BIT;
				break;				
		}
	}
#endif

#if ORIGIN_
	for (i = 0; i < GVSP_CORE; i++)
	{
		
	//	ret = alloc_chrdev_region(&gvsp_dev[i].devID, 0, 1, devname[i]);
	//	if(ret)
	//	{
	//		print_debug(KERN_ALERT"fail to allocate chrdev.\n");
	//		return ret;
	//	}
		 /*--2--*/
		gvsp_dev[i].pcdev = cdev_alloc();
		if(gvsp_dev[i].pcdev == NULL)
		{
			print_debug(KERN_ALERT"gvsp_cdev alloc fail.\n");  
			goto error;
		}
		gvsp_dev[i].pcdev->ops = &dev_fops;
		
		/*--3--*/
		cdev_init(gvsp_dev[i].pcdev, &dev_fops);
		gvsp_dev[i].pcdev->owner = THIS_MODULE;
		
		/*--4--*/
		ret = cdev_add(gvsp_dev[i].pcdev, gvsp_dev[i].devID, 1);
		if(ret)
		{
			print_debug(KERN_ALERT"fail to cdev_add core_id %d\n",i);
			goto error;
		} 
		
		
		gvsp_dev[i].pdev = device_create( gvsp_dev[i].pClass, NULL, gvsp_dev[i].devID, NULL, devname[i]);
		if (IS_ERR( gvsp_dev[i].pdev))
		{
			print_debug(KERN_ALERT"device create fail, error code(%ld).\n", PTR_ERR( gvsp_dev[i].pdev));
			goto error;
		}
		init_waitqueue_head(& gvsp_dev[i].wait_queue);
		if ( i  ==  C5A)
		{
			gvsp_dev[i].reset_index = C5A_SW_RST_BIT;
		}
		else if (i == P6A)
		{
			gvsp_dev[i].reset_index = P6A_SW_RST_BIT;
		}
		else if (i == P6B)
		{
			gvsp_dev[i].reset_index = P6B_SW_RST_BIT;
		}
		else if (i == C5B)
		{
			gvsp_dev[i].reset_index = C5B_SW_RST_BIT;
		}
	}
#endif /*endif ORIGIN*/
    return 1;
error:
	for(i = 0 ; i < GVSP_CORE; i++)
	{
		if ( gvsp_dev[i].pdev)
		{
			device_destroy(gvsp_dev[i].pClass, gvsp_dev[i].devID);
			gvsp_dev[i].pdev = NULL;
		}
	}
	
	if ( gvsp_dev[C5A].pClass)
    {
		class_destroy(gvsp_dev[C5A].pClass);
		gvsp_dev[C5A].pClass = NULL;
	}
	for( i = 0 ; i < GVSP_CORE; i++)
	{
		if(gvsp_dev[i].pcdev)
		{
			cdev_del( gvsp_dev[i].pcdev);	
		}
		if(gvsp_dev[i].devID)
		{
			unregister_chrdev_region(gvsp_dev[i].devID, 1);
		}
	}
	return -1; 
	
#define ZXF_MODIFY
fail_dev:    
    for (i=0;i < GVSP_CORE ; i++)
        device_destroy(dsp_class, MKDEV(gvsp_MAJOR, i));//注销设备文件
    class_destroy(dsp_class); //注销dsp_class 类
fail_class:
    cdev_del(&cdev);//注销cdev驱动字符设备驱动
    unregister_chrdev_region(MKDEV(gvsp_MAJOR, 0), GVSP_CORE);//注销设备号
    return ret;

	#思想： 注册字符设备 vs 创建设备文件 两者需分离开;
	#		字符设备 与之对应的 设备文件 通过设备号进行绑定[注册&注销].
	#注册时顺序： 
	#		1. (动态\静态)分配设备号	 
	#		2. 注册字符设备
	#		3. 创建设备文件[先创建设备类 class_create、再创建设备文件 device_create];
	#注销时顺序： 1.		
	#		1. destory销毁设备文件:device_destroy(dsp_class, MKDEV(gvsp_MAJOR, i));//有多少从设备文件需调用销毁函数多少次
	#		2. destory销毁设备类:class_destroy(dsp_class);
	#		3. 注销字符设备 cdev_del(&cdev);
	#		4. 注销设备号	 unregister_chrdev_region(MKDEV(gvsp_MAJOR, 0), GVSP_CORE);//一次就够.
	for(i = 0 ; i < GVSP_CORE; i++)
	{
		if ( gvsp_dev[i].pdev)
		{
			device_destroy(gvsp_dev[i].pClass, gvsp_dev[i].devID);
			gvsp_dev[i].pdev = NULL;
		}
	}
	
	if ( gvsp_dev[C5A].pClass)
    {
		class_destroy(gvsp_dev[C5A].pClass);
		gvsp_dev[C5A].pClass = NULL;
	}
	for( i = 0 ; i < GVSP_CORE; i++)
	{
		if(gvsp_dev[i].pcdev)
		{
			cdev_del( gvsp_dev[i].pcdev);	
		}
		if(gvsp_dev[i].devID)
		{
			unregister_chrdev_region(gvsp_dev[i].devID, 1);
		}
	}
	return -1; 




#endif
	
}

/*
Before 使用 origin 的方式: 注册了4个字符设备:c5a、p6a、p6b、c5b.
# cat /proc/devices 

241 c5b
242 p6b
243 p6a
244 c5a

生成的设备文件.
# ls /dev/c5*
/dev/c5a  /dev/c5b
# ls /dev/p6*
/dev/p6a  /dev/p6b
*/

After: 使用注册字符设备 与 创建设备文件 分离方法： 注册一个字符设备gvsp
# 生成4 个设备文件: c5a、p6a、p6b、c5b


/*  函数在此之前的跳转都是 Linux 内核设备驱动模型的机制.真正进入到probe 函数
*   进行 字符设备的注册[register_chrdev]、设备文件的创建[device_create]、
*	设备树dts 键值对的解析[platform_get_irq(resource)]=如果有中断,进行中断的注册(中断服务函数);
*	对用户层接口file_operation{open、release、ioctl、fsync、read、write、poll、mmap等函数实现}.	
*   另外内核的/proc 文件系统等等机制。
*/
static int gvsp_probe(struct platform_device *pdev)
{
	int ret = 0,i;
	int irq;
	u32 intc_val;
	/*use platform_set_drvdata & platform_get_drvdata to change data*/
//==================映射系统关于 dsp 复位寄存器 =========================
	gvsp_dev[C5A].reg_reset = ioremap(DSP_RESET_ADDR, IOREMAP_SIZE);
	if (gvsp_dev[C5A].reg_reset  == NULL){
		print_debug(KERN_ERR "c5b reset ioremap:%#x failed\n", DSP_RESET_ADDR);
		return -1;
	}
	for (i = 1; i < GVSP_CORE ; i++)
	{
		 gvsp_dev[i].reg_reset= gvsp_dev[C5A].reg_reset;
	}

//==================映射每个 dsp 核之 中断寄存器 =========================
	
	for (i = 0 ; i < GVSP_CORE ; i++)
	{
		gvsp_dev[i].reg_intl = ioremap(int_addr[i],IOREMAP_SIZE);
		if(gvsp_dev[i].reg_intl == NULL){	
			print_debug(KERN_ERR"  intl ioremap:%#x failed\n", int_addr[i]);
			goto error;
		}
		/*get irq num*/
//==================解析dts 中gvsp 之每个dsp 核的中断号、并注册中断服务函数 =========================		
		irq = platform_get_irq(pdev,i);
		print_debug(" get irq:%d .\n",irq);
		if (irq < 0)
			goto error;
		ret = devm_request_irq(&pdev->dev, irq, irq_function[i],IRQF_SHARED, devname[i], (void *)&gvsp_dev[i]);
		if (ret) {
			dev_err(&pdev->dev, "failure requesting irq %i\n", irq);
			goto error;
		}
	}
	
//===============字符设备的注册[register_chrdev]、设备文件的创建[device_create]===================	
	ret = gvsp_dev_init();
	if(ret == -1)
	{
		print_debug(KERN_ERR"gvsp dev init error\n");
		goto error;	
	}
#ifdef S2C

	intc_val = readl(gvsp_dev[0].reg_reset);
	intc_val |=  DSP_RELEASE_BIT;
	writel(intc_val,gvsp_dev[0].reg_reset);
#else

	//"set clock gating control"
	clock_addr = ioremap(DSP_CLOCK_ADDR, IOREMAP_SIZE);
	if (clock_addr == NULL){	
		print_debug(KERN_ERR"  intl ioremap:%#x failed\n",DSP_CLOCK_ADDR);
		goto error;
	}
	intc_val = readl(clock_addr);
	intc_val &= ~(0x1 << 0 | 0x1 << 1| 0x1 << 2| 0x1 << 3);
	writel(intc_val,clock_addr);
	//"release DSP software reset control"
	intc_val = readl(gvsp_dev[0].reg_reset);
	intc_val |=  DSP_RELEASE_BIT;
	writel(intc_val,gvsp_dev[0].reg_reset);
	//#set DSP altenate reset vector
/*
	for (i = 0; i < GVSP_CORE; i++)
	{
	    writel(0x1,gvsp_dev[i].reg_intl);
		if ( i ==  C5A)
		{
			writel(0x9d400000,gvsp_dev[i].reg_intl + 4);
		}
		else if (i == P6A)
		{
			writel(0xA1600000,gvsp_dev[i].reg_intl + 4);
		}
		else if (i == P6B)
		{
			writel(0xA9A00000,gvsp_dev[i].reg_intl + 4);
		}
		else if (i == C5B)
		{
			writel(0xA5800000,gvsp_dev[i].reg_intl + 4);
		}
	}
*/
#endif
//===== 创建/proc/gvsp 文件        ============
	proc_create_entry();
	print_debug(KERN_ALERT"%s %d  gvsp probe successs\n",__FUNCTION__,__LINE__); 
	return 0;
error:
	for(i = 0 ; i < GVSP_CORE; i++)
	{
		if(gvsp_dev[i].reg_intl)
		{
			iounmap(gvsp_dev[i].reg_intl);
		}
	}
	if(gvsp_dev[C5A].reg_reset)
	{
		iounmap(gvsp_dev[C5A].reg_reset);
	}
	if(clock_addr)
	{
		iounmap(clock_addr);
	}
    return -1;
}

static int gvsp_remove(struct platform_device *pdev)
{
	
	
	int i;
	for (i = 0; i < GVSP_CORE;i++)
	{
		device_destroy(gvsp_dev[i].pClass, gvsp_dev[i].devID);
	}
	for (i = 0; i < GVSP_CORE;i++)
	{
		cdev_del( gvsp_dev[i].pcdev);
		unregister_chrdev_region(gvsp_dev[i].devID, 1);
	}
	class_destroy(gvsp_dev[C5A].pClass);
	for(i = 0 ; i < GVSP_CORE; i++)
	{
		if(gvsp_dev[i].reg_intl)
		{
			iounmap(gvsp_dev[i].reg_intl);
		}
	}
	if(gvsp_dev[C5A].reg_reset)
	{
		iounmap(gvsp_dev[C5A].reg_reset);
	}
	if(clock_addr)
	{
		iounmap(clock_addr);
	}
	proc_remove_entry();
	return 0;
}

static const struct of_device_id gvsp_match[] = {
	{ .compatible = "byavs,byavs-gvsp",},
	{},
};
MODULE_DEVICE_TABLE(of, gvsp_match);

static struct platform_driver gvsp_drv = {
	.probe = gvsp_probe,
	.remove = gvsp_remove,
	.driver = {
		.name = "byavs,byavs-gvsp",
		.of_match_table = gvsp_match,
	},
	
};

static int __init gvsp_init(void)
{
	print_debug(KERN_INFO "(%s:pid=%d), %s : %s : %d - entry.\n",current->comm, current->pid, __FILE__, __func__, __LINE__);
	return platform_driver_register(&gvsp_drv);
}

static void __exit gvsp_exit(void)
{
	print_debug(KERN_INFO "(%s:pid=%d), %s : %s : %d - leave.\n",current->comm, current->pid, __FILE__, __func__, __LINE__);
	platform_driver_unregister(&gvsp_drv);
} 
module_init(gvsp_init);  
module_exit(gvsp_exit);  
  
MODULE_LICENSE("Dual BSD/GPL");  
MODULE_AUTHOR("chengyouliang");  

