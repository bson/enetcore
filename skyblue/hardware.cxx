// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"
#include "thread.h"
#include "network.h"
#include "util.h"
#include "sdcard.h"
#include "fat.h"
#include "platform.h"
#include "arm.h"

extern const char _build_commit[];
extern const char _build_user[];
extern const char _build_date[];
extern const char _build_branch[];

#define BOARD_REV 3

// Define to use P1.25 as clock output
//#define CLKOUTPIN

#define MAYBE_STOP \
    while(!Thread::Bootstrapped()) \
        ;

Thread* _main_thread;
void* _main_thread_stack;
void* _intr_thread_stack;

NVic _nvic;

Pll _pll0(PLL0_BASE); // Main PLL
Pll _pll1(PLL1_BASE); // Alt PLL - used for USB

Gpio _gpio0(GPIO0_BASE);
Gpio _gpio1(GPIO1_BASE);
Gpio _gpio2(GPIO2_BASE);
Gpio _gpio4(GPIO4_BASE);

GpioIntr _gpio0_intr(GPIO0_INTR_BASE);
GpioIntr _gpio2_intr(GPIO2_INTR_BASE);

PinNegOutput<LpcGpio::Pin> _led5; // Green
PinNegOutput<LpcGpio::Pin> _led7; // Green
PinNegOutput<LpcGpio::Pin> _led6; // Red
PinNegOutput<LpcGpio::Pin> _led8; // Blue

PinNegOutput<LpcGpio::Pin> _sd_led;
PinNegOutput<LpcGpio::Pin> _sd_cs;

#if BOARD_REV == 1
PinNegOutput<LpcGpio::Pin> _ten; // CH341T transmit enable, active low
#endif

SpiBus _spi0(SSP0_BASE);
SpiDev _cardslot(_spi0);	// Card slot - on SPI bus 0
SDCard _sd(_cardslot); // Card block device - on card slot
Fat _fat(_sd);		  // Fat device - on SD block dev
I2cBus _i2c2(I2C2_BASE, I2C2_IRQ);

Eintr _eintr0(0);
SerialPort _uart3(UART3_BASE, UART3_IRQ);
Clock _clock;
SysTimer _systimer;
Eeprom _eeprom(EEPROM_BASE, EEPROM_IRQ);

#ifdef ENABLE_USB
Usb _usb(USB_BASE);
#endif

#ifdef ENABLE_PANEL
Panel _panel;
PinNegOutput<LpcGpio::Pin> _panel_reset; // Panel RESET#
PinNegOutput<LpcGpio::Pin> _t_cs; // Touch controller SPI CS#

EventObject _panel_tap(0, EventObject::MANUAL_RESET);
SpiDev _touch_dev(_spi0);
TouchController _touch(_touch_dev);
#endif

#ifdef ENABLE_ENET
// We have two source of interrupts for the LPC ethernet module:
// first, there is the internal controller.  Second, the PHY can
// request service using an external pin.  In this canse on EINT0.
// The bridge below makes the EINT0 pin result in an interrupt call
// to Ethernet::HandleMISRInterrupt().
Ethernet _eth0(ENET_BASE, ENET_IRQ);
Ethernet::InterruptBridge<LpcEintr> _misr_handler(_eth0, _eintr0);
#endif


// Flash the red LED a certain number of times to indicate a fault,
// followed by a pause, where the number of pulses indicares a cause.
void fault0(uint num) {
	for (uint n = num; n; --n) {
		_led6.Raise();
        asm volatile("1: subs %0,%0,#1; bne 1b" : : "r"(CCLK/8) : );
		_led6.Lower();

		if (num) {
            asm volatile("1: subs %0,%0,#1; bne 1b" : : "r"(CCLK/16) : );
        }
	}

    asm volatile("1: subs %0,%0,#1; bne 1b" : : "r"(CCLK/2) : );
}


// Sky Blue
//   LPC4078
//
// Allocation strategy:
//   Main thread stack: Internal RAM
//   Interrupt stack: Internal RAM
//   Thread stack: Internal RAM
//   Heap: Internal RAM
//   Thread context: Internal RAM
//   Network I/O buffers: heap
//   Network buffer data: sram0/sram1
//
// 

