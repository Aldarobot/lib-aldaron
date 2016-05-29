/*
 * JL_Lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLGRmenu.c
 *	This file handles the menubar.
**/
#include "JLGRinternal.h"

typedef struct{
	// Used for all icons on the menubar.
	jl_vo_t* icon;
	// Redraw Functions for 10 icons.
	jlgr_fnct redrawfn[10];
	// Draw thread Cursor
	m_i8_t draw_cursor;
	// Redraw? - 0 = no, 1 = yes
	m_u8_t redraw;
}jl_menu_draw_t;

typedef struct{
	// Pressed & Not Pressed Functions for 10 icons.
	jlgr_input_fnct inputfn[10];
	// Main thread cursor
	m_i8_t main_cursor;
	// Draw context
	jl_menu_draw_t draw;
}jl_menu_main_t;

char *GMessage[3] = {
	"SCREEN: UPPER",
	"SCREEN: LOWER",
	"SCREEN: SINGLE"
};

// Run when the menubar is clicked/pressed
static void _jlgr_menubar_loop_run(jlgr_t* jlgr, jlgr_input_t input) {
	jl_menu_main_t *ctx = jlgr_sprite_getcontext(jlgr->menubar.menubar);
	// Figure out what's selected.
	const uint8_t selected = (uint8_t)((1. - jlgr->main.ct.msx) / .1);

	for( ctx->main_cursor = 0; ctx->main_cursor < 10; ctx->main_cursor++) {
		// If A NULL function then, stop looping menubar.
		if( !(ctx->inputfn[ctx->main_cursor]) ) break;
		// Run the input loop.
		if(ctx->main_cursor == selected && jlgr->main.ct.msy < .1)
			ctx->inputfn[ctx->main_cursor](jlgr, input);
		// If need to redraw run "rdr"
		if(ctx->draw.redraw)
			jlgr_sprite_redraw(jlgr, jlgr->menubar.menubar);
	}
}

static inline void jlgr_menubar_shadow__(jlgr_t* jlgr, jl_menu_draw_t* ctx) {
	// Clear Texture.
	jl_gl_clear(jlgr, 0, 0, 0, 0);
	// Draw Shadows.
	for(ctx->draw_cursor = 0; ctx->draw_cursor < 10; ctx->draw_cursor++) {
		jl_vec3_t tr = { .9 - (.1 * ctx->draw_cursor), 0., 0. };
		jlgr_fnct _draw_icon_=ctx->redrawfn[ctx->draw_cursor];

		if(_draw_icon_ == NULL) break;
		// Draw shadow
		jlgr_draw_vo(jlgr, &(ctx->icon[0]), &tr);
		// Draw Icon
		_draw_icon_(jlgr);
	}
}

// Run whenever a redraw is needed for an icon.
static void jlgr_menubar_draw_(jl_t* jl, uint8_t resize, void* ctx_draw) {
	jlgr_t* jlgr = jl->jlgr;
	jl_menu_draw_t* ctx = ctx_draw;

	// If needed, draw shadow.
	if(ctx->redraw == 2) {
		// Complete redraw of taskbar.
		jlgr_menubar_shadow__(jlgr, ctx);
		// Set redraw = true.
		ctx->redraw = 0;
	}else{
//		JL_PRINT("REDRAW %d\n", ctx->draw_cursor);
		// Redraw only the selected icon.
		if(ctx->redrawfn[ctx->draw_cursor]) {
			JL_PRINT("RRRRRRRRRRRRRRRRRRRRRRRRRRRR\n");
			ctx->redrawfn[ctx->draw_cursor](jlgr);
		}
	}
}

// Runs every frame when menubar is visible.
static void* jlgr_menubar_loop_(jl_t* jl, jl_sprite_t* sprite) {
	jlgr_t* jlgr = jl->jlgr;
	jl_menu_main_t *ctx = jlgr_sprite_getcontext(jlgr->menubar.menubar);

	// Run the proper loops.
	jlgr_input_do(jlgr, JL_CT_PRESS, _jlgr_menubar_loop_run, NULL);
	return &ctx->draw;
}

