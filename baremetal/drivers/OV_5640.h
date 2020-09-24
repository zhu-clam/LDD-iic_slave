#ifndef _SENSOR_MOV5640_H_
#define _SENSOR_MOV5640_H_

#include "iic.h"

//#define __ISP_TEST__
#define __MIPI_TEST__

#define __CFG_0V5640_1__
#define __CFG_0V5640_2__
#define __CFG_0V5640_3__


 
//#define __OV5640_80MBPS__   80
//#define __OV5640_160MBPS__   160
#define __OV5640_800MBPS__   800
//#define __OV5640_400MBPS__   400
//#define __OV5640_600MBPS__   600

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
//#define TEST_PATTERN

struct mov5640_regval_list {
    unsigned short  addr;
    unsigned char   value;
};

//define the image format
#define CSI_YUV422_8B                       0
#define CSI_RGB565                          1
#define CSI_YUV420_8B                       2 // NV12
#define CSI_RGB555                          3
#define CSI_RGB444                          4
#define CSI_RGB888                          5
#define CSI_YUV420_8B_NV21                  6 // NV21
//define for MIPI CSI lane number
#define CSI_1LANES                          1
#define CSI_2LANES                          2
#define CSI_3LANES                          3
#define CSI_4LANES                          4
//define for the image size
#define IMAGE_VGA                           0
#define IMAGE_720P                          1
#define IMAGE_1080P                         2

