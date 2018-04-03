// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef _LPC_PWM_H_
#define _LPC_PWM_H_

class LpcPwm {
    volatile uint32_t* _base;
    uint32_t _match;

    enum {
        RESOLUTION = 1 << 20
    };

public:
    enum {
        REG_IR   = 0x000/4,
        REG_TCR  = 0x004/4,
        REG_TC   = 0x008/4,
        REG_PR   = 0x00C/4,
        REG_PC   = 0x010/4,
        REG_MCR  = 0x014/4,
        REG_MR0  = 0x018/4,
        REG_MR1  = 0x01C/4,
        REG_MR2  = 0x020/4,
        REG_MR3  = 0x024/4,
        REG_CCR  = 0x028/4,
        REG_CR0  = 0x02C/4,
        REG_CR1  = 0x030/4,
        REG_MR4  = 0x040/4,
        REG_MR5  = 0x044/4,
        REG_MR6  = 0x048/4,
        REG_PCR  = 0x04C/4,
        REG_LER  = 0x050/4,
        REG_CTCR = 0x070/4,
    };

    LpcPwm(uintptr_t base)
        : _base((volatile uint32_t*)base)
    { }

    // Set period frequency
    void SetPeriodFreq(uint freq);

    // Set duty cycle in 256ths on output N
    void SetDuty(uint8_t frac, uint n);
};

#endif // _LPC_PWM_H_
