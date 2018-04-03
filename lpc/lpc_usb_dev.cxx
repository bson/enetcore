// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetcore.h"
#include "board.h"
#include "lpc_usb_dev.h"
#include "thread.h"

using namespace usb;

void LpcUsbDev::Init() {
    const uint32_t clkbits = BIT1 | BIT3 | BIT4;

    DMSG("USB: one-time init");

    ScopedNoInt G;

    PCONP |= PCUSB;

    _base[REG_CLKCTRL] = clkbits;

    // Use USB port 1
    _base[REG_PORTSEL] = 0;

    while ((_base[REG_CLKST] & clkbits) != clkbits)
        ;

    // Check that we have the right device
    if (AskSIE(SIECommand::TEST_REG, 2) != 0xa50f) {
        panic("Bad SIE TEST code");
    }

    TellSIE(SIECommand::SET_DEV_STAT, 0);

    memset(_ep, 0, sizeof _ep);

    _base[REG_DEVINTEN] = EP_FASTEN | EP_SLOWEN | EP_STATEN;
    _base[REG_DEVINTPRI] = 0;

#ifdef ENABLE_USB
    NVic::InstallIRQHandler(USB_IRQ, Usb::Interrupt, IPL_USB, &_usb);
    NVic::EnableIRQ(USB_IRQ);
#endif
}


void LpcUsbDev::SetClass(DeviceClass* devclass) {
    _class = devclass;
}


void LpcUsbDev::Setup() {

    DMSG("USB: initializing");

    IPL G(IPL_USB - 1);

    _connected = false;
    _suspended = false;
    _reset = false;
    _configured = 0;
    _wake = false;
    _max_ep = 0;

    _have_addr = false;
    _have_setup = false;

    for (uint n = 0; n < NUM_EP; ++n)
        delete _ep[n];

    memset(_ep, 0, sizeof _ep);

    DefineEP(EP0_OUT, 64, 64);
    DefineEP(EP0_IN, 64, 256);

    // Expect 8 bytes for SETUP
    ExpectRead(EP0_OUT, 8);

    // Clear any pending interrupts
    _base[REG_EPINTCLR]  = ~0;
    _base[REG_DEVINTCLR] = ~0;
    _base[REG_EPINTPRI]  = 0;

    // Disable interrupts on NAK, don't allow the clock to stop on suspend
    TellSIE(SIECommand::SET_MODE, 1);
    
    // Set address to 0
    TellSIE(SIECommand::SET_ADDR, 0x80);

    // Enable interrupts on device status change, slow, and fast EPs
    _base[REG_DEVINTEN] = BIT1 | BIT2 | BIT3;
}


void LpcUsbDev::DefineEP(uint phyep, uint maxpkt, uint bufsize) {
    assert(!_ep[phyep]);

    IPL G(IPL_USB - 1);

    _base[REG_REEP] |= BIT(phyep);
    _base[REG_EPIN] = phyep;
    _base[REG_MAXPSIZE] = maxpkt;

    while (!(_base[REG_DEVINTST] & DEVINTST_EP_RLZED))
        ;

    _base[REG_DEVINTCLR] = DEVINTST_EP_RLZED;

    // Enable slave mode
    _base[REG_EPINTEN] |= BIT(phyep);

    // Initialize EP data
    _ep[phyep] = new EP(bufsize);
    EP& ep = *_ep[phyep];

    ep.maxpkt = maxpkt;
    ep.buf.SetAutoResize(false);

    _max_ep = max<uint>(_max_ep, phyep);

    // Clear EP0 OUT buffer
    TellSIE(SIE_EP_CMD(SELECT_EP, 0));
    TellSIE(SIECommand::CLEAR_BUF);
}

    
void LpcUsbDev::Stall(uint phyep) {
    TellSIE(SIE_EP_CMD(SET_EP_STATUS, phyep), BIT0);
}


void LpcUsbDev::Unstall(uint phyep) {
    TellSIE(SIE_EP_CMD(SET_EP_STATUS, phyep), 0);
}


