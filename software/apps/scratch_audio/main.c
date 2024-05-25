#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "hardware/irq.h"
#include "hardware/sync.h"
#include "hardware/gpio.h"
#include "hardware/vreg.h"
#include "hardware/structs/bus_ctrl.h"
#include "hardware/structs/ssi.h"
#include "hardware/dma.h"
#include "pico/sem.h"

#include "dvi.h"
#include "dvi_serialiser.h"
#include "common_dvi_pin_configs.h"
#include "audio.h"

// Pick one:
#define MODE_640x480_60Hz
// #define MODE_800x600_60Hz
// #define MODE_960x540p_60Hz
// #define MODE_1280x720_30Hz

#if defined(MODE_640x480_60Hz)
// DVDD 1.2V (1.1V seems ok too)
#define FRAME_WIDTH 320
#define FRAME_HEIGHT 240
#define VREG_VSEL VREG_VOLTAGE_1_20
#define DVI_TIMING dvi_timing_640x480p_60hz

#elif defined(MODE_800x600_60Hz)
// DVDD 1.3V, going downhill with a tailwind
#define FRAME_WIDTH 800
#define FRAME_HEIGHT 600
#define VREG_VSEL VREG_VOLTAGE_1_30
#define DVI_TIMING dvi_timing_800x600p_60hz


#elif defined(MODE_960x540p_60Hz)
// DVDD 1.25V (slower silicon may need the full 1.3, or just not work)
#define FRAME_WIDTH 960
#define FRAME_HEIGHT 540
#define VREG_VSEL VREG_VOLTAGE_1_25
#define DVI_TIMING dvi_timing_960x540p_60hz

#elif defined(MODE_1280x720_30Hz)
// 1280x720p 30 Hz (nonstandard)
// DVDD 1.25V (slower silicon may need the full 1.3, or just not work)
#define FRAME_WIDTH 1280
#define FRAME_HEIGHT 720
#define VREG_VSEL VREG_VOLTAGE_1_25
#define DVI_TIMING dvi_timing_1280x720p_30hz

#else
#error "Select a video mode!"
#endif

struct dvi_inst dvi0;

//Audio Related
#define AUDIO_BUFFER_SIZE   256
audio_sample_t      audio_buffer[AUDIO_BUFFER_SIZE];
struct repeating_timer audio_timer;

static const uint32_t __scratch_x("tmds_table") tmds_table[] = {
#include "tmds_table.h"
};

bool __not_in_flash_func(audio_timer_callback)(struct repeating_timer *t) {
	while(true) {
		int size = get_write_size(&dvi0.audio_ring, false);
		if (size == 0) return true;
		audio_sample_t *audio_ptr = get_write_pointer(&dvi0.audio_ring);
		audio_sample_t sample;
		static uint sample_count = 0;
		for (int cnt = 0; cnt < size; cnt++) {
			sample.channels[0] = commodore_argentina[sample_count % commodore_argentina_len] << 8;
			sample.channels[1] = commodore_argentina[(sample_count+1024) % commodore_argentina_len] << 8;
			*audio_ptr++ = sample;
			sample_count = sample_count + 1;
		}
		increase_write_pointer(&dvi0.audio_ring, size);
	}
}

typedef struct {
	uint8_t r[1];
	uint8_t g[1];
	uint8_t b[1];
} Pallet1_t;

typedef struct {
	uint8_t r[2];
	uint8_t g[2];
	uint8_t b[2];
} Pallet2_t;

typedef struct {
	uint16_t d[16];
} Tile16x16p2_t;

void __not_in_flash_func(render_row_mono)(
	uint32_t *dr,
	uint32_t *dg,
	uint32_t *db,
	uint32_t bgr,
	uint32_t bgg,
	uint32_t bgb
) {
	for(int32_t i = 0; i < FRAME_WIDTH; i++) {
		dr[i] = tmds_table[bgr];
		dg[i] = tmds_table[bgg];
		db[i] = tmds_table[bgb];
	}
}

void __not_in_flash_func(render_Tile16x16p1)(
	Tile16x16p2_t *t,
	Pallet1_t *p,
	uint32_t *dr,
	uint32_t *dg,
	uint32_t *db,
	int32_t tdmsI,
	int32_t row
) {
	uint16_t d = t->d[row];
	uint32_t fgr = p->r[0];
	uint32_t fgg = p->g[0];
	uint32_t fgb = p->b[0];
	for(int32_t i = 0; i < 16; i++) {
		int32_t j = tdmsI + i;
		if (d & (1<<15)) {
			dr[j] = tmds_table[fgr];
			dg[j] = tmds_table[fgg];
			db[j] = tmds_table[fgb];
		}
		d <<= 1;
	}
}

