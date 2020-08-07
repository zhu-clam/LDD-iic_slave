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
*@Filename: mmz-userdev.c                                             
*                                                                          
*@Author: zhuxianfei                            
*@Created on     : 2019-9-4               
*------------------------------------------------------------------------------
*@Description:mmz(memory management zone) driver routin                                                          
*                                                                          
*@Modification History                                                                          
*                                                                          
*                                                                          
*/


#include <linux/kernel.h>
#include <linux/version.h>
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
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/time.h>
#include <linux/sched.h>
#include <linux/dma-mapping.h>
	
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/cacheflush.h>
/*csky cpu*/
#include <linux/cache.h>
#include <asm/pgtable.h>
#include <asm/barrier.h>


#include "gv_mmz.h"


#define error(s...) do{ printk(KERN_ERR "mmz_userdev:%s: ", __FUNCTION__); printk(s); }while(0)
#define warning(s...) do{ printk(KERN_WARNING "mmz_userdev:%s: ", __FUNCTION__); printk(s); }while(0)

void dcache_wb_range_csky(unsigned long start, unsigned long end)
{
		   unsigned long i = start & ~(L1_CACHE_BYTES - 1);
 
			   for (;i < end; i += L1_CACHE_BYTES)
						   asm volatile("dcache.cval1 %0\n"::"r"(i):"memory");
			   sync_is();
} 


struct mmz_userdev_info
{
    pid_t pid;
    pid_t mmap_pid;
    struct semaphore sem;
    struct list_head list;
};

static int mmz_flush_dcache_mmb_dirty(struct dirty_area* p_area)
{
    if (p_area == NULL)
    { return -EINVAL; }

	dcache_wb_range_csky((unsigned long)p_area->dirty_virt_start,((unsigned long)p_area->dirty_virt_start +p_area->dirty_size));

#if 0
#ifdef CONFIG_64BIT
    __flush_dcache_area(p_area->dirty_virt_start, p_area->dirty_size);
#else
    /* flush l1 cache, use vir addr */
    __cpuc_flush_dcache_area(p_area->dirty_virt_start, p_area->dirty_size);
    /* flush with clean */
    //dmac_map_area(p_area->dirty_virt_start, p_area->dirty_size, DMA_TO_DEVICE);

#if defined(CONFIG_CACHE_HIL2V200) || defined(CONFIG_CACHE_L2X0)
    /* flush l2 cache, use paddr */
    outer_flush_range(p_area->dirty_phys_start,
                      p_area->dirty_phys_start + p_area->dirty_size);
#endif
#endif
#endif

    return 0;
}

static int mmz_flush_dcache_mmb(struct mmb_info* pmi)
{
    gv_mmb_t* mmb;

    if (pmi == NULL)
    { return -EINVAL; }

    mmb = pmi->mmb;

    if (mmb == NULL )
    {
        printk("%s->%d,error!\n", __func__, __LINE__);
        return -EINVAL;
    }

	//dcache_wb_range((unsigned long)mmb->kvirt,((unsigned long)mmb->kvirt +mmb->length));
	dcache_wb_range_csky((unsigned long)pmi->mapped,((unsigned long)pmi->mapped +pmi->size));
#if 0
#ifdef CONFIG_64BIT
    __flush_dcache_area(pmi->mapped, (size_t)pmi->size);
#else
    /*flush l1 cache, use vir addr*/
    __cpuc_flush_dcache_area(pmi->mapped, (size_t)pmi->size);

#if defined(CONFIG_CACHE_HIL2V200) || defined(CONFIG_CACHE_L2X0)
    /* flush l2 cache, use paddr */
    outer_flush_range(mmb->phys_addr, mmb->phys_addr + mmb->length);
#endif
#endif
#endif
    return 0;
}

/*
 * this function should never be called with local irq disabled,
 * because on_each_cpu marco will raise ipi interrupt.
 */
