//
// Copyright 2026 Jan Brittenson
// See LICENSE for details. 
//

#ifndef __SDMMC_H__
#define __SDMMC_H__

#include <stdint.h>
#include "core/sdio.h"
#include "core/thread.h"
#include "core/mutex.h"


// Assume only one SDIO bus, so specialize it.
template <uintptr_t SDMMC>
class Stm32Sdio: public Sdio {
    enum class Register {
        POWER = 0x000,
        CLKCR = 0x004,
        ARGR = 0x008,
        CMDR = 0x00c,
        RESPCMDR = 0x010,
        RESP1R = 0x014,
        RESP2R = 0x018,
        RESP3R = 0x01c,
        RESP4R = 0x020,
        DTIMER = 0x024,
        DLENR = 0x028,
        DCTRLR = 0x02c,
        DCNTR = 0x030,
        STAR = 0x034,
        ICR = 0x038,
        MASKR = 0x03c,
        ACKTIMER = 0x040,
        IDMACTRLR = 0x050,
        IDMABSIZER = 0x054,
        IDMABASE0R = 0x058,
        IDMABASE1R = 0x05c,
        FIFORx = 0x080,
    };

    
    enum {
        // POWER
        DIRPOL = 4,
        VSWITCHEN = 3,
        VSWITCH = 2,
        PWRCTL = 0,             // 2b
        PWRCTL_W = 2,

        // CLKCR
        SELCLKRX = 20,          // 2b
        SELCLKRX_W = 2,
        BUSSPEED = 19,
        DDR = 18,
        HWFC_EN = 17,
        NEGEDGE = 16,
        WIDBUS = 14,            // 2b
        WIDBUS_W = 2,
        PWRSAV = 12,
        CLKDIV = 0,             // 10b
        CLKDIV_W = 10,

        // CMDR
        CMDSUSPEND = 16,
        BOOTEN = 15,
        BOOTMODE = 14,
        DTHOLD = 13,
        CPSMEN = 12,
        WAITPEND = 11,
        WAITINT = 10,
        WAITRESP = 8,           // 2b
        WAITRESP_W = 2,
        CMDSTOP = 7,
        CMDTRANS = 6,
        CMDINDEX = 0,           // 6b
        CMDINDEX_W = 6,

        // RESPCMDR
        RESPCMD = 0,            // 6b
        RESPCMD_W = 6,

        // DLENR
        DATALENGTH = 0,         // 25b
        DATALENGTH_W = 25,

        // DCTRL
        FIFORST = 13,
        BOOTACKEN = 12,
        SDIOEN = 11,
        RWMOD = 10,
        RWSTOP = 9,
        RWSTART = 8,
        DBLOCKSIZE = 4,         // 4b
        DBLOCKSIZE_W = 4,
        DTMODE = 2,             // 2b
        DTMODE_W = 2,
        DTDIR = 1,
        DTEN = 0,

        // DCNTR
        DATACOUNT = 0,          // 25b
        DATACOUNT_W = 25,

        // STAR
        IDMABTC = 28,
        IDMATE = 27,
        CKSTOP = 26,
        VSWEND = 25,
        ACKTIMEOUT = 24,
        ACKFAIL = 23,
        SDIOIT = 22,
        BUSYD0END = 21,
        BUSYD0 = 20,
        RXFIFOE = 19,
        TXFIFOE = 18,
        RXFIFOF = 17,
        TXFIFOF = 16,
        RXFIFOHF = 15,
        TXFIFOHE = 14,
        CPSMACT = 13,
        DPSMACT = 12,
        DABORT = 11,
        DBCKEND = 10,
        DHOLD = 9,
        DATAEND = 8,
        CMDSENT = 7,
        CMDREND = 6,
        RXOVERR = 5,
        TXUNDERR = 4,
        DTIMEOUT = 3,
        CTIMEOUT = 2,
        DCRCFAIL = 1,
        CCRCFAIL = 0,

        // ICR
        IDMABTCC = 28,
        IDMATEC = 27,
        CKSTOPC = 26,
        VSWENDC = 25,
        ACKTIMEOUTC = 24,
        ACKFAILC = 23,
        SDIOITC = 22,
        BUSYD0ENDC = 21,
        DABORTC = 11,
        DBCKENDC = 10,
        DHOLDC = 9,
        DATAENDC = 8,
        CMDSENTC = 7,
        CMDRENDC = 6,
        RXOVERRC = 5,
        TXUNDERRC = 4,
        DTIMEOUTC = 3,
        CTIMEOUTC = 2,
        DCRCFAILC = 1,
        CCRCFAILC = 0,

        // MASKR
        IDMABTCE = 28,
        CKSTOPIE = 26,
        VSWENDIE = 25,
        ACKTIMEOUTIE = 24,
        ACKFAILIE = 23,
        SDIOITIE = 22,
        BUSYD0ENDIE = 21,
        TXFIFOEIE = 18,
        RXFIFOFIE = 17,
        RXFIFOHFIE = 15,
        TXFIFOHEIE = 14,
        DABORTIE = 11,
        DBCKENDIE = 10,
        DHOLDIE = 9,
        DATAENDIE = 8,
        CMDSENTIE = 7,
        CMDRENDIE = 6,
        RXVERRIE = 5,
        TXUNDERRIE = 4,
        DTIMEOUTIE = 3,
        CTIMEOUTIE = 2,
        DCRCFAILIE = 1,
        CCRCFAILIE = 0,

        // ACKTIMER
        ACKTIME = 0,            // 25b
        ACTIME_W = 25,

        // IDMACTRLR
        IDMABACT = 2,
        IDMABMODE = 1,
        IDMAEN = 0,

        // IDMABSIZE
        IDMABNDT = 5,           // 8b
        IDMABNDT_W = 8,
    };


    enum Widbus {
        SingleBit = 0b00,
        FourBit = 0b01,
        EightBit = 0b10
    };

    enum class State {
        IDLE = 0,
        CMD_IN_PROGRESS,
        DATA_IN_PROGRESS,
        DONE
    };

public:
    enum Result : int {
        SUCCESS = 0,
        CMD_TIMEOUT = -1,
        CMD_CRC_FAIL = -2,
        DATA_TIMEOUT = -3,
        DATA_CRC_FAIL = -4,
    };

private:
    mutable Mutex _lock;
    State         _state;
    Result        _result;

    volatile uint32_t& reg(Register r) { return *(volatile uint32_t*)(SDMMC + (uint32_t)r); }

	// Interrupt handler
	static void Interrupt(void* token) {
        ((Stm32Sdio<SDMMC>*)token)->HandleInterrupt();
    }

    // start new operation, blocking to wait.  Changes to new state.  Clears result.
    void start(State new_state);

public:
    // NVIC needs to set up for IRQ, with object ref to this
    Stm32Sdio();

    // Sdio interface implementation
    void set_clock(uint32_t hz);
    void set_bus_width_1bit();
    void set_bus_width_4bit();
    Result send_cmd(uint8_t cmd, uint32_t arg, uint32_t resp_type, uint32_t *response);
    Result data_read(void *buf, uint32_t bytes);
    Result data_write(const void *buf, uint32_t bytes);
    bool get_data_crc_error();
    bool get_data_timeout();
    void reset_datapath();

    // Interrupt handler
    void HandleInterrupt();
};

typedef Stm32Sdio<BASE_SDMMC1> Stm32Sdmmc1;
typedef Stm32Sdio<BASE_SDMMC2> Stm32Sdmmc2;

#endif // __SDMMC_H__
