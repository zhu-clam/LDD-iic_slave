
#include <stdio.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "24cxx.h"
#include "common.h"

#define ENV_DEV		"EEPROG_DEV"
#define ENV_I2C_ADDR	"EEPROG_I2C_ADDR"

int g_quiet;


#define MAX_DATA_LEN		256

struct e2p_opt {
	unsigned int opr;
	int i2c_num;
	int i2c_addr;
	int mem_addr;
	unsigned int tms;
	unsigned char data[MAX_DATA_LEN];
	char file[FILE_NAME_LEN];
	char i2c_dev[FILE_NAME_LEN];
	unsigned int len;
	int e2p_type;
	int e2p_pgs;
	int e2p_blk;
	FILE *fp;
};
enum e2p_opr {
	OPR_READ = 1,
	OPR_WRITE,
	OPR_TEST,
};

static void usage(char *app)
{
	printf("\n\tE2PROM Debug tool\n");
	printf("Usage: \n");
	printf("%s [options]\n", app);
	printf("options:\n");
	printf(" -r - Read form e2p\n");
	printf(" -w - Write to e2p\n");
	printf(" -l - Write/Read data length\n");
	printf(" -i - i2c bus number, '-i 1' means /dev/i2c-1\n");
	printf(" -d - I2C device physical address, in hex, default: 0x50\n");
	printf(" -m - Offset in e2p, in hex\n");
	printf(" -t - Repeat times, default: 1\n");
	printf(" -a - Address width 8/16bits, default: 16bits, relevant to chip size\n");
	printf(" -p - Pagesize, default: 16\n");
	printf(" -s - Write mode only, follow write data, at end of command line\n");
	printf(" -f - Filename, Read mode, read file to e2p\n");
	printf("                Write mode, read e2p to file\n");
	printf(" -A - Automatic test\n");
	printf(" -q - Quite\n");
}

#define check_next_argv do {\
		i++;	\
		if(i == argc || argv[i][0] == '-') { \
			i--;	\
			break;	\
		}	\
	}while(0)

static int analysis_options(char **argv, char argc, struct e2p_opt *opt)
{
	int i, j;
	opt->i2c_num  = -1;
	opt->i2c_addr = -1;
	opt->mem_addr = -1;
	opt->e2p_pgs  = 16;
	opt->e2p_type = EEPROM_TYPE_16BIT_ADDR;
	for(i = 1; i < argc; i++) {
		switch(argv[i][0] << 8 | argv[i][1]) {
		case ('-' << 8 | 'r'):
			opt->opr = OPR_READ;
			break;
		case '-' << 8 | 'w' :
			opt->opr = OPR_WRITE;
			break;
		case '-' << 8 | 'l' :
			check_next_argv;
			opt->len = int_str_to_int(argv[i]);
			break;
		case '-' << 8 | 'i' :
			check_next_argv;
			opt->i2c_num = int_str_to_int(argv[i]);
			break;
		case '-' << 8 | 'd' :
			check_next_argv;
			opt->i2c_addr = hex_str_to_int(argv[i]);
			break;
		case '-' << 8 | 'p' : 
			check_next_argv;
			opt->e2p_pgs = int_str_to_int(argv[i]);
			break;
		case '-' << 8 | 'a' :
			check_next_argv;
			if(8 == int_str_to_int(argv[i]))
				opt->e2p_type = EEPROM_TYPE_8BIT_ADDR;
			else
				opt->e2p_type = EEPROM_TYPE_16BIT_ADDR;
			break;
		case '-' << 8 | 'm' :
			check_next_argv;
			opt->mem_addr= hex_str_to_int(argv[i]);
			break;
		case '-' << 8 | 't' :
			check_next_argv;
			opt->tms = int_str_to_int(argv[i]);
			break;
		case '-' << 8 | 's' :
			for(j = 0; i < argc && j < MAX_DATA_LEN; j++) {
				i++;
				if(i == argc)
					break;
				if(argv[i][0] == '-') {
					i--;
					break;
				}
				opt->data[j] = hex_str_to_int(argv[i]);
			}
			break;
		case '-' << 8 | 'f' :
			check_next_argv;
			strncpy(opt->file, argv[i], FILE_NAME_LEN);
			break;
		case '-' << 8 | 'A' :
			opt->opr = OPR_TEST;
			break;
		case '-' << 8 | 'q' :
			msg_level = PLV_WARING;
			break;
		default:
			printf("Invalid Test Parameters:%s\n", argv[i]);
			return -1;
		}
	}
	if(opt->i2c_num == -1) {
		printf("Please Specify I2C Bus Number\n");
		return -1;
	} else {
		sprintf(opt->i2c_dev, "/dev/i2c-%d", opt->i2c_num);
	}

	if(opt->i2c_addr == -1) {
		opt->i2c_addr = DEFAULT_EEPROM_ADDR;
	}

	if(opt->len == 0) {
		printf("Please Specify length, e.g: -l 10\n");
		return -1;
	}
	if(opt->e2p_blk > 8) {
		printf("U Specify a Wrong Block Number(%d)\n", opt->e2p_blk);
		return -1;
	}

	if(opt->tms == 0)
		opt->tms = 1;

	if(opt->opr == OPR_TEST) {
		opt->mem_addr = 0;
		return 0;
	}

	if(opt->mem_addr == -1) {
		printf("Please Specify Memory Offset\n");
		return -1;
	}
	if(opt->len == 0) {
		printf("Please Specify length, e.g: -l 10\n");
		return -1;
	}

	if(opt->e2p_pgs % 16) {
		printf("Page Size is Integer Multiple of 16。\n");
		return -1;
	}

	return 0;
}



