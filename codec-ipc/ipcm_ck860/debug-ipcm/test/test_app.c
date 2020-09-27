#ifndef __LITEOS__
/* for linux */
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <string.h>

#else
/* for liteos */
#include "pthread.h"
#include "stdlib.h"
#include "stdio.h"
#include "fcntl.h"
#include "string.h"
#include "time.h"
#include "unistd.h"
#include "malloc.h"
#include "sys/select.h"
#include "shell.h"
#include "shcmd.h"

#endif

#include "ipcm_userdev.h"
#define MAX_POUT_NUM   1023
#define MAX_TRAGET_NUM   7

#define MAX_SEND_LEN   40960

#define NONE          "\033[m"
#define LIGHT_RED     "\033[1;31m"
#define LIGHT_GREEN   "\033[1;32m"
#define LIGHT_BLUE    "\033[1;34m"

struct __message {
	pthread_t thread_id;
	int target;
	int port;
	int flag;
#define FLAG_SENDING   0x01
#define FLAG_RECVING   0x02
	unsigned long times;
	void *priv;

	void *buf;
	int len;
};
/*
* 消息的创建；利用IPCM driver提供的file operation，进行正式的消息创建。 
*
*/

static int create_message(struct __message *msg)
{
	int fd = 0;
	struct ipcm_handle_attr attr;
	int local_id;
	int devices_num = 0;
	unsigned int i;

	fd = open("/dev/ipcm", O_RDWR);//打开设备节点，创建一个port通道
	if (fd < 0) {
		printf("open /dev/ipcm fail. return:%d!\n", fd);
		return fd;
	}

	ioctl(fd, HI_IPCM_IOC_ATTR_INIT, &attr);//handler通道属性初始化

	local_id = ioctl(fd, HI_IPCM_IOC_GET_LOCAL_ID, &attr);//获取本地节点id
	printf("Local device id %d!\n", local_id);

	devices_num = ioctl(fd, HI_IPCM_IOC_GET_REMOTE_ID, &attr);//获取目的节点id
	printf("all devices number: %d\n", devices_num);
	for (i = 0; i < IPCM_MAX_DEV_NR; i++) {
		printf("dev[%d]:%d\n", i, attr.remote_ids[i]);
	}
/*在之前的parsing_args()函数，将命令行参数，赋值给msg*/
	attr.target = msg->target;
	attr.port = msg->port;
	attr.priority = HANDLE_MSG_PRIORITY;//在用户态即设置好handler通道的属性

	printf("connecting\n");
	if (ioctl(fd, HI_IPCM_IOC_CONNECT, &attr)) {//ioctl建立节点间连接
		printf("connect to [%d:%d] failed\n",
				attr.target, attr.port);
	}
	printf("connected\n");
	return fd;
}

static int destory_message(int fd)
{	
	ioctl(fd, HI_IPCM_IOC_DISCONNECT, NULL);
	close(fd);
	printf("disconnected\n");
	return 0;
}

static unsigned long get_current_time(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec*1000000 + tv.tv_usec);
}

/*
* 消息的发送
*/
static int message_send(struct __message *msg, int fd)
{
	int ret;
	fd_set rfds;//文件集，为select()函数配套使用
	struct timeval timeout;
	unsigned long time_0, time_1, time_2;//计时器
	unsigned long send_count = 0;//计数
	unsigned long count = 0;
	void *buf = NULL;
	int flag = (msg->times)? 1:0;

	buf = malloc(msg->len);
	if (NULL == buf) {
		printf("malloc for read buf failed\n");
		return -1;
	}

	time_0 = get_current_time();
	time_1 = time_0;
	while ((flag)? ((--msg->times) > 0):1) {//发送次数
		if ((count) &&(!(count%1000))) {
			time_2 = get_current_time();
			/* calculate the speed */
			printf("Speed:%lu msgs/10ms\n",
					(count)/((time_2 - time_1)/10000));
			time_1 = get_current_time();
			count = 0;
		}
		ret = write(fd, msg->buf, msg->len);/*NOTE:发送消息，调用接口write*/
		if (ret != msg->len) {
			printf("write error which return %d\n", ret);
			ret = -1;
			break;
		}
		send_count++;
		count++;

		timeout.tv_sec  = 2;
		timeout.tv_usec = 0;
		FD_ZERO(&rfds);	   //清除rfds 文件集.
		FD_SET(fd, &rfds);//将文件描述符fd加入到文件集rfds
		ret = select(fd + 1, &rfds, NULL, NULL, &timeout);//对fd进行可读性检查，并设置超时时间2s
		if (-1 == ret) {
			printf(LIGHT_RED"SELECT error\n"NONE);
			break;
		} else if (!ret) {//select返回值等于0，表示检查超时
			if (HANDLE_CONNECTED !=
					ioctl(fd, HI_IPCM_IOC_CHECK, NULL)) {
				printf("DISconnected by remote, exit\n");
				ret = 0;
				break;
			}
		}

		if (FD_ISSET(fd, &rfds)) {//若select函数返回已准备好的描述符数，在rfds范围内
			ret = read(fd, buf, msg->len);//则读取message
			if (ret < 0) {
				printf(LIGHT_RED"read ERR[%d]\n"NONE, ret);
				ret = -1;
				break;
			}
			printf("read mes:%s \n",buf);
			/*TODO: check the buf*/
		}
	}

	free(buf);

	time_2 = get_current_time();
	/* calculate the speed */
	printf("send %ld msgs, %ld bytes\n", send_count, send_count*msg->len);	
	printf("time %ld us\n", time_2 - time_0);
	printf("\n Speed:%lu msgs/10ms\n",
			(send_count)/((time_2 - time_0)/10000));
	if (!msg->times)
		return 0;
	return ret;
}

