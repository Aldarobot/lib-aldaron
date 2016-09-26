/*
 * JL_Lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
#include "JLGRprivate.h"
#include "la_buffer.h"
#include "la_vo.h"

void* aldaron_data(void);
uint64_t aldaron_size(void);

typedef struct {
	jl_vec3_t where[2];
	jl_vo_t vo[3]; // Vertex object [ Full, Slider 1, Slider 2 ].
}jl_gui_slider_draw;

typedef struct {
	float* x1;
	float* x2;
	uint8_t isRange;
	jl_gui_slider_draw draw;
}jl_gui_slider_main;

/** @cond */

void jlgr_mouse_draw__(la_window_t* jlgr);

extern float la_banner_size;

static void _jlgr_popup_loop(jl_t *jl) {
}

/*static void jlgr_gui_textbox_cursor__(la_window_t* jlgr, jlgr_input_t input) {
	if(input.h == 2) jlgr->gui.textbox.counter += jlgr->jl->time.psec;
	if(input.h != 1 && jlgr->gui.textbox.do_it != 1) return;
	switch(input.k) {
		case JLGR_INPUT_DIR_RT: {
			if(jlgr->gui.textbox.string->curs <
			 jlgr->gui.textbox.string->size)
				jlgr->gui.textbox.string->curs++;
			jlgr_input_typing_disable();
			jlgr->gui.textbox.cursor = 1;
			jlgr->gui.textbox.counter = 0.f;
			break;
		}
		case JLGR_INPUT_DIR_LT: {
			if(jlgr->gui.textbox.string->curs)
				jlgr->gui.textbox.string->curs--;
			jlgr_input_typing_disable();
			jlgr->gui.textbox.cursor = 1;
			jlgr->gui.textbox.counter = 0.f;
			break;
		}
	}
}*/

/** @endcond */

void jlgr_dont(la_window_t* jlgr) { }

/**
 * Prepare to draw an image that takes up the entire pre-renderer or
 * screen.
 * @param jlgr: The library context.
 * @param tex: The texture to draw.
 * @param w: Width ( in characters ) of character map.
 * @param h: Height ( in characters ) of character map.
 * @param c: is -1 unless you want to use the image as
 * 	a charecter map, then it will zoom into charecter 'chr'.
**/
void jlgr_fill_image_set(la_window_t* jlgr, uint32_t tex, uint8_t w, uint8_t h, 
	int16_t c)
{
	if(!tex) la_panic("jlgr_fill_image_set: Texture Must Be Nonzero!\n");
	jl_rect_t rc = { 0., (.5f * la_window_h(jlgr)) - 1.f, 1.f, 2.f };

	jlgr_vo_set_image(jlgr, &jlgr->gui.vos.whole_screen, rc, tex);
	jlgr_vo_txmap(jlgr, &jlgr->gui.vos.whole_screen, 0, w, h, c);
}

/**
 * Draw the image prepared with jlgr_fill_image_set().
 * @param jl: The library context.
**/
void jlgr_fill_image_draw(la_window_t* jlgr) {
	jlgr_vo_draw(jlgr, &jlgr->gui.vos.whole_screen);
}

/**
 * Draw an integer on the screen
 * @param 'jl': library context
 * @param 'num': the number to draw
 * @param 'loc': the position to draw it at
 * @param 'f': the font to use.
 */
void jlgr_draw_int(la_window_t* jlgr, int64_t num, jl_vec3_t loc, jl_font_t f) {
	char display[10];
	sprintf(display, "%ld", (long int) num);
	jlgr_text_draw(jlgr, display, loc, f);
}

/**
 * draw a floating point on the screen
 * @param 'jl': library context
 * @param 'num': the number to draw
 * @param 'dec': the number of places after the decimal to include.
 * @param 'loc': the position to draw it at
 * @param 'f': the font to use.
 */
void jlgr_draw_dec(la_window_t* jlgr, double num, uint8_t dec, jl_vec3_t loc,
	jl_font_t f)
{
	char display[10];
	char convert[10];

	sprintf(convert, "%%.%df", dec);
	sprintf(display, convert, num);
	jlgr_text_draw(jlgr, display, loc, f);
}

