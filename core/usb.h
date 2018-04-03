// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef _USB_H_
#define _USB_H_

// Common USB definitions

#include "bits.h"

namespace usb {

// Device class.
class DeviceClass {
public:
    // Callback during configuration - class should define its EPs
    virtual void UsbDevInit() { }

    // Process SETUP.  Called at USB_IPL. Returns true if setup was
    // processed
    virtual bool UsbDevProcessSetup(const struct SetupRequest* setup) { }

    // Process IN to host
    virtual void UsbDevProcessIN(uint phyep) { }

    // Process OUT from host
    virtual void UsbDevProcessOUT(uint pyep) { }

    // Callback for reset
    virtual void UsbDevReset() { }

    // Callback for connect/disconnect
    virtual void UsbDevConnect(bool state) { }

    // Callback for suspend/resume
    virtual void UsbDevSuspend(bool state) { }
};


// Setup request data
struct SetupRequest {
    uint8_t type;
    uint8_t request;
    uint16_t value;
    uint16_t index;
    uint16_t length;
};

// Requests
enum {
    REQ_GET_STATUS = 0,
    REQ_CLEAR_FEATURE = 1,
    REQ_SET_FEATURE = 3,
    REQ_SET_ADDRESS = 5,
    REQ_GET_DESC = 6,
    REQ_SET_DESC = 7,
    REQ_GET_CONF = 8,
    REQ_SET_CONF = 9
};

// Request recipients
enum {
    REQ_DEVICE = 0,
    REQ_INTERFACE,
    REQ_ENDPOINT,
    REQ_OTHER
};

// Descriptor types
enum {
    TYPE_DEVICE = 1,
    TYPE_CONFIG = 2,
    TYPE_STRING = 3,
    TYPE_INTERFACE = 4,
    TYPE_ENDPOINT = 5
};

// Descriptors.  These are passed in verbatim  to the constructor,
// to facilitate storing them preconstructed in ROM.

// Device descriptor
struct DeviceDescriptor {
    uint8_t  length;    // sizeof(DeviceDescriptor)
    uint8_t  type;      // TYPE_DEVICE
    uint16_t bcdusb;    // 0x0200 = 2.0 spec version in BCD
    uint8_t  devclass;  // Device class (USB org assigned)
    uint8_t  subclass;  // Sub class (USB org assigned)
    uint8_t  protocol;  // Protocol (USB org assigned)
    uint8_t  pktsize;    // 64
    uint16_t vid;
    uint16_t pid;
    uint16_t devrel;    // Device release, BCD
    uint8_t  manuf;     // Manufacturer string descriptor
    uint8_t  prod;      // Product string descriptor
    uint8_t  serial;    // Serial number string descriptor
    uint8_t  numconf;   // 1 = number of configurations
};

// Configuration descriptor
struct ConfigDescriptor {
    uint8_t  length;    // sizeof(ConfigDescriptor)
    uint8_t  type;      // TYPE_CONFIG
    uint16_t total;     // total length including interfaces and enpoints
    uint8_t  numif;     // # of interfaces
    uint8_t  confval;   // 1 = config value
    uint8_t  confstr;   // Config string descriptor
    uint8_t  attrs;     // Attribute bitmap
    uint8_t  maxpower;  // Max current draw, times 2mA
};

// Interface descriptor
struct InterfaceDescriptor {
    uint8_t  length;    // sizeof(InterfaceDescriptor)
    uint8_t  type;      // TYPE_INTERFACE
    uint8_t  ifnum;     // interface number (zero based)
    uint8_t  alt;       // alternative setting (not used, zero)
    uint8_t  numep;     // Number of endpoints
    uint8_t  ifclass;   // Interface class (USB org assigned)
    uint8_t  ifsubclass; // (USB org assigned)
    uint8_t  ifproto;    // (USB org assigned)
    uint8_t  ifstring;   // Interface string
};

// Endpoint descriptor
struct EndpointDescriptor {
    uint8_t  length;     // sizeof(EndpointDescriptor)
    uint8_t  type;       // TYPE_ENDPOINT
    uint8_t  addr;       // Address (bit 7 = 0 for out, 1 for in)
    uint8_t  attr;       // Attribute bits
    uint16_t maxpkt;     // Max packet size
    uint8_t  interval;   // Polling interval in ms (2.0) or 0.125ms (2.0 HS)
};

// Endpoint attributes
enum {
    // Transfer type
    EP_ATTR_TTCONTROL = 0b00,
    EP_ATTR_TTISO     = 0b01,  // Isochronous
    EP_ATTR_TTBULK    = 0b10,  // Bulk
    EP_ATTR_TTINTR    = 0b11,  // Interrupt

    // Isochronous sync
    EP_ATTR_ISONOSYNC = 0b0000,
    EP_ATTR_ISOASYNC  = 0b0100,
    EP_ATTR_ISOADAPT  = 0b1000,
    EP_ATTR_ISOSYNC   = 0b1100,

    // Isochronous usage mode
    EP_ATTR_ISO_DATAEP = 0b000000, // Data EP
    EP_ATTR_ISO_FBEP   = 0b010000, // Feedback EP
    EP_ATTR_ISO_DFBEP  = 0b100000  // Data feedback EP
};

// Endpoint direction (for addr)
enum {
    EP_ADDR_IN  = 0,
    EP_ADDR_OUT = 0x80
};

// String descriptor (minus actual string)
struct StringDescriptor {
    uint8_t  length;     // Length of descriptor
    uint8_t  type;       // 3 = string
};

// String desc 0 (english language)
struct StringDesc0 {
    uint8_t  length;     // sizeof(StringDesc0)
    uint8_t  type;       // 3 = string
    uint16_t lang;       // 0x409 = US english
};

} // namespace usb

#endif // _USB_H_
