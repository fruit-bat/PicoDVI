#include "us_voice.h"

void __not_in_flash_func(us_voice_init)(UsVoice* voice, UsWaveFunc wave_func, UsAdsrConfig *adsr_config) {
    voice->wave_func = wave_func;
    voice->gain = 0;
    voice->note = 0;
    voice->channel = 0;
    us_adsr_init(&voice->adsr, adsr_config);
}

int32_t __not_in_flash_func(us_voice_update)(UsVoice* voice) {
    const int32_t adsr = us_adsr_update(&voice->adsr);
    if (us_adsr_is_off(&voice->adsr)) {
        return 0;
    }
    else {
        us_tuner_rotate(&voice->tuner);
        const int32_t a1 = __mul_instruction(voice->wave_func(voice->tuner.bang), voice->gain) >> 8;
        return __mul_instruction(a1, adsr) >> 16;
    }
}

void __not_in_flash_func(us_voice_note_on)(UsVoice* voice, uint32_t channel, uint32_t note, int32_t bend, uint32_t velocity) {
    us_tuner_reset_phase(&voice->tuner); // Set the phase to 0
    us_tuner_set_note(&voice->tuner, note, bend);
    voice->gain = velocity;
    voice->note = note;
    voice->channel = channel;
    us_adsr_attack(&voice->adsr);
}

void __not_in_flash_func(us_voice_note_off)(UsVoice* voice, uint32_t velocity) {
    us_adsr_release(&voice->adsr);
}

void __not_in_flash_func(us_voice_bend)(UsVoice* voice, int32_t bend) {
    us_tuner_set_note(&voice->tuner, voice->note, bend);
}
