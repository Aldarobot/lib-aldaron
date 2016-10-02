/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#ifndef LA_AUDIO
#define LA_AUDIO

#include <la_config.h>
#ifndef LA_FEATURE_AUDIO
	#error "please add #define LA_FEATURE_AUDIO to your la_config.h"
#endif

#include <la_buffer.h>
#include <la_math.h>

// Types:
typedef struct {
	void *audio;
	int32_t channel;
}la_sound_t;

typedef struct{
	double pofr; // Point Of Return (Where Music Should Start)
	int32_t num_channels;
}la_audio_t;

// Prototypes:
void la_audio_play(la_audio_t* jlau, la_sound_t* audio, float in, la_v3_t* vec);
void la_audio_lock(la_audio_t* jlau, la_sound_t* audio, float in, la_v3_t* vec);
void la_audio_pause(la_audio_t* jlau, la_sound_t* audio);
void la_audio_resume(la_audio_t* jlau, la_sound_t* audio);
uint8_t la_audio_wait(la_audio_t* jlau, la_sound_t* w_audio, la_sound_t* n_audio,
	float in, la_v3_t* vec);
void la_audio_stop(la_sound_t* audio, float out);
void la_audio_load(la_audio_t* jlau, la_sound_t* audio, la_buffer_t* zipdata,
	const char* filename, uint8_t music);

#endif
