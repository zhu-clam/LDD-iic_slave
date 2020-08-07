/*
*                                                                          
*Copyright (c)  :2019-01-4  Grand Vision Design Systems Inc.   /
*Permission is hereby granted, free of charge, to any person obtaining  /
*a copy of this software and associated documentation files (the   /
*Software), to deal in the Software without restriction, including   /
*without limitation the rights to use, copy, modify, merge, publish,
*distribute, sublicense, and/or sell copies of the Software, and to   /
*permit persons to whom the Software is furnished to do so, subject to   /
*the following conditions:  /
*The above copyright notice and this permission notice shall be included   /
*in all copies or substantial portions of the Software.  /
*                                                                          
*THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND,  /
*EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF  /
*MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  /
*IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   /
*CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  /
*TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE   /
*SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  /
*                                                                          
*                                                                          
*@Filename: media-mem.c                                             
*                                                                          
*@Author: zhuxianfei                            
*@Created on     : 2019-9-10               
*------------------------------------------------------------------------------
*@Description:mmz(memory management zone) driver routin                                                          
*                                                                          
*@Modification History                                                                          
*                                                                                                                                                
*/


#include <generated/autoconf.h>
#include <linux/kernel.h>

#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h>

#include <asm/cacheflush.h>

/*csky cpu*/
#include <asm/pgtable.h>
#include <asm/barrier.h>
#include <asm/cache.h>


#include <linux/seq_file.h>

#include <linux/string.h>
#include <linux/list.h>

#include <linux/time.h>

#include <linux/dma-mapping.h>

#include "mmz-proc.h"
#include "allocator.h"


LIST_HEAD(mmz_list);
int anony = 1;
static DEFINE_SEMAPHORE(mmz_lock);

module_param(anony, int, S_IRUGO);
int zone_number = 0;
int block_number = 0;
unsigned int mmb_number = 0; /*for mmb id*/

static struct mmz_allocator the_allocator;


gv_mmz_t* gv_mmz_create(const char* name,
                          unsigned long gfp,
                          unsigned long phys_start,
                          unsigned long nbytes)
{
    gv_mmz_t* p = NULL;

    mmz_trace_func();

    if (name == NULL)
    {
        printk(KERN_ERR "%s: 'name' can not be zero!", __FUNCTION__);
        return NULL;
    }

    p = kmalloc(sizeof(gv_mmz_t) + 1, GFP_KERNEL);

    if (p == NULL)
    {
        return NULL;
    }

    memset(p, 0, sizeof(gv_mmz_t) + 1);
    strlcpy(p->name, name, GV_MMZ_NAME_LEN);
    p->gfp = gfp;
    p->phys_start = phys_start;
    p->nbytes = nbytes;

    INIT_LIST_HEAD(&p->list); 
    INIT_LIST_HEAD(&p->mmb_list); 

    p->destructor = kfree;

    return p;
}
EXPORT_SYMBOL(gv_mmz_create);

gv_mmz_t* gv_mmz_create_v2(const char* name,
                             unsigned long gfp,
                             unsigned long phys_start,
                             unsigned long nbytes,
                             unsigned int alloc_type,
                             unsigned long block_align)
{
    gv_mmz_t* p;

    mmz_trace_func();

    if (name == NULL)
    {
        printk(KERN_ERR "%s: 'name' can not be zero!", __FUNCTION__);
        return NULL;
    }

    p = kmalloc(sizeof(gv_mmz_t), GFP_KERNEL);

    if (p == NULL)
    {
        return NULL;
    }

    memset(p, 0, sizeof(gv_mmz_t));
    strlcpy(p->name, name, GV_MMZ_NAME_LEN);
    p->gfp = gfp;
    p->phys_start = phys_start;
    p->nbytes = nbytes;
    p->alloc_type = alloc_type;
    p->block_align = block_align;

    INIT_LIST_HEAD(&p->list);
    INIT_LIST_HEAD(&p->mmb_list);

    p->destructor = kfree;

    return p;
}

