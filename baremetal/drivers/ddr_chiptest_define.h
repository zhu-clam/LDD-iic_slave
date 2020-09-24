//--------------------------------//
//------part 1--------------------//
//--------------------------------//
#define DDR4_2400
#define DDR4_2133
#define DDR4_1600
#define DDR3_1600
#define DDR3_800
#define LPDDR3_1866

//#define DDR_PD_SR_EN
//#define DDR_QOS_TEST

#define DDR_LP_TEST
#define DDR_DCU_TEST
#define DDR_BIST_TEST


//--------------------------------//
//------part 2--------------------//
//--------------------------------//
//DDR4 physic define
#define DDR4_RTT_NOM_DIS 0
#define DDR4_RTT_NOM_4   1
#define DDR4_RTT_NOM_2   2
#define DDR4_RTT_NOM_6   3
#define DDR4_RTT_NOM_1   4
#define DDR4_RTT_NOM_5   5
#define DDR4_RTT_NOM_3   6
#define DDR4_RTT_NOM_7   7

#define DDR4_RTT_WR_ODT_OFF 0
#define DDR4_RTT_WR_2       1
#define DDR4_RTT_WR_1       2
#define DDR4_RTT_WR_Z       3
#define DDR4_RTT_WR_4       4

#define DDR4_RTT_PARK_DIS 0
#define DDR4_RTT_PARK_4   1
#define DDR4_RTT_PARK_2   2
#define DDR4_RTT_PARK_6   3
#define DDR4_RTT_PARK_1   4
#define DDR4_RTT_PARK_5   5
#define DDR4_RTT_PARK_3   6
#define DDR4_RTT_PARK_7   7
//need check
#define DDR4_VREFDQ_RANGE 0
#define DDR4_VREFDQ_VALUE 0x1D
#define DDR4_OUTPUT_DRIVE_IMP_7 0   // 240/7 = 34
#define DDR4_OUTPUT_DRIVE_IMP_5 1    // 240/5 = 48

//DDR3 physic define
//#define DDR3_RTT_NOM_DIS 0
//#define DDR3_RTT_NOM_4   1
//#define DDR3_RTT_NOM_2   2
//#define DDR3_RTT_NOM_6   3
//#define DDR3_RTT_NOM_12  4
//#define DDR3_RTT_NOM_8   5
//
//#define DDR3_RTT_WR_ODT_OFF 0
//#define DDR3_RTT_WR_4       1
//#define DDR3_RTT_WR_2       2

//define mode register value
#define DDR4_2400_MODE_REG0  0x0834
#define DDR4_2133_MODE_REG0  0x0624
#define DDR4_1600_MODE_REG0  0x0214

#define DDR4_2400_MODE_REG1  ((1<<0)|(DDR4_OUTPUT_DRIVE_IMP_7<<1)|(DDR4_RTT_NOM_4<<8))
#define DDR4_2133_MODE_REG1  ((1<<0)|(DDR4_OUTPUT_DRIVE_IMP_7<<1)|(DDR4_RTT_NOM_4<<8))
#define DDR4_1600_MODE_REG1  ((1<<0)|(DDR4_OUTPUT_DRIVE_IMP_7<<1)|(DDR4_RTT_NOM_4<<8)) // 34  60

#define DDR4_2400_MODE_REG2 ((3<<3)|(DDR4_RTT_WR_ODT_OFF<<9))
#define DDR4_2133_MODE_REG2 ((3<<3)|(DDR4_RTT_WR_ODT_OFF<<9))
#define DDR4_1600_MODE_REG2 ((2<<3)|(DDR4_RTT_WR_ODT_OFF<<9))

#define DDR4_2400_MODE_REG3  0x0200
#define DDR4_2133_MODE_REG3  0x0200
#define DDR4_1600_MODE_REG3  0x0

#define DDR4_2400_MODE_REG4  0x0200
#define DDR4_2133_MODE_REG4  0x0200
#define DDR4_1600_MODE_REG4  0x0200

