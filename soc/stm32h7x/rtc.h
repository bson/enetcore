#ifndef __RTC_H__
#define __RTC_H__

#include "core/bits.h"

#error not yet updated for STM32H7

class Stm32Rtc {
    enum class Register {
        TR = 0x00,
        DR = 0x04,
        CR = 0x08,
        ISR = 0x0c,
        PRER = 0x10,
        WUTR = 0x14,
        CALIBR = 0x18,
        ALRMAR = 0x1c,
        ALRMBR = 0x20,
        WPR = 0x24,
        SSR = 0x28,
        SHIFTR = 0x2c,
        TSTR = 0x30,
        TSSR = 0x38,
        CALR = 0x3c,
        TAFCR = 0x40,
        ALRMASSR = 0x44,
        ALRMBSSR = 0x48,
        BKP0R = 0x50
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
        DCE = 7,
        FMT = 6,
        BYPSHAD = 5,
        REFCKON = 4,
        TSEDGE = 3,
        WCKSEL = 0,

        // ISR
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

        // WUTR
        // CALIBR
        DCS = 7,
        DC = 0,

        // ALRMAR,ALRMBR
        MSK4 = 31,
        WDSEL = 30,
        // DT = 28,
        // DU = 24,
        MSK3 = 23,
        // PM = 22,
        // HT = 20,
        // HU = 16,
        MSK2 = 15,
        // MNT = 12,
        // MNU = 8,
        MSK1 = 7,
        // ST = 4,
        // SU = 0,

        // TSTR
        // PM = 22
        // HT = 20
        // HU = 16
        // MNT = 12
        // MNU = 8
        // ST = 4
        // SU = 0

        // CALR
        CALP = 15,
        CALW8 = 14,
        CALW16 = 13,
        CALM = 0,

        // TAFCR
        ALARMOUTTYPE = 18,
        TSINSEL = 17,
        TAMP1INSEL = 16,
        TAMPPUDIS = 15,
        TAMPPRCH = 13,
        TAMPFLT = 11,
        TAMPFREQ = 8,
        TAMPTS = 7,
        TAMP2TRG = 4,
        TAMP2E = 3,
        TAMPIE = 2,
        TAMP1ETRG = 1,
        TAMP1E = 0,

        // ALRMASSR, ALRMBSSR
        MASKSS = 24,
        SS = 0,

        // PWR_CR
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

    // Assumes a 32768Hz RTC CLK
    static void SetDateTime(const DateTime& date_time) {
        assert(date_time.year >= EPOCH + 1 && date_time.year <= EPOCH + 99);
        assert(date_time.month >= 1 && date_time.month <= 12);
        assert(date_time.day >= 1 && date_time.month <= 31);
        assert(date_time.hour >= 0 && date_time.hour <= 23);
        assert(date_time.min >= 0 && date_time.min <= 59);
        assert(date_time.sec >= 0 && date_time.sec <= 59);

        *(volatile uint32_t*)BASE_PWR |= BIT(DBP);

        vreg(Register::WPR) = 0xca;
        vreg(Register::WPR) = 0x53;

        vreg(Register::ISR) |= BIT(INIT);
        while ((vreg(Register::ISR) & BIT(INITF)) == 0)
            ;

        vreg(Register::PRER) = 256-1;
        vreg(Register::PRER) = ((128-1) << PREDIV_A) | (vreg(Register::PRER) & 0x7fff);

        vreg(Register::DR) =
            (((date_time.year - EPOCH) / 10) << YT)
            | (((date_time.year - EPOCH) % 10) << YU)
            | ((date_time.month / 10) << MT)
            | ((date_time.month % 10) << MU)
            | ((date_time.day / 10) << DT)
            | ((date_time.day % 10) << DU)
            | ((uint32_t)date_time.dow << WDU);

        vreg(Register::TR) =
            ((date_time.hour / 10) << HT)
            | ((date_time.hour % 10) << HU)
            | ((date_time.min / 10) << MNT)
            | ((date_time.min % 10) << MU)
            | ((date_time.sec / 10) << ST)
            | ((date_time.sec % 10) << SU);

        vreg(Register::CR) &= ~BIT(FMT);  // 24h

        vreg(Register::WPR) = 0xff;

        vreg(Register::ISR) &= ~BIT(INIT);

        while ((vreg(Register::ISR) & BIT(INITS)) == 0)
            ;
        *(volatile uint32_t*)BASE_PWR &= ~BIT(DBP);
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

    // Enter daylight savings (summer time)
    static void EnterSummerTime() {
        vreg(Register::CR) |= ADD1H;
    }

    // Exit daylight savings (summer time)
    static void EnterWinterTime() {
        vreg(Register::CR) |= SUB1H;
    }

    // Return backup register N
    static uint32_t& BackupReg(uint32_t n) {
        assert(n <= 19);
        return *(uint32_t*)(BASE_RTC + (uint32_t)Register::BKP0R + 4*n);
    }

    class Access {
    public:
        Access() { *(volatile uint32_t*)BASE_PWR |= BIT(DBP); }
        ~Access() { *(volatile uint32_t*)BASE_PWR &= ~BIT(DBP); }
    };
};

#endif // __RTC_H__
