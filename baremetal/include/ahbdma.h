/*****************************************************************************
 *  File: ahbdma.h
 *
 *  Descirption: contains the functions support Synopsys AHB DMA Controller.
 *
 *  Copyright (C) : 2018 VeriSilicon.
 *
 *  Author: Chunming Li
 *  Mail:   chunming.li@verisilicon.com
 *  Date:   Feb 13 2018
 *
 *****************************************************************************/

#ifndef _DMAC_H
#define _DMAC_H

#include "ck810.h"
#include "datatype.h"

#define AHB_DMAC_MAX_CHANNELS 8

/*
 * whether to dump the register information on stdout
 */
#define DMA0_VERBOSE 0

#define DMAC_CHANEL(n) n

//********************************************************************
//Core Configuration
//********************************************************************
#define CC_DMAC_VERSION_ID            0x3231302a
#define CC_DMAC_NUM_MASTER_INT        4
#define CC_DMAC_NUM_CHANNELS          8
#define CC_DMAC_NUM_HS_INT            2
#define CC_DMAC_ID_NUM                0x0
#define CC_DMAC_INTR_POL              0
#define CC_DMAC_INTR_IO               2
#define CC_DMAC_BIG_ENDIAN            0
#define CC_DMAC_M1_AHB_LITE           0
#define CC_DMAC_M2_AHB_LITE           0
#define CC_DMAC_M3_AHB_LITE           0
#define CC_DMAC_M4_AHB_LITE           0
#define CC_DMAC_M1_HDATA_WIDTH        32
#define CC_DMAC_M2_HDATA_WIDTH        32
#define CC_DMAC_M3_HDATA_WIDTH        32
#define CC_DMAC_M4_HDATA_WIDTH        32
#define CC_DMAC_S_HDATA_WIDTH         32
#define CC_DMAC_MABRST                0
#define CC_DMAC_RETURN_ERR_RESP       1
#define CC_DMAC_ADD_ENCODED_PARAMS       0x1
#define CC_DMAC_CH0_FIFO_DEPTH        16
#define CC_DMAC_CH1_FIFO_DEPTH        16
#define CC_DMAC_CH2_FIFO_DEPTH        16
#define CC_DMAC_CH3_FIFO_DEPTH        16
#define CC_DMAC_CH4_FIFO_DEPTH        16
#define CC_DMAC_CH5_FIFO_DEPTH        16
#define CC_DMAC_CH6_FIFO_DEPTH        16
#define CC_DMAC_CH7_FIFO_DEPTH        16
#define CC_DMAC_CH0_STAT_SRC          0
#define CC_DMAC_CH1_STAT_SRC          0
#define CC_DMAC_CH2_STAT_SRC          0
#define CC_DMAC_CH3_STAT_SRC          0
#define CC_DMAC_CH4_STAT_SRC          0
#define CC_DMAC_CH5_STAT_SRC          0
#define CC_DMAC_CH6_STAT_SRC          0
#define CC_DMAC_CH7_STAT_SRC          0
#define CC_DMAC_CH0_STAT_DST          0
#define CC_DMAC_CH1_STAT_DST          0
#define CC_DMAC_CH2_STAT_DST          0
#define CC_DMAC_CH3_STAT_DST          0
#define CC_DMAC_CH4_STAT_DST          0
#define CC_DMAC_CH5_STAT_DST          0
#define CC_DMAC_CH6_STAT_DST          0
#define CC_DMAC_CH7_STAT_DST          0
#define CC_DMAC_CH0_MAX_MULT_SIZE     8
#define CC_DMAC_CH1_MAX_MULT_SIZE     8
#define CC_DMAC_CH2_MAX_MULT_SIZE     8
#define CC_DMAC_CH3_MAX_MULT_SIZE     8
#define CC_DMAC_CH4_MAX_MULT_SIZE     8
#define CC_DMAC_CH5_MAX_MULT_SIZE     8
#define CC_DMAC_CH6_MAX_MULT_SIZE     8
#define CC_DMAC_CH7_MAX_MULT_SIZE     8
#define CC_DMAC_CH0_MAX_BLK_SIZE 31
#define CC_DMAC_CH1_MAX_BLK_SIZE 31
#define CC_DMAC_CH2_MAX_BLK_SIZE 31
#define CC_DMAC_CH3_MAX_BLK_SIZE 31
#define CC_DMAC_CH4_MAX_BLK_SIZE 31
#define CC_DMAC_CH5_MAX_BLK_SIZE 31
#define CC_DMAC_CH6_MAX_BLK_SIZE 31
#define CC_DMAC_CH7_MAX_BLK_SIZE 31
#define CC_DMAC_CH0_FC                0
#define CC_DMAC_CH1_FC                0
#define CC_DMAC_CH2_FC                0
#define CC_DMAC_CH3_FC                0
#define CC_DMAC_CH4_FC                0
#define CC_DMAC_CH5_FC                0
#define CC_DMAC_CH6_FC                0
#define CC_DMAC_CH7_FC                0
#define CC_DMAC_CH0_LMS         4
#define CC_DMAC_CH1_LMS         4
#define CC_DMAC_CH2_LMS         4
#define CC_DMAC_CH3_LMS         4
#define CC_DMAC_CH4_LMS         4
#define CC_DMAC_CH5_LMS         4
#define CC_DMAC_CH6_LMS         4
#define CC_DMAC_CH7_LMS         4
#define CC_DMAC_CH0_SMS         4
#define CC_DMAC_CH1_SMS         4
#define CC_DMAC_CH2_SMS         4
#define CC_DMAC_CH3_SMS         4
#define CC_DMAC_CH4_SMS         4
#define CC_DMAC_CH5_SMS         4
#define CC_DMAC_CH6_SMS         4
#define CC_DMAC_CH7_SMS         4
#define CC_DMAC_CH0_DMS         4
#define CC_DMAC_CH1_DMS         4
#define CC_DMAC_CH2_DMS         4
#define CC_DMAC_CH3_DMS         4
#define CC_DMAC_CH4_DMS         4
#define CC_DMAC_CH5_DMS         4
#define CC_DMAC_CH6_DMS         4
#define CC_DMAC_CH7_DMS         4
#define CC_DMAC_CH0_LOCK_EN         0
#define CC_DMAC_CH1_LOCK_EN         0
#define CC_DMAC_CH2_LOCK_EN         0
#define CC_DMAC_CH3_LOCK_EN         0
#define CC_DMAC_CH4_LOCK_EN         0
#define CC_DMAC_CH5_LOCK_EN         0
#define CC_DMAC_CH6_LOCK_EN         0
#define CC_DMAC_CH7_LOCK_EN         0
#define CC_DMAC_CH0_STW         32
#define CC_DMAC_CH1_STW         32
#define CC_DMAC_CH2_STW         32
#define CC_DMAC_CH3_STW         32
#define CC_DMAC_CH4_STW         32
#define CC_DMAC_CH5_STW         32
#define CC_DMAC_CH6_STW         32
#define CC_DMAC_CH7_STW         32
#define CC_DMAC_CH0_DTW         32
#define CC_DMAC_CH1_DTW         32
#define CC_DMAC_CH2_DTW         32
#define CC_DMAC_CH3_DTW         32
#define CC_DMAC_CH4_DTW         32
#define CC_DMAC_CH5_DTW         32
#define CC_DMAC_CH6_DTW         32
#define CC_DMAC_CH7_DTW         32
#define CC_DMAC_CH0_SRC_NON_OK         1
#define CC_DMAC_CH1_SRC_NON_OK         1
#define CC_DMAC_CH2_SRC_NON_OK         1
#define CC_DMAC_CH3_SRC_NON_OK         1
#define CC_DMAC_CH4_SRC_NON_OK         1
#define CC_DMAC_CH5_SRC_NON_OK         1
#define CC_DMAC_CH6_SRC_NON_OK         1
#define CC_DMAC_CH7_SRC_NON_OK         1
#define CC_DMAC_CH0_DST_NON_OK         1
#define CC_DMAC_CH1_DST_NON_OK         1
#define CC_DMAC_CH2_DST_NON_OK         1
#define CC_DMAC_CH3_DST_NON_OK         1
#define CC_DMAC_CH4_DST_NON_OK         1
#define CC_DMAC_CH5_DST_NON_OK         1
#define CC_DMAC_CH6_DST_NON_OK         1
#define CC_DMAC_CH7_DST_NON_OK         1
#define CC_DMAC_CH0_LLP_NON_OK         0
#define CC_DMAC_CH1_LLP_NON_OK         0
#define CC_DMAC_CH2_LLP_NON_OK         0
#define CC_DMAC_CH3_LLP_NON_OK         0
#define CC_DMAC_CH4_LLP_NON_OK         0
#define CC_DMAC_CH5_LLP_NON_OK         0
#define CC_DMAC_CH6_LLP_NON_OK         0
#define CC_DMAC_CH7_LLP_NON_OK         0
#define CC_DMAC_CH0_SRC_GAT_EN         0
#define CC_DMAC_CH1_SRC_GAT_EN         0
#define CC_DMAC_CH2_SRC_GAT_EN         0
#define CC_DMAC_CH3_SRC_GAT_EN         0
#define CC_DMAC_CH4_SRC_GAT_EN         0
#define CC_DMAC_CH5_SRC_GAT_EN         0
#define CC_DMAC_CH6_SRC_GAT_EN         0
#define CC_DMAC_CH7_SRC_GAT_EN         0
#define CC_DMAC_CH0_DST_SCA_EN         0
#define CC_DMAC_CH1_DST_SCA_EN         0
#define CC_DMAC_CH2_DST_SCA_EN         0
#define CC_DMAC_CH3_DST_SCA_EN         0
#define CC_DMAC_CH4_DST_SCA_EN         0
#define CC_DMAC_CH5_DST_SCA_EN         0
#define CC_DMAC_CH6_DST_SCA_EN         0
#define CC_DMAC_CH7_DST_SCA_EN         0
#define CC_DMAC_CH0_HC_LLP         1
#define CC_DMAC_CH1_HC_LLP         1
#define CC_DMAC_CH2_HC_LLP         1
#define CC_DMAC_CH3_HC_LLP         1
#define CC_DMAC_CH4_HC_LLP         1
#define CC_DMAC_CH5_HC_LLP         1
#define CC_DMAC_CH6_HC_LLP         1
#define CC_DMAC_CH7_HC_LLP         1
#define CC_DMAC_CH0_MULTI_BLK_EN         1
#define CC_DMAC_CH1_MULTI_BLK_EN         0
#define CC_DMAC_CH2_MULTI_BLK_EN         0
#define CC_DMAC_CH3_MULTI_BLK_EN         0
#define CC_DMAC_CH4_MULTI_BLK_EN         0
#define CC_DMAC_CH5_MULTI_BLK_EN         0
#define CC_DMAC_CH6_MULTI_BLK_EN         0
#define CC_DMAC_CH7_MULTI_BLK_EN         0
#define CC_DMAC_CH0_MULTI_BLK_TYPE         0
#define CC_DMAC_CH1_MULTI_BLK_TYPE         0
#define CC_DMAC_CH2_MULTI_BLK_TYPE         0
#define CC_DMAC_CH3_MULTI_BLK_TYPE         0
#define CC_DMAC_CH4_MULTI_BLK_TYPE         0
#define CC_DMAC_CH5_MULTI_BLK_TYPE         0
#define CC_DMAC_CH6_MULTI_BLK_TYPE         0
#define CC_DMAC_CH7_MULTI_BLK_TYPE         0
#define CC_DMAC_CH0_CTL_WB_EN         0
#define CC_DMAC_CH1_CTL_WB_EN         0
#define CC_DMAC_CH2_CTL_WB_EN         0
#define CC_DMAC_CH3_CTL_WB_EN         0
#define CC_DMAC_CH4_CTL_WB_EN         0
#define CC_DMAC_CH5_CTL_WB_EN         0
#define CC_DMAC_CH6_CTL_WB_EN         0
#define CC_DMAC_CH7_CTL_WB_EN         0