int gv_mmz_destroy(gv_mmz_t* zone)
{
    if (zone == NULL)
    { return -1; }

    if (zone->destructor)
    { zone->destructor(zone); }

    return 0;
}
EXPORT_SYMBOL(gv_mmz_destroy);

#if 0
static int _check_mmz(gv_mmz_t* zone)
{
    gv_mmz_t* p;

    unsigned long new_start = zone->phys_start;
    unsigned long new_end = zone->phys_start + zone->nbytes;

    if (zone->nbytes == 0)
    { return -1; }

    if (!((new_start >= __pa(high_memory))
          || (new_start < PHYS_OFFSET && new_end <= PHYS_OFFSET)))
    {
        printk(KERN_ERR "ERROR: Conflict MMZ:\n");
        printk(KERN_ERR gv_MMZ_FMT_S "\n", gv_mmz_fmt_arg(zone));
        printk(KERN_ERR "MMZ conflict to kernel memory (0x%08lX, 0x%08lX)\n",
               (long unsigned int)PHYS_OFFSET,
               (long unsigned int)(__pa(high_memory) - 1));
        return -1;
    }

    osal_list_for_each_entry(p, &mmz_list, list)
    {
        unsigned long start, end;
        start = p->phys_start;
        end   = p->phys_start + p->nbytes;

        if (new_start >= end)
        { continue; }
        else if (new_start < start && new_end <= start)
        { continue; }
        else
            ;

        printk(KERN_ERR "ERROR: Conflict MMZ:\n");
        printk(KERN_ERR "MMZ new:   " gv_MMZ_FMT_S "\n", gv_mmz_fmt_arg(zone));
        printk(KERN_ERR "MMZ exist: " gv_MMZ_FMT_S "\n", gv_mmz_fmt_arg(p));
        printk(KERN_ERR "Add new MMZ failed!\n");
        return -1;
    }

    return 0;
}
#endif

int gv_mmz_register(gv_mmz_t* zone)
{
//	int ret = 0;

    mmz_trace(1, GV_MMZ_FMT_S, gv_mmz_fmt_arg(zone));

    if (zone == NULL)
    { return -1; }

    down(&mmz_lock);

    //#ifndef USE_CMA
#if 0
    ret = _check_mmz(zone);

    if (ret)
    {
        up(&mmz_lock);
        return ret;
    }

#endif
    //#endif

    INIT_LIST_HEAD(&zone->mmb_list);

    list_add(&zone->list, &mmz_list);

    up(&mmz_lock);

    return 0;
}

int gv_mmz_unregister(gv_mmz_t* zone)
{
    int losts = 0;
    gv_mmb_t* p;

    if (zone == NULL)
    { return -1; }

    mmz_trace_func();

    down(&mmz_lock);
    list_for_each_entry(p, &zone->mmb_list, list)
    {
        printk(KERN_WARNING " MB Lost: " GV_MMB_FMT_S "\n", gv_mmb_fmt_arg(p));
        losts++;
    }

    if (losts)
    {
        printk(KERN_ERR "%d mmbs not free, mmz<%s> can not be unregistered!\n",
               losts, zone->name);
        up(&mmz_lock);
        return -1;
    }

    list_del(&zone->list);
    up(&mmz_lock);

    return 0;
}


gv_mmb_t* gv_mmb_alloc(const char* name, unsigned long size, unsigned long align,
                         unsigned long gfp, const char* mmz_name)
{
    gv_mmb_t* mmb;

    down(&mmz_lock);
    mmb = the_allocator.mmb_alloc(name, size, align, gfp, mmz_name, NULL);
    up(&mmz_lock);

    return mmb;
}
EXPORT_SYMBOL(gv_mmb_alloc);

