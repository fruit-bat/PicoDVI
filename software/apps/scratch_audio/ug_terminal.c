#include "ug_terminal.h"
#include "pico/stdio/driver.h"
#include <stdio.h>
#include <string.h>

static char text[UG_TERMINAL_WIDTH * UG_TERMINAL_HEIGHT];
static uint32_t cx = 0;
static uint32_t cy = 0;
static uint32_t ci = 0;

static TextGrid8_t _textGrid = {
       .w = UG_TERMINAL_WIDTH, 
       .h = UG_TERMINAL_HEIGHT,
       .ys = 0,
       .s = (uint8_t *)&text
};

// ys = 0 yl = 0
// ys = 1 yl = 1
// ys = 2 
static inline void ug_terminal_put(char c) {
    text[ci++] = c;
    if (++cx == UG_TERMINAL_WIDTH) {
        cx = 0;
        if (++cy == UG_TERMINAL_HEIGHT) {
            cy = 0;
            ci = 0;
        }
        if (_textGrid.ys == cy) {
            if (++_textGrid.ys == UG_TERMINAL_HEIGHT) _textGrid.ys = 0;
        }
    }
}

static inline void ug_terminal_out_char(char c) {
    if (c == '\n') {
        if (cx == 0) ug_terminal_put(' ');
        while(cx > 0) ug_terminal_put(' ');
    }
    ug_terminal_put(c);
}

static void __not_in_flash_func(ug_terminal_out_chars)(const char*s, int l) {
   while(l--) ug_terminal_out_char(*s++);
}

static stdio_driver_t ug_terminal = {
    .out_chars = ug_terminal_out_chars,
#ifdef PICO_STDIO_ENABLE_CRLF_SUPPORT
    .crlf_enabled = PICO_STDIO_DEFAULT_CRLF
#endif
};

void ug_terminal_init() {
    stdio_set_driver_enabled(&ug_terminal, true);
    memset(text, ' ', sizeof(text));
    printf("Micro Graphics Terminal v0.1\n");
}


TextGrid8_t *ug_terminal_text_grid() {
    return &_textGrid;
}