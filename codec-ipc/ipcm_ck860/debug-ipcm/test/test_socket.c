#ifndef TWO_OS
/* for linux */
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>

#define IPCM_PRINT		printf
#else
/* for liteos */
#include "pthread.h"
#include "stdlib.h"
#include "stdio.h"
#include "fcntl.h"
#include "string.h"
#include "lwip/sockets.h"
#include "unistd.h"
#include "malloc.h"
#include "sys/select.h"

#define IPCM_PRINT		dprintf
#endif

#include "ipcm_dev.h"

#define NONE          "\033[m"
#define LIGHT_RED     "\033[1;31m"
#define LIGHT_GREEN   "\033[1;32m"
#define LIGHT_BLUE    "\033[1;34m"

static char __message1[128] = "abcdefghijklmnopqrstuvwxyz!\n";

#define  BUF_LEN    0x4000

#define  PORT		80

static int parsing_args(struct __message *msg, int argc, char *argv[]);
static int create_proc(struct __message *msg);
static void * pthread_fn(void *data);
//printf("[%s:%d]\n", __func__, __LINE__);

#ifndef TWO_OS
/* for linux */
int main(int argc, char *argv[])
#else
/* for liteos */
extern int inet_pton(int af, const char *src, void *dst);
int ipcm_socket(int argc, char *argv[])
#endif
{
    int ret = 0;
	struct __message *msg = NULL;
	void *thread_ret;

#if 0
    {
        int cnt;
        for (cnt=1; cnt<argc; cnt++) {
            IPCM_PRINT("%s ", argv[cnt]);
        }
        IPCM_PRINT("\n");
    }
#endif

	if (argc == 2)
		if (!strcmp(argv[1], "-h") || (!strcmp(argv[1], "-help"))) {
			test_usage();
			return 0;
		}

	msg = (struct __message *)malloc(sizeof(struct __message));
	if (!msg) {
		IPCM_PRINT("malloc for msg fail.\n");
		return -1;
	}
	memset(msg, 0x00, sizeof(struct __message));

    msg->buf_len = BUF_LEN;
    msg->buf = (char *)malloc(msg->buf_len);
    if (!(msg->buf)) {
        IPCM_PRINT("malloc for msg fail.\n");
        goto err;
    }

	ret = parsing_args(msg, argc, argv);
	if (ret < 0) {
		goto err;
	}

    ret = pthread_create( &msg->thread_id, NULL, pthread_fn, (void *)msg);
    if (ret) {
        IPCM_PRINT("Err, create thread fail. ret:%d\n", ret);
        goto err;
    }
#if 1
    ret = pthread_join(msg->thread_id, &thread_ret);
    if (ret) {
        IPCM_PRINT("Err, join thread fail. ret:%d\n", ret);
        goto err;
    }
	IPCM_PRINT("exited ,%s.\n", (char *)thread_ret);
#endif
	return 0;

err:
    if (msg) {
        if (msg->buf)
            free(msg->buf);
		if (msg->msg)
			free(msg->msg);
        free(msg);
    }
    return ret;
}

static int parsing_args(struct __message *msg, int argc, char *argv[])
{
	int i = 0;

	if (1 == argc) {
		/* server */
		msg->flags |= (MSG_SERVER);
	}
	else if((2 == argc) || (3 == argc) || (4 == argc) || (5 == argc)) {
		/* client */
		msg->flags &= ~(MSG_SERVER);
		/* get ip */
		strncpy(msg->ip, argv[1], 32);
		msg->ip[31] = '\0';
		/* get send string */
		if (argc >= 3) {
			int mutil = 1;
			int msg_len = 0;
			msg_len = strlen(argv[2]);
			/* get string multiple */
			if (argc >= 4) {
				mutil = strtol(argv[3], NULL, 10);
			}
			msg->msg_len = msg_len * mutil;
			msg->msg = malloc(msg->msg_len + 1);
			for (i=0; i<mutil; i++) {
				strncpy(msg->msg + i*msg_len, argv[2], msg_len);
			}
			msg->msg[msg->msg_len-1] = '\0';
			/* get send times */
			if (argc >= 5)
				msg->times = strtoul(argv[4], NULL, 10);
			else
				msg->times = 1;

		} else {
			msg->msg_len = strlen(__message1);
			msg->msg = malloc(msg->msg_len + 1);
			strncpy(msg->msg, __message1, msg->msg_len);
			msg->msg[msg->msg_len-1] = '\0';

			msg->times = 1;
		}
	} else {
		IPCM_PRINT("args error");
		return -1;
	}
	return 0;
}

