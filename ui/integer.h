// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "pstring.h"

class Integer: public Element {
public:
    struct Config {
        Size _size;
        uint8_t _bg_color;
        uint8_t _fg_color;
        const Font* _font;
        const uchar* _fmt;
        TapFunc _tap;
        uint32_t _tap_param;
    };

private:
    const Config* _config;
    Position _pos;
    int _value;

public:
    // * implements Element::Initialize
    virtual void Initialize(const void* config, const Position& pos) {
        _config = (const Config*)config;
        _pos = pos;
        _value = 0;
    }

    // * implements Element::Redraw
    virtual void Redraw() {
        if (Element::IsCovered())
            return;

        Panel& p = GetPanel();

        SetFGColor(_config->_bg_color);
        p.Fill(_pos._x, _pos._y, _config->_size._w, _config->_size._h);

        SetColor(_config->_fg_color, _config->_bg_color);
        const String s = String::Format(_config->_fmt, _value);

        p.Text(_pos._x, _pos._y, *_config->_font, s, 1, true);
    }

    // * implements Element::Tap
    virtual bool Tap(const Position& pos) {
        if (!pos.Inside(_pos, _config->_size))
            return false;

        ui::tap::SetTarget(this, _config->_tap, _config->_tap_param);
        return true;
    }

    // Update value
    void Update(int value) {
        if (value == _value)
            return;

        // A bit simplistic
        _value = value;
        Redraw();
    }
};
