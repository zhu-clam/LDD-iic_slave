// operation types:
// - the invisible bit [4] selects legal DDR controller commands when 0 or 
//   other operations when 1 (i.e. other operations start at 17
// - *_BL4 are for DDR3 on-the-fly burst length of 4 (burst chop); these
#define CTRL_NOP          0x0     // no operation
#define LOAD_MODE         0x1     // SDRAM load mode register
#define SELF_REFRESH      0x2     // SDRAM self refresh entry
#define REFRESH           0x3     // SDRAM refresh
#define PRECHARGE         0x4     // SDRAM single bank precharge
#define PRECHARGE_ALL     0x5     // SDRAM all banks precharge
#define ACTIVATE          0x6     // SDRAM bank activate
#define SPECIAL_CMD       0x7     // SDRAM/controller special command
#define SDRAM_WRITE       0x8     // SDRAM write
#define WRITE_PRECHG      0x9     // SDRAM write with auto-precharge
#define SDRAM_READ        0xA     // SDRAM read
#define READ_PRECHG       0xB     // SDRAM read with auto-precharge
#define ZQCAL_SHORT       0xC     // SDRAM ZQ calibration short
#define READ_MODE         0xC     // LPDDR3/2 only - LPDDR3/2 read mode register
#define ZQCAL_LONG        0xD     // SDRAM ZQ calibration long
#define TERMINATE         0xD     // LPDDR3/2 only - Burst terminate
#define POWER_DOWN        0xE     // SDRAM power down entry
#define SDRAM_NOP         0xF     // SDRAM NOP

// encoded PUB data
// ----------------
// PUB data is encoded: data specified is applied to all bytes and represents
// 4 beats of data
#define NO_OF_DATA_TYPES       16
#define PUB_DATA_TYPE_WIDTH    5
#define PUB_DATA_0000_0000     0  // beat 0 = 8'h00,      beat 1 = 8'h00,       beat 2 = 8'b00,        beat 3 = 8'h00
#define PUB_DATA_FFFF_FFFF     1  // beat 0 = 8'hFF,      beat 1 = 8'hFF,       beat 2 = 8'bFF,        beat 3 = 8'hFF
#define PUB_DATA_5555_5555     2  // beat 0 = 8'h55,      beat 1 = 8'h55,       beat 2 = 8'b55,        beat 3 = 8'h55
#define PUB_DATA_AAAA_AAAA     3  // beat 0 = 8'hAA,      beat 1 = 8'hAA,       beat 2 = 8'bAA,        beat 3 = 8'hAA
#define PUB_DATA_0000_5500     4  // beat 0 = 8'h00,      beat 1 = 8'h55,       beat 2 = 8'b00,        beat 3 = 8'h00
#define PUB_DATA_5555_0055     5  // beat 0 = 8'h55,      beat 1 = 8'h00,       beat 2 = 8'b55,        beat 3 = 8'h55
#define PUB_DATA_0000_AA00     6  // beat 0 = 8'h00,      beat 1 = 8'hAA,       beat 2 = 8'b00,        beat 3 = 8'h00
#define PUB_DATA_AAAA_00AA     7  // beat 0 = 8'hAA,      beat 1 = 8'h00,       beat 2 = 8'bAA,        beat 3 = 8'hAA
#define PUB_DATA_DTDR0         8  // beat 0 = DTDR0[7:0], beat 1 = DTDR0[15:8], beat 2 = DTDR0[23:16], beat 3 = DTDR0[31:24]
#define PUB_DATA_DTDR1         9  // beat 0 = DTDR1[7:0], beat 1 = DTDR1[15:8], beat 2 = DTDR1[23:16], beat 3 = DTDR1[31:24]
#define PUB_DATA_UDDR0         10 // beat 0 = UDDR0[7:0], beat 1 = UDDR0[15:8], beat 2 = UDDR0[23:16], beat 3 = UDDR0[31:24]
#define PUB_DATA_UDDR1         11 // beat 0 = UDDR1[7:0], beat 1 = UDDR1[15:8], beat 2 = UDDR1[23:16], beat 3 = UDDR1[31:24]
#define PUB_DATA_WALKING_1     12 // beat 0 = walkign 1,  beat 1 = walking 1,   beat 2 = walking 1,    beat 3 = walking 1
#define PUB_DATA_WALKING_0     13 // beat 0 = walkign 0,  beat 1 = walking 0,   beat 2 = walking 0,    beat 3 = walking 0
#define PUB_DATA_USER_PATTERN  14 // beat 0 = user pat'n, beat 1 = user pat'n,  beat 2 = user pat'n,   beat 3 = user pat'n
#define PUB_DATA_LFSR          15 // beat 0 = LFSR,       beat 1 = LFSR,        beat 2 = LFSR,         beat 3 = LFSR
#define PUB_DATA_SCHCR0        16 // beat 0 = SCHCR0,     beat 1 = SCHCR0,      beat 2 = SCHCR0,       beat 3 = SCHCR0
#define PUB_DATA_FF00_FF00     17 // beat 0 = 8'h00,      beat 1 = 8'hFF,       beat 2 = 8'b00,        beat 3 = 8'hFF
#define PUB_DATA_FFFF_0000     18 // beat 0 = 8'h00,      beat 1 = 8'h00,       beat 2 = 8'bFF,        beat 3 = 8'hFF
#define PUB_DATA_0000_FF00     19 // beat 0 = 8'h00,      beat 1 = 8'hFF,       beat 2 = 8'b00,        beat 3 = 8'h00
#define PUB_DATA_FFFF_00FF     20 // beat 0 = 8'hFF,      beat 1 = 8'h00,       beat 2 = 8'bFF,        beat 3 = 8'hFF
#define PUB_DATA_00FF_00FF     21 // beat 0 = 8'hFF,      beat 1 = 8'h00,       beat 2 = 8'bFF,        beat 3 = 8'h00
#define PUB_DATA_F0F0_F0F0     22 // beat 0 = 8'hF0,      beat 1 = 8'hF0,       beat 2 = 8'bF0,        beat 3 = 8'hF0
#define PUB_DATA_0F0F_0F0F     23 // beat 0 = 8'h0F,      beat 1 = 8'h0F,       beat 2 = 8'b0F,        beat 3 = 8'h0F

// scheduler and DCU definitions
// -----------------------------
// scheduler DRAM timing parameter (DTP)
#define DTP_tNODTP        0
#define DTP_tRP           1
#define DTP_tRAS          2
#define DTP_tRRD          3
#define DTP_tRC           4
#define DTP_tMRD          5
#define DTP_tMOD          6
#define DTP_tFAW          7
#define DTP_tRFC          8
#define DTP_tWLMRD        9
#define DTP_tWLO          10
#define DTP_tXS           11
#define DTP_tXP           12
#define DTP_tCKE          13
#define DTP_tDLLK         14
#define DTP_tDINITRST     15
#define DTP_tDINITCKELO   16
#define DTP_tDINITCKEHI   17
#define DTP_tDINITZQ      18
#define DTP_tRPA          19
#define DTP_tPRE2ACT      20
#define DTP_tACT2RW       21
#define DTP_tRD2PRE       22
#define DTP_tWR2PRE       23
#define DTP_tRD2WR        24
#define DTP_tWR2RD        25
#define DTP_tRDAP2ACT     26
#define DTP_tWRAP2ACT     27
#define DTP_tDCUT0        28
#define DTP_tDCUT1        29
#define DTP_tDCUT2        30
#define DTP_tDCUT3        31

#define DTP_tBCSTAB       9
#define DTP_tBCMRD        10

// DCU instructions
#define DCU_NOP           0x0
#define DCU_RUN           0x1
#define DCU_STOP          0x2
#define DCU_STOP_LOOP     0x3
#define DCU_RESET         0x4

#define DCU_READ          0
#define DCU_WRITE         1

// DCU repeat code
#define DCU_NORPT         0  // execute once
#define DCU_RPT1X         1  // execute twice
#define DCU_RPT4X         2  // execute 4 times
#define DCU_RPT7X         3  // execute 8 times
#define DCU_tBL           4  // execute to create full DDR burst
#define DCU_tDCUT0        5  // execute tDCUT0+1 times
#define DCU_tDCUT1        6  // execute tDCUT1+1 times
#define DCU_tDCUT2        7  // execute tDCUT2+1 times

#define DCU_NOTAG         0
#define DCU_ALL_RANKS     1

 // cache selection codes
