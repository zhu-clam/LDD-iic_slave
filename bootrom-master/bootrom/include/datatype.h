/*
 * File: datatype.h
 * Description: define the base data type for ckcore boot loader
 *
 * Copyright (C):  2008 C-SKY Microsystem  Ltd.
 * Author(s): Yongjiang Lu   (yongjiang_lu@c-sky.com)
 * Contributors: Chunqiang Li
 * Date:         2008-9-26
 */

#ifndef    __DATATYPE_H__
#define    __DATATYPE_H__

////////////////////////////////////////////////////////////////////////////////////////
//
//
#ifndef NULL
#define NULL  0x00
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

#define STATUS_ERR    1
#define STATUS_OK    0

typedef unsigned char       CK_UINT8;
typedef unsigned short      CK_UINT16;
typedef unsigned int        CK_UINT32;
typedef signed char         CK_INT8;
typedef signed short        CK_INT16;
typedef signed int          CK_INT32;
/* typedef signed long long    CK_INT64; */
typedef signed long         CK_INT64;
typedef unsigned long long  CK_UINT64;
typedef unsigned int        BOOL;
#ifndef BYTE
typedef unsigned char        BYTE;
#endif
#ifndef WORD
typedef unsigned short        WORD;
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

typedef void (*LOAD_ENTRY)(void);

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

#define IN
#define OUT
#define INOUT

#define read_mreg32( addr )                 *(volatile unsigned int *)(addr)
#define write_mreg32( addr, val)            *(volatile unsigned int *)(addr)= (volatile unsigned int)(val)
#define read_mreg16( addr )                 *(volatile unsigned short *)(addr)
#define write_mreg16( addr, val)            *(volatile unsigned short *)(addr) = (volatile unsigned short)(val)
#define read_mreg8( addr )                  *(volatile unsigned char *)(addr)
#define write_mreg8( addr, val)                *(volatile unsigned char *)(addr) = (volatile unsigned char)(val)
#define write_iram32( addr, val)               *(volatile unsigned int *)(addr) = (val)
#define write_iram16( addr, val)               *(volatile unsigned short *)(addr) =(val)
#define read_iram32(addr)                     *(volatile unsigned int *)(addr)
#define read_iram16(addr)                     *(volatile unsigned short *)(addr)

#define roundup(x, y)        ((((x) + ((y) - 1)) / (y)) * (y))

#define BIT(nr)                     (1UL << (nr))

#define BITS_PER_LONG 32
#define GENMASK(h, l) \
    (((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

///////////////////////////////////////////////////////////////////////////////////////
#endif  // __DATATYPE_H__
///////////////////////////////////////////////////////////////////////////////////////
