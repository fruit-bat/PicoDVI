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

inline static int32_t us_wave_quater_sample_q(
    const uint32_t qang,
    const uint32_t qind,
    const uint16_t* samples,
    const uint32_t sample_count
) {
    switch(qang) {
        case 0: return samples[qind];
        case 1: return samples[sample_count - qind];
        case 2: return - samples[qind];
        default: return - samples[sample_count - qind];
    }
}

inline static int32_t us_wave_half_sample_h(
    const uint32_t hang,
    const uint32_t hind,
    const uint16_t* samples,
    const uint32_t sample_count
) {
    return hang ? -samples[sample_count - hind] : samples[hind];
}

inline static int32_t us_wave_quarter_sample(
    const uint32_t sang,
    const uint16_t* samples,
    const uint32_t sample_count_ln2
) {
    const int32_t sample_count = 1L << sample_count_ln2;
    const uint32_t qang = sang >> sample_count_ln2;
    const uint32_t qind = sang & (sample_count - 1L);
    return us_wave_quater_sample_q(qang, qind, samples, sample_count);
}

inline static int32_t us_wave_half_sample(
    const uint32_t sang,
    const uint16_t* samples,
    const uint32_t sample_count_ln2
) {
    const int32_t sample_count = 1L << sample_count_ln2;
    const uint32_t qang = sang >> sample_count_ln2;
    const uint32_t qind = sang & (sample_count - 1L);
    return us_wave_half_sample_h(qang, qind, samples, sample_count);
}

inline static int32_t us_wave_quarter_lerp(
    const uint32_t bang,
    const uint16_t* samples,
    const uint32_t sample_count_ln2
) {
    const uint32_t zang = bang >> (32 - (2 + sample_count_ln2 + 8));
    const uint32_t fang = zang & 255;
    const uint32_t sang1 = zang >> 8;
    const int32_t s1 = us_wave_quarter_sample(sang1, samples, sample_count_ln2);
    const uint32_t sang2 = (sang1 + 1) & ((1 << (2 + sample_count_ln2)) - 1);
    const int32_t s2 = us_wave_quarter_sample(sang2, samples, sample_count_ln2);
    return us_lerp_n(s1, s2, fang, 8);
}

inline static int32_t us_wave_half_lerp(
    const uint32_t bang,
    const uint16_t* samples,
    const uint32_t sample_count_ln2
) {
    const uint32_t zang = bang >> (32 - (1 + sample_count_ln2 + 8));
    const uint32_t fang = zang & 255;
    const uint32_t sang1 = zang >> 8;
    const int32_t s1 = us_wave_half_sample(sang1, samples, sample_count_ln2);
    const uint32_t sang2 = (sang1 + 1) & ((1 << (1 + sample_count_ln2)) - 1);
    const int32_t s2 = us_wave_half_sample(sang2, samples, sample_count_ln2);
    return us_lerp_n(s1, s2, fang, 8);
}

int32_t __not_in_flash_func(us_wave_sin_lerp)(
    const uint32_t bang
) {
    return us_wave_quarter_lerp(bang, us_sin, US_SIN_SAMPLES_LOG2);
}

int32_t __not_in_flash_func(us_wave_sin)(
    const uint32_t bang
) {
    const uint32_t sang = bang >> (32 - (2 + US_SIN_SAMPLES_LOG2));
    return us_wave_quarter_sample(sang, us_sin, US_SIN_SAMPLES_LOG2);
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

// Example quarter wave samples for lerping
const uint16_t us_wave_not_square_samples[] = {
    0,
    15000,
    17000,
    30000,
    32767
};

// Example quarter wave samples for lerping
const uint16_t us_wave_half_not_square_samples[] = {
    0,
    15000,
    17000,
    30000,
    32000,
    32000,
    20000,
    10000,
    400
};

int32_t __not_in_flash_func(us_wave_not_square_lerp)(
    const uint32_t bang
) {
    //return us_wave_half_lerp(bang, us_wave_not_square_samples, 2);
    return us_wave_half_lerp(bang, us_wave_half_not_square_samples, 3);
}
