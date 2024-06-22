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
