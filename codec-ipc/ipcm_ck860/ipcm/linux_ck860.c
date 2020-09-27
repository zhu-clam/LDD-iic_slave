
/*
*                                                                          
*Copyright (c)  :2019-01-4  Grand Vision Design Systems Inc.   /
*Permission is hereby granted, free of charge, to any person obtaining  /
*a copy of this software and associated documentation files (the   /
*Software), to deal in the Software without restriction, including   /
*without limitation the rights to use, copy, modify, merge, publish,
*distribute, sublicense, and/or sell copies of the Software, and to   /
*permit persons to whom the Software is furnished to do so, subject to   /
*the following conditions:  /
*The above copyright notice and this permission notice shall be included   /
*in all copies or substantial portions of the Software.  /
*                                                                          
*THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND,  /
*EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF  /
*MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  /
*IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   /
*CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  /
*TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE   /
*SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  /
*                                                                          
*                                                                          
*@Filename: linux_ck860.c                                             
*                                                                          
*@Author: zhuxianfei                            
*@Created on     : 2019-4 -25               
*------------------------------------------------------------------------------
*@Description:                                                          
*                                                                          
*@Modification History                                                                          
*                                                                          
*                                                                          
*/

#include <linux/interrupt.h>
#include "ipcm_osadapt.h"


#define SYS_CTRL 0xF9701000

//#define CK860_SW_INTR 0xF9701040
//#define CK810_SW_INTR 0xF9701044

#define CK860_TO_OTHER_SW_INTR 0x40
#define CK810_TO_OTHER_SW_INTR 0x44

#define IRQ_NUM 77

extern int irq_callback(int irq, void *data);

static unsigned char *sys_ctrl_reg_base;


static int arch_initialized = 0;



void __interrupt_trigger__(int cpu, int irq)
{

	unsigned int ck860_sw_int;
	ck860_sw_int = readl(sys_ctrl_reg_base + CK860_TO_OTHER_SW_INTR);
	ipcm_trace(TRACE_ZXF_DEBUG,"before CK860 interrupt_trigger ck860_sw_int reg:%#x !",ck860_sw_int);
	
	ck860_sw_int |= (0x1 << 0);
	writel(ck860_sw_int,(sys_ctrl_reg_base + CK860_TO_OTHER_SW_INTR));	
	ipcm_trace(TRACE_ZXF_DEBUG,"after1 CK860 interrupt_trigger ck860_sw_int reg:%#x !",ck860_sw_int);
}


static int __irq_callback(int irq, void *data)
{
	unsigned int ck860_sw_int ;
	irq_callback(irq, data);

	/* after interrupt seriver,clear interrupt flag */
	ck860_sw_int = readl(sys_ctrl_reg_base + CK810_TO_OTHER_SW_INTR);
	ipcm_trace(TRACE_ZXF_DEBUG,"before from_ck860_sw_int:%#x",ck860_sw_int);

	ck860_sw_int &= ~(0x1 << 0);
	writel(ck860_sw_int,(sys_ctrl_reg_base + CK810_TO_OTHER_SW_INTR));
	ipcm_trace(TRACE_ZXF_DEBUG,"after from_ck860_sw_int:%#x",ck860_sw_int);

	return IRQ_HANDLED;
}

/*
* descrption：softirq register 
* return : 0 success; negetive number means fail
*/
int  __arch_init__(void)
{

	unsigned int ck860_sw_int;
	
	sys_ctrl_reg_base = __ipcm_io_mapping__(SYS_CTRL, 0x100);
	if (NULL == sys_ctrl_reg_base) {
		ipcm_err("ipc_int_reg_base iomapping err");
		return -1;
	}

	if (__ipcm_irq_request__(IRQ_NUM, __irq_callback)) {
		__ipcm_io_unmapping__(sys_ctrl_reg_base);
		ipcm_err("request irq [%d] failed!", IRQ_NUM);
		return -1;
	}
	
	ck860_sw_int = readl(sys_ctrl_reg_base + CK810_TO_OTHER_SW_INTR);
	ipcm_trace(TRACE_ZXF_DEBUG,"arch_init1 ck860_sw_int:%#x",ck860_sw_int);

	
	if (ck860_sw_int & (0x1 << 0)) 
	{
		ck860_sw_int &= ~(0x1<<0);
		writel(ck860_sw_int,sys_ctrl_reg_base + CK810_TO_OTHER_SW_INTR);
		ipcm_trace(TRACE_ZXF_DEBUG,"arch_init2 ck860_sw_int:%#x",ck860_sw_int);
		
	}

	arch_initialized = 1;
	return 0;
}

void __arch_free__(void)
{
	if (arch_initialized) {
		__ipcm_irq_free__(IRQ_NUM);
		__ipcm_io_unmapping__(sys_ctrl_reg_base);
	}

	arch_initialized = 0;
}

