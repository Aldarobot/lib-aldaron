/*
 * JL_Lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLGRgraphics.c
 *	A High Level Graphics Library that supports sprites, texture loading,
 *	2D rendering & 3D rendering.
 */
#include "JLGRinternal.h"

typedef struct {
	jl_vec3_t where[2];
	jl_vo_t* vo; // Vertex object [ Full, Slider 1, Slider 2 ].
}jl_gui_slider_draw;

typedef struct {
	float* x1;
	float* x2;
	uint8_t isRange;
	jl_gui_slider_draw draw;
}jl_gui_slider_main;

/***      @cond       ***/
/************************/
/*** Static Functions ***/
/************************/

static inline void _jlgr_init_vos(jlgr_t* jlgr) {
	jlgr->gr.vos.whole_screen = jl_gl_vo_make(jlgr, 1);
}

static void _jlgr_popup_loop(jl_t *jl) {
//		jlgr_draw_rect(jl, .1, .1, .8, .2, 127, 127, 255, 255);
//		jlgr_draw_rect(jl, .1, .3, .8, .8, 64, 127, 127, 255);
}

static void _jlgr_textbox_lt(jlgr_t* jlgr, jlgr_input_t input) {
	jl_ct_typing_disable();
	if(jlgr->gr.textbox_string->curs)
		jlgr->gr.textbox_string->curs--;
}

static void _jlgr_textbox_rt(jlgr_t* jlgr, jlgr_input_t input) {
	jl_ct_typing_disable();
	if(jlgr->gr.textbox_string->curs < jlgr->gr.textbox_string->size)
		jlgr->gr.textbox_string->curs++;
}

/**      @endcond      **/
/************************/
/*** Global Functions ***/
/************************/

void jlgr_dont(jlgr_t* jlgr) { }

/**
 * Prepare to draw an image that takes up the entire pre-renderer or
 * screen.
 * @param jlgr: The library context.
 * @param g: the image group that the image pointed to by 'i' is in.
 * @param i:  the ID of the image.
 * @param c: is 0 unless you want to use the image as
 * 	a charecter map, then it will zoom into charecter 'chr'.
 * @param a: the transparency each pixel is multiplied by; 255 is
 *	solid and 0 is totally invisble.
**/
void jlgr_fill_image_set(jlgr_t* jlgr,u16_t g,u16_t i,u8_t c,u8_t a){
	jl_rect_t rc = { 0., 0., 2., jl_gl_ar(jlgr) };

	jlgr_vos_image(jlgr, jlgr->gr.vos.whole_screen, rc, g, i, c, a);
}

/**
 * Draw the image prepared with jlgr_fill_image_set().
 * @param jl: The library context.
**/
void jlgr_fill_image_draw(jlgr_t* jlgr) {
	jlgr_draw_vo(jlgr, jlgr->gr.vos.whole_screen, NULL);
}

/**
 * Convert a color.
 * @param jl: The library context.
 * @param rgba: The color to convert ( Not freed - Reusable ).
 * @param vc: How many vertices to acount for.
 * @param gradient: 1 if "rgba" is a gradient array, 0 if solid color.
 * @returns: The converted color
**/
jl_ccolor_t* jlgr_convert_color(jlgr_t* jlgr, uint8_t *rgba, uint32_t vc,
	uint8_t gradient)
{
	if(gradient) return jl_gl_clrcg(jlgr, rgba, vc);
	else return jl_gl_clrcs(jlgr, rgba, vc);
}

/**
 * Change the coloring scheme for a vertex object.
 * @param jl: The library context.
 * @param pv: The Vertex Object
 * @param cc: The Converted Color Object to use on the Vertex Object.
 *	The library takes care of freeing this variable.
**/
void jlgr_vo_color(jlgr_t* jlgr, jl_vo_t* pv, jl_ccolor_t* cc) {
	jl_gl_clrc(jlgr, pv, cc);
}

