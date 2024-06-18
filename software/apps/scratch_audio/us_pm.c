#include "us_pm.h"


UsPmCursor us_pm_step(
    UsPmCursor cursor,
    UsPmMessage *message
) {
    if (cursor == NULL) return cursor;
    const uint8_t type = *cursor;
    message->type = type;
    switch(type) {
        case SynCmdPPQ: {

            cursor += SynCmdPPQLen;
            break;
        }
        case SynCmdTempo: {

            cursor += SynCmdTempoLen;
            break;
        }
        case SynCmdOn: {

            cursor += SynCmdOnLen;
            break;
        }
        case SynCmdOff: {

            cursor += SynCmdOffLen;
            break;
        }
        case SynCmdTime: {

            cursor += SynCmdTimeLen;
            break;
        }
        default: {

            cursor = NULL;
            break;
        }
        break;
    }

    return cursor;
}

// TODO From synth_tone-tables.h 
// TODO derived from 44100 sample rate so can't be in here
#define US_US_PER_SAMPLE_BITS 46439L
#define US_US_PER_SAMPLE_NEXP 11L

void us_pm_sequencer_init(
    UsPmSequencer *sequencer
) {
    us_tuner_reset(&sequencer->clock);
    sequencer->clock.fips = 384L * US_US_PER_SAMPLE_BITS;
    sequencer->clock.eips = US_US_PER_SAMPLE_NEXP;
    sequencer->tempo = 600000L;
    sequencer->cursor = NULL;
    sequencer->ticks = 0;
}

// Called every sample
void us_pm_sequencer_sample(
    UsPmSequencer *sequencer
) {
    if (sequencer->cursor == NULL) return;
    us_tuner_rotate(&sequencer->clock);
    while (sequencer->clock.bang > sequencer->tempo) {
        --sequencer->ticks;
        sequencer->clock.bang -= sequencer->tempo;
    }
    while (sequencer->ticks <= 0 && sequencer->cursor != NULL) {
        UsPmMessage message; // TODO probably not here
        sequencer->cursor = us_pm_step(sequencer->cursor, &message);
    }
}
