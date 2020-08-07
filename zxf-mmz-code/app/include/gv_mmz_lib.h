#ifndef _GV_MMZ_INC_H
#define _GV_MMZ_INC_H

#include "gv_mmz_type.h"

#define GV_MMZ_NAME_LEN 32
#define GV_MMB_NAME_LEN 16

#define __phys_addr_type__  unsigned long
#define __phys_len_type__   unsigned long

typedef struct mmb_info
{
    unsigned long phys_addr; 	/* phys-memory address */
    unsigned long  align ;		/* if you need your phys-memory have special align size */
    unsigned long  size;		/* length of memory you need, in bytes */

    void*  mapped;		/* userspace mapped ptr */

    char mmb_name[GV_MMB_NAME_LEN];
    char mmz_name[GV_MMZ_NAME_LEN];
    unsigned long gfp;		/* reserved, do set to 0 */
} GV_MMB_INFO_S __attribute__((aligned(8))) ;

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

#define IOC_MMB_TEST_CACHE	_IOW('t',  11,  struct mmb_info)

//debug
#define mmz_err(s...) do { \
	pr_ipc("<hal_err>[%s:%d] ", __func__, __LINE__); \
	pr_ipc(s); \
	pr_ipc("\r\n"); \
} while (0)


/*malloc mmb*/

GV_MMB_INFO_S* GV_MMZ_GetMMB(unsigned int size,char *mmb_name,int* mmz_fd);

/*free mmb*/
GV_BOOL GV_MMZ_FreeMMB(GV_MMB_INFO_S *mmb,int* mmz_fd);

#endif

