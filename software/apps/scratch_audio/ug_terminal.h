#pragma once
#include "pico/stdlib.h"
#include "ug_renderer.h"

#define UG_TERMINAL_WIDTH 40
#define UG_TERMINAL_HEIGHT 30
#define UG_TERMINAL_SIZE (UG_TERMINAL_HEIGHT * UG_TERMINAL_WIDTH)

void ug_terminal_init();

TextGrid8_t *ug_terminal_text_grid();
