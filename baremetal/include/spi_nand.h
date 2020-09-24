/*
 * SPI interface command hex code, type definition and function prototype.
 *
 */
#ifndef    __SPI_NAND__
#define    __SPI_NAND__

#include "datatype.h"
#include "intc.h"
#include "ck810.h"

/* Select flash device type */
#define W25N01GV
#define DEFAULT_NAND_CS     0
#define SPI_REG_BASE  SPI3_BASE

// SPI
#define  SPI_TXREG              (0x00 + SPI_REG_BASE)
#define  SPI_RXREG              (0x04 + SPI_REG_BASE)
#define  SPI_CSTAT              (0x08 + SPI_REG_BASE)
#define  SPI_INTSTAT            (0x0C + SPI_REG_BASE)
#define  SPI_INTEN              (0x10 + SPI_REG_BASE)
#define  SPI_INTCLR             (0x14 + SPI_REG_BASE)
#define  SPI_GCTL               (0x18 + SPI_REG_BASE)
#define  SPI_CCTL               (0x1c + SPI_REG_BASE)
#define  SPI_SPBRG              (0x20 + SPI_REG_BASE)
#define  SPI_RXDNR              (0x24 + SPI_REG_BASE)
#define  SPI_SCSR               (0x28 + SPI_REG_BASE)
#define  SPI_MIO_2_3_CTL        (0x2c + SPI_REG_BASE)

#define  IO_MASK                0x80
#define  BYTE_MASK              0xFF

// system flags
#define  PASS                   0
#define  FAIL                   1
#define  BUSY                   0
#define  READY                  1
#define  PROTECTED              0
#define  UNPROTECTED            1

// Flash control register mask define
// status register
// status register [7:0]
#define  SR0_OIP               0x01 // Operation in progress
#define  SR1_WEL               0x02
#define  SR2_EraseFail         0x04
#define  SR2_ProgramFail       0x08

#define  FLASH_QE_MASK         0x01

/*
  Flash ID, Timing Information Define
  (The following information could get from device specification)
*/

#ifdef W25N01GV
#define  FlashID                0xefaa21
#define  FlashSize              0x8000000 //128 MB for test

// Support I/O mode
#define  SIO                    0
#define  DIO                    1
#define  QIO                    2
#endif

//SPI_CSTAT
#define SPI_Rxavl_4byte             (1 << 3)
#define SPI_Receive_Empty           (0 << 3)
#define SPI_TxFull                  (1 << 2)
#define SPI_TxNoFull                (0 << 2)
#define SPI_Rxavl                   (1 << 1)
#define SPI_RxEmpty                 (0 << 1)
#define SPI_TxEmpty                 (1 << 0)
#define SPI_TxNoEmpty               (0 << 0)

//SPI_INTSTAT
#define SPI_TxEmptyInt              (1 << 6)
#define SPI_TxNoEmptyInt            (0 << 6)
#define SPI_RxFullInt               (1 << 5)
#define SPI_RxNoFullInt             (0 << 5)
#define SPI_RxMatchInt              (1 << 4)
#define SPI_RxORError               (1 << 3)
#define SPI_TxURError               (1 << 2)
#define SPI_RxIntF                  (1 << 1)
#define SPI_TxIntF                  (1 << 0)

//SPI_INTEN
#define SPI_TxEmptyIntEn            (1 << 6)
#define SPI_RxFullIntEn             (1 << 5)
#define SPI_RxMatchEn               (1 << 4)
#define SPI_RxORIntEn               (1 << 3)
#define SPI_TxURIntEn               (1 << 2)
#define SPI_RxIntEn                 (1 << 1)
#define SPI_TxIntEn                 (1 << 0)

//SPI_INTCLR
#define SPI_TxEmptyIntClr           (1 << 6)
#define SPI_RxFullIntClr            (1 << 5)
#define SPI_RxMatchClr              (1 << 4)
#define SPI_RxORIntClr              (1 << 3)
#define SPI_TxURIntClr              (1 << 2)
#define SPI_RxIntClr                (1 << 1)
#define SPI_TxIntClr                (1 << 0)

