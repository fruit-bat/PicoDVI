#include "us_adsr.h"


void __not_in_flash_func(us_adsr_init)(
    UsAdsr *adsr
) {
    adsr->stage = UsAdsrStageOff;
}

void __not_in_flash_func(us_adsr_on)(
    UsAdsr *adsr
) {
    adsr->stage = UsAdsrStageAttack;
    us_tuner_reset_phase(&adsr->tuner);
    us_tuner_set_pitch(&adsr->tuner, &adsr->attack);
    // TODO set the tuner to an upward ramp
}

inline int32_t us_adsr_bang_to_wave(
    UsAdsr *adsr
) {
    return adsr->wave_func(adsr->tuner.bang);
}

int32_t __not_in_flash_func(us_adsr_update)(
    UsAdsr *adsr
) {
    switch(adsr->stage) {
        // Off
        case UsAdsrStageOff: {
            return 0;
        }
        // Attack
        case UsAdsrStageAttack: {
            uint32_t v;
            if (us_tuner_rotate_check_wrap(&adsr->tuner)) {
                v = adsr->wave_func(US_BANG_MAX);
                adsr->stage = UsAdsrStageDecay;
            }
            else {
                v = us_adsr_bang_to_wave(adsr);
            }
            adsr->sustain = v;
            return v;
        }
        // Decay
        case UsAdsrStageDecay: {
            if (us_tuner_rotate_check_wrap(&adsr->tuner)) {
                us_tuner_reset_phase(&adsr->tuner);
                const int32_t v = adsr->wave_func(US_BANG_MAX);
                adsr->stage = UsAdsrStageSustain;
                adsr->sustain = v;
                return v;
            }
            else {
                return us_adsr_bang_to_wave(adsr);
            }
        }
        // Sustain
        case UsAdsrStageSustain: {
            return adsr->sustain;
        }
        // Release
        case UsAdsrStageRelease: {
            if (us_tuner_rotate_check_wrap(&adsr->tuner)) {
                us_tuner_reset_phase(&adsr->tuner);
                adsr->stage = UsAdsrStageOff;
                return 0;
            }
            else {
                return us_adsr_bang_to_wave(adsr);
            }
        }
        default: {
            return 0;
        }
    }
}
