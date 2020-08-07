
PWD := $(shell pwd)
BIN := $(PWD)/../bin
kdir:= ~/data/linux_v0.5.3/polaris_ck810_860/kernel/linux-asic
SRCDIR :=$(PWD)

obj-m	+= gv_i2c.o
gv_i2c-objs := zxf_i2cslave.o 

EXTRA_CFLAGS     += -Wall

default:
	$(MAKE) -C $(kdir)  M=$(SRCDIR) modules
	@cp gvi2c.ko $(BIN)
clean:
	rm *.ko *.o *.mod* Module* modules* -rfv