// Collect both adjacent peripheral RAMs into a single region
Platform::Region _pram_region(PRAM_REGION_SIZE, PRAM_REGION_START);

uint8_t* AllocThreadStack(uint size) {
    return (uint8_t*)malloc(Util::Align<uint>(size, 4));
}

Thread* AllocThreadContext() { 
    return new Thread();
}

// XXX encapsulate this
// RXDESC = (4+4) * 4 = 32 bytes
// TXDESC = (4+2) * 16 = 96
// Set aside 128 bytes for descriptors
uint NumNetworkBuffers(uint size) {
    return (PRAM_REGION_SIZE - 128) / Util::Align(size, 4U);
}

IOBuffer* AllocNetworkBuffer() { return new IOBuffer(); }
uint8_t* AllocNetworkData(uint size, uint alignment) {
    return (uint8_t*)_pram_region.GetAlignedMem(size, alignment);
}

enum { PIN_END = 0x1000 };

// Install a pin conf table
static void PinConf(volatile uint32_t* iocon, const uint32_t* table) {
    while (*table != PIN_END) {
        iocon[table[0]] = table[1];
        table += 2;
    }
}

void ConfigurePins() {
    // Default all GPIO ports to all input, masked
    _gpio0.MakeInputs(~0);
    _gpio1.MakeInputs(~0);
    _gpio2.MakeInputs(~0);
    _gpio4.MakeInputs(~0);

    _gpio0.Mask(~0);
    _gpio1.Mask(~0);
    _gpio2.Mask(~0);
    _gpio4.Mask(~0);

    //////// Pin plan ////////

    // P0.0  - U3_TXD
    // P0.1  - U3_RXD
    // P0.2  - GPIO (LED D5) Output
    // P0.3  - GPIO (LED D7) Output
    // P0.6  - I2S_RX_SDA
    // P0.7  - I2S_TX_SCK 
    // P0.8  - I2S_TX_WS
    // P0.9  - I2S_TX_SDA
    // P0.10 - I2C2_SDA
    // P0.11 - I2C2_SCL
    // P0.15 - GPIO Out J7.9
    // P0.16 - GPIO Out J7.10
    // P0.17 - GPIO Out J7.8
    // P0.18 - GPIO Out J7.7
    // P0.22 - GPIO In J7.6
    // P0.25 - ADC0_IN2 J8.1
    // P0.26 - DAC_OUT  J8.2
    // P0.29 - USB D+
    // P0.30 - USB D-
    static const uint32_t p0[] = {
        0, D_IOCON_FUNC(2),
        1, D_IOCON_FUNC(2),
        2, D_IOCON_GPIO(IOCON_MODE_NONE, 0, 0, 1, 0),
        3, D_IOCON_GPIO(IOCON_MODE_NONE, 0, 0, 1, 0),
        6, D_IOCON_FUNC(1),
        7, W_IOCON_FUNC(1),
        8, W_IOCON_FUNC(1),
        9, W_IOCON_FUNC(1),
        10, D_IOCON_FUNC(1),
        11, D_IOCON_FUNC(1),
        15, D_IOCON_GPIO(IOCON_MODE_NONE, 0, 0, 1, 0),
        16, D_IOCON_GPIO(IOCON_MODE_NONE, 0, 0, 1, 0),
        17, D_IOCON_GPIO(IOCON_MODE_NONE, 0, 0, 1, 0),
        18, D_IOCON_GPIO(IOCON_MODE_NONE, 0, 0, 1, 0),
        22, D_IOCON_GPIO(IOCON_MODE_NONE, 0, 0, 1, 0),
        25, A_IOCON_ADC(1),
        26, A_IOCON_DAC(2),
        29, U_IOCON_FUNC(1),
        30, U_IOCON_FUNC(1),
        PIN_END, PIN_END
    };

    PinConf(IOCON_P0, p0);

    _gpio0.MakeOutputs(BIT2 | BIT3 | BIT15 | BIT16 | BIT17 | BIT18);

    _led5 = _gpio0.GetPin(2);
    _led7 = _gpio0.GetPin(3);

    _led5.Raise();
    _led7.Raise();

    // P1.0  - ENET_TXD0
    // P1.1  - ENET_TXD1
    // P1.4  - ENET_TX_EN
    // P1.8  - ENET_CRS_DV
    // P1.9  - ENET_RXD0
    // P1.10 - ENET_RXD1
    // P1.14 - ENET_RX_ER
    // P1.15 - ENET_REF_CLK
    // P1.18 - USB_UP_LED1 (XXX this is only used in host mode?  If
    //         so, make it GPIO for LED)
    // P1.19 - GPIO (LED D6) Out
    // P1.20 - SSP0_SCK
    // P1.22 - GPIO (LED D8) Out
    // P1.23 - SSP0_MISO
    // P1.24 - SSP0_MOSI
#ifdef CLKOUTPIN
    // P1.25 - CLKOUT
#else
    // P1.25 - GPIO In (unused with CP2102N)
    // P1.25 - GPIO Out (/TEN with CH341T)
#endif
    // P1.26 - GPIO (LED D9 SD card activity) Out
    // P1.28 - SSP0_SSEL
    // P1.29 - GPIO Out  SD Card CS#
    // P1.30 - USB_VBUS
    // P1.31 - ADC0_IN5  J8.3

    static const uint32_t p1[] = {
        0, D_IOCON_FUNC(1),
        1, D_IOCON_FUNC(1),
        4, D_IOCON_FUNC(1),
        8, D_IOCON_FUNC(1),
        9, D_IOCON_FUNC(1),
        10, D_IOCON_FUNC(1),
        14, W_IOCON_FUNC(1),
        15, D_IOCON_FUNC(1),
        18, D_IOCON_FUNC(1),
        19, D_IOCON_GPIO(IOCON_MODE_NONE, 0, 0, 1, 0),
        20, D_IOCON_FUNC(5),
        22, D_IOCON_GPIO(IOCON_MODE_NONE, 0, 0, 1, 0),
        23, D_IOCON_FUNC(5),
        24, D_IOCON_FUNC(5),
#ifdef CLKOUTPIN
        25, D_IOCON_FUNC(5),    // CLKOUT
#else
        25, D_IOCON_GPIO(IOCON_MODE_NONE, 0, 0, 1, 0),
#endif
        26, D_IOCON_GPIO(IOCON_MODE_NONE, 0, 0, 1, 0),
        28, D_IOCON_FUNC(5),
        29, D_IOCON_GPIO(IOCON_MODE_PULLUP, 0, 0, 1, 0),
        30, A_IOCON_FUNC(2),
        31, A_IOCON_ADC(3),
        PIN_END, PIN_END
    };

    PinConf(IOCON_P1, p1);

#if BOARD_REV==1
    _gpio1.MakeOutputs(BIT19 | BIT22 | BIT26 | BIT29 | BIT25);
    _ten = _gpio1.GetPin(25);
    _ten.Raise();
#else
    _gpio1.MakeOutputs(BIT19 | BIT22 | BIT26 | BIT29);
#endif

    _led6   = _gpio1.GetPin(19);
    _led8   = _gpio1.GetPin(22);
    _sd_led = _gpio1.GetPin(26);
    _sd_cs  = _gpio1.GetPin(29);

    _led6.Raise();
    _led8.Raise();
    _sd_led.Raise();
    _sd_cs.Lower();

    // P2.0  - GPIO Out J7.18
    // P2.1  - GPIO Out J7.17
    // P2.2  - GPIO Out J7.16
    // P2.3  - GPIO Out J7.15
    // P2.4  - GPIO Out J7.14
    // P2.5  - GPIO Out J7.13
    // P2.6  - GPIO Out J7.12
    // P2.7  - GPIO Out J7.11
    // P2.8  - ENET_MDC
    // P2.9  - ENET_MDIO
    // P2.10 - EINT0#   Ethernet MISR Interrupt  J7.5

    static const uint32_t p2[] = {
        0, D_IOCON_GPIO(IOCON_MODE_NONE, 0, 0, 1, 0),
        1, D_IOCON_GPIO(IOCON_MODE_NONE, 0, 0, 1, 0),
        2, D_IOCON_GPIO(IOCON_MODE_NONE, 0, 0, 1, 0),
        3, D_IOCON_GPIO(IOCON_MODE_NONE, 0, 0, 1, 0),
        4, D_IOCON_GPIO(IOCON_MODE_NONE, 0, 0, 1, 0),
        5, D_IOCON_GPIO(IOCON_MODE_NONE, 0, 0, 1, 0),
        6, D_IOCON_GPIO(IOCON_MODE_NONE, 0, 0, 1, 0),
        7, D_IOCON_GPIO(IOCON_MODE_NONE, 0, 0, 1, 0),
        8, D_IOCON_FUNC(4),
        9, D_IOCON_FUNC(4),
        10, D_IOCON_FUNC(1),
        PIN_END, PIN_END
    };

    PinConf(IOCON_P2, p2);

    _gpio2.MakeOutputs(0xff);

    // P4.28 - GPIO Out J7.19
    // P4.29 - GPIO Out J7.20

    static const uint32_t p4[] = {
        28, D_IOCON_GPIO(IOCON_MODE_NONE, 0, 0, 1, 0),
        29, D_IOCON_GPIO(IOCON_MODE_NONE, 0, 0, 1, 0),
        PIN_END, PIN_END
    };

    PinConf(IOCON_P4, p4);
    
    _gpio4.MakeOutputs(BIT29);

#ifdef ENABLE_PANEL
    _panel_reset = _gpio4.GetPin(28);
    _t_cs        = _gpio1.GetPin(28);

    _gpio4.MakeOutputs(BIT28);
    _gpio1.MakeOutputs(BIT28);
    _gpio0.MakeInputs(BIT22);
#endif
}

