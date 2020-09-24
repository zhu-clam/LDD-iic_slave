#include "ck810.h"
#include "datatype.h"
#include "intc.h"
#include "spi_nor.h"
#include "axidma.h"
#include "cache.h"
#include "misc.h"
#include <string.h>

#define M2M_TEST_LEN    64

#define LLI_ADDR        0x10000000
#define M2M_TEST_SRC    0x20000000
#define M2M_TEST_DEST   0x30000000

/**
 * Check result, length = M2M_TEST_LEN
*/
bool check_result(CK_UINT32 src_addr, CK_UINT32 dst_addr, CK_UINT32 length)
{
    CK_UINT32 cnt;
    CK_UINT8 test;
    for (cnt = 0; cnt < length; cnt ++) {
        test = read_mreg8(src_addr + cnt);
        if (test != read_mreg8(dst_addr + cnt)) {
            printf("error at:%d\t (WDATA=0x%x, RDATA=0x%x)\n", cnt, test, read_mreg8(dst_addr + cnt));
            break;
        }
    }
    return (cnt == length)?true:false;
}

void CK_AXIDMA_ID_REG_Test();
/**
 * Test memory to memory
 * **/
void AXIDMA_M2M_Test(CK_UINT32 src_addr, CK_UINT32 dst_addr,
        struct axi_dma_info axi, CK_UINT32 length)
{
    memset((void*)dst_addr, 0, M2M_TEST_LEN);

    AXI_DMA_TRAN_INIT(axi, src_addr, dst_addr, length);

    AXI_DMA_TRANS(axi);

    if (check_result(src_addr, dst_addr, length)) {
        printf("src_msize:%d dst_msize:%d src_width:%d dst_width:%d----PASS\n",
            axi.src_msize, axi.dst_msize, axi.src_width, axi.dst_width);
    } else {
        printf("src_msize:%d dst_msize:%d src_width:%d dst_width:%d----FAIL\n",
            axi.src_msize, axi.dst_msize, axi.src_width, axi.dst_width);
    }
}

/**
 * Test memory to memory (LLI)
*/
void AXIDMA_M2M_LLI_TEST(CK_UINT32 src_addr, CK_UINT32 dst_addr,
        struct axi_dma_info axi, CK_UINT32 length)
{
    memset((void*)dst_addr, 0, M2M_TEST_LEN);

    struct axi_dma_lli* LLI1 = (void*)LLI_ADDR;
    memset(LLI1, 0, sizeof(struct axi_dma_lli));

    struct axi_dma_lli* LLI2 = (void*)(LLI_ADDR + sizeof(struct axi_dma_lli));
    memset(LLI2, 0, sizeof(struct axi_dma_lli));

    LLI1->llp = (long)LLI2;

    AXI_DMA_TRAN_INIT_LLI(axi, src_addr, dst_addr, length);

    printf("Memory to memory transfer (LLI)\n");

    AXI_DMA_TRANS(axi);

    printf("Memory to memory transfer (LLI) Done.");

    if (check_result(src_addr, dst_addr, length))
        printf("----------PASS\n");
    else
        printf("----------FAIL\n");
}