#define DCU_CCACHE        0x0 // command cache
#define DCU_ECACHE        0x1 // expected data cache
#define DCU_RCACHE        0x2 // read data cache
//---------------------------------------------------------
//SNPS DDR4 multiPHY register
//---------------------------------------------------------
#define        DDR_PHY_RIDR       (0x000 << 2) //R   Revision Identification Register
#define        DDR_PHY_PIR        (0x001 << 2) //RW  PHY Initialization Register
#define        DDR_PHY_CGCR       (0x002 << 2) //RW  Clock Gating Configuration Register
#define        DDR_PHY_CGCR1      (0x003 << 2) //RW  Clock Gating Configuration Register 1
#define        DDR_PHY_PGCR0      (0x004 << 2) //RW  PHY General Configuration Registers 0
#define        DDR_PHY_PGCR1      (0x005 << 2) //RW  PHY General Configuration Registers 1
#define        DDR_PHY_PGCR2      (0x006 << 2) //RW  PHY General Configuration Registers 2
#define        DDR_PHY_PGCR3      (0x007 << 2) //RW  PHY General Configuration Registers 3
#define        DDR_PHY_PGCR4      (0x008 << 2) //RW  PHY General Configuration Registers 4
#define        DDR_PHY_PGCR5      (0x009 << 2) //RW  PHY General Configuration Registers 5
#define        DDR_PHY_PGCR6      (0x00A << 2) //RW  PHY General Configuration Registers 6
#define        DDR_PHY_PGCR7      (0x00B << 2) //RW  PHY General Configuration Registers 7
#define        DDR_PHY_PGCR8      (0x00C << 2) //RW  PHY General Configuration Registers 8
#define        DDR_PHY_PGSR0      (0x00D << 2) //R   PHY General Status Registers 0
#define        DDR_PHY_PGSR1      (0x00E << 2) //R   PHY General Status Registers 1
#define        DDR_PHY_PTR0       (0x010 << 2) //RW  PHY Timing Registers 0
#define        DDR_PHY_PTR1       (0x011 << 2) //RW  PHY Timing Registers 1
#define        DDR_PHY_PTR2       (0x012 << 2) //RW  PHY Timing Registers 2
#define        DDR_PHY_PTR3       (0x013 << 2) //RW  PHY Timing Registers 3
#define        DDR_PHY_PTR4       (0x014 << 2) //RW  PHY Timing Registers 4
#define        DDR_PHY_PTR5       (0x015 << 2) //RW  PHY Timing Registers 5
#define        DDR_PHY_PTR6       (0x016 << 2) //RW  PHY Timing Registers 6
#define        DDR_PHY_PLLCR0     (0x01A << 2) //RW  PLL Control Register 0
#define        DDR_PHY_PLLCR1     (0x01B << 2) //RW  PLL Control Register 1
#define        DDR_PHY_PLLCR2     (0x01C << 2) //RW  PLL Control Register 2
#define        DDR_PHY_PLLCR3     (0x01D << 2) //RW  PLL Control Register 3
#define        DDR_PHY_PLLCR4     (0x01E << 2) //RW  PLL Control Register 4
#define        DDR_PHY_PLLCR5     (0x01F << 2) //RW  PLL Control Register 5
#define        DDR_PHY_PLLCR      (0x020 << 2) //RW  PLL Control Register
#define        DDR_PHY_DXCCR      (0x022 << 2) //RW  DATX8 Common Configuration Register
#define        DDR_PHY_DSGCR      (0x024 << 2) //RW  DDR System General Configuration Register
#define        DDR_PHY_ODTCR      (0x026 << 2) //RW  ODT Configuration Register
#define        DDR_PHY_AACR       (0x028 << 2) //RW  Anti-Aging Control Register
#define        DDR_PHY_GPR0       (0x030 << 2) //RW  General Purpose Register 0
#define        DDR_PHY_GPR1       (0x031 << 2) //RW  General Purpose Register 1
#define        DDR_PHY_DCR        (0x040 << 2) //RW  DRAM Configuration Register
#define        DDR_PHY_DTPR0      (0x044 << 2) //RW  DRAM Timing Parameters Register 0
#define        DDR_PHY_DTPR1      (0x045 << 2) //RW  DRAM Timing Parameters Register 1
#define        DDR_PHY_DTPR2      (0x046 << 2) //RW  DRAM Timing Parameters Register 2
#define        DDR_PHY_DTPR3      (0x047 << 2) //RW  DRAM Timing Parameters Register 3
#define        DDR_PHY_DTPR4      (0x048 << 2) //RW  DRAM Timing Parameters Register 4
#define        DDR_PHY_DTPR5      (0x049 << 2) //RW  DRAM Timing Parameters Register 5
#define        DDR_PHY_DTPR6      (0x04A << 2) //RW  DRAM Timing Parameters Register 6
#define        DDR_PHY_RDIMMGCR0  (0x050 << 2) //RW  RDIMM General Configuration Register 0
#define        DDR_PHY_RDIMMGCR1  (0x051 << 2) //RW  RDIMM General Configuration Register 1
#define        DDR_PHY_RDIMMGCR2  (0x052 << 2) //RW  RDIMM General Configuration Register 2
#define        DDR_PHY_RDIMMCR0   (0x054 << 2) //RW  RDIMM Control Register 0
#define        DDR_PHY_RDIMMCR1   (0x055 << 2) //RW  RDIMM Control Register 1
#define        DDR_PHY_RDIMMCR2   (0x056 << 2) //RW  RDIMM Control Register 2
#define        DDR_PHY_RDIMMCR3   (0x057 << 2) //RW  RDIMM Control Register 3
#define        DDR_PHY_RDIMMCR4   (0x058 << 2) //RW  RDIMM Control Register 4
#define        DDR_PHY_SCHCR0     (0x05A << 2) //RW  Scheduler Command Register 0
#define        DDR_PHY_SCHCR1     (0x05B << 2) //RW  Scheduler Command Register 1
#define        DDR_PHY_MR0        (0x060 << 2) //RW  Mode Register 0
#define        DDR_PHY_MR1        (0x061 << 2) //RW  Mode Register 1
#define        DDR_PHY_MR2        (0x062 << 2) //RW  Mode Register 2
#define        DDR_PHY_MR3        (0x063 << 2) //RW  Mode Register 3
#define        DDR_PHY_MR4        (0x064 << 2) //RW  Mode Register 4
#define        DDR_PHY_MR5        (0x065 << 2) //RW  Mode Register 5
#define        DDR_PHY_MR6        (0x066 << 2) //RW  Mode Register 6
#define        DDR_PHY_MR7        (0x067 << 2) //RW  Mode Register 7
#define        DDR_PHY_MR11       (0x06B << 2) //RW  Mode Register 11
#define        DDR_PHY_DTCR0      (0x080 << 2) //RW  Data Training Configuration Register 0
#define        DDR_PHY_DTCR1      (0x081 << 2) //RW  Data Training Configuration Register 1
#define        DDR_PHY_DTAR0      (0x082 << 2) //RW  Data Training Address Register 0
#define        DDR_PHY_DTAR1      (0x083 << 2) //RW  Data Training Address Register 1
#define        DDR_PHY_DTAR2      (0x084 << 2) //RW  Data Training Address Register 2
#define        DDR_PHY_DTDR0      (0x086 << 2) //RW  Data Training Data Register 0
#define        DDR_PHY_DTDR1      (0x087 << 2) //RW  Data Training Data Register 1
#define        DDR_PHY_UDDR0      (0x088 << 2) //RW  User Defined Data Register 0
#define        DDR_PHY_UDDR1      (0x089 << 2) //RW  User Defined Data Register 1
#define        DDR_PHY_DTEDR0     (0x08C << 2) //R   Data Training Eye Data Register 0
#define        DDR_PHY_DTEDR1     (0x08D << 2) //R   Data Training Eye Data Register 1
#define        DDR_PHY_DTEDR2     (0x08E << 2) //R   Data Training Eye Data Register 2
#define        DDR_PHY_VTDR       (0x08F << 2) //R   VREF Training Data Register
#define        DDR_PHY_CATR0      (0x090 << 2) //RW  CA Training Register 0
#define        DDR_PHY_CATR1      (0x091 << 2) //RW  CA Training Register 1
#define        DDR_PHY_DQSDR0     (0x094 << 2) //RW  DQS Drift Register 0
#define        DDR_PHY_DQSDR1     (0x095 << 2) //RW  DQS Drift Register 1
#define        DDR_PHY_DQSDR2     (0x096 << 2) //RW  DQS Drift Register 2
#define        DDR_PHY_DCUAR      (0x0C0 << 2) //RW  DCU Address Register
#define        DDR_PHY_DCUDR      (0x0C1 << 2) //RW  DCU Data Register
#define        DDR_PHY_DCURR      (0x0C2 << 2) //RW  DCU Run Register
#define        DDR_PHY_DCULR      (0x0C3 << 2) //RW  DCU Loop Register
#define        DDR_PHY_DCUGCR     (0x0C4 << 2) //RW  DCU General Configuration Register
#define        DDR_PHY_DCUTPR     (0x0C5 << 2) //RW  DCU Timing Parameters Register
#define        DDR_PHY_DCUSR0     (0x0C6 << 2) //R   DCU Status Register 0
#define        DDR_PHY_DCUSR1     (0x0C7 << 2) //R   DCU Status Register 1
#define        DDR_PHY_BISTRR     (0x100 << 2) //RW BIST Run Register
#define        DDR_PHY_BISTWCR    (0x101 << 2) //RW BIST Word Count Register
#define        DDR_PHY_BISTMSKR0  (0x102 << 2) //RW BIST Mask Register 0
#define        DDR_PHY_BISTMSKR1  (0x103 << 2) //RW BIST Mask Register 1
#define        DDR_PHY_BISTMSKR2  (0x104 << 2) //RW BIST Mask Register 2
#define        DDR_PHY_BISTLSR    (0x105 << 2) //RW BIST LFSR Seed Register
#define        DDR_PHY_BISTAR0    (0x106 << 2) //RW BIST Address Register 0
#define        DDR_PHY_BISTAR1    (0x107 << 2) //RW BIST Address Register 1
#define        DDR_PHY_BISTAR2    (0x108 << 2) //RW BIST Address Register 2
#define        DDR_PHY_BISTAR3    (0x109 << 2) //RW BIST Address Register 3
#define        DDR_PHY_BISTAR4    (0x10A << 2) //RW BIST Address Register 4
#define        DDR_PHY_BISTUDPR   (0x10B << 2) //RW BIST User Data Pattern Register
#define        DDR_PHY_BISTGSR    (0x10C << 2) //R  BIST General Status Register
#define        DDR_PHY_BISTWER0   (0x10D << 2) //R  BIST Word Error Register 0
#define        DDR_PHY_BISTWER1   (0x10E << 2) //R  BIST Word Error Register 1
#define        DDR_PHY_BISTBER0   (0x10F << 2) //R  BIST Bit Error Register 0
#define        DDR_PHY_BISTBER1   (0x110 << 2) //R  BIST Bit Error Register 1
#define        DDR_PHY_BISTBER2   (0x111 << 2) //R  BIST Bit Error Register 2
#define        DDR_PHY_BISTBER3   (0x112 << 2) //R  BIST Bit Error Register 3
#define        DDR_PHY_BISTBER4   (0x113 << 2) //R  BIST Bit Error Register 4
#define        DDR_PHY_BISTWCSR   (0x114 << 2) //R  BIST Word Count Status Register
#define        DDR_PHY_BISTFWR0   (0x115 << 2) //R  BIST Fail Word Register 0
#define        DDR_PHY_BISTFWR1   (0x116 << 2) //R  BIST Fail Word Register 1
#define        DDR_PHY_BISTFWR2   (0x117 << 2) //R  BIST Fail Word Register 2
#define        DDR_PHY_BISTBER5   (0x118 << 2) //R  BIST Bit Error Register 5
#define        DDR_PHY_RANKIDR    (0x137 << 2) //RW Rank ID Register
#define        DDR_PHY_RIOCR0     (0x138 << 2) //RW Rank I/O Configuration Registers 0
#define        DDR_PHY_RIOCR1     (0x139 << 2) //RW Rank I/O Configuration Registers 1
#define        DDR_PHY_RIOCR2     (0x13A << 2) //RW Rank I/O Configuration Registers 2
#define        DDR_PHY_RIOCR3     (0x13B << 2) //RW Rank I/O Configuration Registers 3
#define        DDR_PHY_RIOCR4     (0x13C << 2) //RW Rank I/O Configuration Registers 4
#define        DDR_PHY_RIOCR5     (0x13D << 2) //RW Rank I/O Configuration Registers 5
#define        DDR_PHY_ACIOCR0    (0x140 << 2) //RW AC I/O Configuration Register 0
#define        DDR_PHY_ACIOCR1    (0x141 << 2) //RW AC I/O Configuration Register 1
#define        DDR_PHY_ACIOCR2    (0x142 << 2) //RW AC I/O Configuration Register 2
#define        DDR_PHY_ACIOCR3    (0x143 << 2) //RW AC I/O Configuration Register 3
#define        DDR_PHY_ACIOCR4    (0x144 << 2) //RW AC I/O Configuration Register 4
#define        DDR_PHY_IOVCR0     (0x148 << 2) //RW IO VREF Control Register 0
#define        DDR_PHY_IOVCR1     (0x149 << 2) //RW IO VREF Control Register 1
#define        DDR_PHY_VTCR0      (0x14A << 2) //RW VREF Training Control Register 0
#define        DDR_PHY_VTCR1      (0x14B << 2) //RW VREF Training Control Register 1
#define        DDR_PHY_ACBDLR0    (0x150 << 2) //RW AC Bit Delay Line Register 0
#define        DDR_PHY_ACBDLR1    (0x151 << 2) //RW AC Bit Delay Line Register 1
#define        DDR_PHY_ACBDLR2    (0x152 << 2) //RW AC Bit Delay Line Register 2
#define        DDR_PHY_ACBDLR3    (0x153 << 2) //RW AC Bit Delay Line Register 3
#define        DDR_PHY_ACBDLR4    (0x154 << 2) //RW AC Bit Delay Line Register 4
#define        DDR_PHY_ACBDLR5    (0x155 << 2) //RW AC Bit Delay Line Register 5
#define        DDR_PHY_ACBDLR6    (0x156 << 2) //RW AC Bit Delay Line Register 6
#define        DDR_PHY_ACBDLR7    (0x157 << 2) //RW AC Bit Delay Line Register 7
#define        DDR_PHY_ACBDLR8    (0x158 << 2) //RW AC Bit Delay Line Register 8
#define        DDR_PHY_ACBDLR9    (0x159 << 2) //RW AC Bit Delay Line Register 9
#define        DDR_PHY_ACBDLR10   (0x15A << 2) //RW AC Bit Delay Line Register 10
#define        DDR_PHY_ACBDLR11   (0x15B << 2) //RW AC Bit Delay Line Register 11
#define        DDR_PHY_ACBDLR12   (0x15C << 2) //RW AC Bit Delay Line Register 12
#define        DDR_PHY_ACBDLR13   (0x15D << 2) //RW AC Bit Delay Line Register 13
#define        DDR_PHY_ACBDLR14   (0x15E << 2) //RW AC Bit Delay Line Register 14
#define        DDR_PHY_ACLCDLR    (0x160 << 2) //RW AC Local Calibrated Delay Line Register
#define        DDR_PHY_ACMDLR0    (0x168 << 2) //RW AC Master Delay Line Register 0
#define        DDR_PHY_ACMDLR1    (0x169 << 2) //RW AC Master Delay Line Register 1
#define        DDR_PHY_ZQCR       (0x1A0 << 2) //RW ZQ Impedance Control Register
#define        DDR_PHY_ZQ0PR      (0x1A1 << 2) //RW ZQ 0 Impedance Control Program Register
#define        DDR_PHY_ZQ0DR      (0x1A2 << 2) //RW ZQ 0 Impedance Control Data Register
#define        DDR_PHY_ZQ0SR      (0x1A3 << 2) //R  ZQ 0 Impedance Control Status Register
#define        DDR_PHY_ZQ1PR      (0x1A5 << 2) //RW ZQ 1 Impedance Control Program Register
#define        DDR_PHY_ZQ1DR      (0x1A6 << 2) //RW ZQ 1 Impedance Control Data Register
#define        DDR_PHY_ZQ1SR      (0x1A7 << 2) //R  ZQ 1 Impedance Control Status Register
#define        DDR_PHY_ZQ2PR      (0x1A9 << 2) //RW ZQ 2 Impedance Control Program Register
#define        DDR_PHY_ZQ2DR      (0x1AA << 2) //RW ZQ 2 Impedance Control Data Register
#define        DDR_PHY_ZQ2SR      (0x1AB << 2) //R  ZQ 2 Impedance Control Status Register
#define        DDR_PHY_ZQ3PR      (0x1AD << 2) //RW ZQ 3 Impedance Control Program Register
#define        DDR_PHY_ZQ3DR      (0x1AE << 2) //RW ZQ 3 Impedance Control Data Register
#define        DDR_PHY_ZQ3SR      (0x1AF << 2) //R  ZQ 3 Impedance Control Status Register