int mmz_flush_dcache_all(void)
{
#if 0
#ifdef CONFIG_64BIT
#ifdef CONFIG_SMP
    on_each_cpu((smp_call_func_t)flush_cache_all, NULL, 1);
#else
    flush_cache_all();
#endif
#else /* CONFIG_64BIT */
#ifdef CONFIG_SMP
    on_each_cpu((smp_call_func_t)__cpuc_flush_kern_all, NULL, 1);
#else
    __cpuc_flush_kern_all();
#endif

    outer_flush_all();
#endif /* CONFIG_64BIT */
#endif
    return 0;
}

/*implement file_operations open function */
static int mmz_userdev_open(struct inode* inode, struct file* file)
{
    struct mmz_userdev_info* pmu;

    pmu = kmalloc(sizeof(*pmu), GFP_KERNEL);

    if (pmu == NULL)
    {
        error("alloc mmz_userdev_info failed!\n");
        return -ENOMEM;
    }

    pmu->pid = current->pid;
    pmu->mmap_pid = 0;
    sema_init(&pmu->sem, 1);
    INIT_LIST_HEAD(&pmu->list);

    /* this file could be opened just for once */
    file->private_data = (void*)pmu;

    return 0;
}

static int ioctl_mmb_alloc(struct file* file,
                           unsigned int iocmd,
                           struct mmb_info* pmi)
{
	struct mmz_userdev_info* pmu = file->private_data;
	struct mmb_info* new_mmbinfo;
	gv_mmb_t* mmb;
	mmb = gv_mmb_alloc(pmi->mmb_name, pmi->size,
						pmi->align, pmi->gfp, pmi->mmz_name);

	if (mmb == NULL)
	{
		error("gv_mmb_alloc(%s, %lu, 0x%lx, %lu, %s) failed!\n",
			  pmi->mmb_name, pmi->size, pmi->align,
			  pmi->gfp, pmi->mmz_name);
		return -ENOMEM;
	}
	new_mmbinfo = kmalloc(sizeof(*new_mmbinfo), GFP_KERNEL);

	if (new_mmbinfo == NULL)
	{
		gv_mmb_free(mmb);
		error("alloc mmb_info failed!\n");
		return -ENOMEM;
	}
	
	memcpy(new_mmbinfo, pmi, sizeof(*new_mmbinfo));
	new_mmbinfo->phys_addr = gv_mmb_phys(mmb);
	new_mmbinfo->mmb = mmb;
	list_add_tail(&new_mmbinfo->list, &pmu->list);

	pmi->phys_addr = new_mmbinfo->phys_addr;

	gv_mmb_get(mmb);
	return 0;
}
#if 0
static int ioctl_mmb_alloc_v2(struct file* file, unsigned int iocmd, struct mmb_info* pmi)
{
   struct mmz_userdev_info* pmu = file->private_data;
   struct mmb_info* new_mmbinfo;
   gv_mmb_t* mmb;

   mmb = gv_mmb_alloc_v2(pmi->mmb_name, pmi->size, pmi->align,
						  pmi->gfp, pmi->mmz_name, pmi->order);

   if (mmb == NULL)
   {
	   error("gv_mmb_alloc(%s, %lu, 0x%lx, %lu, %s) failed!\n",
			 pmi->mmb_name, pmi->size, pmi->align,
			 pmi->gfp, pmi->mmz_name);
	   return -ENOMEM;
   }

   new_mmbinfo = kmalloc(sizeof(*new_mmbinfo), GFP_KERNEL);

   if (new_mmbinfo == NULL)
   {
	   gv_mmb_free(mmb);
	   error("alloc mmb_info failed!\n");
	   return -ENOMEM;
   }

   memcpy(new_mmbinfo, pmi, sizeof(*new_mmbinfo));
   new_mmbinfo->phys_addr = gv_mmb_phys(mmb);
   new_mmbinfo->mmb = mmb;
   list_add_tail(&new_mmbinfo->list, &pmu->list);

   pmi->phys_addr = new_mmbinfo->phys_addr;

   gv_mmb_get(mmb);

   return 0;
}
#endif
static struct mmb_info* get_mmbinfo(unsigned long addr,
                                    struct mmz_userdev_info* pmu)
{
    struct mmb_info* p;

