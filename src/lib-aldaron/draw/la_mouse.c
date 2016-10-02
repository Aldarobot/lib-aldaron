/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_config.h>
#ifdef LA_FEATURE_DISPLAY

#include "JLGRprivate.h"
#include "la_vo.h"

//#if JL_PLAT == JL_PLAT_COMPUTER // show mouse if computer
void jlgr_mouse_resize__(la_window_t* jlgr) {
	jl_rect_t rc = { 0.f, 0.f, .05f, .05f };

	jlgr_vo_set_image(jlgr, &jlgr->mouse, rc, jlgr->textures.cursor);
	jlgr_vo_txmap(jlgr, &jlgr->mouse, 0, 4, 1, 0);
	// Set the mouse's collision width and height to 0
	jlgr->mouse.pr.cb.ofs.x = 0.f, jlgr->mouse.pr.cb.ofs.y = 0.f;
}

void jlgr_mouse_draw__(la_window_t* jlgr) {
	if(jlgr->mouse.window == NULL) return;
	la_vo_move(&jlgr->mouse, (la_v3_t) {
		la_safe_get_float(&jlgr->mouse_x),
		la_safe_get_float(&jlgr->mouse_y), 0.f});
	la_vo_draw(&jlgr->mouse);
}

#endif