#if 0
gv_mmb_t* gv_mmb_alloc_v2(const char* name, unsigned long size, unsigned long align,
                            unsigned long gfp, const char* mmz_name, unsigned int order)
{
    gv_mmb_t* mmb;

    down(&mmz_lock);
    mmb = the_allocator.mmb_alloc_v2(name, size, align, gfp, mmz_name, NULL, order);
    up(&mmz_lock);

    return mmb;
}
EXPORT_SYMBOL(gv_mmb_alloc_v2);
#endif
gv_mmb_t* gv_mmb_alloc_in(const char* name, unsigned long size, unsigned long align,
                            gv_mmz_t* _user_mmz)
{
    gv_mmb_t* mmb;

    if (_user_mmz == NULL)
    { return NULL; }

    down(&mmz_lock);
    mmb = the_allocator.mmb_alloc(name, size, align, _user_mmz->gfp, _user_mmz->name, _user_mmz);
    up(&mmz_lock);

    return mmb;
}

gv_mmb_t* gv_mmb_alloc_in_v2(const char* name, unsigned long size, unsigned long align,
                               gv_mmz_t* _user_mmz, unsigned int order)
{
    gv_mmb_t* mmb;

    if (_user_mmz == NULL)
    { return NULL; }

    down(&mmz_lock);
    mmb = the_allocator.mmb_alloc_v2(name, size, align, _user_mmz->gfp, _user_mmz->name, _user_mmz, order);
    up(&mmz_lock);

    return mmb;
}

void* gv_mmb_map2kern(gv_mmb_t* mmb)
{
    void* p;

    if (mmb == NULL)
    { return NULL; }

    down(&mmz_lock);
    p = the_allocator.mmb_map2kern(mmb, 0);
    up(&mmz_lock);

    return p;
}
EXPORT_SYMBOL(gv_mmb_map2kern);

void* gv_mmb_map2kern_cached(gv_mmb_t* mmb)
{
    void* p;

    if (mmb == NULL)
    { return NULL; }

    down(&mmz_lock);
    p = the_allocator.mmb_map2kern(mmb, 1);
    up(&mmz_lock);

    return p;
}
EXPORT_SYMBOL(gv_mmb_map2kern_cached);

#if 0
int gv_mmb_flush_dcache_byaddr(void* kvirt,
                                unsigned long phys_addr,
                                unsigned long length)
{
    if (NULL == kvirt)
    { return -EINVAL; }

    /*
     * Use flush range to instead flush_cache_all,
     * because flush_cache_all only flush local cpu.
     * And on_each_cpu macro cannot used to flush
     * all cpus with irq disabled.
     */

	dcache_wb_range_csky((unsigned long)kvirt, ((unsigned long)kvirt + length));

	
#if 0 
#ifdef CONFIG_64BIT
    __flush_dcache_area(kvirt, length);
#else
    /*
     * dmac_map_area is invalid in  hi3518ev200 kernel,
     * arm9 is not supported yet
     */
#if (HICHIP==0x3516A100)
    /* flush without clean */
    dmac_map_area(kvirt, length, DMA_TO_DEVICE);
#else
    __cpuc_flush_dcache_area(kvirt, length);
#endif
#endif

#if defined(CONFIG_CACHE_HIL2V200) || defined(CONFIG_CACHE_L2X0)
    /* flush l2 cache, use paddr */
    /*
     * if length > L2 cache size, then this interface
     * will call <outer_flush_all>
     */
    outer_flush_range(phys_addr, phys_addr + length);
#endif
#endif

    return 0;
}
EXPORT_SYMBOL(gv_mmb_flush_dcache_byaddr);