    list_for_each_entry(p, &pmu->list, list)
    {
        if ((addr >= p->phys_addr) && addr < (p->phys_addr + p->size))
        { break; }
    }

    if (&p->list == &pmu->list)
    { return NULL; }

    return p;
}

static struct mmb_info* get_mmbinfo_safe(unsigned long addr,
		struct mmz_userdev_info* pmu)
{
	struct mmb_info* p;

	p = get_mmbinfo(addr, pmu);

	if (p == NULL)
	{
		error("mmb(0x%08lX) not found!\n", addr);
		return NULL;
	}

	return p;
}

static int ioctl_mmb_user_unmap(struct file* file, unsigned int iocmd, struct mmb_info* pmi);


static int _usrdev_mmb_free(struct mmb_info* p)
{
    int ret = 0;

    list_del(&p->list);
    gv_mmb_put(p->mmb);
    ret = gv_mmb_free(p->mmb);
    kfree(p);

    return ret;
}


static int ioctl_mmb_free(struct file* file,
                          unsigned int iocmd, struct mmb_info* pmi)
{
    int ret = 0;
    struct mmz_userdev_info* pmu = file->private_data;
    struct mmb_info* p = get_mmbinfo_safe(pmi->phys_addr, pmu);

    if (p == NULL)
    { return -EPERM; }


    if ((p->map_ref > 0) || (p->mmb_ref > 0))
    {
        warning("mmb<%s> is still in use!\n", p->mmb->name);
        return -EBUSY;
    }

    ret = _usrdev_mmb_free(p);

    return ret;
}

static int ioctl_mmb_attr(struct file* file,
						unsigned int iocmd, struct mmb_info* pmi)
{
  struct mmz_userdev_info* pmu = file->private_data;
  struct mmb_info* p;

  if ((p = get_mmbinfo_safe(pmi->phys_addr, pmu)) == NULL)
  { return -EPERM; }

  memcpy(pmi, p, sizeof(*pmi));
  return 0;
}

static int ioctl_mmb_user_remap(struct file* file,
								unsigned int iocmd,
								struct mmb_info* pmi,
								int cached)

{
	struct mmz_userdev_info* pmu = file->private_data;
	struct mmb_info* p;

	unsigned long addr, len, pgoff;

	if ((p = get_mmbinfo_safe(pmi->phys_addr, pmu)) == NULL)
	{ return -EPERM; }

	/*
	 * mmb could be remapped for more than once, but should not
	 * be remapped with confusing cache type.
	 */
	if (p->mapped && (p->map_ref > 0))
	{
		p->map_ref++;
		p->mmb_ref++;

		gv_mmb_get(p->mmb);
		/*
		 * pmi->phys may not always start at p->phys,
		 * and may start with offset from p->phys.
		 * so, we need to calculate with the offset.
		 */
		pmi->mapped = p->mapped + (pmi->phys_addr - p->phys_addr);

		return 0;
	}

	/* mmb first time mapped*/
	if (p->phys_addr & ~PAGE_MASK)
	{ return -EINVAL; }

	addr = 0;
	len = PAGE_ALIGN(p->size);

	pmu->mmap_pid = current->pid;

	pgoff = p->phys_addr;
//	addr  = vm_mmap(file, addr, len, prot, flags, pgoff);
	addr = vm_mmap(file, 0, len, PROT_READ | PROT_WRITE, MAP_SHARED, pgoff);

	pmu->mmap_pid = 0;

	if (IS_ERR_VALUE(addr))
	{
		error("vm_mmap(file, 0, %lu, 0x%08lX) return 0x%08lX\n",
			  len, pgoff, addr);
		return addr;
	}

	p->mapped = (void*)addr;

	p->map_ref++;
	p->mmb_ref++;
	gv_mmb_get(p->mmb);

	/*
	 * pmi->phys may not always start at p->phys,
	 * and may start with offset from p->phys.
	 * so, we need to calculate with the offset.
	 */
	pmi->mapped = p->mapped + (pmi->phys_addr - p->phys_addr);

	return 0;
}

