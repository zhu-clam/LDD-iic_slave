
#include "dvi.h"
#include "ADV7511.h"
#include "ck810.h"
#include "ADV7611.h"
#include "mipi_subsys.h"

#define __SCAN_MODE__
//#define __FOUR_CHANNEL__
//#define __CHANNEL0__
//#define __576i__
////#define __CHANNEL1__
///#define __CHANNEL2__
///#define __CHANNEL3__


#ifdef __FOUR_CHANNEL__
#define __SD_MODE__
#endif

//#define __720P_MODE__
//#define __1080P_MODE__
//#define __SD_MODE__
#define __480P_MODE__

#ifdef __SD_MODE__
static unsigned char image_timing_para  = 2;
#elif defined(__480P_MODE__)
static unsigned char image_timing_para  = 0;
#elif defined(__720P_MODE__)
static unsigned char image_timing_para  = 3;
#elif defined(__1080P_MODE__)
static unsigned char image_timing_para  = 1;
#else  // __HD_MODE__
static unsigned char image_timing_para  = 1;
#endif

static TIMING_PARAMETER image_timing[5] =
{
    {70, 200, 195 , 720, 1650, 5, 20, 5, 480},   
    {70, 200, 195 , 1920, 1650, 5, 20, 5, 1080},    
    {70, 200, 195 , 720, 1650, 5, 20, 5, 240},    
    {70, 200, 195 , 1280, 1650, 5, 20, 5, 720},  
    {70, 200, 195 , 720, 1650, 5, 20, 5, 288},  	
   
};

void dvp_init(void)
{
        unsigned int val;
              unsigned int uv_offset;

        write_mreg32(VI_CTRL_REG0,(1L << 14)|(1L << 3));
        write_mreg32(VI_CTRL_REG1,0x00000001);
        write_mreg32(VI_CTRL_REG2,0xffff0000);
        write_mreg32(VI_TIMESTAMP_CTL,0x00000000);
        write_mreg32(VI_TIMESTAMP_BADDR,0x00000000);
        write_mreg32(IRQ_CLR,0xffffffff);          
        write_mreg32(ISP_SEL, DVP_MODE);       
              uv_offset =  (image_timing[image_timing_para].HACTIVE) * (image_timing[image_timing_para].VACTIVE);
              write_mreg32(VI_IMG_OUT_BADDR_Y, MIPI_Y_BASEADDR);
            write_mreg32(VI_IMG_OUT_BADDR_UV, MIPI_Y_BASEADDR + uv_offset);

#ifdef __NEW_VERSION__
         write_mreg32(VI_IMG_OUT_BADDR_Y1, MIPI_Y1_BASEADDR);
         write_mreg32(VI_IMG_OUT_BADDR_UV1, MIPI_Y1_BASEADDR + uv_offset);
#endif

            write_mreg32(VI_IMG_OUT_PIX_HSIZE, image_timing[image_timing_para].HACTIVE - 1);
            write_mreg32(VI_IMG_OUT_PIX_VSIZE, image_timing[image_timing_para].VACTIVE - 1);
            write_mreg32(VI_IMG_OUT_BLENTH, 0x7);
        write_mreg32(VI_DMA_CTL, 0x0000);
#ifdef __SCAN_MODE__
        write_mreg32(VI_IMG_OUT_PIX_HSTRIDE, image_timing[image_timing_para].HACTIVE*2 - 1);
#else

        write_mreg32(VI_IMG_OUT_PIX_HSTRIDE, image_timing[image_timing_para].HACTIVE - 1);
#endif
        printf(" 2222222222222 read_mreg32(IRQ_STATUS) =0x%x \r\n",read_mreg32(IRQ_STATUS));
        write_mreg32(IRQ_EN,0xffffffff);
        val = read_mreg32(VI_DMA_CTL);
#ifdef __SCAN_MODE__
         write_mreg32(VI_DMA_CTL, val |DMA_EN|PIXEL_TYPE);

#else
        write_mreg32(VI_DMA_CTL, val |DMA_EN);
#endif
        printf("  111111111111   read_mreg32(IRQ_STATUS) =0x%x \r\n",read_mreg32(IRQ_STATUS));
#if 1
        printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_CTRL_REG0),VI_CTRL_REG0);
        printf("  0x%x  (val) =0x%x \r\n",read_mreg32(ISP_SEL),ISP_SEL);
        printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_IMG_OUT_BADDR_Y),VI_IMG_OUT_BADDR_Y);
        printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_IMG_OUT_BADDR_UV),VI_IMG_OUT_BADDR_UV);  
        printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_IMG_OUT_PIX_HSIZE),VI_IMG_OUT_PIX_HSIZE);      
        printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_IMG_OUT_PIX_VSIZE),VI_IMG_OUT_PIX_VSIZE);  
        printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_IMG_OUT_BLENTH),VI_IMG_OUT_BLENTH);     
        printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_IMG_OUT_PIX_HSTRIDE),VI_IMG_OUT_PIX_HSTRIDE);   
        printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_DMA_CTL),VI_DMA_CTL);       
        printf("  0x%x  (val) =0x%x \r\n",read_mreg32(IRQ_EN),IRQ_EN);       
        printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_CTRL_REG2),VI_CTRL_REG2);
         printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_CTRL_REG1),VI_CTRL_REG1);    