//****************************************************************
//Register address
//****************************************************************
#define DMAC_CH_START_ADDR(x, n)              (CK_AHB_DMA_CONTROL(x) + n*0x58)
#define DMAC_COMMON_START_ADDR(x)           (CK_AHB_DMA_CONTROL(x) + 0x2c0)

#define DMAC_SAR_ADDRESS_OFFSET          0x000
#define DMAC_DAR_ADDRESS_OFFSET          0x008
#define DMAC_LLP_ADDRESS_OFFSET          0x010
#define DMAC_CTL_ADDRESS_OFFSET          0x018
#define DMAC_SSTAT_ADDRESS_OFFSET        0x020
#define DMAC_DSTAT_ADDRESS_OFFSET        0x028
#define DMAC_SSTATAR_ADDRESS_OFFSET      0x030
#define DMAC_DSTATAR_ADDRESS_OFFSET      0x038
#define DMAC_CFG_ADDRESS_OFFSET          0x040
#define DMAC_SGR_ADDRESS_OFFSET          0x048
#define DMAC_DSR_ADDRESS_OFFSET          0x050

#define DMAC_SAR(x, n)      (DMAC_CH_START_ADDR(x, n) + DMAC_SAR_ADDRESS_OFFSET)
#define DMAC_DAR(x, n)      (DMAC_CH_START_ADDR(x, n) + DMAC_DAR_ADDRESS_OFFSET)
#define DMAC_LLP(x, n)      (DMAC_CH_START_ADDR(x, n) + DMAC_LLP_ADDRESS_OFFSET)
#define DMAC_CTL(x, n)      (DMAC_CH_START_ADDR(x, n) + DMAC_CTL_ADDRESS_OFFSET)
#define DMAC_CTLH(x, n)     (DMAC_CTL(x, n) + 4)
#define DMAC_SSTAT(x, n)    (DMAC_CH_START_ADDR(x, n) + DMAC_SSTAT_ADDRESS_OFFSET)
#define DMAC_DSTAT(x, n)    (DMAC_CH_START_ADDR(x, n) + DMAC_DSTAT_ADDRESS_OFFSET)
#define DMAC_SSTATAR(x, n)  (DMAC_CH_START_ADDR(x, n) + DMAC_SSTATAR_ADDRESS_OFFSET)
#define DMAC_DSTATAR(x, n)  (DMAC_CH_START_ADDR(x, n) + DMAC_DSTATAR_ADDRESS_OFFSET)
#define DMAC_CFG(x, n)      (DMAC_CH_START_ADDR(x, n) + DMAC_CFG_ADDRESS_OFFSET)
#define DMAC_CFGH(x, n)     (DMAC_CFG(x, n) + 4)
#define DMAC_SGR(x, n)      (DMAC_CH_START_ADDR(x, n) + DMAC_SGR_ADDRESS_OFFSET)
#define DMAC_DSR(x, n)      (DMAC_CH_START_ADDR(x, n) + DMAC_DSR_ADDRESS_OFFSET)

