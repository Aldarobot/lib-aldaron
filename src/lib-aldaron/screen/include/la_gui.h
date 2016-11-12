/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#ifndef LA_GUI
#define LA_GUI

#include <la_config.h>
#ifndef LA_FEATURE_DISPLAY
	#error "please add #define LA_FEATURE_DISPLAY to your la_config.h"
#endif

#include <la_safe.h>
#include <la_draw.h>

typedef struct {
	safe_float_t timeTilVanish;
	safe_string_t message;
}la_notify_t;

typedef struct {
	la_window_t* window;
	la_buffer_t* string;
	float counter;
	uint8_t do_it;
	uint8_t cursor;
	la_rect_t rc;
}la_textbox_t;

void la_notify(la_notify_t* notify, const char* notification, ...);
void la_notify_draw(la_window_t* window, la_notify_t* notify);

void la_textbox_edit(la_textbox_t* textbox, la_window_t* window,
	la_buffer_t* string, la_rect_t rc);
uint8_t la_textbox_loop(la_textbox_t* textbox);
void la_textbox_draw(la_textbox_t* textbox);

void la_gui_bg(la_window_t* window, uint32_t tex);
void la_gui_loadingbar(la_window_t* window, double loaded);

#endif
