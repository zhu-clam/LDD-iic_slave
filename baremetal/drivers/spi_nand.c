/*
 * NAND command functions source code.
 */

#include "spi_nand.h"
#include "misc.h"
#include "ahbdma.h"

static CK_UINT32 SPI_READ_NUM;
static CK_UINT8 SPI_READ_BUF[SPI_NAND_TEST_LEN];
static CK_UINT32 SPI_READ_INDEX;
static CKStruct_SpiNANDInfo CK_Spi_Master = {CK_INTC_SPI3, FALSE, 0};
int spi_nand_int_mode = 0;

void spinand_irq_handler(CK_UINT32 irq)
{
    CK_UINT32 val;

    val = read_mreg32(SPI_INTSTAT);
    write_mreg32(SPI_INTCLR, val);
#if CK_SPI_M_NAND_DEBUG
    printf("spi_irq_handler SPI_INTSTAT=0x%x \r\n", val);
#endif

    if (val & 0x12) {
        do {
            val = read_mreg32(SPI_CSTAT);
            // printf("spi_irq_handler SPI_CSTAT=0x%x \r\n",val);

            if(val & 0x02) {
                SPI_READ_BUF[SPI_READ_INDEX] = read_mreg32(SPI_RXREG);
#if CK_SPI_M_NAND_DEBUG
                printf("spi_irq_handler SPI_READ_BUF[%d]=0x%x\r\n",
                    SPI_READ_INDEX, SPI_READ_BUF[SPI_READ_INDEX]);
#endif
                SPI_READ_INDEX++;
                SPI_READ_NUM--;
            }
       } while (val & 0x02);
    }
}

CK_INT32 spinand_register_isr(void(*handler)(CK_UINT32),
                          CK_UINT16 priority, BOOL fast)
{
    PCKStruct_SpiNANDInfo info;

    info = &CK_Spi_Master;
    if(info->opened)
        return FAILURE;

    /* Initialize IRQ handler */
    if (NULL != handler) {
        info->irqhandler.devname = "SPI_M";
        info->irqhandler.irqid = info->irq;
        info->irqhandler.priority = priority;
        info->irqhandler.handler = handler;
        info->irqhandler.bfast = fast;
        info->irqhandler.next = NULL;
        /* Register timer ISR */
        CK_INTC_RequestIrq(&(info->irqhandler), AUTO_MODE);
    }

    info->opened = TRUE;
    return SUCCESS;
}

CK_INT32 SPINAND_unregister_isr()
{
    PCKStruct_SpiNANDInfo info;

    info = &CK_Spi_Master;
    if(!info->opened)
        return FAILURE;

    // Disable Recv Interrupt
    write_mreg32(SPI_INTEN, read_mreg32(SPI_INTEN) & (~0x12));
    write_mreg32(SPI_GCTL, read_mreg32(SPI_GCTL) & (~SPI_IntEn));
    /* Clear the callback function*/
    CK_INTC_FreeIrq(&(info->irqhandler), AUTO_MODE);
    info->opened = FALSE;

    return SUCCESS;
}

/*
 * Function:       Initial_Spi
 * Arguments:      None
 * Description:    Initial spi flash state
 *                 (enable read/write).
 * Return Message: None
 */
void Initial_Spi()
{
    //Master//Disable SPI//Tx Disable//Rx Disable//Rx TLF//Tx TLF//DMA Disable//CS By SoftWare//8 Bit
    write_mreg32(SPI_GCTL, (SPI_dual_modeDis | SPI_quad_modeDis | \
                            SPI_Normalmode | SPI_Tx4Triglevel | \
                            SPI_Rx4Triglevel | SPI_MasterMode | \
                            SPI_SPIEn | SPI_IntDis));
    //Normal Clock//Character Length 8//MSB//Active Low//Mode 3
    write_mreg32(SPI_CCTL, (SPI_Length8 | SPI_MSBFirst | SPI_CKPLH | SPI_CKPHL));
    if (spi_nand_int_mode) {
        write_mreg32(SPI_INTEN, 0x12); //Enable RX Interrupt
        spinand_register_isr(spinand_irq_handler, CK_INTC_SPI2, FALSE);
        SPI_READ_NUM = 0;
        SPI_READ_INDEX = 0;
        write_mreg32(SPI_GCTL, read_mreg32(SPI_GCTL) | SPI_IntEn);
    } else {
        write_mreg32(SPI_INTEN, 0); //Disable All Interrupt
    }
    write_mreg32(SPI_INTCLR, 0x7F); //Clear All Interrupt
    write_mreg32(SPI_RXDNR, 0x01); //Hold Count
    write_mreg32(SPI_SCSR, 0xFF); //De-select All
#if CONFIG_IS_ASIC
    write_mreg32(SPI_SPBRG, SPI_DEFAULT_FREQ / 25000000); // 25MHz
#else
    write_mreg32(SPI_SPBRG, SPI_DEFAULT_FREQ / 500000); // 500KHz
#endif
}

