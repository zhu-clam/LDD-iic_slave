NAME    = drivers

SRS = $(wildcard *.S)
SRC = uart.c intc.c pll.c timer.c ddr.c pmu.c

ifeq ($(TARGET),)
SRC = $(wildcard *.c)
else ifeq ($(TARGET), all)
SRC = $(wildcard *.c)
else ifeq ($(TARGET), mini_bm)
SRC += 
else ifeq ($(TARGET), intc)
SRC +=
else ifeq ($(TARGET), uart)
SRC +=
else ifeq ($(TARGET), gpio)
SRC += gpio.c pinmux.c
else ifeq ($(TARGET), timer)
SRC += timer.c
else ifeq ($(TARGET), watchdog)
SRC += wdt.c
else ifeq ($(TARGET), i2c)
SRC += iic.c at24c64.c
else ifeq ($(TARGET), i2c-slave)
SRC += iic.c iic-slave.c
else ifeq ($(TARGET), axidma)
SRC += axidma.c
else ifeq ($(TARGET), ahbdma)
SRC += ahbdma.c spi_nand.c iic.c
else ifeq ($(TARGET), rtc)
SRC += rtc.c
else ifeq ($(TARGET), spi_nor)
SRC += gd25q128.c spi_nor.c axidma.c
else ifeq ($(TARGET), spi_nand)
SRC += w25n01_app.c spi_nand.c
else ifeq ($(TARGET), sdio)
SRC += syns_mmc.c
else ifeq ($(TARGET), regs)
SRC +=
else ifeq ($(TARGET), stc)
CSRC += 
else ifeq ($(TARGET), ddr)
SRC += 
else ifeq ($(TARGET), i2s_pts)
SRC += 
else ifeq ($(TARGET), sci)
SRC += 
else ifeq ($(TARGET), mipi)
SRC += mipi_subsys.c iic.c OV_5640.c isp.c gpio.c  pinmux.c mipi_dphy.c
else ifeq ($(TARGET), sdi)
SRC +=  iic.c  gpio.c  pinmux.c  ADV7611.c dvi.c ADV7511.c
else ifeq ($(TARGET), apts)
SRC += gpio.c  pinmux.c
else ifeq ($(TARGET), l2c)
SRC +=
else ifeq ($(TARGET), pmu)
SRC += 
else ifeq ($(TARGET), clock_gate)
SRC += 
else ifeq ($(TARGET), slave_boot)
SRC += 
else ifeq ($(TARGET), nfc)
SRC += nand.c
else ifeq ($(TARGET), pinmux)
SRC += gpio.c pinmux.c
else
$(error wrong TARGET input!)
endif

include $(ROOTDIR)/Rules.make
