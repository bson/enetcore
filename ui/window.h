// Copyright (c) 2018-2021 Jan Brittenson
// See LICENSE for details.

#ifndef __WINDOW_H__
#define __WINDOW_H__

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
    // Return the nth child
    ElementPlacement* Child(uint n) {
        assert_bounds(n < _config->_nchildren);
        return const_cast<ElementPlacement*>(_config->_children + n);
    }

    // * implements Element::Initialize
    virtual void Initialize(const void* config, const Position& pos) {
        _config = (const Config*)config;
        _pos = pos;

        for (uint i = 0; i < _config->_nchildren; ++i) {
            ElementPlacement* child = Child(i);
            const Position childpos(_pos._x + child->_pos_x, _pos._y + child->_pos_y);
            
            child->_element->Initialize(child->_config, childpos);
        }
    }

    // * implements Element::Redraw
    virtual void Redraw() {
        if (Element::IsCovered())
            return;

        Panel& p = GetPanel();

        SetFGColor(_config->_bg_color);
        p.Fill(_pos._x, _pos._y, _config->_size._w, _config->_size._h);

        for (uint i = 0; i < _config->_nchildren; ++i)
            Child(i)->_element->Redraw();
    }

    // * implements Element::SetCovered
    virtual void SetCovered(bool covered) {
        Element::_covered = covered;

        for (uint i = 0; i < _config->_nchildren; ++i)
            Child(i)->_element->SetCovered(covered);
    }

    // * implements Element::Tap
    virtual bool Tap(const Position& pos) {
        for (uint i = 0; i < _config->_nchildren; ++i) {
            if (Child(i)->_element->Tap(pos))
                return true;
        }

        return false;
    }
};

#endif // __WINDOW_H__
