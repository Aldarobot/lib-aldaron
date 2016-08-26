/*
 * JL_Lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLGRthread.c
 *	This file handles a separate thread for drawing graphics.
**/
#include "JLGRprivate.h"

void jlgr_mouse_resize__(la_window_t* jlgr);
void jl_mode_loop__(jl_t* jl);
void jl_wm_resz__(la_window_t* jlgr, uint16_t w, uint16_t h);

static void jlgr_thread_programsresize(la_window_t* jlgr) {
	jlgr_pvar_t* pjlgr = jl_thread_pvar_edit(&jlgr->pvar);
	jl_fnct resize_ = pjlgr->functions.redraw.resize;
	jl_thread_pvar_drop(&jlgr->pvar, (void**)&pjlgr);
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
	jlgr_pvar_t* pjlgr = jl_thread_pvar_edit(&jlgr->pvar);
	uint16_t w = pjlgr->set_width;
	uint16_t h = pjlgr->set_height;
	jl_thread_pvar_drop(&jlgr->pvar, (void**)&pjlgr);

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
	SDL_AtomicSet(&jlgr->running, 1);
	jl_fnct program_init_;
	jlgr_pvar_t* pjlgr = jl_thread_pvar_edit(&jlgr->pvar);
	pjlgr->needs_resize = 1;
	program_init_ = pjlgr->functions.fn;
	jl_thread_pvar_drop(&jlgr->pvar, (void**)&pjlgr);
	program_init_(jl);
	jlgr_thread_resize(jlgr, jlgr_wm_getw(jlgr), jlgr_wm_geth(jlgr));
	jlgr_wm_setwindowname(jlgr, jl->name);
	la_print("Window Created!");
}

void jlgr_thead_check_resize(la_window_t* jlgr) {
	uint8_t should_resize;
	jlgr_pvar_t* pjlgr;

	pjlgr = jl_thread_pvar_edit(&jlgr->pvar);
	should_resize = pjlgr->needs_resize;
	pjlgr->needs_resize = 0;
	jl_thread_pvar_drop(&jlgr->pvar, (void**)&pjlgr);

	if(should_resize == 1) jlgr_thread_programsresize(jlgr);
	if(should_resize == 2) jlgr_thread_windowresize(jlgr);
}

int jlgr_thread_draw(jl_t* jl) {
	la_window_t* jlgr = jl->jlgr;

	jl_thread_wait(jl, &jl->wait);

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
	jlgr_sprite_free(jlgr, &jlgr->sg.bg.up);
	jlgr_sprite_free(jlgr, &jlgr->sg.bg.dn);
	return 0;
}

void jlgr_thread_init(la_window_t* jlgr, jl_fnct fn_) {
	jl_t* jl = jlgr->jl;

	jl_thread_pvar_init(jl, &jlgr->pvar, NULL, sizeof(jlgr_pvar_t));
	// Set init function
	jlgr_pvar_t* pjlgr = jl_thread_pvar_edit(&jlgr->pvar);
	pjlgr->functions.fn = fn_;
	jl_thread_pvar_drop(&jlgr->pvar, (void**)&pjlgr);
	// Start thread
	jlgr_thread_draw(jl);
}