/**
 * Draw text within the boundary of a sprite
 * @param 'jl': library context
 * @param 'spr': the boundary sprite
 * @param 'txt': the text to draw
**/
void jlgr_text_draw_area(la_window_t* jlgr, jl_sprite_t * spr, const char* txt) {
	float fontsize = .9 / strlen(txt);
	jlgr_text_draw(jlgr, txt,
		(jl_vec3_t) { .05,.5 * (jl_gl_ar(jlgr) - fontsize),0. },
		(jl_font_t) { jlgr->textures.icon, 0, jlgr->fontcolor, 
			fontsize});
}

/**
 * Draw a sprite, then draw text within the boundary of a sprite
 * @param 'jl': library context
 * @param 'spr': the boundary sprite
 * @param 'txt': the text to draw
**/
void jlgr_draw_text_sprite(la_window_t* jlgr, jl_sprite_t* spr, const char* txt) {
	jlgr_fill_image_set(jlgr, jlgr->textures.icon, 16, 16, 1);
	jlgr_fill_image_draw(jlgr);
	jlgr_text_draw_area(jlgr, spr, txt);
}

/**
 * Draw centered text across screen
 * @param 'jl': library context.
 * @param 'str': the text to draw
 * @param 'yy': y coordinate to draw it at
 * @param 'color': 1.f = opaque, 0.f = invisible
 */
void jlgr_draw_ctxt(la_window_t* jlgr, char *str, float yy, float* color) {
	jlgr_text_draw(jlgr, str,
		(jl_vec3_t) { 0., yy, 0. },
		(jl_font_t) { jlgr->textures.icon, 0, color, 
			1. / ((float)strlen(str))} );
}

// TODO: MOVE
/*static void jlgr_gui_slider_touch(la_window_t* jlgr, jlgr_input_t input) {
	jl_sprite_t* spr = input.data;
	jl_gui_slider_main* slider = jlgr_sprite_getcontext(spr);

	if(jlgr_sprite_collide(jlgr, &spr->pr, &jlgr->mouse.pr) == 0 ||
	 input.h == 0)
		return;
	float x = al_safe_get_float(&jlgr->main.ct.msx) - (jl_gl_ar(jlgr) * .05 * spr->pr.cb.ofs.x);
	x -= spr->pr.cb.pos.x;
	x /= spr->pr.cb.ofs.x;
//		x += 1.5;// - (jl_gl_ar(jl->jlgr) * .1);
	if(x <= 0.) x = 0.;
	if(x > 1. - (jl_gl_ar(jlgr) * .15))
		x = 1. - (jl_gl_ar(jlgr) * .15);
//
	if(slider->isRange) {
		double v0 = fabs((*slider->x1) - x);
		double v1 = fabs((*slider->x2) - x);
		if(v1 < v0) {
			(*slider->x2) = x /
				(1. - (jl_gl_ar(jlgr) * .15));
			slider->draw.where[1].x = x;
		}else{
			(*slider->x1) = x /
				(1. - (jl_gl_ar(jlgr) * .15));
			slider->draw.where[0].x = x;
		}
	}else{
		(*slider->x1) = x / (1. - (jl_gl_ar(jlgr) * .15));
		slider->draw.where[0].x = x;
	}
	jlgr_sprite_redraw(jlgr, spr, &slider->draw);
}*/

static void jlgr_gui_slider_singleloop(jl_t* jl, jl_sprite_t* spr) {
//	jlgr_input_do(jl->jlgr, JL_INPUT_PRESS, jlgr_gui_slider_touch, spr);
}

static void jlgr_gui_slider_doubleloop(jl_t* jl, jl_sprite_t* spr) {
//	jlgr_input_do(jl->jlgr, JL_INPUT_PRESS, jlgr_gui_slider_touch, spr);
}

