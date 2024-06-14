#include <stdio.h>
#include <math.h>
#include <stdlib.h>

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
    const int first_note_index = -(12*4) - 9; // Concert pitch A4 440Hz
    const int last_note_index = (12*4)+3;
    const int number_of_notes = last_note_index - first_note_index + 1;
    NoteInfo note_infos[number_of_notes];
    printf("#pragma once\n");
    printf("//\n// Tone tables for u-synth at %8.1lfHz\n//\n", (double)sample_frequency);
    for (int i = 0; i < number_of_notes; ++i) {
        const int ni = first_note_index + i;
        const double f = frequency_A_hz*pow((double)2, (((double)(ni)/12)));
        const double g = f / sample_frequency;
        const int b = (int)(log(g)/log(2));
        // We add 1 as we actually only want to use 31 of the bits
        const int k = b + bits_per_word + 1;
        const int j = bits_per_word - k;
        const unsigned long h = g * pow(2, bits_per_word + j);
        const char *nn = note_name[pmod12(ni)];
        const int osi = ni - 3;
        const int o = ((osi + (12*5))/ 12);
        
        printf("//  %3d   %d   %5.5s %10.3lfHz  %4d %4d %10.10ld =(%32.32lb << %2d)=%32.32lb\n", i, o, nn, f, k, j, h, (unsigned long)(g * pow(2, bits_per_word)), j, h);

        NoteInfo *note_info = &note_infos[i];
        note_info->bae = j;
        note_info->bas = h;
        note_info->frequency = f;
        note_info->octave = o;
        note_info->note_name = nn;

    }
    printf("\nconst uint8_t us_bas[] = {\n");
    for (int i = 0; i < number_of_notes; ++i) {
        const NoteInfo *note_info = &note_infos[i];
        printf("/* %3d, %5.5s (%d), %8.3lfHz */ %d,\n", i, note_info->note_name, note_info->octave, note_info->frequency, note_info->bae);
    }
    printf("};\n");
    printf("\nconst uint32_t us_bae[] = {\n");
    for (int i = 0; i < number_of_notes; ++i) {
        const NoteInfo *note_info = &note_infos[i];
        printf("/* %3d, %5.5s (%d), %8.3lfHz */ %lu,\n", i, note_info->note_name, note_info->octave, note_info->frequency, note_info->bas);

    }
    printf("};\n");

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
}