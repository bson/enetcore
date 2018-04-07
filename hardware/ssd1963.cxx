#ifdef _INLINE_CXX_

#include <stdint.h>
#include "enetcore.h"
#include "ssd1963.h"
#include "font.h"
#include "font/runes.h"
#include "hsd04319w1_a.h"

namespace ssd1963 {

using namespace PanelInfo;

#define TEMPLATE \
template <Gpio& _DPORT, \
          Gpio& _CPORT, \
          uint32_t _CTL_CS, \
          uint32_t _CTL_WR, \
          uint32_t _CTL_RD, \
          uint32_t _CTL_RS> \

#define PARAMS \
_DPORT, _CPORT, _CTL_CS, _CTL_WR, _CTL_RD, _CTL_RS

TEMPLATE uint8_t ssd1963::Panel<PARAMS>::_r;
TEMPLATE uint8_t ssd1963::Panel<PARAMS>::_g;
TEMPLATE uint8_t ssd1963::Panel<PARAMS>::_b;
TEMPLATE uint8_t ssd1963::Panel<PARAMS>::_bg_r;
TEMPLATE uint8_t ssd1963::Panel<PARAMS>::_bg_g;
TEMPLATE uint8_t ssd1963::Panel<PARAMS>::_bg_b;

TEMPLATE
[[__optimize]]
void Panel<PARAMS>::Init() {

    _CPORT.MakeOutputs(MASK_CTL);
    _DPORT.MakeOutputs(0xff); // Output by default

    enum {
        XT_IN = 10000000ULL,    // 10MHz crystal
        LCD_A = 0b00000000 | (BUS_24 * BIT5),
        LCD_B = 0b00000000 | (BUS_SERIAL * BIT6),
        LCD_G = 0b00000000,  // R,G,B order
        PIXEL_CLOCK = 1ULL * PXCLOCK_TYP,
        PLL_M = 40ULL,
        PLL_N = 5ULL
    };

    // Sync Timing Config Parameters
    enum {
        // Vertical config parameters
        PARM_VDP = VERT_VISIBLE - 1L,
        PARM_VT  = (VERT_FRONT_PORCH + VERT_VISIBLE + VERT_BACK_PORCH + VSYNC_WIDTH) - 1L,
        PARM_VPW = VSYNC_WIDTH - 1L,
        PARM_VPS = VSYNC_MOVE + VSYNC_WIDTH + VERT_BACK_PORCH,
        PARM_FPS = VSYNC_MOVE,

        // Horizontal config parameters
        PARM_HDP = HOR_VISIBLE - 1L,
        PARM_HT  = (HOR_FRONT_PORCH + HOR_VISIBLE + HSYNC_WIDTH + HOR_BACK_PORCH) - 1L,
        PARM_HPW = HSYNC_WIDTH - 1L,
        PARM_HPS = HSYNC_MOVE + HSYNC_WIDTH + HOR_BACK_PORCH,
        PARM_LPS = HSYNC_MOVE,
        PARM_LPSPP = HSYNC_SUBPIXEL_POS
    };

    enum {
        VCO_FREQ = XT_IN * PLL_M,
        ADJCLK = uint64_t(PIXEL_CLOCK) << 20ULL,

        PARM_LSHIFT = uint64_t(ADJCLK * PLL_N) / VCO_FREQ - 1,
        REFRESH_RATE = uint64_t(PIXEL_CLOCK) / (uint64_t(PARM_HT) * uint64_t(PARM_VT))
    };

    wcommand(CMD_EXIT_SLEEP_MODE);
    Thread::Delay(5000);

    // Disable during init to avoid flickering
    wcommand(CMD_SET_DISPLAY_OFF);

    wcommand8(CMD_SET_PLL, 0);
    static const uint8_t pll[] = { PLL_M - 1, PLL_N - 1, 4 };
    wcommand_barr(CMD_SET_PLL_MN, 3, pll);

    wcommand8(CMD_SET_PLL, 0x01);   // Enable PLL
    Thread::Delay(100);
    wcommand8(CMD_SET_PLL, 0x03);

    wcommand(CMD_SOFT_RESET);
    Thread::Delay(5000);

#define PARM16(P) ((P) >> 8) & 0xff, (P) & 0xff

    static const uint8_t sequence[] = {
       CMD_ENTER_NORMAL_MODE, 0,
       CMD_EXIT_INVERT_MODE, 0,
       CMD_SET_TEAR_OFF, 0,
       CMD_SET_ADDRESS_MODE, 1, 0,   // T-B, L-R etc (b1=flip H, b0=flip V)
       CMD_SET_PIXEL_FORMAT, 1, 0b01110000, // 24bpp
       CMD_SET_LCD_MODE, 7,
           LCD_A, LCD_B, PARM16(PARM_HDP), PARM16(PARM_VDP), LCD_G,
       CMD_SET_HORI_PERIOD, 8,
           PARM16(PARM_HT), PARM16(PARM_HPS), PARM_HPW, PARM16(PARM_LPS), PARM_LPSPP,
       CMD_SET_VERT_PERIOD, 7,
           PARM16(PARM_VT), PARM16(PARM_VPS), PARM_VPW, PARM16(PARM_FPS),
       CMD_SET_LSHIFT_FREQ, 3,
           (PARM_LSHIFT >> 16) & 0xff, PARM16(PARM_LSHIFT),
       CMD_SET_PIXEL_DATA_INTERFACE, 1, 0b000, // 8 bit interface (3x 6 bit per pixel)
       CMD_SET_POST_PROC, 4, 0x40, 0x80, 0x40,1,  // Contrast, Brigthness, Saturation
       CMD_SET_DISPLAY_ON, 0
    };

    for (const uint8_t* p = sequence; p < sequence + sizeof sequence; ) {
        const uint8_t cmd = *p++;
        const uint8_t n   = *p++;
        if (n) {
            wcommand_barr(cmd, n, p);
            p += n;
        } else {
            wcommand(cmd);
        }
    }

    Clear();
}

TEMPLATE
void Panel<PARAMS>::Clear() {
    SetRgb(_bg_r, _bg_g, _bg_b);
    Fill(0, 0, HOR_VISIBLE, VERT_VISIBLE);
}

TEMPLATE
void Panel<PARAMS>::Fill(uint16_t col, uint16_t row, uint16_t w, uint16_t h) {

    set_window(col, row, w, h);

    command_start(CMD_WRITE_MEMORY_START);

#if defined(CORTEX_M)
    asm volatile ("                                                  \
    1:                                                               \
       str    %7, [%0];    /* _CPORT.CLR = MASK_WR */                \
       str    %3, [%2];    /* _DPORT.PIN = _r */                     \
       str    %7, [%1];    /* _CPORT.SET = MASK_WR */                \
       str    %7, [%0];    /* _CPORT.CLR = MASK_WR */                \
       str    %4, [%2];    /* _DPORT.PIN = _g */                     \
       str    %7, [%1];    /* _CPORT.SET = MASK_WR */                \
       str    %7, [%0];    /* _CPORT.CLR = MASK_WR */                \
       str    %5, [%2];    /* _DPORT.PIN = _b */                     \
       str    %7, [%1];    /* _CPORT.SET = MASK_WR */                \
       subs   %6, %6, #1;                                            \
       bne    1b                                                     \
        " : :     "r"(_CPORT.RegClr()),
                  "r"(_CPORT.RegSet()),
                  "r"(_DPORT.RegPin()),
                  "r"(_r),
                  "r"(_g),
                  "r"(_b),
                  "r"(w * h),
                  "r"(MASK_WR) : );
#else
    for (uint n = 0; n < w * h; ++n) {
        data(_r);
        data(_g);
        data(_b);
    }
#endif
    command_end();
    wcommand(CMD_NOP);
}

TEMPLATE
void Panel<PARAMS>::set_window(uint16_t col, uint16_t row, uint16_t w, uint16_t h) {

    command_start(CMD_SET_COLUMN_ADDRESS);
    data16(col);
    data16(col + w - 1);
    command_end();

    command_start(CMD_SET_PAGE_ADDRESS);
    data16(row);
    data16(row + h - 1);
    command_end();
}

TEMPLATE
void Panel<PARAMS>::HLine(uint16_t x, uint16_t y, uint16_t len, uint8_t w) {
    Fill(x, y, len, w);
}

TEMPLATE
void Panel<PARAMS>::VLine(uint16_t x, uint16_t y, uint16_t len, uint8_t w) {
    Fill(x, y, w, len);
}

TEMPLATE
void Panel<PARAMS>::Rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t s) {
    HLine(x, y, w, s);
    HLine(x, y + h - s, w, s);
    VLine(x, y, h, s);
    VLine(x + w - s, y, h, s);
}

