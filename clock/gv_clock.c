#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include "byavs_clock.h"

#define log_info(fmt, args...) seq_printf(m, fmt, ##args)

#define SYS_REF_CLK    24000000
#define TSM_REF_CLK    27000000
#define I2S_MCLK_EXTERNAL_PIN_CLK  0

#define REF_DIV(param) 		((param) & 0x3f)
#define MULTI_DIV(param)    ((param) >> 8) & 0xfff
#define DIV1(param)  		(((param) >> 20) & 0x7)
#define DIV2(param)			(((param) >> 24) & 0x7)
#define CAL_CLK_DIV(cfg, start_bit, bitnum)  (((cfg >> start_bit) & ((1 << (bitnum)) -1)) + 1)

#define DIV_BIT_MASK(bitnum)		((1 << (bitnum)) - 1)
#define DIV_MASK(start_bit, bitnum)  (DIV_BIT_MASK(bitnum) << (start_bit))
#define SET_DIV_BIT(val, start_bit, bitnum)  (((val) & DIV_BIT_MASK(bitnum)) << (start_bit))
 
#define MOD_NAME "driver/clk"

static struct gv_clk clk_ctrl;

//static struct proc_dir_entry * clk_proc = NULL;

#define GV9531_CLOCK "gv9531_clock"

static struct proc_dir_entry * gv9531_clock_proc = NULL;

static inline void clk_write(struct gv_clk * pclk, u32 offset, u32 val)
{
	iowrite32(val, pclk->base + offset);
}

static inline u32 clk_read(struct gv_clk * pclk, u32 offset)
{
	return ioread32(pclk->base + offset);
}