#define DMAC_RAWTFR(x)            (DMAC_COMMON_START_ADDR(x) + 0x000)
#define DMAC_RAWBLOCK(x)          (DMAC_COMMON_START_ADDR(x) + 0x008)
#define DMAC_RAWSRCTRAN(x)        (DMAC_COMMON_START_ADDR(x) + 0x010)
#define DMAC_RAWDSTTRAN(x)        (DMAC_COMMON_START_ADDR(x) + 0x018)
#define DMAC_RAWERR(x)            (DMAC_COMMON_START_ADDR(x) + 0x020)
#define DMAC_STATUSTFR(x)         (DMAC_COMMON_START_ADDR(x) + 0x028)
#define DMAC_STATUSBLOCK(x)       (DMAC_COMMON_START_ADDR(x) + 0x030)
#define DMAC_STATUSSRCTRAN(x)     (DMAC_COMMON_START_ADDR(x) + 0x038)
#define DMAC_STATUSDSTTRAN(x)     (DMAC_COMMON_START_ADDR(x) + 0x040)
#define DMAC_STATUSERR(x)         (DMAC_COMMON_START_ADDR(x) + 0x048)
#define DMAC_MASKTFR(x)           (DMAC_COMMON_START_ADDR(x) + 0x050)
#define DMAC_MASKBLOCK(x)         (DMAC_COMMON_START_ADDR(x) + 0x058)
#define DMAC_MASKSRCTRAN(x)       (DMAC_COMMON_START_ADDR(x) + 0x060)
#define DMAC_MASKDSTTRAN(x)       (DMAC_COMMON_START_ADDR(x) + 0x068)
#define DMAC_MASKERR(x)           (DMAC_COMMON_START_ADDR(x) + 0x070)
#define DMAC_CLEARTFR(x)          (DMAC_COMMON_START_ADDR(x) + 0x078)
#define DMAC_CLEARBLOCK(x)        (DMAC_COMMON_START_ADDR(x) + 0x080)
#define DMAC_CLEARSRCTRAN(x)      (DMAC_COMMON_START_ADDR(x) + 0x088)
#define DMAC_CLEARDSTTRAN(x)      (DMAC_COMMON_START_ADDR(x) + 0x090)
#define DMAC_CLEARERR(x)          (DMAC_COMMON_START_ADDR(x) + 0x098)
#define DMAC_STATUSINT(x)         (DMAC_COMMON_START_ADDR(x) + 0x0a0)
#define DMAC_REQSRCREG(x)         (DMAC_COMMON_START_ADDR(x) + 0x0a8)
#define DMAC_REQDSTREG(x)         (DMAC_COMMON_START_ADDR(x) + 0x0b0)
#define DMAC_SGLRQSRCREG(x)       (DMAC_COMMON_START_ADDR(x) + 0x0b8)
#define DMAC_SGLRQDSTREG(x)       (DMAC_COMMON_START_ADDR(x) + 0x0c0)
#define DMAC_LSTSRCREG(x)         (DMAC_COMMON_START_ADDR(x) + 0x0c8)
#define DMAC_LSTDSTREG(x)         (DMAC_COMMON_START_ADDR(x) + 0x0d0)
#define DMAC_DMACFGREG(x)         (DMAC_COMMON_START_ADDR(x) + 0x0d8)
#define DMAC_CHENREG(x)           (DMAC_COMMON_START_ADDR(x) + 0x0e0)
#define DMAC_DMAIDREG(x)          (DMAC_COMMON_START_ADDR(x) + 0x0e8)
#define DMAC_DMATESTREG(x)        (DMAC_COMMON_START_ADDR(x) + 0x0f0)
#define DMAC_DMA_COMP_PARAM_6(x)  (DMAC_COMMON_START_ADDR(x) + 0x108)
#define DMAC_DMA_COMP_PARAM_5(x)  (DMAC_COMMON_START_ADDR(x) + 0x110)
#define DMAC_DMA_COMP_PARAM_4(x)  (DMAC_COMMON_START_ADDR(x) + 0x118)
#define DMAC_DMA_COMP_PARAM_3(x)  (DMAC_COMMON_START_ADDR(x) + 0x120)
#define DMAC_DMA_COMP_PARAM_2(x)  (DMAC_COMMON_START_ADDR(x) + 0x128)
#define DMAC_DMA_COMP_PARAM_1(x)  (DMAC_COMMON_START_ADDR(x) + 0x130)
#define DMAC_DMA_COMP_ID(x)       (DMAC_COMMON_START_ADDR(x) + 0x138)

