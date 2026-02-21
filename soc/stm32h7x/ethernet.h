// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

#ifndef _STM32_ETHERNET_H_
#define _STM32_ETHERNET_H_

#include <stdint.h>
#include "network.h"
#include "bits.h"
#include "compiler.h"
#include "mutex.h"
#include "eintr.h"


#error Not yet updated for STM32H7

#define SOFT_PHY_ADDR

// Signals state changes for the network thread via _net_event
// (declared in network.h).

class Stm32Ethernet {
    volatile uint32_t* _base;
    EventObject* _eventob;      // Used to signal events

    // Counters
    uint32_t _rx_overrun;
    uint32_t _rx_error;
    uint32_t _rx_packets;       // Packets received
    uint32_t _tx_underrun;
    uint32_t _tx_error;
    uint32_t _tx_packets;       // Packets sent
    uint32_t _rx_bytes;
    uint32_t _tx_bytes;

    volatile uint16_t _phy_status; // PHY status PHY_STAT_xxx
    volatile uint16_t _misr1;      // MISR1 register
    volatile uint16_t _misr2;      // MISR2 register
    volatile uint16_t _bmsr;       // BMSR register

    uint8_t  _macaddr[6];
	static const uint16_t _bcastaddr[3];     // Broadcast address

    const uint8_t _irq;
    bool _misr_atten;           // MISR attention requested via interrupt

    // Frame header
    struct Frame {
        uint8_t dst[6];
        uint8_t src[6];
        uint16_t et;
    };

    // Ethernet controller registers
    enum {
        //////// MAC Registers

        // R/W - MAC configuration register 1.
        REG_MAC1 = 0x000/4,

        // R/W - MAC configuration register 2.
        REG_MAC2 = 0x004/4,

        // R/W - Back-to-Back Inter-Packet-Gap register.
        REG_IPGT = 0x008/4,

        // R/W - Non Back-to-Back Inter-Packet-Gap register. 
        REG_IPGR = 0x00c/4,

        // R/W - Collision window / Retry register.
        REG_CLRT = 0x010/4,

        // R/W - Maximum Frame register.
        REG_MAXF = 0x014/4,

        // R/W - PHY Support register.
        REG_SUPP = 0x018/4,

        // R/W - Test register.
        REG_TEST = 0x01c/4,

        // R/W - MII Mgmt Configuration register.
        REG_MCFG = 0x020/4,

        // R/W - MII Mgmt Command register.
        REG_MCMD = 0x024/4,

        // R/W - MII Mgmt Address register.
        REG_MADR = 0x028/4,

        // WO - MII Mgmt Write Data register.
        REG_MWDT = 0x02c/4,

        // RO - MII Mgmt Read Data register.
        REG_MRDD = 0x030/4,

        // RO - MII Mgmt Indicators register.
        REG_MIND = 0x034/4,

        // R/W - Station Address 0 register.
        REG_SA0 = 0x040/4,

        // R/W - Station Address 1 register.
        REG_SA1 = 0x044/4,

        // R/W - Station Address 2 register.
        REG_SA2 = 0x048/4,


        /////// Control Registers

        // R/W - Command register.
        REG_COMMAND = 0x100/4,

        // RO - Status register.
        REG_STATUS = 0x104/4,

        // R/W - Receive descriptor base address register.        
        REG_RXDESCRIPTOR = 0x108/4,

		// R/W - Receive status base address register.
		REG_RXSTATUS = 0x10c/4,

		// R/W - Receive number of descriptors register.
		REG_RXDESCRIPTORNUMBER = 0x110/4,

		// RO - Receive produce index register.
		REG_RXPRODUCEINDEX = 0x114/4,

		// R/W - Receive consume index register.
		REG_RXCONSUMEINDEX = 0x118/4,

		// R/W - Transmit descriptor base address register.
		REG_TXDESCRIPTOR = 0x11c/4,

		// R/W - Transmit status base address register.
		REG_TXSTATUS = 0x120/4,

		// R/W - Transmit number of descriptors register.
		REG_TXDESCRIPTORNUMBER = 0x124/4,

		// R/W - Transmit produce index register.
		REG_TXPRODUCEINDEX = 0x128/4,

		// RO - Transmit consume index register.
		REG_TXCONSUMEINDEX = 0x12c/4,

		// RO - Transmit status vector 0 register.
		REG_TSV0 = 0x158/4,

		// RO - Transmit status vector 1 register.
		REG_TSV1 = 0x15c/4,

