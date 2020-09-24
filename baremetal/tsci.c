/*****************************************************************************
 *  File: tstc.c
 *
 *  Descirption: this file contains the SCI7816 test cases.
 *
 *  Copyright (C) : 2018 VeriSilicon.
 *
 *  Author: Chunming Li
 *  Date:   Jan 8 2019
 *
 *****************************************************************************/

#include "sci7816.h"
#include "misc.h"
#include "intc.h"
#include "gpio.h"

#define TRANSFER_TIMEOUT	1000

CKStruct_IRQHandler sci7816_irqhandler;

// global variable
static CK_UINT32 error_flag = 0;
static CK_UINT32 send_data_array[20];
static CK_UINT32 receive_data_array[20];
static CK_UINT8  tx_intr_flag;
static CK_UINT8  rx_intr_flag;
static CK_UINT8  tx_byte;
static CK_UINT8  rx_byte;

static void CK_SCI7816_ISR_Handler(u32 irq){
    CK_UINT32 reg_value;

    // check SCIINTIO1 register
    reg_value = read_mreg32(SCIINTIO1);
    printf("\n\tJJJ_DEBUG INT status=0x%x\n", reg_value);

    if(reg_value & 0x4){ // tx_int assert
        printf("\n\tJJJ_DEBUG tx_int assert\n", reg_value);
        write_mreg32(SCIINTIO1, 0x4); // clear tx_int flag
        tx_intr_flag = 0;
    }

    if(reg_value & 0x2){ // rx_int assert
        printf("\n\tJJJ_DEBUG rx_int assert\n", reg_value);
        rx_byte = read_mreg32(SCIBUFHW); // read data from buffer
        write_mreg32(SCIINTIO1, 0x2); // clear rx_int flag
        rx_intr_flag = 0;
    }
}

static void system_config() {
    CK_UINT32 reg_value;

    // PAD setting
    reg_value = read_mreg32(CK_PINMUX_Control+0x2c); // 0xf970_3000 + 0x2c
    reg_value = reg_value & 0xfffffff8; // IO select to SCI
    write_mreg32(CK_PINMUX_Control+0x2c, reg_value);

    // disable clock gating
    reg_value = read_mreg32(CK_CRM_ADDR+0x20c); // 0xF970_0000 + 0x20c
    reg_value = reg_value & 0xfdffffff; // disable clock gating
    write_mreg32(CK_CRM_ADDR+0x20c, reg_value);

    // Config GPIO4 -> SCI7816_MDVCC
    // COnfig GPIO5 -> SCI7816_OFF
    CK_Gpio_Init();
    CK_Gpio_Output(5, 1); // Disable card
    CK_Gpio_Output(4, 1); // Disable VCC

    udelay(1000);

    CK_Gpio_Output(4, 0); // Enable VCC
    CK_Gpio_Output(5, 0); // Enable card
    udelay(100);
}

void design_config(CK_UINT32 t_mode)
{
    CK_UINT32 reg_value;

    // SCIMODHW
    reg_value = read_mreg32(SCIMODHW);
    reg_value = reg_value | t_mode;
    //reg_value = reg_value & 0xfffffffb; // open-drain
    write_mreg32(SCIMODHW, reg_value);

    // SCICTRL
    reg_value = read_mreg32(SCICTRL);
    reg_value = reg_value & 0xffffff7f; // master mode
    reg_value = reg_value & 0xfffffffb; // IO PAD2 disable
    write_mreg32(SCICTRL, reg_value);

    if(t_mode == 1){
        // EDCCTRL
        reg_value = read_mreg32(EDCCTRL);
        reg_value = reg_value | 0x00000001;
        write_mreg32(EDCCTRL, reg_value);
    }

    reg_value = read_mreg32(ETUDATA);
    //reg_value = 0x10; // fixme: to speedup simulaton
    write_mreg32(ETUDATA, reg_value);
    printf("\n\tJJJ_DEBUG design_config ETUDATA=0x%x\n", reg_value);
}

CK_UINT8 receive_one_byte_by_polling(void)
{
    CK_UINT8  receive_byte = 0;
    CK_UINT32 reg_value;
    CK_UINT8 start = 0;

    // polling FIFO empty status
    do {
		reg_value = read_mreg32(SCISTAT);
		if (reg_value & 0x00000001) {
            if((reg_value & 0x00000004) == 0x00000004){
                printf("rx error happen when receive one byte\n");
                error_flag += 1;
            }else{
                receive_byte = read_mreg32(SCIBUFHW);
            }
			break;
        }
        start += 1;
        udelay(50);
	} while ((!(reg_value & 0x00000001)) && (start < TRANSFER_TIMEOUT));

    if (start >= TRANSFER_TIMEOUT) {
        printf("\n\tReceive transfer timeout, Test Fail\n");
        return receive_byte;
    }

    // udelay cycles for CWT, about 4*etudata
    udelay(1000);

    return receive_byte;
}

