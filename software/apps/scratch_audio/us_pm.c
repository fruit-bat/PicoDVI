#include "us_pm.h"


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
    sequencer->clock.fips = __mul_instruction(ppq, US_US_PER_SAMPLE_BITS);
}

UsPmCursor __not_in_flash_func(us_pm_step)(
    UsPmSequencer *sequencer
) {
    UsPmCursor cursor = sequencer->cursor;   
    if (cursor == NULL) return cursor;
    UsVoices *voices = sequencer->voices;
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
        case SynCmdOn: { // voice, key, velociy
            const uint32_t i = cursor[1];
            const uint32_t k = cursor[2];
            const uint32_t v = cursor[3];
            if (i < US_VOICE_COUNT) {
                us_voice_note_on(&voices->voice[i], k, v);
            }
            cursor += SynCmdOnLen;
            break;
        }
        case SynCmdOff: {
            const uint32_t i = cursor[1];
            const uint32_t v = cursor[3];            
            if (i < US_VOICE_COUNT) {
                us_voice_note_off(&voices->voice[i], v);
            }
            cursor += SynCmdOffLen;
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
    UsVoices *voices,
    UsPmCursor sequence,
    bool repeat
) {
    us_tuner_reset(&sequencer->clock);
    us_pm_set_ppq(sequencer, 384L);
    sequencer->clock.eips = US_US_PER_SAMPLE_NEXP;
    sequencer->tempo = 600000L;
    sequencer->cursor = sequence;
    sequencer->sequence = sequence;
    sequencer->ticks = 0;
    sequencer->voices = voices;
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
