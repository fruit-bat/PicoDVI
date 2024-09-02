#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "hardware/irq.h"
#include "hardware/sync.h"
#include "hardware/gpio.h"
#include "hardware/vreg.h"
#include "hardware/structs/bus_ctrl.h"
#include "hardware/dma.h"
#include "pico/sem.h"

#include "dvi.h"
#include "dvi_serialiser.h"
#include "common_dvi_pin_configs.h"

// Graphics stuff
#include "ug_renderer.h"
#include "ug_terminal.h"

// Music stuff
#include "us_midi_uart.h"
#include "us_channels.h"
#include "us_pm.h"
#include "us_lpf.h"
#include "bach_packed_midi_1.h"
//#include "pitch_bend_packed_midi.h"
#include "us_patch_1.h"
#include "us_patch_sample.h"
#include "inv_samples.h"
#include "us_midi_in.h"

// End music stuff



#define FONT_CHAR_WIDTH 8
#define FONT_CHAR_HEIGHT 8
#define FONT_N_CHARS 95
#define FONT_FIRST_ASCII 32

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

#define US_PATCH1_COUNT 64
static UsPatch1Data patch1_data[US_PATCH1_COUNT];
static UsChannelPatchState patch1_state[US_PATCH1_COUNT];
static UsPatch1Config patch1_config;

#define US_PATCH2_COUNT 16
static UsPatch1Data patch2_data[US_PATCH2_COUNT];
static UsChannelPatchState patch2_state[US_PATCH2_COUNT];
static UsPatch1Config patch2_config;

#define US_PATCH_S1_COUNT 4
static UsPatchSampleData patch_s1_data[InvSampleCount][US_PATCH_S1_COUNT];
static UsChannelPatchState patch_s1_state[InvSampleCount][US_PATCH_S1_COUNT];
static UsPatchSampleConfig patch_s1_config[InvSampleCount];

static UsChannels channels;
static UsPmSequencer sequencer;
static UsLpf lpf_l;
static UsLpf lpf_r;
static UsMidiIn us_midi_in;

void setup_synth() {

	us_midi_uart_init();
	us_midi_in_init(&us_midi_in, &channels);

	us_channels_init(&channels);

	us_patch_1_init_config(&patch1_config);

	us_channel_set_patch(
		&channels.channel[0],
		us_patch_1_apply,
		&patch1_config,
		patch1_data,
		sizeof(UsPatch1Data),
		patch1_state,
		US_PATCH1_COUNT);

	us_patch_1_init_config(&patch2_config);

	us_channel_set_patch(
		&channels.channel[1],
		us_patch_1_apply,
		&patch2_config,
		patch2_data,
		sizeof(UsPatch1Data),
		patch2_state,
		US_PATCH2_COUNT);

	for (uint32_t i = 0; i < InvSampleCount; ++i) {

		UsPatchSampleConfig *config = &patch_s1_config[i];
		UsPatchSampleData *data = &patch_s1_data[i][0];
		UsChannelPatchState *state = &patch_s1_state[i][0];

		us_patch_sample_init_config(config, inv_sample(i), inv_sample_size(i));

		us_channel_set_patch(
			&channels.channel[2+i],
			us_patch_sample_apply,
			config,
			data,
			sizeof(UsPatchSampleData),
			state,
			US_PATCH_S1_COUNT);
	}

	us_pm_sequencer_init(&sequencer, &channels, syn_notes, true);

	us_lpf_init(&lpf_l, 25000);
	us_lpf_init(&lpf_r, 25000);
}

