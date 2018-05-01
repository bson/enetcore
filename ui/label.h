// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

class Label: public Element {
public:
    struct Config {
        Size _size;
        uint8_t _bg_color;
        uint8_t _fg_color;
        const Font* _font;
    };

private:
    const Config* _config;
    Position _pos;
    String _value;

public:
    // * implements Element::Initialize
    virtual void Initialize(const void* config, const Position& pos) {
        _config = (const Config*)config;
        _pos = pos;
        _value = STR("");
    }

    // * implements Element::Redraw
    virtual void Redraw() {
        Panel& p = GetPanel();

        SetFGColor(_config->_bg_color);
        p.Fill(_pos._x, _pos._y, _config->_size._w, _config->_size._h);

        SetColor(_config->_fg_color, _config->_bg_color);
        p.Text(_pos._x, _pos._y, *_config->_font, _value, 1, true);
    }

    // Update value
    void Update(const String& s) {
        if (s == _value)
            return;

        // A bit simplistic
        _value = s;
        Redraw();
    }
};
