#include "enetkit.h"
#include "board.h"
#include "mutex.h"
#include "ui.h"

#include "font/runes.inc"

extern EventObject _panel_tap;

// Output memory status
static void memstats() {
    const struct mallinfo mi = dlmallinfo();

    const String s = String::Format(STR("arena=%u, free chunks=%u, alloc=%u, "
                                        "free=%u"),
                                    mi.arena, mi.ordblks, mi.uordblks, mi.fordblks);

    _panel.Text(8, 272-8-6, font_5x8, s);
}


enum class TapState: bool {
    PRESSED = 0,
    RELEASED = !PRESSED
};

static TapState _panel_state = TapState::RELEASED;

// Called to handle tap on panel.  'press' is true if it's a press,
// otherwise false on release.
void HandleTap(TapState state) {
    if (state != _panel_state) {
        uint16_t x = 0, y = 0;

        for (int i = 0; i < 4; i++) {
            uint16_t xsample, ysample;
            
            _touch.ReadPosition(xsample, ysample);
            x += xsample;
            y += ysample;
        }

        x /= 4;
        y /= 4;
                
        DMSG("%s @ %d, %d", state == TapState::PRESSED ? "Press" : "Release", x, y);
    }
}


void* UIThread(void*) {
    enum { DEGLITCH = 25 };

    Thread::SetPriority(UI_THREAD_PRIORITY);

    DMSG("Starting UI");

    _panel.SetBackground(64, 64, 80);
    _panel.Init();

    _touch.Init();
    
#if 1
    _panel.TestPattern();

    _panel.SetRgb(255, 255, 0);
    _panel.Text(120, 8, font_ins_9x16, STR("Hello, world!"));

    _panel.SetRgb(255, 255, 255);
    _panel.Text(10, 200, font_ins_9x16, 
                STR("The quick brown fox jump.s ov,er the lazy dog!"));
    _panel.Text(10, 220, font_5x8, 
                STR("The quick; brown; fox... 'jumps' over: the \"lazy\" dog?! <=>[\\]{|}"));
#endif
    
    _panel.SetRgb(255, 255, 0);

    Time next_memstats = Time::Now();
    Time rearm = Time::Now();

    for (;;) {
        _panel_tap.Wait(Time::FromMsec(1000));
        const uint state = _panel_tap.GetState();
        if (state && state <= 2) {
            if (Time::Now() >= rearm) {
                rearm = Time::Now() + Time::FromMsec(DEGLITCH);
                HandleTap(state == 1 ? TapState::PRESSED : TapState::RELEASED);
            }
        }
        _panel_tap.Reset();

        if (Time::Now() >= next_memstats) {
            memstats();
            next_memstats += Time::FromSec(5);
        }

        const uint val = _clock.GetTime();
        
        const String s = String::Format(STR("%08x"), val);
        _panel.Text(320, 8, font_5x8, s, 1, false);
        _panel.Text(320, 20, font_ins_9x16, s, 1, false);

        const String s2 = String::Format(STR("%08d"), val);
        _panel.Text(200, 48, font_ins_25x37, s2, 3, false);
    }
}
