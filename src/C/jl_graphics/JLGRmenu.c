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

// Run when the menubar is clicked/pressed
static void jlgr_menu_loop_press__(jlgr_t* jlgr, jlgr_input_t input) {
	jl_menu_t* menu = input.data;
	// Figure out what's selected.
	const uint8_t selected = (uint8_t)((1. - jlgr->main.ct.msx) / .1);

	for(menu->cursor = 0; menu->cursor < 10; menu->cursor++){
		// If A NULL function then, stop looping menubar.
		if( !(menu->inputfn[menu->cursor]) ) break;
		// Run the input loop.
		if(menu->cursor == selected && jlgr->main.ct.msy < .1)
			menu->inputfn[menu->cursor](jlgr, input);
	}
}

static inline void jlgr_menubar_shadow__(jlgr_t* jlgr, jl_menu_t* menu) {
	// Clear Texture.
	jl_gl_clear(jlgr, 0., 0., 0., 0.);
	// Draw Shadows.
	for(menu->redraw = 0; menu->redraw < 10; menu->redraw++){
		jl_vec3_t tr = { .9 - (.1 * menu->redraw), 0., 0. };
		jlgr_menu_fnct _draw_icon_ = menu->redrawfn[menu->redraw];

		if(_draw_icon_ == NULL) break;
		// Draw shadow
		jlgr_vo_move(&menu->shadow, tr); 
		jlgr_vo_draw(jlgr, &menu->shadow);
		// Draw Icon
		_draw_icon_(jlgr, menu);
	}
}

// Run whenever a redraw is needed for an icon.
static void jlgr_menubar_draw_(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;
	jl_menu_t* menu = jl_thread_pvar_edit(&jlgr->menubar.pvar);

	if(menu->redraw == MENU_REDRAW_ALL) {
		// If needed, draw shadow.
		jlgr_menubar_shadow__(jlgr, menu);
	}else if(menu->redraw != MENU_REDRAW_NONE) {
		// Redraw only the selected icon.
		if(menu->redrawfn[menu->redraw])
			menu->redrawfn[menu->redraw](jlgr, menu);
	}
	// Done.
	menu->redraw = MENU_REDRAW_NONE;
	jl_thread_pvar_drop(&jlgr->menubar.pvar, (void**)&menu);
}

void jlgr_menubar_init__(jlgr_t* jlgr) {
	jl_rect_t rc = { 0.f, 0.f, 1.f, .11f };
	jl_rect_t rc_icon = { 0., 0., .1, .1};
	jl_rect_t rc_shadow = { -.01, .01, .1, .1 };
	float shadow_color[] = { 0.f, 0.f, 0.f, .5f };
	jl_menu_t* menu;

	jl_thread_pvar_init(jlgr->jl, &jlgr->menubar.pvar, NULL,
		sizeof(jl_menu_t));
	menu = jl_thread_pvar_edit(&jlgr->menubar.pvar);

	jlgr_vo_init(jlgr, &menu->icon);
	jlgr_vo_init(jlgr, &menu->shadow);

	// Make the shadow vertex object.
	jlgr_vo_set_rect(jlgr, &menu->shadow, rc_shadow, shadow_color, 0);
	// Make the icon vertex object.
	jlgr_vo_set_image(jlgr, &menu->icon, rc_icon, jlgr->textures.icon);
	jlgr_vo_txmap(jlgr, &menu->icon, 0, 16, 16, JLGR_ID_UNKNOWN);
	// Clear the menubar & make pre-renderer.
	for( menu->cursor = 0; menu->cursor < 10;
		menu->cursor++)
	{
		menu->inputfn[menu->cursor] = NULL;
		menu->redrawfn[menu->cursor] = NULL;
	}
	menu->cursor = -1;
	// Make the menubar.
	jlgr_vo_init(jlgr, &jlgr->menubar.menubar);
	jlgr_vo_rect(jlgr, &jlgr->menubar.menubar, &rc);
	jl_thread_pvar_drop(&jlgr->menubar.pvar, (void**)&menu);

	jlgr_pr(jlgr, &jlgr->menubar.menubar.pr, jlgr_menubar_draw_);
}

