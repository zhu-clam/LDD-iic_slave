/*****************************************************************************
 *  File: dw_apb_i2s.c
 *
 *  Descirption: Contains the functions support Synopsys apb IIS Controller.
 *
 *  Base on dw-i2s.c and dw-pcm.c in linux.
 *
 *  Copyright (C) : 2019 VeriSilicon.
 *
 *  Author  : Liu.Wei@verisilicon.com
 *  Date    : 2019/3/19 8:59:30
 *
 *****************************************************************************/
#include <string.h>
#include "ck810.h"
#include "dw_apb_i2s.h"
#include "snd_pcm.h"
#include "datatype.h"
#include "misc.h"
#include "soc_dai.h"

//#define _I2S_DEBUG_

static volatile unsigned int i2s_iobase_list[MAX_I2S_NUM_ON_CHIP] = {
    I2S0_BASE,
    I2S1_BASE,
    I2S2_BASE,
    I2S3_BASE,
    I2S4_BASE
};


#define dw_pcm_tx_fn(sample_bits) \
static unsigned int dw_pcm_tx_##sample_bits(struct dw_i2s_dev *dev, \
                struct snd_pcm_substream *substream) \
{ \
    const u##sample_bits *p = (u##sample_bits *)substream->data_base; \
    int i; \
\
    for (i = 0; i < /*dev->fifo_th*/1; i++) { \
        write_mreg32(dev->i2s_base + LRBR_LTHR(0), *(p + substream->ptr++)); \
        write_mreg32(dev->i2s_base + RRBR_RTHR(0), *(p + substream->ptr++)); \
        if (substream->ptr >= substream->sz_tx) \
            return 1; \
    } \
    return 0; \
}

#define dw_pcm_rx_fn(sample_bits) \
static unsigned int dw_pcm_rx_##sample_bits(struct dw_i2s_dev *dev, \
        struct snd_pcm_substream *substream) \
{ \
    u##sample_bits *p = (u##sample_bits *)substream->data_base; \
    int i; \
\
    for (i = 0; i < /*dev->fifo_th*/1; i++) { \
        *(p + substream->ptr++) = read_mreg32(dev->i2s_base + LRBR_LTHR(0)); \
        *(p + substream->ptr++) = read_mreg32(dev->i2s_base + RRBR_RTHR(0)); \
        if (substream->ptr >= substream->sz_rx) \
            return 1; \
    } \
    return 0; \
}

#ifndef _I2S_DEBUG_

dw_pcm_tx_fn(16);
dw_pcm_tx_fn(32);
dw_pcm_rx_fn(16);
dw_pcm_rx_fn(32);

#else

static unsigned int dw_pcm_tx_32(struct dw_i2s_dev *dev,
                struct snd_pcm_substream *substream)
{
    u32 *p = (u32 *)substream->data_base;
    int i;
    for (i = 0; i < 1; i++) {
        write_mreg32(dev->i2s_base + LRBR_LTHR(0), *(p + substream->ptr++));
        write_mreg32(dev->i2s_base + RRBR_RTHR(0), *(p + substream->ptr++));
        if (substream->ptr >= substream->sz_tx)
            return 1;
    }
    return 0;
}

static unsigned int dw_pcm_tx_16(struct dw_i2s_dev *dev,
            struct snd_pcm_substream *substream)
{
    u16 *p = (u16 *)substream->data_base;
    int i;
    for (i = 0; i < 1; i++) {
        write_mreg32(dev->i2s_base + LRBR_LTHR(0), *(p + substream->ptr++));
        write_mreg32(dev->i2s_base + RRBR_RTHR(0), *(p + substream->ptr++));
        if (substream->ptr >= substream->sz_tx)
            return 1;
    }
    return 0;
}


static unsigned int dw_pcm_rx_16(struct dw_i2s_dev *dev,
        struct snd_pcm_substream *substream)
{
    u16 *p = (u16 *)substream->data_base;
    int i;

    for (i = 0; i < /*dev->fifo_th*/1; i++) {
        *(p + substream->ptr++) = read_mreg32(dev->i2s_base + LRBR_LTHR(0));
        *(p + substream->ptr++) = read_mreg32(dev->i2s_base + RRBR_RTHR(0));
        if (substream->ptr >= substream->sz_rx)
            return 1;
    }
    return 0;
}

