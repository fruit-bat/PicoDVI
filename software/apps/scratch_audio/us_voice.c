#include "us_voice.h"

void us_voice_update(UsVoice* voice) {
    if (voice->gain == 0) {
        voice->out = 0;
    }
    else {
        us_tuner_rotate(&voice->tuner);
        voice->out = __mul_instruction(voice->wave_func(voice->tuner.bang), voice->gain) >> 8;
    }
}

void us_voice_note_on(UsVoice* voice, uint32_t note, uint32_t velocity) {
    // Just a hack so we can hear something
    us_tuner_reset(&voice->tuner); // Set the phase to 0
    us_tuner_set_note(&voice->tuner, note);
    voice->gain = velocity;
}

void us_voice_note_off(UsVoice* voice, uint32_t velocity) {
    // Just a hack so we can hear something
    voice->gain = 0;
}