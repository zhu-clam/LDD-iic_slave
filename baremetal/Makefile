# written by

NAME    = bm-polaris

CPU_TYPE ?= ck860

NAME    := $(NAME)_$(LOC)_$(TARGET)_$(CPU_TYPE)

ifeq ($(CPU_TYPE), ck810)
#CROSS-COMPILER = csky-abiv2-elf-
CROSS-COMPILER = csky-linux-gnuabiv2-
CFLAGS += -DCK_CK860=0
else #is ck860
CROSS-COMPILER = /home/zhuxf/tool-chain/bin/csky-linux-gnuabiv2-
CFLAGS += -DCK_CK860=1
#Only CK860 has L2 Cache
EN_L2C ?= 1
ifeq ($(EN_L2C), 1)
CFLAGS += -DCONFIG_CKCPU_L2CACHE=1
else
CFLAGS += -DCONFIG_CKCPU_L2CACHE=0
NAME    := $(NAME)_disl2c
endif

endif

CC      = ${CROSS-COMPILER}gcc
AS      = ${CROSS-COMPILER}as
AR      = ${CROSS-COMPILER}ar
LD      = ${CROSS-COMPILER}ld
DUMP    = ${CROSS-COMPILER}objdump
OBJCOPY = ${CROSS-COMPILER}objcopy
ROOTDIR = $(shell pwd)


INCLUDEDIRS = -I$(ROOTDIR)/include  -I$(ROOTDIR)/drivers -I./

CFLAGS += -mcpu=$(CPU_TYPE) -g3 -c -O0 -mlittle-endian -mno-multiple-stld
CFLAGS += -Wall
CFLAGS += -fno-strict-aliasing -fno-builtin -fomit-frame-pointer
CFLAGS += -frename-registers
CFLAGS += $(INCLUDEDIRS)


LDFLAGS = -EL
ASFLAGS = $(CFLAGS) -Wa,--gstabs
# where to load: DDR or SRAM
ifeq ($(LOC),)
LD_SCRIPTS = ./ckcpu.ld
else ifeq ($(LOC), ddr)
LD_SCRIPTS = ./ckcpu.ld
else ifeq ($(LOC), sram)
LD_SCRIPTS = ./ckcpu.ld_sram
endif

INCFILE = ck810.h

export CC AS AR LD DUMP OBJCOPY CFLAGS LDFLAGS ASFLAGS INCLUDEDIRS ROOTDIR

LDLIBS	=  -lm -lc -lgcc
DEPEND	= .dep

.PHONY : all

all    : childdir $(NAME).elf $(NAME).bin

childdir:
	@for dir in $(SUB_DIRS); do\
		$(MAKE) -C $$dir || exit 1;\
	done

SUB_DIRS = $(shell ls  -F | grep /$ | grep -v Debug | grep -v Release | \
		  grep -v CVS | grep -v include | grep -v obj | \
          sed "s/\///g")

SUBDIR_FILES = $(foreach dir, $(SUB_DIRS), $(dir)/$(dir).o)

#SSRC 代表所有的.S 文件; CSRC 代表 待添加编译的.c文件.
SSRC = $(wildcard *.S)	
CSRC = main.c weak_func.c

ifeq ($(TARGET),)
CSRC = $(wildcard *.c)
else ifeq ($(TARGET), all)
CSRC = $(wildcard *.c)
else ifeq ($(TARGET), mini_bm)
CSRC += tintc.c ttimer.c tuart.c
else ifeq ($(TARGET), intc)
CSRC += tintc.c
else ifeq ($(TARGET), uart)
CSRC += tuart.c
else ifeq ($(TARGET), gpio)
CSRC +=
else ifeq ($(TARGET), timer)
CSRC += ttimer.c
else ifeq ($(TARGET), watchdog)
CSRC += twdt.c
else ifeq ($(TARGET), i2c)
CSRC += tiic.c  
else ifeq ($(TARGET), i2c-slave)
SRC +=
else ifeq ($(TARGET), axidma)
CSRC += taxidma.c
else ifeq ($(TARGET), ahbdma)
CSRC +=
else ifeq ($(TARGET), rtc)
CSRC +=
else ifeq ($(TARGET), spi_nor)
CSRC +=
else ifeq ($(TARGET), spi_nand)
CSRC +=
else ifeq ($(TARGET), sdio)
CSRC += tmmc.c
else ifeq ($(TARGET), regs)
CSRC += tapb_access.c
else ifeq ($(TARGET), ddr)
CSRC += tddr.c
else ifeq ($(TARGET), stc)
CSRC += tstc.c
else ifeq ($(TARGET), i2s_pts)
CSRC += ti2s_pts.c
else ifeq ($(TARGET), sci)
CSRC += tsci.c
else ifeq ($(TARGET), mipi)
CSRC +=
else ifeq ($(TARGET), apts)
CSRC += tapts.c ti2s_pts.c
else ifeq ($(TARGET), l2c)
CSRC += L2_cache.c
else ifeq ($(TARGET), pmu)
CSRC += tpmu.c
else ifeq ($(TARGET), pinmux)
CSRC += tpinmux.c
else ifeq ($(TARGET), clock_gate)
CSRC += tpll.c
else ifeq ($(TARGET), slave_boot)
CSRC += cpu_boot.c tpmu.c 
else ifeq ($(TARGET), nfc)
CSRC += 
else
$(error wrong TARGET input!)
endif

OBJECTS =  $(SSRC:%.S=%.o) $(CSRC:%.c=%.o)

%.o:%.c
	$(CC)  $(CFLAGS)  -o $@ $<
%.o:%.S
	$(CC)  $(ASFLAGS) -o $@ $<

$(NAME).elf: $(OBJECTS)  $(SUBDIR_FILES) ckcpu.ld dep
	$(CC)  -mcpu=$(CPU_TYPE) -mno-multiple-stld -mlittle-endian -nostartfiles -static \
	       -T$(LD_SCRIPTS) -o $(NAME).elf $(OBJECTS) $(SUBDIR_FILES) $(LDLIBS) -Wl,-Map=$(NAME).map
	$(DUMP) -xd $(NAME).elf > $(NAME).asm
#	cp $(NAME).elf ../bm-output/$(NAME)-$(CPU_TYPE)-$(LOC)-$(TARGET).elf
$(NAME).bin: $(NAME).elf
	$(OBJCOPY) -O binary -j .text -j .rodata -j .data $(NAME).elf $(NAME).bin
ifneq ($(wildcard .dep),)
include $(DEPEND)
endif

dep:
	$(SHELL) -ec '$(CC) -MM $(CFLAGS) $(INCLUDEDIRS) $(CSRC) $(SSRC) \
        | sed '\''s@\(.*\)\.o[ :]@\1.o:@g'\'' > $(DEPEND)'
	@for dir in $(SUB_DIRS); do\
                $(MAKE) -C $$dir dep;\
        done

clean:
	rm -f $(OBJECTS)
	rm -f $(NAME).elf
	rm -f $(NAME).asm
	rm -f $(NAME).map
	rm -f .dep
	@for dir in $(SUB_DIRS); do\
		$(MAKE) -C $$dir clean;\
	done

.PHONY : debug-make

DEBUG_VARS = 	SUB_DIRS \
		SUBDIR_FILES\
		INCLUDEDIRS \
		CSRC \
		SSRC \
		OBJECTS \
		ASFLAGS \
		CFLAGS
#:
#: debug-make -- Print a list of Makefile variables

debug-make:
	@$(foreach var, $(DEBUG_VARS), echo $(var)=$($(var)) ; )