int gv_mmb_invalid_cache_byaddr(void* kvirt,
                                 unsigned long phys_addr,
                                 unsigned long length)
{
    if (NULL == kvirt)
    { return -EINVAL; }

	//dcache_inv_range((unsigned long)kvirt, ((unsigned long)kvirt + length));
	cache_wbinv_range((unsigned long)kvirt, ((unsigned long)kvirt + length));
#if 0
#ifdef CONFIG_64BIT
    __flush_dcache_area(kvirt, length);
#else
    /*
     * dmac_map_area is invalid in  hi3518ev200 kernel,
     * arm9 is not supported yet
     */
#if (HICHIP==0x3516A100)
    /* flush without clean */
    dmac_map_area(kvirt, length, DMA_FROM_DEVICE);
#else
    __cpuc_flush_dcache_area(kvirt, length);
#endif
#endif
#endif

    return 0;
}
EXPORT_SYMBOL(gv_mmb_invalid_cache_byaddr);
#endif
int gv_mmb_unmap(gv_mmb_t* mmb)
{
    int ref;

    if (mmb == NULL)
    { return -1; }

    down(&mmz_lock);

    ref = the_allocator.mmb_unmap(mmb);

    up(&mmz_lock);

    return 0;
}
EXPORT_SYMBOL(gv_mmb_unmap);


int gv_mmb_get(gv_mmb_t* mmb)
{
    int ref;

    if (mmb == NULL)
    { return -1; }

    down(&mmz_lock);

    if (mmb->flags & GV_MMB_RELEASED) 
    { printk(KERN_WARNING "gv_mmb_get: amazing, mmb<%s> is released!\n", mmb->name); }

    ref = ++mmb->phy_ref;

    up(&mmz_lock);

    return ref;
}

int gv_mmb_put(gv_mmb_t* mmb)
{
    int ref;

    if (mmb == NULL)
    { return -1; }

    down(&mmz_lock);

    if (mmb->flags & GV_MMB_RELEASED)
    { printk(KERN_WARNING "gv_mmb_put: amazing, mmb<%s> is released!\n", mmb->name); }

    ref = --mmb->phy_ref;

    if ((mmb->flags & GV_MMB_RELEASED) && mmb->phy_ref == 0 && mmb->map_ref == 0)
    {
        the_allocator.mmb_free(mmb);
    }

    up(&mmz_lock);

    return ref;
}

int gv_mmb_free(gv_mmb_t* mmb)
{
    mmz_trace_func();

    if (mmb == NULL)
    { return -1; }

    mmz_trace(1, GV_MMB_FMT_S, gv_mmb_fmt_arg(mmb));
    down(&mmz_lock);

    if (mmb->flags & GV_MMB_RELEASED)
    {
        printk(KERN_WARNING "gv_mmb_free: amazing, mmb<%s> has been released,\
				but is still in use!\n", mmb->name);
        up(&mmz_lock);
        return 0;
    }

    if (mmb->phy_ref > 0)
    {
        printk(KERN_WARNING "gv_mmb_free: free mmb<%s> delayed \
				for which ref-count is %d!\n",
               mmb->name, mmb->map_ref);
        mmb->flags |= GV_MMB_RELEASED;
        up(&mmz_lock);
        return 0;
    }

    if (mmb->flags & GV_MMB_MAP2KERN)
    {
        printk(KERN_WARNING "gv_mmb_free: free mmb<%s> delayed for which \
			       	is kernel-mapped to 0x%p with map_ref %d!\n",
               mmb->name, mmb->kvirt, mmb->map_ref);
        mmb->flags |= GV_MMB_RELEASED;
        up(&mmz_lock);
        return 0;
    }

    the_allocator.mmb_free(mmb);
    up(&mmz_lock);
    return 0;
}
EXPORT_SYMBOL(gv_mmb_free);

#define MACH_MMB(p, val, member) do{\
        gv_mmz_t *__mach_mmb_zone__; \
        (p) = NULL;\
        list_for_each_entry(__mach_mmb_zone__,&mmz_list, list) { \
            gv_mmb_t *__mach_mmb__;\
            list_for_each_entry(__mach_mmb__,&__mach_mmb_zone__->mmb_list, list) { \
                if(__mach_mmb__->member == (val)){ \
                    (p) = __mach_mmb__; \
                    break;\
                } \
            } \
            if(p)break;\
        } \
    }while(0)

