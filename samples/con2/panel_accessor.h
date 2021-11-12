#ifndef __PANEL_ACCESSOR_H__
#define __PANEL_ACCESSOR_H__

extern PinNegOutput<Gpio::Pin> _lcd_cs;
extern PinNegOutput<Gpio::Pin> _panel_bl;

class PanelAccessor {
public:
    [[__finline]] static void Init() {
        _lcd_cs.Lower();
        _panel_bl.Raise();
    }

    [[__finline, __optimize]]
    static void StartCommand(uint8_t byte) {
        _lcd_cs.Raise();
        *(volatile uint16_t*)FSMC_PANEL_CMD = byte;
        _lcd_cs.Lower();
    }

    [[__finline, __optimize]]
    static void EndCommand() { 
    }

    [[__finline, __optimize]]
    static void Write(uint8_t byte) {
        _lcd_cs.Raise();
        *(volatile uint16_t*)FSMC_PANEL_DATA = byte;
        _lcd_cs.Lower();
    }

    [[__finline, __optimize]]
    static uint8_t Read() {
        _lcd_cs.Raise();
        return *(volatile uint16_t*)FSMC_PANEL_DATA;
        _lcd_cs.Lower();
    }
};

#endif //  __PANEL_ACCESSOR_H__
