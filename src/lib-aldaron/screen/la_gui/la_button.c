/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include <la_config.h>
#ifdef LA_FEATURE_DISPLAY

#include <la_gui.h>

/**
 * Re-draw/-size a slide button, and activate if it is pressed.
 * @param spr: The slide button sprite.
 * @param txt: The text to draw on the button.
**/
/*void la_slidebtn_rsz(la_window_t* window, la_ro_t * spr, const char* txt) {
	la_draw_text_sprite(window, spr, txt);
}*/

/**
 * Run the Slide Button loop. ( activated when pressed, moved when
 *  hovered over. ) - And Draw Slide Button.
 * @param 'spr': the Slide Button Sprite.
 * @param 'defaultx': the default x position of the button.
 * @param 'slidex': how much the x should change when hovered above.
 * @param 'prun': the function to run when pressed.
**/
/*void la_slidebtn_loop(la_window_t* window, la_ro_t * spr, float defaultx,
	float slidex, la_draw_fn_t prun)
{
	spr->pr.cb.pos.x = defaultx;
//	if(la_sprite_collide(window, &window->mouse.pr, &spr->pr)) {
//		la_input_do(window, LA_INPUT_PRESS, prun, NULL);
//		spr->pr.cb.pos.x = defaultx + slidex;
//	}
//	la_sprite_draw(window, spr);
}*/

/**
 * Draw a glow button, and activate if it is pressed.
 * @param 'spr': the sprite to draw
 * @param 'txt': the text to draw on the button.
 * @param 'prun': the function to run when pressed.
**/
/*void la_glow_button_draw(la_window_t* window, la_ro_t * spr,
	char *txt, la_draw_fn_t prun)
{
//		la_sprite_redraw(window, spr);
//	la_sprite_draw(window, spr);
	if(la_sprite_collide(window, &window->mouse.pr, &spr->pr)) {
		la_rect_t rc = { spr->pr.cb.pos.x, spr->pr.cb.pos.y,
			spr->pr.cb.ofs.x, spr->pr.cb.ofs.y };
		float glow_color[] = { 1., 1., 1., .25 };

		// Draw glow
		la_ro_set_rect(window, &window->gl.temp_vo, rc, glow_color, 0);
		la_ro_draw(&window->gl.temp_vo);
		// Description
		la_text_draw(window, txt,
			(la_v3_t)
				{0., la_ro_ar(window) - .0625, 0.},
				window->fontcolor, .05 });
		// Run if press
//		la_input_do(window, LA_INPUT_PRESS, prun, NULL);
	}
}*/

#endif
