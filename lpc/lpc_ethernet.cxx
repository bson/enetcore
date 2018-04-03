// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"
#include "network.h"
#include "enet_phy.h"
#include "lpc_ethernet.h"
#include "util.h"

using namespace enet_phy;

// Broadcast address
const uint16_t Ethernet::_bcastaddr[3] = { 0xffff, 0xffff, 0xffff };

void Ethernet::Reset() {
    IPL G(IPL_ENET-1);

    _txdesc = NULL;
    _txstatus = NULL;
    _rxdesc = NULL;
    _rxstatus = NULL;
    _eventob = NULL;
    _phy_status = 0;

    _rx_overrun = 0;
    _rx_error = 0;
    _rx_packets = 0;
    _tx_underrun = 0;
    _tx_error = 0;
    _tx_packets = 0;
    _tx_bytes = 0;
    _rx_bytes = 0;

    _base[REG_COMMAND] = COMMAND_REGRESET | COMMAND_TXRESET | COMMAND_RXRESET;
    _base[REG_MAC1] = MAC1_SOFTRESET;

    // Device is now held in soft reset and can be re-initialized
}

void Ethernet::Initialize(const uint8_t macaddr[6]) {
    memcpy(_macaddr, macaddr, sizeof _macaddr);

    IPL G(IPL_ENET-1);

    EnetPHY::PreConf(*this);

    // Expected to be held in soft reset and pins have been configured
    // elsewhere.
    
    // After reset, the Ethernet software driver needs to initialize
    // the Ethernet block. During initialization the software needs
    // to:
    //
    // • Remove the soft reset condition from the MAC.
    //
    // Remark: it is important to configure the PHY and insure that
    // reference clocks (ENET_REF_CLK signal in RMII mode, or both
    // ENET_RX_CLK and ENET_TX_CLK signals in MII mode) are present at
    // the external pins and connected to the EMAC module (selecting
    // the appropriate pins using the IOCON registers) prior to
    // continuing with Ethernet configuration. Otherwise the CPU can
    // become locked and no further functionality will be
    // possible. This will cause JTAG lose communication with the
    // target, if debug mode is being used.
    //
    // • Select MII or RMII mode
	//
    // • Configure the transmit and receive DMA engines, including the
    //   descriptor arrays.
    //
    // • Configure the host registers (MAC1,MAC2 etc.) in the MAC.
    // • Enable the receive and transmit data paths.
    //

    // Set MDIO clock to 120M/48 = 2.5MHz
    _base[REG_MCFG] = 0b1011 * MCFG_CLOCKSEL;

    // Bring controller out of soft reset
    _base[REG_MAC1] = 0;

    // Check we have the right PHY and it's talking to us
    if (!EnetPHY::CheckPHY(*this)) {
        DMSG("PHY detection failed");
        return;
    }

    EnetPHY::Configure(*this);

    // Enable RMII
    _base[REG_COMMAND] = COMMAND_RMII;

    // Set station address
    SetMacAddr();

    // Initialize descriptors
    CreateDescriptors();

    // Set up RX filtering for broadcast and perfect unicast 
    _base[REG_RXFILTERCTRL] = RXFILTERCTRL_ABE | RXFILTERCTRL_APE;

#ifdef SOFT_PHY_ADDR
    // AUE - for soft ADDR filtering
    _base[REG_RXFILTERCTRL] |= RXFILTERCTRL_AUE;
#endif

    // Enable FD, pad to 64 bytes, enable CRC generation
    // Enable fast retransmit
    _base[REG_MAC2] = MAC2_FULLDUPLEX | MAC2_CRCEN | MAC2_PADCRCEN | MAC2_VLANPADEN
        | MAC2_NOBACKOFF;
    
    // Enable interrupts except SOFTINTEN and WAKEUPINTEN
    _base[REG_INTENABLE] = INTENABLE_RXOVERRUNINTEN | INTENABLE_RXERRORINTEN
        | INTENABLE_RXFINISHEDINTEN | INTENABLE_RXDONEINTEN | INTENABLE_TXUNDERRUNINTEN
        | INTENABLE_TXERRORINTEN | INTENABLE_TXFINISHEDINTEN | INTENABLE_TXDONEINTEN;

    // Start
    _base[REG_MAC1] = MAC1_RXENABLE;
    _base[REG_COMMAND] = COMMAND_RXENABLE | COMMAND_TXENABLE | COMMAND_RMII;

    EnetPHY::PostConf(*this);

    // Clear all interrupts
    _base[REG_INTCLEAR] = 0xffff;

    _phy_status = ReadPHY(PhyReg::PHYSTS);

    DMSG("ENET initialized");
}

