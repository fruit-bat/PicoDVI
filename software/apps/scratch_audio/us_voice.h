#pragma once
#include "us_wave.h"
#include "us_adsr.h"

typedef struct {
    UsTuner tuner;        // TODO Maybe too specific
    UsWaveFunc wave_func; // TODO Maybe too specific
    UsAdsr adsr;
    uint32_t gain;        // 0 <= Gain <= 256 // TODO Think about this one (0-127 coming from midi files)
    
    uint32_t note;        // Remeber the note that was played so we can bend it
    uint32_t channel;
} UsVoice;

void us_voice_init(UsVoice* voice, UsWaveFunc wave_func, UsAdsrConfig *adsr_config);

int32_t us_voice_update(UsVoice* voice);

void us_voice_note_on(UsVoice* voice, uint32_t channel, uint32_t note, int32_t bend, uint32_t velocity);

void us_voice_note_off(UsVoice* voice, uint32_t velocity);

void us_voice_bend(UsVoice* voice, int32_t bend);

bool inline us_voice_is_off(UsVoice* voice) {
    return voice->adsr.stage == UsAdsrStageOff;
}
