/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <assert.h>
#include <la_draw.h>

#if defined(LA_PHONE)
#include "SDL_system.h"
#else
#include "SDL.h"
#include "SDL_mixer.h"
#endif

#include "la_memory.h"
#include "la_time.h"

#include <la.h>
#include <la_file.h>

void la_time_init__(void);

void la_window_start__(void*,la_window_t*,la_draw_fn_t,la_fn_t*,la_fn_t);

#if defined(LA_PHONE)
	#include <jni.h>
	#include "SDL_log.h"

	const char* LA_FILE_ROOT = NULL;
	const char* LA_FILE_LOG = NULL;
	char la_keyboard_press = 0;
	extern la_window_t* la_window;
#endif

float la_banner_size = 0.f;
SDL_atomic_t la_rmcexit;
static char la_errorv[256];

static inline void la_init__(void* ctx, uint64_t ctx_size) {
	la_memory_clear(ctx, ctx_size);
	// Clear error file
	la_file_truncate(NULL);
	// Reset Time
	la_time_init__();
	// Initialize the SDL
	SDL_Init(0
#ifndef LA_ANDROID
	#ifdef LA_FEATURE_AUDIO
		| SDL_INIT_AUDIO
	#endif
	#ifdef LA_FEATURE_DISPLAY
		| SDL_INIT_VIDEO
	#endif
#endif
		);
#ifndef LA_ANDROID
#ifdef LA_FEATURE_AUDIO
	// Open the audio device
	Mix_Init(MIX_INIT_OGG);
	if ( Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 1024) < 0 ) {
		la_panic("Couldn't set 11025 Hz 16-bit audio because: %s",
			(char *)SDL_GetError());
	}else{
		la_print("audio has been set.");
	}
#endif
#endif
}

// EXPORT FUNCTIONS

/**
 * Exit The program on an error.
**/
void la_panic(const char* format, ...) {
	char temp[256];
	va_list arglist;

	va_start(arglist, format);
	vsprintf(temp, format, arglist);
	va_end(arglist);

	la_print("%s", temp);
#ifdef LA_COMPUTER
	assert(0);
#else
	exit(-1);
#endif
}

/**
 * Do Nothing
**/
void la_dont(void* context) { }

const char* la_error(const char* format, ...) {
	va_list arglist;

	va_start( arglist, format );
	vsnprintf(la_errorv, 256, format, arglist);
	va_end( arglist );

	return la_errorv;
}

/**
 * @param fnc_init: The function initialize the program.
 * @param fnc_kill: The function to free anything that needs to be freed.
 * @param name: The name of the program, used for storage / window name etc.
 * @param ctx_size: The size of the program context.
**/
int32_t la_start(void* fnc_init, la_fn_t* loop, la_fn_t fnc_kill,
	void* ctx, uint64_t ctx_size)
{
	SDL_AtomicSet(&la_rmcexit, 1);

#ifndef LA_ANDROID
#ifdef LA_FEATURE_DISPLAY
	la_window_t* la_window = la_memory_allocate(sizeof(la_window_t));
#endif
#endif

	// Initialize Lib Aldaron!
	la_init__(ctx, ctx_size);

	// Open a window, if LA_FEATURE_DISPLAY is set.
#ifdef LA_FEATURE_DISPLAY
	la_window_start__(ctx, la_window, fnc_init, loop, fnc_kill);
#else
	((la_fn_t) fnc_init)(ctx);
	while(SDL_AtomicGet(&la_rmcexit)) {
		(*loop)(ctx);
	}
	fnc_kill(ctx);
#endif
#ifndef LA_ANDROID
	#ifdef LA_FEATURE_AUDIO
		Mix_CloseAudio();
	#endif
	// Exit.
	la_print("SDL_Quit(), Exit 0");
	SDL_Quit();
#endif
	return 0;
}
