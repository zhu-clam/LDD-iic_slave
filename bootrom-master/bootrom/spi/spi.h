/*
 * (Copyright 2017) Verisilicon Inc. All Rights Reserved
 * Company confidential and Proprietary information.
 * This information may not be disclosed to unauthorized individual.
 * 
 * System Application Team <fengyin.wu@verisilicon.com>
 *
 */
 
#ifndef __SPI_H__
#define __SPI_H__
struct spi
{
    u32 spi_base;
    u32 baud_rate;
    u32 apb_clock;
    u32 spilen;//spi length bits
    u8  chip_sel;//chip select
    u8  mst_slv; //master mode:1; slave mode:0
    u8  mode; //mode0,mode1,mode2,mode3
    u8  first_bit;//LSB  MSB
    u8  rx_trig_level;
    u8  tx_trig_level;
    u8  txedge;
    u8  rxedge;
    u8  en_irq;//1:enable irq; 0 disable irq;
    u8  en_dma;// 1:dma access; 0:cpu access
    u32 timeout_cnt;//timeout count
    u8 inited;
};
int spi_init(struct spi *spi_inst);
//int spi_receive(struct spi *spi_inst, u8 *buff, int rxlen);
//int spi_transmit(struct spi *spi_inst, u8 *buff, int txlen);

int spi_master_init(struct spi *spi_inst, u8 cs, u32 baud_rate, void* base);

int spi_master_receive(struct spi *spi_inst, u8 *buff, int rxlen);

int spi_master_transmit(struct spi *spi_inst, u8 *buff, int txlen);




void spi_chip_select(struct spi *spi_inst);
void spi_chip_deselect(struct spi *spi_inst);
#if EN_SPI_SLAVE_MODE
void spi_slave_enable_rx(void);
void spi_slave_disable_rx(void);
int spi_slave_transmit(u8 *buff, int txlen);
int spi_slave_receive(u8 *buff, int rxlen);
int spi_slave_init(struct spi *spi_inst);
#endif

#endif
