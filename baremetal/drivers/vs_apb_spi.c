/*
 * VeriSilicon SPI Controller Driver
 *
 * Copyright (C) 2016 - 2018 VeriSilicon, Inc.
 *
 * Base on vsi_spi.c in linux.
 * 
 */

#include "ck810.h"
#include "datatype.h"
#include "intc.h"
#include "vs_apb_spi.h"
#include "misc.h"

#define DRV_VERSION "1.1.0"
#define DRIVER_NAME "vsi-spi"

static volatile unsigned int *spi_iobase_list[MAX_SPI_NUM_ON_CHIP] = {
    SPI0_BASE, 
    SPI1_BASE
};

/*******************************************************************
 Desc: register offset
 *******************************************************************/
#define SPI_TXREG               (0x00)
#define SPI_RXREG               (0x04)
#define SPI_CSTAT               (0x08)
#define SPI_INTSTAT             (0x0C)
#define SPI_INTEN               (0x10)
#define SPI_INTCLR              (0x14)
#define SPI_GCTL                (0x18)
#define SPI_CCTL                (0x1C)
#define SPI_SPBRG               (0x20)
#define SPI_RXDNR               (0x24)
#define SPI_SCSR                (0x28)
#define SPI_MIO_2_3_CTL         (0x2c)

/*******************************************************************
 Desc: address after ioremap
 *******************************************************************/
#define VSI_SPI_TXREG        ((volatile vsi_spi_txreg*)        (rs->regs+0x00))
#define VSI_SPI_RXREG        ((volatile vsi_spi_rxreg*)        (rs->regs+0x04))
#define VSI_SPI_CSTAT        ((volatile vsi_spi_cstat*)        (rs->regs+0x08))
#define VSI_SPI_INTSTAT      ((volatile vsi_spi_intstat*)      (rs->regs+0x0c))
#define VSI_SPI_INTEN        ((volatile vsi_spi_inten*)        (rs->regs+0x10))
#define VSI_SPI_INTCLR       ((volatile vsi_spi_intclr*)       (rs->regs+0x14))
#define VSI_SPI_GCTL         ((volatile vsi_spi_gctl*)         (rs->regs+0x18))
#define VSI_SPI_CCTL         ((volatile vsi_spi_cctl*)         (rs->regs+0x1c))
#define VSI_SPI_SPBRG        ((volatile vsi_spi_spbrg*)        (rs->regs+0x20))
#define VSI_SPI_RXDNR        ((volatile vsi_spi_rxdnr*)        (rs->regs+0x24))
#define VSI_SPI_SCSR         ((volatile vsi_spi_scsr*)         (rs->regs+0x28))
#define VSI_SPI_MIO_2_3_CTL  ((volatile vsi_spi_mio_2_3_ctl*)  (rs->regs+0x2c))

/*******************************************************************
 Desc: bit definitions of all of registers
 *******************************************************************/
#define SPI_ENABLE               1
#define SPI_DISABLE              0
#define SPI_INTERRUPT_ENABLE     1
#define SPI_INTERRUPT_DISABLE    0
#define MASTER_MODE              1
#define SLAVE_MODE               0
#define TX_ENABLE                1
#define TX_DISABLE               0
#define RX_ENABLE                1
#define RX_DISABLE               0
#define RX_TRIG_LEVEL_V          1
#define RX_TRIG_LEVEL_0          0
#define TX_TRIG_LEVEL_V          1
#define TX_TRIG_LEVEL_0          0
#define DMA_ACCESS_MODE          1
#define NORMAL_ACCESS_MODE       0
#define CS_HARDWARE              1
#define CS_SOFTWARE              0
#define DUAL_MODE_EABLE          1
#define DUAL_MODE_DISABLE        0
#define QUAD_MODE_EABLE          1
#define QUAD_MODE_DISABLE        0
#define TI_MODE_ENABLE           1
#define TI_MODE_DISABLE          0

