#ifndef ___DDR_TEST_TASK_H___
#define ___DDR_TEST_TASK_H___

#define RUN_DDR_IN_SPL_ENV      1

#if RUN_DDR_IN_SPL_ENV
// In Baremetal env
#include "datatype.h"
#include "misc.h"
#include "ck810.h"
#define reg_read32(addr)        (*(volatile u32 *) (addr + PERI_BASE))
#define reg_write32(addr, val)  ((*(volatile u32 *) (addr + PERI_BASE)) = (val))

#define SOFT_DEBUG_FOR_DDR
#define DRAM_init_by_PHY

#if CONFIG_DDR4_1600_SUPPORT
#define add_margin
#define __DDR4_1600M__
// #define __DDR4_1066M__
#endif

#if CONFIG_DDR4_800_SUPPORT
#define add_margin
#define __DDR4_800M__
#endif

#else
//notes: this file include all ddr test task ,used for all test pattern
#include "clib.h"
#include "print_led.h"
#include <stdio.h>
#endif

#include "ddr_all_macro.h"
#include "ddr_chiptest_define.h"

#if RUN_DDR_IN_SPL_ENV
#include "ddr_debug_reg.h"
#endif

void ddr_low_power (unsigned int ddr_num);

//------------------------------------------------------------------------------//
//---------------part 1 : misc task --------------------------------------------//
//------------------------------------------------------------------------------//
void print_task (char string[])
{
#ifdef SOFT_DEBUG_FOR_DDR
printf("%s",string);
#else
print_led(string);
#endif
}

//void reg_write32 (int addr, int data)
//{
//#ifdef SOFT_DEBUG_FOR_DDR
//reg_write32(addr,data) ;
//printf("DDR_CFG_DEBUG : write address = 0x%x , data = 0x%x",addr,data);
//#else
//reg_write32(addr,data) ;
//#endif
//}


void read_reg_delay (int delay)
{
unsigned int read_data = 0 ;
int i ;
  for(i=0;i<delay;i=i+1) {
    read_data = reg_read32(0xF9200000) ;
     }
}

//get bit p right n bits data, for example, get bit0 data, you can set p=1 and n=1
unsigned int getbits(unsigned int x, unsigned int p, unsigned int n)
{
    return (x>>(p-n)) & (~(0xFFFFFFFF<<n));
}

//set x bit p right n bits data to y[n-1:0]
unsigned int setbits(unsigned int x, unsigned int p, unsigned int n, unsigned int y)
{
    return (x&(~((~(0xFFFFFFFF<<n))<<(p-n)))) | ((y&(~(0xFFFFFFFF<<n)))<<(p-n));
}

//------------------------------------------------------------------------------//
//---------------part 2 : reset task -------------------------------------------//
//----------------------------------------------------------------------------//
//DDR APB slave base address is :0xF920_0000
void apb_reset_release (unsigned int ddr_num)
{
    unsigned int read_data = 0 ;
    read_data = reg_read32(0xF9200004) ;
    read_data = read_data | (0x1 << (8+ddr_num)) ; // dmc apb reset
    read_data = read_data | (0x1 << (12+ddr_num)) ; //ddr phy apb reset
reg_write32(0xF9200004 ,read_data ); //release all dmc/phy apb reset
print_task("ddr controller/PHY apb reset release !\n");
}

void apb_reset_release_all (void)
{
reg_write32(0xF9200004 ,0xFF00 ); //release all dmc/phy apb reset
}

void dmc_reset_release (unsigned int ddr_num)
{
    unsigned int read_data = 0 ;
    read_data = reg_read32(0xF9200004) ;
    read_data = read_data | (0x1 << ddr_num) ; // dmc eset
reg_write32(0xF9200004 ,read_data ); //release all dmc reset
print_task("ddr controller reset release !\n");
}

void dmc_reset_release_all (void)
{
reg_write32(0xF9200004 ,0xFF0F );  //release all dmc reset
}

void phy_reset_release (unsigned int ddr_num)
{
    unsigned int read_data = 0 ;
    read_data = reg_read32(0xF9200004) ;
    read_data = read_data | (0x1 << (4+ddr_num)) ; // phy reset
reg_write32(0xF9200004 , read_data); //release all ddr phy reset
print_task("ddr phy reset release !\n");
}

void phy_reset_release_all (void)
{
reg_write32(0xF9200004 ,0xFFFF); //release all dmc reset
}


void axi_reset_release (unsigned int ddr_num)
{
    unsigned int read_data = 0 ;
    read_data = reg_read32(0xF9200000) ;
    read_data = read_data | (0x1F << (8*ddr_num)) ; // axi reset
reg_write32(0xF9200000 ,read_data ); //release all AXI port reset
print_task("AXI port reset release !\n");
}

void axi_reset_release_all (void)
{
axi_reset_release (0) ;
axi_reset_release (1) ;
axi_reset_release (2) ;
axi_reset_release (3) ;
}
//-------------------------------------------------------------------------------------//
//------------------------part 3 : DMC task--------------------------------------------//
//-------------------------------------------------------------------------------------//
//----------------------------------------------------
//enable the dfi_init_complete detection
//----------------------------------------------------
void dfi_init_complete_en (int ddr_ctrl_base_addr)
{
unsigned int read_data = 0 ;
    //enable quasi dynamic register programing
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_SWCTL, 0x0);

    read_data = reg_read32(ddr_ctrl_base_addr+DDR_CTRL_DFIMISC);
    read_data= read_data|0x1 ; //read_data[0] = 1'b1;
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIMISC , read_data);

    //disable quasi dynamic register programing
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_SWCTL, 0x1);

    do {
        read_data = reg_read32(ddr_ctrl_base_addr+DDR_CTRL_SWSTAT);
        read_data = read_data&0x00000001 ;
    }
    while(read_data != 1);
}

//----------------------------------------------------
//disable the dfi_init_complete detection
//----------------------------------------------------
void dfi_init_complete_dis(int ddr_ctrl_base_addr)
{
unsigned int read_data = 0 ;
    //enable quasi dynamic register programing
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_SWCTL, 0x0);

    read_data= reg_read32(ddr_ctrl_base_addr+DDR_CTRL_DFIMISC );
    read_data =read_data&0xFFFFFFFE ; //read_data[0] = 1'b0;
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIMISC , read_data);

    //disable quasi dynamic register programing
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_SWCTL, 0x1);

    do {
        read_data = reg_read32(ddr_ctrl_base_addr+DDR_CTRL_SWSTAT);
        read_data = read_data&0x00000001 ;
    }
    while(read_data != 1);
}

//------------low power task----------------------------//
void ddr_srf_pd_enable ( int ddr_ctrl_base_addr)
{
    int read_data = 0 ;
read_data = reg_read32(ddr_ctrl_base_addr+DDR_CTRL_PWRCTL);
read_data = read_data | 0x3 ;
reg_write32(ddr_ctrl_base_addr+DDR_CTRL_PWRCTL       , read_data);
}

//----------------------------------------------------
//DMC main control register setting
//----------------------------------------------------
//dev_type: DDR device type setting, 0(x4) 1(x8) 2(x16) 3(x32), just for DDR4
void dmc_cfg_main(unsigned int ddr_type, unsigned int dev_type,int ddr_ctrl_base_addr)
{
unsigned int read_data = 0 ;
//bit[0]:ddr3
//bit[1]:lpddr
//bit[2]:lpddr2
//bit[3]:lpddr3
//bit[4]:ddr4
//bit[5]:lpddr4
//bit[31:30]:device_config
//00:x4 device
//01:x8 device
//10:x16 device
//11:x32 device
read_data = reg_read32(ddr_ctrl_base_addr+DDR_CTRL_MSTR);
if(ddr_type== 0){
    read_data = (read_data&0xFFFFFFEF)|0x00000010 ; //read_data[4] ==1'b1
} else if(ddr_type == 1){
    read_data = (read_data&0xFFFFFFFE)|0x00000001 ; //read_data[0] ==1'b1
} else if(ddr_type == 2){
    read_data = (read_data&0xFFFFFFF7)|0x00000008 ; //read_data[3] ==1'b1
} else {
    print_task("Unsupported DDR type!\n");
}

if(ddr_type==0) {
 read_data = (read_data&0x3FFFFFFF) + (dev_type<<30) ; //  read_data[31:30] = dev_type;
}

//set DRAM burst length to BL8
read_data = (read_data&0xFFF0FFFF)|0x00040000 ;//read_data[19:16] = 4'h4;
reg_write32(ddr_ctrl_base_addr+DDR_CTRL_MSTR  , read_data   );

}

//----------------------------------------------------
//DMC DDR timing register setting
//----------------------------------------------------
//speed: DDR speed bin, 0(2400) 1(2133) 2(1600) 3(800)
void dmc_cfg_timing(unsigned int ddr_type, unsigned int speed,int ddr_ctrl_base_addr)
{
if(ddr_type == 0) {//DDR4
    if(speed == 0) {//2400
    //reg_write32(ddr_ctrl_base_addr+DDR_CTRL_PWRCTL       , 0x000000AA);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_PWRTMG       , 0x00221306);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_RFSHCTL0     , 0x00210070);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_RFSHCTL1     , 0x00010008);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_RFSHCTL3     , 0x00000000);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_RFSHTMG      , 0x009200D2);

    #ifdef SOFT_DEBUG_FOR_DDR
    #ifdef DRAM_init_by_PHY
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT0        , 0xC0030126);//TBD,pre_cke_x1024 need modify for chip testing, initial by PHY
    #else
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT0        , 0x00030126);//TBD,pre_cke_x1024 need modify for chip testing, initial by PHY
    #endif
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT1        , 0x00770000);//TBD,dram_rstn_x1024 need modify for chip testing
    #else
    //bit[11:0]:pre_cke_x1024,DDR4 500us
    //bit[25:16]:post_cke_x1024,at lease set to 1 avoid to MRS command violation
    //bit[31:30]:skip_dram_init
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT0        , 0x00010000);//TBD,pre-sim is 10002,post-sim is 0 for speed up
    //bit[3:0]:pre_ocd_x32
    //bit[24:16]:dram_rstn_x1024,DDR4 200us
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT1        , 0x00000000);//TBD,pre-sim is 20000,post-sim is 0 for speed up
    #endif
    //bit[15:0]:MR1
    //bit[31:16]:MR0,set tWR=20/tRTP=10
    //reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT3        , 0x0A340105);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT3        , ((DDR4_2400_MODE_REG0<<16)|(DDR4_2400_MODE_REG1)));
    //bit[15:0]:MR3
    //bit[31:16]:MR2
    //reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT4        , 0x02180200);//CWL=12
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT4        , ((DDR4_2400_MODE_REG2<<16)|(DDR4_2400_MODE_REG3)));
    //bit[9:0]:max_auto_init_x1024,just for LPDDR2/LPDDR3
    //bit[23:16]:dev_zqinit_x32,1024tCK for DDR4
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT5        , 0x00110000);
    //bit[15:0]:MR5
    //bit[31:16]:MR4
    //reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT6        , 0x00000480);//enable read 2T preamble
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT6        , ((DDR4_2400_MODE_REG4<<16)|(DDR4_2400_MODE_REG5)));
    //bit[15:0]:MR6
    //reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT7        , 0x0000081D);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT7        , DDR4_2400_MODE_REG6);

    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG0     , 0x11122914);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG1     , 0x0004051C);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG2     , 0x0608050D);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG3     , 0x0000400C);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG4     , 0x08030409);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG5     , 0x06060403);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG7     , 0x00000606);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG8     , 0x05050D08);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG9     , 0x0002040A);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG11    , 0x1409010e);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG12    , 0x00000008);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG15    , 0x00000000);

    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ZQCTL0       , 0x01000040);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ZQCTL1       , 0x0000493E);
    //bit[5:0] dfi_tphy_wrlat
    //bit[13:8] dfi_tphy_wrdata
    //bit[15] dfi_wrdata_use_dfi_phy_clk 0:HDR clock 1:SDR clock
    //bit[22:16] dfi_t_rddata_en
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFITMG0      , 0x038C820A);
    //bit[4:0]:dfi_t_dram_clk_enable
    //bit[12:8]:dfi_t_dram_clk_disable
    //bit[20:16]:dfi_t_wrdata_delay
    //bit[25:24]:dfi_t_parin_lat
    //bit[31:28]:dfi_t_cmd_lat
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFITMG1      , 0x00090404);

    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFILPCFG0    , 0x07F04011);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFILPCFG1    , 0x000000B0);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIUPD0    , 0xE0400018);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIUPD1    , 0x0048005A);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIUPD2    , 0x80000000);
//    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIMISC      , 0x00000001);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFITMG2    , 0x00000B07);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFITMG3    , 0x00000004);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DBICTL     , 0x00000001);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIPHYMSTR   , 0x00000000);

    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ODTCFG       , 0x06000610);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ODTMAP       , 0x00001323);

    }
    if(speed == 1) {//2133
    //reg_write32(ddr_ctrl_base_addr+DDR_CTRL_PWRCTL       , 0x000000AA);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_PWRTMG       , 0x00221306);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_RFSHCTL0     , 0x00210070);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_RFSHCTL1     , 0x00010008);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_RFSHCTL3     , 0x00000000);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_RFSHTMG      , 0x007000BB);


    #ifdef SOFT_DEBUG_FOR_DDR
    #ifdef DRAM_init_by_PHY
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT0        , 0xc0030106);//TBD,pre_cke_x1024 need modify for chip testing
    #else
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT0        , 0x00030106);//TBD,pre_cke_x1024 need modify for chip testing
    #endif
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT1        , 0x006A0000);//TBD,dram_rstn_x1024 need modify for chip testing
    #else
    //bit[11:0]:pre_cke_x1024,DDR4 500us
    //bit[25:16]:post_cke_x1024,at lease set to 1 avoid to MRS command violation
    //bit[31:30]:skip_dram_init
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT0        , 0x00010000);//TBD,pre-sim is 10002,post-sim is 0 for speed up
    //bit[3:0]:pre_ocd_x32
    //bit[24:16]:dram_rstn_x1024,DDR4 200us
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT1        , 0x00000000);//TBD,pre-sim is 20000,post-sim is 0 for speed up
    #endif
    //bit[15:0]:MR1
    //bit[31:16]:MR0,set tWR=20/tRTP=10
    //reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT3        , 0x0A300105);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT3        , ((DDR4_2133_MODE_REG0<<16)|(DDR4_2133_MODE_REG1)));
    //bit[15:0]:MR3
    //bit[31:16]:MR2
    //reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT4        , 0x02100200);//CWL=11
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT4        , ((DDR4_2133_MODE_REG2<<16)|(DDR4_2133_MODE_REG3)));
    //bit[9:0]:max_auto_init_x1024,just for LPDDR2/LPDDR3
    //bit[23:16]:dev_zqinit_x32,1024tCK for DDR4
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT5        , 0x00110000);
    //bit[15:0]:MR5
    //bit[31:16]:MR4
    //reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT6        , 0x00000480);//enable read 2T preamble
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT6        , ((DDR4_2133_MODE_REG4<<16)|(DDR4_2133_MODE_REG5)));
    //bit[15:0]:MR6
    //reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT7        , 0x0000081D);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT7        , DDR4_2133_MODE_REG6);

    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG0     , 0x10102412);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG1     , 0x00040419);
#ifdef add_margin
    printf("DDRINFO: ADD cmd margin \n");
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG2     , 0x06070a0e); //refine
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG4     , 0x070a0408);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG9     , 0x000a030e);
#else
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG2     , 0x0607040C);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG4     , 0x07030408);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG9     , 0x0002030A);
#endif
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG3     , 0x0000400C);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG5     , 0x06060403);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG7     , 0x00000606);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG8     , 0x04040D07);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG11    , 0x1308010e);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG12    , 0x00000008);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG15    , 0x00000000);

    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ZQCTL0       , 0x01000040);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ZQCTL1       , 0x00004111);
    //bit[5:0] dfi_tphy_wrlat
    //bit[13:8] dfi_tphy_wrdata
    //bit[15] dfi_wrdata_use_dfi_phy_clk 0:HDR clock 1:SDR clock
    //bit[22:16] dfi_t_rddata_en
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFITMG0      , 0x038A820A);
    //bit[4:0]:dfi_t_dram_clk_enable
    //bit[12:8]:dfi_t_dram_clk_disable
    //bit[20:16]:dfi_t_wrdata_delay
    //bit[25:24]:dfi_t_parin_lat
    //bit[31:28]:dfi_t_cmd_lat
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFITMG1      , 0x00090404);

    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFILPCFG0    , 0x07F04011);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFILPCFG1    , 0x000000B0);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIUPD0    , 0xE0400018);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIUPD1    , 0x0048005A);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIUPD2    , 0x80000000);
//    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIMISC      , 0x00000001);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFITMG2    , 0x00000A06);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFITMG3    , 0x00000004);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DBICTL     , 0x00000001);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIPHYMSTR   , 0x00000000);

    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ODTCFG       , 0x06000608);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ODTMAP       , 0x00001323);

    }
    if(speed == 2) {//1600
    //reg_write32(ddr_ctrl_base_addr+DDR_CTRL_PWRCTL       , 0x000000AA);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_PWRTMG       , 0x00221306);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_RFSHCTL0     , 0x00210070);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_RFSHCTL1     , 0x00010008);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_RFSHCTL3     , 0x00000000);
#ifdef __DDR4_800M__
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_RFSHTMG      , 0x00300046);
#else
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_RFSHTMG      , 0x0061008c);
#endif

    #ifdef SOFT_DEBUG_FOR_DDR
    #ifdef DRAM_init_by_PHY
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT0        , 0xc00200C5);//TBD,pre_cke_x1024 need modify for chip testing
    #else
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT0        , 0x000200C5);//TBD,pre_cke_x1024 need modify for chip testing
    #endif
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT1        , 0x00500000);//TBD,dram_rstn_x1024 need modify for chip testing
    #else
    //bit[11:0]:pre_cke_x1024,DDR4 500us
    //bit[25:16]:post_cke_x1024,at lease set to 1 avoid to MRS command violation
    //bit[31:30]:skip_dram_init
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT0        , 0x00010000);//TBD,pre-sim is 10002,post-sim is 0 for speed up
    //bit[3:0]:pre_ocd_x32
    //bit[24:16]:dram_rstn_x1024,DDR4 200us
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT1        , 0x00000000);//TBD,pre-sim is 20000,post-sim is 0 for speed up
    #endif
    //bit[15:0]:MR1
    //bit[31:16]:MR0,set tWR=20/tRTP=10
    //reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT3        , 0x0A100105);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT3        , ((DDR4_1600_MODE_REG0<<16)|(DDR4_1600_MODE_REG1)));
    //bit[15:0]:MR3
    //bit[31:16]:MR2
    //reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT4        , 0x02000200);//CWL=9
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT4        , ((DDR4_1600_MODE_REG2<<16)|(DDR4_1600_MODE_REG3)));
    //bit[9:0]:max_auto_init_x1024,just for LPDDR2/LPDDR3
    //bit[23:16]:dev_zqinit_x32,1024tCK for DDR4
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT5        , 0x00110000);
    //bit[15:0]:MR5
    //bit[31:16]:MR4
    //reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT6        , 0x00000480);//enable read 2T preamble
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT6        , ((DDR4_1600_MODE_REG4<<16)|(DDR4_1600_MODE_REG5)));
    //bit[15:0]:MR6
    //reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT7        , 0x0000081D);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT7        , DDR4_1600_MODE_REG6);

    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG0     , 0x0E0C1B0D);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG1     , 0x00030313);