static void jlgr_menubar_text__(jlgr_t* jlgr, float* color, float y,
	const char* text, jl_menu_t* menu)
{
	jl_vec3_t tr = { .9 - (.1 * menu->redraw), y, 0. };

	jlgr_text_draw(jlgr, text, tr,
		(jl_font_t) { jlgr->textures.icon, 0, color, 
			.1 / strlen(text)});
}

static void jlgr_menu_flip_draw__(jlgr_t* jlgr, void* menu) {
	jlgr_menu_draw_icon(jlgr,jlgr->textures.icon,JLGR_ID_FLIP_IMAGE,menu);
}

static void jlgr_menu_flip_press__(jlgr_t* jlgr, jlgr_input_t input) {
	if(input.h != JLGR_INPUT_PRESS_JUST) return;
	// Actually Flip the screen.
	if(jlgr->sg.cs == JL_SCR_UP) {
		jlgr->sg.cs = JL_SCR_SS;
	}else if(jlgr->sg.cs == JL_SCR_DN) {
		jlgr->sg.cs = JL_SCR_UP;
	}else{
		jlgr->sg.cs = JL_SCR_DN;
	}
	jlgr_notify(jlgr, GMessage[jlgr->sg.cs]);
	jlgr_resz(jlgr, 0, 0);
}

static void jlgr_menu_name_draw2__(jlgr_t* jlgr, void* menu) {
	jlgr_menu_draw_icon(jlgr, jlgr->textures.icon, JLGR_ID_UNKNOWN, menu);
}

static void jlgr_menu_name_draw__(jlgr_t* jlgr, void* menu2) {
	jl_menu_t* menu = menu2;
	float text_size = jl_gl_ar(jlgr) * .5;

	jlgr_menu_name_draw2__(jlgr, menu);
	jlgr_text_draw(jlgr, jlgr->wm.windowTitle[0],
		(jl_vec3_t) { 1. - (jl_gl_ar(jlgr) * (menu->redraw+1.)),
			0., 0. },
		(jl_font_t) { jlgr->textures.icon, 0, jlgr->fontcolor, 
			text_size});
	jlgr_text_draw(jlgr, jlgr->wm.windowTitle[1],
		(jl_vec3_t) { 1. - (jl_gl_ar(jlgr) * (menu->redraw+1.)),
			text_size, 0. },
		(jl_font_t) { jlgr->textures.icon, 0, jlgr->fontcolor, 
			text_size});
}

static void jlgr_menu_slow_draw__(jlgr_t* jlgr, void* menu) {
	jl_t* jl = jlgr->jl;
	float color[] = { .5, .5, 1., 1. };
	char formated[80];

	// Draw the icon based on whether on time or not.
	jlgr_menu_draw_icon(jlgr, jlgr->textures.icon, jlgr->sg.on_time ?
		JLGR_ID_GOOD_IMAGE : JLGR_ID_SLOW_IMAGE, menu);
	// Report the seconds that passed.
	jl_mem_format(formated, "DrawFPS:%d", (int)(1. / jlgr->psec));
	jlgr_menubar_text__(jlgr, color, 0., formated, menu);
	jl_mem_format(formated, "MainFPS:%d", (int)(1. / jl->time.psec));
	jlgr_menubar_text__(jlgr, color, .05, formated, menu);
}

static void jlgr_menu_slow_loop__(jlgr_t* jlgr, jlgr_input_t input) {
	jl_menu_t* menu = input.data;
	menu->redraw = menu->cursor;	
}

void jlgr_menu_resize_(jlgr_t* jlgr) {
	jl_menu_t* menu = jl_thread_pvar_edit(&jlgr->menubar.pvar);
	menu->cursor = -1;
	jlgr_vo_rect(jlgr, &jlgr->menubar.menubar, NULL);
	jl_thread_pvar_drop(&jlgr->menubar.pvar, (void**)&menu);
}

