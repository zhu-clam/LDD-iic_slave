
PWD := $(shell pwd)
SRCDIR :=$(PWD)
#kdir:= ${HOME}/MyProjects/gv9531/kernel/linux-asic
kdir:= ~/data/linux_v0.5.3/polaris_ck810_860/kernel/linux-asic
#kdir:= ~/commit-code-linux/polaris_ck810_860/kernel/linux-asic
obj-m += pci-ep-gv9531.o
pci-ep-gv9531-objs := pci-epf-gv953x.o epf_data_channel.o   epf_channel_proc.o  epf_call_user_app.o# axidma.o # ahbdma.o #add for porting ahbdma

EXTRA_CFLAGS     += -Wall

default:
	$(MAKE) -C $(kdir)  M=$(SRCDIR) modules
clean:
	rm *.ko *.o *.mod* Module.* modules.* -rfv