#ifdef add_margin
    printf("DDRINFO: ADD cmd margin \n");
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG2     , 0x06060a0e);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG4     , 0x060a0307);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG9     , 0x000a030a);
#else
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG2     , 0x0606030B);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG4     , 0x06030307);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG9     , 0x00020309);
#endif
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG3     , 0x0000400C);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG5     , 0x04040302);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG7     , 0x00000404);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG8     , 0x04040d06);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG11    , 0x1206010e);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG12    , 0x00000008);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG15    , 0x00000000);

    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ZQCTL0       , 0x01000040);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ZQCTL1       , 0x000030D4);
    //bit[5:0] dfi_tphy_wrlat
    //bit[13:8] dfi_tphy_wrdata
    //bit[15] dfi_wrdata_use_dfi_phy_clk 0:HDR clock 1:SDR clock
    //bit[22:16] dfi_t_rddata_en
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFITMG0      , 0x03888209);
    //bit[4:0]:dfi_t_dram_clk_enable
    //bit[12:8]:dfi_t_dram_clk_disable
    //bit[20:16]:dfi_t_wrdata_delay
    //bit[25:24]:dfi_t_parin_lat
    //bit[31:28]:dfi_t_cmd_lat
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFITMG1      , 0x00090404);

    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFILPCFG0    , 0x07F04011);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFILPCFG1    , 0x000000B0);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIUPD0    , 0xE0400018);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIUPD1    , 0x0048005A);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIUPD2    , 0x80000000);
//    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIMISC      , 0x00000001);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFITMG2    , 0x00000604);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFITMG3    , 0x00000004);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DBICTL     , 0x00000001);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIPHYMSTR   , 0x00000000);

    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ODTCFG       , 0x06000604);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ODTMAP       , 0x00001323);
    }
}
else if(ddr_type == 1) {//DDR3 , wl=8,rl=11
    if(speed == 2) {//1600
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_RFSHTMG      , 0x00610040);

    #ifdef SOFT_DEBUG_FOR_DDR
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT0        , 0xC00200C5);//TBD,pre_cke_x1024 need modify for chip testing
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT1        , 0x0001000b);//TBD,dram_rstn_x1024 need modify for chip testing
    #else
    //bit[11:0]:pre_cke_x1024,DDR4 500us
    //bit[25:16]:post_cke_x1024,at lease set to 1 avoid to MRS command violation
    //bit[31:30]:skip_dram_init
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT0        , 0x00010000);//TBD,pre_cke_x1024 need modify for chip testing
    //bit[3:0]:pre_ocd_x32
    //bit[24:16]:dram_rstn_x1024,DDR4 200us
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT1        , 0x00000000);//TBD,dram_rstn_x1024 need modify for chip testing
    #endif
    //bit[15:0]:MR1
    //bit[31:16]:MR0,set tWR=12
    //reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT3        , 0x1D700084);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT3        , ((DDR3_1600_MODE_REG0<<16)|(DDR3_1600_MODE_REG1)));
    //bit[15:0]:MR3
    //bit[31:16]:MR2
    //reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT4        , 0x00180000);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT4        , ((DDR3_1600_MODE_REG2<<16)|(DDR3_1600_MODE_REG3)));

    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG0     , 0x0c101b0e);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG1     , 0x00030314);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG2     , 0x04060509);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG3     , 0x00002006);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG4     , 0x06020306);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG5     , 0x04040302);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG8     , 0x00000909);

    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ZQCTL0       , 0x40800020);//disable auto ZQCS
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ZQCTL1       , 0x0000c350);
    //bit[5:0] dfi_tphy_wrlat == WL -2 = 8-2= 6
    //bit[13:8] dfi_tphy_wrdata
    //bit[15] dfi_wrdata_use_dfi_phy_clk 0:HDR clock 1:SDR clock
    //bit[22:16] dfi_t_rddata_en == RL -4 = 11-4 = 7
    //reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFITMG0      , 0x03848202);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFITMG0      , 0x03878206); //change by yangwei
    //bit[4:0]:dfi_t_dram_clk_enable
    //bit[12:8]:dfi_t_dram_clk_disable
    //bit[20:16]:dfi_t_wrdata_delay
    //bit[25:24]:dfi_t_parin_lat
    //bit[31:28]:dfi_t_cmd_lat
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFITMG1      , 0x00090404);

    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFITMG2      , 0x00000603);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFILPCFG0    , 0x07000000);
//  reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIMISC      , 0x00000011);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIUPD0      , 0x00400018);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIUPD1      , 0x0005003C);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIUPD2      , 0x80000000);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIPHYMSTR  , 0x00000000);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ODTCFG       , 0x0600060C);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ODTMAP       , 0x00001323);

    }
   if(speed == 3) {//800
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_RFSHTMG      , 0x00300020);

    #ifdef SOFT_DEBUG_FOR_DDR
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT0        , 0xc0020063);//TBD,pre_cke_x1024 need modify for chip testing
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT1        , 0x0001000b);//TBD,dram_rstn_x1024 need modify for chip testing
    #else
    //bit[11:0]:pre_cke_x1024,DDR4 500us
    //bit[25:16]:post_cke_x1024,at lease set to 1 avoid to MRS command violation
    //bit[31:30]:skip_dram_init
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT0        , 0x00010000);//TBD,pre_cke_x1024 need modify for chip testing
    //bit[3:0]:pre_ocd_x32
    //bit[24:16]:dram_rstn_x1024,DDR4 200us
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT1        , 0x00000000);//TBD,dram_rstn_x1024 need modify for chip testing
    #endif
    //bit[15:0]:MR1
    //bit[31:16]:MR0,set tWR=12
    //reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT3        , 0x1D700084);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT3        , ((DDR3_800_MODE_REG0<<16)|(DDR3_800_MODE_REG1)));
    //bit[15:0]:MR3
    //bit[31:16]:MR2
    //reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT4        , 0x00180000);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT4        , ((DDR3_800_MODE_REG2<<16)|(DDR3_800_MODE_REG3)));

    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG0     , 0x0c101b0e);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG1     , 0x00030314);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG2     , 0x04060509);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG3     , 0x00002006);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG4     , 0x06020306);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG5     , 0x04040302);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG8     , 0x00000909);

    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ZQCTL0       , 0x40800020);//disable auto ZQCS
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ZQCTL1       , 0x0000c350);


    //bit[5:0] dfi_tphy_wrlat == WL -2 = 8-2= 6
    //bit[13:8] dfi_tphy_wrdata
    //bit[15] dfi_wrdata_use_dfi_phy_clk 0:HDR clock 1:SDR clock
    //bit[22:16] dfi_t_rddata_en == RL -4 = 11-4 = 7
    //reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFITMG0      , 0x03848202);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFITMG0      , 0x03878206); //change by yangwei
    //bit[4:0]:dfi_t_dram_clk_enable
    //bit[12:8]:dfi_t_dram_clk_disable
    //bit[20:16]:dfi_t_wrdata_delay
    //bit[25:24]:dfi_t_parin_lat
    //bit[31:28]:dfi_t_cmd_lat
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFITMG1      , 0x00090404);

    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFITMG2      , 0x00000603);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFILPCFG0    , 0x07000000);
//  reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIMISC      , 0x00000011);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIUPD0      , 0x00400018);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIUPD1      , 0x0005003C);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIUPD2      , 0x80000000);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIPHYMSTR  , 0x00000000);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ODTCFG       , 0x0600060C);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ODTMAP       , 0x00001323);

   }
}
else if(ddr_type == 2) {//LPDDR3 wl=8,rl=14
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIMISC      , 0x00000000);//disable dfi_init_complete check,hold DDR initialization
    //bit[31:30]:device_config
    //00:x4 device
    //01:x8 device
    //10:x16 device
    //11:x32 device
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_MSTR         , 0x40040008);
    //bit[2]:per_bank_refresh
    //bit[9:4]:refresh_burst 0-single refresh 1-burst-of-2 refresh 7-burst-of-8 refresh
    //bit[16:12]:refresh_to_x1_x32, for speculative refresh idle set
    //bit[23:20]:refresh_margin
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_RFSHCTL0     , 0x00f08000);
    //bit[9:0]:t_rfc_min
    //bit[27:16]:t_rfc_nom_x1_x32,trefi,unit:32
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_RFSHTMG      , 0x0072003d);
    #ifdef SOFT_DEBUG_FOR_DDR
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT0        , 0xc005c001);//TBD,post_cke_x1024 need modify for chip testing
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT2        , 0x00000005);
    #else
    //bit[11:0]:pre_cke_x1024,LPDDR2/LPDDR3 tINIT1 100ns
    //bit[25:16]:post_cke_x1024,LPDDR2/LPDDR3 need 200us
    //bit[31:30]:skip_dram_init
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT0        , 0x00010001);//TBD,post_cke_x1024 need modify for chip testing
    #endif
    //bit[31:16]:MR1,set nWR=6
    //bit[15:0]:MR2
    //reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT3        , 0x0083001C);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT3        , 0x0083000C); //change MR[4] to 0 ,wr=6 for ddrphy support????
    //bit[31:16]:MR3
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT4        , 0x00020000);
    //bit[23:16]:dev_zqinit_x32,1us
    //bit[9:0]:max_auto_init_x1024,5us
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_INIT5        , 0x00100001);//TBD,max_auto_init_x1024 need modify for chip testing
    //bit[30:24]:wr2pre
    //bit[21:16]:tfaw
    //bit[14:8]:tRASmax
    //bit[5:0]:tRASmin
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG0     , 0xe182014);
    //bit[20:16]:txp
    //bit[13:8]:rd2pre
    //bit[6:0]:tRC
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG1     , 0x4041c);
    //bit[29:24]:write latency,round up
    //bit[21:16]:read latency,round up
    //bit[13:8]:rd2wr RL+BL/2+1+WR_PREAMBLE-WL
    //bit[5:0]:wr2rd CWL+PL+BL+tWTR_L
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG2     , 0x407090b);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG3     , 0x407000);
    //bit[28:24]:tRCD
    //bit[19:16]:tCCD
    //bit[11:8]:tRRD,trrd_l for DDR4
    //bit[4:0]:tRP
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG4     , 0x7020509);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG5     , 0x1010702);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG6     , 0x2020005);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DRAMTMG7     , 0x202);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ZQCTL0       , 0xd0800020);//disable auto ZQCS
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ZQCTL1       , 0x02020000);
    //bit[5:0] dfi_tphy_wrlat == WL -2 = 8-2= 6
    //bit[13:8] dfi_tphy_wrdata
    //bit[15] dfi_wrdata_use_dfi_phy_clk 0:HDR clock 1:SDR clock
    //bit[22:16] dfi_t_rddata_en == RL -4 = 14-4 = A
    //reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFITMG0      , 0x038A8206); //change by yangwei
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFITMG0      , 0x038A8207); //change by yangwei
    //bit[4:0]:dfi_t_dram_clk_enable
    //bit[12:8]:dfi_t_dram_clk_disable
    //bit[20:16]:dfi_t_wrdata_delay
    //bit[25:24]:dfi_t_parin_lat
    //bit[31:28]:dfi_t_cmd_lat
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFITMG1      , 0x00090404);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DFIUPD0      , 0xc0400005);//disable DMC controller update
    //bit[0]:dm_en
    //bit[1]:wr_dbi_en
    //bit[2]:rd_dbi_en
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_DBICTL       , 0x00000001);
    }
    else {
        print_task ("Un-supported DDR type(dmc_cfg_timing)!\n");
    }
}


//----------------------------------------------------
//DMC address mapping setting
//----------------------------------------------------
//ddr_type: 0(DDR4),1(DDR3),2(LPDDR3),3(LPDDR2)
//capacity: DDR space capacity 0(1GB),1(2GB),2(4GB)
void dmc_cfg_addr_map(unsigned int ddr_type , unsigned int capacity,int ddr_ctrl_base_addr)
{
    if(ddr_type == 0){//DDR4
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP0     , 0x00003f1f);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP1     , 0x003f0909);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP2     , 0x01010100);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP3     , 0x01010101);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP4     , 0x00001f1f);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP5     , 0x07070707);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP6     , 0x07070707);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP7     , 0x00000f0f);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP8     , 0x00003f01);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP9     , 0x00000000);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP10    , 0x00000000);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP11    , 0x00000000);
    } else if(ddr_type == 1){//DDR3
          reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP0     , 0x0000001f);
          reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP1     , 0x00080808);
          reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP2     , 0x00000000);
          reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP3     , 0x00000000);
          reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP4     , 0x00001f1f);
          reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP5     , 0x07070707);
          reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP6     , 0x0F0F0707);
          reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP9     , 0x0A020b06);
          reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP10    , 0x0A0A0A0A);
          reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP11    , 0x00000000);
    } else if(ddr_type == 2) {//LPDDR3
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP1     , 0x00090909);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP2     , 0x00000000);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP3     , 0x00000000);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP4     , 0x00001f1f);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP5     , 0x080f0808);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP6     , 0x0f080808);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP9     , 0x08080808);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP10    , 0x08080808);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP11    , 0x001f1f08);
    } else if(ddr_type == 3) {//LPDDR2
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP1     , 0x00090909);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP2     , 0x00000000);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP3     , 0x00000000);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP4     , 0x00001f00);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP5     , 0x080f0808);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP6     , 0x0f0f0808);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP9     , 0x08080808);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP10    , 0x08080808);
            reg_write32(ddr_ctrl_base_addr+DDR_CTRL_ADDRMAP11    , 0x001f1f08);
    } else {
        print_task("Un-supported DDR type!\n");
    }
}




//----------------------------------------------------
//DMC multi-port register setting
//----------------------------------------------------
void dmc_cfg_mp(int ddr_ctrl_base_addr)
{
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_PCTRL_0      , 0x00000001);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_PCTRL_1      , 0x00000001);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_PCTRL_2      , 0x00000001);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_PCTRL_3      , 0x00000001);
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_PCTRL_4      , 0x00000001);
}

//----------------------------------------------------
//DMC core QoS register setting
//----------------------------------------------------
void dmc_cfg_qos(int ddr_ctrl_base_addr)
{
//bit[0]:force_low_pri_n,low valid
//bit[1]:prefer_write
//bit[2]:pageclose
//bit[x:8]:lpr_num_entries
//bit[23:16]:go2critical_hysteresis,unused
//bit[30:24]:rdwr_idle_gap
reg_write32(ddr_ctrl_base_addr+DDR_CTRL_SCHED        , 0x40001801);//reserved 8 entry for HPR
reg_write32(ddr_ctrl_base_addr+DDR_CTRL_SCHED1       , 0x00000033);
//bit[15:0]:hpr_max_starve
//bit[31:24]:hpr_xact_length
reg_write32(ddr_ctrl_base_addr+DDR_CTRL_PERFHPR1     , 0x0f001000);
//bit[15:0]:lpr_max_starve
//bit[31:24]:lpr_xact_run_length
reg_write32(ddr_ctrl_base_addr+DDR_CTRL_PERFLPR1     , 0x0f004000);
//bit[15:0]:w_max_starve
//bit[31:24]:w_xact_run_length
reg_write32(ddr_ctrl_base_addr+DDR_CTRL_PERFWR1      , 0x0f001000);
}

