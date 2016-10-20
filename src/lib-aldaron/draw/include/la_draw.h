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

#define JLGR_TEXT_CMD "\x01"
#define JLGR_TEXT_BOLD JLGR_TEXT_CMD "\x01"
#define JLGR_TEXT_ITALIC JLGR_TEXT_CMD "\x02"
#define JLGR_TEXT_THIN JLGR_TEXT_CMD "\x03"
#define JLGR_TEXT_NORMAL JLGR_TEXT_CMD "\x04"
#define JLGR_TEXT_ALIGNL JLGR_TEXT_CMD "\x10"
#define JLGR_TEXT_ALIGNC JLGR_TEXT_CMD "\x11"
#define JLGR_TEXT_ALIGNR JLGR_TEXT_CMD "\x12"
#define JLGR_TEXT_ALIGNJ JLGR_TEXT_CMD "\x13"

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
typedef void (*jlgr_fnct)(la_window_t* window);

// la_draw.c
void la_draw_dont(void* context, la_window_t* window);

void la_draw_fnchange(la_window_t* window, la_draw_fn_t primary,
	la_draw_fn_t secondary, la_draw_fn_t resize);

// JLGRgraphics.c:
void jlgr_fill_image_set(la_window_t* jlgr, uint32_t tex, uint8_t w, uint8_t h, 
	int16_t c);
void jlgr_fill_image_draw(la_window_t* jlgr);
void jlgr_draw_bg(la_window_t* jlgr, uint32_t tex, uint8_t w, uint8_t h, int16_t c);

// JLGRtext.c:
void jlgr_draw_msge(la_window_t* jlgr, uint32_t tex, uint8_t c, char* format, ...);
void jlgr_gui_textbox_init(la_window_t* jlgr, la_buffer_t* string);
uint8_t jlgr_gui_textbox_loop(la_window_t* jlgr);
void jlgr_gui_textbox_draw(la_window_t* jlgr, la_rect_t rc);
void jlgr_notify(la_window_t* jlgr, const char* notification, ...);

// la_texture.h
uint32_t la_texture_new(la_window_t* jlgr, uint8_t* pixels, uint16_t w, uint16_t h,
	uint8_t bpp);
void la_texture_set(la_window_t* jlgr, uint32_t texture, uint8_t* pixels,
	uint16_t w, uint16_t h, uint8_t bpp);
uint32_t la_texture_fpk(la_window_t* window, la_buffer_t* zipdata,
	const char* filename);

#endif
