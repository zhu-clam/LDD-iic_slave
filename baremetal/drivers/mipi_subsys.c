#include "mipi_subsys.h"
#include "misc.h"
#include "intc.h"

#define MIPI_STOP_FRAME 8

extern void MIPI_PHY_Initial();

TIMING_PARAMETER image_timing[4] =
{
    {100, 652, 500, 640, 1892, 60, 150, 50, 480},  // VGA      640x480
    {70, 200, 195 , 1280, 1650, 5, 20, 5, 720},    // 720P     1280x720
    {80, 350, 300 , 1920, 2500, 5, 25, 10, 1080},  // 1080P    1920x1080
    {52, 150, 100 , 2592, 2844, 4, 8, 4, 1936}     // 5M       2592x1936
};

TIMING_PARAMETER image_isp_timing[4] =
{
    {100, 652, 500, 640, 1892, 60, 150, 50, 480},  // VGA      640x480
    {70, 200, 195 , 1280, 1650, 5, 20, 5, 720},    // 720P     1280x720
    {80, 350, 300 , 1920, 2500, 5, 25, 10, 1080},  // 1080P    1920x1080
    {52, 150, 100 , 2592, 2844, 4, 8, 4, 1936}     // 5M       2592x1936
};

void mipi_host_reset()
{
    write_mreg32(MIPI_HOST_CSI2_RESETN, 0x0);  //controller reset
    write_mreg32(MIPI_HOST_PHY_SHUTDOWNZ, 0x0);//PHY shutdown
    write_mreg32(MIPI_HOST_DPHY_RSTZ, 0x0);   //PHY reset
    udelay(10 * 1000);
    write_mreg32(MIPI_HOST_CSI2_RESETN, 0x1);   //release all the reset and shutdown
    write_mreg32(MIPI_HOST_PHY_SHUTDOWNZ, 0x1);
    write_mreg32(MIPI_HOST_DPHY_RSTZ, 0x1);
    udelay(10 * 1000);
}

void mipi_csi_host_cfg(MIPI_CFG *mipi_subsys_cfg) {
    unsigned int tmp32;

    write_mreg32(MIPI_HOST_CSI2_RESETN, 0x0);  //controller reset
    udelay(10 * 1000);
    write_mreg32(MIPI_HOST_CSI2_RESETN, 0x1);   //release all the reset and shutdown
    udelay(10 * 1000);

    // lane number config
    write_mreg32(MIPI_HOST_N_LANES, 0x1); // 2 lanes

    if((mipi_subsys_cfg->image_format == CSI_YUV420_8B) || (mipi_subsys_cfg->image_format == CSI_YUV420_8B_NV21)||(mipi_subsys_cfg->image_format == CSI_YUV422_8B)) {
        tmp32= read_mreg32(MIPI_HOST_IPI_ADV_FEATURES);
        write_mreg32(MIPI_HOST_IPI_ADV_FEATURES, tmp32 | (1 << 24));
    }
    write_mreg32(MIPI_HOST_IPI_MODE, 0x1010000 ); // IPI enable/ cut-through active/48 bits interface/camera timing
    write_mreg32(MIPI_HOST_IPI_VCID, 0x0); //virtual channel always 0 for sensor OV5640

    if((mipi_subsys_cfg->image_format == CSI_YUV420_8B) || (mipi_subsys_cfg->image_format == CSI_YUV420_8B_NV21))
        write_mreg32(MIPI_HOST_IPI_DATA_TYPE, 0x1c);
    else if(mipi_subsys_cfg->image_format == CSI_YUV422_8B)
        write_mreg32(MIPI_HOST_IPI_DATA_TYPE, 0x1e);
    else if(mipi_subsys_cfg->image_format == CSI_RGB565)
        write_mreg32(MIPI_HOST_IPI_DATA_TYPE, 0x22);
    else if(mipi_subsys_cfg->image_format == CSI_RGB555)
        write_mreg32(MIPI_HOST_IPI_DATA_TYPE, 0x21);
    else if(mipi_subsys_cfg->image_format == CSI_RGB444)
        write_mreg32(MIPI_HOST_IPI_DATA_TYPE, 0x20);
    write_mreg32(MIPI_HOST_IPI_MEM_FLUSH,  0x101);

    //horizontal timing config
    write_mreg32(MIPI_HOST_IPI_HSA_TIME, image_timing[mipi_subsys_cfg->image_size].HSA);
    write_mreg32(MIPI_HOST_IPI_HBP_TIME, image_timing[mipi_subsys_cfg->image_size].HBP);
    write_mreg32(MIPI_HOST_IPI_HLINE_TIME, image_timing[mipi_subsys_cfg->image_size].HLINE);
    write_mreg32(MIPI_HOST_IPI_HSD_TIME, image_timing[mipi_subsys_cfg->image_size].HSD);
    //vertical timing config
    write_mreg32(MIPI_HOST_IPI_VSA_LINES, image_timing[mipi_subsys_cfg->image_size].VSA);
    write_mreg32(MIPI_HOST_IPI_VBP_LINES, image_timing[mipi_subsys_cfg->image_size].VBP);
    write_mreg32(MIPI_HOST_IPI_VFP_LINES, image_timing[mipi_subsys_cfg->image_size].VFP);
    write_mreg32(MIPI_HOST_IPI_VACTIVE_LINES, image_timing[mipi_subsys_cfg->image_size].VACTIVE);
}

