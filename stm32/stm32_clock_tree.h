#ifndef __STM32_CLOCK_TREE__
#define __STM32_CLOCK_TREE__

#include "bits.h"

class Stm32ClockTree {
    // Register byte offsets
    enum {
        RCC_CR       = 0x00,
        RCC_PLLCFGR  = 0x04,
        RCC_CFGR     = 0x08,
        RCC_CIR      = 0x0c,
        RCC_AHB2RSTR = 0x10,
        RCC_AHB2RSTR = 0x14,
        RCC_AHB3RSTR = 0x18,
        RCC_APB1RSTR = 0x20,
        RCC_APB2RSTR = 0x24,
        RCC_AHB1ENR = 0x30,
        RCC_AHB2ENR = 0x34,
        RCC_AHB3ENR = 0x38,
        RCC_APB1ENR = 0x40,
        RCC_APB2ENR = 0x44,
        RCC_AHB1LPENR = 0x50,
        RCC_AHB2LPENR = 0x54,
        RCC_AHB3LPENR = 0x58,
        RCC_APB1LPENR = 0x60,
        RCC_APB2LPENR = 0x64,
        RCC_BDCR    = 0x70,
        RCC_CSR     = 0x74,
        RCC_SSCGR   = 0x80,
        RCC_PLLI2SCFGR = 0x84
    };

    // Register bits
    enum {
        // RCC_CR
        PLLI2SRDY = 27,
        PLLI2SON = 26,
        PLLRDY = 25,
        PLLON = 24,
        HSERDY = 17,
        HSEON = 16,
        HSIRDY = 1,
        HSION = 0,

        // RCC_PLLCFGR
        PLLQ = 24,
        PLLSRC = 22,
        PLLP = 16,
        PLLN = 6,
        PLLM = 0,

        // RCC_CFGR
        MCO2 = 30,
        MCO2PRE = 27,
        MCO1PRE = 24,
        I2SSRC = 23,
        MCO1 = 21,
        RTCPRE = 16,
        PPRE2 = 13,
        PPRE1 = 10,
        HPRE = 4,
        SWS = 2,
        SW = 0,

        // RCC_BDCR
        BDRST = 16,
        RTCEN = 15,
        RTCSEL = 8,
        LSEBYP = 2,
        LSERDY = 1,
        LSEON = 0,

        // RCC_CSR
        LSIRDY = 1,
        LSION = 0,

        // PWR_CR
        DBP = 8
    };
    
    // Various constants
    enum {
        MAX_FREQ = 168000000,
        HSI_FREQ = 16000000
    };

public:
    // Clock sources
    enum SysClkSource {
        HSI = 0,
        HSE = 1,
        PLL = 2
    };

    enum PllClkSource {
        HSI = 0,
        HSE = 1,
        OFF
    };

    enum RtcClkSource {
        OFF = 0,
        LSI = 1,
        LSE = 2,
        HSE = 3
    };

    // Prescaler when using HSE
    enum RtcPrescaler {
    }


    // PLL prescaler
    enum PllPrescale {
        DIV2 = 0,
        DIV4 = 1,
        DIV6 = 2,
        DIV8 = 3
    };

    // HCLK prescaler
    enum HclkPrescale {
        DIV1   = 0,
        DIV2   = 8,
        DIV4   = 9,
        DIV8   = 10,
        DIV16  = 11,
        DIV64  = 12,
        DIV128 = 13,
        DIV256 = 14,
        DIV511 = 15
    };

    // APB1,2 prescalers
    enum Apb1Prescale {
        DIV1  = 0,
        DIV2  = 4,
        DIV4  = 5,
        DIV8  = 6,
        DIV16 = 7
    };

    enum I2sSource {
        PLLI2S = 0,
        I2S_CKIN_PIN = 1
    };
        
    enum PllSysClkDiv {
        DIV2 = 0,
        DIV4 = 1,
        DIV6 = 2,
        DIV8 = 3
    };

