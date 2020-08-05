/*************************************************************************
	> File Name: scull.c
	> Author: amoscykl
	> Mail: amoscykl@163.com 
	> Created Time: 2020年01月07日 星期二 16时49分53秒
 ************************************************************************/

#include<linux/init.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<asm/uaccess.h> /*copy_*_user*/


MODULE_LICENSE("GPL");

int scull_major = 0;
int scull_minor = 0;
struct cdev cdev; /* char device structure */


#define MAX_SIZE 10
size_t size = 0;

char store[MAX_SIZE];


int scull_open(struct inode *inode , struct file *filp)
{
  /* trim to 0 the length of the device if open was write -only
   *   */
  if ( (filp ->f_flags & O_ACCMODE) == O_WRONLY) {
      size = 0;
    }
  return 0; /* success */
}


int scull_release(struct inode *inode , struct file *filp)
{
  return 0;
}

//int scull_read(struct inode *inode , struct file *filp)
ssize_t scull_read(struct file *filp , char __user *buf, size_t count, loff_t *f_pos)
{
  return 0;
}

//int scull_write(struct inode *inode , struct file *filp)
ssize_t scull_write(struct file *filp , const char __user *buf,size_t count ,loff_t *f_pos)
{
  return 0;
}


struct file_operations scull_fops = {
	.owner = THIS_MODULE,
	.read = scull_read,
	.write = scull_write,
	.open = scull_open,
	.release = scull_release,
};

static unsigned int scull_MAJOR = 0;

/*module_init() 里面的函数必须为 int func(void) */
int scull_init_module(void)
{
	printk("hello world begin !\n");
	int result;
#if 0
	dev_t dev = 0;// 在驱动程序中定义设备号;高12位主设备号,低20位为次设备号。

	/*
	 * step1：动态申请设备号函数：
	 * int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count,const char *name )
	 *
	 * 返回值为零 正常，返回值小余零出错.
	 * */
	// 动态申请设备号，设备名称“scull”，设备个数1.次设备号scull_minor,申请到的设备存储在dev中。
	result = alloc_chrdev_region(&dev,scull_minor, 1,"scull");
	if (result < 0) {
		printk(KERN_WARNING "scull:?can't get scull major :%d\n",scull_major);
		return result;
	}	
	// 获取dev的主设备号：scull_major
	scull_major = MAJOR(dev);

	/* 
	 * step2： 注册字符设备		struct cdev cdev;
	 * cdev_init() - initialize a cdev structure
	 *
	 * void cdev_init(struct cdev *cdev, const struct file_operations *fops)
	 * */
	cdev_init(&cdev, &scull_fops);//初始化cdev, 并将scull_fops 绑定cdev.
	cdev.owner = THIS_MODULE;
	cdev.ops = &scull_fops;
	

	/* 
	 * step3: 将字符设备添加到系统
	 *
	 * */
	result = cdev_add(&cdev,dev,1 );//在字符设备中添加上我们的设备
	if(result) {
		printk("Error cdev add result %d!\n ",result);
		unregister_chrdev_region(dev,1); /*添加失败，则注销字符设备*/
		return result;
	}
#endif
	//int register_chardev (unsigned int major, const char *name, struct file_operations *fops)
	//major 主设备号, 该值为0 时，动态分配主设备号。 不为0 时，表示静态注册
	//name 设备名称
	//fops file_operations() 结构体变量地址(指针)
	//返回值: major 值为0，正常注册后，返回分配的主设备号。分配失败，返回负值。
	//指定major 的值，正常注册返回0，失败返回负值。
	scull_MAJOR = register_chrdev( 0 ,"scull",&scull_fops);
	if(result < 0)
	{
		printk("Unable to register character device %d!\n",scull_MAJOR);
		return scull_MAJOR;
	}


	printk("hello world ending !\n");
	return 0;/*succeed*/
}

/*  
 *  模块注销函数： 函数必须： void func(void)
 * */
void scull_cleanup_module(void)
{
#if 0
	dev_t dev;
	cdev_del(&cdev);
	dev = MKDEV(scull_major, scull_minor );
	//void unregister_chrdev_region(dev_t from, unsigned count)
	//参数：dev_t 设备号;count 个数
	unregister_chrdev_region(dev,1);
#endif
	unregister_chrdev(scull_MAJOR,  "scull");
	printk("simple_cleanup_module!\n");
}

/*define module entry func*/
module_init(scull_init_module);
module_exit(scull_cleanup_module);









