/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_config.h>
#ifdef LA_FEATURE_DISPLAY

#include <la_gui.h>

//static void la_slider_touch__(la_window_t* window) {
/*	la_ro_t* spr = input.data;
	la_gui_slider_main* slider = la_sprite_getcontext(spr);

	if(la_sprite_collide(window, &spr->pr, &window->mouse.pr) == 0 ||
	 input.h == 0)
		return;
	float x = al_safe_get_float(&window->main.ct.msx) - (la_ro_ar(window) * .05 * spr->pr.cb.ofs.x);
	x -= spr->pr.cb.pos.x;
	x /= spr->pr.cb.ofs.x;
//		x += 1.5;// - (la_ro_ar(window) * .1);
	if(x <= 0.) x = 0.;
	if(x > 1. - (la_ro_ar(window) * .15))
		x = 1. - (la_ro_ar(window) * .15);
//
	if(slider->isRange) {
		double v0 = fabs((*slider->x1) - x);
		double v1 = fabs((*slider->x2) - x);
		if(v1 < v0) {
			(*slider->x2) = x /
				(1. - (la_ro_ar(window) * .15));
			slider->draw.where[1].x = x;
		}else{
			(*slider->x1) = x /
				(1. - (la_ro_ar(window) * .15));
			slider->draw.where[0].x = x;
		}
	}else{
		(*slider->x1) = x / (1. - (la_ro_ar(window) * .15));
		slider->draw.where[0].x = x;
	}
	la_sprite_redraw(window, spr, &slider->draw);*/
//}

//static void la_slider_singleloop__(void* window, la_ro_t* spr) {
//	la_input_do(window, INPUT_PRESS, la_slider_touch__, spr);
//}

//static void la_slider_doubleloop__(void* window, la_ro_t* spr) {
//	la_input_do(window, INPUT_PRESS, la_slider_touch__, spr);
//}

//static void la_slider_draw__(la_window_t* window, uint8_t resize, void* data){
/*	la_gui_slider_draw* slider = data;

	la_rect_t rc = { 0.005, 0.005, .99, la_ro_ar(window) - .01 };
	la_rect_t rc1 = { 0.0012, 0.0012, (la_ro_ar(window) * .5) + .0075,
		la_ro_ar(window) - .0024};
	la_rect_t rc2 = { 0.005, 0.005, (la_ro_ar(window) * .5) -.001,
		la_ro_ar(window) - .01};
	float colors[] = { .06f, .04f, 0.f, 1.f };

	la_window_clear(.01, .08, 0., 1.);
	la_ro_set_image(window, &(slider->vo[0]), rc, window->textures.font);
	la_ro_txmap(window, &(slider->vo[0]), 0, 16, 16, 235);
	la_ro_set_image(window, &(slider->vo[1]), rc2, window->textures.game);
	la_ro_txmap(window, &(slider->vo[1]), 0, 16, 16, 16);
	
	la_ro_set_rect(window, &(slider->vo[2]), rc1, colors, 0);
	// Draw Sliders
	la_ro_draw(&(slider->vo[0]));
	// Draw Slide 1
	la_ro_move(&slider->vo[2], slider->where[0]);
	la_ro_draw(&slider->vo[2]);
	la_ro_move(&slider->vo[1], slider->where[0]);
	la_ro_draw(&slider->vo[1]);
	// Draw Slide 2
	la_ro_move(&slider->vo[2], slider->where[1]);
	la_ro_draw(&slider->vo[2]);
	la_ro_move(&slider->vo[1], slider->where[1]);
	la_ro_draw(&slider->vo[1]);*/
//}

/**
 * Create a slider sprite.
 * THREAD: Drawing thread only.
 * @param sprite: Uninitialized sprite to initialize.
 * @param rectange: Area to put the slider in.
 * @param isdouble: 1 to select range, 0 to select a specific value.
 * @param x1: Pointer to a permanent location for the slider value.
 * @param x2: Pointer to a permanent location for the second slider
	value.  Ignored if #isdouble is 0.
 * @returns: The slider sprite.
**/
void la_slider(la_window_t* window, la_ro_t* sprite, la_rect_t rectangle,
	uint8_t isdouble, float* x1, float* x2)
{
//	la_sprite_loop_fnt la_gui_slider_loop;

//	if(isdouble) {
//		la_gui_slider_loop = la_slider_doubleloop__;
//	}else{
//		la_gui_slider_loop = la_slider_singleloop__;
//	}

/*	la_gui_slider_main slider;

	slider.draw.where[0] = (la_v3_t) { 0., 0., 0. };
	slider.draw.where[1] = (la_v3_t) { 1. - (la_ro_ar(window) * .075),
		0., 0. };
	slider.x1 = x1, slider.x2 = x2;
	(*slider.x1) = 0.;
	(*slider.x2) = 1.;
	slider.isRange = isdouble;*/
}

#endif
