/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#ifndef LA_VO
#define LA_VO

#include "la_draw.h"

typedef jl_vec3_t la_v3_t;

void la_vo_rect(la_window_t* window, la_vo_t* vo, float w, float h);
void la_vo_color_rect(la_window_t* window, la_vo_t* vo, float* colors,
	float w, float h);
void la_vo_plain_rect(la_window_t* window, la_vo_t* vo, float* colors,
	float w, float h);
void la_vo_move(la_vo_t* vo, la_v3_t pos);
void la_vo_draw(la_vo_t* vo);
void la_vo_pr_draw(la_vo_t* vo, uint8_t orient);
void la_vo_pr(void* context, la_window_t* window, la_vo_t* vo, la_fn_t drawfn);

#endif