#endif
       printf("  3333   read_mreg32(IRQ_STATUS) =0x%x \r\n",read_mreg32(IRQ_STATUS));
       write_mreg32(IRQ_CLR,0xffffffff);
    
}
void sdi_init(void)
{
        unsigned int val;
        unsigned int i ;
              unsigned int uv_offset;
#ifdef __FOUR_CHANNEL__

//ch0 ch1 ch2 ch3
#if 0
        for(i=0;i<3; i++)
        {

                        
                                write_mreg32(VI_CTRL_REG0+i*0x400,0x00000430);
                                write_mreg32(VI_CTRL_REG1+i*0x400,0x00000000);  /////  8bit  16 bit 
                                write_mreg32(VI_CTRL_REG2+i*0x400,(0x000000ff<<(8*i))); ////  16:ffff  8 ff
                        
                                write_mreg32(VI_TIMESTAMP_CTL+i*0x400,0x00000000);
                                write_mreg32(VI_TIMESTAMP_BADDR+i*0x400,0x00000000);
                                write_mreg32(IRQ_CLR+i*0x400,0xffffffff);                 
                                      uv_offset =  (image_timing[image_timing_para].HACTIVE) * (image_timing[image_timing_para].VACTIVE);
                                      write_mreg32(VI_IMG_OUT_BADDR_Y+i*0x400, MIPI_Y_BASEADDR+0x1000000*i);
                                    write_mreg32(VI_IMG_OUT_BADDR_UV+i*0x400, MIPI_Y_BASEADDR + uv_offset+0x1000000*i);
                                    write_mreg32(VI_IMG_OUT_PIX_HSIZE+i*0x400, image_timing[image_timing_para].HACTIVE - 1);
                                    write_mreg32(VI_IMG_OUT_PIX_VSIZE+i*0x400, image_timing[image_timing_para].VACTIVE - 1);
                                    write_mreg32(VI_IMG_OUT_BLENTH+i*0x400, 0x7);
                                write_mreg32(VI_DMA_CTL+i*0x400, 0x0000);
                                write_mreg32(VI_IMG_OUT_PIX_HSTRIDE+i*0x400, image_timing[image_timing_para].HACTIVE*2 - 1);   
                                write_mreg32(IRQ_EN+i*0x400,0xffffffff);
                          



      }
#endif

#ifdef __CHANNEL0__

#ifdef __576i__
                                write_mreg32(VI_CTRL_REG0,0x00000230);
#else
                                write_mreg32(VI_CTRL_REG0,0x00000630);
#endif	
								
                                write_mreg32(VI_CTRL_REG1,0x00000000);
                                write_mreg32(VI_CTRL_REG2,0x000000ff);

                                write_mreg32(VI_TIMESTAMP_CTL,0x00000000);
                                write_mreg32(VI_TIMESTAMP_BADDR,0x00000000);
                                write_mreg32(IRQ_CLR,0xffffffff);                 
                                uv_offset =  (image_timing[image_timing_para].HACTIVE) * (image_timing[image_timing_para].VACTIVE);
                                write_mreg32(VI_IMG_OUT_BADDR_Y, MIPI_Y_BASEADDR);
                                write_mreg32(VI_IMG_OUT_BADDR_UV, MIPI_Y_BASEADDR + uv_offset);
#ifdef __NEW_VERSION__
                                write_mreg32(VI_IMG_OUT_BADDR_Y1, MIPI_Y1_BASEADDR);
                                write_mreg32(VI_IMG_OUT_BADDR_UV1, MIPI_Y1_BASEADDR + uv_offset);
#endif
                                write_mreg32(VI_IMG_OUT_PIX_HSIZE, image_timing[image_timing_para].HACTIVE - 1);
                                write_mreg32(VI_IMG_OUT_PIX_VSIZE, image_timing[image_timing_para].VACTIVE - 1);
                                write_mreg32(VI_IMG_OUT_BLENTH, 0x7);
                                write_mreg32(VI_DMA_CTL, 0x0000);
                                write_mreg32(VI_IMG_OUT_PIX_HSTRIDE, image_timing[image_timing_para].HACTIVE*2 - 1);   
                                write_mreg32(IRQ_EN,0xffffffff);
#endif
#ifdef __CHANNEL1__


                                write_mreg32(VI_CTRL_REG0+0x400,0x00000430);
                                write_mreg32(VI_CTRL_REG1+0x400,0x00000000);
                                write_mreg32(VI_CTRL_REG2+0x400,0x0000ff00);
                        
                                write_mreg32(VI_TIMESTAMP_CTL+0x400,0x00000000);
                                write_mreg32(VI_TIMESTAMP_BADDR+0x400,0x00000000);
                                write_mreg32(IRQ_CLR+0x400,0xffffffff);                 
                                      uv_offset =  (image_timing[image_timing_para].HACTIVE) * (image_timing[image_timing_para].VACTIVE);
                                      write_mreg32(VI_IMG_OUT_BADDR_Y+0x400, MIPI_Y_BASEADDR+0x1000000);
                                    write_mreg32(VI_IMG_OUT_BADDR_UV+0x400, MIPI_Y_BASEADDR + uv_offset+0x1000000);
#ifdef __NEW_VERSION__
                                write_mreg32(VI_IMG_OUT_BADDR_Y1+0x400, MIPI_Y1_BASEADDR+0x1000000);
                                    write_mreg32(VI_IMG_OUT_BADDR_UV1+0x400, MIPI_Y1_BASEADDR + uv_offset+0x1000000);
#endif
                                    write_mreg32(VI_IMG_OUT_PIX_HSIZE+0x400, image_timing[image_timing_para].HACTIVE - 1);
                                    write_mreg32(VI_IMG_OUT_PIX_VSIZE+0x400, image_timing[image_timing_para].VACTIVE - 1);
                                    write_mreg32(VI_IMG_OUT_BLENTH+0x400, 0x7);
                                write_mreg32(VI_DMA_CTL+0x400, 0x0000);
                                write_mreg32(VI_IMG_OUT_PIX_HSTRIDE+0x400, image_timing[image_timing_para].HACTIVE*2 - 1);   
                                write_mreg32(IRQ_EN+0x400,0xffffffff);
#endif
#ifdef __CHANNEL2__

                                write_mreg32(VI_CTRL_REG0+0x800,0x00000430);
                                write_mreg32(VI_CTRL_REG1+0x800,0x00000000);
                                write_mreg32(VI_CTRL_REG2+0x800,0x00ff0000);
                        
                                write_mreg32(VI_TIMESTAMP_CTL+0x800,0x00000000);
                                write_mreg32(VI_TIMESTAMP_BADDR+0x800,0x00000000);
                                write_mreg32(IRQ_CLR+0x800,0xffffffff);                 
                                      uv_offset =  (image_timing[image_timing_para].HACTIVE) * (image_timing[image_timing_para].VACTIVE);
                                      write_mreg32(VI_IMG_OUT_BADDR_Y+0x800, MIPI_Y_BASEADDR+0x2000000);
                                    write_mreg32(VI_IMG_OUT_BADDR_UV+0x800, MIPI_Y_BASEADDR + uv_offset+0x2000000);
#ifdef __NEW_VERSION__
                                write_mreg32(VI_IMG_OUT_BADDR_Y1+0x800, MIPI_Y1_BASEADDR+0x2000000);
                                    write_mreg32(VI_IMG_OUT_BADDR_UV1+0x800, MIPI_Y1_BASEADDR + uv_offset+0x2000000);
#endif
                                    write_mreg32(VI_IMG_OUT_PIX_HSIZE+0x800, image_timing[image_timing_para].HACTIVE - 1);
                                    write_mreg32(VI_IMG_OUT_PIX_VSIZE+0x800, image_timing[image_timing_para].VACTIVE - 1);
                                    write_mreg32(VI_IMG_OUT_BLENTH+0x800, 0x7);
                                write_mreg32(VI_DMA_CTL+0x800, 0x0000);
                                write_mreg32(VI_IMG_OUT_PIX_HSTRIDE+0x800, image_timing[image_timing_para].HACTIVE*2 - 1);   
                                write_mreg32(IRQ_EN+0x800,0xffffffff);

  
                                printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_CTRL_REG0+0x800),VI_CTRL_REG0+0x800);

                                printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_IMG_OUT_BADDR_Y+0x800),VI_IMG_OUT_BADDR_Y+0x800);
                                printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_IMG_OUT_BADDR_UV+0x800),VI_IMG_OUT_BADDR_UV+0x800);  
                                printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_IMG_OUT_PIX_HSIZE+0x800),VI_IMG_OUT_PIX_HSIZE+0x800);      
                                printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_IMG_OUT_PIX_VSIZE+0x800),VI_IMG_OUT_PIX_VSIZE+0x800);  
                                printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_IMG_OUT_BLENTH+0x800),VI_IMG_OUT_BLENTH+0x800);     
                                printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_IMG_OUT_PIX_HSTRIDE+0x800),VI_IMG_OUT_PIX_HSTRIDE+0x800);   
                                printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_DMA_CTL+0x800),VI_DMA_CTL+0x800);       
                                printf("  0x%x  (val) =0x%x \r\n",read_mreg32(IRQ_EN+0x800),IRQ_EN+0x800);       
                                printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_CTRL_REG2+0x800),VI_CTRL_REG2+0x800);
                                 printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_CTRL_REG1+0x800),VI_CTRL_REG1+0x800);    


