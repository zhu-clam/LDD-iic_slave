#ifndef __HI_IPCM_SYSDEPS_HEADER__
#define __HI_IPCM_SYSDEPS_HEADER__

#ifndef NULL
#define NULL ((void *)0)
#endif
#define IPCM_BUG() do { pr("\n\nBUG at %s %d\n\n", __FILE__, __LINE__); *(int *)0xFFFFFFFF = 0; } while (0)

#define IPCM_PROC_NAME		"ck810_ipcm"

#define IPCM_TRACE_MASK		(0x3f)
//#define IPCM_TRACE_MASK	(0)
#define TRACE_DEV		(0<<0)
#define TRACE_MSG	    (0<<1)
#define TRACE_MEM		(0<<2)
#define TRACE_INIT		(1<<3)
#define TRACE_DESC		(0<<4)
#define TRACE_ZXF_DEBUG (0<<5) 

void pr(const char *fmt, ...);

#define ipcm_info(s...)	do { \
	pr("<ipcm> "); \
	pr(s); \
	pr("\r\n"); \
} while (0)

#define ipcm_err(s...) do { \
	pr("<err>[%s:%d] ", __func__, __LINE__); \
	pr(s); \
	pr("\r\n"); \
} while (0)

#define ipcm_trace(mask, s...) do { \
	if (mask & IPCM_TRACE_MASK) { \
		pr("<dbg>[%s:%d] ", __func__, __LINE__); \
		pr(s); \
		pr("\r\n"); \
	} \
} while (0)

struct ipcm_lock {
	long os_lock;
	void *priv;
};

struct ipcm_timer {
	long os_timer;
	void *priv;
};

struct ipcm_task {
	long os_task;
	long priv;
};

struct ipcm_event {
	long os_wait;
	long state;
};
#ifdef __KERNEL__
#include <linux/atomic.h>
typedef atomic_t ipcm_atomic_t;
#elif defined(__LITEOS__)
#include "los_atomic.h"
typedef int ipcm_atomic_t;
#else
typedef int ipcm_atomic_t;
#endif

void *__memcpy__(void *dest, const void *src, unsigned int n);
void *__memset__(void *s, int c, unsigned int n);

int __interrupt_context__(void);
void *__ipcm_mem_alloc__(int size);
void __ipcm_mem_free__(void *mem);
unsigned int __ipcm_irq_save__(void);
void __ipcm_irq_restore__(unsigned long flags);
int __ipcm_lock_init__(struct ipcm_lock *ilock);
int __ipcm_lock__(struct ipcm_lock *ilock);
int __ipcm_unlock__(struct ipcm_lock *ilock);
void __ipcm_lock_free__(struct ipcm_lock *ilock);
void * __ipcm_io_mapping__(unsigned long addr, unsigned int sz);
void __ipcm_io_unmapping__(void *addr);
void __ipcm_msleep__(unsigned int ms);
struct ipcm_task *__ipcm_thread_create__(char *name, int (*fn)(void *p), void *data);
//int __ipcm_thread_check_stop__(void);
int __ipcm_thread_check_stop__(struct ipcm_task *ptask);
void __ipcm_thread_destroy__(struct ipcm_task *ptask);
int __ipcm_irq_request__(int irq, int (*fn)(int irq, void *data));
void __ipcm_irq_free__(int irq);
//void __arch_sgi_init__(int id);
int __arch_init__(void);
void __arch_free__(void);
void __interrupt_trigger__(int cpu, int irq);
//void __ipcm_interrupt_init__(int id);
int __ipcm_timer_create__(struct ipcm_timer *itimer, void (*callback)(unsigned long),
		int ms, void *data);
void __ipcm_timer_free__(struct ipcm_timer *itimer);
void __ipcm_timer_restart__(struct ipcm_timer *itimer, int ms);
int __ipcm_event_init__(struct ipcm_event *ievent);
void __ipcm_event_free__(struct ipcm_event *ievent);
int __ipcm_wait_event__(struct ipcm_event *ievent);
void __ipcm_wakeup_event__(struct ipcm_event *ievent);

int __wait_all_cores_ready__(void);

int __ipcm_read_proc__(void * data);
void __ipcm_proc_printf__(void *data, const char *f, ...);

int __ipcm_atomic_read__(ipcm_atomic_t *v);
void __ipcm_atomic_set__(ipcm_atomic_t *v, int i);
void __ipcm_atomic_inc__(ipcm_atomic_t *v);
void __ipcm_atomic_dec__(ipcm_atomic_t *v);

#endif /* __HI_IPCM_LINUX_SYSDEPS_HEADER__ */