#define        DDR_PHY_DX0GCR0    (0x1C0 << 2) //RW DATX8 0 General Configuration Register 0
#define        DDR_PHY_DX0GCR1    (0x1C1 << 2) //RW DATX8 0 General Configuration Register 1
#define        DDR_PHY_DX0GCR2    (0x1C2 << 2) //RW DATX8 0 General Configuration Register 2
#define        DDR_PHY_DX0GCR3    (0x1C3 << 2) //RW DATX8 0 General Configuration Register 3
#define        DDR_PHY_DX0GCR4    (0x1C4 << 2) //RW DATX8 0 General Configuration Register 4
#define        DDR_PHY_DX0GCR5    (0x1C5 << 2) //RW DATX8 0 General Configuration Register 5
#define        DDR_PHY_DX0GCR6    (0x1C6 << 2) //RW DATX8 0 General Configuration Register 6
#define        DDR_PHY_DX0GCR7    (0x1C7 << 2) //RW DATX8 0 General Configuration Register 7
#define        DDR_PHY_DX0GCR8    (0x1C8 << 2) //RW DATX8 0 General Configuration Register 8
#define        DDR_PHY_DX0GCR9    (0x1C9 << 2) //RW DATX8 0 General Configuration Register 9
#define        DDR_PHY_DX0BDLR0   (0x1D0 << 2) //RW DATX8 0 Bit Delay Line Register 0
#define        DDR_PHY_DX0BDLR1   (0x1D1 << 2) //RW DATX8 0 Bit Delay Line Register 1
#define        DDR_PHY_DX0BDLR2   (0x1D2 << 2) //RW DATX8 0 Bit Delay Line Register 2
#define        DDR_PHY_DX0BDLR3   (0x1D4 << 2) //RW DATX8 0 Bit Delay Line Register 3
#define        DDR_PHY_DX0BDLR4   (0x1D5 << 2) //RW DATX8 0 Bit Delay Line Register 4
#define        DDR_PHY_DX0BDLR5   (0x1D6 << 2) //RW DATX8 0 Bit Delay Line Register 5
#define        DDR_PHY_DX0BDLR6   (0x1D8 << 2) //RW DATX8 0 Bit Delay Line Register 6
#define        DDR_PHY_DX0BDLR7   (0x1D9 << 2) //RW DATX8 0 Bit Delay Line Register 7
#define        DDR_PHY_DX0BDLR8   (0x1DA << 2) //RW DATX8 0 Bit Delay Line Register 8
#define        DDR_PHY_DX0BDLR9   (0x1DB << 2) //RW DATX8 0 Bit Delay Line Register 9
#define        DDR_PHY_DX0LCDLR0  (0x1E0 << 2) //RW DATX8 0 Local Calibrated Delay Line Register 0
#define        DDR_PHY_DX0LCDLR1  (0x1E1 << 2) //RW DATX8 0 Local Calibrated Delay Line Register 1
#define        DDR_PHY_DX0LCDLR2  (0x1E2 << 2) //RW DATX8 0 Local Calibrated Delay Line Register 2
#define        DDR_PHY_DX0LCDLR3  (0x1E3 << 2) //RW DATX8 0 Local Calibrated Delay Line Register 3
#define        DDR_PHY_DX0LCDLR4  (0x1E4 << 2) //RW DATX8 0 Local Calibrated Delay Line Register 4
#define        DDR_PHY_DX0LCDLR5  (0x1E5 << 2) //RW DATX8 0 Local Calibrated Delay Line Register 5
#define        DDR_PHY_DX0MDLR0   (0x1E8 << 2) //RW DATX8 0 Master Delay Line Register 0
#define        DDR_PHY_DX0MDLR1   (0x1E9 << 2) //RW DATX8 0 Master Delay Line Register 1
#define        DDR_PHY_DX0GTR0    (0x1F0 << 2) //RW DATX8 0 General Timing Register 0
#define        DDR_PHY_DX0RSR0    (0x1F4 << 2) //RW DATX8 0 Rank Status Register 0
#define        DDR_PHY_DX0RSR1    (0x1F5 << 2) //RW DATX8 0 Rank Status Register 1
#define        DDR_PHY_DX0RSR2    (0x1F6 << 2) //RW DATX8 0 Rank Status Register 2
#define        DDR_PHY_DX0RSR3    (0x1F7 << 2) //RW DATX8 0 Rank Status Register 3
#define        DDR_PHY_DX0GSR0    (0x1F8 << 2) //R  DATX8 0 General Status Register 0
#define        DDR_PHY_DX0GSR1    (0x1F9 << 2) //R  DATX8 0 General Status Register 1
#define        DDR_PHY_DX0GSR2    (0x1FA << 2) //R  DATX8 0 General Status Register 2
#define        DDR_PHY_DX0GSR3    (0x1FB << 2) //R  DATX8 0 General Status Register 3
#define        DDR_PHY_DX0GSR4    (0x1FC << 2) //R  DATX8 0 General Status Register 4
#define        DDR_PHY_DX0GSR5    (0x1FD << 2) //R  DATX8 0 General Status Register 5
#define        DDR_PHY_DX0GSR6    (0x1FE << 2) //R  DATX8 0 General Status Register 6