static void jlgr_gui_slider_draw(jl_t* jl, uint8_t resize, void* data) {
	jl_gui_slider_draw* slider = data;
	la_window_t* jlgr = jl->jlgr;

	jl_rect_t rc = { 0.005, 0.005, .99, jl_gl_ar(jlgr) - .01 };
	jl_rect_t rc1 = { 0.0012, 0.0012, (jl_gl_ar(jlgr) * .5) + .0075,
		jl_gl_ar(jlgr) - .0024};
	jl_rect_t rc2 = { 0.005, 0.005, (jl_gl_ar(jlgr) * .5) -.001,
		jl_gl_ar(jlgr) - .01};
	float colors[] = { .06f, .04f, 0.f, 1.f };

	jl_gl_clear(jlgr, .01, .08, 0., 1.);
	jlgr_vo_set_image(jlgr, &(slider->vo[0]), rc, jlgr->textures.font);
	jlgr_vo_txmap(jlgr, &(slider->vo[0]), 0, 16, 16, 235);
	jlgr_vo_set_image(jlgr, &(slider->vo[1]), rc2, jlgr->textures.game);
	jlgr_vo_txmap(jlgr, &(slider->vo[1]), 0, 16, 16, 16);
	
	jlgr_vo_set_rect(jlgr, &(slider->vo[2]), rc1, colors, 0);
	// Draw Sliders
	jlgr_vo_draw(jlgr, &(slider->vo[0]));
	// Draw Slide 1
	la_vo_move(&slider->vo[2], slider->where[0]);
	la_vo_draw(&slider->vo[2]);
	la_vo_move(&slider->vo[1], slider->where[0]);
	la_vo_draw(&slider->vo[1]);
	// Draw Slide 2
	la_vo_move(&slider->vo[2], slider->where[1]);
	la_vo_draw(&slider->vo[2]);
	la_vo_move(&slider->vo[1], slider->where[1]);
	la_vo_draw(&slider->vo[1]);
}

/**
 * Create a slider sprite.
 * THREAD: Drawing thread only.
 * @param jlgr: The library context.
 * @param sprite: Uninitialized sprite to initialize.
 * @param rectange: Area to put the slider in.
 * @param isdouble: 1 to select range, 0 to select a specific value.
 * @param x1: Pointer to a permanent location for the slider value.
 * @param x2: Pointer to a permanent location for the second slider
	value.  Ignored if #isdouble is 0.
 * @returns: The slider sprite.
**/
void jlgr_gui_slider(la_window_t* jlgr, jl_sprite_t* sprite, jl_rect_t rectangle,
	uint8_t isdouble, float* x1, float* x2)
{
	jlgr_sprite_loop_fnt jlgr_gui_slider_loop;

	if(isdouble) {
		jlgr_gui_slider_loop = jlgr_gui_slider_doubleloop;
	}else{
		jlgr_gui_slider_loop = jlgr_gui_slider_singleloop;
	}

	jl_gui_slider_main slider;

	slider.draw.where[0] = (jl_vec3_t) { 0., 0., 0. };
	slider.draw.where[1] = (jl_vec3_t) { 1. - (jl_gl_ar(jlgr) * .075),
		0., 0. };
	slider.x1 = x1, slider.x2 = x2;
	(*slider.x1) = 0.;
	(*slider.x2) = 1.;
	slider.isRange = isdouble;

	jlgr_sprite_init(jlgr, sprite, rectangle,
		jlgr_gui_slider_loop, jlgr_gui_slider_draw,
		&slider, sizeof(jl_gui_slider_main),
		&slider.draw, sizeof(jl_gui_slider_draw));
}

/**
 * Draw a background on the screen
**/
void jlgr_draw_bg(la_window_t* jlgr, uint32_t tex, uint8_t w, uint8_t h, int16_t c) {
	jlgr_fill_image_set(jlgr, tex, w, h, c);
	jlgr_fill_image_draw(jlgr);
}

void jlgr_draw_loadingbar(la_window_t* jlgr, double loaded) {
	jl_rect_t bar = { .05, jl_gl_ar(jlgr)*.4,
		.95,jl_gl_ar(jlgr)*.45};
	float colors[] = { 0., 1., 0., 1. };

	jlgr_vo_set_rect(jlgr, NULL, bar, colors, 0);
}

//TODO: MOVE
void jlgr_draw_msge__(jl_t* jl) {
	la_window_t* jlgr = jl->jlgr;

	jlgr_draw_bg(jlgr, jlgr->gui.msge.t, 16, 16, jlgr->gui.msge.c);
	if(jlgr->gui.msge.message[0])
		jlgr_draw_ctxt(jlgr, jlgr->gui.msge.message, 9./32.,
			jlgr->fontcolor);
}