static int ioctl_mmb_user_unmap(struct file* file,
								unsigned int iocmd, struct mmb_info* pmi)
{
    int ret;
    unsigned long addr, len;
    struct mmb_info* p;
    struct mmz_userdev_info* pmu = file->private_data;

    if ((p = get_mmbinfo_safe(pmi->phys_addr, pmu)) == NULL)
    { return -EPERM; }

    if (!p->mapped) 
    {
		warning("mmb(0x%lx) isn't user-mapped!\n", p->phys_addr);
        pmi->mapped = NULL;
        return -EIO;
    }

    if (!((p->map_ref > 0) && (p->mmb_ref > 0)))
    {
        error("mmb<%s> has invalid refer: map_ref=%d, mmb_ref=%d.\n",
              p->mmb->name, p->map_ref, p->mmb_ref);
        return -EIO;
    }

    p->map_ref--;
    p->mmb_ref--;
    gv_mmb_put(p->mmb);
	
    if (p->map_ref > 0)
    { return 0; }
	
    addr = (unsigned long)p->mapped;
    len  = PAGE_ALIGN(p->size);

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0)
    down_write(&current->mm->mmap_sem);
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0)
    ret = do_munmap(current->mm, addr, len);
#else
    ret = vm_munmap(addr, len);
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0)
    up_write(&current->mm->mmap_sem);
#endif

    if (!IS_ERR_VALUE(ret))
    {
        p->mapped = NULL;
        pmi->mapped = NULL;
    }

    if ((p->map_ref == 0) && (p->mmb_ref == 0))
    {
        _usrdev_mmb_free(p);
    }


#if 0
    if (p->delayed_free && (p->map_ref == 0) && (p->mmb_ref == 0))
    {
        _usrdev_mmb_free(p);
    }
#endif
    return ret;
}

unsigned long usr_virt_to_phys(unsigned long virt)
{
    pgd_t* pgd;
    pud_t* pud;
    pmd_t* pmd;
    pte_t* pte;
    int cacheable = 0;
    unsigned long page_addr = 0;
    unsigned long page_offset = 0;
    unsigned long phys_addr = 0;

    if (virt & 0x3)
    {
        error("invalid virt addr 0x%08lx[not 4 bytes align]\n", virt);
        return 0;
    }

    if (virt >= PAGE_OFFSET)
    {
        error("invalid user space virt addr 0x%08lx\n", virt);
        return 0;
    }

/*csky-linux-4.9.56/arch/csky/include/asm/pgtable.h:mm->pgd + pgd_index(address); */
    pgd = pgd_offset(current->mm, virt);
    if (pgd_none(*pgd))
    {
        error("error: not mapped in pgd!\n");
        return 0;
    }

    pud = pud_offset(pgd, virt);

    if (pud_none(*pud))
    {
        error("error: not mapped in pud!\n");
        return 0;
    }

    pmd = pmd_offset(pud, virt);

    if (pmd_none(*pmd))
    {
        error("error: not mapped in pmd!\n");
        return 0;
    }

    pte = pte_offset_map(pmd, virt);

    if (pte_none(*pte))
    {
        error("error: not mapped in pte!\n");
        return 0;
    }

#if 0	
    page_addr = (pte_val(*pte) & PHYS_MASK) & PAGE_MASK;
    page_offset = virt & ~PAGE_MASK;
    phys_addr = page_addr | page_offset;
#else
//	pa = (pte_val(*pte_tmp) & PAGE_MASK) |(va & ~PAGE_MASK);
	page_addr = (pte_val(*pte) & PAGE_MASK);
	page_offset = virt & ~PAGE_MASK;
	phys_addr = page_addr | page_offset;

#endif
#ifdef CONFIG_64BIT
	if (pte_val(*pte) & (1 << 4))
#else
	if (pte_val(*pte) & (1 << 3))
#endif
    { cacheable = 1; }

    /*
     * phys_addr: the lowest bit indicates its cache attribute
     * 1: cacheable
     * 0: uncacheable
     */
    phys_addr |= cacheable;

    return phys_addr;
}
EXPORT_SYMBOL(usr_virt_to_phys);