#define CLK_PHASE_A              0
#define CLK_PHASE_B              1
#define CLK_LEVEL_L              0
#define CLK_LEVEL_H              1
#define MSB_FIRST                0
#define LSB_FIRST                1
#define RX_SAMPLE_MIDDLE         0
#define RX_SAMPLE_EDGE           1
#define TX_SLAVE_SPEED_LOW       0
#define TX_SLAVE_SPEED_FAST      1
#define SPI_LENGTH_32           31
#define SPI_LENGTH_16           15
#define SPI_LENGTH_8             7

#define INT_EN_TX                1
#define INT_DIS_TX               0
#define INT_EN_RX                1
#define INT_DIS_RX               0
#define INT_EN_UNDERRUN          1
#define INT_DIS_UNDERRUN         0
#define INT_EN_OVERRUN           1
#define INT_DIS_OVERRUN          0
#define INT_EN_RXMATCH           1
#define INT_DIS_RXMATCH          0
#define INT_EN_RX_FIFO_FULL      1
#define INT_DIS_RX_FIFO_FULL     0
#define INT_EN_TX_EMPTY          1
#define INT_DIS_TX_EMPTY         0
#define INT_EN_ALL               0x7f
#define INT_DIS_ALL              0

#define INT_CLR_TX               1
#define INT_CLR_NOT_TX           0
#define INT_CLR_RX               1
#define INT_CLR_NOT_RX           0
#define INT_CLR_UNDERRUN         1
#define INT_CLR_NOT_UNDERRUN     0
#define INT_CLR_OVERRUN          1
#define INT_CLR_NOT_OVERRUN      0
#define INT_CLR_RXMATCH          1
#define INT_CLR_NOT_RXMATCH      0
#define INT_CLR_RX_FIFO_FULL     1
#define INT_CLR_NOT_RX_FIFO_FULL 0
#define INT_CLR_TX_EMPTY         1
#define INT_CLR_NOT_TX_EMPTY     0
#define INT_CLR_ALL              0x7F

#define TX_FIFO_AVAILABLE        1
#define TX_FIFO_NOT_AVAILABLE    0
#define RX_FIFO_AVAILABLE        1
#define RX_FIFO_NOT_AVAILABLE    0
#define UNDERRUN_ERR             1
#define NO_UNDERRUN_ERR          0
#define OVERRUN_ERR              1
#define NO_OVERRUN_ERR           0
#define MATCH_RXDNR              1
#define NO_MATCH_RXDNR           0
#define RX_FIFO_FULL             1
#define RX_FIFO_NOT_FULL         0
#define TX_FIFO_EMPTY            1
#define TX_FIFO_NOT_EMPTY        0

#define TX_EMPTY                 1
#define TX_NOT_EMPTY             0
#define RX_AVAILABLE             1
#define RX_NOT_AVAILABLE         0
#define TX_FIFO_FULL             1
#define TX_FIFO_NOT_FULL         0
#define RX_FIFO_MORE_THAN_4      1
#define RX_FIFO_LESS_THAN_4      0

#define CSn_ACTIVE(n)           ((0xff) & (~(1 << n)))
#define CSX_RELEASE             (0xff)

#define MAX_SPI_RX_LEN          (64 * 1024)


#define vsi_spi_writel(v,c)     write_mreg32(c,v)
#define vsi_spi_readl(c)        read_mreg32(c)

typedef union {
    u32 all;
    struct {
        u32 txreg;
    } bit;
} vsi_spi_txreg;

typedef union {
    u32 all;
    struct {
        u32 rxreg;
    } bit;
} vsi_spi_rxreg;

typedef union {
    u32 all;
    struct {
        unsigned txept:1;
        unsigned rxavl:1;
        unsigned txfull:1;
        unsigned rxavl_4:1;
        unsigned reserved4:4;
        u8  reserved8;
        u16 reserved16;
    } bit;
} vsi_spi_cstat;

