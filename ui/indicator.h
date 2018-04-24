// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

class Indicator: public Element {
    const Config* _config;
    Position _pos;
    bool _state;
    uchar _c[2];

public:
    struct Config {
        uint8_t _bg_color;
        uint8_t _fg_color;
        const Font* _font;
        uint8_t _true;
        uint8_t _false;
    };

    // * implements Element::Initialize
    virtual void Initialize(const void* config, const Position& pos) {
        _config = (const Config*)config;
        _pos = pos;
        _state = false;
        _c[1] = 0;
    }

    // * implements Element::Redraw
    virtual void Redraw() {
        Panel& p = GetPanel();

        _c[0] = _state ? _config->_true : _config->_false;

        SetColor(_config->_fg_color, _config->_bg_color);
        p.Text(_pos._x, _pos,_y, *_config->_font, _c, 0, false);
    }

    // Update value
    void Update(bool state) {
        if (state == _state)
            return;

        // A bit simplistic
        _state = state;
        Redraw();
    }
};
