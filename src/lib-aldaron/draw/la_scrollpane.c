/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_config.h>
#ifdef LA_FEATURE_DISPLAY

#include "la_scrollpane.h"

static la_gui_scrollpane_t* la_scrollpane = NULL;
extern float la_banner_size;

static void la_gui_scrollpane_draw__(void* context) {
	float banner_size = la_banner_size;

	la_banner_size = la_safe_get_float(&la_scrollpane->scroller);
	la_scrollpane->drawfn(context);
	la_banner_size = banner_size;
}

void la_gui_scrollpane_redraw(la_window_t* window, la_gui_scrollpane_t* sp,
	jl_rect_t rc, float internal_height, la_fn_t drawfn)
{
	float colors[] = { 0.3f, 0.3f, 0.3f };
//	jl_rect_t irc = (jl_rect_t) { 0.f, 0.f, 1.f, internal_height };

	la_ro_plain_rect(window, &sp->external, colors, rc.w, rc.h);
//	jlgr_vo_set_rect(window, &sp->internal, irc, colors, 0);
	sp->drawfn = drawfn;
//	la_ro_pr(window, &sp->internal, drawfn);
	la_safe_set_float(&sp->scroller, 0.f);
	la_safe_set_uint8(&sp->external_update, 1);
	la_safe_set_float(&sp->internal_height, internal_height);
	la_safe_set_float(&sp->external_height, rc.h);
}

void la_gui_scrollpane_draw(void* context, la_window_t* window,
	la_gui_scrollpane_t* sp)
{
	if(la_safe_get_uint8(&sp->external_update)) {
		la_scrollpane = sp;
		la_ro_pr(context, window, &sp->external,
			la_gui_scrollpane_draw__);
	}
	la_ro_draw(&sp->external);
	la_ro_pr_draw(&sp->external, 0);
}

void la_gui_scrollpane_loop(la_window_t* window, la_gui_scrollpane_t* sp) {
	float limit = -(la_safe_get_float(&sp->internal_height) - la_safe_get_float(&sp->external_height));

	if(limit > 0.f) return;
	if(window->input.scroll.y || window->input.drag.y) {
		float newvalue = la_safe_get_float(&sp->scroller)
			+ (window->input.scroll.y * .05f) + window->input.drag.y;
		if(newvalue > 0.f)
			newvalue = 0.f;
		else if(newvalue < limit)
			newvalue = limit;
		la_safe_set_float(&sp->scroller, newvalue);
		la_safe_set_uint8(&sp->external_update, 1);
	}
}

#endif