/*
 * Function:       CS_Low, CS_High
 * Arguments:      None.
 * Description:    Chip select go low / high.
 * Return Message: None.
 */
void CS_Low()
{
    write_mreg32(SPI_SCSR, 0xff & (~(1 << DEFAULT_NAND_CS))); // Select CS#
}

void CS_High()
{
    write_mreg32(SPI_SCSR, 0xff); // Deselect CS#
}

/*
 * Function:       InsertDummyCycle
 * Arguments:      dummy_cycle, number of dummy clock cycle
 * Description:    Insert dummy cycle of SCLK
 * Return Message: None.
 */
void InsertDummyCycle(CK_UINT8 dummy_cycle)
{
    CK_UINT8 dummy_bytes = dummy_cycle / 8;
    CK_UINT8 i;

    for (i = 0; i < dummy_bytes; i++) {
        SendByte(0x00, SIO);
    }
}

/*
 * Function:       SendBytes
 * Arguments:      buffer, data transfer to flash
 *                 xfer_type, select different type of I/O mode.
 *                 SIO, single IO
 *                 DIO, dual IO
 *                 QIO, quad IO
 * Description:    Send one byte data to flash
 * Return Message: None.
 */
void SendBytes(void *buf, CK_UINT32 len, CK_UINT8 xfer_type)
{
    CK_UINT32 val;
    CK_UINT8 *pbuf;

    pbuf = (CK_UINT8*)buf;
    // Enable transmit
    write_mreg32(SPI_GCTL, read_mreg32(SPI_GCTL) | SPI_TxEn);

    //Write data
    while (1) {
        val = read_mreg32(SPI_CSTAT);
        if ((val & 0x04) == 0x0) { // TX FIFO is not full
            write_mreg32(SPI_TXREG, *pbuf);
            pbuf++;
            len--;
            if (len == 0) break;
        }
    }

    // Wait transfer finish
    while (1) {
        val = read_mreg32(SPI_CSTAT);
        if(val & 0x01) break;
    }

    // Disable transmit
    write_mreg32(SPI_GCTL, read_mreg32(SPI_GCTL) & (~SPI_TxEn));
}

void SendByte(CK_UINT8 byte_value, CK_UINT8 xfer_type)
{
    SendBytes(&byte_value, 1, xfer_type);
}

void GetBytes(void *buf, CK_UINT32 len, CK_UINT8 xfer_type)
{
    CK_UINT32 val;
    CK_UINT8 *pbuf;

    switch (xfer_type) {
    case SIO: // single I/O
        break;
    case DIO: // dual I/O
        write_mreg32(SPI_GCTL, read_mreg32(SPI_GCTL) | SPI_dual_modeEn);
        break;
    case QIO: // quad I/O
        write_mreg32(SPI_GCTL, read_mreg32(SPI_GCTL) | SPI_quad_modeEn);
        break;
    default:
        printf("Invalid transfer type %d, discard payload\n", xfer_type);
        return;
    }

    pbuf = (CK_UINT8 *)buf;
    write_mreg32(SPI_RXDNR, len);
    // Enable SPI receive
    write_mreg32(SPI_GCTL, read_mreg32(SPI_GCTL) | SPI_RxEn);
    if (spi_nand_int_mode) {
        SPI_READ_NUM = len;
        SPI_READ_INDEX = 0;
        while (SPI_READ_NUM); // Wait ISR read data then decrease it.
        for (val = 0; val < len; val++) {
            pbuf[val] = SPI_READ_BUF[val];
        }
    } else {
        while (1) {
            val = read_mreg32(SPI_CSTAT);
            if (val & 0x02) {
                *pbuf = read_mreg32(SPI_RXREG);
                pbuf++;
                len--;
                if (len == 0) break;
            }
        }
    }

    write_mreg32(SPI_GCTL, read_mreg32(SPI_GCTL) & (~SPI_RxEn));
    // Restore to normal mode
    switch (xfer_type) {
    case SIO: // single I/O
        break;
    case DIO: // dual I/O
        write_mreg32(SPI_GCTL, read_mreg32(SPI_GCTL) & (~SPI_dual_modeEn));
        break;
    case QIO: // quad I/O
        write_mreg32(SPI_GCTL, read_mreg32(SPI_GCTL) & (~SPI_quad_modeEn));
        break;
    default:
        break;
    }
}

