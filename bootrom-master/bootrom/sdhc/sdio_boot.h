/*
 * (Copyright 2017) Verisilicon Inc. All Rights Reserved
 * Company confidential and Proprietary information.
 * This information may not be disclosed to unauthorized individual.
 * 
 * System Application Team <fengyin.wu@verisilicon.com>
 *
 */

#include "mmc.h"

int sd_card_init(void);
int sd_card_boot(void);
int sd_card_erase(u32 start_blk, u32 len);
int sd_card_write(u32 start_blk, u32 len, void * src_buff);

int emmc_init(void);
int emmc_boot(void);
int emmc_erase(u32 start_blk, u32 len);
int emmc_write(u32 start_blk, u32 len, void * src_buff);
