/*****************************************************************************
 *  File: ad1938_spi.c
 *
 *  Descirption: Driver for Codec AD1938, only spi port support.
 *
 *  Copyright (C) : 2019 VeriSilicon.
 *
 *  Author	: Liu.Wei@verisilicon.com
 *  Date	: 2019/3/19 8:59:30
 *
 *****************************************************************************/
#include <string.h>
#include "ck810.h"
#include "dw_apb_i2s.h"
#include "snd_pcm.h"
#include "datatype.h"
#include "misc.h"
#include "ad193x.h"
#include "vs_apb_spi.h"
#include "soc_dai.h"


/**
 * The first byte is a global address with a read/write bit. For the
 * AD1938, the address is 0x04, shifted left one bit due to the R/W
 * bit. The second byte is the AD1938 register address and the
 * third byte is the data. ------ad1938.pdf
 * 
 */

#define AD1938_GADDR    0x04

void ad193x_spi_write(struct ad193x_dev *ad193x, unsigned char reg, unsigned char val)
{
    struct spi_transfer xfer;
    unsigned char txbuf[3];
    memset(&xfer, 0, sizeof(struct spi_transfer));
  
    txbuf[0] = AD1938_GADDR << 1;
    txbuf[1] = reg;
    txbuf[2] = val;
    xfer.tx_buf = txbuf;
    xfer.rx_buf = NULL;
    xfer.bits_per_word = 8;
    xfer.len = 3;
    xfer.speed_hz = 500000;
    xfer.rx_nbits = SPI_NBITS_SINGLE;

    vsi_spi_set_cs(ad193x->spidev, true);

    vsi_spi_transfer_one(ad193x->spidev, &xfer);

    vsi_spi_set_cs(ad193x->spidev, false);    
}

unsigned char ad193x_spi_read(struct ad193x_dev *ad193x, unsigned char reg)
{
    struct spi_transfer xfer;
    unsigned char txbuf[2] = {0};
    unsigned char rxbuf[1] = {0};
    memset(&xfer, 0, sizeof(struct spi_transfer));
  
    txbuf[0] = AD1938_GADDR << 1 | 0x01;
    txbuf[1] = reg;
    xfer.tx_buf = txbuf;
    xfer.rx_buf = NULL;
    xfer.bits_per_word = 8;
    xfer.len = 2;
    xfer.speed_hz = 500000;
    xfer.rx_nbits = SPI_NBITS_SINGLE;

    vsi_spi_set_cs(ad193x->spidev, true);

    vsi_spi_transfer_one(ad193x->spidev, &xfer);
    
    xfer.tx_buf = NULL;
    xfer.rx_buf = rxbuf;
    xfer.len = 1;
    vsi_spi_transfer_one(ad193x->spidev, &xfer);

    vsi_spi_set_cs(ad193x->spidev, false);
    
    return rxbuf[0];

}


void ad193x_spi_update_bits(struct ad193x_dev *ad193x, unsigned char reg,
                             unsigned char mask, unsigned char val)
{
    unsigned char t = ad193x_spi_read(ad193x, reg);
    t &= ~mask;
    t |= val;
    ad193x_spi_write(ad193x, reg, t);
}

static int ad193x_set_tdm_slot(struct snd_soc_dai_dev *dai, int slots)
{
    struct ad193x_dev *ad193x = (struct ad193x_dev *)dai->codec_dai;
    unsigned int channels;

    switch (slots) {
    case 2:
        channels = AD193X_2_CHANNELS;
        break;
    case 4:
        channels = AD193X_4_CHANNELS;
        break;
    case 8:
        channels = AD193X_8_CHANNELS;
        break;
    case 16:
        channels = AD193X_16_CHANNELS;
        break;
    default:
        return -1;
    }

    ad193x_spi_update_bits(ad193x, AD193X_DAC_CTRL1,
        AD193X_DAC_CHAN_MASK, channels << AD193X_DAC_CHAN_SHFT);

    ad193x_spi_update_bits(ad193x, AD193X_ADC_CTRL2,
                   AD193X_ADC_CHAN_MASK,
                   channels << AD193X_ADC_CHAN_SHFT);

    return 0;
}

void ad1938_reg_dump(struct ad193x_dev *ad193x)
{
    unsigned char val, i;

    printf("==========AD1938 Register DUMP======\n");
    for(i = 0; i <= 16; i++) {
        val = ad193x_spi_read(ad193x, i);
        printf("Reg_%d, val_0x%x\n", i, val);
    }
    printf("=====================================\n");
    
    
}

