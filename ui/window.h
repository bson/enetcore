// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

class Window: public Element {
public:
    struct Config {
        Size _size;
        uint8_t _bg_color;
        uint8_t _fg_color;
        uint16_t _nchildren;
        ElementPlacement _children[];
    };

private:
    const Config* _config;
    Position _pos;
public:

    // * implements Element::Initialize
    virtual void Initialize(const void* config, const Position& pos) {
        _config = (const Config*)config;
        _pos = pos;

        for (uint i = 0; i < _config->_nchildren; ++i) {
            ElementPlacement* child = const_cast<ElementPlacement*>(_config->_children + i);
            const Position childpos = { uint16_t(_pos._x + child->_pos._x),
                                        uint16_t(_pos._y + child->_pos._y) };
            
            child->_element->Initialize(child->_config, childpos);
        }
    }

    // * implements Element::Redraw
    virtual void Redraw() {
        Panel& p = GetPanel();

        SetFGColor(_config->_bg_color);
        p.Fill(_pos._x, _pos._y, _config->_size._w, _config->_size._h);

        for (uint i = 0; i < _config->_nchildren; ++i) {
            ElementPlacement* child = const_cast<ElementPlacement*>(_config->_children + i);
            child->_element->Redraw();
        }
    }

    // * implements Element::Tap
    virtual bool Tap(const Position& pos, TapFunc& f, uint32_t& a) {
        for (uint i = 0; i < _config->_nchildren; ++i) {
            ElementPlacement* child = const_cast<ElementPlacement*>(_config->_children + i);
            if (child->_element->Tap(pos, f, a))
                return true;
        }
        return false;
    }
};
