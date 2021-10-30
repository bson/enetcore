// Copyright (c) 2018-2021 Jan Brittenson
// See LICENSE for details.

#ifndef __FRAME_H__
#define __FRAME_H__


class Frame: public Element {
public:
    struct Config {
        Size _size;
        uint8_t _width;
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
        p.Rect(_pos._x, _pos._y, _config->_size._w, _config->_size._h, _config->_width);
    }
};


#endif // __FRAME_H__