void jlgr_menubar_init__(jlgr_t* jlgr) {
	jl_rect_t rc = { 0.f, 0.f, 1.f, .11f };
	jl_rect_t rc_icon = { 0., 0., .1, .1};
	jl_rect_t rc_shadow = {-.01, .01, .1, .1 };
	uint8_t shadow_color[] = { 0, 0, 0, 64 };
	jl_menu_main_t menu_main;
	jl_vo_t *icon = jl_gl_vo_make(jlgr, 2);

	// Initialize the context.
	menu_main.draw.redraw = 2;
	// Set the icon & Shadow vertex objects
	menu_main.draw.icon = icon;
	// Make the shadow vertex object.
	jlgr_vos_rec(jlgr, &icon[0], rc_shadow, shadow_color, 0);
	// Make the icon vertex object.
	jlgr_vos_image(jlgr, &icon[1], rc_icon, 0, 1,
		JLGR_ID_UNKNOWN, 255);
	// Clear the menubar & make pre-renderer.
	for( menu_main.draw.draw_cursor = 0; menu_main.draw.draw_cursor < 10;
		menu_main.draw.draw_cursor++)
	{
		menu_main.inputfn[menu_main.draw.draw_cursor] = NULL;
		menu_main.draw.redrawfn[menu_main.draw.draw_cursor] = NULL;
//		jlgr_sprite_redraw(jlgr, jlgr->menubar.menubar);
	}
	menu_main.draw.draw_cursor = -1;

	// Make the menubar.
	jlgr->menubar.menubar = jlgr_sprite_new(
		jlgr, rc, jlgr_menubar_loop_, jlgr_menubar_draw_,
		&menu_main, sizeof(jl_menu_main_t),
		&menu_main.draw, sizeof(jl_menu_draw_t));
	// Redraw menubar.
	// Set the loop.
	jlgr->menubar.menubar->loop = jlgr_menubar_loop_;
}

static void jlgr_menubar_text__(jlgr_t* jlgr, m_u8_t* color, str_t text) {
	jl_menu_main_t* mainctx = jlgr_sprite_getcontext(jlgr->menubar.menubar);
	jl_menu_draw_t* ctx = &(mainctx->draw);
	jl_vec3_t tr = { .9 - (.1 * ctx->draw_cursor), 0., 0. };

	jlgr_draw_text(jlgr, text, tr,
		(jl_font_t) { 0, JL_IMGI_ICON, 0, color, 
			.1 / strlen(text)});
}

