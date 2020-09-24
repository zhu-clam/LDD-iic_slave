/*
 * (Copyright 2017) Verisilicon Inc. All Rights Reserved
 * Company confidential and Proprietary information.
 * This information may not be disclosed to unauthorized individual.
 * 
 * System Application Team <fengyin.wu@verisilicon.com>
 * modify by Jianheng,Zhang@versilicon,com
 */


#include "datatype.h"
#include "platform.h"
#include "common.h"
//#include "printf.h"
#include "spi.h"
#include "string.h"
//#include "gpio.h"
/*summary information
 *
 * The transmitter/receiver FIFO is 8 bytes level FIFO
 * SPI allows 7 or 8 bits of data to be synchronously transmitted and received simultaneously.
 * All four SPI modes are available using the clock polarity ��ckpl�� and phase ��ckph�� settings.
 * The data bit order can also be set to either LSB first or MSB first.
*/


//Register and memory mapping summary table
#define     SPI_TXREG           0x00    //Transmit Data Register
#define     SPI_RXREG           0x04    //Receive Data Register
#define     SPI_CSTAT           0x08    //Current Status Register
#define     SPI_INTSTAT         0x0c    //Interrupt Status Register
#define     SPI_INTEN           0x10    //Interrupt Enable Register
#define     SPI_INTCLR          0x14    //Interrupt Clear Register
#define     SPI_GCTL            0x18    //Global Control Register
#define     SPI_CCTL            0x1c    //Common Control Register
#define     SPI_SPBRG           0x20    //Baud Rate Generator
#define     SPI_RXDNR           0x24    //Receive data number register
#define     SPI_SCSR            0x28    //Slave Chip Select register
#define     SPI_MIO_2_3_CTL     0x2c    //ctl the MIO_2/MIO_3 value for none-quad mode

/*   Register description   */
//Current Status Register (CSTAT)
#define     RXAVL_4     (1<<3) //receiver FIFO has more than 4 avaliable data
#define     TXFULL      (1<<2)  //Transmitter FIFO is full
#define     RXAVL       (1<<1)  //receiver FIFO has a complete available byte data
#define     TXEPT       (1<<0)  //transmitter FIFO and TX shift register are empty

//Interrupt Status Register (INTSTAT)

//Interrupt Enable Register (INTEN)
#define DIS_ALL_INTE    0x00

//Interrupt Clear Register (INTCLR)
#define CLEAR_ALL_INTE      0x7f

//Global Control Register (GCTL)
#define     TI_MOD      (1<<14)     //1:TI mode
#define     QUAD_MOD    (1<<13)     //1: Quad mode enable
#define     DUAL_MOD    (1<<12)     //1:Dual mode enable
#define     CSN_SEL     (1<<10)     //1: chip select signal generation by hareware
#define     DMAMODE     (1<<9)      //1: dma access mode
#define     TXTLF       (1<<7)      //01:Less than FIFO_TRIGGER_VALUE byte space in TX FIFO
#define     RXTLF       (1<5)       //01: more than FIFO_TRIGGER_VALUE byte data in RX FIFO
#define     RXEN        (1<<4)      //1: Enable receive
#define     TXEN        (1<<3)      //1: Transmit enabled
#define     MASTER      (1<<2)      //1: Master mode
#define     INT_EN      (1<<1)      //1: enable SPI interrupt
#define     SPIEN       (1<<0)      //1: SPI enable

//Common Control Register (CCTL)
#define     spilen(x)   ((x-1)<<8)  //spi length bits:0~2: Reserved;3: 4-bit mode;3: 4-bit mode;31: 32bit mode
#define     TXEDGE      (1<<5)  //1:SPBRG==4,fast speed use; Slave mode tx data transmit phase adjust bit(in slave mode, Tspi_clk>=4Tapb_clk)����ǰ�������ݷ��ͣ�,
#define     RXEDGE      (1<<4)  //1:sample the edge of the rxed data, high speed use; Master mode rx data sample edge select(�Ӻ������ڶ�ȡ����)
#define     LSBFE       (1<<2)  //1= Data is transferred or received least significant bit (LSB) first; LSB First Enable
#define     CKPL        (1<<1)  //SPI Clock Polarity Select bit;1=Active-low clocks selected. In idle state SCLK is high.
#define     CKPH        (1<<0)  //Clock Phase Select bit
//                                 Mode ckpl ckph
#define     MODE0       (0x01) //  0    0     1
#define     MODE1       (0x00) //  1    0     0
#define     MODE2       (0x03) //  2    1     1
#define     MODE3       (0x02) //  3    1     0