gv_mmb_t* gv_mmb_getby_phys(unsigned long addr)
{
    gv_mmb_t* p;
    down(&mmz_lock);
    MACH_MMB(p, addr, phys_addr);
    up(&mmz_lock);
    return p;
}
EXPORT_SYMBOL(gv_mmb_getby_phys);
#if 0
gv_mmb_t* gv_mmb_getby_kvirt(void* virt)
{
    gv_mmb_t* p;

    if (virt == NULL)
    { return NULL; }

    down(&mmz_lock);
    MACH_MMB(p, virt, kvirt);
    up(&mmz_lock);

    return p;
}
EXPORT_SYMBOL(gv_mmb_getby_kvirt);
#endif


#define MACH_MMB_2(p, val, member, Outoffset) do{\
        gv_mmz_t *__mach_mmb_zone__; \
        (p) = NULL;\
        list_for_each_entry(__mach_mmb_zone__,&mmz_list, list) { \
            gv_mmb_t *__mach_mmb__;\
            list_for_each_entry(__mach_mmb__,&__mach_mmb_zone__->mmb_list, list) { \
                if ((__mach_mmb__->member <= (val)) && ((__mach_mmb__->length + __mach_mmb__->member) > (val))){ \
                    (p) = __mach_mmb__;\
                    Outoffset = val - __mach_mmb__->member;\
                    break;\
                }\
            } \
            if(p)break;\
        } \
}while(0)


gv_mmb_t* gv_mmb_getby_kvirt(void* virt)
{
    gv_mmb_t* p;
    unsigned long Outoffset;

    if (virt == NULL)
    { return NULL; }

    down(&mmz_lock);
    MACH_MMB_2(p, virt, kvirt, Outoffset);
    up(&mmz_lock);

    return p;
}
EXPORT_SYMBOL(gv_mmb_getby_kvirt);

gv_mmb_t* gv_mmb_getby_phys_2(unsigned long addr, unsigned long* Outoffset)
{
    gv_mmb_t* p;

    down(&mmz_lock);
    MACH_MMB_2(p, addr, phys_addr, *Outoffset);
    up(&mmz_lock);
    return p;
}
EXPORT_SYMBOL(gv_mmb_getby_phys_2);

gv_mmz_t* gv_mmz_find(unsigned long gfp, const char* mmz_name)
{
    gv_mmz_t* p;

    down(&mmz_lock);
    begin_list_for_each_mmz(p, gfp, mmz_name)
    up(&mmz_lock);
    return p;
    end_list_for_each_mmz()
    up(&mmz_lock);

    return NULL;
}
EXPORT_SYMBOL(gv_mmz_find);

unsigned long gv_mmz_get_phys(const char *zone_name)
{
	gv_mmz_t *zone;

	zone = gv_mmz_find(0, zone_name);
	if (zone)
		return zone->phys_start;

	return 0;
}
EXPORT_SYMBOL(gv_mmz_get_phys);

#define MEDIA_MEM_NAME  "media-mem"

#ifdef CONFIG_PROC_FS

#if 0
int get_mmz_info_phys_start(void)
{
    return mmz_info_phys_start;
}
#endif