#define DDR4_2400_MODE_REG5  ((1<<10)|(DDR4_RTT_PARK_DIS<<6))
#define DDR4_2133_MODE_REG5  ((1<<10)|(DDR4_RTT_PARK_DIS<<6))
#define DDR4_1600_MODE_REG5  ((1<<10)|(DDR4_RTT_PARK_DIS<<6))

#define DDR4_2400_MODE_REG6  (DDR4_VREFDQ_VALUE<<0)|(DDR4_VREFDQ_RANGE<<6)|(2<<10)
#define DDR4_2133_MODE_REG6  (DDR4_VREFDQ_VALUE<<0)|(DDR4_VREFDQ_RANGE<<6)|(2<<10)
#define DDR4_1600_MODE_REG6  (DDR4_VREFDQ_VALUE<<0)|(DDR4_VREFDQ_RANGE<<6)|(1<<10)


#define DDR3_1600_MODE_REG0  0x1D70
#define DDR3_800_MODE_REG0   0x1D70

#define DDR3_1600_MODE_REG1  0x0004
#define DDR3_800_MODE_REG1   0x0004

#define DDR3_1600_MODE_REG2  0x0018
#define DDR3_800_MODE_REG2   0x0018

#define DDR3_1600_MODE_REG3  0x0
#define DDR3_800_MODE_REG3   0x0


//--------------------------------//
//------part 3--------------------//
//--------------------------------//
//----------DDR PHY config define -------------------//
//#define PHY_DXCCR ((1<<29)|(1<<23)|(1<<22)|(0xc<<9)|(0x4<<5)|(1<<2))
//#define PHY_DXCCR ((1<<29)|(1<<23)|(1<<22)|(0x0<<9)|(0x0<<5)|(1<<2) | (1<<0)) //???
//#define PHY_ODTCR 0
#define PHY_ODTCR ((0 <<0) | (0x1 <<16))
#define PHY_AACR 0
#define PHY_DTCR0 ((8<<28)|(2<<14)|(1<<13)|(1<<12)|(1<<7)|(7<<0))
#define PHY_DTCR1 ((1<<16)|(1<<11)|(2<<8)|(3<<4)|(1<<2)|(1<<1)|(1<<0))
#define PHY_DTAR0  (4<<24)
#define PHY_DTAR1  (1<<16)
#define PHY_DTAR2  ((3<<16)|(2<<0))
#define PHY_ACIOCR0 ((1<<29)|(1<<28)|(1<<4))
#define PHY_ACIOCR1 0
//#define PHY_ACIOCR2 0
#define PHY_ACIOCR3 0
//#define PHY_ACIOCR4 0
#define PHY_IOVCR0 ((3<<26)|(1<<25)|(1<<24)|(9<<0))
#define PHY_IOVCR1 ((1<<8)|(0x9<<0))
#define PHY_DXnGCR0 ((1<<30)|(1<<9)|(1<<2)|(1<<0))
#define PHY_DXnGCR1 0
#define PHY_DXnGCR2 0
#define PHY_DXnGCR3 0xfffc0000
#define PHY_DXnGCR4 ((3<<26)|(1<<25)|(0xf<<2))

//--------ODT and output impedance define----------------//
#define DDR4_ZPROG_ASYM_DRV_PD 0xb
#define DDR4_ZPROG_ASYM_DRV_PU 0xb
#define DDR4_ZPROG_PU_ODT_ONLY 0x7

#define DDR3_ZPROG_ASYM_DRV_PD 0xb
#define DDR3_ZPROG_ASYM_DRV_PU 0xb
#define DDR3_ZPROG_PU_ODT_ONLY 0x7

#define DX_VREF 0x21   //0x28 ~ 0x34
#define PHY_DXnGCR5 (DX_VREF<<24)|(DX_VREF<<16)|(DX_VREF<<8)|(DX_VREF<<0)
