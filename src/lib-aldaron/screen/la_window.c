/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_config.h>
#ifdef LA_FEATURE_DISPLAY

#include <la_draw.h>
#include <la_time.h>
#include <la_ro.h>
#include <la_memory.h>
#include <la_llgraphics.h>

extern float la_banner_size;

void la_draw_resize(la_window_t *, uint32_t, uint32_t);

static inline void la_window_killedit__(const char *str) {
	la_print(str);
	la_panic(SDL_GetError());
}

#if defined(LA_COMPUTER)
static void la_window_fscreen__(la_window_t* window, uint8_t a) {
	// Make sure the fullscreen value is either a 1 or a 0.
	window->wm.fullscreen = !!a;
	// Actually set whether fullscreen or not.
	if(SDL_SetWindowFullscreen(window->wm.window,
	 window->wm.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0))
		la_window_killedit__("SDL_SetWindowFullscreen");
	la_print("Switched fullscreen on/off");
	// Resize window
	la_draw_resize(window, 0, 0);
}
#endif

void la_window_fullscreen(la_window_t* window, uint8_t is) {
#if defined(LA_COMPUTER)
	la_window_fscreen__(window, is);
#endif
}

void la_window_fullscreen_toggle(la_window_t* window) {
#if defined(LA_COMPUTER)
	la_window_fscreen__(window, !window->wm.fullscreen);
#endif
}

uint16_t la_window_width(la_window_t* window) {
	return window->wm.w;
}

uint16_t la_window_height(la_window_t* window) {
	return window->wm.h;
}

float la_window_banner_size(la_window_t* window) {
	return la_banner_size;
}

void la_window_name(la_window_t* window, const char* window_name) {
#ifndef LA_ANDROID
	SDL_SetWindowTitle(window->wm.window, window_name);
#endif
	uint32_t len = strlen(window_name);

	if(len > 16) {
		la_memory_copy(window_name, window->wm.windowTitle[0], 16);
		window->wm.windowTitle[0][16] = 0;
	}else{
		la_memory_copy(window_name, window->wm.windowTitle[0], len + 1);
	}
}

float la_window_h(la_window_t* window) {
	return (1. - la_banner_size) * la_ro_ar(window);
}

void la_window_clear(float r, float g, float b, float a) {
	la_llgraphics_clear(r, g, b, a);
}

#ifndef LA_ANDROID
static inline SDL_Window* la_window_make__(void) {
	int flags = SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE|SDL_WINDOW_MAXIMIZED;

	SDL_Window* rtn = SDL_CreateWindow(NULL, SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, 640, 360, flags);
#ifdef LA_DEBUG
	if(rtn == NULL) la_window_killedit__("SDL_CreateWindow");
#endif
	SDL_ShowCursor(SDL_DISABLE);
	return rtn;
}
#endif

//Update the SDL_displayMode structure
void la_window_update_size(la_window_t* window) {
	// Get Window Size
#ifndef LA_ANDROID
	SDL_GetWindowSize(window->wm.window, &window->wm.w, &window->wm.h);
#else
	window->wm.w = window->width, window->wm.h = window->height;
#endif
	// Get Aspect Ratio
	window->wm.ar = ((double)window->wm.h) / ((double)window->wm.w);
}

//This is the code that actually creates the window by accessing SDL
static inline void la_window_create__(la_window_t* window) {
#if defined(LA_COMPUTER)
	window->wm.window = la_window_make__();
	window->wm.glcontext = SDL_GL_CreateContext(window->wm.window);
#ifdef LA_DEBUG
	if(window->wm.glcontext == NULL)
		la_window_killedit__("SDL_GL_CreateContext");
#endif
#endif
}

void la_window_update__(la_window_t* window) {
#ifndef LA_ANDROID
	//Update Screen
	SDL_GL_SwapWindow(window->wm.window); //end current draw
#endif
	// milliseconds / 1000 to get seconds
	window->psec = la_time_regulatefps(&window->timer, &window->on_time);
}

void la_window_resize__(la_window_t* window, uint32_t w, uint32_t h) {
	window->wm.w = w;
	window->wm.h = h;
	window->wm.ar = ((float)h) / ((float)w);
	la_llgraphics_viewport(w, h);
	la_ro_rect(window, &window->screen, 1.f, window->wm.ar);
	la_ro_change_orient(&window->screen, 1);
}

void la_window_init__(la_window_t* window) {
	// Create Window
	la_window_create__(window);
	// Get Resize Event
	la_port_input(window);
	// Set default values
	la_safe_set_uint8(&window->has_2_screens, 0);
}

#endif