void __not_in_flash_func(render_Tile16x16p2)(
	Tile16x16p2_t *t,
	Pallet2_t *p,
	uint32_t *dr,
	uint32_t *dg,
	uint32_t *db,
	int32_t tdmsI,
	int32_t row
) {
	uint16_t d = t->d[row];
	uint32_t bgr = p->r[0];
	uint32_t fgr = p->r[1];
	uint32_t bgg = p->g[0];
	uint32_t fgg = p->g[1];
	uint32_t bgb = p->b[0];
	uint32_t fgb = p->b[1];
	for(int32_t i = 0; i < 16; i++) {
		int32_t j = tdmsI + i;
		if (d & (1<<15)) {
			dr[j] = tmds_table[fgr];
			dg[j] = tmds_table[fgg];
			db[j] = tmds_table[fgb];
		}
		else {
			dr[j] = tmds_table[bgr];
			dg[j] = tmds_table[bgg];
			db[j] = tmds_table[bgb];
		}
		d <<= 1;
	}
}

Pallet2_t pallet2_BlackGreen = {
	{ (uint8_t)0, (uint8_t)0 },
	{ (uint8_t)0, (uint8_t)63},
	{ (uint8_t)0, (uint8_t)0 }
};

Pallet1_t pallet1_Green = {
	{ (uint8_t)0 },
	{ (uint8_t)63},
	{ (uint8_t)0 }
};

Tile16x16p2_t tile16x16p2_invader[2] = {
	{{
	//   0123456789012345 
		0b0000000000000000,
		0b0000000000000000,
		0b0000000000000000,
		0b0000000000000000,
		0b0000001111000000,
		0b0001111111111000,
		0b0011111111111100,
		0b0011001111001100,
		0b0011111111111100,
		0b0000111001110000,
		0b0001100110011000,
		0b0000110000110000,
		0b0000000000000000,
		0b0000000000000000,
		0b0000000000000000,
	}},
	{{
	//   0123456789012345 
		0b0000000000000000,
		0b0000000000000000,
		0b0000000000000000,
		0b0000000000000000,
		0b0000001111000000,
		0b0001111111111000,
		0b0011111111111100,
		0b0011001111001100,
		0b0011111111111100,
		0b0000111001110000,
		0b0001100110011000,
		0b0011000000001100,
		0b0000000000000000,
		0b0000000000000000,
		0b0000000000000000,
	}}	
};

void __not_in_flash_func(core1_main)() {
	dvi_register_irqs_this_core(&dvi0, DMA_IRQ_0);
	dvi_start(&dvi0);
	uint32_t frames = 0;
	while (true) {
		for (uint y = 0; y < FRAME_HEIGHT; ++y) {
			uint32_t *tmdsbuf;
			queue_remove_blocking(&dvi0.q_tmds_free, &tmdsbuf);
			uint32_t *dr = tmdsbuf;
			uint32_t *dg = dr + FRAME_WIDTH;
			uint32_t *db = dg + FRAME_WIDTH;

			render_row_mono(
				dr, dg, db,
				8, 0, 8
			);

			for(int32_t i = 0; i < FRAME_WIDTH; i += 16) {
				render_Tile16x16p1(
					&tile16x16p2_invader[(frames >> 6) & 1],
					&pallet1_Green,
					dr, dg, db,
					i,
					y & 15
				);
			}
			queue_add_blocking(&dvi0.q_tmds_valid, &tmdsbuf);
		}
		++frames;
	}
}

int __not_in_flash_func(main)() {
	vreg_set_voltage(VREG_VSEL);
	sleep_ms(10);
	// Run system at TMDS bit clock
	set_sys_clock_khz(DVI_TIMING.bit_clk_khz, true);

	// setup_default_uart();

	dvi0.timing = &DVI_TIMING;
	dvi0.ser_cfg = DVI_DEFAULT_SERIAL_CONFIG;
	dvi_init(&dvi0, next_striped_spin_lock_num(), next_striped_spin_lock_num());

	hw_set_bits(&bus_ctrl_hw->priority, BUSCTRL_BUS_PRIORITY_PROC1_BITS);

	// HDMI Audio related
	dvi_get_blank_settings(&dvi0)->top    = 4 * 0;
	dvi_get_blank_settings(&dvi0)->bottom = 4 * 0;
	dvi_audio_sample_buffer_set(&dvi0, audio_buffer, AUDIO_BUFFER_SIZE);
	dvi_set_audio_freq(&dvi0, 44100, 28000, 6272);
	add_repeating_timer_ms(-2, audio_timer_callback, NULL, &audio_timer);

	printf("starting...\n");

	multicore_launch_core1(core1_main);

	while (1)
		__wfi();
}
	