static inline unsigned int pll_clk_out(struct gv_clk * pclk, unsigned int offset)
{
	unsigned int div1 = 1, div2 = 1;
	unsigned int ref_div = 1;
	unsigned int multi_div = 1;
	unsigned int pll_param = 1;

	//PLL BYPASSED: 0->normal
	if((clk_read(pclk, offset + 0x4) & 0x1) == 0)
	{
		pll_param = clk_read(pclk, offset);
		ref_div = REF_DIV((pll_param));
		multi_div = MULTI_DIV(pll_param);
		div1 = DIV1(pll_param);
		div2 = DIV2(pll_param);
	}
	else{
		printk("module offset: 0x%x, pll is not open\n", offset);
	}
	
	if(offset == TSM_PLL_PARAM)
		return (pclk->tsm_clk_ref / ref_div ) * multi_div / (div1 * div2);
	else
		return (pclk->clk_ref / ref_div ) * multi_div / (div1 * div2);
}
#if 0
static int gvclk_list(struct seq_file *m, void *v)
{
	unsigned int sys_pll_clk;
	unsigned int ck_pll_clk;
	unsigned int ddr_pll_clk;
	unsigned int video0_pll_clk;
	unsigned int video1_pll_clk;
	unsigned int dsp_pll_clk;
	unsigned int tsm_pll_clk;
	unsigned int gmac_pll_clk;
	unsigned int pixel_pll_clk;
	unsigned int audio_pll_clk;
	unsigned int sensor_pll_clk;

	unsigned int clk_cfg;
	unsigned int tmp_clk_out;

	unsigned int sys_aclk_h;
	unsigned int sys_aclk_l;
	unsigned int sys_hclk;
	unsigned int sys_pclk;
	
	struct gv_clk *pclk = (struct gv_clk *)m->private;
	
	log_info("current system input clk: %u\n", pclk->clk_ref);
	sys_pll_clk = pll_clk_out(pclk, SYS_PLL_PARAM);
	ck_pll_clk = pll_clk_out(pclk, CK_PLL_PARAM);
	ddr_pll_clk = pll_clk_out(pclk, DDR_PLL_PARAM);
	video0_pll_clk = pll_clk_out(pclk, VIDEO0_PLL_PARAM);
	video1_pll_clk = pll_clk_out(pclk, VIDEO1_PLL_PARAM);
	dsp_pll_clk = pll_clk_out(pclk, DSP_PLL_PARAM);
	tsm_pll_clk = pll_clk_out(pclk, TSM_PLL_PARAM);
	gmac_pll_clk = pll_clk_out(pclk, GMAC_PLL_PARAM);
	pixel_pll_clk = pll_clk_out(pclk, PIXEL_PLL_PARAM);
	audio_pll_clk = pll_clk_out(pclk, AUDIO_PLL_PARAM);
	sensor_pll_clk = pll_clk_out(pclk, SENSOR_PLL_PARAM);
	
	log_info("*************system pll clk****************\n");
	log_info("sys_pll_clk				%u\n", sys_pll_clk);
	log_info("ck_pll_clk				%u\n", ck_pll_clk);
	log_info("ddr_pll_clk				%u\n", ddr_pll_clk);
	log_info("video0_pll_clk				%u\n", video0_pll_clk);
	log_info("video1_pll_clk				%u\n", video1_pll_clk);
	log_info("dsp_pll_clk				%u\n", dsp_pll_clk);
	log_info("tsm_pll_clk				%u\n", tsm_pll_clk);
	log_info("gmac_pll_clk				%u\n", gmac_pll_clk);
	log_info("pixel_pll_clk				%u\n", pixel_pll_clk);
	log_info("audio_pll_clk				%u\n", audio_pll_clk);
	log_info("sensor_pll_clk				%u\n", sensor_pll_clk);

	log_info("************module clock ouput*************\n");
	clk_cfg = clk_read(pclk, CK860_CLK_CFG);
	log_info("ck860_aclk				%u\n", ck_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 3));
	
	clk_cfg = clk_read(pclk, SYS_CLK_CFG);
	sys_aclk_h = sys_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 3);
	sys_aclk_l = sys_aclk_h / CAL_CLK_DIV(clk_cfg, 4, 3);
	sys_hclk = sys_aclk_l / CAL_CLK_DIV(clk_cfg, 8, 3);
	sys_pclk = sys_aclk_l / CAL_CLK_DIV(clk_cfg, 12, 3);
	log_info("sys_aclk_h				%u\n", sys_aclk_h);
	log_info("sys_aclk_l				%u\n", sys_aclk_l);
	log_info("sys_hclk				%u\n", sys_hclk);
	log_info("sys_pclk				%u\n", sys_pclk);

	clk_cfg = clk_read(pclk, XDMA_CLK_CFG);
	log_info("axi dma aclk				%u\n", sys_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 4));

	clk_cfg = clk_read(pclk, GMAC_CLK_CFG);
	log_info("gmac					%u\n", gmac_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 4));

	clk_cfg = clk_read(pclk, SDIO_CLK_CFG);
	if(clk_cfg & 0x10000){
		log_info("SD0 CLK					%u\n", gmac_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 6));
		log_info("SD1 CLK					%u\n", gmac_pll_clk / CAL_CLK_DIV(clk_cfg, 8, 6));
	}else{
		log_info("SD0 CLK					%u\n", sys_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 6));
		log_info("SD1 CLK					%u\n", sys_pll_clk / CAL_CLK_DIV(clk_cfg, 8, 6));
	}

	clk_cfg = clk_read(pclk, PCIE_CLK_CFG);
	log_info("PCIE CLK				%u\n", gmac_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 4));

	clk_cfg = clk_read(pclk, I2S_CLK_CFG);
	if(clk_cfg & 0x10000){
		//external clock
		tmp_clk_out = pclk->i2s_extern_pin_clk / CAL_CLK_DIV(clk_cfg, 0, 12);
		log_info("I2S MASTER CLK				%u\n", tmp_clk_out);
		log_info("I2S SLAVE CLK					%u\n",  tmp_clk_out / CAL_CLK_DIV(clk_cfg, 0, 12));
	}else{
		tmp_clk_out = audio_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 12);
		log_info("I2S MASTER CLK				%u\n", tmp_clk_out);
		log_info("I2S SLAVE CLK				%u\n",  tmp_clk_out / CAL_CLK_DIV(clk_cfg, 12, 6));
	}

	clk_cfg = clk_read(pclk, VIN_CLK_CFG);
	if(clk_cfg & 0x10)
		log_info("VIN CLK					%u\n",  pixel_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 4));
	else
		log_info("VIN CLK					%u\n",  video0_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 4));

	clk_cfg = clk_read(pclk, VDEC_CLK_CFG);
	if(clk_cfg & 0x10)
		log_info("VDEC CLK				%u\n",  pixel_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 4));
	else
		log_info("VDEC CLK				%u\n",  video0_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 4));

	clk_cfg = clk_read(pclk, VENC_CLK_CFG);
	log_info("VENC CLK				%u\n", video0_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 4));

	clk_cfg = clk_read(pclk, JPEG_CLK_CFG);
	log_info("JPEG CLK				%u\n", video0_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 4));

	clk_cfg = clk_read(pclk, ISP_CLK_CFG);
	if(clk_cfg & 0x10)
		log_info("ISP CLK					%u\n",  gmac_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 4));
	else
		log_info("ISP CLK					%u\n",  video0_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 4));

	clk_cfg = clk_read(pclk, GPU_CLK_CFG);
	if(clk_cfg & 0x10)
		log_info("GPU CLK					%u\n",  pixel_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 4));
	else
		log_info("GPU CLK					%u\n",  video1_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 4));

	clk_cfg = clk_read(pclk, VOUT_CLK_CFG);
	if(clk_cfg & 0x10)
		log_info("VOUT CLK				%u\n",  pixel_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 4));
	else
		log_info("VOUT CLK				%u\n",  video0_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 4));

	clk_cfg = clk_read(pclk, AVS2_CLK_CFG);
	if(clk_cfg & 0x1000)
		log_info("CK610 CLK				%u\n",  pixel_pll_clk / CAL_CLK_DIV(clk_cfg, 8, 4));
	else
		log_info("CK610 CLK				%u\n",  video1_pll_clk / CAL_CLK_DIV(clk_cfg, 8, 4));
	log_info("AVSP CLK				%u\n",  video1_pll_clk / CAL_CLK_DIV(clk_cfg, 4, 4));
	log_info("AVS2 CLK 				%u\n",	video1_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 4));

	clk_cfg = clk_read(pclk, TS_CLK_CFG);
	if(clk_cfg & 0x10000)
		log_info("STC 27M CLK				%u\n",	pixel_pll_clk / CAL_CLK_DIV(clk_cfg, 8, 8));
	else
		log_info("STC 27M CLK				27000000\n");
	tmp_clk_out = tsm_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 4);
	log_info("TSM PSI CLK 				%u\n",	tmp_clk_out);
	log_info("TSM PSI 27M CLK 			%u\n",	tmp_clk_out / CAL_CLK_DIV(clk_cfg, 4, 4));
	
	clk_cfg = clk_read(pclk, C5_CLK_CFG);
	if(clk_cfg & 0x10)
		tmp_clk_out = video1_pll_clk;
	else
		tmp_clk_out = dsp_pll_clk;
	log_info("C5 CLK					%u\n",	tmp_clk_out / CAL_CLK_DIV(clk_cfg, 0, 4));

	clk_cfg = clk_read(pclk, P6_CLK_CFG);
	log_info("P6 CLK					%u\n",	dsp_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 4));

	clk_cfg = clk_read(pclk, CDVS_CLK_CFG);
	if(clk_cfg & 0x10)
		tmp_clk_out = 0; //uni pll clock output;
	else
		tmp_clk_out = video0_pll_clk;
	tmp_clk_out = tmp_clk_out / CAL_CLK_DIV(clk_cfg, 0, 4);
	log_info("CDVS MASTER CLK 			%u\n",	tmp_clk_out);
	log_info("CDVS SLAVE CLK 				%u\n",	tmp_clk_out / CAL_CLK_DIV(clk_cfg, 8, 4));
	
	clk_cfg = clk_read(pclk, VIP_CLK_CFG);
	switch((clk_cfg >> 4) & 0x3){
		case 0:
			tmp_clk_out = video0_pll_clk;
			break;
		case 1:
			tmp_clk_out = tsm_pll_clk;
			break;
		case 2:
			tmp_clk_out = dsp_pll_clk;
			break;
		case 3:
		default:
			tmp_clk_out = 0;
	}
	log_info("VIP CLK					%u\n",	tmp_clk_out / CAL_CLK_DIV(clk_cfg, 0, 4));

	clk_cfg = clk_read(pclk, CAMB_CLK_CFG);
	log_info("CAMB CLK				%u\n",	tsm_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 4));

	clk_cfg = clk_read(pclk, SCI_CLK_CFG);
	log_info("SCI CLK					%u\n",	sys_pclk / CAL_CLK_DIV(clk_cfg, 0, 8));

	clk_cfg = clk_read(pclk, GPIO_DBCLK_CFG);
	log_info("GPIO CLK 				%u\n",	sys_pclk / (clk_cfg + 1));

	clk_cfg = clk_read(pclk, TIMER_CLK_CFG);
	log_info("TIMER CLK				%u\n",	sys_pclk / CAL_CLK_DIV(clk_cfg, 0, 16));
	
	clk_cfg = clk_read(pclk, SENSOR0_CLK_CFG);
	log_info("SENSOR0 CLK				%u\n",	sensor_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 12));

	clk_cfg = clk_read(pclk, SENSOR1_CLK_CFG);
	if(clk_cfg & 0x10000)
		tmp_clk_out = pclk->clk_ref;
	else if(clk_cfg & 0x1000)
		tmp_clk_out = gmac_pll_clk;
	else
		tmp_clk_out = pixel_pll_clk;
	log_info("SENSOR1 CLK				%u\n",	tmp_clk_out / CAL_CLK_DIV(clk_cfg, 0, 12));

	clk_cfg = clk_read(pclk, DISPLAY_CLK_CFG);
	log_info("DISPLAY CLK				%u\n",  pixel_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 8));

	clk_cfg = clk_read(pclk, MIPI_EXT_CLK_CFG);
	if(clk_cfg & 0x10)
		log_info("MIPI CLK				%u\n",  video1_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 4));
	else
		log_info("MIPI CLK				%u\n",  video0_pll_clk / CAL_CLK_DIV(clk_cfg, 0, 4));
	
	return 0;
}
#endif
static int gvclk_clock_list(struct seq_file *m, void *v)
{
	struct gv_clk *pclk = (struct gv_clk *)m->private;
	
	log_info("%s_clk: %dHZ\n", pclk->dir_attr->name, pll_clk_out(pclk, pclk->dir_attr->offset));

	return 0;
}

