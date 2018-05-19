// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef _UI_H_
#define _UI_H_

#include "enetcore.h"
#include "font.h"

namespace ui {

enum { NCOLORS = 8 };

// These are expencted to be found elsewhere
extern Panel& GetPanel();
extern uint32_t _palette[NCOLORS];
inline uint32_t GetColor(uint n) { return _palette[n % NCOLORS]; }
inline void SetBGColor(uint n) {
    const uint32_t rgb = GetColor(n);
    GetPanel().SetBackground(rgb >> 24, rgb >> 16, rgb);
}

inline void SetFGColor(uint n) {
    const uint32_t rgb = GetColor(n);
    GetPanel().SetRgb(rgb >> 24, rgb >> 16, rgb);
}

inline void SetColor(uint fg, uint bg) {
    SetFGColor(fg);
    SetBGColor(bg);
}


typedef void (*TapFunc)(uint32_t);

struct Size {
    uint16_t _w, _h;
};

struct Position {
    uint16_t _x, _y;

    explicit Position(uint16_t x, uint16_t y) : _x(x), _y(y) { }

    bool Inside(const Position& pos, const Size& size) const {
        return _x >= pos._x && _x < pos._x + size._w
               && _y >= pos._y && _y < pos._y + size._h;
    }

    Position operator+(const Position& rhs) const {
        return Position(_x + rhs._x, _y + rhs._y);
    }

    Position operator-(const Position& rhs) const {
        return Position(_x - rhs._x, _y - rhs._y);
    }
};

struct Dimension {
    Position _pos;
    Size _size;
};

class Element {
public:
    virtual void Initialize(const void* config, const Position& pos) = 0;
    virtual void Redraw() = 0;
    virtual bool Tap(const Position& pos, TapFunc& f, uint32_t& a) = 0;
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