typedef union {
    u32 all;
    struct {
        unsigned tx_intf:1;
        unsigned rx_intf:1;
        unsigned underrun_intf:1;
        unsigned rxoerr_intf:1;
        unsigned rxmatch_intf:1;
        unsigned rxfifo_full_intf:1;
        unsigned txept_intf:1;
        unsigned :1;
        u8  reserved8;
        u16 reserved16;
    } bit;
} vsi_spi_intstat;

typedef union {
    u32 all;
    struct {
        unsigned txien:1;
        unsigned rxien:1;
        unsigned underrunen:1;
        unsigned rxoerren:1;
        unsigned rxmatchen:1;
        unsigned rxfifo_full_ien:1;
        unsigned txept_ien:1;
        unsigned :1;
        u8  reserved8;
        u16 reserved16;
    } bit;
} vsi_spi_inten;

typedef union {
    u32 all;
    struct {
        unsigned txiclr:1;
        unsigned rxiclr:1;
        unsigned underrunclr:1;
        unsigned rxoerrclr:1;
        unsigned rxmatchclr:1;
        unsigned rxfifo_full_iclr:1;
        unsigned txept_iclr:1;
        unsigned :1;
        u8  reserved8;
        u16 reserved16;
    } bit;
} vsi_spi_intclr;

typedef union {
    u32 all;
    struct {
        unsigned spien:1;
        unsigned int_en:1;
        unsigned mm:1;
        unsigned txen:1;
        unsigned rxen:1;
        unsigned rxtlf:2;
        unsigned txtlf0:1;
        unsigned txtlf1:1;
        unsigned dmamode:1;
        unsigned csn_sel:1;
        unsigned :1;
        unsigned dual_mod:1;
        unsigned quad_mod:1;
        unsigned ti_mod:1;
        unsigned :1;
        u16 reserved16;
    } bit;
} vsi_spi_gctl;

typedef union {
    u32 all;
    struct {
        unsigned ckph:1;
        unsigned ckpl:1;
        unsigned lsbfe:1;
        unsigned :1;
        unsigned rxedge:1;
        unsigned txedge:1;
        unsigned :2;
        unsigned spilen:5;
        unsigned :3;
        u16 reserved16;
    } bit;
} vsi_spi_cctl;

typedef union {
    u32 all;
    struct {
        u16 spbrg;
        u16 reserved16;
    } bit;
} vsi_spi_spbrg;

typedef union {
    u32 all;
    struct {
        u16 rxdnr;
        u16 reserved16;
    } bit;
} vsi_spi_rxdnr;

typedef union {
    u32 all;
    struct {
        u8 csn;
        u8 reserved8;
        u16 reserved16;
    } bit;
} vsi_spi_scsr;

typedef union {
    u32 all;
    struct {
        unsigned mio_2_oe_nd:1;
        unsigned mio_2_do_nd:1;
        unsigned mio_3_oe_nd:1;
        unsigned mio_3_do_nd:1;
        unsigned :4;
        u8 reserved8;
        u16 reserved16;
    } bit;
} vsi_spi_mio_2_3_ctl;


void vsi_spi_set_cs(struct vsi_spi *rs, bool enable)
{
    vsi_spi_scsr scsr;

    scsr.all = vsi_spi_readl(VSI_SPI_SCSR);

    if (enable) {
        scsr.bit.csn = CSn_ACTIVE(0);
        vsi_spi_writel(scsr.all, VSI_SPI_SCSR);
    } else {
        scsr.bit.csn = CSX_RELEASE;
        vsi_spi_writel(scsr.all, VSI_SPI_SCSR);
    }
}