//Baud Rate Register (SPBRG)

//Receive Data Number Register (RXDNR)

//Slave Chip Select Register (SCSR)
#define     CSN(x)      (x)     //1: slave not selected;Chip select output signal in Master mode. Low active. In SPI slave mode, the bit is not used.

//MIO_2 and MIO_3 value in none dual mode(MIO_2_3_CTL)
#define     MIO_3_DO_ND (1<<3) //do for pin MIO_3 in non-quad mode
#define     MIO_3_OE_ND (1<<2) //oe for pin MIO_3 in non-quad mode
#define     MIO_2_DO_ND (1<<1) //do for pin MIO_2 in non-quad mode
#define     MIO_2_OE_ND (1<<0) //oe for pin MIO_2 in non-quad mode

#define     MASTER_MODE     1
#define     SLAVE_MODE      0
#define     FIFO_DEPTH      32

#define     SPI_OK          0x00
#define     SPI_TIMEOUT 0x01
#define     EN_SPI_SLAVE_MODE   0


//static struct spi spi_master_inst;
//static struct spi spi_slave_inst;
//u32 spi_slave_ip_clk,spi_slave_work_clk;
int spi_reg_access_test(struct spi *spi_inst)
{
    u32 reg = 0;
    int i = 0;
    debug("spi ip base :0x%08x \n",spi_inst->spi_base);
    debug("read all regs val----------------- \n");
    for (i = 0; i < 0x2c; i += 0x4)
    {
        reg = read_mreg32(spi_inst->spi_base + i);
        debug("reg0x%02x: 0x%04x\n", i, reg);
    }
    debug("wirte reg 0x00 to 0xaa------------ \n");
    reg = 0xaa;
    write_mreg32(spi_inst->spi_base,reg);
    reg = read_mreg32(spi_inst->spi_base);
    debug("reg 0x00 :0x%04x \n", reg);
    return 0;
}
int spi_init(struct spi *spi_inst)
{
    u32 val;

    //Baud Rate Register
    if(spi_inst->mst_slv == MASTER_MODE)
    {
        //The SPI clock can be up to pclk/2 ± (pclk is APB CLOCK) in SPI master mode
        if(spi_inst->baud_rate > (spi_inst->apb_clock/2))
            spi_inst->baud_rate = spi_inst->apb_clock/2;
    }
    else if(spi_inst->mst_slv == SLAVE_MODE)
    {
        //The SPI clock can be up to pclk/4 ± in SPI slave mode
        if(spi_inst->baud_rate > (spi_inst->apb_clock/4))
            spi_inst->baud_rate = spi_inst->apb_clock/4;
    }
    val = spi_inst->apb_clock / spi_inst->baud_rate;
    write_mreg32(spi_inst->spi_base+SPI_SPBRG,val);
    if(val > 4)
    {
        spi_inst->txedge = 0;
        spi_inst->rxedge = 0;
    }
    else
    {
        spi_inst->txedge = 1;
        spi_inst->rxedge = 1;
    }

    //Common Control Register
    val = spilen(spi_inst->spilen) | spi_inst->first_bit | spi_inst->mode | ((spi_inst->txedge&0x1)<<5) | ((spi_inst->rxedge&0x1)<<4);
    write_mreg32(spi_inst->spi_base+SPI_CCTL,val);
    
    //init SPI interrupt
    if(spi_inst->en_irq == 0)
    {
        write_mreg32(spi_inst->spi_base+SPI_INTEN, DIS_ALL_INTE);//disable all interrupt
        write_mreg32(spi_inst->spi_base+SPI_INTCLR, CLEAR_ALL_INTE);//clear all interrupt
    }
    else
    {
        debug("bootrom don't support spi interrput.\n");
//        return -EINVAL;
    }
    
    //Slave Chip Select Register
    if(spi_inst->mst_slv == MASTER_MODE)
    {
        write_mreg32(spi_inst->spi_base+SPI_SCSR,0xff); //all slave not selected
    }
    
    //Global Control Register
    if(spi_inst->mst_slv == MASTER_MODE)
    {
        //normal mode,quad/dual disable,chip select by SW,disable RX/TX,Master,disable irq,SPI enable
        val = spi_inst->rx_trig_level |spi_inst->tx_trig_level |spi_inst->en_dma | MASTER | SPIEN;
        write_mreg32(spi_inst->spi_base+SPI_GCTL,val);
    }
    else if(spi_inst->mst_slv == SLAVE_MODE)
    {
        //normal mode,quad/dual disable,chip select by SW, Enable RX/TX,Slave,disable irq,SPI enable
        val = spi_inst->rx_trig_level |spi_inst->tx_trig_level |spi_inst->en_dma | SPIEN;
        write_mreg32(spi_inst->spi_base+SPI_GCTL,val);
    }

    return 0;
}