// GPIO0 pin interrupt handler
static void gpio_intr(void*) {
#ifdef ENABLE_PANEL
    if (_gpio0_intr.PendingF(BIT22)) {
        _panel_tap.Set(1);      // Set to 1 to indicate push
    }
    if (_gpio0_intr.PendingR(BIT22)) {
        _panel_tap.Set(2);      // Set to 2 to indicate release
    }
#endif
    _gpio0_intr.Clear(~0UL);
    _gpio2_intr.Clear(~0UL);
}

static uint8_t _reset_reason;
static uint32_t _wwdt_mod;

// Hard system initialization.  Take control and prepare to run initializers.
void hwinit0() {
    // Disable MPU
    MPU_CTRL &= ~BIT0;

    _assert_stop = true;

    // 16 - Set ROM latency bit for operation > 60MHz
    // Prioitize D bus (prio 3) over I bus (prio 1) in AHB matrix
    // XXX set priority for SYS, ENET and USB?
    MATRIXARB = BIT16 | 0x31;

    // Set flash to 6 cycles
    FLASHCFG = 5 << 12;

    // The boot ROM seems to sometimes start up in weird states... so explicitly
    // reset here.
    CLKSRCSEL = 0;              // Use IRC OSC for pllclk and sysclk
    CCLKSEL   = 1;              // Use sysclk for CPU, divide by 1
    PCLKSEL   = 4;              // Divide sysclk for PCLK

    _reset_reason = RSID;
    RSID = 0x1f;                // Write 1's to clear... O.o

    // Clear all pending interrupts
    ICPR0 = ~0;
    ICPR1 = 0x1f;

    // Save watchdog state
    volatile uint32_t *wwdt = (volatile uint32_t*)WWDT_BASE;
    _wwdt_mod = wwdt[0];

    // Make sure no peripherals sit in reset
    RSTCON0 = 0;
    RSTCON1 = 0;

    // Zero out memory
    memset(MALLOC_REGION_START, 0, MALLOC_REGION_SIZE);
    memset(PRAM_REGION_START, 0, PRAM_REGION_SIZE);
    memset(IRAM_REGION_START, 0, IRAM_REGION_SIZE - MAIN_THREAD_STACK);
}

