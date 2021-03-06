/*
 * Copyright (c) 2016 - 2017 Grand Vision Design Systems Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "xrp_alloc.h"

//#define PAGE_SIZE 4096
#define XRP_PAGE_SIZE 4
#define GFP_KERNEL 0
#define ALIGN(v, a) (((v) + (a) - 1) & -(a))

//#define DEBUG

enum {
	false,
	true,
};

typedef int bool;

#ifdef DEBUG
#define pr_debug printf
#else
static inline void pr_debug(const char *fmt, ...)
{
	(void)fmt;
}
#endif

static void *kzalloc(size_t sz, int flags)
{
	(void)flags;
	return calloc(1, sz);
}

static void kfree(void *p)
{
	free(p);
}

static void xrp_pool_lock(struct xrp_allocation_pool *pool)
{
//	pthread_mutex_lock(pool->pool_mutex);
}

static void xrp_pool_unlock(struct xrp_allocation_pool *pool)
{
//	pthread_mutex_unlock(pool->pool_mutex);
}

static void xrp_allocation_get(struct xrp_allocation *allocation)
{
}

static void atomic_set(atomic_t *p, uint32_t v)
{
	*((volatile atomic_t *)p) = v;
}
/*
* brief： 初始化内存池 
* 参数：pool -  [out]返回初始化的内存池指针	
* 		start - [in] 内存池的起始地址,输入物理地址,起始地址的确定?
*       size  - [in] 内存池的大小,size的确定?
* 返回值： 0 success
*/
long xrp_init_pool(struct xrp_allocation_pool *pool,phys_addr_t start, u32 size)
{
	struct xrp_allocation *allocation = malloc(sizeof(*allocation));

	*allocation = (struct xrp_allocation)
	{
		.start = start,
		.size = size,
		.pool = pool,
	};
	*pool = (struct xrp_allocation_pool)
	{
		.start = start,
		.size = size,
		.free_list = allocation,//用于释放内存时,找到对应需释放的内存指针
	};
	return 0;
}

void xrp_free(struct xrp_allocation *xrp_allocation)
{
	struct xrp_allocation_pool *pool = xrp_allocation->pool;
	struct xrp_allocation **pcur;

	pr_debug("%s: %pap x %d\n", __func__,
		 &xrp_allocation->start, xrp_allocation->size);

	xrp_pool_lock(pool);
	
	/* 遍历申请的内存链表xrp_allocation *next */
	for (pcur = &pool->free_list; ; pcur = &(*pcur)->next) {
		struct xrp_allocation *cur = *pcur;

		/*如果当前内存申请指针*/
		if (cur && cur->start + cur->size == xrp_allocation->start) {
			struct xrp_allocation *next = cur->next;

			pr_debug("merging block tail: %pap x 0x%x ->\n",
				 &cur->start, cur->size);
			cur->size += xrp_allocation->size;
			pr_debug("... -> %pap x 0x%x\n",
				 &cur->start, cur->size);
			kfree(xrp_allocation);

			if (next && cur->start + cur->size == next->start) {
				pr_debug("merging with next block: %pap x 0x%x ->\n",
					 &cur->start, cur->size);
				cur->size += next->size;
				cur->next = next->next;
				pr_debug("... -> %pap x 0x%x\n",
					 &cur->start, cur->size);
				kfree(next);
			}
			break;
		}

		if (!cur || xrp_allocation->start < cur->start) {
			if (cur && xrp_allocation->start + xrp_allocation->size == cur->start) {
				pr_debug("merging block head: %pap x 0x%x ->\n",
					 &cur->start, cur->size);
				cur->size += xrp_allocation->size;
				cur->start = xrp_allocation->start;
				pr_debug("... -> %pap x 0x%x\n",
					 &cur->start, cur->size);
				kfree(xrp_allocation);
			} else {
				pr_debug("inserting new free block\n");
				xrp_allocation->next = cur;
				*pcur = xrp_allocation;
			}
			break;
		}
	}

	xrp_pool_unlock(pool);
}

