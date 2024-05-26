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

#define MODE_640x480_60Hz
// DVDD 1.2V (1.1V seems ok too)
#define FRAME_WIDTH 320
#define FRAME_HEIGHT 240
#define VREG_VSEL VREG_VOLTAGE_1_20
#define DVI_TIMING dvi_timing_640x480p_60hz

//#define AUDIO_RATE 32000
#define AUDIO_RATE 44100
//#define AUDIO_RATE 48000

#if (AUDIO_RATE == 32000)
#define HDMI_N     4096     // From HDMI standard for 32kHz
#elif (AUDIO_RATE == 44100)
#define HDMI_N     6272     // From HDMI standard for 44.1kHz
#else
#define HDMI_N     6144     // From HDMI standard for 48kHz
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

typedef struct {
	uint16_t d[8];
} Tile16x8p2_t;

typedef struct {
	uint32_t d[16];
} Tile32x16p2_t;

typedef uint8_t SpriteId;
typedef uint8_t SpriteCollisionMask;

typedef void (*SpriteRenderer)(
	const void* d1,
	const void* d2,
	uint32_t * const dr,
	uint32_t * const dg,
	uint32_t * const db,
	const int32_t tdmsI,
	const int32_t row,
	const SpriteId spriteId
);

enum SpriteFlags {
  SF_ENABLE = 1
};

typedef struct Sprite {
	int32_t x,y;
	uint32_t w,h,f;
	void *d1, *d2;
	SpriteRenderer r;
} Sprite;

#define MAX_SPRITES (1<<6)

Sprite _sprites[MAX_SPRITES];

void init_sprites() {
	for(uint32_t i = 0; i < MAX_SPRITES; ++i) _sprites[i].f = 0;
}

void init_sprite(
	int i,
	int32_t x,
	int32_t y,
	uint32_t w,
	uint32_t h,
	uint32_t f,
	void *d1, 
	void *d2,
	SpriteRenderer r
) {
	Sprite *s = &_sprites[i];
	s->x = x;
	s->y = y;
	s->w = w;
	s->h = h;
	s->f = f;
	s->d1 = d1;
	s->d2 = d2;
	s->r = r;
}

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

static inline void render_sprite_pixel(
	uint32_t * const dr,
	uint32_t * const dg,
	uint32_t * const db,	
	const uint32_t fgr,
	const uint32_t fgg,
	const uint32_t fgb,
	const SpriteId spriteId,
	const uint32_t j
) {
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

static inline void render_row_16_p1(
	uint32_t d,
	const Pallet1_t * const p,
	uint32_t * const dr,
	uint32_t * const dg,
	uint32_t * const db,
	const int32_t tdmsI,
	const int32_t row,
	const SpriteId spriteId
) {
	if (d)
	{
		const uint32_t fgr = tmds_table[p->r[0]];
		const uint32_t fgg = tmds_table[p->g[0]];
		const uint32_t fgb = tmds_table[p->b[0]];
		const uint32_t bm = 1 << 15;
		if (((uint32_t)tdmsI) < (FRAME_WIDTH - 16))
		{
			for (int32_t i = 0; i < 16; i++)
			{
				const uint32_t j = (uint32_t)tdmsI + i;
				if (d & bm)
				{
					render_sprite_pixel(dr, dg, db, fgr, fgg, fgb, spriteId, j);
				}
				d <<= 1;
			}
		}
		else
		{
			for (int32_t i = 0; i < 16; i++)
			{
				const uint32_t j = (uint32_t)tdmsI + i;
				if ((j < FRAME_WIDTH) && (d & bm))
				{
					render_sprite_pixel(dr, dg, db, fgr, fgg, fgb, spriteId, j);
				}
				d <<= 1;
			}
		}
	}
}

static inline void render_Tile16x16p1(
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
	render_row_16_p1(
		d,
		p,
		dr,
		dg,
		db,
		tdmsI,
		row,
		spriteId
	);
}

static inline void render_Tile16x8p1(
	const Tile16x8p2_t * const t,
	const Pallet1_t * const p,
	uint32_t * const dr,
	uint32_t * const dg,
	uint32_t * const db,
	const int32_t tdmsI,
	const int32_t row,
	const SpriteId spriteId
) {
	uint32_t d = t->d[row];
	render_row_16_p1(
		d,
		p,
		dr,
		dg,
		db,
		tdmsI,
		row,
		spriteId
	);
}

static inline void render_Tile32x16p1(
	const Tile32x16p2_t * const t,
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
		const uint32_t bm = 1 << 31;
		if (((uint32_t)tdmsI) < (FRAME_WIDTH - 16))
		{
			for (int32_t i = 0; i < 32; i++)
			{
				const uint32_t j = (uint32_t)tdmsI + i;
				if (d & bm)
				{
					render_sprite_pixel(dr, dg, db, fgr, fgg, fgb, spriteId, j);
				}
				d <<= 1;
			}
		}
		else
		{
			for (int32_t i = 0; i < 32; i++)
			{
				const uint32_t j = (uint32_t)tdmsI + i;
				if ((j < FRAME_WIDTH) && (d & bm))
				{
					render_sprite_pixel(dr, dg, db, fgr, fgg, fgb, spriteId, j);
				}
				d <<= 1;
			}
		}
	}
}

