#ifndef _VS_APB_SPI_
#define _VS_APB_SPI_

#include "ck810.h"
#include "datatype.h"
#include "intc.h"

#define MAX_SPI_NUM_ON_CHIP 2

#define SPI_CPHA        0x01
#define SPI_CPOL        0x02

#define SPI_MODE_0      (0|0)
#define SPI_MODE_1      (0|SPI_CPHA)
#define SPI_MODE_2      (SPI_CPOL|0)
#define SPI_MODE_3      (SPI_CPOL|SPI_CPHA)

struct spi_transfer {
    /* it's ok if tx_buf == rx_buf (right?)
     * for MicroWire, one buffer must be null
     * buffers must work with dma_*map_single() calls, unless
     *   spi_message.is_dma_mapped reports a pre-existing mapping
     */
    const void  *tx_buf;
    void        *rx_buf;
    unsigned    len;

    unsigned    cs_change:1;
    unsigned    tx_nbits:3;
    unsigned    rx_nbits:3;
#define SPI_NBITS_SINGLE    0x01 /* 1bit transfer */
#define SPI_NBITS_DUAL      0x02 /* 2bits transfer */
#define SPI_NBITS_QUAD      0x04 /* 4bits transfer */
    u8      bits_per_word;
    u16     delay_usecs;
    u32     speed_hz;
};


struct vsi_spi {
    void  *regs;
    u8 id;
    u8 busy;
    u8 mode;
    u8 bpw;
    u32 speed;
    const void *tx;
    void *rx;
    u32 tx_len;
    u32 rx_len;
    u32 xfer_len;
    u32 state;
    u8 n_bytes;
    u8 n_bits;
    u32 fifo_depth;
    u32 clock_phase_mode;
    u32 clk_freq;
    u32 spi_num_cs;
};

int vsi_spi_transfer_one(struct vsi_spi *rs,
                         struct spi_transfer *xfer);

void vsi_spi_set_cs(struct vsi_spi *rs, bool enable);

struct vsi_spi *vsi_spi_probe(unsigned int id);



#endif