static int create_proc(struct __message *msg)
{
	int ret = 0;
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		IPCM_PRINT("open socket error\n");
		goto err;
	}
	IPCM_PRINT("open socket:%d\n", sockfd);
	msg->sock_addr.sin_family = AF_INET;
	msg->sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	msg->sock_addr.sin_port = htons(PORT);

	/* as a server */
	if (msg->flags & MSG_SERVER) {
		ret = bind(sockfd, (struct sockaddr *)&msg->sock_addr, sizeof(msg->sock_addr));
		if (-1 == ret) {
			IPCM_PRINT("bind sockfd error\n");
			goto err;
		}
		ret = listen(sockfd, 10);
		if (ret < 0) {
			IPCM_PRINT("listen error\n");
			goto err;
		}
	} else {
		inet_pton(AF_INET, msg->ip, &msg->sock_addr.sin_addr);
	}
	return sockfd;

err:
	return ret;
}

static int server_proc(struct __message *msg)
{
	int ret = 0;
	int clientfd = 0;
	fd_set readfs;
	struct timeval timeout;
	struct timeval tv0, tv1;

	IPCM_PRINT("accepting...\n");
	clientfd = accept(msg->fd, NULL, NULL);
	IPCM_PRINT("accept socket:%d\n", clientfd);
	if (clientfd < 0) {
		IPCM_PRINT(LIGHT_RED"accept error\n"NONE);
		return -1;
	}
	gettimeofday(&tv0, NULL);
	gettimeofday(&tv1, NULL);
	while (1) {
		if (!(msg->count%1000)) {
			gettimeofday(&tv1, NULL);
			IPCM_PRINT("\nServer:%d,cnt:%lu,us:%lu\n", msg->len, (unsigned long)msg->count,
					((tv1.tv_sec*1000000 + tv1.tv_usec) - (tv0.tv_sec*1000000 + tv0.tv_usec)));
			tv0.tv_sec = tv1.tv_sec;
			tv0.tv_usec = tv1.tv_usec;
		}
		timeout.tv_usec = 0;
		timeout.tv_sec = 2;
		FD_ZERO(&readfs);
		FD_SET(clientfd, &readfs);
		ret = select(clientfd + 1, &readfs, NULL, NULL, &timeout);
		if (ret < 0) {
			IPCM_PRINT(LIGHT_RED"select error,ret:%d\n"NONE,ret);
			goto out;
		} else if (0 == ret) {
			IPCM_PRINT("select timeout\n");
			FD_SET(clientfd, &readfs);
			goto out;
		}
		if (FD_ISSET(clientfd, &readfs)) {
			ret = read(clientfd, msg->buf, msg->buf_len);
			if (ret < 0) {
				IPCM_PRINT(LIGHT_RED"read error, ret:%d\n"NONE, ret);
				goto out;
			} else if (ret == 0) {
				IPCM_PRINT(LIGHT_GREEN"read zero\n"NONE);
				//continue;
				goto out;
			}

			msg->len = ret;
			ret = check_msg_buf(msg);
			if (ret) {
				IPCM_PRINT(LIGHT_RED"check error, ret:%d\n"NONE, ret);
				goto out;
			}
			//IPCM_PRINT("read: %s\n", get_msg_from_buf(msg));
			ret = write(clientfd, msg->buf, msg->len);
		}
	}

out:
	close(clientfd);
	return ret;

}

