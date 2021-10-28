#ifndef __AUDIO_H__
#define __AUDIO_H__

#include <stdint.h>

struct Sound {
    uint16_t nsamples;
    uint16_t samplesize;        // In bytes
    uint16_t samplerate;
    uint16_t samples[];
};

#endif // __AUDIO_H__
