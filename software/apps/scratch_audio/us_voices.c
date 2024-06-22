#include "us_voices.h"

void us_voices_init(UsVoices *voices, UsWaveFunc wave_func, UsAdsrConfig *adsr_config) {
    for(int32_t i = 0; i < US_VOICE_COUNT; ++i) {
        us_voice_init(&voices->voice[i], wave_func, adsr_config);
    }
    voices->out = 0;
}

int32_t __not_in_flash_func(us_voices_update)(UsVoices *voices) {
    int32_t out = 0;
    for(int32_t i = 0; i < US_VOICE_COUNT; ++i) {
        out += us_voice_update(&voices->voice[i]);
    }
    out = out >> (US_VOICE_COUNT_LOG2 / 2);
    if (out > 32767) return 32767;
    if (out < -32768) return -32768;
    return out;
}
