/*****************************************************************************
 *  File: taudio.c
 *
 *  Descirption: Test case about audio card.
 *
 *  Copyright (C) : 2019 VeriSilicon.
 *
 *  Author	: Liu.Wei@verisilicon.com
 *  Date	: 2019/3/19 8:59:30
 *
 *  Usage   : Play   : Play a default sound - "da mei zi...".
 *            Record : Record about 1 second sound, and then play it.
 *                     you can hear what u record.
 *
 *****************************************************************************/
#include <string.h>
#include "ck810.h"
#include "dw_apb_i2s.h"
#include "snd_pcm.h"
#include "datatype.h"
#include "snd_sample.h"
#include "misc.h"
#include "ad193x.h"

#define GS0_AUDIO_EN	(4)
#define GS0_IOPROCEN	(5)
#define GS0_RESET	(9)
#define GS1_AUDIO_EN	(7)
#define GS1_IOPROCEN	(8)
#define GS1_RESET	(6)
#define AD1938_RESET	(15)

#define MAX_SND_STREAM_BUF_LEN (1024 * 1024)

struct snd_pcm_substream snd_substream;
struct snd_soc_dai_dev dai_dev;

void wav_show(struct wav_info info,
                     struct riff_header header)
{
    printf("\n*********** Audio Informations ***********\n");
    printf("Riff Tag    :%c%c%c%c\n",
        header.riff[0], header.riff[1], header.riff[2], header.riff[3]);
    printf("Total DataLen   :%d(B)\n", header.flen);
    printf("Wave Tag    :%c%c%c%c\n",
        header.wave[0], header.wave[1], header.wave[2], header.wave[3]);
    printf("Fmt Tag     :%c%c%c%c\n",
        header.fmt[0], header.fmt[1], header.fmt[2], header.fmt[3]);
    printf("Filter Byte :0x%08x\n", header.filter);
    printf("Format Tag  :0x%04x\n", info.format);
    printf("Channel Number  :%d\n", info.channel);
    printf("Sample Rate :%d\n", info.sample_rate);
    printf("Byte per Second :%d\n", info.byte_per_sec);
    printf("Byte per Sample :%d\n", info.byte_per_sample);
    printf("Bit per Sample  :%d\n", info.bit_per_sample);
    printf("DATA Tag    :%c%c%c%c\n",
        info.datatag[0], info.datatag[1],
        info.datatag[2], info.datatag[3]);
    printf("Data Length :%d(B)\n", info.data_len);
    printf("*********************************************\n\n");

}

struct snd_pcm_substream *wav_stream_init(void *audio_base, unsigned int stream)
{
    struct snd_pcm_substream *substream;
    struct wav_info   info;
    struct riff_header header;

    substream = &snd_substream;

    memset(substream, 0 , sizeof(struct snd_pcm_substream));

    substream->snd_base = audio_base;
    substream->stream = stream;
    substream->ptr = 0;
    substream->done = 0;

    if(stream == SNDRV_PCM_STREAM_CAPTURE) {
        substream->params.chan_nr = DEF_REC_FMT_CHANNEL;
        substream->params.data_width = DEF_REC_FMT_WIDTH;
        substream->params.sample_rate = DEF_REC_FMT_SAMPLE;
        substream->sz_rx = MAX_SND_STREAM_BUF_LEN;
        substream->data_base = audio_base;
        return substream;
    }

    memset(&header, 0, sizeof(struct riff_header));
    memset(&info, 0, sizeof(struct wav_info));

    memcpy(&header, (unsigned char *)audio_base, sizeof(struct riff_header));
    memcpy(&info, (unsigned char *)audio_base + sizeof(struct riff_header),
        sizeof(struct wav_info));

    wav_show(info, header);

    substream->data_base = audio_base +
            sizeof(struct riff_header) +
            sizeof(struct wav_info);
    substream->sz_tx = info.data_len/info.byte_per_sample;
    substream->params.chan_nr = 2;//info.channel;
    substream->params.data_width = info.bit_per_sample;
    substream->params.sample_rate = info.sample_rate;

    return substream;

}

void snd_msg_show(struct snd_pcm_substream *substream)
{
    printf("\n*********** Audio Informations ***********\n");
    printf("Format          : %s\n", "PCM");
    printf("Channel Number  : %d\n", substream->params.chan_nr);
    printf("Sample Rate     : %d\n", substream->params.sample_rate);
    printf("Bit per Sample  : %d\n", substream->params.data_width);
    printf("Data Length     : %d(in Word)\n",
        substream->stream == SNDRV_PCM_STREAM_PLAYBACK ?
        substream->sz_tx : substream->sz_rx);
    printf("Data Base       : 0x%x\n", substream->data_base);
    printf("*******************************************\n\n");

}