static unsigned int dw_pcm_rx_32(struct dw_i2s_dev *dev,
        struct snd_pcm_substream *substream)
{
    u32 *p = (u32 *)substream->data_base;
    int i;

    for (i = 0; i < /*dev->fifo_th*/1; i++) {
        *(p + substream->ptr++) = read_mreg32(dev->i2s_base + LRBR_LTHR(0));
        *(p + substream->ptr++) = read_mreg32(dev->i2s_base + RRBR_RTHR(0));
        if (substream->ptr >= substream->sz_rx)
            return 1;
    }
    return 0;
}

#endif

#undef dw_pcm_tx_fn
#undef dw_pcm_rx_fn


static inline void i2s_write_reg(void *io_base, int reg, u32 val)
{
    write_mreg32(io_base + reg, val);
}

static inline u32 i2s_read_reg(void  *io_base, int reg)
{
    return read_mreg32(io_base + reg);
}

static inline void i2s_disable_channels(struct dw_i2s_dev *dev, u32 stream)
{
    u32 i = 0;

    if (stream == SNDRV_PCM_STREAM_PLAYBACK) {
        for (i = 0; i < 4; i++)
            i2s_write_reg(dev->i2s_base, TER(i), 0);
    } else {
        for (i = 0; i < 4; i++)
            i2s_write_reg(dev->i2s_base, RER(i), 0);
    }
}

static inline void i2s_clear_irqs(struct dw_i2s_dev *dev, u32 stream)
{
    u32 i = 0;

    if (stream == SNDRV_PCM_STREAM_PLAYBACK) {
        for (i = 0; i < 4; i++)
            i2s_read_reg(dev->i2s_base, TOR(i));
    } else {
        for (i = 0; i < 4; i++)
            i2s_read_reg(dev->i2s_base, ROR(i));
    }
}

static inline void i2s_disable_irqs(struct dw_i2s_dev *dev, u32 stream,
                    int chan_nr)
{
    u32 i, irq;

    if (stream == SNDRV_PCM_STREAM_PLAYBACK) {
        for (i = 0; i < (chan_nr / 2); i++) {
            irq = i2s_read_reg(dev->i2s_base, IMR(i));
            i2s_write_reg(dev->i2s_base, IMR(i), irq | 0x30);
        }
    } else {
        for (i = 0; i < (chan_nr / 2); i++) {
            irq = i2s_read_reg(dev->i2s_base, IMR(i));
            i2s_write_reg(dev->i2s_base, IMR(i), irq | 0x03);
        }
    }
}

static inline void i2s_enable_irqs(struct dw_i2s_dev *dev, u32 stream,
                   int chan_nr)
{
    u32 i, irq;

    if (stream == SNDRV_PCM_STREAM_PLAYBACK) {
        for (i = 0; i < (chan_nr / 2); i++) {
            irq = i2s_read_reg(dev->i2s_base, IMR(i));
            i2s_write_reg(dev->i2s_base, IMR(i), irq & ~0x30);
        }
    } else {
        for (i = 0; i < (chan_nr / 2); i++) {
            irq = i2s_read_reg(dev->i2s_base, IMR(i));
            i2s_write_reg(dev->i2s_base, IMR(i), irq & ~0x03);
        }
    }
}

static void dw_pcm_transfer(struct dw_i2s_dev *dev, bool push)
{
    struct snd_pcm_substream *substream = dev->substream;
    unsigned int ret;

    if (push) {
        ret = dev->tx_fn(dev, substream);
    } else {
        ret = dev->rx_fn(dev, substream);
    }
    if(ret)
        substream->done = 1;
}

void dw_pcm_push_tx(struct dw_i2s_dev *dev)
{
    dw_pcm_transfer(dev, true);
}

void dw_pcm_pop_rx(struct dw_i2s_dev *dev)
{
    dw_pcm_transfer(dev, false);
}