//DMC initialization main task
//ddr_type: 0(DDR4) 1(DDR3) 2(LPDDR3)
//speed: DDR speed bin, 0(2400) 1(2133) 2(1600) 3(1333)
//capacity: total DDR space capacity setting, 0(512MB) 1(1GB) 2(2GB)
//dev_type: DDR4 device type setting, 0(x4) 1(x8) 2(x16) 3(x32), just for DDR4
//----------------------------------------------------
void dmc_init(unsigned int ddr_type, unsigned int speed ,unsigned int capacity, unsigned int dev_type,int ddr_ctrl_base_addr)
{
 dmc_cfg_main (ddr_type,dev_type,ddr_ctrl_base_addr);
 dmc_cfg_timing (ddr_type,speed,ddr_ctrl_base_addr);
 dmc_cfg_addr_map (ddr_type,capacity,ddr_ctrl_base_addr);
#ifdef DDR_PD_SR_EN
 ddr_srf_pd_enable(ddr_ctrl_base_addr);
#endif
 dmc_cfg_mp (ddr_ctrl_base_addr);
#ifdef DDR_QOS_TEST
 dmc_cfg_qos (ddr_ctrl_base_addr);
#endif
 print_task("dmc config done!\n");
}
//-------------------------------------------------------------------------------------//
//--------------------------part 4 : DDR PHY task--------------------------------------//
//-------------------------------------------------------------------------------------//
//----------------------------------------------------
//DDR PHY main control register setting
//----------------------------------------------------
void ddr_phy_main (unsigned int ddr_type ,unsigned int speed,int ddr_phy_base_addr)
{
    unsigned int read_data = 0 ;
    //---DDL config :PGCR0-5 ????
    // read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_PGCR1);
    // read_data = read_data & (~(0x3<<13)); //bit[14:13] FDEPTH
    // reg_write32(ddr_phy_base_addr+DDR_PHY_PGCR1, read_data);


    //----PHY PLL config(PLL TYPE A) : PLLCR
    //[20:19]
    //2'b00 = PLL reference clock (ctl_clk/ref_clk) ranges from 440MHz to 667MHz.
    //2'b01 = PLL reference clock (ctl_clk/ref_clk) ranges from 225MHz to 490MHz.
    //2'b10 = reserved
    //2'b11 = PLL reference clock (ctl_clk/ref_clk) ranges from 166MHz to 275MHz

    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_PLLCR);   //TBD
    if(speed == 0) { //2400/4=600
       read_data = read_data&0xFFE7FFFF ; //[20:19]=00
    } else if (speed ==1 ) { // 2133/4 = 533
       read_data = read_data&0xFFE7FFFF ; //[20:19]=00
    } else if (speed ==2 ) { //1600/4=400
       read_data = (read_data&0xFFE7FFFF)|0x00080000 ; //[20:19]=01
    } else if (speed ==3 ) { //800/4=200
       read_data = (read_data&0xFFE7FFFF)|0x00180000 ; //[20:19]=11
    }
    reg_write32(ddr_phy_base_addr+DDR_PHY_PLLCR, read_data);

    //----PHY ZQ calibration
    //configuration PGWAIT
    //3'b000, if ctl_clk <= 100 MHz
    //3'b001, if 100 MHz < ctl_clk <= 200 MHz
    //3'b010, if 200 MHz < ctl_clk <= 267 MHz
    //3'b011, if 267 MHz < ctl_clk <= 300 MHz
    //3'b100, if 300 MHz < ctl_clk <= 333 MHz
    //3'b101, if 333 MHz < ctl_clk <= 400 MHz
    //3'b110, if 400 MHz < ctl_clk <= 467 MHz
    //3'b111, if 467 MHz < ctl_clk
    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_ZQCR);
    if((speed == 0) || (speed == 1)) {
       //read_data[10:8] = 7;
        read_data = (read_data&0xFFFFF8FF)|0x00000700 ;
    } else if(speed == 2) {
       //read_data[10:8] = 5;
        read_data = (read_data&0xFFFFF8FF)|0x00000500 ;
    } else if(speed == 3) {
       //read_data[10:8] = 4;
        read_data = (read_data&0xFFFFF8FF)|0x00000400 ;
    }
    reg_write32(ddr_phy_base_addr+DDR_PHY_ZQCR, read_data);

    //-------------ZQnPR?????

    if(ddr_type==0) { //DDR4
    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_ZQ0PR);
    read_data = read_data | 0x10000000 ; //[28]=1
    read_data = (read_data &0xFFF0FFFF)|(DDR4_ZPROG_PU_ODT_ONLY << 16);//[15:12]  ZPROG_ASYM_DRV_PD
    read_data = (read_data &0xFFFF0FFF)|(DDR4_ZPROG_ASYM_DRV_PD << 12);//[15:12]  ZPROG_ASYM_DRV_PD
    read_data = (read_data &0xFFFFF0FF)|(DDR4_ZPROG_ASYM_DRV_PU << 8);//[11:8]  ZPROG_ASYM_DRV_PU
    reg_write32(ddr_phy_base_addr+DDR_PHY_ZQ0PR, read_data);

    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_ZQ1PR);
    read_data = read_data | 0x10000000 ; //[28]=1
    read_data = (read_data &0xFFF0FFFF)|(DDR4_ZPROG_PU_ODT_ONLY << 16);//[15:12]  ZPROG_ASYM_DRV_PD
    read_data = (read_data &0xFFFF0FFF)|(DDR4_ZPROG_ASYM_DRV_PD << 12);//[15:12]  ZPROG_ASYM_DRV_PD
    read_data = (read_data &0xFFFFF0FF)|(DDR4_ZPROG_ASYM_DRV_PU << 8);//[11:8]  ZPROG_ASYM_DRV_PU
    reg_write32(ddr_phy_base_addr+DDR_PHY_ZQ1PR, read_data);

    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_ZQ2PR);
    read_data = read_data | 0x10000000 ; //[28]=1
    read_data = (read_data &0xFFF0FFFF)|(DDR4_ZPROG_PU_ODT_ONLY << 16);//[15:12]  ZPROG_ASYM_DRV_PD
    read_data = (read_data &0xFFFF0FFF)|(DDR4_ZPROG_ASYM_DRV_PD << 12);//[15:12]  ZPROG_ASYM_DRV_PD
    read_data = (read_data &0xFFFFF0FF)|(DDR4_ZPROG_ASYM_DRV_PU << 8);//[11:8]  ZPROG_ASYM_DRV_PU
    reg_write32(ddr_phy_base_addr+DDR_PHY_ZQ2PR, read_data);

    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_ZQ3PR);
    read_data = read_data | 0x10000000 ; //[28]=1
    read_data = (read_data &0xFFF0FFFF)|(DDR4_ZPROG_PU_ODT_ONLY << 16);//[15:12]  ZPROG_ASYM_DRV_PD
    read_data = (read_data &0xFFFF0FFF)|(DDR4_ZPROG_ASYM_DRV_PD << 12);//[15:12]  ZPROG_ASYM_DRV_PD
    read_data = (read_data &0xFFFFF0FF)|(DDR4_ZPROG_ASYM_DRV_PU << 8);//[11:8]  ZPROG_ASYM_DRV_PU
    reg_write32(ddr_phy_base_addr+DDR_PHY_ZQ3PR, read_data);



    } else if(ddr_type==1) { //DDR3
    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_ZQ0PR);
//    read_data = read_data | 0x10000000 ; //[28]=1
    read_data = (read_data &0xFFF0FFFF)|(DDR3_ZPROG_PU_ODT_ONLY << 16);//[15:12]  ZPROG_ASYM_DRV_PD
    read_data = (read_data &0xFFFF0FFF)|(DDR3_ZPROG_ASYM_DRV_PD << 12);//[15:12]  ZPROG_ASYM_DRV_PD
    read_data = (read_data &0xFFFFF0FF)|(DDR3_ZPROG_ASYM_DRV_PU << 8);//[11:8]  ZPROG_ASYM_DRV_PU
    reg_write32(ddr_phy_base_addr+DDR_PHY_ZQ0PR, read_data);

    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_ZQ1PR);
//    read_data = read_data | 0x10000000 ; //[28]=1
    read_data = (read_data &0xFFF0FFFF)|(DDR3_ZPROG_PU_ODT_ONLY << 16);//[15:12]  ZPROG_ASYM_DRV_PD
    read_data = (read_data &0xFFFF0FFF)|(DDR3_ZPROG_ASYM_DRV_PD << 12);//[15:12]  ZPROG_ASYM_DRV_PD
    read_data = (read_data &0xFFFFF0FF)|(DDR3_ZPROG_ASYM_DRV_PU << 8);//[11:8]  ZPROG_ASYM_DRV_PU
    reg_write32(ddr_phy_base_addr+DDR_PHY_ZQ1PR, read_data);

    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_ZQ2PR);
//    read_data = read_data | 0x10000000 ; //[28]=1
    read_data = (read_data &0xFFF0FFFF)|(DDR3_ZPROG_PU_ODT_ONLY << 16);//[15:12]  ZPROG_ASYM_DRV_PD
    read_data = (read_data &0xFFFF0FFF)|(DDR3_ZPROG_ASYM_DRV_PD << 12);//[15:12]  ZPROG_ASYM_DRV_PD
    read_data = (read_data &0xFFFFF0FF)|(DDR3_ZPROG_ASYM_DRV_PU << 8);//[11:8]  ZPROG_ASYM_DRV_PU
    reg_write32(ddr_phy_base_addr+DDR_PHY_ZQ2PR, read_data);

    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_ZQ3PR);
//    read_data = read_data | 0x10000000 ; //[28]=1
    read_data = (read_data &0xFFF0FFFF)|(DDR3_ZPROG_PU_ODT_ONLY << 16);//[15:12]  ZPROG_ASYM_DRV_PD
    read_data = (read_data &0xFFFF0FFF)|(DDR3_ZPROG_ASYM_DRV_PD << 12);//[15:12]  ZPROG_ASYM_DRV_PD
    read_data = (read_data &0xFFFFF0FF)|(DDR3_ZPROG_ASYM_DRV_PU << 8);//[11:8]  ZPROG_ASYM_DRV_PU
    reg_write32(ddr_phy_base_addr+DDR_PHY_ZQ3PR, read_data);
    }
}



//----------------------------------------------------
//DDR PHY DDR timing register setting
//----------------------------------------------------
void ddr_phy_timing (unsigned int ddr_type ,unsigned int speed,unsigned int skip,int ddr_phy_base_addr)
{
    unsigned int read_data = 0 ;
    //----PHY DRAM type config : DCR
    //[2:0] :configure DDRMD 001 = LPDDR3 011 = DDR3 100 = DDR4
    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_DCR);
    if(ddr_type==0) {
        read_data = (read_data&0xFFFFFFF8)|0x00000004 ; //DDR4
    } else if(ddr_type==1) {
        read_data = (read_data&0xFFFFFFF8)|0x00000003 ;//DDR3
    } else if(ddr_type==2) {
        read_data = (read_data&0xFFFFFFF8)|0x00000001 ;//LPDDR3
    }
    reg_write32(ddr_phy_base_addr+DDR_PHY_DCR, read_data);
    //For DDR3/DDR4 mode,we can enable PUB command 2T timing
    if((ddr_type == 0) || (ddr_type == 1)) {
        read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_DCR);
        //read_data[28] = 1;
        read_data = (read_data&0xEFFFFFFF)|0x10000000 ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_DCR, read_data);
    }

    //-----PHY mode register config : MR0-6
    //ddr_type: 0(DDR4) 1(DDR3) 2(LPDDR3)
    //speed: DDR speed bin, 0(2400) 1(2133) 2(1600) 3(800)
    if(ddr_type == 0) {//DDR4
        if(speed == 0) {//2400
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR0, DDR4_2400_MODE_REG0);
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR1, DDR4_2400_MODE_REG1);
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR2, DDR4_2400_MODE_REG2);
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR3, DDR4_2400_MODE_REG3);
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR4, DDR4_2400_MODE_REG4);
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR5, DDR4_2400_MODE_REG5);
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR6, DDR4_2400_MODE_REG6);
        }
        if(speed == 1) {//2133
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR0, DDR4_2133_MODE_REG0);
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR1, DDR4_2133_MODE_REG1);
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR2, DDR4_2133_MODE_REG2);
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR3, DDR4_2133_MODE_REG3);
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR4, DDR4_2133_MODE_REG4);
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR5, DDR4_2133_MODE_REG5);
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR6, DDR4_2133_MODE_REG6);
        }
        if(speed == 2) {//1600
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR0, DDR4_1600_MODE_REG0);
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR1, DDR4_1600_MODE_REG1);
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR2, DDR4_1600_MODE_REG2);
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR3, DDR4_1600_MODE_REG3);
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR4, DDR4_1600_MODE_REG4);
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR5, DDR4_1600_MODE_REG5);
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR6, DDR4_1600_MODE_REG6);
        }
    } else if (ddr_type == 1) {//DDR3
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR0, DDR3_1600_MODE_REG0);
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR1, DDR3_1600_MODE_REG1);
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR2, DDR3_1600_MODE_REG2);
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR3, DDR3_1600_MODE_REG3);

    } else if (ddr_type== 2) { //LPDDR3
        read_data = (0x4<<5) | 0x3;//BL=8. WR=6
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR1, read_data);
        read_data = 0xc ;//WL=8 ,RL=14
        reg_write32(ddr_phy_base_addr+DDR_PHY_MR2, read_data);
    }

    //-----PHY DRAM timing config : DTPR0-6
    if(ddr_type == 0) {//DDR4
        if(speed == 0) {//2400
        read_data = 9 | (16<<8) | (39<<16) | (8<<24);
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR0, read_data);

        read_data = 8 | (0<<8) | (36<<16) | (40<<24);
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR1, read_data);
        read_data = 768 | (7<<16) | (0<<24) | (0<<28);
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR2, read_data);

        read_data = (896<<16) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR3, read_data);

        read_data = 9 | (12<<8) | (420<<16) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR4, read_data);

        read_data = 9 | (24<<8) | (16<<16) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR5, read_data);

        #ifdef SOFT_DEBUG_FOR_DDR
        read_data = 10 | (2402<<6) | (602<<21) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_PTR0, read_data);

        read_data = 5402 | (60002<<15) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_PTR1, read_data);

        read_data = 600000 | (432<<20) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_PTR3, read_data);

        read_data = 240000 | (1024<<18) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_PTR4, read_data);
        #endif
        }
        if(speed == 1) {//2133
        read_data = 8 | (14<<8) | (35<<16) | (7<<24);
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR0, read_data);

        read_data = 8 | (0<<8) | (32<<16) | (40<<24);
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR1, read_data);

        read_data = 768 | (7<<16) | (0<<24) | (0<<28);
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR2, read_data);

        read_data = (896<<16) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR3, read_data);

        read_data = 7 | (11<<8) | (374<<16) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR4, read_data);

        read_data = 9 | (22<<8) | (14<<16) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR5, read_data);

        #ifdef SOFT_DEBUG_FOR_DDR
        read_data = 10 | (2134<<6) | (535<<21) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_PTR0, read_data);

        read_data = 4799 | (53302<<15) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_PTR1, read_data);

        read_data = 533000 | (384<<20) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_PTR3, read_data);

        read_data = 213200 | (1024<<18) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_PTR4, read_data);
        #endif
        }
        if(speed == 2) {//1600
        read_data = 6 | (12<<8) | (26<<16) | (6<<24);
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR0, read_data);

        read_data = 8 | (0<<8) | (24<<16) | (40<<24);
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR1, read_data);

        read_data = 768 | (5<<16) | (0<<24) | (0<<28);
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR2, read_data);

        read_data = (896<<16) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR3, read_data);

        read_data = 4 | (8<<8) | (280<<16) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR4, read_data);

        read_data = 9 | (20<<8) | (12<<16) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR5, read_data);

        #ifdef SOFT_DEBUG_FOR_DDR
        read_data = 10 | (1602<<6) | (402<<21) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_PTR0, read_data);

        read_data = 3602 | (40002<<15) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_PTR1, read_data);

        read_data = 400000 | (288<<20) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_PTR3, read_data);

        read_data = 160000 | (1024<<18) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_PTR4, read_data);
        #endif
        }
    } else if (ddr_type== 1) { //DDR3
        if(speed == 2) {//1600
        read_data = 8 | (11<<8) | (28<<16) | (6<<24);
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR0, read_data);

        read_data = 4 | (0<<8) | (32<<16) | (40<<24);
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR1, read_data);

        read_data = 512 | (5<<16) | (0<<24) | (0<<28);
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR2, read_data);

        read_data = (640<<16) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR3, read_data);

        read_data = 4 | (8<<8) | (128<<16) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR4, read_data);

        read_data = 9 | (19<<8) | (39<<16) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR5, read_data);

        #ifdef SOFT_DEBUG_FOR_DDR
        read_data = 10 | (1602<<6) | (402<<21) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_PTR0, read_data);

        read_data = 3602 | (40002<<15) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_PTR1, read_data);

        read_data = 400000 | (136<<20) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_PTR3, read_data);

        read_data = 160000 | (1024<<18) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_PTR4, read_data);
        #endif
        }

        if(speed == 3) {//800
        read_data = 8 | (11<<8) | (28<<16) | (6<<24);
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR0, read_data);

        read_data = 4 | (0<<8) | (32<<16) | (40<<24);
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR1, read_data);

        read_data = 512 | (5<<16) | (0<<24) | (0<<28);
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR2, read_data);

        read_data = (640<<16) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR3, read_data);

        read_data = 4 | (8<<8) | (128<<16) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR4, read_data);

        read_data = 9 | (19<<8) | (39<<16) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR5, read_data);

        #ifdef SOFT_DEBUG_FOR_DDR
        read_data = 10 | (1602<<6) | (402<<21) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_PTR0, read_data);

        read_data = 3602 | (40002<<15) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_PTR1, read_data);

        read_data = 400000 | (136<<20) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_PTR3, read_data);

        read_data = 160000 | (1024<<18) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_PTR4, read_data);
        #endif
        }

    } else if (ddr_type== 2) { //LPDDR3
        read_data = 0x9 | (0x11<<8) | (0x27<<16) | (0x4<<24);
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTPR0, read_data);
    }



    //DSGCR ?
    //post-sim or chip testing maybe need adjust
    //[7:6] DQSGX
    //DQS Gate Extension: Specifies if set that the read DQS gate will be extended. This should be set ONLY when used with DQS pulldown and DQSn pullup.
    //Valid settings are:
    //00: Do not extend the gate
    //01: Extend the gate by 1/2 tCK in both directions (but never earlier than zero read latency)
    //10: Extend the gate earlier by 1/2 tCK and later by 2 * tCK (to facilitate LPDDR2/LPDDR3 usage without training for systems supporting upto 800Mbps)
    //11: Extend the gate earlier by 1/2 tCK and later by 3 * tCK (to facilitate LPDDR2/LPDDR3 usage without training for systems supporting up to 1600Mbps)
   /// read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_DSGCR);
   // //read_data[7:6] = 1;
   /// read_data = (read_data&0xFFFFFF3F)|(1L << 6) ;
   //// reg_write32(ddr_phy_base_addr+DDR_PHY_DSGCR, read_data);

    //ODTCR ?
    reg_write32(ddr_phy_base_addr+DDR_PHY_ODTCR, PHY_ODTCR);

    //AACR
    reg_write32(ddr_phy_base_addr+DDR_PHY_AACR, PHY_AACR);

    //---PHY data training config : DRCR0-1 (in the ddr_phy_train_cfg)!!!use default cfg
    //reg_write32(ddr_phy_base_addr+DDR_PHY_DTCR0, PHY_DTCR0);
    //reg_write32(ddr_phy_base_addr+DDR_PHY_DTCR1, PHY_DTCR1);
    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_DTCR0);
    read_data = (read_data&0xFFFFFFBF)| (1<<6) ; //!!!, [6] DTMPR =1
    reg_write32(ddr_phy_base_addr+DDR_PHY_DTCR0, read_data);

    //---PHY data training address config : DRAR0-3 (in the ddr_phy_train_cfg)!!!use default cfg
    //reg_write32(ddr_phy_base_addr+DDR_PHY_DTAR0, PHY_DTAR0);
    //reg_write32(ddr_phy_base_addr+DDR_PHY_DTAR1, PHY_DTAR1);
    //reg_write32(ddr_phy_base_addr+DDR_PHY_DTAR2, PHY_DTAR2);

    //----PHY AC IO config : ACIOCR0-4 , 2/4 is reserved
