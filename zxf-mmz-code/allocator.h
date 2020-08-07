

#ifndef __ALLOCATOR_H__
#define __ALLOCATOR_H__

#include <linux/device.h>
#include "gv_mmz.h"

#define NAME_LEN_MAX	64


struct mmz_allocator
{
    int (*init)(char* args);
    gv_mmb_t* (*mmb_alloc)(const char* name,
                            unsigned long size,
                            unsigned long align,
                            unsigned long gfp,
                            const char* mmz_name,
                            gv_mmz_t* _user_mmz);
    gv_mmb_t* (*mmb_alloc_v2)(const char* name,
                               unsigned long size,
                               unsigned long align,
                               unsigned long gfp,
                               const char* mmz_name,
                               gv_mmz_t* _user_mmz,
                               unsigned int order);
    void* (*mmb_map2kern)(gv_mmb_t* mmb, int cached);
    int (*mmb_unmap)(gv_mmb_t* mmb);
    void (*mmb_free)(gv_mmb_t* mmb);
};

//int cma_allocator_setopt(struct mmz_allocator* allocator);
int gv_allocator_setopt(struct mmz_allocator* allocator);

#endif

