#pragma once
#include "us_wave.h"

typedef struct {
    UsTuner tuner;        // TODO Maybe too specific
    UsWaveFunc wave_func; // TODO Maybe too specific
    uint32_t gain;        // 0 <= Gain <= 256
} UsVoice;

inline void us_voice_init(UsVoice* voice, UsWaveFunc wave_func) {
    voice->wave_func = wave_func;
    voice->gain = 0;
}

int32_t us_voice_update(UsVoice* voice);

void us_voice_note_on(UsVoice* voice, uint32_t note, uint32_t velocity);

void us_voice_note_off(UsVoice* voice, uint32_t velocity);