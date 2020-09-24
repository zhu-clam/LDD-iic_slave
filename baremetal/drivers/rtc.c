#include "ck810.h"
#include "datatype.h"
#include "misc.h"
#include "rtc.h"

#define RTC_MS_OUTL 0x00
#define RTC_MS_OUTH 0x04
#define RTC_SE_OUT  0x08
#define RTC_MIN_OUT 0x0C
#define RTC_HR_OUT  0x10
#define RTC_DAY_OUTL    0x14
#define RTC_DAY_OUTH    0x18
#define RTC_YEAR_OUTL   0x1C
#define RTC_YEAR_OUTH   0x20
#define RTC_MS_MATL 0x24
#define RTC_MS_MATH 0x28
#define RTC_SE_MAT  0x2C
#define RTC_MIN_MAT 0x30
#define RTC_HR_MAT  0x34
#define RTC_DAY_MATL    0x38
#define RTC_DAY_MATH    0x3C
#define RTC_MS_LDL  0x40
#define RTC_MS_LDH  0x44
#define RTC_SE_LD   0x48
#define RTC_MIN_LD  0x4C
#define RTC_HR_LD   0x50
#define RTC_DAY_LDL 0x54
#define RTC_DAY_LDH 0x58
#define RTC_YEAR_LDL    0x5C
#define RTC_YEAR_LDH    0x60
#define RTC_DIN_L   0x64
#define RTC_DIN_H   0x68
#define RTC_CTRL    0x6C
#define RTC_INT_EN  0x70
#define RTCRIS  0x74
#define RTCMIS  0x78
#define RTCICR  0x7c
#define RTC_USER0   0x80
#define RTC_USER1   0x84

typedef struct rtc_time {
    int tm_msec;
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_yday;
} rtc_time_s;

//static int month_days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static int month_ofset[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

static inline unsigned int rtc_reg_read(unsigned int offset)
{
    return read_mreg32(CK_RTC_ADDR + offset);
}

static inline void rtc_reg_write(unsigned int offset, unsigned int val)
{
    write_mreg32(CK_RTC_ADDR + offset, val);
}

// 0: for 24 hour format
// 1: for am/pm hour format
static inline unsigned int get_hr_format() {
    return ((rtc_reg_read(RTC_CTRL) >> 4) & 0x1);
}

static int yday_to_month(int yday) {
    int i;
    for (i = 0; i < 12; i++) {
        if (yday < month_ofset[i]) {
            break;
        }
    }

    return i;
}

static int yday_to_mday(int yday) {
    return (yday - month_ofset[yday_to_month(yday) - 1]);
}

static rtc_time_s build_rtc_time(int year, int mon, int day,
                                     int hour, int min, int sec, int msec) {
    rtc_time_s rtc_time;

    rtc_time.tm_year = year;
    rtc_time.tm_mon = mon;
    rtc_time.tm_mday = day;
    rtc_time.tm_hour = hour;
    rtc_time.tm_min = min;
    rtc_time.tm_sec = sec;
    rtc_time.tm_msec = msec;
    rtc_time.tm_yday = month_ofset[mon - 1] + day;

    return rtc_time;
}

static rtc_time_s get_rtc_out() {
    unsigned int hour, msec;
    rtc_time_s rtc_time;

    rtc_time.tm_year = (rtc_reg_read(RTC_YEAR_OUTH) << 8) | rtc_reg_read(RTC_YEAR_OUTL) ;
    rtc_time.tm_yday = (rtc_reg_read(RTC_DAY_OUTH) << 8) | rtc_reg_read(RTC_DAY_OUTL) ;
    rtc_time.tm_min = rtc_reg_read(RTC_MIN_OUT);
    rtc_time.tm_sec = rtc_reg_read(RTC_SE_OUT);
    hour = rtc_reg_read(RTC_HR_OUT);
    if (get_hr_format() == 1)  {
        // The bit 7 shows the am/pm of the time, 0 for am, 1 for pm
        hour = (hour & 0x7f) + (((hour >> 7) & 0x1)  == 1) ? 0 : 12;
    }
    msec = (rtc_reg_read(RTC_MS_OUTH) << 8) | rtc_reg_read(RTC_MS_OUTL) ;
    msec = msec / 32.768;
    rtc_time.tm_hour = hour;
    rtc_time.tm_msec = msec;

    return rtc_time;
}