static int ioctl_mmb_virt2phys(struct file* file,
                               unsigned int iocmd, struct mmb_info* pmi)
{
	int ret = 0;
	unsigned long virt = 0, phys = 0;
	unsigned long offset = 0;
#if 1
	virt = (unsigned long)pmi->mapped;
	phys = usr_virt_to_phys(virt); 

	if (!phys)
	{
		ret = -ENOMEM;
	}

	if (!gv_mmb_getby_phys_2(phys, &offset))
	{
		error("Not mmz alloc memory[0x%lx 0x%lx]! 0x%lx\n", virt, phys, offset);
		return -EINVAL;
	}

	//printk("##### phys 0x%lx offset 0x%lx\n", phys, offset);
	pmi->phys_addr = phys;
#endif

	return ret;
}


static int mmz_userdev_ioctl_m(struct file* file, unsigned int cmd, struct mmb_info* pmi)
{
	int ret = 0;

	switch (_IOC_NR(cmd))
	{
		case _IOC_NR(IOC_MMB_ALLOC):
			ret = ioctl_mmb_alloc(file, cmd, pmi);
			break;
#if 0
		case _IOC_NR(IOC_MMB_ALLOC_V2):
			ret = ioctl_mmb_alloc_v2(file, cmd, pmi);
			break;
#endif
		case _IOC_NR(IOC_MMB_ATTR):
			ret = ioctl_mmb_attr(file, cmd, pmi);
			break;

		case _IOC_NR(IOC_MMB_FREE):
			ret = ioctl_mmb_free(file, cmd, pmi);
			break;

		case _IOC_NR(IOC_MMB_USER_REMAP):
			ret = ioctl_mmb_user_remap(file, cmd, pmi, 0);
			break;

		case _IOC_NR(IOC_MMB_USER_REMAP_CACHED):
			ret = ioctl_mmb_user_remap(file, cmd, pmi, 1);
			break;

		case _IOC_NR(IOC_MMB_USER_UNMAP):
			ret = ioctl_mmb_user_unmap(file, cmd, pmi);
			break;

		case _IOC_NR(IOC_MMB_VIRT_GET_PHYS):
			ret = ioctl_mmb_virt2phys(file, cmd, pmi);
			break;

		default:
			error("invalid ioctl cmd = %08X\n", cmd);
			ret = -EINVAL;
			break;
	}

    return ret;

}

static int mmz_userdev_ioctl_r(struct file* file, unsigned int cmd, struct mmb_info* pmi)
{
	switch (_IOC_NR(cmd))
	{
		case _IOC_NR(IOC_MMB_ADD_REF): 
			pmi->mmb_ref++;
			gv_mmb_get(pmi->mmb);
			break;

		case _IOC_NR(IOC_MMB_DEC_REF):
			if (pmi->mmb_ref <= 0)
			{
				error("mmb<%s> mmb_ref is %d!\n", pmi->mmb->name, pmi->mmb_ref);
				return -EPERM;
			}

			pmi->mmb_ref--;
			gv_mmb_put(pmi->mmb);

			if (pmi->mmb_ref == 0 && pmi->mmb_ref == 0) 
			{
				_usrdev_mmb_free(pmi);
			}

#if 0
			if (pmi->delayed_free && pmi->mmb_ref == 0 && pmi->mmb_ref == 0)
			{
				_usrdev_mmb_free(pmi);
			}
#endif
			break;

		default:
			return -EINVAL;
			break;
	}

	return 0;
}