void Ethernet::SetMacAddr() {
    _base[REG_SA0] = (uint16_t(_macaddr[0]) << 8) | _macaddr[1];
    _base[REG_SA1] = (uint16_t(_macaddr[2]) << 8) | _macaddr[3];
    _base[REG_SA2] = (uint16_t(_macaddr[4]) << 8) | _macaddr[5];
}

void Ethernet::WritePHY(PhyReg reg, uint16_t value) {
    _base[REG_MCMD] = 0;
    _base[REG_MADR] = 1 * MADR_PHYADDR + uint32_t(reg) * MADR_REGADDR;

    _base[REG_MWDT] = value;
    
    while (_base[REG_MIND] & MIND_BUSY)
        ;
}

uint16_t Ethernet::ReadPHY(PhyReg reg) {
    _base[REG_MCMD] = MCMD_READ;
    _base[REG_MADR] = 1 * MADR_PHYADDR + uint32_t(reg) * MADR_REGADDR;

    while (_base[REG_MIND] & MIND_BUSY)
        ;

    _base[REG_MCMD] = 0;
    return _base[REG_MRDD];
}

void Ethernet::WriteXPHY(EnetPHY::XPhyReg reg, uint16_t value) {
    // Set address
    WritePHY(PhyReg::REGCR, 0x001f);
    WritePHY(PhyReg::ADDAR, uint16_t(reg));

    // Write value
    WritePHY(PhyReg::REGCR, 0b01000000 | 0x1f); // No inc.
    WritePHY(PhyReg::ADDAR, uint16_t(value));
}

uint16_t Ethernet::ReadXPHY(EnetPHY::XPhyReg reg) {
    // Set address
    WritePHY(enet_phy::PhyReg::REGCR, 0x001f);
    WritePHY(enet_phy::PhyReg::ADDAR, uint16_t(reg));

    // Read
    WritePHY(enet_phy::PhyReg::REGCR, 0b01000000 | 0x1f); // No inc.
    return ReadPHY(enet_phy::PhyReg::ADDAR);
}