static int gvclk_proc_open(struct inode *inode, struct file *file)
{
	struct gv_clk *pclk = &clk_ctrl;
	struct file_desc *file_attr = (struct file_desc *)PDE_DATA(inode);
	struct dir_desc * dir_attr = (struct dir_desc *)proc_get_parent_data(inode);
	
	pclk->file_attr = file_attr;
	pclk->dir_attr = dir_attr;
	
	pclk->base = ioremap(CRM_BASE, CRM_SIZE);
	if(pclk->base == NULL){
		printk("Failed to ioreamp CRM base, addr:0x%x, size:0x%x\n", CRM_BASE, CRM_SIZE);
		return -1;
	}
	pclk->clk_ref = SYS_REF_CLK;
	pclk->tsm_clk_ref = TSM_REF_CLK;
	pclk->i2s_extern_pin_clk = I2S_MCLK_EXTERNAL_PIN_CLK;
	
	return single_open(file, gvclk_clock_list, pclk);
}

static int gvclk_proc_release(struct inode *inode, struct file *file)
{
	struct seq_file *m = (struct seq_file *)file->private_data;
	struct gv_clk *pclk = (struct gv_clk *)m->private;

	if(pclk->base != NULL){
		iounmap(pclk->base);
		pclk->base = NULL;
	}
	
	return seq_release(inode, file);
}

