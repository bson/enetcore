// Copyright (c) 2021 Jan Brittenson
// See LICENSE for details.

#ifndef __STM32_CLOCK_TREE_H__
#define __STM32_CLOCK_TREE_H__

#include <stdint.h>

class Stm32ClockTree {
    // Register byte offsets
    enum {
        RCC_CR          = 0x00, // Source control reg
        RCC_HSICFGR     = 0x04, // HSI configuration
        RCC_CRRCR       = 0x08, // Clock recovery CR
        RCC_CSICFGR     = 0x0c, // CSI configuration
        RCC_CFGR        = 0x10, // Clock configuration
        RCC_D1CFGR      = 0x18, // D1 config
        RCC_D2CFGR      = 0x1c, // D2 config
        RCC_D3CFGR      = 0x20, // D3 config
        RCC_PLLCKSELR   = 0x28, // PLL clock source sel
        RCC_PLLCFGR     = 0x2c, // PLL conf
        RCC_PLL1DIVR    = 0x30, // PLL1 dividers conf
        RCC_PLL1FRACR   = 0x34, // PLL1 fractional div
        RCC_PLL2DIVR    = 0x38, // PLL2 dividers conf
        RCC_PLL2FRACR   = 0x3c, // PLL2 fractional div
        RCC_PLL3DIVR    = 0x40, // PLL3 dividers conf
        RCC_PLL3FRACR   = 0x44, // PLL3 fractional div
        RCC_D1CCIPR     = 0x4c, // D1 kernel clock conf
        RCC_D2CCIP1R    = 0x50, // D2 kernel clock conf reg 1
        RCC_D2CCIP2R    = 0x54, // D2 kernel clock conf reg 2
        RCC_D3CCIPR     = 0x58, // D3 kernel clock conf
        RCC_CIER        = 0x60, // Clock source intr ena reg
        RCC_CIFR        = 0x64, // Clock source intr flag reg
        RCC_CICR        = 0x68, // Clock source intr clear reg
        RCC_BDCR        = 0x70, // Backup domain ctrl reg
        RCC_CSR         = 0x74, // Clock control and stature reg
        RCC_AHB3RSTR    = 0x7c, // AHB3 reset (same as AHB3*EN bits)
        RCC_AHB1RSTR    = 0x80, // AHB1 peripheral reset (same as AHB1*EN bits)
        RCC_AHB2RSTR    = 0x84, // AHB2 peripheral reset (same as AHB2*EN bits)
        RCC_AHB4RSTR    = 0x88, // AHB4 peripheral reset (same AS AHB4*EN bits)
        RCC_APB3RSTR    = 0x8c, // APB3 peripheral reset (same as APB3*EN bits)
        RCC_APB1LRSTR   = 0x90, // APB1 peripheral reset (same as APB1L*EN bits)
        RCC_APB1HRSTR   = 0x94, // APB1 peripheral reset (same as APB1H*EN bits)
        RCC_APB2RSTR    = 0x98, // APB2 peripheral reset (same as APB2*EN bits)
        RCC_APB4RSTR    = 0x9c, // APB4 peripheral reset (same as APB4*EN bits)
        RCC_GCR         = 0xa0, // Global control
        RCC_D3AMR       = 0xa8, // D3 autonomous mode reg
        RCC_RSR         = 0xd0, // Reset status reg
        RCC_AHB3ENR     = 0xd4, // AHB3 clock reg, AHB3*EN
        RCC_AHB1ENR     = 0xd8, // AHB1 clock reg, AHB1*EN
        RCC_AHB2ENR     = 0xdc, // AHB2 clock reg, AHB2*EN
        RCC_AHB4ENR     = 0xe0, // AHB4 clock reg, AHB4*EN
        RCC_APB3ENR     = 0xe4, // APB3 clock reg, APB3*EN
        RCC_APB1LENR    = 0xe8, // APB1L clock reg, APB1L*EN
        RCC_APB1HENR    = 0xec, // APB1H clock reg, APB1H*EN
        RCC_APB2ENR     = 0xf0, // APB2 clock reg, APB2*EN
        RCC_APB4ENR     = 0xf4, // APB4 clock reg, APB4*EN
        RCC_AHB3LPENR   = 0xfc, // AHB3 sleep clock reg
        RCC_AHB1LPENR   = 0x100, // AHB1 sleep clock reg (uses AHB1*EN)
        RCC_AHB2LPENR   = 0x104, // AHB2 sleep clock reg (uses AHB2*EN)
        RCC_AHB4LPENR   = 0x0x108, // AHB4 sleep clock reg (uses AHB4*EN)
        RCC_APB3LPENR   = 0x10c, // APB3 sleep clock reg (uses APB3*EN)
        RCC_APB1LLPENR  = 0x110, // APB1L sleep clock reg (uses APB1L*EN)
        RCC_APB1HLPENR  = 0x114, // APB1H sleep clock reg (uses APB1H*EN)
        RCC_APB2LPENR   = 0x118, // APB2 sleep clock reg (uses APB2*EN)
        RCC_APB4LPENR   = 0x11c, // APB4 sleep clock reg (uses APB4*EN)

    };