void mipi_csi_host_isp_cfg(MIPI_CFG *mipi_subsys_cfg) {
    unsigned int tmp32;

    write_mreg32(MIPI_HOST_CSI2_RESETN, 0x0);  //controller reset
    udelay(10 * 1000);
    write_mreg32(MIPI_HOST_CSI2_RESETN, 0x1);   //release all the reset and shutdown
    udelay(10 * 1000);

    // lane number config
    write_mreg32(MIPI_HOST_N_LANES, 0x1); // 2 lanes

    tmp32= read_mreg32(MIPI_HOST_IPI_ADV_FEATURES);
    write_mreg32(MIPI_HOST_IPI_ADV_FEATURES, tmp32 | (1 << 24));

    write_mreg32(MIPI_HOST_IPI_MODE, 0x1010100 ); /////////// IPI enable/ cut-through active/48 bits interface/camera timing  
    write_mreg32(MIPI_HOST_IPI_VCID, 0x0); //virtual channel always 0 for sensor OV5640

  
    write_mreg32(MIPI_HOST_IPI_DATA_TYPE, 0x2B);///////////////  0x2B raw 10bit ///2A raw 8bit 

    
    write_mreg32(MIPI_HOST_IPI_MEM_FLUSH,  0x101);

    //horizontal timing config
    write_mreg32(MIPI_HOST_IPI_HSA_TIME, image_isp_timing[IMAGE_720P].HSA);
    write_mreg32(MIPI_HOST_IPI_HBP_TIME, image_isp_timing[IMAGE_720P].HBP);
    write_mreg32(MIPI_HOST_IPI_HLINE_TIME, image_isp_timing[IMAGE_720P].HLINE);
    write_mreg32(MIPI_HOST_IPI_HSD_TIME, image_isp_timing[IMAGE_720P].HSD);
    //vertical timing config
    write_mreg32(MIPI_HOST_IPI_VSA_LINES, image_isp_timing[IMAGE_720P].VSA);
    write_mreg32(MIPI_HOST_IPI_VBP_LINES, image_isp_timing[IMAGE_720P].VBP);
    write_mreg32(MIPI_HOST_IPI_VFP_LINES, image_isp_timing[IMAGE_720P].VFP);
    write_mreg32(MIPI_HOST_IPI_VACTIVE_LINES, image_isp_timing[IMAGE_720P].VACTIVE);
}


