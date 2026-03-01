//
// Copyright 2026 Jan Brittenson
// See LICENSE for details. 
//
#include <stdint.h>

#include "core/enetcore.h"
#include "core/mutex.h"
#include "core/thread.h"
#include "core/bits.h"
#include "core/bitfield.h"

#include "soc/stm32h7x/sdmmc.h"


// Note: device needs to be enabled before use

static uint32_t get_kernel_clk()
{
    return AHB_FREQ;
}


template <uintptr_t SDMMC>
Stm32Sdio<SDMMC>::Stm32Sdio()
    : _state(State::IDLE), _result(0)
{
    reg(Register::MASKR) = Bitfield(0)
        .bit(CMDRENDIE)
        .bit(CMDSENTIE)
        .bit(CTIMEOUTIE)
        .bit(CCRCFAILIE)
        .bit(DATAENDIE)
        .bit(DTIMEOUTIE)
        .bit(DCRCFAILIE);
}


template <uintptr_t SDMMC>
void Stm32Sdio<SDMMC>::set_clock(uint32_t hz)
{
    uint32_t div = (get_kernel_clk() / hz / 2);

    // Round div up (freq down) to avoid overshooting
    if (get_kernel_clk() % hz)
        ++div;

    if (div > 0)
        --div;

    assert(div <= 0x3ff);

    DMSG("SDMMC: ker_ck = %u, div = %u  =>  SDIO_CK = %u", AHB_FREQ, div, AHB_FREQ/div);

    reg(Register::CLKCR) = Bitfield(reg(Register::CLKCR))
        .f(CLKDIV_W, CLKDIV, div);
}


template <uintptr_t SDMMC>
void Stm32Sdio<SDMMC>::set_bus_width_1bit()
{
    reg(Register::CLKCR) = Bitfield(reg(Register::CLKCR))
        .f(WIDBUS_W, WIDBUS, Widbus::SingleBit);
}


template <uintptr_t SDMMC>
void Stm32Sdio<SDMMC>::set_bus_width_4bit()
{
    reg(Register::CLKCR) = Bitfield(reg(Register::CLKCR))
        .f(WIDBUS_W, WIDBUS, Widbus::FourBit);
}


template <uintptr_t SDMMC>
void Stm32Sdio<SDMMC>::start(typename Stm32Sdio<SDMMC>::State new_state)
{
    _lock.Lock();
    while (_state != State::IDLE) {
        _lock.Unlock();
        Thread::WaitFor(this);
        _lock.Lock();
    }

    _state = new_state;
    _result = Result::SUCCESS;

    _lock.Unlock();
}


template <uintptr_t SDMMC>
typename Stm32Sdio<SDMMC>::Result
Stm32Sdio<SDMMC>::send_cmd(uint8_t cmd,
                           uint32_t arg,
                           uint32_t resp_type,
                           uint32_t *response)
{
    start(State::CMD_IN_PROGRESS);

    reg(Register::ICR) = 0xffffffff;
    reg(Register::ARGR) = arg;

    uint32_t cmdreg = Bitfield(0)
        .f(CMDINDEX_W, CMDINDEX, cmd)
        .bit(CPSMEN);

    if (resp_type == 2)
        cmdreg = Bitfield(cmdreg).f(WAITRESP_W, WAITRESP, 2);
    else if (resp_type != 0)
        cmdreg = Bitfield(cmdreg).f(WAITRESP_W, WAITRESP, 1);

    reg(Register::CMDR) = cmdreg;

    while (_state != State::DONE)
        Thread::WaitFor(this);

    _state = State::IDLE;
    Thread::WakeSingle(this);

    return _result;
}


template <uintptr_t SDMMC>
typename Stm32Sdio<SDMMC>::Result
Stm32Sdio<SDMMC>::data_read(void *buf, uint32_t bytes)
{
    // Buffer must be aligned on 32-byte boundary
    assert(((uintptr_t)buf & 31) == 0);

    start(State::DATA_IN_PROGRESS);

    reg(Register::DTIMER) = 0xffffffff;
    reg(Register::DLENR)   = bytes;

    reg(Register::DMABASE0) = (uintptr_t)buf;
    reg(Register::IDMACTRL) = BIT(IDMAEN);

    reg(Register::DCTRL) = Bitfield(0)
        .bit(DTEN)
        .bit(DTDIR)
        .f(DBLOCKSIZE_W, DBLOCKSIZE, 9); // 512-byte block

    while (_state != State::DONE)
        Thread::WaitFor(this);

    invalidate_dcache(buf, bytes);

    _state = State::IDLE;
    Thread::WakeSingle(this);

    return _result;
}


template <uintptr_t SDMMC>
typename Stm32Sdio<SDMMC>::Result
Stm32Sdio<SDMMC>::data_write(const void *buf, uint32_t bytes)
{
    // Buffer must be aligned on 32-byte boundary
    assert(((uintptr_t)buf & 31) == 0);

    start(State::DATA_IN_PROGRESS);

    flush_dcache(buf, bytes);

    reg(Register::DTIMER) = 0xffffffff;
    reg(Register::DLENR)   = bytes;

    reg(Register::IDMABASE0) = (uintptr_t)buf;
    reg(Register::IDMACTRL)  = BIT(IDMAEN);

    reg(Register::DCTRL) = Bitfield(0)
        .bit(DTEN)
        .f(DBLOCKSIZE_W, DBLOCKSIZE, 9); // 512-byte block

    while (_state != State::DONE)
        Thread::WaitFor(this);

    _state = State::IDLE;
    Thread::WakeSingle(this);

    return _result;
}


template <uintptr_t SDMMC>
bool Stm32Sdio<SDMMC>::get_data_crc_error()
{
    return reg(Register::STAR) & BIT(DCRCFAIL);
}


template <uintptr_t SDMMC>
bool Stm32Sdio<SDMMC>::get_data_timeout()
{
    return reg(Register::STAR) & BIT(DTIMEOUT);
}


template <uintptr_t SDMMC>
void Stm32Sdio<SDMMC>::reset_datapath()
{
    reg(Register::DCTRL) = 0;
    reg(Register::IDMACTRL) = 0;
    reg(Register::ICR) = 0xffffffff;
}


template <uintptr_t SDMMC>
void Stm32Sdio<SDMMC>::HandleInterrupt()
{
    uint32_t sta = reg(Register::STAR);

    /* Command errors */
    if (sta & BIT(CTIMEOUT))
        _result = Result::CMD_TIMEOUT;

    if (sta & BIT(CCRCFAIL))
        _result = Result::CMD_CRC_FAIL;

    /* Data errors */
    if (sta & BIT(DTIMEOUT))
        _result = Result::DATA_TIMEOUT;

    if (sta & BIT(DCRCFAIL))
        _result = Result::DATA_CRC_FAIL;

    /* Command complete */
    if (sta & (BIT(CMDREND) | BIT(CMDSENT))) {
        reg(Register::ICR) = Bitfield(0)
            .bit(CMDRENDC)
            .bit(CMDSENTC)
            .bit(CTIMEOUTC)
            .bit(CCRCFAILC);

        if (_state == State::CMD_IN_PROGRESS) {
            _state = State::DONE;
            Thread::WakeSingle(this);
        }
    }

    /* Data complete */
    if (sta & BIT(DATAEND)) {

        reg(Register::IDMACTRL) = 0;
        reg(Register::ICR) = Bitfield(0)
            .bit(DATAENDC)
            .bit(DTIMEOUTC)
            .it(DCRCFAILC);

        _state = State::DONE;
        Thread::WakeSingle(this);
    }
}
