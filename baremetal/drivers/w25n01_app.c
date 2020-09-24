/*
 * winbond, w25n01, Application program of SPI NAND flash
 *
 */

#include "misc.h"
#include <string.h>
#include "spi_nand.h"

extern int spi_nand_int_mode;

// Block#6, U-Boot offset in NAND flash
// #define  FLASH_TARGET_ADDR  0x180000 /* Block#6 -> Reserve 6*64K = 384KB*/

// Baremetal test space
#define  FLASH_TARGET_ADDR  0x7d00000 /* Offset 125MB, Block#1000 */

/*
 * Simple flash ID test
 */
CK_UINT8 FlashID_Test(void)
{
    CK_UINT32 flash_id = 0;

    /* Read flash id */
    CMD_RDID(&flash_id);

    /* Compare to expected value */
    if (flash_id != FlashID) {
        printf("\tCheck SPI NAND flash ID(0x%x, expect 0x%x) --- FAIL\n", flash_id, FlashID);
        return FALSE;
    }

    printf("\tCheck SPI NAND flash ID(0x%x) --- PASS\n", flash_id);
    return TRUE;
}

/*
 * Simple flash read/write test
 */
void FlashReadWrite_Test(void)
{
    CK_UINT32 flash_addr;
    CK_UINT32 trans_len;
    CK_UINT16 col_address = 0;
    CK_UINT8 status = 0;
    CK_UINT16 i;
    ReturnMsg result;
    CK_UINT8 buff_wr[SPI_NAND_TEST_LEN] = {
        0x27, 0x05, 0x19, 0x56, 0xE6, 0x3F, 0xF0, 0x46,
        0x5A, 0xB4, 0x9A, 0xD8, 0x00, 0x01, 0x86, 0x04,
        0x17, 0xA0, 0x00, 0x00, 0x17, 0xA0, 0x01, 0x80,
        0x34, 0xB4, 0xD1, 0x84, 0x11, 0x13, 0x05, 0x00,
        0x55, 0x2D, 0x42, 0x6F, 0x6F, 0x74, 0x20, 0x32,
        0x30, 0x31, 0x36, 0x2E, 0x30, 0x37, 0x2D, 0x67,
        0x35, 0x31, 0x37, 0x61, 0x31, 0x38, 0x34, 0x2D,
        0x64, 0x69, 0x72, 0x74, 0x79, 0x20, 0x66, 0x6F
    };
    CK_UINT8 buff_rd[SPI_NAND_TEST_LEN] = {0};

    /* Assign initial condition */
    flash_addr = FLASH_TARGET_ADDR;
    trans_len = SPI_NAND_TEST_LEN;

    /* Clear the block protection bit*/
    CMD_GET_FEATURE(0xa0, &status);
    if (status & 0x38) {
        printf("Clear SPI NAND block protection bit\n");
        CMD_SET_FEATURE(0xa0, (status & 0xc7));
    }

    /* Program data to flash memory */
    CMD_PP_LOAD(flash_addr, buff_wr, trans_len, 0);
    CMD_PROGRAM_EXEC(flash_addr);
#if CK_SPI_M_NAND_DEBUG
    CMD_READ(flash_addr);
    col_address = 0;
    CMD_READ_CACHE(col_address, buff_rd, trans_len, 0);
    printf("Dump data before erasing...\n");
    for (i = 0; i < (trans_len); i++) {
        if (i % 8 == 0)
            printf("\n");
        printf("%d ", buff_rd[i]);
    }
    printf("\n");
#endif

    /* Erase 128K bytes of flash memory
     * Note: It needs to erase dirty sector before program
     */
    result = CMD_BE(flash_addr);
    if (result == Flash_Success) {
        /* Check erase result, should be 0b1*/
        /* Read flash memory data to memory buffer */
        CMD_READ(flash_addr);
        col_address = 0;
        CMD_READ_CACHE(col_address, buff_rd, trans_len, 0);
#if CK_SPI_M_NAND_DEBUG
        printf("Dump data after erasing...");
#endif
        /* Compare original data and flash data */
        for (i = 0; i < (trans_len); i++) {
#if CK_SPI_M_NAND_DEBUG
            printf("--0x%x\n", buff_rd[i]);
#endif
            if (buff_rd[i] != 0xff) {
                result = Flash_DataInvalid;
                printf("Block erase result error: WDATA[%d]=0x%x, RDATA=0x%x\n\r\n",
                    i, 0xff, buff_rd[i]);
            }
        }
    }

    if (result != Flash_Success) {
        printf("\tErase SPI NAND flash block --- FAIL (0x%x)\n", result);
    } else {
        printf("\tErase SPI NAND flash block --- PASS\n");
    }

    /* Program data to flash memory */
    CMD_PP_LOAD(flash_addr, buff_wr, trans_len, 0);
    CMD_PROGRAM_EXEC(flash_addr);

    /* Read flash memory data to memory buffer */
    CMD_READ(flash_addr);
    col_address = 0;
    CMD_READ_CACHE(col_address, buff_rd, trans_len, 0);
    result = Flash_Success;
    /* Compare original data and flash data */
    for (i = 0; i < (trans_len); i++) {
        if (buff_wr[i] != buff_rd[i]) {
            result = Flash_DataInvalid;
            printf("Single IO read data error: WDATA[%d]=0x%x, RDATA=0x%x\n\r\n",
                i, buff_wr[i], buff_rd[i]);
        }
    }

    if (result != Flash_Success) {
        printf("\tSPI NAND flash single IO r/w --- FAIL (0x%x)\n", result);
    } else {
        printf("\tSPI NAND flash single IO r/w --- PASS\n");
    }

    /* Dual IO */
    memset(&buff_rd[0], 0x00, sizeof(buff_rd));
    CMD_READ_CACHE2(col_address, buff_rd, trans_len, 0);
    /* Compare original data and flash data */
    result = Flash_Success;
    for (i = 0; i < (trans_len); i++) {
        if (buff_wr[i] != buff_rd[i]) {
            result = Flash_DataInvalid;
            printf("Dual IO read data error: WDATA[%d]=0x%x, RDATA=0x%x\n\r\n",
                i, buff_wr[i], buff_rd[i]);
        }
    }

    if (result != Flash_Success) {
        printf("\tSPI NAND flash dual IO r/w --- FAIL (0x%x)\n", result);
    } else {
        printf("\tSPI NAND flash dual IO r/w --- PASS\n");
    }
#if 0
    /* Enable QE */
    if (IsFlashQIO() != TRUE) {
        CMD_GET_FEATURE(0xb0, &status);
        CMD_SET_FEATURE(0xb0, status | FLASH_QE_MASK);
    }
    /* Quad IO */
    memset(&buff_rd[0], 0x00, sizeof(buff_rd));
    CMD_READ_CACHE4(col_address, buff_rd, trans_len, 0);
    /* Compare original data and flash data */
    result = Flash_Success;
    for (i = 0; i < (trans_len); i++) {
        if (buff_wr[i] != buff_rd[i]) {
            result = Flash_DataInvalid;
            printf("Quad IO read data error: WDATA[%d]=0x%x, RDATA=0x%x\n\r\n",
                i, buff_wr[i], buff_rd[i]);
        }
    }

    if (result != Flash_Success) {
        printf("\tSPI NAND flash quad IO r/w --- FAIL (0x%x)\n", result);
    } else {
        printf("\tSPI NAND flash quad IO r/w --- PASS\n");
    }

    /* Disable QE */
    if (IsFlashQIO() == TRUE) {
        CMD_SET_FEATURE(0xb0, status & (~FLASH_QE_MASK));
    }
#endif
    /* Erase 128K sector of flash memory */
    CMD_BE(flash_addr);
}