		// RO - Receive status vector register.
		REG_RSV = 0x160/4,

		// R/W - Flow control counter register.
		REG_FLOWCONTROLCOUNTER = 0x170/4,

		// RO - Flow control status register.
		REG_FLOWCONTROLSTATUS = 0x174/4,


        ///////// Rx filter registers

		// R/W - Receive filter control register.
		REG_RXFILTERCTRL = 0x200/4,

		// RO - Receive filter WoL status register.
		REG_RXFILTERWOLSTATUS = 0x204/4,

		// WO - Receive filter WoL clear register.
		REG_RXFILTERWOLCLEAR = 0x208/4,

		// R/W - Hash filter table LSBs register.
		REG_HASHFILTERL = 0x210/4,

		// R/W - Hash filter table MSBs register.
		REG_HASHFILTERH = 0x214/4,


        //////// Module control registers

		// RO - Interrupt status register.
		REG_INTSTATUS = 0xfe0/4,

		// R/W - Interrupt enable register.
		REG_INTENABLE = 0xfe4/4,

		// WO - Interrupt clear register.
		REG_INTCLEAR = 0xfe8/4,

		// WO - Interrupt set register.
		REG_INTSET = 0xfec/4,

		// R/W - Power-down register.
		REG_POWERDOWN = 0xff4/4
    };

    // Flags and bits used. (Incomplete.)
    enum {
        MAC1_RXENABLE   = BIT0,
        MAC1_PARF       = BIT1,
        MAC1_RXFLOWCTRL = BIT2,
        MAC1_TXFLOWCTRL = BIT3,
        MAC1_LOOPBACK   = BIT4,
        MAC1_RESETTX    = BIT8,
        MAC1_RESETMCSTX = BIT9,
        MAC1_RESETRX    = BIT10,
        MAC1_RESETMCSRX = BIT11,
        MAC1_SIMRESET   = BIT14,
        MAC1_SOFTRESET  = BIT15,

        MAC2_FULLDUPLEX = BIT0,
        MAC2_FLC        = BIT1,
        MAC2_HFEN       = BIT2,
        MAC2_DELAYEDCRC = BIT3,
        MAC2_CRCEN      = BIT4,
        MAC2_PADCRCEN   = BIT5,
        MAC2_VLANPADEN  = BIT6,
        MAC2_AUTODETPADEN = BIT7,
        MAC2_PPENF      = BIT8,
        MAC2_LPENF      = BIT9,
        MAC2_NOBACKOFF  = BIT12,
        MAC2_BP_NOBACKOFF = BIT13,
        MAC2_EXCESSDEFER = BIT14,
        
        IPGR_NBTOBINTEGAP2 = BIT0,
        IPGR_NBTOBINTEGAP1 = BIT8,
        
        CLRT_RETRANSMAX = BIT0,
        CLRT_COLLWIN     = BIT8,

        SUPP_SPEED      = BIT8,

        TEST_SCPQ = BIT0,
        TEST_TESTPAUSE = BIT1,
        TEST_TESTBP = BIT2,
        
        MCFG_SCANINC = BIT0,
        MCFG_SUPPPREAMBLE = BIT1,
        MCFG_CLOCKSEL = BIT2,
        MCFG_RESETMIIMGMT = BIT15,

        MCMD_READ = BIT0,
        MCMD_SCAN = BIT1,

        MADR_REGADDR = BIT0,
        MADR_PHYADDR = BIT8,

        MIND_BUSY = BIT0,
        MIND_SCANNING = BIT1,
        MIND_NOTVALID = BIT2,
        MIND_MIILINKFAIL = BIT3,

        SA0_SADDR2 = BIT0,
        SA0_SADDR1 = BIT8,

        SA1_SADDR4 = BIT0,
        SA1_SADDR3 = BIT8,

        SA2_SADDR6 = BIT0,
        SA2_SADDR5 = BIT8,

        COMMAND_RXENABLE = BIT0,
        COMMAND_TXENABLE = BIT1,
        COMMAND_REGRESET = BIT3,
        COMMAND_TXRESET = BIT4,
        COMMAND_RXRESET = BIT5,
        COMMAND_PASSRUNTFRAME = BIT6,
        COMMAND_PASSRXFILTER = BIT7,
        COMMAND_TXFLOWCONTROL = BIT8,
        COMMAND_RMII = BIT9,
        COMMAND_FULLDUPLEX = BIT10,

