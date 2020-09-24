#ifndef _AXI_DMA_H
#define _AXI_DMA_H

#define AXI_DMAC_ID 0x0

#define AXI_DMAC_MAX_CHANNELS   4

#define DMA_TFR_DONE    2

/**
 * Tranfer Type
*/
#define TYPE_SINGLE  1
#define TYPE_LLI     2
/**
 * AXI_DMA Common_Registers_Address_Block Registers
 *
 * **/

#define AXI_DMAC_IDREG          (CK_AXI_DMA_ADDRESS + 0x00)
#define AXI_DMAC_COMPVERREG     (CK_AXI_DMA_ADDRESS + 0x08)
#define AXI_DMAC_CFGREG         (CK_AXI_DMA_ADDRESS + 0x10)
#define AXI_DMAC_CHENREG        (CK_AXI_DMA_ADDRESS + 0x18)
#define AXI_DMAC_CHENREG2       (CK_AXI_DMA_ADDRESS + 0x20)
#define AXI_DMAC_CHSUSPREG      (CK_AXI_DMA_ADDRESS + 0x28)
#define AXI_DMAC_INTSTATUSREG   (CK_AXI_DMA_ADDRESS + 0x30)
#define AXI_DMAC_INTSTATUSREG2  (CK_AXI_DMA_ADDRESS + 0x30)
#define AXI_DMAC_COMMONREG_INTCLEARREG          (CK_AXI_DMA_ADDRESS + 0x38)
#define AXI_DMAC_COMMONREG_INTSTATUS_ENABLEREG  (CK_AXI_DMA_ADDRESS + 0x40)
#define AXI_DMAC_COMMONREG_INTSIGNAL_ENABLEREG  (CK_AXI_DMA_ADDRESS + 0x48)
#define AXI_DMAC_COMMONREG_INTSTATUSREG         (CK_AXI_DMA_ADDRESS + 0x50)
#define AXI_DMAC_RESETREG           (CK_AXI_DMA_ADDRESS + 0x58)
#define AXI_DMAC_LOWPOWER_CFGREG    (CK_AXI_DMA_ADDRESS + 0x60)

/**
 * AXI_DMA Channelx_Registers_Address_Block Registers
 * **/
#define CHx_SAR_OFFSET          0x100
#define CHx_DAR_OFFSET          0x108
#define CHx_BLOCK_TS_OFFSET     0x110
#define CHx_CTL_OFFSET          0x118
#define CHx_CFG_OFFSET          0x120
#define CHx_LLP_OFFSET          0x128
#define CHx_STATUSREG_OFFSET    0x130
#define CHx_SWHSSRCREG_OFFSET   0x138
#define CHx_SWHSDSTREG_OFFSET   0x140
#define CHx_BLK_TFR_RESUMEREQREG_OFFSET 0x148
#define CHx_AXI_IDREG_OFFSET    0x150
#define CHx_AXI_QOSREG_OFFSET   0x158
#define CHx_SSTAT_OFFSET        0x160
#define CHx_DSTAT_OFFSET        0x168
#define CHx_SSTATAR_OFFSET      0x170
#define CHx_DSTATAR_OFFSET      0x178
#define CHx_INTSTATUS_ENABLEREG_OFFSET  0x180
#define CHx_INTSTATUS_OFFSET    0x188
#define CHx_INTSIGNAL_ENABLEREG_OFFSET  0x190
#define CHx_INTCLEARREG_OFFSET  0x198

