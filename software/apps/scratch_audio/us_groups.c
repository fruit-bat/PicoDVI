#include "us_groups.h"

void us_groups_init(UsGroups *groups) {
    for(int32_t i = 0; i < US_GROUP_COUNT; ++i) {
        us_group_init(&groups->group[i]);
    }
}