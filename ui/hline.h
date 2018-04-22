// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

class HLine: public Element {
    const Config* _config;
    Position _pos;

public:
    struct Config {
        Size _size;
        uint8_t _fg_color;
    };

    // * implements Element::Initialize
    virtual void Initialize(const void* config, const Position& pos) {
        _config = (const Config*)config;
        _pos = pos;
    }

    // * implements Element::Redraw
    virtual void Redraw() {
        Panel& p = GetPanel();

        SetFGColor(_config->_bg_color);
        p.HLine(_pos._x, _pos._y, _config->_size._w, _config->_size._h);
    }
};