/**
 * Display on the screen.
 * @param jlgr: The library context.
 * @param draw_routine: Function that draws on screen.
**/
void jlgr_draw_loadscreen(la_window_t* jlgr, jl_fnct draw_routine) {
	jl_thread_mutex_lock(&jlgr->protected.mutex);
	jlgr_redraw_t old_redrawfns = jlgr->protected.functions.redraw;

	// Set Graphical loops.
	jlgr->protected.functions.redraw = (jlgr_redraw_t) {
		draw_routine, draw_routine,
		draw_routine, jl_dont };

	jl_thread_mutex_unlock(&jlgr->protected.mutex);

	// Update events
	la_port_input(jlgr);
	// Redraw screen.
	_jl_sg_loop(jlgr);
	// Update Screen.
	jl_wm_loop__(jlgr);
	//
	jl_thread_mutex_lock(&jlgr->protected.mutex);
	jlgr->protected.functions.redraw = old_redrawfns;
	jl_thread_mutex_unlock(&jlgr->protected.mutex);
}

/**
 * Print message on the screen.
 * @param jlgr: The library context.
 * @param tex: The background texture.
 * @param c: The character map setting.
 * @param format: The message
 */
void jlgr_draw_msge(la_window_t* jlgr, uint32_t tex, uint8_t c, char* format, ...) {
	la_print("jlgr_draw_msge");
	if(format) {
		va_list arglist;

		// Print on screen.
		va_start(arglist, format);
		vsprintf(jlgr->gui.msge.message, format, arglist);
		va_end(arglist);
	}else{
		jlgr->gui.msge.message[0] = '\0';
	}
	jlgr->gui.msge.t = tex;
	jlgr->gui.msge.c = c;
	la_print("DRAW LOADSCREEN");
	
	jlgr_draw_loadscreen(jlgr, jlgr_draw_msge__);
	la_print("DREW LOADSCREEN");
}

/**
 * Print a message on the screen and then terminate the program
 * @param 'jl': library context
 * @param 'message': the message 
 */
void jlgr_term_msge(la_window_t* jlgr, char *message) {
	jlgr_draw_msge(jlgr, jlgr->textures.icon, 1, message);
	la_panic(message);
}

/**
 * Create a popup window.
 */
void jlgr_popup(la_window_t* jlgr, char *name, char *message,
	jl_popup_button_t *btns, uint8_t opt_count)
{
	jlgr->gui.popup.window_name = name;
	jlgr->gui.popup.message = message;
	jlgr->gui.popup.btns = btns;
	jl_mode_override(jlgr->jl, (jl_mode_t)
		{jl_mode_exit, _jlgr_popup_loop, jl_dont});
}

/**
 * Re-draw/-size a slide button, and activate if it is pressed.
 * @param jlgr: The library context.
 * @param spr: The slide button sprite.
 * @param txt: The text to draw on the button.
**/
void jlgr_slidebtn_rsz(la_window_t* jlgr, jl_sprite_t * spr, const char* txt) {
	jlgr_draw_text_sprite(jlgr, spr, txt);
}

/**
 * Run the Slide Button loop. ( activated when pressed, moved when
 *  hovered over. ) - And Draw Slide Button.
 * @param 'jl': the libary context
 * @param 'spr': the Slide Button Sprite.
 * @param 'defaultx': the default x position of the button.
 * @param 'slidex': how much the x should change when hovered above.
 * @param 'prun': the function to run when pressed.
**/
void jlgr_slidebtn_loop(la_window_t* jlgr, jl_sprite_t * spr, float defaultx,
	float slidex, jlgr_fnct prun)
{
	spr->pr.cb.pos.x = defaultx;
//	if(jlgr_sprite_collide(jlgr, &jlgr->mouse.pr, &spr->pr)) {
//		jlgr_input_do(jlgr, JL_INPUT_PRESS, prun, NULL);
//		spr->pr.cb.pos.x = defaultx + slidex;
//	}
	jlgr_sprite_draw(jlgr, spr);
}