// read Current Status Register for master;
static int check_read_status_for_master(struct spi *spi_inst)
{
    u32 count = spi_inst->timeout_cnt;

    do
    {
        if((read_mreg32(spi_inst->spi_base+SPI_CSTAT)&0x02) == 0x02)//1: receiver FIFO has a complete available byte data
            break;
        count--;
        if(count == 0)
        {
            debug("spi check_read_status timeout.\n");
            return SPI_TIMEOUT;
        }
    }while(count > 0);
    
    return SPI_OK;
}


void spi_chip_select(struct spi *spi_inst)
{
    write_mreg32(spi_inst->spi_base+SPI_SCSR,(~spi_inst->chip_sel&0xff));
}

void spi_chip_deselect(struct spi *spi_inst)
{
    write_mreg32(spi_inst->spi_base+SPI_SCSR,(spi_inst->chip_sel&0xff));
}

int spi_master_receive(struct spi *spi_inst, u8 *buff, int rxlen)
{
    u32 val,i;
    int ret =  SPI_OK;
    
    if(spi_inst->mst_slv == MASTER_MODE)
    {
        val = read_mreg32(spi_inst->spi_base+SPI_GCTL);
        val |= (RXEN|TXEN);
        write_mreg32(spi_inst->spi_base+SPI_GCTL,val); //enable rx and tx;
        while(rxlen)
        {
            if(rxlen>FIFO_DEPTH)
            {
                for(i=0; i<FIFO_DEPTH; i++)//send clock 
                {
                    write_mreg32(spi_inst->spi_base+SPI_TXREG,0);
                }
                
                for(i=0; i<FIFO_DEPTH; i++)
                {
                    if(check_read_status_for_master(spi_inst)==SPI_OK)
                    {
                        val = read_mreg32(spi_inst->spi_base+SPI_RXREG);
                        buff[i] = val & ((~0)>>(32-spi_inst->spilen));
                    }
                    else
                    {
                        ret = SPI_TIMEOUT;
                        goto END;
                    }    
                }
                
                rxlen -= FIFO_DEPTH;
                buff += FIFO_DEPTH;
            }
            else
            {
                for(i=0; i<rxlen; i++)
                {
                    write_mreg32(spi_inst->spi_base+SPI_TXREG,0);
                }
                for(i=0; i<rxlen; i++)
                {
                    if(check_read_status_for_master(spi_inst)==SPI_OK)
                    {
                        val = read_mreg32(spi_inst->spi_base+SPI_RXREG);
                        buff[i] = val & ((~0)>>(32-spi_inst->spilen));
                    }
                    else
                    {
                        ret = SPI_TIMEOUT;
                        goto END;
                    }    
                }
                rxlen -= rxlen;
                buff += rxlen;
            }
        }
    }
    else
    {
        debug("spi read paramter error.\n");
        return -EINVAL;
    }
    
END:    
    val = read_mreg32(spi_inst->spi_base+SPI_GCTL);
    val &= ~(RXEN|TXEN);
    write_mreg32(spi_inst->spi_base+SPI_GCTL,val); //disable rx and tx;
    
    return ret; 
}

