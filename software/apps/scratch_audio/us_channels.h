#pragma once
#include "us_channel.h"

#define US_CHANNEL_COUNT_LOG2 1L
#define US_CHANNEL_COUNT (1 << US_CHANNEL_COUNT_LOG2)

typedef struct {
    UsChannel channel[US_CHANNEL_COUNT];
} UsChannels;

void us_channels_init(UsChannels *channels);

inline UsChannel *us_channels_get(UsChannels *channels, uint32_t g) {
    return g < US_CHANNEL_COUNT ? &channels->channel[g] : NULL;
}

int32_t us_channels_update(UsChannels* channels);