// Copyright (c) 2021 Jan Brittenson
// See LICENSE for details.

#include "core/enetkit.h"
#include "core/thread.h"
#include "core/util.h"
#include "core/platform.h"
#include "arch/armv7m/arm.h"
#include "arch/armv7m/fpu.h"


extern const char _build_commit[];
extern const char _build_user[];
extern const char _build_date[];
extern const char _build_branch[];

#define BOARD_REV 3

// Define to use MCO1 as clock output
#undef CLKOUTPIN

#define MAYBE_STOP \
    while(!Thread::Bootstrapped()) \
        ;

Thread* _main_thread;
void* _main_thread_stack;
void* _intr_thread_stack;

NVic _nvic;

Gpio _gpio_a(Gpio::Port::A);
Gpio _gpio_b(Gpio::Port::B);
Gpio _gpio_c(Gpio::Port::C);
Gpio _gpio_d(Gpio::Port::D);
Gpio _gpio_e(Gpio::Port::E);

PinOutput<Gpio::Pin> _led; // Green LED

Clock _clock(BASE_TIM5, APB1_TIMERCLK);
SysTimer _systimer;
Timer16 _tim6(BASE_TIM6, APB1_TIMERCLK); // DAC sample clock
Timer16 _tim11(BASE_TIM11, APB2_TIMERCLK); // LCD backlight PWM

static const uint16_t _dma1_irqs[] = {
    INTR_DMA1_Stream0, INTR_DMA1_Stream1, INTR_DMA1_Stream2, INTR_DMA1_Stream3,
    INTR_DMA1_Stream4, INTR_DMA1_Stream5, INTR_DMA1_Stream6, INTR_DMA1_Stream7
};
static const uint16_t _dma2_irqs[] = {
    INTR_DMA2_Stream0, INTR_DMA2_Stream1, INTR_DMA2_Stream2, INTR_DMA2_Stream3,
    INTR_DMA2_Stream4, INTR_DMA2_Stream5, INTR_DMA2_Stream6, INTR_DMA2_Stream7
};

Dma _dma1(BASE_DMA1, _dma1_irqs);
Dma _dma2(BASE_DMA2, _dma2_irqs);

Uart<USART3_SENDQ_SIZE, USART3_RECVQ_SIZE>
_usart3(BASE_USART3, Dma::Target::USART3_TX, Dma::Target::USART3_RX);

Uart<UART4_SENDQ_SIZE, UART4_SENDQ_SIZE>
_uart4(BASE_UART4, Dma::Target::UART4_TX, Dma::Target::UART4_RX);

Dac _dac(BASE_DAC, Dma::Target::DAC1);

SpiBus _spi2(BASE_SPI2, APB1_CLK,
             _dma1, Dma::Target::SPI2_TX, Dma::Target::SPI2_RX,
             DMA_PRIORITY_SPI2, IPL_DMA);

Swo _swo;

#ifdef ENABLE_PANEL
Panel _panel;
PinNegOutput<Gpio::Pin> _t_cs; // Touch controller SPI CS#
Gpio::Pin _t_irq;
EventObject _panel_tap(0, EventObject::MANUAL_RESET);

SpiDev _touch_dev(_spi2, &_t_cs);
TouchController _touch(_touch_dev, 480, 272);

// Backlight
PinNegOutput<Gpio::Pin> _panel_bl;
#endif

// ESP12E interface
PinOutput<Gpio::Pin> _esp_boot_sel;
PinNegOutput<Gpio::Pin> _esp_rst;

//XXX Interrupt on PA2 ESP_INT (active low)
PinNegOutput<Gpio::Pin> _esp_spi_cs0;

// SSR switch
PinOutput<Gpio::Pin> _ssr_conduct;


