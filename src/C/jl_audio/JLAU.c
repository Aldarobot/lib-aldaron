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
 * load music from data pointed to by "data" of length "dataSize" into slot
 * "IDinStack", set volume of music to "p_vol"
 */
void jlau_load(jlau_t* jlau, u32_t IDinStack, const void *data, u32_t dataSize,
	u8_t volumeChange)
{
	JLAU_DEBUG_CHECK(jlau);
	jl_print(jlau->jl, "m %p", jlau->jmus);
	SDL_RWops *rw;
	Mix_Music* music;

	//Open Block "AUDI/LOAD"
	jl_print_function(jlau->jl, "AU_Load");

	jl_print(jlau->jl,"loading music %d....",IDinStack);
	jl_print(jlau->jl, "ausize: %d", dataSize);
	jl_print(jlau->jl, "audata: \"%4s\"", data);
	rw = SDL_RWFromConstMem(data, dataSize);
	jl_print(jlau->jl, "Read step 2:");
	music = Mix_LoadMUS_RW(rw, 1);
	jl_print(jlau->jl, "Read step 4: %p", jlau->jmus);
	jlau->jmus[IDinStack]._MUS = music;
	jl_print(jlau->jl, "Read step 3:");
	jlau->jmus[IDinStack]._VOL = volumeChange;
	if(jlau->jmus[IDinStack]._MUS == NULL) {
		jl_print(jlau->jl, ":Couldn't load music because: %s",
			(char *)SDL_GetError());
		jl_sg_kill(jlau->jl);
	}else{
		jl_print(jlau->jl, "loaded music #%d!", IDinStack);
	}
	
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
	Mix_VolumeMusic(jlau->jmus[jlau->idis]._VOL);
	Mix_FadeInMusic(jlau->jmus[jlau->idis]._MUS, 1, jlau->sofi * 1000);
}

/** @endcond **/

/**
 * fade out any previously playing music (if there is any) for
 * "p_secondsOfFadeOut" seconds
 * @param p_secondsOfFadeOut: How many seconds to fade music.
*/
void jlau_mus_halt(u8_t p_secondsOfFadeOut) {
	Mix_FadeOutMusic(p_secondsOfFadeOut * 1000);
}

/** 
 * fade out any previously playing music (If there is any) for
 * "p_secondsOfFadeOut" seconds, then fade in music with ID "IDinStack"
 * for "p_secondsOfFadeIn" seconds
 * @param jlau: The jlau library context.
 * @param 
*/
void jlau_mus_play(jlau_t* jlau, u32_t IDinStack, u8_t secondsOfFadeOut,
	u8_t secondsOfFadeIn)
{
	JLAU_DEBUG_CHECK(jlau);
	jlau->idis = IDinStack;
	jlau->sofi = secondsOfFadeIn;
	jlau->sofo = secondsOfFadeOut;
	//If music playing already, then halt
	if(Mix_PlayingMusic()) jlau_mus_halt(jlau->sofo);
	//If music not playing, then start.
	else _jlau_play(jlau);
}

/** @cond **/
// TODO: Figure out whether needed or not.
void jlau_loop(jlau_t* jlau) {
	JLAU_DEBUG_CHECK(jlau);
	//If Music Isn't playing AND Music isn't disabled: play music
/*	if ( !Mix_PlayingMusic() && (_jl->au.idis!=UINT32_MAX)) {
		_jlau_play(_jl);
	}*/
}
/** @endcond **/

