#include <stdio.h>
#include <stdlib.h>
#include "defineall.h"
#include "testdata.h"

// 声明handle结构体





int main()
{
	//编码：
#if 0
	struct handleAPI handle1;
	handle1.type = 1;
	handle1.sampleRate = 8000;
	handle1.bitRate = 32000;  //目标码率
	handle1.bitsPerSample = 1;
	handle1.numOfChannels = 1;
	handle1.amu = 2;
	handle1.input = inputwavg711;           //输入PCM地址
	handle1.numofinput = sizeof(inputwavg711) / sizeof(float); //PCM数据量
	handle1.numofoutput = 0;  //
	handle1.output = NULL;    //
	//解码：
#else 
	struct handleAPI handle1;
	handle1.type = 6;
	handle1.sampleRate = 8000;
	handle1.bitRate = 32000;
	handle1.bitsPerSample = 1;
	handle1.numOfChannels = 1;
	handle1.amu = 2;
	handle1.input = inputcodeavs2;           // 输入码流地址
	handle1.numofinput = sizeof(inputcodeavs2) / sizeof(unsigned char); //输入码流数据量
	handle1.numofoutput = 0;  //
	handle1.output = NULL;    //
#endif		
	//估计所需缓存区

	if (handle1.type < 5)
	{
		handle1.numofoutput = (long)((((float)handle1.numofinput / (float)handle1.sampleRate) * (float)handle1.bitRate * 1.5) / 8.0);
	}
	else
	{
		handle1.numofoutput = (long)(((float)handle1.numofinput * 8.0 / (float)handle1.bitRate) * (float)handle1.sampleRate * 1.5);
	}
	if (handle1.type < 5)
	{
		//根据numofCode开辟编码缓冲区
		handle1.output = (void*)malloc(sizeof(unsigned char) * handle1.numofoutput);
	}
	else
	{
		//根据numofPCM开辟解码缓冲区
		handle1.output = (void*)malloc(sizeof(float) * handle1.numofoutput);
	}
	printf("numofinput:%ld,numofoutput:%ld \n",handle1.numofinput,handle1.numofoutput);
	//执行open
	handle1 = Open(handle1);
	if (handle1.type < 5)
	{
		unsigned char *out = (unsigned char *)handle1.output;
		FILE *fp = fopen("avs2_out_test01.txt", "w");
		for (int i = 0; i < handle1.numofoutput; i++)
		{
			fprintf(fp, "0x");
			fprintf(fp, "%x", out[i]);
			fprintf(fp, "%s", ",");
		}
		fclose(fp);
	}
	else
	{
		float *out = (float *)handle1.output;
		FILE *fp = fopen("avs2_out_test03.txt", "w");
		for (int i = 0; i < handle1.numofoutput; i++)
		{
			
			fprintf(fp, "%f,", out[i]);
			
		}
		fclose(fp);
	}
	
	
	Close(handle1);
	return 0;
}


