#include "us_channel.h"
#include "us_debug.h"

static UsUint8DlistEntry * __not_in_flash_func(get_patch_state_entry)(void* entries, uint8_t patch_index)  {
    UsChannelPatchState *patch_state = entries;
    return &patch_state[patch_index].links;
}

inline void* get_patch_data(UsChannel *channel, uint8_t patch_index) {
    uint8_t* patch_data = channel->patch_data;
    return patch_data + (channel->patch_data_size * patch_index);
}

inline bool us_channel_active(UsChannel *channel) {
    return channel->patch_state_lists[UsPatchStateRelease].head != US_UINT8_DLIST_NULL ||
        channel->patch_state_lists[UsPatchStateOn].head != US_UINT8_DLIST_NULL;
}

inline bool us_channel_inactive(UsChannel *channel) {
    return channel->patch_state_lists[UsPatchStateRelease].head == US_UINT8_DLIST_NULL &&
        channel->patch_state_lists[UsPatchStateOn].head == US_UINT8_DLIST_NULL;
}

void us_channel_init(
    UsChannel *channel,
    uint8_t id, 
    void* callback_data, 
    void (*on)(void *data, uint8_t id), 
    void (*off)(void *data, uint8_t id)) {
    channel->out_l = 0;
    channel->out_r = 0;
    channel->bend = 0;
    channel->gain = 64; // Half volume
    channel->pan = 64; // Centre
    channel->patch_count = 0;
    channel->patch_callbacks.off = us_channel_patch_cb_off;
    channel->patch_callbacks.release = us_channel_patch_cb_release;
    channel->id = id;
    channel->callback_data = callback_data;
    channel->callback_on = on;
    channel->callback_off = off;
}

void us_channel_set_patch(
    UsChannel *channel,
    void (*init_patch)(UsPatch *patch),
    void * patch_config,
    void * patch_data,
    size_t patch_data_size,
    UsChannelPatchState *patch_state,
    uint32_t patch_count) {

    init_patch(&channel->patch);
    channel->patch_data = patch_data;
    channel->patch_state = patch_state;
    channel->patch_data_size = patch_data_size;
    channel->patch_count = patch_count;
    channel->patch_config = patch_config;

    us_uint8_dlist_anchor_init_array(channel->patch_state_lists, UsPatchStateCount);

    uint8_t* data = patch_data;
    UsUint8DlistAnchor *patch_state_list_off = &channel->patch_state_lists[UsPatchStateOff];

    for(uint32_t patch_index = 0; patch_index < patch_count; ++patch_index) {
        channel->patch.init_data(data, patch_config);
        data += patch_data_size;

        UsChannelPatchState *patch_state = &channel->patch_state[patch_index];

        patch_state->status = UsPatchStateOff;
        patch_state->note = US_NOT_A_NOTE;

        us_uint8_dlist_entry_init(
            &patch_state->links,
            patch_index);

        us_uint8_dlist_link_head(
            patch_state_list_off,
            channel->patch_state,
            get_patch_state_entry,
            patch_index
        );
    }

    for(uint32_t note_index = 0; note_index < US_NOTE_COUNT; ++note_index) {
        channel->notes[note_index] = US_NOT_A_NOTE;
    }
}

static UsChannelPatchState * __not_in_flash_func(us_channel_relink_patch_state)(
    UsChannel* channel,
    uint8_t patch_index,
    uint8_t patch_status
) {
    US_DEBUG("US_CHANNEL: relinking pi %u, status %u\n", patch_index, patch_status);

    UsChannelPatchState *patch_state = &channel->patch_state[patch_index];
    us_uint8_dlist_unlink(
        &channel->patch_state_lists[patch_state->status],
        channel->patch_state,
        get_patch_state_entry,
        patch_index
    );
    patch_state->status = patch_status;
    us_uint8_dlist_link_head(
        &channel->patch_state_lists[patch_state->status],
        channel->patch_state,
        get_patch_state_entry,
        patch_index
    );
    US_DEBUG("US_CHANNEL: relinked pi %u, status %u\n", patch_index, patch_status);

    return patch_state;
}

void __not_in_flash_func(us_channel_note_on)(UsChannel* channel, uint32_t note, uint32_t velocity) {
    if (channel->patch_count) {
        uint8_t patch_index = channel->notes[note];
        if (patch_index != US_NOT_A_NOTE) {
            us_channel_note_release(channel, note, 0);
        }

        UsUint8DlistAnchor *patch_state_list_off = &channel->patch_state_lists[UsPatchStateOff];
        if (us_uint8_dlist_is_empty(patch_state_list_off)) {
            UsUint8DlistAnchor *patch_state_list_release = &channel->patch_state_lists[UsPatchStateRelease];
            if (us_uint8_dlist_is_empty(patch_state_list_release)) {
                // Failure to allocate the note to a patch instance
                US_DEBUG("US_CHANNEL: can't play note %lu\n", note);
                return;
            }
            else {
                patch_index = patch_state_list_release->tail;
            }
        }
        else {
            patch_index = patch_state_list_off->tail;
        }

        if (patch_index == US_NOT_A_NOTE) return;

        US_DEBUG("US_CHANNEL: playing note %lu on patch %u\n", note, patch_index);

        channel->notes[note] = patch_index;

        if (us_channel_inactive(channel)) {
            channel->callback_on(channel->callback_data, channel->id);
        }

        // Move the patch instance into the 'on' list
        us_channel_relink_patch_state(
            channel,
            patch_index,
            UsPatchStateOn
        )->note = note;


        // Tell the patch to turn on a note
        channel->patch.note_on(
            get_patch_data(channel, patch_index),
            channel->patch_config,
            note,
            channel->bend,
            velocity
        );
    }
}

