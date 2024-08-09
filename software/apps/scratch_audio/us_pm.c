#include "us_pm.h"
#include "us_debug.h"


// TODO From synth_tone-tables.h 
// TODO derived from 44100 sample rate so can't be in here
#define US_US_PER_SAMPLE_BITS 46439L
#define US_US_PER_SAMPLE_NEXP 11L

inline uint32_t us_pm_word8(UsPmCursor cursor) {
    return cursor[0];
}

inline uint32_t us_pm_word16(UsPmCursor cursor) {
    uint32_t r = cursor[0];
    r <<= 8;
    r |= cursor[1];
    return r;
}

inline int32_t us_pm_int16(UsPmCursor cursor) {
    int16_t r = cursor[0];
    r <<= 8;
    r |= cursor[1];
    return (int32_t)r;
}

inline uint32_t us_pm_word24(UsPmCursor cursor) {
    uint32_t r = cursor[0];
    r <<= 8;
    r |= cursor[1];
    r <<= 8;
    r |= cursor[2];    
    return r;
}

static inline void us_pm_set_ppq(
    UsPmSequencer *sequencer,
    uint32_t ppq
) {
    sequencer->clock.pitch.fips = __mul_instruction(ppq, US_US_PER_SAMPLE_BITS);
}

UsPmCursor __not_in_flash_func(us_pm_step)(
    UsPmSequencer *sequencer
) {
    UsPmCursor cursor = sequencer->cursor;   
    if (cursor == NULL) return cursor;
    UsChannels *channels = sequencer->channels;
    const uint8_t type = *cursor;
    switch(type) {
        case SynCmdPPQ: {
            us_pm_set_ppq(sequencer, us_pm_word16(cursor + 1));
            cursor += SynCmdPPQLen;
            break;
        }
        case SynCmdTempo: {
            sequencer->tempo = us_pm_word24(cursor + 1);
            cursor += SynCmdTempoLen;
            break;
        }
        case SynCmdOn: { // channel, key, velociy
            const uint32_t c = cursor[1];
            const uint32_t k = cursor[2];
            const uint32_t v = cursor[3];

            US_DEBUG("US_PM: channel %ld, note on %ld, velocity %ld\n", c, k, v);

            UsChannel *channel = us_channels_get(channels, c);

            if (channel) us_channel_note_on(channel, k, v);

            cursor += SynCmdOnLen;
            break;
        }
        case SynCmdOff: {
            const uint32_t c = cursor[1];
            const uint32_t k = cursor[2];
            const uint32_t v = cursor[3];

            US_DEBUG("US_PM: channel %ld, note off %ld, velocity %ld\n", c, k, v);

            UsChannel *channel = us_channels_get(channels, c);

            if (channel) us_channel_note_release(channel, k, v);

            cursor += SynCmdOffLen;
            break;
        }
        case SynCmdBend: {
            const uint32_t c = cursor[1];
            const int32_t bend = us_pm_int16(cursor + 1);

            US_DEBUG("US_PM: channel %ld, bend %ld\n", c, bend);

            UsChannel *channel = us_channels_get(channels, c);

            if (channel) us_channel_bend(channel, bend);

            cursor += SynCmdBendLen;
            break;
        }        
        case SynCmdTime: {
            sequencer->ticks += us_pm_word16(cursor + 1);
            cursor += SynCmdTimeLen;
            break;
        }
        default: {
            cursor = sequencer->repeat ? sequencer->sequence :  NULL;
            break;
        }
        break;
    }
    return cursor;
}

void us_pm_sequencer_init(
    UsPmSequencer *sequencer,
    UsChannels *channels,
    UsPmCursor sequence,
    bool repeat
) {
    us_tuner_reset_phase(&sequencer->clock);
    us_pm_set_ppq(sequencer, 384L);
    sequencer->clock.pitch.eips = US_US_PER_SAMPLE_NEXP;
    sequencer->tempo = 600000L;
    sequencer->cursor = sequence;
    sequencer->sequence = sequence;
    sequencer->ticks = 0;
    sequencer->channels = channels;
    sequencer->repeat = repeat;
}

// Called every sample
void __not_in_flash_func(us_pm_sequencer_update)(
    UsPmSequencer *sequencer
) {
    if (sequencer->cursor == NULL) return;
    us_tuner_rotate(&sequencer->clock);
    while (sequencer->clock.bang > sequencer->tempo) {
        --sequencer->ticks;
        sequencer->clock.bang -= sequencer->tempo;
    }
    while (sequencer->ticks <= 0 && sequencer->cursor != NULL) {
        sequencer->cursor = us_pm_step(sequencer);
    }
}
