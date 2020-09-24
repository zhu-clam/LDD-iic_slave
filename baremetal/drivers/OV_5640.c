#include "OV_5640.h"
#include "stdio.h"
#include "gpio.h"
#include "ck810.h"
#include "mipi_subsys.h"
extern CK_UINT32 dw_i2c_write_int(CK_UINT32 id, CK_UINT32 dev, CK_UINT32 addr,
            CK_UINT32 alen, CK_UINT8 *buffer, CK_UINT32 len);
extern CK_UINT32 dw_i2c_read_int(CK_UINT32 id, CK_UINT32 dev, CK_UINT32 addr,
               CK_UINT32 alen, CK_UINT8 *buffer, CK_UINT32 len);

#define OV_I2C_NB		(0)
#define OV_I2C_NB_CH1		(1)
#define OV_PWDN_IONB		(0)
#define OV_RESET_IONB		(1)
#define OV_PWDN_IONB_CH1		(2)
#define OV_RESET_IONB_CH1		(3)
#define ADV_I2C_NB		(2)
#define MIPI_PHY_I2C_ADDR 	(0x28)
#define OV5640_I2C_ADDR   	(0x3c)

static void ADV7511_Reg_Write(CK_UINT8 addr, CK_UINT8 val) {
    dw_i2c_write(ADV_I2C_NB, 0x39, addr, 1, &val, 1);
}
static void ADV7511_Reg_Read(CK_UINT8 addr, CK_UINT8 *val) {
    dw_i2c_read(ADV_I2C_NB, 0x39, addr, 1, val, 1);
    printf("ADV7511_Reg_Read  addr 0x%x (*val) =0x%x \r\n",addr,*val);
}

void I2C_MIPI_PHY_Write(CK_UINT8 addr, CK_UINT8 val) {
    CK_UINT8 val2;
	
    dw_i2c_write_int(0, MIPI_PHY_I2C_ADDR, addr, 1, &val, 1);
    dw_i2c_read_int(0, MIPI_PHY_I2C_ADDR, addr, 1, &val2, 1);

    printf("I2C_MIPI_PHY_Write  0x%x  (val) =0x%x \r\n",val2,val);
}

CK_UINT8 I2C_OV5640_Read(CK_UINT16 addr) {
    CK_UINT8 val;
#if defined( MIPI_ISP_CH1) ||defined(__TEST_ISP1__)
     dw_i2c_read(OV_I2C_NB_CH1, OV5640_I2C_ADDR, addr, 2, &val, 1);
#else	
    //dw_i2c_read_int(0, OV5640_I2C_ADDR, addr, 2, &val, 1);
    dw_i2c_read(OV_I2C_NB, OV5640_I2C_ADDR, addr, 2, &val, 1);
#endif	
    return val;
}

void I2C_OV5640_Write(CK_UINT16 addr, CK_UINT8 val) {
    //dw_i2c_write_int(0, OV5640_I2C_ADDR, addr, 2, &val, 1);
#if defined( MIPI_ISP_CH1) ||defined(__TEST_ISP1__)
    dw_i2c_write(OV_I2C_NB_CH1, OV5640_I2C_ADDR, addr, 2, &val, 1);
#else	
	dw_i2c_write(OV_I2C_NB, OV5640_I2C_ADDR, addr, 2, &val, 1);
#endif
        printf("I2C_OV5640_Write  0x%x  (val) =0x%x \r\n",I2C_OV5640_Read(addr),val);
}



void I2C_MIPI_PHY_Initial()
{
    // mipi slave
    write_mreg32(CK_MIPI_Slave+0x08, 0x1);  //release all the reset and shutdown
    write_mreg32(CK_MIPI_Slave+0x40, 0x1);
    write_mreg32(CK_MIPI_Slave+0x44, 0x1);
#if 0
    //pinmux, set output drive strenth to 4mA
    write_mreg32(0xfc20906c, 0x8);
    write_mreg32(0xfc209070, 0x8);
#endif
   #if defined( MIPI_ISP_CH1) ||defined(__TEST_ISP1__)
	dw_i2c_init(OV_I2C_NB_CH1, I2C_STANDARD_SPEED, 1);
	#else
    // I2C0
    dw_i2c_init(OV_I2C_NB, I2C_STANDARD_SPEED, 1);
	#endif
}

