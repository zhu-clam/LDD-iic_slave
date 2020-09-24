// STC registers addr define

#ifndef ___STC_H___
#define ___STC_H___

#include "ck810.h"

#define VSI_STC_ENABLE                  CK_STC_ADDR + 0x000
#define VSI_STC_EN                      BIT(0)
#define VSI_STC_CLK_DIVIDER             CK_STC_ADDR + 0x004
#define VSI_STC_TIMER_INIT_VALUE_L      CK_STC_ADDR + 0x008
#define VSI_STC_TIMER_INIT_VALUE_H      CK_STC_ADDR + 0x00c
#define VSI_STC_TIMER_CUR_VALUE_L       CK_STC_ADDR + 0x010
#define VSI_STC_TIMER_CUR_VALUE_H       CK_STC_ADDR + 0x014

#endif // ___STC_H___
