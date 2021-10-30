//
// SSD1963 based panel
//

#ifndef _SSD1306_H_
#define _SSD1306_H_

#include <stdint.h>
#include "core/enetcore.h"
#include "ui/font.h"
#include "ui/font/runes.h"

namespace ssd1963 {

// DBPORT is the data port
// CTLPORT is the control port
// Operates in 8080 mode
template <Gpio& _DPORT,
          Gpio& _CPORT,
          uint32_t _CTL_CS,
          uint32_t _CTL_WR,
          uint32_t _CTL_RD,
          uint32_t _CTL_RS>
class Panel {
    static uint8_t _r;
    static uint8_t _g;
    static uint8_t _b;
    static uint8_t _bg_r;
    static uint8_t _bg_g;
    static uint8_t _bg_b;

public:
    enum {
        MASK_CS = _CTL_CS,
        MASK_WR = _CTL_WR,
        MASK_RD = _CTL_RD,
        MASK_RS = _CTL_RS,
        MASK_CTL = MASK_CS | MASK_WR | MASK_RD | MASK_CS
    };

    enum {
        CMD_NOP                = 0,
        CMD_SOFT_RESET         = 0x01,
        CMD_GET_POWER_MODE     = 0x0a,
        CMD_GET_ADDRESS_MODE   = 0x0b,
        CMD_GET_PIXEL_FORMAT   = 0x0c,
        CMD_GET_DISPLAY_MODE   = 0x0d,
        CMD_GET_SIGNAL_MODE    = 0x0e,
        CMD_ENTER_SLEEP_NODE   = 0x10,
        CMD_EXIT_SLEEP_MODE    = 0x11,
        CMD_ENTER_PARTIAL_MODE = 0x12,
        CMD_ENTER_NORMAL_MODE  = 0x13,
        CMD_EXIT_INVERT_MODE   = 0x20,
        CMD_ENTER_INVERT_MODE  = 0x21,
        CMD_SET_GAMMA_CURVE    = 0x26,
        CMD_SET_DISPLAY_OFF    = 0x28,
        CMD_SET_DISPLAY_ON     = 0x29,
        CMD_SET_COLUMN_ADDRESS = 0x2a,
        CMD_SET_PAGE_ADDRESS   = 0x2b,
        CMD_WRITE_MEMORY_START = 0x2c,
        CMD_READ_MEMORY_START  = 0x2e,
        CMD_SET_PARTIAL_AREA   = 0x30,
        CMD_SET_SCROLL_AREA    = 0x33,
        CMD_SET_TEAR_OFF       = 0x34,
        CMD_SET_TEAR_ON        = 0x35,
        CMD_SET_ADDRESS_MODE   = 0x36,
        CMD_SET_SCROLL_MODE    = 0x37,
        CMD_EXIT_IDLE_MODE     = 0x38,
        CMD_ENTER_IDLE_MODE    = 0x39,
        CMD_SET_PIXEL_FORMAT   = 0x3a,
        CMD_WRITE_MEMORY_CONTINUE = 0x3c,
        CMD_READ_MEMORY_CONTINUE = 0x3e,
        CMD_SET_TEAR_SCANLINE  = 0x44,
        CMD_GET_SCANLINE       = 0x45,
        CMD_READ_DDB           = 0xa1,
        CMD_SET_LCD_MODE       = 0xb0,
        CMD_GET_LCD_MODE       = 0xb1,
        CMD_SET_HORI_PERIOD    = 0xb4,
        CMD_GET_HORI_PERIOD    = 0xb5,
        CMD_SET_VERT_PERIOD    = 0xb6,
        CMD_GET_VERT_PERIOD    = 0xb7,
        CMD_SET_GPIO_CONF      = 0xb8,
        CMD_GET_GPIO_CONF      = 0xb9,
        CMD_SET_GPIO_VALUE     = 0xba,
        CMD_GET_GPIO_STATUS    = 0xbb,
        CMD_SET_POST_PROC      = 0xbc,
        CMD_GET_POST_PROC      = 0xbd,
        CMD_SET_PWM_CONF       = 0xbe,
        CMD_GET_PWM_CONF       = 0xbf,
        CMD_SET_LCD_GEN0       = 0xc0,
        CMD_GET_LCD_GEN0       = 0xc1,
        CMD_SET_LCD_GEN1       = 0xc2,
        CMD_GET_LCD_GEN1       = 0xc3,
        CMD_SET_LCD_GEN2       = 0xc4,
        CMD_GET_LCD_GEN2       = 0xc5,
        CMD_SET_LCD_GEN3       = 0xc6,
        CMD_GET_LCD_GEN3       = 0xc7,
        CMD_SET_GPIO0_ROP      = 0xc8,
        CMD_GET_GPIO0_ROP      = 0xc9,
        CMD_SET_GPIO1_ROP      = 0xca,
        CMD_GET_GPIO1_ROP      = 0xcb,
        CMD_SET_GPIO2_ROP      = 0xcc,
        CMD_GET_GPIO2_ROP      = 0xcd,
        CMD_SET_GPIO3_ROP      = 0xce,
        CMD_GET_GPIO3_ROP      = 0xcf,
        CMD_SET_DBC_CONF       = 0xd0,
        CMD_GET_DBC_CONF       = 0xd1,
        CMD_SET_DBC_TH         = 0xd4,
        CMD_GET_DBC_TH         = 0xd5,
        CMD_SET_PLL            = 0xe0,
        CMD_SET_PLL_MN         = 0xe2,
        CMD_GET_PLL_MN         = 0xe3,
        CMD_GET_PLL_STATUS     = 0xe4,
        CMD_SET_DEEP_SLEEP     = 0xe5,
        CMD_SET_LSHIFT_FREQ    = 0xe6,
        CMD_GET_LSHIFT_FREQ    = 0xe7,
        CMD_SET_PIXEL_DATA_INTERFACE = 0xf0,
        CMD_GET_PIXEL_DATA_INTERFACE = 0xf1
    };