static inline void _jlau_init_sounds(jlau_t* jlau, uint8_t *data) {
	uint32_t fil = 0;
	uint32_t fid = 0;

	jl_print_function(jlau->jl, "AU_Load");

//	uint32_t cursor = 0;

	jl_print(jlau->jl, "meanwhile....");
	jl_print(jlau->jl, "m %p", jlau->jmus);
	while(1) {
		uint32_t *bytes = (void *)(data + fil);
		uint32_t size = *bytes;

		if(size == 0) break; // If Size Is 0 signal to stop.
		jl_print(jlau->jl,"getting data of size %d....\n", size);
		fil += sizeof(uint32_t); //move init next location
		jl_print(jlau->jl,"jlau_load() we are at [data+%d]",fil);
		jlau_load(jlau,fid,data + fil,size,255);
		fil += size; //move init next location
		fid++; //increase to the next music id
	}
	jlau->jl->info = fid;
	jl_print(jlau->jl, "loaded music!");
	
	jl_print_return(jlau->jl, "AU_Load");
}

/**
 * Load all audiotracks from a zipfile and give them ID's.
 * info: info is set to number of images loaded.
 * @param jl: library context
 * @param pzipfile: full file name of a zip file.
 * @param pigid: which audio group to load the soundtracks into.
*/
void jlau_add_audio(jlau_t* jlau, str_t pzipfile, uint16_t pigid) {
	JLAU_DEBUG_CHECK(jlau);
	data_t aud;
	jl_file_media(jlau->jl, &aud, "jlex/2/_aud", pzipfile,
		jl_gem(), jl_gem_size());
	jl_print_function(jlau->jl, "AU_Load");
	jl_print(jlau->jl, "Audio Size: %d", jlau->jl->info);
	jl_print(jlau->jl, "Loading audiostuffs....");
	if(jlau->jl->info > 4) {
		_jlau_init_sounds(jlau, aud.data);
	}
	jl_print(jlau->jl, "Loaded audiostuffs!");
	jl_print_return(jlau->jl, "AU_Load");
}

/** @cond **/
static inline void _jlau_print_openblock(jl_t* jl) {
	jl_print_function(jl, "AU");
}

static inline void _jlau_print_closeblock(jl_t* jl) {
	jl_print_return(jl, "AU");
}

jlau_t* jlau_init(jl_t* jl) {
	jlau_t* jlau = jl_memi(jl, sizeof(jlau_t));

	jlau->jl = jl;
	jl->jlau = jlau;
	JLAU_DEBUG_CHECK(jlau);
	//audio by default is disabled
	jlau->jmus = jl_memi(jlau->jl, 10 * sizeof(jlau_jmus_t__));
	jlau->total = 10;
	jl_print(jlau->jl, "m %p", jlau->jmus);

	jlau->idis = UINT32_MAX; 
	//Open Block AUDI
	_jlau_print_openblock(jl);
	// Open the audio device
	jl_print(jl, "initializing audio....");
	if ( Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 1024) < 0 ) {
		jl_print(jl,
			":Couldn't set 11025 Hz 16-bit audio because: %s",
			(char *)SDL_GetError());
		_jlau_print_closeblock(jl);
		jl_sg_kill(jl);
	}else{
		jl_print(jl, "audio has been set.");
	}
	//Load Sound Effects & Music
//	jlau_add_audio(jl,jl_file_get_resloc(jl, JL_MAIN_DIR, JL_MAIN_MEF), 0);
//	jl_gr_draw_msge(jl, 0, 0, 0, "LOADED AUDIOSTUFFS!");
	//Close Block AUDI
	_jlau_print_closeblock(jl);
	// Return the context.
	return jlau;
}

void jlau_kill(jlau_t* jlau) {
	JLAU_DEBUG_CHECK(jlau);
	//Open Block AUDI
	_jlau_print_openblock(jlau->jl);
	jl_print(jlau->jl, "Quiting....");
	//Free Everything
	Mix_CloseAudio();
	m_u32_t i;
	for(i = 0; i < jlau->total; i++) {
		Mix_FreeMusic(jlau->jmus[i]._MUS);
	}
	free(jlau->jmus);
	
	jl_print(jlau->jl, "Quit Successfully!");
	//Close Block AUDI
	_jlau_print_closeblock(jlau->jl);
}
/** @endcond **/