static void jlgr_menu_flip_draw__(jlgr_t* jlgr) {
	jlgr_menu_draw_icon(jlgr, 0, JL_IMGI_ICON, JLGR_ID_FLIP_IMAGE);
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

static void jlgr_menu_name_draw2__(jlgr_t* jlgr) {
	jlgr_menu_draw_icon(jlgr, 0, JL_IMGI_ICON, JLGR_ID_UNKNOWN);
}

static void jlgr_menu_name_draw__(jlgr_t* jlgr) {
	jl_menu_main_t* ctx = jlgr_sprite_getcontext(jlgr->menubar.menubar);
	f32_t text_size = jl_gl_ar(jlgr) * .5;

	jlgr_menu_name_draw2__(jlgr);
	jlgr_draw_text(jlgr, jlgr->wm.windowTitle[0],
		(jl_vec3_t) { 1. - (jl_gl_ar(jlgr) * (ctx->draw.draw_cursor+1.)),
			0., 0. },
		(jl_font_t) { 0, JL_IMGI_ICON, 0, jlgr->fontcolor, 
			text_size});
	jlgr_draw_text(jlgr, jlgr->wm.windowTitle[1],
		(jl_vec3_t) { 1. - (jl_gl_ar(jlgr) * (ctx->draw.draw_cursor+1.)),
			text_size, 0. },
		(jl_font_t) { 0, JL_IMGI_ICON, 0, jlgr->fontcolor, 
			text_size});
}

static void _jlgr_menu_slow_draw(jlgr_t* jlgr) {
	jl_t* jl = jlgr->jl;
	m_u8_t color[] = { 255, 255, 255, 255 };

	// Draw the icon based on whether on time or not.
	jlgr_menu_draw_icon(jlgr, 0, JL_IMGI_ICON, jlgr->sg.on_time ?
		JLGR_ID_GOOD_IMAGE : JLGR_ID_SLOW_IMAGE);
	// If not on time report the seconds that passed.
	if(!jlgr->sg.on_time)
		jlgr_menubar_text__(jlgr, color,
			jl_mem_format(jl, "%d fps", jl->time.fps));

	jl_print(jlgr->jl, "MENU.SLOW.DRAW");
}

static void _jlgr_menu_slow_loop(jlgr_t* jlgr, jlgr_input_t input) {
	jl_menu_main_t* ctx = jlgr_sprite_getcontext(jlgr->menubar.menubar);

//	jl_print(jlgr->jl, "MENU>SLOW>LOOP %d", ctx->draw_cursor);
/*	if(jlgr->sg.changed || !jlgr->sg.on_time)*/ ctx->draw.redraw = 1;
}

//
// Exported Functions
//

/**
 * Toggle whether or not to show the menu bar.
 *
 * @param jl: the libary context
**/
void jlgr_menu_toggle(jlgr_t* jlgr) {
	if(jlgr->menubar.menubar->loop == jlgr_sprite_dont)
		jlgr->menubar.menubar->loop = jlgr_menubar_loop_;
	else
		jlgr->menubar.menubar->loop = jlgr_sprite_dont;
}

void jlgr_menu_draw_icon(jlgr_t* jlgr, u16_t g, u16_t i, u8_t c) {
	jl_rect_t rc_icon = { 0., 0., .1, .1};
	jl_menu_main_t* ctx = jlgr_sprite_getcontext(jlgr->menubar.menubar);
	jl_vec3_t tr = { .9 - (.1 * ctx->draw.draw_cursor), 0., 0. };

	jlgr_vos_image(jlgr, &(ctx->draw.icon[1]), rc_icon, g, i, c, 255);
	jlgr_draw_vo(jlgr, &(ctx->draw.icon[1]), &tr);
}

/**
 * Add an icon to the menubar
 *
 * @param jl: the libary context
 * @param inputfn: The function to run when the icon is / isn't pressed.
 * @param rdr: the function to run when redraw is called.
**/
void jlgr_menu_addicon(jlgr_t* jlgr, jlgr_input_fnct inputfn, jlgr_fnct rdr) {
	jl_menu_main_t* ctx = jlgr_sprite_getcontext(jlgr->menubar.menubar);
	m_u8_t i;

	ctx->draw.redraw = 2;
	for(i = 0; i < 10; i++) if(!ctx->inputfn[i]) break;
	// Set functions for: draw, press, not press
	ctx->inputfn[i] = inputfn;
	ctx->draw.redrawfn[i] = rdr;
}

/**
 * Add the flip screen icon to the menubar.
 * @param jl: the libary context
**/
void jlgr_menu_addicon_flip(jlgr_t* jlgr) {
	jlgr_menu_addicon(jlgr, jlgr_menu_flip_press__, jlgr_menu_flip_draw__);	
}

/**
 * Add slowness detector to the menubar.
 * @param jl: the libary context
**/
void jlgr_menu_addicon_slow(jlgr_t* jlgr) {
	jlgr_menu_addicon(jlgr, _jlgr_menu_slow_loop, _jlgr_menu_slow_draw);
}

/**
 * Add program title to the menubar.
 * @param jl: the libary context
**/
void jlgr_menu_addicon_name(jlgr_t* jlgr) {
	int i;
	for(i = 0; i < 4; i++) {
		jlgr_menu_addicon(jlgr, jlgr_input_dont, jlgr_menu_name_draw2__);
	}
	jlgr_menu_addicon(jlgr, jlgr_input_dont, jlgr_menu_name_draw__);
}
