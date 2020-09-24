
#include "ADV7611.h"
#include "ck810.h"



static CK_UINT8 adv7611_sadd ;
static CK_UINT8 io_map = 0x98;
static CK_UINT8 ksv_map = 0x64;
static CK_UINT8 hdmi_map = 0x68;
static CK_UINT8 cp_map = 0x44;

#define ADV7611_RST (11)

#define  ADV7611_IIC_CS  (3) 
void adv7611_write_reg(CK_UINT8 dev_addr, CK_UINT8 addr, CK_UINT8 val) {
    CK_UINT8 val2;
    ///dw_i2c_write_int(0, dev_addr>>1, addr, 1, &val, 1);
    ///dw_i2c_read_int(0, dev_addr>>1, addr, 1, &val2, 1);

    dw_i2c_write(ADV7611_IIC_CS, dev_addr>>1, addr, 1, &val, 1);
    dw_i2c_read(ADV7611_IIC_CS, dev_addr>>1, addr, 1, &val2, 1);
    printf("ADV7611_Reg_SingleWrite  0x%x  (val) =0x%x \r\n",val2,val);
}

void adv7611_read_reg(CK_UINT8 dev_addr,CK_UINT8 addr, CK_UINT8 *val) {
   //// dw_i2c_read_int(0, dev_addr>>1, addr, 1, val, 1);
    dw_i2c_read(ADV7611_IIC_CS, dev_addr>>1, addr, 1, val, 1);
   //// dw_i2c_write(0, 0x39, addr, 1, &val, 1);
    printf("ADV7611_Reg_SingleRead  addr 0x%x (*val) =0x%x \r\n",addr,*val);
}


#if 0