static int strtoint(const char *str)
{
	unsigned long val = 1;
	
	if((str[0] == '0') && (str[1] == 'x')){ //16
		str++;
		str++;
	
		while(*str ==  '0')
			str++;
		kstrtoul(str, 16, &val);
	}
	else if (str[0] == '0' ){
        
return -1;
	}
	else{
		kstrtoul(str, 10, &val);
	}
	return val;
}

ssize_t gvclk_proc_write (struct file *filp, const char __user * buf, size_t count, loff_t * offset)
{
	int start_bit, bitnum;
	unsigned int pllparam, val, new_val = 1;
	char div_str[16];
	struct seq_file *m = (struct seq_file *)filp->private_data;
	struct gv_clk *pclk = (struct gv_clk *)m->private;

	if( copy_from_user(div_str, buf, count)){
		printk("Failed to copy user div val\n");
		return -1;
	}

	new_val = strtoint(div_str);
	//printk("new_val:%u\n", new_val);
	
	start_bit = pclk->file_attr->start_bit;
	bitnum = pclk->file_attr->bit_cnt;

	//printk("%s: start bit:%d, bit count:%d\n", pclk->file_attr->name, start_bit, bitnum);
	//printk("%s: offset:%d\n", pclk->dir_attr->name,  pclk->dir_attr->offset);
	
	pllparam = clk_read(pclk, pclk->dir_attr->offset);
	pllparam &= (~ DIV_MASK(start_bit, bitnum));
	val = SET_DIV_BIT(new_val, start_bit, bitnum);
	pllparam |= val;
	//clk_write(pclk, pclk->dir_attr->offset, pllparam);
	
	return count;
}

