

#CROSS = csky-buildroot-linux-gnuabiv2-
CROSS = /home/zhuxianfei/ck860/toolchain-ck860/bin/csky-abiv2-linux-

CC = $(CROSS)gcc
CPP = $(CROSS)g++
AR = $(CROSS)ar -r
CPP = $(CROSS)g++

#link flag macro
LFLAG = 
#compiler flag macro
CFLAG = -I$(DIR)/../include/. -g -mcpu=ck860f #-march=ck860

DIR = $(shell pwd)

BIN = $(DIR)/../lib/libmmz.a

SHARE = --share



SRC=$(wildcard $(DIR)/*.c)
OBJ=$(patsubst %.c, %.o, $(SRC))

#链接生成静态库
all:$(BIN)
$(BIN):$(OBJ)
	$(AR) $@ $^ $(LFLAG)

#编译
$(DIR)/%.o: $(DIR)/%.c
	$(CC) -o $@ -c $< $(CFLAG)
#	$(CC) -o $@ -c $< $(CFLAG)


.PHONY:clean

clean:
	rm -f $(OBJ) $(BIN)




