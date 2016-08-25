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

static void jlgr_loop_(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;

	// Update events.
	jl_ct_loop__(jlgr);
	// Run Main Loop
	main_loop_(jl);
}

//
// Global Functions
//

/**
 * Create a window.
 * @param jlgr: The window.
 * @param fn_: Graphic initialization function run on graphical thread.
**/
void la_window_init(jlgr_t* jlgr, jl_fnct fn_) {
	jl_t* jl = la_jl_deprecated;

	jl->jlgr = jlgr;
	jl->loop = jlgr_loop_;
#if JL_PLAT == JL_PLAT_COMPUTER
	jlgr->wm.fullscreen = 0;
#endif
	jlgr->jl = jl;
	jlgr->fl.inloop = 1;
	// Initialize Subsystem
#ifndef LA_PHONE_ANDROID
	SDL_VideoInit(NULL);
#endif
	la_print("Initializing Input....");
	jl_ct_init__(jlgr); // Prepare to read input.
	la_print("Initialized CT! / Initializing file viewer....");
	jlgr_fl_init(jlgr);
	la_print("Initializing file viewer!");
	// Start Drawing thread.
	jlgr_thread_init(jlgr, fn_);
	//
	jl->mode.count = 0;
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
void jlgr_loop_set(jlgr_t* jlgr, jl_fnct onescreen, jl_fnct upscreen,
	jl_fnct downscreen, jl_fnct resize)
{
	jlgr_pvar_t* pjlgr = jl_thread_pvar_edit(&jlgr->pvar);
	pjlgr->functions.redraw.single = onescreen;
	pjlgr->functions.redraw.upper = upscreen;
	pjlgr->functions.redraw.lower = downscreen;
	pjlgr->functions.redraw.resize = resize;
	pjlgr->needs_resize = 1;
	jl_thread_pvar_drop(&jlgr->pvar, (void**)&pjlgr);
}

/**
 * Resize the window.
 * @param jlgr: The library context.
**/
void jlgr_resz(jlgr_t* jlgr, uint16_t w, uint16_t h) {
	jlgr_pvar_t* pjlgr = jl_thread_pvar_edit(&jlgr->pvar);
	pjlgr->needs_resize = 2;
	pjlgr->set_width = w;
	pjlgr->set_height = h;
	jl_thread_pvar_drop(&jlgr->pvar, (void**)&pjlgr);
}

/**
 * Destroy the window and free the jlgr library context.
 * @param jlgr: The jlgr library context.
**/
void jlgr_kill(jlgr_t* jlgr) {
	la_print("Sending Kill to threads....");
	SDL_AtomicSet(&jlgr->running, 0);
	la_print("Removing clump filelist for fileviewer....");
	jlgr_file_kill_(jlgr);
	la_print("Fileviewer is dead....");
#ifndef LA_PHONE_ANDROID
	SDL_VideoQuit();
#endif
	la_print("Killed SDL/VIDEO Subsystem!");
}

// End of file.
