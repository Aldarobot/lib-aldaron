/*
 * JL_Lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLGR.c
 *	A High Level Graphics Library that supports sprites, texture loading,
 *	2D rendering & 3D rendering.
 */
#include "JLGRprivate.h"
#include "la_memory.h"

extern jl_t* la_jl_deprecated;
extern SDL_atomic_t la_rmc;
extern SDL_atomic_t la_rmcexit;

static void jlgr_loop_(jl_t* jl) {
	// Update events.
	la_port_input(jl->jlgr);
	// Run Main Loop
	main_loop_(jl);
}

void jlgr_mouse_resize__(la_window_t* jlgr);
void jl_mode_loop__(jl_t* jl);
void jl_wm_resz__(la_window_t* jlgr, uint16_t w, uint16_t h);

jl_mutex_t* la_mutex;

static void jlgr_thread_programsresize(la_window_t* jlgr) {
	jl_thread_mutex_lock(&jlgr->protected.mutex);
	jl_fnct resize_ = jlgr->protected.functions.redraw.resize;
	jl_thread_mutex_unlock(&jlgr->protected.mutex);
	resize_(jlgr->jl);
}

static void jlgr_thread_resize(la_window_t* jlgr, uint16_t w, uint16_t h) {
	la_print("Resizing to %dx%d....", w, h);
	jl_wm_resz__(jlgr, w, h);
	la_print("Load the stuff....");
	jl_mode_loop__(jlgr->jl);
	la_print("User's resize....");
	jlgr_thread_programsresize(jlgr);
	la_print("Updating the size of the background....");
	jl_sg_resz__(jlgr->jl);
	la_print("Resizing the menubar....");
	jlgr_menu_resize_(jlgr);
	la_print("Resizing the mouse....");
	jlgr_mouse_resize__(jlgr);
	la_print("Resized.");
}

static void jlgr_thread_windowresize(la_window_t* jlgr) {
	jl_thread_mutex_lock(&jlgr->protected.mutex);
	uint16_t w = jlgr->protected.set_width;
	uint16_t h = jlgr->protected.set_height;
	jl_thread_mutex_unlock(&jlgr->protected.mutex);

	jl_wm_updatewh_(jlgr);
	if(w == 0) w = jlgr_wm_getw(jlgr);
	if(h == 0) h = jlgr_wm_geth(jlgr);
	jlgr_thread_resize(jlgr, w, h);
}

static void jlgr_thread_draw_init__(jl_t* jl) {
	la_window_t* jlgr = jl->jlgr;

	// Initialize subsystems
	la_print("Creating the window....");
	jl_wm_init__(jlgr);
	la_print("Creating font....");
	jlgr_text_init__(jlgr);
	la_print("Loading default graphics from package....");
	jl_sg_init__(jlgr);
	la_print("Setting up OpenGL....");
	jl_gl_init__(jlgr);
	la_print("Setting up effects....");
	jlgr_effects_init__(jlgr);
	la_print("Load graphics....");
	jlgr_init__(jlgr);
	la_print("Creating Taskbar sprite....");
	jlgr_menubar_init__(jlgr);
	la_print("User's Init....");
	SDL_AtomicSet(&la_rmc, 1);
	jl_fnct program_init_;
	jl_thread_mutex_lock(&jlgr->protected.mutex);
	jlgr->protected.needs_resize = 1;
	program_init_ = jlgr->protected.functions.fn;
	jl_thread_mutex_unlock(&jlgr->protected.mutex);
	program_init_(jl);
	jlgr_thread_resize(jlgr, jlgr_wm_getw(jlgr), jlgr_wm_geth(jlgr));
	jlgr_wm_setwindowname(jlgr, jl->name);
	la_print("Window Created!");
}

void jlgr_thead_check_resize(la_window_t* jlgr) {
	uint8_t should_resize;

	jl_thread_mutex_lock(&jlgr->protected.mutex);
	should_resize = jlgr->protected.needs_resize;
	jlgr->protected.needs_resize = 0;
	jl_thread_mutex_unlock(&jlgr->protected.mutex);

	if(should_resize == 1) jlgr_thread_programsresize(jlgr);
	if(should_resize == 2) jlgr_thread_windowresize(jlgr);
}

void la_window_loop__(la_window_t* window) {
	// Check for resize
	jlgr_thead_check_resize(window);
	// Deselect any pre-renderer.
	window->gl.cp = NULL;
	//Redraw screen.
	_jl_sg_loop(window);
	//Update Screen.
	jl_wm_loop__(window);
}

void la_window_kill__(la_window_t* window) {
	jlgr_sprite_free(window, &window->sg.bg.up);
	jlgr_sprite_free(window, &window->sg.bg.dn);
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
void la_window_init(la_window_t* window, jl_fnct fn_) {
	jl_t* jl = la_jl_deprecated;

	jl->jlgr = window;
	jl->loop = jlgr_loop_;
#if JL_PLAT == JL_PLAT_COMPUTER
	window->wm.fullscreen = 0;
#endif
	window->jl = jl;
	window->fl.inloop = 1;
	// Initialize Subsystem
#ifndef LA_PHONE_ANDROID
	SDL_VideoInit(NULL);
#endif

	jl_thread_mutex_new(jl, &window->protected.mutex);
	la_mutex = &window->protected.mutex;
	// Set init function
	jl_thread_mutex_lock(&window->protected.mutex);
	window->protected.functions.fn = fn_;
	jl_thread_mutex_unlock(&window->protected.mutex);
	// Wait ....
	jl_thread_wait(jl, &jl->wait);
	// Initialize subsystems
	jlgr_thread_draw_init__(jl);
#ifndef LA_PHONE_ANDROID
	// Redraw loop
	while(SDL_AtomicGet(&la_rmc))
		la_window_loop__(window);
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
void jlgr_loop_set(la_window_t* jlgr, jl_fnct onescreen, jl_fnct upscreen,
	jl_fnct downscreen, jl_fnct resize)
{
	jl_thread_mutex_lock(&jlgr->protected.mutex);
	jlgr->protected.functions.redraw.single = onescreen;
	jlgr->protected.functions.redraw.upper = upscreen;
	jlgr->protected.functions.redraw.lower = downscreen;
	jlgr->protected.functions.redraw.resize = resize;
	jlgr->protected.needs_resize = 1;
	jl_thread_mutex_unlock(&jlgr->protected.mutex);
}

/**
 * Resize the window.
 * @param jlgr: The library context.
**/
void jlgr_resz(la_window_t* jlgr, uint16_t w, uint16_t h) {
	jl_thread_mutex_lock(&jlgr->protected.mutex);
	jlgr->protected.needs_resize = 2;
	jlgr->protected.set_width = w;
	jlgr->protected.set_height = h;
	jl_thread_mutex_unlock(&jlgr->protected.mutex);
}

/**
 * Destroy the window and free the jlgr library context.
 * @param jlgr: The jlgr library context.
**/
void jlgr_kill(la_window_t* jlgr) {
	while(SDL_AtomicGet(&la_rmcexit));
	la_print("Removing clump filelist for fileviewer....");
	jlgr_file_kill_(jlgr);
#ifndef LA_PHONE_ANDROID
	la_print("Destroying window....");
	SDL_DestroyWindow(jlgr->wm.window);
	la_print("SDL_VideoQuit()....");
	SDL_VideoQuit();
#endif
	la_print("Killed SDL/VIDEO Subsystem!");
}

// End of file.