#define        DDR_PHY_DX1GCR0    (0x200 << 2) //RW DATX8 1 General Configuration Register 0
#define        DDR_PHY_DX1GCR1    (0x201 << 2) //RW DATX8 1 General Configuration Register 1
#define        DDR_PHY_DX1GCR2    (0x202 << 2) //RW DATX8 1 General Configuration Register 2
#define        DDR_PHY_DX1GCR3    (0x203 << 2) //RW DATX8 1 General Configuration Register 3
#define        DDR_PHY_DX1GCR4    (0x204 << 2) //RW DATX8 1 General Configuration Register 4
#define        DDR_PHY_DX1GCR5    (0x205 << 2) //RW DATX8 1 General Configuration Register 5
#define        DDR_PHY_DX1GCR6    (0x206 << 2) //RW DATX8 1 General Configuration Register 6
#define        DDR_PHY_DX1GCR7    (0x207 << 2) //RW DATX8 1 General Configuration Register 7
#define        DDR_PHY_DX1GCR8    (0x208 << 2) //RW DATX8 1 General Configuration Register 8
#define        DDR_PHY_DX1GCR9    (0x209 << 2) //RW DATX8 1 General Configuration Register 9
#define        DDR_PHY_DX1BDLR0   (0x210 << 2) //RW DATX8 1 Bit Delay Line Register 0
#define        DDR_PHY_DX1BDLR1   (0x211 << 2) //RW DATX8 1 Bit Delay Line Register 1
#define        DDR_PHY_DX1BDLR2   (0x212 << 2) //RW DATX8 1 Bit Delay Line Register 2
#define        DDR_PHY_DX1BDLR3   (0x214 << 2) //RW DATX8 1 Bit Delay Line Register 3
#define        DDR_PHY_DX1BDLR4   (0x215 << 2) //RW DATX8 1 Bit Delay Line Register 4
#define        DDR_PHY_DX1BDLR5   (0x216 << 2) //RW DATX8 1 Bit Delay Line Register 5
#define        DDR_PHY_DX1BDLR6   (0x218 << 2) //RW DATX8 1 Bit Delay Line Register 6
#define        DDR_PHY_DX1BDLR7   (0x219 << 2) //RW DATX8 1 Bit Delay Line Register 7
#define        DDR_PHY_DX1BDLR8   (0x21A << 2) //RW DATX8 1 Bit Delay Line Register 8
#define        DDR_PHY_DX1BDLR9   (0x21B << 2) //RW DATX8 1 Bit Delay Line Register 9
#define        DDR_PHY_DX1LCDLR0  (0x220 << 2) //RW DATX8 1 Local Calibrated Delay Line Register 0
#define        DDR_PHY_DX1LCDLR1  (0x221 << 2) //RW DATX8 1 Local Calibrated Delay Line Register 1
#define        DDR_PHY_DX1LCDLR2  (0x222 << 2) //RW DATX8 1 Local Calibrated Delay Line Register 2
#define        DDR_PHY_DX1LCDLR3  (0x223 << 2) //RW DATX8 1 Local Calibrated Delay Line Register 3
#define        DDR_PHY_DX1LCDLR4  (0x224 << 2) //RW DATX8 1 Local Calibrated Delay Line Register 4
#define        DDR_PHY_DX1LCDLR5  (0x225 << 2) //RW DATX8 1 Local Calibrated Delay Line Register 5
#define        DDR_PHY_DX1MDLR0   (0x228 << 2) //RW DATX8 1 Master Delay Line Register 0
#define        DDR_PHY_DX1MDLR1   (0x229 << 2) //RW DATX8 1 Master Delay Line Register 1
#define        DDR_PHY_DX1GTR0    (0x230 << 2) //RW DATX8 1 General Timing Register 0
#define        DDR_PHY_DX1RSR0    (0x234 << 2) //RW DATX8 1 Rank Status Register 0
#define        DDR_PHY_DX1RSR1    (0x235 << 2) //RW DATX8 1 Rank Status Register 1
#define        DDR_PHY_DX1RSR2    (0x236 << 2) //RW DATX8 1 Rank Status Register 2
#define        DDR_PHY_DX1RSR3    (0x237 << 2) //RW DATX8 1 Rank Status Register 3
#define        DDR_PHY_DX1GSR0    (0x238 << 2) //R  DATX8 1 General Status Register 0
#define        DDR_PHY_DX1GSR1    (0x239 << 2) //R  DATX8 1 General Status Register 1
#define        DDR_PHY_DX1GSR2    (0x23A << 2) //R  DATX8 1 General Status Register 2
#define        DDR_PHY_DX1GSR3    (0x23B << 2) //R  DATX8 1 General Status Register 3
#define        DDR_PHY_DX1GSR4    (0x23C << 2) //R  DATX8 1 General Status Register 4
#define        DDR_PHY_DX1GSR5    (0x23D << 2) //R  DATX8 1 General Status Register 5
#define        DDR_PHY_DX1GSR6    (0x23E << 2) //R  DATX8 1 General Status Register 6

