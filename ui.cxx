#include "enetkit.h"
#include "board.h"
#include "mutex.h"
#include "ui.h"
#include "uidecls.h"

#include "font/runes.inc"

extern EventObject _panel_tap;
extern GpioIntr _gpio0_intr;

Panel& GetPanel() { return _panel; }

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

        if ((bool)state) {
            ui::tap::Clear();
            if (uibuilder::main_readout.Tap(ui::Position(x, y))) {
                ui::tap::_element->Highlight();
                Thread::Sleep(Time::Now() + Time::FromMsec(TAP_HIGHLIGHT_MSEC));
                ui::tap::_element->Normal();

                ui::tap::Dispatch();
            }
        }
    }
}


// Initialize UI widget hierarchy
void Initialize() {
    static const ui::Position topleft(0, 0);

    DMSG("UI init");

    uibuilder::main_readout.Initialize(&uibuilder::main_readout_conf, topleft);
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
            rearm_tap = now + Time::FromMsec(TAP_DEBOUNCE_MSEC);
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