/****************************MOV5640 CFG INIT with MIPI interface**************************/
//**fugui maintain*************************************************************************
//**Reference: MOV5640_Auto_Focus_Camera_Module_Application_Notes(with MIPI interface)_R2.15
//*****************************************************************************************
const static struct mov5640_regval_list StrMOV5640_CFG_INIT[] =
{
    //24 MHz input clock
    {0x3103, 0x11}, // system clock from pad
    {0x3008, 0x82}, // software reset
    // delay 5ms
    {0x3008, 0x42}, // software power down
    {0x3103, 0x03}, // system clock from PLL
    {0x3017, 0x00}, // set FREX, Vsync, HREF, PCLK, D[9:6] input, D9-4 share with MIPI pins
    {0x3018, 0x00}, // set D[5:0], GPIO[1:0] input
    {0x3034, 0x18}, // MIPI 8-bit   mode
    //{0x3037, 0x12}, //rxbyteclk=40MHz
    //{0x3037, 0x13}, // PLL root divider(by 2), PLL pre-divider(3) , rxbyteclk=26MHz, it must make FPGA pixclk > rxbyteclk
    {0x3037, 0x14}, // rxbytclk=20MHz
    //{0x3037, 0x16}, //PLL频率输出更改??,  rxbyteclk=13MHz, sensor can not work normal at this clock
    {0x3108, 0x01}, // PCLK root divider(/1), SCLK2x root divider(/1), SCLK root divider(/2)
    {0x3630, 0x36}, //
    {0x3631, 0x0e}, //
    {0x3632, 0xe2}, //
    {0x3633, 0x12}, //
    {0x3621, 0xe0}, //
    {0x3704, 0xa0}, //
    {0x3703, 0x5a}, //
    {0x3715, 0x78}, //
    {0x3717, 0x01}, //
    {0x370b, 0x60}, //
    {0x3705, 0x1a}, //
    {0x3905, 0x02}, //
    {0x3906, 0x10}, //
    {0x3901, 0x0a}, //
    {0x3731, 0x12}, //
    {0x3600, 0x08}, //
    {0x3601, 0x33}, //
    {0x302d, 0x60}, //
    {0x3620, 0x52}, //
    {0x371b, 0x20}, //
    {0x471c, 0x50}, //
    {0x3a13, 0x43}, // AGC pre-gain = 1.047x
    {0x3a18, 0x00}, // gain ceiling
    {0x3a19, 0xf8}, // gain ceiling = 15.5x
    {0x3635, 0x13}, //
    {0x3636, 0x03}, //
    {0x3634, 0x40}, //
    {0x3622, 0x01}, //
    // 50/60Hz detection 50/60Hz 灯光条纹过滤
    {0x3c01, 0x34}, // sum auto mode enable, band counter enable, band counter(4)
    {0x3c04, 0x28}, // threshold for low sum
    {0x3c05, 0x98}, // threshold for high sum
    {0x3c06, 0x00}, // light meter 1 threshold[15:8]
    //{0x3c07, 0x08}, // light meter 1 threshold[7:0]
    {0x3c08, 0x00}, // light meter 2 threshold[15:8]
    {0x3c09, 0x1c}, // light meter 2 threshold[7:0]
    {0x3c0a, 0x9c}, // sample number[15:8]
    {0x3c0b, 0x40}, // sample number[7:0]
    //timing
    {0x3800, 0x00}, // Timing HS, x address start[11:8]
    {0x3801, 0x00}, // Timing HS, x address start[7:0]
    {0x3802, 0x00}, // Timing VS, y address start[11:8]
    //{0x3803, 0x00}, // Timing VS, y address start[7:0]
    {0x3804, 0x0a}, // Timing HW, x address end[11:8], default
    {0x3805, 0x3f}, // Timing HW, x address end[7:0], default
    //{0x3806, 0x07}, // Timing VH, y address end[10:8], default
    //{0x3807, 0x9f}, // Timing VH, y address end[7:0], default
    {0x3810, 0x00}, // Timing Hoffset[11:8], H offset high
    {0x3811, 0x10}, // Timing Hoffset[7:0], H offset low
    {0x3812, 0x00}, // Timing Voffset[10:8], V offset high
    //{0x3813, 0x04}, // Timing Voffset[7:0], V offset low
    {0x3708, 0x64},
    {0x3a08, 0x01}, // B50[9:8], 50Hz band width
    //{0x3a09, 0x27}, // B50[7:0]
    {0x4001, 0x02}, // BLC start from line 2
    {0x4005, 0x1a}, // BLC always update
    {0x3000, 0x00}, // system reset 0, 0: enable block, 1: reset block
    {0x3002, 0x1c}, // system reset 2
    {0x3004, 0xff}, // clock enable 0, 0: disable clock, 1: enable clock
    {0x3006, 0xc3}, // clock enable 2
#ifdef TEST_PATTERN
    {0x503d, 0x80}, // color bar enable, standard 8 color bar
    {0x4741, 0x00},
#else
    {0x503d, 0x00}, // color bar enable, standard 8 color bar
    {0x4741, 0x00},
#endif
    //{0x4800, 0x0c}, // lane2 as default lane
    //{0x4800, 0x10}, //add by fuwei, send line short packet
    {0x300e, 0x45}, // MIPI control, 2 lane, MIPI enable
    {0x302e, 0x08}, // system control
    //{0x4300, 0x30}, // YUV 422, YUYV
    {0x4300, 0x32}, // YUV 422, UYVY
    //{0x501f, 0x00}, // ISP YUV 422
    {0x4407, 0x04}, // JPEG QS
    {0x440e, 0x00}, //
    //{0x4740, 0x21}, //HS、VS、PLCK极性设置
    {0x5000, 0xa7}, // ISP control, Lenc on, raw gamma on, BPC on, WPC on, CIP on
    // AWB 自动白平衡
    {0x5180, 0xff}, // AWB B block
    {0x5181, 0xf2}, // AWB control
    {0x5182, 0x00}, // [7:4] max local counter, [3:0] max fast counter
    {0x5183, 0x14}, // AWB advanced
    {0x5184, 0x25},
    {0x5185, 0x24},
    {0x5186, 0x09},
    {0x5187, 0x09},
    {0x5188, 0x09},
    {0x5189, 0x75},
    {0x518a, 0x54},
    {0x518b, 0xe0},
    {0x518c, 0xb2},
    {0x518d, 0x42},
    {0x518e, 0x3d},
    {0x518f, 0x56},
    {0x5190, 0x46},
    {0x5191, 0xf8}, // AWB top limit
    {0x5192, 0x04}, // AWB bottom limit
    {0x5193, 0x70}, // red limit
    {0x5194, 0xf0}, // green limit
    {0x5195, 0xf0}, // blue limit
    {0x5196, 0x03}, // AWB control
    {0x5197, 0x01}, // local limit
    {0x5198, 0x04}, //
    {0x5199, 0x12}, //
    {0x519a, 0x04}, //
    {0x519b, 0x00}, //
    {0x519c, 0x06}, //
    {0x519d, 0x82}, //
    {0x519e, 0x38}, // AWB control
    // color matrix 色彩矩阵
    {0x5381, 0x1e}, // CMX1 for Y
    {0x5382, 0x5b}, // CMX2 for Y
    {0x5383, 0x08}, // CMX3 for Y
    {0x5384, 0x0a}, // CMX4 for U
    {0x5385, 0x7e}, // CMX5 for U
    {0x5386, 0x88}, // CMX6 for U
    {0x5387, 0x7c}, // CMX7 for V
    {0x5388, 0x6c}, // CMX8 for V
    {0x5389, 0x10}, // CMX9 for V
    {0x538a, 0x01}, // sign[9]
    {0x538b, 0x98}, // sign[8:1]
    // CIP 锐化和降噪
    {0x5300, 0x08}, // CIP sharpen MT threshold 1
    {0x5301, 0x30}, // CIP sharpen MT threshold 2
    {0x5302, 0x10}, // CIP sharpen MT offset 1
    {0x5303, 0x00}, // CIP sharpen MT offset 2
    {0x5304, 0x08}, // CIP DNS threshold 1
    {0x5305, 0x30}, // CIP DNS threshold 2
    {0x5306, 0x08}, // CIP DNS offset 1
    {0x5307, 0x16}, // CIP DNS offset 2
    {0x5309, 0x08}, // CIP sharpen TH threshold 1
    {0x530a, 0x30}, // CIP sharpen TH threshold 2
    {0x530b, 0x04}, // CIP sharpen TH offset 1
    {0x530c, 0x06}, // CIP sharpen TH offset 2
    // Gamma 伽玛曲线
    {0x5480, 0x01}, // Gamma bias plus on, bit[0]
    {0x5481, 0x08},
    {0x5482, 0x14},
    {0x5483, 0x28},
    {0x5484, 0x51},
    {0x5485, 0x65},
    {0x5486, 0x71},
    {0x5487, 0x7d},
    {0x5488, 0x87},
    {0x5489, 0x91},
    {0x548a, 0x9a},
    {0x548b, 0xaa},
    {0x548c, 0xb8},
    {0x548d, 0xcd},
    {0x548e, 0xdd},
    {0x548f, 0xea},
    {0x5490, 0x1d},
    // UV adjust UV色彩饱和度调整
    {0x5580, 0x06}, // saturation on, contrast on
    {0x5583, 0x40}, // sat U
    {0x5584, 0x10}, // sat V
    {0x5589, 0x10}, // UV adjust th1
    {0x558a, 0x00}, // UV adjust th2[8]
    {0x558b, 0xf8}, // UV adjust th2[7:0]
    {0x501d, 0x04},//0x40, // enable manual offset of contrast
    // Lens correction for ? 镜头补偿
    {0x5800, 0x23},
    {0x5801, 0x14},
    {0x5802, 0x0f},
    {0x5803, 0x0f},
    {0x5804, 0x12},
    {0x5805, 0x26},
    {0x5806, 0x0c},
    {0x5807, 0x08},
    {0x5808, 0x05},
    {0x5809, 0x05},
    {0x580a, 0x08},
    {0x580b, 0x0d},
    {0x580c, 0x08},
    {0x580d, 0x03},
    {0x580e, 0x00},
    {0x580f, 0x00},
    {0x5810, 0x03},
    {0x5811, 0x09},
    {0x5812, 0x07},
    {0x5813, 0x03},
    {0x5814, 0x00},
    {0x5815, 0x01},
    {0x5816, 0x03},
    {0x5817, 0x08},
    {0x5818, 0x0d},
    {0x5819, 0x08},
    {0x581a, 0x05},
    {0x581b, 0x06},
    {0x581c, 0x08},
    {0x581d, 0x0e},
    {0x581e, 0x29},
    {0x581f, 0x17},
    {0x5820, 0x11},
    {0x5821, 0x11},
    {0x5822, 0x15},
    {0x5823, 0x28},
    {0x5824, 0x46},
    {0x5825, 0x26},
    {0x5826, 0x08},
    {0x5827, 0x26},
    {0x5828, 0x64},
    {0x5829, 0x26},
    {0x582a, 0x24},
    {0x582b, 0x22},
    {0x582c, 0x24},
    {0x582d, 0x24},
    {0x582e, 0x06},
    {0x582f, 0x22},
    {0x5830, 0x40},
    {0x5831, 0x42},
    {0x5832, 0x24},
    {0x5833, 0x26},
    {0x5834, 0x24},
    {0x5835, 0x22},
    {0x5836, 0x22},
    {0x5837, 0x26},
    {0x5838, 0x44},
    {0x5839, 0x24},
    {0x583a, 0x26},
    {0x583b, 0x28},
    {0x583c, 0x42},
    {0x583d, 0xce}, // lenc BR offset
    // AEC target 自动曝光控制
    {0x5025, 0x00},
    {0x3a0f, 0x30}, // stable range in high
    {0x3a10, 0x28}, // stable range in low
    {0x3a1b, 0x30}, // stable range out high
    {0x3a1e, 0x26}, // stable range out low
    {0x3a11, 0x60}, // fast zone high
    {0x3a1f, 0x14}, // fast zone low
    {0x4202, 0x0f}, // MIPI stream off
    {0x3008, 0x02}, // wake up from standby, bit[6]
};

