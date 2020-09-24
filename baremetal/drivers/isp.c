#include "isp.h"
#include "ck810.h"
#include "mipi_subsys.h"


void Isp_func_WDR2_On(void)
{
   unsigned int i;
   unsigned int val ;
 #ifdef MIPI_ISP_CH1
   
    val = read_mreg32(CK_SYS_CTRL_ADDR+0x88) ;
	 printf("Isp_func_WDR2_On  isp ctrl = 0x%x\n", val);
	
    ///// write_mreg32(CK_SYS_CTRL_ADDR+0x88, 0x01);
 
   for(i = 0; i < ARRAY_SIZE(Isp_On_Params); i++) 
   { 
 #if 1
        /*  
        if(Isp_On_Params[i].addr == 0x00000400) 
	{
          write_mreg32(CK_ISP1_Slave+Isp_On_Params[i].addr, Isp_On_Params[i].val&0xfffffff1);

	}else if(Isp_On_Params[i].addr == 0x00001400) 
	{
           write_mreg32(CK_ISP1_Slave+Isp_On_Params[i].addr,((Isp_On_Params[i].val&0xfffffff0)|0x00000008));
	}else*/
         
         
    if(Isp_On_Params[i].addr == 0x00000404) 
	{
          write_mreg32(CK_ISP1_Slave+Isp_On_Params[i].addr, 0x00000007);

	}else if(Isp_On_Params[i].addr == 0x00001400) 
	{
          write_mreg32(CK_ISP1_Slave+Isp_On_Params[i].addr, (Isp_On_Params[i].val&0xFF3FFFFF)|0x00800000);

	}else
         write_mreg32(CK_ISP1_Slave+Isp_On_Params[i].addr, Isp_On_Params[i].val);
#else
        
         write_mreg32(CK_ISP1_Slave+Isp_On_Params[i].addr, Isp_On_Params[i].val);

#endif
         printf("Isp_func_WDR2_On  (0x%x)0x%x\n", Isp_On_Params[i].val,read_mreg32(CK_ISP1_Slave+Isp_On_Params[i].addr));
   }
   printf("Isp_func_WDR2_On  (0x404===========0x%x\n", read_mreg32(CK_ISP1_Slave+0x404));

#else 
	 val = read_mreg32(CK_SYS_CTRL_ADDR+0x88) ;
	 printf("Isp_func_WDR2_On  isp ctrl = 0x%x\n", val);
	
  ////   write_mreg32(CK_SYS_CTRL_ADDR+0x88, 0x02);
   for(i = 0; i < ARRAY_SIZE(Isp_On_Params); i++) 
   { 
 #if 1
        /*  
        if(Isp_On_Params[i].addr == 0x00000400) 
	{
          write_mreg32(CK_ISP0_Slave+Isp_On_Params[i].addr, Isp_On_Params[i].val&0xfffffff1);

	}else if(Isp_On_Params[i].addr == 0x00001400) 
	{
           write_mreg32(CK_ISP0_Slave+Isp_On_Params[i].addr,((Isp_On_Params[i].val&0xfffffff0)|0x00000008));
	}else*/
         
         
    if(Isp_On_Params[i].addr == 0x00000404) 
	{
          write_mreg32(CK_ISP0_Slave+Isp_On_Params[i].addr, 0x00000007);

	}else if(Isp_On_Params[i].addr == 0x00001400) 
	{
          write_mreg32(CK_ISP0_Slave+Isp_On_Params[i].addr, (Isp_On_Params[i].val&0xFF3FFFFF)|0x00800000);

	}else
         write_mreg32(CK_ISP0_Slave+Isp_On_Params[i].addr, Isp_On_Params[i].val);
#else
        
         write_mreg32(CK_ISP0_Slave+Isp_On_Params[i].addr, Isp_On_Params[i].val);

#endif
         printf("Isp_func_WDR2_On  (0x%x)0x%x\n", Isp_On_Params[i].val,read_mreg32(CK_ISP0_Slave+Isp_On_Params[i].addr));
   }
   
#endif   
}

void Isp_func_WDR2_Off(void)
{
   unsigned int i;
   unsigned int val;
    #ifdef MIPI_ISP_CH1
	 val = read_mreg32(CK_SYS_CTRL_ADDR+0x88) ;
	 printf("Isp_func_WDR2_On  isp ctrl = 0x%x\n", val);
	
     ///write_mreg32(CK_SYS_CTRL_ADDR+0x88, 0x01);
	 printf("Isp_func_WDR2_Off 0x10 = 0x%x\n", read_mreg32(CK_ISP1_Slave+0x10));
   printf("Isp_func_WDR2_Off 0x414 = 0x%x\n", read_mreg32(CK_ISP1_Slave+0x414));
   printf("Isp_func_WDR2_Off 0x41c = 0x%x\n", read_mreg32(CK_ISP1_Slave+0x41c));
   for(i = 0; i < ARRAY_SIZE(Isp_Off_Params); i++) 
   {       
         write_mreg32(CK_ISP1_Slave+Isp_Off_Params[i].addr, Isp_Off_Params[i].val);
   }
	#else
		 val = read_mreg32(CK_SYS_CTRL_ADDR+0x88) ;
	 printf("Isp_func_WDR2_On  isp ctrl = 0x%x\n", val);
	
     ///write_mreg32(CK_SYS_CTRL_ADDR+0x88, 0x02);
   printf("Isp_func_WDR2_Off 0x10 = 0x%x\n", read_mreg32(CK_ISP0_Slave+0x10));
   printf("Isp_func_WDR2_Off 0x414 = 0x%x\n", read_mreg32(CK_ISP0_Slave+0x414));
   printf("Isp_func_WDR2_Off 0x41c = 0x%x\n", read_mreg32(CK_ISP0_Slave+0x41c));
   for(i = 0; i < ARRAY_SIZE(Isp_Off_Params); i++) 
   {       
         write_mreg32(CK_ISP0_Slave+Isp_Off_Params[i].addr, Isp_Off_Params[i].val);
   }
   
   #endif
}			
			


void Isp_Check_status(void)
{
    	unsigned int val;
       printf("start waiting .......  Isp_Check_status  \r\n"); 
#ifdef MIPI_ISP_CH1

           	while(1)
    	{
    		val  =  read_mreg32(CK_ISP1_Slave+0x1500);
                
                if((val&0x03))
                  break;
	}
        printf("Succeed  Isp_Check_status val = 0x%x \r\n",val); 
#else		
    	while(1)
    	{
    		val  =  read_mreg32(CK_ISP0_Slave+0x1500);
                
                if((val&0x03))
                  break;
	}
        printf("Succeed  Isp_Check_status val = 0x%x \r\n",val); 
		
#endif		
}
