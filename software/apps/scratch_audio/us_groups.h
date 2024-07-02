#pragma once
#include "us_group.h"

#define US_GROUP_COUNT_LOG2 4L
#define US_GROUP_COUNT (1 << US_GROUP_COUNT_LOG2)

typedef struct {
    UsGroup group[US_GROUP_COUNT];
} UsGroups;

void us_groups_init(UsGroups *groups);

inline UsGroup *us_groups_get(UsGroups *groups, uint32_t g) {
    return &groups->group[g < US_GROUP_COUNT ? g : 0];
}
