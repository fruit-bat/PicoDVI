#include "us_voice.h"

int32_t __not_in_flash_func(us_voice_update)(UsVoice* voice) {
    if (voice->gain == 0) {
        return 0;
    }
    else {
        us_tuner_rotate(&voice->tuner);
        return __mul_instruction(voice->wave_func(voice->tuner.bang), voice->gain) >> 8;
    }
}

void __not_in_flash_func(us_voice_note_on)(UsVoice* voice, uint32_t note, uint32_t velocity) {
    // Just a hack so we can hear something
    us_tuner_reset_phase(&voice->tuner); // Set the phase to 0
    us_tuner_set_note(&voice->tuner, note);
    voice->gain = velocity;
}

void __not_in_flash_func(us_voice_note_off)(UsVoice* voice, uint32_t velocity) {
    // Just a hack so we can hear something
    voice->gain = 0;
}

