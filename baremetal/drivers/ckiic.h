
/*****************************************************************************
 *  File: ckiic.h
 *
 *  Descirption: contains the functions support Synopsys IIC.
 *
 *  Copyright (C) : 2018 VeriSilicon.
 *
 *  Author: Chunming Li
 *  Mail:   chunming.li@verisilicon.com
 *  Date:   Feb 12 2018
 *
 *****************************************************************************/

#ifndef __CKWDT_H__
#define __CKWDT_H__

#include "ck810.h"
#include "datatype.h"
#include "intc.h"

/*
 * define number of the timer interrupt
 */
#define  CK_I2C_IRQ0   CK_INTC_I2C0
#define  CK_I2C_IRQ1   CK_INTC_I2C1
#define  CK_I2C_IRQ2   CK_INTC_I2C2
#define  CK_I2C_IRQ3   CK_INTC_I2C3

typedef struct CKS_I2C
{
    volatile CK_REG  ic_con;
    volatile CK_REG  ic_tar;
    volatile CK_REG  ic_sar;
    volatile CK_REG  ic_hs_maddr;
    volatile CK_REG  ic_cmd_data;
    volatile CK_REG  ic_ss_scl_hcnt;
    volatile CK_REG  ic_ss_scl_lcnt;
    volatile CK_REG  ic_fs_scl_hcnt;
    volatile CK_REG  ic_fs_scl_lcnt;
    volatile CK_REG  ic_hs_scl_hcnt;
    volatile CK_REG  ic_hs_scl_lcnt;
    volatile CK_REG  ic_intr_stat;
    volatile CK_REG  ic_intr_mask;
    volatile CK_REG  ic_raw_intr_stat;
    volatile CK_REG  ic_rx_tl;
    volatile CK_REG  ic_tx_tl;
    volatile CK_REG  ic_clr_intr;
    volatile CK_REG  ic_clr_rx_under;
    volatile CK_REG  ic_clr_rx_over;
    volatile CK_REG  ic_clr_tx_over;
    volatile CK_REG  ic_clr_rd_req;
    volatile CK_REG  ic_clr_tx_abrt;
    volatile CK_REG  ic_clr_rx_done;
    volatile CK_REG  ic_clr_activity;
    volatile CK_REG  ic_clr_stop_det;
    volatile CK_REG  ic_clr_start_det;
    volatile CK_REG  ic_clr_gen_call;
    volatile CK_REG  ic_enable;
    volatile CK_REG  ic_status;
    volatile CK_REG  ic_txflr;
    volatile CK_REG  ic_rxflr;
    volatile CK_REG  ic_sda_hold;
    volatile CK_REG  ic_tx_abrt_source;
    volatile CK_REG  ic_slv_data_nack_only;
    volatile CK_REG  ic_dma_cr;
    volatile CK_REG  ic_dma_tdlr;
    volatile CK_REG  ic_dma_rdlr;
    volatile CK_REG  ic_sda_setup;
    volatile CK_REG  ic_ack_general_call;
    volatile CK_REG  ic_enable_status;
    volatile CK_REG  ic_fs_spklen;
    volatile CK_REG  ic_hs_spklen;
    volatile CK_REG  ic_clr_restart_det;
} CKStruct_I2C,* PCKPStruct_I2C;

typedef struct CK_I2C_Info_t {
        CK_UINT32 id;               /* the number of timer */
        PCKPStruct_I2C  addr;       /* the base-address of the I2C */
        CK_UINT32 irq;              /* the interrupt number of I2C */
        BOOL      bopened;          /* indicate whether have been opened or not */
        CKStruct_IRQHandler irqhandler; /* ISR */
} CKStruct_I2CInfo, * PCKStruct_I2CInfo;

#endif

