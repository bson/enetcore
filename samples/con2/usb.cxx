// Copyright (c) 2018-2021 Jan Brittenson
// See LICENSE for details.

#ifdef ENABLE_USB

#include "enetcore.h"
#include "usb.h"
#include "usbtmc.h"

using namespace usb;

// For USB, if used
enum {
    USB_VID = 0x0401,
    USB_PID = 0x0703
};

static const DeviceDescriptor usbdev = {
        sizeof(DeviceDescriptor),  // 18
        TYPE_DEVICE,
        0x0200,       // USB 2.0.0
        0, 0, 0,      // Class, subclass, protocol defined in interface
        64,           // Max packet size for EP0
        USB_VID, USB_PID, // VID, PID
        0x0100,      // Device version 1.0.0
        1, 2, 3,     // Mfg, prod, sn strings
        1            // 1 configuration
};

static const ConfigDescriptor usbconf = {
        sizeof(ConfigDescriptor), // 8
        TYPE_CONFIG,
        0,         // Total length (filled in when sent)
        1,         // 1 = # of interfaces
        1,         // 1 = config value for to select this config
        0,         // Config string
        1 << 6,    // Self powered
        20         // Will draw max 50mA
};

static const InterfaceDescriptor usbif = {
        sizeof(InterfaceDescriptor), // 8
        TYPE_INTERFACE,
        0,           // 0 = first interface
        0,           // no alt config
        2,           // two endpoints (interrupt IN, OUT)
        0xfe,3,1,    // USBTMC (app) class, subclass, protocol (usb488)
        0,           // Interface string
};

static const EndpointDescriptor usbep[] = {
    {
     // EP1 OUT - bulk
     sizeof(EndpointDescriptor), TYPE_ENDPOINT,
     EP_ADDR_IN | 2, EP_ATTR_TTBULK, 64, 0
    },
    {
     // EP1 IN - bulk
     sizeof(EndpointDescriptor), TYPE_ENDPOINT,
     EP_ADDR_OUT | 2, EP_ATTR_TTBULK, 64, 0
    },
    {
     // EP2 IN - interrupt, poll 50ms.  Packet size 2 is mandated by USB488
     sizeof(EndpointDescriptor), TYPE_ENDPOINT,
     EP_ADDR_IN | 1, EP_ATTR_TTINTR, 0x02, 50,
    }
};

static const char* strs[3] = {
    "JB Design",
    "Sky Blue",
    "0000001",
};

const Usb::Descriptors descriptors = {
    &usbdev,
    &usbconf,
    &usbif, 1,
    usbep, 3,
    strs, 3,
};


const char *idn = "JB DESIGN,Sky Blue,0000001,01.00.00";


UsbTmc _usbtmc(_usb);

Thread* _usb_thread = NULL;

void UsbInit() {
    _usb.SetDescriptors(&descriptors);
    _usb.Init();
    _usbtmc.Init();

    _usb_thread = Thread::Create("usb", Usb::Start, &_usb, USB_THREAD_STACK);
}

#endif // ENABLE_USB