void LpcUsbDev::Write(uint phyep, const void* data, uint len, bool last) {
    assert(_ep[phyep]);

    IPL G(IPL_USB - 1);

    EP& ep = *_ep[phyep];

    ep.done = false;
    ep.zlp  = false;
    ep.buf.PushBack((const uint8_t*)data, len);

    FillINEP(phyep);
}


void LpcUsbDev::WriteDone(uint phyep, bool zlp) {
    assert(_ep[phyep]);

    IPL G(IPL_USB - 1);

    EP& ep = *_ep[phyep];

    ep.done = true;

    // Fill any trailing short packet
    FillINEP(phyep);

    // Flag to follow with ZLP if desired
    ep.zlp = zlp;

    // If nothing in transmit buffer and we want a ZLP, emit it now
    if (!ep.filled && ep.zlp) {
        EmitZLP(phyep);
        ep.zlp = false;
    }
}


void LpcUsbDev::WriteHold(uint phyep, bool state) {
    assert(_ep[phyep]);

    IPL G(IPL_USB - 1);

    _ep[phyep]->hold = state;
    if (!state)
        FillINEP(phyep);
}


void LpcUsbDev::EmitZLP(uint phyep) {
    assert(_ep[phyep]);

    IPL G(IPL_USB - 1);

    _base[REG_CTRL] = CTRL_WR_EN | CTRL_LOG_EP * (phyep/2);
    _base[REG_TXPLEN] = 0;
    TellSIE(SIE_EP_CMD(SELECT_EP, phyep));
    TellSIE(SIECommand::VALIDATE_BUF);

    EP& ep = *_ep[phyep];
    ep.filled = true;
    ep.zlp = false;
}
    
void LpcUsbDev::FillINEP(uint phyep) {
    assert(_ep[phyep]);
    
    EP& ep = *_ep[phyep];

    while (!ep.hold && !ep.filled && (ep.done || ep.buf.Size() >= ep.maxpkt)) {
        const uint count = min<uint>(ep.buf.Size(), ep.maxpkt);

        _base[REG_CTRL] = CTRL_WR_EN | CTRL_LOG_EP * (phyep/2);
        _base[REG_TXPLEN] = count;

        if (count) {
            const uint32_t *data = (const uint32_t*)&ep.buf.Front();
            uint n = count;
            while (n >= 4) {
                _base[REG_TXDATA] = *data++;
                n -= 4;
            }
            if (n) {
                uint32_t tmp = 0;
                uint bit = 0;
                uint pos = count - n;
                while (n--) {
                    tmp |= ep.buf[pos++] << bit;
                    bit += 8;
                }
                _base[REG_TXDATA] = tmp;
            }

            ep.buf.Erase(0, count);
        }

        // If it fit perfectly follow it with a ZLP
        if (count == ep.maxpkt && ep.buf.Empty())
            ep.zlp = true;

        TellSIE(SIE_EP_CMD(SELECT_EP, phyep));
        TellSIE(SIECommand::VALIDATE_BUF);

        ep.filled = true;
    }
}


void LpcUsbDev::DrainOUTEP(uint phyep) {
    assert(_ep[phyep]);
    
    EP& ep = *_ep[phyep];

    _base[REG_CTRL] = CTRL_RD_EN | CTRL_LOG_EP * (phyep/2);

    while (!(_base[REG_RXPLEN] & RXPLEN_PKT_RDY))
        ;

    uint len = _base[REG_RXPLEN] & 0x3f;

    while (len >= 4) {
        const uint32_t data = _base[REG_RXDATA];
        ep.buf.PushBack((const uint8_t*)&data, 4);
        len -= 4;
    }

    if (len) {
        const uint32_t data = _base[REG_RXDATA];
        ep.buf.PushBack((const uint8_t*)&data, len);
    }

    if (phyep == EP0_OUT) {
        const uint8_t po = AskSIE(SIECommand::CLEAR_BUF, 1);

        // On SETUP overwrite, discard
        if (po & 1) {
            //_console.WriteCStr("USB: SETUP PO\r\n");
            //_have_setup = false;
            //ep.buf.Flush();
            _base[REG_EPINTCLR] = EP0_OUT;
            return;
        }
    }

    ep.done = true;
}