const static struct mov5640_regval_list StrMOV5640_CFG_ISP_INIT[] =
{
  //24 MHz input clock
    {0x3103, 0x11}, // system clock from pad
    {0x3008, 0x82}, // software reset
    // delay 5ms
    {0x3008, 0x42}, // software power down
    {0x3103, 0x03}, // system clock from PLL
    {0x3017, 0x00}, // set FREX, Vsync, HREF, PCLK, D[9:6] input, D9-4 share with MIPI pins
    {0x3018, 0x00}, // set D[5:0], GPIO[1:0] input
#if 0
    {0x3034, 0x18}, // MIPI  8-bit mdoe
    {0x3037, 0x18}, // rxbytclk=10MHz
#else
    {0x3034, 0x1A}, // MIPI 10-bit   mode
    {0x3037, 0x14}, // rxbytclk=20MHz
#endif
    //{0x3037, 0x12}, //rxbyteclk=40MHz
    //{0x3037, 0x13}, // PLL root divider(by 2), PLL pre-divider(3) , rxbyteclk=26MHz, it must make FPGA pixclk > rxbyteclk
    //{0x3037, 0x16}, //PLL频率输出更改??,  rxbyteclk=13MHz, sensor can not work normal at this clock
    {0x3108, 0x01}, // PCLK root divider(/1), SCLK2x root divider(/1), SCLK root divider(/2)
    {0x3630, 0x36}, //
    {0x3631, 0x0e}, //
    {0x3632, 0xe2}, //
    {0x3633, 0x12}, //
    {0x3621, 0xe0}, //
    {0x3704, 0xa0}, //
    {0x3703, 0x5a}, //
    {0x3715, 0x78}, //
    {0x3717, 0x01}, //
    {0x370b, 0x60}, //
    {0x3705, 0x1a}, //
    {0x3905, 0x02}, //
    {0x3906, 0x10}, //
    {0x3901, 0x0a}, //
    {0x3731, 0x12}, //
    {0x3600, 0x08}, //
    {0x3601, 0x33}, //
    {0x302d, 0x60}, //
    {0x3620, 0x52}, //
    {0x371b, 0x20}, //
    {0x471c, 0x50}, //
    {0x3a13, 0x43}, // AGC pre-gain = 1.047x
    {0x3a18, 0x00}, // gain ceiling
    {0x3a19, 0xf8}, // gain ceiling = 15.5x
    {0x3635, 0x13}, //
    {0x3636, 0x03}, //
    {0x3634, 0x40}, //
    {0x3622, 0x01}, //
    // 50/60Hz detection 50/60Hz 灯光条纹过滤
    {0x3c01, 0x34}, // sum auto mode enable, band counter enable, band counter(4)
    {0x3c04, 0x28}, // threshold for low sum
    {0x3c05, 0x98}, // threshold for high sum
    {0x3c06, 0x00}, // light meter 1 threshold[15:8]
    //{0x3c07, 0x08}, // light meter 1 threshold[7:0]
    {0x3c08, 0x00}, // light meter 2 threshold[15:8]
    {0x3c09, 0x1c}, // light meter 2 threshold[7:0]
    {0x3c0a, 0x9c}, // sample number[15:8]
    {0x3c0b, 0x40}, // sample number[7:0]
    //timing
    {0x3800, 0x00}, // Timing HS, x address start[11:8]
    {0x3801, 0x00}, // Timing HS, x address start[7:0]
    {0x3802, 0x00}, // Timing VS, y address start[11:8]
    //{0x3803, 0x00}, // Timing VS, y address start[7:0]
    {0x3804, 0x0a}, // Timing HW, x address end[11:8], default
    {0x3805, 0x3f}, // Timing HW, x address end[7:0], default
    //{0x3806, 0x07}, // Timing VH, y address end[10:8], default
    //{0x3807, 0x9f}, // Timing VH, y address end[7:0], default
    {0x3810, 0x00}, // Timing Hoffset[11:8], H offset high
    {0x3811, 0x10}, // Timing Hoffset[7:0], H offset low
    {0x3812, 0x00}, // Timing Voffset[10:8], V offset high
    //{0x3813, 0x04}, // Timing Voffset[7:0], V offset low
    {0x3708, 0x64},
    {0x3a08, 0x01}, // B50[9:8], 50Hz band width
    //{0x3a09, 0x27}, // B50[7:0]
    {0x4001, 0x02}, // BLC start from line 2
    {0x4005, 0x1a}, // BLC always update
    {0x3000, 0x00}, // system reset 0, 0: enable block, 1: reset block
    {0x3002, 0x1c}, // system reset 2
    {0x3004, 0xff}, // clock enable 0, 0: disable clock, 1: enable clock
    {0x3006, 0xc3}, // clock enable 2
#ifdef TEST_PATTERN
    {0x503d, 0x80}, // color bar enable, standard 8 color bar
    {0x4741, 0x00},
#else
    {0x503d, 0x00}, // color bar enable, standard 8 color bar
    {0x4741, 0x00},
#endif
    //{0x4800, 0x0c}, // lane2 as default lane
    //{0x4800, 0x10}, //add by fuwei, send line short packet
    {0x300e, 0x45}, // MIPI control, 2 lane, MIPI enable
    {0x302e, 0x08}, // system control
    //{0x4300, 0x30}, // YUV 422, YUYV
    {0x4300, 0x02},
    {0x501f, 0x03},
    {0x4407, 0x04}, // JPEG QS
    {0x440e, 0x00}, //
    //{0x4740, 0x21}, //HS、VS、PLCK极性设置
    {0x5000, 0xa7}, // ISP control, Lenc on, raw gamma on, BPC on, WPC on, CIP on
    // AWB 自动白平衡
    {0x5180, 0xff}, // AWB B block
    {0x5181, 0xf2}, // AWB control
    {0x5182, 0x00}, // [7:4] max local counter, [3:0] max fast counter
    {0x5183, 0x14}, // AWB advanced
    {0x5184, 0x25},
    {0x5185, 0x24},
    {0x5186, 0x09},
    {0x5187, 0x09},
    {0x5188, 0x09},
    {0x5189, 0x75},
    {0x518a, 0x54},
    {0x518b, 0xe0},
    {0x518c, 0xb2},
    {0x518d, 0x42},
    {0x518e, 0x3d},
    {0x518f, 0x56},
    {0x5190, 0x46},
    {0x5191, 0xf8}, // AWB top limit
    {0x5192, 0x04}, // AWB bottom limit
    {0x5193, 0x70}, // red limit
    {0x5194, 0xf0}, // green limit
    {0x5195, 0xf0}, // blue limit
    {0x5196, 0x03}, // AWB control
    {0x5197, 0x01}, // local limit
    {0x5198, 0x04}, //
    {0x5199, 0x12}, //
    {0x519a, 0x04}, //
    {0x519b, 0x00}, //
    {0x519c, 0x06}, //
    {0x519d, 0x82}, //
    {0x519e, 0x38}, // AWB control
    // color matrix 色彩矩阵
    {0x5381, 0x1e}, // CMX1 for Y
    {0x5382, 0x5b}, // CMX2 for Y
    {0x5383, 0x08}, // CMX3 for Y
    {0x5384, 0x0a}, // CMX4 for U
    {0x5385, 0x7e}, // CMX5 for U
    {0x5386, 0x88}, // CMX6 for U
    {0x5387, 0x7c}, // CMX7 for V
    {0x5388, 0x6c}, // CMX8 for V
    {0x5389, 0x10}, // CMX9 for V
    {0x538a, 0x01}, // sign[9]
    {0x538b, 0x98}, // sign[8:1]
    // CIP 锐化和降噪
    {0x5300, 0x08}, // CIP sharpen MT threshold 1
    {0x5301, 0x30}, // CIP sharpen MT threshold 2
    {0x5302, 0x10}, // CIP sharpen MT offset 1
    {0x5303, 0x00}, // CIP sharpen MT offset 2
    {0x5304, 0x08}, // CIP DNS threshold 1
    {0x5305, 0x30}, // CIP DNS threshold 2
    {0x5306, 0x08}, // CIP DNS offset 1
    {0x5307, 0x16}, // CIP DNS offset 2
    {0x5309, 0x08}, // CIP sharpen TH threshold 1
    {0x530a, 0x30}, // CIP sharpen TH threshold 2
    {0x530b, 0x04}, // CIP sharpen TH offset 1
    {0x530c, 0x06}, // CIP sharpen TH offset 2
    // Gamma 伽玛曲线
    {0x5480, 0x01}, // Gamma bias plus on, bit[0]
    {0x5481, 0x08},
    {0x5482, 0x14},
    {0x5483, 0x28},
    {0x5484, 0x51},
    {0x5485, 0x65},
    {0x5486, 0x71},
    {0x5487, 0x7d},
    {0x5488, 0x87},
    {0x5489, 0x91},
    {0x548a, 0x9a},
    {0x548b, 0xaa},
    {0x548c, 0xb8},
    {0x548d, 0xcd},
    {0x548e, 0xdd},
    {0x548f, 0xea},
    {0x5490, 0x1d},
    // UV adjust UV色彩饱和度调整
    {0x5580, 0x06}, // saturation on, contrast on
    {0x5583, 0x40}, // sat U
    {0x5584, 0x10}, // sat V
    {0x5589, 0x10}, // UV adjust th1
    {0x558a, 0x00}, // UV adjust th2[8]
    {0x558b, 0xf8}, // UV adjust th2[7:0]
    {0x501d, 0x04},//0x40, // enable manual offset of contrast
    // Lens correction for ? 镜头补偿
    {0x5800, 0x23},
    {0x5801, 0x14},
    {0x5802, 0x0f},
    {0x5803, 0x0f},
    {0x5804, 0x12},
    {0x5805, 0x26},
    {0x5806, 0x0c},
    {0x5807, 0x08},
    {0x5808, 0x05},
    {0x5809, 0x05},
    {0x580a, 0x08},
    {0x580b, 0x0d},
    {0x580c, 0x08},
    {0x580d, 0x03},
    {0x580e, 0x00},
    {0x580f, 0x00},
    {0x5810, 0x03},
    {0x5811, 0x09},
    {0x5812, 0x07},
    {0x5813, 0x03},
    {0x5814, 0x00},
    {0x5815, 0x01},
    {0x5816, 0x03},
    {0x5817, 0x08},
    {0x5818, 0x0d},
    {0x5819, 0x08},
    {0x581a, 0x05},
    {0x581b, 0x06},
    {0x581c, 0x08},
    {0x581d, 0x0e},
    {0x581e, 0x29},
    {0x581f, 0x17},
    {0x5820, 0x11},
    {0x5821, 0x11},
    {0x5822, 0x15},
    {0x5823, 0x28},
    {0x5824, 0x46},
    {0x5825, 0x26},
    {0x5826, 0x08},
    {0x5827, 0x26},
    {0x5828, 0x64},
    {0x5829, 0x26},
    {0x582a, 0x24},
    {0x582b, 0x22},
    {0x582c, 0x24},
    {0x582d, 0x24},
    {0x582e, 0x06},
    {0x582f, 0x22},
    {0x5830, 0x40},
    {0x5831, 0x42},
    {0x5832, 0x24},
    {0x5833, 0x26},
    {0x5834, 0x24},
    {0x5835, 0x22},
    {0x5836, 0x22},
    {0x5837, 0x26},
    {0x5838, 0x44},
    {0x5839, 0x24},
    {0x583a, 0x26},
    {0x583b, 0x28},
    {0x583c, 0x42},
    {0x583d, 0xce}, // lenc BR offset
    // AEC target 自动曝光控制
    {0x5025, 0x00},
    {0x3a0f, 0x30}, // stable range in high
    {0x3a10, 0x28}, // stable range in low
    {0x3a1b, 0x30}, // stable range out high
    {0x3a1e, 0x26}, // stable range out low
    {0x3a11, 0x60}, // fast zone high
    {0x3a1f, 0x14}, // fast zone low
    {0x4202, 0x0f}, // MIPI stream off
    {0x3008, 0x02}, // wake up from standby, bit[6]

};

