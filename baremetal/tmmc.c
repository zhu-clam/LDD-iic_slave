/*****************************************************************************
 *  File: tmmc.c
 *
 *  Descirption: contains the test support Synopsys DesignWare Cores
 *               Mobile Storage Host Controller - DWC_mshc – Product Code: A555-0.
 *
 *  Copyright (C) : 2018 VeriSilicon.
 *
 *  Author: Chunming Li
 *  Mail:   chunming.li@verisilicon.com
 *  Date:   Mar 5 2018
 *
 *****************************************************************************/

#include <string.h>
#include "ck810.h"
#include "misc.h"
#include "intc.h"
#include "syns_mmc.h"

#define TEST_DATA           0xFFEEDD00
#define CARD_ADDR           32
#define BLOCK_NUM           2
#define MEM_TEST_SRC        0xF0040000
#define MEM_TEST_DEST       0xF0050000
#define TRANSFER_TIMEOUT	1000

#if CONFIG_IS_ASIC
#define SDIO0_CLK       SDIO0_DEFAULT_FREQ
#define SDIO1_CLK       SDIO1_DEFAULT_FREQ
#define SDIOID_MAX          2
#else
#define SDIO0_CLK       30000000
#define SDIO1_CLK       30000000
#define SDIOID_MAX          1
#endif

static CK_UINT8  intc_test;

CKStruct_IRQHandler sdio_irqhandler;

static int CK_SDIO_Table[SDIOID_MAX][7] =
{

    {0, CK_SDIO0_BASEADDRESS, SDCARD, CK_INTC_SDIO0, 4, 1000000, MMC_VDD_32_33 | MMC_VDD_33_34},
    //{1, CK_SDIO1_BASEADDRESS, SDCARD, CK_INTC_SDIO1, 4, 1000000, MMC_VDD_32_33 | MMC_VDD_33_34},
#if CONFIG_IS_ASIC
    {1, CK_SDIO1_BASEADDRESS, SDCARD, CK_INTC_SDIO1, 4, 1000000, MMC_VDD_32_33 | MMC_VDD_33_34},
#endif
};

static inline void __cpu_idle(void)
{
  __asm__ __volatile__ ("wait\n");
  //__asm__ __volatile__ ("doze\n");
  //__asm__ __volatile__ ("stop\n");
}

static void CK_SDIO_ISR_Handler(u32 irq) {
    int i;
    u32 stat;
    u32 val;
    u32 ioaddr;
    u32 id;

    for(i = 0; i < SDIOID_MAX; i++) {
        id = CK_SDIO_Table[i][0];
        ioaddr = CK_SDIO_Table[i][1];

        stat = read_mreg32(ioaddr + SDHCI_INT_STATUS);
        // Check SD Card insert
        if ((stat & SDHCI_INT_CARD_INSERT) == SDHCI_INT_CARD_INSERT) {
            intc_test = 1;
            printf("\n\t\tCard insert into SDIO %d\n", id);
            write_mreg32(ioaddr + SDHCI_INT_STATUS, SDHCI_INT_CARD_INSERT);
        }

        // Check SD card remove
        if ((stat & SDHCI_INT_CARD_REMOVE) == SDHCI_INT_CARD_REMOVE) {
            intc_test = 1;
            printf("Card removed from SDIO %d\n", id);
            write_mreg32(ioaddr + SDHCI_INT_STATUS, SDHCI_INT_CARD_REMOVE);
        }

        // Check command complete
        if ((stat & SDHCI_INT_RESPONSE) == SDHCI_INT_RESPONSE) {
            printf("\n\t\t\tCommand Complete IRQ for SDIO %d\n", id);
            intc_test = 1;
            write_mreg32(ioaddr + SDHCI_INT_STATUS, SDHCI_INT_RESPONSE);
            // Disable Command Complete IRQ Signal
            val = read_mreg32(ioaddr + SDHCI_SIGNAL_ENABLE);
            write_mreg32(ioaddr + SDHCI_INT_STATUS, val & ~SDHCI_INT_RESPONSE);
        }
    }
}