struct snd_pcm_substream *audio_stream_init(void *audio_base, unsigned int stream)
{
    struct snd_pcm_substream *substream;

    substream = &snd_substream;

    memset(substream, 0 , sizeof(struct snd_pcm_substream));

    substream->data_base = audio_base;
    substream->ptr = 0;
    substream->done = 0;
    substream->stream = stream;
    substream->params.chan_nr = sample_info.channel;
    substream->params.data_width = sample_info.bit_per_sample;
    substream->params.sample_rate = sample_info.sample_rate;

    if(stream == SNDRV_PCM_STREAM_PLAYBACK) {
        substream->sz_tx = sizeof(snd_sample) / sample_info.byte_per_sample;
        substream->fmt = SAMPLE_DAI_FMT_PLAYBACK;
    } else {
        substream->sz_rx = sizeof(snd_sample) / sample_info.byte_per_sample;
        substream->fmt = SAMPLE_DAI_FMT_CAPTURE;
    }

    snd_msg_show(substream);

    return substream;

}


void Audio_Test_OLD()
{
    struct dw_i2s_dev *i2s_dev;
    struct ad193x_dev *codec_dev;
    struct snd_pcm_substream *substream;

    dai_dev.sysclk = MCLK_IN_FREQ;

    // Init cpu dai(i2s)
    if(dw_apb_i2s_probe(&dai_dev, I2S_FOR_PLAY)) {
        printf("Probe I2S.%d falied\n", I2S_FOR_PLAY);
        goto failed_label;
    }
    i2s_dev = (struct dw_i2s_dev *)dai_dev.cpu_dai;

    // Init codec dai(ad1938)
    if(ad193x_codec_probe(&dai_dev, SPI_FOR_AD1938)) {
        printf("Probe codec falied(spi:%d)\n", SPI_FOR_AD1938);
        goto failed_label;
    }
    codec_dev = (struct ad193x_dev *)dai_dev.codec_dai;

    // Init audio stream.
    substream = audio_stream_init(snd_sample, SNDRV_PCM_STREAM_PLAYBACK);
    if(!substream) {
        printf("Initialize substream falied\n");
        goto failed_label;
    }

    i2s_dev->substream = substream;

    if(codec_dev->dai_ops->startup(&dai_dev, substream) ||
        i2s_dev->dai_ops->startup(&dai_dev, substream))
        return;

    if(codec_dev->dai_ops->hw_params(&dai_dev, substream) ||
        i2s_dev->dai_ops->hw_params(&dai_dev, substream))
        return;

    if(i2s_dev->dai_ops->prepare(&dai_dev, substream))
        return;

    printf("Playing....");
    if(i2s_dev->dai_ops->trigger(&dai_dev, SNDRV_PCM_TRIGGER_START, substream))
        return;

    while(!substream->done)
        i2s_dev->poll(i2s_dev);
    printf("Done\n");

    i2s_dev->dai_ops->trigger(&dai_dev, SNDRV_PCM_TRIGGER_STOP, substream);

    printf("I2S.%d Test Success.\n", I2S_FOR_PLAY);


    return;

failed_label:
    printf("I2S.%dTest Failed.\n", I2S_FOR_PLAY);
}

