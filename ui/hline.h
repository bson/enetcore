// Copyright (c) 2018-2021 Jan Brittenson
// See LICENSE for details.

#ifndef __HLINE_H__
#define __HLINE_H__

class HLine: public Element {
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
        p.HLine(_pos._x, _pos._y, _config->_size._w, _config->_size._h);
    }
};

#endif // __HLINE_H__