    // Register bits
    enum {
        // RCC_CR
        PLL3RDY = 29,
        PLL3ON = 28,
        PLL2RDY = 27,
        PLL2ON = 26,
        PLL1RDY = 25,
        PLL1ON = 24,
        HSECSSON = 19,          // New in H7
        HSEBYP = 18,            // New in H7
        HSERDY = 17,
        HSEON = 16,
        D2CKRDY = 15,           // D2 clock ready
        D1CKRDY = 14,           // D1 clock ready
        HSI48RDY = 13,          // HSI48 clock ready
        HSI45ON = 12,           // HSI48 clock enable
        CSIKERON = 9,           // CSI clock enable in Stop mode
        CSIRDY = 8,             // CSI clock ready
        CSION = 7,              // CSI clock enable
        HSIDIVF = 5,            // HSI divider changed flag
        HSIDIV = 3,             // HSI divider, 2-bit field
        HSIRDY = 2,
        HSIKERON = 1,           // HSI enable in Stop mode
        HSION = 0,

        // RCC_HSICFGR
        HSITRIM = 24,           // HSI trimming, 7-bit field
        HSICAL = 0,             // HSI clock calibration, 12-bit field

        // RCC_CSICFG
        CSITRIM = 24,           // CSI clock trimming, 6 bits
        CSICAL = 0,             // CSI clock calibration, 10 bits

        // RCC_CFGR
        MCO2 = 29,              // 3-bit clock output 2
        MCO2PRE = 25,           // 4-bit MCO2 prescaler
        MCO1 = 22,              // 3-bit clock output 1
        MCO1PRE = 18,           // 4-bit MCO1 prescaler
        TIMPRE = 15,            // Timing clock prescale from kernel clock, for APB1,APB2 timers
        HRTIMSEL = 14,          // HRTIM prescaler selection (0: same as other timers, 1: CPU clock)
        RTCPRE = 8,             // 6-bit HSE division factor for RTC clock (must be < 1MHz)
        STOPKERWUCK = 7,        // Kernel clock selection after wake up from system Stop (0 HSI, 1 CSI)
        STOPWUCK = 6,           // System clock selection after a wake up from system Stop
        SWS = 3,                // 3-bit System clock switch status
        SW = 0,                 // 3-bit System clock switch (SysClkSource)

        // RCC_D1CFGR
        D1CPRE = 8,             // 4-bit D1 domain Core bus ck prescaler (sys_ck/1, 2, 4... 512)
        D1PPRE = 4,             // 3-bit D1 domain APB3 bus ck prescaler (rcc_pclk3), specific values
        HPRE = 0,               // 4-bit D1 domain AHB bus ck prescaler

        // RCC_D2CFGR
        D2PPRE2 = 8,            // 3-bit D2 domain APB2 bus ck prescaler; rcc_pclk2 = rcc_hclk1/n
        D2PPRE1 = 4,            // 3-bit D2 domain APB1 bus ck prescaler; rcc_pclk1 = rcc_hclk1/n
        
        // RCC_D3CFGR
        D3PPRE = 3,             // 3-bit D3 domain APB4 bus ck prescaler; rcc_pclk4 = rcc_hclk4/n

        // RCC_PLLCKSELR
        DIVM3 = 20,             // 6-bit prescaler for PLL3
        DIVM2 = 12,             // 6-bit prescaler for PLL2
        DIVM1 = 4,              // 6-bit prescaler for PLL1
        PLLSRC = 0,             // 2-bit PLLx clock source sel (PllClkSource)

        // RCC_PLLCFGR
        DIVR3EN = 24,           // PLL3 DIVR divider output enable
        DIVQ3EN = 23,           // PLL3 DIVQ divider output enable
        DIVP3EN = 22,           // PLL3 DIVP divider output enable
        DIVR2EN = 21,           // PLL2 DIVR divider output enable
        DIVQ2EN = 20,           // PLL2 DIVQ divider output enable
        DIVP2EN = 19,           // PLL2 DIVP divider output enable
        DIVR1EN = 18,           // PLL1 DIVR divider output enable
        DIVQ1EN = 17,           // PLL1 DIVQ divider output enable
        DIVP1EN = 16,           // PLL1 DIVP divider output enable

        PLL3RGE = 10,           // 2-bit PLL3 input freq range, PllInputRange
        PLL3VCOSEL = 9,         // PLL3 VCO range selection (0: wide = 192-960MHz, 1: medium = 150-420Mhz)
        PLL3FRACGEN = 8,        // PLL3 frac latch ena
        
        PLL2RGE = 6,            // 2-bi2 PLL3 input freq range, PllInputRange
        PLL2VCOSEL = 5,         // PLL2 VCO range selection (0: wide = 192-960MHz, 1: medium = 150-420Mhz)
        PLL2FRACGEN = 4,        // PLL2 frac latch ena
        
        PLL2RGE = 2,            // 2-bi2 PLL3 input freq range, PllInputRange
        PLL2VCOSEL = 1,         // PLL2 VCO range selection (0: wide = 192-960MHz, 1: medium = 150-420Mhz)
        PLL2FRACGEN = 0,        // PLL2 frac latch ena
        
        // RCC_PLL1DIVR, RCC_PLL2DIVR, RCC_PLL2DIVR
        DIVR = 24,             // 7-bit PLL1/2/3 DIVR div fac (0 => 1 ... 127 -> 128)
        DIVQ = 16,             // 7-bit PLL1/2/3 DIVQ div fac (0 => 1 ... 127 -> 128)
        DIVP = 9,              // 7-bit PLL1/2/3 DIVP div fac (0 => 1 ... 127 -> 128)
        DIVN = 0,              // 9-bit PLL1/2/3 multiplier (3 => 4 ... 511 -> 512)