void mipi_ipi2axi_cfg(MIPI_CFG *mipi_subsys_cfg)
{
    unsigned int uv_offset;

    if((mipi_subsys_cfg->image_format == CSI_YUV420_8B)
        || (mipi_subsys_cfg->image_format == CSI_YUV420_8B_NV21)
        || (mipi_subsys_cfg->image_format == CSI_YUV422_8B))
        uv_offset = (image_timing[mipi_subsys_cfg->image_size].HACTIVE) * (image_timing[mipi_subsys_cfg->image_size].VACTIVE);
    else
        uv_offset = 2 * (image_timing[mipi_subsys_cfg->image_size].HACTIVE) * (image_timing[mipi_subsys_cfg->image_size].VACTIVE);

    write_mreg32(MIPI_IPI2AXI_IMG_OUT_BADDR_Y, mipi_subsys_cfg->y_address);
    write_mreg32(MIPI_IPI2AXI_IMG_OUT_BADDR_UV, mipi_subsys_cfg->y_address + uv_offset);
    write_mreg32(MIPI_IPI2AXI_IMG_OUT_PIX_HSIZE, image_timing[mipi_subsys_cfg->image_size].HACTIVE - 1);
    write_mreg32(MIPI_IPI2AXI_IMG_OUT_PIX_VSIZE, image_timing[mipi_subsys_cfg->image_size].VACTIVE - 1);

    write_mreg32(MIPI_IPI2AXI_IMG_OUT_BLENTH, 0x3);

    if((mipi_subsys_cfg->image_format == CSI_YUV420_8B) || (mipi_subsys_cfg->image_format == CSI_YUV420_8B_NV21))
    {
        write_mreg32(MIPI_IPI2AXI_MIPI_DMA_CTL, 0x711 | MIPI_IPI2AXI_YUV420);
        write_mreg32(MIPI_IPI2AXI_IMG_OUT_PIX_HSTRIDE, image_timing[mipi_subsys_cfg->image_size].HACTIVE - 1);
    }
    else if(mipi_subsys_cfg->image_format == CSI_YUV422_8B)
    {
        write_mreg32(MIPI_IPI2AXI_MIPI_DMA_CTL, 0x711 | MIPI_IPI2AXI_YUV422);
        write_mreg32(MIPI_IPI2AXI_IMG_OUT_PIX_HSTRIDE, image_timing[mipi_subsys_cfg->image_size].HACTIVE - 1);
    }
    else if(mipi_subsys_cfg->image_format == CSI_RGB565)
    {
        write_mreg32(MIPI_IPI2AXI_MIPI_DMA_CTL, 0x711 | MIPI_IPI2AXI_RGB565);
        write_mreg32(MIPI_IPI2AXI_IMG_OUT_PIX_HSTRIDE, image_timing[mipi_subsys_cfg->image_size].HACTIVE * 2 - 1);
    }
    else if(mipi_subsys_cfg->image_format == CSI_RGB555)
    {
        write_mreg32(MIPI_IPI2AXI_MIPI_DMA_CTL, 0x711 | MIPI_IPI2AXI_RGB555_666);
        write_mreg32(MIPI_IPI2AXI_IMG_OUT_PIX_HSTRIDE, image_timing[mipi_subsys_cfg->image_size].HACTIVE * 2 - 1);
    }
    else if(mipi_subsys_cfg->image_format == CSI_RGB444)
    {
        write_mreg32(MIPI_IPI2AXI_MIPI_DMA_CTL, 0x711 | MIPI_IPI2AXI_RGB444);
        write_mreg32(MIPI_IPI2AXI_IMG_OUT_PIX_HSTRIDE, image_timing[mipi_subsys_cfg->image_size].HACTIVE * 2 - 1);
    }

    write_mreg32(MIPI_IPI2AXI_IRQ_CLR, 0x7);
    write_mreg32(MIPI_IPI2AXI_IRQ_EN, 0x7);
}

