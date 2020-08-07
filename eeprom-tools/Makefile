
PACKAGE_NAME = test_e2p
#CROSS_COMPILE="/home/share/Tarana/Toolchain/gcc-linaro-7.3.1-2018.05-x86_64_arm-linux-gnueabi/bin/arm-linux-gnueabi-"

#### Modify Stop Here ####

CC=${CROSS_COMPILE}gcc
AS=${CROSS_COMPILE}as
LD=${CROSS_COMPILE}ld
AR=${CROSS_COMPILE}ar
OBJCOPY=${CROSS_COMPILE}objcopy

SCRIPTS := $(wildcard *.sh)
SRCS	:= $(wildcard *.c)
OBJS	:= $(patsubst %.c,%.o,${SRCS})
PROGS	:= $(patsubst %.c,%.so,${SRCS})

LDFLAGS += -g -lc
LIBS    += 
CFLAGS  += -g -Wall -O2 -fPIC

PHONY += all

all: ${PACKAGE_NAME}

${PACKAGE_NAME}: ${OBJS} $(LIBS)
	@echo "make $@"
	@$(CC)  ${LDFLAGS} ${OBJS} -o $@

%.o: %.c
	@echo "make $@"
	@${CC} -c $(CFLAGS) $< -o $@

clean:
	-@rm  -f *.o 
	-@rm ${PACKAGE_NAME}