int mmz_read_proc(struct seq_file* sfile)
{
    gv_mmz_t* p;
    int len = 0;
    unsigned int zone_number = 0;
    unsigned int block_number = 0;
    unsigned int used_size = 0;
    unsigned int free_size = 0;
    unsigned int mmz_total_size = 0;

    mmz_trace_func();

    down(&mmz_lock);
    list_for_each_entry(p, &mmz_list, list)
    {
        gv_mmb_t* mmb;
        seq_printf(sfile, "+---ZONE: " GV_MMZ_FMT_S "\n", gv_mmz_fmt_arg(p));
        mmz_total_size += p->nbytes / 1024;
        ++zone_number;

        list_for_each_entry(mmb, &p->mmb_list, list)
        {
            seq_printf(sfile, "   |-MMB: " GV_MMB_FMT_S "\n", gv_mmb_fmt_arg(mmb));
            used_size += mmb->length / 1024;
            ++block_number;
        }
    }

    if (0 != mmz_total_size)
    {
        free_size = mmz_total_size - used_size;
        seq_printf(sfile, "\n---MMZ_USE_INFO:\n total size=%dKB(%dMB),"
                   "used=%dKB(%dMB + %dKB),remain=%dKB(%dMB + %dKB),"
                   "zone_number=%d,block_number=%d\n",
                   mmz_total_size, mmz_total_size / 1024,
                   used_size, used_size / 1024, used_size % 1024,
                   free_size, free_size / 1024, free_size % 1024,
                   zone_number, block_number);
        mmz_total_size = 0;
        zone_number = 0;
        block_number = 0;
    }

    up(&mmz_lock);

    return len;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
static int mmz_write_proc(struct file* file, const char __user* buffer,
                          unsigned long count, void* data)
#else
static ssize_t mmz_write_proc(struct file* file, const char __user* buffer,
                              size_t count, loff_t* data)
#endif

{
    char buf[256];

    if (count >= sizeof(buf))
    {
        printk(KERN_ERR "MMZ: your parameter string is too long!\n");
        return -EIO;
    }

    memset(buf, 0, sizeof(buf));

    if (copy_from_user(buf, buffer, count))
    {
        printk("\nmmz_userdev_ioctl: copy_from_user error.\n");
        return 0;
    }

    the_allocator.init(buf);
    //media_mem_parse_cmdline(buf);

    return count;
}

#define MMZ_PROC_ROOT  NULL

static const struct seq_operations mmz_seq_ops =
{
    .start = mmz_seq_start,
    .next = mmz_seq_next,
    .stop = mmz_seq_stop,
    .show = mmz_seq_show
};

static int mmz_proc_open(struct inode* inode, struct file* file)
{
    return seq_open(file, &mmz_seq_ops);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)

static struct file_operations mmz_proc_ops =
{
    .owner = THIS_MODULE,
    .open = mmz_proc_open,
    .read = seq_read,
    .release = seq_release,
};
static int __init media_mem_proc_init(void)
{
    struct proc_dir_entry* p;

    p = create_proc_entry(MEDIA_MEM_NAME, 0644, MMZ_PROC_ROOT);

    if (p == NULL)
    { return -1; }

    p->write_proc = mmz_write_proc;
    p->proc_fops = &mmz_proc_ops;

    return 0;
}

#else

static struct file_operations mmz_proc_ops =
{
    .owner = THIS_MODULE,
    .open = mmz_proc_open,
    .read = seq_read,
    .write = mmz_write_proc,
    .release = seq_release,
};

static int __init media_mem_proc_init(void)
{
    struct proc_dir_entry* p;

    p = proc_create(MEDIA_MEM_NAME, 0, MMZ_PROC_ROOT, &mmz_proc_ops); 
    if (!p)
    {
        printk(KERN_ERR "Create mmz proc fail!\n");
        return -1;
    }

    return 0;
}
#endif

static void __exit media_mem_proc_exit(void)
{
    remove_proc_entry(MEDIA_MEM_NAME, MMZ_PROC_ROOT);
}

#else
static int __init media_mem_proc_init(void) { return 0; }
static void __exit media_mem_proc_exit(void) { }

#endif /* CONFIG_PROC_FS */

#define MMZ_SETUP_CMDLINE_LEN 	256
#define MMZ_ALLOCATOR_NAME_LEN	32

#ifndef MODULE

static char __initdata setup_zones[MMZ_SETUP_CMDLINE_LEN];
static int __init parse_kern_cmdline(char* line)
{
    strlcpy(setup_zones, line, sizeof(setup_zones));

    return 1;
}
__setup("mmz=", parse_kern_cmdline);

static char __initdata setup_allocator[MMZ_ALLOCATOR_NAME_LEN];
static int __init parse_kern_allocator(char* line)
{
    strlcpy(setup_allocator, line, sizeof(setup_allocator));
    return 1;
}
__setup("mmz_allocator=", parse_kern_allocator);

#else
static char setup_zones[MMZ_SETUP_CMDLINE_LEN] = {'\0'};
static char setup_allocator[MMZ_ALLOCATOR_NAME_LEN] = {'\0'};
module_param_string(mmz, setup_zones, MMZ_SETUP_CMDLINE_LEN, 0600);
module_param_string(mmz_allocator, setup_allocator, MMZ_ALLOCATOR_NAME_LEN, 0600);
MODULE_PARM_DESC(mmz, "mmz_allocator=allocator mmz=name,0,start,size,type,eqsize:[others]");
#endif


/*dynamic insmod MODULE For Debug*/
#if 0
static char setup_zones[MMZ_SETUP_CMDLINE_LEN] = {'\0'};
static char setup_allocator[MMZ_ALLOCATOR_NAME_LEN] = {'\0'};
module_param_string(mmz, setup_zones, MMZ_SETUP_CMDLINE_LEN, 0600);
module_param_string(mmz_allocator, setup_allocator, MMZ_ALLOCATOR_NAME_LEN, 0600);
MODULE_PARM_DESC(mmz, "mmz_allocator=allocator mmz=name,0,start,size,type,eqsize:[others]");
#endif

int mem_check_module_param(void)
{
    if(1 != anony)
    {
        printk("The module param \"anony\" should only be 1 which is %d \n",anony);
        return -1;
    }
    return 0;
}

static void mmz_exit_check(void)
{
    gv_mmz_t* pmmz;
    struct list_head* p, *n;

    mmz_trace_func();

    list_for_each_safe(p, n, &mmz_list)
    {
        pmmz = list_entry(p, gv_mmz_t, list);
        printk(KERN_WARNING "MMZ force removed: " GV_MMZ_FMT_S "\n", gv_mmz_fmt_arg(pmmz));
        gv_mmz_unregister(pmmz);
    }
}

 int __init media_mem_init(void)
{
    int ret = 0;

    printk(KERN_INFO "grand vision Media Memory Zone Manager\n");

#if 1
    if (1 != anony) //anony = 1
    {
        printk("The module param \"anony\" should only be 1 which is %d\n", anony);
        return -1;
    }

#endif
#if 0
    if (0 == strcmp(setup_allocator, "cma"))
    {
        ret = cma_allocator_setopt(&the_allocator);
        //printk(KERN_ERR "fn %s %d\n", __func__, __LINE__);
    }
#endif
    if (0 == strcmp(setup_allocator, "grandvision"))
    {
        ret = gv_allocator_setopt(&the_allocator);
        //printk(KERN_ERR "fn %s %d\n", __func__, __LINE__);
    }
    else
    {
        mmz_exit_check();
        //printk(KERN_ERR "fn %s %d\n", __func__, __LINE__);
        return -EINVAL;
    }

#if 1
    //ret = media_mem_parse_cmdline(setup_zones);
    ret = the_allocator.init(setup_zones); 

    if (ret != 0)
    {
        mmz_exit_check();
        printk(KERN_ERR "fn %s %d\n", __func__, __LINE__);
        return ret;
    }

#endif

    media_mem_proc_init();

    //printk(KERN_ERR "fn %s %d\n", __func__, __LINE__);
    mmz_userdev_init();

    //printk(KERN_ERR "fn %s %d\n", __func__, __LINE__);
    return 0;
}

#ifdef MODULE

 void __exit media_mem_exit(void)
{
    mmz_userdev_exit();

    mmz_exit_check();

    media_mem_proc_exit();
}

module_init(media_mem_init);
module_exit(media_mem_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("grandvision");
#else

subsys_initcall(media_mem_init);

#endif




