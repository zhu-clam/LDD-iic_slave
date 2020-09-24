#ifndef _SND_PCM_H_
#define _SND_PCM_H_

#include "datatype.h"
#include "dw_apb_i2s.h"

#define DEF_REC_FMT_CHANNEL	2
#define DEF_REC_FMT_WIDTH	32
#define DEF_REC_FMT_SAMPLE	48000

enum {
	SNDRV_PCM_STREAM_PLAYBACK = 0,
	SNDRV_PCM_STREAM_CAPTURE,
	SNDRV_PCM_STREAM_LAST = SNDRV_PCM_STREAM_CAPTURE,
};

#define SNDRV_PCM_TRIGGER_STOP		0
#define SNDRV_PCM_TRIGGER_START		1


struct riff_header {
	u8 riff[4];
	u32 flen;
	u8 wave[4];
	u8 fmt[4];
	u32 filter;
};

struct wav_info {
	u16 format;
	u16 channel;
	u32 sample_rate;
	u32 byte_per_sec;
	u16 byte_per_sample;
	u16 bit_per_sample;
/**
 * Sometime, there are some optional dummy byte after wav 
 * header(before data tag). For example .wav is converted form 
 * some software. .
 */
	// u8 pad[2];
	u8 datatag[4];
	u32 data_len;
}/*__attribute__((packed))*/;



struct snd_pcm_hw_params {
	unsigned int chan_nr;
	unsigned int data_width;
	unsigned int sample_rate;
};

struct snd_pcm_substream {
	void *snd_base;
	void *data_base;
	unsigned int ptr;
	unsigned int stream;
	unsigned int sz_tx;
	unsigned int sz_rx;
	unsigned int done;
    unsigned int fmt;
	struct snd_pcm_hw_params params;
};

struct i2s_clk_config_data {
    int mclk;
	int chan_nr;
	u32 data_width;
	u32 sample_rate;
};



#endif
