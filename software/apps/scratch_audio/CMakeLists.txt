add_executable(scratch_audio
	inv_samples.c
	ug_renderer.c
	ug_terminal.c
	main.c
	us_patch.c
	us_patch_1.c
	us_patch_sample.c
	us_lpf.c
	us_pitch.c
	us_tuner.c
	us_wave.c
	us_adsr.c
	us_uint8_dlist.c
	us_channel.c
	us_channels.c
	us_pm.c
	us_midi_uart.c
	us_midi_in.c
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")

target_compile_definitions(scratch_audio PRIVATE
	DVI_VERTICAL_REPEAT=2
	DVI_N_TMDS_BUFFERS=3	
)

# We have a lot in SRAM4 (particularly TMDS LUT) but don't need much stack on
# core 1. Probably even 256 bytes would be fine.
target_compile_definitions(scratch_audio PRIVATE
	DVI_DEFAULT_SERIAL_CONFIG=${DVI_DEFAULT_SERIAL_CONFIG}
	PICO_CORE1_STACK_SIZE=0x400
)

target_link_libraries(scratch_audio
	pico_stdlib
	pico_multicore
	libdvi
)

pico_enable_stdio_uart(scratch_audio 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(scratch_audio)
