#include <assert.h>

#include "JLprivate.h"
#include "jlau.h"
#include "jlgr.h"
#if JL_PLAT == JL_PLAT_PHONE
#include "SDL_system.h"
#else
#include "SDL.h"
#endif

#include "la_thread.h"
#include "la_memory.h"

void jlau_kill(jlau_t* jlau);
void jlgr_kill(la_window_t* jlgr);

void jl_mode_loop__(jl_t* jl);

void jlgr_fl_init(la_window_t* jlgr);

#if JL_PLAT == JL_PLAT_PHONE
	#include <jni.h>
	#include "SDL_log.h"

	const char* LA_FILE_ROOT = NULL;
	const char* LA_FILE_LOG = NULL;
	char la_keyboard_press = 0;
#endif

float la_banner_size = 0.f;
jl_t* la_jl_deprecated = NULL;

static inline void jl_init_libs__(jl_t* jl) {
	la_print("Initializing file system....");
	jl_file_init_(jl);
	la_print("Initializing modes....");
	jl_mode_init__(jl);
	la_print("Initializing time....");
	jl_sdl_init__(jl);
	la_print("Initializing SDL....");
	SDL_Init(0);
	la_print("Initialized!");
}

static inline void la_init__(jl_t* jl, jl_fnct _fnc_init_, const char* nm,
	uint64_t ctx1s)
{
	//
	jl->loop = main_loop_;
	la_print("Initializing subsystems....");
	// Run the library's init function.
	jl_init_libs__(jl);
	// Allocate the program's context.
	jl->prg_context = la_memory_allocate(ctx1s);
	jl->name = jl_mem_copy(jl, nm, strlen(nm) + 1);
	// Run the program's init function.
	_fnc_init_(jl);
	// Run the mode loop
	jl_mode_loop__(jl);
	//
	jl->time.timer = jl_time_get(jl);
	la_print("Started JL_Lib!");
}

static void jl_time_reset__(jl_t* jl, uint8_t on_time) {
	if(jl->jlgr) {
		la_window_t* jlgr = jl->jlgr;

		if((jlgr->sg.changed = ( jlgr->sg.on_time != on_time)))
			jlgr->sg.on_time = on_time;
	}
}

//return how many seconds passed since last call
static inline void jl_seconds_passed__(jl_t* jl) {
	uint8_t isOnTime;

	jl->time.psec = jl_time_regulatefps(jl, &jl->time.timer, &isOnTime);
	jl_time_reset__(jl, isOnTime);
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

// EXPORT FUNCTIONS

/**
 * Exit The program on an error.
**/
void la_panic(const char* format, ...) {
	va_list arglist;

	va_start( arglist, format );
	la_print( format, arglist );
	va_end( arglist );

	assert(0);
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

typedef struct {
	jl_t* jl;
	la_window_t* jlgr;
} la_main_thread_t;

static int32_t la_main_thread(la_main_thread_t* ctx) {
	jl_t* jl = ctx->jl;
	la_window_t* jlgr = ctx->jlgr;
	if(jlgr) {
		la_print("Initialized CT! / Initializing file viewer....");
		jlgr_fl_init(jlgr);
		la_print("Initializing file viewer!");
	}
	// Run the Loop
	while(jl->mode.count) ((jl_fnct)jl->loop)(jl);
	// Kill the program
	if(jlgr) {
		la_print("Sending Kill to draw thread....");
		SDL_AtomicSet(&jlgr->running, 0);
	}
	if(jl->jlau) {
		la_print("Kill Audio....");
		jlau_kill(jl->jlau);
	}
	la_print("Kill Program....");
	((jl_fnct)jl->kill)(jl);
	la_print("Success!");
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
	jl_t* jl = jl_mem_init_(); // Create The Library Context
	la_thread_t la_main;
	la_window_t* window = openwindow ? la_memory_allocate(sizeof(la_window_t)) : NULL;

	// Initialize JL_lib!
	la_init__(jl, openwindow ? la_dont : fnc_init, name, ctx_size);
	jl->kill = fnc_kill;
	la_jl_deprecated = jl;
	// Start a new thread.
	la_main_thread_t ctx = (la_main_thread_t) { jl, window };
	la_thread_new(&la_main, (la_thread_fn_t)la_main_thread, "la_main", &ctx);
	// Open a window, if "openwindow" is set.
	if(openwindow) la_window_init(window, fnc_init);
	// Wait for the thread to finish.
//	int32_t rtn = la_thread_old(&la_main);
	la_print("Kill Window....");
	if(openwindow) jlgr_kill(window);
	la_print("SDL_Quit()");
	SDL_Quit();
	la_print("Free library context....");
	jl_mem_kill_(jl);
	la_print("| success |");
	return 0;
}