void adv7611_init(void)
{

	CK_UINT8 val;
	

	dw_i2c_init(ADV7611_IIC_CS, I2C_STANDARD_SPEED, 1);

	CK_Gpio_Output(ADV7611_RST, 0);

    
    udelay(100 * 1000);
	CK_Gpio_Output(ADV7611_RST, 1);
	
	

	adv7611_sadd = io_map;
	//adv7611_write_reg(adv7611_sadd, 0x20, 0xf8);

	//read chip id
	adv7611_read_reg(adv7611_sadd, 0xea, &val);  //0x20
	printf(" id1  = 0x%x \r\n" , val) ;
	adv7611_read_reg(adv7611_sadd, 0xeb, &val);  //0x51
	printf(" id2  = 0x%x \r\n" , val) ;
	
	

	////adv7611_write_reg(adv7611_sadd, 0x00, 0x02);/////////////////////
	adv7611_write_reg(adv7611_sadd, 0x00, 0x02);/////////////////////
	//adv7611_write_reg(adv7611_sadd, 0x01, 0x05);
	/////////adv7611_write_reg(adv7611_sadd, 0x01, 0x06);
	adv7611_write_reg(adv7611_sadd, 0x01, 0x06);
	adv7611_write_reg(adv7611_sadd, 0x02, 0xf5);
        
#ifndef __ADV7611_SD__  //BT1120
	adv7611_write_reg(adv7611_sadd, 0x03, 0x80); //16-bit BT1120
	adv7611_write_reg(adv7611_sadd, 0x19, 0x83);
	adv7611_write_reg(adv7611_sadd, 0x33, 0x40);
#else //BT656
#if 1
	adv7611_write_reg(adv7611_sadd, 0x03, 0x00); //8-bit bt656
	adv7611_write_reg(adv7611_sadd, 0x19, 0xc3);
	adv7611_write_reg(adv7611_sadd, 0x33, 0x40);
#else
	adv7611_write_reg(adv7611_sadd, 0x03, 0x80); //16-bit bt1120
	adv7611_write_reg(adv7611_sadd, 0x19, 0x80);
	adv7611_write_reg(adv7611_sadd, 0x33, 0x40);
        adv7611_write_reg(adv7611_sadd, 0x19, 0x83);
#endif

#endif
	adv7611_write_reg(adv7611_sadd, 0x05, 0x2c);
	//adv7611_write_reg(adv7611_sadd, 0x05, 0x2d); //bit0 swap uv order
	//adv7611_write_reg(adv7611_sadd, 0x06, 0xa6);
	adv7611_write_reg(adv7611_sadd, 0x06, 0xa7); //set output pixclk polarity 
	
	adv7611_write_reg(adv7611_sadd, 0x0b, 0x44);
	adv7611_write_reg(adv7611_sadd, 0x0c, 0x42);
	adv7611_write_reg(adv7611_sadd, 0x14, 0x7f);  
	//adv7611_write_reg(adv7611_sadd, 0x14, 0x5f); //driver strengh
	adv7611_write_reg(adv7611_sadd, 0x15, 0x80);
       


	//I2C addresses
	adv7611_write_reg(adv7611_sadd, 0xf4, 0x80);
	adv7611_write_reg(adv7611_sadd, 0xf5, 0x7c);
	adv7611_write_reg(adv7611_sadd, 0xf8, 0x4c);
	adv7611_write_reg(adv7611_sadd, 0xf9, 0x64);
	adv7611_write_reg(adv7611_sadd, 0xfa, 0x6c);
	adv7611_write_reg(adv7611_sadd, 0xfb, 0x68);
	adv7611_write_reg(adv7611_sadd, 0xfd, 0x44);

	adv7611_read_reg(adv7611_sadd, 0xf4, &val);
	adv7611_read_reg(adv7611_sadd, 0xf5, &val);
	udelay(50 * 1000);
	//////////////mdelay(500);
	adv7611_sadd = cp_map;
	adv7611_write_reg(adv7611_sadd, 0xba, 0x01);
	adv7611_write_reg(adv7611_sadd, 0x6c, 0x00);
	udelay(50 * 1000);
	/////////////////mdelay(500);
	adv7611_sadd = ksv_map;
	adv7611_write_reg(adv7611_sadd, 0x40, 0x81);
#if 0
	adv7611_write_reg(adv7611_sadd, 0x77, 0x00);
	adv7611_write_reg(adv7611_sadd, 0x52, 0x20);
	adv7611_write_reg(adv7611_sadd, 0x53, 0x00);
	adv7611_write_reg(adv7611_sadd, 0x70, 0x9e);
	adv7611_write_reg(adv7611_sadd, 0x74, 0x03);
#endif
	udelay(50 * 1000);
	///////////////////mdelay(500);
	adv7611_sadd = hdmi_map;
	adv7611_write_reg(adv7611_sadd, 0x9b, 0x03);
	//adv7611_write_reg(adv7611_sadd, 0x6f, 0x0c);
	adv7611_write_reg(adv7611_sadd, 0x6f, 0x08);
	adv7611_write_reg(adv7611_sadd, 0x85, 0x1f);
	adv7611_write_reg(adv7611_sadd, 0x87, 0x70);
	adv7611_write_reg(adv7611_sadd, 0x57, 0xda);
	adv7611_write_reg(adv7611_sadd, 0x58, 0x01);
	adv7611_write_reg(adv7611_sadd, 0x03, 0x98);
	adv7611_write_reg(adv7611_sadd, 0x4c, 0x44);

	adv7611_write_reg(adv7611_sadd, 0xc1, 0x01);
	adv7611_write_reg(adv7611_sadd, 0xc2, 0x01);
	adv7611_write_reg(adv7611_sadd, 0xc3, 0x01);
	adv7611_write_reg(adv7611_sadd, 0xc4, 0x01);
	adv7611_write_reg(adv7611_sadd, 0xc5, 0x01);
	adv7611_write_reg(adv7611_sadd, 0xc6, 0x01);
	adv7611_write_reg(adv7611_sadd, 0xc7, 0x01);
	adv7611_write_reg(adv7611_sadd, 0xc8, 0x01);
	adv7611_write_reg(adv7611_sadd, 0xc9, 0x01);
	adv7611_write_reg(adv7611_sadd, 0xca, 0x01);
	adv7611_write_reg(adv7611_sadd, 0xcb, 0x01);
	adv7611_write_reg(adv7611_sadd, 0xcc, 0x01);

	adv7611_write_reg(adv7611_sadd, 0x00, 0x00);
	adv7611_write_reg(adv7611_sadd, 0x83, 0xfe);

	adv7611_write_reg(adv7611_sadd, 0x8d, 0x04);
	//////////////////adv7611_write_reg(adv7611_sadd, 0x8e, 0x1e);  //above 480p/576p
	adv7611_write_reg(adv7611_sadd, 0x8e, 0x35); //480p/576p
	adv7611_write_reg(adv7611_sadd, 0x1a, 0x8a);

	adv7611_write_reg(adv7611_sadd, 0x75, 0x10);

	udelay(50 * 1000);

	//while(1)
	{
        adv7611_sadd = hdmi_map;
        adv7611_read_reg(adv7611_sadd, 0x05, &val);
        adv7611_sadd = hdmi_map;
        adv7611_read_reg(adv7611_sadd, 0x04, &val);

        adv7611_sadd = hdmi_map;
        adv7611_read_reg(adv7611_sadd, 0x0B, &val);

        adv7611_sadd = hdmi_map;
        adv7611_read_reg(adv7611_sadd, 0x51, &val); 
        adv7611_sadd = hdmi_map;
        adv7611_read_reg(adv7611_sadd, 0x52, &val);

        adv7611_sadd = io_map;
        adv7611_read_reg(adv7611_sadd, 0x6A, &val);
	}

	//adv7611_read_reg(adv7611_sadd, 0x01, &val);

	return;
}

