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

typedef uint8_t SpriteId;
typedef uint8_t SpriteCollisionMask;

typedef struct {
	Tile16x16p2_t *t;
	Pallet1_t *p;
	int32_t x;
	int32_t y;
} Sprite16x16p1;

#define MAX_SPRITES (1<<8)

// ----------------------------------------------------------------------------
// Sprite collisions
// 
// Collide with the drawn sprite only 
// ----------------------------------------------------------------------------
#define SPRITE_ID_ROW_WORDS ((FRAME_WIDTH + 3) >> 2)

typedef union {
	SpriteId id[FRAME_WIDTH];
	uint32_t word[SPRITE_ID_ROW_WORDS];
} SpriteIdRow;

typedef union {
	SpriteCollisionMask m[MAX_SPRITES];
	uint32_t word[MAX_SPRITES >> 2];
} SpriteCollisions;

static SpriteCollisionMask _spriteCollisionMasks[MAX_SPRITES];
static SpriteIdRow _spriteIdRow; 
static SpriteCollisions _spriteCollisions;

void __not_in_flash_func(clear_sprite_id_row)() {
	for(uint32_t i = 0; i < SPRITE_ID_ROW_WORDS; ++i) _spriteIdRow.word[i] = 0;
}

void __not_in_flash_func(clear_sprite_collisions)() {
	for(uint32_t i = 0; i < MAX_SPRITES >> 2; ++i) _spriteCollisions.word[i] = 0;
}

// ----------------------------------------------------------------------------

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
	const Tile16x16p2_t * const t,
	const Pallet1_t * const p,
	uint32_t * const dr,
	uint32_t * const dg,
	uint32_t * const db,
	const int32_t tdmsI,
	const int32_t row,
	const SpriteId spriteId
) {
	uint32_t d = t->d[row];
	if (d)
	{
		const uint32_t fgr = tmds_table[p->r[0]];
		const uint32_t fgg = tmds_table[p->g[0]];
		const uint32_t fgb = tmds_table[p->b[0]];
		if (((uint32_t)tdmsI) < (FRAME_WIDTH - 16))
		{
			for (int32_t i = 0; i < 16; i++)
			{
				const uint32_t j = (uint32_t)tdmsI + i;
				if (d & (1 << 15))
				{
					const SpriteId ncid = _spriteIdRow.id[j];
					if (ncid) {
						const SpriteId cid = ncid - 1;
						_spriteCollisions.m[cid] |= _spriteCollisionMasks[spriteId];
						_spriteCollisions.m[spriteId] |= _spriteCollisionMasks[cid];
					}
					else {
						dr[j] = fgr;
						dg[j] = fgg;
						db[j] = fgb;
						_spriteIdRow.id[j] = spriteId + 1;
					}
				}
				d <<= 1;
			}
		}
		else
		{
			for (int32_t i = 0; i < 16; i++)
			{
				const uint32_t j = (uint32_t)tdmsI + i;
				if ((j < FRAME_WIDTH) && (d & (1 << 15)))
				{
					const SpriteId ncid = _spriteIdRow.id[j];
					if (ncid) {
						const SpriteId cid = ncid - 1;
						_spriteCollisions.m[cid] |= _spriteCollisionMasks[spriteId];
						_spriteCollisions.m[spriteId] |= _spriteCollisionMasks[cid];
					}
					else {
						dr[j] = fgr;
						dg[j] = fgg;
						db[j] = fgb;
						_spriteIdRow.id[j] = spriteId + 1;
					}
				}
				d <<= 1;
			}
		}
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

Pallet1_t pallet1_Red = {
	{ (uint8_t)63 },
	{ (uint8_t)0 },
	{ (uint8_t)0 }
};

Pallet1_t pallet1_Blue = {
	{ (uint8_t)0 },
	{ (uint8_t)0 },
	{ (uint8_t)63 }
};

Pallet1_t pallet1_Purple = {
	{ (uint8_t)32 },
	{ (uint8_t)0 },
	{ (uint8_t)32 }
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

Sprite16x16p1 sprites[] = {
    {&tile16x16p2_invader[0], &pallet1_Green, 50, 50},
    {&tile16x16p2_invader[0], &pallet1_Green, 66, 50},
	
   {&tile16x16p2_invader[0], &pallet1_Green,   6, 100},
   {&tile16x16p2_invader[0], &pallet1_Green,   16, 100},
   {&tile16x16p2_invader[0], &pallet1_Green,   26, 100},
   {&tile16x16p2_invader[0], &pallet1_Green,   36, 100},
   {&tile16x16p2_invader[0], &pallet1_Green,   46, 100},
   {&tile16x16p2_invader[0], &pallet1_Green,   56, 100},
   {&tile16x16p2_invader[0], &pallet1_Green,   66, 100},
   {&tile16x16p2_invader[0], &pallet1_Green,   76, 100},
   {&tile16x16p2_invader[0], &pallet1_Green,   86, 100},
   {&tile16x16p2_invader[0], &pallet1_Green,   96, 100},
   {&tile16x16p2_invader[0], &pallet1_Green,   106, 100},
   {&tile16x16p2_invader[0], &pallet1_Green,   116, 100},
};


void __not_in_flash_func(core1_main)() {
_spriteCollisionMasks[0] = (SpriteCollisionMask)1;
_spriteCollisionMasks[1] = (SpriteCollisionMask)2;

	dvi_register_irqs_this_core(&dvi0, DMA_IRQ_0);
	dvi_start(&dvi0);
	uint32_t frames = 0;
	while (true) {
		clear_sprite_collisions();
		for (uint32_t y = 0; y < FRAME_HEIGHT; ++y) {
			uint32_t *tmdsbuf;
			clear_sprite_id_row();
			queue_remove_blocking(&dvi0.q_tmds_free, &tmdsbuf);
			uint32_t *db = tmdsbuf;
			uint32_t *dg = db + FRAME_WIDTH;
			uint32_t *dr = dg + FRAME_WIDTH;

			render_row_mono(
				dr, dg, db,
				8, 0, 8);

			for (uint32_t i = 0; i < 14; ++i)
			{
				const Sprite16x16p1 *sprite = &sprites[i];
				const uint32_t r = y - sprite->y;
				if (r < 16)
				{
					render_Tile16x16p1(
						sprite->t,
						sprite->p,
						dr, dg, db,
						sprite->x,
						r,
						i);
				}
			}
			queue_add_blocking(&dvi0.q_tmds_valid, &tmdsbuf);
		}
		++frames;

		// Just messing about - start
		for (uint32_t i = 0; i < 2; ++i)
		{
			Sprite16x16p1 *sprite = &sprites[i];
			sprite->t = &tile16x16p2_invader[frames >> 2 & 1];
			if (_spriteCollisions.m[i]) sprite->p = &pallet1_Red;
		}
		sprites[0].x++; if (sprites[0].x > FRAME_WIDTH) {
			sprites[0].x = -16;
			sprites[0].p = &pallet1_Blue;
		}
		sprites[1].x--; if (sprites[1].x < -16) {
			sprites[1].x = FRAME_WIDTH + 16; 
			sprites[1].p = &pallet1_Purple;
		}
		// Just messing about - end
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
	
