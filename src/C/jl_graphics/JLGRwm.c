/*
 * JLGRwm.c: upper level SDL2
 *	Window Manager - Manages creating / destroying / redrawing windows.
*/

#include "JLGRinternal.h"
#include "jl_opengl.h"

#define JL_WM_INIT SDL_INIT_AUDIO|SDL_INIT_VIDEO
#define JL_WM_FULLSCREEN SDL_WINDOW_FULLSCREEN_DESKTOP

//PROTOTYPES
static void _jl_wm_fscreen(jlgr_t* jlgr, uint8_t a);

//EXPORT FUNCTIONS
void jlgr_wm_setfullscreen(jlgr_t* jlgr, uint8_t is) {
	_jl_wm_fscreen(jlgr, is);
}

void jlgr_wm_togglefullscreen(jlgr_t* jlgr) {
	_jl_wm_fscreen(jlgr, !jlgr->wm.fullscreen);
}

uint16_t jlgr_wm_getw(jlgr_t* jlgr) {
	return jlgr->wm.w;
}

uint16_t jlgr_wm_geth(jlgr_t* jlgr) {
	return jlgr->wm.h;
}

/**
 * THREAD: Drawing.
 * Set the title of a window.
 * @param jlgr: The library context.
 * @param window_name: What to name the window.
**/
void jlgr_wm_setwindowname(jlgr_t* jlgr, str_t window_name) {
	int ii;

	SDL_SetWindowTitle(jlgr->wm.displayWindow->w, window_name);
	for(ii = 0; ii < 16; ii++) {
		jlgr->wm.windowTitle[0][ii] = window_name[ii];
		if(window_name[ii] == '\0') { break; }
	}
	jlgr->wm.windowTitle[0][15] = '\0';
}

//STATIC FUNCTIONS
static void jl_wm_killedit(jl_t* jl, char *str) {
	jl_print(jl, str);
	jl_print(jl, SDL_GetError());
	exit(-1);
}

static inline SDL_Window* jl_wm_mkwindow(jlgr_t* jlgr, u32_t width, u32_t height) {
	int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
	if(jlgr->wm.fullscreen) flags |= SDL_WINDOW_FULLSCREEN;
	SDL_Window* rtn = SDL_CreateWindow(
		"Initializing....",				// window title
		SDL_WINDOWPOS_UNDEFINED,		// initial x position
		SDL_WINDOWPOS_UNDEFINED,		// initial y position
		width, height, flags
    	);
	if(rtn == NULL) jl_wm_killedit(jlgr->jl, "SDL_CreateWindow");
#if JL_PLAT == JL_PLAT_COMPUTER
	SDL_ShowCursor(SDL_DISABLE);
#endif
	return rtn;
}

static inline SDL_GLContext* jl_wm_gl_context(jlgr_t* jlgr) {
	SDL_GLContext* rtn = SDL_GL_CreateContext(jlgr->wm.displayWindow->w);
	if(rtn == NULL) jl_wm_killedit(jlgr->jl, "SDL_GL_CreateContext");
	return rtn;
}

// Set fullscreen or not.
static void _jl_wm_fscreen(jlgr_t* jlgr, uint8_t a) {
	// Make sure the fullscreen value is either a 1 or a 0.
	jlgr->wm.fullscreen = !!a;
	// Actually set whether fullscreen or not.
	if(SDL_SetWindowFullscreen(jlgr->wm.displayWindow->w,
	 JL_WM_FULLSCREEN * jlgr->wm.fullscreen))
		jl_wm_killedit(jlgr->jl, "SDL_SetWindowFullscreen");
	JL_PRINT_DEBUG(jlgr->jl, "Switched fullscreen on/off");
	// Resize window
	jlgr_resz(jlgr, 0, 0);
}

//Update the SDL_displayMode structure
void jl_wm_updatewh_(jlgr_t* jlgr) {
	// Get Window Size
	SDL_GetWindowSize(jlgr->wm.displayWindow->w, &jlgr->wm.w, &jlgr->wm.h);
}

//This is the code that actually creates the window by accessing SDL
static inline void jlgr_wm_create__(jlgr_t* jlgr, u32_t w, u32_t h) {
	// Allocate space for "displayWindow"
	jlgr->wm.displayWindow = jl_memi(jlgr->jl, sizeof(jl_window_t));
	//
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	// Create window.
	jlgr->wm.displayWindow->w = jl_wm_mkwindow(jlgr, w, h);
//	SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 0);
	jlgr->wm.displayWindow->c = jl_wm_gl_context(jlgr);
	// Clear and update
	jl_gl_clear(jlgr, 2, 255, 5, 255);
	jl_wm_loop__(jlgr);
}

// ETOM FUNCTIONS

int oldtick = 0;
int newtick = 0;

void jl_wm_loop__(jlgr_t* jlgr) {
	//Update Screen
	SDL_GL_SwapWindow(jlgr->wm.displayWindow->w); //end current draw
	// Clear the screen of anything wierd.
	jl_gl_clear(jlgr, 2, 5, 255, 255);
//
	oldtick = newtick;
	newtick = SDL_GetTicks();
	// milliseconds / 1000 to get seconds
	jlgr->psec = jl_sdl_timer(jlgr->jl, &jlgr->timer);

//JL_PRINT("GR %f, N %f\n", 1./jlgr->psec, 1./jlgr->jl->time.psec);
}

void jl_wm_resz__(jlgr_t* jlgr, uint16_t w, uint16_t h) {
	jlgr->wm.w = w;
	jlgr->wm.h = h;
	jlgr->wm.ar = ((double)h) / ((double)w);
	jl_gl_viewport_screen(jlgr);
	JL_PRINT_DEBUG(jlgr->jl, "Resized");
}

void jl_wm_init__(jlgr_t* jlgr) {
	SDL_DisplayMode current;

	SDL_Init(JL_WM_INIT);
	// Get Information On How Big To Make Window
	if(SDL_GetCurrentDisplayMode(0, &current)) {
		jl_print(jlgr->jl, "failed to get current display mode:%s",
			(char *)SDL_GetError());
		jl_sg_kill(jlgr->jl);
	}
	JL_PRINT_DEBUG(jlgr->jl, "Got wh: %d,%d", current.w, current.h);
	// Create Window
	jlgr_wm_create__(jlgr, current.w, current.h);
	// Maximize if not in fullscreen mode.
	jl_ct_quickloop_(jlgr);
	// Get Window Size
	jl_wm_updatewh_(jlgr);
	JL_PRINT_DEBUG(jlgr->jl, "size = %dx%d", jlgr->wm.w, jlgr->wm.h);
	//Update screensize to fix any rendering glitches
	jl_wm_resz__(jlgr, jlgr->wm.w, jlgr->wm.h);
	// Update The Screen
	jl_wm_loop__(jlgr);
}

void jl_wm_kill__(jlgr_t* jlgr) {
	JL_PRINT_DEBUG(jlgr->jl, "Closing Window....");
	if (jlgr->wm.displayWindow->c != NULL) {
//		SDL_free(jlgr->wm.displayWindow->c);
		SDL_free(jlgr->wm.displayWindow->w);
	}
	JL_PRINT_DEBUG(jlgr->jl, "Closed Window!");
}
