/*
 * main.c - main function Modulation.
 *
 * Copyright (C):
 * Author:
 * Contributor:
 * Date:
 *
 */

#include "datatype.h"
#include "platform.h"
#include "uart.h"
#include <stdio.h>
#include "spi.h"
#include "spi_nor.h"
#include "spi_nand.h"
#include "timer.h"
#include "nand.h"
#include "boot.h"
#include "sdio_boot.h"

#define NFC_SW_RST_BIT      (0x1 << 16)
#define QSPI_SW_RST_BIT     (0x1 << 17)
#define SDIO0_SW_RST_BIT    (0x1 << 11)

extern void uart_console(void);

CK_Uart_Device consoleuart;

spi_nor nor;
spi_nand s_nand;
struct spi spi[4];
struct spi qspi;
nand_flash nfc;
/*
 * The main function of Bootrom
 */
int main ( void )
{
    u32 chip_id = 0;
    u32 boot_sel = 0;
    int boot_mode = 0;
    u32 sys_rest = 0;

    timer_init();

    consoleuart = CONFIG_TERMINAL_UART;
    CK_Uart_Open(CONFIG_TERMINAL_UART, 0);

    printf("\nHello BoYa Polaris ...\n");

    nor.spi = &qspi;
    s_nand.spi = &spi[3];
    nfc.base = (void *)NFC_BASE;

    boot_mode = get_bootmode();

    printf("boot mode : 0x%x \n", boot_mode);

    chip_id = read_mreg32(CHIP_ID_ADDR);
    boot_sel = read_mreg32(BOOT_SEL_ADDR);
    printf("boot sel : 0x%x \n", boot_sel);
    printf("chip id : 0x%x \n", chip_id);


    sys_rest = read_mreg32(0xf9700810);
    printf("BLK_SW_RST0 : 0x%x \n", sys_rest);
    sys_rest = read_mreg32(0xf9700814);
    printf("BLK_SW_RST1 : 0x%x \n", sys_rest);
    sys_rest = read_mreg32(0xf9700818);
    printf("BLK_SW_RST2 : 0x%x \n", sys_rest);
    sys_rest = read_mreg32(0xf970081c);
    printf("BLK_SW_RST3 : 0x%x \n", sys_rest);

#ifdef CONFIG_DEBUG
    printf("release all IP reset \n");
    write_mreg32(0xf9700810, 0xffffffff);
    write_mreg32(0xf9700814, 0xffffffff);
    write_mreg32(0xf9700818, 0xffffffff);
    write_mreg32(0xf970081c, 0xffffffff);
#else
    printf("release NFC QSPI SDIO0 \n");
    write_mreg32(0xf9700810, NFC_SW_RST_BIT
                           | QSPI_SW_RST_BIT
                           | SDIO0_SW_RST_BIT);
#endif
    sys_rest = read_mreg32(0xf9700810);
    printf("BLK_SW_RST0 : 0x%x \n", sys_rest);
    sys_rest = read_mreg32(0xf9700814);
    printf("BLK_SW_RST1 : 0x%x \n", sys_rest);
    sys_rest = read_mreg32(0xf9700818);
    printf("BLK_SW_RST2 : 0x%x \n", sys_rest);
    sys_rest = read_mreg32(0xf970081c);
    printf("BLK_SW_RST3 : 0x%x \n", sys_rest);

    switch (boot_mode)
    {
        case BOOT_MODE_SPI_NOR:
            printf("boot from spi nor... \n");
            spi_master_init(&qspi, 0x01, 2000000, (void*)QSPI_BASE);
            spi_nor_init(&nor);
            spi_nor_boot(&nor);
            break;
        case BOOT_MODE_SPI_NAND_2K_64:
            printf("boot from spi nand... \n");
            spi_master_init(&spi[3], 0x01, 2000000, (void*)SPI3_BASE);
            spi_nand_init(&s_nand);
            s_nand.info.page_size = 2048;
            s_nand.info.oob_size = 64;
            spi_nand_boot(&s_nand);
            break;
        case BOOT_MODE_SPI_NAND_2K_128:
            printf("boot from spi nand... \n");
            spi_master_init(&spi[3], 0x01, 2000000, (void*)SPI3_BASE);
            spi_nand_init(&s_nand);
            s_nand.info.page_size = 2048;
            s_nand.info.oob_size = 128;
            spi_nand_boot(&s_nand);
            break;
        case BOOT_MODE_SPI_NAND_4K_256:
            printf("boot from spi nand... \n");
            spi_master_init(&spi[3], 0x01, 2000000, (void*)SPI3_BASE);
            spi_nand_init(&s_nand);
            s_nand.info.page_size = 4096;
            s_nand.info.oob_size = 256;
            spi_nand_boot(&s_nand);
            break;
        case BOOT_MODE_SD:
            printf("boot from sd... \n");
            sd_card_init();
            sd_card_boot();
            break;
        case BOOT_MODE_NAND_2K_64:
            printf("boot from nand... \n");
            nand_init(&nfc);
            nfc_set_page_size(&nfc, 2048, 64);
            nand_boot(&nfc);
            break;
        case BOOT_MODE_NAND_4K_218:
            printf("boot from nand... \n");
            nand_init(&nfc);
            nfc_set_page_size(&nfc, 4096, 218);
            nand_boot(&nfc);
            break;
        case BOOT_MODE_NAND_8K_448:
            printf("boot from nand... \n");
            nand_init(&nfc);
            nfc_set_page_size(&nfc, 8192, 448);
            nand_boot(&nfc);
            break;
        case BOOT_MODE_UART:
            printf("boot from uart... \n");
            uart_console();
            break;
        case BOOT_MODE_PCIE:
            printf("boot from pcie... \n");
            pcie_boot();
            break;
        default:
            printf("invalid boot mode ... \n");
            break;
    }
    while (1) {

    }
    printf("\n");

    return 0;
}

