#pragma once
#include "pico/stdlib.h"

enum InvSampleInd {
    InvSampleShootInd = 0,
    InvSampleWalk1Ind,
    InvSampleWalk2Ind,
    InvSampleWalk3Ind,
    InvSampleDeathInd,
    InvSampleWoopInd,
    InvSampleHitInd,
    InvSampleCount
};

uint32_t inv_sample_count();
const int16_t *inv_sample(uint32_t i);
uint32_t inv_sample_size(uint32_t i);