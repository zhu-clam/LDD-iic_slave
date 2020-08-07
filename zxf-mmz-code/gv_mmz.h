#ifndef _GV_MMZ_H
#define _GV_MMZ_H

#define	CACHE_LINE_SIZE (0x40)
#define GV_MMZ_NAME_LEN 32
#define GV_MMB_NAME_LEN 16

struct gv_media_memory_zone {
	char name[GV_MMZ_NAME_LEN];

	unsigned long gfp;

	unsigned long phys_start;
	unsigned long nbytes; 

	struct list_head list;
    union
    {
        struct device* cma_dev;
        unsigned char* bitmap;
    };
	struct list_head mmb_list;
	
	unsigned int alloc_type;
	unsigned long block_align;

	void (*destructor)(const void *);
};
typedef struct gv_media_memory_zone gv_mmz_t;

#define GV_MMZ_FMT_S "PHYS(0x%08lX, 0x%08lX), GFP=%lu, nBYTES=%luKB,	NAME=\"%s\""
#define gv_mmz_fmt_arg(p) (p)->phys_start,(p)->phys_start+(p)->nbytes-1,(p)->gfp,(p)->nbytes/SZ_1K,(p)->name

#define GV_MMB_NAME_LEN 16
struct gv_media_memory_block {
	#ifndef MMZ_V2_SUPPORT
	unsigned int id;
	#endif
	char name[GV_MMB_NAME_LEN];
	struct gv_media_memory_zone *zone;
	struct list_head list;

	unsigned long phys_addr;
	void *kvirt;
	unsigned long length;

	unsigned long flags;
	
	unsigned int order;
	
	int phy_ref;
	int map_ref;
};
typedef struct gv_media_memory_block gv_mmb_t;

/*for mmz module Debug*/
//extern int osal_printk(const char *fmt, ...);
//extern void osal_panic(const char *fmt, const char * file, const char * fun, int line, const char *);
#define OSAL_BUG() do { } while (1)

#define OSAL_BUG_ON(expr)  \
do{\
    if(expr){\
        printk("BUG: failure at %s:%d/%s()!\n", __FILE__, __LINE__, __func__); \
        OSAL_BUG();\
    }\
}while(0)

#define gv_mmb_kvirt(p)	({gv_mmb_t *__mmb=(p); OSAL_BUG_ON(__mmb==NULL); __mmb->kvirt;})
#define gv_mmb_phys(p)		({gv_mmb_t *__mmb=(p); OSAL_BUG_ON(__mmb==NULL); __mmb->phys_addr;})/*safety get mmb->phys_addr value*/ 
#define gv_mmb_length(p)	({gv_mmb_t *__mmb=(p); OSAL_BUG_ON(__mmb==NULL); __mmb->length;})
#define gv_mmb_name(p)		({gv_mmb_t *__mmb=(p); OSAL_BUG_ON(__mmb==NULL); __mmb->name;})
#define gv_mmb_zone(p)		({gv_mmb_t *__mmb=(p); OSAL_BUG_ON(__mmb==NULL); __mmb->zone;})

#define GV_MMB_MAP2KERN	(1<<0)    
//#define GV_MMB_MAP2KERN_CACHED	(1<<1) 
#define GV_MMB_RELEASED	(1<<2) 	


#define GV_MMB_FMT_S "phys(0x%08lX, 0x%08lX), kvirt=0x%p, flags=0x%08lX, length=%luKB,	name=\"%s\""
#define gv_mmb_fmt_arg(p) (p)->phys_addr,mmz_grain_align((p)->phys_addr+(p)->length)-1,(p)->kvirt,(p)->flags,(p)->length/SZ_1K,(p)->name


#define DEFAULT_ALLOC 0
#define SLAB_ALLOC 1
#define EQ_BLOCK_ALLOC 2

#define LOW_TO_HIGH 0
#define HIGH_TO_LOW 1

#define MMZ_DBG_LEVEL 0xf
#define mmz_trace(level, s, params...) do{ if(level & MMZ_DBG_LEVEL)\
            printk(KERN_INFO "[%s, %d]: " s "\n", __FUNCTION__, __LINE__, params);\
    }while(0)

#define mmz_trace_func() mmz_trace(0x02,"%s", __FILE__)

#define MMZ_GRAIN PAGE_SIZE
#define mmz_bitmap_size(p) (mmz_align2(mmz_length2grain((p)->nbytes),8)/8)

#define mmz_get_bit(p,n) (((p)->bitmap[(n)/8]>>((n)&0x7))&0x1)
#define mmz_set_bit(p,n) (p)->bitmap[(n)/8] |= 1<<((n)&0x7)
#define mmz_clr_bit(p,n) (p)->bitmap[(n)/8] &= ~(1<<((n)&0x7))

#define mmz_pos2phy_addr(p,n) ((p)->phys_start+(n)*MMZ_GRAIN)
#define mmz_phy_addr2pos(p,a) (((a)-(p)->phys_start)/MMZ_GRAIN)

#define mmz_align2low(x,g) (((x)/(g))*(g))
#define mmz_align2(x,g) ((((x)+(g)-1)/(g))*(g))
#define mmz_grain_align(x) mmz_align2(x,MMZ_GRAIN)
#define mmz_length2grain(len) (mmz_grain_align(len)/MMZ_GRAIN)

#define begin_list_for_each_mmz(p,gfp,mmz_name) list_for_each_entry(p,&mmz_list, list) {\
        if (gfp==0 ? 0:(p)->gfp!=(gfp))\
            continue;\
        if ((mmz_name == NULL) || (*mmz_name == '\0')) {\
            if (anony == 1) {\
                if (strcmp("anonymous", p->name))\
                    continue;\
            } else\
                break;\
        } else {\
            if (strcmp(mmz_name, p->name))\
                continue;\
        }\
        mmz_trace(1, GV_MMZ_FMT_S, gv_mmz_fmt_arg(p));
