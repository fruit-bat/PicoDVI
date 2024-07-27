#include "us_adsr.h"

void us_adsr_config_init(
    UsAdsrConfig *adsr
) {
    // Some vaugly pleasant values
    adsr->sustain = 16384;
    adsr->attack.fips = 1246611822UL;
    adsr->attack.eips = 9;
    adsr->decay.fips = 1246611822UL;
    adsr->decay.eips = 15;
    adsr->release.fips = 1246611822UL;
    adsr->release.eips = 14; 
}

void __not_in_flash_func(us_adsr_init)(
    UsAdsr *adsr,
    UsAdsrConfig *adsr_config
) {
    adsr->stage = UsAdsrStageOff;
    adsr->config = adsr_config;
    adsr->velocity = 0;
}

void __not_in_flash_func(us_adsr_attack)(
    UsAdsr *adsr,
    uint32_t velocity
) {
    adsr->stage = UsAdsrStageAttack;
    us_tuner_reset_phase(&adsr->tuner);
    us_tuner_set_pitch(&adsr->tuner, &adsr->config->attack);
    adsr->wave_func = us_wave_ramp_up;
    adsr->vol = 0;
    adsr->velocity = velocity;
}

void __not_in_flash_func(us_adsr_release)(
    UsAdsr *adsr
) {
    adsr->stage = UsAdsrStagePreRelease;
    us_tuner_reset_phase(&adsr->tuner);
    us_tuner_set_pitch(&adsr->tuner, &adsr->config->release);
    adsr->wave_func = us_wave_ramp_up;
}

inline int32_t us_adsr_bang_to_wave(
    UsAdsr *adsr
) {
    return adsr->wave_func(adsr->tuner.bang);
}

int32_t __not_in_flash_func(us_adsr_update)(
    UsAdsr *adsr,
    UsPatchCallbacks* callbacks,
    void *callback_data,
    uint32_t callback_id
) {
    switch(adsr->stage) {
        // Off
        case UsAdsrStageOff: {
            if (callbacks) callbacks->off(callback_data, callback_id);
            return 0;
        }
        // Attack
        case UsAdsrStageAttack: {
            if (us_tuner_rotate_check_wrap(&adsr->tuner)) {
                adsr->vol = 32767;
                adsr->stage = UsAdsrStageDecay;
                us_tuner_reset_phase(&adsr->tuner);
                us_tuner_set_pitch(&adsr->tuner, &adsr->config->decay);                
                adsr->wave_func = us_wave_ramp_down;
            }
            else {
                adsr->vol = us_adsr_bang_to_wave(adsr);
            }
            return __mul_instruction(adsr->vol, adsr->velocity) >> 8;
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
                if (adsr->vol <= adsr->config->sustain){
                    adsr->stage = UsAdsrStageSustain;
                }
            }
        }
        // Sustain
        case UsAdsrStageSustain: {
            return __mul_instruction(adsr->vol, adsr->velocity) >> 8;
        }
        // Pre Release
        case UsAdsrStagePreRelease: {
            // if (callbacks) callbacks->release(callback_data, callback_id);
            adsr->stage = UsAdsrStageRelease;
            // Should drop through into release
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
                    if (callbacks) callbacks->off(callback_data, callback_id);
                    return 0;
                }
                return __mul_instruction(adsr->vol - d, adsr->velocity) >> 8;
            }
        }
        default: {
            return 0;
        }
    }
}
