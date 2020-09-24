// SCI7816 registers addr define

#ifndef ___SCI7816_H___
#define ___SCI7816_H___

#include "ck810.h"

#define SCIMODHW        SCI7816_ADDR + 0x00
#define SCIMODSW        SCI7816_ADDR + 0x04
#define SCICTRL         SCI7816_ADDR + 0x08
#define SCISTAT         SCI7816_ADDR + 0x0C
#define SCIINTIO1       SCI7816_ADDR + 0x10
#define SCIINTIO2       SCI7816_ADDR + 0x14
#define EDCCTRL         SCI7816_ADDR + 0x18
#define WTCTRL          SCI7816_ADDR + 0x1C
#define SCIBUFHW        SCI7816_ADDR + 0x20
#define SCIBUFSW        SCI7816_ADDR + 0x24
#define ETUDATA         SCI7816_ADDR + 0x28
#define BGTDATA         SCI7816_ADDR + 0x2C
#define CWTDATA         SCI7816_ADDR + 0x30
#define EDCDATA         SCI7816_ADDR + 0x34
#define SCIMODSW2       SCI7816_ADDR + 0x38
#define SCIBUFSW2       SCI7816_ADDR + 0x3C
#define SCIINTRST       SCI7816_ADDR + 0x40
#define BWTDATA         SCI7816_ADDR + 0x44

#endif // ___SCI7816_H___
