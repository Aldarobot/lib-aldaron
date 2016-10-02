/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_config.h>
#ifdef LA_FEATURE_DISPLAY

#include "JLGRprivate.h"

#include <la_time.h>
#include <la_ro.h>

#define JL_WM_FULLSCREEN SDL_WINDOW_FULLSCREEN_DESKTOP

extern float la_banner_size;

void la_draw_resize(la_window_t *, uint32_t, uint32_t);

static inline void jl_wm_killedit(const char *str) {
	la_print(str);
	la_panic(SDL_GetError());
}

#if JL_PLAT == JL_PLAT_COMPUTER
static void jlgr_wm_fscreen__(la_window_t* jlgr, uint8_t a) {
	// Make sure the fullscreen value is either a 1 or a 0.
	jlgr->wm.fullscreen = !!a;
	// Actually set whether fullscreen or not.
	if(SDL_SetWindowFullscreen(jlgr->wm.window,
	 jlgr->wm.fullscreen ? JL_WM_FULLSCREEN : 0))
		jl_wm_killedit("SDL_SetWindowFullscreen");
	la_print("Switched fullscreen on/off");
	// Resize window
	la_draw_resize(jlgr, 0, 0);
}
#endif

//
// EXPORT FUNCTIONS
//

void jlgr_wm_setfullscreen(la_window_t* jlgr, uint8_t is) {
#if JL_PLAT == JL_PLAT_COMPUTER
	jlgr_wm_fscreen__(jlgr, is);
#endif
}

void jlgr_wm_togglefullscreen(la_window_t* jlgr) {
#if JL_PLAT == JL_PLAT_COMPUTER
	jlgr_wm_fscreen__(jlgr, !jlgr->wm.fullscreen);
#endif
}

uint16_t jlgr_wm_getw(la_window_t* jlgr) {
	return jlgr->wm.w;
}

uint16_t jlgr_wm_geth(la_window_t* jlgr) {
	return jlgr->wm.h;
}

float la_window_banner_size(la_window_t* jlgr) {
	return la_banner_size;
}

/**
 * THREAD: Drawing.
 * Set the title of a window.
 * @param jlgr: The library context.
 * @param window_name: What to name the window.
**/
void jlgr_wm_setwindowname(la_window_t* jlgr, const char* window_name) {
	int ii;

#ifndef LA_PHONE_ANDROID
	SDL_SetWindowTitle(jlgr->wm.window, window_name);
#endif
	for(ii = 0; ii < 16; ii++) {
		jlgr->wm.windowTitle[0][ii] = window_name[ii];
		if(window_name[ii] == '\0') { break; }
	}
	jlgr->wm.windowTitle[0][16] = '\0';
}

//STATIC FUNCTIONS

#ifndef LA_PHONE_ANDROID
static inline SDL_Window* jlgr_wm_mkwindow__(la_window_t* jlgr) {
	int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

	if(jlgr->wm.fullscreen)
		flags |= JL_WM_FULLSCREEN;
	else
		flags |= SDL_WINDOW_MAXIMIZED;
	SDL_Window* rtn = SDL_CreateWindow(
		"Initializing....",			// window title
		SDL_WINDOWPOS_UNDEFINED,		// initial x position
		SDL_WINDOWPOS_UNDEFINED,		// initial y position
		640, 360, flags
	);
	if(rtn == NULL) jl_wm_killedit("SDL_CreateWindow");
	SDL_ShowCursor(SDL_DISABLE);
	return rtn;
}

static inline SDL_GLContext* jl_wm_gl_context(la_window_t* jlgr) {
	SDL_GLContext* rtn = SDL_GL_CreateContext(jlgr->wm.window);
	if(rtn == NULL) jl_wm_killedit("SDL_GL_CreateContext");
	return rtn;
}
#endif

//Update the SDL_displayMode structure
void jl_wm_updatewh_(la_window_t* window) {
	// Get Window Size
#ifndef LA_PHONE_ANDROID
	SDL_GetWindowSize(window->wm.window, &window->wm.w, &window->wm.h);
#else
	window->wm.w = window->width, window->wm.h = window->height;
#endif
	// Get Aspect Ratio
	window->wm.ar = ((double)window->wm.h) / ((double)window->wm.w);
	la_print("size = %dx%d", window->wm.w, window->wm.h);
}

//This is the code that actually creates the window by accessing SDL
static inline void jlgr_wm_create__(la_window_t* jlgr) {
#if JL_PLAT == JL_PLAT_COMPUTER
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	// OpenGL Version
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	// Create window.
	jlgr->wm.window = jlgr_wm_mkwindow__(jlgr);
	SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
	jlgr->wm.glcontext = jl_wm_gl_context(jlgr);
#endif
}

// ETOM FUNCTIONS

void jl_wm_loop__(la_window_t* jlgr) {
#ifndef LA_PHONE_ANDROID
	//Update Screen
	SDL_GL_SwapWindow(jlgr->wm.window); //end current draw
#endif
	// milliseconds / 1000 to get seconds
	jlgr->psec = la_time_regulatefps(&jlgr->timer, &jlgr->on_time);
}

void la_window_resize__(la_window_t* window, uint32_t w, uint32_t h) {
	window->wm.w = w;
	window->wm.h = h;
	window->wm.ar = ((float)h) / ((float)w);
	jl_gl_viewport_screen(window);
	la_ro_rect(window, &window->screen, 1.f, window->wm.ar);
}

void jl_wm_init__(la_window_t* window) {
	// Create Window
	jlgr_wm_create__(window);
	// Get Resize Event
	la_port_input(window);
	// Get Window Size
	jl_wm_updatewh_(window);
	// Set default values
	la_safe_set_uint8(&window->has_2_screens, 0);
}

#endif