/**
 * Draw a vertex object with offset by translation.
 * @param jl: The library context.
 * @param pv: The vertex object to draw.
 * @param vec: The vector of offset/translation.
**/
void jlgr_draw_vo(jlgr_t* jlgr, jl_vo_t* pv, jl_vec3_t* vec) {
	if(vec == NULL) {
		jl_gl_transform_vo_(jlgr, pv,
			0.f, 0.f, 0.f, 1., 1., 1.);
	}else{
		jl_gl_transform_vo_(jlgr, pv,
			vec->x, vec->y, vec->z, 1., 1., 1.);
	}
	jl_gl_draw(jlgr, pv);
}

/**
 * Set a Vertex object to vector graphics.
 * 
**/
void jlgr_vos_vec(jlgr_t* jlgr, jl_vo_t *pv, uint16_t tricount,
	float* triangles, uint8_t* colors, uint8_t multicolor)
{
	// Overwrite the vertex object
	jl_gl_vect(jlgr, pv, tricount * 3, triangles);
	// Texture the vertex object
	if(multicolor) jl_gl_clrg(jlgr, pv, colors);
	else jl_gl_clrs(jlgr, pv, colors);
}

/**
 * Set a vertex object to a rectangle.
 * @param jl: The library context.
 * @param pv: The vertex object
 * @param rc: The rectangle coordinates.
 * @param colors: The color(s) to use - [ R, G, B, A ]
 * @param multicolor: If 0: Then 1 color is used.
 *	If 1: Then 1 color per each vertex is used.
**/
void jlgr_vos_rec(jlgr_t* jlgr, jl_vo_t *pv, jl_rect_t rc, u8_t* colors,
	uint8_t multicolor)
{
	float rectangle_coords[] = {
		rc.x,		rc.h+rc.y,	0.f,
		rc.x,		rc.y,		0.f,
		rc.w + rc.x,	rc.y,		0.f,
		rc.w + rc.x,	rc.h+rc.y,	0.f };

	// Overwrite the vertex object
	jl_gl_poly(jlgr, pv, 4, rectangle_coords);
	// Texture the vertex object
	if(multicolor) jl_gl_clrg(jlgr, pv, colors);
	else jl_gl_clrs(jlgr, pv, colors);
}

/**
 * Set a vertex object to an Image.
 *
 * @param jl: The library context
 * @param pv: The vertex object
 * @param rc: the rectangle to draw the image in.
 * @param g: the image group that the image pointed to by 'i' is in.
 * @param i:  the ID of the image.
 * @param c: is 0 unless you want to use the image as
 * 	a charecter map, then it will zoom into charecter 'chr'.
 * @param a: the transparency each pixel is multiplied by; 255 is
 *	solid and 0 is totally invisble.
**/
void jlgr_vos_image(jlgr_t* jlgr, jl_vo_t *pv, jl_rect_t rc,
	u16_t g, u16_t i, u8_t c, u8_t a)
{
	//From bottom left & clockwise
	float Oone[] = {
		rc.x,		rc.h+rc.y,	0.f,
		rc.x,		rc.y,		0.f,
		rc.w+rc.x,	rc.y,		0.f,
		rc.w+rc.x,	rc.h+rc.y,	0.f };
	// Overwrite the vertex object
	jl_gl_poly(jlgr, pv, 4, Oone);
	// Texture the vertex object
	jl_gl_txtr(jlgr, pv, c, a, g, i);
}

void jlgr_vos_texture(jlgr_t* jlgr, jl_vo_t *pv, jl_rect_t rc,
	jl_tex_t* tex, u8_t c, u8_t a)
{
	//From bottom left & clockwise
	float Oone[] = {
		rc.x,		rc.h+rc.y,	0.f,
		rc.x,		rc.y,		0.f,
		rc.w+rc.x,	rc.y,		0.f,
		rc.w+rc.x,	rc.h+rc.y,	0.f };
	// Overwrite the vertex object
	jl_gl_poly(jlgr, pv, 4, Oone);
	// Texture the vertex object
	jl_gl_txtr_(jlgr, pv, c, a, tex->gl_texture);
}

