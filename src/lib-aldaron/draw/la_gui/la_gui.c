/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_config.h>
#ifdef LA_FEATURE_DISPLAY

#include <la_draw.h>
#include <la_ro.h>
#include <la_text.h>

typedef struct {
	la_v3_t where[2];
	la_ro_t vo[3]; // Vertex object [ Full, Slider 1, Slider 2 ].
}la_gui_slider_draw;

typedef struct {
	float* x1;
	float* x2;
	uint8_t isRange;
	la_gui_slider_draw draw;
}la_gui_slider_main;

extern float la_banner_size;

void la_gui_bg(la_window_t* window, uint32_t tex) {
	la_ro_image_rect(window, &window->gui.vos.whole_screen, tex, 1.f, 2.f);
//	la_ro_change_image(&window->gui.vos.whole_screen, tex, w, h, c, 0);
	la_ro_draw(&window->gui.vos.whole_screen);
}

void la_gui_loadingbar(la_window_t* window, double loaded) {
	float colors[] = { 0., 1., 0., 1. };

	la_ro_plain_rect(window, NULL, colors, .95, la_ro_ar(window) * .45);
}

#endif