int Audio_Play_Test()
{
    struct dw_i2s_dev *i2s_dev;
    struct ad193x_dev *codec_dev;
    struct snd_pcm_substream *substream;

    memset(&dai_dev, 0, sizeof(dai_dev));

    dai_dev.sysclk = MCLK_IN_FREQ;


    if(dw_apb_i2s_probe(&dai_dev, I2S_FOR_PLAY)) {
        printf("Probe I2S.%d falied\n", I2S_FOR_PLAY);
        goto failed_label;
    }
    i2s_dev = (struct dw_i2s_dev *)dai_dev.cpu_dai;


    if(ad193x_codec_probe(&dai_dev, SPI_FOR_AD1938)) {
        printf("Probe codec falied(spi:%d)\n", SPI_FOR_AD1938);
        goto failed_label;
    }
    codec_dev = (struct ad193x_dev *)dai_dev.codec_dai;

    substream = audio_stream_init(snd_sample, SNDRV_PCM_STREAM_PLAYBACK);
    if(!substream) {
        printf("Initialize substream falied\n");
        goto failed_label;
    }

    i2s_dev->substream = substream;

    if(codec_dev->dai_ops->startup(&dai_dev, substream) ||
        i2s_dev->dai_ops->startup(&dai_dev, substream))
        goto failed_label;

    if(codec_dev->dai_ops->hw_params(&dai_dev, substream) ||
        i2s_dev->dai_ops->hw_params(&dai_dev, substream))
        goto failed_label;

    if(i2s_dev->dai_ops->prepare(&dai_dev, substream))
        goto failed_label;

    printf("Playing....");
    if(i2s_dev->dai_ops->trigger(&dai_dev, SNDRV_PCM_TRIGGER_START, substream))
        goto failed_label;

    i2s_dev->poll(i2s_dev);

    i2s_dev->dai_ops->trigger(&dai_dev, SNDRV_PCM_TRIGGER_STOP, substream);
    printf("Done\n");

    return 0;

failed_label:
    printf("I2S.%d Play Failed.\n", I2S_FOR_PLAY);
    return -1;

}


int Audio_Record_Test(int i2s_id)
{
    struct dw_i2s_dev *i2s_dev;
    struct ad193x_dev *codec_dev;
    struct snd_pcm_substream *substream;

    memset(&dai_dev, 0, sizeof(dai_dev));

    dai_dev.sysclk = MCLK_IN_FREQ;

    if(i2s_id == I2S_FOR_PLAY) {
        printf("You selected a master port.\n");
        goto failed_label;
    }

    if(ad193x_codec_probe(&dai_dev, SPI_FOR_AD1938)) {
        printf("Probe codec falied(spi:%d)\n", SPI_FOR_AD1938);
        goto failed_label;
    }
    codec_dev = (struct ad193x_dev *)dai_dev.codec_dai;

    if(dw_apb_i2s_probe(&dai_dev, i2s_id)) {
        printf("Probe I2S.%d falied\n", i2s_id);
        goto failed_label;
    }
    i2s_dev = (struct dw_i2s_dev *)dai_dev.cpu_dai;

    substream = audio_stream_init(snd_sample, SNDRV_PCM_STREAM_CAPTURE);
    if(!substream) {
        printf("Initialize substream falied\n");
        goto failed_label;
    }

    i2s_dev->substream = substream;

    if(codec_dev->dai_ops->startup(&dai_dev, substream) ||
        i2s_dev->dai_ops->startup(&dai_dev, substream))
        goto failed_label;

    if(codec_dev->dai_ops->hw_params(&dai_dev, substream) ||
        i2s_dev->dai_ops->hw_params(&dai_dev, substream))
        goto failed_label;

    if(i2s_dev->dai_ops->prepare(&dai_dev, substream))
        goto failed_label;

    printf("Recording....");
    if(i2s_dev->dai_ops->trigger(&dai_dev, SNDRV_PCM_TRIGGER_START, substream))
        goto failed_label;

    i2s_dev->poll(i2s_dev);

    printf("Done\n");

    i2s_dev->dai_ops->trigger(&dai_dev, SNDRV_PCM_TRIGGER_STOP, substream);

    return 0;

failed_label:
    printf("I2S.%d Record Failed.\n", i2s_id);
    return -1;
}

void Audio_Test()
{
    char choose;
    int i2s_id;

    CK_Gpio_Output(GS0_AUDIO_EN, 1);
    CK_Gpio_Output(GS0_IOPROCEN, 1);
    CK_Gpio_Output(GS0_RESET, 1);
    CK_Gpio_Output(GS1_AUDIO_EN, 1);
    CK_Gpio_Output(GS1_IOPROCEN, 1);
    CK_Gpio_Output(GS1_RESET, 1);
    CK_Gpio_Output(AD1938_RESET, 0);
re_select:
    printf("Please select whitch I2S you want to test.(0~3-slave, 4-master)\n");
    //scanf("%d", &i2s_id);
    choose = getchar();
    putchar(choose);
    putchar('\n');
    i2s_id = choose - '0';

    if(i2s_id < 0 || i2s_id > 4) {
        printf("Invalid i2s port %d\n", i2s_id);
        goto re_select;
    }

    if(i2s_id != I2S_FOR_PLAY) {
        if(Audio_Record_Test(i2s_id))
            return;
        printf("\nPlaing your record...\n");
    }

    Audio_Play_Test();
}
