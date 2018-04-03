// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"
#include "util.h"

extern const char _build_commit[];
extern const char _build_user[];
extern const char _build_date[];
extern const char _build_branch[];

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

extern void UsbInit();

int main() {
    _malloc_region.SetReserve(64);

    console("\r\nSky Blue Rev 3 [%s:%s %s %s]",
            _build_branch, _build_commit, _build_user, _build_date);

    console("Copyright (c) 2018 Jan Brittenson");
    console("All Rights Reserved\r\n");

    DMSG("Random uint: 0x%x", Util::Random<uint>());

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
    _panel.SetBackground(64, 64, 80);
    _panel.Init();

#if 1
    _panel.TestPattern();
#endif

    _panel.SetRgb(255, 255, 0);
    _panel.Text(120, 8, font_ins_9x16, STR("Hello, world!"));

    _panel.SetRgb(255, 255, 255);
    _panel.Text(10, 200, font_ins_9x16, 
                STR("The quick brown fox jump.s ov,er the lazy dog!"));
    _panel.Text(10, 220, font_5x8, 
                STR("The quick; brown; fox... 'jumps' over: the \"lazy\" dog?! <=>[\\]{|}"));
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

    _panel.SetRgb(255, 255, 0);

    uint8_t duty = 1;
    for (;;) {
        uint val = _clock.GetTime();
        
        const String s = String::Format(STR("%08x"), val);
        _panel.Text(320, 8, font_5x8, s, 1, false);
        _panel.Text(320, 20, font_ins_9x16, s, 1, false);

        const String s2 = String::Format(STR("%08d"), val);
        _panel.Text(200, 48, font_ins_25x40, s2, 3, false);

        _led8.Raise();
        Thread::Sleep(Time::Now() + Time::FromMsec(500));
        _led8.Lower();
        Thread::Sleep(Time::Now() + Time::FromMsec(250));

    }
}