#define CHx_SAR(x)                  (CK_AXI_DMA_ADDRESS + CHx_SAR_OFFSET + (x-1)*0x100)
#define CHx_DAR(x)                  (CK_AXI_DMA_ADDRESS + CHx_DAR_OFFSET + (x-1)*0x100)
#define CHx_BLOCK_TS(x)             (CK_AXI_DMA_ADDRESS + CHx_BLOCK_TS_OFFSET + (x-1)*0x100)
#define CHx_CTL_LOW(x)              (CK_AXI_DMA_ADDRESS + CHx_CTL_OFFSET + (x-1)*0x100)
#define CHx_CTL_HIGH(x)             (CK_AXI_DMA_ADDRESS + (CHx_CTL_OFFSET + 4) + (x-1)*0x100)
#define CHx_CFG_LOW(x)              (CK_AXI_DMA_ADDRESS + CHx_CFG_OFFSET + (x-1)*0x100)
#define CHx_CFG_HIGH(x)             (CK_AXI_DMA_ADDRESS + (CHx_CFG_OFFSET + 4) + (x-1)*0x100)
#define CHx_LLP(x)                  (CK_AXI_DMA_ADDRESS + CHx_LLP_OFFSET + (x-1)*0x100)
#define CHx_STATUSREG(x)            (CK_AXI_DMA_ADDRESS + CHx_STATUSREG_OFFSET + (x-1)*0x100)
#define CHx_SWHSSRCREG(x)           (CK_AXI_DMA_ADDRESS + CHx_SWHSSRCREG_OFFSET + (x-1)*0x100)
#define CHx_SWHSDSTREG(x)           (CK_AXI_DMA_ADDRESS + CHx_SWHSDSTREG_OFFSET + (x-1)*0x100)
#define CHx_BLK_TFR_RESUMEREQREG(x) (CK_AXI_DMA_ADDRESS + CHx_BLK_TFR_RESUMEREQREG_OFFSET + (x-1)*0x100)
#define CHx_AXI_IDREG(x)            (CK_AXI_DMA_ADDRESS + CHx_AXI_IDREG_OFFSET + (x-1)*0x100)
#define CHx_AXI_QOSREG(x)           (CK_AXI_DMA_ADDRESS + CHx_AXI_QOSREG_OFFSET + (x-1)*0x100)
#define CHx_SSTAT(x)                (CK_AXI_DMA_ADDRESS + CHx_SSTAT_OFFSET + (x-1)*0x100)
#define CHx_DSTAT(x)                (CK_AXI_DMA_ADDRESS + CHx_DSTAT_OFFSET + (x-1)*0x100)
#define CHx_SSTATAR(x)              (CK_AXI_DMA_ADDRESS + CHx_SSTATAR_OFFSET + (x-1)*0x100)
#define CHx_DSTATAR(x)              (CK_AXI_DMA_ADDRESS + CHx_DSTATAR_OFFSET + (x-1)*0x100)
#define CHx_INTSTATUS_ENABLEREG(x)  (CK_AXI_DMA_ADDRESS + CHx_INTSTATUS_ENABLEREG_OFFSET + (x-1)*0x100)
#define CHx_INTSTATUS(x)            (CK_AXI_DMA_ADDRESS + CHx_INTSTATUS_OFFSET + (x-1)*0x100)
#define CHx_INTSIGNAL_ENABLEREG(x)      (CK_AXI_DMA_ADDRESS + CHx_INTSIGNAL_ENABLEREG_OFFSET + (x-1)*0x100)
#define CHx_INTCLEARREG(x)              (CK_AXI_DMA_ADDRESS + CHx_INTCLEARREG_OFFSET + (x-1)*0x100)

/**
 * AXI_DMA configuration register DMAC_CFGREG(0x10)
 * **/
#define AXI_DMAC_EN     (1<<0)
#define AXI_DMAC_DE     (0<<0)
#define AXI_DMAC_INT_EN (1<<1)
#define AXI_DMAC_INT_DE (0<<1)

/**
 * AXI_DMAC configuration register DMAC_CHENREG(0x18)
 * **/
#define CHx_EN(x)       (1<<(x-1))
#define CHx_DE(x)       (0<<(x-1))
#define CHx_EN_WE(x)    (1<<(x+(AXI_DMAC_MAX_CHANNELS+4)-1))
#define CHx_DE_WE(x)    (0<<(x+(AXI_DMAC_MAX_CHANNELS+4)-1))

/**
 * DMAC_INTSTATUSREG(0x30)
*/
#define CHx_IntStat(x)      (1<<(x-1))
#define CommonReg_IntStat   (1<<16)
/**
 * Channel configuration register DMAC_CHx_CTL(0x118)
 * **/
#define CHx_DST_MSIZE(x)    (x<<18)
#define CHx_SRC_MSIZE(x)    (x<<14)
#define CHx_DST_WIDTH(x)    (x<<11)
#define CHx_SRC_WIDTH(x)    (x<<8)
#define CHx_DINC            (1<<6)
#define CHx_SINC            (1<<4)
#define CHx_DMS             (0<<2)
#define CHx_SMS             (0<<0)
#define LAST_ITEM           (1<<30)
#define NOT_LAST_ITEM       (0<<30)
#define VALID               (1<<31)
#define INVALID             (0<<31)

/**
 * Channel configuration register CHx_CFG_HIGH(0x124)
 * **/