#define        DDR_PHY_DX2GCR0    (0x240 << 2) //RW DATX8 2 General Configuration Register 0
#define        DDR_PHY_DX2GCR1    (0x241 << 2) //RW DATX8 2 General Configuration Register 1
#define        DDR_PHY_DX2GCR2    (0x242 << 2) //RW DATX8 2 General Configuration Register 2
#define        DDR_PHY_DX2GCR3    (0x243 << 2) //RW DATX8 2 General Configuration Register 3
#define        DDR_PHY_DX2GCR4    (0x244 << 2) //RW DATX8 2 General Configuration Register 4
#define        DDR_PHY_DX2GCR5    (0x245 << 2) //RW DATX8 2 General Configuration Register 5
#define        DDR_PHY_DX2GCR6    (0x246 << 2) //RW DATX8 2 General Configuration Register 6
#define        DDR_PHY_DX2GCR7    (0x247 << 2) //RW DATX8 2 General Configuration Register 7
#define        DDR_PHY_DX2GCR8    (0x248 << 2) //RW DATX8 2 General Configuration Register 8
#define        DDR_PHY_DX2GCR9    (0x249 << 2) //RW DATX8 2 General Configuration Register 9
#define        DDR_PHY_DX2BDLR0   (0x250 << 2) //RW DATX8 2 Bit Delay Line Register 0
#define        DDR_PHY_DX2BDLR1   (0x251 << 2) //RW DATX8 2 Bit Delay Line Register 1
#define        DDR_PHY_DX2BDLR2   (0x252 << 2) //RW DATX8 2 Bit Delay Line Register 2
#define        DDR_PHY_DX2BDLR3   (0x254 << 2) //RW DATX8 2 Bit Delay Line Register 3
#define        DDR_PHY_DX2BDLR4   (0x255 << 2) //RW DATX8 2 Bit Delay Line Register 4
#define        DDR_PHY_DX2BDLR5   (0x256 << 2) //RW DATX8 2 Bit Delay Line Register 5
#define        DDR_PHY_DX2BDLR6   (0x258 << 2) //RW DATX8 2 Bit Delay Line Register 6
#define        DDR_PHY_DX2BDLR7   (0x259 << 2) //RW DATX8 2 Bit Delay Line Register 7
#define        DDR_PHY_DX2BDLR8   (0x25A << 2) //RW DATX8 2 Bit Delay Line Register 8
#define        DDR_PHY_DX2BDLR9   (0x25B << 2) //RW DATX8 2 Bit Delay Line Register 9
#define        DDR_PHY_DX2LCDLR0  (0x260 << 2) //RW DATX8 2 Local Calibrated Delay Line Register 0
#define        DDR_PHY_DX2LCDLR1  (0x261 << 2) //RW DATX8 2 Local Calibrated Delay Line Register 1
#define        DDR_PHY_DX2LCDLR2  (0x262 << 2) //RW DATX8 2 Local Calibrated Delay Line Register 2
#define        DDR_PHY_DX2LCDLR3  (0x263 << 2) //RW DATX8 2 Local Calibrated Delay Line Register 3
#define        DDR_PHY_DX2LCDLR4  (0x264 << 2) //RW DATX8 2 Local Calibrated Delay Line Register 4
#define        DDR_PHY_DX2LCDLR5  (0x265 << 2) //RW DATX8 2 Local Calibrated Delay Line Register 5
#define        DDR_PHY_DX2MDLR0   (0x268 << 2) //RW DATX8 2 Master Delay Line Register 0
#define        DDR_PHY_DX2MDLR1   (0x269 << 2) //RW DATX8 2 Master Delay Line Register 1
#define        DDR_PHY_DX2GTR0    (0x270 << 2) //RW DATX8 2 General Timing Register 0
#define        DDR_PHY_DX2RSR0    (0x274 << 2) //RW DATX8 2 Rank Status Register 0
#define        DDR_PHY_DX2RSR1    (0x275 << 2) //RW DATX8 2 Rank Status Register 1
#define        DDR_PHY_DX2RSR2    (0x276 << 2) //RW DATX8 2 Rank Status Register 2
#define        DDR_PHY_DX2RSR3    (0x277 << 2) //RW DATX8 2 Rank Status Register 3
#define        DDR_PHY_DX2GSR0    (0x278 << 2) //R  DATX8 2 General Status Register 0
#define        DDR_PHY_DX2GSR1    (0x279 << 2) //R  DATX8 2 General Status Register 1
#define        DDR_PHY_DX2GSR2    (0x27A << 2) //R  DATX8 2 General Status Register 2
#define        DDR_PHY_DX2GSR3    (0x27B << 2) //R  DATX8 2 General Status Register 3
#define        DDR_PHY_DX2GSR4    (0x27C << 2) //R  DATX8 2 General Status Register 4
#define        DDR_PHY_DX2GSR5    (0x27D << 2) //R  DATX8 2 General Status Register 5
#define        DDR_PHY_DX2GSR6    (0x27E << 2) //R  DATX8 2 General Status Register 6

#define        DDR_PHY_DX3GCR0    (0x280 << 2) //RW DATX8 3 General Configuration Register 0
#define        DDR_PHY_DX3GCR1    (0x281 << 2) //RW DATX8 3 General Configuration Register 1
#define        DDR_PHY_DX3GCR2    (0x282 << 2) //RW DATX8 3 General Configuration Register 2
#define        DDR_PHY_DX3GCR3    (0x283 << 2) //RW DATX8 3 General Configuration Register 3
#define        DDR_PHY_DX3GCR4    (0x284 << 2) //RW DATX8 3 General Configuration Register 4
#define        DDR_PHY_DX3GCR5    (0x285 << 2) //RW DATX8 3 General Configuration Register 5
#define        DDR_PHY_DX3GCR6    (0x286 << 2) //RW DATX8 3 General Configuration Register 6
#define        DDR_PHY_DX3GCR7    (0x287 << 2) //RW DATX8 3 General Configuration Register 7
#define        DDR_PHY_DX3GCR8    (0x288 << 2) //RW DATX8 3 General Configuration Register 8
#define        DDR_PHY_DX3GCR9    (0x289 << 2) //RW DATX8 3 General Configuration Register 9
#define        DDR_PHY_DX3BDLR0   (0x290 << 2) //RW DATX8 3 Bit Delay Line Register 0
#define        DDR_PHY_DX3BDLR1   (0x291 << 2) //RW DATX8 3 Bit Delay Line Register 1
#define        DDR_PHY_DX3BDLR2   (0x292 << 2) //RW DATX8 3 Bit Delay Line Register 2
#define        DDR_PHY_DX3BDLR3   (0x294 << 2) //RW DATX8 3 Bit Delay Line Register 3
#define        DDR_PHY_DX3BDLR4   (0x295 << 2) //RW DATX8 3 Bit Delay Line Register 4
#define        DDR_PHY_DX3BDLR5   (0x296 << 2) //RW DATX8 3 Bit Delay Line Register 5
#define        DDR_PHY_DX3BDLR6   (0x298 << 2) //RW DATX8 3 Bit Delay Line Register 6
#define        DDR_PHY_DX3BDLR7   (0x299 << 2) //RW DATX8 3 Bit Delay Line Register 7
#define        DDR_PHY_DX3BDLR8   (0x29A << 2) //RW DATX8 3 Bit Delay Line Register 8
#define        DDR_PHY_DX3BDLR9   (0x29B << 2) //RW DATX8 3 Bit Delay Line Register 9
#define        DDR_PHY_DX3LCDLR0  (0x2A0 << 2) //RW DATX8 3 Local Calibrated Delay Line Register 0
#define        DDR_PHY_DX3LCDLR1  (0x2A1 << 2) //RW DATX8 3 Local Calibrated Delay Line Register 1
#define        DDR_PHY_DX3LCDLR2  (0x2A2 << 2) //RW DATX8 3 Local Calibrated Delay Line Register 2
#define        DDR_PHY_DX3LCDLR3  (0x2A3 << 2) //RW DATX8 3 Local Calibrated Delay Line Register 3
#define        DDR_PHY_DX3LCDLR4  (0x2A4 << 2) //RW DATX8 3 Local Calibrated Delay Line Register 4
#define        DDR_PHY_DX3LCDLR5  (0x2A5 << 2) //RW DATX8 3 Local Calibrated Delay Line Register 5
#define        DDR_PHY_DX3MDLR0   (0x2A8 << 2) //RW DATX8 3 Master Delay Line Register 0
#define        DDR_PHY_DX3MDLR1   (0x2A9 << 2) //RW DATX8 3 Master Delay Line Register 1
#define        DDR_PHY_DX3GTR0    (0x2B0 << 2) //RW DATX8 3 General Timing Register 0
#define        DDR_PHY_DX3RSR0    (0x2B4 << 2) //RW DATX8 3 Rank Status Register 0
#define        DDR_PHY_DX3RSR1    (0x2B5 << 2) //RW DATX8 3 Rank Status Register 1
#define        DDR_PHY_DX3RSR2    (0x2B6 << 2) //RW DATX8 3 Rank Status Register 2
#define        DDR_PHY_DX3RSR3    (0x2B7 << 2) //RW DATX8 3 Rank Status Register 3
#define        DDR_PHY_DX3GSR0    (0x2B8 << 2) //R  DATX8 3 General Status Register 0
#define        DDR_PHY_DX3GSR1    (0x2B9 << 2) //R  DATX8 3 General Status Register 1
#define        DDR_PHY_DX3GSR2    (0x2BA << 2) //R  DATX8 3 General Status Register 2
#define        DDR_PHY_DX3GSR3    (0x2BB << 2) //R  DATX8 3 General Status Register 3
#define        DDR_PHY_DX3GSR4    (0x2BC << 2) //R  DATX8 3 General Status Register 4
#define        DDR_PHY_DX3GSR5    (0x2BD << 2) //R  DATX8 3 General Status Register 5
#define        DDR_PHY_DX3GSR6    (0x2BE << 2) //R  DATX8 3 General Status Register 6

