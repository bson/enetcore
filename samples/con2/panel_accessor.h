#ifndef __PANEL_ACCESSOR_H__
#define  __PANEL_ACCESSOR_H__

class PanelAccessor {
public:
    [[__finline]] static void Init() { }

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
