#include "enetkit.h"
#include "board.h"
#include "mutex.h"
#include "ui.h"
#include "uibuilder.h"

#include "font/runes.inc"

extern EventObject _panel_tap;
extern GpioIntr _gpio0_intr;

// Output memory status
static void memstats() {
    const struct mallinfo mi = dlmallinfo();

    const String s = String::Format(STR("arena=%u, free chunks=%u, alloc=%u, "
                                        "free=%u"),
                                    mi.arena, mi.ordblks, mi.uordblks, mi.fordblks);

    _panel.Text(8, 272-8-6, font_5x8, s);
}


// Enable panel tap interrupts
static void EnableTapIntr() {
    _gpio0_intr.Clear(BIT22);
    _gpio0_intr.EnableR(BIT22);
    _gpio0_intr.EnableF(BIT22);
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


// Initialize UI widget hierarchy
void Initialize() {
    static const Position topleft = { 0, 0 };

    DMSG("UI init");

    ui::Initialize(&main_readout_conf, topleft);
}


void* UIThread(void*) {
    enum { DEBOUNCE_MSEC = 25 };

    Thread::SetPriority(UI_THREAD_PRIORITY);

    DMSG("UI start");

    void Initialize();

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
    Time rearm_tap = Time::Now();
    Time display = Time::Now();

    EnableTapIntr();

    for (;;) {
        const Time next_wake = min<Time>(next_memstats, min<Time>(rearm_tap, display));

        if (next_wake > Time::Now())
            _panel_tap.Wait(next_wake);

        const Time now = Time::Now();

        if (now >= rearm_tap)
            EnableTapIntr();

        const uint state = _panel_tap.GetState();
        if (state) {
            rearm_tap = now + Time::FromMsec(DEBOUNCE_MSEC);
            HandleTap(state == 1 ? TapState::PRESSED : TapState::RELEASED);
            _panel_tap.Reset();
        }

        if (now >= next_memstats) {
            memstats();
            next_memstats += Time::FromSec(5);
        }

        if (now >= display) {
            const uint val = _clock.GetTime();
        
            const String s = String::Format(STR("%08x"), val);
            _panel.Text(320, 8, font_5x8, s, 1, false);
            _panel.Text(320, 20, font_ins_9x16, s, 1, false);

            const String s2 = String::Format(STR("%08d"), val);
            _panel.Text(200, 48, font_ins_25x37, s2, 3, false);

            display += Time::FromMsec(100);
        }
    }
}
