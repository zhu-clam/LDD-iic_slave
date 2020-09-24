/*
 * (Copyright 2017) Verisilicon Inc. All Rights Reserved
 * Company confidential and Proprietary information.
 * This information may not be disclosed to unauthorized individual.
 * 
 * System Application Team <fengyin.wu@verisilicon.com>
 *
 */

#include <string.h>
#include "datatype.h"
#include "platform.h"
#include "common.h"
#include "mmc.h"
#include "sdhci.h"
#include "timer.h"
#include "boot.h"
#include "sdio_boot.h"
//#define SDIO_DB
#ifdef SDIO_DB
#define sdio_debug(format, ...)  debug(format,##__VA_ARGS__)
#else
#define sdio_debug(format, ...)  do {} while (0)
#endif
#ifdef CONFIG_MMC
static struct sdhci_host emmc_host;
static struct mmc emmc_mmc;
#endif
static struct mmc sd_card_mmc;
static struct sdhci_host sd_card_host;

//extern u32 sdio_card_ip_clk,sdio_card_work_max_clk,sdio_card_work_min_clk;
//extern u32 sdio_emmc_ip_clk,sdio_emmc_work_max_clk,sdio_emmc_work_min_clk;
u32 sdio_card_ip_clk        = APB_DEFAULT_FREQ;//400KHz
u32 sdio_card_work_max_clk  = 400000;//400KHz
u32 sdio_card_work_min_clk  = 400000;//400KHz

int sd_card_init(void)
{
	char *deepeye_sd = "Polaris SDHCI";
	int ret;
	
	if(sd_card_mmc.mmc_inited == 0x55)
		return 0;

	memset(&sd_card_host,0x0,sizeof(sd_card_host));
	memset(&sd_card_mmc,0x0,sizeof(sd_card_mmc));

	sd_card_host.name = deepeye_sd;
	sd_card_host.ops = NULL;
	sd_card_host.ioaddr = (void *)CK_SDIO0_BASEADDRESS;
	sd_card_host.index = 0;
	sd_card_host.bus_width = 1;
	sd_card_host.quirks = 0;
	sd_card_host.max_clk = sdio_card_ip_clk; // bus clock
	sd_card_host.voltages = MMC_VDD_32_33 | MMC_VDD_33_34;// | MMC_VDD_165_195;
	
	if (sd_card_host.bus_width == 4)
		sd_card_host.host_caps |= MMC_MODE_4BIT;
	if (sd_card_host.bus_width == 8)
		sd_card_host.host_caps |= MMC_MODE_8BIT;

	 sdhci_setup_cfg(&(sd_card_host.cfg), &sd_card_host, sdio_card_work_max_clk, sdio_card_work_min_clk);//ip support max,min clock

	sd_card_mmc.cfg = &(sd_card_host.cfg);
	sd_card_mmc.priv = &sd_card_host;
	sd_card_mmc.voltages = sd_card_mmc.cfg->voltages;
	sd_card_mmc.f_max = sd_card_mmc.cfg->f_max;
	sd_card_mmc.f_min = sd_card_mmc.cfg->f_min;
	sd_card_mmc.b_max = sd_card_mmc.cfg->b_max;

	ret = mmc_init(&sd_card_mmc);
	if(ret)
	{
		debug("sdio initialize failed.\n");
		return ret;
	}
	else
	{
		sd_card_mmc.mmc_inited = 0x55;
		info_debug("sdio initialize done.\n");
		return 0;
	}
}

