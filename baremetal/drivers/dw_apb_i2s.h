#ifndef _DW_APB_I2S_H_
#define _DW_APB_I2S_H_

#include "datatype.h"
#include "snd_pcm.h"
#include "soc_dai.h"

#define MAX_I2S_NUM_ON_CHIP 5

#define I2S_FOR_PLAY    4

/* Common register for all channel */
#define IER		0x000
#define IRER		0x004
#define ITER		0x008
#define CER		0x00C
#define CCR		0x010
#define RXFFR		0x014
#define TXFFR		0x018

/* Interrupt status register fields */
#define ISR_TXFO	BIT(5)
#define ISR_TXFE	BIT(4)
#define ISR_RXFO	BIT(1)
#define ISR_RXDA	BIT(0)

/* I2STxRxRegisters for all channels */
#define LRBR_LTHR(x)	(0x40 * x + 0x020)
#define RRBR_RTHR(x)	(0x40 * x + 0x024)
#define RER(x)		(0x40 * x + 0x028)
#define TER(x)		(0x40 * x + 0x02C)
#define RCR(x)		(0x40 * x + 0x030)
#define TCR(x)		(0x40 * x + 0x034)
#define ISR(x)		(0x40 * x + 0x038)
#define IMR(x)		(0x40 * x + 0x03C)
#define ROR(x)		(0x40 * x + 0x040)
#define TOR(x)		(0x40 * x + 0x044)
#define RFCR(x)		(0x40 * x + 0x048)
#define TFCR(x)		(0x40 * x + 0x04C)
#define RFF(x)		(0x40 * x + 0x050)
#define TFF(x)		(0x40 * x + 0x054)

/* I2SCOMPRegisters */
#define I2S_COMP_PARAM_2	0x01F0
#define I2S_COMP_PARAM_1	0x01F4
#define I2S_COMP_VERSION	0x01F8
#define I2S_COMP_TYPE		0x01FC

/* DMA Configurate registers */
#define DMACR			0x0200
#define RXDMA_CH(x)		(0x40 * x + 0x0204)
#define TXDMA_CH(x)		(0x40 * x + 0x0214)

#define DMA_TXBLOCK_EN	(1 << 17)
#define DMAEN_TXCH0	(1 << 8)
#define DMAEN_TXCH1	(1 << 9)
#define DMAEN_TXCH2	(1 << 10)
#define DMAEN_TXCH3	(1 << 11)

#define DMA_RXBLOCK_EN	(1 << 16)
#define DMAEN_RXCH0	(1 << 0)
#define DMAEN_RXCH1	(1 << 1)
#define DMAEN_RXCH2	(1 << 2)
#define DMAEN_RXCH3	(1 << 3)

/*
 * Component parameter register fields - define the I2S block's
 * configuration.
 */
#define	COMP1_TX_WORDSIZE_3(r)	(((r) & GENMASK(27, 25)) >> 25)
#define	COMP1_TX_WORDSIZE_2(r)	(((r) & GENMASK(24, 22)) >> 22)
#define	COMP1_TX_WORDSIZE_1(r)	(((r) & GENMASK(21, 19)) >> 19)
#define	COMP1_TX_WORDSIZE_0(r)	(((r) & GENMASK(18, 16)) >> 16)
#define	COMP1_TX_CHANNELS(r)	(((r) & GENMASK(10, 9)) >> 9)
#define	COMP1_RX_CHANNELS(r)	(((r) & GENMASK(8, 7)) >> 7)
#define	COMP1_RX_ENABLED(r)	(((r) & BIT(6)) >> 6)
#define	COMP1_TX_ENABLED(r)	(((r) & BIT(5)) >> 5)
#define	COMP1_MODE_EN(r)	(((r) & BIT(4)) >> 4)
#define	COMP1_FIFO_DEPTH_GLOBAL(r)	(((r) & GENMASK(3, 2)) >> 2)
#define	COMP1_APB_DATA_WIDTH(r)	(((r) & GENMASK(1, 0)) >> 0)

