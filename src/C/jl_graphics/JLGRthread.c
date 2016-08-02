/*
 * JL_Lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLGRthread.c
 *	This file handles a separate thread for drawing graphics.
**/
#include "JLGRprivate.h"

static void jlgr_thread_programsresize(jlgr_t* jlgr) {
	jlgr_pvar_t* pjlgr = jl_thread_pvar_edit(&jlgr->pvar);
	jl_fnct resize_ = pjlgr->functions.redraw.resize;
	jl_thread_pvar_drop(&jlgr->pvar, (void**)&pjlgr);
	resize_(jlgr->jl);
}

static void jlgr_thread_resize(jlgr_t* jlgr, uint16_t w, uint16_t h) {
	JL_PRINT_DEBUG(jlgr->jl, "Resizing to %dx%d....", w, h);
	// Set window size & aspect ratio stuff.
	jl_wm_resz__(jlgr, w, h);
	// Update the size of the background.
	jl_sg_resz__(jlgr->jl);
	// Taskbar resize.
	jlgr_menu_resize_(jlgr);
	// Mouse resize
	if(jlgr->mouse.mutex.jl) jlgr_sprite_resize(jlgr, &jlgr->mouse, NULL);
}

static void jlgr_thread_windowresize(jlgr_t* jlgr) {
	jlgr_pvar_t* pjlgr = jl_thread_pvar_edit(&jlgr->pvar);
	uint16_t w = pjlgr->set_width;
	uint16_t h = pjlgr->set_height;
	jl_thread_pvar_drop(&jlgr->pvar, (void**)&pjlgr);

	jl_wm_updatewh_(jlgr);
	if(w == 0) w = jlgr_wm_getw(jlgr);
	if(h == 0) h = jlgr_wm_geth(jlgr);
	jlgr_thread_resize(jlgr, w, h);
	jlgr_thread_programsresize(jlgr);
}

static void jlgr_thread_draw_init__(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;

	jl_print_function(jl, "thead-draw-init");
	// Initialize subsystems
	JL_PRINT_DEBUG(jl, "Creating the window....");
	jl_wm_init__(jlgr);
	
	jlgr_text_init__(jlgr);
	JL_PRINT_DEBUG(jl, "Loading default graphics from package....");
	jl_sg_init__(jlgr);
	JL_PRINT_DEBUG(jl, "Setting up OpenGL....");
	jl_gl_init__(jlgr);
	JL_PRINT_DEBUG(jl, "Setting up effects....");
	jlgr_effects_init__(jlgr);
	JL_PRINT_DEBUG(jl, "Load graphics....");
	jlgr_init__(jlgr);
	JL_PRINT_DEBUG(jl, "Creating Taskbar sprite....");
	jlgr_menubar_init__(jlgr);
	JL_PRINT_DEBUG(jl, "Creating Mouse sprite....");
	jlgr_mouse_init__(jlgr);
	JL_PRINT_DEBUG(jl, "User's Init....");
	SDL_AtomicSet(&jlgr->running, 1);
	jl_fnct program_init_;
	jlgr_pvar_t* pjlgr = jl_thread_pvar_edit(&jlgr->pvar);
	pjlgr->needs_resize = 1;
	program_init_ = pjlgr->functions.fn;
	jl_thread_pvar_drop(&jlgr->pvar, (void**)&pjlgr);
	program_init_(jl);
	jlgr_thread_resize(jlgr, jlgr_wm_getw(jlgr), jlgr_wm_geth(jlgr));
	jlgr_wm_setwindowname(jlgr, jl->name);
	JL_PRINT_DEBUG(jl, "Sending finish packet....");
	// Tell main thread to stop waiting.
	jl_thread_wait_stop(jl, &jlgr->wait);
	jl_print_return(jl, "thead-draw-init");
}

void jlgr_thead_check_resize(jlgr_t* jlgr) {
	uint8_t should_resize;
	jlgr_pvar_t* pjlgr;

	pjlgr = jl_thread_pvar_edit(&jlgr->pvar);
	should_resize = pjlgr->needs_resize;
	pjlgr->needs_resize = 0;
	jl_thread_pvar_drop(&jlgr->pvar, (void**)&pjlgr);

	if(should_resize == 1) jlgr_thread_programsresize(jlgr);
	if(should_resize == 2) jlgr_thread_windowresize(jlgr);
}

int jlgr_thread_draw(void* data) {
	jl_t* jl = data;
	jlgr_t* jlgr = jl->jlgr;

	jl_thread_wait(jl, &jl->wait);

	jl_print(jl, "THREAD#1=#%d", jl_thread_current(jl));

	// Initialize subsystems
	jlgr_thread_draw_init__(jl);
	// Redraw loop
	while(SDL_AtomicGet(&jlgr->running)) {
		// Check for resize
		jlgr_thead_check_resize(jlgr);
		// Deselect any pre-renderer.
		jlgr->gl.cp = NULL;
		//Redraw screen.
		_jl_sg_loop(jlgr);
		//Update Screen.
		jl_wm_loop__(jlgr);
	}
	jl_wm_kill__(jlgr); // Kill window
	jlgr_sprite_free(jlgr, &jlgr->sg.bg.up);
	jlgr_sprite_free(jlgr, &jlgr->sg.bg.dn);
	return 0;
}

void jlgr_thread_init(jlgr_t* jlgr, jl_fnct fn_) {
	jl_t* jl = jlgr->jl;

	jl_print_function(jl, "jl-thread-init");
	jl_thread_pvar_init(jl, &jlgr->pvar, NULL, sizeof(jlgr_pvar_t));
	// Set init function
	jlgr_pvar_t* pjlgr = jl_thread_pvar_edit(&jlgr->pvar);
	pjlgr->functions.fn = fn_;
	jl_thread_pvar_drop(&jlgr->pvar, (void**)&pjlgr);
	// Start thread
	jlgr->thread = jl_thread_new(jl, "JL_Lib/Graphics",
		jlgr_thread_draw);
	jl_print_return(jl, "jl-thread-init");
}

void jlgr_thread_kill(jlgr_t* jlgr) {
	jl_thread_old(jlgr->jl, jlgr->thread);
}
