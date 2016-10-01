/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_mode.h>
#include <la.h>

void la_mode_init(void* context, la_mode_t* mode, la_mode_t newmode) {
	// Init new mode
	((la_fn_t) newmode.init)(context);
	// Update mode
	*mode = newmode;
}

void la_mode_change(void* context, la_mode_t* mode, la_mode_t newmode) {
	// Kill old mode
	((la_fn_t) mode->kill)(context);
	// Init new mode
	la_mode_init(context, mode, newmode);
}

void la_mode_run(void* context, la_mode_t mode) {
	((la_fn_t) mode.loop)(context);
}