static long mmz_userdev_ioctl(struct file* file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	struct mmz_userdev_info* pmu = file->private_data;

	down(&pmu->sem);

	if (_IOC_TYPE(cmd) == 'm')
	{
		struct mmb_info mi;

		if (_IOC_SIZE(cmd) > sizeof(mi) || arg == 0)
		{
			error("_IOC_SIZE(cmd)=%d, arg==0x%08lX\n", _IOC_SIZE(cmd), arg);
			ret = -EINVAL;
			goto __error_exit;
		}

        memset(&mi, 0, sizeof(mi));

        if (copy_from_user(&mi, (void*)arg, _IOC_SIZE(cmd)))
        {
            printk("\nmmz_userdev_ioctl: copy_from_user error.\n");
            ret = -EFAULT;
            goto __error_exit;
        }

		ret = mmz_userdev_ioctl_m(file, cmd, &mi);
		
		if (!ret && (cmd & IOC_OUT))
		{
		    if (copy_to_user((void*)arg, &mi, _IOC_SIZE(cmd)))
            {
                printk("\nmmz_userdev_ioctl: copy_to_user error.\n");
                ret = -EFAULT;
                goto __error_exit;
            }
		}
		
	}
	else if (_IOC_TYPE(cmd) == 'r')
	{
		struct mmb_info* pmi;
		
		if ( (pmi = get_mmbinfo_safe(arg, pmu)) == NULL)
		{
			ret = -EPERM;
			goto __error_exit;
		}
		ret = mmz_userdev_ioctl_r(file, cmd, pmi);		
	}
	
	else if (_IOC_TYPE(cmd) == 'c')
	{

		struct mmb_info* pmi;

		if (arg == 0)
		{
			//mmz_flush_dcache_all();/*do not use this function*/
			ret = -EPERM;
			goto __error_exit;
		}

		if ( (pmi = get_mmbinfo_safe(arg, pmu)) == NULL)
		{
			ret = -EPERM;
			goto __error_exit;
		}

		switch (_IOC_NR(cmd))
		{
			case _IOC_NR(IOC_MMB_FLUSH_DCACHE):
				mmz_flush_dcache_mmb(pmi);
				break;

			default:
				ret = -EINVAL;
				break;
		}

	}
	else if (_IOC_TYPE(cmd) == 'd')
	{
		gv_mmb_t* mmb;
		struct mmb_info* pmi;
		struct dirty_area area;
		unsigned long offset, orig_addr;
		unsigned long virt_addr;

		if (_IOC_SIZE(cmd) != sizeof(area) || arg == 0)
		{
			error("_IOC_SIZE(cmd)=%d, arg==0x%08lx\n", _IOC_SIZE(cmd), arg);
			ret = -EINVAL;
			goto __error_exit;
		}

		memset(&area, 0, sizeof(area));

		if (copy_from_user(&area, (void*)arg, _IOC_SIZE(cmd)))
		{
			printk(KERN_WARNING "\nmmz_userdev_ioctl: copy_from_user error.\n");
			ret = -EFAULT;
			goto __error_exit;
		}

		if ((mmb = gv_mmb_getby_phys_2(area.dirty_phys_start, &offset)) == NULL)
		{
#ifdef __ARCH_ARM_64__		
			error("dirty_phys_addr=0x%llx\n", area.dirty_phys_start);
#else
			error("dirty_phys_addr=0x%lx\n", area.dirty_phys_start);
#endif
			ret = -EFAULT;
			goto __error_exit;
		}

		pmi = get_mmbinfo_safe(mmb->phys_addr, pmu);

		if (pmi == NULL)
		{
			ret = -EPERM;
			goto __error_exit;
		}

		if ((unsigned long)(area.dirty_virt_start) != (unsigned long)pmi->mapped + offset)
		{
			printk(KERN_WARNING \
				   "dirty_virt_start addr was not consistent with dirty_phys_start addr!\n");
			ret = -EFAULT;
			goto __error_exit;
		}

		if (area.dirty_phys_start + area.dirty_size > mmb->phys_addr + mmb->length)
		{
			printk(KERN_WARNING "\ndirty area overflow!\n");
			ret = -EFAULT;
			goto __error_exit;
		}

		/*cache line aligned*/
		orig_addr = area.dirty_phys_start;
		area.dirty_phys_start &= ~(CACHE_LINE_SIZE - 1);
		virt_addr = (unsigned long)area.dirty_virt_start;
		virt_addr &= ~(CACHE_LINE_SIZE - 1);
		area.dirty_virt_start = (void*)virt_addr;
		//area.dirty_virt_start &= ~(CACHE_LINE_SIZE - 1);
		area.dirty_size = (area.dirty_size + (orig_addr - area.dirty_phys_start)
						   + (CACHE_LINE_SIZE - 1)) & ~(CACHE_LINE_SIZE - 1);
		
		mmz_flush_dcache_mmb_dirty(&area);

	}
    else
    {
        ret = -EINVAL;
    }

__error_exit:

	up(&pmu->sem);

	 return ret;
}