//SPI_GCTL
#define SPI_ti_modeDis              (0 << 14)
#define SPI_ti_modeEn               (1 << 14)

#define SPI_quad_modeDis            (0 << 13)
#define SPI_quad_modeEn             (1 << 13)

#define SPI_dual_modeDis            (0 << 12)
#define SPI_dual_modeEn             (1 << 12)

#define SPI_CS_softc                (0 << 10)
#define SPI_CS_hardc                (1 << 10)
#define SPI_DMAmode                 (1 << 9)
#define SPI_Normalmode              (0 << 9)
#define SPI_Tx4Triglevel            (1 << 7)
#define SPI_Tx1Triglevel            (0 << 7)
#define SPI_Rx1Triglevel            (0 << 5)
#define SPI_Rx4Triglevel            (1 << 5)
#define SPI_RxEn                    (1 << 4)
#define SPI_RxDis                   (0 << 4)
#define SPI_TxEn                    (1 << 3)
#define SPI_TxDis                   (0 << 3)
#define SPI_MasterMode              (1 << 2)
#define SPI_SlaveMode               (0 << 2)
#define SPI_IntEn                   (1 << 1)
#define SPI_IntDis                  (0 << 1)
#define SPI_SPIEn                   (1 << 0)
#define SPI_SPIDis                  (0 << 0)

//SPI_CCTL
#define SPI_Length8                 (7 << 8)

#define SPI_TxEdgeHS                (1 << 5)

#define SPI_RxEdgeHS                (1 << 4)
#define SPI_RxEdgeMS                (0 << 4)

#define SPI_LSBFirst                (1 << 2)
#define SPI_MSBFirst                (0 << 2)
#define SPI_CKPLH                   (1 << 1)
#define SPI_CKPLL                   (0 << 1)
#define SPI_CKPHH                   (1 << 0)
#define SPI_CKPHL                   (0 << 0)

/*** MX35 series command hex code definition ***/
//ID commands
#define    FLASH_CMD_RDID                       0x9F    //RDID (Read Identification)
#define    FLASH_CMD_GET_FEATURE                0x0F    //Get features
#define    FLASH_CMD_SET_FEATURE                0x1F    //Set features
#define    FLASH_CMD_READ                       0x13    //Array Read
#define    FLASH_CMD_READ_CACHE                 0x03    //Read From Cache
#define    FLASH_CMD_READ_CACHE2                0x3B    //Read From Cache*2
#define    FLASH_CMD_READ_CACHE4                0x6B    //Read From Cache*4
#define    FLASH_CMD_WREN                       0x06    //Write Enable
#define    FLASH_CMD_WRDI                       0x04    //Write Disable
#define    FLASH_CMD_PP_LOAD                    0x02    //Page Program Load
#define    FLASH_CMD_PP_RAND_LOAD               0x84    //Page Program Random Input
#define    FLASH_CMD_4PP_LOAD                   0x32    //Quad IO Page Program Load
#define    FLASH_CMD_4PP_RAND_LOAD              0x34    //Quad IO Page Program Random Input
#define    FLASH_CMD_PROGRAM_EXEC               0x10    //Program Execute
#define    FLASH_CMD_BE                         0xD8    //BLock Erase
#define    FLASH_CMD_ECC_STAT_READ              0x7C    //Internal ECC status read
#define    FLASH_CMD_RESET                      0xFF    //Reset the device

#define    SPI_NAND_TEST_LEN                    64

typedef struct {
    CK_UINT32 irq;      /* the interrupt number of SPI master */
    BOOL      opened;   /* indicate whether have been opened or not */
    CK_UINT32 timeout;  /* the set time (us) */
    CKStruct_IRQHandler irqhandler;
} CKStruct_SpiNANDInfo, *PCKStruct_SpiNANDInfo;