void us_channel_note_release(UsChannel* channel, uint32_t note, uint32_t velocity) {
    if (channel->patch_count) {
        uint8_t patch_index = channel->notes[note];
        if (patch_index != US_NOT_A_NOTE)
        {
            UsChannelPatchState *patch_state = &channel->patch_state[patch_index];
            if (patch_state->note == note) {

                US_DEBUG("US_CHANNEL: releasing note %lu\n", note);

               channel->patch.note_release(
                    get_patch_data(channel, patch_index),
                    channel->patch_config,
                    velocity
                );

                us_channel_relink_patch_state(
                    channel,
                    patch_index,
                    UsPatchStateRelease
                );

            }
            channel->notes[note] = US_NOT_A_NOTE;
        }
    }
}

void __not_in_flash_func(us_channel_bend)(UsChannel* channel, int32_t bend) {
    channel->bend = bend;
    if (channel->patch_count) {
        // Update all of the patch instances on the 'release' list
        {
            uint8_t patch_index = channel->patch_state_lists[UsPatchStateRelease].head;
            while(patch_index != US_UINT8_DLIST_NULL) {
                UsChannelPatchState *patch_state = &channel->patch_state[patch_index];
                channel->patch.bend(
                    get_patch_data(channel, patch_index), // The data the patch needs to function
                    channel->patch_config,
                    patch_state->note,
                    bend
                );
                patch_index = patch_state->links.next;
            }
        }

        // Update all of the patch instances on the 'on' list
        {
            uint8_t patch_index = channel->patch_state_lists[UsPatchStateOn].head;
            while(patch_index != US_UINT8_DLIST_NULL) {
                UsChannelPatchState *patch_state = &channel->patch_state[patch_index];
                channel->patch.bend(
                    get_patch_data(channel, patch_index), // The data the patch needs to function
                    channel->patch_config,
                    patch_state->note,
                    bend
                );
                patch_index = patch_state->links.next;
            }
        }
    }
}

void __not_in_flash_func(us_channel_update)(UsChannel *channel) {
    int32_t out = 0;
    if (channel->patch_count) {
        UsPatchCallbacks *patch_callbacks = &channel->patch_callbacks;
        void* config = channel->patch_config;

        // Update all of the patch instances on the 'release' list
        {
            uint8_t patch_index = channel->patch_state_lists[UsPatchStateRelease].head;
            while(patch_index != US_UINT8_DLIST_NULL) {
                uint8_t patch_index_next = channel->patch_state[patch_index].links.next;
                out += channel->patch.update(
                    get_patch_data(channel, patch_index), // The data the patch needs to function
                    config,          // The config common to all voices with this patch
                    patch_callbacks, // Callbacks so the patch can report its state back to the channel
                    channel,         // The channel data to be used in the patch callbacks
                    patch_index      // The patch index for use in the callback
                );
                patch_index = patch_index_next;
            }
        }

        // Update all of the patch instances on the 'on'' list
        {
            uint8_t patch_index = channel->patch_state_lists[UsPatchStateOn].head;
            while(patch_index != US_UINT8_DLIST_NULL) {
               uint8_t patch_index_next = channel->patch_state[patch_index].links.next;
               out += channel->patch.update(
                    get_patch_data(channel, patch_index), // The data the patch needs to function
                    config,          // The config common to all voices with this patch
                    patch_callbacks, // Callbacks so the patch can report its state back to the channel
                    channel,         // The channel data to be used in the patch callbacks
                    patch_index      // The patch index for use in the callback
                );
                patch_index = patch_index_next;
            }
        }
    }
    channel->out_l =  __mul_instruction(out, __mul_instruction(channel->gain, (128 - channel->pan)));
    channel->out_r =  __mul_instruction(out, __mul_instruction(channel->gain, channel->pan));
}

// Optional callback
void __not_in_flash_func(us_channel_patch_cb_release)(void *d, uint32_t patch_index) {
    UsChannel *channel = (UsChannel *)d;

    // Move the patch instance into the 'release' list
    us_channel_relink_patch_state(
        channel,
        patch_index,
        UsPatchStateRelease
    );
}

// Mandatory callback
void __not_in_flash_func(us_channel_patch_cb_off)(void *d, uint32_t patch_index) {
    US_DEBUG("US_CHANNEL: off callback %lu\n", patch_index);

    UsChannel *channel = (UsChannel *)d;
    // Move the patch instance into the 'off' list
    UsChannelPatchState *patch_state = us_channel_relink_patch_state(
        channel,
        patch_index,
        UsPatchStateOff
    );
    patch_state->note = US_NOT_A_NOTE;

    if (us_channel_inactive(channel)) {
        channel->callback_off(channel->callback_data, channel->id);
    }
}
