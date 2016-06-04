/*
 * JL_Lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLGRmouse.c
 *	This file handles the mouse.
**/
#include "JLGRinternal.h"

//
// Static Functions
//

//
// Used elsewhere in library.
//

// Run when mouse needs to be redrawn.
void jlgr_mouse_draw_(jl_t* jl, uint8_t resize, void* ctx) {
	// Draw mouse, if using a computer.
#if JL_PLAT == JL_PLAT_COMPUTER //if computer
	jl_vo_t* mouse_vo = ctx;
	jl_rect_t rc = { 0.f, 0.f, 1.f, 1.f };

	jlgr_vos_image(jl->jlgr, mouse_vo, rc, 0, JL_IMGI_FONT, 255, 255);
	jlgr_draw_vo(jl->jlgr, mouse_vo, NULL);
#endif
}

// Run every frame for mouse
void jlgr_mouse_loop_(jl_t* jl, jl_sprite_t* sprite) {
	jlgr_t* jlgr = jl->jlgr;
	jl_sprite_t* mouse = jlgr->mouse;

//Update Mouse
	mouse->pr.cb.pos.x = jl_ct_gmousex(jlgr);
	mouse->pr.cb.pos.y = jl_ct_gmousey(jlgr);
}

void jlgr_mouse_init__(jlgr_t* jlgr) {
	jl_rect_t rc = { 0.f, 0.f, .075f, .075f };
	jl_vo_t *mouse_vo;
	uint32_t mouse_vo_size;
	#if JL_PLAT == JL_PLAT_COMPUTER //if computer - show mouse
		mouse_vo = jl_gl_vo_make(jlgr, 1);
		mouse_vo_size = sizeof(jl_vo_t);
	#elif JL_PLAT == JL_PLAT_PHONE // if phone - don't show mouse
		mouse_vo = NULL;
		mouse_vo_size = 0;
	#endif

	// Make sprite.
	jlgr->mouse = jlgr_sprite_new(
		jlgr, rc, jlgr_mouse_loop_, jlgr_mouse_draw_,
		NULL, 0, mouse_vo, mouse_vo_size);
	jl_print(jlgr->jl, "MOUSE VO: %p", mouse_vo);
	// Resize sprite.
	jlgr_sprite_resize(jlgr, jlgr->mouse, NULL);
	// Set the mouse's collision width and height to 0
	jlgr->mouse->pr.cb.ofs.x = 0.f, jlgr->mouse->pr.cb.ofs.y = 0.f;
}