static volatile unsigned int mipi_frame_count;
void CK_Mipi_Intc_Handler() {
    u32 val;

    val = read_mreg32(MIPI_IPI2AXI_IRQ_STATUS);
    if(val & FS_IRQ_STATUS) {
        write_mreg32(MIPI_IPI2AXI_IRQ_CLR, val);
        mipi_frame_count++;

        // update end frame size to 0
        if (mipi_frame_count == MIPI_STOP_FRAME) {
            write_mreg32(MIPI_IPI2AXI_IMG_OUT_PIX_HSIZE, 0);
            write_mreg32(MIPI_IPI2AXI_IMG_OUT_PIX_VSIZE, 0);
            write_mreg32(MIPI_IPI2AXI_IMG_OUT_PIX_HSTRIDE, 0);
        }

        // stop IPI2AXI DMA
        if (mipi_frame_count > MIPI_STOP_FRAME) {
            write_mreg32(MIPI_IPI2AXI_MIPI_DMA_CTL, 0x0);
            write_mreg32(MIPI_IPI2AXI_IRQ_EN, 0);
        }
    }
    printf("CK_Mipi_Intc_Handler status=0x%x, mipi_frame_count=%d\n",val, mipi_frame_count - 1);
    printf("CK_Mipi_Intc_Handler MIPI_HOST_INT_ST_MAIN=0x%x\r\n",read_mreg32(MIPI_HOST_INT_ST_MAIN));
    printf("CK_Mipi_Intc_Handler MIPI_HOST_INT_ST_PKT_FATAL=0x%x\r\n",read_mreg32(MIPI_HOST_INT_ST_PKT_FATAL));
    printf("CK_Mipi_Intc_Handler MIPI_HOST_INT_ST_FRAME_FATAL=0x%x\r\n",read_mreg32(MIPI_HOST_INT_ST_FRAME_FATAL));
    printf("CK_Mipi_Intc_Handler MIPI_HOST_INT_ST_IPI=0x%x\r\n",read_mreg32(MIPI_HOST_INT_ST_IPI));
	printf("CK_Mipi_Intc_Handler MIPI_HOST_INT_ST_PHY=0x%x\r\n",read_mreg32(MIPI_HOST_INT_ST_PHY));
	printf("CK_Mipi_Intc_Handler MIPI_HOST_INT_ST_PHY_FATAL=0x%x\r\n",read_mreg32(MIPI_HOST_INT_ST_PHY_FATAL));
	

}

static CKStruct_IRQHandler mipi_irq_info = {
    .devname = "MIPI",
    .irqid = CK_INTC_MIPI,
    .priority = CK_INTC_MIPI,
    .handler = CK_Mipi_Intc_Handler,
    .bfast = FALSE,
    .next = NULL
};

// YUV422SP -> UYVY
void yuv422_image_transfer(int image_size)
{
    unsigned int h_size;
    unsigned int v_size;
    unsigned int y_addr;
    unsigned int uv_addr;
    unsigned int des_addr;
    unsigned i, tmp1, tmp2, tmp3, tmp4;

    printf("YUV422SP -> UYVY\n");
    h_size = image_timing[image_size].HACTIVE;
    v_size = image_timing[image_size].VACTIVE;
    memcpy(0x12000000, MIPI_Y_BASEADDR, h_size * v_size * 2);
    y_addr = 0x12000000;
    uv_addr = y_addr + h_size * v_size;
    des_addr = MIPI_Y_BASEADDR;

    for (i = 0; i < v_size * h_size / 4; i++) {
        tmp1 = read_mreg32(y_addr);
        tmp2 = read_mreg32(uv_addr);

        tmp3 = ((tmp1 & 0xff) << 8) | ((tmp1 & 0xff00) << 16)
               | (tmp2 & 0xff) | ((tmp2 & 0xff00) << 8);
        tmp4 = ((tmp1 & 0xff0000) >> 8) | (tmp1 & 0xff000000)
               | ((tmp2 & 0xff0000) >> 16) | ((tmp2 & 0xff000000) >> 8);
        write_mreg32(des_addr, tmp3);
        write_mreg32(des_addr + 0x4, tmp4);
        y_addr += 4;
        uv_addr += 4;
        des_addr += 8;
    }
}

// BGR565 -> RGB565
void rgb_image_transfer(int image_size)
{
    unsigned int h_size;
    unsigned int v_size;
    unsigned int src_addr;
    unsigned int des_addr;
    unsigned i, tmp1, tmp2;
    unsigned int r, g, b;

    printf("BGR565 -> RGB565\n");
    h_size = image_timing[image_size].HACTIVE;
    v_size = image_timing[image_size].VACTIVE;
    memcpy(0x12000000, MIPI_Y_BASEADDR, h_size * v_size * 2);
    src_addr = 0x12000000;
    des_addr = MIPI_Y_BASEADDR;

    for (i = 0; i < v_size * h_size / 2; i++)
    {
        tmp1 = read_mreg32(src_addr);
        r = tmp1 & 0x1f;
        g = (tmp1 & 0x7e0) >> 5;
        b = (tmp1 & 0xf800) >> 11;
        tmp2 = (r << 11) | (g << 5) | b;
        r = tmp1 & 0x1f0000;
        g =(tmp1 & 0x7e00000) >> 5;
        b = (tmp1 & 0xf8000000) >> 11;
        tmp2 |= (r << 11) | (g << 5) | b;
        write_mreg32(des_addr, tmp2);
        src_addr += 4;
        des_addr += 4;
    }
}