static unsigned int i2s_poll(struct dw_i2s_dev *dev)
{
    bool irq_valid = false;
    u32 isr[4];
    int i;

    for (i = 0; i < 4; i++) {
        isr[i] = i2s_read_reg(dev->i2s_base, ISR(i));
    }

    i2s_clear_irqs(dev, SNDRV_PCM_STREAM_PLAYBACK);
    i2s_clear_irqs(dev, SNDRV_PCM_STREAM_CAPTURE);

    for (i = 0; i < 4; i++) {
        /*
         * Check if TX fifo is empty. If empty fill FIFO with samples
         * NOTE: Only two channels supported
         */
        if ((isr[i] & ISR_TXFE) && (i == 0) && dev->use_pio) {
            dw_pcm_push_tx(dev);
        }

        /*
         * Data available. Retrieve samples from FIFO
         * NOTE: Only two channels supported
         */
        if ((isr[i] & ISR_RXDA) && (i == 0) && dev->use_pio) {
            dw_pcm_pop_rx(dev);
            irq_valid = true;
        }

        /* Error Handling: TX */
        if (isr[i] & ISR_TXFO) {
            printf("TX overrun (ch_id=%d)\n", i);
            irq_valid = true;
        }

        /* Error Handling: TX */
        if (isr[i] & ISR_RXFO) {
            printf("RX overrun (ch_id=%d)\n", i);
            irq_valid = true;
        }

    }

    if (irq_valid)
        return 1;
    else
        return 0;
}

static unsigned int i2s_poll_rx(struct dw_i2s_dev *dev)
{
    u32 isr;
    struct snd_pcm_substream *substream = dev->substream;
    i2s_read_reg(dev->i2s_base, ROR(0));
    while(!substream->done) {
        isr = i2s_read_reg(dev->i2s_base, ISR(0));
        if(isr & ISR_RXDA)
            dw_pcm_pop_rx(dev);

        if(isr & ISR_RXFO) {
            printf("#");
            i2s_read_reg(dev->i2s_base, ROR(0));
        }
    }
    return 0;
}

static unsigned int i2s_poll_tx(struct dw_i2s_dev *dev)
{
    u32 isr;
    struct snd_pcm_substream *substream = dev->substream;
    i2s_read_reg(dev->i2s_base, TOR(0));
    while(!substream->done) {
        isr = i2s_read_reg(dev->i2s_base, ISR(0));
        if(isr & ISR_TXFE)
            dw_pcm_push_tx(dev);
        if(isr & ISR_TXFO) {
            printf("@");
            i2s_read_reg(dev->i2s_base, TOR(0));
        }
    }
    return 0;
}


static void dw_i2s_dump_regs(struct dw_i2s_dev *dev)
{
    printf("IER   : 0x%x\n", i2s_read_reg(dev->i2s_base, CER));
    printf("CCR   : 0x%x\n", i2s_read_reg(dev->i2s_base, CCR));
    printf("DMACR : 0x%x\n", i2s_read_reg(dev->i2s_base, DMACR));
    printf("ITER  : 0x%x\n", i2s_read_reg(dev->i2s_base, ITER));
    printf("CER   : 0x%x\n", i2s_read_reg(dev->i2s_base, CER));
    printf("IMR0  : 0x%x\n", i2s_read_reg(dev->i2s_base, IMR(0)));
    printf("TFCR0 : 0x%x\n", i2s_read_reg(dev->i2s_base, TFCR(0)));
    printf("TER0  : 0x%x\n", i2s_read_reg(dev->i2s_base, TER(0)));
}

static void i2s_start(struct dw_i2s_dev *dev, u32 stream)
{
    struct i2s_clk_config_data *config = &dev->config;

    if(!dev->use_pio) {
        if (stream == SNDRV_PCM_STREAM_PLAYBACK)
            i2s_write_reg(dev->i2s_base, DMACR, 0x00020000);/*DMA_TXBLOCK_EN*/
        else
            i2s_write_reg(dev->i2s_base, DMACR, 0x00010000);/*DMA_TXBLOCK_EN*/
    }

    i2s_write_reg(dev->i2s_base, IER, 1);
    i2s_enable_irqs(dev, stream, config->chan_nr);

    if (stream == SNDRV_PCM_STREAM_PLAYBACK)
        i2s_write_reg(dev->i2s_base, ITER, 1);
    else
        i2s_write_reg(dev->i2s_base, IRER, 1);

    i2s_write_reg(dev->i2s_base, CER, 1);
    //dw_i2s_dump_regs(dev);
}

static void i2s_stop(struct dw_i2s_dev *dev, u32 stream)
{
    i2s_clear_irqs(dev, stream);
    if (stream == SNDRV_PCM_STREAM_PLAYBACK)
        i2s_write_reg(dev->i2s_base, ITER, 0);
    else
        i2s_write_reg(dev->i2s_base, IRER, 0);

    i2s_disable_irqs(dev, stream, 8);


    i2s_write_reg(dev->i2s_base, CER, 0);
    i2s_write_reg(dev->i2s_base, IER, 0);

}

