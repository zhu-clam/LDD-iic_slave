#include "gs2971.h"
#include "ck810.h"
#include "datatype.h"




int gs2971_read_register( unsigned short offset, unsigned short * value)
{	
	int status;
	CK_UINT8 dst_data[2];
	spi_gs2971_read_byte(dst_data, 2,offset, 0);
        *value  = dst_data[1];
        *value  = (*value<< 8 ) | dst_data[0];
	return status;
}

int gs2971_write_register(unsigned short offset, unsigned short value)
{
        CK_UINT8 dst_mem[2];
        dst_mem[1] = value & 0xFF;
        dst_mem[0] = (value >> 8) & 0xFF;
	spi_gs2971_write_byte(dst_mem, 2,offset,0);
	return 0;
}


static int get_std(void)
{
	
	unsigned short std_lock_value;
	unsigned short sync_lock_value;
	unsigned short words_per_actline_value;
	unsigned short words_per_line_value;
	unsigned short lines_per_frame_value;
	unsigned short actlines_per_frame_value;
	unsigned short interlaced_flag;
	int status;
	unsigned short ds1, ds2;

	printf("-> In function %s\n", __func__);

	

	status = gs2971_read_register(GS2971_RASTER_STRUCT4, &std_lock_value);
	actlines_per_frame_value = std_lock_value & GS_RS4_ACTLINES_PER_FIELD;
	interlaced_flag = std_lock_value & GS_RS4_INT_nPROG;

	status = gs2971_read_register(GS2971_FLYWHEEL_STATUS, &sync_lock_value);
	if (status)
		return status;
	printf("--> lock_value %x\n", sync_lock_value);
	if (!sync_lock_value) {
		printf("%s: no lock, gs2971\n", __func__);
		return 0;
	}

	status = gs2971_read_register(GS2971_DATA_FORMAT_DS1, &ds1);
	if (status)
		return status;
	status = gs2971_read_register(GS2971_DATA_FORMAT_DS2, &ds2);
	if (status)
		return status;
	printf("--> ds1=%x\n--> ds2=%x\n", ds1, ds2);

	status =
	    gs2971_read_register(GS2971_RASTER_STRUCT1,
				 &words_per_actline_value);
	if (status)
		return status;
	words_per_actline_value &= GS_RS1_WORDS_PER_ACTLINE;

	status =
	    gs2971_read_register(GS2971_RASTER_STRUCT2,
				 &words_per_line_value);
	if (status)
		return status;
	words_per_line_value &= GS_RS2_WORDS_PER_LINE;

	status =
	    gs2971_read_register(GS2971_RASTER_STRUCT3,
				 &lines_per_frame_value);
	if (status)
		return status;
	lines_per_frame_value &= GS_RS3_LINES_PER_FRAME;

	printf("--> Words per line %u/%u Lines per frame %u/%u\n",
		 (unsigned int)words_per_actline_value,
		 (unsigned int)words_per_line_value,
		 (unsigned int)actlines_per_frame_value,
		 (unsigned int)lines_per_frame_value);
	printf("--> SyncLock: %s %s StdLock: 0x%04x\n",
		 (sync_lock_value & GS_FLY_V_LOCK_DS1) ? "Vsync" : "NoVsync",
		 (sync_lock_value & GS_FLY_H_LOCK_DS1) ? "Hsync" : "NoHsync",
		 (unsigned int)(std_lock_value));


	

	if (!lines_per_frame_value) {
		printf("%s: 0 frame size\n", __func__);
		return -1;
	}

	
	if (interlaced_flag && lines_per_frame_value == 525) {
		printf("--> V4L2_STD_525_60\n");
		
	} else if (interlaced_flag && lines_per_frame_value == 625) {
		printf("--> V4L2_STD_625_50\n");
		
	} else if (interlaced_flag && lines_per_frame_value == 525) {
		printf("--> V4L2_STD_525P_60\n");
		
	} else if (interlaced_flag && lines_per_frame_value == 625) {
		printf("--> V4L2_STD_625P_50\n");
		
	} else if (!interlaced_flag && 749 <= lines_per_frame_value
		   && lines_per_frame_value <= 750) {
		if (words_per_line_value > 1650) {
			printf("--> V4L2_STD_720P_50\n");
			
		} else {
			printf("--> V4L2_STD_720P_60\n");
			
			
		}
	} else if (!interlaced_flag && 1124 <= lines_per_frame_value
		   && lines_per_frame_value <= 1125) {
		

		if (words_per_line_value >= 2200 + 550) {
			printf("--> V4L2_STD_1080P_24\n");
			
		} else if (words_per_line_value >= 2200 + 440) {
			printf("--> V4L2_STD_1080P_25\n");
			
		} else {
			printf("--> V4L2_STD_1080P_60\n");
			
		}
	} else if (interlaced_flag && 1124 <= lines_per_frame_value
		   && lines_per_frame_value <= 1125) {

		
		if (words_per_line_value >= 2200 + 440) {
			printf("--> V4L2_STD_1080I_50\n");
			
		} else {
			printf("--> V4L2_STD_1080I_60\n");
			
		}
	} else {
		printf("Std detection failed: interlaced_flag: %u words per line %u/%u Lines per frame %u/%u SyncLock: %s %s StdLock: 0x%04x\n",
			(unsigned int)interlaced_flag,
			(unsigned int)words_per_actline_value,
			(unsigned int)words_per_line_value,
			(unsigned int)actlines_per_frame_value,
			(unsigned int)lines_per_frame_value,
			(sync_lock_value & GS_FLY_V_LOCK_DS1) ? "Vsync" :
			"NoVsync",
			(sync_lock_value & GS_FLY_H_LOCK_DS1) ? "Hsync" :
			"NoHsync", (unsigned int)(std_lock_value));
		return -1;
	}

	

	status =
	    gs2971_write_register(GS2971_ANC_CONTROL, ANCCTL_ANC_DATA_DEL);
	if (status)
		return status;
	printf("--> remove anc data\n");

	return 0;
}