#define        DDR_PHY_DX4GCR0    (0x2C0 << 2) //RW DATX8 4 General Configuration Register 0
#define        DDR_PHY_DX4GCR1    (0x2C1 << 2) //RW DATX8 4 General Configuration Register 1
#define        DDR_PHY_DX4GCR2    (0x2C2 << 2) //RW DATX8 4 General Configuration Register 2
#define        DDR_PHY_DX4GCR3    (0x2C3 << 2) //RW DATX8 4 General Configuration Register 3
#define        DDR_PHY_DX4GCR4    (0x2C4 << 2) //RW DATX8 4 General Configuration Register 4
#define        DDR_PHY_DX4GCR5    (0x2C5 << 2) //RW DATX8 4 General Configuration Register 5
#define        DDR_PHY_DX4GCR6    (0x2C6 << 2) //RW DATX8 4 General Configuration Register 6
#define        DDR_PHY_DX4GCR7    (0x2C7 << 2) //RW DATX8 4 General Configuration Register 7
#define        DDR_PHY_DX4GCR8    (0x2C8 << 2) //RW DATX8 4 General Configuration Register 8
#define        DDR_PHY_DX4GCR9    (0x2C9 << 2) //RW DATX8 4 General Configuration Register 9
#define        DDR_PHY_DX4BDLR0   (0x2D0 << 2) //RW DATX8 4 Bit Delay Line Register 0
#define        DDR_PHY_DX4BDLR1   (0x2D1 << 2) //RW DATX8 4 Bit Delay Line Register 1
#define        DDR_PHY_DX4BDLR2   (0x2D2 << 2) //RW DATX8 4 Bit Delay Line Register 2
#define        DDR_PHY_DX4BDLR3   (0x2D4 << 2) //RW DATX8 4 Bit Delay Line Register 3
#define        DDR_PHY_DX4BDLR4   (0x2D5 << 2) //RW DATX8 4 Bit Delay Line Register 4
#define        DDR_PHY_DX4BDLR5   (0x2D6 << 2) //RW DATX8 4 Bit Delay Line Register 5
#define        DDR_PHY_DX4BDLR6   (0x2D8 << 2) //RW DATX8 4 Bit Delay Line Register 6
#define        DDR_PHY_DX4BDLR7   (0x2D9 << 2) //RW DATX8 4 Bit Delay Line Register 7
#define        DDR_PHY_DX4BDLR8   (0x2DA << 2) //RW DATX8 4 Bit Delay Line Register 8
#define        DDR_PHY_DX4BDLR9   (0x2DB << 2) //RW DATX8 4 Bit Delay Line Register 9
#define        DDR_PHY_DX4LCDLR0  (0x2E0 << 2) //RW DATX8 4 Local Calibrated Delay Line Register 0
#define        DDR_PHY_DX4LCDLR1  (0x2E1 << 2) //RW DATX8 4 Local Calibrated Delay Line Register 1
#define        DDR_PHY_DX4LCDLR2  (0x2E2 << 2) //RW DATX8 4 Local Calibrated Delay Line Register 2
#define        DDR_PHY_DX4LCDLR3  (0x2E3 << 2) //RW DATX8 4 Local Calibrated Delay Line Register 3
#define        DDR_PHY_DX4LCDLR4  (0x2E4 << 2) //RW DATX8 4 Local Calibrated Delay Line Register 4
#define        DDR_PHY_DX4LCDLR5  (0x2E5 << 2) //RW DATX8 4 Local Calibrated Delay Line Register 5
#define        DDR_PHY_DX4MDLR0   (0x2E8 << 2) //RW DATX8 4 Master Delay Line Register 0
#define        DDR_PHY_DX4MDLR1   (0x2E9 << 2) //RW DATX8 4 Master Delay Line Register 1
#define        DDR_PHY_DX4GTR0    (0x2F0 << 2) //RW DATX8 4 General Timing Register 0
#define        DDR_PHY_DX4RSR0    (0x2F4 << 2) //RW DATX8 4 Rank Status Register 0
#define        DDR_PHY_DX4RSR1    (0x2F5 << 2) //RW DATX8 4 Rank Status Register 1
#define        DDR_PHY_DX4RSR2    (0x2F6 << 2) //RW DATX8 4 Rank Status Register 2
#define        DDR_PHY_DX4RSR3    (0x2F7 << 2) //RW DATX8 4 Rank Status Register 3
#define        DDR_PHY_DX4GSR0    (0x2F8 << 2) //R  DATX8 4 General Status Register 0
#define        DDR_PHY_DX4GSR1    (0x2F9 << 2) //R  DATX8 4 General Status Register 1
#define        DDR_PHY_DX4GSR2    (0x2FA << 2) //R  DATX8 4 General Status Register 2
#define        DDR_PHY_DX4GSR3    (0x2FB << 2) //R  DATX8 4 General Status Register 3
#define        DDR_PHY_DX4GSR4    (0x2FC << 2) //R  DATX8 4 General Status Register 4
#define        DDR_PHY_DX4GSR5    (0x2FD << 2) //R  DATX8 4 General Status Register 5
#define        DDR_PHY_DX4GSR6    (0x2FE << 2) //R  DATX8 4 General Status Register 6

#define        DDR_PHY_DX5GCR0    (0x300 << 2) //RW DATX8 5 General Configuration Register 0
#define        DDR_PHY_DX5GCR1    (0x301 << 2) //RW DATX8 5 General Configuration Register 1
#define        DDR_PHY_DX5GCR2    (0x302 << 2) //RW DATX8 5 General Configuration Register 2
#define        DDR_PHY_DX5GCR3    (0x303 << 2) //RW DATX8 5 General Configuration Register 3
#define        DDR_PHY_DX5GCR4    (0x304 << 2) //RW DATX8 5 General Configuration Register 4
#define        DDR_PHY_DX5GCR5    (0x305 << 2) //RW DATX8 5 General Configuration Register 5
#define        DDR_PHY_DX5GCR6    (0x306 << 2) //RW DATX8 5 General Configuration Register 6
#define        DDR_PHY_DX5GCR7    (0x307 << 2) //RW DATX8 5 General Configuration Register 7
#define        DDR_PHY_DX5GCR8    (0x308 << 2) //RW DATX8 5 General Configuration Register 8
#define        DDR_PHY_DX5GCR9    (0x309 << 2) //RW DATX8 5 General Configuration Register 9
#define        DDR_PHY_DX5BDLR0   (0x310 << 2) //RW DATX8 5 Bit Delay Line Register 0
#define        DDR_PHY_DX5BDLR1   (0x311 << 2) //RW DATX8 5 Bit Delay Line Register 1
#define        DDR_PHY_DX5BDLR2   (0x312 << 2) //RW DATX8 5 Bit Delay Line Register 2
#define        DDR_PHY_DX5BDLR3   (0x314 << 2) //RW DATX8 5 Bit Delay Line Register 3
#define        DDR_PHY_DX5BDLR4   (0x315 << 2) //RW DATX8 5 Bit Delay Line Register 4
#define        DDR_PHY_DX5BDLR5   (0x316 << 2) //RW DATX8 5 Bit Delay Line Register 5
#define        DDR_PHY_DX5BDLR6   (0x318 << 2) //RW DATX8 5 Bit Delay Line Register 6
#define        DDR_PHY_DX5BDLR7   (0x319 << 2) //RW DATX8 5 Bit Delay Line Register 7
#define        DDR_PHY_DX5BDLR8   (0x31A << 2) //RW DATX8 5 Bit Delay Line Register 8
#define        DDR_PHY_DX5BDLR9   (0x31B << 2) //RW DATX8 5 Bit Delay Line Register 9
#define        DDR_PHY_DX5LCDLR0  (0x320 << 2) //RW DATX8 5 Local Calibrated Delay Line Register 0
#define        DDR_PHY_DX5LCDLR1  (0x321 << 2) //RW DATX8 5 Local Calibrated Delay Line Register 1
#define        DDR_PHY_DX5LCDLR2  (0x322 << 2) //RW DATX8 5 Local Calibrated Delay Line Register 2
#define        DDR_PHY_DX5LCDLR3  (0x323 << 2) //RW DATX8 5 Local Calibrated Delay Line Register 3
#define        DDR_PHY_DX5LCDLR4  (0x324 << 2) //RW DATX8 5 Local Calibrated Delay Line Register 4
#define        DDR_PHY_DX5LCDLR5  (0x325 << 2) //RW DATX8 5 Local Calibrated Delay Line Register 5
#define        DDR_PHY_DX5MDLR0   (0x328 << 2) //RW DATX8 5 Master Delay Line Register 0
#define        DDR_PHY_DX5MDLR1   (0x329 << 2) //RW DATX8 5 Master Delay Line Register 1
#define        DDR_PHY_DX5GTR0    (0x330 << 2) //RW DATX8 5 General Timing Register 0
#define        DDR_PHY_DX5RSR0    (0x334 << 2) //RW DATX8 5 Rank Status Register 0
#define        DDR_PHY_DX5RSR1    (0x335 << 2) //RW DATX8 5 Rank Status Register 1
#define        DDR_PHY_DX5RSR2    (0x336 << 2) //RW DATX8 5 Rank Status Register 2
#define        DDR_PHY_DX5RSR3    (0x337 << 2) //RW DATX8 5 Rank Status Register 3
#define        DDR_PHY_DX5GSR0    (0x338 << 2) //R  DATX8 5 General Status Register 0
#define        DDR_PHY_DX5GSR1    (0x339 << 2) //R  DATX8 5 General Status Register 1
#define        DDR_PHY_DX5GSR2    (0x33A << 2) //R  DATX8 5 General Status Register 2
#define        DDR_PHY_DX5GSR3    (0x33B << 2) //R  DATX8 5 General Status Register 3
#define        DDR_PHY_DX5GSR4    (0x33C << 2) //R  DATX8 5 General Status Register 4
#define        DDR_PHY_DX5GSR5    (0x33D << 2) //R  DATX8 5 General Status Register 5
#define        DDR_PHY_DX5GSR6    (0x33E << 2) //R  DATX8 5 General Status Register 6

