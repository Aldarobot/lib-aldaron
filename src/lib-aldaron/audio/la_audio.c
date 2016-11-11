/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_config.h>
#ifdef LA_FEATURE_AUDIO

#include <la_audio.h>
#include <la_memory.h>
#include <la_file.h>
#include <la.h>

#include "SDL_mixer.h"

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
static inline void la_audio_load2(la_sound_t* audio,
	uint8_t volumeChange, const void *data, uint32_t dataSize,
	uint8_t isMusic)
{
#ifdef LA_ANDROID
#else
	SDL_RWops *rw;

	la_print("ausize: %d", dataSize);
	la_print("audata: \"%4s\"", data);
	rw = SDL_RWFromConstMem(data, dataSize);
	if(isMusic)
		audio->audio = Mix_LoadMUS_RW(rw, 1);
	else
		audio->audio = Mix_LoadWAV_RW(rw, 1);
	if(audio->audio == NULL) {
		la_panic("Couldn't load audio because: %s", (char *)
			SDL_GetError());
	}
	audio->channel = isMusic ? LA_AUDIO_CHANNEL_MUSIC : LA_AUDIO_CHANNEL_SOUND;
	Mix_VolumeChunk(audio->audio, volumeChange);
//	SDL_RWclose(rw);
	la_print("Loaded audio!");
#endif
}
/** @endcond **/

/** 
 * Fade in audio.
 * @param laa: The laa library context.
 * @param audio: The audio to play.
 * @param in: Seconds of fade-in.
 * @param vec: Where sound is coming from, or NULL for no position effect.
*/
void la_audio_play(la_audio_t* laa, la_sound_t* audio, float in, la_v3_t* vec) {
	int32_t ms = la_audio_sec2ms__(in);

	if(audio->channel == LA_AUDIO_CHANNEL_MUSIC) {
#ifdef LA_ANDROID
#else
		Mix_HaltMusic();
		// Get around SDL_Mixer Bug.
		if(ms >= 20) Mix_FadeInMusic(audio->audio, 1, ms);
		else Mix_PlayMusic(audio->audio, 1);
#endif
	}else{
#ifdef LA_ANDROID
#else
		LA_AUDIO_PLAY_MIX_PLAY_CHANNEL:
		if((audio->channel = Mix_FadeInChannel(-1, audio->audio, 0,
			ms + 1)) == -1)
		{
			laa->num_channels++;
			Mix_ReserveChannels(laa->num_channels);
			goto LA_AUDIO_PLAY_MIX_PLAY_CHANNEL;
		}
		// Calculate Position
		if(vec == NULL) {
			Mix_SetPosition(audio->channel, 0, 0);
		}else if(vec->x == 0 && vec->y == 0 && vec->z == 0) {
			Mix_SetPosition(audio->channel, 0, 0);
		}else{
			float distance = sqrtf((vec->x*vec->x) + (vec->y*vec->y)
				+ (vec->z*vec->z));
			float angle = atan2f(vec->z, vec->x) * (180.f / M_PI);
			if(angle < 0.f) angle += 360.f;

			Mix_SetPosition(audio->channel, angle, distance);
		}
#endif
	}
}

void la_audio_lock(la_audio_t* laa, la_sound_t* audio, float in, la_v3_t* vec) {
	if(audio->channel != LA_AUDIO_CHANNEL_LOCK) la_audio_play(laa, audio, in, vec);
	audio->channel = LA_AUDIO_CHANNEL_LOCK;
}

void la_audio_pause(la_sound_t* audio) {
#ifdef LA_ANDROID
#else
	if(audio->channel == LA_AUDIO_CHANNEL_MUSIC)
		Mix_PauseMusic();
	else
		Mix_Pause(audio->channel);
#endif
}

void la_audio_resume(la_sound_t* audio) {
#ifdef LA_ANDROID
#else
	if(audio->channel == LA_AUDIO_CHANNEL_MUSIC)
		Mix_ResumeMusic();
	else
		Mix_Resume(audio->channel);
#endif
}

/**
 * Wait on an audio track to stop.  If it is stopped, fade in a new one.
 * @param laa: The laa library context.
 * @param w_audio: The audio to wait on, NULL to wait on music to finish.
 * @param n_audio: The audio to play.
 * @param in: Seconds of fade-in.
 * @param vec: Where sound is coming from, or NULL for no position effect.
 * @returns 1: Audio has finished, and new audio is playing
 * @returns 0: Audio is still playing.
**/
uint8_t la_audio_wait(la_audio_t* laa, la_sound_t* w_audio, la_sound_t* n_audio,
	float in, la_v3_t* vec)
{
#ifdef LA_ANDROID
#else
	// If audio is still playing, quit
	if(w_audio ? Mix_Playing(w_audio->channel) : Mix_PlayingMusic())
		return 0;
#endif
	la_audio_play(laa, n_audio, in, vec);
	return 1;
}

void la_audio_stop(la_sound_t* audio, float out) {
#ifdef LA_ANDROID
#else
	int32_t channel = (audio == NULL) ? -1 : audio->channel;
	int32_t ms = la_audio_sec2ms__(out);

	if(channel == LA_AUDIO_CHANNEL_MUSIC)
		Mix_FadeOutMusic(ms + 1);
	else
		Mix_FadeOutChannel(channel, ms + 1);
#endif
}

void la_audio_load(la_sound_t* audio, la_buffer_t* zipdata,
	const char* filename, uint8_t music)
{
	la_buffer_t aud;

	la_file_loadzip(&aud, zipdata, filename);
	la_audio_load2(audio, 255, aud.data, aud.size, music);
}

#endif
