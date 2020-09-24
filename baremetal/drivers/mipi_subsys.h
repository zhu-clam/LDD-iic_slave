#include "ck810.h"
#include "datatype.h"
#include "OV_5640.h"
#include "isp.h"

#define MIPI_SYS_BASE_ADDR  CK_MIPI_Slave
#define MIPI_ISP_CH1       

//#define MIPI_HOST_CTRL_BASE MIPI_SYS_BASE_ADDR
//#define MIPI_IPI2AXI_BASE   MIPI_SYS_BASE_ADDR + 0x400
#define MIPI_DPHY_BASE      MIPI_SYS_BASE_ADDR + 0x800


#ifdef MIPI_ISP_CH1
#define MIPI_HOST_CTRL_BASE MIPI_SYS_BASE_ADDR + 0x1800   
#define MIPI_IPI2AXI_BASE   MIPI_SYS_BASE_ADDR + 0x1c00    
#else

#ifdef __TEST_ISP1__
#define MIPI_HOST_CTRL_BASE MIPI_SYS_BASE_ADDR + 0x1800   
#define MIPI_IPI2AXI_BASE   MIPI_SYS_BASE_ADDR + 0x1c00    

#else	
#define MIPI_HOST_CTRL_BASE MIPI_SYS_BASE_ADDR + 0x1000    // 0x1800
#define MIPI_IPI2AXI_BASE   MIPI_SYS_BASE_ADDR + 0x1400    // 0x1c00
#endif


#endif

//MIPI PHY
#define MIPI_PHY_CIL_APBWR_CTRL			MIPI_DPHY_BASE
#define MIPI_PHY_CNT_HSCK_SETTLE		MIPI_DPHY_BASE+4*0xe
#define MIPI_PHY_CNT_HSD_SETTLE			MIPI_DPHY_BASE+4*0x14



//MIPI host controller register define
#define MIPI_HOST_VERSION               MIPI_HOST_CTRL_BASE + 0x0
#define MIPI_HOST_N_LANES               MIPI_HOST_CTRL_BASE + 0x4
#define MIPI_HOST_CSI2_RESETN           MIPI_HOST_CTRL_BASE + 0x8
#define MIPI_HOST_INT_ST_MAIN           MIPI_HOST_CTRL_BASE + 0xc
#define MIPI_HOST_DATA_IDS_1            MIPI_HOST_CTRL_BASE + 0x10
#define MIPI_HOST_DATA_IDS_2            MIPI_HOST_CTRL_BASE + 0x14
#define MIPI_HOST_PHY_SHUTDOWNZ         MIPI_HOST_CTRL_BASE + 0x40
#define MIPI_HOST_DPHY_RSTZ             MIPI_HOST_CTRL_BASE + 0x44
#define MIPI_HOST_PHY_RX                MIPI_HOST_CTRL_BASE + 0x48
#define MIPI_HOST_PHY_STOPSTATE         MIPI_HOST_CTRL_BASE + 0x4c
#define MIPI_HOST_PHY_TEST_CTRL0        MIPI_HOST_CTRL_BASE + 0x50
#define MIPI_HOST_PHY_TEST_CTRL1        MIPI_HOST_CTRL_BASE + 0x54
#define MIPI_HOST_PHY2_TEST_CTRL0       MIPI_HOST_CTRL_BASE + 0x58
#define MIPI_HOST_PHY2_TEST_CTRL1       MIPI_HOST_CTRL_BASE + 0x5c
#define MIPI_HOST_IPI_MODE              MIPI_HOST_CTRL_BASE + 0x80
#define MIPI_HOST_IPI_VCID              MIPI_HOST_CTRL_BASE + 0x84
#define MIPI_HOST_IPI_DATA_TYPE         MIPI_HOST_CTRL_BASE + 0x88
#define MIPI_HOST_IPI_MEM_FLUSH         MIPI_HOST_CTRL_BASE + 0x8c
#define MIPI_HOST_IPI_HSA_TIME          MIPI_HOST_CTRL_BASE + 0x90
#define MIPI_HOST_IPI_HBP_TIME          MIPI_HOST_CTRL_BASE + 0x94
#define MIPI_HOST_IPI_HSD_TIME          MIPI_HOST_CTRL_BASE + 0x98
#define MIPI_HOST_IPI_HLINE_TIME        MIPI_HOST_CTRL_BASE + 0x9c
#define MIPI_HOST_IPI_SOFTRSTN          MIPI_HOST_CTRL_BASE + 0xa0
#define MIPI_HOST_IPI_ADV_FEATURES      MIPI_HOST_CTRL_BASE + 0xac
#define MIPI_HOST_IPI_VSA_LINES         MIPI_HOST_CTRL_BASE + 0xb0
#define MIPI_HOST_IPI_VBP_LINES         MIPI_HOST_CTRL_BASE + 0xb4
#define MIPI_HOST_IPI_VFP_LINES         MIPI_HOST_CTRL_BASE + 0xb8
#define MIPI_HOST_IPI_VACTIVE_LINES     MIPI_HOST_CTRL_BASE + 0xbc
#define MIPI_HOST_PHY_CAL               MIPI_HOST_CTRL_BASE + 0xcc
#define MIPI_HOST_INT_ST_PHY_FATAL      MIPI_HOST_CTRL_BASE + 0xe0
#define MIPI_HOST_INT_MSK_PHY_FATAL     MIPI_HOST_CTRL_BASE + 0xe4
#define MIPI_HOST_INT_FORCE_PHY_FATAL   MIPI_HOST_CTRL_BASE + 0xe8
#define MIPI_HOST_INT_ST_PKT_FATAL      MIPI_HOST_CTRL_BASE + 0xf0
#define MIPI_HOST_INT_MSK_PKT_FATAL     MIPI_HOST_CTRL_BASE + 0xf4
#define MIPI_HOST_INT_FORCE_PKT_FATAL   MIPI_HOST_CTRL_BASE + 0xf8
#define MIPI_HOST_INT_ST_FRAME_FATAL    MIPI_HOST_CTRL_BASE + 0x100
#define MIPI_HOST_INT_MSK_FRAME_FATAL   MIPI_HOST_CTRL_BASE + 0x104
#define MIPI_HOST_INT_FORCE_FRAME_FATAL MIPI_HOST_CTRL_BASE + 0x108
#define MIPI_HOST_INT_ST_PHY            MIPI_HOST_CTRL_BASE + 0x110
#define MIPI_HOST_INT_MSK_PHY           MIPI_HOST_CTRL_BASE + 0x114
#define MIPI_HOST_INT_FORCE_PHY         MIPI_HOST_CTRL_BASE + 0x118
#define MIPI_HOST_INT_ST_PKT            MIPI_HOST_CTRL_BASE + 0x120
#define MIPI_HOST_INT_MSK_PKT           MIPI_HOST_CTRL_BASE + 0x124
#define MIPI_HOST_INT_FORCE_PKT         MIPI_HOST_CTRL_BASE + 0x128
#define MIPI_HOST_INT_ST_LINE           MIPI_HOST_CTRL_BASE + 0x130
#define MIPI_HOST_INT_MSK_LINE          MIPI_HOST_CTRL_BASE + 0x134
#define MIPI_HOST_INT_FORCE_LINE        MIPI_HOST_CTRL_BASE + 0x138
#define MIPI_HOST_INT_ST_IPI            MIPI_HOST_CTRL_BASE + 0x140
#define MIPI_HOST_INT_MSK_IPI           MIPI_HOST_CTRL_BASE + 0x144
#define MIPI_HOST_INT_FORCE_IPI         MIPI_HOST_CTRL_BASE + 0x148

