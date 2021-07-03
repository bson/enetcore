// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef _ENET_PHY_H_
#define _ENET_PHY_H_

namespace enet_phy {

// Standard PHY registers
enum class PhyReg: uint16_t {
    // (RW) - Basic Mode Control Register
    BMCR = 0x00,

    // (RO) - Basic Mode Status Register
    BMSR = 0x01,

    // (RO) - PHY Identifier Register 1
    PHYIDR1 = 0x02,

    // (RO) - PHY Identifier Register 2
    PHYIDR2 = 0x03,

    // (RW) - Auto-Negotiation Advertisement Register
    ANAR = 0x04,

    // (RO) - Auto-Negotiation Link Partner Ability Register
    ANLPAR = 0x05,

    // (RO) - Auto-Negotiation Expansion Register
    ANER = 0x06,

    // (RW) - Auto-Negotiation Next Page TX
    ANNPTR = 0x07,

    // (RO) - Auto-Negotiation Link Partner Ability Next Page Register
    ANLNPTR = 0x08,

    // (RW) - Software Strap Control Register 1
    SWSCR1 = 0x09,

    // (RW) - Software Strap Control Register 2
    SWSCR2 = 0x0a,

    // (RW) - Software Strap Control Register 3
    SWSCR3 = 0x0b,

    // (RW) - Register control register
    REGCR = 0x0d,

    // (RW) - Address or Data register
    ADDAR = 0x0e,

    // (RO) - PHY Status Register
    PHYSTS = 0x10,

    // (RW) - PHY Specific Control Register
    PHYSCR = 0x11,

    // (RW) - MII Interrupt Status Register 1
    MISR1 = 0x12,

    // (RW) - MII Interrupt Status Register 2
    MISR2 = 0x13,

    // (RO) - False Carrier Sense Counter Register
    FCSCR = 0x14,

    // (RO) - Receive Error Count Register
    RECR = 0x15,

    // (RW) - BIST Control Register
    BISCR = 0x16,

    // (RO) - RMII and Status Register
    RCSR = 0x17,

    // (RW) - LED Control Register
    LEDCR = 0x18,

    // (RW) - PHY Control Register
    PHYCR = 0x19,

    // (RW) - 10Base-T Status/Control Register
    R10BTSCR = 0x1a,

    // (RW) - BIST Control and Status Register 1
    BICSR1 = 0x1b,

    // (RO) - BIST Control and Status Register 2
    BICSR2 = 0x1c,

    // (RW) - Cable Diagnostic Control Register
    CDCR = 0x1e,

    // (RW) - PHY Reset Control Register
    PHYRCR = 0x1f,
};

// PHY status bits.  These map directly to IEEE PHYSTS
// (0x10) register bits
enum {
    STATUS_MDI_X_MODE = BIT14, // MDI-X mode
    STATUS_RECV_ERR   = BIT13, // Receive error latch
    STATUS_POLA       = BIT12, // Polarity
    STATUS_FALSE_CARR = BIT11, // False carrier latch
    STATUS_SIG_DET    = BIT10, // Signal detect
    STATUS_DES_LOCK   = BIT9,  // Descramble lock
    STATUS_PAGE_RECV  = BIT8,  // Page receive
    STATUS_MII_INTR   = BIT7,  // MII interrupt
    STATUS_REM_FAULT  = BIT6,  // Remote fault
    STATUS_JABBER     = BIT5,  // Jabber detect
    STATUS_AN         = BIT4,  // Auto negotiation status
    STATUS_LB         = BIT3,  // Loopback status
    STATUS_FD         = BIT2,  // Full/half duplex status
    STATUS_100M       = BIT1,  // Speed 100M/10M (1/0)
    STATUS_LINK       = BIT0   // Link status
};

};  // ns enet_phy

#endif // _ENET_PHY_H_