int spi_master_transmit(struct spi *spi_inst, u8 *buff, int txlen)
{
    u32 count = spi_inst->timeout_cnt;
    u32 val;
    int ret = SPI_OK;
    
    if(spi_inst->mst_slv == MASTER_MODE)
    {
        val = read_mreg32(spi_inst->spi_base+SPI_GCTL);
        val |= (RXEN|TXEN);
        write_mreg32(spi_inst->spi_base+SPI_GCTL,val); //enable rx and tx;
        
        while(txlen)
        {
            do
            {
                if((read_mreg32(spi_inst->spi_base+SPI_CSTAT)&0x4) == 0)//0:Transmitter FIFO is not full
                    break;
                count--;
                if(count == 0)
                {
                    debug("spi write timeout.\n");
                    ret = SPI_TIMEOUT;
                    goto END;
                }
            }while(count > 0);

            val = *buff & ((~0)>>(32-spi_inst->spilen));
            write_mreg32(spi_inst->spi_base+SPI_TXREG, val);
            buff++;
            txlen--;
        }
    }
    else
    {
        debug("spi write paramter error.\n");
        return -EINVAL;
    }
    
END:        
    while(1) //wait transmit finish
    {
        val = read_mreg32(spi_inst->spi_base+SPI_CSTAT);
        if(val&0x01)
            break;
    }
    val = read_mreg32(spi_inst->spi_base+SPI_GCTL);
    val &= ~(TXEN | RXEN);
    write_mreg32(spi_inst->spi_base+SPI_GCTL,val); //disable rx and tx;
    
    return ret; 
}

int spi_master_init(struct spi *spi_inst, u8 cs, u32 baud_rate, void* base)
{

    if(spi_inst->inited == 0x55)
        return 0;
    memset(spi_inst,0x0,sizeof(struct spi));

    spi_inst->apb_clock = APB_DEFAULT_FREQ;
    spi_inst->baud_rate = baud_rate; // be up to apb_clock/2
    spi_inst->chip_sel = cs; // 1 -- 8
    spi_inst->en_dma = 0;
    spi_inst->en_irq = 0;
    spi_inst->mode = MODE0;
    spi_inst->mst_slv = MASTER_MODE;
    spi_inst->rx_trig_level = 1;
    spi_inst->tx_trig_level = 1;
    spi_inst->spilen = 8;
    spi_inst->spi_base = base;
    spi_inst->timeout_cnt = 1000000;
    spi_init(spi_inst);
    spi_inst->inited = 0x55;

    return 0;
}

#if EN_SPI_SLAVE_MODE
// read Current Status Register for slave;
static int check_read_status_for_slave(struct spi *spi_inst)
{
    do
    {
        if((read_mreg32(spi_inst->spi_base+SPI_CSTAT)&0x02) == 0x02)//1: receiver FIFO has a complete available byte data
            break;
    }while(1);

    return SPI_OK;
}


int spi_slave_init(void)
{
    if(spi_slave_inst.inited == 0x55)
        return 0;

    memset(&spi_slave_inst,0x0,sizeof(spi_slave_inst));

    spi_slave_inst.apb_clock = spi_slave_ip_clk;
    spi_slave_inst.baud_rate = spi_slave_work_clk; // be up to apb_clock/2
    spi_slave_inst.en_dma = 0;
    spi_slave_inst.en_irq = 0;
    spi_slave_inst.mode = MODE0;
    spi_slave_inst.mst_slv = SLAVE_MODE;
    spi_slave_inst.rx_trig_level = 0;
    spi_slave_inst.tx_trig_level = 0;
    spi_slave_inst.spilen = 8;
    spi_slave_inst.spi_base = SPI_SLAVE_BASE;
    spi_slave_inst.timeout_cnt = 0xffffffff;
    spi_init(&spi_slave_inst);
    spi_slave_inst.inited = 0x55;

    return 0;
}