#define end_list_for_each_mmz() }

#define __phys_addr_type__  unsigned long
#define __phys_len_type__   unsigned long
#define __phys_addr_align__

struct mmb_info
{
    __phys_addr_type__ phys_addr; /* phys-memory address */
    __phys_addr_type__  align ;		/* if you need your phys-memory have special align size */
    __phys_len_type__  size;		/* length of memory you need, in bytes */
    void* mapped;			/* userspace mapped ptr */

    char mmb_name[GV_MMB_NAME_LEN];
    char mmz_name[GV_MMZ_NAME_LEN];
    unsigned long  gfp;		/* reserved, do set to 0 */

#ifdef __KERNEL__
		int map_ref;
		int mmb_ref;
	
		struct list_head list;
		gv_mmb_t* mmb;
#endif
} __attribute__((aligned(8)));


struct dirty_area
{
    __phys_addr_type__ dirty_phys_start;	/* dirty physical address */
    void* __phys_addr_align__ dirty_virt_start;		/* dirty virtual  address,
					   must be coherent with dirty_phys_addr */
    __phys_len_type__ __phys_addr_align__ dirty_size;
} __phys_addr_align__;

#define IOC_MMB_ALLOC		_IOWR('m', 10,  struct mmb_info)
#define IOC_MMB_ATTR		_IOR('m',  11,  struct mmb_info)
#define IOC_MMB_FREE		_IOW('m',  12,  struct mmb_info)
#define IOC_MMB_ALLOC_V2	_IOWR('m', 13,  struct mmb_info)

#define IOC_MMB_USER_REMAP	_IOWR('m', 20,  struct mmb_info)
#define IOC_MMB_USER_REMAP_CACHED _IOWR('m', 21,  struct mmb_info)
#define IOC_MMB_USER_UNMAP	_IOWR('m', 22,  struct mmb_info)

#define IOC_MMB_VIRT_GET_PHYS	_IOWR('m',  23,  struct mmb_info)

#define IOC_MMB_ADD_REF		_IO('r', 30)	/* ioctl(file, cmd, arg), arg is mmb_addr */
#define IOC_MMB_DEC_REF		_IO('r', 31)	/* ioctl(file, cmd, arg), arg is mmb_addr */

#define IOC_MMB_FLUSH_DCACHE	_IO('c', 40)

#define IOC_MMB_FLUSH_DCACHE_DIRTY		_IOW('d', 50, struct dirty_area)
#define IOC_MMB_TEST_CACHE	_IOW('t',  11,  struct mmb_info)


#define MMZ_SETUP_CMDLINE_LEN 256
typedef struct gvMMZ_MODULE_PARAMS_S
{
    char mmz[MMZ_SETUP_CMDLINE_LEN];
    int anony;
}MMZ_MODULE_PARAMS_S;

/*
 * APIs
 */
extern gv_mmz_t* gv_mmz_create(const char* name, unsigned long gfp, unsigned long phys_start,
                                 unsigned long nbytes);
extern gv_mmz_t* gv_mmz_create_v2(const char* name, unsigned long gfp, unsigned long phys_start,
                                    unsigned long nbytes,  unsigned int alloc_type, unsigned long block_align);

extern int gv_mmz_destroy(gv_mmz_t* zone);

extern int gv_mmz_register(gv_mmz_t* zone);
extern int gv_mmz_unregister(gv_mmz_t* zone);
extern gv_mmz_t* gv_mmz_find(unsigned long gfp, const char* mmz_name);

extern gv_mmb_t* gv_mmb_alloc(const char* name, unsigned long size, unsigned long align,
                                unsigned long gfp, const char* mmz_name);
//extern gv_mmb_t* gv_mmb_alloc_v2(const char* name, unsigned long size, unsigned long align,
//                                  unsigned long gfp, const char* mmz_name, unsigned int order);
extern int gv_mmb_free(gv_mmb_t* mmb);
extern gv_mmb_t* gv_mmb_getby_phys(unsigned long addr);
extern gv_mmb_t* gv_mmb_getby_phys_2(unsigned long addr, unsigned long* Outoffset);
extern gv_mmb_t* gv_mmb_getby_kvirt(void* virt);
extern unsigned long gv_mmz_get_phys(const char *zone_name);

extern gv_mmb_t* gv_mmb_alloc_in(const char* name, unsigned long size, unsigned long align,
                                   gv_mmz_t* _user_mmz);
extern gv_mmb_t* gv_mmb_alloc_in_v2(const char* name, unsigned long size, unsigned long align,
                                      gv_mmz_t* _user_mmz, unsigned int order);

#define gv_mmb_freeby_phys(phys_addr) gv_mmb_free(gv_mmb_getby_phys(phys_addr))
#define gv_mmb_freeby_kvirt(kvirt) gv_mmb_free(gv_mmb_getby_kvirt(kvirt))

extern void* gv_mmb_map2kern(gv_mmb_t* mmb);
extern void* gv_mmb_map2kern_cached(gv_mmb_t* mmb);
#if 0
extern int gv_mmb_flush_dcache_byaddr(void* kvirt, unsigned long phys_addr, unsigned long length);
extern int gv_mmb_invalid_cache_byaddr(void* kvirt, unsigned long phys_addr, unsigned long length);
#endif
extern int gv_mmb_unmap(gv_mmb_t* mmb);
extern int gv_mmb_get(gv_mmb_t* mmb);
extern int gv_mmb_put(gv_mmb_t* mmb);

/* for mmz userdev */
int mmz_userdev_init(void);
void mmz_userdev_exit(void);
int mmz_flush_dcache_all(void);

#endif