void CK_SDIO_PIO_Single_Test(struct sdhci_host *host) {
    int err = 0;
    u32 flags = 0;
    int i;
    int start;
    u32 data;
    u32 result = 0;
    u32 stat;
    struct mmc_cmd cmd;

    printf("\n\t\tStart PIO Single Block Write/Read Test \n");

    /* Set Block Size as 512 bytes */
    sdhci_writew(host, BLOCK_SIZE, SDHCI_BLOCK_SIZE);
    /* Set single block, disable Auto CMD12, write mode */
    sdhci_writew(host, 0, SDHCI_TRANSFER_MODE);

    /* Send CMD16 to set Block length */
    /* Set block length = 512bytes for CMD16 */
    cmd.cmdidx = MMC_CMD_SET_BLOCKLEN;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = BLOCK_SIZE;
    err = mmc_send_cmd(host, &cmd, 0);

    if (err) {
        printf("CMD16 SET_BLOCKLEN fail\n");
        return;
    }

    /* Send CMD24 to write Single Block */
    /* Set block length = 512bytes for CMD24 */
    cmd.cmdidx = MMC_CMD_WRITE_SINGLE_BLOCK;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = CARD_ADDR;
    flags = SDHCI_CMD_DATA;
    err = mmc_send_cmd(host, &cmd, flags);

    if (err) {
        printf("CMD24 WRITE_SINGLE_BLOCK fail\n");
        return;
    }

    /* Write data into SDIO Buffer */
    for (i = 0; i < (BLOCK_SIZE / 4); i++) {
        sdhci_writel(host, TEST_DATA + i, SDHCI_BUFFER);
        udelay(1000);
    }

    start = 0;
    do {
		stat = sdhci_readl(host, SDHCI_INT_STATUS);
		if (stat & SDHCI_INT_DATA_END) {
            sdhci_writel(host, SDHCI_INT_DATA_END, SDHCI_INT_STATUS);
			break;
        }
        start += 1;
        udelay(1000);
	} while (((stat & SDHCI_INT_DATA_END) != SDHCI_INT_DATA_END) &&
		 (start < TRANSFER_TIMEOUT));


    if (start >= TRANSFER_TIMEOUT) {
        printf("Transfer not complete, Test Fail\n");
        return;
    }

    /* Set Block Size as 512 bytes */
    sdhci_writew(host, BLOCK_SIZE, SDHCI_BLOCK_SIZE);
    /* Set single block, disable Auto CMD12, read mode */
    sdhci_writew(host, SDHCI_TRNS_READ, SDHCI_TRANSFER_MODE);

    /* Send CMD16 to set Block length */
    /* Set block length = 512bytes for CMD16 */
    cmd.cmdidx = MMC_CMD_SET_BLOCKLEN;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = BLOCK_SIZE;
    err = mmc_send_cmd(host, &cmd, 0);

    if (err) {
        printf("CMD16 SET_BLOCKLEN fail\n");
        return;
    }

    /* Send CMD17 to read Single Block */
    cmd.cmdidx = MMC_CMD_READ_SINGLE_BLOCK;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = CARD_ADDR;
    flags = SDHCI_CMD_DATA;
    err = mmc_send_cmd(host, &cmd, flags);

    if (err) {
        printf("CMD17 READ_SINGLE_BLOCK fail\n");
        return;
    }

    start = 0;
    do {
		stat = sdhci_readl(host, SDHCI_INT_STATUS);
		if (stat & SDHCI_INT_DATA_AVAIL)
			break;
        start += 1;
        udelay(1000);
	} while (((stat & SDHCI_INT_DATA_AVAIL) != SDHCI_INT_DATA_AVAIL) &&
		 (start < TRANSFER_TIMEOUT));

    if (start >= TRANSFER_TIMEOUT) {
        printf("Read data timeout, Test Fail\n");
			return;
    }

    for (i = 0; i < (BLOCK_SIZE / 4); i++) {
        data = sdhci_readl(host, SDHCI_BUFFER);
        if (data != (TEST_DATA + i)) {
            printf("Read data 0x%x not equal expected data 0x%x\n", data, TEST_DATA + i);
            result = 1;
        }
        udelay(1000);
    }

    if (result)
        printf("\n\t\t\t ---FAIL\n");
    else
        printf("\n\t\t\t ---PASS\n");
}