static int dw_i2s_startup(struct snd_soc_dai_dev *dai_dev,
            struct snd_pcm_substream *substream)
{
    struct dw_i2s_dev *dev = (struct dw_i2s_dev *)dai_dev->cpu_dai;
    if (!(dev->capability & DWC_I2S_RECORD) &&
            (substream->stream == SNDRV_PCM_STREAM_CAPTURE))
        return -1;

    if (!(dev->capability & DWC_I2S_PLAY) &&
            (substream->stream == SNDRV_PCM_STREAM_PLAYBACK))
        return -1;
    return 0;
}

static void dw_i2s_config(struct dw_i2s_dev *dev, int stream)
{
    u32 ch_reg;
    struct i2s_clk_config_data *config = &dev->config;


    i2s_disable_channels(dev, stream);

    for (ch_reg = 0; ch_reg < (config->chan_nr / 2); ch_reg++) {
        if (stream == SNDRV_PCM_STREAM_PLAYBACK) {
            i2s_write_reg(dev->i2s_base, TCR(ch_reg),
                      dev->xfer_resolution);
            i2s_write_reg(dev->i2s_base, TFCR(ch_reg),
                      /*dev->fifo_th - 1*/0);
            i2s_write_reg(dev->i2s_base, TER(ch_reg), 1);
        } else {
            i2s_write_reg(dev->i2s_base, RCR(ch_reg),
                      dev->xfer_resolution);
            i2s_write_reg(dev->i2s_base, RFCR(ch_reg),
                      /*dev->fifo_th - 1*/0);
            i2s_write_reg(dev->i2s_base, RER(ch_reg), 1);
        }
    }
}

static int dw_i2s_clk_cfg(struct i2s_clk_config_data *config)
{
    unsigned int mclk, sclk, mdiv, sdiv;
    unsigned int reg = 0;

    sclk = config->chan_nr * config->data_width * config->sample_rate;
#ifdef CONFIG_AUDIO_MCLK_FROM_PLL
    /* By the default, MCLK = 256fs */
    mclk = config->sample_rate << 8;
    mdiv = AUDIO_MCLK_BASE_FREQ / mclk - 1;
    sdiv = mclk / sclk - 1;
    reg |= I2S_SCLK_FROM_PLL;
#else
    mclk = AUDIO_MCLK_BASE_FREQ;
    mdiv = 0;
    sdiv = mclk / sclk - 1;
    reg |= I2S_SCLK_FROM_EXT;
#endif
    reg |= (mdiv | sdiv << 12);

    write_mreg32(CK_CRM_ADDR + CRM_I2S_CLK_CFG_REG, reg);

    return 0;
}


static int dw_i2s_hw_params(struct snd_soc_dai_dev *dai_dev,
            struct snd_pcm_substream *substream)
{
    struct dw_i2s_dev *dev = (struct dw_i2s_dev *)dai_dev->cpu_dai;
    struct i2s_clk_config_data *config = &dev->config;
    int ret;

    switch (substream->params.data_width) {
    case 16:
        config->data_width = 16;
        dev->ccr = 0x00;
        dev->xfer_resolution = 0x02;
        dev->tx_fn = dw_pcm_tx_16;
        dev->rx_fn = dw_pcm_rx_16;
        break;

    case 24:
        config->data_width = 24;
        dev->ccr = 0x08;
        dev->xfer_resolution = 0x04;
        dev->tx_fn = dw_pcm_tx_32;
        dev->rx_fn = dw_pcm_rx_32;
        break;

    case 32:
        config->data_width = 32;
        dev->ccr = 0x10;
        dev->xfer_resolution = 0x05;
        dev->tx_fn = dw_pcm_tx_32;
        dev->rx_fn = dw_pcm_rx_32;
        break;

    default:
        printf("designware-i2s: unsupported PCM fmt");
        return -1;
    }

    config->chan_nr = substream->params.chan_nr;

    switch (config->chan_nr) {
    case EIGHT_CHANNEL_SUPPORT:
    case SIX_CHANNEL_SUPPORT:
    case FOUR_CHANNEL_SUPPORT:
    case TWO_CHANNEL_SUPPORT:
        break;
    default:
        printf("channel not supported\n");
        return -1;
    }

