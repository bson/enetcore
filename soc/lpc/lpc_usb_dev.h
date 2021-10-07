// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef _USB_DEV_H_
#define _USB_DEV_H_

#include <stdint.h>
#include "usb.h"
#include "bits.h"
#include "deque.h"


class LpcUsbDev {
    volatile uint32_t* _base;
public:
    typedef Deque<uint8_t> Buffer;

    // Physical control endpoints
    enum {
        EP0_OUT = 0,
        EP0_IN  = 1
    };

private:
    // Device registers
    enum {
        // (RW) - USB Port Select. This register is also used for OTG
        // configuration. In device-only operations only bits 0 and 1
        // of this register are used to control the routing of USB
        // pins to Port 1 or Port 2.
        REG_PORTSEL =  0x110/4,

        //////// Device interrupt registers

        REG_DEVINTST  = 0x200/4,      // (RO) - USB Device Interrupt Status
        REG_DEVINTEN  = 0x204/4,      // (RW) - USB Device Interrupt Enable
        REG_DEVINTCLR = 0x208/4,      // (WO) - USB Device Interrupt Clear
        REG_DEVINTSET = 0x20c/4,      // (WO) - USB Device Interrupt Set
        REG_DEVINTPRI = 0x22c/4,      // (WO) - USB Device Interrupt Priority

        //////// Endpoint interrupt registers

        REG_EPINTST  = 0x230/4,      // (RO) - USB Endpoint Interrupt Status
        REG_EPINTEN  = 0x234/4,      // (RW) - USB Endpoint Interrupt Enable
        REG_EPINTCLR = 0x238/4,      // (WO) - USB Endpoint Interrupt Clear
        REG_EPINTSET = 0x23c/4,      // (WO) - USB Endpoint Interrupt Set
        REG_EPINTPRI = 0x240/4,      // (WO) - USB Endpoint Priority

        //////// Endpoint realization registers

        REG_REEP     = 0x244/4,       // (RW) - USB Realize Endpoint
        REG_EPIN     = 0x248/4,       // (WO) - USB Endpoint Index
        REG_MAXPSIZE = 0x24c/4,       // (RW) - USB Max Packet Size

        //////// Transfer registers
        REG_RXDATA = 0x218/4,         // (RO) - USB Receive Data
        REG_RXPLEN = 0x220/4,         // (RO) - USB Receive Data Length
        REG_TXDATA = 0x21c/4,         // (WO) - USB Transmit Data
        REG_TXPLEN = 0x224/4,         // (WO) - USB Transmit Data Length
        REG_CTRL   = 0x228/4,         // (RW) - USB Control

        //////// SIE Command Registers

        REG_CMDCODE   = 0x210/4,      // (WO) - USB Command Code
        REG_CMDDATA   = 0x214/4,      // (RO) - USB Command Data

        //////// DMA Registers

        REG_DMARST       = 0x250/4,   // (RO) - USB DMA Request Status
        REG_DMARCLR      = 0x254/4,   // (WO) - USB DMA Request Clear
        REG_DMARSET      = 0x258/4,   // (WO) - USB DMA Request Set
        REG_UDCAH        = 0x280/4,   // (RW) - USB UDCA Head
        REG_EPDMAST      = 0x284/4,   // (RO) - USB Endpoint DMA Status
        REG_EPDMAEN      = 0x288/4,   // (WO) - USB Endpoint DMA Enable
        REG_EPDMADIS     = 0x28c/4,   // (WO) - USB Endpoint DMA Disable
        REG_DMAINTST     = 0x290/4,   // (RO) - USB DMA Interrupt Status
        REG_DMAINTEN     = 0x294/4,   // (RW) - USB DMA Interrupt Enable
        REG_EOTINTST     = 0x2a0/4,   // (RO) - USB End of Transfer Interrupt Status
        REG_EOTINTCLR    = 0x2b0/4,   // (WO) - USB End of Transfer Interrupt Clear 
        REG_EOTINTSET    = 0x2a8/4,   // (WO) - USB End of Transfer Interrupt Set
        REG_NDDRINTST    = 0x2ac/4,   // (WO) - USB New DD Request Interrupt Status 
        REG_NDDRINTCLR   = 0x2b0/4,   // (WO) - USB New DD Request Interrupt Clear
        REG_NDDRINTSET   = 0x2b4/4,   // (WO) - USB New DD Request Interrupt Set
        REG_SYSERRINTST  = 0x2b8/4,   // (RO) - USB System Error Interrupt Status 
        REG_SYSERRINTCLR = 0x2bc/4,   // (WO) - USB System Error Interrupt Clear
        REG_SYSERRINTSET = 0x2c0/4,   // (WO) - USB System Error Interrupt Set

