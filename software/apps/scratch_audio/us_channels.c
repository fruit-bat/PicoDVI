#include "us_channels.h"

void us_channels_init(UsChannels *channels) {
    for(int32_t i = 0; i < US_CHANNEL_COUNT; ++i) {
        us_channel_init(&channels->channel[i]);
    }
}