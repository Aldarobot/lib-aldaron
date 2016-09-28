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

#include "la_thread.h"
#include "la_memory.h"
#include "la_time.h"

#include <la.h>
#include <la_file.h>
#include <la_audio.h>

void la_time_init__(void);

void jlau_kill(jlau_t* jlau);
void jlgr_kill(la_window_t* jlgr);

void la_window_init__(void* context, la_window_t* window, la_draw_fn_t fn_,
	const char* name);

#if JL_PLAT == JL_PLAT_PHONE
	#include <jni.h>
	#include "SDL_log.h"

	const char* LA_FILE_ROOT = NULL;
	const char* LA_FILE_LOG = NULL;
	char la_keyboard_press = 0;
	extern la_window_t* la_window;
#endif

float la_banner_size = 0.f;
SDL_atomic_t la_rmc; // running / mode count
SDL_atomic_t la_rmcexit;
static char la_errorv[256];

//return how many seconds passed since last call
static inline void jl_seconds_passed__(la_window_t* window) {
/*	uint8_t isOnTime;

	jl->time.psec = la_time_regulatefps(&jl->time.timer, &isOnTime);

	if(window) {
		if((window->sg.changed = ( window->sg.on_time != isOnTime)))
			window->sg.on_time = isOnTime;
	}*/
}

static void main_loop_(void* context, la_window_t* window) {
	// Check the amount of time passed since last frame.
	//jl_seconds_passed__(window);
}

static void jlgr_loop_(void* context, la_window_t* window) {
	// Update events.
	la_port_input(window);
	// Run Main Loop
	main_loop_(context, window);
}

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

	va_start( arglist, format );
	vsprintf( temp, format, arglist );
	va_end( arglist );

	la_print( "%s", temp );
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

typedef struct {
	void* context;
	void(*libloop)(void* context, la_window_t* window);
	la_window_t* window;
	jl_fnct loop;
	jl_fnct kill;
} la_main_thread_t;

static int32_t la_main_thread(la_main_thread_t* ctx) {
	void* context = ctx->context;
	la_window_t* window = ctx->window;

	SDL_AtomicSet(&la_rmcexit, 1);
	// Run the Loop
	while(SDL_AtomicGet(&la_rmc)) {
		// Library loop
		ctx->libloop(context, window);
		// Program loop
		ctx->loop(context);
	}
	// Kill the program
/*	if(jl->jlau) {
		la_print("Kill Audio....");
		jlau_kill(jl->jlau);
	}*/
	ctx->kill(context);
	la_print("Killed Program!");
	SDL_AtomicSet(&la_rmcexit, 0);
	return 0;
}

/**
 * Start Lib-Aldaron on a separate thread.
 * @param fnc_init: The function initialize the program.
 * @param fnc_kill: The function to free anything that needs to be freed.
 * @param name: The name of the program, used for storage / window name etc.
 * @param ctx_size: The size of the program context.
**/
int32_t la_start(void* fnc_init, jl_fnct fnc_loop, jl_fnct fnc_kill,
	uint8_t openwindow, const char* name, size_t ctx_size)
{
	la_thread_t la_main;

// Terminal application support
#ifndef LA_PHONE_ANDROID
	la_window_t* la_window = openwindow ?
		la_memory_allocate(sizeof(la_window_t)) : NULL;

	// Initialize Lib Aldaron!
	void* context = la_init__(name, ctx_size);

	// If Terminal Only...
	if(!openwindow) ((jl_fnct) fnc_init)(context);
#else
	void* context = la_init__(name, ctx_size);
#endif
	// Start a new thread.
	la_main_thread_t ctx = (la_main_thread_t) { context, openwindow ?
		jlgr_loop_ : main_loop_, la_window, fnc_loop, fnc_kill };
	la_thread_new(&la_main, (la_thread_fn_t)la_main_thread, "la_main", &ctx);
	// Open a window, if "openwindow" is set.
#ifndef LA_PHONE_ANDROID
	if(openwindow) la_window_init__(context, la_window, fnc_init, name);
	// Wait for the thread to finish.
	la_print("Kill Window....");
	if(openwindow) jlgr_kill(la_window);
	la_print("SDL_Quit()");
	SDL_Quit();
	la_print("Free Context....");
	free(context);
	la_print("| success |");
#else
	la_print("android pre init %d %d", la_window->width, la_window->height);
	la_window_init__(context, la_window, fnc_init, name);
#endif
	return 0;
}
