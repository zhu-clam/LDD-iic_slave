//OTP registers addr define
#include "ck810.h"

#ifndef ___OTP_H___
#define ___OTP_H___

#define ADDR_COL              OTP_BASE_ADDR + 0x8000
#define ADDR_ROW              OTP_BASE_ADDR + 0x8004
#define PAS                   OTP_BASE_ADDR + 0x8008
#define GLB_CTRL              OTP_BASE_ADDR + 0x800C
#define TEST_CTRL             OTP_BASE_ADDR + 0x8010
#define PDIN                  OTP_BASE_ADDR + 0x8014
#define PDOUT                 OTP_BASE_ADDR + 0x8018
#define PDOUT_DUMMY           OTP_BASE_ADDR + 0x801C
#define OTP_STATUS            OTP_BASE_ADDR + 0x8020
#define ABNORMAL_STATUS       OTP_BASE_ADDR + 0x8024
#define OTP_FLAG              OTP_BASE_ADDR + 0x8028
#define LOW_POWER             OTP_BASE_ADDR + 0x802C
#define INT_EN                OTP_BASE_ADDR + 0x8030
#define INT_RAW               OTP_BASE_ADDR + 0x8034
#define INT_MASK              OTP_BASE_ADDR + 0x8038
#define INT_STATUS            OTP_BASE_ADDR + 0x803C
#define INT_CLR               OTP_BASE_ADDR + 0x8040



#endif // ___OTP_H___
