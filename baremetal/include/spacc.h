//SPACC registers addr define

#ifndef ___SPACC_H___
#define ___SPACC_H___

#include "ck810.h"

#define IRQ_EN                 SPACC_BASE_ADDR+0x0
#define IRQ_STAT               SPACC_BASE_ADDR+0x4
#define IRQ_CTRL               SPACC_BASE_ADDR+0x8
#define FIFO_STAT              SPACC_BASE_ADDR+0xC
#define SDMA_BRST_SZ           SPACC_BASE_ADDR+0x10
#define SRC_PTR                SPACC_BASE_ADDR+0x20
#define DST_PTR                SPACC_BASE_ADDR+0x24
#define OFFSET                 SPACC_BASE_ADDR+0x28
#define PRE_AAD_LEN            SPACC_BASE_ADDR+0x2C
#define POST_AAD_LEN           SPACC_BASE_ADDR+0x30
#define PROC_LEN               SPACC_BASE_ADDR+0x34
#define ICV_LEN                SPACC_BASE_ADDR+0x38
#define ICV_OFFSET             SPACC_BASE_ADDR+0x3C
#define IV_OFFSET              SPACC_BASE_ADDR+0x40
#define SW_CTRL                SPACC_BASE_ADDR+0x44
#define AUX_INFO               SPACC_BASE_ADDR+0x48
#define CTRL                   SPACC_BASE_ADDR+0x4C
#define STAT_POP               SPACC_BASE_ADDR+0x50
#define STATUS                 SPACC_BASE_ADDR+0x54
#define STAT_WD_CTRL           SPACC_BASE_ADDR+0x80
#define KEY_SZ                 SPACC_BASE_ADDR+0x100


#define CIPH_CTX               SPACC_BASE_ADDR+0x4000
#define HAS_CTX                SPACC_BASE_ADDR+0x8000

#define SRC_PTR_ADDR           0xf0000000+0x0
#define DST_PTR_ADDR           0xf0000000+0x8000

#define PLAIN_SRAM_BASE        0xf0010000
#define CIPH_SRAM_BASE         0xf0020000
#define DECYP_SRAM_BASE        0xf0030000
#define PLAIN_SRAM_1_BASE      0xf00e0000
#define CIPH_SRAM_1_BASE       0xf0120000
#define DECYP_SRAM_1_BASE      0xf0160000

#define PLAIN_DDR_BASE         0x30000000
#define CIPH_DDR_BASE          0x60000000
#define DECYP_DDR_BASE         0x90000000
#define DECYP_DDR_1_BASE       0xd0000000


#endif // ___SPACC_H___
