#pragma once
#include "us_voice.h"

#define US_VOICE_COUNT_LOG2 3L
#define US_VOICE_COUNT (1 << US_VOICE_COUNT_LOG2)

typedef struct {
    UsVoice voice[US_VOICE_COUNT];
    int32_t out;
} UsVoices;

void us_voices_init(UsVoices *voices, UsWaveFunc wave_func);

int32_t us_voices_update(UsVoices *voices);
