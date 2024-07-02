#pragma once
#include <pico/stdlib.h>

typedef struct  {
    int32_t bend;
} UsGroup;

inline void us_group_init(UsGroup *group) {
    group->bend = 0;
}