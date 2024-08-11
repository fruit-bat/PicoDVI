#include "inv_samples.h"
#include "inv_shoot_int16.h"
#include "inv_walk1_int16.h"
#include "inv_walk2_int16.h"
#include "inv_walk3_int16.h"
#include "inv_death_int16.h"
#include "inv_woop_int16.h"
#include "inv_hit_int16.h"



uint32_t inv_sample_count() {
    return InvSampleCount;
}

const int16_t *inv_sample(uint32_t i) {
    switch (i) {
        case InvSampleShootInd: return (const int16_t *)inv_shoot_int16;
        case InvSampleWalk1Ind: return (const int16_t *)inv_walk1_int16;
        case InvSampleWalk2Ind: return (const int16_t *)inv_walk2_int16;
        case InvSampleWalk3Ind: return (const int16_t *)inv_walk3_int16;
        case InvSampleDeathInd: return (const int16_t *)inv_death_int16;
        case InvSampleWoopInd: return (const int16_t *)inv_woop_int16;
        case InvSampleHitInd: return (const int16_t *)inv_hit_int16;
        default: return NULL;
    }
}

uint32_t inv_sample_size(uint32_t i) {
    switch (i) {
        case InvSampleShootInd: return sizeof(inv_shoot_int16)/2;
        case InvSampleWalk1Ind: return sizeof(inv_walk1_int16)/2;
        case InvSampleWalk2Ind: return sizeof(inv_walk2_int16)/2;
        case InvSampleWalk3Ind: return sizeof(inv_walk3_int16)/2;
        case InvSampleDeathInd: return sizeof(inv_death_int16)/2;
        case InvSampleWoopInd: return sizeof(inv_woop_int16)/2;
        case InvSampleHitInd: return sizeof(inv_hit_int16)/2;        
        default: return 0;
    }
}