/**
 * Test M2M (LLI)
*/
void CK_AXIDMA_Test()
{
    CK_AXIDMA_ID_REG_Test();

    CHOSE_TYPE(0); //polling
    printf("Test in polling mode!\n");

    //Init test memory area
    CK_UINT32 cnt;
    for (cnt = 0; cnt < M2M_TEST_LEN; cnt++)
        write_mreg8(M2M_TEST_SRC + cnt, cnt);

    struct axi_dma_info axi;
    int i, j;

    for (i = 1; i <= 4; i++) {
        axi.channel = i;

        printf("channel %d: change(src_msize:0-9) fix(dst_msize:%d, src_width:%d, dst_width:%d)\n",
               axi.channel, DST_MSIZE128, SRC_WIDTH16, DST_WIDTH32);
        axi.direction = CHx_M2M_DMAC;
        axi.dst_msize = DST_MSIZE128;
        axi.src_width = SRC_WIDTH16;
        axi.dst_width = DST_WIDTH32;
        for (j = SRC_MSIZE1; j <= SRC_MSIZE1024; j++) {
            axi.src_msize = j;
            AXIDMA_M2M_Test(M2M_TEST_SRC, M2M_TEST_DEST, axi, M2M_TEST_LEN);
        }

        printf("channel %d: change(dst_msize:0-9) fix(dst_msize:%d, src_width:%d, dst_width:%d)\n",
               axi.channel, DST_MSIZE128, SRC_WIDTH16, DST_WIDTH32);
        axi.direction = CHx_M2M_DMAC;
        axi.src_msize = SRC_MSIZE16;
        axi.src_width = SRC_WIDTH64;
        axi.dst_width = DST_WIDTH16;
        for (j = DST_MSIZE1; j <= DST_MSIZE1024; j++) {
            axi.dst_msize = j;
            AXIDMA_M2M_Test(M2M_TEST_SRC, M2M_TEST_DEST, axi, M2M_TEST_LEN);
        }

        //nonsupport src_width equal 512 or 256
        printf("channel %d: change(src_width:0-9) fix(dst_msize:%d, src_width:%d, dst_width:%d)\n",
               axi.channel, DST_MSIZE128, SRC_WIDTH16, DST_WIDTH32);
        axi.direction = CHx_M2M_DMAC;
        axi.src_msize = SRC_MSIZE1;
        axi.dst_msize = DST_MSIZE128;
        axi.dst_width = DST_WIDTH256;
        for (j = SRC_WIDTH8; j <= SRC_WIDTH128; j++){
            axi.src_width = j;
            AXIDMA_M2M_Test(M2M_TEST_SRC, M2M_TEST_DEST, axi, M2M_TEST_LEN);
        }

        printf("channel %d: change(src_width:0-9) fix(dst_msize:%d, src_width:%d, dst_width:%d)\n",
               axi.channel, DST_MSIZE128, SRC_WIDTH16, DST_WIDTH32);
        axi.direction = CHx_M2M_DMAC;
        axi.src_msize = SRC_MSIZE1024;
        axi.dst_msize = DST_MSIZE16;
        axi.src_width = SRC_WIDTH64;
        for (j = DST_WIDTH8; j <= DST_WIDTH512; j++){
            axi.dst_width = j;
            AXIDMA_M2M_Test(M2M_TEST_SRC, M2M_TEST_DEST, axi, M2M_TEST_LEN);
        }
		
        printf("channel %d: fix(src_width:%d) fix(dst_msize:%d, src_width:%d, dst_width:%d)\n",
              SRC_MSIZE4, DST_MSIZE4, SRC_WIDTH8, DST_WIDTH8);
        axi.direction = CHx_M2M_DMAC;
        axi.src_msize = SRC_MSIZE4;
        axi.dst_msize = DST_MSIZE4;
        axi.src_width = SRC_WIDTH8;
		axi.dst_width = DST_WIDTH8;
		AXIDMA_M2M_Test(M2M_TEST_SRC, M2M_TEST_DEST, axi, M2M_TEST_LEN);

        printf("---------------------------------------------\n");
    }

    printf("Memory to memory cover test finish!\n");

    CHOSE_TYPE(1);  //interrupt
    printf("Test in interrupt mode!\n");

    axi.channel = 1;
    axi.direction = CHx_M2M_DMAC;
    axi.src_msize = SRC_MSIZE1024;
    axi.dst_msize = DST_MSIZE64;
    axi.src_width = SRC_WIDTH16;
    axi.dst_width = DST_WIDTH8;
    AXIDMA_M2M_LLI_TEST(M2M_TEST_SRC, M2M_TEST_DEST, axi ,M2M_TEST_LEN);
}

