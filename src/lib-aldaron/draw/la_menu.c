/*
 * JL_Lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLGRmenu.c
 *	This file handles the menubar.
**/
#include "JLGRprivate.h"

#define MENU_REDRAW_NONE -1
#define MENU_REDRAW_ALL -2

char *GMessage[3] = {
	"SCREEN: UPPER",
	"SCREEN: LOWER",
	"SCREEN: SINGLE"
};

static inline void jlgr_menubar_shadow__(la_window_t* window) {
	// Clear Texture.
	jl_gl_clear(window, 0.f, 0.f, 0.f, 0.f);
	// Draw Shadows.
	for(window->menu.redraw = 0; window->menu.redraw < 10; window->menu.redraw++){
		jl_vec3_t tr = { .895 - (.1 * window->menu.redraw), 0.005, 0. };
		jlgr_fnct _draw_icon_ = window->menu.redrawfn[window->menu.redraw];

		if(_draw_icon_ == NULL) break;
		// Draw shadow
		jlgr_vo_move(&window->menu.shadow, tr);
		jlgr_vo_draw(window, &window->menu.shadow);
		// Draw Icon
		_draw_icon_(window);
	}
}

// Run whenever a redraw is needed for an icon.
static void jlgr_menubar_draw_(jl_t* jl) {
	la_window_t* window = jl->jlgr;
	jl_thread_mutex_lock(&window->menu.mutex);

	if(window->menu.redraw == MENU_REDRAW_ALL) {
		// If needed, draw shadow.
		jlgr_menubar_shadow__(window);
	}else if(window->menu.redraw != MENU_REDRAW_NONE) {
		// Redraw only the selected icon.
		if(window->menu.redrawfn[window->menu.redraw])
			((jlgr_fnct)window->menu.redrawfn[window->menu.redraw])(window);
	}
	// Done.
	window->menu.redraw = MENU_REDRAW_NONE;
	jl_thread_mutex_unlock(&window->menu.mutex);
}

void jlgr_menubar_init__(la_window_t* window) {
	jl_rect_t rc = { 0.f, 0.f, 1.f, .11f };
	jl_rect_t rc_icon = { 0., 0., .1, .1};
	jl_rect_t rc_shadow = { -.01, .01, .1, .1 };
	float shadow_color[] = { 0.f, 0.f, 0.f, .75f };

	jl_thread_mutex_new(window->jl, &window->menu.mutex);
	jl_thread_mutex_lock(&window->menu.mutex);

	// Make the shadow vertex object.
	jlgr_vo_set_rect(window, &window->menu.shadow, rc_shadow, shadow_color, 0);
	// Make the icon vertex object.
	jlgr_vo_set_image(window, &window->menu.icon, rc_icon, window->textures.icon);
	jlgr_vo_txmap(window, &window->menu.icon, 0, 16, 16, JLGR_ID_UNKNOWN);
	// Clear the menubar & make pre-renderer.
	for( window->menu.cursor = 0; window->menu.cursor < 10;
		window->menu.cursor++)
	{
		window->menu.inputfn[window->menu.cursor] = NULL;
		window->menu.redrawfn[window->menu.cursor] = NULL;
	}
	window->menu.cursor = -1;
	// Make the menubar.
	jlgr_vo_rect(window, &window->menu.menubar, &rc);
	jl_thread_mutex_unlock(&window->menu.mutex);

	jlgr_pr(window, &window->menu.menubar.pr, jlgr_menubar_draw_);
}

static void jlgr_menubar_text__(la_window_t* window, float* color, float y,
	const char* text)
{
	jl_vec3_t tr = { .9 - (.1 * window->menu.redraw), y, 0. };

	jlgr_text_draw(window, text, tr,
		(jl_font_t) { window->textures.icon, 0, color, 
			.1 / strlen(text)});
}

static void jlgr_menu_flip_draw__(la_window_t* window) {
	jlgr_menu_draw_icon(window, window->textures.icon, JLGR_ID_FLIP_IMAGE);
}

static void jlgr_menu_flip_press__(la_window_t* window) {
	if(!window->input.mouse.h && !window->input.touch.h) return;
	if(!window->input.mouse.p && !window->input.touch.p) return;
	// Actually Flip the screen.
	if(window->sg.cs == JL_SCR_UP) window->sg.cs = JL_SCR_SS;
	else if(window->sg.cs == JL_SCR_DN) window->sg.cs = JL_SCR_UP;
	else window->sg.cs = JL_SCR_DN;
	jlgr_notify(window, GMessage[window->sg.cs]);
	jlgr_resz(window, 0, 0);
}

static void jlgr_menu_name_draw2__(la_window_t* window) {
	jlgr_menu_draw_icon(window, window->textures.icon, JLGR_ID_UNKNOWN);
}

static void jlgr_menu_name_draw__(la_window_t* window) {
	float text_size = jl_gl_ar(window) * .5;

	jlgr_menu_name_draw2__(window);
	jlgr_text_draw(window, window->wm.windowTitle[0],
		(jl_vec3_t) { 1. - (jl_gl_ar(window) * (window->menu.redraw+1.)),
			0., 0. },
		(jl_font_t) { window->textures.icon, 0, window->fontcolor, 
			text_size});
	jlgr_text_draw(window, window->wm.windowTitle[1],
		(jl_vec3_t) { 1. - (jl_gl_ar(window) * (window->menu.redraw+1.)),
			text_size, 0. },
		(jl_font_t) { window->textures.icon, 0, window->fontcolor, 
			text_size});
}