// "Soft" system initialization.  Initializers have been run now.
void hwinit() {
    // Enable main oscillator.  4- range select (1-20MHz), 5 - enable
    SCS = (SCS & ~BIT4) | BIT5;

    // Set power boost, if needed
    PBOOST = CCLK >= 100000000 ? 3 : 0;

    // Power on/off peripherals
    PCONP = CLOCK_PCON | PCUART3 | PCSSP0 | PCI2C2 | PCI2S | PCADC | PCGPIO | PCSSP0;
    PCONP1 = 0;

#ifdef ENABLE_ENET
    PCONP |= PCENET;
#endif
#ifdef ENABLE_USB
    PCONP |= PCUSB;
#endif

    // Configure pins.  Don't do this before powering on GPIO.
    ConfigurePins();

    // Set up PCLK divider before changing CCLK
    static_assert(CCLK/PCLK >= 1 && CCLK/PCLK <= 4);
    PCLKSEL = CCLK/PCLK;

    // Wait for main osc to stabilize
    while (!(SCS & BIT6))
        ;

    // Set flash cycle count (1 per 20MHz, starting at 0)
    FLASHCFG = (CCLK / 20000000) * BIT12;

    // Use main osc as clock source for main PLL0 and sysclk
    CLKSRCSEL = 1;

#ifdef ENABLE_PLL_CLK
	// Start main PLL
	_pll0.Init(CCLK);

    // CCLK sourced off main PLL, no divider
    CCLKSEL = BIT8 | 1;
#else
    // CCLK sourced off sysclk
    CCLKSEL = 1;
#endif

#ifdef ENABLE_USB
    // Start ALT PLL for USB CLK
#if 0
    _pll1.Init(USBCLK);
#else
    volatile uint32_t* usb = (volatile uint32_t*)PLL1_BASE;

    // M = 48/12 = 4

    // Fcco = pll_in_clk * M * 2 * P
    // M=4 P=2 => Fcco = 192MHz
    // pll_clk_out = Fcco/(2*P)
    // pll_clk_out = 192/(2*2) = 48MHz

    usb[LpcPll::REG_CFG] = (4-1) | (1 << 5);
    usb[LpcPll::REG_CON] = 1;   // Enable
    usb[LpcPll::REG_FEED] = 0xaa;
    usb[LpcPll::REG_FEED] = 0x55;

    USBCLKSEL = 0;

    // Wait to settle
    while (!(usb[LpcPll::REG_STAT] & BIT0))
        ;
#endif
    // USB directly from ALT PLL (_pll1)
    USBCLKSEL = (2 << 8) | 1;
#endif

#ifdef CLKOUTPIN
    // 0x3 - usbclk, 8 - enable, div by four
    CLKOUTCFG = 3 | BIT8 | (3 << 4);
#endif

#ifdef ENABLE_PANEL
    // P4.28 = RESET#
    // P0.15-18 CS#/WR#/RD#/RS
    // P0.22 = T_IRQ#
    // P1.28 = T_CS#
    _gpio0.MakeOutputs(BIT15 | BIT16 | BIT17 | BIT18);
    _gpio0.Set(BIT15 | BIT16 | BIT17 | BIT18);

    _gpio1.MakeOutputs(BIT28);
    _gpio0.MakeInputs(BIT22);

    _panel_reset.Raise();
    _t_cs.Lower();
#endif

    // Initialize peripheral memory region
    _pram_region.Init(PRAM_REGION_SIZE, PRAM_REGION_START);

	// Initialize main thread and set up stacks
	_main_thread = &Thread::Bootstrap();

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

	// Grab 32 bytes from AIN2
    // XXX encapsulate the ADC!
    volatile uint32_t& ADCR = ((volatile uint32_t*)ADC_BASE)[0];
    volatile uint32_t& GDR = ((volatile uint32_t*)ADC_BASE)[1];
    volatile uint32_t& INTEN = ((volatile uint32_t*)ADC_BASE)[3];

    INTEN = 0;

    // 2 - AIN2;  PCLK/12400000-1 divider; 21 - enable; 
    enum { ADC_DIV = (PCLK / 12400000) - 1};
    ADCR = BIT2 | ((ADC_DIV >= 0 ? ADC_DIV : 0) << 8) | BIT21;
    
	uint8_t buf[32];
	for (uint8_t* p = (uint8_t*)buf; p < (uint8_t*)buf + sizeof buf; ) {
		ADCR |= BIT24;		// Go
		uint32_t ad;
		do {
			ad = GDR;
		} while (!(ad & BIT31)); // 31 - done
		*p++ = ad & 0xffff;
	}
	ADCR = 0;					// Power down ADC

	Util::RandomSeed(buf, sizeof buf);

	// Install IRQ handlers
	NVic::InstallIRQHandler(CLOCK_IRQ, Clock::Interrupt, IPL_CLOCK, &_clock);
	NVic::EnableIRQ(CLOCK_IRQ);

	NVic::InstallIRQHandler(UART3_IRQ, SerialPort::Interrupt, IPL_UART, &_uart3);
	NVic::EnableIRQ(UART3_IRQ);
    
    NVic::InstallIRQHandler(EEPROM_IRQ, Eeprom::Interrupt, IPL_EEPROM, &_eeprom);
    NVic::EnableIRQ(EEPROM_IRQ);

    // Turn on proper assert handling
    _assert_stop = false;

	NVic::InstallIRQHandler(I2C2_IRQ, I2cBus::Interrupt, IPL_I2C, &_i2c2);
	NVic::EnableIRQ(I2C2_IRQ);

#ifdef ENABLE_ENET
    _eintr0.Init();
    NVic::InstallIRQHandler(EINT0_IRQ, LpcEintr::Interrupt, IPL_MISR, &_eintr0);
    NVic::InstallIRQHandler(ENET_IRQ, Ethernet::Interrupt, IPL_ENET, &_eth0);
    NVic::EnableIRQ(ENET_IRQ);
    NVic::EnableIRQ(EINT0_IRQ);
#endif
    NVic::InstallIRQHandler(GPIO_IRQ, gpio_intr, IPL_GPIO, NULL);

#ifdef ENABLE_PANEL
    _gpio0_intr.EnableR(BIT22);      // Enable P0.22 Rising edge interrupts
    _gpio0_intr.EnableF(BIT22);      // Enable P0.22 Falling edge interrupts
    _gpio0_intr.Clear(BIT22);
#endif

	_uart3.Init(19200, SerialPort::FRAMING_8N1);
	_uart3.SetInterrupts(true);

	// Enable global interrupts by restoring to a non-disabled state :)
    RestoreInterrupts(0);

    // Disable IPLs
    SetIPL(0);

	// Start clock
	_clock.SetResolution(TIME_RESOLUTION);
	_clock.RunTimerFreq(HZ);

    // First line of text
    _uart3.Write("\r\nEnetcore booting up...\r\n");

    // Initialize EEPROM early so we can pull config from it
    _eeprom.Init();

    assert(IntEnabled());

    // Turn off LEDs
    _led5.Lower();
    _led6.Lower();
    _led7.Lower();
    _led8.Lower();
    _sd_led.Lower();

    assert(IntEnabled());

	_spi0.Init();
	_cardslot.SetSSEL(&_sd_cs);

	_i2c2.Init();
	_i2c2.SetSpeed(I2C_BUS_SPEED);
	_sd.SetLock(&_sd_led);

    assert(IntEnabled());

#ifdef ENABLE_PANEL
    // Release from reset
    _panel_reset.Lower();
#endif

    DMSG("RSID: 0x%x  WWDT_MOD: 0x%x", _reset_reason, _wwdt_mod);
    DMSG("CCLK: %d  PCLK: %d", CCLK, PCLK);

    _malloc_region.SetReserve(64);

    console("\r\nSky Blue Rev 3 [%s:%s %s %s]",
            _build_branch, _build_commit, _build_user, _build_date);

    console("Copyright (c) 2018 Jan Brittenson");
    console("All Rights Reserved\r\n");

    DMSG("Random uint: 0x%x", Util::Random<uint>());
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
#include "ssd1963.cxx"
#endif // ENABLE_PANEL
