/*
 * JL_lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLAU.c
 *	Audio
 * 		This library can play/record music/sound effects.
*/
#include "jl_pr.h"
#include "JLau.h"

#define JLAU_DEBUG_CHECK(jlau) jlau_checkthread__(jlau)

void jlau_checkthread__(jlau_t* jlau) {
	uint8_t thread = jl_thread_current(jlau->jl);
	if(thread != 0) {
		jl_print(jlau->jl, "Audio fn is on the Wrong Thread: %d",
			thread);
		jl_print(jlau->jl, "Must be on thread 1!");
			jl_print_stacktrace(jlau->jl);
		exit(-1);
	}
}

/**
 * load music from data pointed to by "data" of length "dataSize" into "music",
 * set volume of music to "p_vol"
 */
void jlau_load(jlau_t* jlau, jlau_music_t* music, uint8_t volumeChange,
	const void *data, uint32_t dataSize)
{
	SDL_RWops *rw;

	JLAU_DEBUG_CHECK(jlau);
	jl_print_function(jlau->jl, "AU_Load");
	JL_PRINT_DEBUG(jlau->jl, "ausize: %d", dataSize);
	JL_PRINT_DEBUG(jlau->jl, "audata: \"%4s\"", data);
	rw = SDL_RWFromConstMem(data, dataSize);
	music->_MUS = Mix_LoadMUS_RW(rw, 1);
	if(music->_MUS == NULL) {
		jl_print(jlau->jl, ":Couldn't load music because: %s",
			(char *)SDL_GetError());
		exit(-1);
	}
	music->_VOL = volumeChange;
//	SDL_RWclose(rw);
	JL_PRINT_DEBUG(jlau->jl, "Loaded music!");
	jl_print_return(jlau->jl, "AU_Load");
}

/**
 * Test if music is playing. 
 * @returns 1: if music is playing
 * @returns 0: otherwise
*/
uint8_t jlau_mus_playing(void) {
	return Mix_PlayingMusic();
}

/**
 * set where the music is coming from.  
 * @param left: How much to the left compared to the right.
 *	255 is left 0 is right
 * @param toCenter: Overall volume
*/
void jlau_panning(uint8_t left, uint8_t toCenter) {
	uint8_t right = 255 - left;
	if(toCenter) {
		if(right > left) {
			left += (uint8_t)(((float)right/255.f) * ((float)toCenter));
		}else{
			right+= (uint8_t)(((float)left /255.f) * ((float)toCenter));
		}
	}
	Mix_SetPanning(MIX_CHANNEL_POST, left, right);
}

/**
 * sets where music is comming from to center ( resets panning )
*/
void jlau_panning_default(void) {
	Mix_SetPanning(MIX_CHANNEL_POST,255,255);
}

/** @cond **/
void _jlau_play(jlau_t* jlau) {
	Mix_VolumeMusic(jlau->next._VOL);
	Mix_FadeInMusic(jlau->next._MUS, 1, (int)(jlau->fade * 1000.));
}

/** @endcond **/

/**
 * fade out any previously playing music (if there is any) for
 * "p_secondsOfFadeOut" seconds
 * @param out: How many seconds to fade music.
*/
void jlau_mus_halt(float out) {
	Mix_FadeOutMusic((int)(out * 1000.));
}

/** 
 * fade out any previously playing music (If there is any) for
 * "p_secondsOfFadeOut" seconds, then fade in music with ID "IDinStack"
 * for "p_secondsOfFadeIn" seconds
 * @param jlau: The jlau library context.
 * @param music: The music to play.
 * @param out: Seconds of fade-out.
 * @param in: Seconds of fade-in.
*/
void jlau_play_music(jlau_t* jlau, jlau_music_t* music, float out, float in) {
	JLAU_DEBUG_CHECK(jlau);
	jlau->next = *music;
	jlau->fade = in;
	//If music playing already, then halt
	if(Mix_PlayingMusic()) jlau_mus_halt(out);
	//If music not playing, then start.
	else _jlau_play(jlau);
}

/** @cond **/
// TODO: Figure out whether needed or not.
void jlau_loop(jlau_t* jlau) {
	JLAU_DEBUG_CHECK(jlau);
	//If Music Isn't playing AND Music isn't disabled: play music
	if(!Mix_PlayingMusic()) {
		_jlau_play(jlau);
	}
}
/** @endcond **/

/**
 * Load an audiotrack from a zipfile.
 * info: info is set to number of audiotracks loaded.
 * @param jlau: The library context
 * @param music: Unintialized music object.
 * @param zipdata: data for a zip file.
 * @param filename: Name of the audio file in the package.
*/
void jlau_add_audio(jlau_t* jlau, jlau_music_t* music, data_t* zipdata,
	const char* filename)
{
	data_t aud;

	JLAU_DEBUG_CHECK(jlau);
	jl_file_pk_load_fdata(jlau->jl, &aud, zipdata, filename);
	jl_print_function(jlau->jl, "jlau_add_audio");
	JL_PRINT_DEBUG(jlau->jl, "Loading audiostuffs....");
	jlau_load(jlau, music, 255, aud.data, aud.size);
	JL_PRINT_DEBUG(jlau->jl, "Loaded audiostuffs!");
	jl_print_return(jlau->jl, "jlau_add_audio");
}

/** @cond **/
static inline void _jlau_print_openblock(jl_t* jl) {
	jl_print_function(jl, "JLaudio");
}

static inline void _jlau_print_closeblock(jl_t* jl) {
	jl_print_return(jl, "JLaudio");
}

jlau_t* jlau_init(jl_t* jl) {
	jlau_t* jlau = jl_memi(jl, sizeof(jlau_t));

	jlau->jl = jl;
	jl->jlau = jlau;
	JLAU_DEBUG_CHECK(jlau);

	jlau->next._MUS = NULL; 
	//Open Block AUDI
	_jlau_print_openblock(jl);
	// Open the audio device
	JL_PRINT_DEBUG(jl, "initializing audio....");
	if ( Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 1024) < 0 ) {
		jl_print(jl,
			":Couldn't set 11025 Hz 16-bit audio because: %s",
			(char *)SDL_GetError());
		_jlau_print_closeblock(jl);
		jl_sg_kill(jl);
	}else{
		JL_PRINT_DEBUG(jl, "audio has been set.");
	}
	//Close Block AUDI
	_jlau_print_closeblock(jl);
	// Return the context.
	return jlau;
}

void jlau_kill(jlau_t* jlau) {
	JLAU_DEBUG_CHECK(jlau);
	//Open Block AUDI
	_jlau_print_openblock(jlau->jl);
	JL_PRINT_DEBUG(jlau->jl, "Quiting....");
	//Free Everything
	Mix_CloseAudio();	
	JL_PRINT_DEBUG(jlau->jl, "Quit Successfully!");
	//Close Block AUDI
	_jlau_print_closeblock(jlau->jl);
}
/** @endcond **/
