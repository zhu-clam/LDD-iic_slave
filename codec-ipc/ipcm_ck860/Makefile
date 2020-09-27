#
# -mcpu=ck860f -march=ck860
# /home/liuwei/project/polaris/buildroot/output/host/csky-buildroot-linux-gnuabiv2/sysroot/usr/
# Fixme: LDFLAGS set to "buildroot/output/host/csky-buildroot-linux-gnuabiv2/sysroot/usr/lib", will cause a "libc.so not found" error.!
#        So copy the lib and include form buildroot  to local.!
#

#APP_NAME ?= caputure
APP_NAME ?= ./bin/ck860_recv_send
#APP_NAME ?= ./bin/ck860_send
#APP_NAME ?= ./ipcm_for_gitup/ck860_play_up
CROSS_COMPILE=/home/zhuxianfei/ck860/toolchain-ck860/bin/csky-abiv2-linux-
CC=${CROSS_COMPILE}gcc
AS=${CROSS_COMPILE}as
LD=${CROSS_COMPILE}ld
AR=${CROSS_COMPILE}ar
OBJCOPY=${CROSS_COMPILE}objcopy
CPP=${CROSS_COMPILE}g++


SRCS	:= $(wildcard ./src/*.c)
OBJS	:= $(patsubst %.c,%.o,${SRCS})

CFLAGS		+= -O2 -I./inc
INCLUDE		+= -I./include
LDFLAGS		+= -L./lib 
LIBS		+= -lasound -lipcm_860

all:${APP_NAME}

${APP_NAME}:${OBJS}
	$(CC) $(LDFLAGS) ${OBJS} $(LIBS) -o $(APP_NAME) -lpthread

%.o:%.c
	@echo "make $@"
	@$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	@echo "clean object files"
	-@rm -f $(OBJS) $(APP_NAME)

distclean:clean
	-@rm -rf ${APP_NAME}

rebuild : clean all
