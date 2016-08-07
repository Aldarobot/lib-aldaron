/*
 * JL_Lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLGRmouse.c
 *	This file handles the mouse.
**/
#include "JLGRprivate.h"

#if JL_PLAT == JL_PLAT_COMPUTER // show mouse if computer
void jlgr_mouse_resize__(jlgr_t* jlgr) {
	jl_rect_t rc = { 0.f, 0.f, .05f, .05f };

	jlgr_vo_set_image(jlgr, &jlgr->mouse, rc, jlgr->textures.font);
	jlgr_vo_txmap(jlgr, &jlgr->mouse, 0, 16, 16, 255);
	// Set the mouse's collision width and height to 0
	jlgr->mouse.pr.cb.ofs.x = 0.f, jlgr->mouse.pr.cb.ofs.y = 0.f;
}

void jlgr_mouse_draw__(jlgr_t* jlgr) {
	jl_print(jlgr->jl, "drawing mouse....");
	jlgr_vo_draw(jlgr, &jlgr->mouse);
}

// Run every frame for mouse
void jlgr_mouse_loop__(jlgr_t* jlgr) {
	jlgr_vo_move(&jlgr->mouse, (jl_vec3_t) {
		jlgr->main.ct.msx, jlgr->main.ct.msy, 0.f});
}

void jlgr_mouse_init__(jlgr_t* jlgr) {
	jlgr_vo_init(jlgr, &jlgr->mouse);
	jlgr_mouse_resize__(jlgr);
}
#endif