int mipi_isp_test(MIPI_CFG *mipi_subsys_info)
{
    int ret;

    mipi_frame_count = 0;
    // register irq handler
    CK_INTC_RequestIrq(&mipi_irq_info, AUTO_MODE); 

    Isp_func_WDR2_On();

    mipi_csi_host_isp_cfg(mipi_subsys_info);
    MIPI_PHY_Initial();
    ///write_mreg32(MIPI_IPI2AXI_IRQ_CLR, 0x7);
    ///write_mreg32(MIPI_IPI2AXI_IRQ_EN, 0x7);
   
    ret = mov5640_isp_init(mipi_subsys_info->image_size, mipi_subsys_info->image_format);
    if (ret) {
        return -1;
    }
    MOV5640_MIPI_stream_on();

  #if 1
    
    while (1) {
#ifdef MIPI_ISP_CH1
        printf("mipi_test  isp1 \r\n");
        printf("mipi_test    0x1500 = 0x%x \r\n ",read_mreg32(CK_ISP1_Slave+0x1500));
    	printf("mipi_test    0x14fc = 0x%x \r\n ",read_mreg32(CK_ISP1_Slave+0x14fc));
		printf("mipi_test    0x150c = 0x%x \r\n ",read_mreg32(CK_ISP1_Slave+0x150c));
		printf("mipi_test    0x5bc = 0x%x \r\n ",read_mreg32(CK_ISP1_Slave+0x5bc));
		printf("mipi_test    0x5c0 = 0x%x \r\n ",read_mreg32(CK_ISP1_Slave+0x5c0));
		printf("mipi_test    0x5c4 = 0x%x \r\n ",read_mreg32(CK_ISP1_Slave+0x5c4));
		printf("mipi_test    0x63c = 0x%x \r\n ",read_mreg32(CK_ISP1_Slave+0x63c));
		printf("mipi_test    0x5a8 = 0x%x \r\n ",read_mreg32(CK_ISP1_Slave+0x5a8));
		printf("mipi_test    0x404 = 0x%x \r\n ",read_mreg32(CK_ISP1_Slave+0x404));
		printf("mipi_test    0x400+0x1A8 = 0x%x \r\n ",read_mreg32(CK_ISP1_Slave+0x5A8));
		printf("mipi_test  0x400+0x1AC = 0x%x \r\n ",read_mreg32(CK_ISP1_Slave+0x5AC));
		printf("mipi_test  0x400+0x1B8 = 0x%x \r\n ",read_mreg32(CK_ISP1_Slave+0x5B8));
		printf("mipi_test  0x400+0x1C0 = 0x%x \r\n ",read_mreg32(CK_ISP1_Slave+0x5C0));
		printf("mipi_test  0x400+0x1B4 = 0x%x \r\n ",read_mreg32(CK_ISP1_Slave+0x5B4));
		
		 printf("CK_Mipi_Intc_Handler MIPI_HOST_INT_ST_MAIN=0x%x\r\n",read_mreg32(MIPI_HOST_INT_ST_MAIN));
    printf("CK_Mipi_Intc_Handler MIPI_HOST_INT_ST_PKT_FATAL=0x%x\r\n",read_mreg32(MIPI_HOST_INT_ST_PKT_FATAL));
    printf("CK_Mipi_Intc_Handler MIPI_HOST_INT_ST_FRAME_FATAL=0x%x\r\n",read_mreg32(MIPI_HOST_INT_ST_FRAME_FATAL));
    printf("CK_Mipi_Intc_Handler MIPI_HOST_INT_ST_IPI=0x%x\r\n",read_mreg32(MIPI_HOST_INT_ST_IPI));
	printf("CK_Mipi_Intc_Handler MIPI_HOST_INT_ST_PHY=0x%x\r\n",read_mreg32(MIPI_HOST_INT_ST_PHY));
	printf("CK_Mipi_Intc_Handler MIPI_HOST_INT_ST_PHY_FATAL=0x%x\r\n",read_mreg32(MIPI_HOST_INT_ST_PHY_FATAL));

#else
	    printf("mipi_test  isp0 \r\n");
        printf("mipi_test    0x1500 = 0x%x \r\n ",read_mreg32(CK_ISP0_Slave+0x1500));
    	printf("mipi_test    0x14fc = 0x%x \r\n ",read_mreg32(CK_ISP0_Slave+0x14fc));
		printf("mipi_test    0x150c = 0x%x \r\n ",read_mreg32(CK_ISP0_Slave+0x150c));
		printf("mipi_test    0x5bc = 0x%x \r\n ",read_mreg32(CK_ISP0_Slave+0x5bc));
		printf("mipi_test    0x5c0 = 0x%x \r\n ",read_mreg32(CK_ISP0_Slave+0x5c0));
		printf("mipi_test    0x5c4 = 0x%x \r\n ",read_mreg32(CK_ISP0_Slave+0x5c4));
		printf("mipi_test    0x63c = 0x%x \r\n ",read_mreg32(CK_ISP0_Slave+0x63c));
		printf("mipi_test    0x5a8 = 0x%x \r\n ",read_mreg32(CK_ISP0_Slave+0x5a8));
		printf("mipi_test    0x404 = 0x%x \r\n ",read_mreg32(CK_ISP0_Slave+0x404));
		
		printf("mipi_test    0x400+0x1A8 = 0x%x \r\n ",read_mreg32(CK_ISP0_Slave+0x5A8));
		printf("mipi_test    0x400+0x1AC = 0x%x \r\n ",read_mreg32(CK_ISP0_Slave+0x5AC));
		printf("mipi_test    0x400+0x1B8 = 0x%x \r\n ",read_mreg32(CK_ISP0_Slave+0x5B8));
		printf("mipi_test    0x400+0x1C0 = 0x%x \r\n ",read_mreg32(CK_ISP0_Slave+0x5C0));
		printf("mipi_test  0x400+0x1B4 = 0x%x \r\n ",read_mreg32(CK_ISP0_Slave+0x5B4));
		 printf("CK_Mipi_Intc_Handler MIPI_HOST_INT_ST_MAIN=0x%x\r\n",read_mreg32(MIPI_HOST_INT_ST_MAIN));
    printf("CK_Mipi_Intc_Handler MIPI_HOST_INT_ST_PKT_FATAL=0x%x\r\n",read_mreg32(MIPI_HOST_INT_ST_PKT_FATAL));
    printf("CK_Mipi_Intc_Handler MIPI_HOST_INT_ST_FRAME_FATAL=0x%x\r\n",read_mreg32(MIPI_HOST_INT_ST_FRAME_FATAL));
    printf("CK_Mipi_Intc_Handler MIPI_HOST_INT_ST_IPI=0x%x\r\n",read_mreg32(MIPI_HOST_INT_ST_IPI));
	printf("CK_Mipi_Intc_Handler MIPI_HOST_INT_ST_PHY=0x%x\r\n",read_mreg32(MIPI_HOST_INT_ST_PHY));
	printf("CK_Mipi_Intc_Handler MIPI_HOST_INT_ST_PHY_FATAL=0x%x\r\n",read_mreg32(MIPI_HOST_INT_ST_PHY_FATAL));
		
#endif		
                for(int i = 0 ;i < 8;i++)
		{
  
		    printf("MIPI_Y_BASEADDR(0x%x) = 0x%x \r\n",MIPI_Y_BASEADDR+4*i,read_mreg32(MIPI_Y_BASEADDR+4*i));
		    printf("MIPI_Y_BASEADDR(0x%x) = 0x%x \r\n",MIPI_CB_BASEADDR+4*i,read_mreg32(MIPI_CB_BASEADDR+4*i));
		    printf("MIPI_Y_BASEADDR(0x%x) = 0x%x \r\n",MIPI_CR_BASEADDR+4*i,read_mreg32(MIPI_CR_BASEADDR+4*i));
		}
#ifdef MIPI_ISP_CH1	
		if (read_mreg32(CK_ISP1_Slave+0x1500)&0x03) {
		    break;
		}
#else
	
        if (read_mreg32(CK_ISP0_Slave+0x1500)&0x03) {
		    break;
		}

#endif	
    }
#ifdef MIPI_ISP_CH1
    printf("mipi_test    isp1 status = 0x%x \r\n ",read_mreg32(CK_ISP1_Slave+0x1500));
#else	
    printf("mipi_test    isp0 status = 0x%x \r\n ",read_mreg32(CK_ISP0_Slave+0x1500));
#endif
  #else

    printf("mipi_test  mipi_frame_count  = %d \r\n ",mipi_frame_count);
    while (1) {
        if (mipi_frame_count > MIPI_STOP_FRAME) {
            break;
        }

       //printf("MIPI_HOST_INT_ST_PKT = 0x%x \r\n ",read_mreg32(MIPI_HOST_INT_ST_PKT));
	//printf("MIPI_HOST_INT_ST_FRAME_FATAL = 0x%x \r\n ",read_mreg32(MIPI_HOST_INT_ST_FRAME_FATAL));
	//printf("MIPI_HOST_INT_ST_PHY = 0x%x \r\n ",read_mreg32(MIPI_HOST_INT_ST_PHY));
    }
    printf(" MIPI_HOST_INT_ST_PKT = 0x%x \r\n ",read_mreg32(MIPI_HOST_INT_ST_PKT));
    printf("mipi_test  mipi_frame_count  = %d \r\n ",mipi_frame_count);
 #endif  
    // unregister irq handler
    CK_INTC_FreeIrq(&mipi_irq_info, AUTO_MODE);
    return 0;
}
int mipi_test(MIPI_CFG *mipi_subsys_info)
{
    int ret;

    mipi_frame_count = 0;
    // register irq handler
    CK_INTC_RequestIrq(&mipi_irq_info, AUTO_MODE);

    mipi_csi_host_cfg(mipi_subsys_info);
    mipi_ipi2axi_cfg(mipi_subsys_info);
    MIPI_PHY_Initial();
    ret = mov5640_init(mipi_subsys_info->image_size, mipi_subsys_info->image_format);
    if (ret) {
        return -1;
    }
    MOV5640_MIPI_stream_on();

    printf("mipi_test  mipi_frame_count  = %d \r\n ",mipi_frame_count);
    while (1) {
        if (mipi_frame_count > MIPI_STOP_FRAME) {
            break;
        }
    }
    printf("mipi_test  mipi_frame_count  = %d \r\n ",mipi_frame_count);
    if (mipi_subsys_info->image_format == CSI_YUV422_8B)
        yuv422_image_transfer(mipi_subsys_info->image_size);

    if (mipi_subsys_info->image_format == CSI_RGB565)
        rgb_image_transfer(mipi_subsys_info->image_size);

    // unregister irq handler
    CK_INTC_FreeIrq(&mipi_irq_info, AUTO_MODE);

    return 0;
}









