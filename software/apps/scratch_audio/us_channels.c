#include "us_channels.h"
#include "us_debug.h"

UsUint8DlistEntry * __not_in_flash_func(us_channel_get_link)(void *entries, uint8_t index){
    UsUint8DlistEntry* channel_links = (UsUint8DlistEntry*)entries;
    return &channel_links[index];
}

static void __not_in_flash_func(us_channels_on_cb)(void *data, uint8_t id) {
    UsChannels *channels = (UsChannels *)data;

    US_DEBUG("US_CHANNELS: on %u\n", id);

    us_uint8_dlist_link_head(
        &channels->active_channels,
        &channels->channel_links,
        us_channel_get_link,
        id
    );
}

static void __not_in_flash_func(us_channels_off_cb)(void *data, uint8_t id) {
    UsChannels *channels = (UsChannels *)data;

    US_DEBUG("US_CHANNELS: off %u\n", id);

    us_uint8_dlist_unlink(
        &channels->active_channels,
        &channels->channel_links,
        us_channel_get_link,
        id
    );
}

void us_channels_init(UsChannels *channels) {
    US_DEBUG("US_PM: us_channels_init\n");
    us_uint8_dlist_anchor_init(&channels->active_channels);
    channels->out_l = 0;
    channels->out_r = 0;
    for(uint8_t i = 0; i < US_CHANNEL_COUNT; ++i) {
        us_channel_init(&channels->channel[i], i, channels, us_channels_on_cb, us_channels_off_cb);
        us_uint8_dlist_entry_init(&channels->channel_links[i], i);
    }
}

void __not_in_flash_func(us_channels_update)(UsChannels *channels) {
    int32_t out_l = 0;
    int32_t out_r = 0;
    // Update all of the channels on the 'on' list
    uint8_t ci = channels->active_channels.head;
    while(ci != US_UINT8_DLIST_NULL) {
        uint8_t ci_next = channels->channel_links[ci].next;
        UsChannel * const channel = &channels->channel[ci];
        // Note, this can remove a channel from the on list
        us_channel_update(channel);
        out_l += channel->out_l;
        out_r += channel->out_r;
        ci = ci_next;
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