TEMPLATE
void Panel<PARAMS>::Blit(uint16_t x, uint16_t y, const uint8_t* image, 
                         uint16_t w, uint16_t h)  {
    set_window(x, y, w, h);

    command_start(CMD_WRITE_MEMORY_START);

#if defined(CORTEX_M)
    // Asm args:
    // %0 - CPORT.CLR
    // %1 - CPORT.SET
    // %2 - DPORT.PIN
    // %3 - FG RGB
    // %4 - BG RGB
    // %5 - image
    // %6 - w*h
    // %7 - MASK_WR
    // %8 - tmp (image bits)
    // %9 - tmp (image bit counter)
    // %10 - tmp
    asm volatile (
#if CORTEX_M == 0
// CM0 doesn't support unaligned word loads, so load bytewise
"   1:                                                               \
       ldrb   %8, [%5], #1;                                          \
       mov    %9, #9;                                                \
"
#else
// CM4, CM7 do, so load in groups of 32 bits (yes, we may overrun at
// the end).
"   1:                                                               \
       ldr    %8, [%5], #4;                                          \
       mov    %9, #33;                                               \
"
#endif
"   2:                                                               \
       subs   %9, %9, #1;                                            \
       beq    1b;                                                    \
                                                                     \
       asrs   %8, %8, #1;                                            \
       movcs  %10, %3;      /* Set: use FG RGB */                    \
       movcc  %10, %4;      /* Clear: use BG RGB */                  \
                                                                     \
       str    %10, [%2];    /* _DPORT.PIN = r */                     \
       str    %7, [%0];     /* _CPORT.CLR = MASK_WR */               \
       lsr    %10, %10, #8;                                          \
       str    %7, [%1];     /* _CPORT.SET = MASK_WR */               \
                                                                     \
       str    %10, [%2];    /* _DPORT.PIN = g */                     \
       str    %7, [%0];     /* _CPORT.CLR = MASK_WR */               \
       lsr    %10, %10, #8;                                          \
       str    %7, [%1];     /* _CPORT.SET = MASK_WR */               \
                                                                     \
       str    %10, [%2];    /* _DPORT.PIN = b */                     \
       str    %7, [%0];     /* _CPORT.CLR = MASK_WR */               \
       subs   %6, %6, #1;                                            \
       str    %7, [%1];     /* _CPORT.SET = MASK_WR */               \
       bne    2b                                                     \
        " : :     "r"(_CPORT.RegClr()),
                  "r"(_CPORT.RegSet()),
                  "r"(_DPORT.RegPin()),
                  "r"((_b << 16) | (_g << 8) | _r),
                  "r"((_bg_b << 16) | (_bg_g << 8) | _bg_r),
                  "r"(image),
                  "r"(uint(w) * uint(h)),
                  "r"(MASK_WR),
                  "r"(0),
                  "r"(0),
                  "r"(0)
                  : );
#else
    uint8_t v = *image++;
    uint bits = 8;

    for (uint n = w * h; n > 0; --n) {
        if (v & 1) {
            data(_r);
            data(_g);
            data(_b);
        } else {
            data(_bg_r); 
            data(_bg_g);
            data(_bg_b);
        }
        if (!--bits) {
            v = *image++;
            bits = 8;
        } else {
            v >>= 1;
        }
    }
#endif

    command_end();
    wcommand(CMD_NOP);
}