void CK_SDIO_PIO_Multi_Test(struct sdhci_host *host) {
    int err = 0;
    u32 flags = 0;
    int i;
    int start;
    u32 data;
    u32 result = 0;
    u32 stat;
    u32 block = 0;
    u32 mask = 0;
    struct mmc_cmd cmd;

    printf("\n\t\tStart PIO Multi %d Blocks and Auto CMD23 enabled Write/Read Test\n", BLOCK_NUM);

    /* Set Block Size as 512 bytes */
    sdhci_writew(host, BLOCK_SIZE, SDHCI_BLOCK_SIZE);
    /* Set block count = BLOCK_NUM*/
    sdhci_writew(host, BLOCK_NUM, SDHCI_BLOCK_COUNT);
    /* Set multi block, enable Auto CMD23, write mode */
    sdhci_writew(host, SDHCI_TRNS_BLK_CNT_EN | SDHCI_TRNS_AUTO_CMD23 |
                    SDHCI_TRNS_MULTI, SDHCI_TRANSFER_MODE);
#if CK_SDIO_DEBUG
    printf("[%s:%d] mode=0x%x\n", __FUNCTION__, __LINE__, sdhci_readw(host, SDHCI_TRANSFER_MODE));
#endif
    /* Send CMD16 to set Block length */
    /* Set block length = 512bytes for CMD16 */
    cmd.cmdidx = MMC_CMD_SET_BLOCKLEN;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = BLOCK_SIZE;
    err = mmc_send_cmd(host, &cmd, 0);

    if (err) {
        printf("CMD16 SET_BLOCKLEN fail\n");
        return;
    }

    /* Send CMD25 to write Multi Block */
    /* Set block length = 512bytes for CMD25 */
    cmd.cmdidx = MMC_CMD_WRITE_MULTIPLE_BLOCK;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = CARD_ADDR;
    flags = SDHCI_CMD_DATA;
    err = mmc_send_cmd(host, &cmd, flags);

    if (err) {
        printf("CMD25 WRITE_MULTIPLE_BLOCK fail\n");
        return;
    }

    /* Write block data into SDIO Buffer */
    for (block = 0; block < BLOCK_NUM; block++) {
        for (i = 0; i < (BLOCK_SIZE / 4); i++) {
            sdhci_writel(host, TEST_DATA + i, SDHCI_BUFFER);
            udelay(1000);
        }

        if(block == (BLOCK_NUM - 1))
            mask = SDHCI_INT_DATA_END;
        else
            mask = SDHCI_INT_SPACE_AVAIL;

        start = 0;
        do {
            stat = sdhci_readl(host, SDHCI_INT_STATUS);
#if CK_SDIO_DEBUG
            printf("[%s:%d] stat=0x%x\n", __FUNCTION__, __LINE__, stat);
#endif
            if (stat & mask) {
                sdhci_writel(host, mask, SDHCI_INT_STATUS);
                break;
            }
            start += 1;
            udelay(1000);
        } while (((stat & mask) != mask) &&
            (start < TRANSFER_TIMEOUT));


        if (start >= TRANSFER_TIMEOUT) {
            printf("Transfer not complete, Test Fail\n");
            return;
        }
    }

    /* Set Block Size as 512 bytes */
    sdhci_writew(host, BLOCK_SIZE, SDHCI_BLOCK_SIZE);
    /* Set block count = BLOCK_NUM*/
    sdhci_writew(host, BLOCK_NUM, SDHCI_BLOCK_COUNT);
    /* Set multi block, disable Auto CMD12, read mode */
    sdhci_writew(host, SDHCI_TRNS_READ | SDHCI_TRNS_BLK_CNT_EN |
                SDHCI_TRNS_AUTO_CMD23 | SDHCI_TRNS_MULTI, SDHCI_TRANSFER_MODE);
#if CK_SDIO_DEBUG
    printf("[%s:%d] mode=0x%x\n", __FUNCTION__, __LINE__, sdhci_readw(host, SDHCI_TRANSFER_MODE));
#endif
    /* Send CMD16 to set Block length */
    /* Set block length = 512bytes for CMD16 */
    cmd.cmdidx = MMC_CMD_SET_BLOCKLEN;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = BLOCK_SIZE;
    err = mmc_send_cmd(host, &cmd, 0);

    if (err) {
        printf("CMD16 SET_BLOCKLEN fail\n");
        return;
    }

    /* Send CMD18 to read Multi Block */
    cmd.cmdidx = MMC_CMD_READ_MULTIPLE_BLOCK;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = CARD_ADDR;
    flags = SDHCI_CMD_DATA;
    err = mmc_send_cmd(host, &cmd, flags);

    if (err) {
        printf("CMD18 READ_MULTIPLE_BLOCK fail\n");
        return;
    }

    udelay(1000*100);

    for (block = 0; block < BLOCK_NUM; block++) {
        for (i = 0; i < (BLOCK_SIZE / 4); i++) {
            data = sdhci_readl(host, SDHCI_BUFFER);
            if (data != (TEST_DATA + i)) {
                printf("Read data 0x%x from block %d not equal expected data 0x%x\n", data, block, TEST_DATA + i);
                result = 1;
            }
            udelay(1000);
        }

        if(block == (BLOCK_NUM - 1))
            mask = SDHCI_INT_DATA_END;
        else
            mask = SDHCI_INT_DATA_AVAIL;

        start = 0;
        do {
            stat = sdhci_readl(host, SDHCI_INT_STATUS);
#if CK_SDIO_DEBUG
            printf("[%s:%d] stat=0x%x\n", __FUNCTION__, __LINE__, stat);
#endif
            if (stat & mask) {
                sdhci_writel(host, mask, SDHCI_INT_STATUS);
                break;
            }
            start += 1;
            udelay(1000);
        } while (((stat & mask) != mask) &&
            (start < TRANSFER_TIMEOUT));


        if (start >= TRANSFER_TIMEOUT) {
            printf("Transfer not complete, Test Fail\n");
            return;
        }
    }

    if (result)
        printf("\n\t\t\t ---FAIL\n");
    else
        printf("\n\t\t\t ---PASS\n");
}