static const struct file_operations gvclk_proc_operation = {
	.owner		= THIS_MODULE,
	.open		= gvclk_proc_open,
	.read		= seq_read,
	.llseek 	= seq_lseek,
	.release	= gvclk_proc_release,
	.write 		= gvclk_proc_write,
};
	
struct dir_desc pll_dir[] = {
	{"syspll", 		0x0 << 4,},
	{"ckpll", 		0x1 << 4,},
	{"unipll", 		0x2 << 4,},
	{"ddrpll", 		0x3 << 4,},
	{"video0pll", 	0x4 << 4,},
	{"video1pll", 	0x5 << 4,},
	{"dsppll", 		0x6 << 4,},
	{"tsmpll", 		0x7 << 4,},
	{"gmacpll", 	0x8 << 4,},
	{"pixelpll", 	0x9 << 4,},
	{"audiopll", 	0xa << 4,},
	{"sensorpll", 	0xb << 4,},
};

struct file_desc pll_file[] = {
	{"out", 	0,  0,},
	{"refdiv", 	0,  6,},
	{"fbdiv", 	8,  12,},
	{"post1",	20, 3,},
	{"post2", 	24, 3,},
};

void gvclock_proc_init(void)
{
	int i, j;
	umode_t mode;
	struct proc_dir_entry * dir_entry = NULL;
	
	gv9531_clock_proc = proc_mkdir(GV9531_CLOCK, NULL);
	if(gv9531_clock_proc == NULL){
		printk("Failed to create %s proc fs\n", GV9531_CLOCK);
		return;
	}

	for(i = 0; i < sizeof(pll_dir) / sizeof(struct dir_desc); i++){
		dir_entry = proc_mkdir_data(pll_dir[i].name, 0666, gv9531_clock_proc, &pll_dir[i]);
		if(dir_entry != NULL){
			for(j = 0; j < sizeof(pll_file) / sizeof(struct file_desc); j++){
				mode = ((j == 0) ? 0400 : 0200);
				proc_create_data(pll_file[j].name, mode, dir_entry, &gvclk_proc_operation, &pll_file[j]);
			}
		}
		
	}
}

void gvclock_proc_exit(void)
{
	if(gv9531_clock_proc != NULL){
		proc_remove(gv9531_clock_proc);
		gv9531_clock_proc = NULL;
	}
}

static int __init gv_clock_init(void)
{
    gvclock_proc_init();
	
    return 0;
}

static void __exit gv_clock_exit(void)
{   
	gvclock_proc_exit();
}

module_init(gv_clock_init);
module_exit(gv_clock_exit);

MODULE_AUTHOR("yongliang.tu@byavs.com");
MODULE_DESCRIPTION("Byavs Clock Driver");
MODULE_LICENSE("GPL");