        STATUS_RXSTATUS = BIT0,
        STATUS_TXSTATUS = BIT1,

        TSV0_CRCERR = BIT0,
        TSV0_LCE = BIT1,
        TSV0_LOR = BIT2,
        TSV0_DONE = BIT3,
        TSV0_MULTICAST = BIT4,
        TSV0_BROADCAST = BIT5,
        TSV0_PACKETDEFER = BIT6,
        TSV0_EXDF = BIT7,
        TSV0_EXCOL = BIT8,
        TSV0_LCOL = BIT9,
        TSV0_GIANT = BIT10,
        TSV0_UNDERRUN = BIT11,
        TSV0_TOTALBYTES = BIT12,
        TSV0_CONTROLFRAME = BIT28,
        TSV0_PAUSE = BIT29,
        TSV0_BACKPRESSURE = BIT30,
        TSV0_VLAN = BIT31,
        
        TSV1_TBC = BIT0,
        TSV1_TCC = BIT16,

        RSV_RBC = BIT0,
        RSV_PPI = BIT16,
        RSV_RXDVSEEN = BIT17,
        RSV_CESEEN = BIT18,
        RSV_RCV = BIT19,
        RSV_CRCERR = BIT20,
        RSV_LCERR = BIT21,
        RSV_LOR = BIT22,
        RSV_ROK = BIT23,
        RSV_MULTICAST = BIT24,
        RSV_BROADCAST = BIT25,
        RSV_DRIBBLENIBBLE = BIT26,
        RSV_CONTROLFRAME = BIT27,
        RSV_PAUSE = BIT28,
        RSV_UO = BIT29,
        RSV_VLAN = BIT30,

        FLOWCONTROLCOUNTER_MC = BIT0,
        FLOWCONTROLCOUNTER_PT = BIT16,

        FLOWCONTROLSTATUS_MCC = BIT0,

        RXFILTERCTRL_AUE = BIT0,
        RXFILTERCTRL_ABE = BIT1,
        RXFILTERCTRL_AME = BIT2,
        RXFILTERCTRL_AUHE = BIT3,
        RXFILTERCTRL_AMHE = BIT4,
        RXFILTERCTRL_APE = BIT5,
        RXFILTERCTRL_MPEW = BIT12,
        RXFILTERCTRL_RFEW = BIT13,

        RXFILTERWOLSTATUS_AUW = BIT0,
        RXFILTERWOLSTATUS_ABW = BIT1,
        RXFILTERWOLSTATUS_AMW = BIT2,
        RXFILTERWOLSTATUS_AUHW = BIT3,
        RXFILTERWOLSTATUS_AMHW = BIT4,
        RXFILTERWOLSTATUS_APW = BIT5,
        RXFILTERWOLSTATUS_RFW = BIT7,
        RXFILTERWOLSTATUS_MPW = BIT8,

        RXFILTERWOLCLEAR_AUWCLR = BIT0,
        RXFILTERWOLCLEAR_ABWCLR = BIT1,
        RXFILTERWOLCLEAR_AMWCLR = BIT2,
        RXFILTERWOLCLEAR_AUHWCLR = BIT3,
        RXFILTERWOLCLEAR_AMHWCLR = BIT4,
        RXFILTERWOLCLEAR_APWCLR = BIT5,
        RXFILTERWOLCLEAR_RFWCLR = BIT7,
        RXFILTERWOLCLEAR_MPWCLR = BIT8,

        INTSTATUS_RXOVERRUNINT = BIT0,
        INTSTATUS_RXERRORINT = BIT1,
        INTSTATUS_RXFINISHEDINT = BIT2,
        INTSTATUS_RXDONEINT = BIT3,
        INTSTATUS_TXUNDERRUNINT = BIT4,
        INTSTATUS_TXERRORINT = BIT5,
        INTSTATUS_TXFINISHEDINT = BIT6,
        INTSTATUS_TXDONEINT = BIT7,
        INTSTATUS_SOFTINT = BIT12,
        INTSTATUS_WAKEUPINT = BIT13,

        INTENABLE_RXOVERRUNINTEN = BIT0,
        INTENABLE_RXERRORINTEN = BIT1,
        INTENABLE_RXFINISHEDINTEN = BIT2,
        INTENABLE_RXDONEINTEN = BIT3,
        INTENABLE_TXUNDERRUNINTEN = BIT4,
        INTENABLE_TXERRORINTEN = BIT5,
        INTENABLE_TXFINISHEDINTEN = BIT6,
        INTENABLE_TXDONEINTEN = BIT7,
        INTENABLE_SOFTINTEN = BIT12,
        INTENABLE_WAKEUPINTEN = BIT13,

