// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

#ifndef __RTC_H__
#define __RTC_H__

#include "core/bits.h"
#include "core/bitfield.h"

class Stm32Rtc {
    enum class Register {
        TR = 0x00,              // Time reg
        DR = 0x04,              // Date reg
        CR = 0x08,
        ISR = 0x0c,
        PRER = 0x10,
        WUTR = 0x14,
        CALIBR = 0x18,
        ALRMAR = 0x1c,
        ALRMBR = 0x20,
        WPR = 0x24,
        SSR = 0x28,             // Subsecond reg
        SHIFTR = 0x2c,
        TSTR = 0x30,
        TDSR = 0x34,
        TSSR = 0x38,
        CALR = 0x3c,
        TAMPCR = 0x40,
        ALRMASSR = 0x44,
        ALRMBSSR = 0x48,
        OR = 0x4c,              // Option reg
        BKP0R = 0x50,
        NBKPxR = 32             // 32 backup regs, BKP0R-BKP31R
    };

    enum {
        // TR
        PM = 22,
        HT = 20,
        HU = 16,
        MNT = 12,
        MNU = 8,
        ST = 4,
        SU = 0,
        
        // DR
        YT = 20,
        YU = 16,
        WDU = 13,
        MT = 12,
        MU = 8,
        DT = 4,
        DU = 0,

        // CR
        ITSE = 24,
        COE = 23,
        OSEL = 21,
        POL = 20,
        COSEL = 19,
        BKP = 18,
        SUB1H = 17,
        ADD1H = 16,
        TSIE = 15,
        WUTIE = 14,
        ALRBIE = 13,
        ALRAIE = 12,
        TSE = 11,
        WUTE = 10,
        ALRBE = 9,
        ALRAE = 8,
        FMT = 6,
        BYPSHAD = 5,
        REFCKON = 4,
        TSEDGE = 3,
        WUCKSEL = 0,

        // ISR
        ITSF = 17,
        RECALPF = 16,
        TAMP3F = 15,
        TAMP2F = 14,
        TAMP1F = 13,
        TSOVF = 12,
        TSF = 11,
        WUTF = 10,
        ALRBF = 9,
        ALRAF = 8,
        INIT = 7,
        INITF = 6,
        RSF = 5,
        INITS = 4,
        SHPF = 3,
        WUTWF = 2,
        ALRBWF = 1,
        ALRAWF = 0,

        // PRER
        PREDIV_A = 16,
        PREDIV_S = 0,

        // CALIBR
        DCS = 7,
        DC = 0,

        // ALRMAR,ALRMBR
        A_MSK4 = 31,
        A_WDSEL = 30,
        A_DT = 28,
        A_DU = 24,
        A_MSK3 = 23,
        A_PM = 22,
        A_HT = 20,
        A_HU = 16,
        A_MSK2 = 15,
        A_MNT = 12,
        A_MNU = 8,
        A_MSK1 = 7,
        A_ST = 4,
        A_SU = 0,

        // SHIFTR
        ADD1S = 31,
        SUBFS = 0,

        // TSTR
        TS_PM = 22,
        TS_HT = 20,
        TS_HU = 16,
        TS_MNT = 12,
        TS_MNU = 8,
        TS_ST = 4,
        TS_SU = 0,

        // TSDR
        TS_WDU = 13,
        TS_MT = 12,
        TS_MU = 8,
        TS_DT = 4,
        TS_DU = 0,

        // CALR
        CALP = 15,
        CALW8 = 14,
        CALW16 = 13,
        CALM = 0,

        // TAMPCR
        TAMP3MF = 24,
        TAMP3NOERASE = 23,
        TAMP3IE = 22,
        TAMP2MF = 21,
        TAMP2NOERASE = 20,
        TAMP2IE = 19,
        TAMP1MF = 18,
        TAMP1NOERASE = 17,
        TAMP1IE = 16,
        TAMPPUDIS = 15,
        TAMPPRCH = 13,
        TAMPFLT = 11,
        TAMPFREQ = 10,
        TAMPTS = 7,
        TAMP3TRG = 6,
        TAMP3E = 5,
        TAMP2TRG = 4,
        TAMP2E = 3,
        TAMPIE = 2,
        TAMP1ETRG = 1,
        TAMP1E = 0,

        // OR
        RTC_OUT_RMP = 1,
        RTC_ALARM_TYPE = 0,

        // ALRMASSR, ALRMBSSR
        MASKSS = 24,
        SS = 0,

        // BASE_PWR.CR
        DBP = 8
    };
       
