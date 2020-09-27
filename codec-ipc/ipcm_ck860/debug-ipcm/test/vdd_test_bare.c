/* vdd interface testing */

#include "errno.h"
#include "stdio.h"
#include "stdlib.h"

#include "string.h"
#include "ipcm_nodes.h"
#include "ipcm_funcs.h"
#include "ipcm_osadapt.h"

char __message0[32] = "hello world!\n";
char __message1[32] = "abcdefg";
int __message2[32] = {0x87654321, 0x12345678, 0,};
char __message3[128] = "aaaaaaaaaaaabbbbbbbbbbb101";
int __message4[32] = {0x80808080, 0x60606060, 0x40404040, 0x20202020, 0xa0a0a0a0,};
char __message5[32] = "love";
char __message6[32] = "come on, BB";

struct ipcm_vdd_opt g_opt;

struct __message {
	int target;
	int port;
	void *buf;
	int len;
};

int msg_acked(void *handle, void *buf, unsigned int len)
{
	//printk("##msg acked:0x%x...len 0x%x\n", *(int*)buf);
	return 0;
}

void g_opt_init(void)
{
	g_opt.recv = &msg_acked;
	g_opt.data = 0;

}

#define TARGET  0
void start_message_com(int target, int port, void *buf, int sz);
volatile _Uncached int delay_loop = 100000;

struct ipcm_transfer_handle *g_handle[MAX_PORTS];

void msg_recv_fn(void *data)
{
	int ret;
	char *buf = malloc(4096);
	int source, port;
	int cnt = 0;
	int flags = 0;

	while (1) {
		//delay_loop = 100000;
		//printf("++ %s %d\n\r", __func__, __LINE__);
		//fflush(stdout);

		ret = ipcm_vdd_recvmsg(&source, &port, buf, 4096);
		if (ret < 0) {
			printf("recv err!!!\n");
			fflush(stdout);
			//break;
		}
		if (ret > 0) {
			cnt++;
			printf("# [%d:%d]message received,size %d,cnt %d\n\r",
					source, port, ret, cnt);
			fflush(stdout);
			ipcm_vdd_sendmsg(g_handle[port], buf, ret);
		}
		//while (delay_loop--);
		if (cnt == 10) {
			if (!flags) {
				flags = 1;
				start_message_com(TARGET, 3, __message0, strlen(__message0));
				start_message_com(TARGET, 4, __message0, strlen(__message0));
			}
		}
	}
	free(buf);

	for (port=0; port < MAX_PORTS; port++) {
		if (!g_handle[port])
			continue;
		ipcm_vdd_disconnect(g_handle[port]);
		ipcm_vdd_setopt(g_handle[port], NULL);
		ipcm_vdd_close(g_handle[port]);
		g_handle[port] = NULL;
	}
}

void start_message_com(int target, int port, void *buf, int sz)
{
	struct __message msg;
	msg.target = target;
	msg.port = port;

	printf("++ %s %d nid %d:%d waiting...\n\r",
			__func__, __LINE__, target, port);
	fflush(stdout);
	while (!ipcm_node_ready(target)) {
		delay_loop = 1000000;
		while (delay_loop--);
	}

	printf("++ %s %d nid %d:%d ready \n\r",
			__func__, __LINE__, target, port);
	fflush(stdout);

	g_handle[port] = ipcm_vdd_open(target, port, __HANDLE_MSG_NORMAL);
	ipcm_vdd_setopt(g_handle[port], &g_opt);
	/* must be NONBLOCK */
	ipcm_vdd_connect(g_handle[port], CONNECT_NONBLOCK);
	printf("++ %s %d nid %d:%d connect \n\r",
			__func__, __LINE__, target, port);

	//msg_recv_fn(&msg);
}

void stop_message_com(int target, int port)
{

}

void test_message_sending(void)
{
	g_opt_init();
	start_message_com(TARGET, 0, __message0, strlen(__message0));
	start_message_com(TARGET, 1, __message0, strlen(__message0));
	start_message_com(TARGET, 2, __message0, strlen(__message0));

	//start_message_com(0, 10, __message1, strlen(__message1));
	//start_message_com(0, 110, __message2, 16 * 4);
	//start_message_com(0, 4, __message3, strlen(__message3));
	//start_message_com(0, 5, __message5, strlen(__message5));
	//start_message_com(0, 8, __message4, 24 * 4);

	//start_message_com(1, 1, __message0, strlen(__message0));
	//start_message_com(1, 2, __message0, strlen(__message0));
	//start_message_com(1, 4, __message6, strlen(__message6));
	//start_message_com(1, 6, __message6, strlen(__message6));
	//start_message_com(1, 200, __message4, 32 * 4);
	//start_message_com(1, 201, __message4, 28 * 4);
	//start_message_com(1, 511, __message2, 20 * 4);

	msg_recv_fn(NULL);
}