void LpcUsbDev::ExpectRead(uint phyep, uint amount) {
    assert(_ep[phyep]);

    EP& ep = *_ep[phyep];

    ep.expect = amount;

    ep.done = false;
    ep.service = false;
}


void LpcUsbDev::GetDevDescReq() {

    const uint8_t n = _setup.index & 0xff;
    const uint8_t type = _setup.value >> 8;

    DMSG("USB: get dev desc %x,%x", n, type);

    ClearSetup();

    switch (type) {
    case TYPE_DEVICE:
        DMSG("USB: dev desc req len=%d", _setup.length);
        _ep[EP0_IN]->filled = false;
        _ep[EP0_IN]->zlp = false;
        _ep[EP0_IN]->buf.Flush();
        Write(EP0_IN, _descriptors->dev, min<uint>(_setup.length,
                                                   _descriptors->dev->length));
        WriteDone(EP0_IN);
        break;

    case TYPE_STRING: {
        DMSG("USB: dev: get str desc");

        if (n == 0) {
            static const StringDesc0 s0 = {
                sizeof(StringDesc0), TYPE_STRING, 0x409
            };
            Write(EP0_IN, &s0, sizeof s0);
            WriteDone(EP0_IN);
            break;
        }
        const uint8_t index = n - 1;
        if (index >= _descriptors->n_str) {
            StallControl();
            break;
        }

        const char* s = _descriptors->str[index];
        const uint8_t l = strlen(s);
        char h[2] = {uint8_t(l + 2), TYPE_STRING};
        Write(EP0_IN, h, 2);
        Write(EP0_IN, s, l);
        WriteDone(EP0_IN);
        break;
    }
    case TYPE_CONFIG: {
        DMSG("USB: dev: get config");

        WriteHold(EP0_IN, true);

        Write(EP0_IN, _descriptors->conf, _descriptors->conf->length);
        Write(EP0_IN, _descriptors->interf, _descriptors->interf->length);

        const uint8_t n = min<uint8_t>(_descriptors->n_ep, 15);
        Write(EP0_IN, _descriptors->ep, n * sizeof(EndpointDescriptor));

        // Fill in length
        Buffer& buf = GetBuffer(EP0_IN);

        assert_bounds(buf.Size() >= 4);
        (uint16_t&)buf[2] = buf.Size();

        const uint len = min<uint>(buf.Size(), _setup.length);
        buf.SetTail(len);
                  
        WriteHold(EP0_IN, false);
        WriteDone(EP0_IN);
        break;
    }
    case TYPE_INTERFACE: {
        DMSG("USB: dev: get intf desc");

        const uint len = _setup.length;
        Write(EP0_IN, _descriptors->interf, len);
        WriteDone(EP0_IN);
        break;
    }
    case TYPE_ENDPOINT: {
        DMSG("USB: dev: get ep desc");

        const uint len = _setup.length;
        Write(EP0_IN, _descriptors->ep + (n & 0xf) - 1, len);
        WriteDone(EP0_IN);
        break;
    }
    default:
        DMSG("USB: dev: get ??? desc");
        StallControl();
        break;
    }
}


