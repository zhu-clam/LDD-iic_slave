

#ifndef __DEVICE_CONFIG_HEADER__
#define __DEVICE_CONFIG_HEADER__

#if 1
#define CK860_CPU
#ifdef CK860_CPU
#define LOCAL_ID 0                                                                                                                                                      
#define REMOTE_ID 1
#define __SHARE_MEM_RECV_BASE__ 0xF0001000  /*ck860_recv :From SRAM0*/
#define __SHARE_MEM_SEND_BASE__ 0xF0000000 /*ck860_send*/
#else
#define LOCAL_ID 1
#define REMOTE_ID 0
#define __SHARE_MEM_RECV_BASE__ 0xF0000000 /*ck810_recv = ck860_send*/
#define __SHARE_MEM_SEND_BASE__ 0xF0001000 /*ck810_send = ck860_recv*/
#endif

#define __SHARE_MEM_SIZE__ 0x1000;//4K
#define IRQ_NUM 77

#define __NODES_DESC_MEM_BASE__ 0xF0002000
#endif

#if 0
#define CK860_CPU
#ifdef CK860_CPU
#define LOCAL_ID 0                                                                                                                                                      
#define REMOTE_ID 1
#define __SHARE_MEM_RECV_BASE__ 0xF0002000  /*ck860_recv*/
#define __SHARE_MEM_SEND_BASE__ 0xF0001000 /*ck860_send*/
#else
#define LOCAL_ID 1
#define REMOTE_ID 0
#define __SHARE_MEM_RECV_BASE__ 0xF0001000 /*ck810_recv = ck860_send*/
#define __SHARE_MEM_SEND_BASE__ 0xF0002000 /*ck810_send = ck860_recv*/
#endif

#define __SHARE_MEM_SIZE__ 0x1000;//4K
#define IRQ_NUM 77

#define __NODES_DESC_MEM_BASE__ 0xF0003000
#endif

#endif