#define DMAC_INTERRUPT_BLOCK 0x01
#define DMAC_INTERRUPT_TFR   0x02
#define DMAC_INTERRUPT_ERROR 0x04
#define DMAC_INTERRUPT_NO    0x00

//********************************************************************
//define struction
typedef struct dmac_ch_info
{
	CK_UINT32 sarx;					//Source address register
	CK_UINT32 darx;					//Destination address register
	CK_UINT32 ctlHx;					//Control register high 32-bit
	CK_UINT32 ctlLx;					//Control register low 32-bit
	CK_UINT32 cfgHx;					//Configuration register high 32-bit
	CK_UINT32 cfgLx;					//Configuration register low 32-bit
	CK_UINT32 sgrx;					//Source gather register
	CK_UINT32 dsrx;					//Destination scatter register
	CK_UINT32 llpx;
} DMAC_CH_INFO;


#if CC_DMAC_CH0_MULTI_BLK_EN
typedef struct dma_list_item
{
	CK_UINT32 sarx;					//Source address register
	CK_UINT32 darx;					//Destination address register
	CK_UINT32 llpx;
	CK_UINT32 ctlLx;					//Control register
	CK_UINT32 ctlHx;					//Configuration register high 32-bit
#if CC_DMAC_CH0_STAT_DST
	CK_UINT32 sstatx;

#endif	/*  */
	 CK_UINT32 dstatx;
} DMA_LIST_ITEM;

