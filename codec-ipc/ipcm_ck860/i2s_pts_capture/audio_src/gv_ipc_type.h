/*************************************************************************
	> File Name: gv_ipc_type.h
	> Author: xianfei.zhu@byavs.com
	> 
 ************************************************************************/


#ifndef __GV_IPCM_TYPE_H__
#define __GV_IPCM_TYPE_H__

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

/*
 * assign node id for cpu
 * cpu	   :ck860  ck810  
 * node id : 0		1
 * */
//#define CK810_CPU

#define CK860_CPU

#ifdef CK860_CPU
#define TARGET_NODE 1
#define DEV_NAME "/dev/ipcm_ck860"
#else 
#define TARGET_NODE 0
#define DEV_NAME "/dev/ipcm_ck810"
#endif

#define MAX_SEND_LEN 2016
#define MAX_PORT_NUM 1023

/* Return Value of grand vision ipcm module */
typedef enum gv_ipcm_RetValue_enum {
	GV_SUCCESS = 0,
	GV_FAILED = 1,
	GV_EVALID = 2,
	GV_ERR_MALLOC = 3,
	GV_ERR_DISCONNECT,
	GV_ERR_WRITE,
	GV_ERR_READ,
	GV_ERR_FILE,
	GV_ERR_TIMEOUT,
} GV_IPCM_RetValue_E;

typedef enum {
	HANDLE_MSG_NORMAL = 0,
	HANDLE_MSG_PRIORITY = 1
} GV_IPCM_MessagePrio_E ;

typedef enum {
	HANDLE_DISCONNECTED,
	HANDLE_CONNECTING,
	HANDLE_CONNECTED
} GV_IPCM_HandleState_E;

#if 0
typedef struct gvsp_ipcm_msg_struct {
//	GV_VOID *pBody;
	GV_U8  pBody[16];/*多余一个指针*/
	GV_U32 u32Bodylen;
	GV_U8  u8Module;
	GV_U32 u32CMD;
	GV_BOOL bIsResp;
	
	struct xrp_allocation* msg_alloc;/* 保存着内存的起始物理地址,大小 */
	
} GV_IPCM_MSG_S;
#endif

typedef struct gv_ipcm_handle_attr_struct {
	GV_S32 target;
	GV_S32 port;
	GV_S32 priority;
} GV_IPCM_HANDLE_ATTR_S;

/*
* ioctl CMD use for ipcm 
*/
#define	GV_IPCM_IOC_BASE  'M'

#define GV_IPCM_IOC_CONNECT  \
	_IOW(GV_IPCM_IOC_BASE, 1, GV_IPCM_HANDLE_ATTR_S)
#define GV_IPCM_IOC_TRY_CONNECT  \
	_IOW(GV_IPCM_IOC_BASE, 2, GV_IPCM_HANDLE_ATTR_S)

#define GV_IPCM_IOC_CHECK  \
	_IOW(GV_IPCM_IOC_BASE, 3, GV_UL32)
#define GV_IPCM_IOC_DISCONNECT  \
	_IOW(GV_IPCM_IOC_BASE, 4, GV_UL32)
#define GV_IPCM_IOC_GET_LOCAL_ID \
	_IOW(GV_IPCM_IOC_BASE, 5, GV_UL32)

	
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus*/

#endif /* _GV_IPCM_TYPE_H__*/
