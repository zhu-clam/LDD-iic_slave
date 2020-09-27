
PWD := $(shell pwd)
#CK860
#kdir := /home/zhuxianfei/ck860/boya_1218/linux/
#CK810
kdir := /home/zhuxianfei/ck810/polaris_ck810_860/kernel/linux-4.9.56
#/home/zhuxianfei/new_csky/polaris_ck810_860/kernel/linux-4.9.56/

SRCDIR :=$(PWD)

#head file path

obj-m	+= ipcm_dev_ck810.o
ipcm_dev_ck810-objs :=  ./ipcm.o \
	./ipcm_data.o \
	./ipcm_node_discover.o \
	./ipcm_proc.o \
	./os_adapt.o \
	./linux_ck810.o	./ipcm_userdev.o 

EXTRA_CFLAGS     += -Wall

default:
	$(MAKE) -C $(kdir) M=$(SRCDIR) modules
clean:
	rm *.ko *.o *.mod* -rfv
