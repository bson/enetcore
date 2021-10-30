// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef __VLINE_H__
#define __VLINE_H__

class VLine: public Element {
public:
    struct Config {
        Size _size;
        uint8_t _fg_color;
    };

private:
    const Config* _config;
    Position _pos;

public:
    // * implements Element::Initialize
    virtual void Initialize(const void* config, const Position& pos) {
        _config = (const Config*)config;
        _pos = pos;
    }

    // * implements Element::Redraw
    virtual void Redraw() {
        if (Element::IsCovered())
            return;

        Panel& p = GetPanel();

        SetFGColor(_config->_fg_color);
        p.VLine(_pos._x, _pos._y, _config->_size._w, _config->_size._h);
    }
};

#endif // __VLINE_H__
