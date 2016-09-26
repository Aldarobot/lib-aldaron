#ifndef LA_VO
#define LA_VO

#include "la_draw.h"

typedef jl_vo_t la_vo_t;
typedef jl_vec3_t la_v3_t;

void la_vo_pr(la_window_t* window, jl_vo_t* vo, jl_fnct drawfn);
void la_vo_rect(la_window_t* window, jl_vo_t* vo, float w, float h);
void la_vo_color_rect(la_window_t* window, la_vo_t* vo, float* colors,
	float w, float h);
void la_vo_plain_rect(la_window_t* window, la_vo_t* vo, float* colors,
	float w, float h);
void la_vo_move(la_vo_t* vo, la_v3_t pos);
void la_vo_draw(la_vo_t* vo);

#endif