const static struct mov5640_regval_list StrMOV40_1920X1080[] =
{
	#ifdef __OV5640_80MBPS__
		    {0x3035, 0x61}, //80MBPS
		    {0x3036, 0x13},
		    {0x3037, 0x11},
	#endif
	#ifdef __OV5640_160MBPS__
		    {0x3035, 0x61}, //160MBPS
		    {0x3036, 0x27},
		    {0x3037, 0x11},
	#endif 
	#ifdef __OV5640_800MBPS__
		{0x3035, 0x11}, //800Mbps
		{0x3036, 0x64},
		{0x3037, 0x13},
	#endif
	#ifdef __OV5640_400MBPS__
		{0x3035, 0x11}, //400Mbps
		{0x3036, 0x64},
		{0x3037, 0x16},
    #endif
	
	#ifdef __OV5640_600MBPS__
		{0x3035, 0x11}, //600Mbps
		{0x3036, 0x64},
		{0x3037, 0x14},
    #endif

    {0x3c07, 0x07}, // lightm eter 1 threshold[7:0]
    {0x3820, 0x40}, // flip
    {0x3821, 0x06}, // mirror
    {0x3814, 0x11}, // timing X inc
    {0x3815, 0x11}, // timing Y inc
    {0x3800, 0x01}, // HS
    {0x3801, 0x50}, // HS
    {0x3802, 0x01}, // VS
    {0x3803, 0xb2}, // VS
    {0x3804, 0x08}, // HW (HE)
    {0x3805, 0xef}, // HW (HE)
    {0x3806, 0x05},// VH (VE)
    {0x3807, 0xf1},// VH (VE)
    {0x3808, 0x07}, // DVPHO??
    {0x3809, 0x80}, // DVPHO??
    {0x380a, 0x04}, // DVPVO??
    {0x380b, 0x38}, // DVPVO??
    {0x380c, 0x09}, // HTS
    {0x380d, 0xc4}, // HTS
    {0x380e, 0x04}, // VTS
    {0x380f, 0x60}, // VTS

    {0x3813, 0x04}, // timing V offset
    {0x3618, 0x04},
    {0x3612, 0x2b},
    {0x3709, 0x12},
    {0x370c, 0x00},
    {0x4004, 0x06}, // BLC line number
    {0x3002, 0x1c}, // reset JFIFO, SFIFO, JPG
    {0x3006, 0xc3}, // disable clock of JPEG2x, JPEG
    {0x4713, 0x02}, // JPEG mode 3
    {0x4407, 0x0c}, // Quantization sacle
    {0x460b, 0x37},
    {0x460c, 0x20},
    {0x4837, 0x18}, // MIPI global timing
    {0x3824, 0x01}, // PCLK manual divider
    {0x5001, 0x83}, // SDE on, CMX on, AWB on
    {0x3503, 0x00}, // AEC/AGC on
};