        // RCC_PLL1FRACR, RCC_PLL2FRACR, RCC_PLL3FRACR
        FRACN = 2,             // 13-bit PLL1/2/3 fractional multiplier

        // RCC_D1CCIPR
        CKPERSEL = 28,          // 2-bit per_ck ck src sel (0: hsi_ker_ck, 1: csi_ker_ck, 2_he_ck, 3: off)
        SDMMCSEL = 16,          // SDMMC kernel ck src sel (0: pll1_q_ck, 1: pll2_r_clk)
        QSPISEL = 4, // 2-bit QSPI kernel ck src srl (0:rcc_hclk3, 1:pll1_q_ck, 2:pll2_r_ck, 3:per_ck)
        FMCSEL = 0, // 2-bit FMC kernel ck src sel (0:rcc_hclk3, 1:pll1_q_clk, 2:pll2_r_clk, 3:per_ck)

        // RCC_D2CCIP1R
        SWPSEL = 31,            // SWPMI kernel ck src sel (0: pclk, 1: hsi_ker_ck)
        FDCANSEL = 28, // 2-bit FDCAN kernel ck src sel (0: cse_ck, 1: pll1_q_ck, 2: pll2_q_ck, 3: off)
        DFSDM1SEL = 24,         // DFSDM1 kernel ck src sel (0: rcc_pclk2, 1: sys_ck)
        SPDIFSEL = 20, // 2-bit SPDIFRX kernel ck src sel (0: pll1_q_clk, 1: pll2_r_ck, 2: pll3_r_ck, 3: hsi_ker_ck)
        SPI45SEL = 16, // 3-bit SPI4,5 ck sel (0: APB ck, 1: pll2_q_ck, 2: pll3_q_ck, 3: hsi_ker_ck, 5: hse_ck, others: off)
        SPI123SEL = 12, // 3-bit SPI/I2S 1,2,3 ck sel (0: pll1_q, 1: pll2_p, 2: pll3_p, 3: I2S_CKIN, 4: per_ck, others: off)
        SAI23SEL = 6,           // 3-bit SAI2,3 kernel clock source sel
        SAI1SEL = 0,            // 3-bit SAI1 and DFSDM1 kernel *Aclk* clock source sel

        // RCC_D2CCIP1R
        LPTIM1SEL = 28,         // 3-bit clock source for LPTIM1
        CECSEL = 22,            // 2-bit HDMI-CEC clock source sel 
        USBSEL = 20,            // 2-bit UBOTG1,2 clock source sel
        I2C123SEL = 13,         // 2-bit I2C1,2,3 clock sel
        RNGSEL = 8,             // 2-bit RNG kernel clock source sel
        USART16SEL = 3,         // 3-bit USART1,6 clock source sel
        USART234567SEL = 0,     // 3-bit USART2,3; UART 4,5,7/8 (APB) kernel ck src sel
        
        // RCC_D3CCIPR
        SPI6SEL = 28,           // 3-bit SPI6 kernel ck src sel
        SAI4BSEL = 24,          // 3-bit sub-block B of SAI4 kernel ck src sel
        SAI4ASEL = 21,          // 3-bit sub-block A of SAI4 kernel ck src sel
        ADCSEL = 16,            // 2-bit SAR ADC kernel ck src sel
        LPTIM345SEL = 13,       // 3-bit LPTIM3,4,5 kernel ck src sel
        LPTIM2SEL = 10,         // 3-bit LPTIM2 kernel ck src sel
        I2C4SEL = 8,            // 2-bit I2C2 kernel ck src sel
        LPUART1SEL = 0,         // 3-bit LPUART1 kernel ck src sel

        // RCC_CIER
        LSECSSIE = 9,           // LSE clock security system IE
        PLL3RDYIE = 8,          // PLL3 ready IE
        PLL2RDYIE = 7,          // PLL2 ready IE
        PLL1RDYIE = 6,          // PLL1 ready IE
        HSI48RDYIE = 5,         // HSI48 ready IE
        CSIRDYIE = 4,           // CSI ready IE
        HSERDYIE = 3,           // HSE ready IE
        HSIRDYIE = 2,           // HSI ready IE
        LSERDYIE = 1,           // LSE ready IE
        LSIRDYIE = 0,           // LSI ready IE
        
        // RCC_CIFR
        HSECSSF = 10,           // HSE clock security intr flag
        LSECSSF = 9,            // LSE clock security system F
        PLL3RDYF = 8,           // PLL3 ready F
        PLL2RDYF = 7,           // PLL2 ready F
        PLL1RDYF = 6,           // PLL1 ready F
        HSI48RDYF = 5,          // HSI48 ready F
        CSIRDYF = 4,            // CSI ready F
        HSERDYF = 3,            // HSE ready F
        HSIRDYF = 2,            // HSI ready F
        LSERDYF = 1,            // LSE ready F
        LSIRDYF = 0,            // LSI ready F

