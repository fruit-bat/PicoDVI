#pragma once
#include "us_channel.h"
#include "us_uint8_dlist.h"

#define US_CHANNEL_COUNT_LOG2 4L
#define US_CHANNEL_COUNT (1 << US_CHANNEL_COUNT_LOG2)

typedef struct {
    UsChannel channel[US_CHANNEL_COUNT];
    UsUint8DlistAnchor active_channels;
    UsUint8DlistEntry channel_links[US_CHANNEL_COUNT];
    int32_t out_l;
    int32_t out_r;
} UsChannels;

void us_channels_init(UsChannels *channels);

inline UsChannel *us_channels_get(UsChannels *channels, uint32_t g) {
    return g < US_CHANNEL_COUNT ? &channels->channel[g] : NULL;
}

void us_channels_update(UsChannels* channels);