#ifdef  BUILD_TEST_LIB
int e2p_main(int argc, char **argv)
#else
int main(int argc, char** argv)
#endif
{
#define BUFFER_LEN 32768
	struct eeprom e;
	int i, ret;
	struct e2p_opt opt;
	unsigned char ee_buf[BUFFER_LEN];
	unsigned char ee_buf_in[BUFFER_LEN];

	memset(&opt, 0, sizeof(struct e2p_opt));

	if(analysis_options(argv, argc, &opt)) {
		usage(argv[0]);
		return -1;
	}

	if(eeprom_open(opt.i2c_dev,  opt.i2c_addr, 
			opt.e2p_type, &e, opt.e2p_pgs, EE_BANK0) < 0) {
		printf("Open I2C Device %s Fail\n", opt.i2c_dev);
		return -1;
	}
	switch(opt.opr) {
	case OPR_READ:
		msg_print(PLV_INFO, 
			"From 0x%x Read %d Byte:\n", 
			opt.mem_addr, opt.len);
		ret = i2c_read_eeprom_block(&e, opt.mem_addr, ee_buf, opt.len);
		if(ret != opt.len) {
			printf("Read Fail : %d\n", ret);
			goto i2c_error;
		}
		for ( i = 0; i < opt.len; i++)
		{
			msg_print(PLV_INFO, "%2x ", ee_buf[i]);
		}
		printf("\n\n");
		break;
	case OPR_WRITE:
		msg_print(PLV_INFO, 
			  "Write %d Byte to 0x%x\n", 
			  opt.len, opt.mem_addr);
		ret = i2c_write_eeprom_block(&e, opt.mem_addr, opt.data, opt.len);
		if(ret != opt.len) { 
			printf("Write Fail :%d\n", ret);
			goto i2c_error;
		}
		break;
	case OPR_TEST:
		while(opt.tms--) {
			// BUG: if tms = 1, then always success. ??
			srand(opt.tms);
			for(i = 0; i < opt.len; i++) {
				ee_buf[i] = rand();
			}
			i2c_write_eeprom_block(&e, 
						opt.mem_addr, 
						ee_buf, 
						opt.len);
			sleep(1);
			i2c_read_eeprom_block(&e, 
						opt.mem_addr, 
						ee_buf_in, 
						opt.len);
			if(memcmp(ee_buf, ee_buf_in, opt.len)) {
				msg_print(PLV_INFO, "E2PPROM TESRT	[Failure]\n");
				goto i2c_error;
			}
			memset(ee_buf_in, 0, BUFFER_LEN);
		}
		msg_print(PLV_INFO, "E2PPROM TEST	[Success]\n");
		break;
	default:
		printf("Invalid Opration\n");
	}
	eeprom_close(&e);
	return 0;
i2c_error :
	eeprom_close(&e);
	return -1;
}