void I2C_OV5640_Initial()
{
	printf("I2C_OV5640_Initial \r\n");
    #if defined( MIPI_ISP_CH1) ||defined(__TEST_ISP1__)
	dw_i2c_init(OV_I2C_NB_CH1, I2C_STANDARD_SPEED, 1);
	#else
    // I2C0
    dw_i2c_init(OV_I2C_NB, I2C_STANDARD_SPEED, 1);
	#endif
}

extern void mipi_dphy_initial(int freq);
void MIPI_PHY_Initial()
{
#if CONFIG_IS_ASIC //for ASIC version, PHY configure by APB port
    #ifdef __OV5640_80MBPS__
	mipi_dphy_initial(80);
	#endif
    #ifdef __OV5640_160MBPS__
	mipi_dphy_initial(160);
	#endif
	#ifdef __OV5640_400MBPS__
	mipi_dphy_initial(400);
	#endif
	
	#ifdef __OV5640_800MBPS__
	mipi_dphy_initial(800);
	#endif
	#ifdef __OV5640_600MBPS__
	mipi_dphy_initial(600);
	#endif
#else // //for FPGA version, PHY configure by I2C port
    I2C_MIPI_PHY_Initial();
#if 0
    I2C_MIPI_PHY_Write(ADDR_CIL_APBWR_CTRL, 0xff);
    I2C_MIPI_PHY_Write(TCDPHY_GCTRL, 0x2f);
    I2C_MIPI_PHY_Write(TCDPHY_ENABLE, 0x13); // 2 lanes
    I2C_MIPI_PHY_Write(ADDR_CIL_CNT_HSD_SETTLE, 0x2);
    I2C_MIPI_PHY_Write(ADDR_CIL_CNT_HSCK_SETTLE, 0x8);
#else
    I2C_MIPI_PHY_Write(0x00, 0x01);
    I2C_MIPI_PHY_Write(0x0e, 0x00);
    I2C_MIPI_PHY_Write(0x14, 0x06);
    I2C_MIPI_PHY_Write(0x07, 0x2);
    I2C_MIPI_PHY_Write(0xf0, 0x43);
    I2C_MIPI_PHY_Write(0xf1, 0x13);
    I2C_MIPI_PHY_Write(0xf0, 0x63);
#endif
#endif
}

void MOV5640_Format_Init(int format)
{
    int i;

    for(i = 0; i < ARRAY_SIZE(StrMOV5640_CFG_INIT); i++) //初始化中间sensor所有寄存器(1280*720)
    {
        if(StrMOV5640_CFG_INIT[i].addr == 0x4300) //set yuv422 format
        {
            if(format == CSI_YUV422_8B)
            {
                I2C_OV5640_Write(0x4300, 0x3f); //UYVY
                I2C_OV5640_Write(0x501f, 0x00);
            }
            else if (format == CSI_RGB565)
            {
                I2C_OV5640_Write(0x4300, 0x6f);
                I2C_OV5640_Write(0x501f, 0x01);
            }
            else if (format == CSI_RGB555)
            {
                I2C_OV5640_Write(0x4300, 0x7f);
                I2C_OV5640_Write(0x501f, 0x01);
            }
            else if (format == CSI_RGB444)
            {
                I2C_OV5640_Write(0x4300, 0x9f);
                I2C_OV5640_Write(0x501f, 0x01);
            }
            else if(format == CSI_YUV420_8B)
            {
                I2C_OV5640_Write(0x4300, 0x4f);
                I2C_OV5640_Write(0x501f, 0x00);
            }
            else if(format == CSI_YUV420_8B_NV21)
            {
                I2C_OV5640_Write(0x4300, 0x43);
                I2C_OV5640_Write(0x501f, 0x00);
            }
        }
        else
        {
            I2C_OV5640_Write(StrMOV5640_CFG_INIT[i].addr, StrMOV5640_CFG_INIT[i].value);
        }
        if(i < 3) //i<3的目的是增加前三个寄存器读写时间间隔，保证MOV5640软复位成功
        {
            udelay(5 * 1000);
        }
        else
        {
            delay(1);
        }
    }
}