void CK_SDIO_SDMA_Single_Test(struct sdhci_host *host) {
    int err = 0;
    u32 flags = 0;
    int i;
    int start;
    u32 data;
    u32 result = 0;
    u32 stat;
    u32 ctrl;
    struct mmc_cmd cmd;

    printf("\n\t\tStart SDMA Single Block Write/Read Test Addr=0x%x\n", MEM_TEST_SRC);

    // Init test memory area
    for(i = 0; i < (BLOCK_SIZE / 4); i++) {
        write_mreg32((MEM_TEST_SRC + i * 4), TEST_DATA + i);
    }

    /* Set system memory address = MEM_TEST_SRC */
    sdhci_writel(host, MEM_TEST_SRC, SDHCI_DMA_ADDRESS);
    /* Set Block size as 512bytes ,set SDMA buffer = 4k bytes */
    sdhci_writew(host, BLOCK_SIZE, SDHCI_BLOCK_SIZE);
    /* Set SDMA mode */
    ctrl = sdhci_readb(host, SDHCI_HOST_CONTROL);
    ctrl |= SDHCI_CTRL_SDMA;
    sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);
    /* Set single block, enable DMA, disable Auto CMD12, write mode */
    sdhci_writew(host, SDHCI_TRNS_DMA, SDHCI_TRANSFER_MODE);

    /* Send CMD16 to set Block length */
    /* Set block length = 512bytes for CMD16 */
    cmd.cmdidx = MMC_CMD_SET_BLOCKLEN;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = BLOCK_SIZE;
    err = mmc_send_cmd(host, &cmd, 0);

    if (err) {
        printf("CMD16 SET_BLOCKLEN fail\n");
        return;
    }

    /* Send CMD24 to write Single Block */
    /* Set block length = 512bytes for CMD24 */
    cmd.cmdidx = MMC_CMD_WRITE_SINGLE_BLOCK;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = CARD_ADDR;
    flags = SDHCI_CMD_DATA;
    err = mmc_send_cmd(host, &cmd, flags);

    if (err) {
        printf("CMD24 WRITE_SINGLE_BLOCK fail\n");
        return;
    }

    start = 0;
    do {
		stat = sdhci_readl(host, SDHCI_INT_STATUS);
		if (stat & (SDHCI_INT_DATA_END | SDHCI_INT_DMA_END)) {
            sdhci_writel(host, (SDHCI_INT_DATA_END | SDHCI_INT_DMA_END), SDHCI_INT_STATUS);
			break;
        }
        start += 1;
        udelay(1000);
	} while (!(stat & (SDHCI_INT_DATA_END | SDHCI_INT_DMA_END)) &&
		 (start < TRANSFER_TIMEOUT));


    if (start >= TRANSFER_TIMEOUT) {
        printf("Transfer not complete, Test Fail\n");
        return;
    }