#endif
#ifdef __CHANNEL3__
                                write_mreg32(VI_CTRL_REG0+0xc00,0x00000430);
                                write_mreg32(VI_CTRL_REG1+0xc00,0x00000000);
                                write_mreg32(VI_CTRL_REG2+0xc00,0xff000000);
                        
                                write_mreg32(VI_TIMESTAMP_CTL+0xc00,0x00000000);
                                write_mreg32(VI_TIMESTAMP_BADDR+0xc00,0x00000000);
                                write_mreg32(IRQ_CLR+0xc00,0xffffffff);                 
                                      uv_offset =  (image_timing[image_timing_para].HACTIVE) * (image_timing[image_timing_para].VACTIVE);
                                      write_mreg32(VI_IMG_OUT_BADDR_Y+0xc00, MIPI_Y_BASEADDR+0x3000000);
                                    write_mreg32(VI_IMG_OUT_BADDR_UV+0xc00, MIPI_Y_BASEADDR + uv_offset+0x3000000);
#ifdef __NEW_VERSION__
                                write_mreg32(VI_IMG_OUT_BADDR_Y1+0xc00, MIPI_Y1_BASEADDR+0x3000000);
                                    write_mreg32(VI_IMG_OUT_BADDR_UV1+0xc00, MIPI_Y1_BASEADDR + uv_offset+0x3000000);