    struct Config {
        PllClkSource pll_clk_source;
        uint16_t     pll_vco_mult:9;
        uint8_t      pll_vco_div:6;
        uint8_t      pll_periph_div:4;  // 2-15
        PllSysClkDiv pll_sysclk_div;
        SysClkSrc    sys_clk_source;
        HclkPrescale hclk_prescale;
        ApbPrescale  apb1_prescale;
        ApbPrescale  apb2_prescale;
        RtcClkSource rtc_clk_source;
        uint8_t      rtc_clk_div:5; // 2-31
        I2sSource    i2s_source;
    };

    enum Mco2Output {
        SYSCLK = 0,
        PLLI2S = 1,
        HSE = 2,
        PLL = 3
    };

    enum Mco1Output {
        HSI = 0,
        LSE,
        HSE,
        PLL
    };

    enum McoPrescaler {
        DIV1 = 0,
        DIV2 = 4,
        DIV3 = 5,
        DIV4 = 6,
        DIV5 = 7
    };


#define REG(offset) (*((uint32_t*)(RCC_BASE+(offset))))
#define VREG(offset) (*((volatile uint32_t*)(RCC_BASE+(offset))))

    // Reset startup init, assumes we're running off the HSI, with HSE and PLL disabled.
    // LSE may be running.
    static void Init(const Config& config) {
        // Start HSE if used as source
        if (config.pll_clk_source == PllClkSource::HSE || config.sys_clk_source == SysClkSource::HSE
            || config.rtc_clk_source == RtcClkSource::HSE) {
            VREG(RCC_CR) |= BIT(HSEON);
            while ((VREG(RCC_CR) & BIT(HSERDY)) == 0)
                ;
        }

        // Start the main PLL if wanted
        if (config.pll_clk_source != PllClkSource::OFF) {
            VREG(RCC_PLLCFGR) = (config.pll_periph_div << PLLQ)
                | (config.pll_sysclk_div << PLLP)
                | (config.pll_vco_mult << PLLN)
                | (config.pll_vco_div << PLLM)
                | ((uint32_t)config.pll_clk_source << PLLSRC);
            VREG(RCC_CR) |= BIT(PLLON);
            while ((VREG(RCC_CR) & BIT(PLLRDY)) == 0)
                ;
        }

        // Set prescalers for AHB, APB1, APB2
        VREG(RCC_CFGR) = (REG(RCC_CFGR) & ~(0b1111 << HPRE) & ~(0b111 << PPRE1) & ~(0b111 << PPRE2))
            | ((uint32_t)hclk_prescale << HPRE)
            | ((uint32_t)apb1_prescale << PPRE1)
            | ((uint32_t)apb2_prescale << PPRE2);

        // Set the system clock source
        VREG(RCC_CFGR) = (REG(RCC_CFGR) & ~(3 << SW))
            | ((uin32_t)config.sys_clk_source << SW);
        while ((VREG(RCC_CFGR) & (3 << SWS)) != ((uint32_t)config.sys_clk_source << SWS))
            ;

        // Maybe enable RTC
        if (config.rtc_clk_source != RtcClkSource::OFF) {
            RtcAccess _ra();

            switch (config.rtc_clk_source) {
            case RtcClkSource::LSE:
                VREG(RCC_BDCR) |= BIT(LSEON);
                while ((VREG(RCC_BDCR) & BIT(LSERDY)) == 0)
                    ;
                break;
            case RtcClkSource::HSE:
                VREG(RCC_CFGR)= (REG(RCC_CFGR) & ~(31 << RTCPRE))
                    | (config.rtc_clk_div << RTCPRE);
                break;
            case RtcClkSource::LSI:
                VREG(RCC_CSR) |= BIT(LSION);
                while ((VREG(RCC_CSR) & BIT(LSIRDY)) == 0)
                    ;
                break;
            }
            VREG(RCC_BDCR) = (REG(RCC_BDCR) & ~(3 << RTCSEL))
                | ((uint32_t)config.rtc_clk_source << RTCSEL);
            VREG(RCC_BDCR) |= BIT(RTCEN);
        }

        // Stop HSI if unused
        if (config.pll_clk_source != PllClkSource::HSI
            && config.sys_clk_source != SysClkSource::HSI) {
            VREG(RCC_CR) &= ~BIT(HSION);
        }
    }