void __not_in_flash_func(sprite_renderer_sprite_16x8_p1)(
	const void* d1,
	const void* d2,
	uint32_t * const dr,
	uint32_t * const dg,
	uint32_t * const db,
	const int32_t tdmsI,
	const int32_t row,
	const SpriteId spriteId
) {
	render_Tile16x8p1(
		d1,
		d2,
		dr,
		dg,
		db,
		tdmsI,
		row,
		spriteId
	);
}

void __not_in_flash_func(sprite_renderer_sprite_16x16_p1)(
	const void* d1,
	const void* d2,
	uint32_t * const dr,
	uint32_t * const dg,
	uint32_t * const db,
	const int32_t tdmsI,
	const int32_t row,
	const SpriteId spriteId
) {
	render_Tile16x16p1(
		d1,
		d2,
		dr,
		dg,
		db,
		tdmsI,
		row,
		spriteId
	);
}

void __not_in_flash_func(sprite_renderer_sprite_32x16_p1)(
	const void* d1,
	const void* d2,
	uint32_t * const dr,
	uint32_t * const dg,
	uint32_t * const db,
	const int32_t tdmsI,
	const int32_t row,
	const SpriteId spriteId
) {
	render_Tile32x16p1(
		d1,
		d2,
		dr,
		dg,
		db,
		tdmsI,
		row,
		spriteId
	);
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

Tile16x8p2_t tile16x8p2_invader[2] = {
	{{
	//   0123456789012345 
		0b0000001111000000,
		0b0001111111111000,
		0b0011111111111100,
		0b0011001111001100,
		0b0011111111111100,
		0b0000111001110000,
		0b0001100110011000,
		0b0000110000110000,
	}},
	{{
	//   0123456789012345 
		0b0000001111000000,
		0b0001111111111000,
		0b0011111111111100,
		0b0011001111001100,
		0b0011111111111100,
		0b0000111001110000,
		0b0001100110011000,
		0b0011000000001100,
	}}	
};

Tile32x16p2_t tile32x16p2_base = {
	{
	//   0123456789012345 
		0b00000000011111111111111000000000,
		0b00000000111111111111111100000000,
		0b00000001111111111111111110000000,
		0b00000011111111111111111111000000,
		0b00000111111111111111111111100000,
		0b00000111111111111111111111100000,
		0b00000111111111111111111111100000,
		0b00000111111111111111111111100000,
		0b00000111111111111111111111100000,
		0b00000111111111111111111111100000,
		0b00000111111111111111111111100000,
		0b00000111111111111111111111100000,
		0b00000111111100000000111111100000,
		0b00000111111000000000011111100000,
		0b00000111110000000000001111100000,
		0b00000111110000000000001111100000,
	}
};

void __not_in_flash_func(sprite_renderer_invader_16x8_p1)(
	const void* d1,
	const void* d2,
	uint32_t * const dr,
	uint32_t * const dg,
	uint32_t * const db,
	const int32_t tdmsI,
	const int32_t row,
	const SpriteId spriteId
) {
	const Tile16x8p2_t *tile16x8p2_invader = (Tile16x8p2_t *)d1;
	render_Tile16x8p1(
		&tile16x8p2_invader[(tdmsI >> 2) & 1],
		d2,
		dr,
		dg,
		db,
		tdmsI,
		row,
		spriteId
	);
}

static uint32_t inv_index;
static int32_t inv_v = 1;
void init_game() {
	_spriteCollisionMasks[0] = (SpriteCollisionMask)1;
	_spriteCollisionMasks[1] = (SpriteCollisionMask)2;
	_spriteCollisionMasks[2] = (SpriteCollisionMask)8;

	uint32_t si = 0;	
	init_sprite(si++, 50, 5, 16, 8, SF_ENABLE, &tile16x8p2_invader, &pallet1_Green, sprite_renderer_invader_16x8_p1);
	init_sprite(si++, 66, 9, 16, 8, SF_ENABLE, &tile16x8p2_invader, &pallet1_Green, sprite_renderer_invader_16x8_p1);
	init_sprite(si++, 66, 200, 32, 16, SF_ENABLE, &tile32x16p2_base, &pallet1_Green, sprite_renderer_sprite_32x16_p1);

	inv_index = si;
	for(uint32_t x = 0; x < 11; ++x) {
		for(uint32_t y = 0; y < 5; ++y) {
			init_sprite(si, x << 4, 30 + (y << 4), 16, 8, SF_ENABLE, &tile16x8p2_invader, &pallet1_Green, sprite_renderer_invader_16x8_p1);
			_spriteCollisionMasks[si] = (SpriteCollisionMask)4;
			si++;
		}
	}
}

void __not_in_flash_func(core1_main)() {

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

			// Render a blank row
			// TODO optionally render a tiled background
			render_row_mono(
				dr, dg, db,
				8, 0, 8);

			for (uint32_t i = 0; i < MAX_SPRITES; ++i)
			{
				const Sprite *sprite = &_sprites[i];
				const uint32_t r = y - sprite->y;
				if ((sprite-> f & SF_ENABLE) && r < sprite->h)
				{
					(sprite->r)(
						sprite->d1,
						sprite->d2,
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
			Sprite *sprite = &_sprites[i];
			if (_spriteCollisions.m[i]) sprite->d2 = &pallet1_Red;
		}
		_sprites[0].x++; if (_sprites[0].x > FRAME_WIDTH) {
			_sprites[0].x = -16;
			_sprites[0].d2 = &pallet1_Blue;
		}
		_sprites[1].x--; if (_sprites[1].x < -16) {
			_sprites[1].x = FRAME_WIDTH + 16; 
			_sprites[1].d2 = &pallet1_Purple;
		}

		bool reverse = false;
		for (uint32_t i = inv_index; i < inv_index + (5*11); ++i)
		{
			Sprite *sprite = &_sprites[i];
			sprite->x += inv_v;
			if (inv_v > 0) {
				if(sprite->x + 16 >= FRAME_WIDTH) reverse = true;
			}
			else {
				if(sprite->x <= 0) reverse = true;
			}
		}
		if (reverse) inv_v = -inv_v;
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
    dvi_set_audio_freq(&dvi0, AUDIO_RATE, dvi0.timing->bit_clk_khz*HDMI_N/(AUDIO_RATE/100)/128, HDMI_N);
	add_repeating_timer_ms(-2, audio_timer_callback, NULL, &audio_timer);

	init_sprites();
	
	printf("starting...\n");
	init_game();

	multicore_launch_core1(core1_main);

	while (1)
		__wfi();
}
	
