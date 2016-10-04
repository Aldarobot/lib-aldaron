/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#ifndef LA_WINDOW
#define LA_WINDOW

#include <la_config.h>
#ifndef LA_FEATURE_DISPLAY
	#error "please add #define LA_FEATURE_DISPLAY to your la_config.h"
#endif

void la_window_fullscreen(la_window_t* window, uint8_t is);
void la_window_fullscreen_toggle(la_window_t* window);
uint16_t la_window_width(la_window_t* window);
uint16_t la_window_height(la_window_t* window);
void la_window_name(la_window_t* window, const char* window_name);
void la_window_icon(la_window_t* window,la_buffer_t* buffer,const char* fname);

#endif
