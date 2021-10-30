// Copyright (c) 2018-2021 Jan Brittenson
// See LICENSE for details.

#ifndef __BUTTON_H__
#define __BUTTON_H__

class Button: public Element {
public:
    struct Config {
        Size _size;
        uint16_t _indent;
        uint8_t _bg_color;
        uint8_t _fg_color;
        const Font* _font;
        TapFunc _tap;
        uint32_t _tap_param;
    };

private:
    const Config* _config;
    Position _pos;
    String _value;

public:
    Button() { }

    // * implements Element::Initialize
    virtual void Initialize(const void* config, const Position& pos) {
        _config = (const Config*)config;
        _pos = pos;
        _value = STR("");
    }

    // * implements Element::Redraw
    virtual void Redraw() {
        if (Element::IsCovered())
            return;

        Panel& p = GetPanel();

        SetFGColor(_config->_bg_color);
        p.Fill(_pos._x, _pos._y, _config->_size._w, _config->_size._h);

        SetColor(_config->_fg_color, _config->_bg_color);
        p.Rect(_pos._x, _pos._y, _config->_size._w, _config->_size._h, 2);
        p.Text(_pos._x, _pos._y, *_config->_font, _value, 1, true);
    }

    // * implements Element::Tap
    virtual bool Tap(const Position& pos) {
        if (!pos.Inside(_pos, _config->_size))
            return false;

        ui::tap::SetTarget(this, _config->_tap, _config->_tap_param);
        return true;
    }

    // Update value
    void Update(const String& s) {
        if (s == _value)
            return;

        // A bit simplistic
        _value = s;
        Redraw();
    }
private:
    Button(const Button&);
    Button& operator=(const Button&);
};


#endif // __BUTTON_H__
