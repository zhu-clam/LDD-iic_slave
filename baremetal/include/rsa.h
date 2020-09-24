//RSA registers addr define

#ifndef ___RSA_H___
#define ___RSA_H___

#include "ck810.h"

#define CTRL                  RSA_BASE_ADDR+0x000
#define ENTRY_PNT             RSA_BASE_ADDR+0x004
#define RTN_CODE              RSA_BASE_ADDR+0x008
#define BUILD_CONF            RSA_BASE_ADDR+0x00C
#define STACK_PNTR            RSA_BASE_ADDR+0x010
#define INSTR_SINCE_GO        RSA_BASE_ADDR+0x014
#define CONFIG                RSA_BASE_ADDR+0x01C
#define STAT                  RSA_BASE_ADDR+0x020
#define FLAGS                 RSA_BASE_ADDR+0x024
#define WATCHDOG              RSA_BASE_ADDR+0x028
#define CYCLE_SINCE_GO        RSA_BASE_ADDR+0x02C
#define INDEX_I               RSA_BASE_ADDR+0x030
#define INDEX_J               RSA_BASE_ADDR+0x034
#define INDEX_K               RSA_BASE_ADDR+0x038
#define INDEX_L               RSA_BASE_ADDR+0x03C
#define IRQ_EN                RSA_BASE_ADDR+0x040
#define JMP_PROB              RSA_BASE_ADDR+0x044
#define JMP_PROB_LFSR         RSA_BASE_ADDR+0x048
#define BANK_SW_A             RSA_BASE_ADDR+0x050
#define BANK_SW_B             RSA_BASE_ADDR+0x054
#define BANK_SW_C             RSA_BASE_ADDR+0x058
#define BANK_SW_D             RSA_BASE_ADDR+0x05C


#define PKA_REGION_A          RSA_BASE_ADDR+0x400
#define PKA_REGION_B          RSA_BASE_ADDR+0x800
#define PKA_REGION_C          RSA_BASE_ADDR+0xC00
#define PKA_REGION_D          RSA_BASE_ADDR+0x1000
#define PKA_FW                RSA_BASE_ADDR+0x4000


#endif // ___RSA_H___
