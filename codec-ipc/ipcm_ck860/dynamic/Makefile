

#=============================================tool-chain declear
CROSS = /home/zhuxianfei/ck860/toolchain-ck860/bin/csky-abiv2-linux-

CC = $(CROSS)gcc
CPP = $(CROSS)g++
AR = $(CROSS)ar -r
CPP = $(CROSS)g++


#========================================= target & source file path declear

DIR = $(shell pwd)
TARGET = $(DIR)/../dy_lib/libipcm_860.so

#动态链接库编译时需要-fPic选项
CFLAG +=-fPIC
#链接动态库时需要 --share选项
LFLAG += --share

SRC=$(wildcard $(DIR)/*.c)
OBJ=$(patsubst %.c, %.o, $(SRC))

CFLAG += -I$(DIR)/../inc/.

#=======================================link & compiler command

#链接生成动态库
all:$(TARGET)
$(TARGET):$(OBJ)
	$(CC) -o $@ $^ $(LFLAG)

#编译
$(DIR)/%.o: $(DIR)/%.c
	$(CC) -o $@ -c $< $(CFLAG)

#======================================make clean command
.PHONY:clean

clean:
	rm -f $(OBJ) $(BIN)




