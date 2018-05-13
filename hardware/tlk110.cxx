#include "enetcore.h"
#include "enet_phy.h"
#include "tlk110.h"


namespace EnetPHY {

using namespace enet_phy;

// Probe for PHY, returns true if it's there and looks alright.
bool CheckPHY(Ethernet& eth) {
    const uint32_t phyidr1 = eth.ReadPHY(PhyReg::PHYIDR1);
    const uint32_t phyidr2 = eth.ReadPHY(PhyReg::PHYIDR2);

    const uint32_t oui = (phyidr1 << 6) | ((phyidr2 >> 10) & 0x3f);
    const uint8_t mdl  = (phyidr2 >> 4) & 0x3f;
    const uint8_t rev  = phyidr2 & 0xf;

    DMSG("ENET PHY: OUI=0x%x, MDL=%d, REV=%d", oui, mdl, rev);

    return oui == 0x080028;
}

// Configure
void Configure(Ethernet& eth) {
    // Set software strap bits
    const uint16_t sws = eth.ReadPHY(PhyReg::SWSCR1);
    const uint16_t sws3 = eth.ReadPHY(PhyReg::SWSCR3);
    // 0  - INT OE
    // 7  - enable link loss reconvery
    // 13 - AN
    // 11, 12 - 11 for 100BaseTX
    // 15 - config done
    // XX 10 - LED_CFG?
    eth.WritePHY(PhyReg::SWSCR1, sws | BIT7 | BIT13 | BIT11| BIT12 | BIT0);

    // Bits 0-3: fast link down detection
    eth.WritePHY(PhyReg::SWSCR3, sws3 | 0xf);

    eth.WritePHY(PhyReg::SWSCR1, sws | BIT7 | BIT13 | BIT11| BIT12 | BIT0 | BIT15);
    
    // Reset to activate software strap
    eth.WritePHY(PhyReg::BMCR, BIT15);
    
    while (eth.ReadPHY(PhyReg::BMCR) & BIT15)
        ;

        // Check that PHY is configured to RMII
    if (!(eth.ReadPHY(PhyReg::RCSR) & BIT5)) {
        console("ERROR: PHY is not operating in RMII mode");
        // XXX is it worth hard disabling it? should tell the controller code though,
        //     maybe through eth.Error() or such.
        return;
    }

    // PHY specific
    // 3 - INT_POL = H inactive, L active
    // 1 - INT enable
    // 0 - INT output enable
    eth.WritePHY(PhyReg::PHYSCR, BIT3 | BIT1 | BIT0);

    // 8 - full dupliex, 13 - 100M, 12 - AN
    eth.WritePHY(PhyReg::BMCR, BIT8 | BIT13 | BIT12);
}

// Post-config initialization
void PostConf(Ethernet& eth) {
    // Read registers to clear status bits
    eth.ReadPHY(PhyReg::PHYSTS);
    eth.ReadPHY(PhyReg::MISR1);
    eth.ReadPHY(PhyReg::MISR2);
}

};
