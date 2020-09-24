// APTS registers addr define

#ifndef ___APTS_H___
#define ___APTS_H___

#include "ck810.h"

#define APTS_Tx0                APTS_BASE + 0x000
#define APTS_Tx1                APTS_BASE + 0x004
#define APTS_CTRL1              APTS_BASE + 0x008
#define SPI_EN(x)               (x << 1)
#define APTS_EN(x)              x
#define APTS_CTRL2              APTS_BASE + 0x00c
#define WD_LEN(x)               (x << 12)
#define APTS_OPMOD(x)           (x << 10)
#define CHAR_LEN(x)             (x << 3)
#define ASS(x)                  (x << 2)
#define LSB(x)                  (x << 1)
#define NEG(x)                  x
#define APTS_DIVIDER            APTS_BASE + 0x010
#define APTS_SS                 APTS_BASE + 0x014
#define APTS_AUDCTL             APTS_BASE + 0x018
#define AOFFSET(x)              (x << 16)
#define AUDLEN(x)               x
#define APTS_APTSDLY            APTS_BASE + 0x01c
#define APTS_INTSTS             APTS_BASE + 0x020
#define APTS_INTEN              APTS_BASE + 0x024
#define APTS_INTCLR             APTS_BASE + 0x028
#define APTS_STAT1              APTS_BASE + 0x02c


#endif // ___APTS_H___
