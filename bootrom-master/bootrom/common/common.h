/*
 * (Copyright 2017) Verisilicon Inc. All Rights Reserved
 * Company confidential and Proprietary information.
 * This information may not be disclosed to unauthorized individual.
 * 
 * System Application Team <fengyin.wu@verisilicon.com>
 *
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include "datatype.h"
#include "platform.h"

//error num
#define OK                0    /* OK */
#define EINVAL            1    /* Invalid argument */
#define EIO               2    /* I/O error */
#define EPERM             3    /* Operation not permitted */
#define ENOMEM            4    /* Out of memory */
#define EBUSY             5    /* Device or resource busy */
#define EFAULT            6    /* Bad address */
#define ENODEV            7    /* No such device */
#define ECRC              8    /* crc error */
#define ETIMEDOUT         9    /* time out*/
#define BOOT_FAILED       10  /* boot failed */
#define ECOMM             70    /* Communication error on send */
#define EOPNOTSUPP        95    /* Operation not supported on transport endpoint */

#define ENOMEDIUM         123    /* No medium found */

#define ENOTSUPP          524    /* Operation is not supported */

#define  inline
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

#define ARRAY_SIZE(x)        (sizeof(x) / sizeof((x)[0]))
//#define true  1
//#define false 0

#define SZ_1                0x00000001
#define SZ_2                0x00000002
#define SZ_4                0x00000004
#define SZ_8                0x00000008
#define SZ_16                0x00000010
#define SZ_32                0x00000020
#define SZ_64                0x00000040
#define SZ_128                0x00000080
#define SZ_256                0x00000100
#define SZ_512                0x00000200

#define SZ_1K                0x00000400
#define SZ_2K                0x00000800
#define SZ_4K                0x00001000
#define SZ_8K                0x00002000
#define SZ_16K                0x00004000
#define SZ_32K                0x00008000
#define SZ_64K                0x00010000
#define SZ_128K                0x00020000
#define SZ_256K                0x00040000
#define SZ_512K                0x00080000

#define SZ_1M                0x00100000
#define SZ_2M                0x00200000
#define SZ_4M                0x00400000
#define SZ_8M                0x00800000
#define SZ_16M                0x01000000
#define SZ_32M                0x02000000
#define SZ_64M                0x04000000
#define SZ_128M                0x08000000
#define SZ_256M                0x10000000
#define SZ_512M                0x20000000

#define SZ_1G                0x40000000
#define SZ_2G                0x80000000
/*
 * min()/max()/clamp() macros that also do
 * strict type-checking.. See the
 * "unnecessary" pointer comparison.
 */
#define min(x, y) ({                \
    typeof(x) _min1 = (x);          \
    typeof(y) _min2 = (y);          \
    (void) (&_min1 == &_min2);      \
    _min1 < _min2 ? _min1 : _min2; })

#define max(x, y) ({                \
    typeof(x) _max1 = (x);          \
    typeof(y) _max2 = (y);          \
    (void) (&_max1 == &_max2);      \
    _max1 > _max2 ? _max1 : _max2; })

#define MIN min
#define MAX max
#define ffs generic_ffs
void usleep(unsigned long useconds);
void delay_ms(unsigned long mseconds);
void delay_s(unsigned long seconds);
/*
 * ffs: find first bit set. This is defined the same way as
 * the libc and compiler builtin ffs routines, therefore
 * differs in spirit from the above ffz (man ffs).
 */

static inline int generic_ffs(int x)
{
    int r = 1;

    if (!x)
        return 0;
    if (!(x & 0xffff)) {
        x >>= 16;
        r += 16;
    }
    if (!(x & 0xff)) {
        x >>= 8;
        r += 8;
    }
    if (!(x & 0xf)) {
        x >>= 4;
        r += 4;
    }
    if (!(x & 3)) {
        x >>= 2;
        r += 2;
    }
    if (!(x & 1)) {
        x >>= 1;
        r += 1;
    }
    return r;
}
//void timer_usleep(unsigned long useconds);

#endif
