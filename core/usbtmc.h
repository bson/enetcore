// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef  _USBTMC_H_
#define _USBTMC_H_

#include "enetcore.h"
#include "board.h"
#include "usb.h"

// Simple USBTMC implementation, sitting on top of the simple USB device.
// Implements the usb488 subclass

// Capabilities response
struct [[gnu::packed]] USBTMC_Capabilities {
    uint8_t  status;    // USBTMC status
    uint8_t  res;       // 0
    uint16_t bcdusbtmc; // 0x0100 = USBTMC version 1.0.0
    uint8_t  cap_if;    // Interface capabilities
    uint8_t  cap_dev;   // Device capabilities
    uint8_t  res1[6+12]; // 0
};


class UsbTmc: public usb::DeviceClass {
    Usb& _usb;

private:
    // Class control requests
    enum {
        INITIATE_ABORT_BULK_OUT = 1,
        CHECK_ABORT_BULK_OUT_STATUS = 2,
        INITIATE_ABORT_BULK_IN = 3,
        CHECK_ABORT_BULK_IN_STATUS = 4,
        INITIATE_CLEAR = 5,
        CHECK_CLEAR_STATUS = 6,
        GET_CAPABILITIES = 7,
        INDICATOR_PULSE = 64,
        READ_STATUS_BYTE = 128, // USB488
        REN_CONTROL = 160,      // USB488
        GO_TO_LOCAL = 161,      // USB488
        LOCAL_LOCKOUT = 162     // USB488
    };

    // USBTMC status
    enum {
        STATUS_SUCCESS = 1,
        STATUS_PENDING = 2,
        STATUS_FAILED = 0x80,
        STATUS_TRANSFER_NOT_IN_PROGRESS = 0x81,
        STATUS_SPLIT_NOT_IN_PROGRESS = 0x82,
        STATUS_SPLIT_IN_PROGRESS = 0x83
    };

    // Base Bulk-OUT and Bulk-IN header (they're the same)
    struct [[gnu::packed]] BulkBase {
        uint8_t  msgid;     // Message
        uint8_t  tag;       // Request tag
        uint8_t  taginv;    // Inverse tag
        uint8_t  reserved;  // Reserved
    };

    // Bulk-OUT request and also Bulk-IN response
    struct [[gnu::packed]] DevDepBulk {
        BulkBase base;
        uint32_t size;      // Transfer size
        uint8_t  attrs;     // Attributes
        uint8_t  reserved[3];
        uint8_t  data[1];
    };

    struct [[gnu::packed]] ReqDevDepBulkIn {
        BulkBase base;
        uint32_t size;      // Transfer size
        uint8_t  attrs;     // Attributes
        uint8_t  term;
        uint8_t  reserved[2];
        uint8_t  data[1];
    };


    enum {
        ATTR_EOM = 1
    };

    // Messages
    enum {
        // OUT
        DEV_DEP_MSG_OUT = 1,        // Device-dependent OUT with no response
        REQUEST_DEV_DEP_MSG_IN = 2, // Dev dep OUT with DEV_DEP_MSG_IN in return
        VENDOR_SPECIFIC_OUT = 126,  // Vendor specific message OUT with no response
        REQUEST_VENDOR_SPECIFIC_IN = 127, // Vend spec message OUT with VENDOR_SPECIFIC_IN back
        TRIGGER = 128,              // USB488 message (optional, not currently supported)

        // IN
        DEV_DEP_MSG_IN = 2,        // Response to DEV_EP_MSG_OUT
        VENDOR_SPECIFIC_IN = 127   // Response to REQUEST_VENDOR_SPECIFIC_IN
    };

    // For a request this contains the message, which we use to
    // initialize the response
    DevDepBulk _bulk_out_req;

public:
    UsbTmc(Usb& usb);

    void Init();

    // Callback during configuration - class should define its EPs
    void UsbDevInit();

    // Process SETUP.  Called at USB_IPL. Returns true if setup was
    // processed
    bool UsbDevProcessSetup(const usb::SetupRequest* setup);

    // Process IN to host
    void UsbDevProcessIN(uint phyep);

    // Process OUT from host
    void UsbDevProcessOUT(uint phyep);

    // Callback for reset
    void UsbDevReset();

    // Callback for connect/disconnect
    void UsbDevConnect(bool state);

    // Callback for suspend/resume
    void UsbDevSuspend(bool state);

    // Send reply to current Bulk-OUT request
    void Reply(String& s);

    // Issue IEEE-488 service request
    // XXX implement the full 488.2 SR, ESB, and MAV mechanism
    // See e.g. http://www.ni.com/white-paper/4056/en/
    void Srq();

    // Flash die blinkenlights
    virtual void Pulse() { };

    // Perform query
    virtual void Query(String& s) { };

    // Perform ask
    virtual void Ask(String& s);

private:
    UsbTmc(const UsbTmc&);
    UsbTmc& operator=(const UsbTmc&);
};

#endif // _USBTMC_H_