//
// Exported Functions
//

/**
 * Draw the menu bar.  Should be paired with jlgr_menu_loop().
 * @param jlgr: The library context
 * @param resize: Is window is being resized?
**/
void jlgr_menu_draw(jlgr_t* jlgr, uint8_t resize) {
	uint8_t redraw;

	jl_menu_t* menu = jl_thread_pvar_edit(&jlgr->menubar.pvar);
	if(resize) menu->redraw = MENU_REDRAW_ALL;
	redraw = menu->redraw;
	jl_thread_pvar_drop(&jlgr->menubar.pvar, (void**)&menu);
	// Pre-Render
	if(redraw != MENU_REDRAW_NONE)
		jlgr_pr(jlgr, &jlgr->menubar.menubar.pr, jlgr_menubar_draw_);
	// Draw Pre-Rendered
	jlgr_vo_draw_pr(jlgr, &jlgr->menubar.menubar);
}

/**
 * Check for menubar input.  Should be paired with jlgr_menu_draw().
 * @param jlgr: The library context
**/
void jlgr_menu_loop(jlgr_t* jlgr) {
	jl_menu_t* menu = jl_thread_pvar_edit(&jlgr->menubar.pvar);

	// Run the proper loops.
	jlgr_input_do(jlgr, JL_INPUT_PRESS, jlgr_menu_loop_press__, menu);

	jl_thread_pvar_drop(&jlgr->menubar.pvar, (void**)&menu);
}

void jlgr_menu_draw_icon(jlgr_t* jlgr,uint32_t tex,uint8_t c,jl_menu_t* menu) {
	jl_rect_t rc_icon = { 0., 0., .1, .1};
	jl_vec3_t tr = { .9 - (.1 * menu->redraw), 0., 0. };

	jlgr_vo_set_image(jlgr, &menu->icon, rc_icon, tex);
	jlgr_vo_txmap(jlgr, &menu->icon, 0, 16, 16, c);
	jlgr_vo_move(&menu->icon, tr);
	jlgr_vo_draw(jlgr, &menu->icon);
}

/**
 * Add an icon to the menubar
 *
 * @param jlgr: the libary context
 * @param inputfn: The function to run when the icon is / isn't pressed.
 * @param rdr: the function to run when redraw is called.
**/
void jlgr_menu_addicon(jlgr_t* jlgr, jlgr_input_fnct inputfn, jlgr_menu_fnct rdr) {
	jl_menu_t* menu = jl_thread_pvar_edit(&jlgr->menubar.pvar);
	uint8_t i;

	menu->cursor = -1;
	for(i = 0; i < 10; i++) if(!menu->inputfn[i]) break;
	// Set functions for: draw, press, not press
	menu->inputfn[i] = inputfn;
	menu->redrawfn[i] = rdr;

	jl_thread_pvar_drop(&jlgr->menubar.pvar, (void**)&menu);
}

/**
 * Add the flip screen icon to the menubar.
 * @param jlgr: the libary context
**/
void jlgr_menu_addicon_flip(jlgr_t* jlgr) {
	jlgr_menu_addicon(jlgr, jlgr_menu_flip_press__, jlgr_menu_flip_draw__);	
}

/**
 * Add slowness detector to the menubar.
 * @param jlgr: the libary context
**/
void jlgr_menu_addicon_slow(jlgr_t* jlgr) {
	jlgr_menu_addicon(jlgr, jlgr_menu_slow_loop__, jlgr_menu_slow_draw__);
}

/**
 * Add program title to the menubar.
 * @param jlgr: the libary context
**/
void jlgr_menu_addicon_name(jlgr_t* jlgr) {
	int i;
	for(i = 0; i < 4; i++) {
		jlgr_menu_addicon(jlgr, jlgr_input_dont, jlgr_menu_name_draw2__);
	}
	jlgr_menu_addicon(jlgr, jlgr_input_dont, jlgr_menu_name_draw__);
}
