/* vdd interface testing */

#include "errno.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"

#include "los_task.h"
#include "string.h"

#include "ipcm_funcs.h"

//#define IPCM_TIME_COUNT

char __message0[32] = "hello world!\n";
char __message1[32] = "abcdefg";
char __message2[128] = "aaaaaaaaaaaabbbbbbbbbbb101";
char __message3[256] = "aaaaaaaaaaaabbbbbbbbbbb101";
char __message4[256] = "abcdefghijklmnopqrstuvwxyz?!~";
char __message5[256] = "zyxwvutsrqponmlkjihgfedcba....";
char __message6[256] = "______________________________";

struct __message {
	int target;
	int port;
	int flag;
#define FLAG_SENDING  0x01
#define FLAG_ACKING   0x02
	unsigned long long time;
	unsigned int count;
	int len;
	void *buf;
	void *priv;
};

struct ipcm_vdd_opt g_opt;

#define MAX_MSG_SEND_COUNT    5000

int msg_acked(void *handle, void *buf, unsigned int len)
{
	struct __message *pmsg = buf;
#ifdef IPCM_TIME_COUNT
	struct timeval tv0;
#endif
	unsigned long time;

	if (FLAG_SENDING == pmsg->flag) {
		pmsg->flag = FLAG_ACKING;
		ipcm_vdd_sendmsg(handle,
				pmsg,
				sizeof(struct __message) + pmsg->len);
	} else {
#ifdef IPCM_TIME_COUNT
		gettimeofday(&tv0, NULL);
		time = tv0.tv_sec*1000000 + tv0.tv_usec;
		printf("[%d:%d]msg cnt: %d, cost: %lu us\n",
				pmsg->target,
				pmsg->port,
				pmsg->count,
				time - pmsg->time);
#else
		printf("[%d:%d]msg cnt: %d\n",
				pmsg->target,
				pmsg->port,
				pmsg->count);
#endif
	}
	return 0;
}

void g_opt_init(void)
{
	g_opt.recv = &msg_acked;
	g_opt.data = 0;
}

void msg_send_fn(void *data)
{
	struct __message *pmsg = data;
	int target = pmsg->target;
	int port = pmsg->port;
	struct ipcm_transfer_handle *handle;
	int ret;
	int countii = MAX_MSG_SEND_COUNT;
#ifdef IPCM_TIME_COUNT
	struct timeval tv0;
#endif
	printf("++ %s %d, [%d:%d]\n", __func__, __LINE__, target, port);
	/* wait until node ready */
	while (!ipcm_node_ready(target)) {
		LOS_Msleep(400);
	}

	printf("++ %s %d nid %d:%d ready \n",
			__func__, __LINE__, target, port);

	handle = ipcm_vdd_open(target, port, __HANDLE_MSG_NORMAL);
	ipcm_vdd_setopt(handle, &g_opt);
	ipcm_vdd_connect(handle, CONNECT_BLOCK);
	printf("++ %s %d\n", __func__, __LINE__);
	while (countii) {
#ifdef IPCM_TIME_COUNT
		gettimeofday(&tv0, NULL);
		pmsg->time = tv0.tv_sec*1000000 + tv0.tv_usec;
#endif
		pmsg->count++;
		pmsg->flag = FLAG_SENDING;
		ret = ipcm_vdd_sendmsg(handle,
				pmsg,
				sizeof(struct __message) + pmsg->len);
		if (ret != pmsg->len + sizeof(struct __message)) {
			printf("msg to dev[%d:%d] failed\n",
					target, port);
			break;
		}
		LOS_Msleep(100);
	}

	printf("++ %s %d, [%d:%d]\n", __func__, __LINE__, target, port);
	free(pmsg);

	ipcm_vdd_disconnect(handle);
	ipcm_vdd_setopt(handle, NULL);
	ipcm_vdd_close(handle);
	printf("++ %s %d, [%d:%d]\n", __func__, __LINE__, target, port);
}

void start_message_com(int target, int port, void *buf, int sz)
{
	struct __message *msg;
	TSK_INIT_PARAM_S los_task;
	int ret;
	int id;

	msg = malloc(sizeof(struct __message));
	if (NULL == msg) {
		printf("fn %s %d malloc for message[%d:%d] failed\n",
				__func__, __LINE__, target, port);
		return;
	}

	memset(&los_task, 0, sizeof(TSK_INIT_PARAM_S));
	msg->target = target;
	msg->port = port;
	msg->buf = buf;
	msg->count = 0;
	msg->len = sz;

	los_task.pfnTaskEntry = (TSK_ENTRY_FUNC)msg_send_fn;
	los_task.auwArgs[0] = (unsigned int)msg;
	los_task.uwStackSize = 0x500;
	los_task.pcName = "msgsend";
	los_task.uwResved = LOS_TASK_STATUS_DETACHED;
	los_task.usTaskPrio = 5;
	ret = LOS_TaskCreate(&id, &los_task);
	if (LOS_OK != ret) {
		printf("[%d:%d]create task failed\n",
				target, port);
		free(msg);
		return;
	}
}

void test_message_sending(void)
{
	g_opt_init();

	//start_message_com(0, 998, __message0, strlen(__message0));
	//start_message_com(0, 3, __message1, strlen(__message1));
	//start_message_com(0, 10, __message2, strlen(__message2));

	start_message_com(1, 3, __message1, strlen(__message1));
	//start_message_com(1, 4, __message3, strlen(__message3));
	//start_message_com(1, 5, __message5, strlen(__message5));
	//start_message_com(1, 8, __message4, 24 * 4);
	start_message_com(1, 10, __message2, strlen(__message2));
	start_message_com(1, 110, __message2, 16 * 4);
	start_message_com(1, 998, __message0, strlen(__message0));

	start_message_com(0, 3, __message1, strlen(__message1));
	//start_message_com(0, 4, __message3, strlen(__message3));
	//start_message_com(0, 5, __message5, strlen(__message5));
	//start_message_com(0, 8, __message4, 24 * 4);
	start_message_com(0, 10, __message2, strlen(__message2));
	start_message_com(0, 110, __message2, 16 * 4);
	start_message_com(0, 998, __message0, strlen(__message0));

	//start_message_com(1, 1, __message0, strlen(__message0));
	//start_message_com(1, 2, __message0, strlen(__message0));
	//start_message_com(1, 4, __message6, strlen(__message6));
	//start_message_com(1, 6, __message6, strlen(__message6));
	//start_message_com(1, 200, __message4, 32 * 4);
	//start_message_com(1, 201, __message4, 28 * 4);
	//start_message_com(1, 511, __message2, 20 * 4);

}

void stop_message_com(void)
{
}