//    reg_write32(ddr_phy_base_addr+DDR_PHY_ACIOCR0, PHY_ACIOCR0);
//    reg_write32(ddr_phy_base_addr+DDR_PHY_ACIOCR1, PHY_ACIOCR1);
////    reg_write32(ddr_phy_base_addr+DDR_PHY_ACIOCR2, PHY_ACIOCR2);
//    reg_write32(ddr_phy_base_addr+DDR_PHY_ACIOCR3, PHY_ACIOCR3);
////    reg_write32(ddr_phy_base_addr+DDR_PHY_ACIOCR4, PHY_ACIOCR4);

//    //----PHY IO VREF config : IOVCR0-1
//    reg_write32(ddr_phy_base_addr+DDR_PHY_IOVCR0, PHY_IOVCR0);
   reg_write32(ddr_phy_base_addr+DDR_PHY_IOVCR1, PHY_IOVCR1);

    //----PHY misc config  DXnGCR0-4
//    reg_write32(ddr_phy_base_addr+DDR_PHY_DX0GCR0, PHY_DXnGCR0);
//    reg_write32(ddr_phy_base_addr+DDR_PHY_DX1GCR0, PHY_DXnGCR0);
//    reg_write32(ddr_phy_base_addr+DDR_PHY_DX2GCR0, PHY_DXnGCR0);
//    reg_write32(ddr_phy_base_addr+DDR_PHY_DX3GCR0, PHY_DXnGCR0);
////    reg_write32(ddr_phy_base_addr+DDR_PHY_DX4GCR0, PHY_DXnGCR0);
////    reg_write32(ddr_phy_base_addr+DDR_PHY_DX5GCR0, PHY_DXnGCR0);
////    reg_write32(ddr_phy_base_addr+DDR_PHY_DX6GCR0, PHY_DXnGCR0);
////    reg_write32(ddr_phy_base_addr+DDR_PHY_DX7GCR0, PHY_DXnGCR0);
//
//    reg_write32(ddr_phy_base_addr+DDR_PHY_DX0GCR1, PHY_DXnGCR1);
//    reg_write32(ddr_phy_base_addr+DDR_PHY_DX1GCR1, PHY_DXnGCR1);
//    reg_write32(ddr_phy_base_addr+DDR_PHY_DX2GCR1, PHY_DXnGCR1);
//    reg_write32(ddr_phy_base_addr+DDR_PHY_DX3GCR1, PHY_DXnGCR1);
////    reg_write32(ddr_phy_base_addr+DDR_PHY_DX4GCR1, PHY_DXnGCR1);
////    reg_write32(ddr_phy_base_addr+DDR_PHY_DX5GCR1, PHY_DXnGCR1);
////    reg_write32(ddr_phy_base_addr+DDR_PHY_DX6GCR1, PHY_DXnGCR1);
////    reg_write32(ddr_phy_base_addr+DDR_PHY_DX7GCR1, PHY_DXnGCR1);
//
//    reg_write32(ddr_phy_base_addr+DDR_PHY_DX0GCR2, PHY_DXnGCR2);
//    reg_write32(ddr_phy_base_addr+DDR_PHY_DX1GCR2, PHY_DXnGCR2);
//    reg_write32(ddr_phy_base_addr+DDR_PHY_DX2GCR2, PHY_DXnGCR2);
//    reg_write32(ddr_phy_base_addr+DDR_PHY_DX3GCR2, PHY_DXnGCR2);
////    reg_write32(ddr_phy_base_addr+DDR_PHY_DX4GCR2, PHY_DXnGCR2);
////    reg_write32(ddr_phy_base_addr+DDR_PHY_DX5GCR2, PHY_DXnGCR2);
////    reg_write32(ddr_phy_base_addr+DDR_PHY_DX6GCR2, PHY_DXnGCR2);
////    reg_write32(ddr_phy_base_addr+DDR_PHY_DX7GCR2, PHY_DXnGCR2);
//
//    reg_write32(ddr_phy_base_addr+DDR_PHY_DX0GCR3, PHY_DXnGCR3);
//    reg_write32(ddr_phy_base_addr+DDR_PHY_DX1GCR3, PHY_DXnGCR3);
//    reg_write32(ddr_phy_base_addr+DDR_PHY_DX2GCR3, PHY_DXnGCR3);
//    reg_write32(ddr_phy_base_addr+DDR_PHY_DX3GCR3, PHY_DXnGCR3);
////    reg_write32(ddr_phy_base_addr+DDR_PHY_DX4GCR3, PHY_DXnGCR3);
////    reg_write32(ddr_phy_base_addr+DDR_PHY_DX5GCR3, PHY_DXnGCR3);
////    reg_write32(ddr_phy_base_addr+DDR_PHY_DX6GCR3, PHY_DXnGCR3);
////    reg_write32(ddr_phy_base_addr+DDR_PHY_DX7GCR3, PHY_DXnGCR3);
//
    reg_write32(ddr_phy_base_addr+DDR_PHY_DX0GCR4, PHY_DXnGCR4);
    reg_write32(ddr_phy_base_addr+DDR_PHY_DX1GCR4, PHY_DXnGCR4);
    reg_write32(ddr_phy_base_addr+DDR_PHY_DX2GCR4, PHY_DXnGCR4);
    reg_write32(ddr_phy_base_addr+DDR_PHY_DX3GCR4, PHY_DXnGCR4);
////    reg_write32(ddr_phy_base_addr+DDR_PHY_DX4GCR4, PHY_DXnGCR4);
////    reg_write32(ddr_phy_base_addr+DDR_PHY_DX5GCR4, PHY_DXnGCR4);
////    reg_write32(ddr_phy_base_addr+DDR_PHY_DX6GCR4, PHY_DXnGCR4);
////    reg_write32(ddr_phy_base_addr+DDR_PHY_DX7GCR4, PHY_DXnGCR4);
    reg_write32(ddr_phy_base_addr+DDR_PHY_DX0GCR5, PHY_DXnGCR5);
    reg_write32(ddr_phy_base_addr+DDR_PHY_DX1GCR5, PHY_DXnGCR5);
    reg_write32(ddr_phy_base_addr+DDR_PHY_DX2GCR5, PHY_DXnGCR5);
    reg_write32(ddr_phy_base_addr+DDR_PHY_DX3GCR5, PHY_DXnGCR5);
    printf("DDRINFO: Set DXnGCR5 = %x \n",  PHY_DXnGCR5);
}

//----------------------------------------------------
//DDR PHY initialization main task
//----------------------------------------------------
//skip=1 will skip impedance calibration, speedy the RTL simulation,
//note: it can not enable for chip testing
void ddr_phy_init_cfg (unsigned int ddr_type , unsigned int speed,unsigned int skip,int ddr_phy_base_addr)
{
    //PHY basic register setting
    ddr_phy_main(ddr_type, speed,ddr_phy_base_addr);
    ddr_phy_timing(ddr_type, speed,1,ddr_phy_base_addr); //1 for RTL simulation speed up

    unsigned int read_data = 0 ;
    //trigger PHY init
    //[0] INIT Initialization Trigger
    //[1] ZCAL Impedance Calibration
    //[2] CA CA Training: Performs PHY LPDDR3 CA training
    //[4] PLLINIT PLL Initialization
    //[5] DCAL Digital Delay Line (DDL) Calibration
    //[6] PHYRST PHY Reset
    //[7] DRAMRST DRAM Reset Issues a reset to the DRAM
    //[8] DRAMINIT DRAM Initialization
    //[9] WL Write Leveling
    //[10] QSGATE Read DQS Gate Training
    //[11] WLADJ Write Leveling Adjust,Note: DDR4 and DDR3 only
    //[12] RDDSKW Read Data Bit Deskew
    //[13] WRDSKW Write Data Bit Deskew
    //[14] RDEYE Read Data Eye Training
    //[15] WREYE Write Data Eye Training
    //[16] SRD Static Read Training
    //[17] VREF VREF training,Note: VREF Training can be used with DDR4 only
    //[18] CTLDINIT Controller DRAM Initialization
    //[19] RDIMMINIT RDIMM Initialization
    //[29] DCALPSE Digital Delay Line (DDL) Calibration Pause
    //[30] ZCALBYP Impedance Calibration Bypass
    if (skip ==0) {
#ifdef DRAM_init_by_PHY
    read_data =   (1<<0)
                | (1<<1)
                | (1<<4)
                | (1<<5)
        | (1<<8)
        ;
    printf("DDRINTO: DRAM init by PHY \n");
#else
    read_data =   (1<<0)
                | (1<<1)
                | (1<<4)
                | (1<<5)
        ;
#endif
    } else {
    read_data =   (1<<0)
                | (1<<30)
                | (1<<4)
                | (1<<5);
    }

    reg_write32(ddr_phy_base_addr+DDR_PHY_PIR,read_data);

}

void ddr_phy_init_check (int ddr_phy_base_addr)
{
    int read_data = 0 ;
    //check initial done
    do {
        read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_PGSR0);
        read_data = read_data&0x00000001 ;
    }
    while(read_data != 1);

    //repeat(20) @(posedge sync_vif.clk);

    //tell PHY, let DMC do DRAM initialization, it will pull low PUB mode, then DMC DFI command can
    //send to DRAM device
    reg_write32(ddr_phy_base_addr+DDR_PHY_PIR,(1<<0) | (1<<18));
}


void ddr_phy_init (unsigned int ddr_type , unsigned int speed,unsigned int skip,int ddr_phy_base_addr)
{
ddr_phy_init_cfg ( ddr_type ,  speed, skip, ddr_phy_base_addr) ;
ddr_phy_init_check ( ddr_phy_base_addr) ;
print_task("ddr phy init done!\n");
}

//--------------DDRR vref training task ----------------------//
void ddr4_vref_training (int ddr_phy_base_addr )
{
int read_data = 0 ;
int tmp = 0;

print_task("kanzi: vref_training!\n");
    reg_write32(ddr_phy_base_addr+DDR_PHY_DCUTPR , 0xff);



    //3 disable refresh during data training
    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_DTCR0);
    read_data= read_data & 0x0FFFFFFF ; //[31:28] = 0;
    reg_write32(ddr_phy_base_addr+DDR_PHY_DTCR0 , read_data);

    //4, data
    reg_write32(ddr_phy_base_addr+DDR_PHY_BISTUDPR , 0xA5A5A5A5);

    //5, pattern select
    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_BISTRR);
    read_data= (read_data & 0xFFC1FFFF)| (0xe<<17) ; //[21:17] = E;
    reg_write32(ddr_phy_base_addr+DDR_PHY_BISTRR , read_data);

//    //6 ??? kanzi, will auto add 8
//    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_BISTAR1);
//    read_data= (read_data & 0xFFFF000F)| (0x8<<4) ; //[15:4] = E;
//    reg_write32(ddr_phy_base_addr+DDR_PHY_BISTAR1 , read_data);

    //7 ???

    //8, LCDL point
    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_VTCR1);
    //read_data= read_data | 0x2 ; //[2] = 1;
    read_data= read_data | 0x4 ; //[2] = 1;
    reg_write32(ddr_phy_base_addr+DDR_PHY_VTCR1 , read_data);

    //9, step
    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_VTCR1);
    read_data= read_data & 0x0FFFFFFF ; //[31:28] = 0;default
    read_data= (read_data &0xFFFFFF1F) |(5<<5) ; //tVRFEFIO[7:5] = 5; >200ns
    reg_write32(ddr_phy_base_addr+DDR_PHY_VTCR1 , read_data);

    //10, step
    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_VTCR0);
    read_data= read_data & 0xFFC3FFFF ; //[21:18] = 0; default
    read_data= (read_data &0x1FFFFFFF) |(5<<29) ; //tVRFEF[31:29] = 5; >150ns
    reg_write32(ddr_phy_base_addr+DDR_PHY_VTCR0 , read_data);

    //11, repeat
    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_VTCR1);
    read_data= (read_data &0xFFFF0FFF) | (0xe<<12) ; //[15:12] = F; on silicon
    //read_data= (read_data &0xFFFF0FFF) | (0x1<<12) ; //[15:12] = F; for sim past
    reg_write32(ddr_phy_base_addr+DDR_PHY_VTCR1 , read_data);

    //12/13
    read_data =   (1<<0)
                | (1<<17) ; //VREF training
    reg_write32(ddr_phy_base_addr+DDR_PHY_PIR,read_data);

    //14
    do {
       read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_PGSR0);
       tmp = read_data&0x1 ;
    }
    while(tmp!= 1);
    print_task("DDR PHY vref training done !\n");

    //read_data = (read_data & 0x00004000) |(read_data & 0x00080000) ; //[14] and [19]
    tmp = (read_data >> 19)&0x1 ; //[19]
    if(tmp == 0) { //bit[19]=0
    print_task("0928 DDR PHY vref training success !\n");
    } else {
    print_task("0928 DDR PHY vref training fail !\n");
    }

}

//vref_train:0:disable DDR4 vref training, 1:enable DDR4 vref training
//skip=1 will just run gate taining and LPDDR3 CA training
void ddr_phy_train_cfg (unsigned int ddr_type , unsigned int vref_train,unsigned int skip ,int ddr_phy_base_addr)
{
     unsigned int read_data = 0 ;
     read_data=0x0 ;

   if(ddr_type ==0){ //for DDR4
         //---ODT setting  : DXCCR
    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_DXCCR);
    read_data = read_data &0xFFFFE01F; //[12:9]=0000 [8:5] =0000
    read_data = read_data|(8L << 9);
    read_data = read_data|(1L << 20);
    reg_write32(ddr_phy_base_addr+DDR_PHY_DXCCR, read_data);
        printf("DDRINIFO: disconnect DQS for DDR4  DDR_PHY_DXCCR = 0x%x  \n",reg_read32(ddr_phy_base_addr+DDR_PHY_DXCCR));
   }

    //For DDR3/DDR4, use MPR mode for read training
    //ddr_type: 0(DDR4) 1(DDR3) 2(LPDDR3)
    //if ((ddr_type == 0) | (ddr_type == 1)) {
    if ((ddr_type == 0) | (ddr_type == 1)|(ddr_type == 2)) {
        read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_DTCR0);
        //////////read_data = read_data | (1<<6);//DTMPR
       /////////// read_data = read_data & 0xFFFFFFF0;//clear bit[3:0]
        ////////////read_data = read_data & 0xFFFFFF7F; // clear bit7
        read_data = read_data |(1L << 6);
       ///////////// read_data = read_data | 0x1;//DTRPTN, note:set to 1 for accelarate RTL sim, silicon test need modify
        reg_write32(ddr_phy_base_addr+DDR_PHY_DTCR0, read_data);
    }

    //----PHY data training address config : DTAR0-3
    //    reg_write32(ddr_phy_base_addr+DDR_PHY_DTAR0, read_data);


    //[0] INIT Initialization Trigger
    //[1] ZCAL Impedance Calibration
    //[2] CA CA Training: Performs PHY LPDDR3 CA training
    //[4] PLLINIT PLL Initialization
    //[5] DCAL Digital Delay Line (DDL) Calibration
    //[6] PHYRST PHY Reset
    //[7] DRAMRST DRAM Reset Issues a reset to the DRAM
    //[8] DRAMINIT DRAM Initialization
    //[9] WL Write Leveling
    //[10] QSGATE Read DQS Gate Training
    //[11] WLADJ Write Leveling Adjust,Note: DDR4 and DDR3 only
    //[12] RDDSKW Read Data Bit Deskew
    //[13] WRDSKW Write Data Bit Deskew
    //[14] RDEYE Read Data Eye Training
    //[15] WREYE Write Data Eye Training
    //[16] SRD Static Read Training
    //[17] VREF VREF training,Note: VREF Training can be used with DDR4 only
    //[18] CTLDINIT Controller DRAM Initialization
    //[19] RDIMMINIT RDIMM Initialization
    //[29] DCALPSE Digital Delay Line (DDL) Calibration Pause
    //[30] ZCALBYP Impedance Calibration Bypass
    if (skip==0) {
    read_data =   (1<<0)
                | (1<<9)
                | (1<<10)
                | (1<<11)
                | (1<<12)
                | (1<<13)
                | (1<<14)
                | (1<<15)
                | (1<<16) ;
    } else {
    read_data =   (1<<0)
                | (1<<10) ;
    }
     //ddr_type: 0(DDR4) 1(DDR3) 2(LPDDR3)
    if(ddr_type==2) {
        read_data = read_data | (1<<2);//CA training
    }

    reg_write32(ddr_phy_base_addr+DDR_PHY_PIR,read_data);
}

void ddr_phy_train_check (int ddr_phy_base_addr,unsigned int ddr_type , unsigned int vref_train)
{
    int read_data = 0 ;
    //check all initial flow done and no error
    do {
       read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_PGSR0);
       read_data = read_data&0x1 ;
    }
    while(read_data!= 1);

    printf("ddr_phy_train_check  read_data =0x%x \r\n",reg_read32(ddr_phy_base_addr+DDR_PHY_PGSR0));
    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_PGSR0);
    //check error
    read_data = read_data&0x7FF80000 ;//|read_data[30:19]
    if(read_data == 0) {
        print_task("DDR PHY training pass (without vref training)!\n");
    } else {
        print_task("DDR PHY training fail (without vref training)!\n");
    }

    if((ddr_type==0) && (vref_train == 1)) {
       ddr4_vref_training (ddr_phy_base_addr );
    }

#if 0
    //---ODT setting  : DXCCR
    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_DXCCR);
    read_data = read_data &0xFFFFE01F; //[12:9]=0000 [8:5] =0000
    reg_write32(ddr_phy_base_addr+DDR_PHY_DXCCR, read_data);
#endif

}

void ddr_phy_train (unsigned int ddr_type , unsigned int vref_train,unsigned int skip ,int ddr_phy_base_addr)
{
ddr_phy_train_cfg (ddr_type , vref_train, skip ,ddr_phy_base_addr);
ddr_phy_train_check (ddr_phy_base_addr,ddr_type,vref_train) ;
print_task("ddr tranining done!\n");
}


