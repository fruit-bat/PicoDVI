#include "u_packed_midi.h"


UsPmCursor us_packed_midi_step(
    UsPmCursor cursor,
    UsPmMessage *message
) {
    if (cursor == NULL) return cursor;
    const uint8_t type = *cursor++;
    message->type = type;
    switch(type) {
        case SynCmdPPQ:
        case SynCmdTempo:
        case SynCmdOn:
        case SynCmdOff:
        case SynCmdTime:
        default:
        case SynCmdEnd:
        break;
    }

    return cursor;
}