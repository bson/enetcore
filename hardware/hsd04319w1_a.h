Make UART use Ring<> (see ring.h) instead of Deque<>.  It's much lower
overhead and less contention.  Just block threads if it's full.


Refactor chip support
- Move LPC in under nxp/lpc407x and nxp/lpc (common)
- Move stuff under arm into arm/cm4, cm0, cm7, etc
- Much of what's in board.h (e.g. skyblue/board.h) belongs in the application config.h
- Make more hardware support optional (e.g. SD, FAT)
- Make DHCP support a compile time option

- Move application out of enetcore tree
- Make enetcore a submodule of application
#ifndef _HSD04319W1_A_H_
#define _HSD04319W1_A_H_

namespace PanelInfo {

// Panel timing
enum {
    // Pixel clock
    PXCLOCK_MIN = 5000000,
    PXCLOCK_TYP = 9000000,  // 9MHz
    PXCLOCK_MAX = 12000000,

    // 18/24 bit bus (1=24, 0=18
    BUS_24 = 1,                 // 24 bit bus

    // TFT/Serial (0=TFT, 1=Serial)
    BUS_SERIAL = 0,

    // Vertical timing
    VERT_VISIBLE = 272,
    VERT_FRONT_PORCH = 8,
    VERT_BACK_PORCH = 8-1,  // Datasheet includes VSYNC in porch
    VSYNC_WIDTH = 1,
    VSYNC_MOVE = 0,

    // Horizontal timing
    HOR_VISIBLE = 480,
    HOR_FRONT_PORCH = 5,
    HOR_BACK_PORCH = 40-8,  // Datasheet includes HSYNC in porch
    HSYNC_WIDTH = 8,
    HSYNC_MOVE = 0,
    HSYNC_SUBPIXEL_POS = 0
};

};
#endif // _HSD04319W1_A_H_
