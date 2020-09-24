/*****************************************************************************
 *
 *  Copyright (C) : 2018 VeriSilicon.
 *
 *  Author: Chunming Li
 *  Mail:   chunming.li@verisilicon.com
 *  Date:   Mar 26 2018
 *
 *****************************************************************************/

Project: polaris

Description:  Baremetal test for polaris project.

OS:         Ubuntu 16.04
Toolchain:  csky-linux-gnuabiv2-tools-x86_64-glibc-linux-4.9.56-20181101.tar.gz

Steps:
    - Install C-SKY cross compiler and export it to global path.

        Download the toolchain like:
            csky-linux-gnuabiv2-tools-x86_64-glibc-linux-4.9.56-20181101.tar.gz
	
        export PATH=[your_csky_toolchain_path]:$PATH
    -   Enter Baremetal folder and use "make" to get bm-test.elf.
    -   Open "C-Sky Debugger Server" and connected with target board.
    -   Open console and configure UART speed(baud) to 115200.
    -   Configure "gdb.init" with your own environment.
    -   Run command "csky-linux-gnuabiv2-gdb -x gdb.init"
    -   Run command "c" in GDB command line.
    -   You can find test menu in UART like this:
            Bare-metal test begin ...

                please input test module ID:
                0 -- Synopsys UART
                1 -- C-SKY Interrupt Controller
                2 -- Synopsys AHB DMA Controller
                3 -- Synopsys GPIO Controller
                4 -- DesignWare DW_apb_timers
                5 -- Synopsys Watchdog Timer
                6 -- Synopsys I2C
                7 -- VSI AXI DMA Controller
                8 -- VSI RTC
                9 -- VSI SPI controller with GD25Q128 NOR flash
                a -- VSI SPI controller with w25n01 NAND flash
                b -- Synopsys SDIO Controller
                c -- Synopsys USB DWC3 Device Mode
                d -- Rest IP register access
                >
                
    -   Put in the item you want to test and then you can find the result.