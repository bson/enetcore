// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef _FONT_H_
#define _FONT_H_

#include "enetcore.h"

// Font: container for font-related data A font is a set of runes of a
// uniform size, optionally with a translation table (if sparse).
// This is a struct to simplify direct initialization without calling
// a constructor.

struct Font {
    const uchar* _runes;        // Font runes
    const uchar* _map;          // Font map (or NULL if none)
    uint8_t _width;             // Rune width
    uint8_t _height;            // Rune height
    uint16_t _size;             // Rune size in bytes
    bool _rle:1;                // Run-length encoded

    // Get width, height
    uint8_t GetWidth() const { return _width; }
    uint8_t GetHeight() const { return _height; }

    // Get number of pixels in each rune
    uint GetSize() const {  _width * _height; }

    // Get number of bytes in each rune
    uint GetBytes() const { return _size; }

    // Get image data for rune C (translated through map)
    const uint8_t* GetData(uchar c) const {
        if (_map) 
            c = _map[c & 0x7f] & 0x7f;

        return _runes + c * GetBytes();
    }

    // Get kern bit.  The kern bit specifies the character has reduced kerning
    // relative to the next following character.  It's set for narrow characters.
    bool GetKern(uchar c) const { 
        return _map && (_map[c & 0x7f] & 0x80);
    }
};

#endif // _FONT_H_