/**
 * Free a vertex object.
 * @param jl: The library context
 * @param pv: The vertex object to free
**/
void jlgr_vo_old(jlgr_t* jlgr, jl_vo_t* pv) {
	jl_gl_vo_free(jlgr, pv);
}

/**
 * Draw text on the current pre-renderer.
 * @param 'jl': library context
 * @param 'str': the text to draw
 * @param 'loc': the position to draw it at
 * @param 'f': the font to use.
**/
void jlgr_draw_text(jlgr_t* jlgr, str_t str, jl_vec3_t loc, jl_font_t f) {
	const void *Str = str;
	const uint8_t *STr = Str;
	uint32_t i;
	jl_rect_t rc = { loc.x, loc.y, f.size, f.size };
	jl_vec3_t tr = { 0., 0., 0. };
	jl_vo_t* vo = jlgr->gl.temp_vo;

	if(str == NULL) return;
	for(i = 0; i < strlen(str); i++) {
		//Font 0:0
		jlgr_vos_image(jlgr,vo,rc,0,JL_IMGI_FONT,STr[i],255);
		jl_gl_transform_chr_(jlgr, vo, tr.x, tr.y, tr.z,
			1., 1., 1.);
		jl_gl_draw_chr(jlgr, vo,((double)f.colors[0])/255.,
			((double)f.colors[1])/255.,
			((double)f.colors[2])/255.,
			((double)f.colors[3])/255.);
		tr.x += f.size;
	}
}

/**
 * draw an integer on the screen
 * @param 'jl': library context
 * @param 'num': the number to draw
 * @param 'loc': the position to draw it at
 * @param 'f': the font to use.
 */
void jlgr_draw_int(jlgr_t* jlgr, i64_t num, jl_vec3_t loc, jl_font_t f) {
	char display[10];
	sprintf(display, "%ld", (long int) num);
	jlgr_draw_text(jlgr, display, loc, f);
}

/**
 * draw a floating point on the screen
 * @param 'jl': library context
 * @param 'num': the number to draw
 * @param 'dec': the number of places after the decimal to include.
 * @param 'loc': the position to draw it at
 * @param 'f': the font to use.
 */
void jlgr_draw_float(jlgr_t* jlgr, f64_t num, u8_t dec, jl_vec3_t loc,
	jl_font_t f)
{
	char display[10];
	char convert[10];

	sprintf(convert, "%%%df", dec);
	sprintf(display, convert, num);
	jlgr_draw_text(jlgr, display, loc, f);
}

/**
 * Draw text within the boundary of a sprite
 * @param 'jl': library context
 * @param 'spr': the boundary sprite
 * @param 'txt': the text to draw
**/
void jlgr_draw_text_area(jlgr_t* jlgr, jl_sprite_t * spr, str_t txt){
	float fontsize = .9 / strlen(txt);
	jlgr_draw_text(jlgr, txt,
		(jl_vec3_t) { .05,.5 * (jl_gl_ar(jlgr) - fontsize),0. },
		(jl_font_t) { 0, JL_IMGI_ICON, 0, jlgr->fontcolor, 
			fontsize});
}

/**
 * Draw a sprite, then draw text within the boundary of a sprite
 * @param 'jl': library context
 * @param 'spr': the boundary sprite
 * @param 'txt': the text to draw
**/
void jlgr_draw_text_sprite(jlgr_t* jlgr,jl_sprite_t * spr, str_t txt) {
	jlgr_fill_image_set(jlgr, 0, JL_IMGI_ICON, 1, 255);
	jlgr_fill_image_draw(jlgr);
	jlgr_draw_text_area(jlgr, spr, txt);
}

/**
 * Draw centered text across screen
 * @param 'jl': library context.
 * @param 'str': the text to draw
 * @param 'yy': y coordinate to draw it at
 * @param 'color': 255 = opaque, 0 = invisible
 */