    Panel() { }

    static void Init();

    // Erase display, sets active color to black
    static void Clear();

    // Fill a box
    static void Fill(uint16_t col, uint16_t row, uint16_t w, uint16_t h);

    // Horizontal line of width w
    static void HLine(uint16_t x, uint16_t y, uint16_t len, uint8_t w);

    // Vertical line of width w
    static void VLine(uint16_t x, uint16_t y, uint16_t len, uint8_t w);

    // Rectangle of width s
    static void Rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t s);

    // Set active color
    static void SetRgb(uint8_t r, uint8_t g, uint8_t b) {
        _r = r; _g = g; _b = b;
    }

    // Set background color
    static void SetBackground(uint8_t r, uint8_t g, uint8_t b) {
        _bg_r = r; _bg_g = g; _bg_b = b;
    }

    static void Blit(uint16_t x, uint16_t y, const uint8_t* data, 
                     uint16_t w, uint16_t h);

    // Draw 7-bit text using a specific font. Kern is the number of
    // pixels of space to leave between chars.  vkern enables variable
    // kerning.  Use false for fixed field sizes, and true for text.
    // True may require clearing any residual; the function returns
    // the width in pixels blitted to the screen for this purpose.  The
    // text will wrap; if it wraps the total reflects the width on the
    // last line.  (Which is probably of limited use.)
    static uint Text(uint x, uint y, const Font& font, const String& s, 
                     uint8_t kern = 1, bool vkern = true);

    // Set PWM brightness, only usable for modules that use the PWM
    static void SetBrightness(uint8_t b) {
        command_start(CMD_SET_PWM_CONF);
        data(0x0e);  // 300Hz @ 120MHz PLL
        data(b);
        data(0x01);  // Enable
        data(0);
        data(0);
        data(0);
        command_end();
    }

    static void AdjustCbs(uint8_t cont  = 0x40,
                          uint8_t brite = 0x80,
                          uint8_t sat   = 0x40) {
        uint8_t v[4] = { cont, brite, sat, 1 };
        wcommand_barr(CMD_SET_POST_PROC, 4, v);
    }

    static void TestPattern();

private:
    // Set window
    static void set_window(uint16_t col, uint16_t row, uint16_t w, uint16_t h);

    // A bunch of inlined primitives
    [[__optimize, __finline]] static void command_start(uint8_t cmd) {
        _CPORT.Set(MASK_RS | MASK_RD | MASK_WR | MASK_CS);
        _CPORT.Clear(MASK_CS | MASK_RS | MASK_WR);  // CS active across command
        _DPORT.Output(0xff, cmd);            // Data on bus
        _CPORT.Set(MASK_WR); // WR inactive - data is latched on this edge
        _CPORT.Set(MASK_RS); // Restore RS
    }
    [[__finline, __optimize]] static void command_end() {
        _CPORT.Set(MASK_CS | MASK_RD | MASK_WR | MASK_RS);  // CS inactive
    }

    // In a series of data writes CS is kept low across all of them, then
    // released by command_end().
    [[__optimize, __finline]] static void data(uint8_t d) {
        _CPORT.Clear(MASK_WR); // WR active; RS = 1 = data
        _DPORT.Output(0xff, d); // Data on bus
        _CPORT.Set(MASK_WR); // WR inactive - data is latched on this edge
    }
    static void data16(uint16_t d) {
        data(d >> 8);
        data(d);
    }
    static void wcommand(uint8_t cmd) {
        command_start(cmd);
        command_end();
    }
    static void wcommand8(uint8_t cmd, uint8_t param) {
        command_start(cmd);
        data(param);
        command_end();
    }
    // Yet another variation: command followed by array of bytes
    static void wcommand_barr(uint8_t cmd, uint8_t nbytes, const uint8_t* d) {
        command_start(cmd);
        while (nbytes--)
            data(*d++);

        command_end();
    }

    static uint8_t rdata() {
        _DPORT.MakeInputs(0xff);  // Input
        _CPORT.Set(MASK_RS);  // Data
        _CPORT.Clear(MASK_RD);
        const uint8_t tmp = _DPORT.Input() & 0xff;
        _CPORT.Set(MASK_RD);
        _DPORT.MakeOutputs(0xff);  // Return to output
        return tmp;
    }
    static uint8_t rcommand(uint8_t cmd) {
        command_start(cmd);
        const uint8_t val = rdata();
        command_end();
        return val;
    }
};
}; // namespace ssd1963

#endif // _SSD1963_H_

