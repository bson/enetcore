#ifndef __STM32_CLOCK_TREE__
#define __STM32_CLOCK_TREE__

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
        SW = 0

    };
    
    // Various constants
    enum {
        MAX_FREQ = 168000000,
        HSI_FREQ = 16000000
    };

public:
    // Clock sources
    enum ClkSource {
        HSI = 0,
        HSE = 1
    };

    // RTC clock source
    enum RtcClkSource {
        LSI = 0,
        LSE = 1,
        OFF
    };

    // PLL prescaler
    enum PllPrescale {
        DIV2 = 0,
        DIV4 = 1,
        DIV6 = 2,
        DIV8 = 3
    };

    // HCLK prescaler
    enum HclkPrescale {
        DIV1 = 0,
        DIV2,
        DIV4,
        DIV8,
        DIV16,
        DIV32,
        DIV64,
        DIV128,
        DIV256,
        DIV511
    };

    // APB1 prescaler
    enum Apb1Prescale {
        DIV1 = 0,
        DIV2,
        DIV4,
        DIV8,
        DIV16
    };

    // APB2 prescaler
    enum Apb2Prescale {
        DIV1 = 0,
        DIV2,
        DIV4,
        DIV8,
        DIV16
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
        ClkSource clk_source;
        uint32_t xtal_freq;
        boolean pll_enable;
        uint16_t pll_vco_mult:9;
        uint16_t pll_vco_div:6;
        PllSysClkDiv pll_sysclk_div;
        uint16_t pll_periph_div:4;  // 2-15
        HclkPrescale hclk_prescale;
        Apb1Prescale apb1_prescale;
        Apb2Prescale apb2_prescale;
        RtcClkSource rtc_clk_source;
        I2sSource i2s_source;
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

    enum Mco2Prescaler {
        DIV1 = 0,
        DIV2 = 4,
        DIV3 = 5,
        DIV4 = 6,
        DIV5 = 7
    };

    enum Mco1Prescaler {
        DIV1 = 0,
        DIV2 = 4,
        DIV3 = 5,
        DIV4 = 6,
        DIV5 = 7
    };

#define REG(offset) (*((uint32_t*)(RCC_BASE+(offset))))
#define VREG(offset) (*((volatile uint32_t*)(RCC_BASE+(offset))))


    static void Init(const Config& config) {
        


    }

};

#undef REG
#undef VREG

#endif // __STM32_CLOCK_TREE__