int sd_card_boot(void)
{
	int i;
	sb_header header;
	u8* p_header = &header;
	u32 buffer_addr = 0,block_cnt=0,col=0;
	u16 crc16 = 0;
	LOAD_ENTRY enter_jump_func;
	
	char buffer_r[MMC_MAX_BLOCK_LEN];
	memset(buffer_r,0x00,MMC_MAX_BLOCK_LEN);

	if (mmc_bread(&sd_card_mmc,SD_CARD_BOOT_ADDR, 1, (u32)buffer_r) != 1)
	{
		debug("mmc_bread failed.\n");
		return -BOOT_FAILED;
	}
	memcpy(p_header,buffer_r,sizeof(header));
	sdio_debug("magic=0x%x size=0x%x load_addr=0x%x entry_addr=0x%x crc16=0x%x \n",\
		header.magic_num,header.data_size,header.load_addr,header.entry_point,header.crc16);

	if(header.magic_num != MAGIC_NUM)
	{
		debug("magic_num error.\n");
		return -BOOT_FAILED;//boot failed
	}
	//get data len, may be need check len (max) error return -1
	if(header.data_size > SPL_MAX_LEN)
	{
		debug("data_size error.\n");
		return -BOOT_FAILED;//boot failed
	}
	//check load address
	if(header.load_addr < SRAM_START_ADDRESS)
	{
		debug("load_addr error.\n");
		return -BOOT_FAILED;//boot failed
	}
	if((header.load_addr+header.data_size)>SPL_MAX_ADDRESS)
	{
		debug("the data is out of bounds.\n");
		return -BOOT_FAILED;//boot failed
	}

	buffer_addr = header.load_addr;
	block_cnt = (header.data_size+sizeof(header)) / sd_card_mmc.read_bl_len;
	col = (header.data_size+sizeof(header))%sd_card_mmc.read_bl_len;

	for(i=0;i<block_cnt;i++)
	{
		if (mmc_bread(&sd_card_mmc,SD_CARD_BOOT_ADDR+i, 1, (u32)buffer_addr) != 1)
		{
			debug("mmc_bread failed.\n");
			return -BOOT_FAILED;
		}
		if(i == 0)
		{
			memcpy((void*)buffer_addr,(void*)(buffer_r+sizeof(header)),(sd_card_mmc.read_bl_len-sizeof(header)));
			buffer_addr += (sd_card_mmc.read_bl_len-sizeof(header));
		}
		else
		{
			buffer_addr += sd_card_mmc.read_bl_len;
		}	
	}
	if(col)
	{
		if (mmc_bread(&sd_card_mmc,SD_CARD_BOOT_ADDR+block_cnt, 1, (u32)buffer_r) != 1)
		{
			debug("mmc_bread failed.\n");
			return -BOOT_FAILED;
		}
		memcpy((void*)buffer_addr,(void*)buffer_r,col);

	}
	info_debug("sd card read done.\n");

	crc16 = check_sum((u8*)header.load_addr, header.data_size);
	if(crc16 != (u16)header.crc16)
	{
		debug("checksum error.\n");
		return -BOOT_FAILED;//boot failed
	}

	enter_jump_func = (LOAD_ENTRY)header.entry_point;
	enter_jump_func();
	return 0;
}

int sd_card_write(u32 start_blk, u32 len, void * src_buffer)
{
	u32 i=0,j=1;
	u32 block_cnt=0,col=0;
	
	char buffer_w[MMC_MAX_BLOCK_LEN];
	memset(buffer_w,0x00,MMC_MAX_BLOCK_LEN);
		
	block_cnt = len / sd_card_mmc.write_bl_len;
	col = len % sd_card_mmc.write_bl_len;
	for(i=0;i<block_cnt;i++)
	{
		sdio_debug("w - %d  0x%x \n", start_blk+i, src_buffer);
		if (mmc_bwrite(&sd_card_mmc, start_blk+i, 1, src_buffer) != 1)
		{
			debug("mmc_bwrite failed.\n");
			return -1;
		}
		src_buffer += sd_card_mmc.write_bl_len;
		
		if(j%4 == 0)
			debug(".");
		if(j++%128 == 0)
			debug("\n");
	}
	if(col)
	{
		sdio_debug("wc - %d  0x%x \n", start_blk+block_cnt, src_buffer);
		memcpy((void*)buffer_w,(void*)src_buffer,col);
		if (mmc_bwrite(&sd_card_mmc, start_blk+block_cnt, 1, (void*)buffer_w) != 1)
		{
			debug("mmc_bwrite failed.\n");
			return -1;
		}		
		debug(".");
	}
	debug("\n");

	return 0;

}

