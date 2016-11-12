/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_config.h>
#ifdef LA_FEATURE_DISPLAY

#include <la_draw.h>
#include <la_ro.h>

//#if defined(LA_COMPUTER) // show mouse if computer
void la_mouse_resize__(la_window_t* window) {
	la_ro_image_rect(window, &window->mouse, window->textures.cursor, .05f, .05f);
	la_ro_change_image(&window->mouse, window->textures.cursor, 4, 1, 0, 0);
	// Set the mouse's collision width and height to 0
	window->mouse.cb.ofs.x = 0.f, window->mouse.cb.ofs.y = 0.f;
}

void la_mouse_draw__(la_window_t* window) {
	if(window->mouse.window == NULL) return;
	la_ro_move(&window->mouse, (la_v3_t) {
		la_safe_get_float(&window->mouse_x),
		la_safe_get_float(&window->mouse_y), 0.f});
	la_ro_draw(&window->mouse);
}

#endif
