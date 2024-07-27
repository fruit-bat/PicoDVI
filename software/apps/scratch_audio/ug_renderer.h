#pragma once
#include "pico/stdlib.h"
#include "dvi.h"

#define FRAME_WIDTH 320
#define FRAME_HEIGHT 240

#define MAX_SPRITES (1<<6)

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

typedef struct {
	uint16_t w;
	uint8_t *s;
} TextGrid8_t;

typedef uint8_t SpriteId;
typedef uint8_t SpriteCollisionMask;

typedef union {
	SpriteCollisionMask m[MAX_SPRITES];
	uint32_t word[MAX_SPRITES >> 2];
} SpriteCollisions;

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

void init_sprites();

void init_sprite(
	const int i,
	const int32_t x,
	const int32_t y,
	const uint32_t w,
	const uint32_t h,
	const uint32_t f,
	void * const d1, 
	void * const d2,
	const SpriteRenderer r,
    const SpriteCollisionMask m
);

void sprite_renderer_sprite_16x8_p1(
	const void* d1,
	const void* d2,
	uint32_t * const dr,
	uint32_t * const dg,
	uint32_t * const db,
	const int32_t tdmsI,
	const int32_t row,
	const SpriteId spriteId
);

void sprite_renderer_sprite_16x16_p1(
	const void* d1,
	const void* d2,
	uint32_t * const dr,
	uint32_t * const dg,
	uint32_t * const db,
	const int32_t tdmsI,
	const int32_t row,
	const SpriteId spriteId
);

void sprite_renderer_sprite_32x16_p1(
	const void* d1,
	const void* d2,
	uint32_t * const dr,
	uint32_t * const dg,
	uint32_t * const db,
	const int32_t tdmsI,
	const int32_t row,
	const SpriteId spriteId
);

void text_renderer_8x8_p1(
	const void* d1,
	const void* d2,
	uint32_t * const dr,
	uint32_t * const dg,
	uint32_t * const db,
	const int32_t tdmsI,
	const int32_t row,
	const SpriteId spriteId
);

void render_Tile16x16p2(
	const Tile16x16p2_t *t,
	const Pallet2_t *p,
	uint32_t * const dr,
	uint32_t * const dg,
	uint32_t * const db,
	const int32_t tdmsI,
	const int32_t row
);

void sprite_renderer_altx_16x8_p1(
	const void* d1,
	const void* d2,
	uint32_t * const dr,
	uint32_t * const dg,
	uint32_t * const db,
	const int32_t tdmsI,
	const int32_t row,
	const SpriteId spriteId
);

void core1_render_loop(struct dvi_inst *dvi0);

void core1_render_inter_frame(
	const uint32_t frames,
	Sprite * const sprites,
	const SpriteCollisions *spriteCollisions
);