#define	COMP2_RX_WORDSIZE_3(r)	(((r) & GENMASK(12, 10)) >> 10)
#define	COMP2_RX_WORDSIZE_2(r)	(((r) & GENMASK(9, 7)) >> 7)
#define	COMP2_RX_WORDSIZE_1(r)	(((r) & GENMASK(5, 3)) >> 3)
#define	COMP2_RX_WORDSIZE_0(r)	(((r) & GENMASK(2, 0)) >> 0)

/* Number of entries in WORDSIZE and DATA_WIDTH parameter registers */
#define	COMP_MAX_WORDSIZE	(1 << 3)
#define	COMP_MAX_DATA_WIDTH	(1 << 2)

#define MAX_CHANNEL_NUM		8
#define MIN_CHANNEL_NUM		2

#define CONFIG_AUDIO_MCLK_FROM_PLL
#define MCLK_IN_FREQ 			12288000
#define CRM_I2S_CLK_CFG_REG		(0x120)
#ifdef CONFIG_AUDIO_MCLK_FROM_PLL
#define AUDIO_MCLK_BASE_FREQ		1536000000
#else
#define AUDIO_MCLK_BASE_FREQ		12288000
#endif
#define I2S_SCLK_FROM_PLL		(0 << 20)
#define I2S_SCLK_FROM_EXT		(1 << 20)



static const u32 fifo_width[COMP_MAX_WORDSIZE] = {
	12, 16, 20, 24, 32, 0, 0, 0
};

enum dma_slave_buswidth {
	DMA_SLAVE_BUSWIDTH_UNDEFINED = 0,
	DMA_SLAVE_BUSWIDTH_1_BYTE = 1,
	DMA_SLAVE_BUSWIDTH_2_BYTES = 2,
	DMA_SLAVE_BUSWIDTH_3_BYTES = 3,
	DMA_SLAVE_BUSWIDTH_4_BYTES = 4,
	DMA_SLAVE_BUSWIDTH_8_BYTES = 8,
	DMA_SLAVE_BUSWIDTH_16_BYTES = 16,
	DMA_SLAVE_BUSWIDTH_32_BYTES = 32,
	DMA_SLAVE_BUSWIDTH_64_BYTES = 64,
};

/* Width of (DMA) bus */
static const u32 bus_widths[COMP_MAX_DATA_WIDTH] = {
	DMA_SLAVE_BUSWIDTH_1_BYTE,
	DMA_SLAVE_BUSWIDTH_2_BYTES,
	DMA_SLAVE_BUSWIDTH_4_BYTES,
	DMA_SLAVE_BUSWIDTH_UNDEFINED
};


#define TWO_CHANNEL_SUPPORT	2	/* up to 2.0 */
#define FOUR_CHANNEL_SUPPORT	4	/* up to 3.1 */
#define SIX_CHANNEL_SUPPORT	6	/* up to 5.1 */
#define EIGHT_CHANNEL_SUPPORT	8	/* up to 7.1 */


struct dw_i2s_dev {
	void *i2s_base;
#define DWC_I2S_PLAY	(1 << 0)
#define DWC_I2S_RECORD	(1 << 1)
#define DW_I2S_SLAVE	(1 << 2)
#define DW_I2S_MASTER	(1 << 3)
	unsigned int capability;
	u32 ccr;
	u32 xfer_resolution;
	u32 fifo_th;

	struct i2s_clk_config_data config;
	int (*i2s_clk_cfg)(struct i2s_clk_config_data *config);

	bool use_pio;
	unsigned int (*tx_fn)(struct dw_i2s_dev *,
			struct snd_pcm_substream *substream);
	unsigned int (*rx_fn)(struct dw_i2s_dev *,
			struct snd_pcm_substream *substream);
	
	unsigned int (*poll)(struct dw_i2s_dev *);

	struct snd_pcm_substream *substream;
	const struct snd_soc_dai_ops *dai_ops;
};


int dw_apb_i2s_probe(struct snd_soc_dai_dev *dai_dev, unsigned char id);


#endif