/*
 * Function:       GetByte
 * Arguments:      byte_value, data receive from flash
 *                 xfer_type, select different type of I/O mode.
 *                 SIO, single IO
 *                 DIO, dual IO
 *                 QIO, quad IO
 * Description:    Get one byte data to flash
 * Return Message: 8 bit data
 */
CK_UINT8 GetByte(CK_UINT8 xfer_type)
{
    CK_UINT8 data;
    GetBytes(&data, 1, xfer_type);
    return data;
}

/*
 * Function:     Reset_OP
 * Arguments:    None
 * Return Value: Flash_Success
 * Description:  The reset command FFh resets the read/program/erase operation
 */
ReturnMsg CMD_RESET_OP()
{
    CS_Low();
    /* Send reset command */
    SendByte(FLASH_CMD_RESET, SIO);
    CS_High();

    /* Wait reset finish */
    delay(5);

    return Flash_Success;
}

/*
 * Function:       WaitFlashReady
 * Arguments:      None
 * Description:    If flash is ready return READY.
 *                 If flash is time-out return TIMEOUT.
 * Return Message: READY, TIMEOUT
 */
BOOL WaitFlashReady()
{
    while (TRUE) {
        if (CheckStatus(SR0_OIP) == READY)
            return READY;
    }

    return BUSY;
}

/*
 * Function:     CheckStatus
 * Arguments:    check_flag -> the status bit to check
 * Return Value: READY, BUSY
 * Description:  Check status register bit 7 ~ bit 0
 */
BOOL CheckStatus(CK_UINT8 check_flag)
{
    CK_UINT8 status;

    CMD_GET_FEATURE(0xc0, &status);
    if ((status & check_flag) == check_flag)
        return BUSY;
    else
        return READY;
}

/*
 * Function:       IsFlashQIO
 * Arguments:      None.
 * Description:    If flash QE bit = 1: return TRUE
 *                                 = 0: return FALSE.
 * Return Message: TRUE, FALSE
 */
BOOL IsFlashQIO(void)
{
    CK_UINT8 status;
    CMD_GET_FEATURE(0xb0, &status);
    if ((status & FLASH_QE_MASK) == FLASH_QE_MASK)
        return TRUE;
    else
        return FALSE;
}

/*
 * Function:       CMD_Internal_ECC_Status_Read
 * Arguments:      status, 8 bit buffer to store register value
 * Description:    Check ECC status
 * Return Message: Flash_Success
 */
ReturnMsg CMD_ECC_STAT_READ(CK_UINT8 *status)
{
   // Chip select go low to start a flash command
    CS_Low();
    // Send command
    SendByte(FLASH_CMD_ECC_STAT_READ, SIO);
    InsertDummyCycle(8);
    // Get status
    *status = GetByte(SIO);
    // Chip select go high to end a flash command
    CS_High();

    return Flash_Success;
}

/*
 * Function:       Get_Feature
 * Arguments:      addr, Set Feature Address
 *                 status, 8 bit buffer to store Feature register value
 * Description:    Check Features Settings.
 * Return Message: Flash_Success
 */
ReturnMsg CMD_GET_FEATURE(CK_UINT8 addr, CK_UINT8 *status)
{
   // Chip select go low to start a flash command
    CS_Low();
    // Send command
    SendByte(FLASH_CMD_GET_FEATURE, SIO);
    //Send one byte Address
    SendByte(addr, SIO);
    //Get Features
    *status = GetByte(SIO);
    // Chip select go high to end a flash command
    CS_High();

    return Flash_Success;
}

/*
 * Function:       Set_Feature
 * Arguments:      addr, Set Feature Address
 *                 value, 8 bit Feature register value to update
 * Description:    Write Features Settings.
 * Return Message: Flash_Success
 */