        //////// Clock control registers

        REG_CLKCTRL = 0xff4/4,        // (RW) - USB Clock Control
        REG_CLKST   = 0xff8/4,        // (RO) - USB Clock Status
    };

    // SIE Command Codes
    enum class SIECommand: uint8_t {
        // Device commands
        SET_ADDR = 0xd0,         // Set address (W1)
        CONFIG_DEV = 0xd8,       // Configure device (W1)
        SET_MODE = 0xf3,         // Set mode (W1)
        FRAME_NUM = 0xf5,        // Read current frame number (R1 or R2)
        TEST_REG = 0xfd,         // Read test register (R2)
        SET_DEV_STAT = 0xfe,     // Set device status (W1)
        GET_DEV_STAT = 0xfe,     // Get device status (R1)
        GET_ERROR_CODE = 0xff,   // Get error code (R1)
        GET_ERROR_STATUS = 0xfb,   // Get error status (R1)

        // Endpoint commands
        SELECT_EP = 0x00,        // Select EP (add # to code) (optional R1 for status)
        SELECT_EP_CLRINT = 0x40, // Select EP/Clear interrupt (add # to code) (R1)
        SET_EP_STATUS = 0x40,    // Select EP/Set status (W1)
        CLEAR_BUF = 0xf2,        // Clear buffer on selected EP (optional R1)
        VALIDATE_BUF = 0xfa      // Validate buffer on selected EP
    };

#define SIE_EP_CMD(CMD, N) (SIECommand)((uint8_t)(SIECommand::CMD) + (N))


    // Various command bits (this is an incomplete set)
    enum {
        CCEMPTY = BIT4,
        CDFULL  = BIT5,
        CMD_PHASE_READ = BIT8 * 0x02,
        CMD_PHASE_WRITE = BIT8 * 0x01,
        CMD_PHASE_CMD = BIT8 * 0x05,
        CMD_CODE_WDATA = BIT16,
        SIE_STP = BIT2,
        SIE_PO  = BIT3,
        DEVINTST_EP_RLZED = BIT8,
        DEVSTAT_CON = BIT0,
        DEVSTAT_CONCH = BIT1,
        DEVSTAT_SUS = BIT2,
        DEVSTAT_SUSCH = BIT3,
        DEVSTAT_RST = BIT4,
        CTRL_RD_EN = BIT0,
        CTRL_WR_EN = BIT1,
        CTRL_LOG_EP = BIT2,
        RXPLEN_PKT_RDY = BIT11,
        RXPLEN_DV = BIT10,
        EP_FASTEN = BIT1,
        EP_SLOWEN = BIT2,
        EP_STATEN = BIT3,
        EP_FAST = BIT1,
        EP_SLOW = BIT2,
        EP_STAT = BIT3,
        ERR_INT = BIT9,
    };
    
    // For LPC407x/8x EPs, see UN10562 table 252, "Fixed endpoint configuration"
    enum {
        NUM_EP = 32             // # of physical endpoints
    };

    // Per-EP data
    struct EP {
        Buffer   buf;           // RX/TX buf
        uint16_t expect;        // RX transfer size to expect
        uint8_t  maxpkt;        // Max packet size
        uint8_t  owner:3;       // Index of logical device XXX NYI
        bool     done:1;        // Flagged as done
        bool     filled:1;      // EP has been filled from buffer
        bool     service:1;     // EP needs service
        bool     zlp:1;         // Follow up last write with ZLP
        bool     hold:1;        // Hold writes until unheld

        EP(uint bufsize)
            : buf(bufsize),
              done(false),
              filled(false),
              service(false),
              zlp(false),
              hold(false)
        { }
    };

    // Endpoint table.  Non-NULL means an entry is realized.
    EP *_ep[NUM_EP];            // EP state

public:
    // Descriptors
    struct Descriptors {
        const usb::DeviceDescriptor *dev;
        const usb::ConfigDescriptor *conf;
        const usb::InterfaceDescriptor *interf;
        uint n_interf;
        const usb::EndpointDescriptor *ep;
        uint n_ep;
        const char **str;
        uint n_str;
    };
private:

    const Descriptors* _descriptors;

    usb::SetupRequest _setup;   // SETUP packet if _have_setup is true

    // XXX we should have a list of logical devices, each with a class...
    usb::DeviceClass* _class;  // Device class

    uint8_t _max_ep;
    bool _have_setup;           // EP0_OUT received a SETUP
    bool _have_addr;            // We have an address
    bool _wake;                 // A wake event occurred
    uint8_t _configured;
    bool _suspended;            // Device is suspended
    bool _connected;            // Device is connected
    bool _reset;                // Device has been reset

public:
    LpcUsbDev(uintptr_t base)
        : _base((volatile uint32_t*)base)
    { }

    // One-time initialization
    void Init();

    // Set class - one time, kept across device resets
    // XXX make this a variadic SetClasses?  We should have a list of logical devices.
    void SetClass(usb::DeviceClass* devclass);

    // Set descriptors - one time, kept across device resets
    void SetDescriptors(const Descriptors* descriptors) {
        _descriptors = descriptors;
    }

    // Add a physical endpoint and realize it.  Bufsize is how much
    // buffer to allocate to the EP.
    // XXX should be tied to a logical device
    void DefineEP(uint phyep, uint maxpkt, uint bufsize);

    // Get control OUT EP0 status - this includes the STP bit to
    // identify SETUP, PO (for SETUP overwrite), etc.
    uint8_t ControlStatus();
    
    // STALL an endpoint
    void Stall(uint phyep);

    // Stall logical EP0 IN and OUT
    void StallControl() { Stall(0); Stall(1); }

    // Unstall an endpoint
    void Unstall(uint phyep);

    // Write data to an IN EP.  If 'last' it's the end of the transfer.
    void Write(uint phyep, const void* data, uint len, bool last = false);

    // Hold writes until unheld.  This permits composing variant
    // responses directly in the output buffer.
    void WriteHold(uint phyep, bool state);

    // Declare we're done, ending the transfer when the buffer is empty.
    // Variable-length transfers are terminated with a zero length packet,
    // which will be output if ZLP is set.
    void WriteDone(uint phyep, bool zlp = false);

    // Declare how much to expect in an OUT EP.  When a packet is
    // received that would exceed this it's NAKed.  The amount can
    // be used either to loop through a transfer stream that is ended
    // with a ZLP+NAK, or to receive a fixed amount.
    void ExpectRead(uint phyep, uint amount);

    // Check if read transfer is done
    bool ReadDone(uint phyep) const {
        assert(_ep[phyep]);

        return _ep[phyep]->done;
    }

    // Returns endpoint buffer
    Buffer& GetBuffer(uint phyep) {
        assert(_ep[phyep]);

        return _ep[phyep]->buf;
    }

    // This is the entry point for a service thread that should be
    // started.  it will sit indefinitely in:
    //
    //   repeat
    //     initialize
    //     repeat
    //       wait
    //       check for reset      -> notify class, exit to outer loop
    //       check for disconnect -> notify class
    //       check for suspend    -> notify class
    //       call usb service()   -> notify class
    //       -> class callbacks for SETUP, EP IN, EP OUT
    //     end
    //   end
    static void* Start(void* arg);

private:
    void Service();

    // Setup device out of reset state.
    void Setup();

    // Handle SETUP packet(s)
    bool ProcessSetup();
    void ProcessDeviceSetup();
    void ProcessInterfaceSetup();
    void ProcessEndpointSetup();
    
    void GetDevDescReq();

    static void Interrupt(void*);

    void ServiceEPs();

    // Done processing SETUP.. prepare for the next one - for a
    // successful request this should be done before the response is
    // sent since otherwise we race with the host to send the next
    // setup.
    void ClearSetup();

    // Write command to SIE
    void TellSIE(SIECommand cmd);

    // Write command to SIE with 1 byte
    void TellSIE(SIECommand cmd, uint8_t arg);

    // Write command to SIE, read 1 (default) or 2 bytes
    uint16_t AskSIE(SIECommand cmd, uint n = 1);

    // Emit a ZLP
    void EmitZLP(uint phyep);

    // Fill IN EP from buffer
    void FillINEP(uint phyep);

    // Drain OUT EP to buffer
    void DrainOUTEP(uint phyep);

    // Instance interrupt handler
    inline void HandleInterrupt();
};

#endif // _USB_DEV_H_