static void Run_w25n01() {
    /* 01 */
#if CONFIG_IS_ASIC
    printf("\nSet SPI controller to work in mode 3 and 25MHz\n");
#else
    printf("\nSet SPI controller to work in mode 3 and 500KHz\n");
#endif
    Initial_Spi();
    SendByte(FLASH_CMD_RESET, SIO);
    /* Simple test: flash ID */
    FlashID_Test();
    /* Simple test: flash read / write */
    FlashReadWrite_Test();

    /* 02 */
    printf("\nSet SPI controller to work in mode 0 and 2.5MHz\n");
    write_mreg32(SPI_GCTL, read_mreg32(SPI_GCTL) & (~SPI_SPIEn));
    /* Mode 0, 2500KHz */
    write_mreg32(SPI_CCTL, (SPI_Length8 | SPI_MSBFirst | SPI_CKPLL | SPI_CKPHH));
    write_mreg32(SPI_SCSR, 0xff); // De-select All
    write_mreg32(SPI_SPBRG, SPI_DEFAULT_FREQ / 2500000); // 2.5MHz
    write_mreg32(SPI_GCTL, read_mreg32(SPI_GCTL) | SPI_SPIEn);
    FlashReadWrite_Test();

    /* 03 */
    printf("\nSet SPI controller to work in mode 0 and 8KHz\n");
    write_mreg32(SPI_GCTL, read_mreg32(SPI_GCTL) & (~SPI_SPIEn));
    /* Mode 0, 8000Hz */
    write_mreg32(SPI_CCTL, (SPI_Length8 | SPI_MSBFirst | SPI_CKPLL | SPI_CKPHH));
    write_mreg32(SPI_SCSR, 0xff); // De-select All
    write_mreg32(SPI_SPBRG, SPI_DEFAULT_FREQ / 8000);
    write_mreg32(SPI_GCTL, read_mreg32(SPI_GCTL) | SPI_SPIEn);
    FlashReadWrite_Test();

    SPINAND_unregister_isr();
}

/*
 * Main Program
 */
void SPI_Master_w25n01_App()
{
    printf("\tStart SPI NAND flash test . . .\n");
    printf("\nRun test with interrupt disabled:\n");
    spi_nand_int_mode = 0;
    Run_w25n01();

    printf("\nRun test with interrupt enabled:\n");
    spi_nand_int_mode = 1;
    Run_w25n01();

    printf("\n\tSPI NAND flash test finish . . .\n");
}