static void jlgr_menu_slow_draw__(la_window_t* window) {
	jl_t* jl = window->jl;
	float color[] = { .5, .5, 1., 1. };
	char formated[80];

	// Draw the icon based on whether on time or not.
	jlgr_menu_draw_icon(window, window->textures.icon, window->sg.on_time ?
		JLGR_ID_GOOD_IMAGE : JLGR_ID_SLOW_IMAGE);
	// Report the seconds that passed.
	jl_mem_format(formated, "DrawFPS:%d", (int)(round(1. / window->psec)));
	jlgr_menubar_text__(window, color, 0., formated);
	jl_mem_format(formated, "MainFPS:%d", (int)(round(1. / jl->time.psec)));
	jlgr_menubar_text__(window, color, .05, formated);
}

static void jlgr_menu_slow_loop__(la_window_t* window) {
	window->menu.redraw = window->menu.cursor;	
}

void jlgr_menu_resize_(la_window_t* window) {
	jl_thread_mutex_lock(&window->menu.mutex);
	window->menu.cursor = -1;
	jlgr_vo_rect(window, &window->menu.menubar, NULL);
	jl_thread_mutex_unlock(&window->menu.mutex);
}

//
// Exported Functions
//

/**
 * Draw the menu bar.  Should be paired with jlgr_menu_loop().
 * @param jlgr: The library context
 * @param resize: Is window is being resized?
**/
void jlgr_menu_draw(la_window_t* window, uint8_t resize) {
	uint8_t redraw;

	jl_thread_mutex_lock(&window->menu.mutex);
	if(resize) window->menu.redraw = MENU_REDRAW_ALL;
	redraw = window->menu.redraw;
	jl_thread_mutex_unlock(&window->menu.mutex);
	// Pre-Render
	if(redraw != MENU_REDRAW_NONE)
		jlgr_pr(window, &window->menu.menubar.pr, jlgr_menubar_draw_);
	// Draw Pre-Rendered
	jlgr_vo_draw_pr(window, &window->menu.menubar);
}

/**
 * Check for menubar input.  Should be paired with jlgr_menu_draw().
 * @param jlgr: The library context
**/
void jlgr_menu_loop(la_window_t* window) {
	jl_thread_mutex_lock(&window->menu.mutex);
	const float mouse_x = la_safe_get_float(&window->mouse_x);
	const float mouse_y = la_safe_get_float(&window->mouse_y);
	const uint8_t selected = (uint8_t) ((1. - mouse_x) / .1);

	for(window->menu.cursor = 0; window->menu.cursor < 10; window->menu.cursor++) {
		// If A NULL function then, stop looping menubar.
		if( !(window->menu.inputfn[window->menu.cursor]) ) break;
		// Run the input loop.
		if(window->menu.cursor == selected && mouse_y < .1)
			((jlgr_fnct)window->menu.inputfn[window->menu.cursor])(window);
	}
	jl_thread_mutex_unlock(&window->menu.mutex);
}

void jlgr_menu_draw_icon(la_window_t* window, uint32_t tex, uint8_t c) {
	jl_rect_t rc_icon = { 0., 0., .1, .1};
	jl_vec3_t tr = { .9 - (.1 * window->menu.redraw), 0., 0. };

	jlgr_vo_set_image(window, &window->menu.icon, rc_icon, tex);
	jlgr_vo_txmap(window, &window->menu.icon, 0, 16, 16, c);
	jlgr_vo_move(&window->menu.icon, tr);
	jlgr_vo_draw(window, &window->menu.icon);
}

/**
 * Add an icon to the menubar
 *
 * @param jlgr: the libary context
 * @param inputfn: The function to run when the icon is / isn't pressed.
 * @param rdr: the function to run when redraw is called.
**/
void jlgr_menu_addicon(la_window_t* window, jlgr_fnct inputfn, jlgr_fnct rdr) {
	uint8_t i;

	jl_thread_mutex_lock(&window->menu.mutex);
	window->menu.cursor = -1;
	for(i = 0; i < 10; i++) if(!window->menu.inputfn[i]) break;
	// Set functions for: draw, press, not press
	window->menu.inputfn[i] = inputfn;
	window->menu.redrawfn[i] = rdr;
	jl_thread_mutex_unlock(&window->menu.mutex);
}

/**
 * Add the flip screen icon to the menubar.
 * @param jlgr: the libary context
**/
void jlgr_menu_addicon_flip(la_window_t* window) {
	jlgr_menu_addicon(window, jlgr_menu_flip_press__, jlgr_menu_flip_draw__);	
}

/**
 * Add slowness detector to the menubar.
 * @param jlgr: the libary context
**/
void jlgr_menu_addicon_slow(la_window_t* window) {
	jlgr_menu_addicon(window, jlgr_menu_slow_loop__, jlgr_menu_slow_draw__);
}

/**
 * Add program title to the menubar.
 * @param jlgr: the libary context
**/
void jlgr_menu_addicon_name(la_window_t* window) {
	int i;
	for(i = 0; i < 4; i++) {
		jlgr_menu_addicon(window, NULL, jlgr_menu_name_draw2__);
	}
	jlgr_menu_addicon(window, NULL, jlgr_menu_name_draw__);
}
