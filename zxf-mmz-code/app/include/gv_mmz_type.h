/*************************************************************************
	> File Name: gv_mmz_type.h
	> Author: ma6174
	> Mail: ma6174@163.com 
	> Created Time: Tue 17 Sep 2019 08:19:57 PM PDT
 ************************************************************************/

#ifndef __GV_MMZ_TYPE_H__
#define __GV_MMZ_TYPE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*
 * base data type defination
 * */

typedef unsigned char           GV_U8;
typedef unsigned short          GV_U16;
typedef unsigned int            GV_U32;
typedef unsigned long           GV_UL32;

typedef signed char             GV_S8;
typedef short                   GV_S16;
typedef int                     GV_S32;
typedef signed long             GV_SL32;

/*float*/
typedef float               	GV_FLOAT;
/*double*/
typedef double                  GV_DOUBLE;

typedef char                    GV_CHAR;
/*macro define void type */
#define GV_VOID                 void

typedef enum {
    GV_FALSE = 0,
    GV_TRUE  = 1,
} GV_BOOL;

/*File descriptor*/
typedef     int                FD;


typedef unsigned int phys_addr_t;




#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus*/


#endif  /* _GV_MMZ_TYPE_H__*/

