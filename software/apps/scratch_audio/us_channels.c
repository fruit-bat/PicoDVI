#include "us_channels.h"
#include "us_debug.h"

void us_channels_init(UsChannels *channels) {
    US_DEBUG("US_PM: us_channels_init\n");

    for(int32_t i = 0; i < US_CHANNEL_COUNT; ++i) {
        us_channel_init(&channels->channel[i]);
    }
}

int32_t us_channels_update(UsChannels *channels) {
    int32_t out = 0;
    for(int32_t i = 0; i < US_CHANNEL_COUNT; ++i) {
        out += us_channel_update(&channels->channel[i]);
    }
    if (out > 32767) return 32767;
    if (out < -32768) return -32768;    
    return out;
}