void send_one_byte_by_polling(CK_UINT8 send_byte)
{
    CK_UINT32 reg_value;

    while(1){
        reg_value = read_mreg32(SCISTAT);
        // check tx FIFO is not full
        if((reg_value & 0x00000010) == 0x0)
            break;

        udelay(50);
    }

    // write data into buffer
    write_mreg32(SCIBUFHW, send_byte);
}

CK_UINT32 receive_ATR(CK_UINT32 t_mode)
{
    CK_UINT32 i;
    CK_UINT8  K;
    CK_UINT8  tmp_byte;
    CK_UINT8  one_byte;
    CK_UINT32 reg_value;
    CK_UINT32 tmp_value;
    CK_UINT32 TA2_present;

    // TS
    one_byte = receive_one_byte_by_polling();
    printf("\n\tJJJ_DEBUG TS=0x%x\n", one_byte);

    // T0
    one_byte = receive_one_byte_by_polling();
    K = one_byte & 0x0f; // historical bytes
    tmp_byte = one_byte;
    printf("\n\tJJJ_DEBUG T0=0x%x, tmp_byte=0x%x\n", one_byte, tmp_byte);

    // TA1
    if((tmp_byte & 0x10) == 0x10) {
        one_byte = receive_one_byte_by_polling();
        printf("\n\tJJJ_DEBUG TA1=0x%x\n", one_byte);
    }

    // TB1
    if((tmp_byte & 0x20) == 0x20) {
        one_byte = receive_one_byte_by_polling();
        printf("\n\tJJJ_DEBUG TB1=0x%x\n", one_byte);
    }

    // TC1
    if((tmp_byte & 0x40) == 0x40) {
        one_byte = receive_one_byte_by_polling();
        printf("\n\tJJJ_DEBUG TC1=0x%x\n", one_byte);
    }

    // TD1
    if((tmp_byte & 0x80) == 0x80){
        one_byte = receive_one_byte_by_polling();
        tmp_byte = one_byte;
        t_mode = 0x00000001;
        printf("\n\tJJJ_DEBUG TD1=0x%x\n", one_byte);
    } else {
        tmp_byte = 0x00;
        t_mode = 0x00000000;
    }

    printf("\n\tJJJ_DEBUG tmp_byte=0x%x\n", tmp_byte);
    // TA2
    if((tmp_byte & 0x10) == 0x10){
        one_byte = receive_one_byte_by_polling();
        TA2_present = 1;
        printf("\n\tJJJ_DEBUG TA2=0x%x\n", one_byte);
    } else {
        TA2_present = 0;
    }

    // TB2
    if((tmp_byte & 0x20) == 0x20) {
        one_byte = receive_one_byte_by_polling();
        printf("\n\tJJJ_DEBUG TB2=0x%x\n", one_byte);
    }

    // TC2
    if((tmp_byte & 0x40) == 0x40) {
        one_byte = receive_one_byte_by_polling();
        printf("\n\tJJJ_DEBUG TC2=0x%x\n", one_byte);
    }

    // TD2
    if((tmp_byte & 0x80) == 0x80){
        one_byte = receive_one_byte_by_polling();
        tmp_byte = one_byte;
        printf("\n\tJJJ_DEBUG TD2=0x%x\n", one_byte);
    } else {
        tmp_byte = 0;
    }

    // TA3
    if((tmp_byte & 0x10) == 0x10) {
        one_byte = receive_one_byte_by_polling();
        printf("\n\tJJJ_DEBUG TA3=0x%x\n", one_byte);
    }

    // TB3
    if((tmp_byte & 0x20) == 0x20) {
        one_byte = receive_one_byte_by_polling();
        printf("\n\tJJJ_DEBUG TB3=0x%x\n", one_byte);
    }

    // TC3
    if((tmp_byte & 0x40) == 0x40) {
        one_byte = receive_one_byte_by_polling();
        printf("\n\tJJJ_DEBUG TC3=0x%x\n", one_byte);
    }

    // TD3
    if((tmp_byte & 0x80) == 0x80){
        one_byte = receive_one_byte_by_polling();
        tmp_byte = one_byte;
        printf("\n\tJJJ_DEBUG TD3=0x%x\n", one_byte);
    } else {
        tmp_byte = 0x00;
    }

    // un-support TD3 is present by now

    // historical bytes
    for(i=0; i<K; i++){
        one_byte = receive_one_byte_by_polling();
        printf("\n\tJJJ_DEBUG historical bytes %d=0x%x\n", i, one_byte);
    }

    if(t_mode == 1){
        one_byte = receive_one_byte_by_polling();

        // check TCK via EDCDATA equal to 0 or not
        reg_value = read_mreg32(EDCDATA);
        if(reg_value != 0){
            printf("receive TCK(0x%x) is wrong\n", one_byte);
            error_flag += 1;
        }
    }

    // configure convention selection based on internal dirc_sel
    reg_value = read_mreg32(SCIMODHW);
    tmp_value = (reg_value & 0x00000100) >> 7; // dirc_sel_internal
    reg_value = reg_value | tmp_value; // dirc_sel = dirc_sel_internal
    write_mreg32(SCIMODHW, reg_value);

    // dealy for some CWT
    udelay(500);

    return TA2_present;
}

