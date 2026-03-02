// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

#include "core/enetcore.h"
#include "core/util.h"
#include "core/sdcard.h"
#include "core/gptmap.h"
#include "core/fat32.h"
#include "core/blkcache.h"
#include "soc/stm32h7x/sdmmc.h"


extern PinOutput<Gpio::Pin> _card_led;

Stm32Sdmmc1      _sdmmc;
SDCard           _sdcard(_sdmmc);
GPTMap::Table*   _gpt;
GPTMap::Mapper*  _mapper;
CacheBlockDev*   _bcache;
Fat32::FileSys*  _fs;


template <typename T>
static T abs(const T& a) {
    return a < (T)0 ? -a : a;
}


static void logDateTime() {
    const auto now = Rtc::GetDateTime();
    static const char* const days[] = {
        "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
    };

    DMSG("%s %04d-%02d-%02d %02d:%02d:%02d\n",
         days[now.dow-1],
         now.year,
         now.month,
         now.day,
         now.hour,
         now.min,
         now.sec);
}

int main() {
    Thread::EnableFP();

    DMSG("Main: blinking lights");
    
    _sdcard.init();
    _gpt = new GPTMap::Table(_sdcard);
    
    int fatpart = -1;
    for (int i = 0;  i < _gpt->count(); i++) {
        DMSG("\n--- partition #%d ---\n", i+1);
        DMSG("Name: \"%s\"\n", _gpt->get(i).name);
        DMSG("Type: %s (as %s)\n",
               GPTMap::Table::typestr(_gpt->get(i).type),
               GPTMap::Table::typestr(_gpt->get(i).otype)
            );
        DMSG("Index: %d\n", _gpt->get(i).entry_index);
        DMSG("First LBA: %llu\n", _gpt->get(i).first_lba);
        DMSG("Last LBA: %llu\n", _gpt->get(i).last_lba);

        const uint64_t lba_count = _gpt->get(i).last_lba - _gpt->get(i).first_lba;
        DMSG("LBA size: %llu (%lluMB)\n",  lba_count, (lba_count * _sdcard.sector_size()) >> 20);
        DMSG("Attributes: 0x%llx\n\n", _gpt->get(i).attributes);

        if (_gpt->get(i).type == GPTMap::TYPE_FAT32)
            fatpart = i;
    }

    assert(fatpart != -1);

    _mapper = new GPTMap::Mapper(_sdcard, _gpt->get(fatpart));
    _bcache = new CacheBlockDev(*_mapper, false);

    _fs = new Fat32::FileSys(*_bcache);

    Time wake = Time::Now();
    for (;;) {
        _card_led.Raise();
        logDateTime();
        wake += Time::FromMsec(150);
        Thread::Sleep(wake);

        _card_led.Lower();
        logDateTime();
        wake += Time::FromMsec(500);
        Thread::Sleep(wake);
    }
}