void jlgr_draw_ctxt(jlgr_t* jlgr, char *str, float yy, uint8_t* color) {
	jlgr_draw_text(jlgr, str,
		(jl_vec3_t) { 0., yy, 0. },
		(jl_font_t) { 0, JL_IMGI_ICON, 0, color, 
			1. / ((float)strlen(str))} );
}

// TODO: MOVE
static void jlgr_gui_slider_touch(jlgr_t* jlgr, jlgr_input_t input) {
	jl_sprite_t* spr = input.data;
	jl_gui_slider_main* slider = jlgr_sprite_getcontext(spr);

	if(jlgr_sprite_collide(jlgr, spr, &jlgr->mouse) == 0 ||
	 input.h == 0)
		return;
	float x = jl_ct_gmousex(jlgr) - (jl_gl_ar(jlgr) * .05 * spr->pr.cb.ofs.x);
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
}

static void jlgr_gui_slider_singleloop(jl_t* jl, jl_sprite_t* spr) {
	jlgr_input_do(jl->jlgr, JL_CT_PRESS, jlgr_gui_slider_touch, spr);
}

static void jlgr_gui_slider_doubleloop(jl_t* jl, jl_sprite_t* spr) {
	jlgr_input_do(jl->jlgr, JL_CT_PRESS, jlgr_gui_slider_touch, spr);
}

static void jlgr_gui_slider_draw(jl_t* jl, uint8_t resize, void* data) {
	jl_gui_slider_draw* slider = data;

	jl_rect_t rc = { 0.005, 0.005, .99, jl_gl_ar(jl->jlgr) - .01 };
	jl_rect_t rc1 = { 0.0012, 0.0012, (jl_gl_ar(jl->jlgr) * .5) + .0075,
		jl_gl_ar(jl->jlgr) - .0024};
	jl_rect_t rc2 = { 0.005, 0.005, (jl_gl_ar(jl->jlgr) * .5) -.001,
		jl_gl_ar(jl->jlgr) - .01};
	uint8_t colors[] = { 15, 10, 0, 255 };

	jl_gl_clear(jl->jlgr, 25, 20, 0, 255);
	jlgr_vos_image(jl->jlgr, &(slider->vo[0]), rc, 0, JL_IMGI_FONT,
		235, 255);
	jlgr_vos_image(jl->jlgr, &(slider->vo[1]), rc2, 0, JL_IMGI_GAME,
		16, 255);
	jlgr_vos_rec(jl->jlgr, &(slider->vo[2]), rc1, colors, 0);
	// Draw Sliders
	jlgr_draw_vo(jl->jlgr, &(slider->vo[0]), NULL);
	// Draw Slide 1
	jlgr_draw_vo(jl->jlgr, &(slider->vo[2]), &slider->where[0]);
	jlgr_draw_vo(jl->jlgr, &(slider->vo[1]), &slider->where[0]);
	// Draw Slide 2
	jlgr_draw_vo(jl->jlgr, &(slider->vo[2]), &slider->where[1]);
	jlgr_draw_vo(jl->jlgr, &(slider->vo[1]), &slider->where[1]);
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
void jlgr_gui_slider(jlgr_t* jlgr, jl_sprite_t* sprite, jl_rect_t rectangle,
	u8_t isdouble, m_f32_t* x1, m_f32_t* x2)
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
	slider.draw.vo = jl_gl_vo_make(jlgr, 3);
	slider.isRange = isdouble;

	jlgr_sprite_init(jlgr, sprite, rectangle,
		jlgr_gui_slider_loop, jlgr_gui_slider_draw,
		&slider, sizeof(jl_gui_slider_main),
		&slider.draw, sizeof(jl_gui_slider_draw));
}

/**
 * Draw a background on the screen
**/
void jlgr_draw_bg(jlgr_t* jlgr, u16_t g, u16_t i, u8_t c) {
	jlgr_fill_image_set(jlgr, g, i, c, 255);
	jlgr_fill_image_draw(jlgr);
}

