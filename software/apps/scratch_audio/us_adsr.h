#pragma once
#include "us_adsr.h"
#include "us_wave.h"

enum UsAdsrStage {
    UsAdsrStageOff = 0,
    UsAdsrStageAttack,
    UsAdsrStageDecay,
    UsAdsrStageSustain,
    UsAdsrStageRelease,
};

typedef struct {
    UsPitch attack;    // Attack time
    UsPitch decay;     // Decay time
    uint32_t sustain;  // Susstain level
    UsPitch release;   // Release time

    uint8_t stage;        // the current stage
    UsTuner tuner;        // the current stage timer
    UsWaveFunc wave_func; // the current wave function
    uint32_t vang;        // the current volume angle
    uint32_t vol;         // the current volume
} UsAdsr;

void us_adsr_init(
    UsAdsr *adsr
);

void us_adsr_attack(
    UsAdsr *adsr
);

void us_adsr_release(
    UsAdsr *adsr
);

int32_t us_adsr_update(
    UsAdsr *adsr
);

bool inline us_adsr_is_off(
    UsAdsr *adsr
) {
    return adsr->stage == UsAdsrStageOff;
}