#include "us_lpf.h"

void us_lpf_init(UsLpf *filter, uint32_t alpha) {
    filter->prev = 0;
    filter->alpha = alpha;
}

int32_t __not_in_flash_func(us_lpf_sample)(UsLpf *filter, int32_t s) {
    const uint32_t alpha = filter->alpha;
    if (alpha >= 1<<16) return s;
    const int32_t a1 = __mul_instruction(s, alpha) >> 15;
    const int32_t a2 = __mul_instruction(filter->prev, (65536 - alpha)) >> 15;
    const int32_t a3 = (a1 + a2) >> 1;
    filter->prev = a3;
    return a3;
}