        // RCC_CICR
        HSECSSSC = 10,          // HSE clock security intr flag
        LSECSSSC = 9,           // LSE clock security system clear
        PLL3RDYSC = 8,          // PLL3 ready clear
        PLL2RDYSC = 7,          // PLL2 ready clear
        PLL1RDYSC = 6,          // PLL1 ready clear
        HSI48RDYSC = 5,         // HSI48 ready clear
        CSIRDYSC = 4,           // CSI ready clear
        HSERDYSC = 3,           // HSE ready clear
        HSIRDYSC = 2,           // HSI ready clear
        LSERDYSC = 1,           // LSE ready clear
        LSIRDYSC = 0,           // LSI ready clear

        // RCC_BDCR
        BDRST = 16,             // Backup domain software reset
        RTCEN = 15,             // RTC clock enable
        RTCSEL = 8,             // 2-bit RTC clock source selection RtcClkSource
        LSECSSD = 6,            // LSE clock security system failure detection
        LSECSSON = 5,           // LSE clock sec sys ena
        LSEDRV = 3,             // 2-bit LSE osc driving capability
        LSEBYP = 2,             // LSE osc bypass
        LSERDY = 1,             // LSE osc ready
        LSEON = 0,              // LSE osc on

        // RCC_CSR
        LSIRDY = 1,
        LSION = 0,

        // RCC_GCR
        WW1RSC = 0,             // WWDG1 reset scope control (set to 1 before enabling WWDG1)

        // RCC_D3AMR
        SRAM4AMEN = 29,         // SRAM4 autonomous mode enable
        BKPRAMAMEN = 28,        // Backup RAM AMEN
        ADC3AMEN = 24,          // ADC3 AMEN
        SAI4AMEN = 41,          // SAI4 AMEN
        CRCAMEN = 19,           // CRC AMEN
        RTCAMEN = 16,           // RTC AMEN
        VREFAMEN = 15,          // VREF AMEN
        COMP12AMEN = 14,        // COMP12 AMEN
        LPTIM5AMEN = 12,        // LPTIM5 AMEN
        LPTIM4AMEN = 11,        // LPTIM4 AMEN
        LPTIM3AMEN = 10,        // LPTIM3 AMEN
        LPTIM2AMEN = 9,         // LPTIM2 AMEN
        I2C4AMEN = 7,           // I2C4 AMEN
        SPI6AMEN = 5,           // SPI6 AMEN
        LPUART1AMEN = 3,        // LPUART1 AMEN
        BDMAAMEN = 0,           // BDMA AMEN

        // RCC_RSR
        LPWRRSTF = 30,          //  Reset due to ill D1 DStandy or CPU CStop flag
        WWDG1RST = 28,          //  Window Watchdog
        IWDG1RSTF = 26,         //  Independent Watchdog
        SFTRSTF = 24,           //  System reset from CPU
        PORRSTF = 23,           //  POR/PDR
        PINRSTF = 22,           //  Pin
        BORRSTF = 21,           //  BOR
        D2RSTF = 20,            //  D2 domain power switch
        D1RSTF = 19,            //  D1 domain power switch
        CPURSTF = 17,           //  CPU reset
        RMVF = 16,              //  Clear all reset flags
    };
    
public:
    // sys_ck sources
    enum class SysClkSource {
        HSI = 0,
        CSI = 1,
        HSE = 2,
        PLL1 = 3                // pll1_p_ck
    };

    // HSI frequencies (divider values)
    enum class HsiFreq {
        FREQ_64MHZ = 0,         // DIV 0 => 64MHz
        FREQ_32MHZ = 1,
        FREQ_16MHZ = 2,
        FREQ_8MHZ = 3
    };

    // CSI is always 4MHz
    enum { CSI_FREQ = 4000000 };

    // If using HSI, this is the desired frequency
    enum { HSI_FREQ = 8000000 };

    enum class PllClkSource {
        HSI = 0,
        CSI = 1,
        HSE = 2,
        OFF = 3
    };

    enum class RtcClkSource {
        OFF = 0,
        LSE = 1,
        LSI = 2,
        HSE = 3                 // Divided by RTCPRE
    };

    enum class PllInputRange {
        RANGE_1_2_MHZ = 0,      // 1-2 MHz
        RANGE_2_4_MHZ = 1,      // 2-4 MHz
        RANGE_4_8_MHZ = 2,      // 4-8 MHz
        RANGE_8_16_MHZ = 3      // 8-16 MHz
    };

    // PLL prescaler
    enum class PllPrescale {
        DIV2 = 0,
        DIV4 = 1,
        DIV6 = 2,
        DIV8 = 3
    };

    // HCLK prescaler
    enum class HclkPrescale {
        DIV1   = 0,
        DIV2   = 8,
        DIV4   = 9,
        DIV8   = 10,
        DIV16  = 11,
        DIV64  = 12,
        DIV128 = 13,
        DIV256 = 14,
        DIV512 = 15
    };

    // APB1,2,3,4 prescalers
    enum class ApbPrescale {
        DIV1  = 0,
        DIV2  = 4,
        DIV4  = 5,
        DIV8  = 6,
        DIV16 = 7
    };


    // BASE_PWR (PWR_CR1 at offset 0)
    enum {
        DBP = 8
    };
    
