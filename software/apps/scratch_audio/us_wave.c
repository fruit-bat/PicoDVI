#include "us_wave.h"

// Needed for the sin tables
// TODO split this out from the other tables
#include "u_synth_tone_tables.h"

inline static int32_t us_lerp_n(
    const int32_t s1,           // Sample 1
    const int32_t s2,           // Sample 2
    const uint32_t n_bit_frac,  // n bits of fraction
    const uint32_t n            // number of fraction bits
) {
    const int32_t d = __mul_instruction(s2 - s1, n_bit_frac) >> n;
    return s1 + d;
}

#define US_SIN_SAMPLES_LOG2 7L

inline static int32_t us_wave_sin_sample_q(
    const uint32_t qang,
    const uint32_t qind
) {
    switch(qang) {
        case 0: return us_sin[qind];
        case 1: return us_sin[(1 << US_SIN_SAMPLES_LOG2) - qind];
        case 2: return - us_sin[qind];
        default: return - us_sin[(1 << US_SIN_SAMPLES_LOG2) - qind];
    }
}

inline static int32_t us_wave_sin_sample(
    const uint32_t sang
) {
    const uint32_t qang = sang >> US_SIN_SAMPLES_LOG2;
    const uint32_t qind = sang & ((1L << US_SIN_SAMPLES_LOG2) - 1L);
    return us_wave_sin_sample_q(qang, qind);
}

int32_t __not_in_flash_func(us_wave_sin_lerp)(
    const uint32_t bang
) {
    const uint32_t zang = bang >> (32 - (2 + US_SIN_SAMPLES_LOG2 + 8));
    const uint32_t fang = zang & 255;
    const uint32_t sang1 = zang >> 8;
    const int32_t s1 = us_wave_sin_sample(sang1);
    const uint32_t sang2 = (sang1 + 1) & ((1 << (2 + US_SIN_SAMPLES_LOG2)) - 1);
    const int32_t s2 = us_wave_sin_sample(sang2);
    return us_lerp_n(s1, s2, fang, 8);
}

int32_t __not_in_flash_func(us_wave_sin)(
    const uint32_t bang
) {
    const uint32_t sang = bang >> (32 - (2 + US_SIN_SAMPLES_LOG2));
    return us_wave_sin_sample(sang);
}

int32_t __not_in_flash_func(us_wave_saw)(
    const uint32_t bang
) {
    const uint32_t qang = bang >> (32 - 2);
    const uint32_t qind = (bang >> (32 - 2 - 15)) && 32767;
    switch(qang) {
        case 0: return qind;
        case 1: return 32767 - qind;
        case 2: return - qind;
        default: return qind - 32768;
    }
}

int32_t __not_in_flash_func(us_wave_square)(
    const uint32_t bang
) {
    const uint32_t qang = bang >> (32 - 1);
    return qang ? 32767 : - 32768;
}

int32_t __not_in_flash_func(us_wave_ramp_up)(
    const uint32_t bang
) {
    const uint32_t qang = bang >> 16;
    return qang > 32767 ? 32767 : qang;
}

int32_t __not_in_flash_func(us_wave_ramp_down)(
    const uint32_t bang
) {
    const int32_t qang = 32767L - (int32_t)(bang >> 16);
    return qang < 0 ? 0 : qang;
}
