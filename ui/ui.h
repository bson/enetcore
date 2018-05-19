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

    // Some handy methods.  This is still pod.

    explicit Position(uint16_t x, uint16_t y) : _x(x), _y(y) { }

    bool Inside(const Position& pos, const Size& size) const {
        return _x >= pos._x && _x < pos._x + size._w
               && _y >= pos._y && _y < pos._y + size._h;
    }

    // Simple vector arithmetic, to flatten relative positions into absolute
    Position operator+(const Position& rhs) const {
        return Position(_x + rhs._x, _y + rhs._y);
    }

    // Simple vector arithmetic, to deflatten absolute positions into relative
    Position operator-(const Position& rhs) const {
        return Position(_x - rhs._x, _y - rhs._y);
    }

    // Ordering: if above, or to the left if at the same height, then
    // a position is "less than" another one.  This is somewhat
    // arbitrary and its purpose is only for container use and to
    // support algorithms related to containers, e.g. to find an
    // enclosing widget through bisection.
    bool LessThan(const Position& arg) const {
        return _y < arg._y || (_y == arg._y && _x < arg._x);
    }

    bool operator<(const Position& rhs) const { return LessThan(rhs); }

    // Strict equality
    bool Equals(const Position& arg) const { return _x == arg._x && _y == arg._y; }
    bool operator=(const Position& rhs) const { return Equals(rhs); }
};

struct Rectangle {
    Position _pos;
    Size _size;

    explicit Rectangle(const Position& pos, const Size& size)
        : _pos(pos), _size(size) {
    }

    bool Contains(const Position& pos) const { return pos.Inside(_pos. _size); }

    // The four corners
    Position TopLeft() const { return _pos; }
    Position TopRight() const { return _pos + Position(_size._w, 0); }
    Position BotLeft() const { return _pos + Position(0, _size._h); }
    Position BotRight() const { return _pos + Position(_size._w, _size._h); }

    // True if two rectangles intersect
    bool Intersects(const Rectangle& other) const {
        return Contains(other.TopLeft()) || Contains(other.TopRight())
            || Contains(other.BotLeft()) || Contains(other.BotRight());
    }
};

typedef Rectangle Dimension;

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
