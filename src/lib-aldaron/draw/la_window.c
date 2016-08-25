/*
 * JLGRwm.c: upper level SDL2
 *	Window Manager - Manages creating / destroying / redrawing windows.
*/

#include "JLGRprivate.h"

#define JL_WM_FULLSCREEN SDL_WINDOW_FULLSCREEN_DESKTOP

extern float la_banner_size;

static void jl_wm_killedit(jl_t* jl, char *str) {
	la_print(str);
	la_panic(SDL_GetError());
}

#if JL_PLAT == JL_PLAT_COMPUTER
static void jlgr_wm_fscreen__(jlgr_t* jlgr, uint8_t a) {
	// Make sure the fullscreen value is either a 1 or a 0.
	jlgr->wm.fullscreen = !!a;
	// Actually set whether fullscreen or not.
	if(SDL_SetWindowFullscreen(jlgr->wm.window,
	 jlgr->wm.fullscreen ? JL_WM_FULLSCREEN : 0))
		jl_wm_killedit(jlgr->jl, "SDL_SetWindowFullscreen");
	la_print("Switched fullscreen on/off");
	// Resize window
	jlgr_resz(jlgr, 0, 0);
}
#endif

//
// EXPORT FUNCTIONS
//

void jlgr_wm_setfullscreen(jlgr_t* jlgr, uint8_t is) {
#if JL_PLAT == JL_PLAT_COMPUTER
	jlgr_wm_fscreen__(jlgr, is);
#endif
}

void jlgr_wm_togglefullscreen(jlgr_t* jlgr) {
#if JL_PLAT == JL_PLAT_COMPUTER
	jlgr_wm_fscreen__(jlgr, !jlgr->wm.fullscreen);
#endif
}

uint16_t jlgr_wm_getw(jlgr_t* jlgr) {
	return jlgr->wm.w;
}

uint16_t jlgr_wm_geth(jlgr_t* jlgr) {
	return jlgr->wm.h;
}

float la_window_banner_size(jlgr_t* jlgr) {
	return la_banner_size;
}

/**
 * THREAD: Drawing.
 * Set the title of a window.
 * @param jlgr: The library context.
 * @param window_name: What to name the window.
**/
void jlgr_wm_setwindowname(jlgr_t* jlgr, const char* window_name) {
	int ii;

#ifndef LA_PHONE_ANDROID
	SDL_SetWindowTitle(jlgr->wm.window, window_name);
#endif
	for(ii = 0; ii < 16; ii++) {
		jlgr->wm.windowTitle[0][ii] = window_name[ii];
		if(window_name[ii] == '\0') { break; }
	}
	jlgr->wm.windowTitle[0][15] = '\0';
}

//STATIC FUNCTIONS

#ifndef LA_PHONE_ANDROID
static inline SDL_Window* jlgr_wm_mkwindow__(jlgr_t* jlgr) {
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
	if(rtn == NULL) jl_wm_killedit(jlgr->jl, "SDL_CreateWindow");
	SDL_ShowCursor(SDL_DISABLE);
	return rtn;
}

static inline SDL_GLContext* jl_wm_gl_context(jlgr_t* jlgr) {
	SDL_GLContext* rtn = SDL_GL_CreateContext(jlgr->wm.window);
	if(rtn == NULL) jl_wm_killedit(jlgr->jl, "SDL_GL_CreateContext");
	return rtn;
}
#endif

//Update the SDL_displayMode structure
void jl_wm_updatewh_(jlgr_t* jlgr) {
	// Get Window Size
#ifndef LA_PHONE_ANDROID
	SDL_GetWindowSize(jlgr->wm.window, &jlgr->wm.w, &jlgr->wm.h);
#else
	jlgr->wm.w = 640, jlgr->wm.h = 480; // TODO: actual dimensions
#endif
	// Get Aspect Ratio
	jlgr->wm.ar = ((double)jlgr->wm.h) / ((double)jlgr->wm.w);
	la_print("size = %dx%d", jlgr->wm.w, jlgr->wm.h);
}

//This is the code that actually creates the window by accessing SDL
static inline void jlgr_wm_create__(jlgr_t* jlgr) {
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

void jl_wm_loop__(jlgr_t* jlgr) {
#ifndef LA_PHONE_ANDROID
	//Update Screen
	SDL_GL_SwapWindow(jlgr->wm.window); //end current draw
#endif
	// milliseconds / 1000 to get seconds
	jlgr->psec=jl_time_regulatefps(jlgr->jl, &jlgr->timer, &jlgr->on_time);
}

void jl_wm_resz__(jlgr_t* jlgr, uint16_t w, uint16_t h) {
	jlgr->wm.w = w;
	jlgr->wm.h = h;
	jlgr->wm.ar = ((float)h) / ((float)w);
	jl_gl_viewport_screen(jlgr);
}

void jl_wm_init__(jlgr_t* jlgr) {
	// Create Window
	jlgr_wm_create__(jlgr);
	// Get Resize Event
	jl_ct_quickloop_(jlgr);
	// Get Window Size
	jl_wm_updatewh_(jlgr);
}