#endif
                                    write_mreg32(VI_IMG_OUT_PIX_HSIZE+0xc00, image_timing[image_timing_para].HACTIVE - 1);
                                    write_mreg32(VI_IMG_OUT_PIX_VSIZE+0xc00, image_timing[image_timing_para].VACTIVE - 1);
                                    write_mreg32(VI_IMG_OUT_BLENTH+0xc00, 0x7);
                                write_mreg32(VI_DMA_CTL+0xc00, 0x0000);
                                write_mreg32(VI_IMG_OUT_PIX_HSTRIDE+0xc00, image_timing[image_timing_para].HACTIVE*2 - 1);   
                                write_mreg32(IRQ_EN+0xc00,0xffffffff);
#endif	
#ifdef __CHANNEL0__	
                                val = read_mreg32(VI_DMA_CTL);				
                                write_mreg32(VI_DMA_CTL, val |DMA_EN|PIXEL_TYPE);
#endif

#ifdef __CHANNEL1__	
                                val = read_mreg32(VI_DMA_CTL+0x400);
                                write_mreg32(VI_DMA_CTL+0x400, val |DMA_EN|PIXEL_TYPE);	
#endif

#ifdef __CHANNEL2__			
                                val = read_mreg32(VI_DMA_CTL+0x800);
                                write_mreg32(VI_DMA_CTL+0x800, val |DMA_EN|PIXEL_TYPE);	
#endif

#ifdef __CHANNEL3__			
                                val = read_mreg32(VI_DMA_CTL+0xc00);
                                write_mreg32(VI_DMA_CTL+0xc00, val |DMA_EN|PIXEL_TYPE);
#endif				



#else


        write_mreg32(VI_WR_BD_CTL, 0);
#ifdef __SD_MODE__
        write_mreg32(VI_CTRL_REG0,0x00000430);
        write_mreg32(VI_CTRL_REG1,0x00000000);
        write_mreg32(VI_CTRL_REG2,0x000000ff);
#elif defined(__480P_MODE__)
        write_mreg32(VI_CTRL_REG0, 0x00000210);
         write_mreg32(VI_CTRL_REG1, 0x00000001);
        write_mreg32(VI_CTRL_REG2, 0xffff0000);
#elif defined(__720P_MODE__)
        write_mreg32(VI_CTRL_REG0,0x00000010);
         write_mreg32(VI_CTRL_REG1,0x00000001);
        write_mreg32(VI_CTRL_REG2,0xffff0000);
#elif defined(__1080P_MODE__)
        write_mreg32(VI_CTRL_REG0,0x00000010);
        write_mreg32(VI_CTRL_REG1,0x00000001);
        write_mreg32(VI_CTRL_REG2,0xffff0000);
#else
        write_mreg32(VI_CTRL_REG0,0x00000010);
         write_mreg32(VI_CTRL_REG1,0x00000001);
        write_mreg32(VI_CTRL_REG2,0xffff0000);
#endif
        write_mreg32(VI_TIMESTAMP_CTL,0x00000000);
        write_mreg32(VI_TIMESTAMP_BADDR,0x00000000);
        write_mreg32(IRQ_CLR,0xffffffff);
        write_mreg32(ISP_SEL, DVP_MODE);
        uv_offset =  (image_timing[image_timing_para].HACTIVE) * (image_timing[image_timing_para].VACTIVE);
        write_mreg32(VI_IMG_OUT_BADDR_Y, MIPI_Y_BASEADDR);
        write_mreg32(VI_IMG_OUT_BADDR_UV, MIPI_Y_BASEADDR + uv_offset);
        write_mreg32(VI_IMG_OUT_BADDR_Y1, 0);
        write_mreg32(VI_IMG_OUT_BADDR_UV1, 0);

#ifdef __SD_MODE__
        uv_offset =  (image_timing[image_timing_para].HACTIVE) * (image_timing[image_timing_para].VACTIVE);
        write_mreg32(VI_IMG_OUT_BADDR_Y0, MIPI_Y_BASEADDR);
        write_mreg32(VI_IMG_OUT_BADDR_UV0, MIPI_Y_BASEADDR + uv_offset);
        write_mreg32(VI_IMG_OUT_BADDR_Y1, MIPI_Y_BASEADDR + uv_offset * 2);
        write_mreg32(VI_IMG_OUT_BADDR_UV1, MIPI_Y_BASEADDR + uv_offset * 3);
#endif

#ifdef __NEW_VERSION__
        //write_mreg32(VI_IMG_OUT_BADDR_Y1, MIPI_Y1_BASEADDR);
        //write_mreg32(VI_IMG_OUT_BADDR_UV1, MIPI_Y1_BASEADDR + uv_offset);