void LpcUsbDev::ProcessDeviceSetup() {

    DMSG("USB: dev setup");

    ClearSetup();

    switch (_setup.request) {
    case REQ_GET_STATUS: {
        DMSG("USB: Get status");

        const uint16_t response = 1; // Self powered, no remote wake
        Write(EP0_IN, &response, 2);
        WriteDone(EP0_IN);
        break;
    }

    case REQ_SET_ADDRESS:
        // Address is set after ack and status
        DMSG("USB: addr %d", _setup.value & 0x7f);

        // ZLP ACK
        _ep[EP0_IN]->filled = false; // Nothing in buffer
        WriteDone(EP0_IN, true);

        TellSIE(SIECommand::SET_ADDR, 0x80 | (_setup.value & 0x7f));

        _have_addr = true;
        break;

    case REQ_GET_DESC: 
        GetDevDescReq();
        break;

    case REQ_GET_CONF: {
        DMSG("USB: device get_conf");

        Write(EP0_IN, &_configured, 1);
        WriteDone(EP0_IN);
        break;
    }
    case REQ_SET_CONF:
        DMSG("USB: device set_conf %d", _setup.value & 0xff);

        if ((_setup.value & 0xff) != 1) {
            StallControl();
            break;
        }

        _configured = 1;
        TellSIE(SIECommand::CONFIG_DEV, BIT0);

        _ep[EP0_IN]->filled = false; // Nothing in buffer
        WriteDone(EP0_IN, true);
        break;

    case REQ_CLEAR_FEATURE:
    case REQ_SET_FEATURE:
    case REQ_SET_DESC:
        DMSG("USB: device clear feature");
        StallControl();
        break;

    default:
        DMSG("USB: unknown request %d", _setup.request);
    }
}


void LpcUsbDev::ProcessInterfaceSetup() {

    DMSG("USB: intf setup");

    ClearSetup();

    switch (_setup.request) {
    case REQ_GET_STATUS: {
        const uint16_t response = 0;
        Write(EP0_IN, &response, 2);
        WriteDone(EP0_IN);
        break;
    }

    case REQ_GET_DESC:
    case REQ_GET_CONF:
    case REQ_SET_CONF:
    case REQ_SET_ADDRESS:
    case REQ_CLEAR_FEATURE:
    case REQ_SET_FEATURE:
    case REQ_SET_DESC:
    default:
        StallControl();
        break;
    }
}


void LpcUsbDev::ProcessEndpointSetup() {

    DMSG("USB: EP setup");

    ClearSetup();

    switch (_setup.request) {
    case REQ_GET_STATUS: {
        const uint16_t response = 0;
        Write(EP0_IN, &response, 2);
        WriteDone(EP0_IN);
        break;
    }

    case REQ_CLEAR_FEATURE:
    case REQ_SET_FEATURE: {
        if (_setup.value == 0x00) {
            int ep = _setup.index & 0x0f;
            if (ep != 0) {
                ep <<= 1;
                ep |= (_setup.index & 0x80) >> 7;

                if (_setup.request == REQ_CLEAR_FEATURE) {
                    Unstall(ep);
                } else {
                    Stall(ep);
                }
            }
        }
        break;
    }

    case REQ_GET_DESC:
    case REQ_GET_CONF:
    case REQ_SET_CONF:
    case REQ_SET_ADDRESS:
    case REQ_SET_DESC:
    default:
        StallControl();
        break;
    }
}


void LpcUsbDev::ClearSetup() {
    _have_setup = false;
    _ep[EP0_OUT]->buf.Flush();
    ExpectRead(EP0_OUT, 8);
}

bool LpcUsbDev::ProcessSetup() {
    //DMSG("USB SETUP: T=0x%x R=0x%x V=0x%x I=0x%x L=0x%x",
    // _setup.type, _setup.request, _setup.value, _setup.index, _setup.length);

    const uint8_t recipient = _setup.type & 31;
    const uint8_t type = (_setup.type >> 5) & 3;

    if (type == 1 || type == 2) {
        // Class/vendor based request... pass on to class
        return false;
    }

    if (type != 0) {
        DMSG("USB: bad type");
        StallControl();
        return true;
    }

    switch (recipient) {
    case REQ_DEVICE:
        ProcessDeviceSetup();
        break;
    case REQ_INTERFACE:
        ProcessInterfaceSetup();
        break;
    case REQ_ENDPOINT:
        ProcessEndpointSetup();
        break;
    default:
        DMSG("USB: bad recipient");
        StallControl();
        break;
    }

    return true;
}


