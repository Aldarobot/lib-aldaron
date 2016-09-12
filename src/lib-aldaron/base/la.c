#include <assert.h>

#include "jlau.h"
#include "jlgr.h"
#if JL_PLAT == JL_PLAT_PHONE
#include "SDL_system.h"
#else
#include "SDL.h"
#endif

#include "la_thread.h"
#include "la_memory.h"
#include "la_file.h"

void jl_mode_init__(jl_t* jl);
void jl_sdl_init__(jl_t* jl);

void jlau_kill(jlau_t* jlau);
void jlgr_kill(la_window_t* jlgr);

void jl_mode_loop__(jl_t* jl);

void jlgr_fl_init(la_window_t* jlgr);
void la_window_init__(la_window_t* window, jl_fnct fn_, const char* name);

#if JL_PLAT == JL_PLAT_PHONE
	#include <jni.h>
	#include "SDL_log.h"

	const char* LA_FILE_ROOT = NULL;
	const char* LA_FILE_LOG = NULL;
	char la_keyboard_press = 0;
	extern la_window_t* la_window;
#endif

float la_banner_size = 0.f;
jl_t* la_jl_deprecated = NULL;
SDL_atomic_t la_rmc; // running / mode count
SDL_atomic_t la_rmcexit;
static char la_errorv[256];

static inline void jl_init_libs__(jl_t* jl) {
	// Clear error file.
	la_file_truncate(NULL);
	jl_mode_init__(jl);
	jl_sdl_init__(jl);
	SDL_Init(0);
}

//return how many seconds passed since last call
static inline void jl_seconds_passed__(jl_t* jl) {
	uint8_t isOnTime;

	jl->time.psec = jl_time_regulatefps(jl, &jl->time.timer, &isOnTime);

	if(jl->jlgr) {
		la_window_t* jlgr = jl->jlgr;

		if((jlgr->sg.changed = ( jlgr->sg.on_time != isOnTime)))
			jlgr->sg.on_time = isOnTime;
	}
}

void main_loop_(jl_t* jl) {
	jl_fnct loop_ = jl->mode.mode.loop;

	// Check the amount of time passed since last frame.
	jl_seconds_passed__(jl);
	// Run the user's mode loop.
	loop_(jl);
	// Run the mode loop
	jl_mode_loop__(jl);
}

static inline void la_init__(jl_t* jl, jl_fnct _fnc_init_, const char* nm,
	uint64_t ctx1s)
{
	//
	jl->loop = main_loop_;
	// Run the library's init function.
	jl_init_libs__(jl);
	// Allocate the program's context.
	jl->prg_context = la_memory_allocate(ctx1s);
	// Run the program's init function.
	_fnc_init_(jl);
	// Run the mode loop
	jl_mode_loop__(jl);
	//
	jl->time.timer = jl_time_get(jl);
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
void la_dont(jl_t* jl) { }

/**
 * Get the program's context.
 * @param jl: The library's context.
**/
void* la_context(jl_t* jl) {
	return jl->prg_context;
}

const char* la_error(const char* format, ...) {
	va_list arglist;

	va_start( arglist, format );
	vsnprintf(la_errorv, 256, format, arglist);
	va_end( arglist );

	return la_errorv;
}

typedef struct {
	jl_t* jl;
	la_window_t* jlgr;
	jl_fnct kill;
} la_main_thread_t;

static int32_t la_main_thread(la_main_thread_t* ctx) {
	jl_t* jl = ctx->jl;
	la_window_t* jlgr = ctx->jlgr;

	SDL_AtomicSet(&la_rmcexit, 1);
	if(jlgr) {
		la_print("Initializing file viewer....");
		jlgr_fl_init(jlgr);
		la_print("Initialized file viewer!");
	}
	// Run the Loop
	while(SDL_AtomicGet(&la_rmc)) ((jl_fnct)jl->loop)(jl);
	// Kill the program
	if(jlgr) {
		la_print("Sending Kill to draw thread....");
		SDL_AtomicSet(&la_rmc, 0);
	}
	if(jl->jlau) {
		la_print("Kill Audio....");
		jlau_kill(jl->jlau);
	}
	ctx->kill(jl);
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
int32_t la_start(jl_fnct fnc_init, jl_fnct fnc_kill, uint8_t openwindow,
	const char* name, size_t ctx_size)
{
	jl_t* jl = la_memory_allocate(sizeof(jl_t)); // Create The Library Context
	la_thread_t la_main;

#ifndef LA_PHONE_ANDROID
	la_window_t* la_window = openwindow ?
		la_memory_allocate(sizeof(la_window_t)) : NULL;

	// Initialize JL_lib!
	la_init__(jl, openwindow ? la_dont : fnc_init, name, ctx_size);
#else
	la_init__(jl, la_dont, name, ctx_size);
#endif
	la_jl_deprecated = jl;
	// Start a new thread.
	la_main_thread_t ctx = (la_main_thread_t) { jl, la_window, fnc_kill };
	la_thread_new(&la_main, (la_thread_fn_t)la_main_thread, "la_main", &ctx);
	// Open a window, if "openwindow" is set.
#ifndef LA_PHONE_ANDROID
	if(openwindow) la_window_init__(la_window, fnc_init, name);
	// Wait for the thread to finish.
	la_print("Kill Window....");
	if(openwindow) jlgr_kill(la_window);
	la_print("SDL_Quit()");
	SDL_Quit();
	la_print("Free library context....");
	free(jl);
	la_print("| success |");
#else
	la_print("android pre init %d %d", la_window->width, la_window->height);
	la_window_init__(la_window, fnc_init, name);
#endif
	return 0;
}
