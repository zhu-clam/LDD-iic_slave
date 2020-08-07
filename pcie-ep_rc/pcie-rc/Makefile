
# Makefile
# Author: Yang Liu


# 被屏蔽的部分

ZXF_TEST := y

ifeq ($(ZXF_TEST),n)

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

endif

# 正式进入Makefile 的部分

TARGET_ARCH := ck860
CROSS := /home/zhuxianfei/ck860/toolchain-ck860/bin/csky-abiv2-linux-

output := pci_host_rc_api

CC      = $(CROSS)gcc
LD      = $(CROSS)g++
CPP     = $(CROSS)g++
AS      = $(CROSS)as
AR      = $(CROSS)ar
LD      = $(CROSS)ld
DUMP    = $(CROSS)objdump
OBJCOPY = $(CROSS)objcopy

# 编译标志 -g 表示编译出的文件包含gdb调试信息
CFLAGS = -g -O3 -W   -mhard-float
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
INCLUDE := -I.

.PHONY: clean

#target : components
#	rule

# main.elf 文件依赖于 main.o 文件
# 编译命令 gcc -o main.elf main.o codec.a
# link 链接过程
#main.elf : main.o
$(output).elf : $(output).o 
	$(CPP) -g -o $@ $^
#	$(CPP) -o $@ $^ $(LIBS)
# compile 编译过程 gcc -I include/ -c main.c -o main.o ;
# 其中$^ 表示所有的依赖文件,此处就是main.c 一个
# 其中$@ 表示所有的输出文件,此处对于main.o
$(output).o : $(output).c
	$(CC) $(INCLUDE) -g -c $^ -o $@

# main.o 文件依赖于 main.c 文件	
# 编译命令 	g++ -I. -c main.c -o main.o

clean:
	-@rm -f *.o $(output).elf