static void print_cur_time() {
    rtc_time_s rtc_time;

    rtc_time = get_rtc_out();
    rtc_time.tm_mon = yday_to_month(rtc_time.tm_yday);
    rtc_time.tm_mday = yday_to_mday(rtc_time.tm_yday);

    printf ("[%s:%d], time: %d-%d-%d_%d:%d:%d\n", __FUNCTION__, __LINE__,
        rtc_time.tm_year, rtc_time.tm_mon, rtc_time.tm_mday,
        rtc_time.tm_hour, rtc_time.tm_min, rtc_time.tm_sec);
}

static void set_rtc_load(rtc_time_s load_time) {
    int year, day, msec;
    unsigned int ctrl;

    year = load_time.tm_year;
    day = load_time.tm_yday;
    msec = load_time.tm_msec * 32.768;

    rtc_reg_write(RTC_YEAR_LDH, (year >> 8) & 0xff);
    rtc_reg_write(RTC_YEAR_LDL, year & 0xff);
    rtc_reg_write(RTC_DAY_LDH, (day >> 8) & 0xff);
    rtc_reg_write(RTC_DAY_LDL, day & 0xff);
    rtc_reg_write(RTC_HR_LD, load_time.tm_hour);
    rtc_reg_write(RTC_MIN_LD, load_time.tm_min);
    rtc_reg_write(RTC_SE_LD, load_time.tm_sec);
    rtc_reg_write(RTC_MS_LDH, (msec >> 8) & 0xff);
    rtc_reg_write(RTC_MS_LDL, msec & 0xff);

    // RTC_SET, need keep 1 in 18ms
    ctrl = rtc_reg_read(RTC_CTRL);
    rtc_reg_write(RTC_CTRL, ctrl | 0x2);
    udelay(1000 * 100);
    rtc_reg_write(RTC_CTRL, ctrl & ~0x2);
}

static void set_rtc_match(rtc_time_s match_time) {
    //int year;
    int day, msec;
    unsigned int ctrl;

    //year = match_time.tm_year;
    day = match_time.tm_yday;
    msec = match_time.tm_msec * 32.768;

    rtc_reg_write(RTC_DAY_MATH, (day >> 8) & 0xff);
    rtc_reg_write(RTC_DAY_MATL, day & 0xff);
    rtc_reg_write(RTC_HR_MAT, match_time.tm_hour);
    rtc_reg_write(RTC_MIN_MAT, match_time.tm_min);
    rtc_reg_write(RTC_SE_MAT, match_time.tm_sec);
    rtc_reg_write(RTC_MS_MATH, (msec >> 8) & 0xff);
    rtc_reg_write(RTC_MS_MATL, msec & 0xff);

    ctrl = rtc_reg_read(RTC_CTRL);
    rtc_reg_write(RTC_CTRL, ctrl | 0x8);
}

void CK_Rtc_Init() {
    printf ("[%s:%d], RTC enable\n", __FUNCTION__, __LINE__);
    rtc_reg_write(RTC_DIN_H, 0x80);
    rtc_reg_write(RTC_DIN_L, 0x0);
    rtc_reg_write(RTC_CTRL, 0x5);
    udelay(1000 * 100);
    printf ("[%s:%d], RTC_DIN_L=0x%x\n", __FUNCTION__, __LINE__, rtc_reg_read(RTC_DIN_L));
    printf ("[%s:%d], RTC_DIN_H=0x%x\n", __FUNCTION__, __LINE__, rtc_reg_read(RTC_DIN_H));
    printf ("[%s:%d], RTC_CTRL=0x%x\n", __FUNCTION__, __LINE__, rtc_reg_read(RTC_CTRL));
    print_cur_time();
}