    // HSE/HSI/CSI/pll1_p_ck -> sys_ck
    //
    // sys_ck -> D1CPRE -> sys_d1pre_ck (max 480MHz)
    //   sys_d1cpre_ck -> CPU clocks
    //   sys_d1cpre_ck -> DIV8 -> systick
    //   sys_d1cpre_ck/rcc_timy_ker_ck -> HRTIMSEL -> HRTIM prescaler
    //
    //   sys_d1cpre_ck -> HPRE (/1...512) -> hpre_ck (max 240MHz)
    //
    //      hpre_ck -> rcc_aclk (AXI per clk)
    //      hpre_ck -> hclk3 (AHB3 per clk)
    //      hpre_ck -> D1PPRE -> rcc_pclk3 (APB3 per clk)
    //      hpre_ck -> rcc_hlk1,2   (AHB1, AHB2 per clk)
    //      hpre_ck -> D2PPRE1 -> rcc_pclk1, rcc_timx_ker_ck (APB1 per clk, timers prescale clk)
    //      hpre_ck -> D2PPRE2 -> rcc_pclk2, rcc_timy_ker_ck (APB2 per clk, timers prescale clk)
    //      hpre_ck -> rcc_hclk4, rcc_fclk_d3 (AHB4 per clk)
    //      hpre_ck -> D3PPRE  -> rcc_pclk4 (APB4 per clk)

    // Various constants
    enum {
        CPU_MAX  = 480000000,      // Max CPU clock
        AHB_MAX = 240000000,       // Max AHB
        RTC_MAX = 1000000          // RTC clock
        
    };

    struct Config {
        PllClkSource  pll1_clk;    // PLL1 clock source
        bool          hsi48_ena;   // Enable HSI48 OSC (NYI)
        bool          lsi_ena;     // Enable LSI
        uint32_t      hse_freq;    // If HSE is used, this is its frequency

        uint32_t      cpu_freq;    // Desired CPU clock
        uint32_t      ahb_freq;    // AHB peripheral freq, determines HPRE prescaler
        uint32_t      apb1_freq;   // APB1 peripheral frequency, determines D2PPRE1 ApbPrescaler
        uint32_t      apb2_freq;   // APB2 peripheral frequency, determines D2PPRE2 ApbPrescaler
        uint32_t      apb3_freq;   // APB3 peripheral frequency, determines D1PPRE ApbPrescaler
        uint32_t      apb4_freq;   // APB4 peripheral frequency, determines D3PPRE ApbPrescaler

        RtcClkSource  rtc_clk;     // RTC clk source
        uint8_t       rtcpre:6;    // 2-63
    };

    enum class Mco2Output {
        SYSCLK = 0,
        PLLI2S = 1,
        HSE = 2,
        PLL = 3
    };

    enum class Mco1Output {
        HSI = 0,
        LSE,
        HSE,
        PLL
    };

    enum class McoPrescaler {
        DIV1 = 0,
        DIV2 = 4,
        DIV3 = 5,
        DIV4 = 6,
        DIV5 = 7
    };


#define VREG(offset) (*((volatile uint32_t*)(BASE_RCC+(offset))))

    // Assumes running on HSI at start.
    // Starts HSI, HSI48, CSI or LSI as needed, if used as a clock source.
    // LSE may be running.

    // sys_freq, ahb_freq, apb*_freq are the main inputs:
    //   vco_freq = 2*sys_freq
    //   D1CPRE = sys_freq/cpu_freq - this must be an integer multiple
    //   HPRE = cpu_freq/ahb_freq
    //   D[123]PPRE = ahb_freq/apb*_freq
    // PLL1 is configured to produce sys_freq on pll1_p_ck
    //   Source = HSI/LSI/HSE, if HSE hse_freq is used
    //   osc_freq = HSI ? HSI_FREQ
    //            : CSI ? CSI_FREQ
    //            : HSE ? hse_freq
    //
    //   PLL1 DIVM = 1
    //   PLL1 DIVP = VCO_FREQ/sys_freq
    //   PLL1 DIVQ, DIVR = 0 (off)
    //   PLL1 DIVN = vco_freq / osc_freq

    // Example:
    //   cpu_freq = 200
    //   ahb_freq = 100
    //   apb*_freq = 20
    //   source = HSI
    // Gives:
    //   sys_freq = cpu_freq = 200 (VCOH)
    //   vco_freq = 2 * 200 = 400
    //   HSI switched to VCOL (400 < 420)
    //   PLL1 DIVM = 1
    //   PLL1 DIVN = vco_freq/osc_freq = 400/4 = 100
    //   PLL1 DIVP = vco_freq/sys_freq = 400/200 = 2 (fixed)
    //   D1CPRE = cpu_freq/sys_freq = 200/200 = 1
    //   HPRE = cpu_freq/ahb_freq = 200/100 = 2
    //   D*PRE = ahb_freq/apb*freq = 100/20 = 5
    //

    // Startup sequence, integer mode (RM0433 fig 48, p. 348)
    //
    //  1. Select clock source (RCC_CKSELR.PLLSRC) - shared for all PLLs
    //  2. Set pre-divider DIVM1 (RCC_CKSELR)
    //  3. PLL1 config:
    //     RCC_PLLCFGR:
    //       PLL1VCOSEL = VCOL (150-420)/VCOH (192-960)
    //       PLL1RGE = range
    //       PLL1FRACEN = 0
    //       DIVP1EN = 1
    //       DIVQ1EN = 0
    //       DIVR1EN = 0
    //     RCC_PLL1DIVR:
    //       DIVN1 = vco_freq/osc_freq
    //       DIVP1 = vco_freq/sys_freq
    //       DIVQ1 = DIVR1 = 0
    //  4. Enable PLL1 (RCC_CR.PLL1ON=1)
    //  5. Wait until RCC_CR.PLL1RDY = 1
    //
    // Configure AHB, APB clocks:
    //   
    // Set sys_ck to use pll1_p_ck

