#include "pthread.h"
#include "stdlib.h"
#include "stdio.h"
#include "fcntl.h"
#include "string.h"
#include "lwip/sockets.h"
#include "unistd.h"
#include "malloc.h"
#include "sys/select.h"
#include "ipcm_dev.h"

static char __message1[128] = "abcdefghijklmnopqrstuvwxyz! Yon know nothing, Jon Snow.\n";
#define  BUF_LEN    0x4000

#define  PORT		80

static int stop = -1;

static int parsing_args(struct __message *msg, int argc, char *argv[]);
static int create_proc(struct __message *msg);
static void * pthread_fn(void *data);
//printf("[%s:%d]\n", __func__, __LINE__);
extern int inet_pton(int af, const char *src, void *dst);
int ipcm_socket(int argc, char *argv[])
{
    int ret = 0;
	struct __message *msg = NULL;
	void *thread_ret;
#if 0
    {
        int cnt;
        for (cnt=0; cnt<argc; cnt++) {
            printf("%s ", argv[cnt]);
        }
        printf("\n");
    }
	if (!strcmp(argv[1], "-h") || (!strcmp(argv[1], "-help"))) {
		test_usage();
		return 0;
	}
#endif

	msg = (struct __message *)malloc(sizeof(struct __message));
	if (!msg) {
		printf("malloc for msg fail.\n");
		return -1;
	}
	memset(msg, 0x00, sizeof(struct __message));

    msg->buf_len = BUF_LEN;
    msg->buf = (char *)malloc(msg->buf_len);
    if (!(msg->buf)) {
        printf("malloc for msg fail.\n");
        goto err;
    }

	ret = parsing_args(msg, argc, argv);
	if (ret < 0) {
		goto err;
	}
	ret = create_proc(msg);
	if (ret < 0) {
		printf("create socket fail");
		goto err;
	}
	msg->fd = ret;

    ret = pthread_create( &msg->thread_id, NULL, pthread_fn, (void *)msg);
    if (ret) {
        printf("Err, create thread fail. ret:%d\n", ret);
        goto err;
    }

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
		printf("args error");
		return -1;
	}
	return 0;
}

static int create_proc(struct __message *msg)
{
	int ret = 0;
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		printf("open socket error\n");
		goto err;
	}
	printf("open socket:%d\n", sockfd);
	msg->sock_addr.sin_family = AF_INET;
	msg->sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	msg->sock_addr.sin_port = htons(PORT);

	/* as a server */
	if (msg->flags & MSG_SERVER) {
		ret = bind(sockfd, (struct sockaddr *)&msg->sock_addr, sizeof(msg->sock_addr));
		if (-1 == ret) {
			printf("bind sockfd error\n");
			goto err;
		}
		ret = listen(sockfd, 10);
		if (ret < 0) {
			printf("listen error\n");
			goto err;
		}
	} else {
		inet_pton(AF_INET, msg->ip, &msg->sock_addr.sin_addr);
	}
	return sockfd;

err:
	return ret;
}

static void * pthread_fn(void *data)
{
	int ret = 0;
	int clientfd = 0;
	fd_set readfs;
	struct timeval timeout;
	struct timeval tv0, tv1;
	struct __message *msg = (struct __message *)data;
	FD_ZERO(&readfs);
	timeout.tv_usec = 100;
	timeout.tv_sec = 5;

	if (msg->flags & MSG_SERVER) {
		printf("accepting...\n");
		clientfd = accept(msg->fd, NULL, NULL);
		printf("accept socket:%d\n", clientfd);
		if (clientfd < 0) {
			printf("accept error\n");
			goto out;
		}
		FD_SET(clientfd, &readfs);
		gettimeofday(&tv0, NULL);
		gettimeofday(&tv1, NULL);
		while (1) {
			if (!(msg->count%1000000)) {
				gettimeofday(&tv1, NULL);
				printf("\nlen:%d,cnt:%lu,us:%lu\n", msg->len, (unsigned long)msg->count,
						((tv1.tv_sec*1000000 + tv1.tv_usec) - (tv0.tv_sec*1000000 + tv0.tv_usec)));
				tv0.tv_sec = tv1.tv_sec;
				tv0.tv_usec = tv1.tv_usec;
			}
			ret = select(clientfd + 1, &readfs, NULL, NULL, &timeout);
			if (ret < 0) {
				printf("select error,ret:%d\n",ret);
				goto out;
			} else if (0 == ret) {
				printf("select timeout\n");
				FD_SET(clientfd, &readfs);
				continue;
			}

			ret = read(clientfd, msg->buf, msg->buf_len);
			if (ret <= 0) {
				printf("read error or no data, ret:%d\n", ret);
				goto out;
			}
			msg->len = ret;
			ret = check_msg_buf(msg);
			if (ret) {
				printf("check error, ret:%d\n", ret);
				goto out;
			}
			//printf("read: %s\n", get_msg_from_buf(msg));

			ret = write(clientfd, msg->buf, msg->len);
		}
	} else {
		printf("connecting...\n");
		ret = connect(msg->fd, (struct sockaddr *)&msg->sock_addr,
				sizeof(msg->sock_addr));
		if (ret < 0) {
			printf("connect error\n");
			goto out;
		}
		gettimeofday(&tv0, NULL);
		gettimeofday(&tv1, NULL);
		while (msg->times ? (msg->times-- == 0):1) {
			update_msg_buf(msg, msg->msg, msg->msg_len);
			if (!(msg->count%1000000)) {
				gettimeofday(&tv1, NULL);
				printf("\nRlen:%d,cnt:%lu,us:%lu\n", msg->len, (unsigned long)msg->count,
						((tv1.tv_sec*1000000 + tv1.tv_usec) - (tv0.tv_sec*1000000 + tv0.tv_usec)));
				tv0.tv_sec = tv1.tv_sec;
				tv0.tv_usec = tv1.tv_usec;
			}
			ret = write(msg->fd, msg->buf, msg->len);
			if (ret != msg->len) {
				printf("write error,ret:%d, send:%d\n", ret, msg->len);
				goto out;
			}
			FD_SET(msg->fd, &readfs);
			ret = select(msg->fd +1, &readfs, NULL, NULL, &timeout);
			if (ret < 0) {
				printf("select error,ret:%d\n",ret);
				goto out;
			} else if (0 == ret) {
				printf("select timeout\n");
				FD_SET(msg->fd, &readfs);
				continue;
			}
			ret = read(msg->fd, msg->buf, msg->buf_len);
			if (ret <= 0) {
				printf("read error or no data, ret:%d\n", ret);
				goto out;
			}
			//printf("read: %s\n", get_msg_from_buf(msg));
			ret = check_msg_buf(msg);
			if (ret) {
				printf("check error, ret:%d\n", ret);
				goto out;
			}
		}
	}
	sleep(1);

out:
	if (msg->flags & MSG_SERVER) {
		close(clientfd);
	}
	close(msg->fd);
exit:
	if (msg) {
		if (msg->buf)
			free(msg->buf);
		if (msg->msg)
			free(msg->msg);
		free(msg);
	}
	printf("pt[0x%x],p[%d]exited.\n", msg->thread_id, msg->attr.port);
    pthread_detach(msg->thread_id);
    return 0;
}