const static struct mov5640_regval_list StrMOV40_640X480[] =
{
	#ifdef __OV5640_80MBPS__
		    {0x3035, 0x61}, //80MBPS
		    {0x3036, 0x13},
		    {0x3037, 0x11},
	#endif
	
	#ifdef __OV5640_160MBPS__
		    {0x3035, 0x61}, //160MBPS
		    {0x3036, 0x27},
		    {0x3037, 0x11},
	#endif
	#ifdef __OV5640_800MBPS__
		{0x3035, 0x11}, //800Mbps
		{0x3036, 0x64},
		{0x3037, 0x13},
    #endif
	#ifdef __OV5640_400MBPS__
		{0x3035, 0x11}, //400Mbps
		{0x3036, 0x64},
		{0x3037, 0x16},
    #endif
	#ifdef __OV5640_600MBPS__
		{0x3035, 0x11}, //600Mbps
		{0x3036, 0x64},
		{0x3037, 0x14},
    #endif
    {0x3c07, 0x07}, // lightmeter 1 threshold[7:0]
    {0x3820, 0x40}, // flip //垂直反相
    {0x3821, 0x01}, // mirror
    {0x3814, 0x31}, // timing X inc
    {0x3815, 0x31}, // timing Y inc
    {0x3800, 0x00}, // HS
    {0x3801, 0x00}, // HS
    {0x3802, 0x00}, // VS
    {0x3803, 0xfa}, // VS
    {0x3804, 0x0a}, // HW (HE)
    {0x3805, 0x3f}, // HW (HE)
    {0x3806, 0x06}, // VH (VE)
    {0x3807, 0xa9}, // VH (VE)
    {0x3808, 0x02}, // DVPHO
    {0x3809, 0x80}, // DVPHO
    {0x380a, 0x01}, // DVPVO
    {0x380b, 0xE0}, // DVPVO
    {0x380c, 0x07}, // HTS
    {0x380d, 0x64}, // HTS
    {0x380e, 0x02}, // VTS
    {0x380f, 0xe4}, // VTS
    {0x3813, 0x04}, // timing V offset
    {0x3618, 0x00},
    {0x3612, 0x29},
    {0x3709, 0x52},
    {0x370c, 0x03},
    {0x3a02, 0x03},//0x02, // 60Hz max exposure
    {0x3a03, 0xd8},//0xe0, // 60Hz max exposure
    {0x3a09, 0x27},//0x6f, // B50 step
    {0x3a0a, 0x00}, // B60 step
    {0x3a0b, 0xf6},//0x5c, // B60 step
    {0x3a0e, 0x03},//0x06, // 50Hz max band
    {0x3a0d, 0x04},//0x08, // 60Hz max band
    {0x3a14, 0x03},//0x02, // 50Hz max exposure
    {0x3a15, 0xd8},//0xe0, // 50Hz max exposure
    {0x4004, 0x02}, // BLC line number
    {0x4713, 0x03}, // JPEG mode 3
    {0x460b, 0x35},//0x37,
    {0x460c, 0x20},
    {0x4837, 0x18}, // MIPI global timing
    {0x3824, 0x01}, // PCLK manual divider
    {0x5001, 0x83}, // SDE on, CMX on, AWB on
    {0x3503, 0x00}, // AEC/AGC on
};