void spi_slave_enable_rx(void)
{
    u32 val;
    val = read_mreg32(spi_slave_inst.spi_base+SPI_GCTL);
    val |= (RXEN);
    write_mreg32(spi_slave_inst.spi_base+SPI_GCTL,val); //enable rx;
}
void spi_slave_disable_rx(void)
{
    u32 val;
    val = read_mreg32(spi_slave_inst.spi_base+SPI_GCTL);
    val &= ~(RXEN);
    write_mreg32(spi_slave_inst.spi_base+SPI_GCTL,val); //disable rx;
}
#endif
#if EN_SPI_SLAVE_MODE
int spi_slave_transmit(u8 *buff, int txlen)//txlen must less than fifo length,need host clock
{
    u32 count = spi_slave_inst.timeout_cnt;
    u32 val,i;
    int ret = SPI_OK;
    
     if(spi_slave_inst.mst_slv == SLAVE_MODE)
    {
        if(txlen > FIFO_DEPTH)
        {
            debug("txlen is greater than FIFO_DEPTH as slave.\n");
            return -EINVAL;
        }
        val = read_mreg32(spi_slave_inst.spi_base+SPI_GCTL);
        val |= (TXEN);
        write_mreg32(spi_slave_inst.spi_base+SPI_GCTL,val); //enable rx and tx;
//        while(txlen)
        {
            do
            {
                if((read_mreg32(spi_slave_inst.spi_base+SPI_CSTAT)&0x1) == 0x1)// 1 = transmitter FIFO and TX shift register are empty
                    break;
            //    count--;
            //    if(count == 0)
            //    {
            //        debug("spi write timeout.\n");
            //        ret = SPI_TIMEOUT;
            //        goto END;
            //    }
            }while(1/*count!=0*/);
            for(i=0;i<txlen;i++)
            {
                val = *buff & (~0)>>(32-spi_slave_inst.spilen);
                write_mreg32(spi_slave_inst.spi_base+SPI_TXREG, val);
                buff++;
            }    
        }
    }
    else
    {
        debug("spi write paramter error.\n");
        return -EINVAL;
    }
    
END:
    //gpio trigger host
    //gpio_set_data(40, 0);
    while(1) //wait transmit finish
    {
        val = read_mreg32(spi_slave_inst.spi_base+SPI_CSTAT);
        if(val&0x01)
            break;
    }
    //gpio_set_data(40, 1);
    val = read_mreg32(spi_slave_inst.spi_base+SPI_GCTL);
    val &= ~(TXEN);
    write_mreg32(spi_slave_inst.spi_base+SPI_GCTL,val); //disable tx;
    
    return ret; 
}
#endif
#if EN_SPI_SLAVE_MODE
int spi_slave_receive(u8 *buff, int rxlen)
{
    u32 val,i;
    int ret =  SPI_OK;

    if(spi_slave_inst.mst_slv == SLAVE_MODE)
    {
//        val = read_mreg32(spi_inst->spi_base+SPI_GCTL);
//        val |= (RXEN);
//        write_mreg32(spi_inst->spi_base+SPI_GCTL,val); //enable rx;
        i = 0;
        while(rxlen)
        {
            if(check_read_status_for_slave(&spi_slave_inst)==SPI_OK)
            {
                val = read_mreg32(spi_slave_inst.spi_base+SPI_RXREG);
                buff[i] = val & ((~0)>>(32-spi_slave_inst.spilen));
            }
            else
            {
                ret = SPI_TIMEOUT;
                val = read_mreg32(spi_slave_inst.spi_base+SPI_GCTL);
                val &= ~(RXEN|TXEN);
                write_mreg32(spi_slave_inst.spi_base+SPI_GCTL,val); //disable rx and tx;
                return ret;
            }

            rxlen--;
            i++;
        }
    }
    else
    {
        debug("spi read paramter error.\n");
        return -EINVAL;
    }

//END:
//    val = read_mreg32(spi_slave_inst.spi_base+SPI_GCTL);
//    val &= ~(RXEN|TXEN);
//    write_mreg32(spi_slave_inst.spi_base+SPI_GCTL,val); //disable rx and tx;

    return ret;
}
#endif