/**
 * Draw a glow button, and activate if it is pressed.
 * @param 'jl': the libary context
 * @param 'spr': the sprite to draw
 * @param 'txt': the text to draw on the button.
 * @param 'prun': the function to run when pressed.
**/
void jlgr_glow_button_draw(la_window_t* jlgr, jl_sprite_t * spr,
	char *txt, jlgr_fnct prun)
{
//		jlgr_sprite_redraw(jlgr, spr);
	jlgr_sprite_draw(jlgr, spr);
/*	if(jlgr_sprite_collide(jlgr, &jlgr->mouse.pr, &spr->pr)) {
		jl_rect_t rc = { spr->pr.cb.pos.x, spr->pr.cb.pos.y,
			spr->pr.cb.ofs.x, spr->pr.cb.ofs.y };
		float glow_color[] = { 1., 1., 1., .25 };

		// Draw glow
		jlgr_vo_set_rect(jlgr, &jlgr->gl.temp_vo, rc, glow_color, 0);
		jlgr_vo_draw(jlgr, &jlgr->gl.temp_vo);
		// Description
		jlgr_text_draw(jlgr, txt,
			(jl_vec3_t)
				{0., jl_gl_ar(jlgr) - .0625, 0.},
			(jl_font_t) { jlgr->textures.icon, 0,
				jlgr->fontcolor, .05 });
		// Run if press
//		jlgr_input_do(jlgr, JL_INPUT_PRESS, prun, NULL);
	}*/
}

/**
 * Set the data string for a textbox.
 * @param jlgr: The library context.
 * @param string: The string to store to.
**/
void jlgr_gui_textbox_init(la_window_t* jlgr, data_t* string) {
	jlgr->gui.textbox.string = string;
}

/**
 * Check for keyboard input and store in string.  Do not call before
 *	jlgr_gui_textbox_init().
 * @param jlgr: The library context.
 * @returns 1: if return/enter is pressed.
 * @returns 0: if not.
**/
uint8_t jlgr_gui_textbox_loop(la_window_t* jlgr) {
	int i;

	jlgr->gui.textbox.counter += jlgr->jl->time.psec;
	if(jlgr->gui.textbox.counter > .5) {
		jlgr->gui.textbox.counter -= .5;
		jlgr->gui.textbox.do_it = 1;
		if(jlgr->gui.textbox.cursor) jlgr->gui.textbox.cursor = 0;
		else jlgr->gui.textbox.cursor = 1;
	}else{
		jlgr->gui.textbox.do_it = 0;
	}
#if JL_PLAT == JL_PLAT_COMPUTER
//	jlgr_input_do(jlgr, JL_INPUT_JOYC, jlgr_gui_textbox_cursor__, NULL);
#endif
	for(i = 0; i < strlen(jlgr->input.text); i++) {
		jl_data_insert_byte(jlgr->jl, jlgr->gui.textbox.string,
			jlgr->input.text[i]);
	}
	if(jlgr->input.keyboard.h && jlgr->input.keyboard.p) {
		switch(jlgr->input.keyboard.k) {
		case '\b':
			if(jlgr->gui.textbox.string->curs == 0) return 0;
			jlgr->gui.textbox.string->curs--;
			jl_data_delete_byte(jlgr->jl, jlgr->gui.textbox.string);
			break;
		case '\02':
			jl_data_delete_byte(jlgr->jl, jlgr->gui.textbox.string);
			break;
		case '\n':
			return 1;
		default:
			break;
		}
	}
	return 0;
}

/**
 * Draw A Textbox.  Must be paired on main thread by jlgr_gui_textbox_loop().
 * @param jlgr: The library context.
 * @param rc: Dimensions to draw textbox.
*/
void jlgr_gui_textbox_draw(la_window_t* jlgr, jl_rect_t rc){
	float cursor_color[] = { 0.1f, 0.1f, 0.1f, 1.f };
	if(jlgr->gui.textbox.cursor) {
		jlgr_vo_set_rect(jlgr, &jlgr->gl.temp_vo, (jl_rect_t) {
			rc.x + (jlgr->gui.textbox.string->curs * rc.h * .75),
			rc.y, rc.h * .05, rc.h }, cursor_color, 0);
		jlgr_vo_draw(jlgr, &jlgr->gl.temp_vo);
	}
	jlgr_text_draw(jlgr, (char*)(jlgr->gui.textbox.string->data),
		(jl_vec3_t) {rc.x, rc.y, 0.},
		(jl_font_t) {jlgr->textures.icon,0,jlgr->fontcolor,rc.h});
//		jlgr_draw_image(jl, 0, 0,
//			x + (h*((float)string->curs-.5)), y, h, h, 252, 1.);

}