    static void StartPll(int num, PllInputRange range, uint16_t divm, int vcol, uint16_t divn,
                        uint16_t divp, uint16_t divq, uint16_t divr) {
        assert(divp <= 128);
        assert(divq <= 128);
        assert(divr <= 128);
        assert(divn >= 4 && divn <= 512);

        --num;

        // Turn off
        VREG(RCC__CR) &= ~BIT(num * 2 + PLL1ON);

        VREG(RCC_PLLCKSELR) &= ~(0b11111 << (DIVM1 + num * 8));
        VREG(RCC_PLLCKSELR) |= (uint32_t)divm << (DIVM1 + num * 8);

        const uint32_t cfgval = (range << 2) | (vcol << 1);
        VREG(RCC_PLLCFGR) &= ~(0b1111 << (4 * num));
        VREG(RCC_PLLCFGR) |= cfgval << (4 * num);

        const uint32_t diven = ((divr != 0) << 2) | ((divq != 0) << 1) | (divp != 0);
        VREG(RCC_PLLCFGR) &= ~(0b111 << (DIVP1EN + num * 3));
        VREG(RCC_PLLCFGR) |= diven << (DIVP1EN + num * 3);

        volatile uint32_t& divr_reg = num == 2 ? VREG(RCC_PLL3DIVR)
            : num == 1 ? VREG(RCC_PLL2DIVR) : VREG(RCC_PLL1DIVR);
        const uint32_t div = ((divr-1) << DIVR1) | ((divq-1) << DIVQ1) | ((divp-1) << DIVP1)
            | ((divn - 1) << DIVN1);
        divr_reg = div;

        // Turn on
        VREG(RCC__CR) |= BIT(num * 2 + PLL1ON);

        // Wait to stabilize
        while ((RCC_CR & BIT(num * 2 + PLL1RDY)) == 0)
            ;
    }


    // Return PLL range value for a frequency
    static PllInputRange range_for_freq(uint32_t f) {
        assert(f >= 1000000 && f <= 16000000);
        f /= 1000000;
        if (f <= 2) return PllInputRange::RANGE_1_2_MHZ;
        if (f <= 4) return PllInputRange::RANGE_2_4_MHZ;
        if (f <= 8) return PllInputRange::RANGE_4_8_MHZ;
        return PllInputRange::RANGE_8_16_MHZ;
    }


    // Find highest bit set
    static uint32_t fhs(uint32_t val) {
        assert(val != 0);

        for (int bitnum = 31; bitnum >= 0; --bitnum) {
            if (val & BIT(31))
                return bitnum;
            val <<= 1;
        }

        // Not reached
        return 0;
    }