void PPS_exchange(CK_UINT8 PPS_Fi, CK_UINT8 PPS_Di, CK_UINT32 t_mode)
{
    CK_UINT8  one_byte;
    CK_UINT8  tmp_byte;
    CK_UINT8  PCK;
    CK_UINT32 Fi_value;
    CK_UINT32 Di_value;
    CK_UINT32 reg_value;

    // 1. send PPS request
    // PPSS
    one_byte = 0xff;
    send_one_byte_by_polling(one_byte);
    PCK = one_byte;

    // PPS0
    one_byte = 0x00;
    if(t_mode == 0)
        one_byte = one_byte | 0x00;
    else
        one_byte = one_byte | 0x01;

    one_byte = one_byte | 0x10; // PPS1 is present, PPS2/3 is absent
    send_one_byte_by_polling(one_byte);
    PCK = PCK ^ one_byte;
    tmp_byte = one_byte;

    // PPS1
    if((tmp_byte & 0x10) == 0x10){
        one_byte = 0x00;
        one_byte = one_byte | ((PPS_Fi & 0x0f) << 4) | (PPS_Di & 0x0f);
        send_one_byte_by_polling(one_byte);
        PCK = PCK ^ one_byte;
    }

    // PPS2
    if((tmp_byte & 0x20) == 0x20){
        //JJJ_DEBUGone_byte = rand()%256;
        one_byte = 0x10;
        send_one_byte_by_polling(one_byte);
        PCK = PCK ^ one_byte;
    }

    // PPS3
    if((tmp_byte & 0x40) == 0x40){
        //JJJ_DEBUGone_byte = rand()%256;
        one_byte = 0x20;
        send_one_byte_by_polling(one_byte);
        PCK = PCK ^ one_byte;
    }

    // PCK
    send_one_byte_by_polling(PCK);

    // 2. receive PPS response
    // PPSS response
    one_byte = receive_one_byte_by_polling();

    // PPS0 response
    one_byte = receive_one_byte_by_polling();
    tmp_byte = one_byte;

    // PPS1 response
    if((tmp_byte & 0x10) == 0x10)
        one_byte = receive_one_byte_by_polling();


    // PPS2 response
    if((tmp_byte & 0x20) == 0x20)
        one_byte = receive_one_byte_by_polling();

    // PPS3 response
    if((tmp_byte & 0x40) == 0x40)
        one_byte = receive_one_byte_by_polling();


    // PCK response
    one_byte = receive_one_byte_by_polling();

    // only consider PPS response is identical to PPS request
    if(one_byte != PCK){
        printf("PCK mismatch between request and response, request PCK is 0x%x but response one is 0x%x\n", PCK, one_byte);
        error_flag += 1;
    }

    switch(PPS_Fi){
        case 0x00: Fi_value = 372;
        case 0x01: Fi_value = 372;
        case 0x02: Fi_value = 558;
        case 0x03: Fi_value = 744;
        case 0x04: Fi_value = 1116;
        case 0x05: Fi_value = 1488;
        case 0x06: Fi_value = 1860;
        case 0x09: Fi_value = 512;
        case 0x0a: Fi_value = 768;
        case 0x0b: Fi_value = 1024;
        case 0x0c: Fi_value = 1536;
        case 0x0d: Fi_value = 2048;
        default  : Fi_value = 372;
    }

    switch(PPS_Di){
        case 0x01: Di_value = 1;
        case 0x02: Di_value = 2;
        case 0x03: Di_value = 4;
        case 0x04: Di_value = 8;
        case 0x05: Di_value = 16;
        case 0x06: Di_value = 32;
        case 0x07: Di_value = 64;
        case 0x08: Di_value = 12;
        case 0x09: Di_value = 20;
        default  : Di_value = 1;
    }

    // update edudata
    //JJJ_DEBBUGreg_value = 10; // fixme: to speedup simulation
    reg_value = Fi_value/Di_value;
    write_mreg32(ETUDATA, reg_value);

    // udelay some cycle for etudata*4
    udelay(1000);
}

