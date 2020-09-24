/*
 * cache.h - CKCORE cpu cache flush interface
 * 
 * Copyright (C): 2008 2009 Hangzhou C-SKY Microsystem Co.,LTD.
 * Author: Ye Yun  (yun_ye@c-sky.com)
 * Contrbutior: Chunqiang Li
 * Date: 2008-12-29
 */

#ifndef __BOOTLOAD_INCLUDE_CACHE_H
#define __BOOTLOAD_INCLUDE_CACHE_H

/*
 * Cache handling functions once operation
 */
static inline void __flush_cache_all(void)
{
  register long __b;

  __asm__ __volatile__ ("movi	%0, 0x33\n\t"
                        "mtcr	%0, cr17"
                        : "=r" (__b)); 
}



/*
* enable & disable Cache forevery
*/

void __disbale_cache_all(void)
{
  	int value = 0xc;

	/*set bit2-IE & bit3-DE*/

		__asm__ __volatile__("mfcr r1,cr18\n\t");
		__asm__ __volatile__("bclri r1,2\n\t");
		__asm__ __volatile__("bclri r1,3\n\t");
		__asm__ __volatile__("mtcr r1,cr18\n\t");


		__asm__ __volatile__("mfcr r1,cr23\n\t");
		__asm__ __volatile__("bclri r1,3\n\t");
		__asm__ __volatile__("mtcr r1,cr23\n\t");
}		





/*
 * Instruction Cache handling functions
 */
static inline void __flush_icache(void)
{
  register long __b;

  __asm__ __volatile__ ("movi	%0, 0x11\n\t"
                        "mtcr	%0, cr17"
                        : "=r" (__b)); 
}

/*
 * Data Cache handling functions
 */
static inline void __flush_dcache(void)
{
  register long __b;

  __asm__ __volatile__ ("movi	%0, 0x32\n\t"
                        "mtcr	%0, cr17"
                        : "=r" (__b)); 
}


/*
 * Data Cache handling functions
 */
static inline void __clear_dcache(void)
{
  register long __b;

  __asm__ __volatile__ ("movi	%0, 0x22\n\t"
                        "mtcr	%0, cr17"
                        : "=r" (__b));
}

/*
 * L2 Cache handling functions
 */
static inline void __flush_l2cache(void)
{
  register long __b;

  __asm__ __volatile__ ("movi	%0, 0x30\n\t"
                        "mtcr	%0, cr24"
                        : "=r" (__b));
}

#define CK_Cache_FlushAll()     __flush_cache_all()
#define CK_Cache_FlushI()       __flush_icache()
#define CK_Cache_FlushD()       __flush_dcache()
#define CK_Cache_FlushL2()      __flush_l2cache()

#endif /* __BOOTLOAD_INCLUDE_CACHE_H */
