#pragma once
#include "us_adsr.h"
#include "us_wave.h"
#include "us_patch.h"

enum UsAdsrStage {
    UsAdsrStageOff = 0,
    UsAdsrStageAttack,
    UsAdsrStageDecay,
    UsAdsrStageSustain,
    UsAdsrStagePreRelease,
    UsAdsrStageRelease,
};

typedef struct {
    UsPitch attack;    // Attack time
    UsPitch decay;     // Decay time
    uint32_t sustain;  // Susstain level
    UsPitch release;   // Release time
} UsAdsrConfig;

typedef struct {
    UsAdsrConfig *config; // ADSR configuration
    uint32_t velocity;    // the key press velocity
    uint8_t stage;        // the current stage
    UsTuner tuner;        // the current stage timer
    UsWaveFunc wave_func; // the current wave function
    uint32_t vang;        // the current volume angle
    uint32_t vol;         // the current volume
} UsAdsr;

void us_adsr_config_init(
    UsAdsrConfig *adsr
);

void us_adsr_init(
    UsAdsr *adsr,
    UsAdsrConfig *adsr_config
);

void us_adsr_attack(
    UsAdsr *adsr,
    uint32_t velocity
);

void us_adsr_release(
    UsAdsr *adsr
);

int32_t us_adsr_update(
    UsAdsr *adsr,
    UsPatchCallbacks* callbacks,
    void *callback_data,
    uint32_t callback_id
);

bool inline us_adsr_is_off(
    UsAdsr *adsr
) {
    return adsr->stage == UsAdsrStageOff;
}