#define        DDR_PHY_DX6GCR0    (0x340 << 2) //RW DATX8 6 General Configuration Register 0
#define        DDR_PHY_DX6GCR1    (0x341 << 2) //RW DATX8 6 General Configuration Register 1
#define        DDR_PHY_DX6GCR2    (0x342 << 2) //RW DATX8 6 General Configuration Register 2
#define        DDR_PHY_DX6GCR3    (0x343 << 2) //RW DATX8 6 General Configuration Register 3
#define        DDR_PHY_DX6GCR4    (0x344 << 2) //RW DATX8 6 General Configuration Register 4
#define        DDR_PHY_DX6GCR5    (0x345 << 2) //RW DATX8 6 General Configuration Register 5
#define        DDR_PHY_DX6GCR6    (0x346 << 2) //RW DATX8 6 General Configuration Register 6
#define        DDR_PHY_DX6GCR7    (0x347 << 2) //RW DATX8 6 General Configuration Register 7
#define        DDR_PHY_DX6GCR8    (0x348 << 2) //RW DATX8 6 General Configuration Register 8
#define        DDR_PHY_DX6GCR9    (0x349 << 2) //RW DATX8 6 General Configuration Register 9
#define        DDR_PHY_DX6BDLR0   (0x350 << 2) //RW DATX8 6 Bit Delay Line Register 0
#define        DDR_PHY_DX6BDLR1   (0x351 << 2) //RW DATX8 6 Bit Delay Line Register 1
#define        DDR_PHY_DX6BDLR2   (0x352 << 2) //RW DATX8 6 Bit Delay Line Register 2
#define        DDR_PHY_DX6BDLR3   (0x354 << 2) //RW DATX8 6 Bit Delay Line Register 3
#define        DDR_PHY_DX6BDLR4   (0x355 << 2) //RW DATX8 6 Bit Delay Line Register 4
#define        DDR_PHY_DX6BDLR5   (0x356 << 2) //RW DATX8 6 Bit Delay Line Register 5
#define        DDR_PHY_DX6BDLR6   (0x358 << 2) //RW DATX8 6 Bit Delay Line Register 6
#define        DDR_PHY_DX6BDLR7   (0x359 << 2) //RW DATX8 6 Bit Delay Line Register 7
#define        DDR_PHY_DX6BDLR8   (0x35A << 2) //RW DATX8 6 Bit Delay Line Register 8
#define        DDR_PHY_DX6BDLR9   (0x35B << 2) //RW DATX8 6 Bit Delay Line Register 9
#define        DDR_PHY_DX6LCDLR0  (0x360 << 2) //RW DATX8 6 Local Calibrated Delay Line Register 0
#define        DDR_PHY_DX6LCDLR1  (0x361 << 2) //RW DATX8 6 Local Calibrated Delay Line Register 1
#define        DDR_PHY_DX6LCDLR2  (0x362 << 2) //RW DATX8 6 Local Calibrated Delay Line Register 2
#define        DDR_PHY_DX6LCDLR3  (0x363 << 2) //RW DATX8 6 Local Calibrated Delay Line Register 3
#define        DDR_PHY_DX6LCDLR4  (0x364 << 2) //RW DATX8 6 Local Calibrated Delay Line Register 4
#define        DDR_PHY_DX6LCDLR5  (0x365 << 2) //RW DATX8 6 Local Calibrated Delay Line Register 5
#define        DDR_PHY_DX6MDLR0   (0x368 << 2) //RW DATX8 6 Master Delay Line Register 0
#define        DDR_PHY_DX6MDLR1   (0x369 << 2) //RW DATX8 6 Master Delay Line Register 1
#define        DDR_PHY_DX6GTR0    (0x370 << 2) //RW DATX8 6 General Timing Register 0
#define        DDR_PHY_DX6RSR0    (0x374 << 2) //RW DATX8 6 Rank Status Register 0
#define        DDR_PHY_DX6RSR1    (0x375 << 2) //RW DATX8 6 Rank Status Register 1
#define        DDR_PHY_DX6RSR2    (0x376 << 2) //RW DATX8 6 Rank Status Register 2
#define        DDR_PHY_DX6RSR3    (0x377 << 2) //RW DATX8 6 Rank Status Register 3
#define        DDR_PHY_DX6GSR0    (0x378 << 2) //R  DATX8 6 General Status Register 0
#define        DDR_PHY_DX6GSR1    (0x379 << 2) //R  DATX8 6 General Status Register 1
#define        DDR_PHY_DX6GSR2    (0x37A << 2) //R  DATX8 6 General Status Register 2
#define        DDR_PHY_DX6GSR3    (0x37B << 2) //R  DATX8 6 General Status Register 3
#define        DDR_PHY_DX6GSR4    (0x37C << 2) //R  DATX8 6 General Status Register 4
#define        DDR_PHY_DX6GSR5    (0x37D << 2) //R  DATX8 6 General Status Register 5
#define        DDR_PHY_DX6GSR6    (0x37E << 2) //R  DATX8 6 General Status Register 6

#define        DDR_PHY_DX7GCR0    (0x380 << 2) //RW DATX8 7 General Configuration Register 0
#define        DDR_PHY_DX7GCR1    (0x381 << 2) //RW DATX8 7 General Configuration Register 1
#define        DDR_PHY_DX7GCR2    (0x382 << 2) //RW DATX8 7 General Configuration Register 2
#define        DDR_PHY_DX7GCR3    (0x383 << 2) //RW DATX8 7 General Configuration Register 3
#define        DDR_PHY_DX7GCR4    (0x384 << 2) //RW DATX8 7 General Configuration Register 4
#define        DDR_PHY_DX7GCR5    (0x385 << 2) //RW DATX8 7 General Configuration Register 5
#define        DDR_PHY_DX7GCR6    (0x386 << 2) //RW DATX8 7 General Configuration Register 6
#define        DDR_PHY_DX7GCR7    (0x387 << 2) //RW DATX8 7 General Configuration Register 7
#define        DDR_PHY_DX7GCR8    (0x388 << 2) //RW DATX8 7 General Configuration Register 8
#define        DDR_PHY_DX7GCR9    (0x389 << 2) //RW DATX8 7 General Configuration Register 9
#define        DDR_PHY_DX7BDLR0   (0x390 << 2) //RW DATX8 7 Bit Delay Line Register 0
#define        DDR_PHY_DX7BDLR1   (0x391 << 2) //RW DATX8 7 Bit Delay Line Register 1
#define        DDR_PHY_DX7BDLR2   (0x392 << 2) //RW DATX8 7 Bit Delay Line Register 2
#define        DDR_PHY_DX7BDLR3   (0x394 << 2) //RW DATX8 7 Bit Delay Line Register 3
#define        DDR_PHY_DX7BDLR4   (0x395 << 2) //RW DATX8 7 Bit Delay Line Register 4
#define        DDR_PHY_DX7BDLR5   (0x396 << 2) //RW DATX8 7 Bit Delay Line Register 5
#define        DDR_PHY_DX7BDLR6   (0x398 << 2) //RW DATX8 7 Bit Delay Line Register 6
#define        DDR_PHY_DX7BDLR7   (0x399 << 2) //RW DATX8 7 Bit Delay Line Register 7
#define        DDR_PHY_DX7BDLR8   (0x39A << 2) //RW DATX8 7 Bit Delay Line Register 8
#define        DDR_PHY_DX7BDLR9   (0x39B << 2) //RW DATX8 7 Bit Delay Line Register 9
#define        DDR_PHY_DX7LCDLR0  (0x3A0 << 2) //RW DATX8 7 Local Calibrated Delay Line Register 0
#define        DDR_PHY_DX7LCDLR1  (0x3A1 << 2) //RW DATX8 7 Local Calibrated Delay Line Register 1
#define        DDR_PHY_DX7LCDLR2  (0x3A2 << 2) //RW DATX8 7 Local Calibrated Delay Line Register 2
#define        DDR_PHY_DX7LCDLR3  (0x3A3 << 2) //RW DATX8 7 Local Calibrated Delay Line Register 3
#define        DDR_PHY_DX7LCDLR4  (0x3A4 << 2) //RW DATX8 7 Local Calibrated Delay Line Register 4
#define        DDR_PHY_DX7LCDLR5  (0x3A5 << 2) //RW DATX8 7 Local Calibrated Delay Line Register 5
#define        DDR_PHY_DX7MDLR0   (0x3A8 << 2) //RW DATX8 7 Master Delay Line Register 0
#define        DDR_PHY_DX7MDLR1   (0x3A9 << 2) //RW DATX8 7 Master Delay Line Register 1
#define        DDR_PHY_DX7GTR0    (0x3B0 << 2) //RW DATX8 7 General Timing Register 0
#define        DDR_PHY_DX7RSR0    (0x3B4 << 2) //RW DATX8 7 Rank Status Register 0
#define        DDR_PHY_DX7RSR1    (0x3B5 << 2) //RW DATX8 7 Rank Status Register 1
#define        DDR_PHY_DX7RSR2    (0x3B6 << 2) //RW DATX8 7 Rank Status Register 2
#define        DDR_PHY_DX7RSR3    (0x3B7 << 2) //RW DATX8 7 Rank Status Register 3
#define        DDR_PHY_DX7GSR0    (0x3B8 << 2) //R  DATX8 7 General Status Register 0
#define        DDR_PHY_DX7GSR1    (0x3B9 << 2) //R  DATX8 7 General Status Register 1
#define        DDR_PHY_DX7GSR2    (0x3BA << 2) //R  DATX8 7 General Status Register 2
#define        DDR_PHY_DX7GSR3    (0x3BB << 2) //R  DATX8 7 General Status Register 3
#define        DDR_PHY_DX7GSR4    (0x3BC << 2) //R  DATX8 7 General Status Register 4
#define        DDR_PHY_DX7GSR5    (0x3BD << 2) //R  DATX8 7 General Status Register 5
#define        DDR_PHY_DX7GSR6    (0x3BE << 2) //R  DATX8 7 General Status Register 6


