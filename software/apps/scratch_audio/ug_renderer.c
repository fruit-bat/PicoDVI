#include "ug_renderer.h"
#include "font_inv.h"
#include "string.h"
#include "pico/multicore.h"


static const uint32_t __scratch_x("tmds_table") tmds_table[] = {
#include "tmds_table.h"
};

static Sprite _sprites[MAX_SPRITES];

void init_sprites() {
	for(uint32_t i = 0; i < MAX_SPRITES; ++i) _sprites[i].f = 0;
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

static SpriteCollisionMask _spriteCollisionMasks[MAX_SPRITES];
static SpriteIdRow _spriteIdRow; 
static SpriteCollisions _spriteCollisions;

inline static void clear_sprite_id_row() {
	//for(uint32_t i = 0; i < SPRITE_ID_ROW_WORDS; ++i) _spriteIdRow.word[i] = 0;
    memset(&_spriteIdRow.word, 0, SPRITE_ID_ROW_WORDS << 2);
}

inline static void clear_sprite_collisions() {
	//for(uint32_t i = 0; i < MAX_SPRITES >> 2; ++i) _spriteCollisions.word[i] = 0;
    memset(&_spriteCollisions.word, 0, MAX_SPRITES);
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
	SpriteRenderer r,
    SpriteCollisionMask m
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
    _spriteCollisionMasks[i] = m;
}

// ----------------------------------------------------------------------------

static inline void render_row_mono(
	uint32_t *dr,
	uint32_t *dg,
	uint32_t *db,
	uint32_t bgr,
	uint32_t bgg,
	uint32_t bgb
) {
    const uint32_t r = tmds_table[bgr];
    const uint32_t g = tmds_table[bgg];
    const uint32_t b = tmds_table[bgb];

	for(int32_t i = 0; i < FRAME_WIDTH; i++) {
		dr[i] = r;
		dg[i] = g;
		db[i] = b;
	}
}

inline static void render_sprite_pixel(
	uint32_t *const dr,
	uint32_t *const dg,
	uint32_t *const db,
	const uint32_t r,
	const uint32_t g,
	const uint32_t b,
	const SpriteId spriteId,
	const uint32_t j)
{
	const SpriteId ncid = _spriteIdRow.id[j];
	if (ncid)
	{
		const SpriteId cid = ncid - 1;
		_spriteCollisions.m[cid] |= _spriteCollisionMasks[spriteId];
		_spriteCollisions.m[spriteId] |= _spriteCollisionMasks[cid];
	}
	else
	{
		dr[j] = r;
		dg[j] = g;
		db[j] = b;
		_spriteIdRow.id[j] = spriteId + 1;
	}
}

inline static void render_pixel(
	uint32_t * const dr,
	uint32_t * const dg,
	uint32_t * const db,	
	const uint32_t r,
	const uint32_t g,
	const uint32_t b,
	const uint32_t j
) {
	dr[j] = r;
	dg[j] = g;
	db[j] = b;
}

inline static void __not_in_flash_func(render_sprite_row_n_p1)(
	uint32_t d,
	const Pallet1_t * const p,
	uint32_t * const dr,
	uint32_t * const dg,
	uint32_t * const db,
	const int32_t tdmsI,
	const SpriteId spriteId,
	const uint32_t w
) {
	if (d)
	{
		const uint32_t fgr = tmds_table[p->r[0]];
		const uint32_t fgg = tmds_table[p->g[0]];
		const uint32_t fgb = tmds_table[p->b[0]];
		const uint32_t bm = 1 << (w-1);
		if (((uint32_t)tdmsI) < (FRAME_WIDTH - w))
		{
			for (int32_t i = 0; i < w; i++)
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
			for (int32_t i = 0; i < w; i++)
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

inline static void __not_in_flash_func(render_row_n_p1)(
	uint32_t d,
	const Pallet1_t * const p,
	uint32_t * const dr,
	uint32_t * const dg,
	uint32_t * const db,
	const int32_t tdmsI,
	const uint32_t w
) {
	if (d)
	{
		const uint32_t fgr = tmds_table[p->r[0]];
		const uint32_t fgg = tmds_table[p->g[0]];
		const uint32_t fgb = tmds_table[p->b[0]];
		const uint32_t bm = 1 << (w-1);
		if (((uint32_t)tdmsI) < (FRAME_WIDTH - w))
		{
			for (int32_t i = 0; i < w; i++)
			{
				const uint32_t j = (uint32_t)tdmsI + i;
				if (d & bm)
				{
					render_pixel(dr, dg, db, fgr, fgg, fgb, j);
				}
				d <<= 1;
			}
		}
		else
		{
			for (int32_t i = 0; i < w; i++)
			{
				const uint32_t j = (uint32_t)tdmsI + i;
				if ((j < FRAME_WIDTH) && (d & bm))
				{
					render_pixel(dr, dg, db, fgr, fgg, fgb, j);
				}
				d <<= 1;
			}
		}
	}
}

inline static void __not_in_flash_func(render_row_text_8_p1)(
	const TextGrid8_t *tg,
	const Pallet1_t * const p,
	uint32_t * const dr,
	uint32_t * const dg,
	uint32_t * const db,
	const int32_t tdmsI,
	const int32_t row
) {
	uint8_t * const s = tg->s + __mul_instruction((row >> 3), tg->w);
	const uint32_t r = row & 7;
	const uint32_t w = tg->w;
	const uint32_t v = w >> 2;
	for(uint32_t i = 0; i < v; ++i) {
		const uint32_t q = i << 2;
		const uint8_t d1 = font_8x8[r + ((s[q] -  32) << 3)];
		const uint8_t d2 = font_8x8[r + ((s[q+1] -  32) << 3)];
		const uint8_t d3 = font_8x8[r + ((s[q+2] -  32) << 3)];
		const uint8_t d4 = font_8x8[r + ((s[q+3] -  32) << 3)];
		const uint32_t g = (((uint32_t)d1) << 24) | (((uint32_t)d2) << 16) | (((uint32_t)d3) << 8) | d4;
		render_row_n_p1(
			g,
			p,
			dr,
			dg,
			db,
			tdmsI + (i << 5),
			32
		);
	}
	for(uint32_t i = w & -4; i < w; ++i) {
		const uint8_t d = font_8x8[r + ((s[i] -  32) << 3)];
		render_row_n_p1(
			d,
			p,
			dr,
			dg,
			db,
			tdmsI + (i << 3),
			8
		);
	}
}

inline static void render_Tile16x16p1(
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
	render_sprite_row_n_p1(
		d,
		p,
		dr,
		dg,
		db,
		tdmsI,
		spriteId,
		16
	);
}

inline static void render_Tile16x8p1(
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
	render_sprite_row_n_p1(
		d,
		p,
		dr,
		dg,
		db,
		tdmsI,
		spriteId,
		16
	);
}

inline static void render_Tile32x16p1(
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
	render_sprite_row_n_p1(
		d,
		p,
		dr,
		dg,
		db,
		tdmsI,
		spriteId,
		32
	);
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

inline void __not_in_flash_func(sprite_renderer_sprite_16x16_p1)(
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

inline void __not_in_flash_func(sprite_renderer_sprite_32x16_p1)(
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

inline void __not_in_flash_func(text_renderer_8x8_p1)(
	const void* d1,
	const void* d2,
	uint32_t * const dr,
	uint32_t * const dg,
	uint32_t * const db,
	const int32_t tdmsI,
	const int32_t row,
	const SpriteId spriteId
) {
	render_row_text_8_p1(
		(TextGrid8_t *)d1,
		(Pallet1_t *)d2,
		dr,
		dg,
		db,
		tdmsI,
		row
	);
}

inline void __not_in_flash_func(render_Tile16x16p2)(
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

inline void __not_in_flash_func(sprite_renderer_altx_16x8_p1)(
	const void* d1,
	const void* d2,
	uint32_t * const dr,
	uint32_t * const dg,
	uint32_t * const db,
	const int32_t tdmsI,
	const int32_t row,
	const SpriteId spriteId
) {
	const Tile16x8p2_t *tile = (Tile16x8p2_t *)d1;
	render_Tile16x8p1(
		&tile[(tdmsI >> 2) & 1],
		d2,
		dr,
		dg,
		db,
		tdmsI,
		row,
		spriteId
	);
}

void __not_in_flash_func(core1_render_loop)(struct dvi_inst *dvi0) {
	uint32_t frames = 0;
	while (true) {
		clear_sprite_collisions();
		for (uint32_t y = 0; y < FRAME_HEIGHT; ++y) {
			uint32_t *tmdsbuf;
			clear_sprite_id_row();
			queue_remove_blocking(&dvi0->q_tmds_free, &tmdsbuf);
			uint32_t *db = tmdsbuf;
			uint32_t *dg = db + FRAME_WIDTH;
			uint32_t *dr = dg + FRAME_WIDTH;

			// Render a blank row
			// TODO optionally render a tiled background
			render_row_mono(
				dr, dg, db,
				0, 0, 0);

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
			queue_add_blocking(&dvi0->q_tmds_valid, &tmdsbuf);
		}
		++frames;

        core1_render_inter_frame(
            frames,
            _sprites,
            &_spriteCollisions
        );
	}
}