#endif
        write_mreg32(VI_IMG_OUT_PIX_HSIZE, image_timing[image_timing_para].HACTIVE - 1);
        write_mreg32(VI_IMG_OUT_PIX_VSIZE, image_timing[image_timing_para].VACTIVE - 1);
        write_mreg32(VI_IMG_OUT_BLENTH, 0x7);

        write_mreg32(VI_DMA_CTL, 0x0000);
#ifdef __SCAN_MODE__
        write_mreg32(VI_IMG_OUT_PIX_HSTRIDE, image_timing[image_timing_para].HACTIVE*2 - 1);
#else
        write_mreg32(VI_IMG_OUT_PIX_HSTRIDE, image_timing[image_timing_para].HACTIVE - 1);
#endif

        write_mreg32(IRQ_EN,0xffffffff);
        val = read_mreg32(VI_DMA_CTL);
#ifdef __SCAN_MODE__
         write_mreg32(VI_DMA_CTL, val |DMA_EN|PIXEL_TYPE);
#else
        write_mreg32(VI_DMA_CTL, val |DMA_EN);
#endif

#if 1
        printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_CTRL_REG0),VI_CTRL_REG0);
        printf("  0x%x  (val) =0x%x \r\n",read_mreg32(ISP_SEL),ISP_SEL);
        printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_IMG_OUT_BADDR_Y),VI_IMG_OUT_BADDR_Y);
        printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_IMG_OUT_BADDR_UV),VI_IMG_OUT_BADDR_UV);  
        printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_IMG_OUT_BADDR_Y1),VI_IMG_OUT_BADDR_Y1);
        printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_IMG_OUT_BADDR_UV1),VI_IMG_OUT_BADDR_UV1);  
        printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_IMG_OUT_PIX_HSIZE),VI_IMG_OUT_PIX_HSIZE);      
        printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_IMG_OUT_PIX_VSIZE),VI_IMG_OUT_PIX_VSIZE);  
        printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_IMG_OUT_BLENTH),VI_IMG_OUT_BLENTH);     
        printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_IMG_OUT_PIX_HSTRIDE),VI_IMG_OUT_PIX_HSTRIDE);   
        printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_DMA_CTL),VI_DMA_CTL);       
        printf("  0x%x  (val) =0x%x \r\n",read_mreg32(IRQ_EN),IRQ_EN);       
        printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_CTRL_REG2),VI_CTRL_REG2);
         printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_CTRL_REG1),VI_CTRL_REG1);    

#endif

#endif 
     
    
}

