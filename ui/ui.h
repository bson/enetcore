// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef _UI_H_
#define _UI_H_

#include "enetcore.h"
#include "font.h"

namespace uibuilder {
extern uint32_t GetColor(uint n);
};

// This one is expencted to be found elsewhere
extern Panel& GetPanel();

namespace ui {

enum { NCOLORS = 8 };

static void SetBGColor(uint n) {
    const uint32_t rgb = uibuilder::GetColor(n);
    GetPanel().SetBackground(rgb >> 24, rgb >> 16, rgb);
}

static void SetFGColor(uint n) {
    const uint32_t rgb = uibuilder::GetColor(n);
    GetPanel().SetRgb(rgb >> 24, rgb >> 16, rgb);
}

static void SetColor(uint fg, uint bg) {
    SetFGColor(fg);
    SetBGColor(bg);
}


typedef void (*TapFunc)(class Element*, uint32_t);

struct Size {
    uint16_t _w, _h;
};

struct Position {
    uint16_t _x, _y;

    // Some handy methods.  This is still pod.

    explicit Position(uint16_t x = 0, uint16_t y = 0) : _x(x), _y(y) { }

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

    bool Contains(const Position& pos) const { return pos.Inside(_pos, _size); }

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
protected:
    bool _covered;

public:
    virtual void Initialize(const void* config, const Position& pos) = 0;
    virtual void Redraw() = 0;
    virtual bool Tap(const Position& pos) { return false; }
    virtual void Highlight() { }
    virtual void Normal() { }
    virtual void SetCovered(bool covered) {
        _covered = covered;
    }
    bool IsCovered() const { return _covered; }
};

struct ElementPlacement {
    uint16_t _pos_x;
    uint16_t _pos_y;
    Element* _element;
    const void* _config;
};

// Set by the Tap() Element lookup call chain.  Making this state
// global prevents multiple concurrent taps.  Since that's not a
// functional requirement we take the code size and stack reduction
// from making this state global.
namespace tap {

extern Element* _element;
extern TapFunc  _func;
extern uint32_t _parm;

static void SetTarget(Element* e, TapFunc f, uint32_t p) {
    _element = e;
    _func    = f;
    _parm    = p;
}

static void Clear() {
    SetTarget(NULL, NULL, 0);
}

static void Dispatch() {
    if (_element && _func)
        _func(_element, _parm);

    Clear();
}

};

#include "view.h"
#include "window.h"
#include "label.h"
#include "hline.h"
#include "vline.h"
#include "frame.h"
#include "integer.h"
#include "indicator.h"
#include "button.h"

};  // ns ui

#endif // _UI_H_