// Flash the LED a certain number of times to indicate a fault,
// followed by a pause, where the number of pulses indicares a cause.
void fault0(uint num) {
	for (uint n = num; n; --n) {
		_led.Raise();
        asm volatile("1: subs %0,%0,#1; bne 1b" : : "r"(CCLK/8) : );
		_led.Lower();

		if (num) {
            asm volatile("1: subs %0,%0,#1; bne 1b" : : "r"(CCLK/16) : );
        }
	}

    asm volatile("1: subs %0,%0,#1; bne 1b" : : "r"(CCLK/2) : );
}


// CON2
//   STM32F405

uint8_t* AllocThreadStack(uint size) {
    return (uint8_t*)Platform::_iram_region.GetMem(Util::Align<size_t>(size, 4));
}

Thread* AllocThreadContext() { 
    return new (Platform::_iram_region.GetMem(Util::Align<size_t>(sizeof(Thread), 4)))
        Thread();
}

void ConfigurePins() {
    //////// Pin plan ////////
    // On reset all pins are GPIO inputs, floating
    //
    // 13 PH1   OSC_OUT     Pin function follows HSE
    // 12 PH0   OSC_IN
    //
    // 9 PC15   OSC32_OUT   Pin function follows LSE
    // 8 PC14   OSC32_IN
    //
    // 47 PB10  AF7   USART3_TX      TXD       3.3V serial
    // 48 PB11  AF7   USART3_RX      RXD
    //
    // 24 PA1   AF8   UART4_RX       ESP_TX       ESP12E
    // 23 PA0   AF8   UART4_TX       ESP_RX
    // 34 PC5   out                  ESP_BOOT_SEL
    // 33 PC4   out                  ESP_RST#
    // 24 PA2   in                   ESP_INT
    // 31 PA6   AF5   SPI1_MISO      ESP_MISO
    // 32 PA7   AF5   SPI1_MOSI      ESP_MOSI
    // 30 PA5   AF5   SPI1_SCK       ESP_SCLK
    // 26 PA3   out                  ESP_SPI_CS0#
    //
    // 58 PD11  AF12  FSMC_A16       LCD_RS    Panel 8080 bus
    // 88 PD7   AF12  FSMC_NE1       LCD_CS#
    // 86 PD5   AF12  FSMC_WE#       LCD_WR#
    // 85 PD4   AF12  FSMC_OE#       LCD_RD#
    // 57 PD10  AF12  FSMC_D15       LCD_D15
    // 56 PD9   AF12  FSMC_D14       LCD_D14
    // 55 PD8   AF12  FSMC_D13       LCD_D13
    // 46 PE15  AF12  FSMC_D12       LCD_D12
    // 45 PE14  AF12  FSMC_D11       LCD_D11
    // 44 PE13  AF12  FSMC_D10       LCD_D10
    // 43 PE12  AF12  FSMC_D9        LCD_D9
    // 42 PE11  AF12  FSMC_D8        LCD_D8
    // 41 PE10  AF12  FSMC_D7        LCD_D7
    // 40 PE9   AF12  FSMC_D6        LCD_D6
    // 39 PE8   AF12  FSMC_D5        LCD_D5
    // 38 PE7   AF12  FSMC_D4        LCD_D4
    // 82 PD1   AF12  FSMC_D3        LCD_D3
    // 81 PD0   AF12  FSMC_D2        LCD_D2
    // 62 PD15  AF12  FSMC_D1        LCD_D1
    // 61 PD14  AF12  FSMC_D0        LCD_D0
    //
    // #96 PB9  AF2   TIM4_CH4                 Panel backlight, BL#
    // #96 PB9  AF3   TIM11_CH1
    // 96 PB9   out
    //
    // 71 PA12  in                  T_IRQ#     Touch controller INTR
    // 53 PB14  AF5   SPI2_MISO     T_DO       Touch controller
    // 54 PB15  AF5   SPI2_MOSI     T_DIN      Touch controller
    // 51 PB12  AF5   SPI2_SS#      T_CS#      Touch controller CS#
    // 52 PB13  AF5   SPI2_SCK      T_CLK      Touch controller clk
    //
    // 35 PB0   an    ADC12_IN8     THSENSE    Thermal sense voltage
    //
    // 93 PB7   out                 LED        LED light when high
    //
    // 29 PA4   an    DAC_OUT1      SOUND      Speaker
    // 67 PA8   AF0   MCO1          CLKOUT     MCO1 clock output
    // 15 PC0   out                 SWITCH     SSR switch
    //
    static const Stm32Gpio::PinConf pinconf[] = {
        PINCONF(B, 10,  AF,  7, NONE, LOW),
        PINCONF(B, 11,  AF,  7, NONE, LOW),
        PINCONF(A,  1,  AF,  8, NONE, LOW),
        PINCONF(A,  0,  AF,  8, NONE, LOW),
        PINCONF(C,  5, OUT,  0, NONE, MEDIUM),
        PINCONF(C,  4, OUT,  0, NONE, MEDIUM),
        PINCONF(A,  2,  IN,  0,  PDR, MEDIUM),
        PINCONF(A,  6,  AF,  5, NONE, MEDIUM),
        PINCONF(A,  7,  AF,  5, NONE, MEDIUM),
        PINCONF(A,  5,  AF,  5, NONE, MEDIUM),
        PINCONF(A,  3, OUT,  0, NONE, MEDIUM),
        PINCONF(D, 11,  AF, 12, NONE, FAST),
        PINCONF(D,  7,  AF, 12, NONE, FAST),
        PINCONF(D,  5,  AF, 12, NONE, FAST),
        PINCONF(D,  4,  AF, 12, NONE, FAST),
        PINCONF(D, 10,  AF, 12, NONE, FAST),
        PINCONF(D,  9,  AF, 12, NONE, FAST),
        PINCONF(D,  8,  AF, 12, NONE, FAST),
        PINCONF(E, 15,  AF, 12, NONE, FAST),
        PINCONF(E, 14,  AF, 12, NONE, FAST),
        PINCONF(E, 13,  AF, 12, NONE, FAST),
        PINCONF(E, 12,  AF, 12, NONE, FAST),
        PINCONF(E, 11,  AF, 12, NONE, FAST),
        PINCONF(E, 10,  AF, 12, NONE, FAST),
        PINCONF(E,  9,  AF, 12, NONE, FAST),
        PINCONF(E,  8,  AF, 12, NONE, FAST),
        PINCONF(E,  7,  AF, 12, NONE, FAST),
        PINCONF(D,  1,  AF, 12, NONE, FAST),
        PINCONF(D,  0,  AF, 12, NONE, FAST),
        PINCONF(D, 15,  AF, 12, NONE, FAST),
        PINCONF(D, 14,  AF, 12, NONE, FAST),
        //PINCONF(B,  9,  AF,  3, NONE, MEDIUM),   // TIM11_CH4
        PINCONF(B,  9, OUT,  0, NONE, MEDIUM),     // Gpio B9
        PINCONF(A, 12,  IN,  0,  PUR, MEDIUM),
        PINCONF(B, 14,  AF,  5, NONE, MEDIUM),
        PINCONF(B, 15,  AF,  5, NONE, MEDIUM),
        PINCONF(B, 12, OUT,  0, NONE, MEDIUM),
        PINCONF(B, 13,  AF,  5, NONE, MEDIUM),
        PINCONF(B,  0, ANALOG,0,NONE, LOW),
        PINCONF(B,  7, OUT,  0, NONE, LOW),
        PINCONF(A,  4, ANALOG,0,NONE, LOW),
#ifdef CLKOUTPIN
        PINCONF(A,  8,  AF,  0, NONE, FAST),
#endif
        PINCONF(C,  0, OUT,  0, NONE, LOW),
        PINCONF(END,0,  IN,  0, NONE, LOW)
    };
#undef PINCONF    

    Stm32Gpio::PortConfig(pinconf);

    _led          = _gpio_b.GetPin(7);
    _panel_bl     = _gpio_b.GetPin(9);
    _esp_boot_sel = _gpio_c.GetPin(5);
    _esp_rst      = _gpio_c.GetPin(4);
    _esp_spi_cs0  = _gpio_a.GetPin(3);
    _ssr_conduct  = _gpio_c.GetPin(0);
#ifdef ENABLE_PANEL
    _t_cs         = _gpio_b.GetPin(12);
    _t_irq        = _gpio_a.GetPin(12);
#endif

    const Stm32Eintr::EintrConf eintrconf[] = {
        EINTRCONF(A, 12, BOTH),   // T_INT#
        EINTRCONF(END, 0, BOTH)
    };
    Stm32Eintr::EintrConfig(eintrconf);
}


