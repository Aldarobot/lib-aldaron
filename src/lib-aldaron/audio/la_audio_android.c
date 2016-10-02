/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include "la.h"

#ifdef LA_PHONE_ANDROID
#include "la_audio.h"
#include "la_memory.h"

#define LA_AUDIO_CHANNEL_MUSIC -2
#define LA_AUDIO_CHANNEL_SOUND -3
#define LA_AUDIO_CHANNEL_LOCK -4

/** @cond **/
static inline int32_t la_audio_sec2ms__(float sec) {
	int32_t ms = ((int32_t)(sec * 1000.f));
	if(ms < 20) ms = 0;
	return ms;
}

/**
 * Load audio from data pointed to by "data" of length "dataSize" into "audio",
 * set volume of audio to "volumeChange"
 */
static inline void la_audio_load2(la_audio_t* jlau, la_sound_t* audio,
	uint8_t volumeChange, const void *data, uint32_t dataSize,
	uint8_t isMusic)
{
}
/** @endcond **/

/** 
 * Fade in audio.
 * @param jlau: The jlau library context.
 * @param audio: The audio to play.
 * @param in: Seconds of fade-in.
 * @param vec: Where sound is coming from, or NULL for no position effect.
*/
void la_audio_play(la_audio_t* jlau, la_sound_t* audio, float in, la_v3_t* vec) {
	int32_t ms = la_audio_sec2ms__(in);

	if(audio->channel == LA_AUDIO_CHANNEL_MUSIC) {

	}else{

	}
}

void la_audio_lock(la_audio_t* jlau, la_sound_t* audio, float in, la_v3_t* vec) {
	if(audio->channel != LA_AUDIO_CHANNEL_LOCK) la_audio_play(jlau, audio, in, vec);
	audio->channel = LA_AUDIO_CHANNEL_LOCK;
}

void la_audio_pause(la_audio_t* jlau, la_sound_t* audio) {
}

void la_audio_resume(la_audio_t* jlau, la_sound_t* audio) {
}

/**
 * Wait on an audio track to stop.  If it is stopped, fade in a new one.
 * @param jlau: The jlau library context.
 * @param w_audio: The audio to wait on, NULL to wait on music to finish.
 * @param n_audio: The audio to play.
 * @param in: Seconds of fade-in.
 * @param vec: Where sound is coming from, or NULL for no position effect.
 * @returns 1: Audio has finished, and new audio is playing
 * @returns 0: Audio is still playing.
**/
uint8_t la_audio_wait(la_audio_t* jlau, la_sound_t* w_audio, la_sound_t* n_audio,
	float in, la_v3_t* vec)
{
	// If audio is still playing, quit
	//
	la_audio_play(jlau, n_audio, in, vec);
	return 1;
}

/**
 * Fade out a currently playing audio (if there is any) for "out" seconds
 * @param audio: Audio to fade out, or NULL to fade out all audio.
 * @param out: How many seconds to fade audio.
*/
void la_audio_stop(la_sound_t* audio, float out) {
}

void la_audio_load(la_audio_t* jlau, la_sound_t* audio, la_buffer_t* zipdata,
	const char* filename, uint8_t music)
{
	la_buffer_t aud;

	la_file_loadzip(&aud, zipdata, filename);
	la_print("Loading audiostuffs....");
	la_audio_load2(jlau, audio, 255, aud.data, aud.size, music);
	la_print("Loaded audiostuffs!");
}

la_audio_t* la_audio_init(void) {
	la_audio_t* jlau = la_memory_allocate(sizeof(la_audio_t));

	jlau->num_channels = 0;
	return jlau;
}

void la_audio_kill(la_audio_t* jlau) {
}
#endif
