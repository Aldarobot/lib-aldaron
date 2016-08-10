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
 * @param jl: The library context.
 * @param fullscreen: 0 for windowed mode, 1 for fullscreen.
 * @param fn_: Graphic initialization function run on graphical thread.
 * @returns The jlgr library context.
**/
jlgr_t* jlgr_init(jl_t* jl, uint8_t fullscreen, jl_fnct fn_) {
	jlgr_t* jlgr = jl_memi(jl, sizeof(jlgr_t));

	jl_print_function(jl, "jlgr-init");
	jl->jlgr = jlgr;
	jl->loop = jlgr_loop_;
#if JL_PLAT == JL_PLAT_COMPUTER
	jlgr->wm.fullscreen = fullscreen;
#endif
	jlgr->jl = jl;
	jlgr->fl.inloop = 1;
	// Initialize Subsystem
	JL_PRINT_DEBUG(jl, "Initializing Input....");
	jl_ct_init__(jlgr); // Prepare to read input.
	JL_PRINT_DEBUG(jl, "Initialized CT! / Initializing file viewer....");
	jlgr_fl_init(jlgr);
	JL_PRINT_DEBUG(jl, "Initializing file viewer!");
	jl_print_return(jl, "jlgr-init");
	jl_print_function(jl, "jlgr-init2");
	// Create communicators for multi-threading
	jl_thread_wait_init(jl, &jlgr->wait);
	// Start Drawing thread.
	jlgr_thread_init(jlgr, fn_);
	// Wait for drawing thread to initialize, if not initialized already.
	jl_thread_wait(jlgr->jl, &jlgr->wait);
	jl_print_return(jl, "jlgr-init2");
	return jlgr;
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
	JL_PRINT_DEBUG(jlgr->jl, "Sending Kill to threads....");
	SDL_AtomicSet(&jlgr->running, 0);
	JL_PRINT_DEBUG(jlgr->jl, "Waiting on threads....");
	jlgr_thread_kill(jlgr); // Shut down thread.
	JL_PRINT_DEBUG(jlgr->jl, "Threads are dead....");
	jlgr_file_kill_(jlgr); // Remove clump filelist for fileviewer.
	JL_PRINT_DEBUG(jlgr->jl, "Fileviewer is dead....");
}

// End of file.