void jlgr_draw_loadingbar(jlgr_t* jlgr, f64_t loaded) {
	jl_rect_t bar = { .05, jl_gl_ar(jlgr)*.4,
		.95,jl_gl_ar(jlgr)*.45};
	u8_t colors[] = { 0, 255, 0, 255};

	jlgr_vos_rec(jlgr, NULL, bar, colors, 0);
}

//TODO: MOVE
void jlgr_draw_msge__(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;

	jl_print_function(jlgr->jl, "JLGR_MSGE2");
	jlgr_draw_bg(jlgr, jlgr->gr.msge.g, jlgr->gr.msge.i,
		jlgr->gr.msge.c);
	if(jlgr->gr.msge.message)
		jlgr_draw_ctxt(jlgr, jlgr->gr.msge.message, 9./32.,
			jlgr->fontcolor);
	jl_print_return(jlgr->jl, "JLGR_MSGE2");
}

/**
 * Print message on the screen.
 * @param 'jl': library context.
 * @param 'g':
 * @param 'i':
 * @param 'c':
 * @param 'format': the message
 */
void jlgr_draw_msge(jlgr_t* jlgr, u16_t g, u16_t i, u8_t c,
	m_str_t format, ...)
{
//		jl_mem_format(jlgr->jl, __VA_ARGS__);
/*		jl_print_function(jlgr->jl, "JLGR_MSGE");
	JL_PRINT_DEBUG(jlgr->jl, "Printing %p", message);

	jvct_t* _jl = jlgr->jl->_jl;
	if(_jl->has.quickloop) {
		if(jl_sdl_seconds_past__(jlgr->jl) <
			(1.f/(float)JL_FPS))
		{
			jl_print_return(jlgr->jl, "JLGR_MSGE");
			return;
		}else{
			jlgr->jl->time.prev_tick =
				jlgr->jl->time.this_tick;
		}
	}
	jlgr->gr.msge.message = message;
	jlgr->gr.msge.g = g;
	jlgr->gr.msge.i = i;
	jlgr->gr.msge.c = c;
	// Get old values
	jl_fnct upscreen = jlgr->sg.redraw.upper;
	jl_fnct downscreen  = jlgr->sg.redraw.lower;
	jl_fnct onescreen = jlgr->sg.redraw.single;
	u8_t inloop = _jl->fl.inloop;
	// Set Graphical loops.
	jlgr_loop_set(jlgr, jlgr_draw_msge__, jl_dont,
		jlgr_draw_msge__);
	_jl->fl.inloop = 1;
	// Set mode to EXIT.
	JL_PRINT_DEBUG(jlgr->jl, "Run Minimal Graphical Loop.");
	// Update events ( minimal )
	jl_ct_quickloop_(jlgr);
	// Deselect any pre-renderer.
	jlgr->gl.cp = NULL;
	// Redraw screen.
	_jl_sg_loop(jlgr);
	// Update Screen.
	jl_dl_loop__(jlgr);
	//
	JL_PRINT_DEBUG(jlgr->jl, "Set old values");
	jlgr_loop_set(jlgr, onescreen, upscreen, downscreen);
	_jl->fl.inloop = inloop;
	jl_print_return(jlgr->jl, "JLGR_MSGE");*/
}

/**
 * Print a message on the screen and then terminate the program
 * @param 'jl': library context
 * @param 'message': the message 
 */
void jlgr_term_msge(jlgr_t* jlgr, char *message) {
	jlgr_draw_msge(jlgr, 0, JL_IMGI_ICON, 1, message);
	jl_print(jlgr->jl, message);
	jl_sg_kill(jlgr->jl);
}

/**
 * Create a popup window.
 */