void Ethernet::CreateDescriptors() {
    _txdesc = (TxDesc*)AllocNetworkData(TX_DESC_NUM * sizeof (TxDesc), 8);
    _rxdesc = (RxDesc*)AllocNetworkData(RX_DESC_NUM * sizeof (RxDesc), 8);
    _rxstatus = (RxStatus*)AllocNetworkData(RX_DESC_NUM * sizeof (RxStatus), 8);
    _txstatus = (uint32_t*)AllocNetworkData(TX_DESC_NUM * sizeof (uint32_t), 8);

    memset(_txdesc, 0, TX_DESC_NUM * sizeof (TxDesc));
    memset(_rxdesc, 0, RX_DESC_NUM * sizeof (RxDesc));
    memset(_txstatus, 0, TX_DESC_NUM * sizeof (uint32_t));
    memset(_rxstatus, 0, RX_DESC_NUM * sizeof (RxStatus));

    _base[REG_TXDESCRIPTOR]       = (uint32_t)_txdesc;
    _base[REG_TXDESCRIPTORNUMBER] = TX_DESC_NUM - 1;

    _base[REG_RXDESCRIPTOR]       = (uint32_t)_rxdesc;
    _base[REG_RXDESCRIPTORNUMBER] = RX_DESC_NUM - 1;

    _base[REG_TXSTATUS]     = (uint32_t)_txstatus;
    _base[REG_RXSTATUS]     = (uint32_t)_rxstatus;

    _txbuffers = (IOBuffer**)xmalloc(sizeof (IOBuffer*) * TX_DESC_NUM);
    memset(_txbuffers, 0, sizeof (IOBuffer*) * TX_DESC_NUM);

    _rxbuffers = (IOBuffer**)xmalloc(sizeof (IOBuffer*) * RX_DESC_NUM);

    for (uint i = 0; i < RX_DESC_NUM; ++i) {
        IOBuffer *buf;
        if (!(buf = BufferPool::AllocRx())) {
            DMSG("Failed to allocate RX buffers");
            abort();
        }
        _rxbuffers[i] = buf;

        buf->SetHead(2);
        _rxdesc[i].packet = (uint8_t*)(*buf + 0);
        _rxdesc[i].control = (RxDesc::CONTROL_Size * (buf->GetReserve() - 2 - 1))
            | RxDesc::CONTROL_Interrupt;
    }
}

IOBuffer* Ethernet::Receive(uint16_t& et) {
    IPL G(IPL_ENET-1);

    const uint i = _base[REG_RXCONSUMEINDEX];

    // Empty: nothing to see here
    if (i == _base[REG_RXPRODUCEINDEX])
        return NULL;

    RxDesc* r = _rxdesc + i;

    // Use the length to indicate we've processed this buffer.
    // Otherwise, if we fail to allocate a replacement buffer we'll
    // end up receiving the same one over and over because we'll never
    // advance the consumer.
    if (!(r->control & 0x3ff) || !_rxbuffers[i])
        return NULL;

    const uint len = r->control & 0x3ff;

    r->control = 0;

    IOBuffer* buf = exch<IOBuffer*>(_rxbuffers[i], NULL);
    buf->SetHead(2);
    buf->SetTail(len + 2);

    _rx_bytes += len;

    RestockRx();

    // Extract ethertype
    const Frame* f = (const Frame*)(*buf + 0);
    et = Ntohs(f->et);

    return buf;
}

void Ethernet::RestockRx() {
    uint i;
    do {
        i = _base[REG_RXCONSUMEINDEX];

        RxDesc* r = _rxdesc + i;

        // Stop if not yet processed
        if (r->control)
            break;

        IOBuffer *buf;

        if ((buf = BufferPool::AllocRx())) {
            buf->SetHead(2);

            _rxdesc[i].packet = (uint8_t*)(*buf + 0);
            _rxdesc[i].control = (RxDesc::CONTROL_Size * (buf->GetReserve() - 2))
                | RxDesc::CONTROL_Interrupt;

            _rxbuffers[i] = buf;

            _base[REG_RXCONSUMEINDEX] = (i + 1) % RX_DESC_NUM;
        } else {
            DMSG("Failed to allocate RX buffer");
            break;
        }
    } while (((i + 1) % RX_DESC_NUM) != _base[REG_RXPRODUCEINDEX]);
}