#if CK_SDIO_DEBUG
    printf("\n\t\t start SDMA single block read to addr=0x%x", MEM_TEST_DEST);
#endif
    /* Set system memory address = MEM_TEST_DEST */
    sdhci_writel(host, MEM_TEST_DEST, SDHCI_DMA_ADDRESS);
    /* Set Block size as 512bytes ,set SDMA buffer = 4k bytes */
    sdhci_writew(host, BLOCK_SIZE, SDHCI_BLOCK_SIZE);
    /* Set SDMA mode */
    ctrl = sdhci_readb(host, SDHCI_HOST_CONTROL);
    ctrl |= SDHCI_CTRL_SDMA;
    sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);
    /* Set single block, enable DMA, disable Auto CMD12, write mode */
    sdhci_writew(host, SDHCI_TRNS_DMA | SDHCI_TRNS_READ, SDHCI_TRANSFER_MODE);

    /* Send CMD16 to set Block length */
    /* Set block length = 512bytes for CMD16 */
    cmd.cmdidx = MMC_CMD_SET_BLOCKLEN;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = BLOCK_SIZE;
    err = mmc_send_cmd(host, &cmd, 0);

    if (err) {
        printf("CMD16 SET_BLOCKLEN fail\n");
        return;
    }

    /* Send CMD17 to read Single Block */
    /* Set block length = 512bytes for CMD17 */
    cmd.cmdidx = MMC_CMD_READ_SINGLE_BLOCK;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = CARD_ADDR;
    flags = SDHCI_CMD_DATA;
    err = mmc_send_cmd(host, &cmd, flags);

    if (err) {
        printf("CMD17 READ_SINGLE_BLOCK fail\n");
        return;
    }

    start = 0;
    do {
		stat = sdhci_readl(host, SDHCI_INT_STATUS);
		if (stat & (SDHCI_INT_DATA_END | SDHCI_INT_DMA_END)) {
            sdhci_writel(host, (SDHCI_INT_DATA_END | SDHCI_INT_DMA_END), SDHCI_INT_STATUS);
			break;
        }
        start += 1;
        udelay(1000);
	} while (!(stat & (SDHCI_INT_DATA_END | SDHCI_INT_DMA_END)) &&
		 (start < TRANSFER_TIMEOUT));

    if (start >= TRANSFER_TIMEOUT) {
        printf("Read data timeout, Test Fail\n");
			return;
    }

    for (i = 0; i < (BLOCK_SIZE / 4); i++) {
        data = read_mreg32(MEM_TEST_DEST + i * 4);
        if (data != (TEST_DATA + i)) {
            printf("Read data 0x%x not equal expected data 0x%x\n", data, TEST_DATA + i);
            result = 1;
        }
        udelay(1000);
    }

    if (result)
        printf("\n\t\t\t ---FAIL\n");
    else
        printf("\n\t\t\t ---PASS\n");
}