void mipi_reset_test() {
    unsigned int reset_reg_val, def_val, rd_val, wr_val;

   

    // csi host reset test
    def_val = read_mreg32(MIPI_HOST_IPI_HSA_TIME);
    printf("read MIPI_HOST_IPI_HSA_TIME=0x%x\n", def_val);
    wr_val = 0x5a5;
    write_mreg32(MIPI_HOST_IPI_HSA_TIME, wr_val);
    rd_val = read_mreg32(MIPI_HOST_IPI_HSA_TIME);
    printf("read MIPI_HOST_IPI_HSA_TIME=0x%x\n", rd_val);
    if (rd_val == wr_val) {
        printf ("MIPI CSI host reg write test\t - - -PASS\n");
    } else {
        printf ("MIPI CSI host reg write test\t - - -FAILURE\n");
    }
    printf ("\n\n");
}

void ISP_test(void)
{
    MIPI_CFG mipi_subsys_info;
    unsigned char ch;
    int ret;
    int i ; 
    int depth, size;

    mipi_reset_test();

    for(i = 0 ;i < 1024;i++) {
        write_mreg32(MIPI_Y_BASEADDR+4*i,0xFFFFFFFF) ;
        write_mreg32(MIPI_CB_BASEADDR+4*i,0xFFFFFFFF) ;
        write_mreg32(MIPI_CR_BASEADDR+4*i,0xFFFFFFFF) ;
        //printf("MIPI_Y_BASEADDR(0x%x) = 0x%x \r\n",MIPI_Y_BASEADDR+4*i,read_mreg32(MIPI_Y_BASEADDR+4*i));
    }


/*
    mipi_subsys_info.image_format = CSI_YUV420_8B;
    depth = 12;
       
    
    mipi_subsys_info.image_size = IMAGE_720P;
    size = 1280 * 720 * depth / 8;
    
 
    mipi_subsys_info.y_address = MIPI_Y_BASEADDR;
*/
    ret = mipi_isp_test(&mipi_subsys_info);
    if (ret) {
        printf ("\t - - -FAILURE\n");
        return;
    } else {
        printf ("\t - - -PASS\n");
    }
   
    ///printf("Please use below command in GDB to dump image:\n");
   //// printf("\tdump binary memory ./image.yuv 0x20000000 0x%x\n", 0x20000000 + size);



}


