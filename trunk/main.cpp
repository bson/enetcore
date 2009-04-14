#include "enetkit.h"
#include "serial.h"
#include "util.h"
#include "sdcard.h"


struct NOVTABLE PartEnt {
	uint8_t boot_flag;
	uint8_t chs_begin[3];
	uint8_t type_code;
	uint8_t chs_end[3];
	uint32_t lba_begin;
	uint32_t num_sect;
};


static PartEnt _part[4];


int	main ()
{
	_malloc_region.SetReserve(4096);

	_lcd.Write(String(STR("\xfe\1Enetcore 0.1 DEV")));
	_lcd.SyncDrain();

	_sd.Init();

	static uint8_t sector[512];
	_sd.ReadSector(0, sector);

	memcpy(_part, sector + 446, sizeof _part);

	for (uint i = 0; i < 4; ++i) {
		PartEnt* p = _part + i;
		DMSG("%u: flag=%d  type=0x%x  lba=%u  num_sec=%u",
			 i, (uint)p->boot_flag, (uint)p->type_code, p->lba_begin, p->num_sect);
	}

	abort();
}