void CK_SDIO_SDMA_Multi_Test(struct sdhci_host *host) {
    int err = 0;
    u32 flags = 0;
    int i;
    int start;
    u32 data;
    u32 result = 0;
    u32 stat;
    u32 ctrl;
    struct mmc_cmd cmd;

    printf("\n\t\tStart SDMA Multi %d Blocks and Auto CMD12 enabled Write/Read Test \n", BLOCK_NUM);

#if CK_SDIO_DEBUG
    printf("\n\t\tJJJ_DEBUG start SDMA multi block write to addr=0x%x", MEM_TEST_SRC);
#endif

    // Init test memory area
    for(i = 0; i < (BLOCK_SIZE * BLOCK_NUM / 4); i++) {
        write_mreg32((MEM_TEST_SRC + i * 4), TEST_DATA + i);
    }

    /* Set system memory address = MEM_TEST_SRC */
    sdhci_writel(host, MEM_TEST_SRC, SDHCI_DMA_ADDRESS);
    /* Set Block size as 512bytes ,set SDMA buffer = 4k bytes */
    sdhci_writew(host, BLOCK_SIZE, SDHCI_BLOCK_SIZE);
    /* Set SDMA mode */
    ctrl = sdhci_readb(host, SDHCI_HOST_CONTROL);
    ctrl |= SDHCI_CTRL_SDMA;
    sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);
    /* Set multi block, enable DMA, enable Auto CMD12, write mode */
    sdhci_writew(host, SDHCI_TRNS_DMA | SDHCI_TRNS_BLK_CNT_EN | SDHCI_TRNS_AUTO_CMD12 |
                    SDHCI_TRNS_MULTI, SDHCI_TRANSFER_MODE);
    /* Set block count = BLOCK_NUM*/
    sdhci_writew(host, BLOCK_NUM, SDHCI_BLOCK_COUNT);

    /* Send CMD16 to set Block length */
    /* Set block length = 512bytes for CMD16 */
    cmd.cmdidx = MMC_CMD_SET_BLOCKLEN;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = BLOCK_SIZE;
    err = mmc_send_cmd(host, &cmd, 0);

    if (err) {
        printf("CMD16 SET_BLOCKLEN fail\n");
        return;
    }

    /* Send CMD25 to write Single Block */
    /* Set block length = 512bytes for CMD25 */
    cmd.cmdidx = MMC_CMD_WRITE_MULTIPLE_BLOCK;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = CARD_ADDR;
    flags = SDHCI_CMD_DATA;
    err = mmc_send_cmd(host, &cmd, flags);

    if (err) {
        printf("CMD25 WRITE_SINGLE_BLOCK fail\n");
        return;
    }

    start = 0;
    do {
		stat = sdhci_readl(host, SDHCI_INT_STATUS);
		if (stat & (SDHCI_INT_DATA_END | SDHCI_INT_DMA_END)) {
            sdhci_writel(host, (SDHCI_INT_DATA_END | SDHCI_INT_DMA_END), SDHCI_INT_STATUS);
			break;
        }
        start += 1;
        udelay(1000);
	} while (!(stat & (SDHCI_INT_DATA_END | SDHCI_INT_DMA_END)) &&
		 (start < TRANSFER_TIMEOUT));


    if (start >= TRANSFER_TIMEOUT) {
        printf("Transfer not complete, Test Fail\n");
        return;
    }

#if CK_SDIO_DEBUG
    printf("\n\t\t start SDMA multi block read to addr=0x%x", MEM_TEST_DEST);
#endif
    /* Set system memory address = MEM_TEST_DEST */
    sdhci_writel(host, MEM_TEST_DEST, SDHCI_DMA_ADDRESS);
    /* Set Block size as 512bytes ,set SDMA buffer = 4k bytes */
    sdhci_writew(host, BLOCK_SIZE, SDHCI_BLOCK_SIZE);
    /* Set block count = BLOCK_NUM*/
    sdhci_writew(host, BLOCK_NUM, SDHCI_BLOCK_COUNT);
    /* Set SDMA mode */
    ctrl = sdhci_readb(host, SDHCI_HOST_CONTROL);
    ctrl |= SDHCI_CTRL_SDMA;
    sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);
    /* Set single block, enable DMA, disable Auto CMD12, write mode */
    sdhci_writew(host, SDHCI_TRNS_DMA | SDHCI_TRNS_BLK_CNT_EN | SDHCI_TRNS_AUTO_CMD12 |
                    SDHCI_TRNS_MULTI | SDHCI_TRNS_READ, SDHCI_TRANSFER_MODE);

    /* Send CMD16 to set Block length */
    /* Set block length = 512bytes for CMD16 */
    cmd.cmdidx = MMC_CMD_SET_BLOCKLEN;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = BLOCK_SIZE;
    err = mmc_send_cmd(host, &cmd, 0);

    if (err) {
        printf("CMD16 SET_BLOCKLEN fail\n");
        return;
    }

    /* Send CMD18 to read Multi Block */
    /* Set block length = 512bytes for CMD18 */
    cmd.cmdidx = MMC_CMD_READ_MULTIPLE_BLOCK;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = CARD_ADDR;
    flags = SDHCI_CMD_DATA;
    err = mmc_send_cmd(host, &cmd, flags);

    if (err) {
        printf("CMD18 READ_MULTIPLE_BLOCK fail\n");
        return;
    }

    start = 0;
    do {
		stat = sdhci_readl(host, SDHCI_INT_STATUS);
		if (stat & (SDHCI_INT_DATA_END | SDHCI_INT_DMA_END)) {
            sdhci_writel(host, (SDHCI_INT_DATA_END | SDHCI_INT_DMA_END), SDHCI_INT_STATUS);
			break;
        }
        start += 1;
        udelay(1000);
	} while (!(stat & (SDHCI_INT_DATA_END | SDHCI_INT_DMA_END)) &&
		 (start < TRANSFER_TIMEOUT));

    if (start >= TRANSFER_TIMEOUT) {
        printf("Read data timeout, Test Fail\n");
			return;
    }

    for (i = 0; i < (BLOCK_SIZE * BLOCK_NUM / 4); i++) {
        data = read_mreg32(MEM_TEST_DEST + i * 4);
        if (data != (TEST_DATA + i)) {
            printf("Read data 0x%x not equal expected data 0x%x\n", data, TEST_DATA + i);
            result = 1;
        }
        udelay(1000);
    }

    if (result)
        printf("\n\t\t\t ---FAIL\n");
    else
        printf("\n\t\t\t ---PASS\n");
}

