/* Lib Aldaron --- Copyright (c) 2016 Jeron A. Lau */
/* This file must be distributed with the GNU LESSER GENERAL PUBLIC LICENSE. */
/* DO NOT REMOVE THIS NOTICE */

#include "JLGRprivate.h"
#include "la_menu.h"
#include "la_text.h"

#include <la_pr.h>

#define MENU_REDRAW_NONE -1
#define MENU_REDRAW_ALL -2

static char *GMessage[3] = {
	"SCREEN: UPPER",
	"SCREEN: LOWER",
	"SCREEN: SINGLE"
};

void la_draw_resize(la_window_t *, uint32_t, uint32_t);

static inline void jlgr_menubar_shadow__(la_menu_t* menu) {
	// Clear Texture.
	jl_gl_clear(menu->window, 0.f, 0.f, 0.f, 0.f);
	// Draw Shadows.
	for(menu->redraw = 0; menu->redraw < 10; menu->redraw++) {
		la_menu_fn_t _draw_icon_;
		if(!(_draw_icon_ = menu->redrawfn[menu->redraw]))
			break;

		// Draw shadow
		jlgr_vo_move(&menu->shadow, (jl_vec3_t) {
			.895 - (.1 * menu->redraw), 0.005, 0. });
		jlgr_vo_draw(menu->window, &menu->shadow);
		// Draw Icon
		_draw_icon_(menu);
	}
}

// Run whenever a redraw is needed for an icon.
static void jlgr_menubar_draw_(la_menu_t* menu) {
	if(menu->redraw == MENU_REDRAW_ALL) {
		// If needed, draw shadow.
		jlgr_menubar_shadow__(menu);
	}else if(menu->redraw != MENU_REDRAW_NONE) {
		// Redraw only the selected icon.
		if(menu->redrawfn[menu->redraw])
			((la_menu_fn_t)menu->redrawfn[menu->redraw])(menu);
	}
	// Done.
	menu->redraw = MENU_REDRAW_NONE;
}

void la_menu_init(la_menu_t* menu, la_window_t* window) {
	jl_rect_t rc_icon = { 0., 0., .1, .1};
	jl_rect_t rc_shadow = { -.01, .01, .1, .1 };
	float shadow_color[] = { 0.f, 0.f, 0.f, .75f };
	int i;

	menu->window = window;

	// Make the shadow vertex object.
	jlgr_vo_set_rect(window, &menu->shadow, rc_shadow, shadow_color, 0);
	// Make the icon vertex object.
	jlgr_vo_set_image(window, &menu->icon, rc_icon, window->textures.icon);
	jlgr_vo_txmap(window, &menu->icon, 0, 16, 16, JLGR_ID_UNKNOWN);
	// Clear the menubar & make pre-renderer.
	for( i = 0; i < 10; i++) {
		menu->inputfn[i] = NULL;
		menu->redrawfn[i] = NULL;
	}
	// Make the menubar.
	la_vo_rect(window, &menu->menubar, 1.f, .11f);

	la_pr(menu, window, &menu->menubar.pr, (jl_fnct) jlgr_menubar_draw_);
}

static void jlgr_menubar_text__(la_menu_t* menu, float* color, float y,
	const char* text)
{
	jl_vec3_t tr = { .9 - (.1 * menu->redraw), y, 0. };

	jlgr_text_draw(menu->window, text, tr,
		(jl_font_t) { menu->window->textures.icon, 0, color, 
			.1 / strlen(text)});
}

static void jlgr_menu_flip_draw__(la_menu_t* menu) {
	la_menu_drawicon(menu, menu->window->textures.icon, JLGR_ID_FLIP_IMAGE);
}

static void jlgr_menu_flip_press__(la_menu_t* menu) {
	if(!menu->window->input.mouse.h && !menu->window->input.touch.h) return;
	if(!menu->window->input.mouse.p && !menu->window->input.touch.p) return;
	// Actually Flip the screen.
	if(menu->window->sg.cs == JL_SCR_UP) menu->window->sg.cs = JL_SCR_SS;
	else if(menu->window->sg.cs == JL_SCR_DN) menu->window->sg.cs = JL_SCR_UP;
	else menu->window->sg.cs = JL_SCR_DN;
	jlgr_notify(menu->window, GMessage[menu->window->sg.cs]);
	la_draw_resize(menu->window, 0, 0);
}

static void la_menu_name_drawa__(la_menu_t* menu) {
	la_menu_drawicon(menu, menu->window->textures.icon, 21);
}

static void la_menu_name_drawb__(la_menu_t* menu) {
	la_menu_drawicon(menu, menu->window->textures.icon, 16);
}