/* gs2971_initialize :
 * This function will set the video format standard
 */
static int gs2971_initialize(void)
{
	int status = 0;
	int retry = 0;
	unsigned short value;

	unsigned short cfg = GS_VCFG1_861_PIN_DISABLE_MASK;

	
	cfg |= GS_VCFG1_TIMING_861_MASK;

	printf("-> In function %s\n", __func__);

	for (;;) {
		status = gs2971_write_register(GS2971_VCFG1, cfg);
		if (status)
			return status;

		status = gs2971_read_register(GS2971_VCFG1, &value);
		if (status)
			return status;
		if (value == cfg)
			break;
		printf("status=%x, read value of 0x%04x, expected 0x%04x\n", status,(unsigned int)value, cfg);
		if (retry++ >= 20)
			return -1;
		udelay(50 * 1000);
	}

	status = gs2971_write_register(GS2971_VCFG2, 0);
	if (status)
		return status;

	status = gs2971_write_register(GS2971_IO_CONFIG,(GS_IOCFG_HSYNC << 0) | (GS_IOCFG_VSYNC<< 5) |(GS_IOCFG_DE << 10));
	if (status)
		return status;

	status = gs2971_write_register(GS2971_IO_CONFIG2,(GS_IOCFG_LOCKED << 0) | (GS_IOCFG_Y_ANC<< 5) |(GS_IOCFG_DATA_ERROR << 10));
	if (status)
		return status;

	status =
	    gs2971_write_register(GS2971_TIM_861_CFG, GS_TIMCFG_TRS_861);
	if (status)
		return status;

}




static int gs2971_probe(void)
{
	int ret = 0;
	printf("-> In function %s\n", __func__);
        spi_init(0);
	ret = gs2971_initialize();
        get_std();
        printf("-> Out function %s(0x%x)\n", __func__,ret);
	return ret;

}