#define CHx_M2M_DMAC    (0<<0)
#define CHx_M2P_DMAC    (1<<0)
#define CHx_P2M_DMAC    (2<<0)
#define CHx_P2P_DMAC    (3<<0)
#define CHx_P2M_SRC     (4<<0)
#define CHx_P2P_SRC     (5<<0)
#define CHx_M2P_DST     (6<<0)
#define CHx_P2P_DST     (7<<0)
#define CHx_HS_SEL_SRC_H    (0<<3)
#define CHx_HS_SEL_SRC_S    (1<<3)
#define CHx_HS_SEL_DST_H    (0<<4)
#define CHx_HS_SEL_DST_S    (1<<4)
#define CHx_SRC_PER         (1<<7)
#define CHx_PRIOR(x)        ((x-1)<<17)

/**
 * Channel configuration register CHx_SWHSSRCREG(0x138)
 * **/
#define ACTIVE_SWHS_REQ_SRC     (1<<0)
#define INACTIVE_SWHS_REQ_SRC   (0<<0)
#define ENABLE_SWHS_REQ_SRC     (1<<1)
#define DISABLE_SWHS_REQ_SRC    (0<<1)
#define ACTIVE_SWHS_SGLREQ_SRC      (1<<2)
#define INACTIVE_SWHS_SGLREQ_SRC    (0<<2)
#define ENABLE_SWHS_SGLREQ_SRC  (1<<3)
#define DISABLE_SWHS_SGLREQ_SRC (0<<3)
#define OPEN_SRC_SWHS           0xf

/**
 * Channel configuration register CHx_SWHSDSTREG(0x140)
 * **/
#define ACTIVE_SWHS_REQ_DST     (1<<0)
#define INACTIVE_SWHS_REQ_DST   (0<<0)
#define ENABLE_SWHS_REQ_DST     (1<<1)
#define DISABLE_SWHS_REQ_DST    (0<<1)
#define ACTIVE_SWHS_SGLREQ_DST      (1<<2)
#define INACTIVE_SWHS_SGLREQ_DST    (0<<2)
#define ENABLE_SWHS_SGLREQ_DST  (1<<3)
#define DISABLE_SWHS_SGLREQ_DST (0<<3)
#define OPEN_DST_SWHS           0xf

/**
 * Channel configuration register CHx_INSIGNAL_ENBALEREG(0x190)
 * **/
#define CHx_INSIGNAL_EN_DMA_TFR_DONE    (1<<1)

/**
 * Channel configuration register CHx_INTCLEARREG(0x198)
 * **/
#define CLEAR_DMA_TFR_DONE  (1<<1)

/**
 * Parameters
*/
struct axi_dma_info{
    u32 src_msize;
    u32 dst_msize;
    u32 src_width;
    u32 dst_width;
    u32 channel;
    u32 direction;
};

struct axi_dma_lli {
    u64 sar;
    u64 dar;
    u32 block_ts_lo;
    u32 block_ts_hi;
    u64 llp;
    u32 ctl_lo;
    u32 ctl_hi;
    u32 sstat;
    u32 dstat;
    u32 status_lo;
    u32 ststus_hi;
    u32 reserved_lo;
    u32 reserved_hi;
};

enum SRC_MSIZE{
    SRC_MSIZE1 = 0,
    SRC_MSIZE4,
    SRC_MSIZE8,
    SRC_MSIZE16,
    SRC_MSIZE32,
    SRC_MSIZE64,
    SRC_MSIZE128,
    SRC_MSIZE256,
    SRC_MSIZE512,
    SRC_MSIZE1024
};

enum DST_MSIZE{
    DST_MSIZE1 = 0,
    DST_MSIZE4,
    DST_MSIZE8,
    DST_MSIZE16,
    DST_MSIZE32,
    DST_MSIZE64,
    DST_MSIZE128,
    DST_MSIZE256,
    DST_MSIZE512,
    DST_MSIZE1024
};

enum SRC_WIDTH{
    SRC_WIDTH8 = 0,
    SRC_WIDTH16,
    SRC_WIDTH32,
    SRC_WIDTH64,
    SRC_WIDTH128,
    SRC_WIDTH256,
    SRC_WIDTH512
};
    
enum DST_WIDTH{
    DST_WIDTH8 = 0,
    DST_WIDTH16,
    DST_WIDTH32,
    DST_WIDTH64,
    DST_WIDTH128,
    DST_WIDTH256,
    DST_WIDTH512
};

/**
 * Functions
*/
void CHOSE_TYPE(CK_UINT32 type);

void AXI_DMA_TRAN_INIT(struct axi_dma_info axi, CK_UINT32 src_addr, CK_UINT32 dst_addr, CK_UINT32 length);

void AXI_DMA_TRAN_INIT_LLI(struct axi_dma_info axi, CK_UINT32 src_addr, CK_UINT32 dst_addr, CK_UINT32 length);

void AXI_DMA_TRANS(struct axi_dma_info axi);

#endif