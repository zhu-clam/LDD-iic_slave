
# Makefile
# Author: Yang Liu

#SRCPATH = /home/yangchangfu/avs/avs-encode/src/
#INCLUDEPATH = /home/yangchangfu/avs/avs-encode/include/
#vpath %.c $(SRCPATH)
#vpath %.h $(INCLUDEPATH)
#VPATH=src:include

TARGET_ARCH := ck810

ifeq ($(TARGET_ARCH), arm)
   CROSS:= arm-xilinx-linux-gnueabi-
else ifeq ($(TARGET_ARCH), ck860)
   CROSS:= csky-abiv2-linux-
else ifeq ($(TARGET_ARCH), ck810)
   CROSS:= csky-buildroot-linux-gnuabiv2-
else 
   CROSS:= arm-xilinx-linux-gnueabi-
endif



CC      = $(CROSS)gcc
LD      = $(CROSS)g++
CPP     = $(CROSS)g++
AS      = $(CROSS)as
AR      = $(CROSS)ar
LD      = $(CROSS)ld
DUMP    = $(CROSS)objdump
OBJCOPY = $(CROSS)objcopy



 

CFLAGS = -g -O3 -W  
LDFLAGS=

ifeq ($(TARGET_ARCH), ck860)
   CFLAGS +=-mcpu=ck860f
   LDFLAGS +=-mcpu=ck860f
else ifeq ($(TARGET_ARCH), ck810)
   CFLAGS += -mcpu=ck810f
   LDFLAGS +=-mcpu=ck810f
endif

LDFLAGS := -L.
#LIBS := codec.a
LIBS := codec_gcc.a
INCLUDE := -I.

.PHONY: clean


main.elf : main.o
	$(CC) -o $@ $^ $(LIBS) -lm
main.o : main.c
	$(CC) $(INCLUDE) -c $^ -o $@

	


clean:
	-@rm -f *.o main.elf

