/* vdd interface testing */
#include <linux/kthread.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/ktime.h>

#include "ipcm_funcs.h"
#include "ipcm_osadapt.h"

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
	unsigned long long  time;
	unsigned int count;
	int len;
	void *buf;
	void *priv;
};

struct ipcm_vdd_opt g_opt;
int cpu_type = 0;
#define CPU_TYPE_A53 0x01
#define CPU_TYPE_A73 0x02

#define MAX_MSG_SEND_COUNT    30

int msg_acked(void *handle, void *buf, unsigned int len)
{
	struct __message *pmsg = buf;
	struct timeval tv0;
	unsigned long long time;

	if (FLAG_SENDING == pmsg->flag) {
		pmsg->flag = FLAG_ACKING;
		ipcm_vdd_sendmsg(handle,
				pmsg,
				sizeof(struct __message) + pmsg->len);
	} else {
		do_gettimeofday(&tv0);
		time = tv0.tv_sec*1000000 + tv0.tv_usec;
		printk("[%d:%d]msg cnt: %d, cost: %llu us\n",
				pmsg->target,
				pmsg->port,
				pmsg->count,
				time - pmsg->time);
	}
	return 0;
}

void g_opt_init(void)
{
	g_opt.recv = &msg_acked;
	g_opt.data = 0;
}

int msg_send_fn(void *data)
{
	struct __message *pmsg = data;
	int target = pmsg->target;
	int port = pmsg->port;
	struct ipcm_transfer_handle *handle;
	int ret = 0;
	int countti = MAX_MSG_SEND_COUNT;
	struct timeval tv0, tv1;

	printk("++ %s %d, [%d:%d]\n", __func__, __LINE__, target, port);

	/* wait until node ready */
	while (!ipcm_node_ready(target)) {
		msleep(400);
		if (kthread_should_stop()) {
			printk("++ %s %d, [%d:%d]\n", __func__, __LINE__, target, port);
			return ret;
		}
	}
	printk("++ %s %d, [%d:%d]\n", __func__, __LINE__, target, port);

	handle = ipcm_vdd_open(target, port, __HANDLE_MSG_PRIORITY);
	ipcm_vdd_setopt(handle, &g_opt);
	do_gettimeofday(&tv0);
	ipcm_vdd_connect(handle, CONNECT_BLOCK);
	do_gettimeofday(&tv1);
	printk("++[%s:%d],connect:%ld\n", __func__, __LINE__,
			(tv1.tv_sec*1000000 - tv0.tv_sec*1000000)
			+ (tv1.tv_usec -tv0.tv_usec));
	printk("++ %s %d, [%d:%d]\n", __func__, __LINE__, target, port);

	while (!kthread_should_stop()&&(countti)) {
		do_gettimeofday(&tv0);
		pmsg->time = tv0.tv_sec*1000000 + tv0.tv_usec;
		pmsg->count++;
		pmsg->flag = FLAG_SENDING;
		ret = ipcm_vdd_sendmsg(handle,
				pmsg,
				sizeof(struct __message) + pmsg->len);	
		if (ret != pmsg->len + sizeof(struct __message)) {
			printk(KERN_ERR "msg to dev[%d:%d] failed\n",
					target, port);
			break;
		}
		msleep(500);
	}

	kfree(pmsg);

	ipcm_vdd_disconnect(handle);
	ipcm_vdd_setopt(handle, NULL);
	ipcm_vdd_close(handle);

	printk("++ %s %d, [%d:%d]\n", __func__, __LINE__, target, port);
	return ret;
}

void start_message_com(int target, int port, void *buf, int sz)
{
	struct __message *msg;
	struct task_struct *task;

	msg = kmalloc(sizeof(struct __message), GFP_ATOMIC);
	if (NULL == msg) {
		printk(KERN_ERR "fn %s %d malloc for message[%d:%d] failed\n",
				__func__, __LINE__, target, port);
		return;
	}
	msg->target = target;
	msg->port = port;
	msg->buf = buf;
	msg->count = 0;
	msg->len = sz;
	task = kthread_run(msg_send_fn, msg, "send_fn");
	if (NULL == task) {
		printk(KERN_ERR "[%d:%d]kthread create failed!\n",
				target, port);
		kfree(msg);
		return;
	}
}

#include "device_config.h"

void test_message_sending(void)
{
	char cpu_type_buf[0x08];
	memcpy(cpu_type_buf, CPU_TYPE, 3);
	if ((strncmp(cpu_type_buf, "a53", 3) == 0)) {
		cpu_type = CPU_TYPE_A53;
	} else if ((strncmp(cpu_type_buf, "a73", 3) == 0)) {
		cpu_type = CPU_TYPE_A73;
	} else {
		ipcm_err("unknow cpu type!");
		return;
	}
	g_opt_init();
	if (CPU_TYPE_A53 == cpu_type) {
		start_message_com(1, 0, __message0, strlen(__message0));
		start_message_com(1, 1, __message1, strlen(__message1));

		start_message_com(1, 510, __message2, strlen(__message2));
		start_message_com(1, 511, __message3, strlen(__message3));
		//start_message_com(1, 1023, __message0, strlen(__message0));
		//start_message_com(1, 1022, __message1, strlen(__message1));
	}
	else {
		start_message_com(0, 0, __message0, strlen(__message0));
		start_message_com(0, 1, __message1, strlen(__message1));

		start_message_com(0, 510, __message0, strlen(__message0));
		start_message_com(0, 511, __message1, strlen(__message1));
		//start_message_com(0, 1023, __message0, strlen(__message0));
		//start_message_com(0, 1022, __message1, strlen(__message1));
	}

	//start_message_com(2, 3, __message1, strlen(__message1));
	//start_message_com(2, 4, __message3, strlen(__message3));
	//start_message_com(2, 5, __message5, strlen(__message5));
	//start_message_com(2, 8, __message4, 24 * 4);
	//start_message_com(2, 10, __message2, strlen(__message2));
	//start_message_com(2, 110, __message2, 16 * 4);
	//start_message_com(2, 998, __message0, strlen(__message0));


	//start_message_com(2, 1, __message0, strlen(__message0));
	//start_message_com(2, 2, __message0, strlen(__message0));
	//start_message_com(2, 4, __message6, strlen(__message6));
	//start_message_com(2, 6, __message6, strlen(__message6));
	//start_message_com(2, 200, __message4, 32 * 4);
	//start_message_com(2, 201, __message4, 28 * 4);
	//start_message_com(2, 511, __message2, 20 * 4);
}

void stop_message_com(void)
{
}

