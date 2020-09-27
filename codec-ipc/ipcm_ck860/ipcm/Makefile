
PWD := $(shell pwd)
#kdir := /home/zhuxianfei/ck860/boya_1218/linux/
#kdir := /home/zhuxianfei/ck860/boya_December_29/linux
kdir := ~/ck860/new-buildroot/buildroot/buildroot2_c860_sc_byhl_20181018/output/build/linux-4.9.56/

SRCDIR :=$(PWD)

#head file path

obj-m	+= ipcm_dev_ck860.o
ipcm_dev_ck860-objs :=  ./ipcm.o \
	./ipcm_data.o \
	./ipcm_node_discover.o \
	./ipcm_proc.o \
	./os_adapt.o \
	./linux_ck860.o	./ipcm_userdev.o 

EXTRA_CFLAGS     += -Wall

default:
	$(MAKE) -C $(kdir) M=$(SRCDIR) modules
clean:
	rm *.ko *.o *.mod* -rfv
