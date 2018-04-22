// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

class Integer: public Element {
    const Config* _config;
    Position _pos;
    int _value;

public:
    struct Config {
        Size _size;
        uint8_t _bg_color;
        uint8_t _fg_color;
        const Font* _font;
        const char* _fmt;
    };

    // * implements Element::Initialize
    virtual void Initialize(const void* config, const Position& pos) {
        _config = (const Config*)config;
        _pos = pos;
        _value = 0;
    }

    // * implements Element::Redraw
    virtual void Redraw() {
        Panel& p = GetPanel();

        SetFGColor(_config->_bg_color);
        p.Fill(_pos._x, _pos._y, _config->_size._w, _config->_size._h);

        SetColor(_config->_fg_color, _config->_bg_color);
        const String s = Util::Format(_config->_fmt, _value);

        p.Text(_pos._x, _pos,_y, *_config->_font, s, 1, true);
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
