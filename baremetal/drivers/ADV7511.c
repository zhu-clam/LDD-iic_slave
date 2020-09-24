//********************************************************************
//(Copyright 2008) Verisilicon Inc. All Rights Reserved
//Company confidential and Properietary information.
//This information may not be disclosed to unauthorized individual.
//********************************************************************
//
// VeriSilicon Microelectronics Co.,Ltd.
//
// File Name: 
// Author:    
// Version:
// Created:   
// Modified:
// Description:
//********************************************************************

#include "ADV7511.h"
#include "ck810.h"

#define ADV7511_I2C_ADDR 0x39
void ADV7511_Reg_SingleWrite(CK_UINT8 addr, CK_UINT8 val) {
    CK_UINT8 val2;
    dw_i2c_write_int(2, ADV7511_I2C_ADDR, addr, 1, &val, 1);
    dw_i2c_read_int(2, ADV7511_I2C_ADDR, addr, 1, &val2, 1);
    printf("ADV7511_Reg_SingleWrite  0x%x  (val) =0x%x \r\n",val2,val);
}

void ADV7511_Reg_SingleRead(CK_UINT8 addr, CK_UINT8 *val) {
    dw_i2c_read_int(2, ADV7511_I2C_ADDR, addr, 1, val, 1);
    printf("ADV7511_Reg_SingleRead  addr 0x%x (*val) =0x%x \r\n",addr,*val);
}

void ADV7511_Initial()
{
        CK_UINT8  read_data;
        unsigned int delay;
	dw_i2c_init(2, I2C_STANDARD_SPEED, 1);
	
	ADV7511_Reg_SingleRead(0x00,&read_data);
	//delay >200ms for hardware power up
	for (delay =0x1000;delay>0;delay--);
	ADV7511_Reg_SingleWrite(0x41, 0x10);
	for (delay =0x1000;delay>0;delay--);
	
	ADV7511_Reg_SingleWrite(0x98, 0x03);
	ADV7511_Reg_SingleWrite(0x9a, 0xe0);
	ADV7511_Reg_SingleWrite(0x9c, 0x30);
	ADV7511_Reg_SingleWrite(0x9d, 0x61);
	ADV7511_Reg_SingleWrite(0xa2, 0xa4);
	ADV7511_Reg_SingleWrite(0xa3, 0xa4);
	ADV7511_Reg_SingleWrite(0xe0, 0xd0);
	ADV7511_Reg_SingleWrite(0xf9, 0x00);

#if 1	
	ADV7511_Reg_SingleWrite(0x15, 0x00); //RGB444 separate syncs
	ADV7511_Reg_SingleWrite(0x16, 0x30);  
#else

        ADV7511_Reg_SingleWrite(0x15, 0x01);   //yuv422
        ADV7511_Reg_SingleWrite(0x16, 0xB3);   //yuv422
	ADV7511_Reg_SingleWrite(0x55, 0x20);   //yuv422
        ADV7511_Reg_SingleWrite(0x56, 0x28);   //16:9
#endif

	ADV7511_Reg_SingleWrite(0x17, 0x02);
	ADV7511_Reg_SingleWrite(0x18, 0x40);

	//ADV7511_Reg_SingleWrite(0x3c, 0x10); //1080pX60
	//ADV7511_Reg_SingleWrite(0x3c, 0x22); //1080pX30
	ADV7511_Reg_SingleWrite(0x3c, 0x3); //480pX60
        /////ADV7511_Reg_SingleWrite(0x3c, 0x4); //720pX60
	
	ADV7511_Reg_SingleWrite(0xaf, 0x06);  //dvi mode 0x04 / hdmi mode 0x06 
	
	//those three read operation read back the status of ADV7511
//while(1)
{
	ADV7511_Reg_SingleRead(0x3e,&read_data);
	ADV7511_Reg_SingleRead(0x42,&read_data);
	ADV7511_Reg_SingleRead(0x9e,&read_data);
}
}


void ADV7511_sdi_Initial()
{
        CK_UINT8  read_data;
        unsigned int delay;
	dw_i2c_init(2, I2C_STANDARD_SPEED, 1);
	
	ADV7511_Reg_SingleRead(0x00,&read_data);
	//delay >200ms for hardware power up
	for (delay =0x1000;delay>0;delay--);
	ADV7511_Reg_SingleWrite(0x41, 0x10);
	for (delay =0x1000;delay>0;delay--);
	
	ADV7511_Reg_SingleWrite(0x98, 0x03);
	ADV7511_Reg_SingleWrite(0x9a, 0xe0);
	ADV7511_Reg_SingleWrite(0x9c, 0x30);
	ADV7511_Reg_SingleWrite(0x9d, 0x61);
	ADV7511_Reg_SingleWrite(0xa2, 0xa4);
	ADV7511_Reg_SingleWrite(0xa3, 0xa4);
	ADV7511_Reg_SingleWrite(0xe0, 0xd0);
	ADV7511_Reg_SingleWrite(0xf9, 0x00);

#if 1	
	ADV7511_Reg_SingleWrite(0x15, 0x00); //RGB444 separate syncs
	ADV7511_Reg_SingleWrite(0x16, 0x30);  
#else

        ADV7511_Reg_SingleWrite(0x15, 0x01);   //yuv422
        ADV7511_Reg_SingleWrite(0x16, 0xB3);   //yuv422
	ADV7511_Reg_SingleWrite(0x55, 0x20);   //yuv422
        ADV7511_Reg_SingleWrite(0x56, 0x28);   //16:9
#endif

	ADV7511_Reg_SingleWrite(0x17, 0x02);
	ADV7511_Reg_SingleWrite(0x18, 0x40);

	//ADV7511_Reg_SingleWrite(0x3c, 0x10); //1080pX60
	///ADV7511_Reg_SingleWrite(0x3c, 0x22); //1080pX30
	////ADV7511_Reg_SingleWrite(0x3c, 0x12); //576pX50
	///ADV7511_Reg_SingleWrite(0x3c, 0x3); //480pX60
        /////ADV7511_Reg_SingleWrite(0x3c, 0x4); //720pX60

        ADV7511_Reg_SingleWrite(0x3c, 0x0B); //480iX60
	
	ADV7511_Reg_SingleWrite(0xaf, 0x06);  //dvi mode 0x04 / hdmi mode 0x06 
	
	//those three read operation read back the status of ADV7511
//while(1)
{
	ADV7511_Reg_SingleRead(0x3e,&read_data);
	ADV7511_Reg_SingleRead(0x42,&read_data);
	ADV7511_Reg_SingleRead(0x9e,&read_data);
}
}

