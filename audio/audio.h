// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

#pragma once

#include <stdint.h>

struct Sound {
    uint16_t nsamples;
    uint16_t samplesize;        // In bytes
    uint16_t samplerate;
    uint16_t samples[];
};
