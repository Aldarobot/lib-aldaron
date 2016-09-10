#include "la_scrollpane.h"

static la_gui_scrollpane_t* la_scrollpane = NULL;
extern float la_banner_size;

static void la_gui_scrollpane_draw__(jl_t* jl) {
	la_window_t* window = jl->jlgr;
	float banner_size = la_banner_size;

	la_banner_size = la_safe_get_float(&la_scrollpane->scroller);
	jl_gl_clear(window, 0.f, 0.f, 0.f, 0.f);
	la_scrollpane->drawfn(jl);
	la_banner_size = banner_size;
}

void la_gui_scrollpane_redraw(la_window_t* window, la_gui_scrollpane_t* sp,
	jl_rect_t rc, float internal_height, jl_fnct drawfn)
{
	float colors[] = { 0.3f, 0.3f, 0.3f };
//	jl_rect_t irc = (jl_rect_t) { 0.f, 0.f, 1.f, internal_height };

	jlgr_vo_set_rect(window, &sp->external, rc, colors, 0);
//	jlgr_vo_set_rect(window, &sp->internal, irc, colors, 0);
	sp->drawfn = drawfn;
//	la_vo_pr(window, &sp->internal, drawfn);
	la_safe_set_float(&sp->scroller, 0.f);
	la_safe_set_uint8(&sp->external_update, 1);
	la_safe_set_float(&sp->internal_height, internal_height);
	la_safe_set_float(&sp->external_height, rc.h);
}

void la_gui_scrollpane_draw(la_window_t* window, la_gui_scrollpane_t* sp) {
	if(la_safe_get_uint8(&sp->external_update)) {
		la_scrollpane = sp;
		la_vo_pr(window, &sp->external, la_gui_scrollpane_draw__);
	}
	jlgr_vo_draw(window, &sp->external);
	jlgr_vo_draw_pr(window, &sp->external);
}

void la_gui_scrollpane_loop(la_window_t* window, la_gui_scrollpane_t* sp) {
	float limit = -(la_safe_get_float(&sp->internal_height) - la_safe_get_float(&sp->external_height));

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
