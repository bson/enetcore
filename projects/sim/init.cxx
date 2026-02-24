// Copyright (c) 2026 Jan Brittenson
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

Clock _clock(BASE_TIM5, APB2_FREQ);
SysTimer _systimer;
//Timer16 _tim6(BASE_TIM6, APB2_FREQ); // DAC sample clock


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

Uart<UART5_SENDQ_SIZE, UART5_RECVQ_SIZE>
_sart5(BASE_UART5, Dma::Target::uart5_tx_dma, Dma::Target::uart5_rx_dma, 0xffffff01);

Uart<UART7_SENDQ_SIZE, UART7_SENDQ_SIZE>
_uart7(BASE_UART7, Dma::Target::uart7_tx_dma, Dma::Target::uart7_rx_dma, 0xffffff00);

Swo _swo;

// Flash the card LED a certain number of times to indicate a fault,
// followed by a pause, where the number of pulses indicares a cause.
void fault0(uint num) {
	for (uint n = num; n; --n) {
		_card_led.Raise();
        asm volatile("1: subs %0,%0,#1; bne 1b" : : "r"(CCLK/8) : );
		_card_led.Lower();

		if (num) {
            asm volatile("1: subs %0,%0,#1; bne 1b" : : "r"(CCLK/16) : );
        }
	}

    asm volatile("1: subs %0,%0,#1; bne 1b" : : "r"(CCLK/2) : );
}