int mmz_userdev_mmap(struct file* file, struct vm_area_struct* vma)
{
	struct mmb_info* p = NULL;
	struct mmz_userdev_info* pmu = file->private_data;
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
//	gv_mmb_t* mmb = NULL;

	p = get_mmbinfo(offset, pmu);

	if (p == NULL)
	{
		error("mmb(0x%08lX) not found?!\n", offset);
		return -EPERM;
	}


	if (p->mapped)
	{
		error("mmb(0x%08lX) have been mapped already?!\n", offset);
		return -EIO;
	}

#if 0
	if (p == NULL)
	{
		unsigned long mmb_offset;
		mmb = gv_mmb_getby_phys_2(offset, &mmb_offset);

		if (mmb == NULL)
		{
			/* Allow mmap MMZ allocated by other core. */
			error("mmb(0x%08lX) not found?!\n", offset);
			return -EPERM;
		}
		else
		{
			mmb_cached = mmb->flags & GV_MMB_MAP2KERN;
		  //mmb_cached = mmb->flags & GV_MMB_MAP2KERN_CACHED;
		}
	}
	else
	{
		if (p->mapped)
		{
#if 1
			error("mmb(0x%08lX) have been mapped already?!\n", offset);
			return -EIO;
#endif
		}
	}
#endif	
		/*
		 * Remap-pfn-range will mark the range
		 * as VM_IO and VM_RESERVED
		 */
		if (remap_pfn_range(vma,
							vma->vm_start,
							vma->vm_pgoff,
							vma->vm_end - vma->vm_start,
							vma->vm_page_prot))
		{ 
			error("remap_pfn_range fail!\n");
			return -EAGAIN; 
		}

	return 0;
}




static int mmz_userdev_release(struct inode* inode, struct file* file)
{
    struct mmz_userdev_info* pmu = file->private_data;
    struct mmb_info* p, *n;

    list_for_each_entry_safe(p, n, &pmu->list, list)
    {
        error("MMB LEAK(pid=%d): 0x%lX, %lu bytes, '%s'\n", \
              pmu->pid, gv_mmb_phys(p->mmb), \
              gv_mmb_length(p->mmb),
              gv_mmb_name(p->mmb));

        /*
         * we do not need to release mapped-area here,
         * system will do it for us
         */
        if (p->mapped)
        {
			warning("mmb<0x%lx> mapped to userspace 0x%p will be unmaped!\n",
					p->phys_addr, p->mapped);
        }

        for (; p->mmb_ref > 0; p->mmb_ref--)
        { gv_mmb_put(p->mmb); }

        _usrdev_mmb_free(p);
    }

    file->private_data = NULL;
    kfree(pmu);

    return 0;
}

static struct file_operations mmz_userdev_fops =
{
    .owner	= THIS_MODULE,
    .open	= mmz_userdev_open,
    .release = mmz_userdev_release,

#ifdef CONFIG_COMPAT
	.compat_ioctl = compat_mmz_userdev_ioctl,
#endif
	.unlocked_ioctl = mmz_userdev_ioctl,

    .mmap	= mmz_userdev_mmap,
};

static struct miscdevice mmz_userdev =
{
    .minor	= MISC_DYNAMIC_MINOR,
    .fops	= &mmz_userdev_fops,
    .name	= "mmz_userdev"
};


int __init mmz_userdev_init(void)
{
	int ret;
	ret = misc_register(&mmz_userdev);

	if(ret)
	{
		printk("register mmz dev failure!\n");
		return -1;
	}

	return 0;
}

void __exit mmz_userdev_exit(void)
{
	misc_deregister(&mmz_userdev);
}


