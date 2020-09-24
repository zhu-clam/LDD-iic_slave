
#include "datatype.h"
#include "misc.h"
#include "gd25q128.h"
#include "spi_nor.h"
#include <string.h>

#define NOR_TARGET_ADDR     0xFFC000    // MAX - 16K
#define M2M_TEST_SRC    0xF0040000
#define M2M_TEST_DEST   0xF0050000

extern int spi_nor_int_mode;
extern void spi_read_byte_dma_x4(CK_UINT32 dst_data, CK_UINT32 length,
                       CK_UINT32 offset, CK_UINT8 PortN);
void spi_write_byte_dma(CK_UINT32 dst_mem, CK_UINT32 data_length, CK_UINT32 addr, CK_UINT8 PortN);
void spi_read_byte_dma(CK_UINT32 dst_data, CK_UINT32 length, CK_UINT32 offset, CK_UINT8 PortN);

/********************************************************
* Parameter PortN is not used.
*********************************************************/
void Run_GD25Q128(CK_UINT8 PortN)
{
    CK_UINT8 dst_mem[NOR_TEST_LEN] = {
        0x27, 0x05, 0x19, 0x56, 0xE6, 0x3F, 0xF0, 0x46,
        0x5A, 0xB4, 0x9A, 0xD8, 0x00, 0x01, 0x86, 0x04,
        0x17, 0xA0, 0x00, 0x00, 0x17, 0xA0, 0x01, 0x80,
        0x34, 0xB4, 0xD1, 0x84, 0x11, 0x13, 0x05, 0x00,
        0x55, 0x2D, 0x42, 0x6F, 0x6F, 0x74, 0x20, 0x32,
        0x30, 0x31, 0x36, 0x2E, 0x30, 0x37, 0x2D, 0x67,
        0x35, 0x31, 0x37, 0x61, 0x31, 0x38, 0x34, 0x2D,
        0x64, 0x69, 0x72, 0x74, 0x79, 0x20, 0x66, 0x6F
    };
    CK_UINT8 read_mem[NOR_TEST_LEN] = {0};
    CK_UINT32 id;
    CK_UINT32 i;

    spi_init(PortN);
    printf("\nSPI controller work in %dHz\n", QSPI_DEFAULT_FREQ/read_mreg32(SPI_SPBRG));

    // Manufacturer ID + Memory Type + Capacity
    id = SPI_READID(PortN);
    if (id != NOR_FLASH_ID) {
        printf("\tCheck SPI NOR flash ID(0x%x, expect 0x%x) --- FAIL\n",
            id, NOR_FLASH_ID);
    } else {
        printf("\tCheck SPI NOR flash ID(0x%x) --- PASS\n", id);
    }

    id = SPI_READMID(PortN);
    printf("\t\tMID is 0x%x\n", id);

    flash_checkbusy(PortN);
    spi_erase_sector(PortN, NOR_TARGET_ADDR);   // Sector 4K
    // Then all data should be 0b1
    spi_read_byte(read_mem, NOR_TEST_LEN, NOR_TARGET_ADDR, PortN);
    for (i = 0; i < NOR_TEST_LEN; i++) {
        if (read_mem[i] != 0xff) {
            printf("\tErase SPI NOR flash sector (got [%d]=0x%x) --- FAIL\n",
                i, read_mem[i]);
            break;
        }
    }
    if (i >= NOR_TEST_LEN)
        printf("\tErase SPI NOR flash sector --- PASS\n");

    spi_write_byte(dst_mem, NOR_TEST_LEN, NOR_TARGET_ADDR, PortN); // Normal mode
    spi_write_byte(dst_mem, NOR_TEST_LEN, NOR_TARGET_ADDR + NOR_TEST_LEN, PortN); // Dual mode
    spi_write_byte(dst_mem, NOR_TEST_LEN, NOR_TARGET_ADDR + NOR_TEST_LEN * 2, PortN); // Fast dual
    // spi_write_byte(dst_mem, NOR_TEST_LEN, NOR_TARGET_ADDR + NOR_TEST_LEN * 3, PortN); // Quad mode

    // Normal mode
    memset(&read_mem[0], 0x00, sizeof(read_mem));
    spi_read_byte(read_mem, NOR_TEST_LEN, NOR_TARGET_ADDR, PortN);
    for (i = 0; i < NOR_TEST_LEN; i++) {
        if (dst_mem[i] != read_mem[i]) {
            printf("\tNormal read SPI NOR flash (WDATA=0x%x, RDATA=0x%x) --- FAIL\n" ,
                dst_mem[i], read_mem[i]);
            break;
        }
    }
    if (i >= NOR_TEST_LEN)
        printf("\tNormal write/read SPI NOR flash --- PASS\n");

    memset(&read_mem[0], 0x00, sizeof(read_mem));
    spi_dual_read_byte(read_mem, NOR_TEST_LEN, NOR_TARGET_ADDR + NOR_TEST_LEN, PortN);
    for (i = 0; i < NOR_TEST_LEN; i++) {
        if (dst_mem[i] != read_mem[i]) {
            printf("\tDual I/O fast read SPI NOR flash (WDATA=0x%x, RDATA=0x%x) --- FAIL\n" ,
                dst_mem[i], read_mem[i]);
            break;
        }
    }
    if (i >= NOR_TEST_LEN)
        printf("\tDual I/O fast read SPI NOR flash --- PASS\n");

    memset(&read_mem[0], 0x00, sizeof(read_mem));
    spi_fast_read_byte(read_mem, NOR_TEST_LEN, NOR_TARGET_ADDR + NOR_TEST_LEN * 2, PortN);
    for (i = 0; i < NOR_TEST_LEN; i++) {
        if (dst_mem[i] != read_mem[i]) {
            printf("\tDual Output fast read(0x3b) SPI NOR flash (WDATA=0x%x, RDATA=0x%x) --- FAIL\n" ,
                    dst_mem[i], read_mem[i]);
            break;
        }
    }
    if (i >= NOR_TEST_LEN)
        printf("\tDual Output fast read(0x3b) SPI NOR flash --- PASS\n");

    BOOL ret;
    ret = spi_quad_enable(PortN);
    if (ret) {
        spi_quad_write_byte(dst_mem, NOR_TEST_LEN, NOR_TARGET_ADDR + NOR_TEST_LEN * 3, PortN); // Write data with Quad mode
        memset(&read_mem[0], 0x00, sizeof(read_mem));
        spi_quad_fast_read_byte(read_mem, NOR_TEST_LEN, NOR_TARGET_ADDR + NOR_TEST_LEN * 3, PortN);
        for (i = 0; i < NOR_TEST_LEN; i++) {
            if (dst_mem[i] != read_mem[i]) {
                printf("\tQuad Output fast read(0x6b) SPI NOR flash (WDATA=0x%x, RDATA=0x%x) --- FAIL\n" ,
                    dst_mem[i], read_mem[i]);
                break;
            }
        }
        if (i >= NOR_TEST_LEN)
            printf("\tQuad Output fast write(0x32)/read(0x6b) SPI NOR flash --- PASS\n");
        ret = spi_quad_disable(PortN);
    }
    if (spi_nor_int_mode) {
        spi_unregister_isr();
    }
}

