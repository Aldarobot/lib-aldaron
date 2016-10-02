/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <assert.h>
#include <la_draw.h>

#if JL_PLAT == JL_PLAT_PHONE
#include "SDL_system.h"
#else
#include "SDL.h"
#endif

#include "la_memory.h"
#include "la_time.h"

#include <la.h>
#include <la_file.h>

void la_time_init__(void);

void la_window_start__(void*,la_window_t*,la_draw_fn_t,la_fn_t,la_fn_t,
	const char*);

#if JL_PLAT == JL_PLAT_PHONE
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

static inline void* la_init__(const char* nm, uint64_t ctx1s) {
	void* context = la_memory_allocate(ctx1s);
	// Clear error file
	la_file_truncate(NULL);
	// Reset Time
	la_time_init__();
	// Initialize the SDL
	SDL_Init(0);
	return context;
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
 * @param jl: The library's context.
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
 * Start Lib-Aldaron on a separate thread.
 * @param fnc_init: The function initialize the program.
 * @param fnc_kill: The function to free anything that needs to be freed.
 * @param name: The name of the program, used for storage / window name etc.
 * @param ctx_size: The size of the program context.
**/
int32_t la_start(void* fnc_init, la_fn_t fnc_loop, la_fn_t fnc_kill,
	uint8_t openwindow, const char* name, size_t ctx_size)
{
	SDL_AtomicSet(&la_rmcexit, 1);

#ifndef LA_PHONE_ANDROID
	la_window_t* la_window = openwindow ?
		la_memory_allocate(sizeof(la_window_t)) : NULL;
#endif

	// Initialize Lib Aldaron!
	void* context = la_init__(name, ctx_size);

	// Open a window, if "openwindow" is set.
	if(openwindow) {
		la_window_start__(context, la_window, fnc_init, fnc_loop,
			fnc_kill, name);
	}else{
		((la_fn_t) fnc_init)(context);
		while(SDL_AtomicGet(&la_rmcexit))
			fnc_loop(context);
		fnc_kill(context);
	}
#ifndef LA_PHONE_ANDROID
	// Exit.
	la_print("SDL_Quit()");
	SDL_Quit();
	la_print("| Free Context & Exit |");
	free(context);
#endif
	return 0;
}
