
PWD := $(shell pwd)
BIN := $(PWD)/../bin

SRCDIR :=$(PWD)

obj-m	+= gvi2c.o
gvi2c-objs := i2c_slave.o 

EXTRA_CFLAGS     += -Wall

default:
	$(MAKE) -C $(kdir)  M=$(SRCDIR) modules
	@cp gvi2c.ko $(BIN)
clean:
	rm *.ko *.o *.mod* Module* modules* -rfv
