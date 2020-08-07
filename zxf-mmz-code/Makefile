
PWD := $(shell pwd)
#kdir := /home/zhuxianfei/ck860/boya_1218/linux/
#kdir := /home/zhuxianfei/ck860/boya_December_29/linux
kdir := ~/ck860/new-buildroot/buildroot/buildroot2_c860_sc_byhl_20181018/output/build/linux-4.9.56/

SRCDIR :=$(PWD)

#head file path

obj-y	+= mmz-drv.o
mmz-drv-objs :=  ./mmz-userdev.o \
	./gv_allocator.o \
	./media-mem.o \

EXTRA_CFLAGS     += -Wall -mcpu=ck860f

default:
	$(MAKE) -C $(kdir) M=$(SRCDIR) modules
clean:
	rm *.ko *.o *.mod* modules.order Module.symvers -rfv