#endif	/*  */
//********************************************************************


//********************************************************************
//register bit function define
//********************************************************************
//LLP register.
//********************************************************************
#define DMAC_LLP_MASTER(n)      (n)
//********************************************************************
//SGR/DSR register.
//********************************************************************
#define DMAC_SG_SET(cnt,inc)   ((cnt << 20) | (inc & 0xFFFFF))
//********************************************************************
//Control register.
//********************************************************************
//CTLHx:
#define DMAC_CTL_DONE           (1<<12)
#define DMAC_CTL_BLOCK_TS(n)    (n<<0)
//CTLLx:
#define DMAC_CTL_LLP_SRC_EN     (1L<<28)
#define DMAC_CTL_LLP_DST_EN     (1L<<27)
#define DMAC_CTL_SMS_M1         (0L<<25)
#define DMAC_CTL_SMS_M2         (1L<<25)
#define DMAC_CTL_SMS_M3         (2L<<25)
#define DMAC_CTL_SMS_M4         (3L<<25)
#define DMAC_CTL_DMS_M1         (0L<<23)
#define DMAC_CTL_DMS_M2         (1L<<23)
#define DMAC_CTL_DMS_M3         (2L<<23)
#define DMAC_CTL_DMS_M4         (3L<<23)
#define DMAC_CTL_M2M_DW         (0L<<20)
#define DMAC_CTL_M2P_DW         (1L<<20)
#define DMAC_CTL_P2M_DW         (2L<<20)
#define DMAC_CTL_P2P_DW         (3L<<20)
#define DMAC_CTL_P2M_PER        (4L<<20)
#define DMAC_CTL_P2P_SrcP       (5L<<20)
#define DMAC_CTL_M2P_PER        (6L<<20)
#define DMAC_CTL_P2P_DesP       (7L<<20)
#define DMAC_CTL_DST_SCATTER_EN (1L<<18)
#define DMAC_CTL_SRC_GATHER_EN  (1L<<17)
#define DMAC_CTL_SRC_MSIZE1     (0L<<14)
#define DMAC_CTL_SRC_MSIZE4     (1L<<14)
#define DMAC_CTL_SRC_MSIZE8     (2L<<14)
#define DMAC_CTL_SRC_MSIZE16    (3L<<14)
#define DMAC_CTL_SRC_MSIZE32    (4L<<14)
#define DMAC_CTL_SRC_MSIZE64    (5L<<14)
#define DMAC_CTL_SRC_MSIZE128   (6L<<14)
#define DMAC_CTL_SRC_MSIZE256   (7L<<14)
#define DMAC_CTL_DEST_MSIZE1    (0L<<11)
#define DMAC_CTL_DEST_MSIZE4    (1L<<11)
#define DMAC_CTL_DEST_MSIZE8    (2L<<11)
#define DMAC_CTL_DEST_MSIZE16   (3L<<11)
#define DMAC_CTL_DEST_MSIZE32   (4L<<11)
#define DMAC_CTL_DEST_MSIZE64   (5L<<11)
#define DMAC_CTL_DEST_MSIZE128  (6L<<11)
#define DMAC_CTL_DEST_MSIZE256  (7L<<11)
#define DMAC_CTL_SINC_INC       (0L<<9)
#define DMAC_CTL_SINC_DEC       (1L<<9)
#define DMAC_CTL_SINC_NO        (2L<<9)
#define DMAC_CTL_DINC_INC       (0L<<7)
#define DMAC_CTL_DINC_DEC       (1L<<7)
#define DMAC_CTL_DINC_NO        (2L<<7)
#define DMAC_CTL_SRC_TR_WIDTH8  (0L<<4)
#define DMAC_CTL_SRC_TR_WIDTH16 (1L<<4)
#define DMAC_CTL_SRC_TR_WIDTH32 (2L<<4)
#define DMAC_CTL_SRC_TR_WIDTH64 (3L<<4)
#define DMAC_CTL_DST_TR_WIDTH8  (0L<<1)
#define DMAC_CTL_DST_TR_WIDTH16 (1L<<1)
#define DMAC_CTL_DST_TR_WIDTH32 (2L<<1)
#define DMAC_CTL_DST_TR_WIDTH64 (3L<<1)
#define DMAC_CTL_INT_EN         (1<<0)
#define DMAC_CTL_INT_DIS        (0<<0)

