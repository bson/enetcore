#ifndef  _TLK110_H_
#define  _TLK110_H_

#include <stdint.h>
#include "core/enet_phy.h"

class Ethernet;

namespace EnetPHY {

// TLK110 Extended PHY registers
enum class XPhyReg: uint16_t {
    // (RO) - TX_CLK Phase Shift Register
    TXCPSR = 0x0042,

    // (RW) - Power Back Off Control Register
    PWRBOCR = 0x00ae,

    // (RW) - Voltage Regulator Control Register
    VRCR = 0x00d0,

    // (RW) - ALCD Control and Results 1
    ALCDRR1 = 0x0155,

    // (RW) - Cable Diagnostic Specific Control Register 1
    CDSCR1 = 0x0170,

    // (RW) - Cable Diagnostic Specific Control Register 2
    CDSCR2 = 0x0171,

    // (RW) - Cable Diagnostic Specific Control Register 3
    CDSCR3 = 0x0173,

    // (RW) - Cable Diagnostic Specific Control Register 4
    CDSCR4 = 0x0177,

    // (RO) - Cable Diagnostic Location Result Register 1
    CDLRR1 = 0x0180,

    // (RO) - Cable Diagnostic Location Result Register 2
    CDLRR2 = 0x0181,

    // (RO) - Cable Diagnostic Location Result Register 3
    CDLRR3 = 0x0182,

    // (RO) - Cable Diagnostic Location Result Register 4
    CDLRR4 = 0x0183,

    // (RO) - Cable Diagnostic Location Result Register 5
    CDLRR5 = 0x0184,

    // (RO) - Cable Diagnostic Amplitude Result Register 1
    CDLAR1 = 0x0185,

    // (RO) - Cable Diagnostic Amplitude Result Register 2
    CDLAR2 = 0x0186,

    // (RO) - Cable Diagnostic Amplitude Result Register 3
    CDLAR3 = 0x0187,

    // (RO) - Cable Diagnostic Amplitude Result Register 4
    CDLAR4 = 0x0188,

    // (RO) - Cable Diagnostic Amplitude Result Register 5
    CDLAR5 = 0x0189,

    // (RW) - Cable Diagnostic General Result Register
    CDGRR = 0x018a,

    // (RW) - ALCD Control and Results 2 Register
    ALCDRR2 = 0x0215
};

// Probe for PHY, returns true if it's there and looks alright.
bool CheckPHY(Ethernet& eth);

// Pre-configuration initialization
static inline void PreConf(Ethernet& eth) { }
    
// Configure
void Configure(Ethernet& eth);

// Post-configuration initialization
void PostConf(Ethernet& eth);

}; // ns EnetPHY

#endif // _TLK110_H_