static void la_menu_name_drawc__(la_menu_t* menu) {
	float offset = .9f - (.1f * menu->redraw) + 0.005f;

	la_menu_drawicon(menu, menu->window->textures.icon, 20);
	la_text(menu->window, LA_PXSIZE("0.04") LA_PXMOVE("%f", "0.005") "%s", offset, menu->window->wm.windowTitle[0]);
	la_text(menu->window, LA_PXSIZE("0.04") LA_PXMOVE("%f", "0.055") "%s", offset, menu->window->wm.windowTitle[1]);
}

static void jlgr_menu_slow_draw__(la_menu_t* menu) {
	float color[] = { .5, .5, 1., 1. };
//	char formated[80];

	// Draw the icon based on whether on time or not.
	la_menu_drawicon(menu, menu->window->textures.icon,
		/*menu->window->sg.on_time?*/JLGR_ID_GOOD_IMAGE/*:JLGR_ID_SLOW_IMAGE*/);
	// Report the seconds that passed.
//	jl_mem_format(formated, "DrawFPS:%d", (int)(round(1. / menu->window->psec)));
	jlgr_menubar_text__(menu, color, 0., "DRAW");
//	jl_mem_format(formated, "MainFPS:%d", (int)(round(1. / jl->time.psec)));
	jlgr_menubar_text__(menu, color, .05, "MAIN");
}

static void jlgr_menu_slow_loop__(la_menu_t* menu) {
	menu->redraw = menu->cursor;
}

//
// Exported Functions
//

/**
 * Draw the menu bar.  Should be paired with jlgr_menu_loop().
 * @param jlgr: The library context
 * @param resize: Is window is being resized?
**/
void la_menu_draw(la_menu_t* menu, uint8_t resize) {
	if(resize) menu->redraw = MENU_REDRAW_ALL;

	// Pre-Render
	if(menu->redraw != MENU_REDRAW_NONE) {
		la_pr(menu, menu->window, &menu->menubar.pr, (jl_fnct)
			jlgr_menubar_draw_);
	}
	// Draw Pre-Rendered
	la_vo_pr_draw(&menu->menubar, 0);
}

/**
 * Check for menubar input.  Should be paired with la_menu_draw().
 * @param jlgr: The library context
**/
void la_menu_loop(la_menu_t* menu) {
	const float mouse_x = la_safe_get_float(&menu->window->mouse_x);
	const float mouse_y = la_safe_get_float(&menu->window->mouse_y);
	const uint8_t selected = (uint8_t) ((1. - mouse_x) / .1);

	for(menu->cursor = 0; menu->cursor < 10; menu->cursor++) {
		// If A NULL function then, stop looping menubar.
		if( !(menu->inputfn[menu->cursor]) ) break;
		// Run the input loop.
		if(menu->cursor == selected && mouse_y < .1)
			((la_menu_fn_t)menu->inputfn[menu->cursor])(menu);
	}
}

void la_menu_drawicon(la_menu_t* menu, uint32_t tex, uint8_t c) {
	jl_rect_t rc_icon = { 0., 0., .1, .1};
	jl_vec3_t tr = { .9 - (.1 * menu->redraw), 0., 0. };

	jlgr_vo_set_image(menu->window, &menu->icon, rc_icon, tex);
	jlgr_vo_txmap(menu->window, &menu->icon, 0, 16, 16, c);
	jlgr_vo_move(&menu->icon, tr);
	jlgr_vo_draw(menu->window, &menu->icon);
}

void la_menu_dont(la_menu_t* menu) { }

/**
 * Add an icon to the menubar
 *
 * @param jlgr: the libary context
 * @param inputfn: The function to run when the icon is / isn't pressed.
 * @param rdr: the function to run when redraw is called.
**/
void la_menu_addicon(la_menu_t* menu, la_menu_fn_t inputfn, la_menu_fn_t rdr) {
	uint8_t i;

	for(i = 0; i < 10; i++) if(!menu->inputfn[i]) break;
	// Set functions for: draw, press, not press
	menu->inputfn[i] = inputfn;
	menu->redrawfn[i] = rdr;
}

/**
 * Add the flip screen icon to the menubar.
 * @param jlgr: the libary context
**/
void la_menu_addicon_flip(la_menu_t* menu) {
	la_menu_addicon(menu, jlgr_menu_flip_press__, jlgr_menu_flip_draw__);	
}

/**
 * Add slowness detector to the menubar.
 * @param jlgr: the libary context
**/
void la_menu_addicon_slow(la_menu_t* menu) {
	la_menu_addicon(menu, jlgr_menu_slow_loop__, jlgr_menu_slow_draw__);
}

/**
 * Add program title to the menubar.
 * @param jlgr: the libary context
**/
void la_menu_addicon_name(la_menu_t* menu) {
	int i;
	la_menu_addicon(menu, la_menu_dont, la_menu_name_drawa__);
	for(i = 0; i < 3; i++)
		la_menu_addicon(menu, la_menu_dont, la_menu_name_drawb__);
	la_menu_addicon(menu, la_menu_dont, la_menu_name_drawc__);
}
