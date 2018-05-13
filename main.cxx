// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"
#include "util.h"

#include "font/runes.inc"

extern Eeprom _eeprom;
extern PinNegOutput<LpcGpio::Pin> _led8;

struct Config {
    uint32_t version;

private:
    uint16_t crc16;             // CRC-16 with initial 0xffff, bit reverse, complement sum

public:
    // Calculate data checksum
    uint16_t Checksum() {
        return Crc16::Checksum(this, (uintptr_t)&crc16 - (uintptr_t)this, 0xffff);
    }

    // Fill from EEPROM
    bool Load(uint16_t addr) {
        DMSG("Loading config from EEPROM");
        _eeprom.Read(addr, this, sizeof *this);
        return Checksum() == crc16;
    }

    // Store to EEPROM
    bool Store(uint16_t addr) {
        DMSG("Storing config to EEPROM");

        crc16 = Checksum();
        _eeprom.Write(addr, this, sizeof *this);

        // Read it back
        return Load(addr);
    }
};

Config _config;

enum { CONFIG_ADDR = 0x100 };       // EEPROM address for config

Thread* _net_thread;
Thread* _ui_thread;

extern void UsbInit();
extern void* UIThread(void*);

int main() {
    if (!_config.Load(CONFIG_ADDR)) {
        console("NOTE: No valid config, initializing");

        _config.version = 0xd00dad;

        if (!_config.Store(CONFIG_ADDR)) {
            panic("Config store failed");
        }
    }

#ifdef ENABLE_USB
    UsbInit();
#endif

#ifdef ENABLE_ENET
    _net_thread = Thread::Create("network", NetThread, NULL, NET_THREAD_STACK);
#endif

#ifdef ENABLE_PANEL
    _ui_thread = Thread::Create("ui", UIThread, NULL, UI_THREAD_STACK);
#endif


    _fat.Mount(0, false);

    File* file = _fat.Open("issue");

    if (file) {
        uint8_t* buffer = (uint8_t*)xmalloc(100);

        const uint len = file->Read(buffer, 99);
        buffer[len] = 0;
        
        console("%s", buffer);
        xfree(buffer);

        file->Close();
    }

    DMSG("Main: blinking lights");

    Time wake = Time::Now();
    for (;;) {
        _led8.Raise();
        wake += Time::FromMsec(500);
        Thread::Sleep(wake);

        _led8.Lower();
        wake += Time::FromMsec(250);
        Thread::Sleep(wake);
    }
}