//********************************************************************
//Configuration register
//********************************************************************
//CFGHx:
#define DMAC_CFG_DEST_PER(n)      (n<<11)	//Hardware handshake interface
#define DMAC_CFG_SRC_PER(n)       (n<<7)	//Hardware handshake interface
#define DMAC_CFG_SS_UPD_EN        (1<<6)	//Status update enable
#define DMAC_CFG_DS_UPD_EN        (1<<5)	//Status update enable
#define DMAC_CFG_PROTCTL(n)       (n<<2)
#define DMAC_CFG_FIFO_MODE_1      (1<<1)
#define DMAC_CFG_FIFO_MODE_0      (0<<1)
#define DMAC_CFG_FCMODE_1         (1<<0)	//Enable prefetch
#define DMAC_CFG_FCMODE_0         (0<<0)	//Enable prefetch
//CFGLx:
#define DMAC_CFG_RELOAD_DST       (1L<<31)
#define DMAC_CFG_RELOAD_SRC       (1L<<30)
#define DMAC_CFG_SRC_HS_POL_H     (0L<<19)	//Handshake polarity
#define DMAC_CFG_SRC_HS_POL_L     (1L<<19)	//Handshake polarity
#define DMAC_CFG_DST_HS_POL_H     (0L<<18)	//Handshake polarity
#define DMAC_CFG_DST_HS_POL_L     (1L<<18)	//Handshake polarity
#define DMAC_CFG_LOCK_B           (1L<<17)
#define DMAC_CFG_LOCK_CH          (1L<<16)
#define DMAC_CFG_LOCK_B_TSF       (0L<<14)	//Lock level
#define DMAC_CFG_LOCK_B_BLK       (1L<<14)	//Lock level
#define DMAC_CFG_LOCK_B_TSC       (2L<<14)	//Lock level
#define DMAC_CFG_LOCK_CH_TSF      (0L<<12)	//Lock level
#define DMAC_CFG_LOCK_CH_BLK      (1L<<12)	//Lock level
#define DMAC_CFG_LOCK_CH_TSC      (2L<<12)	//Lock level
#define DMAC_CFG_HS_SRC_HARDWARE  (0L<<11)	//Handshake mode
#define DMAC_CFG_HS_SRC_SOFTWARE  (1L<<11)	//Handshake mode
#define DMAC_CFG_HS_DST_HARDWARE  (0L<<10)	//Handshake mode
#define DMAC_CFG_HS_DST_SOFTWARE  (1L<<10)	//Handshake mode
#define DMAC_CFG_FIFO_EMPTY       (1<<9)
#define DMAC_CFG_CH_SUSP          (1<<8)
#define DMAC_CFG_CH_PRIOR(n)      (n<<5)	//n should be 0 to 7