    inline static volatile uint32_t& vreg(Register r) {
        return *(volatile uint32_t*)(BASE_RTC+(uint32_t)r);
    }

public:
    enum {
        EPOCH = 2020
    };

    enum DayOfWeek {
        Monday = 1,
        Tuesday = 2,
        Wednesday = 3,
        Thursday = 4,
        Friday = 5,
        Saturday = 6,
        Sunday = 7
    };

    struct DateTime {
        uint16_t  year;
        uint8_t   month;
        uint8_t   day;
        uint8_t   hour;
        uint8_t   min;
        uint8_t   sec;
        DayOfWeek dow;

        DateTime(uint16_t yy, uint8_t mm, uint8_t dd, uint8_t hh, uint8_t m, uint8_t s, DayOfWeek d)
            : year(yy), month(mm), day(dd), hour(hh), min(m), sec(s), dow(d) {
        }
    };


    // RAII for write access
    class Access {
    public:
        Access() {
            *(volatile uint32_t*)BASE_PWR |= BIT(DBP);
            vreg(Register::WPR) = 0xca;
            vreg(Register::WPR) = 0x53;
        }
        ~Access() {
            *(volatile uint32_t*)BASE_PWR &= ~BIT(DBP);
            vreg(Register::WPR) = 0xff;
        }
    };


    // Assumes a 32kHz RTC CLK
    static void SetDateTime(const DateTime& date_time) {
        assert(date_time.year >= EPOCH + 1 && date_time.year <= EPOCH + 99);
        assert(date_time.month >= 1 && date_time.month <= 12);
        assert(date_time.day >= 1 && date_time.month <= 31);
        assert(date_time.hour >= 0 && date_time.hour <= 23);
        assert(date_time.min >= 0 && date_time.min <= 59);
        assert(date_time.sec >= 0 && date_time.sec <= 59);

        Access a_;

        vreg(Register::ISR) |= BIT(INIT);
        while ((vreg(Register::ISR) & BIT(INITF)) == 0)
            ;

        vreg(Register::PRER) = Bitfield(vreg(Register::PRER))
            .f(7, PREDIV_A, 128-1);

        vreg(Register::DR) = Bitfield(vreg(Register::DR))
            .f(4, YT, (date_time.year - EPOCH) / 10)
            .bit(MT, date_time.month >= 10)
            .f(4, MU, date_time.month % 10)
            .bit(DT, date_time.day >= 10)
            .f(4, DU, date_time.day % 10)
            .f(3, WDU, date_time.dow);

        vreg(Register::TR) = Bitfield(vreg(Register::TR))
            .f(2, HT, date_time.hour / 10)
            .f(4, HU, date_time.hour % 10)
            .f(3, MNT, date_time.min / 10)
            .f(4, MU, date_time.min % 10)
            .f(3, ST, date_time.sec / 10)
            .f(4, SU, date_time.sec % 10);

        vreg(Register::CR) &= ~BIT(FMT);  // 24h

        vreg(Register::ISR) &= ~BIT(INIT);

        while ((vreg(Register::ISR) & BIT(INITS)) == 0)
            ;
    }

    static DateTime GetDateTime() {
        while ((vreg(Register::ISR) & BIT(RSF)) == 0)
            ;

        const uint32_t dr = vreg(Register::DR);
        const uint32_t tr = vreg(Register::TR);

        return DateTime(((dr >> YT) & 0xf)  * 10 + ((dr >> YU) & 0xf) + EPOCH,
                        ((dr >> MT) & 1)    * 10 + ((dr >> MU) & 0xf),
                        ((dr >> DT) & 3)    * 10 + ((dr >> DU) & 0xf),
                        ((tr >> HT) & 3)    * 10 + ((tr >> HU) & 0xf),
                        ((tr >> MNT) & 0xf) * 10 + ((tr >> MNU) & 0xf),
                        ((tr >> ST) & 3)    * 10 + ((tr >> SU) & 0xf),
                        DayOfWeek((dr >> WDU) & 7));
    }

    // Enter daylight savings (enter summer time)
    static void EnterSummerTime() {
        vreg(Register::CR) |= ADD1H;
    }

    // Exit daylight savings (enter winter time)
    static void EnterWinterTime() {
        vreg(Register::CR) |= SUB1H;
    }

    // Backup register N accessor
    static uint32_t& BackupReg(uint32_t n) {
        assert(n < (uint32_t)Register::NBKPxR);
        return *(uint32_t*)(BASE_RTC + (uint32_t)Register::BKP0R + 4*n);
    }
};

#endif // __RTC_H__