/**
 * THREAD: Main thread.
 * Pop-Up a notification bar.
 * @param jl: the libary context
 * @param notification: The message to display.
*/
void jlgr_notify(la_window_t* jlgr, const char* notification, ...) {
	jl_thread_mutex_lock(&jlgr->protected.mutex);
	va_list arglist;

	va_start( arglist, notification );
	vsprintf( jlgr->protected.notification.message, notification, arglist );
	va_end( arglist );
	jlgr->protected.notification.timeTilVanish = 4.5;

	jl_thread_mutex_unlock(&jlgr->protected.mutex);
}

/***      @cond       ***/
/************************/
/***  ETOM Functions  ***/
/************************/

void _jlgr_loopb(la_window_t* jlgr) {
	//Message Display
	jl_thread_mutex_lock(&jlgr->protected.mutex);
	if(jlgr->protected.notification.timeTilVanish > 0.f) {
		if(jlgr->protected.notification.timeTilVanish > .5) {
			float color[] = { 1., 1., 1., 1. };
			jlgr_draw_ctxt(jlgr, jlgr->protected.notification.message, 0,
				color);
		}else{
			float color[] = { 1., 1., 1.,
				(jlgr->protected.notification.timeTilVanish / .5)};
			jlgr_draw_ctxt(jlgr, jlgr->protected.notification.message, 0,
				color);
		}
		jlgr->protected.notification.timeTilVanish-=jlgr->psec;
	}
	jl_thread_mutex_unlock(&jlgr->protected.mutex);
}

void _jlgr_loopa(la_window_t* jlgr) {
	// Update messages.
	_jlgr_loopb(jlgr);
#if JL_PLAT == JL_PLAT_COMPUTER
	// Draw mouse
	jlgr_mouse_draw__(jlgr);
#endif
}

void jlgr_init__(la_window_t* jlgr) {
	data_t packagedata;

	la_buffer_fdata(&packagedata, aldaron_data(), aldaron_size());
	jlgr->textures.logo = jl_sg_add_image(jlgr, &packagedata,
		"/logo.png");
	la_print("Draw Loading Screen");
	jlgr_draw_msge(jlgr, jlgr->textures.logo, 0, 0);
	la_print("Drew Loading Screen");
	// Load Graphics
	jlgr->textures.font = jl_sg_add_image(jlgr, &packagedata,
		"/font.png");
	// Create Font
	jlgr->fontcolor[0] = 0.;
	jlgr->fontcolor[1] = 0.;
	jlgr->fontcolor[2] = 0.;
	jlgr->fontcolor[3] = 1.;
	jlgr->font = (jl_font_t)
		{ jlgr->textures.font, 0, jlgr->fontcolor, .04 };
	// Draw message on the screen
	jlgr_draw_msge(jlgr, jlgr->textures.logo, 0, "Loading Lib Aldaron....");
	// Set other variables
	jl_thread_mutex_lock(&jlgr->protected.mutex);
	jlgr->protected.notification.timeTilVanish = 0.f;
	jl_thread_mutex_unlock(&jlgr->protected.mutex);
	// Load other images....
	jlgr->textures.icon = jl_sg_add_image(jlgr, &packagedata,
		"/taskbar_items.png");
	jlgr->textures.game = jl_sg_add_image(jlgr, &packagedata,
		"/landscape.png");
	jlgr->textures.cursor = jl_sg_add_image(jlgr, &packagedata, "/cursor.png");
	jlgr->textures.backdrop = jl_sg_add_image(jlgr, &packagedata, "/backdrop.png");
}

/**      @endcond      **/
/***   #End of File   ***/