#ifdef DDR_LP_TEST
//----------------------------------------------------
//DDR PHY loopback test
//----------------------------------------------------
//bist_mode:0:AC bist, 1:DX bist
//lpbk_mode:0:PAD side loopback, 1:core siede loopback
void ddr_phy_lpbk_test (unsigned int bist_mode , unsigned int lpbk_mode,int ddr_phy_base_addr)
{
    unsigned int read_data = 0 ;

    //reset PHY read FIFO before DX loopback BIST by set PGCR0.PHYFRST
    if(bist_mode == 1) {

        reg_read32(ddr_phy_base_addr+DDR_PHY_PGCR0);
        read_data = read_data & (~(1<<26));
        reg_write32(ddr_phy_base_addr+DDR_PHY_PGCR0, read_data);
        //repeat(20) @(posedge sync_vif.clk);
        reg_read32(ddr_phy_base_addr+DDR_PHY_PGCR0);
        read_data = read_data | (1<<26);
        reg_write32(ddr_phy_base_addr+DDR_PHY_PGCR0, read_data);
        //repeat(20) @(posedge sync_vif.clk);
    }

    ////set RDQSD/RDQSND to 0x0
    //reg_write(`DDR_PHY_DX0LCDLR3, 32'h0);
    //reg_write(`DDR_PHY_DX0LCDLR4, 32'h0);
    //reg_write(`DDR_PHY_DX1LCDLR3, 32'h0);
    //reg_write(`DDR_PHY_DX1LCDLR4, 32'h0);
    //reg_write(`DDR_PHY_DX2LCDLR3, 32'h0);
    //reg_write(`DDR_PHY_DX2LCDLR4, 32'h0);
    //reg_write(`DDR_PHY_DX3LCDLR3, 32'h0);
    //reg_write(`DDR_PHY_DX3LCDLR4, 32'h0);

   //For AC bist and PAD side loopback mode, it need enable AC IO receiver,
    //AC IO receiver be power down default
    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_RIOCR0);
    //read_data[27:16] = 12'h0;//CSPDR
    read_data = read_data &0xF000FFFF ;
    reg_write32(ddr_phy_base_addr+DDR_PHY_RIOCR0, read_data);

    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_RIOCR1);
    //read_data[15:8]  = 8'h0;//CKEPDR
    //read_data[31:24] = 8'h0;//ODTPDR
    read_data =read_data & 0x00FF00FF ;
    reg_write32(ddr_phy_base_addr+DDR_PHY_RIOCR1, read_data);

    //set PHY/PUB into loopback mode
    //[27] IOLB I/O Loop-Back Select
    //[28] LBDQSS Loopback DQS Shift
    //[30:29] LBGDQS Loopback DQS Gating
    //[31] LBMODE Loopback Mode
    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_PGCR1);
    //read_data[27] = lpbk_mode;
    //read_data[28] = 0;//PUB set the read DQS LCDL to 0
    //read_data[30:29] = 2'b0;//DQS gate always on
    //read_data[31] = 1;
    read_data = (read_data&0x8EFFFFFF)|(lpbk_mode<<27) ;
    reg_write32(ddr_phy_base_addr+DDR_PHY_PGCR1,read_data);

    //set DXDDLLD[4:3] to 2'b11 for DX loopback
    if(bist_mode == 1) {
        read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_PGCR4);
        //read_data[20:19] = 2'b11;
        read_data = read_data | (3 < 19) ;
        reg_write32(ddr_phy_base_addr+DDR_PHY_PGCR4,read_data);
    }

    //set loopback mode
    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_DXCCR);
    if(lpbk_mode == 0) {
        //read_data[4:3] = 0;
        read_data = read_data & 0xFFFFFFE7 ;
    } else {
        //read_data[4:3] = 1;
        read_data = (read_data & 0xFFFFFFE7) | 0x00000008 ; ;
    }
    reg_write32(ddr_phy_base_addr+DDR_PHY_DXCCR,read_data);


    //LFSR seed config for random pattern
    reg_write32(ddr_phy_base_addr+DDR_PHY_BISTLSR,0x1234ABCD);//BIST LFSR Seed Register

    //word count setting, just can be multiple of 4
    reg_write32(ddr_phy_base_addr+DDR_PHY_BISTWCR,0x200);//BIST Word Count Register

    //run bist setting
    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_BISTRR);
    //read_data[2:0] = 3'h1;
    //read_data[3]   = 1'b0;//loopback mode
    read_data = (read_data & 0xFFFFFFF0) | 0x00000001 ;
    if(bist_mode==0) {
        //read_data[15:14] = 2'b10;//AC bist
        read_data = (read_data & 0xFFFF3FFF) | 0x00008000 ;
    } else {
        //read_data[15:14] = 2'b01;//DX bist
        read_data = (read_data & 0xFFFF3FFF) | 0x00004000 ;
    }

    //[16] BDMEN

    //[21:17] BDPAT
    //5'b00000 = PUB_DATA_0000_0000
    //5'b00001 = PUB_DATA_FFFF_FFFF
    //5'b00010 = PUB_DATA_5555_5555
    //5'b00011 = PUB_DATA_AAAA_AAAA
    //5'b00100 = PUB_DATA_0000_5500
    //5'b00101 = PUB_DATA_5555_0055
    //5'b00110 = PUB_DATA_0000_AA00
    //5'b00111 = PUB_DATA_AAAA_00AA
    //5'b01000 = PUB_DATA_DTDR0
    //5'b01001 = PUB_DATA_DTDR1
    //5'b01010 = PUB_DATA_UDDR0
    //5'b01011 = PUB_DATA_UDDR1
    //5'b01100 = PUB_DATA_WALKING_1
    //5'b01101 = PUB_DATA_WALKING_0
    //5'b01110 = PUB_DATA_USER_PATTERN
    //5'b01111 = PUB_DATA_LFSR
    //5'b10000 = PUB_DATA_SCHCR0
    //5'b10001 = PUB_DATA_FF00_FF00
    //5'b10010 = PUB_DATA_FFFF_0000
    //5'b10011 = PUB_DATA_0000_FF00
    //5'b10100 = PUB_DATA_FFFF_00FF
    //5'b10101 = PUB_DATA_00FF_00FF
    //5'b10110 = PUB_DATA_F0F0_F0F0
    //5'b10111 = PUB_DATA_0F0F_0F0F
    //read_data[21:17] = 5'hF;
      read_data = (read_data & 0xFFC0FFFF) | 0x001E0000 ;

    //[25:22] BDXSEL
    //[27:26] BCKSEL
    //[29:28] BCCSEL
    //[30]    BSOMA
    reg_write32(ddr_phy_base_addr+DDR_PHY_BISTRR,read_data);

    //bist running
    //repeat(2000) @(posedge sync_vif.clk);

    //stop bist
    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_BISTRR);
    //read_data[2:0] = 3'h2;
    read_data = (read_data &0xFFFFFFF8)|0x2 ;
    reg_write32(ddr_phy_base_addr+DDR_PHY_BISTRR,read_data);

    //Check BIST general status
    do {
        read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_BISTGSR);//BIST General Status Register
        read_data = read_data &0x1 ;
    } while(read_data != 0x1);

    //check is there any error, any error will print detail error log
    read_data = read_data&0x3FFFFFFE ;//  |read_data[29:1]
    if(read_data == 0) {
        print_task ( "DDR PHY loopback test pass!!!\n");
    }
    //else begin
    //    `uvm_error(get_name(), "DDR PHY loopback test Fail!!!\n")
    //    reg_read(`DDR_PHY_BISTGSR ,read_data);//BIST General Status Register
    //    `uvm_info(get_name(),$psprintf("DDR_PHY_BISTGSR=0x%0h\n",read_data),UVM_LOW)
    //    reg_read(`DDR_PHY_BISTWER0,read_data);//BIST Word Error Register 0
    //    `uvm_info(get_name(),$psprintf("DDR_PHY_BISTWER0=0x%0h\n",read_data),UVM_LOW)
    //    reg_read(`DDR_PHY_BISTWER1,read_data);//BIST Word Error Register 1
    //    `uvm_info(get_name(),$psprintf("DDR_PHY_BISTWER1=0x%0h\n",read_data),UVM_LOW)
    //    reg_read(`DDR_PHY_BISTBER0,read_data);//BIST Bit Error Register 0
    //    `uvm_info(get_name(),$psprintf("DDR_PHY_BISTBER0=0x%0h\n",read_data),UVM_LOW)
    //    reg_read(`DDR_PHY_BISTBER1,read_data);//BIST Bit Error Register 1
    //    `uvm_info(get_name(),$psprintf("DDR_PHY_BISTBER1=0x%0h\n",read_data),UVM_LOW)
    //    reg_read(`DDR_PHY_BISTBER2,read_data);//BIST Bit Error Register 2
    //    `uvm_info(get_name(),$psprintf("DDR_PHY_BISTBER2=0x%0h\n",read_data),UVM_LOW)
    //    reg_read(`DDR_PHY_BISTBER3,read_data);//BIST Bit Error Register 3
    //    `uvm_info(get_name(),$psprintf("DDR_PHY_BISTBER3=0x%0h\n",read_data),UVM_LOW)
    //    reg_read(`DDR_PHY_BISTBER4,read_data);//BIST Bit Error Register 4
    //    `uvm_info(get_name(),$psprintf("DDR_PHY_BISTBER4=0x%0h\n",read_data),UVM_LOW)
    //    reg_read(`DDR_PHY_BISTWCSR,read_data);//BIST Word Count Status Register
    //    `uvm_info(get_name(),$psprintf("DDR_PHY_BISTWCSR=0x%0h\n",read_data),UVM_LOW)
    //    reg_read(`DDR_PHY_BISTFWR0,read_data);//BIST Fail Word Register 0
    //    `uvm_info(get_name(),$psprintf("DDR_PHY_BISTFWR0=0x%0h\n",read_data),UVM_LOW)
    //    reg_read(`DDR_PHY_BISTFWR1,read_data);//BIST Fail Word Register 1
    //    `uvm_info(get_name(),$psprintf("DDR_PHY_BISTFWR1=0x%0h\n",read_data),UVM_LOW)
    //    reg_read(`DDR_PHY_BISTFWR2,read_data);//BIST Fail Word Register 2
    //    `uvm_info(get_name(),$psprintf("DDR_PHY_BISTFWR2=0x%0h\n",read_data),UVM_LOW)
    //    reg_read(`DDR_PHY_BISTBER5,read_data);//BIST Bit Error Register 5
    //    `uvm_info(get_name(),$psprintf("DDR_PHY_BISTBER5=0x%0h\n",read_data),UVM_LOW)
    //end

    //clear all BIST run time regsiter
    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_BISTRR);
    //read_data[2:0] = 3'h3;
    read_data = (read_data &0xFFFFFFF8)|0x3 ;
    reg_write32(ddr_phy_base_addr+DDR_PHY_BISTRR,read_data);

    //set PHY/PUB out loopback mode
    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_PGCR1);
    //read_data[31] = 0;
    read_data = read_data &0x7FFFFFFF ;
    reg_write32(ddr_phy_base_addr+DDR_PHY_PGCR1,read_data);

}
#endif


#ifdef DDR_DCU_TEST
//----------------------------------------------------
//DDR PHY DCU test
//----------------------------------------------------
//cache_sel,00=Command cache,01=Expected data cache,10=Read data cache
void set_dcu_cache_access(unsigned int cache_sel ,unsigned int inc_addr ,unsigned int acc_type ,unsigned int  word_addr ,unsigned int slice_addr,unsigned int ddr_phy_base_addr)
{
    int dcuar;
    dcuar  = word_addr
           | slice_addr<< 4
           | cache_sel << 8
           | inc_addr  << 10
           | acc_type  << 11;
    reg_write32(ddr_phy_base_addr+DDR_PHY_DCUAR, dcuar);
   // printf("set_dcu_cache_access data is %x \n",dcuar);
    //repeat(4) @(posedge sync_vif.clk);
} // set_dcu_cache_access

void load_dcu_command(unsigned int rpt ,
                      unsigned int dtp ,
                      unsigned int tag ,
                      unsigned int cmd ,
                      unsigned int bank,
                      unsigned int addr,
                      unsigned int mask,
                      unsigned int data,
                      unsigned int ddr_phy_base_addr)
{
   unsigned int  read_data ;
   unsigned int  cmd_bit1 ;
   unsigned int cmd_other_bit ;
   cmd_bit1 = cmd &0x1 ;
   cmd_other_bit = (cmd &0xFFFFFFFE)>>1  ;
   read_data =(cmd_bit1<<31)+(bank<<27)+(addr<<9)+(mask<<5)+data ;

  // printf("load_dcu_command first data is %x \n",read_data);
   reg_write32(ddr_phy_base_addr+DDR_PHY_DCUDR, read_data);

   read_data = (rpt<<10)+ (dtp<<5) + (tag<<3) + cmd_other_bit ;
  // printf("load_dcu_command second data is %x \n",read_data);
   reg_write32(ddr_phy_base_addr+DDR_PHY_DCUDR, read_data);

} // load_dcu_command

void load_expected_data (unsigned int data,unsigned int ddr_phy_base_addr)
{
    reg_write32(ddr_phy_base_addr+DDR_PHY_DCUDR, data);
    //repeat(4) @(posedge sync_vif.clk);
} // load_expected_data

///????????
void read_dcu_rdata(unsigned int ddr_phy_base_addr)
{
    int read_data = 0;
    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_DCUDR);
    //repeat(4) @(posedge sync_vif.clk);
    //$display("DCU read data value = 0x%h", read_data);
} // load_expected_data

void ddr_phy_dcu_test (unsigned int ddr_num)
{
    int ddr_phy_base_addr = 0xF9100000;
if (ddr_num==0) {
    ddr_phy_base_addr  = 0xF9100000 ;
} else if (ddr_num==1) {
    ddr_phy_base_addr  = 0xF9101000 ;
} else if (ddr_num==2) {
    ddr_phy_base_addr  = 0xF9102000 ;
} else if (ddr_num==3) {
    ddr_phy_base_addr  = 0xF9103000 ;
} else {
print_task("Un-spuuort DDR device number!\n");
}
     unsigned int read_data = 0;
    //implementation a simple DCU test sequence in the task
    //PREA->ACT->WR->RD->PREA
    //command code
    //4'b0000 NOP
    //4'b0101 PRECHARGE_ALL
    //4'b0110 ACTIVATE
    //4'b1000 WRITE
    //4'b1010 READ

    //configure user data pattern
    reg_write32(ddr_phy_base_addr+DDR_PHY_UDDR0, 0x76543210);//DATA0
    reg_write32(ddr_phy_base_addr+DDR_PHY_UDDR1, 0xFEDCBA98);//DATA1

    //----------------------------------------------------------------------
    //configuration DCU command cache
    //----------------------------------------------------------------------
    // Setup for loading DCU caches (with auto address increment)
    set_dcu_cache_access(DCU_CCACHE, 1, 0, 0, 0,ddr_phy_base_addr);
    load_dcu_command(DCU_NORPT, DTP_tRPA    , DCU_NOTAG , PRECHARGE_ALL  , 0, 0, 0xf, PUB_DATA_0000_0000,ddr_phy_base_addr);
    load_dcu_command(DCU_NORPT, DTP_tACT2RW , DCU_NOTAG , ACTIVATE       , 0, 0, 0xf, PUB_DATA_0000_0000,ddr_phy_base_addr);
    load_dcu_command(DCU_NORPT, DTP_tWR2RD  , DCU_NOTAG , SDRAM_WRITE    , 0, 0, 0xf, PUB_DATA_UDDR0    ,ddr_phy_base_addr);//BEAT0~3
    load_dcu_command(DCU_NORPT, DTP_tWR2RD  , DCU_NOTAG , SDRAM_WRITE    , 0, 0, 0xf, PUB_DATA_UDDR1    ,ddr_phy_base_addr);//BEAT4~7
    //load_dcu_command(DCU_NORPT, DTP_tRD2PRE , DCU_NOTAG , SDRAM_READ     , 0, 0, 0xf, PUB_DATA_0000_0000,ddr_phy_base_addr);

    load_dcu_command(DCU_NORPT, DTP_tRDAP2ACT , DCU_NOTAG , READ_PRECHG     , 0, 0, 0xf, PUB_DATA_0000_0000,ddr_phy_base_addr);

    //load_dcu_command(DCU_NORPT, DTP_tRPA    , DCU_NOTAG , PRECHARGE_ALL  , 0, 0, 0xf, PUB_DATA_0000_0000,ddr_phy_base_addr);
    //load_dcu_command(DCU_NORPT, DTP_tRP    , DCU_NOTAG , PRECHARGE  , 0, 0, 0xf, PUB_DATA_0000_0000,ddr_phy_base_addr);

    //----------------------------------------------------------------------
    //configuration DCU expected cache
    //----------------------------------------------------------------------
    set_dcu_cache_access(DCU_ECACHE, 1, 0, 0, 0,ddr_phy_base_addr);
    //read_data = (5'hA << (5*2)) | (5'hB << (5*3));
    //read_data = 0xA | (0xB << 5);
    read_data = PUB_DATA_UDDR0 | (PUB_DATA_UDDR1 << 5);
    load_expected_data(read_data,ddr_phy_base_addr);
    load_expected_data(0x0,ddr_phy_base_addr);
    load_expected_data(0x0,ddr_phy_base_addr);

    //----------------------------------------------------------------------
    //run DCU test
    //----------------------------------------------------------------------
    reg_write32(ddr_phy_base_addr+DDR_PHY_DCURR,   (1<<0 )   //DINST 1=run 2=stop
                              | (0<<4 )   //SADDR
                              | (11<<8 )  //EADDR
                              | (0<<12)   //NFAIL
                              | (0<<20)   //SONF
                              | (0<<21)   //SCOF
                              | (1<<22)   //RCEN,Read Capture Enable
                              | (1<<23)); //XCEN,Expected Compare Enable

    //----------------------------------------------------------------------
    //wait run done
    //----------------------------------------------------------------------
    do {
        read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_DCUSR0);
        read_data = read_data & 0x00000001 ;
    }
    //while(read_data[0]);
    while(read_data==0);
        print_task("DDR PHY DCU test done!\n");

    //read dcu read data
    set_dcu_cache_access(DCU_RCACHE, 1, 1, 0, 0,ddr_phy_base_addr);
    read_dcu_rdata(ddr_phy_base_addr);//read row 0
    read_dcu_rdata(ddr_phy_base_addr);//read row 1
    read_dcu_rdata(ddr_phy_base_addr);//read row 2
    read_dcu_rdata(ddr_phy_base_addr);//read row 3

    read_data =read_data &0x2 ;
    if(read_data == 0) {
         print_task ("DDR PHY DCU test pass!!!\n" ) ;
    }
    else {
        print_task("DDR PHY DCU test Fail!!!\n") ;
    }

    //----------------------------------------------------------------------
    //stop DCU test
    //----------------------------------------------------------------------
    reg_write32(ddr_phy_base_addr+DDR_PHY_DCURR,   (2<<0 )   //DINST 1=run 2=stop
                              | (0<<4 )   //SADDR
                              | (0<<8 )   //EADDR
                              | (0<<12)   //NFAIL
                              | (0<<20)   //SONF
                              | (0<<21)   //SCOF
                              | (1<<22)   //RCEN,Read Capture Enable
                              | (1<<23)); //XCEN,Expected Compare Enable

}
#endif

