# written by

NAME    = bootrom
CPU_TYPE= ck810
#CPU_TYPE= ck860
LINKFILE = ckcpu_rom.ld
#LINKFILE = ckcpu_sram.ld
#LINKFILE = ckcpu_ddr.ld

HOSTOS := $(shell uname -s | tr '[:upper:]' '[:lower:]' | \
	    sed -e 's/\(cygwin\).*/cygwin/')

ifeq ($(HOSTOS), linux)
MAPFLAG = MAP
ifeq ($(CPU_TYPE), ck810)
#CROSS-COMPILER = csky-abiv2-linux-
CROSS-COMPILER = csky-abiv2-elf-
#CROSS-COMPILER = csky-buildroot-linux-gnuabiv2-
else
CROSS-COMPILER = csky-buildroot-linux-uclibcabiv2-
#CROSS-COMPILER = csky-buildroot-linux-uclibcabiv2-
endif
else
MAPFLAG = ckmap
ifeq ($(CPU_TYPE), ck810)
CROSS-COMPILER = csky-abiv2-elf-
else
CROSS-COMPILER = csky-buildroot-linux-uclibcabiv2-
endif
endif

CC      = ${CROSS-COMPILER}gcc
AS      = ${CROSS-COMPILER}as
AR      = ${CROSS-COMPILER}ar
LD      = ${CROSS-COMPILER}ld
DUMP    = ${CROSS-COMPILER}objdump
OBJCOPY = ${CROSS-COMPILER}objcopy
ROOTDIR = $(shell pwd)


INCLUDEDIRS = -I$(ROOTDIR)/boot -I$(ROOTDIR)/common -I$(ROOTDIR)/include -I$(ROOTDIR)/nand -I$(ROOTDIR)/sdhc -I$(ROOTDIR)/spi -I$(ROOTDIR)/uart -I$(ROOTDIR)/timer -I./

CFLAGS += -mcpu=$(CPU_TYPE) -g3 -c -O0 -mlittle-endian -mno-multiple-stld
CFLAGS += -Wall
CFLAGS += -fno-strict-aliasing -fno-builtin -fomit-frame-pointer
CFLAGS += -frename-registers
CFLAGS += $(INCLUDEDIRS)

LDFLAGS = -EL
ASFLAGS = $(CFLAGS) -Wa,--gstabs

INCFILE = ck810.h

export CC AS AR LD DUMP OBJCOPY CFLAGS LDFLAGS ASFLAGS INCLUDEDIRS ROOTDIR

LDLIBS	=  -lm -lc -lgcc
DEPEND	= .dep

.PHONY : all

all    : $(NAME).elf $(NAME).bin

childdir:
	@for dir in $(SUB_DIRS); do\
		$(MAKE) -C $$dir || exit 1;\
	done

SUB_DIRS = $(shell ls  -F | grep /$ | grep -v Debug | grep -v Release | \
		  grep -v CVS | grep -v include | grep -v obj | \
		  grep -v bus-test | grep -v spi_app \
		| sed "s/\///g")

#SUBDIR_FILES = $(foreach dir, $(SUB_DIRS), $(dir)/$(dir).o)

SSRC = $(wildcard *.S) 
CSRC = $(wildcard *.c)

APP_C_SRC := $(foreach dir,$(SUB_DIRS),$(wildcard $(dir)/*.c))
APP_S_SRC := $(foreach dir,$(SUB_DIRS),$(wildcard $(dir)/*.S))
#OBJ_S_FILES := $(foreach file,$(SUB_DIRS),$(patsubst %.S,%.o,$(subst src,obj,$(file))))
#OBJ_C_FILES := $(foreach file,$(SUB_DIRS),$(patsubst %.c,%.o,$(subst src,obj,$(file))))
OBJECTS =  $(SSRC:%.S=%.o) $(CSRC:%.c=%.o) $(APP_C_SRC:%.c=%.o) $(APP_S_SRC:%.S=%.o)
#OBJECTS-y +=  $(OBJ_S_FILES) $(OBJ_C_FILES)
DEP_FILES := $(OBJ_FILES:%=%.d)

arch/crt0.o: arch/crt0.S
	$(CC)  $(ASFLAGS) -o $@ $<
%.o:%.c
	$(CC)  $(CFLAGS)  -o $@ $<
%.o:%.S
	$(CC)  $(ASFLAGS) -o $@ $<

$(NAME).elf: arch/crt0.o $(OBJECTS)  $(SUBDIR_FILES) $(LINKFILE)
	#@echo "SUB_DIRS:"$(SUB_DIRS)
	#@echo "APP_C_SRC:"$(APP_C_SRC)
	#@echo "APP_S_SRC:"$(APP_S_SRC)
	#@echo "OBJECTS:"$(OBJECTS)
	@echo "HOSTOS:"$(HOSTOS)
	$(CC)  -mcpu=$(CPU_TYPE) -mno-multiple-stld -mlittle-endian -nostartfiles -nostdlib \
	       -T./$(LINKFILE) -o $(NAME).elf  $(OBJECTS) $(SUBDIR_FILES) $(LDLIBS) -Wl,-$(MAPFLAG)=$(NAME).map
	$(DUMP) -xd $(NAME).elf > $(NAME).asm

ifneq ($(wildcard .dep),)
include $(DEPEND)
endif
$(NAME).bin: $(NAME).elf
	$(OBJCOPY) -O binary -j .text -j .rodata -j .data $(NAME).elf $(NAME).bin
	
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
	rm -f $(NAME).bin

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
	
-include $(DEP_FILES)