void LpcUsbDev::ServiceEPs() {

    for (uint phyep = 0; phyep <= _max_ep; ++ phyep) {
        if (!_ep[phyep])
            continue;

        EP& ep = *_ep[phyep];

        const uint prev_ipl = SetIPL(IPL_USB - 1);

        if (ep.service) {
            ep.service = false;
                
            if (phyep == EP0_OUT) {
                // EP0 OUT
                if (_have_setup) {
                    // SETUP
                    DMSG("USB: got SETUP");

                    EP& ep0in = *_ep[EP0_IN];
                    ep0in.buf.Flush();
                    ep0in.filled = false;
                    ep0in.done = false;
                    ep0in.zlp = false;
                    ep0in.expect = 0;
                    ep0in.hold = false;

                    //_base[REG_EPINTCLR] = 1;

                    if (!ProcessSetup()) {
                        DMSG("USB: class setup");
                        if (_class)
                            _class->UsbDevProcessSetup(&_setup);
                    }

                    ClearSetup();

                    DMSG("USB: setup done");
                }
            } else {
                if (phyep & 1) {
                    SetIPL(prev_ipl);
                    _class->UsbDevProcessIN(phyep);
                    SetIPL(IPL_USB - 1);
                } else {
                    SetIPL(prev_ipl);
                    _class->UsbDevProcessOUT(phyep);
                    SetIPL(IPL_USB - 1);
                }
            }
        }
        SetIPL(prev_ipl);
    }
}


void LpcUsbDev::Service() {
    // Entered with interrupts disabled
    Thread::SetPriority(USB_THREAD_PRIORITY);

    RestoreInterrupts(0);

    DMSG("USB service thread started");

    Init();

    _class->UsbDevInit();

    for (;;) {

        bool prev_connected = false;
        bool prev_suspended = false;

        Setup();

        // Connect
        //TellSIE(SIECommand::SET_DEV_STAT, DEVSTAT_CON);

        while (/* !_reset */ true) {
            /// if (_reset)
            ///     break;

            // If see a transition from disconnected to connected
            // (cable), connect to host (enumeration).
            if ( _connected != prev_connected) {
                prev_connected = _connected;
                DMSG("USB: connect change: %s", _connected ? "up" : "down");
                _class->UsbDevConnect(_connected);
            }

            if (_suspended != prev_suspended) {
                prev_suspended = _suspended;
                DMSG("USB: %s", _suspended ? "suspended" : "resumed");

                _class->UsbDevSuspend(_suspended);
                continue;
            }

            ServiceEPs();

            {
                ScopedNoInt G;
                if (!_wake)
                    Thread::WaitFor(this, Time::Now() + Time::FromMsec(1000));

                _wake = false;
            }

            //DMSG("USB Wake");
        }

        prev_connected = _connected;
        prev_suspended = _suspended;

        DMSG("USB reset");
        _class->UsbDevReset();
    }
}

void* LpcUsbDev::Start(void* arg) {
    ((LpcUsbDev*)arg)->Service();
}


void LpcUsbDev::TellSIE(SIECommand cmd) {
    IPL G(IPL_USB - 1);

    _base[REG_DEVINTCLR] = CCEMPTY | CDFULL;
    _base[REG_CMDCODE]   = CMD_PHASE_CMD | (CMD_CODE_WDATA * (uint8_t)cmd);

    while (!(_base[REG_DEVINTST] & CCEMPTY))
        ;

    _base[REG_DEVINTCLR] = CCEMPTY;
}


void LpcUsbDev::TellSIE(SIECommand cmd, uint8_t arg) {
    IPL G(IPL_USB - 1);

    TellSIE(cmd);

    _base[REG_CMDCODE] = CMD_PHASE_WRITE | (CMD_CODE_WDATA * arg);
    
    while (!(_base[REG_DEVINTST] & CCEMPTY))
        ;

    _base[REG_DEVINTCLR] = CCEMPTY;
}