void CK_Rtc_Test_Normal() {
    rtc_time_s load_time, out_time;

    printf ("[%s:%d], -----RTC normal test begin-----\n", __FUNCTION__, __LINE__);
    load_time = build_rtc_time(2018, 2, 10, 14, 30, 10, 800);
    set_rtc_load(load_time);
    print_cur_time();

    printf ("[%s:%d], check out rtc\n", __FUNCTION__, __LINE__);
    out_time = get_rtc_out();
    if ((load_time.tm_year == out_time.tm_year)
        && (load_time.tm_yday == out_time.tm_yday)
        && (load_time.tm_hour == out_time.tm_hour)
        && (load_time.tm_min == out_time.tm_min)) {
        printf ("[%s:%d], \t- - -PASS\n", __FUNCTION__, __LINE__);
    } else {
        printf ("[%s:%d], \t- - -FAILURE\n", __FUNCTION__, __LINE__);
    }

    printf ("[%s:%d], check rtc runing\n", __FUNCTION__, __LINE__);
    udelay(1000 * 1000 * 5);
    print_cur_time();
    out_time = get_rtc_out();
    if ((load_time.tm_hour == out_time.tm_hour)
        && (load_time.tm_min == out_time.tm_min)
        && (load_time.tm_sec != out_time.tm_sec)) {
        printf ("[%s:%d], \t- - -PASS\n", __FUNCTION__, __LINE__);
    } else {
        printf ("[%s:%d], \t- - -FAILURE\n", __FUNCTION__, __LINE__);
    }

    printf ("[%s:%d], -----RTC normal test end-----\n", __FUNCTION__, __LINE__);
}

void CK_Rtc_Test_Intc() {
    rtc_time_s load_time, match_time;

    printf ("[%s:%d], -----RTC interrupt test begin-----\n", __FUNCTION__, __LINE__);

    // interrupt unmasked
    rtc_reg_write(RTC_INT_EN, 0);
    // clear interrupt
    rtc_reg_write(RTCICR, 1);

    // set load time and match time
    load_time = build_rtc_time(2018, 2, 10, 14, 30, 10, 800);
    match_time = build_rtc_time(2018, 2, 10, 14, 30, 15, 800);
    set_rtc_match(match_time);
    set_rtc_load(load_time);

    // Enable interrupt
    rtc_reg_write(RTC_INT_EN, 1);

    // wait for interrupt
    while (1) {
        if (rtc_reg_read(RTCRIS) != 0) {
            break;
        }
    }

    printf ("[%s:%d], check interrupt status\n", __FUNCTION__, __LINE__);
    if (rtc_reg_read(RTCRIS) && rtc_reg_read(RTCMIS)) {
        printf ("[%s:%d], \t- - -PASS\n", __FUNCTION__, __LINE__);
    } else {
        printf ("[%s:%d], \t- - -FAILURE\n", __FUNCTION__, __LINE__);
    }

    printf ("[%s:%d], clear interrupt\n", __FUNCTION__, __LINE__);
    rtc_reg_write(RTCICR, 1);
    if (rtc_reg_read(RTCRIS) || rtc_reg_read(RTCMIS)) {
        printf ("[%s:%d], \t- - -FAILURE\n", __FUNCTION__, __LINE__);
    } else {
        printf ("[%s:%d], \t- - -PASS\n", __FUNCTION__, __LINE__);
    }

    printf ("[%s:%d], -----RTC interrupt test end-----\n", __FUNCTION__, __LINE__);
}

void CK_Rtc_Test() {
    CK_Rtc_Init();

    CK_Rtc_Test_Normal();
    CK_Rtc_Test_Intc();
}

