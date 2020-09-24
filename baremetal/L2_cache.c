/*****************************************************************************


 *****************************************************************************/
#include <string.h>
#include "ck810.h"
#include "datatype.h"
#include "timer.h"
#include "cache.h"
#include "misc.h"

CK_UINT32 matrix[4196][32]={{1,2},}; //262K byte

void CK_Timer0_Start()
{
    CK_Timer_Init();
    CK_Timer_Open(0, 0x0, CK_INTC_TIM0 + 0, FALSE);
    CK_Timer_Start(0, 0x8888888);
}

CK_UINT32 CK_Timer0_Read()
{
    CK_UINT32 Tvalue=0;
    Tvalue = CK_Timer_CurrentValue(0);
	return Tvalue;
}

void bad_acesss()
{
    CK_UINT32 k,j ,sum=0;
	for(k=0;k<32;k++)
		for(j=0;j<4196;j++)
			sum +=matrix[j][k];

}

static inline void counter_init(void)
{
  register long __b;

  __asm__ __volatile__ ("lrw r1,0xc000000d\n\t"
                        "cpwcr r1, <0, 0x0>"
                        : "=r" (__b)); 
}
/* SCE =1  ,TCE=1*/
static inline void Event_Counter_Close(void)
{
  register long __b;

  __asm__ __volatile__ ("lrw r1,0x0000001d\n\t"
                        "cpwcr r1, <0, 0x0>"
                        : "=r" (__b)); 
}

/* SCE =1 ,TCE=0,event counter start*/
static inline void Event_Counter_Start(void)
{
  register long __b;

  __asm__ __volatile__ ("lrw r1,0x0000000d\n\t"
                        "cpwcr r1, <0, 0x0>"
                        : "=r" (__b)); 
}


static inline void counter_start(void)
{
  register long __b;

  __asm__ __volatile__ ("lrw r1,CK_CPU_TEST_3\n\t"
                        "cpwcr r1, <0, 0x1>"
                        : "=r" (__b)); 
}

static inline void counter_end(void)
{
  register long __b;

  __asm__ __volatile__ ("lrw r1, CK_CPU_End\n\t"
                        "cpwcr r1, <0, 0x2>"
                        : "=r" (__b)); 
}

#if 1
static inline CK_UINT32 Hard_Profile_Counter_Low(void)
{
        CK_UINT32 ret = 0;
		__asm volatile("nop");
		
		__asm volatile(
						"cprgr r3, <0, 0x0>\n\t"
						"mov   %0, r3      \n\t"
						:"=r"(ret)
						:
						:"r3"
						);

		  __asm volatile("nop");	
		 return ret;
}
static inline CK_UINT32 Hard_Profile_Counter_High(void)
{
        CK_UINT32 ret = 0;
		__asm volatile("nop");
		
		__asm volatile(
						"cprgr r3, <0, 0x1>\n\t"
						"mov   %0, r3      \n\t"
						:"=r"(ret)
						:
						:"r3"
						);

		  __asm volatile("nop");	
		 return ret;
}


static inline CK_UINT32 Cycle_Counter_Low(void)
{
        CK_UINT32 ret = 0;
		__asm volatile("nop");
		
		__asm volatile(
						"cprgr r3, <0, 0x2>\n\t"
						"mov   %0, r3      \n\t"
						:"=r"(ret)
						:
						:"r3"
						);

		  __asm volatile("nop");	
		 return ret;
}
static inline CK_UINT32 Cycle_Counter_High(void)
{
        CK_UINT32 ret = 0;
		__asm volatile("nop");
		
		__asm volatile(
						"cprgr r3, <0, 0x3>\n\t"
						"mov   %0, r3      \n\t"
						:"=r"(ret)
						:
						:"r3"
						);

		  __asm volatile("nop");	
		 return ret;
}
static inline CK_UINT32 Total_Instruct_Counter_Low(void)
{
        CK_UINT32 ret = 0;
		__asm volatile("nop");
		
		__asm volatile(
						"cprgr r3, <0, 0x4>\n\t"
						"mov   %0, r3      \n\t"
						:"=r"(ret)
						:
						:"r3"
						);

		  __asm volatile("nop");	
		 return ret;
}
static inline CK_UINT32 Total_Instruct_Counter_High(void)
{
        CK_UINT32 ret = 0;
		__asm volatile("nop");
		
		__asm volatile(
						"cprgr r3, <0, 0x5>\n\t"
						"mov   %0, r3      \n\t"
						:"=r"(ret)
						:
						:"r3"
						);

		  __asm volatile("nop");	
		 return ret;
}
#endif