#else
	
void adv7611_init(void)
{

	CK_UINT8 val;
	

	dw_i2c_init(ADV7611_IIC_CS, I2C_STANDARD_SPEED, 1);

	CK_Gpio_Output(ADV7611_RST, 0);

    
    udelay(100 * 1000);
	CK_Gpio_Output(ADV7611_RST, 1);
	

	

	adv7611_sadd = io_map;
	//adv7611_write_reg(adv7611_sadd, 0x20, 0xf8);

	//read chip id
	adv7611_read_reg(adv7611_sadd, 0xea, &val);  //0x20
	adv7611_read_reg(adv7611_sadd, 0xeb, &val);  //0x51

	////adv7611_write_reg(adv7611_sadd, 0x00, 0x02);/////////////////////
	adv7611_write_reg(adv7611_sadd, 0x00, 0x02);/////////////////////
	//adv7611_write_reg(adv7611_sadd, 0x01, 0x05);
	adv7611_write_reg(adv7611_sadd, 0x01, 0x06);
	adv7611_write_reg(adv7611_sadd, 0x02, 0xf5);
        
#if 1 //BT1120
	adv7611_write_reg(adv7611_sadd, 0x03, 0x80); //16-bit BT1120
	adv7611_write_reg(adv7611_sadd, 0x19, 0x83);
	adv7611_write_reg(adv7611_sadd, 0x33, 0x40);
#else //BT656
#if 0
	adv7611_write_reg(adv7611_sadd, 0x03, 0x00); //8-bit bt656
	adv7611_write_reg(adv7611_sadd, 0x19, 0xc3);
	adv7611_write_reg(adv7611_sadd, 0x33, 0x40);
#else
	adv7611_write_reg(adv7611_sadd, 0x03, 0x80); //16-bit bt1120
	adv7611_write_reg(adv7611_sadd, 0x19, 0x80);
	adv7611_write_reg(adv7611_sadd, 0x33, 0x40);
        adv7611_write_reg(adv7611_sadd, 0x19, 0x83);
#endif

#endif
	adv7611_write_reg(adv7611_sadd, 0x05, 0x2c);
	//adv7611_write_reg(adv7611_sadd, 0x05, 0x2d); //bit0 swap uv order
	//adv7611_write_reg(adv7611_sadd, 0x06, 0xa6);
	adv7611_write_reg(adv7611_sadd, 0x06, 0xa7); //set output pixclk polarity 
	adv7611_write_reg(adv7611_sadd, 0x0b, 0x44);
	adv7611_write_reg(adv7611_sadd, 0x0c, 0x42);
	adv7611_write_reg(adv7611_sadd, 0x14, 0x7f);  
	//adv7611_write_reg(adv7611_sadd, 0x14, 0x5f); //driver strengh
	adv7611_write_reg(adv7611_sadd, 0x15, 0x80);
       


	//I2C addresses
	adv7611_write_reg(adv7611_sadd, 0xf4, 0x80);
	adv7611_write_reg(adv7611_sadd, 0xf5, 0x7c);
	adv7611_write_reg(adv7611_sadd, 0xf8, 0x4c);
	adv7611_write_reg(adv7611_sadd, 0xf9, 0x64);
	adv7611_write_reg(adv7611_sadd, 0xfa, 0x6c);
	adv7611_write_reg(adv7611_sadd, 0xfb, 0x68);
	adv7611_write_reg(adv7611_sadd, 0xfd, 0x44);

	adv7611_read_reg(adv7611_sadd, 0xf4, &val);
	adv7611_read_reg(adv7611_sadd, 0xf5, &val);
	udelay(50 * 1000);
	//////////////mdelay(500);
	adv7611_sadd = cp_map;
	adv7611_write_reg(adv7611_sadd, 0xba, 0x01);
	adv7611_write_reg(adv7611_sadd, 0x6c, 0x00);
	udelay(50 * 1000);
	/////////////////mdelay(500);
	adv7611_sadd = ksv_map;
	adv7611_write_reg(adv7611_sadd, 0x40, 0x81);
#if 0
	adv7611_write_reg(adv7611_sadd, 0x77, 0x00);
	adv7611_write_reg(adv7611_sadd, 0x52, 0x20);
	adv7611_write_reg(adv7611_sadd, 0x53, 0x00);
	adv7611_write_reg(adv7611_sadd, 0x70, 0x9e);
	adv7611_write_reg(adv7611_sadd, 0x74, 0x03);
#endif
	udelay(50 * 1000);
	///////////////////mdelay(500);
	adv7611_sadd = hdmi_map;
	adv7611_write_reg(adv7611_sadd, 0x9b, 0x03);
	//adv7611_write_reg(adv7611_sadd, 0x6f, 0x0c);
	adv7611_write_reg(adv7611_sadd, 0x6f, 0x08);
	adv7611_write_reg(adv7611_sadd, 0x85, 0x1f);
	adv7611_write_reg(adv7611_sadd, 0x87, 0x70);
	adv7611_write_reg(adv7611_sadd, 0x57, 0xda);
	adv7611_write_reg(adv7611_sadd, 0x58, 0x01);
	adv7611_write_reg(adv7611_sadd, 0x03, 0x98);
	adv7611_write_reg(adv7611_sadd, 0x4c, 0x44);

	adv7611_write_reg(adv7611_sadd, 0xc1, 0x01);
	adv7611_write_reg(adv7611_sadd, 0xc2, 0x01);
	adv7611_write_reg(adv7611_sadd, 0xc3, 0x01);
	adv7611_write_reg(adv7611_sadd, 0xc4, 0x01);
	adv7611_write_reg(adv7611_sadd, 0xc5, 0x01);
	adv7611_write_reg(adv7611_sadd, 0xc6, 0x01);
	adv7611_write_reg(adv7611_sadd, 0xc7, 0x01);
	adv7611_write_reg(adv7611_sadd, 0xc8, 0x01);
	adv7611_write_reg(adv7611_sadd, 0xc9, 0x01);
	adv7611_write_reg(adv7611_sadd, 0xca, 0x01);
	adv7611_write_reg(adv7611_sadd, 0xcb, 0x01);
	adv7611_write_reg(adv7611_sadd, 0xcc, 0x01);

	adv7611_write_reg(adv7611_sadd, 0x00, 0x00);
	adv7611_write_reg(adv7611_sadd, 0x83, 0xfe);

	adv7611_write_reg(adv7611_sadd, 0x8d, 0x04);
	//////////////////adv7611_write_reg(adv7611_sadd, 0x8e, 0x1e);  //above 480p/576p
	adv7611_write_reg(adv7611_sadd, 0x8e, 0x35); //480p/576p
	adv7611_write_reg(adv7611_sadd, 0x1a, 0x8a);

	adv7611_write_reg(adv7611_sadd, 0x75, 0x10);

	udelay(50 * 1000);

	//while(1)
	{
        adv7611_sadd = hdmi_map;
        adv7611_read_reg(adv7611_sadd, 0x05, &val);
        adv7611_sadd = hdmi_map;
        adv7611_read_reg(adv7611_sadd, 0x04, &val);

        adv7611_sadd = hdmi_map;
        adv7611_read_reg(adv7611_sadd, 0x0B, &val);

        adv7611_sadd = hdmi_map;
        adv7611_read_reg(adv7611_sadd, 0x51, &val); 
        adv7611_sadd = hdmi_map;
        adv7611_read_reg(adv7611_sadd, 0x52, &val);

        adv7611_sadd = io_map;
        adv7611_read_reg(adv7611_sadd, 0x6A, &val);
	}

	//adv7611_read_reg(adv7611_sadd, 0x01, &val);

	return;
}

#endif