#include "us_channels.h"
#include "us_debug.h"

void us_channels_init(UsChannels *channels) {
    US_DEBUG("US_PM: us_channels_init\n");
    us_uint8_dlist_anchor_init(&channels->active_channels);
    channels->out_l = 0;
    channels->out_r = 0;
    for(uint8_t i = 0; i < US_CHANNEL_COUNT; ++i) {
        us_channel_init(&channels->channel[i], i);
        us_uint8_dlist_entry_init(&channels->channel_links[i], i);
    }
}

void __not_in_flash_func(us_channels_on_cb)(void *data, uint8_t id) {
    UsChannels *channels = (UsChannels *)data;

}

void __not_in_flash_func(us_channels_update)(UsChannels *channels) {
    int32_t out_l = 0;
    int32_t out_r = 0;
    for(uint8_t i = 0; i < US_CHANNEL_COUNT; ++i) {
        UsChannel * const channel = &channels->channel[i];
        us_channel_update(channel);
        out_l += channel->out_l;
        out_r += channel->out_r;
    }
    out_l >>= 13;
    out_r >>= 13;
    if (out_l > 32767) out_l = 32767;
    if (out_l < -32768) out_l = -32768;
    if (out_r > 32767) out_r = 32767;
    if (out_r < -32768) out_r = -32768;
    channels->out_l = out_l;
    channels->out_r = out_r;
}