void CK_SDIO_IRQ_Test(struct sdhci_host *host) {
    u32 val;
    u32 start;
    struct mmc_cmd cmd;

    printf("\n\t\tStart IRQ Trigger Test \n");

    intc_test = 0;
    // Enable Command Complete IRQ Signal
    val = sdhci_readl(host, SDHCI_SIGNAL_ENABLE);
    sdhci_writel(host, val | SDHCI_INT_RESPONSE, SDHCI_SIGNAL_ENABLE);

    // Send reset command to card
    cmd.cmdidx = MMC_CMD_GO_IDLE_STATE;
	cmd.cmdarg = 0;
	cmd.resp_type = MMC_RSP_NONE;

	sdhci_writel(host, cmd.cmdidx, SDHCI_ARGUMENT);
	sdhci_writew(host, SDHCI_MAKE_CMD(cmd.cmdarg, 0), SDHCI_COMMAND);

    start = 0;
    do {
		if (intc_test)
			break;
        start += 1;
        udelay(1000);
	} while ((!intc_test) && (start < TRANSFER_TIMEOUT));


    if (intc_test)
        printf("\n\t\t\t ---PASS\n");
    else
        printf("\n\t\t\t ---FAIL\n");
}

void CK_SDIO_Hotplug_Test(struct sdhci_host *host) {
    u32 stat;
    u32 timeout;
    u32 val;

    printf("\n\t\tStart SD card Hotplug Test\n");

    // disable remove/insert Card IRQ Signal
    val = sdhci_readl(host, SDHCI_SIGNAL_ENABLE);
    sdhci_writel(host, val & (~ (SDHCI_INT_CARD_INSERT | SDHCI_INT_CARD_REMOVE)), SDHCI_SIGNAL_ENABLE);

    printf("\n\t\t\t Now remove SD card\n");

    timeout = 600;
    do {
        stat = sdhci_readl(host, SDHCI_INT_STATUS);

        if (timeout-- > 0)
			mdelay(100);
        else {
			printf("Wait remove interrupt timeout, Test Fail\n");
			return;
		}
    } while (!(stat & SDHCI_INT_CARD_REMOVE));

    printf("\n\t\t\t Now insert SD card\n");

    timeout = 600;
    do {
        stat = sdhci_readl(host, SDHCI_INT_STATUS);

        if (timeout-- > 0)
            mdelay(100);
        else {
            printf("Wait insert interrupt timeout, Test Fail\n");
            return;
        }
    } while (!(stat & SDHCI_INT_CARD_INSERT));

    printf("\t\tSD card Hotplug Test\n \t\t\t ---PASS\n");
}