void CK_MIPI_Test() {
    MIPI_CFG mipi_subsys_info;
    unsigned char ch;
    int ret;
    int i ; 
    int depth, size;

    mipi_reset_test();
    for(i = 0 ;i < 4; i++) {
        write_mreg32(MIPI_Y_BASEADDR+4*i,0xFFFFFFFF) ;
        printf("MIPI_Y_BASEADDR(0x%x) = 0x%x \r\n",MIPI_Y_BASEADDR+4*i,read_mreg32(MIPI_Y_BASEADDR+4*i));
    }

    while (1) {
        printf("\nplease select image format:\n");
        printf("1 -- CSI_YUV420_8B_NV12\n");
        printf("2 -- CSI_YUV420_8B_NV21\n");
        printf("3 -- CSI_YUV422_8B_UYVY\n");
        printf("4 -- CSI_RGB565\n");
        printf("5 -- CSI_RGB444\n");
        printf("6 -- CSI_RGB555\n");
        printf(">");
        ch = getchar();
        putchar(ch);
        if (ch >= '1' && ch <= '6') {
            break;
        }
    }
    printf ("\n");
    switch(ch) {
    case '1':
        mipi_subsys_info.image_format = CSI_YUV420_8B;
        depth = 12;
        break;
    case '2':
        mipi_subsys_info.image_format = CSI_YUV420_8B_NV21;
        depth = 12;
        break;
    case '3':
        mipi_subsys_info.image_format = CSI_YUV422_8B;
        depth = 16;
        break;
    case '4':
        mipi_subsys_info.image_format = CSI_RGB565;
        depth = 16;
        break;
    case '5':
        mipi_subsys_info.image_format = CSI_RGB444;
        depth = 16;
        break;
    case '6':
        mipi_subsys_info.image_format = CSI_RGB555;
        depth = 16;
        break;
    }

    while (1) {
        printf("\nplease select image size:\n");
        printf("1 -- IMAGE_VGA\n");
        printf("2 -- IMAGE_720P\n");
        printf("3 -- IMAGE_1080P\n");
        printf(">");
        ch = getchar();
        putchar(ch);
        if (ch >= '1' && ch <= '3') {
            break;
        }
    }
    printf ("\n");
    switch(ch) {
    case '1':
        mipi_subsys_info.image_size = IMAGE_VGA;
        size = 640 * 480 * depth / 8;
        break;
    case '2':
        mipi_subsys_info.image_size = IMAGE_720P;
        size = 1280 * 720 * depth / 8;
        break;
    case '3':
        mipi_subsys_info.image_size = IMAGE_1080P;
        size = 1920 * 1080 * depth / 8;
        break;
    }

    mipi_subsys_info.y_address = MIPI_Y_BASEADDR;
    ret = mipi_test(&mipi_subsys_info);
    if (ret) {
        printf ("\t - - -FAILURE\n");
        return;
    } else {
        printf ("\t - - -PASS\n");
    }

    // dump size = hight * width * depth / 8
    // virtual address 0x90000000 = 0x10000000 + 0x8000000
    printf("Capture image done\n");
    printf("Please use below command in GDB to dump image:\n");
    printf("\tdump binary memory ./image.yuv 0x20000000 0x%x\n", 0x20000000 + size);
    for(i = 0 ;i < 4;i++)
        printf("MIPI_Y_BASEADDR(0x%x) = 0x%x \r\n",MIPI_Y_BASEADDR+4*i,read_mreg32(MIPI_Y_BASEADDR+4*i));
}