ReturnMsg CMD_SET_FEATURE(CK_UINT8 addr, CK_UINT8 value)
{
   // Chip select go low to start a flash command
    CS_Low();
    // Send command
    SendByte(FLASH_CMD_SET_FEATURE, SIO);
    //Send one byte Address
    SendByte(addr, SIO);
    //Set Features
    SendByte(value, SIO);
    // Chip select go high to end a flash command
    CS_High();

    return Flash_Success;
}

/*
 * Function:     SendColAddr
 * Arguments:    address, 16 bit col address
 *               wrap, wrap address bits define the four wrap length
 *               io_mode, I/O mode to transfer address
 * Return Value: None.
 * Description:  Send col address
 */
void SendColAddr(CK_UINT16 addr, CK_UINT8 wrap, CK_UINT8 io_mode)
{
    CK_UINT16 ColAddr = addr & 0x0fff;
    /* Send 16 bit address data */
    // wrap: for plane select,
    // For 1Gb, wrap[1:0] keep default value
    SendByte(((ColAddr >> 8) & BYTE_MASK) | (wrap << 4), io_mode);
    SendByte((ColAddr & BYTE_MASK), io_mode);
}

/*
 * Function:     SendRowAddr
 * Arguments:    address, 32 bit Row address
 *               io_mode, I/O mode to transfer address
 * Return Value: None.
 * Description:  Send Row address
 */
void SendRowAddr(CK_UINT32 addr, CK_UINT8 io_mode)
{
    /* Send 24 bit address data */
    SendByte(((addr >> 16) & BYTE_MASK), io_mode);
    SendByte(((addr >> 8) & BYTE_MASK), io_mode);
    SendByte((addr & BYTE_MASK), io_mode);
}

/*
 * Function:       CMD_WREN
 * Arguments:      None
 * Description:    The WREN instruction is for setting write Enable Latch
 *                 (WEL) bit.
 * Return Message: Flash_Success
 */
ReturnMsg CMD_WREN()
{
    // Chip select go low to start a flash command
    CS_Low();
    // Write Enable command = 0x06, Setting Write Enable Latch Bit
    SendByte(FLASH_CMD_WREN, SIO);
    // Chip select go high to end a flash command
    CS_High();

    return Flash_Success;
}

/*
 * Function:       CMD_WRDI
 * Arguments:      None
 * Description:    The WRDI instruction is to reset
 *                 Write Enable Latch (WEL) bit.
 * Return Message: Flash_Success
 */
ReturnMsg CMD_WRDI(void)
{
    // Chip select go low to start a flash command
    CS_Low();
    // Write Disable command = 0x04, resets Write Enable Latch Bit
    SendByte(FLASH_CMD_WRDI, SIO);
    CS_High();

    return Flash_Success;
}

/*
 * Function:       CMD_RDID
 * Arguments:      id, 16 bit buffer to store id
 * Description:    The RDID instruction is to read the manufacturer ID
 *                 of 1-byte and followed by Device ID of 1-byte.
 * Return Message: Flash_Busy,Flash_Success
 */
ReturnMsg CMD_RDID(CK_UINT32 *id)
{
    CK_UINT32 temp;
    CK_UINT8 gDataBuffer[3];

    /* Check flash is busy or not */
    if (CheckStatus(SR0_OIP) != READY) {
        printf("Can't execute READ ID command, flash is busy now.\n");
        return Flash_Busy;
    }

    // Chip select go low to start a flash command
    CS_Low();
    // Send command
    SendByte(FLASH_CMD_RDID, SIO);
    InsertDummyCycle(8);
    // Get manufacturer identification, device identification
    gDataBuffer[0] = GetByte(SIO);
    gDataBuffer[1] = GetByte(SIO);
    gDataBuffer[2] = GetByte(SIO);
    // Chip select go high to end a command
    CS_High();
#if CK_SPI_M_NAND_DEBUG
    printf("SPI NAND flash manufacturer ID: 0x%x, device ID: 0x%x%x\n", gDataBuffer[0], gDataBuffer[1], gDataBuffer[1]);
#endif
    // Store identification
    temp = (gDataBuffer[0] << 16) | (gDataBuffer[1] << 8) | gDataBuffer[2];
    *id = temp;

    return Flash_Success;
}

/*
 * Function:       CMD_READ
 * Arguments:      flash_address, 32 bit flash memory address
 * Description:    The READ instruction is for reading data from array to cache.
 * Return Message: Flash_AddrInvalid, Flash_Busy, Flash_Success,Flash_OperationTimeOut
 */