    dw_i2s_config(dev, substream->stream);

    i2s_write_reg(dev->i2s_base, CCR, dev->ccr);

    config->sample_rate = substream->params.sample_rate;

    if (dev->i2s_clk_cfg) {
        ret = dev->i2s_clk_cfg(config);
        if (ret < 0) {
            printf("runtime audio clk config fail\n");
            return ret;
        }
    } else {
        printf("can not config clock\n");
    }

    return 0;
}

static int dw_i2s_prepare(struct snd_soc_dai_dev *dai_dev,
            struct snd_pcm_substream *substream)
{
    struct dw_i2s_dev *dev = (struct dw_i2s_dev *)dai_dev->cpu_dai;
    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
        i2s_write_reg(dev->i2s_base, TXFFR, 1);
    else
        i2s_write_reg(dev->i2s_base, RXFFR, 1);

    return 0;
}

static int dw_i2s_trigger(struct snd_soc_dai_dev *dai_dev, u32 cmd,
            struct snd_pcm_substream *substream)
{
    struct dw_i2s_dev *dev = (struct dw_i2s_dev *)dai_dev->cpu_dai;
    int ret = 0;
    switch (cmd) {
    case SNDRV_PCM_TRIGGER_START:
        i2s_start(dev, substream->stream);
        break;

    case SNDRV_PCM_TRIGGER_STOP:
        i2s_stop(dev, substream->stream);
        break;
    default:
        ret = -1;
        break;
    }
    return ret;
}

static const struct snd_soc_dai_ops dw_i2s_dai_ops = {
    .startup    = dw_i2s_startup,
    .hw_params  = dw_i2s_hw_params,
    .prepare    = dw_i2s_prepare,
    .trigger    = dw_i2s_trigger,
};

static int dw_configure_by_comp(struct dw_i2s_dev *dev)
{
    u32 comp1 = i2s_read_reg(dev->i2s_base, I2S_COMP_PARAM_1);
    //u32 comp2 = i2s_read_reg(dev->i2s_base, I2S_COMP_PARAM_2);
    u32 fifo_depth = 1 << (1 + COMP1_FIFO_DEPTH_GLOBAL(comp1));

    if (COMP1_MODE_EN(comp1)) {
        printf("i2s master mode supported\n");
        dev->capability |= DW_I2S_MASTER;
    } else {
        printf("i2s slave mode supported\n");
        dev->capability |= DW_I2S_SLAVE;
    }

    if (COMP1_TX_ENABLED(comp1)) {
        dev->capability |= DWC_I2S_PLAY;
    }
    if (COMP1_RX_ENABLED(comp1)) {
        dev->capability |= DWC_I2S_RECORD;
    }

    dev->fifo_th = fifo_depth;
    //printf("comp1 = 0x%x, fifo_th: %d\n", comp1, fifo_depth);
    return 0;
}


struct dw_i2s_dev i2s_dev[MAX_I2S_NUM_ON_CHIP];

int dw_apb_i2s_probe(struct snd_soc_dai_dev *dai_dev,
                                unsigned char id)
{
    struct dw_i2s_dev *dev;

    if(id >= MAX_I2S_NUM_ON_CHIP) {
        printf("Bad i2s id\n");
        return -1;
    }

    dev = &i2s_dev[id];

    memset(dev, 0 , sizeof(struct dw_i2s_dev));

    dev->i2s_base = i2s_iobase_list[id];
    dev->dai_ops = &dw_i2s_dai_ops;

    dai_dev->cpu_dai = dev;

    if(dw_configure_by_comp(dev))
        goto error;

    dev->i2s_clk_cfg = dw_i2s_clk_cfg;
    dev->config.mclk = dai_dev->sysclk;
    if(dev->capability & DW_I2S_MASTER)
        dev->poll = i2s_poll_tx;
    else
        dev->poll = i2s_poll_rx;

    dev->use_pio = true;

    printf("i2s.0x%x %s probe done, use %s\n",
            (unsigned int)dev->i2s_base,
            dev->capability & DW_I2S_MASTER ? "master" : "slave",
            dev->use_pio == true ? "pio" : "ahb dma");

    return 0;

error:
    return -1;
}

