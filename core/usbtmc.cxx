// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifdef ENABLE_USBTMC

#include "enetcore.h"
#include "usb.h"
#include "usbtmc.h"

using namespace usb;


// Physical endpoints
enum {
    EP_BULK_IN = 5,             // Logical 2
    EP_BULK_OUT = 4,            // Logical 2
    EP_INTR_IN = 3              // Logical 1
};


static USBTMC_Capabilities cap = {
  0, 0,        // Status, reserved
  0x0100,      // Version, 1.0.0
  0b00000100,  // Interface cap: D2=USB488.2, no REN_CONTROL/LOCKOUT, no TRIGGER
  0b00001000,  // Device cap: D3=SCPI, D2=SR1 capable (intr-IN EP), RL0, DL0
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0  // Reserved
};


UsbTmc::UsbTmc(Usb& usb)
    : _usb(usb) {
}

      
void UsbTmc::Init() {
    _usb.SetClass(this);
}


void UsbTmc::UsbDevInit() {
    DMSG("USBTMC: init, defining endpoints");

    // XXX MAP EP
    _usb.DefineEP(EP_INTR_IN, 64, 64);
    _usb.DefineEP(EP_BULK_IN, 64, 256);
    _usb.DefineEP(EP_BULK_OUT, 64, 128);
}

void UsbTmc::UsbDevReset() {
    DMSG("USBTMC: reset");
}


void UsbTmc::UsbDevConnect(bool state) {
    DMSG("USBTMC: connect");
}


void UsbTmc::UsbDevSuspend(bool state) {
    DMSG("USBTMC: suspend");
}


bool UsbTmc::UsbDevProcessSetup(const SetupRequest* setup) {
    DMSG("USBTMC: processing SETUP");
    
    switch (setup->request) {
    case GET_CAPABILITIES:
        cap.status = setup->value != 0 ? STATUS_FAILED : STATUS_SUCCESS;

        _usb.Write(Usb::EP0_IN, &cap, sizeof cap);
        _usb.WriteDone(Usb::EP0_IN);
        break;

    case INITIATE_ABORT_BULK_IN:
    case INITIATE_ABORT_BULK_OUT: {
        // We don't current have a mechanism for long bulk ops, so just fail them
        const uint8_t tag = setup->value;
        const uint16_t response = (tag << 8) | STATUS_FAILED;

        _usb.Write(Usb::EP0_IN, &response, 2);
        _usb.WriteDone(Usb::EP0_IN);

        break;
    }
    case INITIATE_CLEAR: {

        const uint8_t response = STATUS_FAILED;

        _usb.Write(Usb::EP0_IN, &response, 1);
        _usb.WriteDone(Usb::EP0_IN);

        break;
    }
    case READ_STATUS_BYTE: {
        const uint16_t tag = setup->value & 0x7f;
        const uint32_t response = (tag << 8) | STATUS_SUCCESS;

        _usb.Write(EP_BULK_IN, &response, 3); // XXX MAP EP
        _usb.WriteDone(EP_BULK_IN);
        break;
    }
    case CHECK_ABORT_BULK_OUT_STATUS:
    case CHECK_ABORT_BULK_IN_STATUS:
    case CHECK_CLEAR_STATUS:
        DMSG("Unimplemented USBTMC class control request %x\n", setup->request);
        _usb.StallControl();
        break;

    case INDICATOR_PULSE:
        Pulse();
        break;

    default:
        _usb.StallControl();
        break;
    }

    return true;
}


void UsbTmc::Reply(String& s) {
    DevDepBulk r;
    
    memset(&r.size, 0, sizeof r - sizeof r.base);
    memcpy(&r.base, &_bulk_out_req, sizeof r.base);

    const uint len = s.Size();

    r.size = len + 1;
    r.attrs = ATTR_EOM;

    memcpy(r.data, s.CStr(), len);
    r.data[len] = '\n';   // USB488

    _usb.Write(EP_BULK_IN, &r, sizeof r + len);
    _usb.WriteDone(EP_BULK_IN);
}


void UsbTmc::UsbDevProcessIN(uint phyep) {
}


void UsbTmc::UsbDevProcessOUT(uint phyep) {
    if (phyep != EP_BULK_OUT)
        return;

    // Must receive full base header in first packet or screw this
    if (_usb.GetBuffer(EP_BULK_OUT).Size() < sizeof(BulkBase)) {
        _usb.Stall(EP_BULK_OUT);
        return;
    }

    Usb::Buffer& buf = _usb.GetBuffer(EP_BULK_OUT);
    const BulkBase* base = (const BulkBase*)&buf.Front();

    if (base->tag != ~base->taginv & 0xff) {
        DMSG("USBTMC Tag mismatch: %x,%x\n", base->tag, base->taginv);
        _usb.Stall(EP_BULK_OUT);
        return;
    }

    const DevDepBulk* msg = (const DevDepBulk*)&buf.Front();

    if (base->msgid == DEV_DEP_MSG_OUT || base->msgid == DEV_DEP_MSG_IN) {
        // XXX Multi-transfer message are NYI
        if (!(msg->attrs & ATTR_EOM) || msg->size == 0) {
            DMSG("USBTMC: refusing MSG without OOM");
            _usb.Stall(EP_BULK_OUT);
            return;
        }

        // If we don't have it all in the buffer, maybe next time
        if (buf.Size() < msg->size) {
            _usb.ExpectRead(EP_BULK_OUT, msg->size);
            return;
        }
    }

    // Save base request for the response
    memcpy(&_bulk_out_req, base, sizeof *base);

    // Got a complete message, process it
    String s(msg->data, msg->size);

    buf.Flush();

    switch (base->msgid) {
    case DEV_DEP_MSG_OUT: {
        Query(s);
        break;
    }

    case REQUEST_DEV_DEP_MSG_IN:{
        Ask(s);
        break;
    }

    default:
        _usb.Stall(EP_BULK_OUT);
        break;
    }
}


void UsbTmc::Ask(String& s) {
    extern char *idn;

    if (s.Equals(STR("*IDN?\n"))) {
        String r((uchar*)idn, strlen(idn));
        Reply(r);
    }
}

#endif // ENABLE_USBTMC
