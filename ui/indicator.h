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
        TapFunc _tap;
        uint32_t _tap_param;
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

    // * implements Element::Tap
    virtual bool Tap(const Position& pos) {
        const Size size = { _config->_font->GetWidth(), _config->_font->GetHeight() };

        if (!pos.Inside(_pos, size))
            return false;

        _config->_tap(_config->_tap_param);
        return true;
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