    static void Configure(const Config& config) {
        uin32_t osc_freq;
        switch (config.pll1_clk) {
        case HSI: osc_freq = HSI_FREQ; break;
        case HSE: osc_freq = config.hse_freq; break;
        case CSI: osc_freq = CSI_FREQ; break;
        default: osc_freq = 0; break;
        }

        assert(osc_freq != 0);
        assert(config.cpu_freq <= MAX_CPU_FREQ);
        assert(config.ahb_freq <= MAX_AHB_FREQ && config.ahb_freq <= config.cpu_freq);
        assert(config.apb1_freq <= config.ahb_freq);
        assert(config.apb2_freq <= config.ahb_freq);
        assert(config.apb3_freq <= config.ahb_freq);
        assert(config.apb4_freq <= config.ahb_freq);

        // These should be multiples
        assert((config.cpu_freq % osc_freq) == 0);
        assert((config.cpu_freq % config.ahb_freq) == 0);

        const uint32_t sys_freq = config.cpu_freq;
        const uint32_t vco_freq = 2 * sys_freq;
        const int vcol = vco_freq < 420000000;
        const uint16_t pll1_divn = vco_freq / osc_freq;
        const uint16_t pll1_divp = vco_freq / sys_freq;
        const uint16_t d1cpre = config.cpu_freq / sys_freq;
        const uint16_t hpre = config.cpu_freq / config.ahb_freq;
        const uint16_t d2ppre1 = config.ahb_freq / apb1_freq;
        const uint16_t d2ppre2 = config.ahb_freq / apb2_freq;
        const uint16_t d1ppre = config.ahb_freq / apb3_freq;
        const uint16_t d3ppre = config.ahb_freq / apb4_freq;

        assert(vco_freq >= 150000000 && vco_freq <= 960000000);
        assert(d1cpre >= 0 && d1cpre <= 512);
        assert(hpre >= 1 && hpre <= 512);
        assert(d1ppre >= 1 && d1ppre <= 16);
        assert(d2ppre1 >= 1 && d2ppre1 <= 16);
        assert(d2ppre2 >= 1 && d2ppre2 <= 16);
        assert(d3ppre >= 1 && d3ppre <= 16);
        assert(pll1_divn >= 4 && pll1_div <= 512);
        assert(pll1_divp >= 2 && pll1_divp <= 128);


        // Start HSE if used as source
        if (config.pll_clk == PllClkSource::HSE || config.rtc_clk == RtcClkSource::HSE) {
            VREG(RCC_CR) |= BIT(HSEON);
            while ((VREG(RCC_CR) & BIT(HSERDY)) == 0)
                ;
        }

        // If HSI is used, drop it to 8MHz before using it as a PLL source
        if (config.pll_clk == PllClkSource::HSI || config.rtc_clk == RtcClkSource::HSI) {
            VREG(RCC_CR) &= ~(0b11 << HSIDIV);
            VREG(RCC_CR) |= HsiFreq::FREQ_8MHZ << HSIDIV;
        }

        // Set clock source for PLLs
        VREG(RCC_PLLCKSELR) = config.pll_clk;

        // Start PLL1
        StartPll(1, range_for_freq(osc_freq), 1, vcol, divn, divp, 0, 0);

        // These default to unprescaled, so set them before switching over the system clock

        // Set prescalers for CPU, AHB, APB3
        // D1CPRE (CPU), HPRE (AHB), D1PPRE (APB3)
        const uint32_t d1cpre_val = d1cpre ? (0b1000 | fhs(d1cpre) - 1) : 0;
        const uint32_t d1ppre_val = d1ppre ? (0b100 | fhs(d1ppre) -1) : 0;
        const uint32_t hpre_val = hpre ? (0b1000 | fhs(hpre) - 1) : 0;
        VREG(RCC_D1CFGR) = (d1cpre_val << D1CPRE) | (d1ppre_val << D1PPRE) | (hpre_val << HPRE);
        
        // Set prescalers for APB2, APB1
        const uint32_t d2ppre2_val = d2ppre2 ? (0b100 | fhs(d2ppre2) - 1) : 0;
        const uint32_t d2ppre1_val = d2ppre1 ? (0b100 | fhs(d2ppre1) - 1) : 0;
        VREG(RCC_D2CFGR) = (d2ppre2_val << D2PPRE2) | (d2ppre1_val << D2PPRE1);

        // Set prescalers for APB4
        const uint32_t d3ppre_val = d3ppre ? (fhs(d3ppre) - 1) : 0;
        VREG(RCC_D3CFGR) = (d3ppre_val << D3PPRE);

        // Set the system clock source
        VREG(RCC_CFGR) = (VREG(RCC_CFGR) & ~(7 << SW)) | ((uint32_t)config.sys_clk << SW);
        while ((VREG(RCC_CFGR) & (7 << SWS)) != ((uint32_t)config.sys_clk << SWS))
            ;

        // Maybe enable RTC clock
        if ((uint32_t)config.rtc_clk != (VREG(RCC_BDCR) >> RTCSEL) & 3) {
            *(volatile uint32_t*)BASE_PWR |= BIT(DBP);

            switch (config.rtc_clk) {
            case RtcClkSource::LSE:
                VREG(RCC_CSR) &= ~BIT(LSION);
                VREG(RCC_BDCR) |= BIT(LSEON);
                while ((VREG(RCC_BDCR) & BIT(LSERDY)) == 0)
                    ;
                break;

            case RtcClkSource::HSE:
                assert(config.rtc >= 2);
                VREG(RCC_BDCR) &= ~BIT(LSEON);
                VREG(RCC_CSR) &= ~BIT(LSION);
                VREG(RCC_CFGR)= (VREG(RCC_CFGR) & ~(1b111111 << RTCPRE))
                    | (config.rtcpre << RTCPRE);
                break;

            case RtcClkSource::LSI:
                VREG(RCC_BDCR) &= ~BIT(LSEON);
                VREG(RCC_CSR) |= BIT(LSION);
                while ((VREG(RCC_CSR) & BIT(LSIRDY)) == 0)
                    ;
                break;
            }
            
            VREG(RCC_BDCR) |= BIT(BDRST);
            VREG(RCC_BDCR) &= ~BIT(BDRST);

            *(volatile uint32_t*)BASE_PWR |= BIT(DBP);

            VREG(RCC_BDCR) = (VREG(RCC_BDCR) & ~(3 << RTCSEL))
                | ((uint32_t)config.rtc_clk << RTCSEL);

            VREG(RCC_BDCR) |= BIT(RTCEN);

            *(volatile uint32_t*)BASE_PWR &= ~BIT(DBP);
        }

        // Stop HSI if unused
        if (config.pll_clk != PllClkSource::HSI)
            VREG(RCC_CR) &= ~BIT(HSION);

        // Start/stop HSI48
        if (config.hsi48_ena && !(VREG(RCC_CR) & BIT(HSI48ON))) {
            VREG(RCC_CR) |= BIT(HSI48ON);
            while (!(VREG(RCC_CR) & BIT(HSI48RDY)))
                ;
        } else if (!config.hsi48_ena && (VREG(RCC_CR) & BIT(HSI48ON))) {
            VREG(RCC_CR) &= ~BIT(HSI48ON);
        }
    }

