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

void us_voice_note_on(UsVoice* voice, uint8_t note, uint8_t velocity) {
    
}

void us_voice_note_off(UsVoice* voice, uint8_t velocity) {

}