// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef _UI_H_
#define _UI_H_

#include "enetcore.h"
#include "font.h"

namespace ui {

enum { NCOLORS = 8 };

// These are expencted to be found elsewhere
extern class Panel& GetPanel();
extern uint32_t _palette[NCOLORS];

[[__finline]] uint32_t GetColor(uint n) { return _palette[n % NCOLORS]; }
[[__finline]] void SetBGColor(uint n) {
    const uint32_t rgb = GetColor(n);
    GetPanel().SetBackground(rgb >> 24, rgb >> 16, rgb);
}

[[__finline]] void SetFGColor(uint n) {
    const uint32_t rgb = GetColor(n);
    GetPanel().SetRGB(rgb >> 24, rgb >> 16, rgb);
}

[[__finline]] void SetColor(uint fg, uint bg) {
    SetFGColor(fg);
    SetBGColor(bg);
}


struct Position {
    uint16_t _x, _y;
};

struct Size {
    uint16_t _w, _h;
};

struct Dimension {
    Position _pos;
    Size _size;
};

class Element {
public:
    virtual void Initialize(const void* config, const Position& pos) = 0;
    virtual void Redraw() = 0;
};

struct ElementPlacement {
    Position _pos;
    Element* _element;
    void* _config;
};

#include "window.h"
#include "label.h"
#include "vline.h"
#include "hline.h"
#include "integer.h"

};  // ns ui

#endif // _UI_H_
