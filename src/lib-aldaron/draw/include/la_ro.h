/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#ifndef LA_RO
#define LA_RO

#include <la_config.h>
#ifndef LA_FEATURE_DISPLAY
	#error "please add #define LA_FEATURE_DISPLAY to your la_config.h"
#endif

#include <la_math.h>
#include <la_draw.h>

void la_ro_rect(la_window_t* window, la_ro_t* ro, float w, float h);
void la_ro_color_rect(la_window_t* window, la_ro_t* ro, float* colors,
	float w, float h);
void la_ro_plain_rect(la_window_t* window, la_ro_t* ro, float* colors,
	float w, float h);
void la_ro_image_rect(la_window_t* window, la_ro_t *ro, uint32_t tex, float w,
	float h);
void la_ro_change_image(la_ro_t *ro, uint32_t img,
	uint8_t w, uint8_t h, int16_t map, uint8_t orient);
void la_ro_move(la_ro_t* ro, la_v3_t pos);
void la_ro_draw(la_ro_t* ro);
void la_ro_pr_draw(la_ro_t* ro, uint8_t orient);
void la_ro_pr(void* context, la_window_t* window, la_ro_t* ro, la_fn_t drawfn);
void la_ro_free(la_ro_t *ro);

#endif