void data_check(CK_UINT32 data1, CK_UINT32 data2) {
    if(data1 != data2){
        printf("data compare fail, data1 0x%x != data2 0x%x\n", data1, data2);
        error_flag += 1;
    }
}

void CK_SCI7816_Test() {
    CK_UINT32 t_mode = 0;
    CK_UINT32 TA2_present;
    CK_UINT32 rand_index;
    CK_UINT8  PPS_Fi, PPS_Di;
    CK_UINT32 reg_value;

    printf("\nVSI SCI7816 T0 Transfer Interrupt Test\n");

    // system configure
    system_config();

    // configure design
    design_config(t_mode);

    // software reset
    reg_value = read_mreg32(CK_CRM_ADDR+0x81c); // 0xF970_0000 + 0x81c
    reg_value = reg_value | ~0xfdffffff; // reset release
    write_mreg32(CK_CRM_ADDR+0x81c, reg_value); // 0xF970_0000 + 0x81c

    udelay(500);

    reg_value = read_mreg32(CK_CRM_ADDR+0x81c); // 0xF970_0000 + 0x81c
    reg_value = reg_value & 0xfdffffff; // reset assert
    write_mreg32(CK_CRM_ADDR+0x81c, reg_value); // 0xF970_0000 + 0x81c
    udelay(500);

    reg_value = read_mreg32(CK_CRM_ADDR+0x81c); // 0xF970_0000 + 0x81c
    reg_value = reg_value | ~0xfdffffff; // reset release
    write_mreg32(CK_CRM_ADDR+0x81c, reg_value); // 0xF970_0000 + 0x81c
    // wait to receive ATR
    TA2_present = receive_ATR(t_mode);

    printf("\n\tJJJ_DEBUG 0x10\n");

    // PPS exchange if TA2 absent
    if(TA2_present){
        printf("\n\tJJJ_DEBUG 0x20\n");
        //JJJ_DEBUGrand_index = rand()%4;
        rand_index = 0;

        printf("\n\tJJJ_DEBUG 0x21\n");
        switch(rand_index){
            case 0 : PPS_Fi = 0x00; PPS_Di = 0x01;
            case 1 : PPS_Fi = 0x01; PPS_Di = 0x02;
            case 2 : PPS_Fi = 0x09; PPS_Di = 0x04;
            case 3 : PPS_Fi = 0x0b; PPS_Di = 0x06;
            default: PPS_Fi = 0x00; PPS_Di = 0x01;
        }

        PPS_exchange(PPS_Fi, PPS_Di, t_mode);
        printf("\n\tJJJ_DEBUG 0x22\n");
    }

    write_mreg32(SCIINTIO1, 0x7); // clear tx_int/rx_int/io_int for following interrupt test
    write_mreg32(SCIINTRST, 0x1); // clear rst_int for following interrupt test

    printf("\n\tJJJ_DEBUG 0x23\n");
    memset(&sci7816_irqhandler,0,sizeof(PCKStruct_IRQHandler));
    printf("\n\tJJJ_DEBUG 0x24\n");
    // sci7816 int handle
    sci7816_irqhandler.devname  = "SCI7816";
    sci7816_irqhandler.irqid    = CK_INTC_SCI7816;
    sci7816_irqhandler.priority = CK_INTC_SCI7816;
    sci7816_irqhandler.handler  = CK_SCI7816_ISR_Handler;
    sci7816_irqhandler.bfast    = FALSE;
    sci7816_irqhandler.next     = NULL;
    CK_INTC_RequestIrq(&sci7816_irqhandler, AUTO_MODE);
    printf("\n\tJJJ_DEBUG 0x25\n");
    //start_intr_flag = 0; // start to interrupt isr test


    // Send 1 byte data to card
    tx_byte = 0x32;
    write_mreg32(SCIBUFHW, tx_byte); // write data into buffer
    while(tx_intr_flag != 0) {
        printf("\n\tJJJ_DEBUG 0x30\n");
    }
    tx_intr_flag = 1;

    // show simulation result
    if(error_flag == 0)
        printf("\n\tVSI SCI Test PASS\n");
    else
        printf("\n\tVSI SCI Test FAIL\n");
}