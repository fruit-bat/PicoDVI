#pragma once
#include "us_wave.h"
#include "us_adsr.h"

typedef struct {
    UsTuner tuner;        // TODO Maybe too specific
    UsWaveFunc wave_func; // TODO Maybe too specific
    UsAdsr adsr;
    uint32_t gain;        // 0 <= Gain <= 256 // TODO Think about this one (0-127 coming from midi files)
} UsVoice;

void us_voice_init(UsVoice* voice, UsWaveFunc wave_func);

int32_t us_voice_update(UsVoice* voice);

void us_voice_note_on(UsVoice* voice, uint32_t note, uint32_t velocity);

void us_voice_note_off(UsVoice* voice, uint32_t velocity);