TEMPLATE
[[__optimize]]
uint Panel<PARAMS>::Text(uint x, uint y, const Font& font, 
                         const String& s, uint8_t kern, bool vkern) {
    uint x0 = x;
    const uint w = font.GetWidth();
    const uint h = font.GetHeight();

    for (uint i = 0; i < s.Size(); ++i) {
        const uchar c = s[i];

        Blit(x, y, font.GetData(c), w, h);

        if (vkern && font.GetKern(c)) {
            x += w + kern - 1;
        } else {
            x += w + kern;
        }
        
        if (x > HOR_VISIBLE - w) {
            x = x0 = 0;
            y += h + kern * 2;
        }
    }

    return x - x0;
}


TEMPLATE
void Panel<PARAMS>::TestPattern() {

    // Test patterns to verify scan order, RGB mapping, and timing
    SetRgb(255, 0, 0);
    Fill(0, 0, 100, 100);

    SetRgb(0, 255, 0);
    Fill(75, 75, 100, 100);

    SetRgb(0, 0, 255);
    Fill(150, 150, 100, 100);

    SetRgb(0xff, 0xff, 0xff);
    Rect(0, 0, 480, 272, 1);
    Rect(2, 2, 476, 268, 1);
    Rect(4, 4, 472, 264, 1);
}

};

#undef TEMPLATE
#undef PARAMS

#endif // _INLINE_CXX_