void AXI_DMA_SPI_TEST(CK_UINT8 PortN){
    CK_UINT32 cnt;
    for(cnt = 0; cnt < NOR_TEST_LEN; cnt++)
        write_mreg8(M2M_TEST_SRC + cnt, cnt);

    memset((void *)M2M_TEST_DEST, 0, NOR_TEST_LEN);

    CK_UINT32 id;

    spi_init(PortN);
    printf("\nSPI controller work in %dHz\n", QSPI_DEFAULT_FREQ/read_mreg32(SPI_SPBRG));

    // Manufacturer ID + Memory Type + Capacity
    id = SPI_READID(PortN);
    if (id != NOR_FLASH_ID) {
        printf("\tCheck SPI NOR flash ID(0x%x, expect 0x%x) --- FAIL\n",
            id, NOR_FLASH_ID);
    } else {
        printf("\tCheck SPI NOR flash ID(0x%x) --- PASS\n", id);
    }

    id = SPI_READMID(PortN);
    printf("\t\tMID is 0x%x\n", id);

    flash_checkbusy(PortN);
    spi_erase_sector(PortN, NOR_TARGET_ADDR);   // Sector 4K

    // Then all data should be 0b1
    spi_read_byte_dma(M2M_TEST_DEST, NOR_TEST_LEN, NOR_TARGET_ADDR, PortN);
    for (cnt = 0; cnt < NOR_TEST_LEN; cnt++) {
        if (read_mreg8(M2M_TEST_DEST + cnt) != 0xff) {
            printf("\tErase SPI NOR flash sector (got [%d]=0x%x) --- FAIL\n",cnt , read_mreg8(M2M_TEST_DEST + cnt));
            break;
        }
    }
    if (cnt >= NOR_TEST_LEN)
        printf("\tErase SPI NOR flash sector --- PASS\n");

    memset((void *)M2M_TEST_DEST, 0, NOR_TEST_LEN);

    spi_write_byte_dma(M2M_TEST_SRC, NOR_TEST_LEN, NOR_TARGET_ADDR, PortN); // Normal mode

    spi_read_byte_dma(M2M_TEST_DEST, NOR_TEST_LEN, NOR_TARGET_ADDR, PortN);
    for (cnt = 0; cnt < NOR_TEST_LEN; cnt++) {
        if (read_mreg8(M2M_TEST_SRC + cnt) != read_mreg8(M2M_TEST_DEST + cnt)) {
            printf("\tNormal read SPI NOR flash %d (WDATA=0x%x, RDATA=0x%x) --- FAIL\n" ,read_mreg8(M2M_TEST_SRC + cnt), read_mreg8(M2M_TEST_DEST + cnt));
            break;
        }
    }
    if (cnt >= NOR_TEST_LEN)
        printf("\tNormal read SPI NOR flash --- PASS\n");

    BOOL ret;
    ret = spi_quad_enable(PortN);
    if (ret) {
        memset((void *)M2M_TEST_DEST, 0, NOR_TEST_LEN);
        spi_read_byte_dma_x4(M2M_TEST_DEST, NOR_TEST_LEN, NOR_TARGET_ADDR, PortN);

        for (cnt = 0; cnt < NOR_TEST_LEN; cnt++) {
            if (read_mreg8(M2M_TEST_SRC + cnt) != read_mreg8(M2M_TEST_DEST + cnt)) {
                printf("\tNormal read SPI NOR flash %d (WDATA=0x%x, RDATA=0x%x) --- FAIL\n" ,read_mreg8(M2M_TEST_SRC + cnt), read_mreg8(M2M_TEST_DEST + cnt));
                break;
            }
        }
        if (cnt >= NOR_TEST_LEN)
            printf("\tQuad Output fast read(0x6b) SPI NOR flash --- PASS\n");
        ret = spi_quad_disable(PortN);
    }

}

/*
 * Main Program
 */
void SPI_Master_GD25Q128_APP(CK_UINT8 PortN)
{
    printf("\tStart SPI NOR flash test . . .\n");
    printf("\nRun test with interrupt disabled:\n");
    spi_nor_int_mode = 0;
    Run_GD25Q128(PortN);

    printf("\nRun test with interrupt enabled:\n");
    spi_nor_int_mode = 1;
    Run_GD25Q128(PortN);

    printf("\n\tSPI NOR flash test done . . .\n");

    printf("\nAXI DMA & SPI TEST...\n");
    spi_nor_int_mode = 0;
    AXI_DMA_SPI_TEST(0);
    printf("\nAXI DMA & SPI TEST done...\n");
}