static int ad193x_set_dai_fmt(struct snd_soc_dai_dev *dai,
        unsigned int fmt)
{
    struct ad193x_dev *ad193x = (struct ad193x_dev *)dai->codec_dai;
    unsigned int adc_serfmt = 0;
    unsigned int adc_fmt = 0;
    unsigned int dac_fmt = 0;

    /* At present, the driver only support AUX ADC mode(SND_SOC_DAIFMT_I2S
     * with TDM) and ADC&DAC TDM mode(SND_SOC_DAIFMT_DSP_A)
     */
    switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
    case SND_SOC_DAIFMT_I2S:
        adc_serfmt |= AD193X_ADC_SERFMT_TDM;
        break;
    case SND_SOC_DAIFMT_DSP_A:
        adc_serfmt |= AD193X_ADC_SERFMT_AUX;
        break;
    default:
        break;
    }

    switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
    case SND_SOC_DAIFMT_NB_NF: /* normal bit clock + frame */
        break;
    case SND_SOC_DAIFMT_NB_IF: /* normal bclk + invert frm */
        adc_fmt |= AD193X_ADC_LEFT_HIGH;
        dac_fmt |= AD193X_DAC_LEFT_HIGH;
        break;
    case SND_SOC_DAIFMT_IB_NF: /* invert bclk + normal frm */
        adc_fmt |= AD193X_ADC_BCLK_INV;
        dac_fmt |= AD193X_DAC_BCLK_INV;
        break;
    case SND_SOC_DAIFMT_IB_IF: /* invert bclk + frm */
        adc_fmt |= AD193X_ADC_LEFT_HIGH;
        adc_fmt |= AD193X_ADC_BCLK_INV;
        dac_fmt |= AD193X_DAC_LEFT_HIGH;
        dac_fmt |= AD193X_DAC_BCLK_INV;
        break;
    default:
        return -1;
    }

    switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
    case SND_SOC_DAIFMT_CBM_CFM: /* codec clk & frm master */
        adc_fmt |= AD193X_ADC_LCR_MASTER;
        adc_fmt |= AD193X_ADC_BCLK_MASTER;
        dac_fmt |= AD193X_DAC_LCR_MASTER;
        dac_fmt |= AD193X_DAC_BCLK_MASTER;
        break;
    case SND_SOC_DAIFMT_CBS_CFM: /* codec clk slave & frm master */
        adc_fmt |= AD193X_ADC_LCR_MASTER;
        dac_fmt |= AD193X_DAC_LCR_MASTER;
        break;
    case SND_SOC_DAIFMT_CBM_CFS: /* codec clk master & frame slave */
        adc_fmt |= AD193X_ADC_BCLK_MASTER;
        dac_fmt |= AD193X_DAC_BCLK_MASTER;
        break;
    case SND_SOC_DAIFMT_CBS_CFS: /* codec clk & frm slave */
        break;
    default:
        return -1;
    }


    ad193x_spi_update_bits(ad193x, AD193X_ADC_CTRL1,
        AD193X_ADC_SERFMT_MASK, adc_serfmt);
    ad193x_spi_update_bits(ad193x, AD193X_ADC_CTRL2,
        AD193X_ADC_FMT_MASK, adc_fmt);
    
    ad193x_spi_update_bits(ad193x, AD193X_DAC_CTRL1,
        AD193X_DAC_FMT_MASK, dac_fmt);

    return 0;
}

static int ad193x_set_dai_sysclk(struct snd_soc_dai_dev *dai)
{
    struct ad193x_dev *ad193x = (struct ad193x_dev *)dai->codec_dai;
    switch (dai->sysclk) {
    case 12288000:
    case 18432000:
    case 24576000:
    case 36864000:
        ad193x->sysclk = dai->sysclk;
        return 0;
    }
    return -1;
}

static int ad193x_startup(struct snd_soc_dai_dev *dai, 
                            struct snd_pcm_substream *substream)
{
    if(ad193x_set_dai_sysclk(dai)) {
        printf("Bad systerm clock\n");
        return -1;
    }
    if(ad193x_set_tdm_slot(dai, substream->params.chan_nr)){
        printf("Bad channel number\n");
        return -1;
    }
        
    if(ad193x_set_dai_fmt(dai, substream->fmt)) {
        printf("Bad fmt\n");
        return -1;
    }
    return 0;
}

static int ad193x_hw_params(struct snd_soc_dai_dev *dai, 
                            struct snd_pcm_substream *substream)
{
    int word_len = 0, master_rate = 0;
    struct ad193x_dev *ad193x = (struct ad193x_dev *)dai->codec_dai;

    /* bit size */
    switch (substream->params.data_width) {
    case 16:
        word_len = 3;
        break;
    case 20:
        word_len = 1;
        break;
    case 24:
    case 32:
        word_len = 0;
        break;
    default:
        return -1;
    }