static void vsi_spi_hwinit(struct vsi_spi *rs)
{
    vsi_spi_gctl gctl;
    vsi_spi_cctl cctl;
    vsi_spi_inten inten;
    vsi_spi_intclr intclr;
    vsi_spi_rxdnr rxdnr;
    vsi_spi_scsr scsr;
    vsi_spi_spbrg spbrg;

    gctl.all = vsi_spi_readl(VSI_SPI_GCTL);
    gctl.bit.spien = SPI_ENABLE;
    gctl.bit.int_en = SPI_INTERRUPT_DISABLE;
    gctl.bit.mm = rs->mode;
    gctl.bit.txen = TX_DISABLE;
    gctl.bit.rxen = RX_DISABLE;
    gctl.bit.rxtlf = RX_TRIG_LEVEL_V;
    gctl.bit.txtlf0 = TX_TRIG_LEVEL_V;
    gctl.bit.csn_sel = CS_SOFTWARE;
    gctl.bit.dmamode = NORMAL_ACCESS_MODE;
    gctl.bit.dual_mod = DUAL_MODE_DISABLE;
    gctl.bit.quad_mod = QUAD_MODE_DISABLE;
    gctl.bit.ti_mod = TI_MODE_DISABLE;
    vsi_spi_writel(gctl.all, VSI_SPI_GCTL);

    cctl.all = vsi_spi_readl(VSI_SPI_CCTL);
    switch (rs->clock_phase_mode) {
    case SPI_MODE_0:
        cctl.bit.ckph = CLK_PHASE_B; // 1
        cctl.bit.ckpl = CLK_LEVEL_L; // 0
        break;
    //case SPI_MODE_1:
    //    cctl.bit.ckph = CLK_PHASE_A; // 0
    //    cctl.bit.ckpl = CLK_LEVEL_L; // 0
    //    break;
    //case SPI_MODE_2:
    //    cctl.bit.ckph = CLK_PHASE_B; // 1
    //    cctl.bit.ckpl = CLK_LEVEL_H; // 1
    //    break;
    case SPI_MODE_3:
        cctl.bit.ckph = CLK_PHASE_A; // 0
        cctl.bit.ckpl = CLK_LEVEL_H; // 1
        break;
    default:
        printf("unsupported spi mode, set to mode3\n");
        cctl.bit.ckph = CLK_PHASE_A;
        cctl.bit.ckpl = CLK_LEVEL_H;
        break;
    }

    cctl.bit.lsbfe = MSB_FIRST;
    cctl.bit.rxedge = RX_SAMPLE_MIDDLE;
    cctl.bit.txedge = TX_SLAVE_SPEED_LOW;
    cctl.bit.spilen = SPI_LENGTH_8;
    vsi_spi_writel(cctl.all, VSI_SPI_CCTL);

    inten.all = INT_DIS_ALL;
    vsi_spi_writel(inten.all, VSI_SPI_INTEN);

    intclr.all = INT_CLR_ALL;
    vsi_spi_writel(intclr.all, VSI_SPI_INTCLR);

    rxdnr.all = 0;
    rxdnr.bit.rxdnr = 1;
    vsi_spi_writel(rxdnr.all, VSI_SPI_RXDNR);

    scsr.all = 0xff;
    scsr.bit.csn = CSX_RELEASE;
    vsi_spi_writel(scsr.all, VSI_SPI_SCSR);

    spbrg.bit.spbrg = 30;
    vsi_spi_writel(spbrg.all, VSI_SPI_SPBRG);
}

/*******************************************************************
 Desc: config hardware again before transfer
 *******************************************************************/
