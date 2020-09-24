/*
 * File: datatype.h
 * Description: define the base data type for ckcore boot loader
 *
 * Copyright (C):  2008 C-SKY Microsystem  Ltd.
 * Author(s): Yongjiang Lu   (yongjiang_lu@c-sky.com)
 * Contributors: Chunqiang Li
 * Date:         2008-9-26
 */

#ifndef	__DATATYPE_H__
#define	__DATATYPE_H__

////////////////////////////////////////////////////////////////////////////////////////
//
//
#ifndef NULL
#define	NULL  0x00
#endif

#ifndef TRUE
#define TRUE  0x01
#endif
#ifndef FALSE
#define FALSE 0x00
#endif

#ifndef true
#define true  0x01
#endif
#ifndef false
#define false 0x00
#endif

#ifndef SUCCESS
#define SUCCESS  0
#endif
#ifndef FAILURE
#define FAILURE  -1
#endif
#define TIMEOUT         0x1000

#define	STATUS_ERR	1
#define	STATUS_OK	0

typedef	unsigned char       CK_UINT8;
typedef unsigned short      CK_UINT16;
typedef unsigned int        CK_UINT32;
typedef	signed char         CK_INT8;
typedef signed short        CK_INT16;
typedef signed int          CK_INT32;
/* typedef signed long long    CK_INT64; */
typedef signed long         CK_INT64;
typedef unsigned long long  CK_UINT64;
typedef unsigned int        BOOL;
#ifndef BYTE
typedef	unsigned char	    BYTE;
#endif
#ifndef WORD
typedef unsigned short	    WORD;
#endif

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;

typedef unsigned int        bool;

#define CK_REG  CK_UINT32
#define CK_SREG CK_UINT16
#define CK_CREG CK_UINT8

#define ALIGN_4 __attribute__((aligned (4)))
#define ALIGN_16 __attribute__((aligned (16)))
#define ALIGN_32 __attribute__((aligned (32)))
#define ALIGN_256 __attribute__((aligned (256)))
#define INT_DATA __attribute((section(".intdata, \"aw\"")))

// FIXME:
typedef struct
{
	CK_UINT16 year;
	CK_UINT8  month;
	CK_UINT8  day;
	CK_UINT8  weekday;
	CK_UINT8  hour;
	CK_UINT8  min;
	CK_UINT8  sec;
}__attribute__((packed)) RTCTIME, *PRTCTIME;


#if defined(DEBUG)
#define Debug     printf
#else
#define Debug
#endif

#define  IN
#define  OUT
#define INOUT

#define read_mreg32( addr )				*(volatile unsigned int *)(addr)
#define write_mreg32( addr, val)		*(volatile unsigned int *)(addr)= (volatile unsigned int)(val)
#define read_mreg16( addr )				*(volatile unsigned short *)(addr)
#define write_mreg16( addr, val)		*(volatile unsigned short *)(addr) = (volatile unsigned short)(val)
#define read_mreg8( addr )				*(volatile unsigned char *)(addr)
#define write_mreg8( addr, val)			*(volatile unsigned char *)(addr) = (volatile unsigned char)(val)
#define write_iram32( addr, val)		*(volatile unsigned int *)(addr) = (val)
#define write_iram16( addr,	val)		*(volatile unsigned short *)(addr) =(val)
#define read_iram32(addr)				*(volatile unsigned int *)(addr)
#define read_iram16(addr)				*(volatile unsigned short *)(addr)

#define roundup(x, y)		((((x) + ((y) - 1)) / (y)) * (y))

#define BIT(nr)                     (1UL << (nr))

#define BITS_PER_LONG 32
#define GENMASK(h, l) \
	(((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

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

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

///////////////////////////////////////////////////////////////////////////////////////
#endif  // __DATATYPE_H__
///////////////////////////////////////////////////////////////////////////////////////
