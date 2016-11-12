/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#ifndef LA_DRAW
#define LA_DRAW

#include <la_config.h>
#ifndef LA_FEATURE_DISPLAY
	#error "please add #define LA_FEATURE_DISPLAY to your la_config.h"
#endif

#include <la.h>
#include <la_math.h>

#include "SDL_events.h"
#include "la_safe.h"

//#define LA_TEXT_CMD "\x01"
//#defineLA_TEXT_BOLD LA_TEXT_CMD "\x01"
//#define LA_TEXT_ITALIC LA_TEXT_CMD "\x02"
//#define LA_TEXT_THIN LA_TEXT_CMD "\x03"
//#definLA_TEXT_NORMAL LA_TEXT_CMD "\x04"
//#define LA_TEXT_ALIGNL LA_TEXT_CMD "\x10"
//#define LA_TEXT_ALIGNC LA_TEXT_CMD "\x11"
//#define LA_TEXT_ALIGNR LA_TEXT_CMD "\x12"
//#define LA_TEXT_ALIGNJ LA_TEXT_CMD "\x13"

// Graphical stuff

typedef struct{
	struct {
		int32_t position;
		int32_t texpos_color;
	}attributes;

	struct {
		int32_t texture;
		// Matrices
		int32_t scale_object;
		int32_t rotate_object;
		int32_t translate_object;
		int32_t rotate_camera;
		int32_t project_scene;
	}uniforms;

	uint32_t program;
}la_shader_t;

#include "la_port.h"

typedef void (*la_draw_fn_t)(void* context, la_window_t* window);

// la_draw.c
void la_draw_dont(void* context, la_window_t* window);
void la_screen_setprimary(la_window_t* window, la_draw_fn_t new_fn);
void la_draw_fnchange(la_window_t* window, la_draw_fn_t primary,
	la_draw_fn_t secondary, la_draw_fn_t resize);

// la_texture.h
uint32_t la_texture_new(la_window_t*, uint8_t* pixels, uint32_t w, uint32_t h,
	uint8_t bpp);
void la_texture_set(la_window_t*, uint32_t texture, uint8_t* pixels,
	uint32_t w, uint32_t h, uint8_t bpp);
uint32_t la_texture_fpk(la_window_t* window, la_buffer_t* zipdata,
	const char* filename);

#endif