int sd_card_erase(u32 start_blk, u32 len)
{
	u32 i=0, block_cnt=0;
	int j=1;
			
	block_cnt = len / sd_card_mmc.write_bl_len;
	for(i=0;i<block_cnt;i++)
	{
		if (mmc_berase(&sd_card_mmc, start_blk+i, 1) != 1)
		{
			debug("mmc_berase failed.\n");
			return -1;
		}
		
		if(j%4 == 0)
			debug(".");
		if(j++%128 == 0)
			debug("\n");
	}
	debug("\n");
	
	return 0;
}
#ifdef CONFIG_MMC
int emmc_init(void)
{
	char *deepeye_sd = "DeepEye SDHCI";
	int ret;
	
	if(emmc_mmc.mmc_inited == 0x55)
		return 0;
		
	memset(&emmc_host,0x0,sizeof(emmc_host));
	memset(&emmc_mmc,0x0,sizeof(emmc_mmc));

	emmc_host.name = deepeye_sd;
	emmc_host.ops = NULL;
	emmc_host.ioaddr = (void *)SDIO1_BASE;
	emmc_host.index = 0;
	emmc_host.bus_width = 1;
	emmc_host.quirks = 0;
	emmc_host.max_clk = sdio_emmc_ip_clk;//ip clock
	emmc_host.voltages = MMC_VDD_32_33 | MMC_VDD_33_34;// | MMC_VDD_165_195;

	if (emmc_host.bus_width == 4)
		emmc_host.host_caps |= MMC_MODE_4BIT;
	if (emmc_host.bus_width == 8)
		emmc_host.host_caps |= MMC_MODE_8BIT;

	 sdhci_setup_cfg(&(emmc_host.cfg), &emmc_host, sdio_emmc_work_max_clk, sdio_emmc_work_min_clk);// support max,min clock

	emmc_mmc.cfg = &(emmc_host.cfg);
	emmc_mmc.priv = &emmc_host;
	emmc_mmc.voltages = emmc_mmc.cfg->voltages;
	emmc_mmc.f_max = emmc_mmc.cfg->f_max;
	emmc_mmc.f_min = emmc_mmc.cfg->f_min;
	emmc_mmc.b_max = emmc_mmc.cfg->b_max;

	ret = mmc_init(&emmc_mmc);
	if(ret)
	{
		debug("sdio initialize failed.\n");
		return ret;
	}
	else
	{
		info_debug("sdio initialize done.\n");
		emmc_mmc.mmc_inited = 0x55;
		return 0;
	}
}

int emmc_boot(void)
{
	int i;
	sb_header header;
	u8* p_header = &header;
	u32 buffer_addr = 0,block_cnt=0,col=0;
	u16 crc16 = 0;
	LOAD_ENTRY enter_jump_func;
	
	char buffer_r[MMC_MAX_BLOCK_LEN];
	memset(buffer_r,0x00,MMC_MAX_BLOCK_LEN);

	sdio_debug("boot from emmc:%d \n",EMMC_BOOT_ADDR);
	if (mmc_bread(&emmc_mmc,EMMC_BOOT_ADDR, 1, (u32)buffer_r) != 1)
	{
		debug("mmc_bread failed.\n");
		return -BOOT_FAILED;//boot failed
	}
	memcpy(p_header,buffer_r,sizeof(header));
	sdio_debug("magic=0x%x size=0x%x load_addr=0x%x entry_addr=0x%x crc16=0x%x \n",\
		header.magic_num,header.data_size,header.load_addr,header.entry_point,header.crc16);

	if(header.magic_num != MAGIC_NUM)
	{
		debug("magic_num error.\n");
		return -BOOT_FAILED;//boot failed
	}
	//get data len, may be need check len (max) error return -1
	if(header.data_size > SPL_MAX_LEN)
	{
		debug("data_size error.\n");
		return -BOOT_FAILED;//boot failed
	}
	
	//check load address
	if(header.load_addr < SRAM_START_ADDRESS)
	{
		debug("load_addr error.\n");
		return -BOOT_FAILED;//boot failed
	}
	if((header.load_addr+header.data_size)>SPL_MAX_ADDRESS)
	{
		debug("the data is out of bounds.\n");
		return -BOOT_FAILED;//boot failed
	}

	buffer_addr = header.load_addr;
	block_cnt = (header.data_size+sizeof(header)) / emmc_mmc.read_bl_len;
	col = (header.data_size+sizeof(header))%emmc_mmc.read_bl_len;

	for(i=0;i<block_cnt;i++)
	{
		if (mmc_bread(&emmc_mmc,EMMC_BOOT_ADDR+i, 1, (u32)buffer_addr) != 1)
		{
			debug("mmc_bread failed.\n");
			return -BOOT_FAILED;//boot failed
		}
		if(i == 0)
		{
			memcpy((void*)buffer_addr,(void*)(buffer_r+sizeof(header)),(emmc_mmc.read_bl_len-sizeof(header)));
			buffer_addr += (emmc_mmc.read_bl_len-sizeof(header));
		}
		else
		{
			buffer_addr += emmc_mmc.read_bl_len;
		}	
	}
	if(col)
	{
		if (mmc_bread(&emmc_mmc,EMMC_BOOT_ADDR+block_cnt, 1, (u32)buffer_r) != 1)
		{
			debug("mmc_bread failed.\n");
			return -BOOT_FAILED;//boot failed
		}
		memcpy((void*)buffer_addr,(void*)buffer_r,col);

	}
	info_debug("eMMC read done.\n");

	crc16 = check_sum((u8*)header.load_addr, header.data_size);
	if(crc16 != (u16)header.crc16)
	{
		debug("checksum error.\n");
		return -BOOT_FAILED;//boot failed
	}

	enter_jump_func = (LOAD_ENTRY)header.entry_point;
	enter_jump_func();
	return 0;
}