void CK_SDIO_Wakeup_Test(struct sdhci_host *host) {
    u32 value;
    u32 get;

    memset(&sdio_irqhandler,0,sizeof(PCKStruct_IRQHandler));
    // Request Interrupt ISR
    sdio_irqhandler.devname = "SDIO_Wakeup";
    sdio_irqhandler.irqid = CK_INTC_SDIO0_WAKEUP;
    sdio_irqhandler.priority = CK_INTC_SDIO0_WAKEUP;
    sdio_irqhandler.handler = CK_SDIO_ISR_Handler;
    sdio_irqhandler.bfast = FALSE;
    sdio_irqhandler.next = NULL;
    /* Register timer ISR */
    CK_INTC_RequestIrq(&sdio_irqhandler, AUTO_MODE);

    value = sdhci_readb(host, SDHCI_WAKE_UP_CONTROL);
    sdhci_writeb(host, SDHCI_WAKE_ON_INT | SDHCI_WAKE_ON_INSERT | SDHCI_WAKE_ON_REMOVE, SDHCI_WAKE_UP_CONTROL);
#if CK_SDIO_DEBUG
    printf("JJJ_DEBUG SDHCI_WAKE_UP_CONTROL=0x%x\n", sdhci_readb(host, SDHCI_WAKE_UP_CONTROL));
#endif

    value = sdhci_readl(host, SDHCI_INT_ENABLE);
    sdhci_writel(host, value | SDHCI_INT_CARD_INSERT | SDHCI_INT_CARD_REMOVE, SDHCI_INT_ENABLE);
#if CK_SDIO_DEBUG
    printf("JJJ_DEBUG SDHCI_INT_ENABLE=0x%x\n", sdhci_readl(host, SDHCI_INT_ENABLE));
#endif

    value = sdhci_readl(host, SDHCI_SIGNAL_ENABLE);
    sdhci_writel(host, value | SDHCI_INT_CARD_INSERT | SDHCI_INT_CARD_REMOVE, SDHCI_SIGNAL_ENABLE);
#if CK_SDIO_DEBUG
    printf("JJJ_DEBUG SDHCI_SIGNAL_ENABLE=0x%x\n", sdhci_readl(host, SDHCI_INT_ENABLE));
#endif

    printf("\n\t\tStart SD card Hotplug Test\n");

    sdhci_writel(host, 0xFFFFFFFF, SDHCI_INT_STATUS); // Clear all interrupt

    intc_test = 0;
    printf("\n\n\t- - - Ready to start wakeup test?...\n");
    printf("- - - [y/n] ");

    while(1){
        get = CK_WaitForReply();
        if((get == 1)) {
            printf("\n\n\t CPU enter idle state \n");
            __cpu_idle();

            if(intc_test)
                break;
        } else
        printf("\n\tPlease enter 'y' or 'n'   ");
    }
}
/*
 * main function of timer test program.
 */
void CK_SDIO_Test()
{
    int err;
    CK_UINT32 i;
    struct sdhci_host host;

    printf("\nSynopsys SDIO Controller Test. . . \n");

    for(i = 0; i < SDIOID_MAX; i++) {
        memset(&host,0,sizeof(struct sdhci_host));
        memset(&sdio_irqhandler,0,sizeof(PCKStruct_IRQHandler));

        host.id = CK_SDIO_Table[i][0];
        host.ioaddr = CK_SDIO_Table[i][1];
        host.card_type = CK_SDIO_Table[i][2];
        host.irq = CK_SDIO_Table[i][3];
        host.bus_width = CK_SDIO_Table[i][4];
        host.clock = CK_SDIO_Table[i][5];
        if (i == 0)
        	host.f_max = SDIO0_CLK;
        else
        	host.f_max = SDIO1_CLK;
        host.voltages = CK_SDIO_Table[i][6];
        host.has_init = 0;

        printf("\n\tStart Test SDIO %d with %s card %d bit mode\n", host.id,
                host.card_type ? "eMMC" : "SD", host.bus_width);

        // Request Interrupt ISR
        sdio_irqhandler.devname = "SDIO";
        sdio_irqhandler.irqid = host.irq;
        sdio_irqhandler.priority = host.irq;
        sdio_irqhandler.handler = CK_SDIO_ISR_Handler;
        sdio_irqhandler.bfast = FALSE;
        sdio_irqhandler.next = NULL;
        /* Register timer ISR */
        CK_INTC_RequestIrq(&sdio_irqhandler, AUTO_MODE);

        err = sdhci_setup_host(&host);

        if(err) {
            printf("\n\tSDIO %d setup fail, test Fail\n", host.id);
            break;
        }

        CK_SDIO_PIO_Single_Test(&host);
        CK_SDIO_PIO_Multi_Test(&host);
        CK_SDIO_SDMA_Single_Test(&host);
        CK_SDIO_SDMA_Multi_Test(&host);
        CK_SDIO_IRQ_Test(&host);
        if (host.card_type == SDCARD)
            CK_SDIO_Hotplug_Test(&host);
        if (i == 0)
            CK_SDIO_Wakeup_Test(&host);
        printf("\n\tEnd Test SDIO %d \n", host.id);
    }

    printf("\nEnd Synopsys SDIO Controller Test. . . \n");
}


