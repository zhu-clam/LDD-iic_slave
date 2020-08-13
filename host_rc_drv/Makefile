ifeq ($(KERNELRELEASE),)

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
       
	gcc -o tun4gv9531_x tun_dev.c gv_host_channel_api.c -lpthread
	gcc -o gv-cp gv_cp.c gv_host_channel_api.c
	gcc -o gv-shell gv_shell.c gv_host_channel_api.c
	gcc -o gv-reset reset_dev.c gv_host_channel_api.c
clean:
	rm -rf *.o *.ko .depend *.mod.o *.mod.c Module.* modules.* *.o.ur-safeI tun4gv9531_x gv-cp gv-shell gv-reset 

.PHONY:modules clean

else

obj-m := pci_gv9531_host.o
pci_gv9531_host-objs := pci_endpoint_test.o pci_data_channel.o pci_channel_proc.o


endif