    // Output clock on MCO1, MCO2, with given divider
    static void EnableMCO(Mco1Output clk, McoPrescaler div) {
        VREG(RCC_CFGR) = (VREG(RCC_CFGR) & ~(3 << MCO1) & ~(7 << MCO1PRE))
            | ((uint32_t)clk << MCO1)
            | ((uint32_t)div << MCO1PRE);
    }
    static void EnableMCO(Mco2Output clk, McoPrescaler div) {
        VREG(RCC_CFGR) = (VREG(RCC_CFGR) & ~(3 << MCO2) & ~(7 << MCO2PRE))
            | ((uint32_t)clk << MCO2)
            | ((uint32_t)div << MCO2PRE);
    }

    // Enable/disable peripheral clocks
    static void EnableAHB1(uint32_t flags) { VREG(RCC_AHB1ENR) |= flags; }
    static void EnableAHB2(uint32_t flags) { VREG(RCC_AHB2ENR) |= flags; }
    static void EnableAHB3(uint32_t flags) { VREG(RCC_AHB3ENR) |= flags; }
    static void EnableAHB4(uint32_t flags) { VREG(RCC_AHB4ENR) |= flags; }

    static void EnableAPB1L(uint32_t flags) { VREG(RCC_APB1LENR) |= flags; }
    static void EnableAPB1H(uint32_t flags) { VREG(RCC_APB1HENR) |= flags; }
    static void EnableAPB2(uint32_t flags) { VREG(RCC_APB2ENR) |= flags; }
    static void EnableAPB3(uint32_t flags) { VREG(RCC_APB3ENR) |= flags; }
    static void EnableAPB4(uint32_t flags) { VREG(RCC_APB4ENR) |= flags; }

    static void DisableAHB1(uint32_t flags) { VREG(RCC_AHB1ENR) &= ~flags; }
    static void DisableAHB2(uint32_t flags) { VREG(RCC_AHB2ENR) &= ~flags; }
    static void DisableAHB3(uint32_t flags) { VREG(RCC_AHB3ENR) &= ~flags; }
    static void DisableAHB4(uint32_t flags) { VREG(RCC_AHB4ENR) &= ~flags; }

    static void DisableAPB1L(uint32_t flags) { VREG(RCC_APB1LENR) &= ~flags; }
    static void DisableAPB1H(uint32_t flags) { VREG(RCC_APB1HENR) &= ~flags; }
    static void DisableAPB2(uint32_t flags) { VREG(RCC_APB2ENR) &= ~flags; }
    static void DisableAPB3(uint32_t flags) { VREG(RCC_APB3ENR) &= ~flags; }
    static void DisableAPB4(uint32_t flags) { VREG(RCC_APB4ENR) &= ~flags; }

    // Peripheral enable/disable, low power
    static void EnableAHB1LP(uint32_t flags) { VREG(RCC_AHB1LPENR) |= flags; }
    static void EnableAHB2LP(uint32_t flags) { VREG(RCC_AHB2LPENR) |= flags; }
    static void EnableAHB3LP(uint32_t flags) { VREG(RCC_AHB3LPENR) |= flags; }
    static void EnableAHB4LP(uint32_t flags) { VREG(RCC_AHB4LPENR) |= flags; }

    static void EnableAPB1LLP(uint32_t flags) { VREG(RCC_APB1LLPENR) |= flags; }
    static void EnableAPB1HLP(uint32_t flags) { VREG(RCC_APB1HLPENR) |= flags; }
    static void EnableAPB2LP(uint32_t flags) { VREG(RCC_APB2LPENR) |= flags; }
    static void EnableAPB3LP(uint32_t flags) { VREG(RCC_APB3LPENR) |= flags; }
    static void EnableAPB4LP(uint32_t flags) { VREG(RCC_APB4LPENR) |= flags; }

    static void DisableAHB1LP(uint32_t flags) { VREG(RCC_AHB1LPENR) &= ~flags; }
    static void DisableAHB2LP(uint32_t flags) { VREG(RCC_AHB2LPENR) &= ~flags; }
    static void DisableAHB3LP(uint32_t flags) { VREG(RCC_AHB3LPENR) &= ~flags; }
    static void DisableAHB4LP(uint32_t flags) { VREG(RCC_AHB4LPENR) &= ~flags; }

    static void DisableAPB1LLP(uint32_t flags) { VREG(RCC_APB1LLPENR) &= ~flags; }
    static void DisableAPB1HLP(uint32_t flags) { VREG(RCC_APB1HLPENR) &= ~flags; }
    static void DisableAPB2LP(uint32_t flags) { VREG(RCC_APB2LPENR) &= ~flags; }
    static void DisableAPB3LP(uint32_t flags) { VREG(RCC_APB3LPENR) &= ~flags; }
    static void DisableAPB4LP(uint32_t flags) { VREG(RCC_APB4LPENR) &= ~flags; }

    // Reset cause word
    static uint32_t ResetCause() { return VREG(RCC_RSR); }

    // Check if LSE or LSI is running (indicates power loss if off)
    static bool CheckPowerLoss() {
        return ((VREG(RCC_BDCR) & BIT(LSERDY)) == 0 || (VREG(RCC_CSR) & BIT(LSIRDY)) == 0; }

    class RtcAccess {
    public:
        RtcAccess() { *(volatile uint32_t*)BASE_PWR |= BIT(DBP); }
        ~RtcAccess() { *(volatile uint32_t*)BASE_PWR &= ~BIT(DBP); }
    };
};

#undef VREG

#endif // __STM32_CLOCK_TREE_H__
