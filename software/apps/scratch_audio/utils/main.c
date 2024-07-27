// gcc  main.c -lm  && ./a.out > ~/pico/PicoDVI/software/apps/scratch_audio/u_synth_tone_tables.h

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

#define US_SAMPLE_RATE_HZ 44100

typedef struct {
    uint32_t fips; // Increment per sample ((2^(32+be))*f/sf)
    uint32_t eips;  // Negative binary exponent of fips (-be)
} UsPitch;

void us_set_freqency_hz(
    UsPitch *pitch,
    uint32_t fhz // fixed pt 16 16
) {
    // Assume f/fs < 1
    uint64_t t1 = (((uint64_t)fhz) << 32ULL) / ((uint64_t)US_SAMPLE_RATE_HZ);
    // Shift right until only 31 bits are used (slow)
    int32_t e = 0;
    while(t1 > ((1L << 31L) - 1L)) {
        e++;
        t1 >>= 1;
    }
    pitch->fips = t1;
    pitch->eips = 32 - e;
}

void us_set_freqency_hz_double(
    UsPitch *pitch,
    double fhz // fixed pt 16 16
) {
    // Assume f/fs < 1
    uint32_t f16 = (fhz * pow(2, 16));
    us_set_freqency_hz(pitch, f16);
}

// gcc main.c -lm  && ./a.out
//
// 440.000Hz 0027 0005 1371273005 01010001101110111111011100101101
//
static char* note_name[] = {
  "A",
  "A#,Bb",
  "B",
  "C",
  "C#,Db",
  "D",
  "D#,Eb",
  "E",
  "F",
  "F#,Gb",
  "G",
  "G#,Ab"
};

static int pmod12(int ni) {
    const int nim = ni % 12;
    return nim < 0 ? 12 + nim : nim;
}

typedef struct  {
    unsigned char bae;
    unsigned long bas;
    const char* note_name;
    int octave;
    double frequency;
} NoteInfo;

int main(int argc, char**argv) {
    const int octaves = 8;
    const int sample_frequency = 44100;
    const int bits_per_word = 32;
    const double frequency_A_hz = 440; // Concert pitch A4 440Hz
    const int midi_note_offset = 12;
    const int first_note_index = -(12*5) - (9+midi_note_offset); // Concert pitch A4 440Hz
    const int last_note_index = (12*4)+10;
    const int number_of_notes = 12; // last_note_index - first_note_index + 1;
    const int note_fractions_log2 = 6;
    NoteInfo note_infos[number_of_notes << note_fractions_log2];
    printf("#pragma once\n");
    printf("#define US_NOTE_FRACTIONS_LOG2 %d\n", note_fractions_log2);
    printf("#define US_NOTE_FRACTIONS %d\n", 1<<note_fractions_log2);
    printf("#define US_NOTE_MIDI_OFFSET %d\n", midi_note_offset);
    printf("#define US_NOTE_TABLE_SIZE (12 << US_NOTE_FRACTIONS_LOG2)\n");

    printf("//\n// Tone tables for u-synth at %8.1lfHz\n//\n", (double)sample_frequency);
    const int note_fractions = 1 << note_fractions_log2;
    for (int i = 0; i < (number_of_notes << note_fractions_log2); ++i) {
        const int fi = (first_note_index * note_fractions) + i;
        const double f = frequency_A_hz*pow((double)2, (((double)(fi)/(number_of_notes << note_fractions_log2))));
        const double g = f / sample_frequency;
        const int b = (int)(log(g)/log(2));
        // We add 1 as we actually only want to use 31 of the bits
        const int k = b + bits_per_word + 1;
        const int j = bits_per_word - k;
        const unsigned long h = g * pow(2, bits_per_word + j);
        const int ni = fi >> note_fractions_log2;
        const char *nn = note_name[pmod12(ni)];
        const int osi = ni - 3;
        const int o = ((osi + (12*7))/ 12) - 2;
        
        printf("//  %3d %3d %3d   %5.5s %10.3lfHz  %4d %4d %10.10ld =(%32.32lb << %2d)=%32.32lb\n", i, (i >> note_fractions_log2) - midi_note_offset, o, nn, f, k, j, h, (unsigned long)(g * pow(2, bits_per_word)), j, h);

        //UsPitch pitch;
        //us_set_freqency_hz_double(&pitch, f);
        //printf("//  %3d %3d %3d   %5.5s %10.3lfHz  %4d %4d %10.10ld =(%32.32lb << %2d)=%32.32lb\n\n", i,  o, nn, f, k, pitch.eips, pitch.fips, (unsigned long)(g * pow(2, bits_per_word)), pitch.eips, pitch.fips);



        NoteInfo *note_info = &note_infos[i];
        note_info->bae = j;
        note_info->bas = h;
        note_info->frequency = f;
        note_info->octave = o;
        note_info->note_name = nn;
    }
    {
        // Calculate values for a timer for microseconds (us) per sample
        const double f = pow(10, 6);
        const double g = f / sample_frequency;
        const int b = (int)(log(g)/log(2));
        // We add 1 as we actually only want to use 16 of the bits so we can multiply by PPQ
        const int j = 16 - b - 1;
        const unsigned long h = g * pow(2, j);
        printf("#define US_US_PER_SAMPLE_BITS %ld\n", h);
        printf("#define US_US_PER_SAMPLE_NEXP %d\n", j);
    }

    printf("\nconst uint8_t us_pitch_table_eips[] = {\n");
    for (int i = 0; i < (number_of_notes << note_fractions_log2); ++i) {
        const NoteInfo *note_info = &note_infos[i];
        printf("/* %3d, %5.5s (%d), %8.3lfHz */ %d,\n", i, note_info->note_name, note_info->octave, note_info->frequency, note_info->bae);
    }
    printf("};\n");
    printf("\nconst uint32_t us_pitch_table_fips[] = {\n");
    for (int i = 0; i < (number_of_notes << note_fractions_log2); ++i) {
        const NoteInfo *note_info = &note_infos[i];
        printf("/* %3d, %5.5s (%d), %8.3lfHz */ %lu,\n", i, note_info->note_name, note_info->octave, note_info->frequency, note_info->bas);

    }
    printf("};\n");
#if 0
    printf("\nconst uint16_t us_sin[] = {\n");
    {
        const int n = 7;
        const double max = 32767;
        for (int i = 0; i <= (1 << n); ++i)
        {
            const double a = M_PI * ((double)i) / (double)(1 << (n + 1));
            const unsigned long v = (unsigned long)(sin(a) * (double)(32767));
            printf("/* %3d, %1.3lf  */ %d,\n", i, a, v);
        }
        printf("};\n");
    }
#endif    
}