void MOV5640_isp_Format_Init(int format)
{
    int i;

    for(i = 0; i < ARRAY_SIZE(StrMOV5640_CFG_ISP_INIT); i++)
    {

        /*if(StrMOV5640_CFG_ISP_INIT[i].addr == 0x4300)
        {
                I2C_OV5640_Write(0x4300, 0x4f);
                I2C_OV5640_Write(0x501f, 0x00);

        }
        else*/
         I2C_OV5640_Write(StrMOV5640_CFG_ISP_INIT[i].addr, StrMOV5640_CFG_ISP_INIT[i].value);

        if(i < 3) //i<3的目的是增加前三个寄存器读写时间间隔，保证MOV5640软复位成功
        {
            udelay(5 * 1000);
        }
        else
        {
            udelay(10);
        }
    }
}

void MOV5640_640x480(int format)
{
    unsigned int i;
    unsigned char reg_value = 0x00;

    for(i = 0; i < ARRAY_SIZE(StrMOV40_640X480); i++)
    {
        I2C_OV5640_Write(StrMOV40_640X480[i].addr, StrMOV40_640X480[i].value);
        udelay(10);
    }

    //FLIP 翻转
    reg_value = I2C_OV5640_Read(0x3820);
    reg_value |= 0x06;
    I2C_OV5640_Write(0x3820, reg_value);

    reg_value = I2C_OV5640_Read(0x3821);
    reg_value |= 0x06;
    I2C_OV5640_Write(0x3821, reg_value);
}

void MOV5640_1280x720(int format)
{
    unsigned int i;
    unsigned char reg_value = 0x00;

    for(i = 0; i < ARRAY_SIZE(StrMOV40_1280X720); i++)
    {
        I2C_OV5640_Write(StrMOV40_1280X720[i].addr, StrMOV40_1280X720[i].value);
        udelay(10);
    }
    //FLIP 翻转
    reg_value = I2C_OV5640_Read(0x3820);
    reg_value |= 0x06;
    I2C_OV5640_Write(0x3820, reg_value);

    reg_value = I2C_OV5640_Read(0x3821);
    reg_value |= 0x06;
    I2C_OV5640_Write(0x3821, reg_value);
}

void MOV5640_RAW_1280x720(int format)
{
    unsigned int i;
    unsigned char reg_value = 0x00;

    for(i = 0; i < ARRAY_SIZE(StrMOV40_RAW_1280X720); i++)
    {
        I2C_OV5640_Write(StrMOV40_RAW_1280X720[i].addr, StrMOV40_RAW_1280X720[i].value);
        udelay(10);
    }
    //FLIP 翻转
    reg_value = I2C_OV5640_Read(0x3820);
    reg_value |= 0x06;
    I2C_OV5640_Write(0x3820, reg_value);

    reg_value = I2C_OV5640_Read(0x3821);
    reg_value |= 0x06;
    I2C_OV5640_Write(0x3821, reg_value);
}

void MOV5640_1920x1080(int format)
{
    unsigned int i;
    unsigned char reg_value = 0x00;

    for(i = 0; i < ARRAY_SIZE(StrMOV40_1920X1080); i++)
    {
        I2C_OV5640_Write(StrMOV40_1920X1080[i].addr, StrMOV40_1920X1080[i].value);
        udelay(10);
    }

    //FLIP 翻转
    reg_value = I2C_OV5640_Read(0x3820);
    reg_value |= 0x06;
    I2C_OV5640_Write(0x3820, reg_value);

    reg_value = I2C_OV5640_Read(0x3821);
    reg_value |= 0x06;
    I2C_OV5640_Write(0x3821, reg_value);
}

//MOV5640 MIPI stream on
void MOV5640_MIPI_stream_on(void)
{
    unsigned char reg_value = 0x00;
    I2C_OV5640_Write(0x4202, reg_value);

}

//MOV5640 MIPI stream off
void MOV5640_MIPI_stream_off(void)
{
    unsigned char reg_value = 0x0f;
    I2C_OV5640_Write(0x4202, reg_value);

}