        INTCLEAR_RXOVERRUNINTCLR = BIT0,
        INTCLEAR_RXERRORINTCLR = BIT1,
        INTCLEAR_RXFINISHEDINTCLR = BIT2,
        INTCLEAR_RXDONEINTCLR = BIT3,
        INTCLEAR_TXUNDERRUNINTCLR = BIT4,
        INTCLEAR_TXERRORINTCLR = BIT5,
        INTCLEAR_TXFINISHEDINTCLR = BIT6,
        INTCLEAR_TXDONEINTCLR = BIT7,
        INTCLEAR_SOFTINTCLR = BIT12,
        INTCLEAR_WAKEUPINTCLR = BIT13,

        INTSET_RXOVERRUNINTSET = BIT0,
        INTSET_RXERRORINTSET = BIT1,
        INTSET_RXFINISHEDINTSET = BIT2,
        INTSET_RXDONEINTSET = BIT3,
        INTSET_TXUNDERRUNINTSET = BIT4,
        INTSET_TXERRORINTSET = BIT5,
        INTSET_TXFINISHEDINTSET = BIT6,
        INTSET_TXDONEINTSET = BIT7,
        INTSET_SOFTINTSET = BIT12,
        INTSET_WAKEUPINTSET = BIT13,

        POWERDOWN_PD = BIT31
    };

    // Transmit descriptor
    struct __novtable TxDesc {
        const void* packet;           // Packet address
        uint32_t    control;          // Control word
        // Control bits
        enum {
            CONTROL_Size = BIT0,
            CONTROL_Override = BIT26,
            CONTROL_Huge = BIT27,
            CONTROL_Pad = BIT28,
            CONTROL_CRC = BIT29,
            CONTROL_Last = BIT30,
            CONTROL_Interrupt = BIT31
        };
    };

    // Transmit status word bits
    enum {
        TXSTATUS_CollisionCount = BIT21,
        TXSTATUS_Defer = BIT25,
        TXSTATUS_ExcessiveDefer = BIT26,
        TXSTATUS_ExcessiveCollision = BIT27,
        TXSTATUS_LateCollision = BIT28,
        TXSTATUS_Underrun = BIT29,
        TXSTATUS_NoDescriptor = BIT30,
        TXSTATUS_Error = BIT31
    };

    // Receive descriptor
    struct __novtable RxDesc {
        void*    packet;        // Packet address
        uint32_t control;       // Control word
        // Control bits
        enum {
            CONTROL_Size = BIT0,
            CONTROL_Interrupt = BIT31
        };
    };

    // Receive status.  Capitalization matches data sheet.
    struct __novtable RxStatus {
        uint32_t StatusInfo;
        uint32_t StatusHashCRC;

        // StatusHashCRC bits
        enum {
            StatusHashCRC_SAHashCRC = BIT0,
            StatusHashCRC_DAHashCRC = BIT16
        };

        // StatusInfo bits
        enum {
            StatusInfo_RxSize = BIT0,
            StatusInfo_ControlFrame = BIT18,
            StatusInfo_VLAN = BIT19,
            StatusInfo_FailFilter = BIT20,
            StatusInfo_Multicast = BIT21,
            StatusInfo_Broadcast = BIT22,
            StatusInfo_CRCError = BIT23,
            StatusInfo_SymbolError = BIT24,
            StatusInfo_LengthError = BIT25,
            StatusInfo_RangeError = BIT26,
            StatusInfo_AlignmentError = BIT27,
            StatusInfo_Overrun = BIT28,
            StatusInfo_NoDescriptor = BIT29,
            StatusInfo_LastFlag = BIT30,
            StatusInfo_Error = BIT31,
        };
    };

    TxDesc*    _txdesc;          // TX descriptors
    uint32_t*  _txstatus;        // TX statuses
    RxDesc*    _rxdesc;          // RX descriptors
    RxStatus*  _rxstatus;        // RX statuses
    IOBuffer** _rxbuffers;       // RX buffers
    IOBuffer** _txbuffers;       // TX buffers


public:
    enum {
        // 1526 plus a little extra to allow alignment to 32 bit
        // words.  We skip the first two bytes so IP the payload is
        // aligned.  This means we have 16 bytes for the frame header
        // instead of 14, with the first two bytes unused.
        // Ip::GetIph() does this to locate the header.
        MAX_FRAME_SIZE = 1532,

