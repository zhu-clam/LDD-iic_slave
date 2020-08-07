#ifndef _COMMON_H_
#define _COMMON_H_

#include <sys/time.h>


#define PLV_DEBUG	7
#define PLV_INFO	6
#define PLV_WARING	5
#define PLV_RESULT	4
#define PLV_ERROR	3

#define DEFAULT_PRINT_LEVEL	6
char msg_level;

#define FILE_NAME_LEN		20
#define DEVICE_NAME_LEN		20
#define MAX_OPTION_LEN		10

unsigned int hex_str_to_int(const char *str);
unsigned int int_str_to_int(const char *str);
void lower2upper(char *str);
void clear_line(int len);
int read_file(char *filename, unsigned char *p_dat, int *size);
void write_file(char *filename, unsigned char *p_dat, int size);
void msg_print(char plevel, char *msg, ...);
unsigned short crc16(unsigned short crc, const unsigned char *buffer, unsigned int len);
double current_time();

#define _BIT(n)			(1 << (n))
#define MIN(x,y)		((x)<(y)?(x):(y))
#define MAX(x,y)		((x)>(y)?(x):(y))

#endif