ReturnMsg CMD_READ(CK_UINT32 flash_addr)
{
    // Check flash address
    if (flash_addr > FlashSize) return Flash_AddrInvalid;

    /* Check flash is busy or not */
    if (CheckStatus(SR0_OIP) != READY) return Flash_Busy;
#if CK_SPI_M_NAND_DEBUG
    printf("[%s:%d] do array read\n", __FUNCTION__, __LINE__);
#endif
    // Chip select go low to start a flash command
    CS_Low();
    // Write READ command and address
    SendByte(FLASH_CMD_READ, SIO);
    SendRowAddr(flash_addr >> 12, SIO);
    // Chip select go high to end a flash command
    CS_High();

    /* Wait data transfer from array to cache finish */
    if (WaitFlashReady() == READY)
        return Flash_Success;
    else
        return Flash_OperationTimeOut;
}

/*
 * Function:       CMD_READCache
 * Arguments:      col_address, 16 bit flash memory address
 *                 buf, Data buffer address to store returned data
 *                 len, length of returned data in byte unit
 *                 addr_flag, define wrap bit and Plane select bit (only for 2Gb and 4Gb)
 * Description:    The READCache instruction is for reading data out from cache on SO.
 * Return Message: Flash_Success
 */
ReturnMsg CMD_READ_CACHE(CK_UINT16 col_addr, CK_UINT8 *buf,
                         CK_UINT32 len, CK_UINT8 addr_flag)
{
#if CK_SPI_M_NAND_DEBUG
    printf("[%s:%d] do read from cache\n", __FUNCTION__, __LINE__);
#endif

    // Chip select go low to start a flash command
    CS_Low();

    // Write READ Cache command and address
    SendByte(FLASH_CMD_READ_CACHE, SIO);
    SendColAddr(col_addr, addr_flag, SIO);
    InsertDummyCycle(8);
    GetBytes(buf, len, SIO);

    // Chip select go high to end a flash command
    CS_High();

    return Flash_Success;
}

/*
 * Function:       CMD_READCache2
 * Arguments:      col_address, 16 bit flash memory address
 *                 buf, Data buffer address to store returned data
 *                 len, length of returned data in byte unit
 *                 addr_flag, define wrap bit and Plane select bit (only for 2Gb and 4Gb)
 * Description:    The READCache instruction is for reading data out from cache on SI and SO.
 * Return Message: Flash_Success
 */
ReturnMsg CMD_READ_CACHE2(CK_UINT16 col_address, CK_UINT8 *buf,
                          CK_UINT32 len, CK_UINT8 addr_flag)
{
#if CK_SPI_M_NAND_DEBUG
    printf("[%s:%d] do read from cache2\n", __FUNCTION__, __LINE__);
#endif
    // Chip select go low to start a flash command
    CS_Low();
    // Write READ Cache command and address
    SendByte(FLASH_CMD_READ_CACHE2, SIO);
    SendColAddr(col_address, addr_flag, SIO);
    InsertDummyCycle(8);
    GetBytes(buf, len, DIO);

    // Chip select go high to end a flash command
    CS_High();

    return Flash_Success;
}

/*
 * Function:       CMD_READCache4
 * Arguments:      col_address, 16 bit flash memory address
 *                 buf, Data buffer address to store returned data
 *                 len, length of returned data in byte unit
 *                 addr_flag, define wrap bit and Plane select bit (only for 2Gb and 4Gb)
 * Description:    The READCache4 instruction is for reading data out from cache on SI, SO,
 *                 WP and HOLD.
 * Return Message: Flash_QuadNotEnable, Flash_Success
 */
ReturnMsg CMD_READ_CACHE4(CK_UINT16 col_address, CK_UINT8 *buf,
                          CK_UINT32 len, CK_UINT8 addr_flag)
{
#if CK_SPI_M_NAND_DEBUG
    printf("[%s:%d] do read from cache4\n", __FUNCTION__, __LINE__);
#endif
    // Check QE bit
    if (IsFlashQIO() != TRUE) return Flash_QuadNotEnable;

    // Chip select go low to start a flash command
    CS_Low();
    // Write READ Cache command and address
    SendByte(FLASH_CMD_READ_CACHE4, SIO);
    SendColAddr(col_address, addr_flag, SIO);
    InsertDummyCycle(8);
    GetBytes(buf, len, QIO);

    // Chip select go high to end a flash command
    CS_High();

    return Flash_Success;
}

