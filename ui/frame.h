// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

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
    bool _covered;

public:
    // * implements Element::Initialize
    virtual void Initialize(const void* config, const Position& pos) {
        _config = (const Config*)config;
        _pos = pos;
    }

    // * implements Element::Redraw
    virtual void Redraw() {
        if (_covered)
            return;

        Panel& p = GetPanel();

        SetFGColor(_config->_fg_color);
        p.Rect(_pos._x, _pos._y, _config->_size._w, _config->_size._h, _config->_width);
    }

    // * implements Element::SetCovered
    virtual void SetCovered(bool covered) {
        _covered = covered;
    }

    // * implements Element::Tap
    virtual bool Tap(const Position& pos) {
        return false;
    }
};