/* Return Message */
typedef enum{
    Flash_Success,
    Flash_Busy,
    Flash_OperationTimeOut,
    Flash_ProgramFailed,
    Flash_EraseFailed,
    Flash_ReadIDFailed,
    Flash_CmdInvalid,
    Flash_DataInvalid,
    Flash_AddrInvalid,
    Flash_QuadNotEnable
} ReturnMsg;

/* Basic functions */
void CS_High();
void CS_Low();
void InsertDummyCycle(CK_UINT8 dummy_cycle);
void SendByte(CK_UINT8 byte_value, CK_UINT8 transfer_type);
CK_UINT8 GetByte(CK_UINT8 transfer_type);

/* Utility functions */
void Initial_Spi();
BOOL WaitFlashReady(void);

void SendRowAddr(CK_UINT32 Address, CK_UINT8 io_mode);
BOOL IsFlashQIO(void);

/* Flash commands */
ReturnMsg CMD_RDID(CK_UINT32 *Identification);
ReturnMsg CMD_WREN(void);
ReturnMsg CMD_WRDI(void);
ReturnMsg CMD_GET_FEATURE(CK_UINT8 addr, CK_UINT8 *StatusReg);
ReturnMsg CMD_SET_FEATURE(CK_UINT8 addr, CK_UINT8 value);
ReturnMsg CMD_RESET_OP(void);
BOOL CheckStatus(CK_UINT8 CheckFlag);
ReturnMsg CMD_READ(CK_UINT32 flash_address);
ReturnMsg CMD_READ_CACHE(CK_UINT16 col_address, CK_UINT8 *DataBuf, CK_UINT32 byte_length, CK_UINT8 addr_flag);
ReturnMsg CMD_READ_CACHE2(CK_UINT16 col_address, CK_UINT8 *DataBuf, CK_UINT32 byte_length, CK_UINT8 addr_flag);
ReturnMsg CMD_READ_CACHE4(CK_UINT16 col_address, CK_UINT8 *DataBuf, CK_UINT32 byte_length, CK_UINT8 addr_flag);
ReturnMsg CMD_PP_LOAD(CK_UINT16 col_address, CK_UINT8 *DataBuf, CK_UINT32 Length, CK_UINT8 addr_flag);
ReturnMsg CMD_PP_RAND_LOAD(CK_UINT16 col_address, CK_UINT8 *DataBuf, CK_UINT32 Length, CK_UINT8 addr_flag);
ReturnMsg CMD_4PP_LOAD(CK_UINT16 col_address, CK_UINT8 *DataBuf, CK_UINT32 Length, CK_UINT8 addr_flag);
ReturnMsg CMD_4PP_RAND_LOAD(CK_UINT16 col_address, CK_UINT8 *DataBuf, CK_UINT32 Length, CK_UINT8 addr_flag);
ReturnMsg CMD_PROGRAM_EXEC(CK_UINT32 address);
ReturnMsg CMD_BE(CK_UINT32 flash_address);
ReturnMsg CMD_ECC_STAT_READ(CK_UINT8 *StatusReg);
CK_INT32 SPINAND_unregister_isr();
ReturnMsg DMA_Write_NAND(CK_UINT32 dma_id, CK_UINT8 spi_id, CK_UINT32 addr,
                            CK_UINT8 *buf, CK_UINT32 len,
                            CK_UINT8 dma_channel, CK_UINT8 dma_intr,
                            CK_UINT32 dst_tr_width, CK_UINT32 dst_msize);
void DMA_Read_NAND(CK_UINT32 dma_id, CK_UINT8 spi_id, CK_UINT32 addr,
                            CK_UINT8 *buf, CK_UINT32 len,
                            CK_UINT8 dma_channel, CK_UINT8 dma_intr);

#endif    /* __SPI_NAND__ */
