#include "us_adsr.h"


void __not_in_flash_func(us_adsr_init)(
    UsAdsr *adsr
) {
    adsr->stage = UsAdsrStageOff;
    adsr->sustain = 32767;

    // TOTO remove - some test values for now

    // 10ms attack
    adsr->attack.fips = 1246611822UL;
    adsr->attack.eips = 10;

    // 10ms decay
    adsr->decay.fips = 1246611822UL;
    adsr->decay.eips = 10;

    // 10ms release
    adsr->release.fips = 1246611822UL;
    adsr->release.eips = 12;

}

void __not_in_flash_func(us_adsr_attack)(
    UsAdsr *adsr
) {
    adsr->stage = UsAdsrStageAttack;
    us_tuner_reset_phase(&adsr->tuner);
    us_tuner_set_pitch(&adsr->tuner, &adsr->attack);
    adsr->wave_func = us_wave_ramp_up;
    adsr->vol = 0;
}

void __not_in_flash_func(us_adsr_release)(
    UsAdsr *adsr
) {
    adsr->stage = UsAdsrStageRelease;
    us_tuner_reset_phase(&adsr->tuner);
    us_tuner_set_pitch(&adsr->tuner, &adsr->release);
    adsr->wave_func = us_wave_ramp_up;
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
            if (us_tuner_rotate_check_wrap(&adsr->tuner)) {
                adsr->vol = 32767;
                adsr->stage = UsAdsrStageDecay;
                us_tuner_reset_phase(&adsr->tuner);
                us_tuner_set_pitch(&adsr->tuner, &adsr->decay);                
                adsr->wave_func = us_wave_ramp_down;
            }
            else {
                adsr->vol = us_adsr_bang_to_wave(adsr);
            }
            return adsr->vol;
        }
        // Decay
        case UsAdsrStageDecay: {
            if (us_tuner_rotate_check_wrap(&adsr->tuner)) {
                // This should never happen
                adsr->vol = 0;
                adsr->stage = UsAdsrStageOff;
            }
            else {
                adsr->vol = us_adsr_bang_to_wave(adsr);
                if (adsr->vol <= adsr->sustain){
                    adsr->vol = adsr->sustain;
                    adsr->stage = UsAdsrStageSustain;
                }
            }
        }
        // Sustain
        case UsAdsrStageSustain: {
            return adsr->sustain;
        }
        // Release
        case UsAdsrStageRelease: {
            if (us_tuner_rotate_check_wrap(&adsr->tuner)) {
                adsr->stage = UsAdsrStageOff;
                return 0;
            }
            else {
                uint32_t d = us_adsr_bang_to_wave(adsr);
                if (d > adsr->vol) {
                    adsr->stage = UsAdsrStageOff;
                    return 0;
                }
                return adsr->vol - d;
            }
        }
        default: {
            return 0;
        }
    }
}
