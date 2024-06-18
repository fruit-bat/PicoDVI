#pragma once
#include "us_tuner.h"

typedef int32_t (*UsWaveFunc)(
    const uint32_t bang
);

typedef struct {
    UsTuner tuner;        // TODO Maybe too specific
    UsWaveFunc wave_func; // TODO Maybe too specific
    int32_t out;
    uint32_t gain;        // 0 <= Gain <= 256
} UsVoice;

inline void us_voice_init(UsVoice* voice) {
    voice->out = 0;
    voice->gain = 0;
}

void us_voice_update(UsVoice* voice);