bool __not_in_flash_func(audio_timer_callback)(struct repeating_timer *t) {

	while(true) {
		int size = get_write_size(&dvi0.audio_ring, false);
		if (size == 0) return true;
		audio_sample_t *audio_ptr = get_write_pointer(&dvi0.audio_ring);
		audio_sample_t sample;
		for (int cnt = 0; cnt < size; cnt++) {	
			us_pm_sequencer_update(&sequencer);
			us_midi_in_update(&us_midi_in);
			us_channels_update(&channels);
			sample.channels[0] = us_lpf_sample(&lpf_l, channels.out_l);
			sample.channels[1] = us_lpf_sample(&lpf_r, channels.out_r);
			*audio_ptr++ = sample;
		}
		increase_write_pointer(&dvi0.audio_ring, size);
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
	{ (uint8_t)10 },
	{ (uint8_t)10 },
	{ (uint8_t)63 }
};

Pallet1_t pallet1_Purple = {
	{ (uint8_t)42 },
	{ (uint8_t)0 },
	{ (uint8_t)42 }
};

Pallet1_t pallet1_White = {
	{ (uint8_t)42 },
	{ (uint8_t)42 },
	{ (uint8_t)42 }
};

Tile16x8p2_t tile16x8p2_invader[] = {
	{{
		0b0000000110000000,
		0b0000001111000000,
		0b0000011111100000,
		0b0000110110110000,
		0b0000111111110000,
		0b0000010000100000,
		0b0000100000010000,
		0b0000010000100000,
	}},
	{{
		0b0000000110000000,
		0b0000001111000000,
		0b0000011111100000,
		0b0000110110110000,
		0b0000111111110000,
		0b0000001001000000,
		0b0000010110100000,
		0b0000101001010000,
	}},
	{{
		0b0000100000100000,
		0b0000010001000000,
		0b0000111111100000,
		0b0001101110110000,
		0b0011111111111000,
		0b0101111111110100,
		0b0101000000010100,
		0b0000111011100000,
	}},
	{{
		0b0000100000100000,
		0b0000010001000000,
		0b0100111111100100,
		0b0101101110110100,
		0b0111111111111100,
		0b0011111111111000,
		0b0001000000010000,
		0b0010000000001000,
	}},
	{{
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
		0b0000001111000000,
		0b0001111111111000,
		0b0011111111111100,
		0b0011001111001100,
		0b0011111111111100,
		0b0000111001110000,
		0b0001100110011000,
		0b0011000000001100,
	}},
	{{
		0b0000000000000000,
		0b0000001111000000,
		0b0001111111111000,
		0b0011111111111100,
		0b0110110110110110,
		0b1111111111111111,
		0b0011100110011100,
		0b0001000000001000,
	}},
	{{
		0b0000000010000000,
		0b0000000111000000,
		0b0000000111000000,
		0b0000111111111000,
		0b0011111111111110,
		0b0011111111111110,
		0b0011111111111110,
		0b0011111111111110,
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

const char *kubla = "In Xanadu did Kubla Khan \
A stately pleasure-dome decree: \
Where Alph, the sacred river, ran \
Through caverns measureless to man \
   Down to a sunless sea. \
So twice five miles of fertile ground \
With walls and towers were girdled round; \
And there were gardens bright with sinuous rills, \
Where blossomed many an incense-bearing tree; \
And here were forests ancient as the hills, \
Enfolding sunny spots of greenery. ";

static uint32_t inv_index;
static uint32_t mot_index;
static uint32_t gun_index;

static int32_t inv_v = 1;
void init_game() {

	uint32_t si = 0;	
	init_sprite(si++, 50, 15, 16, 8, SF_ENABLE, &tile16x8p2_invader, &pallet1_Green, sprite_renderer_altx_16x8_p1, (SpriteCollisionMask)1);
	init_sprite(si++, 66, 19, 16, 8, SF_ENABLE, &tile16x8p2_invader, &pallet1_Green, sprite_renderer_altx_16x8_p1, (SpriteCollisionMask)2);
	init_sprite(si++, 66, 200, 32, 16, SF_ENABLE, &tile32x16p2_base, &pallet1_Purple, sprite_renderer_sprite_32x16_p1, (SpriteCollisionMask)8);

	init_sprite(mot_index = si++, -1000, 9, 16, 8, SF_ENABLE, &tile16x8p2_invader[6], &pallet1_Red, sprite_renderer_sprite_16x8_p1, (SpriteCollisionMask)0);
	init_sprite(gun_index = si++, 20, FRAME_HEIGHT - 24, 16, 8, SF_ENABLE, &tile16x8p2_invader[7], &pallet1_Purple, sprite_renderer_sprite_16x8_p1, (SpriteCollisionMask)0);

	inv_index = si;
	uint32_t rt[5] = {0, 2, 2, 4, 4};
	Pallet1_t* rp[5] = {&pallet1_White, &pallet1_Blue, &pallet1_Blue, &pallet1_Purple, &pallet1_Purple};

	for(uint32_t x = 0; x < 11; ++x) {
		for(uint32_t y = 0; y < 5; ++y) {
			init_sprite(si, x << 4, 30 + (y << 4), 16, 8, SF_ENABLE, &tile16x8p2_invader[rt[y]], rp[y], sprite_renderer_altx_16x8_p1, (SpriteCollisionMask)4);
			si++;
		}
	}
	init_sprite(si++, 0, 0, UG_TERMINAL_WIDTH*8, UG_TERMINAL_HEIGHT*8, SF_ENABLE, ug_terminal_text_grid(), &pallet1_Green, text_renderer_8x8_p1, (SpriteCollisionMask)0);

}
void __not_in_flash_func(update_mother_ship)(Sprite *sprites) {	
	Sprite *sprite = &sprites[mot_index];
	sprite->x += 2;
	if(sprite->x > FRAME_WIDTH) sprite->x = -1000;
}

void __not_in_flash_func(core1_main)() {

	dvi_register_irqs_this_core(&dvi0, DMA_IRQ_0);
	dvi_start(&dvi0);
	core1_render_loop(&dvi0);

}

void core1_render_inter_frame(
	const uint32_t frames,
	Sprite * const sprites,
	const SpriteCollisions *spriteCollisions
) {

	// Just messing about - start
	update_mother_ship(sprites);

	for (uint32_t i = 0; i < 2; ++i)
	{
		Sprite *sprite = &sprites[i];
		if (spriteCollisions->m[i]) sprite->d2 = &pallet1_Red;
	}
	sprites[0].x++; if (sprites[0].x > FRAME_WIDTH) {
		sprites[0].x = -16;
		sprites[0].d2 = &pallet1_Blue;
	}
	sprites[1].x--; if (sprites[1].x < -16) {
		sprites[1].x = FRAME_WIDTH + 16; 
		sprites[1].d2 = &pallet1_Purple;
	}

	bool reverse = false;
	for (uint32_t i = inv_index; i < inv_index + (5*11); ++i)
	{
		Sprite *sprite = &sprites[i];
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

int __not_in_flash_func(main)() {
	vreg_set_voltage(VREG_VSEL);
	sleep_ms(10);
	// Run system at TMDS bit clock
	set_sys_clock_khz(DVI_TIMING.bit_clk_khz, true);

	// setup_default_uart();
	stdio_init_all();
	ug_terminal_init();

	dvi0.timing = &DVI_TIMING;
	dvi0.ser_cfg = DVI_DEFAULT_SERIAL_CONFIG;
	dvi_init(&dvi0, next_striped_spin_lock_num(), next_striped_spin_lock_num());

//	hw_set_bits(&bus_ctrl_hw->priority, BUSCTRL_BUS_PRIORITY_PROC1_BITS);

	setup_synth();

	// HDMI Audio related
	dvi_get_blank_settings(&dvi0)->top    = 0;
	dvi_get_blank_settings(&dvi0)->bottom = 0;
	dvi_audio_sample_buffer_set(&dvi0, audio_buffer, AUDIO_BUFFER_SIZE);
    dvi_set_audio_freq(&dvi0, AUDIO_RATE, dvi0.timing->bit_clk_khz*HDMI_N/(AUDIO_RATE/100)/128, HDMI_N);
	add_repeating_timer_ms(-2, audio_timer_callback, NULL, &audio_timer);

	init_sprites();
	
	init_game();

	multicore_launch_core1(core1_main);

  	while (1)
		__wfi();
}
	