/*
 * Function:       CMD_Program_Load
 * Arguments:      col_address, 16 bit col address
 *                 buf, buffer of source data to program
 *                 len, byte length of data to program
 *                 addr_flag, define Plane select bit (only for 2Gb and 4Gb)
 * Description:    load program data with cache reset first
 * Return Message: Flash_Busy, Flash_Success,
 */
ReturnMsg CMD_PP_LOAD(CK_UINT16 col_address, CK_UINT8 *buf,
                      CK_UINT32 len, CK_UINT8 addr_flag)
{
    /* Check flash is busy or not */
    if (CheckStatus(SR0_OIP) != READY) return Flash_Busy;

    //Send write enable command
    CMD_WREN();
#if CK_SPI_M_NAND_DEBUG
    printf("[%s:%d] do page program load\n", __FUNCTION__, __LINE__);
#endif
    // Chip select go low to start a flash command
    CS_Low();
    /* Send program load command */
    SendByte(FLASH_CMD_PP_LOAD, SIO);
    /* Send flash address */
    SendColAddr(col_address, addr_flag, SIO);
    /* Send data to program */
    SendBytes(buf, len, SIO);
    // Chip select go high to end a flash command
    CS_High();

    return Flash_Success;
}

/*
 * Function:       CMD_Program_Load_RandData
 * Arguments:      col_address, 16 bit col address
 *                 buf, buffer of source data to program
 *                 len, byte length of data to program
 *                 addr_flag, define Plane select bit (only for 2Gb and 4Gb)
 * Description:    load program data without cache reset
 * Return Message: Flash_Success
 */
ReturnMsg CMD_PP_RAND_LOAD(CK_UINT16 col_address, CK_UINT8 *buf,
                           CK_UINT32 len, CK_UINT8 addr_flag)
{
    // Chip select go low to start a flash command
    CS_Low();
    /* Send program load command */
    SendByte(FLASH_CMD_PP_RAND_LOAD, SIO);
    /* Send flash address */
    SendColAddr(col_address, addr_flag, SIO);
    /* Send data to program */
    SendBytes(buf, len, SIO);
    // Chip select go high to end a flash command
    CS_High();

    return Flash_Success;
}

/*
 * Function:       CMD_4PP_Load
 * Arguments:      col_address, 16 bit col address
 *                 buf, buffer of source data to program
 *                 byte_length, byte length of data to program
 *                 addr_flag, define Plane select bit (only for 2Gb and 4Gb)
 * Description:    load program data with 4 data input
 * Return Message: Flash_Busy, Flash_Success
 */
ReturnMsg CMD_4PP_LOAD(CK_UINT16 col_address, CK_UINT8 *buf,
                       CK_UINT32 len, CK_UINT8 addr_flag)
{
    // Check QE bit
    if (IsFlashQIO() != TRUE) return Flash_QuadNotEnable;

    /* Check flash is busy or not */
    if (CheckStatus(SR0_OIP) != READY) return Flash_Busy;

    //send write enable command
    CMD_WREN();

    // Chip select go low to start a flash command
    CS_Low();
    /* Send Quad IO program load command */
    SendByte( FLASH_CMD_4PP_LOAD, SIO);
    /* Send flash address */
    SendColAddr( col_address, addr_flag, SIO);
    /* Send data to program */
    SendBytes(buf, len, QIO);
    // Chip select go high to end a flash command
    CS_High();

    return Flash_Success;
}

/*
 * Function:       CMD_4PP_Load_RandData
 * Arguments:      col_address, 16 bit col address
 *                 buf, buffer of source data to program
 *                 byte_length, byte length of data to program
 *                 addr_flag, define Plane select bit (only for 2Gb and 4Gb)
 * Description:    load program random data with 4 data input
 * Return Message: Flash_Success
 */
ReturnMsg CMD_4PP_RAND_LOAD(CK_UINT16 col_address, CK_UINT8 *buf,
                            CK_UINT32 len, CK_UINT8 addr_flag)
{
    // Check QE bit
    if (IsFlashQIO() != TRUE) return Flash_QuadNotEnable;
    // Chip select go low to start a flash command
    CS_Low();
    /* Send Quad io program random data load command */
    SendByte(FLASH_CMD_4PP_RAND_LOAD, SIO);
    /* Send flash address */
    SendColAddr(col_address, addr_flag, SIO);
    /* Send data to program */
    SendBytes(buf, len, QIO);
    // Chip select go high to end a flash command
    CS_High();

    return Flash_Success;
}

