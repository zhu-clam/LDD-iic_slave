#
# -mcpu=ck860f -march=ck860
# /home/liuwei/project/polaris/buildroot/output/host/csky-buildroot-linux-gnuabiv2/sysroot/usr/
# Fixme: LDFLAGS set to "buildroot/output/host/csky-buildroot-linux-gnuabiv2/sysroot/usr/lib", will cause a "libc.so not found" error.!
#        So copy the lib and include form buildroot  to local.!
#

#APP_NAME ?= caputure
#APP_NAME ?= ck860_codec
#APP_NAME ?= ck860_play_audio
APP_NAME ?= ck860_test
#APP_NAME ?= ./ipcm_for_gitup/ck860_play_up
CROSS_COMPILE=/home/zhuxianfei/ck860/toolchain-ck860/bin/csky-abiv2-linux-
CC=${CROSS_COMPILE}gcc
AS=${CROSS_COMPILE}as
LD=${CROSS_COMPILE}ld
AR=${CROSS_COMPILE}ar
OBJCOPY=${CROSS_COMPILE}objcopy
CPP=${CROSS_COMPILE}g++


#SRCS	:= $(wildcard ./src/*.c)
#SRCS	:= $(wildcard ./audio_src/*.c)
SRCS	:= $(wildcard ./test_src/*.c)
#SRCS	:= $(wildcard ./ck860-codec_src/*.c)
#SRCS	:= $(wildcard ./ipcm_audio_app/*.c)
OBJS	:= $(patsubst %.c,%.o,${SRCS})

CFLAGS		+= -O0 -g -mcpu=ck860f #-O0 to gdb jtag debug problem 
INCLUDE		+= -I./include -I./../inc
LDFLAGS		+= -L./../lib
LIBS		+= -lipcm_860
#LIBS		+= -lasound -lipcm_860

#CFLAGS += -Wall -I$(DIR)/../inc

all:${APP_NAME}

${APP_NAME}:${OBJS}
	$(CC) $(LDFLAGS) ${OBJS} $(LIBS) -mcpu=ck860f -o  $(APP_NAME) -lpthread

%.o:%.c
	@echo "make $@"
	@$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	@echo "clean object files"
	-@rm -f $(OBJS) $(APP_NAME)

distclean:clean
	-@rm -rf ${APP_NAME}

rebuild : clean all
