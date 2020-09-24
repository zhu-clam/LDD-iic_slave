/*
 * boot.h
 *
 *  Created on: Oct 9, 2018
 *      Author: cn0971
 */

#ifndef BOOT_BOOT_H_
#define BOOT_BOOT_H_

#define MAGIC_NUM   0x424F5941

#define NOR_BOOT_ADDR       0x00   //the offset of second boot binary in spi nor flash
#define NAND_BOOT_ADDR      0x00    //the offset of second boot binary in spi nand flash
#define SD_CARD_BOOT_ADDR   66      //sector addr, and one sector size is 512

#define SPL_MAX_LEN     0x1E000  //max is 120k
#define MAX_DOWNLOAD_DATA_LEN       SPL_MAX_LEN   //byte
#define SRAM_START_ADDRESS  0xF0000000
#define SPL_MAX_ADDRESS     (SRAM_START_ADDRESS+SPL_MAX_LEN)

/* #define CONFIG_UART_BOOT_SEL */
#define CONFIG_UART_BOOT_SEL    0
#define BOOT_SEL_ADDR    (SYS_CTL_BASE + 0x10)
#define CHIP_ID_ADDR     (SYS_CTL_BASE + 0x20)
#define BOOT_MODE_ADDR   (SYS_CTL_BASE + 0x30)
#define PCIE_BOOT_PC     (SYS_CTL_BASE + 0xa0)

#define BOOT_MODE_SPI_NOR           0x00
#define BOOT_MODE_SPI_NAND_2K_64    0x01
#define BOOT_MODE_SPI_NAND_2K_128   0x02
//#define BOOT_MODE_SPI_NAND_4K_218   0x03
#define BOOT_MODE_SPI_NAND_4K_256   0x04
#define BOOT_MODE_SD                0x05
#define BOOT_MODE_NAND_2K_64        0x06
//#define BOOT_MODE_NAND_2K_128       0x07
#define BOOT_MODE_NAND_4K_218       0x08
//#define BOOT_MODE_NAND_4K_256       0x09
#define BOOT_MODE_NAND_8K_448       0x0a
#define BOOT_MODE_UART              0x0b
#define BOOT_MODE_PCIE              0x0c

typedef struct second_boot_header{
    unsigned int magic_num;
    unsigned int reserve;
    unsigned int data_size;
    unsigned int load_addr;
    unsigned int entry_point;
    unsigned int crc16;
}sb_header;

unsigned short check_sum(unsigned char *data, unsigned int len);
u32 get_bootmode(void);
int pcie_boot(void);
#endif /* BOOT_BOOT_H_ */