//********************************************************************
//Interrupt register
//********************************************************************
//interrupt RAW status registers(0x2c0 to 0x2e0) and interrupt status registers(0x2e8 to 0x308)
#define DMAC_INT_STATUS(n)      (1L<<n)
//interrupt mask register                 //0x310 to 0x330
#define DMAC_INT_MASK(n)      (((0L<<n) | (1L<<(n+8))))
#define DMAC_INT_UNMASK(n)      (((1L<<n) | (1L<<(n+8))))
//interrupt clear register                  //0x338 to 0x358
#define DMAC_INT_CLR(n)       (1L<<n)
//combined interrupt status register            //0x360
#define DMAC_INT_ERR        (1L<<4)
#define DMAC_INT_DSTT       (1L<<3)
#define DMAC_INT_SRCT       (1L<<2)
#define DMAC_INT_BLOCK      (1L<<1)
#define DMAC_INT_TFR        (1L<<0)
#define DMAC_ALL_MASK       ((1L<<AHB_DMAC_MAX_CHANNELS)-1)

//********************************************************************
//Software handshake register
//********************************************************************
#define DMAC_SW_REQ(n)        (((1<<n) | (1<<(n+8))))
#define DMAC_SW_REQ_DIS(n)      (1<<(n+8))

//********************************************************************
//DMA channel enable register
//********************************************************************
#define DMAC_CH0            (1L<<0)
#define DMAC_CH1            (1L<<1)
#define DMAC_CH2            (1L<<2)
#define DMAC_CH3            (1L<<3)
#define DMAC_CH4            (1L<<4)
#define DMAC_CH5            (1L<<5)
#define DMAC_CH6            (1L<<6)
#define DMAC_CH_EN(n)       ((1<<n) | (1<<(n+8)))
#define DMAC_CH_DIS(n)        (1<<(n+8))

//********************************************************************
//DMA configuration register
//********************************************************************
#define DMAC_EN           (1<<0)
//DMA0 peripheral_ID
#define peripheral_spi_tx(id)       (0 + 2 * (id % 2))
#define peripheral_spi_rx(id)       (1 + 2 * (id % 2))

#define peripheral_i2c_tx(id)       (4 + 2 * id)
#define peripheral_i2c_rx(id)       (5 + 2 * id)
#define peripheral_pwm0             12
#define peripheral_pwm1             13
#define peripheral_pwm2             14
#define peripheral_pwm3             15

//DMA1 peripheral_ID
#define peripheral_uart_tx(id)      (4 + 2 * id)
#define peripheral_uart_rx(id)      (5 + 2 * id)
#define peripheral_pwm4             14
#define peripheral_pwm5             15