/*
int emmc_read(u32 start_blk, u32 len,  u32 buffer_addr)
{
	int i;
	u32 block_cnt=0,col=0;


	block_cnt = (len-1+ emmc_mmc.write_bl_len) / emmc_mmc.write_bl_len;

	for(i=0;i<block_cnt;i++)
	{
		sdio_debug("r - emmc_add=%d  src_buffer=0x%x \n", start_blk+i, buffer_addr);
		if (mmc_bread(&emmc_mmc,start_blk+i, 1, (u32)buffer_addr) != 1)
		{
			debug("mmc_bread failed.\n");
			return -1;
		}
		buffer_addr += emmc_mmc.read_bl_len;
	}
	
	return 0;
}
*/

int emmc_write(u32 start_blk, u32 len, void * src_buffer)
{
	u32 i=0,j=1;
	u32 block_cnt=0,col=0;
	
	char buffer_w[MMC_MAX_BLOCK_LEN];
	memset(buffer_w,0x00,MMC_MAX_BLOCK_LEN);
			
	block_cnt = len / emmc_mmc.write_bl_len;
	col = len % emmc_mmc.write_bl_len;
	for(i=0;i<block_cnt;i++)
	{
		sdio_debug("w - %d  0x%x \n", start_blk+i, src_buffer);
		if (mmc_bwrite(&emmc_mmc, start_blk+i, 1, src_buffer) != 1)
		{
			debug("mmc_bwrite failed.\n");
			return -1;
		}
		src_buffer += emmc_mmc.write_bl_len;
		
		if(j%4 == 0)
			debug(".");
		if(j++%128 == 0)
			debug("\n");
	}
	if(col)
	{
		sdio_debug("wc - %d  0x%x \n", start_blk+block_cnt, src_buffer);
		memcpy((void*)buffer_w,(void*)src_buffer,col);
		if (mmc_bwrite(&emmc_mmc, start_blk+block_cnt, 1, (void*)buffer_w) != 1)
		{
			debug("mmc_bwrite failed.\n");
			return -1;
		}		
		debug(".");
	}	
	debug("\n");
		
	return 0;
}

int emmc_erase(u32 start_blk, u32 len)
{
	u32 i=0, block_cnt=0,j=1;
			
	block_cnt = len / emmc_mmc.write_bl_len;
	for(i=0;i<block_cnt;i++)
	{
		if (mmc_berase(&emmc_mmc, start_blk+i, 1) != 1)
		{
			debug("mmc_berase failed.\n");
			return -1;
		}
		if(j%4 == 0)
			debug(".");
		if(j++%128 == 0)
			debug("\n");
	}
	debug("\n");

	return 0;
}
#endif