    // Output clock on MCO1, MCO2, with given divider
    static void EnableMCO(Mco1Output clk, McoPrescaler div) {
        VREG(RCC_CFGR) = (REG(RCC_CFGR) & ~(3 << MCO1) & ~(7 << MCO1PRE))
            | ((uint32_t)clk << MCO1)
            | ((uint32_t)div << MCO1PRE);
    }
    static void EnableMCO(Mco2Output clk, McoPrescaler div) {
        VREG(RCC_CFGR) = (REG(RCC_CFGR) & ~(3 << MCO2) & ~(7 << MCO2PRE))
            | ((uint32_t)clk << MCO2)
            | ((uint32_t)div << MCO2PRE);
    }

    // Enable/disable peripheral clocks
    static void EnableAHB1(uint32_t flags) { VREG(RCC_AHB1ENR) |= flags; }
    static void EnableAHB2(uint32_t flags) { VREG(RCC_AHB2ENR) |= flags; }
    static void EnableAHB3(uint32_t flags) { VREG(RCC_AHB3ENR) |= flags; }
    static void EnableAPB1(uint32_t flags) { VREG(RCC_APB1ENR) |= flags; }
    static void EnableAPB2(uint32_t flags) { VREG(RCC_APB2ENR) |= flags; }

    static void DisableAHB1(uint32_t flags) { VREG(RCC_AHB1ENR) &= ~flags; }
    static void DisableAHB2(uint32_t flags) { VREG(RCC_AHB2ENR) &= ~flags; }
    static void DisableAHB3(uint32_t flags) { VREG(RCC_AHB3ENR) &= ~flags; }
    static void DisableAPB1(uint32_t flags) { VREG(RCC_APB1ENR) &= ~flags; }
    static void DisableAPB2(uint32_t flags) { VREG(RCC_APB2ENR) &= ~flags; }

    // Peripheral enable/disable, low power
    static void EnableAHB1LP(uint32_t flags) { VREG(RCC_AHB1LPENR) |= flags; }
    static void EnableAHB2LP(uint32_t flags) { VREG(RCC_AHB2LPENR) |= flags; }
    static void EnableAHB3LP(uint32_t flags) { VREG(RCC_AHB3LPENR) |= flags; }
    static void EnableAPB1LP(uint32_t flags) { VREG(RCC_APB1LPENR) |= flags; }
    static void EnableAPB2LP(uint32_t flags) { VREG(RCC_APB2LPENR) |= flags; }

    static void DisableAHB1LP(uint32_t flags) { VREG(RCC_AHB1LPENR) &= ~flags; }
    static void DisableAHB2LP(uint32_t flags) { VREG(RCC_AHB2LPENR) &= ~flags; }
    static void DisableAHB3LP(uint32_t flags) { VREG(RCC_AHB3LPENR) &= ~flags; }
    static void DisableAPB1LP(uint32_t flags) { VREG(RCC_APB1LPENR) &= ~flags; }
    static void DisableAPB2LP(uint32_t flags) { VREG(RCC_APB2LPENR) &= ~flags; }

    // Reset cause word
    static uint32_t ResetCause() { return REG(RCC_CSR) & ~3; }

    class RtcAccess {
    public:
        RtcAccess() { VREG(BASE_PWR) |= BIT(DBP); }
        ~RtcAccess() { VREG(BASE_PWR) &= ~BIT(DBP); }
    }
};

#undef REG
#undef VREG

#endif // __STM32_CLOCK_TREE__
