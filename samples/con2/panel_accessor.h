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
    static void StartCommand(uint16_t cmd) {
        _lcd_cs.Raise();
        *(volatile uint16_t*)FSMC_PANEL_CMD = cmd;
    }

    [[__finline, __optimize]]
    static void EndCommand() { 
        _lcd_cs.Lower();
    }

    [[__finline, __optimize]]
    static void Write(uint16_t data) {
        *(volatile uint16_t*)FSMC_PANEL_DATA = data;
    }

    [[__finline, __optimize]]
    static uint16_t Read() {
        return *(volatile uint16_t*)FSMC_PANEL_DATA;
    }
};

#endif //  __PANEL_ACCESSOR_H__
