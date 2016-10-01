/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#ifndef LA_AUDIO
#define LA_AUDIO

#include <la_buffer.h>
#include <la_math.h>

// Types:
typedef struct {
	void *audio;
	int32_t channel;
}jlau_audio_t;

typedef struct{
	double pofr; // Point Of Return (Where Music Should Start)
	int32_t num_channels;
}jlau_t;

// Prototypes:
void jlau_play(jlau_t* jlau, jlau_audio_t* audio, float in, la_v3_t* vec);
void jlau_lock(jlau_t* jlau, jlau_audio_t* audio, float in, la_v3_t* vec);
void jlau_pause(jlau_t* jlau, jlau_audio_t* audio);
void jlau_resume(jlau_t* jlau, jlau_audio_t* audio);
uint8_t jlau_wait(jlau_t* jlau, jlau_audio_t* w_audio, jlau_audio_t* n_audio,
	float in, la_v3_t* vec);
void jlau_stop(jlau_audio_t* audio, float out);
void jlau_add_audio(jlau_t* jlau, jlau_audio_t* audio, la_buffer_t* zipdata,
	const char* filename, uint8_t music);
jlau_t* jlau_init(void);

#endif
