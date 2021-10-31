#ifdef _INLINE_CXX_

#include <stdint.h>
#include "core/enetcore.h"
#include "hardware/ssd1963.h"
#include "core/font.h"
#include "ui/font/runes.h"
#include "hardware/hsd04319w1_a.h"

namespace ssd1963 {

using namespace PanelInfo;

template <typename Accessor> uint8_t ssd1963::Panel<Accessor>::_r;
template <typename Accessor> uint8_t ssd1963::Panel<Accessor>::_g;
template <typename Accessor> uint8_t ssd1963::Panel<Accessor>::_b;
template <typename Accessor> uint8_t ssd1963::Panel<Accessor>::_bg_r;
template <typename Accessor> uint8_t ssd1963::Panel<Accessor>::_bg_g;
template <typename Accessor> uint8_t ssd1963::Panel<Accessor>::_bg_b;

template <typename Accessor>
[[__optimize]]
void Panel<Accessor>::Init() {

    Accessor::Init();

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
    wcommand_barr(CMD_SET_PLL_MN, pll, sizeof pll);

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
            wcommand_barr(cmd, p, n);
            p += n;
        } else {
            wcommand(cmd);
        }
    }

    Clear();
}

template <typename Accessor>
void Panel<Accessor>::Clear() {
    SetRgb(_bg_r, _bg_g, _bg_b);
    Fill(0, 0, HOR_VISIBLE, VERT_VISIBLE);
}

template <typename Accessor>
void Panel<Accessor>::Fill(uint16_t col, uint16_t row, uint16_t w, uint16_t h) {

    set_window(col, row, w, h);

    Accessor::StartCommand(CMD_WRITE_MEMORY_START);

    for (uint n = 0; n < w * h; ++n) {
        Accessor::Write(_r);
        Accessor::Write(_g);
        Accessor::Write(_b);
    }

    Accessor::EndCommand();
    wcommand(CMD_NOP);
}

template <typename Accessor>
void Panel<Accessor>::set_window(uint16_t col, uint16_t row, uint16_t w, uint16_t h) {

    Accessor::StartCommand(CMD_SET_COLUMN_ADDRESS);
    data16(col);
    data16(col + w - 1);
    Accessor::EndCommand();

    Accessor::StartCommand(CMD_SET_PAGE_ADDRESS);
    data16(row);
    data16(row + h - 1);
    Accessor::EndCommand();
}

template <typename Accessor>
void Panel<Accessor>::HLine(uint16_t x, uint16_t y, uint16_t len, uint8_t w) {
    Fill(x, y, len, w);
}

template <typename Accessor>
void Panel<Accessor>::VLine(uint16_t x, uint16_t y, uint16_t len, uint8_t w) {
    Fill(x, y, w, len);
}

template <typename Accessor>
void Panel<Accessor>::Rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t s) {
    HLine(x, y, w, s);
    HLine(x, y + h - s, w, s);
    VLine(x, y, h, s);
    VLine(x + w - s, y, h, s);
}

template <typename Accessor>
void Panel<Accessor>::Blit(uint16_t x, uint16_t y, const uint8_t* image, 
                         uint16_t w, uint16_t h)  {
    set_window(x, y, w, h);

    Accessor::StartCommand(CMD_WRITE_MEMORY_START);
    uint8_t v = *image++;
    uint bits = 8;

    for (uint n = w * h; n > 0; --n) {
        if (v & 1) {
            Accessor::Write(_r);
            Accessor::Write(_g);
            Accessor::Write(_b);
        } else {
            Accessor::Write(_bg_r); 
            Accessor::Write(_bg_g);
            Accessor::Write(_bg_b);
        }
        if (!--bits) {
            v = *image++;
            bits = 8;
        } else {
            v >>= 1;
        }
    }

    Accessor::EndCommand();
    wcommand(CMD_NOP);
}


template <typename Accessor>
[[__optimize]]
uint Panel<Accessor>::Text(uint x, uint y, const Font& font, 
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


template <typename Accessor>
void Panel<Accessor>::TestPattern() {

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

#endif // _INLINE_CXX_