/*
* 消息的接收：同样也是利用ipcm driver 下open、read、write、ioctl、select等接口
*/

static int message_recv(int fd)
{
	int ret;
	ssize_t len;
	fd_set rfds;
	struct timeval timeout;
	unsigned long time_0, time_1;
	unsigned long recv_count = 0;
	unsigned long recv_bytes = 0;
	void *buf = NULL;

	buf = malloc(MAX_SEND_LEN);//40K
	if (NULL == buf) {
		printf("malloc for read buf failed\n");
		return -1;
	}
	time_0 = get_current_time();

	while (1) {
		timeout.tv_sec  = 2;
		timeout.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		ret = select(fd + 1, &rfds, NULL, NULL, &timeout);//对文件fd进行可读性检查
		if (-1 == ret) {
			printf(LIGHT_RED"SELECT error\n"NONE);
			break;
		} else if (!ret) {
			if (HANDLE_CONNECTED != 
					ioctl(fd, HI_IPCM_IOC_CHECK, NULL)) {
				printf("DISconnected by remote, exit\n");
				ret = 0;
				break;
			}
		}
		if (FD_ISSET(fd, &rfds)) {//若检查已可读
			len = read(fd, buf, MAX_SEND_LEN);//读取最大为MAX_SEND_LEN字节的数据到buf中
			if (len < 0) {
				printf(LIGHT_RED"read ERR[%d]\n"NONE, ret);
				ret = -1;
				break;
			}
			printf("[IPCM_TEST]read msg：%s \n",buf);	
			ret = write(fd, buf, len);//读取完之后，又写入到fd当中。
			if (ret != len) {
				printf("write error which return %d\n", ret);
				ret = -1;
				break;
			}
			recv_bytes += len;
			recv_count++;
		}
	}
	free(buf);

	time_1 = get_current_time();
	/* calculate the speed */
	printf("recv %ld msgs, %ld bytes\n", recv_count, recv_bytes);	
	printf("time %ld us\n", time_1 - time_0);	
	printf("\n Speed:%lu msgs/10ms\n",
			recv_count/((time_1 - time_0)/10000));
	return ret;
}


/*
* 创建子线程进行消息的发送、消息的接收。
*/
static void *pthread_fn(void *data)
{
	int ret = 0;
	int fd = 0;
	struct __message *msg = (struct __message *)data;

	printf("msg[%d:%d],%s thread is running.\n",
			msg->target,
			msg->port,
			(msg->flag == FLAG_SENDING)? "send":"recv");

	fd = create_message(msg);//创建消息、设置节点&handle状态，建立handle
	if (fd < 0) {
		goto exit;
	}

	printf("[IPMC_TEST] msg len:%d,msg is %s\n",msg->len,msg->buf);
	if (FLAG_SENDING == msg->flag)
		ret = message_send(msg, fd);//阻塞发送消息，并接收返回消息
	else
		ret = message_recv(fd);//用于接收非阻塞消息

	if (ret)
		printf(LIGHT_RED"message error\n"NONE);

	destory_message(fd);

exit:
	if (msg) {
		if (msg->buf)
			free(msg->buf);
		free(msg);
	}
	pthread_exit("IPCM VFS test finish\n");
	pthread_detach(msg->thread_id);

	return 0;
}