// YUV422SP -> UYVY
static void yuv422sp_image_transfer(int image_size)
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
    memcpy(0x12000000, MIPI_Y_BASEADDR+0x2000000, h_size * v_size * 2);
    y_addr = 0x12000000;
    uv_addr = y_addr + h_size * v_size;
    des_addr = MIPI_Y_BASEADDR+0x2000000;

    for (i = 0; i < v_size * h_size / 4; i++)
    {
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


void dvi_test(void)
{
             unsigned int val,i,j;
         dvp_init();
            ADV7511_Initial();  
        adv7611_init();
        
i = 0 ;
        do
        {          
                val = read_mreg32(IRQ_STATUS);
                if((val&DMA_DONE) == DMA_DONE)
                {
                         i++;
                        write_mreg32(IRQ_CLR,0xffffffff);       
                }
                printf("  val =0x%x \r\n",val);

               // for(int j = 0 ;j < 256;j++)
                //{
  
                    //printf("MIPI_Y_BASEADDR(0x%x) = 0x%x \r\n",MIPI_Y_BASEADDR+4*j,read_mreg32(MIPI_Y_BASEADDR+4*j));
                    
                //}
        }while(((val&DMA_DONE) != DMA_DONE)||(i<6));

        write_mreg32(VI_DMA_CTL, 0x00000000);
#ifdef __SCAN_MODE__

#else
        yuv422sp_image_transfer(0);
#endif
        printf("dvp end  !!!!!!!!!!!!!!!!!!!!!!!  val =0x%x   0x%x  i=%d\r\n",val,image_timing[0].HACTIVE * image_timing[0].VACTIVE * 2,i );

    
}

void sdi_test(void)
{
        unsigned int val,i,j;
        //ADV7511_sdi_Initial();
        adv7611_init();
        sdi_init();
        printf("sdi_test!!!\r\n");
#ifdef __FOUR_CHANNEL__
#ifdef __CHANNEL0__
       i = 0 ;
        do
        {          
                val = read_mreg32(IRQ_STATUS);
                if((val&DMA_DONE) == DMA_DONE)
                {
                         i++;
                        write_mreg32(IRQ_CLR,0xffffffff);       
                }
                printf("  ch0 val =0x%x \r\n",val);

               
        }while(((val&DMA_DONE) != DMA_DONE)||(i<6));

        write_mreg32(VI_DMA_CTL, 0x00000000);

    
        printf("dvp end  !!!!!!!!!!!!!!!!!!!!!!!  val =0x%x   0x%x  i=%d\r\n",val,0x20000000+image_timing[image_timing_para].HACTIVE * image_timing[image_timing_para].VACTIVE * 2,i );
#endif
#ifdef __CHANNEL1__
        i = 0 ;
        do
        {          
                val = read_mreg32(IRQ_STATUS+0x400);
                if((val&DMA_DONE) == DMA_DONE)
                {
                         i++;
                        write_mreg32(IRQ_CLR+0x400,0xffffffff);       
                }
                printf("  ch1 val =0x%x \r\n",val);

               
        }while(((val&DMA_DONE) != DMA_DONE)||(i<6));

        write_mreg32(VI_DMA_CTL+0x400, 0x00000000);

      
        printf("dvp end  !!!!!!!!!!!!!!!!!!!!!!!  val =0x%x   0x%x  i=%d\r\n",val,0x20000000+0x1000000+image_timing[image_timing_para].HACTIVE * image_timing[image_timing_para].VACTIVE * 2,i );  
#endif
#ifdef __CHANNEL3__
        i = 0 ;
        do
        {          
                val = read_mreg32(IRQ_STATUS+0xc00);
                if((val&DMA_DONE) == DMA_DONE)
                {
                         i++;
                        write_mreg32(IRQ_CLR+0xc00,0xffffffff);       
                }
                printf("  ch3 val =0x%x \r\n",val);
                
               
        }while(((val&DMA_DONE) != DMA_DONE)||(i<6));

        write_mreg32(VI_DMA_CTL+0xc00, 0x00000000);

   
        printf("dvp end  !!!!!!!!!!!!!!!!!!!!!!!  val =0x%x   0x%x  i=%d\r\n",val,0x20000000+0x3000000+image_timing[image_timing_para].HACTIVE * image_timing[image_timing_para].VACTIVE * 2,i );  


#endif
#ifdef __CHANNEL2__
        i = 0 ;
        do
        {          
                val = read_mreg32(IRQ_STATUS+0x800);
                if((val&DMA_DONE) == DMA_DONE)
                {
                         i++;
                        write_mreg32(IRQ_CLR+0x800,0xffffffff);       
                }
                printf("  ch2 val =0x%x \r\n",val);
               

               
        }while(((val&DMA_DONE) != DMA_DONE)||(i<6));

        write_mreg32(VI_DMA_CTL+0x800, 0x00000000);

        printf("dvp end  !!!!!!!!!!!!!!!!!!!!!!!  val =0x%x   0x%x  i=%d\r\n",val,0x20000000+0x2000000+image_timing[image_timing_para].HACTIVE * image_timing[image_timing_para].VACTIVE * 2,i );  

#endif
#else

        i = 0;
        int uv_offset;
        do {
                val = read_mreg32(IRQ_STATUS);
                #if 0
                if(val & FS) {
                        i++;
                        write_mreg32(IRQ_CLR, 0xffffffff);
                        //write_mreg32(VI_DMA_CTL, 0x00000000);
                        //udelay(100 * 1000);
                        uv_offset = 720 * 480;
                        write_mreg32(VI_IMG_OUT_BADDR_Y, MIPI_Y_BASEADDR + 0x100000 * i);
                        write_mreg32(VI_IMG_OUT_BADDR_UV, MIPI_Y_BASEADDR + 0x100000 * i + uv_offset);

                        //write_mreg32(VI_DMA_CTL, DMA_EN | PIXEL_TYPE);
                        //printf("dma done val = 0x%x\r\n", val);
                }
                #else
                if((val & DMA_DONE) == DMA_DONE) {
                        i++;
                        write_mreg32(IRQ_CLR, 0xffffffff);
                        //write_mreg32(VI_DMA_CTL, 0x00000000);
                        //udelay(100 * 1000);
                        uv_offset = 720 * 480;
                        write_mreg32(VI_IMG_OUT_BADDR_Y, MIPI_Y_BASEADDR + 0x100000 * i);
                        write_mreg32(VI_IMG_OUT_BADDR_UV, MIPI_Y_BASEADDR + 0x100000 * i + uv_offset);

                        //write_mreg32(VI_DMA_CTL, DMA_EN | PIXEL_TYPE);
                        printf("dma done val = 0x%x\r\n", val);
                }
                #endif
        }while(((val&DMA_DONE) != DMA_DONE)||(i<10));


#ifdef __SCAN_MODE__

#else
        yuv422sp_image_transfer(image_timing_para);
#endif

        printf("dvp end  !!!!!!!!!!!!!!!!!!!!!!!  val =0x%x  %d 0x%x  i=%d\r\n",val,image_timing_para,image_timing[image_timing_para].HACTIVE * image_timing[image_timing_para].VACTIVE * 2,i );
#endif
    
}

void Single_Adv7611_test(void)
{

unsigned int val,i,j;  int uv_offset;

     adv7611_init();
	 
     for(i = 0 ;i < 4; i++) {
        write_mreg32(MIPI_Y_BASEADDR+0x2000000+4*i,0xFFFFFFFF) ;
        printf("MIPI_Y_BASEADDR(0x%x) = 0x%x \r\n",MIPI_Y_BASEADDR+0x2000000+4*i,read_mreg32(MIPI_Y_BASEADDR+0x2000000+4*i));
    }
	
	#ifdef __ADV7611_SD__
								write_mreg32(VI_CTRL_REG0+0x800,0x00000630);
                                write_mreg32(VI_CTRL_REG1+0x800,0x00000000);
                                write_mreg32(VI_CTRL_REG2+0x800,0xff000000);
	
	#else
                                write_mreg32(VI_CTRL_REG0+0x800,0x00000010);
                                write_mreg32(VI_CTRL_REG1+0x800,0x00000001);
                                write_mreg32(VI_CTRL_REG2+0x800,0xffff0000);
	#endif							
                        
                                write_mreg32(VI_TIMESTAMP_CTL+0x800,0x00000000);
                                write_mreg32(VI_TIMESTAMP_BADDR+0x800,0x00000000);
                                write_mreg32(IRQ_CLR+0x800,0xffffffff); 
								///write_mreg32(ISP_SEL+0x800, DVP_MODE);								
                                uv_offset =  (image_timing[image_timing_para].HACTIVE) * (image_timing[image_timing_para].VACTIVE);
                                write_mreg32(VI_IMG_OUT_BADDR_Y+0x800, MIPI_Y_BASEADDR+0x2000000);
                                write_mreg32(VI_IMG_OUT_BADDR_UV+0x800, MIPI_Y_BASEADDR + uv_offset+0x2000000);
#ifdef __NEW_VERSION__
                                write_mreg32(VI_IMG_OUT_BADDR_Y1+0x800, MIPI_Y1_BASEADDR+0x2000000);
                                write_mreg32(VI_IMG_OUT_BADDR_UV1+0x800, MIPI_Y1_BASEADDR + uv_offset+0x2000000);
#endif
                                write_mreg32(VI_IMG_OUT_PIX_HSIZE+0x800, image_timing[image_timing_para].HACTIVE - 1);
                                write_mreg32(VI_IMG_OUT_PIX_VSIZE+0x800, image_timing[image_timing_para].VACTIVE - 1);
                                write_mreg32(VI_IMG_OUT_BLENTH+0x800, 0x7);
                                write_mreg32(VI_DMA_CTL+0x800, 0x0000);
                                write_mreg32(VI_IMG_OUT_PIX_HSTRIDE+0x800, image_timing[image_timing_para].HACTIVE*2 - 1);   
                                write_mreg32(IRQ_EN+0x800,0xffffffff);

                                
        write_mreg32(IRQ_EN+0x800,0xffffffff);
        val = read_mreg32(VI_DMA_CTL+0x800);
#ifdef __SCAN_MODE__
         write_mreg32(VI_DMA_CTL+0x800, val |DMA_EN|PIXEL_TYPE);
#else
        write_mreg32(VI_DMA_CTL+0x800, val |DMA_EN);
#endif
                                printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_CTRL_REG0+0x800),VI_CTRL_REG0+0x800);

                                printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_IMG_OUT_BADDR_Y+0x800),VI_IMG_OUT_BADDR_Y+0x800);
                                printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_IMG_OUT_BADDR_UV+0x800),VI_IMG_OUT_BADDR_UV+0x800);  
                                printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_IMG_OUT_PIX_HSIZE+0x800),VI_IMG_OUT_PIX_HSIZE+0x800);      
                                printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_IMG_OUT_PIX_VSIZE+0x800),VI_IMG_OUT_PIX_VSIZE+0x800);  
                                printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_IMG_OUT_BLENTH+0x800),VI_IMG_OUT_BLENTH+0x800);     
                                printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_IMG_OUT_PIX_HSTRIDE+0x800),VI_IMG_OUT_PIX_HSTRIDE+0x800);   
                                printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_DMA_CTL+0x800),VI_DMA_CTL+0x800);       
                                printf("  0x%x  (val) =0x%x \r\n",read_mreg32(IRQ_EN+0x800),IRQ_EN+0x800);       
                                printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_CTRL_REG2+0x800),VI_CTRL_REG2+0x800);
                                printf("  0x%x  (val) =0x%x \r\n",read_mreg32(VI_CTRL_REG1+0x800),VI_CTRL_REG1+0x800);    

 

  	 
	 
	 
	    i = 0 ;
        do
        {          
                val = read_mreg32(IRQ_STATUS+0x800);
                if((val&DMA_DONE) == DMA_DONE)
                {
                         i++;
                        write_mreg32(IRQ_CLR+0x800,0xffffffff);       
                }
                printf("  ch3 val =0x%x \r\n",val);
                
               
        }while(((val&DMA_DONE) != DMA_DONE)||(i<200));

		#ifdef __SCAN_MODE__
        write_mreg32(VI_DMA_CTL+0x800, PIXEL_TYPE);
		#else
        write_mreg32(VI_DMA_CTL+0x800, 0);
		#endif
        
		
		for(i = 0 ;i < 4; i++) {
        
        printf("MIPI_Y_BASEADDR(0x%x) = 0x%x \r\n",MIPI_Y_BASEADDR+0x2000000+4*i,read_mreg32(MIPI_Y_BASEADDR+0x2000000+4*i));
    }

   #ifdef __SCAN_MODE__