// SIM
//   STM32H753

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
    // Pin# Function       Port      Sel    Desc
    // -------------------------------------------------------------------------
    // 105  SWDIO                           Debug
    // 109  SCLK
    //
    // 143  PDR_ON                          Power-down reset (supervisor enable)
    //  25  NRST                            Reset
    // 138  BOOT0                           Flash bank selector
    //   8  OSC32_IN       PC14     --      32kHz LSE OSC input
    //   9  OSC32_OUT      PC15     --      32kHz LSE OSC out
    //
    // 100  MC01           PA8      AF0      MCO1 clock output
    //
    //  18  UART7_RX       PF6      AF7
    //  19  UART7_TX       PF7      AF7
    //  20  UART7_RTS      PF8      AF7
    //  21  UART7_CTS      PF9      AF7
    // 135  UART5_RX       PB12     AF14
    // 136  UART5_TX       PB13     AF14
    //
    // 119  FMC_NWE        PD5      AF12    WE#
    // 118  FMC_NOE        PD4      AF12    OE#
    //   3  FMC_A20        PE4      AF12
    //   2  FMC_A19        PE3      AF12
    //  82  FMC_A18        PD13     AF12
    //  81  FMC_A17        PD12     AF12
    //  80  FMC_A16        PD11     AF12
    //  90  FMC_A15        PG5      AF12
    //  89  FMC_A14        PG4      AF12
    //  88  FMC_A13        PG3      AF12
    //  87  FMC_A12        PG2      AF12
    //  57  FMC_A11        PG1      AF12
    //  56  FMC_A10        PG0      AF12
    //  55  FMC_A9         PF15     AF12
    //  54  FMC_A8         PF14     AF12
    //  53  FMC_A7         PF13     AF12
    //  50  FMC_A6         PF12     AF12
    //  15  FMC_A5         PF5      AF12
    //  14  FMC_A4         PF4      AF12
    //  13  FMC_A3         PF3      AF12
    //  12  FMC_A2         PF2      AF12
    //  11  FMC_A1         PF1      AF12
    //  10  FMC_A0         PF0      AF12
    // 141  FMC_NBL0       PE0      AF12    LB#
    // 142  FMC_NBL1       PE1      AF12    UB#
    //  79  FMC_D15        PD10     AF12
    //  78  FMC_D14        PD9      AF12
    //  77  FMC_D13        PD8      AF12
    //  68  FMC_D12        PE15     AF12
    //  67  FMC_D11        PE14     AF12
    //  66  FMC_D10        PE13     AF12
    //  65  FMC_D9         PE12     AF12
    //  64  FMC_D8         PE11     AF12
    //  63  FMC_D7         PE10     AF12
    //  60  FMC_D6         PE9      AF12
    //  59  FMC_D5         PE8      AF12
    //  58  FMC_D4         PE7      AF12
    // 115  FMC_D3         PD1      AF12
    // 114  FMC_D2         PD0      AF12
    //  86  FMC_D1         PD15     AF12
    //  85  FMC_D0         PD14     AF12
    //
    //  73  ETH_TXD0       PB12     AF11
    //  74  ETH_TXD1       PB13     AF11
    // 126  ETH_TX_EN      PG11     AF11
    //  44  ETH_RXD0       PC4      AF11
    //  45  ETH_RXD1       PC5      AF11
    //  43  ETH_CRS_DIV    PA7      AF11
    //  36  ETH_MDIO       PA2      AF11
    //  27  ETH_MDC        PC1      AF11
    //  35  ETH_REF_CLK    PA1      AF11 
    //
    // 111  SDMMC1_D2      PC10     AF12
    // 112  SDMMC1_D3      PC11     AF12
    // 116  SDMMC1_CMD     PD2      AF12
    // 113  SDMMC1_CK      PC12     AF12
    //  98  SDMMC1_D0      PC8      AF12
    //  99  SDMMC1_D1      PC9      AF12
    // 110  PA15           IN              CARD_DET#
    // 117  PD3            OUT             CARD_ACT
    //

    static const Stm32Gpio::PinConf pinconf[] = {
        PINCONF(A,  8,  AF,  0, NONE, HIGH),
        PINCONF(F,  6,  AF,  7, NONE, LOW),
        PINCONF(F,  7,  AF,  7, NONE, LOW),
        PINCONF(F,  8,  AF,  7, NONE, LOW),
        PINCONF(F,  9,  AF,  7, NONE, LOW),
        PINCONF(B, 12,  AF, 14, NONE, LOW),
        PINCONF(B, 13,  AF, 14, NONE, LOW),

        PINCONF(D,  5, AF, 12, NONE, HIGH),
        PINCONF(D,  4, AF, 12, NONE, HIGH),
        PINCONF(E,  4, AF, 12, NONE, HIGH),
        PINCONF(E,  3, AF, 12, NONE, HIGH),
        PINCONF(D,  13, AF, 12, NONE, HIGH),
        PINCONF(D,  12, AF, 12, NONE, HIGH),
        PINCONF(D,  11, AF, 12, NONE, HIGH),
        PINCONF(G,  5, AF, 12, NONE, HIGH),
        PINCONF(G,  4, AF, 12, NONE, HIGH),
        PINCONF(G,  3, AF, 12, NONE, HIGH),
        PINCONF(G,  2, AF, 12, NONE, HIGH),
        PINCONF(G,  1, AF, 12, NONE, HIGH),
        PINCONF(G,  0, AF, 12, NONE, HIGH),
        PINCONF(F,  15, AF, 12, NONE, HIGH),
        PINCONF(F,  14, AF, 12, NONE, HIGH),
        PINCONF(F,  13, AF, 12, NONE, HIGH),
        PINCONF(F,  12, AF, 12, NONE, HIGH),
        PINCONF(F,  5, AF, 12, NONE, HIGH),
        PINCONF(F,  4, AF, 12, NONE, HIGH),
        PINCONF(F,  3, AF, 12, NONE, HIGH),
        PINCONF(F,  2, AF, 12, NONE, HIGH),
        PINCONF(F,  1, AF, 12, NONE, HIGH),
        PINCONF(F,  0, AF, 12, NONE, HIGH),
        PINCONF(E,  0, AF, 12, NONE, HIGH),
        PINCONF(E,  1, AF, 12, NONE, HIGH),
        PINCONF(D,  10, AF, 12, NONE, HIGH),
        PINCONF(D,  9, AF, 12, NONE, HIGH),
        PINCONF(D,  8, AF, 12, NONE, HIGH),
        PINCONF(E,  15, AF, 12, NONE, HIGH),
        PINCONF(E,  14, AF, 12, NONE, HIGH),
        PINCONF(E,  13, AF, 12, NONE, HIGH),
        PINCONF(E,  12, AF, 12, NONE, HIGH),
        PINCONF(E,  11, AF, 12, NONE, HIGH),
        PINCONF(E,  10, AF, 12, NONE, HIGH),
        PINCONF(E,  9, AF, 12, NONE, HIGH),
        PINCONF(E,  8, AF, 12, NONE, HIGH),
        PINCONF(E,  7, AF, 12, NONE, HIGH),
        PINCONF(D,  1, AF, 12, NONE, HIGH),
        PINCONF(D,  0, AF, 12, NONE, HIGH),
        PINCONF(D,  15, AF, 12, NONE, HIGH),
        PINCONF(D,  14, AF, 12, NONE, HIGH),

        PINCONF(B,  12, AF, 11, NONE, LOW),
        PINCONF(B,  13, AF, 11, NONE, LOW),
        PINCONF(G,  11, AF, 11, NONE, LOW),
        PINCONF(C,  4, AF, 11, NONE, LOW),
        PINCONF(C,  5, AF, 11, NONE, LOW),
        PINCONF(A,  7, AF, 11, NONE, LOW),
        PINCONF(A,  2, AF, 11, NONE, LOW),
        PINCONF(C,  1, AF, 11, NONE, LOW),
        PINCONF(A,  1, AF, 11 , NONE, LOW),
        PINCONF(C,  10, AF, 12, NONE, LOW),
        PINCONF(C,  11, AF, 12, NONE, LOW),
        PINCONF(D,  2, AF, 12, NONE, LOW),
        PINCONF(C,  12, AF, 12, NONE, LOW),
        PINCONF(C,  8, AF, 12, NONE, LOW),
        PINCONF(C,  9, AF, 12, NONE, LOW),
        PINCONF(A, 15, IN, 0, NONE, LOW),
        PINCONF(D,  3, OUT, 0, NONE, MEDIUM),
    };
