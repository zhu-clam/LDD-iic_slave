struct handleAPI
{
	short type;
	long sampleRate;
	short bitsPerSample;
	short numOfChannels;
	void *input;
	void *output;
	long numofinput;
	long numofoutput;
	long bitRate;
	short amu;
};
struct handle
{
	short type;
	long sampleRate;
	short bitsPerSample;
	short numOfChannels;
	float *PCM;
	unsigned char*Code;
	long numofPCM;
	long numofCode;
	long bitRate;
	short amu;
};
//open
struct handleAPI Open(struct handleAPI openhandle);
//close
void Close(struct handleAPI closehandle);
//G711
int G711encode(struct handle g711encodeinfo);
int G711decode(struct handle g711decodeinfo);
//G729
int G729encode(struct handle g729encodeinfo);
int G729decode(struct handle g729decodeinfo);
//AVS2
int Avs2encode(struct handle avs2encodeinfo);
int Avs2decode(struct handle avs2endodeinfo);