void jlgr_popup(jlgr_t* jlgr, char *name, char *message,
	jl_popup_button_t *btns, uint8_t opt_count)
{
	jlgr->gr.popup.window_name = name;
	jlgr->gr.popup.message = message;
	jlgr->gr.popup.btns = btns;
	jl_mode_override(jlgr->jl, (jl_mode_t)
		{jl_mode_exit, _jlgr_popup_loop, jl_dont});
}

/**
 * Re-draw/-size a slide button, and activate if it is pressed.
 * @param 'txt': the text to draw on the button.
**/
void jlgr_slidebtn_rsz(jlgr_t* jlgr, jl_sprite_t * spr, str_t txt) {
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
void jlgr_slidebtn_loop(jlgr_t* jlgr, jl_sprite_t * spr, float defaultx,
	float slidex, jlgr_input_fnct prun)
{
	spr->pr.cb.pos.x = defaultx;
	if(jlgr_sprite_collide(jlgr, &jlgr->mouse, spr)) {
		jlgr_input_do(jlgr, JL_CT_PRESS, prun, NULL);
		spr->pr.cb.pos.x = defaultx + slidex;
	}
	jlgr_sprite_draw(jlgr, spr);
}

/**
 * Draw a glow button, and activate if it is pressed.
 * @param 'jl': the libary context
 * @param 'spr': the sprite to draw
 * @param 'txt': the text to draw on the button.
 * @param 'prun': the function to run when pressed.
**/
void jlgr_glow_button_draw(jlgr_t* jlgr, jl_sprite_t * spr,
	char *txt, jlgr_input_fnct prun)
{
//		jlgr_sprite_redraw(jlgr, spr);
	jlgr_sprite_draw(jlgr, spr);
	if(jlgr_sprite_collide(jlgr, &jlgr->mouse, spr)) {
		jl_rect_t rc = { spr->pr.cb.pos.x, spr->pr.cb.pos.y,
			spr->pr.cb.ofs.x, spr->pr.cb.ofs.y };
		uint8_t glow_color[] = { 255, 255, 255, 64 };

		// Draw glow
		jlgr_vos_rec(jlgr,
			jlgr->gl.temp_vo, rc, glow_color, 0);
		jlgr_draw_vo(jlgr, jlgr->gl.temp_vo, NULL);
		// Description
		jlgr_draw_text(jlgr, txt,
			(jl_vec3_t)
				{0., jl_gl_ar(jlgr) - .0625, 0.},
			(jl_font_t) { 0, JL_IMGI_ICON, 0,
				jlgr->fontcolor, .05 });
		// Run if press
		jlgr_input_do(jlgr, JL_CT_PRESS, prun, NULL);
	}
}

/**
 * Draw A Textbox.
 * @return 1 if return/enter is pressed.
 * @return 0 if not.
*/
uint8_t jlgr_draw_textbox(jlgr_t* jlgr, float x, float y, float w,
	float h, data_t* string)
{
	uint8_t bytetoinsert = 0;

	jlgr->gr.textbox_string = string;
	if((bytetoinsert = jl_ct_typing_get(jlgr))) {
		if(bytetoinsert == '\b') {
			if(string->curs == 0) return 0;
			string->curs--;
			jl_data_delete_byte(jlgr->jl, string);
		}else if(bytetoinsert == '\02') {
			jl_data_delete_byte(jlgr->jl, string);
		}else if(bytetoinsert == '\n') {
			return 1;
		}else{
			jl_data_insert_byte(jlgr->jl, string,
				 bytetoinsert);
		}
//			JL_PRINT("inserting %1s\n", &bytetoinsert);
	}
	jlgr_input_do(jlgr, JL_CT_MAINLT, _jlgr_textbox_lt, NULL);
	jlgr_input_do(jlgr, JL_CT_MAINRT, _jlgr_textbox_rt, NULL);
//		jlgr_draw_image(jl, 0, 0, x, y, w, h, ' ', 255);
	jlgr_draw_text(jlgr, (char*)(string->data),
		(jl_vec3_t) {x, y, 0.},
		(jl_font_t) {0,JL_IMGI_ICON,0,jlgr->fontcolor,h});
//		jlgr_draw_image(jl, 0, 0,
//			x + (h*((float)string->curs-.5)), y, h, h, 252, 255);
	return 0;
}

/**
 * THREAD: Main thread.
 * Pop-Up a notification bar.
 * @param jl: the libary context
 * @param notification: The message to display.
*/
void jlgr_notify(jlgr_t* jlgr, str_t notification) {
	jlgr_comm_notify_t packet;
	packet.id = JLGR_COMM_NOTIFY;
	jl_mem_copyto(notification, packet.string, 256);
	packet.string[strlen(notification)] = '\0';

	jl_thread_comm_send(jlgr->jl, jlgr->comm2draw, &packet);
}

/***      @cond       ***/
/************************/
/***  ETOM Functions  ***/
/************************/

void _jlgr_loopb(jlgr_t* jlgr) {
	//Message Display
	if(jlgr->gr.notification.timeTilVanish > 0.f) {
		if(jlgr->gr.notification.timeTilVanish > .5) {
			uint8_t color[] = { 255, 255, 255, 255 };
			jlgr_draw_ctxt(jlgr, jlgr->gr.notification.message, 0,
				color);
		}else{
			uint8_t color[] = { 255, 255, 255, (uint8_t)
				(jlgr->gr.notification.timeTilVanish *
					255. / .5)};
			jlgr_draw_ctxt(jlgr, jlgr->gr.notification.message, 0,
				color);
		}
		jlgr->gr.notification.timeTilVanish-=jlgr->psec;
	}
}

void _jlgr_loopa(jlgr_t* jlgr) {
	if(!jlgr->menubar.menubar.mutex) return;
	jl_print_function(jlgr->jl, "GR_LP");
	// Draw the pre-rendered Menubar.
	if(!jlgr->fl.inloop) jlgr_sprite_draw(jlgr, &jlgr->menubar.menubar);
	// Update messages.
	_jlgr_loopb(jlgr);
	jl_print_return(jlgr->jl, "GR_LP");
	// Draw mouse
	if(jlgr->mouse.mutex) jlgr_sprite_draw(jlgr, &jlgr->mouse);
}

void jlgr_init__(jlgr_t* jlgr) {
	_jlgr_init_vos(jlgr);
	JL_PRINT_DEBUG(jlgr->jl, "Draw Loading Screen");
	jlgr_draw_msge(jlgr, 0, 0, 0, 0);
	// Create Font
	jlgr->fontcolor[0] = 0;
	jlgr->fontcolor[1] = 0;
	jlgr->fontcolor[2] = 0;
	jlgr->fontcolor[3] = 255;
	jlgr->font = (jl_font_t)
		{ 0, JL_IMGI_FONT, 0, jlgr->fontcolor, .04 };
	JL_PRINT_DEBUG(jlgr->jl, "Loading 1 image");
	jl_sg_add_some_imgs_(jlgr, 1);
	//
	JL_PRINT_DEBUG(jlgr->jl, "First font use try");
	jlgr_draw_msge(jlgr,0,0,0,"LOADING JL_LIB GRAPHICS...");
	JL_PRINT_DEBUG(jlgr->jl, "First font use succeed");
	// Load the other images.
	JL_PRINT_DEBUG(jlgr->jl, "Loading 2 image");
	jl_sg_add_some_imgs_(jlgr, 2);
	jlgr_draw_msge(jlgr, 0, 0, 0, "LOADED JL_LIB GRAPHICS!");
	// Draw message on the screen
	jlgr_draw_msge(jlgr, 0, 0, 0, "LOADING JLLIB....");
	// Set other variables
	jlgr->gr.notification.timeTilVanish = 0.f;
}

/**      @endcond      **/
/***   #End of File   ***/

m_u8_t* jlgr_load_image(jl_t* jl, data_t* data, m_u16_t* w, m_u16_t* h) {
	return jl_vi_load_(jl, data, w, h);
}