static void vsi_spi_configs(struct vsi_spi *rs)
{
    vsi_spi_gctl gctl;
    vsi_spi_cctl cctl;
    vsi_spi_inten inten;
    vsi_spi_intclr intclr;
    vsi_spi_spbrg spbrg;
    vsi_spi_rxdnr rxdnr;

    gctl.all = vsi_spi_readl(VSI_SPI_GCTL);
    gctl.bit.spien = SPI_ENABLE;
    gctl.bit.int_en = SPI_INTERRUPT_DISABLE;
    gctl.bit.mm = rs->mode;


    gctl.bit.rxtlf = RX_TRIG_LEVEL_0;
    gctl.bit.txtlf0 = TX_TRIG_LEVEL_0;
    gctl.bit.dmamode = NORMAL_ACCESS_MODE;

    gctl.bit.csn_sel = CS_SOFTWARE;


    if (rs->n_bits == SPI_NBITS_SINGLE) {
        gctl.bit.dual_mod = DUAL_MODE_DISABLE;
        gctl.bit.quad_mod = QUAD_MODE_DISABLE;
    } else if (rs->n_bits == SPI_NBITS_DUAL) {
        gctl.bit.dual_mod = DUAL_MODE_EABLE;
        gctl.bit.quad_mod = QUAD_MODE_DISABLE;
    } else if (rs->n_bits == SPI_NBITS_QUAD) {
        gctl.bit.dual_mod = DUAL_MODE_DISABLE;
        gctl.bit.quad_mod = QUAD_MODE_EABLE;
    } else {
        printf("wrong mode, use default single mode.\n");
        gctl.bit.dual_mod = DUAL_MODE_DISABLE;
        gctl.bit.quad_mod = QUAD_MODE_DISABLE;
    }

    gctl.bit.ti_mod = TI_MODE_DISABLE;
    vsi_spi_writel(gctl.all, VSI_SPI_GCTL);

    cctl.all = vsi_spi_readl(VSI_SPI_CCTL);
    switch (rs->clock_phase_mode) {
    case SPI_MODE_0:
        cctl.bit.ckph = CLK_PHASE_B; // 1
        cctl.bit.ckpl = CLK_LEVEL_L; // 0
        break;
    // case SPI_MODE_1:
        // cctl.bit.ckph = CLK_PHASE_A; // 0
        // cctl.bit.ckpl = CLK_LEVEL_L; // 0
        // break;
    // case SPI_MODE_2:
        // cctl.bit.ckph = CLK_PHASE_B; // 1
        // cctl.bit.ckpl = CLK_LEVEL_H; // 1
        // break;
    case SPI_MODE_3:
        cctl.bit.ckph = CLK_PHASE_A; // 0
        cctl.bit.ckpl = CLK_LEVEL_H; // 1
        break;
    default:
        printf("unsupported spi mode, set to mode3\n");
        cctl.bit.ckph = CLK_PHASE_A;
        cctl.bit.ckpl = CLK_LEVEL_H;
        break;
    }
    cctl.bit.lsbfe = MSB_FIRST;
    cctl.bit.rxedge = RX_SAMPLE_MIDDLE;
    cctl.bit.txedge = TX_SLAVE_SPEED_LOW;

    if (rs->bpw == 8)
        cctl.bit.spilen = SPI_LENGTH_8;
    else if (rs->bpw == 16)
        cctl.bit.spilen = SPI_LENGTH_16;
    else if(rs->bpw == 32)
        cctl.bit.spilen = SPI_LENGTH_32;
    else
        printf("%d is a wrong bit per word\n", rs->bpw);

    vsi_spi_writel(cctl.all, VSI_SPI_CCTL);

    inten.all = INT_DIS_ALL;
    vsi_spi_writel(inten.all, VSI_SPI_INTEN);

    intclr.all = INT_CLR_ALL;
    vsi_spi_writel(intclr.all, VSI_SPI_INTCLR);

    rxdnr.all = 0;
    rxdnr.bit.rxdnr = 1;
    vsi_spi_writel(rxdnr.all, VSI_SPI_RXDNR);

    spbrg.bit.spbrg = rs->clk_freq / rs->speed;
    if (spbrg.bit.spbrg < 2) {
        spbrg.bit.spbrg = 2;
        printf("invalid transfer speed %d!\n", rs->speed);
    }
    vsi_spi_writel(spbrg.all, VSI_SPI_SPBRG);
}