    switch (ad193x->sysclk) {
    case 12288000:
        master_rate = AD193X_PLL_INPUT_256;
        break;
    case 18432000:
        master_rate = AD193X_PLL_INPUT_384;
        break;
    case 24576000:
        master_rate = AD193X_PLL_INPUT_512;
        break;
    case 36864000:
        master_rate = AD193X_PLL_INPUT_768;
        break;
    default:
        return -1;
    }
    ad193x_spi_update_bits(ad193x, AD193X_PLL_CLK_CTRL0,
                AD193X_PLL_INPUT_MASK, master_rate);

    ad193x_spi_update_bits(ad193x, AD193X_DAC_CTRL2,
                AD193X_DAC_WORD_LEN_MASK,
                word_len << AD193X_DAC_WORD_LEN_SHFT);

    ad193x_spi_update_bits(ad193x, AD193X_ADC_CTRL1,
            AD193X_ADC_WORD_LEN_MASK, word_len);
    //ad1938_reg_dump(ad193x);
    return 0;
}

static const struct snd_soc_dai_ops ad193x_dai_ops = {
    .hw_params = ad193x_hw_params,
    .startup   = ad193x_startup,
};


struct ad193x_dev ad193x_dev;
/*
 * Note: DAC_CTRL0 & ADC_CTRL0 Bit 0 must clear(power on). 
 * Linux driver : Clear it at snd power control .
 */
int ad193x_codec_probe(struct snd_soc_dai_dev *dai_dev, u8 spi_id)
{
    if(!dai_dev)
        return -1;

    memset(&ad193x_dev, 0, sizeof(struct ad193x_dev));
    dai_dev->codec_dai = &ad193x_dev;

    ad193x_dev.spidev = vsi_spi_probe(spi_id);
    if(!ad193x_dev.spidev) 
        return -1;

    ad193x_dev.dai_ops = &ad193x_dai_ops;
    
    /* unmute dac channels */
    ad193x_spi_write(&ad193x_dev, AD193X_DAC_CHNL_MUTE, 0x0);
    ad193x_spi_write(&ad193x_dev, AD193X_DAC_L1_VOL , 0x0);
    ad193x_spi_write(&ad193x_dev, AD193X_DAC_R1_VOL , 0x0);
    ad193x_spi_write(&ad193x_dev, AD193X_DAC_L2_VOL , 0x0);
    ad193x_spi_write(&ad193x_dev, AD193X_DAC_R2_VOL , 0x0);
    ad193x_spi_write(&ad193x_dev, AD193X_DAC_L3_VOL , 0x0);
    ad193x_spi_write(&ad193x_dev, AD193X_DAC_R3_VOL , 0x0);
    ad193x_spi_write(&ad193x_dev, AD193X_DAC_L4_VOL , 0x0); 
    ad193x_spi_write(&ad193x_dev, AD193X_DAC_R4_VOL , 0x0);

    /* de-emphasis: 48kHz, powedown dac */
    ad193x_spi_write(&ad193x_dev, AD193X_DAC_CTRL2, 0x1A);

    /* dac in tdm mode */
    //ad193x_spi_write(ad193x, AD193X_DAC_CTRL0, 0x40);     /* # 48k, TDM, raw.*/
    //ad193x_spi_write(ad193x, AD193X_DAC_CTRL0, 0x44);     /* 192k, TDM */
    ad193x_spi_write(&ad193x_dev, AD193X_DAC_CTRL0, 0x00);  /* 48k, Stereo. iis,delay=1. */
    //ad193x_spi_write(ad193x, AD193X_DAC_CTRL0, 0x08);     /* 48k, Stereo. iis,delay=0. */

    /* adc only */
    
    /* high-pass filter enable */
    ad193x_spi_write(&ad193x_dev, AD193X_ADC_CTRL0, 0x2);
    /* sata delay=1, adc aux mode */
    ad193x_spi_write(&ad193x_dev, AD193X_ADC_CTRL1, 0x43);

    /* pll input: mclki/xi , 0x99 power down, linux can power up , but bm can not. */
    //ad193x_spi_write(&ad193x_dev, AD193X_PLL_CLK_CTRL0, 0x99);    /* # mclk=24.576Mhz: 0x9D; mclk=12.288Mhz: 0x99 */
    ad193x_spi_write(&ad193x_dev, AD193X_PLL_CLK_CTRL0, 0x98);      /* Power on */

    /* 0x04 or 0x02 all ok. */
    ad193x_spi_write(&ad193x_dev, AD193X_PLL_CLK_CTRL1, 0x04);      /* PLL clock. disable on-chip v-ref */
    //ad193x_spi_write(&ad193x_dev, AD193X_PLL_CLK_CTRL1, 0x02);    /* PLL clock. enable on-chip v-ref, bit3 read only pll lock*/

    // ad1938_reg_dump(&ad193x_dev);

    printf("Codec AD1938 Probed\n");
    return 0;
}