        // Number of receive descriptors.  Should be power of 2.
        RX_DESC_NUM = 4,

        // Number of transmit descriptors.  Should be power of 2.
        TX_DESC_NUM = 16
    };

    Ethernet(uintptr_t base, uint8_t irq)
        : _base((volatile uint32_t*)base), 
          _irq(irq) {
        ;
    }

    // Reset.  Performs a soft reset after which the controller
    // and PHY should be reinitialized.
    void Reset();

    // Initialize
    void Initialize(const uint8_t macaddr[6]);

    // Get link status
    bool GetLinkStatus() {
        // For some reason the PHY never interrupts on MISR changes, so poll
        _phy_status = ReadPHY(enet_phy::PhyReg::PHYSTS);
        _bmsr = ReadPHY(enet_phy::PhyReg::BMSR);
        return _phy_status & enet_phy::STATUS_LINK;
    }

    // Get MAC address
    const uint8_t* GetMacAddr() const { return _macaddr; }

	// Return amount to prealloc for header The ethernet header is 14
	// bytes, but we skip the first two bytes in the packet.  This
	// keeps any following headers 32-bit aligned.  Similarly on the
	// receive side.
	uint GetPrealloc() const { return sizeof(Frame) + 2; }

	// Address length
	static const uint GetAddrLen() { return sizeof _macaddr; }

	// Buffer pad
	static const uint GetBufPad() { return 2; }

	// BOOTP htype
	static const uint8_t GetBootpType() { return 1; }

	// Fill in a buffer for broadcast
	void FillForBcast(IOBuffer* buf, uint16_t et);

	// Get broadcast address
	static const uint8_t* GetBcastAddr() { return (const uint8_t*)_bcastaddr; }

    // Get next unreceived frame or NULL of nothing there.  'et'
    // gets set to the ethertype and the start of the buffer is
    // advanced past the station addresses.
    IOBuffer* Receive(uint16_t& et);

    // Send a frame.  Returns false if there was nowhere to put it.
    bool Send(IOBuffer* buf);
    
    // Return our max frame size for buffering.
    uint GetBufSize() { return MAX_FRAME_SIZE; }

    // Called on external interrupt from PHY, for things like link
    // status changes.  This will trigger an event notification with
    // MISRAtten() true; the service thread should call ServiceMISR();
    void HandleMISRInterrupt();

    static void MISRInterrupt(void* token) {
        if (!token)
            return;

        Ethernet* e = (Ethernet*)token;
        e->HandleMISRInterrupt();
    }

    bool MISRAtten() const { return _misr_atten; }
    uint16_t PHYStatus() { 
        _phy_status = ReadPHY(enet_phy::PhyReg::PHYSTS);
        return _phy_status;
    }

    uint16_t RxErrCount() {
        return ReadPHY(enet_phy::PhyReg::RECR);
    }

    void ServiceMISR();
    
    // Set event object to signal status changes on
    void SetEventObject(EventObject* evob) { _eventob = evob; }

    // Controller interrupt
    static void Interrupt(void* token);

    // Pin intr to MISR bridge.  T is something that has a virtual
    // HandleInterrupt().
    template <typename T>
    class InterruptBridge: public T {
        Ethernet& _receiver; // Interrupt receiver
    public:
        InterruptBridge(Ethernet& r, const T& src)
            : T(src), 
              _receiver(r)
        { }

        // * implements T::HandleInterrupt()
        void HandleInterrupt() {
            _receiver.HandleMISRInterrupt();
        }
    private:
        InterruptBridge(const InterruptBridge&);
        InterruptBridge& operator=(const InterruptBridge&);
    };

private:
    inline void HandleInterrupt();

public:

    // Write to a PHY register
    void WritePHY(enet_phy::PhyReg reg, uint16_t value);

    // Read from a PHY register
    uint16_t ReadPHY(enet_phy::PhyReg reg);

    // Write to an extended PHY register
    void WriteXPHY(EnetPHY::XPhyReg reg, uint16_t value);

    // Read from an extended PHY register
    uint16_t ReadXPHY(EnetPHY::XPhyReg reg);

private:
    // Set controller MAC Addr
    void SetMacAddr();

    // Create descriptor and status arrays
    void CreateDescriptors();

    // Fill up RX descriptors
    void RestockRx();
};

#endif // _LPC_ETHERNET_H_
