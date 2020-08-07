
PWD := $(shell pwd)
BIN := $(PWD)/../bin
kdir:= ~/data/linux_v0.5.3/polaris_ck810_860/kernel/linux-asic
SRCDIR :=$(PWD)

obj-m	+= vip_proc.o
vip_proc-objs := vip_proc_file.o 

EXTRA_CFLAGS     += -Wall

default:
	$(MAKE) -C $(kdir)  M=$(SRCDIR) modules
clean:
	rm *.ko *.o *.mod* Module* modules* -rfv