const static struct mov5640_regval_list StrMOV40_1280X720[] =
{
	#ifdef __OV5640_80MBPS__
		    {0x3035, 0x61}, //80MBPS
		    {0x3036, 0x13},
		    {0x3037, 0x11},
	#endif
	#ifdef __OV5640_160MBPS__
    {0x3035, 0x61}, //160MBPS
    {0x3036, 0x27},
    {0x3037, 0x11},
	#endif
	#ifdef __OV5640_400MBPS__
		{0x3035, 0x11}, //400Mbps
		{0x3036, 0x64},
		{0x3037, 0x16},
    #endif
	#ifdef __OV5640_600MBPS__
		{0x3035, 0x11}, //600Mbps
		{0x3036, 0x64},
		{0x3037, 0x14},
    #endif
	#ifdef __OV5640_800MBPS__
	{0x3035, 0x11}, //800Mbps
	{0x3036, 0x64},
	{0x3037, 0x13},
	#endif

    {0x3c07, 0x07}, // lightmeter 1 threshold[7:0]
    {0x3820, 0x40}, // flip //垂直反相
    {0x3821, 0x01}, // mirror
    {0x3814, 0x31}, // timing X inc
    {0x3815, 0x31}, // timing Y inc
    {0x3800, 0x00}, // HS
    {0x3801, 0x00}, // HS
    {0x3802, 0x00}, // VS
    {0x3803, 0xfa}, // VS
    {0x3804, 0x0a}, // HW (HE)
    {0x3805, 0x3f}, // HW (HE)
    {0x3806, 0x06}, // VH (VE)
    {0x3807, 0xa9}, // VH (VE)
    {0x3808, 0x05}, // DVPHO
    {0x3809, 0x00}, // DVPHO
    {0x380a, 0x02}, // DVPVO
    {0x380b, 0xd0}, // DVPVO
    {0x380c, 0x06}, // HTS
    {0x380d, 0x72}, // HTS
    {0x380e, 0x02},//0x02}, // VTS
    {0x380f, 0xee}, // VTS*/
    {0x3813, 0x04}, // timing V offset
    {0x3618, 0x00},
    {0x3612, 0x29},
    {0x3709, 0x52},
    {0x370c, 0x03},
    {0x3a02, 0x02}, // 60Hz max exposure
    {0x3a03, 0xe0}, // 60Hz max exposure
    {0x3a14, 0x02}, // 50Hz max exposure
    {0x3a15, 0xe0}, // 50Hz max exposure
    {0x4004, 0x02}, // BLC line number
    {0x3002, 0x1c}, // reset JFIFO, SFIFO, JPG
    {0x3006, 0xc3}, // disable clock of JPEG2x, JPEG
    {0x4713, 0x03}, // JPEG mode 3
    {0x4407, 0x04}, // Quantization scale
    {0x460b, 0x37},
    {0x460c, 0x20},
    {0x4837, 0x16}, // MIPI global timing
    {0x3824, 0x01}, // PCLK manual divider
    {0x5001, 0x83}, // SDE on, CMX on, AWB on
    {0x3503, 0x00}, // AEC/AGC on
};

