#ifndef __ACCESSOR_H__
#define __ACCESSOR_H__

// DPORT is the data port
// CTLPORT is the control port
// Operates in 8080 mode
template <Gpio& _DPORT,
          Gpio& _CPORT,
          uint32_t _CTL_CS,
          uint32_t _CTL_WR,
          uint32_t _CTL_RD,
          uint32_t _CTL_RS>
class BitBangAccessor {
    enum {
        MASK_CS = _CTL_CS,
        MASK_WR = _CTL_WR,
        MASK_RD = _CTL_RD,
        MASK_RS = _CTL_RS,
        MASK_CTL = MASK_CS | MASK_WR | MASK_RD | MASK_CS
    };

public:
    static void Init() {
        _CPORT.MakeOutputs(MASK_CTL);
        _DPORT.MakeOutputs(0xff); // Output by default
    }

    [[__finline, __optimize]]
    static void StartCommand(uint8_t byte) {
        _CPORT.Set(MASK_RS | MASK_RD | MASK_WR | MASK_CS);
        _CPORT.Clear(MASK_CS | MASK_RS | MASK_WR); // CS active across command
        _DPORT.Output(0xff, cmd);                  // Data on bus
        _CPORT.Set(MASK_WR); // WR inactive - data is latched on this edge
        _CPORT.Set(MASK_RS); // Restore RS
    }

    [[__finline, __optimize]]
    static void EndCommand() {
        _CPORT.Set(MASK_CS | MASK_RD | MASK_WR | MASK_RS); // CS inactive
    }

    [[__finline, __optimize]]
    static void Write(uint8_t byte) {
        _CPORT.Clear(MASK_WR);  // WR active; RS = 1 = data
        _DPORT.Output(0xff, d); // Data on bus
        _CPORT.Set(MASK_WR); // WR inactive - data is latched on this edge
    }

    [[__finline, __optimize]]
    static uint8_t Read() {
        _DPORT.MakeInputs(0xff); // Input
        _CPORT.Set(MASK_RS);     // Data
        _CPORT.Clear(MASK_RD);
        const uint8_t tmp = _DPORT.Input() & 0xff;
        _CPORT.Set(MASK_RD);
        _DPORT.MakeOutputs(0xff); // Return to output
        return tmp;
    }
};

#endif // __ACCESSOR_H__
