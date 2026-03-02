#pragma once

#include "core/pstring.h"

template <typename T>
class Consumer {
public:
    virtual void Apply(const T v) = 0;
    virtual void Apply(const T* v, uint n) = 0;
    virtual void Apply(const String& s) = 0;
};