#ifdef DDR_BIST_TEST
//----------------------------------------------------
//DDR PHY memory BIST test
//----------------------------------------------------
void ddr_phy_bist_test (unsigned int ddr_phy_base_addr)
{
unsigned int read_data = 0 ;
print_task("phy bist config begin!\n");
//configuration BIST address
    reg_write32(ddr_phy_base_addr+DDR_PHY_BISTAR0,0x00000000);//BIST Address Register 0
    reg_write32(ddr_phy_base_addr+DDR_PHY_BISTAR1,0x00000080);//BIST Address Register 1
    reg_write32(ddr_phy_base_addr+DDR_PHY_BISTAR2,0x700003F8);//BIST Address Register 2,bit[11:0] BMCOL,bit[31:28] BMBANK
    reg_write32(ddr_phy_base_addr+DDR_PHY_BISTAR3,0x00000000);//BIST Address Register 3,bit[17:0] BROW
    reg_write32(ddr_phy_base_addr+DDR_PHY_BISTAR4,0x00000002);//BIST Address Register 4,bit[17:0] BMROW

    //bist user define pattern
    //[15:0] BUDP0 BIST User Data Pattern 0: Data to be applied on even DQ pins during BIST
    //[31:16] BUDP1 BIST User Data Pattern 1: Data to be applied on odd DQ pins during BIST
    reg_write32(ddr_phy_base_addr+DDR_PHY_BISTUDPR,0xFFFF0000);//BIST User Data Pattern Register

    //bist mask register
    reg_write32(ddr_phy_base_addr+DDR_PHY_BISTMSKR0,0x00000000);//BIST Mask Register 0
    reg_write32(ddr_phy_base_addr+DDR_PHY_BISTMSKR1,0x00000000);//BIST Mask Register 1
    reg_write32(ddr_phy_base_addr+DDR_PHY_BISTMSKR2,0x00000000);//BIST Mask Register 2

    //LFSR seed config for random pattern
    reg_write32(ddr_phy_base_addr+DDR_PHY_BISTLSR,0x1234ABCD);//BIST LFSR Seed Register

    //word count setting, just can be multiple of 4
    reg_write32(ddr_phy_base_addr+DDR_PHY_BISTWCR,0x20);//BIST Word Count Register

    //run bist setting
    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_BISTRR);
    //read_data[2:0] = 3'h1;
    //read_data[3]   = 1'b1;//DRAM mode
    read_data = (read_data & 0xFFFFFFF0) | 0x00000009 ;
    //read_data[15:14] = 2'b01;//DX bist, note just can support DX lane DRAM BIST
    read_data = (read_data & 0xFFFF3FFF) | 0x00004000 ;

    //[16] BDMEN

    //[21:17] BDPAT
    //5'b00000 = PUB_DATA_0000_0000
    //5'b00001 = PUB_DATA_FFFF_FFFF
    //5'b00010 = PUB_DATA_5555_5555
    //5'b00011 = PUB_DATA_AAAA_AAAA
    //5'b00100 = PUB_DATA_0000_5500
    //5'b00101 = PUB_DATA_5555_0055
    //5'b00110 = PUB_DATA_0000_AA00
    //5'b00111 = PUB_DATA_AAAA_00AA
    //5'b01000 = PUB_DATA_DTDR0
    //5'b01001 = PUB_DATA_DTDR1
    //5'b01010 = PUB_DATA_UDDR0
    //5'b01011 = PUB_DATA_UDDR1
    //5'b01100 = PUB_DATA_WALKING_1
    //5'b01101 = PUB_DATA_WALKING_0
    //5'b01110 = PUB_DATA_USER_PATTERN
    //5'b01111 = PUB_DATA_LFSR
    //5'b10000 = PUB_DATA_SCHCR0
    //5'b10001 = PUB_DATA_FF00_FF00
    //5'b10010 = PUB_DATA_FFFF_0000
    //5'b10011 = PUB_DATA_0000_FF00
    //5'b10100 = PUB_DATA_FFFF_00FF
    //5'b10101 = PUB_DATA_00FF_00FF
    //5'b10110 = PUB_DATA_F0F0_F0F0
    //5'b10111 = PUB_DATA_0F0F_0F0F

    //read_data[21:17] = 5'hF;
      read_data = (read_data & 0xFFC0FFFF) | 0x001E0000 ;

    //[25:22] BDXSEL
    //[27:26] BCKSEL
    //[29:28] BCCSEL
    //[30]    BSOMA
    reg_write32(ddr_phy_base_addr+DDR_PHY_BISTRR,read_data);

    //wait BIST done
    do {
        read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_BISTGSR);//BIST General Status Register
        read_data = read_data & 0x00000001 ;
    }
    while(read_data != 1);
        print_task("DDR PHY BIST test done!\n");

    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_BISTGSR);//BIST General Status Register
    //check is there any error, any error will print detail error log
    if((read_data & 0xFFFFFFFE) == 0) {
        print_task("DDR PHY BIST test pass!\n");
    }
    else {
        print_task("DDR PHY BIST test Fail!!!\n") ;
    }

    //stop bist
    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_BISTRR);
    //read_data[2:0] = 3'h2;
    read_data = (read_data&0xFFFFFFF8) | 0x2 ;
    reg_write32(ddr_phy_base_addr+DDR_PHY_BISTRR,read_data);

    //clear all BIST run time regsiter
    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_BISTRR);
    //read_data[2:0] = 3'h3;
    read_data = (read_data&0xFFFFFFF8) | 0x3 ;
    reg_write32(ddr_phy_base_addr+DDR_PHY_BISTRR,read_data);

}
#endif

//----------------------------------------------------
//DDR device power up initialization
//----------------------------------------------------
void ddr_device_init (int ddr_ctrl_base_addr)
{
    unsigned int read_data = 0 ;
    //wait DDR controller in normal operation mode
    read_data = reg_read32(ddr_ctrl_base_addr+DDR_CTRL_STAT);
    while(read_data != 0x00000001) {
       read_data = reg_read32(ddr_ctrl_base_addr+DDR_CTRL_STAT);
    }
    print_task("DRAM initial done!\n");
}

//----------------------------------------------------
//DDR device mode register setting
//----------------------------------------------------
//more reg set task
void mode_reg_set (int mr_addr,int mr_data,int ddr_ctrl_base_addr) {
 int status ;
    //set write data to mode reg
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_MRCTRL1,mr_data);

    //trigger mode register write
    //31:mr_wr=1
    //30:pba_mode=0
    //29:16:mr_cid=0
    //15:12:mr_addr=MRx reg address
    //11:4:mr_rank = 1 (cs)
    //3:sw_init_int=0
    //2:pba_en=0
    //1:mpr_en=0
    //0:mr_type=0(write)
    int mr_address_shift = mr_addr<<12 ;
    int data = 0x80000010 | mr_address_shift   ;
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_MRCTRL0,data);

    //check mr_wr_busy
    status = reg_read32(ddr_ctrl_base_addr+DDR_CTRL_MRSTAT);
    status = status&0x1 ;//read_data[0]
    while(status!=0) {
        status = reg_read32(ddr_ctrl_base_addr+DDR_CTRL_MRSTAT);
        status = status&0x1 ;//read_data[0]
    }
}

//----------------------------------------------------
//LPDDR device mode register read
//----------------------------------------------------
void lpddr_mode_reg_read (int mr_addr,int ddr_ctrl_base_addr) {
//mr_addr [7:0] ;
 // int pda = 0x0 ;
 // int mpr = 0x0 ;
 int cs=0x0;
 int status ;
    int mr_addr_mr_data = mr_addr<<8 ; // {mr_addr,mr_data}
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_MRCTRL1,mr_addr_mr_data);
    //trigger mode register write
    int cs_shift = 0x1<<cs ;
    int cs_shift2= (cs_shift<<4)+1  ;
    int data = 0x80000000 | cs_shift2 ; //{1'b1,25'h0,2'b01<<cs,1'b0,pda,mpr,1'b1}
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_MRCTRL0,data);
    //check mr_wr_busy
    status = reg_read32(ddr_ctrl_base_addr+DDR_CTRL_MRSTAT);
    status = status&0x1 ;//read_data[0]
    while(status!=0) {
        status = reg_read32(ddr_ctrl_base_addr+DDR_CTRL_MRSTAT);
        status = status&0x1 ;//read_data[0]
    }
}

//----------------------------------------------------
//DDR4 device mode register read
//----------------------------------------------------
void ddr4_mode_reg_read (int mr_addr,int ddr_ctrl_base_addr) {
//mr_addr [7:0] ;
 int pda = 0x0 ;
 int mpr = 0x1 ;
 int cs=0x0;
 int status ;

    status = reg_read32(ddr_ctrl_base_addr+DDR_CTRL_MRCTRL1) ;
    status = status & 0xFFFFFFFC ;
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_MRCTRL1, status); //[1:0]= 0

    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_MRCTRL0,(1<<0) //mr_type=read
                                                   |(mpr<<1)
                                                   |(pda<<2)
                                                   |(cs<<4)
                                                   |(mr_addr<<12)
                                                   |(1<<31)
                                                   );
    //check mr_wr_busy
    status = reg_read32(ddr_ctrl_base_addr+DDR_CTRL_MRSTAT);
    status = status&0x1 ;//read_data[0]
    while(status!=0) {
        status = reg_read32(ddr_ctrl_base_addr+DDR_CTRL_MRSTAT);
        status = status&0x1 ;//read_data[0]
    }
}



//----------------------------------------------------
//LPDDR device mode register set
//----------------------------------------------------
void lpddr_mode_reg_set (int mr_addr,int mr_data,int ddr_ctrl_base_addr) {
 // int pda = 0x0 ;
 // int mpr = 0x0 ;
    int cs=0x0 ;
 int status ;
    int mr_addr_mr_data = (mr_addr<<8) + mr_data ; // {mr_addr,mr_data}
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_MRCTRL1,mr_addr_mr_data);
    //trigger mode register write
    int cs_shift = 0x1<<cs ;
    int cs_shift2= cs_shift<<4 ;
    int data = 0x80000000 | cs_shift2 ; //{1'b1,25'h0,2'b01<<cs,1'b0,pda,mpr,1'b0}
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_MRCTRL0,data);
    //check mr_wr_busy
    status = reg_read32(ddr_ctrl_base_addr+DDR_CTRL_MRSTAT);
    status = status&0x1 ;//read_data[0]
    while(status!=0) {
        status = reg_read32(ddr_ctrl_base_addr+DDR_CTRL_MRSTAT);
        status = status&0x1 ;//read_data[0]
    }
}

//----------------------------------------------------
//DDR subsystem half data path setting
//----------------------------------------------------
void ddr_half_path (int ddr_ctrl_base_addr,int ddr_phy_base_addr)
{
    unsigned int read_data = 0 ;
    //enable DMC half data path
    read_data = reg_read32(ddr_ctrl_base_addr+DDR_CTRL_MSTR);
    //read_data[13:12] = 1;
    read_data = (read_data & 0xFFFFCFFF) | 0x00001000 ;
    reg_write32(ddr_ctrl_base_addr+DDR_CTRL_MSTR, read_data);

    //set DXEN, disble the higher two byte
    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_DX2GCR0);
    //read_data[0] = 0;
    read_data = read_data & 0xFFFFFFFE ;
    reg_write32(ddr_phy_base_addr+DDR_PHY_DX2GCR0, read_data);

    read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_DX3GCR0);
    //read_data[0] = 0;
    read_data = read_data & 0xFFFFFFFE ;
    reg_write32(ddr_phy_base_addr+DDR_PHY_DX3GCR0, read_data);
}

//----------------------------------------------------
//PHY side DFI PHY update disable
//----------------------------------------------------
void phy_dfi_phyupd_dis(int ddr_ctrl_base_addr)
{
    unsigned int read_data = 0;

    read_data = reg_read32(ddr_ctrl_base_addr+DDR_PHY_DSGCR);
    read_data = read_data | 0x1;//PUREN
    reg_write32(ddr_ctrl_base_addr+DDR_PHY_DSGCR, read_data);
}

//----------------------------------------------------
//PHY side DFI PHY update enable
//----------------------------------------------------
void phy_dfi_phyupd_en(int ddr_ctrl_base_addr)
{
    unsigned int read_data = 0;

    read_data = reg_read32(ddr_ctrl_base_addr+DDR_PHY_DSGCR);
    read_data = read_data & 0xFFFFFFFE;//PUREN
    reg_write32(ddr_ctrl_base_addr+DDR_PHY_DSGCR,read_data);
}




//-------------------------------------------------------------------------------------//
//--------------------------part 5 : DDR initial task----------------------------------//
//-------------------------------------------------------------------------------------//
//ddr_type: 0(DDR4),1(DDR3),2(LPDDR3),3(LPDDR2)
//speed: DDR speed bin, 0(2400) 1(2133) 2(1600) 3(1333)
//capacity: DDR space capacity 0(1GB),1(2GB),2(4GB)
//dev_type: DDR device type setting, 0(x4) 1(x8) 2(x16) 3(x32), just for DDR4

#ifdef DDR4_2400
//--------------------------------------------------------------//
void ddr_one_initial_flow_training (unsigned int ddr_num)
{
    int ddr_ctrl_base_addr = 0xF9000000;
    int ddr_phy_base_addr = 0xF9100000;
if (ddr_num==0) {
    ddr_ctrl_base_addr = 0xF9000000 ;
    ddr_phy_base_addr  = 0xF9100000 ;
} else if (ddr_num==1) {
    ddr_ctrl_base_addr = 0xF9010000 ;
    ddr_phy_base_addr  = 0xF9101000 ;
} else if (ddr_num==2) {
    ddr_ctrl_base_addr = 0xF9020000 ;
    ddr_phy_base_addr  = 0xF9102000 ;
} else if (ddr_num==3) {
    ddr_ctrl_base_addr = 0xF9030000 ;
    ddr_phy_base_addr  = 0xF9103000 ;
} else {
print_task("Un-spuuort DDR device number!\n");
}
unsigned int ddr_type ;
unsigned int speed ;
unsigned int capacity ;
unsigned int dev_type ;
unsigned int skip_training ;
unsigned int vref_training ;
ddr_type = 0 ;
speed = 0 ;
capacity = 0 ;
dev_type = 1 ;
skip_training = 0 ;
vref_training =1 ;

print_task(" one DDR device initiall begin !\n");
apb_reset_release(ddr_num);
dfi_init_complete_dis (ddr_ctrl_base_addr) ;

dmc_init(ddr_type, speed, capacity, dev_type,ddr_ctrl_base_addr);

dmc_reset_release(ddr_num);
phy_reset_release(ddr_num);

ddr_phy_init(ddr_type,speed,skip_training,ddr_phy_base_addr); //skip training

dfi_init_complete_en (ddr_ctrl_base_addr) ;
ddr_device_init(ddr_ctrl_base_addr);

ddr_phy_train(ddr_type,vref_training,skip_training,ddr_phy_base_addr);  //skip training

axi_reset_release(ddr_num);
reg_write32(0xF9200010 , 0x0000001F);  //release noc reset
print_task(" one DDR device initiall end !\n");
}

void chiptest_initial_ddr4_2400 (void)
{
//change frequency ,change pll config  0xf970_0030
//ddr4_2400 frequency is 600M,ref is 24 , 24/4=6 6*200=1200 1200/2=600
//   ddr_pll_refdiv   [5:0]; 4
//   ddr_pll_fbdiv    [19:8]; 200
//   ddr_pll_postdiv1 [22:20]; 2
//   ddr_pll_postdiv2 [26:24]; 1
reg_write32(0xf9700030,   (4<<0 )   // refdiv=4
                        | (199<<8 )   // fbdiv=176
                        | (2<<20 ) // postdiv1= 2
                        | (1<<24)); // postdiv2=1
int status ;
do {
      status = reg_read32(0xF970003C);
      status = status&0x1 ;  //bit 0 ,pll lock
       }
while(status==0x0);//wait re-lock
  print_task(" pll relocked !\n");

ddr_one_initial_flow_training (0);
}
#endif


#ifdef DDR4_2133
//--------------------------------------------------------------//
void ddr_one_initial_flow_training_2133 (unsigned int ddr_num)
{
    int ddr_ctrl_base_addr = 0xF9000000;
    int ddr_phy_base_addr = 0xF9100000;
if (ddr_num==0) {
    ddr_ctrl_base_addr = 0xF9000000 ;
    ddr_phy_base_addr  = 0xF9100000 ;
} else if (ddr_num==1) {
    ddr_ctrl_base_addr = 0xF9010000 ;
    ddr_phy_base_addr  = 0xF9101000 ;
} else if (ddr_num==2) {
    ddr_ctrl_base_addr = 0xF9020000 ;
    ddr_phy_base_addr  = 0xF9102000 ;
} else if (ddr_num==3) {
    ddr_ctrl_base_addr = 0xF9030000 ;
    ddr_phy_base_addr  = 0xF9103000 ;
} else {
print_task("Un-spuuort DDR device number!\n");
}
unsigned int ddr_type ;
unsigned int speed ;
unsigned int capacity ;
unsigned int dev_type ;
unsigned int skip_training ;
unsigned int vref_training ;
ddr_type = 0 ;
speed = 1 ;
capacity = 0 ;
dev_type = 1 ;
skip_training = 0 ;
vref_training =1 ;

print_task(" one DDR device initiall begin !\n");
apb_reset_release(ddr_num);
dfi_init_complete_dis (ddr_ctrl_base_addr) ;

dmc_init(ddr_type, speed, capacity, dev_type,ddr_ctrl_base_addr);

dmc_reset_release(ddr_num);
phy_reset_release(ddr_num);

ddr_phy_init(ddr_type,speed,skip_training,ddr_phy_base_addr); //skip training

dfi_init_complete_en (ddr_ctrl_base_addr) ;
ddr_device_init(ddr_ctrl_base_addr);

ddr_phy_train(ddr_type,vref_training,skip_training,ddr_phy_base_addr);  //skip training

axi_reset_release(ddr_num);
reg_write32(0xF9200010 , 0x0000001F);  //release noc reset
print_task(" one DDR device initiall end !\n");
}