static void TouchInterrupt(void*) {
#ifdef ENABLE_PANEL
    if (Eintr::Pending(12)) {
        Eintr::ClearPending(12);
        Eintr::DisableInt(12);
        if (_t_irq.Test()) {
            _panel_tap.Set(2);  // Low->High
        } else {
            _panel_tap.Set(1);  // High->Low
        }
    }
#endif
}

static uint8_t _reset_reason;

// Hard system initialization.  Take control and prepare to run initializers.
[[__externally_visible]]
void hwinit0() {
    // Disable MPU
    MPU_CTRL &= ~BIT0;

    _assert_stop = true;
    _reset_reason = Stm32ClockTree::ResetCause();

    // Clear all pending interrupts
    ICPR0 = ~0;
    ICPR1 = 0x1f;

    // Set up FPU access
    Fpu::EnableAccess();

    // Zero out memory
    memset((void*)MALLOC_REGION_START, 0, MALLOC_REGION_SIZE);
    memset((void*)IRAM_REGION_START, 0, IRAM_REGION_SIZE - MAIN_THREAD_STACK);

    // Explicitly initialize before static initializers are run
    _malloc_region.Init(MALLOC_REGION_SIZE, (uint8_t*)MALLOC_REGION_START);
}


// "Soft" system initialization.  Initializers have been run now.
[[__externally_visible]]
void hwinit() {
    assert((IRAM_REGION_START & 3) == 0);
    assert((IRAM_REGION_SIZE & 3) == 0);

    if (HCLK > 144000000)
        Power::EnableVos();

    Flash::Latency(uint32_t((HCLK+1000000)/30000000));

    // Power on/off peripherals
    ClockTree::EnableAHB1(AHB1_BKPSRAMEN | AHB1_GPIOAEN | AHB1_GPIOBEN | AHB1_GPIOCEN
                          | AHB1_GPIODEN | AHB1_GPIOEEN | AHB1_DMA2EN | AHB1_DMA1EN
                          | AHB1_CCMDATARAMEN);
    ClockTree::EnableAHB1LP(AHB1_BKPSRAMEN | AHB1_GPIOAEN | AHB1_GPIOBEN | AHB1_GPIOCEN
                            | AHB1_GPIODEN | AHB1_GPIOEEN | AHB1_DMA2EN | AHB1_DMA1EN
                            | AHB1_CCMDATARAMEN);

    ClockTree::EnableAHB2(AHB2_RNGEN);
    ClockTree::EnableAHB2LP(AHB2_RNGEN);

    ClockTree::EnableAHB3(AHB3_FSMCEN);
    ClockTree::EnableAHB3LP(AHB3_FSMCEN);

    ClockTree::EnableAPB1(APB1_DACEN | APB1_PWREN | APB1_UART4EN | APB1_USART3EN | APB1_TIM5EN
                          | APB1_TIM3EN | APB1_TIM6EN | APB1_DACEN | APB1_SPI2EN);
    ClockTree::EnableAPB1LP(APB1_DACEN | APB1_PWREN | APB1_UART4EN | APB1_USART3EN | APB1_TIM5EN
                            | APB1_TIM3EN | APB1_TIM6EN | APB1_DACEN | APB1_SPI2EN);

    ClockTree::EnableAPB2(APB2_SYSCFGEN | APB2_ADC1EN | APB2_TIM11EN);
    ClockTree::EnableAPB2LP(APB2_SYSCFGEN | APB2_ADC1EN | APB2_TIM11EN);

    // Configure pins.  Don't do this before powering on GPIO.
    ConfigurePins();

    // Turn on LED
    _led.Raise();

    // 12MHz crystal
    // * 56/2 = 336MHz (VCO)
    // 336MHz/2 = 168MHz sysclk
    // 336MHz/7 = 48MHz peripheral clock
    // 168MHz/1 = 168MHz HCLK
    // 168MHz HCLK/4 = 42MHz APB1 clock
    // 168MHz HCLK/2 = 84MHz APB2 clock

    static const ClockTree::Config clkconf = {
        .pll_clk_source = ClockTree::PllClkSource::HSE,
        .pll_vco_mult   = 56,
        .pll_vco_div    = 2,
        .pll_sysclk_div = ClockTree::PllSysClkDiv::DIV2,
        .pll_periph_div = 7,
        .sys_clk_source = ClockTree::SysClkSource::PLL,
        .hclk_prescale  = ClockTree::HclkPrescale::DIV1,
        .apb1_prescale  = ClockTree::ApbPrescale::DIV4,
        .apb2_prescale  = ClockTree::ApbPrescale::DIV2,
        .rtc_clk_source = ClockTree::RtcClkSource::LSI
    };

    const bool power_reset = ClockTree::CheckPowerLoss();
    ClockTree::Configure(clkconf);

#ifdef CLKOUTPIN
    // 12/2 = 6MHz on MCO1
    ClockTree::EnableMCO(ClockTree::Mco1Output::HSE, ClockTree::McoPrescaler::DIV2);
#endif

#ifdef DEBUG
    // Enable SWO early
//    _swo.Enable(12000000);
//    _swo.Write((const uint8_t*)"SWO Trace Enabled\r\n", 19);
#endif

	// Initialize main thread and set up stacks
	Thread::Bootstrap();

    // Freeze WWDT while in breakpoint
    Debug::FreezeAPB1(Debug::APB1_WWDT_STOP);

	// Initialize NVIC after threading, because the handler expect threads
    NVic::Init(Unexpected_Interrupt, IPL_UNEXP);

    // Install system exception handlers
    NVic::InstallSystemHandler(NMI_VEC, NMI_Handler, IPL_NMI);
    NVic::InstallSystemHandler(HARD_FAULT_VEC, Hard_Fault_Exception, IPL_HW_EXC, true);
    NVic::InstallSystemHandler(MEM_MAN_FAULT_VEC, Mem_Man_Fault_Exception, IPL_HW_EXC, true);
    NVic::InstallSystemHandler(BUS_FAULT_VEC, Bus_Fault_Exception, IPL_HW_EXC, true);
    NVic::InstallSystemHandler(USAGE_FAULT_VEC, Usage_Fault_Exception, IPL_HW_EXC, true);
    NVic::InstallSystemHandler(SVCALL_VEC, SVCall_Handler, IPL_SW_EXC);
    NVic::InstallSystemHandler(SYSTICK_VEC, SysTick::Interrupt, IPL_SYSTICK);

    NVic::InstallCSWHandler(PENDSV_VEC, IPL_CSW);

    _dma1.InstallHandlers();
    _dma2.InstallHandlers();

    Random::Init();

    uint8_t buf[32];
    Random::Random(buf, sizeof buf);
    Util::RandomSeed(buf, sizeof buf);

	// Install IRQ handlers
	NVic::RouteIRQ(INTR_TIM5, Clock::Interrupt, IPL_CLOCK, &_clock);
	NVic::RouteIRQ(INTR_USART3, _usart3.Interrupt, IPL_UART, &_usart3);
    NVic::RouteIRQ(INTR_EXTI10_15, TouchInterrupt, IPL_COMM, NULL);

    AdcCommon::SetPrescaler(ADC1_PRESCALE);

    Flash::EnableIDCaching();

    // Turn on proper assert handling
    _assert_stop = false;

    _dac.Enable();

	_usart3.InitAsync(115200, _usart3.StopBits::SB_1, APB1_CLK);
	_usart3.SetInterrupts(true);

    _clock.Start();

    // Stop timers while stopped in a breakpoint
    Debug::FreezeAPB1(Debug::APB1_TIM5_STOP);
    Debug::FreezeAPB1(Debug::APB1_TIM3_STOP);
    Debug::FreezeAPB1(Debug::APB1_TIM6_STOP);

    _usart3.Write("\r\nWelcome to Enetcore\r\n");
    _usart3.SyncDrain();

	// Enable global interrupts by restoring to a non-disabled state :)
    RestoreInterrupts(0);
    SetIPL(0);

    _usart3.EnableDmaTx(_dma1, DMA_STREAM_USART3_TX, DMA_CHANNEL_USART3_TX, DMA_PRIORITY_USART3);

    // Turn LED back off
    _led.Lower();

    assert(IntEnabled());

#ifdef ENABLE_PANEL
    Fsmc::ConfigureSRAM(PANEL_BANK, PANEL_DATA_SETUP_CLK,
                        PANEL_DATA_HOLD_CLK, PANEL_BUS_TURN_CLK);
#endif
    _malloc_region.SetReserve(64);

    if (power_reset) {
        DMSG("RTC power loss - initializing");
        Rtc::SetDateTime(Rtc::DateTime(2021, 10, 16, 
                                       20, 35, 00,
                                       Rtc::DayOfWeek::Saturday));
    }

    DMSG("RCC_CSR: 0x%x", _reset_reason);
    DMSG("CCLK: %d  HCLK: %d", CCLK, HCLK);
    DMSG("APB1CLK: %d  APB2CLK: %d", APB1_CLK, APB2_CLK);
    DMSG("APB1TCLK: %d APB2TCLK: %d", APB1_TIMERCLK, APB2_TIMERCLK);

    console("\r\nCON2 Rev 1 [%s:%s %s %s]",
            _build_branch, _build_commit, _build_user, _build_date);

    DMSG("Random uint: 0x%x", Util::Random<uint>());
    DMSG("HW RNG: 0x%x", Random::Random());
}


void Unexpected_Interrupt(void*) 
{
    MAYBE_STOP;
	console("Unhandled interrupt\n");
}

void Hard_Fault_Exception(void*) 
{
#ifdef DEBUG
    asm volatile("bkpt #1" : : : );
#endif

    MAYBE_STOP;
	Thread::Exception(Thread::HARD_FAULT);
}

void NMI_Handler(void*) 
{
    MAYBE_STOP;
    ;
}

void Mem_Man_Fault_Exception(void*) 
{
    MAYBE_STOP;
	Thread::Exception(Thread::MEM_MAN_FAULT);
}

void Bus_Fault_Exception(void*) 
{
    MAYBE_STOP;
	Thread::Exception(Thread::BUS_FAULT);
}

void Usage_Fault_Exception(void*) 
{
    MAYBE_STOP;
	Thread::Exception(Thread::USAGE_FAULT);
}

void SVCall_Handler(void*) 
{
    MAYBE_STOP;
	Thread::Exception(Thread::SVCALL);
}

#ifdef ENABLE_PANEL
#define _INLINE_CXX_
#include "devices/ssd1963.cxx"
#endif // ENABLE_PANEL