static int client_proc(struct __message *msg)
{
	int ret;
	unsigned long i = 0;
	fd_set readfs;
	struct timeval timeout;
	struct timeval tv0, tv1;
	FD_ZERO(&readfs);

	IPCM_PRINT("connecting...\n");
	ret = connect(msg->fd, (struct sockaddr *)&msg->sock_addr,
			sizeof(msg->sock_addr));
	if (ret < 0) {
		IPCM_PRINT(LIGHT_RED"connect error\n"NONE);
		return -1;
	}
	gettimeofday(&tv0, NULL);
	gettimeofday(&tv1, NULL);
	while ((msg->times)? (msg->times > msg->count):1) {
		update_msg_buf(msg, msg->msg, msg->msg_len);
		if (!(i%1000)) {
			gettimeofday(&tv1, NULL);
			IPCM_PRINT("\nClient:%d,cnt:%lu,us:%lu\n", msg->len, (unsigned long)msg->count,
					((tv1.tv_sec*1000000 + tv1.tv_usec) - (tv0.tv_sec*1000000 + tv0.tv_usec)));
			tv0.tv_sec = tv1.tv_sec;
			tv0.tv_usec = tv1.tv_usec;
		}
		i++;

		ret = write(msg->fd, msg->buf, msg->len);
		if (ret != msg->len) {
			IPCM_PRINT(LIGHT_RED"write error,ret:%d, send:%d\n"NONE, ret, msg->len);
			goto out;
		}
		FD_SET(msg->fd, &readfs);
		timeout.tv_usec = 0;
		timeout.tv_sec = 2;
		ret = select(msg->fd +1, &readfs, NULL, NULL, &timeout);
		if (ret < 0) {
			IPCM_PRINT(LIGHT_RED"select error,ret:%d\n"NONE,ret);
			goto out;
		} else if (0 == ret) {
			IPCM_PRINT("select timeout\n");
			FD_SET(msg->fd, &readfs);
			goto out;
		}
		if (FD_ISSET(msg->fd, &readfs)) {
			ret = read(msg->fd, msg->buf, msg->buf_len);
			if (ret <= 0) {
				IPCM_PRINT(LIGHT_RED"read error or no data, ret:%d\n"NONE, ret);
				goto out;
			}
			//IPCM_PRINT("read: %s\n", get_msg_from_buf(msg));
			ret = check_msg_buf(msg);
			if (ret) {
				IPCM_PRINT(LIGHT_RED"check error, ret:%d\n"NONE, ret);
				goto out;
			}
		}
	}
out:
	if (msg->times == msg->count)
		return 0;
	return -1;
}


static void * pthread_fn(void *data)
{
	int ret = 0;
	struct __message *msg = (struct __message *)data;

    ret = create_proc(msg);
    if (ret < 0) {
        IPCM_PRINT("create socket fail");
        goto err;
    }
    msg->fd = ret;

	if (msg->flags & MSG_SERVER) {
		ret = server_proc(msg);
	} else {
		ret = client_proc(msg);
	}
	sleep(1);

	if (!ret) {
		IPCM_PRINT(LIGHT_BLUE"IPCM socket test SUCCESS\n"NONE);
	} else {
		IPCM_PRINT(LIGHT_RED"IPCM socket test FAIL\n"NONE);
	}

	close(msg->fd);

err:
	if (msg) {
		if (msg->buf)
			free(msg->buf);
		if (msg->msg)
			free(msg->msg);
		free(msg);
	}
	IPCM_PRINT("pt[0x%x],p[%d]exited.\n", msg->thread_id, msg->attr.port);
	pthread_detach(msg->thread_id);
	pthread_exit("Bye");

	return 0;
}
