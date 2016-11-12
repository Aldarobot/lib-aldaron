/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#ifndef LA_SCROLLPANE
#define LA_SCROLLPANE

#include <la_config.h>
#ifndef LA_FEATURE_DISPLAY
	#error "please add #define LA_FEATURE_DISPLAY to your la_config.h"
#endif

#include <la_ro.h>
#include <la_safe.h>
#include <la_draw.h>

typedef struct {
	la_ro_t external;
	safe_float_t scroller;
	safe_float_t internal_height;
	safe_float_t external_height;
	safe_uint8_t external_update;
	la_fn_t drawfn;
} la_gui_scrollpane_t;

void la_gui_scrollpane_redraw(la_window_t* window, la_gui_scrollpane_t* sp,
	la_rect_t rc, float internal_height, la_fn_t drawfn);
void la_gui_scrollpane_draw(void* context, la_window_t* window,
	la_gui_scrollpane_t* sp);
void la_gui_scrollpane_loop(la_window_t* window, la_gui_scrollpane_t* sp);

#endif
