#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#define __coredata // __section(".iram") // Locate core data in internal RAM

#include "lpc.h"

// Main init function
extern "C" {
void hwinit();
}


extern "C" {
// These symbols are generated by the linker in link.cmd
extern uint8_t _data;
extern uint8_t _edata;
extern uint8_t _bss_start;
extern uint8_t _bss_end;
extern uint8_t _etext;
extern uint8_t _iram;
extern uint8_t _eiram;
extern uint8_t _xflash;
extern uint8_t _exflash;
};


extern "C" {
void Unexpected_Interrupt() __irq __naked;
void Data_Abort_Exception() __abort __naked;
void Program_Abort_Exception() __abort __naked;
void Undef_Exception() __undef __naked;
void SWI_Trap() __swi __naked;
void busy_wait () __noinstrument __naked;
}

void fault0(uint num);
inline void fault(uint num, bool captive = true) { do fault0(num); while (captive); }


enum { XRAM_SIZE = 1024*1024 };

#define MALLOC_REGION_START  (&_bss_end)
#define MALLOC_REGION_SIZE   ((&_data + XRAM_SIZE) - &_bss_end)

#define IRAM_REGION_START (&_iram)
#define IRAM_REGION_SIZE (&_eiram - &_iram)

#define DATA_REGION_START (&_data)
#define DATA_REGION_SIZE (&_edata - &_data)

#define TEXT_REGION_START  ((uint8_t*)0)
#define TEXT_REGION_SIZE (&_etext - (uint8_t*)0 + (DATA_REGION_SIZE))

#define XFLASH_REGION_START (&_xflash)
#define XFLASH_REGION_SIZE (&_exflash - &_xflash)

extern void* _main_thread_stack;
extern void* _intr_thread_stack;

#endif // __HARDWARE_H__