/*
 * Function:       CMD_Program_Exec
 * Arguments:      address, 32 bit flash memory address
 * Description:    Enter block/page address,no data,execute
 * Return Message: Flash_Success,Flash_OperationTimeOut
 */
ReturnMsg CMD_PROGRAM_EXEC(CK_UINT32 address)
{
    CK_UINT8 status;
#if CK_SPI_M_NAND_DEBUG
    printf("[%s:%d] do program execute\n", __FUNCTION__, __LINE__);
#endif
    // Chip select go low to start a flash command
    CS_Low();
    /* Send program execute command */
    SendByte(FLASH_CMD_PROGRAM_EXEC, SIO);
    /* Send flash address */
    SendRowAddr(address >> 12, SIO);
   // Chip select go high to end a flash command
    CS_High();

    /* Wait page program finish */
    if (WaitFlashReady() == READY) {
        /* Check program result */
        CMD_GET_FEATURE(0xc0, &status);
        if ((status & SR2_ProgramFail) == SR2_ProgramFail) {
            printf("[%s:%d] program failed\n", __FUNCTION__, __LINE__);
            return Flash_ProgramFailed;
        } else {
#if CK_SPI_M_NAND_DEBUG
            printf("[%s:%d] program successfully\n", __FUNCTION__, __LINE__);
#endif
            return Flash_Success;
        }
    } else {
        return Flash_OperationTimeOut;
    }
}

/*
 * Function:       CMD_BE
 * Arguments:      flash_address, 32 bit flash memory address
 * Description:    The BE instruction is for erasing the data
 * Return Message: Flash_AddrInvalid, Flash_Busy, Flash_Success,
 *                 Flash_OperationTimeOut
 */
ReturnMsg CMD_BE(CK_UINT32 flash_addr)
{
    CK_UINT8 status;

    // Check flash address
    if (flash_addr > FlashSize) {
        printf("Block erase input address 0x%x exceed limit 0x%x\n",
            flash_addr, FlashSize);
        return Flash_AddrInvalid;
    }

    /* Check flash is busy or not */
    if (CheckStatus(SR0_OIP) != READY) {
        printf("[%s:%d] flash is busy now\n", __FUNCTION__, __LINE__);
        return Flash_Busy;
    }

    // Setting Write Enable Latch bit
    CMD_WREN();
#if CK_SPI_M_NAND_DEBUG
    printf("[%s:%d] do block erase\n", __FUNCTION__, __LINE__);
#endif
    // Chip select go low to start a flash command
    CS_Low();
    //Write Block Erase command
    SendByte(FLASH_CMD_BE, SIO);
    SendRowAddr(flash_addr >> 12, SIO);
    // Chip select go high to end a flash command
    CS_High();

      /* Wait page program finish */
    if (WaitFlashReady() == READY) {
        /* Check program result */
        CMD_GET_FEATURE(0xc0, &status);
        if ((status & SR2_EraseFail) == SR2_EraseFail) {
            printf("[%s:%d] flash erase failed\n", __FUNCTION__, __LINE__);
            return Flash_EraseFailed;
        } else {
#if CK_SPI_M_NAND_DEBUG
            printf("[%s:%d] flash erase successfully\n", __FUNCTION__, __LINE__);
#endif
            return Flash_Success;
        }
    } else {
        return Flash_OperationTimeOut;
    }
}

ReturnMsg DMA_Write_NAND_pre(CK_UINT32 dma_id, CK_UINT8 spi_id, CK_UINT32 addr,
                            CK_UINT8 *buf, CK_UINT32 len,
                            CK_UINT8 dma_channel, CK_UINT8 dma_intr,
                            CK_UINT32 dst_tr_width, CK_UINT32 dst_msize)
{
    /* Check flash is busy or not */
    if (CheckStatus(SR0_OIP) != READY) return Flash_Busy;
    //Send write enable command
    CMD_WREN();

    // Chip select go low to start a flash command
    CS_Low();

    /* Send program load command */
    SendByte(FLASH_CMD_PP_LOAD, SIO);
    /* Send flash address */
    SendColAddr(addr, 0x00, SIO);
    DMAMem2PeripheralOpen(dma_id, dma_channel, (CK_UINT32)buf, len,
                            peripheral_spi_tx(spi_id), dma_intr,
                            0, 0, 0, dst_tr_width, dst_msize);

    write_mreg32(SPI_GCTL, read_mreg32(SPI_GCTL) | SPI_DMAmode |
                    SPI_TxEn | SPI_SPIEn);

    return Flash_Success;
}