//define the register of IPI to AXI
#define MIPI_IPI2AXI_MIPI_DMA_CTL        MIPI_IPI2AXI_BASE + 0x0
#define MIPI_IPI2AXI_IMG_OUT_BADDR_Y     MIPI_IPI2AXI_BASE + 0x4
#define MIPI_IPI2AXI_IMG_OUT_BADDR_UV    MIPI_IPI2AXI_BASE + 0x8
#define MIPI_IPI2AXI_IMG_OUT_PIX_HSIZE   MIPI_IPI2AXI_BASE + 0xC
#define MIPI_IPI2AXI_IMG_OUT_PIX_VSIZE   MIPI_IPI2AXI_BASE + 0x10
#define MIPI_IPI2AXI_IMG_OUT_PIX_HSTRIDE MIPI_IPI2AXI_BASE + 0x14
#define MIPI_IPI2AXI_IMG_OUT_BLENTH      MIPI_IPI2AXI_BASE + 0x18
#define MIPI_IPI2AXI_IRQ_EN              MIPI_IPI2AXI_BASE + 0x1C
#define MIPI_IPI2AXI_IRQ_CLR             MIPI_IPI2AXI_BASE + 0x20
#define MIPI_IPI2AXI_IRQ_STATUS          MIPI_IPI2AXI_BASE + 0x24

#define MIPI_IPI2AXI_DMA_EN      (1 << 0)
#define MIPI_IPI2AXI_DMA_DIS     (0 << 0)
#define MIPI_IPI2AXI_YUV420      (0 << 1)
#define MIPI_IPI2AXI_YUV422      (1 << 1)
#define MIPI_IPI2AXI_RGB444      (4 << 1)
#define MIPI_IPI2AXI_RGB555_666  (5 << 1)
#define MIPI_IPI2AXI_RGB565      (6 << 1)
#define MIPI_IPI2AXI_RGB888      (7 << 1)
#define MIPI_IPI2AXI_OUTSTAND(x) ((x - 1) << 8)

#define FS_IRQ_STATUS          (1 << 0)
#define DMA_DONE_IRQ_STATUS    (1 << 1)

#define MIPI_Y_BASEADDR  0x20000000
#define MIPI_Y1_BASEADDR 0x40000000
#define MIPI_UV_BASEADDR 0x30000000
#define MIPI_CB_BASEADDR 0x21000000
#define MIPI_CR_BASEADDR 0x22000000 

typedef struct MIPI_CFG
{
    unsigned int image_format;
    unsigned int image_size;
    unsigned int y_address;
} MIPI_CFG;

typedef struct TIMING_PARAMETER
{
    unsigned int HSA;
    unsigned int HBP;
    unsigned int HSD;
    unsigned int HACTIVE;
    unsigned int HLINE;
    unsigned int VSA;
    unsigned int VBP;
    unsigned int VFP;
    unsigned int VACTIVE;
} TIMING_PARAMETER;