static void inline vsi_spi_hw_clr(struct vsi_spi *rs)
{
    vsi_spi_gctl gctl;
    vsi_spi_cstat cstat;
    vsi_spi_rxdnr rxdnr;

    if (rs->rx) {
        gctl.all = vsi_spi_readl(VSI_SPI_GCTL);
        // Will clr fifo.
        gctl.bit.rxen = RX_DISABLE;
        vsi_spi_writel(gctl.all, VSI_SPI_GCTL);

        rxdnr.all = 0;
        rxdnr.bit.rxdnr = 1;
        vsi_spi_writel(rxdnr.all, VSI_SPI_RXDNR);
    }
    if(rs->tx) {
        do{
            cstat.all = vsi_spi_readl(VSI_SPI_CSTAT);
        } while (cstat.bit.txept == TX_NOT_EMPTY);

        gctl.all = vsi_spi_readl(VSI_SPI_GCTL);
        gctl.bit.txen = TX_DISABLE;
        vsi_spi_writel(gctl.all, VSI_SPI_GCTL);
    }
}

static int inline vsi_spi_hw_en(struct vsi_spi *rs)
{
    vsi_spi_gctl gctl;
    vsi_spi_cstat cstat;
    vsi_spi_rxdnr rxdnr;
    vsi_spi_intclr intclr;

    if (rs->rx) {
        // Desc: Setup read_data_len
        rxdnr.all = 0;
        rxdnr.bit.rxdnr = rs->rx_len;
        vsi_spi_writel(rxdnr.all, VSI_SPI_RXDNR);

        // Desc: Enable rx
        gctl.all = vsi_spi_readl(VSI_SPI_GCTL);
        gctl.bit.rxen = RX_ENABLE;
        vsi_spi_writel(gctl.all, VSI_SPI_GCTL);
    }

    if (rs->tx) {
        // Desc: Clear tx fifo
        cstat.all = vsi_spi_readl(VSI_SPI_CSTAT);
        if (cstat.bit.txept == TX_FIFO_NOT_EMPTY) {
            printf("tx fifo isn't empty\n");
            return -1;
        }

        // Desc: Clear tx interrupt
        intclr.all = vsi_spi_readl(VSI_SPI_INTCLR);
        intclr.bit.txiclr = INT_CLR_TX_EMPTY;
        vsi_spi_writel(intclr.all, VSI_SPI_INTCLR);

        // Desc: Enable tx
        gctl.all = vsi_spi_readl(VSI_SPI_GCTL);
        gctl.bit.txen = TX_ENABLE;
        vsi_spi_writel(gctl.all, VSI_SPI_GCTL);
    }
    return 0;
}
#define min_t(a, b)  (a) > (b) ? (b) : (a)
    