const static struct mov5640_regval_list StrMOV40_RAW_1280X720[] =
{

    #ifdef __OV5640_80MBPS__
		    {0x3035, 0x61}, //80MBPS
		    {0x3036, 0x13},
		    {0x3037, 0x11},
	#endif
    #ifdef __OV5640_160MBPS__
    {0x3035, 0x61}, //160MBPS
    {0x3036, 0x27},
    {0x3037, 0x11},
	#endif
	#ifdef __OV5640_400MBPS__
		{0x3035, 0x11}, //400Mbps
		{0x3036, 0x64},
		{0x3037, 0x16},
    #endif
	#ifdef __OV5640_800MBPS__
	{0x3035, 0x11}, //800Mbps
	{0x3036, 0x64},
	{0x3037, 0x13},
	#endif
	#ifdef __OV5640_600MBPS__
		{0x3035, 0x11}, //600Mbps
		{0x3036, 0x64},
		{0x3037, 0x14},
    #endif

    {0x3c07, 0x07}, // lightmeter 1 threshold[7:0]
    {0x3820, 0x40}, // flip //垂直反相
    {0x3821, 0x01}, // mirror
    {0x3814, 0x31}, // timing X inc
    {0x3815, 0x31}, // timing Y inc
    {0x3800, 0x00}, // HS
    {0x3801, 0x00}, // HS
    {0x3802, 0x00}, // VS
    {0x3803, 0xfa}, // VS
    {0x3804, 0x0a}, // HW (HE)
    {0x3805, 0x3f}, // HW (HE)
    {0x3806, 0x06}, // VH (VE)
    {0x3807, 0xa9}, // VH (VE)
    {0x3808, 0x05}, // DVPHO
    {0x3809, 0x00}, // DVPHO
    {0x380a, 0x02}, // DVPVO
    {0x380b, 0xd0}, // DVPVO
    {0x380c, 0x06}, // HTS
    {0x380d, 0x72}, // HTS
    {0x380e, 0x02},//0x02}, // VTS
    {0x380f, 0xee}, // VTS*/
    {0x3813, 0x04}, // timing V offset
    {0x3618, 0x00},
    {0x3612, 0x29},
    {0x3709, 0x52},
    {0x370c, 0x03},
    {0x3a02, 0x02}, // 60Hz max exposure
    {0x3a03, 0xe0}, // 60Hz max exposure
    {0x3a14, 0x02}, // 50Hz max exposure
    {0x3a15, 0xe0}, // 50Hz max exposure
    {0x4004, 0x02}, // BLC line number
    {0x3002, 0x1c}, // reset JFIFO, SFIFO, JPG
    {0x3006, 0xc3}, // disable clock of JPEG2x, JPEG
    {0x4713, 0x03}, // JPEG mode 3
    {0x4407, 0x04}, // Quantization scale
    {0x460b, 0x37},
    {0x460c, 0x20},
    {0x4837, 0x16}, // MIPI global timing
    {0x3824, 0x01}, // PCLK manual divider
    {0x5001, 0x83}, // SDE on, CMX on, AWB on
    {0x3503, 0x00}, // AEC/AGC on


};



//DPHY register map base address for CIL/CTL/PLL
#define TC_DPHYCIL_BASE   0x00
#define TC_DPHYCTRL_BASE  0x90
#define TC_PLL_BASE       0xA0

//DPHYCIL ADDRESS MAP
#define ADDR_CIL_APBWR_CTRL         (TC_DPHYCIL_BASE + 0x00)

#define ADDR_CIL_GLOBAL_CTRL0       (TC_DPHYCIL_BASE + 0x01)
#define ADDR_CIL_GLOBAL_CTRL1       (TC_DPHYCIL_BASE + 0x02)
#define ADDR_CIL_GLOBAL_CTRL2       (TC_DPHYCIL_BASE + 0x03)
#define ADDR_CIL_GLOBAL_CTRL3       (TC_DPHYCIL_BASE + 0x04)

#define ADDR_CIL_CNT_WAKEUP         (TC_DPHYCIL_BASE + 0x10)
#define ADDR_CIL_CNT_DREN_DELAY     (TC_DPHYCIL_BASE + 0x11)
#define ADDR_CIL_CNT_TX_RELINQUISH  (TC_DPHYCIL_BASE + 0x12)
#define ADDR_CIL_CNT_RX_RELINQUISH  (TC_DPHYCIL_BASE + 0x13)
#define ADDR_CIL_CNT_RX_TIMEOUTL    (TC_DPHYCIL_BASE + 0x14)
#define ADDR_CIL_CNT_RX_TIMEOUTH    (TC_DPHYCIL_BASE + 0x15)

