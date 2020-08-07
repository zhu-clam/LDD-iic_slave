/*************************************************************************
	> File Name: mmz_drv_test.c
	> Author: ma6174
	> Mail: ma6174@163.com 
	> Created Time: Thu 19 Sep 2019 04:31:58 AM PDT
 ************************************************************************/

#include<stdio.h>


#include <linux/io.h>
#include <linux/init.h>
#include <linux/module.h>
#include <string.h>
#include <memory.h>

#include "gv_mmz.h"

#define MMB_KER_NAME "polaris-mem" 
#define MMZ_NAME "anonymous" 

static void* kvir;
static gv_mmb_t* mmb_0;

static int __init test_init(void)
{
	printk("insmod test init ok!\n");
	int ret = 0;
	mmb_0 = (gv_mmb_t *)kmalloc(sizeof(gv_mmb_t),GFP_KERNEL);

	
//	memset(&mmb_0,0,sizeof(struct mmb_info));
	memset(mmb_0,0,sizeof(gv_mmb_t));
	unsigned long size = 0x100000;
	unsigned long align = 0x1000;
	unsigned long gfp = 0;

	/*step1: get mmb */	
	mmb_0 = gv_mmb_alloc(MMB_KER_NAME, size, align, gfp, MMZ_NAME);	
	printk("mmb_0 alloc success: phys_addr:0x%x,length:0x%x,mmb is %s\n",mmb_0->phys_addr,mmb_0->length,mmb_0->name);

	/*step2*/
	/*extern void* gv_mmb_map2kern(gv_mmb_t* mmb);*/
	kvir = gv_mmb_map2kern(mmb_0);
	if(!kvir)
	{
		printk("gv_mmb_map2kern failed!\n");
	}

	*kvir = 1111;
	printk("mmb_0 map2kern success:phys_addr:0x%x,length:0x%x,kvirt:%d\n",mmb_0->phys_addr,mmb_0->length,*kvir);

}

static void __exit test_exit(void)
{

	*kvir = 2222; 
	printk("mmb_0 map2kern success:phys_addr:0x%x,length:0x%x,kvirt:%d\n",mmb_0->phys_addr,mmb_0->length,*kvir);
	/*step3*/
	/*extern int gv_mmb_unmap(gv_mmb_t* mmb);*/
	ret = gv_mmb_unmap(mmb_0);
	if(ret)
	{
		printk("mmb_0 gv_mmb_unmap failed!\n");
	}

	/*step4*/
	/*extern int gv_mmb_free(gv_mmb_t* mmb);*/
	ret = gv_mmb_free(mmb_0);
	if(ret)
	{
		printk("mmb_0 gv_mmb_free failed!\n");
	}
	printk("test exit successful!\n");
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");