#undef PINCONF    

    Stm32Gpio::PortConfig(pinconf);

    _card_led = _gpio_d.GetPin(3);
    _card_act = _gpio_a.GetPin(15);
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

    // Zero out memory.  IRAM = AXI_SRAM
    memset((void*)MALLOC_REGION_START, 0, MALLOC_REGION_SIZE);
    memset((void*)IRAM_REGION_START, 0, IRAM_REGION_SIZE - MAIN_THREAD_STACK);

    memset(BASE_SRAM1, 0, SIZE_SRAM1);
    memset(BASE_SRAM2, 0, SIZE_SRAM2);
    memset(BASE_SRAM3, 0, SIZE_SRAM3);
    memset(BASE_SRAM4, 0, SIZE_SRAM4);

    // Explicitly initialize before static initializers are run
    _malloc_region.Init(MALLOC_REGION_SIZE, (uint8_t*)MALLOC_REGION_START);
}


// "Soft" system initialization.  Initializers have been run now.
[[__externally_visible]]
void hwinit() {
    assert((IRAM_REGION_START & 3) == 0);
    assert((IRAM_REGION_SIZE & 3) == 0);

    if (AHB_FREQ > 144000000)
        Power::EnableVos();

    Flash::Latency(uint32_t((AHB_FREQ+1000000)/30000000));

    // Power on/off peripherals
    ClockTree::EnableAHB1(AHB1_DMA1EN | AHB1_DMA2EN | AHB1_ETH1RXEN | AHB1_ETH1TXEN | AHB1_ETH1MACEN);
    ClockTree::EnableAHB1LP(AHB1_DMA1EN | AHB1_DMA2EN | AHB1_ETH1RXEN | AHB1_ETH1TXEN | AHB1_ETH1MACEN);


    ClockTree::EnableAHB2(AHB2_SRAM3EN | AHB2_SRAM2EN | AHB2_SRAM1EN | AHB2_RNGEN);
    ClockTree::EnableAHB2LP(AHB2_SRAM3EN | AHB2_SRAM2EN | AHB2_SRAM1EN | AHB2_RNGEN);

    ClockTree::EnableAHB3(AHB3_SDMMC1EN | AHB3_FMCEN | AHB3_DMA2EN);
    ClockTree::EnableAHB3LP(AHB3_SDMMC1EN | AHB3_FMCEN | AHB3_DMA2EN);

    ClockTree::EnableAHB4(HB4_BKPRAMEN, AHB4_CRCEN, AHB4_GPIOAEN, AHB4_GPIOBEN, AHB4_GPIOCEN,
                          AHB4_GPIODEN, AHB4_GPIOEEN, AHB4_GPIOFEN, AHB4_GPIOGEN);
    ClockTree::EnableAHB4LP(AHB4_SRAM4LPEN | AHB4_BKPRAMEN | AHB4_CRCEN | AHB4_GPIOAEN | AHB4_GPIOBEN
                            | AHB4_GPIOCEN | AHB4_GPIODEN | AHB4_GPIOEEN | AHB4_GPIOFEN | AHB4_GPIOGEN);

    ClockTree::EnableAPB1L(APB1L_UART7EN | APB1L_UART5EN | APB1L_ATIM5EN);
    ClockTree::EnableAPB1LLP(APB1L_UART7EN | APB1L_UART5EN | APB1L_ATIM5EN);

    ClockTree::EnableAPB3(APB3_WWDG1EN);
    ClockTree::EnableAPB2LP(APB3_WWDG1EN);

    ClockTree::EnableAPB4(APB4_RTCAPBEN | APB4_SYSCFGEN);
    ClockTree::EnableAPB4LP(APB4_RTCAPBEN | APB4_SYSCFGEN);


    // Configure pins.  Don't do this before powering on GPIO.
    ConfigurePins();

    // Turn on card LED during init
    _card_led.Raise();

    // 8MHz HSI 
    // 200 MHz CPU
    // 100 MHz AHB, AXI
    //  20 Mhz APB1, 2, 3, 4

    static const ClockTree::Config clkconf = {
        .pll1_clk  = ClockTree::PllClkSource::HSI,
        .hsi48_ena = false,
        .lsi_ena   = true,
        .hse_freq  =  12000000,
        .cpu_freq  = 200000000,
        .ahb_freq  = 100000000,
        .apb1_freq = 20000000,
        .apb2_freq = 20000000,
        .apb3_freq = 20000000,
        .apb4_freq = 20000000,
        .rtc_clk   = ClockTRee::RtcClkSource::LSI,
        .rtcpre    = 1,         // RTC prescaler
    };

    const bool power_reset = ClockTree::CheckPowerLoss();
    ClockTree::Configure(clkconf);

    // 8/8 = 1MHz on MCO1
    ClockTree::EnableMCO(ClockTree::Mco1Output::HSI, ClockTree::McoPrescaler::DIV8);

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
	NVic::RouteIRQ(INTR_UART7, _usart3.Interrupt, IPL_UART, &_uart7);
	NVic::RouteIRQ(INTR_UART5, _usart5.Interrupt, IPL_UART, &_uart5);

    // XXX enable caching
    //Flash::EnableIDCaching();

    // Turn on proper assert handling
    _assert_stop = false;

    _dac.Enable();

	_uart7.InitAsync(115200, _uart7.StopBits::SB_1, APB1_FREQ);
	_uart7.SetInterrupts(true);

	_uart5.InitAsync(19200, _uart5.StopBits::SB_1, APB1_FREQ);
	_uart5.SetInterrupts(true);

    _clock.Start();

    // Stop timers while stopped in a breakpoint
    Debug::FreezeAPB1(Debug::APB1_TIM5_STOP);

    _uart7.Write("\r\nWelcome to Enetcore\r\n");
    _uart7.SyncDrain();

	// Enable global interrupts by restoring to a non-disabled state :)
    RestoreInterrupts(0);
    SetIPL(0);

    _uart7.EnableDmaTx(_dma1, DMA_STREAM_UART7_TX, DMA_PRIORITY_UART7);
    _uart5.EnableDmaTx(_dma1, DMA_STREAM_UART5_TX, DMA_PRIORITY_UART5);

    // Turn LED back off
    _card_led.Lower();

    assert(IntEnabled());

    // External SRAM
    Stm32Fmc::ConfigureSRAM(XSRAM_BANK, DATA_SETUP_CLK, DATA_HOLD_CLK, BUS_TURN_CLK);

    _malloc_region.SetReserve(64);

    if (power_reset) {
        DMSG("RTC power loss - initializing");
        Rtc::SetDateTime(Rtc::DateTime(2021, 10, 16, 
                                       20, 35, 00,
                                       Rtc::DayOfWeek::Saturday));
    }

    DMSG("RCC_CSR: 0x%x", _reset_reason);
    DMSG("CCLK: %d  HCLK: %d", CCLK, HCLK);
    DMSG("APB1CLK: %d  APB2CLK: %d", APB1_FREQ, APB2_FREQ);
    DMSG("APB3CLK: %d  APB4CLK: %d", APB3_FREQ, APB4_FREQ);

    console("\r\nSIM Rev 1 [%s:%s %s %s]",
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