#else
       yuv422sp_image_transfer(image_timing_para);
#endif

        printf("dvp end  !!!!!!!!!!!!!!!!!!!!!!!  val =0x%x   0x%x  i=%d\r\n",val,MIPI_Y_BASEADDR+0x2000000+image_timing[image_timing_para].HACTIVE * image_timing[image_timing_para].VACTIVE * 2,i );  


}



#define GS2971_RST1   6
#define GS2971_RST0   9
void dsi_dvp_test(void)
{
	
	   unsigned int val,i,j;
	   
	   for(i = 0 ;i < 4; i++) {
        write_mreg32(MIPI_Y_BASEADDR+4*i,0xFFFFFFFF) ;
        printf("MIPI_Y_BASEADDR(0x%x) = 0x%x \r\n",MIPI_Y_BASEADDR+4*i,read_mreg32(MIPI_Y_BASEADDR+4*i));
       }
        sdi_init();
        printf("dsi_dvp_test!!!\r\n");
		
		CK_Gpio_Output(GS2971_RST0, 0);
		CK_Gpio_Output(GS2971_RST1, 0);

    
        udelay(100 * 1000);
	    CK_Gpio_Output(GS2971_RST0, 1);
		CK_Gpio_Output(GS2971_RST1, 1);

	

#ifdef __CHANNEL0__
       i = 0 ;
        do
        {          
                val = read_mreg32(IRQ_STATUS);
                if((val&DMA_DONE) == DMA_DONE)
                {
                         i++;
                        write_mreg32(IRQ_CLR,0xffffffff);       
                }
                printf("  ch0 val =0x%x \r\n",val);

               
        }while(((val&DMA_DONE) != DMA_DONE)||(i<6));

        ////write_mreg32(VI_DMA_CTL, 0x00000000);
        for(i = 0 ;i < 4; i++) {
        
        printf("MIPI_Y_BASEADDR(0x%x) = 0x%x \r\n",MIPI_Y_BASEADDR+4*i,read_mreg32(MIPI_Y_BASEADDR+4*i));
		}
    
        printf("dvp end  !!!!!!!!!!!!!!!!!!!!!!!  val =0x%x   0x%x  i=%d\r\n",val,MIPI_Y_BASEADDR+image_timing[image_timing_para].HACTIVE * image_timing[image_timing_para].VACTIVE * 2,i );
#endif
#ifdef __CHANNEL1__
       for(i = 0 ;i < 4; i++) {
        write_mreg32(MIPI_Y_BASEADDR+4*i,0xFFFFFFFF) ;
        printf("MIPI_Y_BASEADDR(0x%x) = 0x%x \r\n",MIPI_Y_BASEADDR+4*i,read_mreg32(MIPI_Y_BASEADDR+4*i));
       }
        i = 0 ;
        do
        {          
                val = read_mreg32(IRQ_STATUS+0x400);
                if((val&DMA_DONE) == DMA_DONE)
                {
                         i++;
                        write_mreg32(IRQ_CLR+0x400,0xffffffff);       
                }
                printf("  ch1 val =0x%x \r\n",val);

               
        }while(((val&DMA_DONE) != DMA_DONE)||(i<6));

        ///write_mreg32(VI_DMA_CTL+0x400, 0x00000000);
        for(i = 0 ;i < 4; i++) {
        
        printf("MIPI_Y_BASEADDR(0x%x) = 0x%x \r\n",MIPI_Y_BASEADDR+4*i,read_mreg32(MIPI_Y_BASEADDR+4*i));
		}
      
        printf("dvp end  !!!!!!!!!!!!!!!!!!!!!!!  val =0x%x   0x%x  i=%d\r\n",val,MIPI_Y_BASEADDR+0x1000000+image_timing[image_timing_para].HACTIVE * image_timing[image_timing_para].VACTIVE * 2,i );  
#endif
#ifdef __CHANNEL3__
        i = 0 ;
        do
        {          
                val = read_mreg32(IRQ_STATUS+0xc00);
                if((val&DMA_DONE) == DMA_DONE)
                {
                         i++;
                        write_mreg32(IRQ_CLR+0xc00,0xffffffff);       
                }
                printf("  ch3 val =0x%x \r\n",val);
                
               
        }while(((val&DMA_DONE) != DMA_DONE)||(i<6));

        write_mreg32(VI_DMA_CTL+0xc00, 0x00000000);

   
        printf("dvp end  !!!!!!!!!!!!!!!!!!!!!!!  val =0x%x   0x%x  i=%d\r\n",val,MIPI_Y_BASEADDR+0x3000000+image_timing[image_timing_para].HACTIVE * image_timing[image_timing_para].VACTIVE * 2,i );  


#endif
#ifdef __CHANNEL2__
        i = 0 ;
        do
        {          
                val = read_mreg32(IRQ_STATUS+0x800);
                if((val&DMA_DONE) == DMA_DONE)
                {
                         i++;
                        write_mreg32(IRQ_CLR+0x800,0xffffffff);       
                }
                printf("  ch2 val =0x%x \r\n",val);
               

               
        }while(((val&DMA_DONE) != DMA_DONE)||(i<6));

        write_mreg32(VI_DMA_CTL+0x800, 0x00000000);

        printf("dvp end  !!!!!!!!!!!!!!!!!!!!!!!  val =0x%x   0x%x  i=%d\r\n",val,MIPI_Y_BASEADDR+0x2000000+image_timing[image_timing_para].HACTIVE * image_timing[image_timing_para].VACTIVE * 2,i );  

#endif


    
}

