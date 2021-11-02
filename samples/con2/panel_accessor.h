#ifndef __PANEL_ACCESSOR_H__
#define  __PANEL_ACCESSOR_H__

extern PinNegOutput<Gpio::Pin> _lcd_cs;

class PanelAccessor {
public:
    [[__finline]] static void Init() {
        Deselect();
    }

    [[__finline, __optimize]]
    static void Select() {
        _lcd_cs.Raise();
    }

    [[__finline, __optimize]]
    static void Deselect() {
        _lcd_cs.Lower();
    }

    [[__finline, __optimize]]
    static void StartCommand(uint8_t byte) {
        *(volatile uint16_t*)FSMC_PANEL_CMD = byte;
    }

    [[__finline, __optimize]]
    static void EndCommand() { }

    [[__finline, __optimize]]
    static void Write(uint8_t byte) {
        *(volatile uint16_t*)FSMC_PANEL_DATA = byte;
    }

    [[__finline, __optimize]]
    static uint8_t Read() {
        return *(volatile uint16_t*)FSMC_PANEL_DATA;
    }
};

#endif //  __PANEL_ACCESSOR_H__