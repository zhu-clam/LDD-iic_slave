#ifndef __DVI_H__
#define __DVI_H__

#define __NEW_VERSION__

#define DVP_CH0_BASE_ADDR      (CK_MIPI_Slave+0x0000)
#define DVP_CH1_BASE_ADDR      (CK_MIPI_Slave+0x0400)
#define DVP_CH2_BASE_ADDR      (CK_MIPI_Slave+0x0800)
#define DVP_CH3_BASE_ADDR      (CK_MIPI_Slave+0x0C00)

#define DVP_BASE_ADDR          DVP_CH0_BASE_ADDR


#define VI_DMA_ARB_MODE   	(DVP_BASE_ADDR+0x0000)
#define VI_DMA_WEIGHT_WR_0   	(DVP_BASE_ADDR+0x0004)
#define VI_DMA_WEIGHT_WR_1   	(DVP_BASE_ADDR+0x0008)
#define VI_DMA_WEIGHT_RD_0   	(DVP_BASE_ADDR+0x000C)
#define VI_DMA_WEIGHT_RD_1   	(DVP_BASE_ADDR+0x0010)
#define VI_DMA_PRIORTY_WR       (DVP_BASE_ADDR+0x0014)
#define VI_DMA_PRIORTY_RD       (DVP_BASE_ADDR+0x0018)
#define VI_DMA_ID_RD            (DVP_BASE_ADDR+0x001C)
#define VI_DMA_ID_WR            (DVP_BASE_ADDR+0x0020)
#define VI_WR_BD_CTL            (DVP_BASE_ADDR+0x0024)
#define VI_CTRL_REG0		(DVP_BASE_ADDR+0x0028)
#define CCIR                    0x0010
#define VIDEO_INTERLANCE_MODE   0x0020                    

#define VI_CTRL_REG1		(DVP_BASE_ADDR+0x002C)
#define VI_CTRL_REG2		(DVP_BASE_ADDR+0x0030)

#define VI_TIMESTAMP_CTL        (DVP_BASE_ADDR+0x0034)
#define VI_TIMESTAMP_BADDR  	(DVP_BASE_ADDR+0x0038)
#define VI_DMA_CTL 		(DVP_BASE_ADDR+0x003C)
#define DMA_EN                  0x0001
#define PIXEL_TYPE              (1L << 1)

#ifdef __NEW_VERSION__

#define VI_IMG_OUT_BADDR_Y	(DVP_BASE_ADDR+0x0040)
#define VI_IMG_OUT_BADDR_UV	(DVP_BASE_ADDR+0x0044)

#define VI_IMG_OUT_BADDR_Y0	(DVP_BASE_ADDR+0x0040)
#define VI_IMG_OUT_BADDR_UV0	(DVP_BASE_ADDR+0x0044)

#define VI_IMG_OUT_BADDR_Y1	(DVP_BASE_ADDR+0x0048)
#define VI_IMG_OUT_BADDR_UV1	(DVP_BASE_ADDR+0x004C)


#define VI_IMG_OUT_PIX_HSIZE	(DVP_BASE_ADDR+0x0050)
#define VI_IMG_OUT_PIX_VSIZE	(DVP_BASE_ADDR+0x0054)
#define VI_IMG_OUT_PIX_HSTRIDE	(DVP_BASE_ADDR+0x0058)
#define VI_IMG_OUT_BLENTH	(DVP_BASE_ADDR+0x005C)
#define VI_STATUS		(DVP_BASE_ADDR+0x0060)
#define FS                      0x0001
#define DMA_DONE                0x0002


#define IRQ_EN        		(DVP_BASE_ADDR+0x0064)
#define IRQ_CLR                 (DVP_BASE_ADDR+0x0068)
#define IRQ_STATUS              (DVP_BASE_ADDR+0x006C)
#define ISP_SEL   		(DVP_BASE_ADDR+0x0070)
#define DVP_MODE                0x0001

#define VI_VSIZE_2ND_OFFSET   	(DVP_BASE_ADDR+0x0074)

#else

#define VI_IMG_OUT_BADDR_Y	(DVP_BASE_ADDR+0x0040)
#define VI_IMG_OUT_BADDR_UV	(DVP_BASE_ADDR+0x0044)


#define VI_IMG_OUT_PIX_HSIZE	(DVP_BASE_ADDR+0x0048)
#define VI_IMG_OUT_PIX_VSIZE	(DVP_BASE_ADDR+0x004C)
#define VI_IMG_OUT_PIX_HSTRIDE	(DVP_BASE_ADDR+0x0050)
#define VI_IMG_OUT_BLENTH	(DVP_BASE_ADDR+0x0054)
#define VI_STATUS		(DVP_BASE_ADDR+0x0058)
#define FS                      0x0001
#define DMA_DONE                0x0002


#define IRQ_EN        		(DVP_BASE_ADDR+0x005C)
#define IRQ_CLR                 (DVP_BASE_ADDR+0x0060)
#define IRQ_STATUS              (DVP_BASE_ADDR+0x0064)
#define ISP_SEL   		(DVP_BASE_ADDR+0x0068)
#define DVP_MODE                0x0001


#endif
void dvi_test(void);

#endif
