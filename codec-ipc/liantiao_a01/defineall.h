struct handle
{
	short type;
	long sampleRate;
	short bitsPerSample;
	short numOfChannels;
	float *PCM;
	unsigned char *Code;
	long numofPCM;
	long numofCode;
	long bitRate;
	short amu;
};
//open
struct handle Open(struct handle openhandle);
//close
void Close(struct handle closehandle);
//G711
int G711encode(struct handle g711encodeinfo);
int G711decode(struct handle g711decodeinfo);

