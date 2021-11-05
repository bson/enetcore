#include "core/enetkit.h"
#include "board.h"

#ifdef ENABLE_PANEL

#include "core/mutex.h"
#include "ui/ui.h"
#include "uidecls.h"
#include "ui/font/runes.inc"


extern PinNegOutput<Gpio::Pin> _panel_bl;
extern EventObject _panel_tap;

Panel& GetPanel() { return _panel; }

// Output memory status
static void ShowMemstats() {
    const struct mallinfo mi = dlmallinfo();

    const String s = String::Format(STR("arena=%u, free chunks=%u, alloc=%u, "
                                        "free=%u"),
                                    mi.arena, mi.ordblks, mi.uordblks, mi.fordblks);

    _panel.Text(8, 272-8-6, font_5x8, s);
}

// Enable panel tap interrupts
static void EnableTapIntr() {
    Eintr::EnableInt(12);
}

enum class TapState: bool {
    PRESSED = false,
    RELEASED = true
};

enum {
    POS_BOXCAR_AVG = 4,
    TAP_HIGHLIGHT_MSEC = 100,
    TAP_DEBOUNCE_MSEC = 25
};

static TapState _panel_state = TapState::RELEASED;

// Called to handle tap on panel.  'press' is true if it's a press,
// otherwise false on release.
static  void HandleTap(TapState state) {
// XXX
#if 0
    if (state != _panel_state) {
        _panel_state = state;

        uint16_t x = 0, y = 0;

        for (int i = 0; i < POS_BOXCAR_AVG; i++) {
            uint16_t xsample, ysample;
            
            _touch.ReadPosition(xsample, ysample);
            x += xsample;
            y += ysample;
        }

        x /= POS_BOXCAR_AVG;
        y /= POS_BOXCAR_AVG;
                
        DMSG("%s @ %d, %d", state == TapState::PRESSED ? "Press" : "Release", x, y);

        if (state != TapState::PRESSED) {
            ui::tap::Clear();
            if (uibuilder::main_view.Tap(ui::Position(x, y))) {
                ui::tap::_element->Highlight();
                Thread::Sleep(Time::Now() + Time::FromMsec(TAP_HIGHLIGHT_MSEC));
                ui::tap::_element->Normal();

                ui::tap::Dispatch();
            }
        }
    }
#endif
}


// Initialize UI widget hierarchy
void Initialize() {
    static const ui::Position topleft(0, 0);

    DMSG("UI init");

    uibuilder::main_view.Initialize(&uibuilder::main_view_conf, topleft);
}


void NoTap(ui::Element*, uint32_t) { }


void TapCcv(ui::Element*, uint32_t) {
    DMSG("Tapped CCV");
}


void TapOvp(ui::Element*, uint32_t) {
    DMSG("Tapped OVP");
}


void MainCommand(ui::Element*, uint32_t cmd) {
    DMSG("Command: %d", cmd);
}


void* UIThread(void*) {
    Thread::SetPriority(UI_THREAD_PRIORITY);

    DMSG("UI start");

    void Initialize();

    _panel_bl.Raise();

    _panel.SetBackground(64, 64, 80);
    _panel.Init();

    // XXX
    // _touch.Init();
    
    _panel.SetBrightness(128);

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

    const Time now = Time::Now();
    Time next_memstats = now;
    Time rearm_tap = now;
    Time display = now;

    EnableTapIntr();
    bool armed = true;

    for (;;) {
        Time next_wake = min<Time>(next_memstats, display);
        if (!armed)
            next_wake = min<Time>(next_wake, rearm_tap);

        if (next_wake > Time::Now())
            _panel_tap.Wait(next_wake);

        const Time now = Time::Now();

        if (now >= rearm_tap) {
            EnableTapIntr();
            armed = true;
        }

        const uint state = _panel_tap.GetState();
        if (state) {
            rearm_tap = now + Time::FromMsec(TAP_DEBOUNCE_MSEC);
            HandleTap(state == 1 ? TapState::PRESSED : TapState::RELEASED);
            _panel_tap.Reset();
            armed = false;
        }

        if (now >= next_memstats) {
            ShowMemstats();
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

#endif
