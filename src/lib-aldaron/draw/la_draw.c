/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include "JLGRprivate.h"
#include "la_memory.h"

extern SDL_atomic_t la_rmc;
extern SDL_atomic_t la_rmcexit;

void jlgr_mouse_resize__(la_window_t* jlgr);
void jl_wm_resz__(la_window_t* jlgr, uint16_t w, uint16_t h);

void la_window_draw__(void* context, la_window_t* window);

static void jlgr_thread_programsresize(void* context, la_window_t* window) {
	jl_fnct resize=la_safe_get_pointer(&window->protected.functions.resize);
	resize(context);
}

static void
jlgr_thread_resize(void* context, la_window_t* window, uint32_t w, uint32_t h) {
	la_print("Resizing to %dx%d....", w, h);
	jl_wm_resz__(window, w, h);
	la_print("User's resize....");
	jlgr_thread_programsresize(context, window);
	la_print("Resizing the mouse....");
	jlgr_mouse_resize__(window);
	la_print("Resized.");
}

static inline void la_draw_windowresize__(void* context, la_window_t* window) {
	uint32_t w = la_safe_get_uint32(&window->protected.set_width);
	uint32_t h = la_safe_get_uint32(&window->protected.set_height);

	jl_wm_updatewh_(window);
	if(w == 0) w = jlgr_wm_getw(window);
	if(h == 0) h = jlgr_wm_geth(window);
	jlgr_thread_resize(context, window, w, h);
}

static inline void
la_draw_init__(void* context, la_window_t* window, const char* name) {
	// Initialize subsystems
	la_print("Creating the window....");
	jl_wm_init__(window);
	la_print("Loading default graphics from package....");
	jl_sg_init__(window);
	la_print("Setting up OpenGL....");
	jl_gl_init__(window);
	la_print("Setting up effects....");
	jlgr_effects_init__(window);
	la_print("Load graphics....");
	jlgr_init__(window);
	la_print("User's Init....");
	SDL_AtomicSet(&la_rmc, 1);
	la_draw_fn_t program_init_;
	la_safe_set_uint8(&window->protected.needs_resize, 1);
	program_init_ = la_safe_get_pointer(&window->protected.functions.fn);
	program_init_(context, window);
	jlgr_thread_resize(context, window, jlgr_wm_getw(window),
		jlgr_wm_geth(window));
	jlgr_wm_setwindowname(window, name);
	la_print("Window Created!");
}

void la_draw_dont(void* context, la_window_t* window) { return; }

void jlgr_thead_check_resize(void* context, la_window_t* window) {
	uint8_t should_resize =
		la_safe_get_uint8(&window->protected.needs_resize);
	la_safe_set_uint8(&window->protected.needs_resize, 0);

	if(should_resize == 1) jlgr_thread_programsresize(context, window);
	if(should_resize == 2) la_draw_windowresize__(context, window);
}

void la_window_loop__(void* context, la_window_t* window) {
	// Check for resize
	jlgr_thead_check_resize(context, window);
	// Deselect any pre-renderer.
	window->gl.cp = NULL;
	//Redraw screen.
	la_window_draw__(context, window);
	//Update Screen.
	jl_wm_loop__(window);
}

void la_window_kill__(la_window_t* window) {
	SDL_AtomicSet(&la_rmc, 0);
}

//
// Global Functions
//

/**
 * Create a window.
 * @param window: The window.
 * @param fn_: Graphic initialization function run on graphical thread.
**/
void la_window_init__(void* context, la_window_t* window, la_draw_fn_t fn_,
	const char* name)
{
#if JL_PLAT == JL_PLAT_COMPUTER
	window->wm.fullscreen = 0;
	// Initialize video subsytem.
	SDL_VideoInit(NULL);
#endif
	// Set init function
	la_safe_set_pointer(&window->protected.functions.fn, fn_);
	// Initialize subsystems
	la_draw_init__(context, window, name);
#ifndef LA_PHONE_ANDROID
	// Redraw loop
	while(SDL_AtomicGet(&la_rmc))
		la_window_loop__(context, window);
	// Free stuff.
	la_window_kill__(window);
#endif
}

/**
 * Set the functions to be called when the window redraws.
 * @param jlgr: The jlgr library context.
 * @param onescreen: The function to redraw the screen when there's only 1 
 *  screen.
 * @param upscreen: The function to redraw the upper or primary display.
 * @param downscreen: The function to redraw the lower or secondary display.
 * @param resize: The function called when window is resized.
**/
void la_draw_fnchange(la_window_t* window, jl_fnct primary, jl_fnct secondary,
	jl_fnct resize)
{
	la_safe_set_pointer(&window->protected.functions.primary, primary);
	la_safe_set_pointer(&window->protected.functions.secondary, secondary);
	la_safe_set_pointer(&window->protected.functions.resize, resize);
	la_safe_set_uint8(&window->protected.needs_resize, 1);
}

/**
 * Resize the window.
 * @param jlgr: The library context.
**/
void la_draw_resize(la_window_t* window, uint32_t w, uint32_t h) {
	la_safe_set_uint32(&window->protected.set_width, w);
	la_safe_set_uint32(&window->protected.set_height, h);
	la_safe_set_uint8(&window->protected.needs_resize, 2);
}

/**
 * Destroy the window and free the jlgr library context.
 * @param jlgr: The jlgr library context.
**/
void jlgr_kill(la_window_t* jlgr) {
	while(SDL_AtomicGet(&la_rmcexit));
#ifndef LA_PHONE_ANDROID
	la_print("Destroying window....");
	SDL_DestroyWindow(jlgr->wm.window);
	la_print("SDL_VideoQuit()....");
	SDL_VideoQuit();
#endif
	la_print("Killed SDL/VIDEO Subsystem!");
}

// End of file.