int mov5640_isp_init(int size, int format)
{
    CK_UINT8 chip_id;

    I2C_OV5640_Initial();
	#if CONFIG_IS_ASIC //for ASIC version, PHY configure by APB port

    CK_Gpio_Output(OV_PWDN_IONB, 1);
	CK_Gpio_Output(OV_RESET_IONB, 0);
	CK_Gpio_Output(OV_PWDN_IONB_CH1, 1);
	CK_Gpio_Output(OV_RESET_IONB_CH1, 0);
    
    udelay(100 * 1000);
	CK_Gpio_Output(OV_PWDN_IONB_CH1, 0);
	CK_Gpio_Output(OV_PWDN_IONB, 0);
	
    udelay(100 * 1000);
	CK_Gpio_Output(OV_RESET_IONB_CH1, 1);
	CK_Gpio_Output(OV_RESET_IONB, 1);
   
    udelay(100 * 1000);	

    #endif

    udelay(10 * 1000);
    printf("  mov5640_init  \r\n");
    //read sensor chip id
    chip_id = I2C_OV5640_Read(0x300a);  //the Chip ID high byte is 0x56
    printf("camera ov5640_mipi is 0x300a,ID read is 0x%x\n",chip_id);
	if(chip_id != 0x56)
	{
	    printf("camera ov5640_mipi is not found,ID read is 0x%x\n",chip_id);
		return -1;
	}
	chip_id = I2C_OV5640_Read(0x300b);  //the Chip ID low byte is 0x40
	if(chip_id != 0x40)
	{
		 printf("camera ov5640_mipi is not found,ID read is 0x%x\n",chip_id);
		return -1;
	}
    printf("camera ov5640_mipi is 0x300b,ID read is 0x%x\n",chip_id);

    MOV5640_MIPI_stream_off();

    //MOV5640 initial
    MOV5640_isp_Format_Init(format);

    MOV5640_RAW_1280x720(format);

    MOV5640_MIPI_stream_off();
    return 0;
}


int mov5640_init(int size, int format)
{
    CK_UINT8 chip_id;

    I2C_OV5640_Initial();
#if CONFIG_IS_ASIC //for ASIC version, PHY configure by APB port

    CK_Gpio_Output(OV_PWDN_IONB, 1);
	CK_Gpio_Output(OV_RESET_IONB, 0);
	CK_Gpio_Output(OV_PWDN_IONB_CH1, 1);
	CK_Gpio_Output(OV_RESET_IONB_CH1, 0);
    
    udelay(100 * 1000);
	CK_Gpio_Output(OV_PWDN_IONB, 0);
	CK_Gpio_Output(OV_PWDN_IONB_CH1, 0);
	
    udelay(100 * 1000);
	CK_Gpio_Output(OV_RESET_IONB, 1);
	CK_Gpio_Output(OV_RESET_IONB_CH1, 1);
   
    udelay(100 * 1000);	

#endif

    //唤醒后，需要足够的延时等待，否则读写寄存器有可能失败
    udelay(100 * 1000);
    printf("  mov5640_init  \r\n");
    //read sensor chip id
    chip_id = I2C_OV5640_Read(0x300a);  //the Chip ID high byte is 0x56
    printf("camera ov5640_mipi is 0x300a,ID read is 0x%x\n",chip_id);
	if(chip_id != 0x56)
	{
	    printf("camera ov5640_mipi is not found,ID read is 0x%x\n",chip_id);
		return -1;
	}
	chip_id = I2C_OV5640_Read(0x300b);  //the Chip ID low byte is 0x40
	if(chip_id != 0x40)
	{
		 printf("camera ov5640_mipi is not found,ID read is 0x%x\n",chip_id);
		return -1;
	}
    printf("camera ov5640_mipi is 0x300b,ID read is 0x%x\n",chip_id);

    MOV5640_MIPI_stream_off();

    //MOV5640 initial
    MOV5640_Format_Init(format);
    //MOV5640 video set
    if(size == IMAGE_VGA)
    {
        MOV5640_640x480(format);
    }
    else if(size == IMAGE_720P)
    {
        MOV5640_1280x720(format);
    }
    else if(size == IMAGE_1080P)
    {
        MOV5640_1920x1080(format);
    }

    MOV5640_MIPI_stream_off();
    return 0;
}

