#include <stdio.h>
#include <stdlib.h>
#include "defineall.h"
#include "testdata.h"

// ????handle?ṹ??

int main()
{
	
	printf("hello world!\n");
	struct handle handle1;
	handle1.type = 7;
	handle1.sampleRate = 0;
	handle1.bitRate = 64000;
	handle1.bitsPerSample = 1;
	handle1.numOfChannels = 1;
	handle1.amu = 2;
	/* 如果输入的为编码数据,open函数则会执行解码，如果输入的为PCM数据,open函数则会执行编码    */
	handle1.PCM = inputwavg711; /* 编解码前数据存放的地址 */
	
	handle1.numofPCM = sizeof(inputwavg711)/sizeof(float); /* PCM数据的大小 */
	handle1.numofCode = sizeof(inputcodeg711)/sizeof(unsigned char); /* 编解码后的数据大小 */
	handle1.Code = inputcodeg711; /* 编解码后数据存放的地址 */

	//ִ??open
	handle1 = Open(handle1);
	
	Close(handle1);
	return 0;
}