/* 
* 向内存池中 申请 size 大小的内存 , 返回申请内存的起始地址
* 参数：pool 之前初始化的内存池
* 		size 需要申请内存大小
*       align 按多少字节对齐
*       ** alloc [OUT]返回 申请内存的结构体地址的地址
*/
long xrp_allocate(struct xrp_allocation_pool *pool,u32 size, u32 align, struct xrp_allocation **alloc)
{
	struct xrp_allocation **pcur;
	struct xrp_allocation *cur = NULL;
	struct xrp_allocation *new;
	phys_addr_t aligned_start = 0;//起始物理地址
	bool found = false;

	pr_debug("entry xrp_allocate func!\n");
	if (!size || (align & (align - 1)))
		return -EINVAL;
	if (!align)
		align = 1;

	/* 作为临时保存返回申请内存的地址 */
	new = kzalloc(sizeof(struct xrp_allocation), GFP_KERNEL);
	if (!new)
		return -ENOMEM;

	align = ALIGN(align, XRP_PAGE_SIZE);//页UP对齐
	size = ALIGN(size, XRP_PAGE_SIZE);

	xrp_pool_lock(pool);

	/* on exit free list is fixed */
	for (pcur = &pool->free_list; *pcur; pcur = &(*pcur)->next) {
		cur = *pcur;
		aligned_start = ALIGN(cur->start, align);

		if (aligned_start >= cur->start &&
		    aligned_start - cur->start + size <= cur->size) {// 内存池的内存,比要申请的内存多时
			if (aligned_start == cur->start) {
				if (aligned_start + size == cur->start + cur->size) {//size == cur->size 申请使用所有的块
					pr_debug("reusing complete block: %pap x %x\n", &cur->start, cur->size);
					*pcur = cur->next;
				} else {
					pr_debug("cutting block head: %pap x %x ->\n", &cur->start, cur->size);
					cur->size -= aligned_start + size - cur->start;//cur->size 代表内存池剩余内存大小,aligned_start-cur->start代表对齐消耗
					cur->start = aligned_start + size;//cur->start 代表内存池剩余内存起始地址
					pr_debug("... -> %pap x %x\n", &cur->start, cur->size);
					cur = NULL;
				}
			} else {
				if (aligned_start + size == cur->start + cur->size) {
					pr_debug("cutting block tail: %pap x %x ->\n", &cur->start, cur->size);
					cur->size = aligned_start - cur->start;
					pr_debug("... -> %pap x %x\n", &cur->start, cur->size);
					cur = NULL;
				} else {
					pr_debug("splitting block into two: %pap x %x ->\n", &cur->start, cur->size);
					new->start = aligned_start + size;
					new->size = cur->start + cur->size - new->start;

					cur->size = aligned_start - cur->start;

					new->next = cur->next;
					cur->next = new;
					pr_debug("... -> %pap x %x + %pap x %x\n", &cur->start, cur->size, &new->start, new->size);

					cur = NULL;
					new = NULL;
				}
			}
			found = true;
			break;
		} else {
			cur = NULL;
		}
	}

	xrp_pool_unlock(pool);

	if (!found) {
		kfree(cur);
		kfree(new);
		return -ENOMEM;
	}

	if (!cur) {
		cur = new;
		new = NULL;
	}
	if (!cur) {
		cur = kzalloc(sizeof(struct xrp_allocation), GFP_KERNEL);
		if (!cur)
			return -ENOMEM;
	}
	if (new)
		kfree(new);

	pr_debug("returning: %pap x %x\n", &aligned_start, size);
	/* 将输入的物理地址,转换成虚拟地址 */
	cur->start = aligned_start;
	cur->size = size;
	cur->pool = pool;
	atomic_set(&cur->ref, 0);
	xrp_allocation_get(cur);
	*alloc = cur;// 返回物理地址.

	return 0;
}

phys_addr_t xrp_allocation_offset(const struct xrp_allocation *allocation)
{
	return allocation->start - allocation->pool->start;
}