static void usage(void)
{
    printf("receive:\n");
    printf("./ipcm_app <target> <port>\n");

    printf("send:\n");
    printf("./ipcm_app <target> <port> <bytes> <times>&\n");

    printf("target: 0~6, assign a target\n");
    printf("port: 0~1023, assign a port number\n");
	printf("bytes: send length\n");
    printf("times: send times\n");
    printf("       0--infinite times,\n");
    printf("       num--defined times.\n");
}

/*
* main函数的参数解析；发送时需要一共5个参数，接收时需要一共3个参数
*/
static int parsing_args(struct __message *msg, int argc, char *argv[])
{
	/* send */
	if (5 == argc) {
		msg->target = strtol(argv[1], NULL, 10);//第一个参数代表目的节点，节点号对应CPU核.main函数参数从第0个开始
		if ((msg->target < 0) || (msg->target >= MAX_TRAGET_NUM)) {
			printf("target error which is %d.\n",
					msg->target);
			usage();
			return -1;
		}

		msg->port = strtol(argv[2], NULL, 10);//第二个参数代表端口号，端口号代表什么含义？字符型转long int型
		if ((msg->port < 0) || (msg->port >= MAX_POUT_NUM)) {
			printf("port error which is %d.\n",//猜测每个CPU节点，最多含有1024个端口，用于发送消息。
					msg->port);
			usage();
			return -1;
		}
		msg->len = strtol(argv[3], NULL, 10);//第三个参数代表传输消息的大小
		if (msg->len > MAX_SEND_LEN) {
			printf("send len should not be more than %d.\n",
					MAX_SEND_LEN);
			usage();
			return -1;
		}

		msg->buf = malloc(msg->len);//应用申请len大小的字节空间
		if (!msg->buf) {
			printf("malloc for msg fail.\n");
			return -1;
		}
		memset(msg->buf, 0xa5, msg->len);//初始化消息buf，长度为len， 值等于0xa5.用意：发送全是0xa5的消息

		msg->times = strtol(argv[4], NULL, 10);//消息传输的次数
		msg->flag  = FLAG_SENDING;//消息的标志

		printf("msg Len:%d, Times:%ld\n", msg->len, msg->times);
	}
	/* receive */
	else if (3 == argc) {
		msg->target = strtol(argv[1], NULL, 10);//要读取消息的目的节点
		if ((msg->target < 0) || (msg->target >= MAX_TRAGET_NUM)) {
			printf("target error which is %d.\n",
					msg->target);
			usage();
			return -1;
		}

		msg->port = strtol(argv[2], NULL, 10);//要读取消息的目的端口
		if ((msg->port < 0) || (msg->port >= MAX_POUT_NUM)) {
			printf("port error which is %d.\n",
					msg->port);
			usage();
			return -1;
		}
		msg->flag = FLAG_RECVING;
	} else {
		usage();
		return -1;
	}
	return 0;
}

#ifndef __LITEOS__
int main(int argc, char *argv[])
#else
int ipcm_main(int argc, char *argv[])
#endif
{
	int ret = 0;
	struct __message *msg = NULL;
	void *thread_ret;

#if 1
	{
		int cnt;
		for (cnt=1; cnt<argc; cnt++) {
			printf("%s ", argv[cnt]);
		}
		printf("\n");
	}
#endif

	if (1 == argc) {
		usage();
		return 0;
	}
	if (!strcmp(argv[1], "-h") || (!strcmp(argv[1], "-help"))) {
		usage();
		return 0;
	}
//申请 __message 结构体大小的内存，并将内存地址返回给msg
	msg = (struct __message *)malloc(sizeof(struct __message));
	if (!msg) {
		printf("malloc for msg fail.\n");
		return -1;
	}
	memset(msg, 0x00, sizeof(struct __message));//初始化变量msg 为0
//将命令行参数进行解析，得出需要读取消息/发送消息、目的节点和端口号
	ret = parsing_args(msg, argc, argv);
	if (ret < 0) {
		goto err;
	}
//创建线程
	ret = pthread_create(&msg->thread_id, NULL, pthread_fn, (void *)msg);
	if (ret) {
		printf("Err, create thread fail. ret:%d\n", ret);
		goto err;
	}

#ifndef __LITEOS__
	ret = pthread_join(msg->thread_id, &thread_ret);
	if (ret) {
		printf("Err, join thread fail. ret:%d\n", ret);
		goto err;
	}
	printf("exited ,%s.\n", (char *)thread_ret);
#endif

	return 0;

err:
	if (msg) {
		if (msg->buf)
			free(msg->buf);
		free(msg);
	}
	return ret;
}


#ifdef __LITEOS__
SHELLCMD_ENTRY(ipcm_shellcmd, CMD_TYPE_STD, "ipcm_app", 0, (CMD_CBK_FUNC)ipcm_main);
#endif
