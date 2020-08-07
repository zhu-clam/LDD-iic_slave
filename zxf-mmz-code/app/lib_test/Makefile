#
# -mcpu=ck860f -march=ck860
# /home/liuwei/project/polaris/buildroot/output/host/csky-buildroot-linux-gnuabiv2/sysroot/usr/
# Fixme: LDFLAGS set to "buildroot/output/host/csky-buildroot-linux-gnuabiv2/sysroot/usr/lib", will cause a "libc.so not found" error.!
#        So copy the lib and include form buildroot  to local.!
#

#####Makefile for ck860 mmz-app test########################
#####step1: export compiler-tool macro#####################

APP_NAME ?= mmzlib_test_app
CROSS_COMPILE=/home/zhuxianfei/ck860/toolchain-ck860/bin/csky-abiv2-linux-
CC=${CROSS_COMPILE}gcc
AS=${CROSS_COMPILE}as
LD=${CROSS_COMPILE}ld
AR=${CROSS_COMPILE}ar
OBJCOPY=${CROSS_COMPILE}objcopy
CPP=${CROSS_COMPILE}g++

#####step2: define source file path ###################
SRCS	:= $(wildcard ./*.c)
OBJS	:= $(patsubst %.c,%.o,${SRCS})


CFLAGS		+= -O0 -g -mcpu=ck860f #-march=ck860 #-O0 to gdb jtag debug problem 
INCLUDE		+= -I./../include
LDFLAGS		+= -L./../lib
LIBS		+= -lmmz
#LIBS		+= -lasound -lipcm_860

######step3: compiler command for generate app ############
all:${APP_NAME}

${APP_NAME}:${OBJS}
	$(CC) $(LDFLAGS) ${OBJS} $(LIBS) -o  $(APP_NAME) -lpthread -mcpu=ck860f

%.o:%.c
	@echo "make $@"
	@$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

#${APP_NAME}:${OBJS}
#	$(CC) $(LDFLAGS) ${OBJS} $(LIBS) -o  $(APP_NAME) -lpthread
#%.o:%.c
#	@echo "make $@"
#	@$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	@echo "clean object files"
	-@rm -f $(OBJS) $(APP_NAME)

distclean:clean
	-@rm -rf ${APP_NAME}

rebuild : clean all