uint16_t LpcUsbDev::AskSIE(SIECommand cmd, uint n) {
    assert_bounds(n == 1 || n == 2);

    IPL G(IPL_USB - 1);

    TellSIE(cmd);

    _base[REG_CMDCODE]   = CMD_PHASE_READ | (CMD_CODE_WDATA * (uint8_t)cmd);

    while (!(_base[REG_DEVINTST] & CDFULL))
        ;

    _base[REG_DEVINTCLR] = CDFULL;
    uint32_t val = _base[REG_CMDDATA];

    if (n == 2) {
        _base[REG_CMDCODE]   = CMD_PHASE_READ | (CMD_CODE_WDATA * (uint8_t)cmd);
        while (!(_base[REG_DEVINTST] & CDFULL))
            ;

        _base[REG_DEVINTCLR] = CDFULL;
        val |= _base[REG_CMDDATA] << 8;
    }

    return val;
}


void LpcUsbDev::HandleInterrupt() {
#ifdef DEBUG
    ScopedNoInt G2;
#else
    IPL G(IPL_USB - 1);
#endif

    const uint32_t epint = _base[REG_EPINTST];

    uint32_t mask = 1;
    uint phyep = 0;

    while (epint >= mask) {
        if (!_ep[phyep]) {
            _base[REG_EPINTCLR] = mask;
            ++phyep;
            mask <<= 1;
            continue;
        }

        EP& ep = *_ep[phyep];

        if (epint & mask) {
            ep.buf.SetAutoCompact(false);
            if (phyep & 1) {
                ep.filled = false;
                FillINEP(phyep);

                if (!ep.filled && ep.done && ep.buf.Empty() && ep.zlp) {
                    // Wrote the last part, add a ZLP ACK if desired
                    EmitZLP(phyep);
                    ep.zlp = false;
                    ep.filled = true;
                }
            } else {
                // If on EP0_OUT we also record the SETUP state since
                // it's lost on the EPINTCLR write.
                if (phyep == EP0_OUT) {
                    //_console.WriteCStr("EP0 out drain\r\n");
                    ep.done = false;
                    DrainOUTEP(phyep);

                    const uint8_t epstat = AskSIE(SIE_EP_CMD(SELECT_EP, EP0_OUT), 1);
                    if (epstat & SIE_STP) {
                        if (ep.buf.Size() < 8) {
                            //_console.WriteCStr("USB Undersize setup\r\n");
                            StallControl();
                            ExpectRead(EP0_OUT, 8);
                        } else {
                            _have_setup = true;
                            memcpy(&_setup, &ep.buf.Front(), 8);
                            ep.buf.Flush();
                            ExpectRead(EP0_OUT, 8);
                        }
                    }
                } else {
                    DrainOUTEP(phyep);
                }
            }
            ep.buf.SetAutoCompact(true);

            ep.service = true;

            _base[REG_EPINTCLR] = mask;
        }

        ++phyep;
        mask <<= 1;

        _base[REG_DEVINTCLR] = EP_SLOW | EP_FAST | BIT0;
    }
        
    // Check for a status change
    if (_base[REG_DEVINTST] & EP_STAT) {
        _base[REG_DEVINTCLR] = EP_STAT;

        const uint8_t devstat = AskSIE(SIECommand::GET_DEV_STAT);

        if ((_reset = devstat & DEVSTAT_RST)) {
            //_suspended = false;
            //_connected = false;
            _have_setup = false;
            _have_addr = false;
        } else {
            if (devstat & DEVSTAT_SUSCH)
                _suspended = devstat & DEVSTAT_SUS;

            if (devstat & DEVSTAT_CONCH)
                _connected = devstat & DEVSTAT_CON;
        }
    }

    if (_base[REG_DEVINTST] & ERR_INT) {
        static uint8_t last_err = 0;

        last_err = AskSIE(SIECommand::GET_ERROR_STATUS);
        _base[REG_DEVINTCLR] = ERR_INT;
    }

    _wake = true;
    Thread::WakeSingle(this);
}


void LpcUsbDev::Interrupt(void* token) {
    LpcUsbDev *usb = (LpcUsbDev*)token;

    usb->HandleInterrupt();
}