void chiptest_initial_ddr4_2133 (void)
{
//change frequency ,change pll config  0xf970_0030
//ddr4_2133, frequency is 533M,ref is 24 , 24/4=6 6*177=1062 1062/2=531
//   ddr_pll_refdiv   [5:0]; 4
//   ddr_pll_fbdiv    [19:8]; 177
//   ddr_pll_postdiv1 [22:20]; 2
//   ddr_pll_postdiv2 [26:24]; 1

#if CONFIG_DDR4_2400_SUPPORT //2400
    print_task(" _________________2400___________________ !\n");
 reg_write32(0xf9700030,   (4<<0 )   // refdiv=4
                        | (199<<8 )   // fbdiv=176
                        | (2<<20 ) // postdiv1= 2
                        | (1<<24)); // postdiv2=1
#else
    print_task(" _________________2133___________________ !\n");
reg_write32(0xf9700030,   (3<<0 )   // refdiv=4
                        | (133<<8 )   // fbdiv=176
                        | (2<<20 ) // postdiv1= 2
                        | (1<<24)); // postdiv2=1

#endif
int status ;
do {
      status = reg_read32(0xF970003C);
      status = status&0x1 ;  //bit 0 ,pll lock
       }
while(status==0x0);//wait re-lock
  print_task(" pll relocked !\n");

    int i;
    for (i = 0; i < CONFIG_NR_DDR_CHANNELS; i++) {
        ddr_one_initial_flow_training_2133(i);
        if (i == 0)
printf("DDR_PHY_DX0GCR5 = %x\r\n",reg_read32(0xF9100000+DDR_PHY_DX0GCR5));
    }

#ifdef CONFIG_CHANNEL_IN_LOW_POWER
    for (i = CONFIG_NR_DDR_CHANNELS; i < 4; i++) {
        ddr_one_initial_flow_training_2133(i);
        ddr_low_power(i);
    }
#endif
}
#endif


#ifdef DDR4_1600
//--------------------------------------------------------------//
void ddr_one_initial_flow_training_1600 (unsigned int ddr_num)
{
    int ddr_ctrl_base_addr = 0xF9000000;
    int ddr_phy_base_addr = 0xF9100000;
if (ddr_num==0) {
    ddr_ctrl_base_addr = 0xF9000000 ;
    ddr_phy_base_addr  = 0xF9100000 ;
} else if (ddr_num==1) {
    ddr_ctrl_base_addr = 0xF9010000 ;
    ddr_phy_base_addr  = 0xF9101000 ;
} else if (ddr_num==2) {
    ddr_ctrl_base_addr = 0xF9020000 ;
    ddr_phy_base_addr  = 0xF9102000 ;
} else if (ddr_num==3) {
    ddr_ctrl_base_addr = 0xF9030000 ;
    ddr_phy_base_addr  = 0xF9103000 ;
} else {
print_task("Un-spuuort DDR device number!\n");
}
unsigned int ddr_type ;
unsigned int speed ;
unsigned int capacity ;
unsigned int dev_type ;
unsigned int skip_training ;
unsigned int vref_training ;
ddr_type = 0 ;
speed = 2 ;
capacity = 0 ;
dev_type = 1 ;
skip_training = 0 ;
vref_training =1 ;

print_task(" one DDR device initiall begin !\n");
apb_reset_release(ddr_num);
dfi_init_complete_dis (ddr_ctrl_base_addr) ;

dmc_init(ddr_type, speed, capacity, dev_type,ddr_ctrl_base_addr);

dmc_reset_release(ddr_num);
phy_reset_release(ddr_num);

ddr_phy_init(ddr_type,speed,skip_training,ddr_phy_base_addr); //skip training

dfi_init_complete_en (ddr_ctrl_base_addr) ;
ddr_device_init(ddr_ctrl_base_addr);

ddr_phy_train(ddr_type,vref_training,skip_training,ddr_phy_base_addr);  //skip training

axi_reset_release(ddr_num);
reg_write32(0xF9200010 , 0x0000001F);  //release noc reset
print_task(" one DDR device initiall end !\n");
}

void chiptest_initial_ddr4_1600 (void)
{
    print_task(" begin  chiptest_initial_ddr4_1600   !\n");

//change frequency ,change pll config  0xf970_0030
//ddr4_1600 frequency is 400M,ref is 24 , 24/4=6 6*200=1200 1200/3=400
//   ddr_pll_refdiv   [5:0]; 4
//   ddr_pll_fbdiv    [19:8]; 200
//   ddr_pll_postdiv1 [22:20]; 3
//   ddr_pll_postdiv2 [26:24]; 1

#ifdef __DDR4_2133M__
reg_write32(0xf9700030,   (3<<0 )   // refdiv=4
                        | (133<<8 )   // fbdiv=176
                        | (2<<20 ) // postdiv1= 2
                        | (1<<24)); // postdiv2=1
#endif

#ifdef __DDR4_2000M__
 reg_write32(0xf9700030,   (4<<0 )   // refdiv=4
                        | (167<<8 )   // fbdiv=176
                        | (2<<20 ) // postdiv1= 2
                        | (1<<24)); // postdiv2=1
#endif

#ifdef __DDR4_1800M__
    reg_write32(0xf9700030,   (4<<0 )   // refdiv=4
                        | (150<<8 )   // fbdiv=176
                        | (2<<20 ) // postdiv1= 2
                        | (1<<24)); // postdiv2=1
#endif

#ifdef __DDR4_1600M__
    print_task(" _________________1600___________________ !\n");
    reg_write32(0xf9700030,   (3<<0 )   // refdiv=4
                            | (200<<8 )   // fbdiv=176
                            | (2<<20 ) // postdiv1= 2
                            | (2<<24)); // postdiv2=1
#endif

#ifdef __DDR4_1066M__
    print_task(" _________________1066___________________ !\n");
reg_write32(0xf9700030,   (3<<0 )   // refdiv=4
                        | (133<<8 )   // fbdiv=176
                        | (2<<20 ) // postdiv1= 2
                        | (2<<24)); // postdiv2=1
#endif

#ifdef __DDR4_800M__
    print_task(" _________________800___________________ !\n");
    reg_write32(0xf9700030,   (3<<0 )   // refdiv=4
                            | (200<<8 )   // fbdiv=176
                            | (4<<20 ) // postdiv1= 2
                            | (2<<24)); // postdiv2=1
#endif

    printf(" 0xf9700030  = 0x%x \r\n",reg_read32(0xf9700030));
int status ;
do {
      status = reg_read32(0xF970003C);
      status = status&0x1 ;  //bit 0 ,pll lock
}while(status==0x0);//wait re-lock
  print_task(" pll relocked !\n");

    int i;
    for (i = 0; i < CONFIG_NR_DDR_CHANNELS; i++) {
        ddr_one_initial_flow_training_1600(i);
        if (i == 0)
printf("DDR_PHY_DX0GCR5 = %x\r\n",reg_read32(0xF9100000+DDR_PHY_DX0GCR5));
    }

#ifdef CONFIG_CHANNEL_IN_LOW_POWER
    for (i = CONFIG_NR_DDR_CHANNELS; i < 4; i++) {
        ddr_one_initial_flow_training_1600(i);
        ddr_low_power(i);
    }
#endif
}

#endif


#ifdef DDR3_1600
//-----------------ddr3 initial flow-----------------------------------//
void ddr_one_initial_flow_ddr3 (unsigned int ddr_num)
{
    int ddr_ctrl_base_addr = 0xF9000000;
    int ddr_phy_base_addr = 0xF9100000;
if (ddr_num==0) {
    ddr_ctrl_base_addr = 0xF9000000 ;
    ddr_phy_base_addr  = 0xF9100000 ;
} else if (ddr_num==1) {
    ddr_ctrl_base_addr = 0xF9010000 ;
    ddr_phy_base_addr  = 0xF9101000 ;
} else if (ddr_num==2) {
    ddr_ctrl_base_addr = 0xF9020000 ;
    ddr_phy_base_addr  = 0xF9102000 ;
} else if (ddr_num==3) {
    ddr_ctrl_base_addr = 0xF9030000 ;
    ddr_phy_base_addr  = 0xF9103000 ;
} else {
print_task("Un-spuuort DDR device number!\n");
}
unsigned int ddr_type ;
unsigned int speed ;
unsigned int capacity ;
unsigned int dev_type ;
unsigned int skip_training ;
unsigned int vref_training ;
ddr_type = 1 ;
speed = 2 ;
capacity = 0 ;
dev_type = 1 ;
skip_training = 0 ;
vref_training = 0  ;

print_task(" one DDR device initiall begin !\n");
apb_reset_release(ddr_num);
dfi_init_complete_dis (ddr_ctrl_base_addr) ;

dmc_init(ddr_type, speed, capacity, dev_type,ddr_ctrl_base_addr);

dmc_reset_release(ddr_num);
phy_reset_release(ddr_num);

ddr_phy_init(ddr_type,speed,skip_training,ddr_phy_base_addr); //skip training

dfi_init_complete_en (ddr_ctrl_base_addr) ;
ddr_device_init(ddr_ctrl_base_addr);

ddr_phy_train(ddr_type,vref_training,skip_training,ddr_phy_base_addr);  //skip training

axi_reset_release(ddr_num);
reg_write32(0xF9200010 , 0x0000001F);  //release noc reset
print_task(" one DDR device initiall end !\n");
}

void chiptest_initial_ddr3_1600 (void)
{
//change frequency ,change pll config  0xf970_0030
//ddr3_1600, frequency is 400M,ref is 24 , 24/4=6 6*200=1200 1200/3=400
//   ddr_pll_refdiv   [5:0]; 4
//   ddr_pll_fbdiv    [19:8]; 200
//   ddr_pll_postdiv1 [22:20]; 2
//   ddr_pll_postdiv2 [26:24]; 2
reg_write32(0xf9700030,   (4<<0 )   // refdiv=4
                        | (198<<8 )   // fbdiv=200
                        | (3<<20 ) // postdiv1= 3
                        | (1<<24)); // postdiv2=1
int status ;
do {
      status = reg_read32(0xF970003C);
      status = status&0x1 ;  //bit 0 ,pll lock
       }
while(status==0x0);//wait re-lock
  print_task(" pll relocked !\n");

    int i;
    for (i = 0; i < CONFIG_NR_DDR_CHANNELS; i++) {
        ddr_one_initial_flow_ddr3 (i);
    }

#ifdef CONFIG_CHANNEL_IN_LOW_POWER
    for (i = CONFIG_NR_DDR_CHANNELS; i < 4; i++) {
        ddr_one_initial_flow_ddr3 (i);
        ddr_low_power(i);
    }
#endif
}




#endif

#ifdef DDR3_800
void ddr_one_initial_flow_ddr3_800 (unsigned int ddr_num)
{
    int ddr_ctrl_base_addr = 0xF9000000;
    int ddr_phy_base_addr = 0xF9100000;
if (ddr_num==0) {
    ddr_ctrl_base_addr = 0xF9000000 ;
    ddr_phy_base_addr  = 0xF9100000 ;
} else if (ddr_num==1) {
    ddr_ctrl_base_addr = 0xF9010000 ;
    ddr_phy_base_addr  = 0xF9101000 ;
} else if (ddr_num==2) {
    ddr_ctrl_base_addr = 0xF9020000 ;
    ddr_phy_base_addr  = 0xF9102000 ;
} else if (ddr_num==3) {
    ddr_ctrl_base_addr = 0xF9030000 ;
    ddr_phy_base_addr  = 0xF9103000 ;
} else {
print_task("Un-spuuort DDR device number!\n");
}
unsigned int ddr_type ;
unsigned int speed ;
unsigned int capacity ;
unsigned int dev_type ;
unsigned int skip_training ;
unsigned int vref_training ;
ddr_type = 1 ;
speed = 3 ;
capacity = 0 ;
dev_type = 1 ;
skip_training = 0 ;
vref_training = 1 ;

print_task(" one DDR device initiall begin !\n");
apb_reset_release(ddr_num);
dfi_init_complete_dis (ddr_ctrl_base_addr) ;

dmc_init(ddr_type, speed, capacity, dev_type,ddr_ctrl_base_addr);

dmc_reset_release(ddr_num);
phy_reset_release(ddr_num);

ddr_phy_init(ddr_type,speed,skip_training,ddr_phy_base_addr); //skip training

dfi_init_complete_en (ddr_ctrl_base_addr) ;
ddr_device_init(ddr_ctrl_base_addr);

ddr_phy_train(ddr_type,vref_training,skip_training,ddr_phy_base_addr);  //skip training

axi_reset_release(ddr_num);
reg_write32(0xF9200010 , 0x0000001F);  //release noc reset
print_task(" one DDR device initiall end !\n");
}

void chiptest_initial_ddr3_800 (void)
{
//change frequency ,change pll config  0xf970_0030
//ddr3_800, frequency is 200M,ref is 24 , 24/4=6 6*200=1200 1200/6=200
//   ddr_pll_refdiv   [5:0]; 4
//   ddr_pll_fbdiv    [19:8]; 200
//   ddr_pll_postdiv1 [22:20]; 2
//   ddr_pll_postdiv2 [26:24]; 2
reg_write32(0xf9700030,   (4<<0 )   // refdiv=4
                        | (198<<8 )   // fbdiv=200
                        | (3<<20 ) // postdiv1= 3
                        | (2<<24)); // postdiv2=2
int status ;
do {
      status = reg_read32(0xF970003C);
      status = status&0x1 ;  //bit 0 ,pll lock
       }
while(status==0x0);//wait re-lock
  print_task(" pll relocked !\n");

    int i;
    for (i = 0; i < CONFIG_NR_DDR_CHANNELS; i++) {
        ddr_one_initial_flow_ddr3_800 (i);
    }
#ifdef CONFIG_CHANNEL_IN_LOW_POWER
    for (i = CONFIG_NR_DDR_CHANNELS; i < 4; i++) {
        ddr_one_initial_flow_ddr3_800 (i);
        ddr_low_power(i);
    }
#endif
}


#endif

#ifdef LPDDR3_1866
//-----------------lpddr3 initial flow-----------------------------------//
void ddr_one_initial_flow_lpddr3 (unsigned int ddr_num)
{
    int ddr_ctrl_base_addr = 0xF9000000;
    int ddr_phy_base_addr = 0xF9100000;
if (ddr_num==0) {
    ddr_ctrl_base_addr = 0xF9000000 ;
    ddr_phy_base_addr  = 0xF9100000 ;
} else if (ddr_num==1) {
    ddr_ctrl_base_addr = 0xF9010000 ;
    ddr_phy_base_addr  = 0xF9101000 ;
} else if (ddr_num==2) {
    ddr_ctrl_base_addr = 0xF9020000 ;
    ddr_phy_base_addr  = 0xF9102000 ;
} else if (ddr_num==3) {
    ddr_ctrl_base_addr = 0xF9030000 ;
    ddr_phy_base_addr  = 0xF9103000 ;
} else {
print_task("Un-spuuort DDR device number!\n");
}
unsigned int ddr_type ;
unsigned int speed ;
unsigned int capacity ;
unsigned int dev_type ;
unsigned int skip ;
ddr_type = 2 ;
speed = 0 ;
capacity = 0 ; //1GB
dev_type = 3 ; //x32
skip = 1 ;

//change frequency ,change pll config  0xf970_0030
//lpddr3_1866, frequency is 466M,ref is 24 , 24/4=6 6*233=1398 1398/3=466
//   ddr_pll_refdiv   [5:0]; 4
//   ddr_pll_fbdiv    [19:8]; 200
//   ddr_pll_postdiv1 [22:20]; 2
//   ddr_pll_postdiv2 [26:24]; 2
reg_write32(0xf9700030,   (4<<0 )   // refdiv=4
                        | (232<<8 )   // fbdiv=233
                        | (3<<20 ) // postdiv1= 3
                        | (1<<24)); // postdiv2=1
//After change PLL setting, it maybe the PLL clock status still not be update,
//so we first wait lock status pull low, then check the lock status pull high
//again
int status ;
//do {
//      status = reg_read32(0xF970003C);
//      status = status&0x1 ;  //bit 0 ,pll lock
//       }
//while(status==0x1);//wait un-lock
do {
      status = reg_read32(0xF970003C);
      status = status&0x1 ;  //bit 0 ,pll lock
       }
while(status==0x0);//wait re-lock
  print_task(" pll relocked !\n");

print_task(" one DDR device initiall begin !\n");
apb_reset_release(ddr_num);
dfi_init_complete_dis (ddr_ctrl_base_addr) ;

dmc_init(ddr_type, speed, capacity, dev_type,ddr_ctrl_base_addr);

dmc_reset_release(ddr_num);
phy_reset_release(ddr_num);

ddr_phy_init(ddr_type,speed,skip,ddr_phy_base_addr); //skip training

dfi_init_complete_en (ddr_ctrl_base_addr) ;
ddr_device_init(ddr_ctrl_base_addr);

ddr_phy_train(ddr_type,0,skip,ddr_phy_base_addr);  //skip training

axi_reset_release(ddr_num);
reg_write32(0xF9200010 , 0x0000001F);  //release noc reset
print_task(" one DDR device initiall end !\n");
}
#endif


//--------------------------------------------------------------//
void ddr_one_initial_flow_half_data (unsigned int ddr_num)
{
    int ddr_ctrl_base_addr = 0xF9000000;
    int ddr_phy_base_addr = 0xF9100000;
    int ddr_half_config_addr = 0xF9200018;
if (ddr_num==0) {
    ddr_ctrl_base_addr = 0xF9000000 ;
    ddr_phy_base_addr  = 0xF9100000 ;
    ddr_half_config_addr = 0xF9200018 ;
} else if (ddr_num==1) {
    ddr_ctrl_base_addr = 0xF9010000 ;
    ddr_phy_base_addr  = 0xF9101000 ;
    ddr_half_config_addr = 0xF920001C ;
} else if (ddr_num==2) {
    ddr_ctrl_base_addr = 0xF9020000 ;
    ddr_phy_base_addr  = 0xF9102000 ;
    ddr_half_config_addr = 0xF9200020 ;
} else if (ddr_num==3) {
    ddr_ctrl_base_addr = 0xF9030000 ;
    ddr_phy_base_addr  = 0xF9103000 ;
    ddr_half_config_addr = 0xF9200024 ;
} else {
print_task("Un-spuuort DDR device number!\n");
}
unsigned int ddr_type ;
unsigned int speed ;
unsigned int capacity ;
unsigned int dev_type ;
unsigned int skip_training ;
unsigned int vref_training ;
ddr_type = 0 ;
speed = 1 ;
capacity = 0 ;
dev_type = 1 ;
skip_training = 0 ;
vref_training =1 ;

print_task(" one DDR device initiall begin !\n");
apb_reset_release(ddr_num);
dfi_init_complete_dis (ddr_ctrl_base_addr) ;

dmc_init(ddr_type, speed, capacity, dev_type,ddr_ctrl_base_addr);

dmc_reset_release(ddr_num);
phy_reset_release(ddr_num);
ddr_phy_init(ddr_type,speed,skip_training,ddr_phy_base_addr); //skip training

//add half data config
ddr_half_path (ddr_ctrl_base_addr,ddr_phy_base_addr);
reg_write32(ddr_half_config_addr,0x000C0000);
print_task("half data config done!\n");

dfi_init_complete_en (ddr_ctrl_base_addr) ;
ddr_device_init(ddr_ctrl_base_addr);

ddr_phy_train(ddr_type,vref_training,skip_training,ddr_phy_base_addr);  //skip training
axi_reset_release(ddr_num);
reg_write32(0xF9200010 , 0x0000001F);  //release noc reset
print_task(" one DDR device initiall end !\n");
}