//*******************************************************************
//Miscellaneous DMA register
//********************************************************************
#define DMAC_NORMAL_MODE      (0<<0)

#define	DMAC_BLK_SIZE_256 256
//#define	DMAC_BLK_SIZE_256 48//256
#define	DMAC_BLK_SIZE_128 128
#define	DMAC_BLK_SIZE_64  64
#define	DMAC_BLK_SIZE_512 512
#define Align8  0x01
#define Align16 0x02
#define Align32 0x04
#define Align64 0x08

#define CHANNEL_WR   0
#define CHANNEL_RD   1

/**************************************************************
* parameters for Subsystem test
**************************************************************/
#define AHB_DMA_SUB_TEST_L 200
#define AHB_DMA_SUB_SRC_UART 0x20000000
#define AHB_DMA_SUB_SRC1_UART 0x20000400
#define AHB_DMA_SUB_SRC_SPI  0x20000100
#define AHB_DMA_SUB_SRC_I2C  0x20000200

#define AHB_DMA_SUB_DST_UART 0x20004000
#define AHB_DMA_SUB_DST1_UART 0x20004400
#define AHB_DMA_SUB_DST_SPI  0x20004100
#define AHB_DMA_SUB_DST_I2C  0x20004200

#define DST_MSIZE(x)    (1 << (((x >> 11) == 0) ? 0 : ((x >> 11) + 1)))
#define DST_TR_WIDTH(x) (1 << ((x >> 1) + 3))

#define DMA_CHANNEL_0           0
#define DMA_CHANNEL_1           1
#define DMA_CHANNEL_2           2
#define DMA_CHANNEL_3           3
#define DMA_CHANNEL_4           4
#define DMA_CHANNEL_5           5
#define DMA_CHANNEL_6           6
#define DMA_CHANNEL_7           7

//function
extern void DMAC_Init(CK_UINT32 id);
extern void DMAC_Interrupt_en(CK_UINT32 id, CK_UINT32 type);
extern void DMAC_Open(CK_UINT32 id, DMAC_CH_INFO * channel, CK_UINT32 channel_number,
                        CK_UINT16 BlockSize);
extern void DMAC_Close(CK_UINT32 id, CK_UINT32 channel_number);
extern void dmac0_isr_handler(CK_UINT32 irq);
extern void dmac1_isr_handler(CK_UINT32 irq);
extern CK_UINT32 DMAC_CheckDone(CK_UINT32 id, CK_UINT32 channel_number, CK_UINT8 dma_intr);
extern void DMAC_Config(CK_UINT32 id, DMAC_CH_INFO *channel, CK_UINT32 channel_number,
                        CK_UINT16 BlockSize);
extern void DMAC_Start(CK_UINT32 id, CK_UINT32 channel_flag);
void DMAMem2MemOpen(CK_UINT32 id, CK_UINT8 channel, CK_UINT32 src_addr, CK_UINT32 dst_addr,
                    CK_UINT32 count, CK_UINT8 dma_intr,
                    CK_UINT8 UNIT, CK_UINT8 prio, CK_UINT32 llpx,
                    CK_UINT8 llp_src_upd, CK_UINT8 llp_dst_upd);
extern void DMAMem2PeripheralOpen(CK_UINT32 id, CK_UINT8 channel, CK_UINT32 src_addr,
                                    CK_UINT32 count, CK_UINT8 peripheral_ID,
                                    CK_UINT8 dma_intr, CK_UINT32 PortNum,
                                    CK_UINT16 src_gth_cnt,
                                    CK_UINT32 src_gth_intvl,
                                    CK_UINT32 dst_tr_width,
                                    CK_UINT32 dst_msize);
extern void DMAPeripheral2MemOpen(CK_UINT32 id, CK_UINT8 channel, CK_UINT32 dst_addr,
                                    CK_UINT32 count, CK_UINT8 peripheral_ID,
                                    CK_UINT8 dma_intr, CK_UINT32 PortNum,
                                    CK_UINT16 dst_sct_cnt,
                                    CK_UINT32 dst_sct_intvl);

extern volatile CK_UINT32 DMAC_INT_Flag[2];
void DMAPeripheral2PeripheralOpen(CK_UINT8 channel, CK_UINT32 count,
                                    CK_UINT8 PeripheraPassl_ID,
                                    CK_UINT8 dma_intr);
#endif