#define ADDR_CIL_CNT_HSCK_LPX       (TC_DPHYCIL_BASE + 0x20)
#define ADDR_CIL_CNT_HSCK_PREP      (TC_DPHYCIL_BASE + 0x21)
#define ADDR_CIL_CNT_HSCK_ZERO      (TC_DPHYCIL_BASE + 0x22)
#define ADDR_CIL_CNT_HSCK_PRE       (TC_DPHYCIL_BASE + 0x23)
#define ADDR_CIL_CNT_HSCK_TRAIL     (TC_DPHYCIL_BASE + 0x24)
#define ADDR_CIL_CNT_HSCK_EXIT      (TC_DPHYCIL_BASE + 0x25)
#define ADDR_CIL_CNT_HSCK_POST      (TC_DPHYCIL_BASE + 0x26)

#define ADDR_CIL_CNT_HSCK_SETTLE    (TC_DPHYCIL_BASE + 0x27)

#define ADDR_CIL_CNT_HSD_LPX        (TC_DPHYCIL_BASE + 0x30)
#define ADDR_CIL_CNT_HSD_ZERO       (TC_DPHYCIL_BASE + 0x31)
#define ADDR_CIL_CNT_HSD_PREP       (TC_DPHYCIL_BASE + 0x32)
#define ADDR_CIL_CNT_HSD_TRAIL      (TC_DPHYCIL_BASE + 0x33)
#define ADDR_CIL_CNT_HSD_EXIT       (TC_DPHYCIL_BASE + 0x34)

#define ADDR_CIL_CNT_HSD_SETTLE     (TC_DPHYCIL_BASE + 0x35)

#define ADDR_CIL_CNT_TAGO           (TC_DPHYCIL_BASE + 0x40)
#define ADDR_CIL_CNT_TAGET          (TC_DPHYCIL_BASE + 0x41)
#define ADDR_CIL_CNT_TASURE         (TC_DPHYCIL_BASE + 0x42)
#define ADDR_CIL_CNT_TABG           (TC_DPHYCIL_BASE + 0x43)
#define ADDR_CIL_CNT_TAREQ_DLY      (TC_DPHYCIL_BASE + 0x44)

#define ADDR_CIL_DEBUG_REG0         (TC_DPHYCIL_BASE + 0x51)
#define ADDR_CIL_DCHECKERR          (TC_DPHYCIL_BASE + 0x52)
#define ADDR_CIL_RCALOUT            (TC_DPHYCIL_BASE + 0x53)

#define ADDR_CIL_CKLANE_CTL0        (TC_DPHYCIL_BASE + 0x60)
#define ADDR_CIL_CKLANE_CTL1        (TC_DPHYCIL_BASE + 0x61)
#define ADDR_CIL_CKLANE_CTL2        (TC_DPHYCIL_BASE + 0x62)

#define ADDR_CIL_D0LANE_CTL0        (TC_DPHYCIL_BASE + 0x63)
#define ADDR_CIL_D0LANE_CTL1        (TC_DPHYCIL_BASE + 0x64)
#define ADDR_CIL_D0LANE_CTL2        (TC_DPHYCIL_BASE + 0x65)

#define ADDR_CIL_D1LANE_CTL0        (TC_DPHYCIL_BASE + 0x66)
#define ADDR_CIL_D1LANE_CTL1        (TC_DPHYCIL_BASE + 0x67)
#define ADDR_CIL_D1LANE_CTL2        (TC_DPHYCIL_BASE + 0x68)

#define ADDR_CIL_D2LANE_CTL0        (TC_DPHYCIL_BASE + 0x69)
#define ADDR_CIL_D2LANE_CTL1        (TC_DPHYCIL_BASE + 0x6A)
#define ADDR_CIL_D2LANE_CTL2        (TC_DPHYCIL_BASE + 0x6B)

#define ADDR_CIL_D3LANE_CTL0        (TC_DPHYCIL_BASE + 0x6C)
#define ADDR_CIL_D3LANE_CTL1        (TC_DPHYCIL_BASE + 0x6D)
#define ADDR_CIL_D3LANE_CTL2        (TC_DPHYCIL_BASE + 0x6E)

#define ADDR_CIL_CKLANE_ST0         (TC_DPHYCIL_BASE + 0x70)
#define ADDR_CIL_D0LANE_ST0         (TC_DPHYCIL_BASE + 0x71)
#define ADDR_CIL_D0LANE_ST1         (TC_DPHYCIL_BASE + 0x72)
#define ADDR_CIL_D1LANE_ST0         (TC_DPHYCIL_BASE + 0x73)
#define ADDR_CIL_D1LANE_ST1         (TC_DPHYCIL_BASE + 0x74)
#define ADDR_CIL_D2LANE_ST0         (TC_DPHYCIL_BASE + 0x75)
#define ADDR_CIL_D2LANE_ST1         (TC_DPHYCIL_BASE + 0x76)
#define ADDR_CIL_D3LANE_ST0         (TC_DPHYCIL_BASE + 0x77)
#define ADDR_CIL_D3LANE_ST1         (TC_DPHYCIL_BASE + 0x78)

//DPHYCTL ADDRESS MAP
#define TCDPHY_GCTRL                (TC_DPHYCTRL_BASE + 0x00)
#define TCDPHY_ENABLE               (TC_DPHYCTRL_BASE + 0x01)

//DPHYPLL ADDRESS MAP
#define TCPLL_DM                    (TC_PLL_BASE + 0x00)
#define TCPLL_CLKF                  (TC_PLL_BASE + 0x01)
#define TCPLL_GCTRL                 (TC_PLL_BASE + 0x02)

#define TCOSCPLL_IN0                (TC_PLL_BASE + 0x08)
#define TCOSCPLL_IN1                (TC_PLL_BASE + 0x09)
#define TCOSCPLL_IN2                (TC_PLL_BASE + 0x0A)

int mov5640_init( int size, int format);
void MOV5640_MIPI_stream_on(void);

#endif //_SENSOR_MOV5640_H_