ReturnMsg DMA_Write_NAND_post(CK_UINT32 dma_id, CK_UINT32 addr,
                            CK_UINT8 dma_channel, CK_UINT8 dma_intr)
{
    CK_UINT32 val = 0;

    CK_UINT32 timeout = 0;
    while (1) {
        if (DMAC_CheckDone(dma_id, dma_channel, dma_intr) == 1) {
            break;
        } else {
            if (timeout++ > 0x5000) {
                printf("\n\twaiting for DMA Done timeout!\n");
                return Flash_OperationTimeOut;
            }
        }
    }

    DMAC_Close(dma_id, dma_channel);
    val = read_mreg32(SPI_CSTAT);

    while (!(val & SPI_TxEmpty)) {
        val = read_mreg32(SPI_CSTAT);
    }

    write_mreg32(SPI_GCTL, read_mreg32(SPI_GCTL) & (~SPI_TxEn));
    CS_High();
    CMD_PROGRAM_EXEC(addr);
    return Flash_Success;
}

ReturnMsg DMA_Write_NAND(CK_UINT32 dma_id, CK_UINT8 spi_id, CK_UINT32 addr,
                            CK_UINT8 *buf, CK_UINT32 len,
                            CK_UINT8 dma_channel, CK_UINT8 dma_intr,
                            CK_UINT32 dst_tr_width, CK_UINT32 dst_msize)
{
    ReturnMsg ret = 0;
    DMAC_Init(dma_id);
    ret = DMA_Write_NAND_pre(dma_id, spi_id, addr, buf, len,
                                dma_channel, dma_intr,
                                dst_tr_width, dst_msize);

    if (ret == Flash_Busy) {
        printf("\n\tFlash is busy...\n");
        return Flash_Busy;
    }

    DMAC_Start(dma_id, dma_channel);

    ret = DMA_Write_NAND_post(dma_id, addr, dma_channel, dma_intr);
    return ret;

}

void DMA_Read_NAND_pre(CK_UINT32 dma_id, CK_UINT8 spi_id, CK_UINT32 addr,
                        CK_UINT8 *buf, CK_UINT32 len,
                        CK_UINT8 dma_channel, CK_UINT8 dma_intr)
{
    /* Read flash memory data to memory buffer */
    CMD_READ(addr);

    // Chip select go low to start a flash command
    CS_Low();

    // Write READ Cache command and address
    SendByte(FLASH_CMD_READ_CACHE, SIO);
    SendColAddr(addr, 0x00, SIO);
    InsertDummyCycle(8);

    //Set SPI_RXDNR
    write_mreg32(SPI_RXDNR, len);
    DMAPeripheral2MemOpen(dma_id, dma_channel, (CK_UINT32)buf, len,
                            peripheral_spi_rx(spi_id), dma_intr, 0, 0, 0);
}

void DMA_Read_NAND_post(CK_UINT32 dma_id,
                        CK_UINT8 dma_channel, CK_UINT8 dma_intr)
{
    CK_UINT32 val = 0;

    write_mreg32(SPI_GCTL, read_mreg32(SPI_GCTL) |
                    SPI_DMAmode | SPI_RxEn | SPI_SPIEn);

    while (1) { //Check Read Done
        val = DMAC_CheckDone(dma_id, dma_channel, dma_intr);
        if (val) break;
    }
    DMAC_Close(dma_id, dma_channel);
    val = read_mreg32(SPI_CSTAT);

    while ((val & SPI_RxEmpty)) {
        val = read_mreg32(SPI_CSTAT);
    }

    write_mreg32(SPI_GCTL, read_mreg32(SPI_GCTL) & (~SPI_RxEn));
    CS_High();
}

void DMA_Read_NAND(CK_UINT32 dma_id, CK_UINT8 spi_id, CK_UINT32 addr,
                        CK_UINT8 *buf, CK_UINT32 len,
                        CK_UINT8 dma_channel, CK_UINT8 dma_intr)
{
    DMAC_Init(dma_id);
    DMA_Read_NAND_pre(dma_id, spi_id, addr, buf, len, dma_channel, dma_intr);
    DMAC_Start(dma_id, dma_channel);
    DMA_Read_NAND_post(dma_id, dma_channel, dma_intr);
}