//---------------------------------------------------------
//SNPS uMCLT2 register
//---------------------------------------------------------
#define   DDR_CTRL_MSTR               0x0   
#define   DDR_CTRL_STAT               0x4 
#define   DDR_CTRL_MRCTRL0            0x10 
#define   DDR_CTRL_MRCTRL1            0x14 
#define   DDR_CTRL_MRSTAT             0x18 
#define   DDR_CTRL_MRCTRL2            0x1c 
#define   DDR_CTRL_DERATEEN           0x20 
#define   DDR_CTRL_DERATEINT          0x24 
#define   DDR_CTRL_DERATECTL          0x2c 
#define   DDR_CTRL_PWRCTL             0x30 
#define   DDR_CTRL_PWRTMG             0x34 
#define   DDR_CTRL_HWLPCTL            0x38 
#define   DDR_CTRL_RFSHCTL0           0x50 
#define   DDR_CTRL_RFSHCTL1           0x54 
#define   DDR_CTRL_RFSHCTL3           0x60 
#define   DDR_CTRL_RFSHTMG            0x64 
#define   DDR_CTRL_RFSHTMG1           0x68 
#define   DDR_CTRL_CRCPARCTL0         0xc0 
#define   DDR_CTRL_CRCPARCTL1         0xc4 
#define   DDR_CTRL_CRCPARSTAT         0xcc 
#define   DDR_CTRL_INIT0              0xd0 
#define   DDR_CTRL_INIT1              0xd4 
#define   DDR_CTRL_INIT2              0xd8 
#define   DDR_CTRL_INIT3              0xdc 
#define   DDR_CTRL_INIT4              0xe0 
#define   DDR_CTRL_INIT5              0xe4 
#define   DDR_CTRL_INIT6              0xe8 
#define   DDR_CTRL_INIT7              0xec 
#define   DDR_CTRL_DIMMCTL            0xf0 
#define   DDR_CTRL_RANKCTL            0xf4
#define   DDR_CTRL_DRAMTMG0           0x100 
#define   DDR_CTRL_DRAMTMG1           0x104 
#define   DDR_CTRL_DRAMTMG2           0x108 
#define   DDR_CTRL_DRAMTMG3           0x10c 
#define   DDR_CTRL_DRAMTMG4           0x110 
#define   DDR_CTRL_DRAMTMG5           0x114 
#define   DDR_CTRL_DRAMTMG6           0x118 
#define   DDR_CTRL_DRAMTMG7           0x11c 
#define   DDR_CTRL_DRAMTMG8           0x120 
#define   DDR_CTRL_DRAMTMG9           0x124 
#define   DDR_CTRL_DRAMTMG10          0x128 
#define   DDR_CTRL_DRAMTMG11          0x12c 
#define   DDR_CTRL_DRAMTMG12          0x130 
#define   DDR_CTRL_DRAMTMG13          0x134 
#define   DDR_CTRL_DRAMTMG14          0x138 
#define   DDR_CTRL_DRAMTMG15          0x13c 
#define   DDR_CTRL_ZQCTL0             0x180 
#define   DDR_CTRL_ZQCTL1             0x184 
#define   DDR_CTRL_ZQCTL2             0x188 
#define   DDR_CTRL_ZQSTAT             0x18c 
#define   DDR_CTRL_DFITMG0            0x190 
#define   DDR_CTRL_DFITMG1            0x194 
#define   DDR_CTRL_DFILPCFG0          0x198 
#define   DDR_CTRL_DFILPCFG1          0x19c 
#define   DDR_CTRL_DFIUPD0            0x1a0 
#define   DDR_CTRL_DFIUPD1            0x1a4 
#define   DDR_CTRL_DFIUPD2            0x1a8 
#define   DDR_CTRL_DFIMISC            0x1b0 
#define   DDR_CTRL_DFITMG2            0x1b4 
#define   DDR_CTRL_DFITMG3            0x1b8 
#define   DDR_CTRL_DFISTAT            0x1bc 
#define   DDR_CTRL_DBICTL             0x1c0 
#define   DDR_CTRL_DFIPHYMSTR         0x1c4 
#define   DDR_CTRL_ADDRMAP0           0x200 
#define   DDR_CTRL_ADDRMAP1           0x204 
#define   DDR_CTRL_ADDRMAP2           0x208 
#define   DDR_CTRL_ADDRMAP3           0x20c 
#define   DDR_CTRL_ADDRMAP4           0x210 
#define   DDR_CTRL_ADDRMAP5           0x214 
#define   DDR_CTRL_ADDRMAP6           0x218 
#define   DDR_CTRL_ADDRMAP7           0x21c 
#define   DDR_CTRL_ADDRMAP8           0x220 
#define   DDR_CTRL_ADDRMAP9           0x224 
#define   DDR_CTRL_ADDRMAP10          0x228 
#define   DDR_CTRL_ADDRMAP11          0x22c 
#define   DDR_CTRL_ODTCFG             0x240 
#define   DDR_CTRL_ODTMAP             0x244 
#define   DDR_CTRL_SCHED              0x250 
#define   DDR_CTRL_SCHED1             0x254 
#define   DDR_CTRL_PERFHPR1           0x25c 
#define   DDR_CTRL_PERFLPR1           0x264 
#define   DDR_CTRL_PERFWR1            0x26c 
#define   DDR_CTRL_DBG0               0x300 
#define   DDR_CTRL_DBG1               0x304 
#define   DDR_CTRL_DBGCAM             0x308 
#define   DDR_CTRL_DBGCMD             0x30c 
#define   DDR_CTRL_DBGSTAT            0x310 
#define   DDR_CTRL_SWCTL              0x320 
#define   DDR_CTRL_SWSTAT             0x324 
#define   DDR_CTRL_POISONCFG          0x36c 
#define   DDR_CTRL_POISONSTAT         0x370 
#define   DDR_CTRL_DERATESTAT         0x3f0 
#define   DDR_CTRL_PSTAT              0x3fc 
#define   DDR_CTRL_PCCFG              0x400
#define   DDR_CTRL_PCFGR_0            0x404 
#define   DDR_CTRL_PCFGW_0            0x408 
#define   DDR_CTRL_PCTRL_0            0x490 
#define   DDR_CTRL_PCFGQOS0_0         0x494 
#define   DDR_CTRL_PCFGQOS1_0         0x498 
#define   DDR_CTRL_PCFGWQOS0_0        0x49c 
#define   DDR_CTRL_PCFGWQOS1_0        0x4a0 
#define   DDR_CTRL_PCFGR_1            0x404 + 0x0b0
#define   DDR_CTRL_PCFGW_1            0x408 + 0x0b0
#define   DDR_CTRL_PCTRL_1            0x490 + 0x0b0 
#define   DDR_CTRL_PCFGQOS0_1         0x494 + 0x0b0 
#define   DDR_CTRL_PCFGQOS1_1         0x498 + 0x0b0 
#define   DDR_CTRL_PCFGWQOS0_1        0x49c + 0x0b0
#define   DDR_CTRL_PCFGWQOS1_1        0x4a0 + 0x0b0
#define   DDR_CTRL_PCFGR_2            0x404 + (0x0b0 * 2)
#define   DDR_CTRL_PCFGW_2            0x408 + (0x0b0 * 2)
#define   DDR_CTRL_PCTRL_2            0x490 + (0x0b0 * 2)
#define   DDR_CTRL_PCFGQOS0_2         0x494 + (0x0b0 * 2)
#define   DDR_CTRL_PCFGQOS1_2         0x498 + (0x0b0 * 2)
#define   DDR_CTRL_PCFGWQOS0_2        0x49c + (0x0b0 * 2)
#define   DDR_CTRL_PCFGWQOS1_2        0x4a0 + (0x0b0 * 2)
#define   DDR_CTRL_PCFGR_3            0x404 + (0x0b0 * 3)
#define   DDR_CTRL_PCFGW_3            0x408 + (0x0b0 * 3)
#define   DDR_CTRL_PCTRL_3            0x490 + (0x0b0 * 3)
#define   DDR_CTRL_PCFGQOS0_3         0x494 + (0x0b0 * 3)
#define   DDR_CTRL_PCFGQOS1_3         0x498 + (0x0b0 * 3)
#define   DDR_CTRL_PCFGWQOS0_3        0x49c + (0x0b0 * 3)
#define   DDR_CTRL_PCFGWQOS1_3        0x4a0 + (0x0b0 * 3)
#define   DDR_CTRL_PCFGR_4            0x404 + (0x0b0 * 4)
#define   DDR_CTRL_PCFGW_4            0x408 + (0x0b0 * 4)
#define   DDR_CTRL_PCTRL_4            0x490 + (0x0b0 * 4)
#define   DDR_CTRL_PCFGQOS0_4         0x494 + (0x0b0 * 4)
#define   DDR_CTRL_PCFGQOS1_4         0x498 + (0x0b0 * 4)
#define   DDR_CTRL_PCFGWQOS0_4        0x49c + (0x0b0 * 4)
#define   DDR_CTRL_PCFGWQOS1_4        0x4a0 + (0x0b0 * 4)