static inline CK_UINT32 l1icache_access_Lowcounter(void)
{
        CK_UINT32 ret = 0;
		__asm volatile("nop");
		
		__asm volatile(
						"cprgr r3, <0, 0x6>\n\t"
						"mov   %0, r3      \n\t"
						:"=r"(ret)
						:
						:"r3"
						);

		  __asm volatile("nop");	
		 return ret;
}
static inline CK_UINT32 l1icache_access_Highcounter(void)
{
        CK_UINT32 ret = 0;
		__asm volatile("nop");
		
		__asm volatile(
						"cprgr r3, <0, 0x7>\n\t"
						"mov   %0, r3      \n\t"
						:"=r"(ret)
						:
						:"r3"
						);

		  __asm volatile("nop");	
		 return ret;
}

static inline CK_UINT32 l1icache_miss_Lowcounter(void)
{
        CK_UINT32 ret = 0;
		__asm volatile("nop");
		
		__asm volatile(
						"cprgr r3, <0, 0x8>\n\t"
						"mov   %0, r3      \n\t"
						:"=r"(ret)
						:
						:"r3"
						);

		  __asm volatile("nop");	
		 return ret;
}

static inline CK_UINT32 l1icache_miss_Highcounter(void)
{
        CK_UINT32 ret = 0;
		__asm volatile("nop");
		
		__asm volatile(
						"cprgr r3, <0, 0x9>\n\t"
						"mov   %0, r3      \n\t"
						:"=r"(ret)
						:
						:"r3"
						);

		  __asm volatile("nop");	
		 return ret;
}



static inline CK_UINT32 l1dcache_acess_lowcounter(void)
{
        CK_UINT32 ret = 0;
		__asm volatile("nop");
		
		__asm volatile(
						"cprgr r3, <0, 0xA>\n\t"
						"mov   %0, r3      \n\t"
						:"=r"(ret)
						:
						:"r3"
						);

		  __asm volatile("nop");	
		 return ret;
}



static inline CK_UINT32 l1dcache_acess_Highcounter(void)
{
        CK_UINT32 ret = 0;
		__asm volatile("nop");
		
		__asm volatile(
						"cprgr r3, <0, 0xB>\n\t"
						"mov   %0, r3      \n\t"
						:"=r"(ret)
						:
						:"r3"
						);

		  __asm volatile("nop");	
		 return ret;
}


static inline CK_UINT32 l1dcache_miss_lowcounter(void)
{
        CK_UINT32 ret = 0;
		__asm volatile("nop");
		
		__asm volatile(
						"cprgr r3, <0, 0xC>\n\t"
						"mov   %0, r3      \n\t"
						:"=r"(ret)
						:
						:"r3"
						);

		  __asm volatile("nop");	
		 return ret;
}

static inline CK_UINT32 l1dcache_miss_Highcounter(void)
{
        CK_UINT32 ret = 0;
		__asm volatile("nop");
		
		__asm volatile(
						"cprgr r3, <0, 0xD>\n\t"
						"mov   %0, r3      \n\t"
						:"=r"(ret)
						:
						:"r3"
						);

		  __asm volatile("nop");	
		 return ret;
}


static inline CK_UINT32 l2cache_access_lowcounter(void)
{
        CK_UINT32 ret = 0;
		__asm volatile("nop");
		
		__asm volatile(
						"cprgr r3, <0, 0xE>\n\t"
						"mov   %0, r3      \n\t"
						:"=r"(ret)
						:
						:"r3"
						);

		  __asm volatile("nop");	
		 return ret;
}


static inline CK_UINT32 l2cache_access_highcounter(void)
{
        CK_UINT32 ret = 0;
		__asm volatile("nop");
		
		__asm volatile(
						"cprgr r3, <0, 0xF>\n\t"
						"mov   %0, r3      \n\t"
						:"=r"(ret)
						:
						:"r3"
						);

		  __asm volatile("nop");	
		 return ret;
}


static inline CK_UINT32 l2cache_miss_lowcounter(void)
{
        CK_UINT32 ret = 0;
		__asm volatile("nop");
		
		__asm volatile(
						"cprgr r3, <0, 0x10>\n\t"
						"mov   %0, r3      \n\t"
						:"=r"(ret)
						:
						:"r3"
						);

		  __asm volatile("nop");	
		 return ret;
}