static int vsi_spi_pio_transfer(struct vsi_spi *rs)
{
    vsi_spi_cstat cstat;
    vsi_spi_txreg txreg;
    vsi_spi_rxdnr rxdnr;
    vsi_spi_rxreg rxreg;

    if (!rs->tx && rs->rx) {
        while (rs->xfer_len > 0) {
            if (rs->xfer_len < MAX_SPI_RX_LEN) /* 64K */
                rs->rx_len = rs->xfer_len;
            else
                rs->rx_len = 0xffff;

            rs->xfer_len -= rs->rx_len;
            vsi_spi_hw_en(rs);

            while (rs->rx_len) {
                do {
                    cstat.all = vsi_spi_readl(VSI_SPI_CSTAT);
                } while (cstat.bit.rxavl == RX_NOT_AVAILABLE);

                rxreg.all = vsi_spi_readl(VSI_SPI_RXREG);
                // Note: rx_len must be integer multiple of rs->bpw!!!
                if ((rs->bpw == 16)) {
                    *(u16*)rs->rx = (u16)rxreg.bit.rxreg;
                    rs->rx += 2;
                    rs->rx_len -= min_t(rs->rx_len, 2);
                } else if ((rs->bpw == 32)) {
                    *(u32*)rs->rx = rxreg.bit.rxreg;
                    rs->rx += 4;
                    rs->rx_len -= min_t(rs->rx_len, 4);
                } else {
                    *(u8*)rs->rx = (u8)rxreg.bit.rxreg;
                    rs->rx += 1;
                    rs->rx_len -= 1;
                }
            }
            vsi_spi_hw_clr(rs);
        }

        rxdnr.all = 0;
        rxdnr.bit.rxdnr = 1;
        vsi_spi_writel(rxdnr.all, VSI_SPI_RXDNR);
        return 0;
    }

    if (rs->tx && !rs->rx) {
        if(vsi_spi_hw_en(rs))
            return -1;

        // Desc: send data
        // Note: tx_len must be integer multiple of rs->bpw!!!
        rs->tx_len = rs->xfer_len;
        while (rs->tx_len) {
            do {
                cstat.all = vsi_spi_readl(VSI_SPI_CSTAT);
            } while (cstat.bit.txfull == TX_FIFO_FULL);

            if ((rs->bpw == 16)) {
                txreg.bit.txreg = *(u16*)(rs->tx);
                rs->tx_len -= min_t(rs->tx_len, 2);
                rs->tx += 2;
            } else if ((rs->bpw == 32)) {
                txreg.bit.txreg = *(u32*)(rs->tx);
                rs->tx_len -= min_t(rs->tx_len, 4);
                rs->tx += 4;
            } else {
                txreg.bit.txreg = *(u8*)(rs->tx);
                rs->tx_len--;
                rs->tx++;
            }
            vsi_spi_writel(txreg.bit.txreg, VSI_SPI_TXREG);
        }

        vsi_spi_hw_clr(rs);
        return 0;
    }
    return 0;
}
#undef min_t
/*******************************************************************
 Desc: spi transfer function
 *******************************************************************/
int vsi_spi_transfer_one(struct vsi_spi *rs,
                                struct spi_transfer *xfer)
{
    rs->speed = xfer->speed_hz;
    rs->bpw = xfer->bits_per_word;
    rs->n_bytes = rs->bpw >> 3;

    rs->tx = xfer->tx_buf;
    rs->rx = xfer->rx_buf;
    rs->xfer_len = xfer->len;

    // Support quad mode
    if (rs->tx) {
        rs->tx_len = rs->xfer_len;
        rs->n_bits = xfer->tx_nbits;
    }

    if (rs->rx) {
        rs->rx_len = rs->xfer_len;
        rs->n_bits = xfer->rx_nbits;
    }

    if ((rs->n_bits != SPI_NBITS_SINGLE) &&
        (rs->n_bits != SPI_NBITS_DUAL) &&
        (rs->n_bits != SPI_NBITS_QUAD))
        rs->n_bits   = SPI_NBITS_SINGLE;

    vsi_spi_configs(rs);

    return vsi_spi_pio_transfer(rs);

}

struct vsi_spi spi_device[MAX_SPI_NUM_ON_CHIP];

struct vsi_spi *vsi_spi_probe(unsigned int id)
{
    struct vsi_spi *rs;

    if(id >= ARRAY_SIZE(spi_iobase_list)) {
        printf("Invalid spi index - %d\n", id);
        return NULL;
    }

    if(spi_device[id].busy) {
        return &spi_device[id];
    }
    spi_device[id].busy = 1;
    rs = &spi_device[id];

    rs->regs = spi_iobase_list[id];
    rs->clock_phase_mode = SPI_MODE_3;
    rs->fifo_depth = 32;
    rs->clk_freq = 30000000;
    rs->mode = MASTER_MODE;
    //rs->mode = SLAVE_MODE;

    vsi_spi_hwinit(rs);

    printf("SPI.0x%x %s driver probed, version %s\n", rs->regs,
             rs->mode == MASTER_MODE ? "master" : "slave", DRV_VERSION);

    return rs;
}