#ifdef DDR_BIST_TEST
//--------------------------------------------------------------//
void ddr_one_initial_flow_phy_bist (unsigned int ddr_num)
{
    int ddr_ctrl_base_addr = 0xF9000000;
    int ddr_phy_base_addr = 0xF9100000;
if (ddr_num==0) {
    ddr_ctrl_base_addr = 0xF9000000 ;
    ddr_phy_base_addr  = 0xF9100000 ;
} else if (ddr_num==1) {
    ddr_ctrl_base_addr = 0xF9010000 ;
    ddr_phy_base_addr  = 0xF9101000 ;
} else if (ddr_num==2) {
    ddr_ctrl_base_addr = 0xF9020000 ;
    ddr_phy_base_addr  = 0xF9102000 ;
} else if (ddr_num==3) {
    ddr_ctrl_base_addr = 0xF9030000 ;
    ddr_phy_base_addr  = 0xF9103000 ;
} else {
print_task("Un-spuuort DDR device number!\n");
}
unsigned int ddr_type ;
unsigned int speed ;
unsigned int capacity ;
unsigned int dev_type ;
unsigned int skip_training ;
unsigned int vref_training ;
ddr_type = 0 ;
speed = 1 ;
capacity = 0 ;
dev_type = 1 ;
skip_training = 0 ;
vref_training =1 ;

print_task(" one DDR device initiall begin !\n");
apb_reset_release(ddr_num);
dfi_init_complete_dis (ddr_ctrl_base_addr) ;

dmc_init(ddr_type, speed, capacity, dev_type,ddr_ctrl_base_addr);

dmc_reset_release(ddr_num);
phy_reset_release(ddr_num);
ddr_phy_init(ddr_type,speed,skip_training,ddr_phy_base_addr); //skip training

dfi_init_complete_en (ddr_ctrl_base_addr) ;
ddr_device_init(ddr_ctrl_base_addr);

ddr_phy_train(ddr_type,vref_training,skip_training,ddr_phy_base_addr);  //skip training

//add phy bist test
ddr_phy_bist_test (ddr_phy_base_addr);
print_task("phy bist done!\n");

axi_reset_release(ddr_num);

reg_write32(0xF9200010 , 0x0000001F);  //release noc reset
print_task(" one DDR device initiall end !\n");
}

#endif

#ifdef DDR_LP_TEST
//--------------------------------------------------------------//
void ddr_one_initial_flow_cpu_loopback (unsigned int ddr_num)
{
    // int ddr_ctrl_base_addr ;
    int ddr_phy_base_addr = 0xF9100000;
if (ddr_num==0) {
    // ddr_ctrl_base_addr = 0xF9000000 ;
    ddr_phy_base_addr  = 0xF9100000 ;
} else if (ddr_num==1) {
    // ddr_ctrl_base_addr = 0xF9010000 ;
    ddr_phy_base_addr  = 0xF9101000 ;
} else if (ddr_num==2) {
    // ddr_ctrl_base_addr = 0xF9020000 ;
    ddr_phy_base_addr  = 0xF9102000 ;
} else if (ddr_num==3) {
    // ddr_ctrl_base_addr = 0xF9030000 ;
    ddr_phy_base_addr  = 0xF9103000 ;
} else {
print_task("Un-spuuort DDR device number!\n");
}
unsigned int ddr_type ;
unsigned int speed ;
// unsigned int capacity ;
// unsigned int dev_type ;
unsigned int skip_training ;
// unsigned int vref_training ;
ddr_type = 0 ;
speed = 1 ;
// capacity = 0 ;
// dev_type = 1 ;
skip_training = 0 ;
// vref_training =1 ;

print_task(" one DDR device initiall begin !\n");
apb_reset_release(ddr_num);

phy_reset_release(ddr_num);
ddr_phy_init(ddr_type,speed,skip_training,ddr_phy_base_addr); //skip training

//AC loopback, PAD side
ddr_phy_lpbk_test(0,0,ddr_phy_base_addr);
//AC loopback, core side
ddr_phy_lpbk_test(0,1,ddr_phy_base_addr);
//DX loopback, PAD side
ddr_phy_lpbk_test(1,0,ddr_phy_base_addr);
//DX loopback, core side
ddr_phy_lpbk_test(1,1,ddr_phy_base_addr);

print_task("ddr loop_back done!\n");

}
#endif

#ifdef DDR_DCU_TEST
//--------------------------------------------------------------//
void ddr_one_initial_flow_dcu_test (unsigned int ddr_num)
{
    int ddr_ctrl_base_addr = 0xF9000000;
    int ddr_phy_base_addr = 0xF9100000;
if (ddr_num==0) {
    ddr_ctrl_base_addr = 0xF9000000 ;
    ddr_phy_base_addr  = 0xF9100000 ;
} else if (ddr_num==1) {
    ddr_ctrl_base_addr = 0xF9010000 ;
    ddr_phy_base_addr  = 0xF9101000 ;
} else if (ddr_num==2) {
    ddr_ctrl_base_addr = 0xF9020000 ;
    ddr_phy_base_addr  = 0xF9102000 ;
} else if (ddr_num==3) {
    ddr_ctrl_base_addr = 0xF9030000 ;
    ddr_phy_base_addr  = 0xF9103000 ;
} else {
print_task("Un-spuuort DDR device number!\n");
}
unsigned int ddr_type ;
unsigned int speed ;
unsigned int capacity ;
unsigned int dev_type ;
unsigned int skip_training ;
unsigned int vref_training ;
ddr_type = 0 ;
speed = 0 ;
capacity = 0 ;
dev_type = 1 ;
skip_training = 1 ;
vref_training =1 ;

print_task(" one DDR device initiall begin !\n");
apb_reset_release(ddr_num);
dfi_init_complete_dis (ddr_ctrl_base_addr) ;

dmc_init(ddr_type, speed, capacity, dev_type,ddr_ctrl_base_addr);

dmc_reset_release(ddr_num);
phy_reset_release(ddr_num);

ddr_phy_init(ddr_type,speed,skip_training,ddr_phy_base_addr); //skip training

dfi_init_complete_en (ddr_ctrl_base_addr) ;
ddr_device_init(ddr_ctrl_base_addr);

ddr_phy_train(ddr_type,vref_training,skip_training,ddr_phy_base_addr);  //skip training

//axi_reset_release(ddr_num);
//reg_write32(0xF9200010 , 0x0000001F);  //release noc reset
print_task(" one DDR device initiall end !\n");

//reset dmc ?
//    unsigned int read_data = 0 ;
//    read_data = reg_read32(0xF9200004) ;
//    read_data = read_data | (0x0 << ddr_num) ; // dmc eset
//reg_write32(0xF9200004 ,read_data ); //release all dmc reset
//print_task("ddr controller reset assert !\n");

}
#endif

//--------------------------------------------------------------//
void ddr_all_initial_flow (void)
{
unsigned int ddr_type ;
unsigned int speed ;
unsigned int capacity ;
unsigned int dev_type ;
unsigned int skip ;
unsigned int vref_train;
ddr_type = 0 ;
speed = 0 ;
capacity = 0 ;
dev_type = 1 ;
skip = 0 ;
vref_train =1 ;

int ddr_ctrl_base_addr0 ;
int ddr_ctrl_base_addr1 ;
int ddr_ctrl_base_addr2 ;
int ddr_ctrl_base_addr3 ;
int ddr_phy_base_addr0 ;
int ddr_phy_base_addr1 ;
int ddr_phy_base_addr2 ;
int ddr_phy_base_addr3 ;

    ddr_ctrl_base_addr0 = 0xF9000000 ;
    ddr_phy_base_addr0  = 0xF9100000 ;
    ddr_ctrl_base_addr1 = 0xF9010000 ;
    ddr_phy_base_addr1  = 0xF9101000 ;
    ddr_ctrl_base_addr2 = 0xF9020000 ;
    ddr_phy_base_addr2  = 0xF9102000 ;
    ddr_ctrl_base_addr3 = 0xF9030000 ;
    ddr_phy_base_addr3  = 0xF9103000 ;


  apb_reset_release_all ();
  dfi_init_complete_dis (ddr_ctrl_base_addr0) ;
  dfi_init_complete_dis (ddr_ctrl_base_addr1) ;
  dfi_init_complete_dis (ddr_ctrl_base_addr2) ;
  dfi_init_complete_dis (ddr_ctrl_base_addr3) ;

  dmc_init(ddr_type, speed, capacity, dev_type,ddr_ctrl_base_addr0);
  dmc_init(ddr_type, speed, capacity, dev_type,ddr_ctrl_base_addr1);
  dmc_init(ddr_type, speed, capacity, dev_type,ddr_ctrl_base_addr2);
  dmc_init(ddr_type, speed, capacity, dev_type,ddr_ctrl_base_addr3);

  dmc_reset_release_all();

  phy_reset_release_all();

  ddr_phy_init_cfg ( ddr_type ,  speed, skip, ddr_phy_base_addr0) ;
  ddr_phy_init_cfg ( ddr_type ,  speed, skip, ddr_phy_base_addr1) ;
  ddr_phy_init_cfg ( ddr_type ,  speed, skip, ddr_phy_base_addr2) ;
  ddr_phy_init_cfg ( ddr_type ,  speed, skip, ddr_phy_base_addr3) ;
  ddr_phy_init_check ( ddr_phy_base_addr0) ;
  ddr_phy_init_check ( ddr_phy_base_addr1) ;
  ddr_phy_init_check ( ddr_phy_base_addr2) ;
  ddr_phy_init_check ( ddr_phy_base_addr3) ;
  print_task(" all DDR device phy init done !\n");

  dfi_init_complete_en (ddr_ctrl_base_addr0) ;
  dfi_init_complete_en (ddr_ctrl_base_addr1) ;
  dfi_init_complete_en (ddr_ctrl_base_addr2) ;
  dfi_init_complete_en (ddr_ctrl_base_addr3) ;

  ddr_device_init(ddr_ctrl_base_addr0);
  ddr_device_init(ddr_ctrl_base_addr1);
  ddr_device_init(ddr_ctrl_base_addr2);
  ddr_device_init(ddr_ctrl_base_addr3);

ddr_phy_train_cfg (ddr_type , vref_train, skip ,ddr_phy_base_addr0);
ddr_phy_train_cfg (ddr_type , vref_train, skip ,ddr_phy_base_addr1);
ddr_phy_train_cfg (ddr_type , vref_train, skip ,ddr_phy_base_addr2);
ddr_phy_train_cfg (ddr_type , vref_train, skip ,ddr_phy_base_addr3);

ddr_phy_train_check (ddr_phy_base_addr0,ddr_type,vref_train) ;
ddr_phy_train_check (ddr_phy_base_addr1,ddr_type,vref_train) ;
ddr_phy_train_check (ddr_phy_base_addr2,ddr_type,vref_train) ;
ddr_phy_train_check (ddr_phy_base_addr3,ddr_type,vref_train) ;
print_task(" all DDR device phy training done !\n");

axi_reset_release_all();

reg_write32(0xF9200010 , 0x0000001F);  //release noc reset
print_task(" all DDR device initiall end !\n");
}



#ifndef SOFT_DEBUG_FOR_DDR
//------------------------------------------------------------------------------//
//---------------part 6 : ddr access test task ---------------------------------//
//------------------------------------------------------------------------------//
int ddr_write_read_test(int access_num ,int database) {
 int i ;
 int j ;
 int base_address ;
 int access_address ;
  int read_data = 0 ;
 int write_data ;
 int m  ;
 int n  ;

 int access_step ;

if (database==4) { //4G
access_step = ((0xefffffff/4)/access_num) - 1;
} else if (database==1) { //1G
access_step = ((0x3bffffff/4)/access_num) - 1;
}

access_address = 0;
for(i=0;i<access_num;i = i + 1 )
{
    //write_data = access_address + i + 0x1234;
    write_data = access_address + i*i ;
    reg_write32(access_address,write_data);
    access_address = access_address + access_step*4;
}

print_task("sim_note  DDR write done ! begin read !\n");
access_address = 0;
for(j=0;j<access_num;j=j+1)
{
    read_data = reg_read32(access_address) ;
    //write_data = access_address + j + 0x1234;
    write_data = access_address + j*j ;
    if (write_data==read_data) {
    //printf("read_data = %x , write_data = %x ,addr = %x \n",read_data ,write_data,access_address );
    //print_task("sim_note  DDR write and read test success !\n");
    } else {
    printf("read_data = %x , write_data = %x \n",read_data ,write_data );
    print_task("sim_note  DDR write and read test FAIL !\n");
    return 1;  //fail
    }
    access_address = access_address + access_step*4;
}
return 0 ; //0 pass
}


//ddr_access task
int ddr_access (int database)
{
int a,b,c,d,e,f ;
int result ;
a=ddr_write_read_test(111,database);
read_reg_delay (100) ;

b=ddr_write_read_test(222,database);
read_reg_delay (150) ;

c=ddr_write_read_test(333,database);
read_reg_delay (300) ;

d=ddr_write_read_test(444,database);
read_reg_delay (100) ;

e=ddr_write_read_test(555,database);
read_reg_delay (150) ;

f=ddr_write_read_test(666,database);

result = a+b+c+d+e+f ;
return result ;

}
#endif

void ddr_low_power (unsigned int ddr_num)
{
    // int ddr_ctrl_base_addr = 0xF9000000;
    int ddr_phy_base_addr = 0xF9100000;
    int ddr_phy_macro_gen_addr = 0xF9200018;
    int ddr_phy_pub_cg_en_addr = 0xF920002C;
if (ddr_num==0) {
    // ddr_ctrl_base_addr 	   = 0xF9000000 ;
    ddr_phy_base_addr  	   = 0xF9100000 ;
    ddr_phy_macro_gen_addr = 0xF9200018 ;
    ddr_phy_pub_cg_en_addr = 0xF920002C ;
} else if (ddr_num==1) {
    // ddr_ctrl_base_addr = 0xF9010000 ;
    ddr_phy_base_addr  = 0xF9101000 ;
    ddr_phy_macro_gen_addr = 0xF920001C ;
    ddr_phy_pub_cg_en_addr = 0xF9200030 ;
} else if (ddr_num==2) {
    // ddr_ctrl_base_addr = 0xF9020000 ;
    ddr_phy_base_addr  = 0xF9102000 ;
    ddr_phy_macro_gen_addr = 0xF9200020 ;
    ddr_phy_pub_cg_en_addr = 0xF9200034 ;
} else if (ddr_num==3) {
    // ddr_ctrl_base_addr = 0xF9030000 ;
    ddr_phy_base_addr  = 0xF9103000 ;
    ddr_phy_macro_gen_addr = 0xF9200024 ;
    ddr_phy_pub_cg_en_addr = 0xF9200038 ;
} else {
print_task("Un-spuuort DDR device number!\n");
}

printf("%s: channel %d\n", __FUNCTION__, ddr_num);

int read_data =0;
//DDR PHY low power config 
//1,ddr phy pll power down
read_data = reg_read32(0xF9200004) ;
read_data = read_data | (0x1 << (12+ddr_num)) ; //ddr phy apb reset
reg_write32(0xF9200004 ,read_data ); //release phy apb reset

read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_PLLCR);
read_data = read_data |(0x1<<29) ; //[29]=0 ;
reg_write32(ddr_phy_base_addr+DDR_PHY_PLLCR,read_data);

read_data = reg_read32(ddr_phy_base_addr+DDR_PHY_PLLCR0);
read_data = read_data |(0x1<<29) ; //[29]=0 ;
reg_write32(ddr_phy_base_addr+DDR_PHY_PLLCR0,read_data);
//2,ddr phy pub clock gating 
reg_write32(ddr_phy_pub_cg_en_addr,0x0);
//3,ddr phy marco clock gating
reg_write32(ddr_phy_macro_gen_addr,0x7fff);

//DDR controller lower config 
//1, axi port clock gating
reg_write32(0xF9200008 ,(0x1F << (8*ddr_num))); 
//2, controller clock gating
reg_write32(0xF920000C ,(0x1 << ddr_num)); 

}  //endtask

void sdram_init(void)
{
#if CONFIG_IS_ASIC
    reg_write32(0xf9700038, 0x0a);
    reg_write32(0xf9700034, 0x00);

#if CONFIG_DDR4_2400_SUPPORT
    printf("ddr4 2400 init...\n");
    chiptest_initial_ddr4_2133();
#endif

#if CONFIG_DDR4_2133_SUPPORT
    printf("ddr4 2133 201910051028 init...\n");
    chiptest_initial_ddr4_2133();
#endif

#if CONFIG_DDR4_1600_SUPPORT
    printf("ddr4 1600 201910051014 init...\n");
    chiptest_initial_ddr4_1600();
#endif

#if CONFIG_DDR4_800_SUPPORT
    printf("ddr4 800 201910051014 init...\n");
    chiptest_initial_ddr4_1600();
#endif

#if CONFIG_DDR3_1600_SUPPORT
    printf("ddr3 1600 init...\n");
    chiptest_initial_ddr3_1600();
#endif

#if CONFIG_DDR3_800_SUPPORT
    printf("ddr3 800 init...\n");
    chiptest_initial_ddr3_800();
#endif

#endif
}

#endif // ___DDR_TEST_TASK_H___
