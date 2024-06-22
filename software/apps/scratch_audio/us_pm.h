#pragma once
//
// MicroSynth PackedMidi 
//
#include "us_tuner.h"
#include "us_voices.h"

enum UsPmCommands {
    SynCmdPPQ = 0,
    SynCmdTempo,
    SynCmdOn,
    SynCmdOff,
    SynCmdTime,
    SynCmdEnd
};

enum UsPmCommandLen {
    SynCmdPPQLen = 3,
    SynCmdTempoLen = 4,
    SynCmdOnLen = 4,
    SynCmdOffLen = 3,
    SynCmdTimeLen = 3,
    SynCmdEndLen = 1
};

typedef uint8_t const * UsPmCursor;

typedef struct {
    UsTuner clock;      // Runs as micro seconds * parts per quater beat (us*PPQ)
    uint32_t tempo;     // Micro seconds per quarter beat (us)
    UsPmCursor cursor;  // Cursor into packed midi file
    int32_t ticks;      // Ticks to wait
    UsVoices *voices;   // Something to play music on
    UsPmCursor sequence;// Start of the packed midi file
    bool repeat;        // Play over an over
} UsPmSequencer;

void us_pm_sequencer_init(
    UsPmSequencer *sequencer,
    UsVoices *voices,
    UsPmCursor sequence,
    bool repeat
);

void us_pm_sequencer_update(
    UsPmSequencer *sequencer
);