bool Ethernet::Send(IOBuffer* buf) {
    IPL G(IPL_ENET-1);

    const uint current = _base[REG_TXPRODUCEINDEX];
    const uint next = (current + 1) % TX_DESC_NUM;

    // Refuse if full or link is down
    if (next == _base[REG_TXCONSUMEINDEX] || !(_phy_status & STATUS_LINK)) {
        BufferPool::FreeBuffer(buf);
        return false;
    }

    const uint len = buf->Size() - 2;
    if (len > 1514) {
        BufferPool::FreeBuffer(buf);
        return false;
    }

    enum {
        TXBITS = (TxDesc::CONTROL_Interrupt + TxDesc::CONTROL_Pad + TxDesc::CONTROL_CRC
                  + TxDesc::CONTROL_Last + TxDesc::CONTROL_Override)
    };

    TxDesc* t = _txdesc + current;

    // If there's an unprocessed packet, emergency reclaim it
    if (t->packet && _txbuffers[next]) {
        DMSG("NOTE: Reclaiming unprocessed IOBuffer!");
        t->packet = NULL;
        BufferPool::FreeBuffer(_txbuffers[next]);
        ++_tx_packets;
        _tx_bytes += t->control & 0x3ff;
    }

    _txbuffers[current] = buf;

    buf->SetHead(2);

    t->packet  = (const uint8_t*)(*buf + 0);
    t->control = buf->Size() * TxDesc::CONTROL_Size + TXBITS;

    // Ship it!
    _base[REG_TXPRODUCEINDEX] = next;

    return true;
}

void Ethernet::FillForBcast(IOBuffer* buf, uint16_t et) {
    const uint hlen = GetAddrLen();
    buf->SetHead(2);

    Frame* f = (Frame*)(*buf + 0);
	memcpy(f->dst, GetBcastAddr(), hlen);
	memcpy(f->src, GetMacAddr(), hlen);
	f->et = Htons(et);
}

void Ethernet::ServiceMISR() {
    _misr_atten = false;
}

void Ethernet::HandleMISRInterrupt() {
    // Read regs to clear interrupt status
    _misr1 = ReadPHY(PhyReg::MISR1);
    _misr2 = ReadPHY(PhyReg::MISR2);

    _phy_status = ReadPHY(PhyReg::PHYSTS);
    _bmsr = ReadPHY(PhyReg::BMSR);

    _misr_atten = true;
    if (_eventob)
        _eventob->Set(true);
}

void Ethernet::HandleInterrupt() {
    bool sig = false;
    uint32_t intbits = _base[REG_INTSTATUS];
    while (intbits) {
        // Pull out next interrupt
        const uint32_t i = intbits & ~(intbits - 1);
        intbits -= i;

        switch (i) {
        case INTSTATUS_RXOVERRUNINT:
            ++_rx_overrun;
            break;

        case INTSTATUS_RXERRORINT:
            ++_rx_error;
            break;

        case INTSTATUS_RXFINISHEDINT:
            break;

        case INTSTATUS_RXDONEINT:
            ++_rx_packets;
            sig = true;
            break;

        case INTSTATUS_TXUNDERRUNINT:
            ++_tx_underrun;
            break;

        case INTSTATUS_TXERRORINT:
            ++_tx_error;
            break;

        case INTSTATUS_TXFINISHEDINT:
            break;

        case INTSTATUS_TXDONEINT:{
            uint i = (_base[REG_TXCONSUMEINDEX] - 1) % TX_DESC_NUM;

            // Walk backwards to the producer and process sent packets
            while (i != _base[REG_TXPRODUCEINDEX] && _txdesc[i].packet != NULL) {
                _tx_bytes += _txdesc[i].control & 0x3ff;

                IOBuffer* buf = _txbuffers[i];
                BufferPool::FreeBuffer(buf);

                _txdesc[i].packet = NULL;
                _txbuffers[i] = NULL;

                ++_tx_packets;

                i = (i - 1) % TX_DESC_NUM;
                sig = true;
            }
            break;
        }
        case INTSTATUS_SOFTINT:
            break;

        case INTSTATUS_WAKEUPINT:
            break;

        default:
            break;
        }

        _base[REG_INTCLEAR] = i;
    }
    
    if (_eventob && sig)
        _eventob->Set(true);
}

void Ethernet::Interrupt(void* token) {
    if (!token)
        return;

    Ethernet* e = (Ethernet*)token;
    e->HandleInterrupt();
}
