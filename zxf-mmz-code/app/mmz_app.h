#ifndef _GV_MMZ_INC_H
#define _GV_MMZ_INC_H

#define GV_MMZ_NAME_LEN 32
#define GV_MMB_NAME_LEN 16

#define __phys_addr_type__  unsigned long
#define __phys_len_type__   unsigned long

struct mmb_info
{
    unsigned long phys_addr; 	/* phys-memory address */
    unsigned long  align ;		/* if you need your phys-memory have special align size */
    unsigned long  size;		/* length of memory you need, in bytes */

    void*  mapped;		/* userspace mapped ptr */

    char mmb_name[GV_MMB_NAME_LEN];
    char mmz_name[GV_MMZ_NAME_LEN];
    unsigned long gfp;		/* reserved, do set to 0 */
} __attribute__((aligned(8)));

#if 0
struct mmb_info
{
    __phys_addr_type__ phys_addr; 	/* phys-memory address */
    __phys_addr_type__ __phys_addr_align__ align ;		/* if you need your phys-memory have special align size */
    __phys_len_type__ __phys_addr_align__ size;		/* length of memory you need, in bytes */
    unsigned int __phys_addr_align__ order;

    void* __phys_addr_align__ mapped;		/* userspace mapped ptr */

	union
	{
		struct
		{
			unsigned long prot	: 8;	/* PROT_READ or PROT_WRITE */
			unsigned long flags : 12; /* MAP_SHARED or MAP_PRIVATE */

#ifdef __KERNEL__
			unsigned long reserved : 8; 
			unsigned long delayed_free : 1;
			unsigned long map_cached : 1; 
#endif
		};
		unsigned long w32_stuf;
	} __phys_addr_align__;

    char mmb_name[GV_MMB_NAME_LEN];
    char mmz_name[GV_MMZ_NAME_LEN];
    unsigned long __phys_addr_align__ gfp;		/* reserved, do set to 0 */
} __attribute__((aligned(8)));
#endif
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

#endif
