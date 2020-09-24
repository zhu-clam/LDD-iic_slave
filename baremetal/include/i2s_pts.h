// I2S PTS registers addr define

#ifndef ___I2S_PTS_H___
#define ___I2S_PTS_H___

#include "ck810.h"

#define I2S_PTS_ENABLE                  I2S_PTS_BASE + 0x000
#define I2S_AUDIO_LEN                   I2S_PTS_BASE + 0x004
#define I2S_SAMPLE_EDGE                 I2S_PTS_BASE + 0x008
#define I2S_PTS_IRQ_EN                  I2S_PTS_BASE + 0x00c
#define I2S_PTS_IRQ_CLR                 I2S_PTS_BASE + 0x010
#define I2S_PTS_IRQ_STATUS              I2S_PTS_BASE + 0x014
#define I2S_SAMPLE_CNT0                 I2S_PTS_BASE + 0x018
#define I2S_SAMPLE_CNT1                 I2S_PTS_BASE + 0x01c
#define I2S_FIFO_STATUS                 I2S_PTS_BASE + 0x020
#define I2S_FIFO_WORDS                  I2S_PTS_BASE + 0x024
#define I2S_CH0_FIFO_DATA               I2S_PTS_BASE + 0x028
#define I2S_CH1_FIFO_DATA               I2S_PTS_BASE + 0x02c
#define I2S_CH2_FIFO_DATA               I2S_PTS_BASE + 0x030
#define I2S_CH3_FIFO_DATA               I2S_PTS_BASE + 0x034

#endif // ___I2S_PTS_H___
