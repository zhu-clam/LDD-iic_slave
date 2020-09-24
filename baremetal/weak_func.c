/*****************************************************************************
 *  File: weak_func.c
 *
 *  Descirption: contains the weak functions that may not be included in building
 *
 *  Copyright (C) : 2018 VeriSilicon.
 *
 *  Author: Lu Gao
 *  Mail:   lu.gao@verisilicon.com
 *  Date:   September 21 2018
 *
 *****************************************************************************/
#include <include/datatype.h>
//#include <include/ahbdma.h>
//void __attribute__((weak)) CK_AHBDMA_Test() {}
void __attribute__((weak)) CK_PWM_test(){};
void __attribute__((weak)) CK_Gpio_Test() {}
void __attribute__((weak)) CK_Watchdog_Test() {}
void __attribute__((weak)) CK_I2C_Test() {}
void __attribute__((weak)) CK_Rtc_Test() {}
void __attribute__((weak)) SPI_Master_GD25Q128_APP(CK_UINT8 PortN) {}
void __attribute__((weak)) SPI_Master_w25n01_App() {}
void __attribute__((weak)) apb_access_test() {}
void __attribute__((weak)) CK_SDIO_Test() {}
void __attribute__((weak)) CK_REGS_Test() {}
void __attribute__((weak)) CK_Mailbox_Test() {}
void __attribute__((weak)) CK_MIPI_Test() {}
void __attribute__((weak)) CK_PM_Test() {}
void __attribute__((weak)) CK_PM_AHB_DMA_Test() {}
void __attribute__((weak)) CK_SPI_SLAVE_Test() {}
void __attribute__((weak)) CK_CPU_L2_Test() {}
void __attribute__((weak)) CK_SUB1_Test() {}
void __attribute__((weak)) CK_SUB2_Test() {}
void __attribute__((weak)) DMAMem2PeripheralOpen(CK_UINT8 channel, CK_UINT32 src_addr,
                                    CK_UINT32 count, CK_UINT8 peripheral_ID,
                                    CK_UINT8 dma_intr, CK_UINT32 PortNum,
                                    CK_UINT16 src_gth_cnt,
                                    CK_UINT32 src_gth_intvl,
                                    CK_UINT32 dst_tr_width,
                                    CK_UINT32 dst_msize) {}
void __attribute__((weak)) DMAPeripheral2MemOpen(CK_UINT8 channel, CK_UINT32 dst_addr,
                                    CK_UINT32 count, CK_UINT8 peripheral_ID,
                                    CK_UINT8 dma_intr, CK_UINT32 PortNum,
                                    CK_UINT16 dst_sct_cnt,
                                    CK_UINT32 dst_sct_intvl) {}
//CK_UINT32 __attribute__((weak)) DMAC_CheckDone(CK_UINT32 id, CK_UINT32 channel_number, CK_UINT8 dma_intr) {}
//void __attribute__((weak)) DMAC_Open(DMAC_CH_INFO * channel, CK_UINT32 channel_number,
//                        CK_UINT16 BlockSize){}
void __attribute__((weak)) DMAC_Close(CK_UINT32 channel_number) {}
void __attribute__((weak)) DMAC_Init() {}
void __attribute__((weak)) CK_Timer_Test() {}
void __attribute__((weak)) CK_zxdmac_test() {}
void __attribute__((weak)) CK_UART_Test() {}
void __attribute__((weak)) CK_INTC_Test() {}
void __attribute__((weak)) CK_Common_Handler() {}
void __attribute__((weak)) DMAC_Start(CK_UINT32 channel_number) {}
void __attribute__((weak)) CK_STC_Test() {}
void __attribute__((weak)) CK_I2S_PTS_Test() {}
void __attribute__((weak)) CK_SCI7816_Test() {}
void __attribute__((weak)) CK_APTS_Test() {}
void __attribute__((weak)) init_time0() {}
void __attribute__((weak)) CK_AHBDMA_Test() {}
void __attribute__((weak)) CK_I2C_Slave_Test() {}
void __attribute__((weak)) CK_OTP_Test() {}
void __attribute__((weak)) CK_RSA_Test() {}
void __attribute__((weak)) CK_SPACC_Reg_RW_Test() {}
void __attribute__((weak)) CK_SPACC_Test() {}
void __attribute__((weak)) CK_nfc_test() {}
void __attribute__((weak)) CK_CPU_Boot_Test() {}
void __attribute__((weak)) CK_AXIDMA_Test() {}
void __attribute__((weak)) CK_DDR_Interleave_Test() {}
void __attribute__((weak)) Audio_Test() {}
void __attribute__((weak)) domain_power_test() {}
void __attribute__((weak)) ip_clk_gating_test() {}
void __attribute__((weak)) DDR_test(void) {}
void __attribute__((weak)) pin_mux_test(void) {}