static inline CK_UINT32 l2cache_miss_highcounter(void)
{
        CK_UINT32 ret = 0;
		__asm volatile("nop");
		
		__asm volatile(
						"cprgr r3, <0, 0x11>\n\t"
						"mov   %0, r3      \n\t"
						:"=r"(ret)
						:
						:"r3"
						);

		  __asm volatile("nop");	
		 return ret;

}

void CK_CPU_End() {
    int i = 0;
    
    i++;
}

void CK_CPU_TEST_3()
{
    CK_UINT32 i;
	CK_UINT32 TEST_START,TEST_END;
	TEST_START=CK_Timer0_Read();

	for(i=0;i<1000;i++)
    // CK_Cache_FlushAll();// clean cache
        bad_acesss();

	TEST_END=CK_Timer0_Read();

    printf("\n--- Test cost time =0x%x\n", TEST_START - TEST_END);
}


void CK_CPU_L2_Test()
{
    CK_UINT64 counter_low;
    CK_UINT64 counter_high;

    printf("\n--- CK860 L2 Cache Test start---\n");

    CK_Timer0_Start();

    counter_init();
	counter_start();
	counter_end();
     
    CK_CPU_TEST_3();
    CK_CPU_End();

    //JJJ_DEBUGcounter_low = Hard_Profile_Counter_Low();
    //JJJ_DEBUGcounter_high = Hard_Profile_Counter_High();
    //JJJ_DEBUGprintf("\tHard_Profile_Counter = 0x%x\n", counter_high << 32 | counter_low);
    //JJJ_DEBUG
    //JJJ_DEBUGcounter_low = Cycle_Counter_Low();
    //JJJ_DEBUGcounter_high = Cycle_Counter_High();
    //JJJ_DEBUGprintf("\tCycle_Counter = 0x%x\n", counter_high << 32 | counter_low);
    //JJJ_DEBUG
    //JJJ_DEBUGcounter_low = Total_Instruct_Counter_Low();
    //JJJ_DEBUGcounter_high = Total_Instruct_Counter_High();
    //JJJ_DEBUGprintf("\tTotal_Instruct_Counter = 0x%x\n", counter_high << 32 | counter_low);
    //JJJ_DEBUG
    //JJJ_DEBUGcounter_low = l1icache_access_Lowcounter();
    //JJJ_DEBUGcounter_high = l1icache_access_Highcounter();
    //JJJ_DEBUGprintf("\tl1icache_access_Counter = 0x%x\n", counter_high << 32 | counter_low);
    //JJJ_DEBUG
    //JJJ_DEBUGcounter_low = l1icache_miss_Lowcounter();
    //JJJ_DEBUGcounter_high = l1icache_miss_Highcounter();
    //JJJ_DEBUGprintf("\tl1icache_miss_Counter = 0x%x\n", counter_high << 32 | counter_low);
    //JJJ_DEBUG
    //JJJ_DEBUGcounter_low = l1dcache_acess_lowcounter();
    //JJJ_DEBUGcounter_high = l1dcache_acess_Highcounter();
    //JJJ_DEBUGprintf("\tl1dcache_acess_counter = 0x%x\n", counter_high << 32 | counter_low);
    //JJJ_DEBUG
    //JJJ_DEBUGcounter_low = l1dcache_miss_lowcounter();
    //JJJ_DEBUGcounter_high = l1dcache_miss_Highcounter();
    //JJJ_DEBUGprintf("\tl1dcache_miss_counter = 0x%x\n", counter_high << 32 | counter_low);
    //JJJ_DEBUG
    //JJJ_DEBUGcounter_low = l2cache_access_lowcounter();
    //JJJ_DEBUGcounter_high = l2cache_access_highcounter();
    //JJJ_DEBUGprintf("\tl2cache_access_counter = 0x%x\n", counter_high << 32 | counter_low);
    //JJJ_DEBUG
    //JJJ_DEBUGcounter_low = l2cache_miss_lowcounter();
    //JJJ_DEBUGcounter_high = l2cache_miss_highcounter();
    //JJJ_DEBUGprintf("\tl2cache_miss_counter = 0x%x\n", counter_high << 32 | counter_low);
    
    printf("\n--- CK860 L2 Cache Test end---\n");

}

