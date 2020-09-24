#export PATH=/home/share/BoYa/Toolchain/csky-abiv2-linux-toolchain-V3.8.12_glibc/csky-linux-gnuabiv2-tools-x86_64-glibc-linux-4.9.56-20181101/bin/:$PATH
export PATH=/home/zhuxf/tool-chain/bin/:$PATH

cmd_help() {
    echo "Basic mode:"
    echo "$0 h ---> command help"
    echo "$0 a ---> make all for DDR"
    echo "$0 sram ---> make SRAM"
    echo "$0 c ---> make clean"
    echo "$0 ca ---> clean and remove all .bin .elf .asm .map files"
}

#
# ------------------------------------------------------------------------------
#
if [ $# != 1 -a $# != 2 ]
then
    echo "wrong args."
    cmd_help
    exit
fi

if [ $1 = "h" ]
then
    cmd_help
elif [ $1 = "sram" ]
then
    echo "do make SRAM"
    #make LOC=sram  TARGET=mini_bm
    #make LOC=sram  TARGET=gpio
    #make LOC=sram  TARGET=watchdog
    #make LOC=sram  TARGET=i2c
    #make LOC=sram  TARGET=i2c-slave
    #make LOC=sram  TARGET=axidma
    #make LOC=sram  TARGET=spi_nor
    #make LOC=sram  TARGET=spi_nand
    #make LOC=sram  TARGET=sdio
    #make LOC=sram  TARGET=regs
    #make LOC=sram  #by zxf
    make LOC=sram  TARGET=ddr
    #make LOC=sram  TARGET=ahbdma
    #make LOC=sram  TARGET=rtc
    #make LOC=sram  TARGET=stc
    #make LOC=sram  TARGET=i2s_pts
    #make LOC=sram  TARGET=sci
    #make LOC=sram  TARGET=mipi
    #make LOC=sram  TARGET=apts
    #make LOC=sram  TARGET=pmu
    #make LOC=sram  TARGET=clock_gate
    #make LOC=sram  TARGET=slave_boot
    #make LOC=sram  TARGET=l2c EN_L2C=0
    #make clean
    #make LOC=sram  TARGET=l2c EN_L2C=1
    #make clean
    #make LOC=sram  TARGET=mini_bm CPU_TYPE=ck810
elif [ $1 = "a" ]
then
    make
elif [ $1 = "c" ]
then
    make clean
elif [ $1 = "ca" ]
then
    make clean
    rm *.elf *.bin *.asm *.map
else
    echo "wrong args."
    cmd_help
fi
